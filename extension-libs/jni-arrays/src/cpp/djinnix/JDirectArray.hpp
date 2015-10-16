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

#ifndef DJINNIX_JDIRECTARRAY_HPP_
#define DJINNIX_JDIRECTARRAY_HPP_

#pragma once

#include <cstdint>

#include "djinni_support.hpp"

namespace djinnix {

class DirectArrayRef;
class JDirectArray;

namespace jni { struct JDirectArrayTranslator; } /* namespace jni */

/*
 * A (weak) reference to an array backed by off-JVM-heap (i.e. unmanaged) memory.
 */
class DirectArrayRef final {
public:

  // Direct access to underlying data
  inline void * data() const noexcept { return data_; }
  inline size_t size() const noexcept { return size_; }

  // Is the array either empty or invalid?
  inline bool empty() const noexcept { return !(data_ && size_ > 0); }

  // Fully copyable and movable -- not an owning reference
  DirectArrayRef(const DirectArrayRef &) = default;
  DirectArrayRef &operator=(const DirectArrayRef &) = default;
  DirectArrayRef(DirectArrayRef &&other) = default;
  DirectArrayRef &operator=(DirectArrayRef &&other) = default;

protected:
  friend class JDirectArray;

  DirectArrayRef() : data_(nullptr), size_(0) { }

  explicit DirectArrayRef(void * data, size_t size)
    : data_(data), size_(size)
  { }

private:
  void *data_;
  size_t size_;
};



/*
 * TODO: docs and support for Unsafe-allocated
 */
class JDirectArray final {
public:

  inline bool empty() const noexcept { return jdbb_ == nullptr; }

  // Create and return an (unowning) reference to the underlying array
  inline DirectArrayRef getArray() const {
    if (empty()) {
      return DirectArrayRef();
    } else {
      JNIEnv * jniEnv = djinni::jniGetThreadEnv();
      DJINNI_ASSERT("Failed to obtain JNI env", jniEnv);

      void *addr = jniEnv->GetDirectBufferAddress(jdbb_.get());
      jlong capacity = jniEnv->GetDirectBufferCapacity(jdbb_.get());
      djinni::jniExceptionCheck(jniEnv);

      return DirectArrayRef(addr, capacity);
    }
    // TODO: branch for Unsafe?
  }

  // Have the JVM wrap the native (off-JVM-heap) array at `data` of `size`
  // with a nio.DirectByteBuffer and return a JDirectArray that serves
  // as a handle to the nio.DirectByteBuffer.  Note that the JDirectArray
  // does NOT take ownership of the given `data` memory.
  inline static JDirectArray createDirectFascadeFor(void * data, size_t size) {
    // Prevent invalid JVM calls
    if (!data || size == 0) { return JDirectArray(); }

    JNIEnv * jniEnv = djinni::jniGetThreadEnv();
    DJINNI_ASSERT("Failed to obtain JNI env", jniEnv);

    JDirectArray da;
    da.jdbb_ =
      djinni::GlobalRef<jobject>(
        jniEnv,
        jniEnv->NewDirectByteBuffer(data, size));
    djinni::jniExceptionCheck(jniEnv);

    return da;
  }

  // Only movable
  JDirectArray(const JDirectArray &) = delete;
  JDirectArray &operator=(const JDirectArray &) = delete;
  JDirectArray(JDirectArray &&other) = default;
  JDirectArray &operator=(JDirectArray &&other) = default;

protected:
  friend struct ::djinnix::jni::JDirectArrayTranslator;

  JDirectArray() { }

  // Wrap the djinnix.DirectArray instance `j` on the JVM heap
  // and return the created wrapper.
  inline static JDirectArray wrapDirectByteBuffer(JNIEnv *jniEnv, jobject j) {
    JDirectArray da;
    da.jdbb_ = djinni::GlobalRef<jobject>(jniEnv, j);
      // Don't let the JVM GC this array until the wrapper expires.
    return da;
  }

  inline djinni::LocalRef<jobject> getDirectByteBufferLocalRef(JNIEnv *jniEnv) const {
    return djinni::LocalRef<jobject>(jniEnv, jdbb_.get());
  }

private:
  // The wrapped nio.DirectByteBuffer
  djinni::GlobalRef<jobject> jdbb_;

  // TODO: wrap unsafe
};


namespace jni {

// djinni type translator for JDirectArray
struct JDirectArrayTranslator {
  using CppType = JDirectArray;
  using JniType = jobject;

  struct DirectArrayInfo {
    const djinni::GlobalRef<jclass> clazz { djinni::jniFindClass("com/dropbox/djinnix/DirectArray") };
    const jmethodID method_get_as_direct_byte_buffer {
      djinni::jniGetMethodID(clazz.get(), "getAsDirectByteBuffer", "()Ljava/nio/ByteBuffer;")
    };
    const jmethodID method_wrap_byte_buffer {
      djinni::jniGetStaticMethodID(
	    clazz.get(),
		"wrap",
		"(Ljava/nio/ByteBuffer;)Lcom/dropbox/djinnix/DirectArray;")
    };
  };

  using Boxed = JDirectArrayTranslator;

  static CppType toCpp(JNIEnv* jniEnv, JniType j) {
    assert(j != nullptr); // TODO: eliable asserts ?
    const auto &data = djinni::JniClass<DirectArrayInfo>::get();
    assert(jniEnv->IsInstanceOf(j, data.clazz.get())); // TODO: eliable asserts ?

    // Does the DirectArray wrap a DirectByteBuffer?
    jobject jdbb = jniEnv->CallObjectMethod(j, data.method_get_as_direct_byte_buffer);
    djinni::jniExceptionCheck(jniEnv);
    if (jdbb != NULL) {
      return JDirectArray::wrapDirectByteBuffer(jniEnv, jdbb);
    }

    return JDirectArray();
  }

  static djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) {
    auto dbb_ref = c.getDirectByteBufferLocalRef(jniEnv);
    if (dbb_ref) {
      const auto& data = djinni::JniClass<DirectArrayInfo>::get();
      auto j =
        djinni::LocalRef<jobject>(
          jniEnv,
          jniEnv->CallStaticObjectMethod(data.clazz.get(), data.method_wrap_byte_buffer, *dbb_ref));
      djinni::jniExceptionCheck(jniEnv);
      return j;
    }

    return nullptr;
  }
};

} /* namespace jni */
} /* namespace djinnix */

#endif /* DJINNIX_JDIRECTARRAY_HPP_ */

