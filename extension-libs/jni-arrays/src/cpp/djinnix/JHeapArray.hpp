//
// Copyright 2015 Dropbox, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef DJINNIX_JHEAPARRAY_HPP_
#define DJINNIX_JHEAPARRAY_HPP_

#pragma once

#include <cstdint>

#include "djinni_support.hpp"

namespace djinnix {

class CriticalArray;
class JHeapArray;

namespace jni { struct JHeapArrayTranslator; } /* namespace jni */

/*
 * A *critical* array backed with data from a Java byte[], where
 * the reference is critical because it may block Java GC.
 */
class CriticalArray final {
public:

  ///
  /// Read Access
  ///

  // Direct access to underlying `byte[]` (if the JVM supports zero-copy reads)
  inline void *data() const noexcept { return data_; }
  inline size_t size() const noexcept { return size_; }

  // Is the array either empty or invalid?
  inline bool empty() const noexcept { return !(data_ && jarr_); }

  ///
  /// Write Access
  ///

  // Does this array provide direct access to `byte[]` data?
  // If not (i.e. it wraps a copy), you MUST use the `writeTo()` API below,
  // else writing to `data()` just writes to a temporary copy of the
  // wrapped `byte[]`.
  inline bool isDirect() const noexcept { return is_direct_; }

  // Write data from `src` into the `byte[]` and return the no. bytes written
  inline int32_t writeTo(const void * src, int32_t length) {
    return writeTo(0, src, length);
  }

  // Write data from `src` into `byte[]` starting at position `start`
  // and return the number of bytes written; returns 0 upon error.
  inline int32_t writeTo(int32_t start, const void * src, int32_t length) {

    if (empty()) { return 0; }
      // Don't write if this wrapper is invalid

    if (start + length > size_) { length = size_ - start; }
      // Don't write past the end of the array

    if (length <= 0) { return 0; }
      // Skip empty writes


    if (isDirect()) {
      std::memcpy((uint8_t *)data_ + start, src, length);
    } else {
      JNIEnv * jniEnv = djinni::jniGetThreadEnv();
      DJINNI_ASSERT("Failed to obtain JNI env", jniEnv);

      jniEnv->SetByteArrayRegion(jarr_, start, length, reinterpret_cast<const jbyte*>(src));
      djinni::jniExceptionCheck(jniEnv);
    }

	return length;
  }

  ///
  /// Utils
  ///

  // Forcibly release the underlying critical handle on the reference `byte[]`.
  // Useful if you want to, for example, unblock the JVM GC before this
  // CriticalArray instance expires.
  inline void release() {
    if (!empty()) {
      JNIEnv * jniEnv = djinni::jniGetThreadEnv();
      DJINNI_ASSERT("Failed to obtain JNI env", jniEnv);

      jniEnv->ReleasePrimitiveArrayCritical(jarr_, data_, JNI_ABORT);
      djinni::jniExceptionCheck(jniEnv);
    }

    data_ = nullptr;
    size_ = 0;
    is_direct_ = false;
    jarr_ = nullptr;
  }


  // Only movable
  CriticalArray(const CriticalArray &) = delete;
  CriticalArray &operator=(const CriticalArray &) = delete;

  inline CriticalArray(CriticalArray &&other)
    : data_(other.data_),
      size_(other.size_),
      is_direct_(other.is_direct_),
      jarr_(other.jarr_) {

    other.data_ = nullptr;
    other.size_ = 0;
    other.is_direct_ = false;
    other.jarr_ = nullptr;
  }

  inline CriticalArray &operator=(CriticalArray &&other) {
    if (jarr_ == other.jarr_) {
      // Don't let `other` release() our data
      other.data_ = nullptr;
      other.size_ = 0;
      other.is_direct_ = false;
      other.jarr_ = nullptr;
    } else {
      if (!empty()) { release(); }
      std::swap(data_, other.data_);
      std::swap(size_, other.size_);
      std::swap(is_direct_, other.is_direct_);
      std::swap(jarr_, other.jarr_);
    }
    return *this;
  }

  ~CriticalArray() { release(); }

protected:
  friend class JHeapArray;

  CriticalArray() : data_(nullptr), size_(0), is_direct_(false), jarr_(nullptr) { }

  static CriticalArray createCritical(jbyteArray jarr) {
    CriticalArray arr;

    if (!jarr) { return arr; }

    JNIEnv * jniEnv = djinni::jniGetThreadEnv();
    DJINNI_ASSERT("Failed to obtain JNI env", jniEnv);

    const jsize arr_length = jniEnv->GetArrayLength(jarr);
    if (arr_length == 0) { return arr; }

    jboolean is_copy = true;
    void *data = jniEnv->GetPrimitiveArrayCritical(jarr, &is_copy);
    if (!data) {
      djinni::jniExceptionCheck(jniEnv); // Note any JVM exceptions
      return arr;
    }

    arr.data_ = data;
    arr.size_ = arr_length;
    arr.is_direct_ = !is_copy;
    arr.jarr_ = jarr;

    djinni::jniExceptionCheck(jniEnv); // TODO: remove?
    return arr;
  }

private:
  void *data_;
  size_t size_;
  bool is_direct_;
  jbyteArray jarr_; // Copy of a pointer; a *weak* reference
};



/*
 * Wraps a `byte[]` on the JVM's managed heap and provides read/write
 * access.  This class aims to abstract away JNI complexities and provide
 * best practices for reading/writing to on-heap arrays.  Notes:
 *  * Zero-copy (direct) access is only available on some JVMs (E.g. OpenJDK).
 *     You may need to use the class's write API when direct access is
 *     unavailable.
 *  * Creating and accessing JHeapArray data entail JNI calls, which are
 *     typically slow.  If your use case involves many arrays, try to
 *     concatenate them into a single contiguous array.  Use `ByteBuffer`
 *     in Java and pointers in C++ to reference individual segments.
 */
class JHeapArray final {
public:

  inline bool empty() const noexcept { return jarr_ == nullptr; }

  // Create and return a readable, *critical* reference to the wrapped `byte[]`
  // TODO notes about thread safety, etc
  inline CriticalArray getCritical() const {
    return CriticalArray::createCritical(jarr_.get());
  }

  // TODO: non-critical access and a straight up fast copy might be nice.

  // Have the JVM allocate a `byte[]` of `size` and return a JHeapArray
  // wrapping the created array.
  inline static JHeapArray create(int32_t size) {
    JNIEnv * jniEnv = djinni::jniGetThreadEnv();
    DJINNI_ASSERT("Failed to obtain JNI env", jniEnv);

    JHeapArray ha;
    if (size == 0) { return ha; }

    ha.jarr_ =
      djinni::GlobalRef<jbyteArray>(
        jniEnv,
        jniEnv->NewByteArray(static_cast<jsize>(size)));
    djinni::jniExceptionCheck(jniEnv);

    return ha;
  }

  // Only movable
  JHeapArray(const JHeapArray &) = delete;
  JHeapArray &operator=(const JHeapArray &) = delete;
  JHeapArray(JHeapArray &&other) = default;
  JHeapArray &operator=(JHeapArray &&other) = default;

protected:
  friend struct ::djinnix::jni::JHeapArrayTranslator;

  JHeapArray() { }

  // Wrap the existing `jarr` (a `byte[]`) on the JVM heap and return
  // the created wrapper.
  inline static JHeapArray wrap(JNIEnv *jniEnv, jbyteArray jarr) {
    JHeapArray ha;
    ha.jarr_ = djinni::GlobalRef<jbyteArray>(jniEnv, jarr);
      // Don't let the JVM GC this array until the wrapper expires.
    return ha;
  }

  inline djinni::LocalRef<jbyteArray> createLocalRef(JNIEnv *jniEnv) const {
    return djinni::LocalRef<jbyteArray>(jniEnv, jarr_.get());
  }

private:
  djinni::GlobalRef<jbyteArray> jarr_;
};



namespace jni {

// djinni type translator for JHeapArray
struct JHeapArrayTranslator {
  using CppType = JHeapArray;
  using JniType = jbyteArray;

  struct Boxed {
    using JniType = jobjectArray;
    static CppType toCpp(JNIEnv* jniEnv, JniType j) {
      assert(false); // TODO
      return JHeapArray();
    }

    static djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, CppType c) {
      assert(false); // TODO
      return nullptr;
    }
  };

  static CppType toCpp(JNIEnv* jniEnv, JniType jarr) {
    return JHeapArray::wrap(jniEnv, jarr);
  }

  static djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) {
    return c.createLocalRef(jniEnv);
  }
};

} /* namespace jni */
} /* namespace djinnix */

#endif /* DJINNIX_JHEAPARRAY_HPP_ */

