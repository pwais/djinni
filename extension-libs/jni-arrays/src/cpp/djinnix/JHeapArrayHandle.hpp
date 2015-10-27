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

#ifndef DJINNIX_JHEAPARRAYHANDLE_HPP_
#define DJINNIX_JHEAPARRAYHANDLE_HPP_

#pragma once

#include <cstdint>

#include "djinni_support.hpp"

#include "djinnix/JArrayRef.hpp"

namespace djinnix {

class ArrayElementsRef;
class CriticalArrayRef;
class JHeapArrayHandle;

namespace jni { struct JHeapArrayHandleTranslator; }

/**
 * Wraps a `byte[]` on the JVM's managed heap and provides read/write
 * access.  This class aims to abstract away JNI complexities and provide
 * best practices for reading/writing to on-heap arrays.  Notes:
 *  * Zero-copy (direct) access is only available on some JVMs (E.g. OpenJDK).
 *     You may need to use the JArrayRef write API when direct access is
 *     unavailable.
 *  * Creating and accessing JHeapArray data entail JNI calls, which are
 *     typically slow.  If your use case involves many arrays, try to
 *     concatenate them into a single contiguous array.  Use `ByteBuffer`
 *     in Java and pointers in C++ to reference individual segments.
 */
class JHeapArrayHandle final {
public:

  inline bool empty() const noexcept { return jarr_ == nullptr; }

  // Create and return a *critical* reference to the wrapped `byte[]`
  CriticalArrayRef getCritical() const;

  // Create and return a standard (safer) reference to the wrapped `byte[]`
  ArrayElementsRef getElements() const;

  // Have the JVM allocate a `byte[]` of `size` and return a JHeapArray
  // wrapping the created array.
  static JHeapArrayHandle create(int32_t size);

  // Have the JVM allocate a `byte[]` of `size` containing a copy of the
  // given `data` (of `size` bytes)
  inline static JHeapArrayHandle makeCopy(const void *data, int32_t size) {
    auto jah = create(size);
    jah.getCritical().writeTo(data, size);
    return jah;
  }

  // Only movable
  JHeapArrayHandle(const JHeapArrayHandle &) = delete;
  JHeapArrayHandle &operator=(const JHeapArrayHandle &) = delete;
  JHeapArrayHandle(JHeapArrayHandle &&other) = default;
  JHeapArrayHandle &operator=(JHeapArrayHandle &&other) = default;

  JHeapArrayHandle() { }

protected:

  ///
  /// Protected API for JDirectArrayTranslator
  ///

  friend struct ::djinnix::jni::JHeapArrayHandleTranslator;

  inline djinni::GlobalRef<jbyteArray> &jarrRef() { return jarr_; }
  inline const djinni::GlobalRef<jbyteArray> &jarr() const { return jarr_; }

private:
  djinni::GlobalRef<jbyteArray> jarr_;
};



// ===========================================================================
// Array Reference types

/**
 * A *critical* array backed with data from a Java byte[];
 * the reference is *critical* because it may block Java GC
 * and is not necessarily thread-safe.  Provides the most
 * performant access but carries some constraints.
 *
 * In particular, JNI *REQUIRES* that this `CriticalArrayRef` be used in
 * the same thread (and during the same JNI call) as the parent `JHeapArray`
 * that created this instance.  Otherwise, the JVM is free to invalidate the
 * pointer (e.g. through a move of the underlying array data).
 */
class CriticalArrayRef final : public JArrayRef {
public:

  /**
   * Forcibly release the underlying critical handle on the referenced
   * `byte[]`. Useful if you want to, for example, unblock the JVM GC before
   * this CriticalArrayRef instance expires.
   */
  void release();

  // Only movable, since we release() upon dealloc
  CriticalArrayRef(CriticalArrayRef &&other) noexcept;
  CriticalArrayRef &operator=(CriticalArrayRef &&other) noexcept;

  ~CriticalArrayRef() { release(); }

protected:

  // Copy to array via JNI call
  int32_t writeToIndirect(
      int32_t start,
      const void *src,
      int32_t length) override;

  ///
  /// Protected JHeapArray Interface
  ///

  friend class JHeapArrayHandle;

  inline CriticalArrayRef()  // Only JHeapArrayHandle-constructible
    : JArrayRef(),
      jarr_(nullptr),
      jniEnv_(nullptr)
  { }

  static CriticalArrayRef create(JNIEnv *jniEnv, jbyteArray jarr);

private:
  jbyteArray jarr_;
  JNIEnv *jniEnv_;
};



/**
 * An array backed with data from a Java byte[]. This reference is safer
 * than `CriticalArrayRef` as it may be passed between threads and
 * persists through JNI calls. Uses the `{Get,Release}ByteArrayElements()`
 * API.
 *
 * Note the ref is *unlikely* to be zero-copy unless the JVM supports
 * memory pinning.
 */
class ArrayElementsRef final : public JArrayRef {
public:

  // Forcibly release the underlying handle on the referenced `byte[]`
  // (and write back any changes to the `byte[]`).
  void release();

  // Only movable
  ArrayElementsRef(ArrayElementsRef &&other) noexcept;
  ArrayElementsRef &operator=(ArrayElementsRef &&other) noexcept;

  ~ArrayElementsRef() { release(); }

protected:

  /*
   * NB: No need to override `writeToIndirect()` because
   * we use the `ReleaseByteArrayElements()` API,
   * which automatically copies data from our local
   * temporary if we were indeed given a temporary.
   */

  ///
  /// Protected JHeapArray Interface
  ///
  friend class JHeapArrayHandle;

  inline ArrayElementsRef()  // Only JHeapArrayHandle-constructible
    : JArrayRef(),
      jarr_(nullptr),
      jniEnv_(nullptr)
  { }

  static ArrayElementsRef create(JNIEnv *jniEnv, jbyteArray jarr);

private:
  jbyteArray jarr_;
  JNIEnv *jniEnv_;
};

} /* namespace djinnix */

#include "JHeapArray-inl.hpp"

#endif /* DJINNIX_JHEAPARRAYHANDLE_HPP_ */

