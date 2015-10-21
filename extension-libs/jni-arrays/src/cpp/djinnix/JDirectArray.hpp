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

  inline bool hasArray() const noexcept {
    return jdbb_ != nullptr || unsafe_data_ != nullptr;
  }

  // Create and return an (unowning) reference to the underlying array
  inline DirectArrayRef getArray() const {
    if (jdbb_) {
      JNIEnv * jniEnv = djinni::jniGetThreadEnv();
      DJINNI_ASSERT(jniEnv, jniEnv);

      void *addr = jniEnv->GetDirectBufferAddress(jdbb_.get());
      jlong capacity = jniEnv->GetDirectBufferCapacity(jdbb_.get());
      djinni::jniExceptionCheck(jniEnv);

      return DirectArrayRef(addr, capacity);
    } else if (unsafe_data_) {
      return DirectArrayRef(unsafe_data_, unsafe_size_);
    } else {
      return DirectArrayRef();
    }
  }

  /**
   * Have the JVM wrap the native (off-JVM-heap) array at `data` of `size`
   * with a nio.DirectByteBuffer and return a JDirectArray that serves
   * as a handle to the nio.DirectByteBuffer.  Note that the JDirectArray
   * does NOT take ownership of the given `data` memory.
   */
  inline static JDirectArray createDirectFascadeFor(void * data, size_t size) {
    JNIEnv * jniEnv = djinni::jniGetThreadEnv();
    DJINNI_ASSERT(jniEnv, jniEnv);

    JDirectArray da;
    da.jdbb_ =
      djinni::GlobalRef<jobject>(
        jniEnv,
        jniEnv->NewDirectByteBuffer(data, size));
    djinni::jniExceptionCheck(jniEnv);

    return da;
  }

  /**
   * Have the JVM allocate a (direct) ByteBuffer of `size` and create
   * a JDirectArray referencing the buffer.  Use this factory if you
   * want to move ownership of memory to the JVM and expose the
   * memory as a ByteBuffer.
   */
  inline static JDirectArray allocateDirectBB(int32_t size) {
    if (size < 0) { return JDirectArray(); } // Otherwise, Java will throw

    JNIEnv *jniEnv = djinni::jniGetThreadEnv();
    DJINNI_ASSERT(jniEnv, jniEnv);

    const auto &data = djinni::JniClass<ByteBufferInfo>::get();

    const jint capacity = size;
    jobject jdbb =
        jniEnv->CallStaticObjectMethod(
            data.clazz.get(),
            data.method_allocate_direct,
            capacity);
    djinni::jniExceptionCheck(jniEnv);
      // NB: the DirectByteBuffer class is *not* an official part of
      // a JRE; Android doesn't have it. OpenJDK uses sun.misc.Unsafe
      // to allocate the DirectByteBuffer, tho Unsafe may undergo API
      // changes.  We leverage ByteBuffer#allocateDirect() because it's
      // likely the most stable API.  Performance isn't great, but
      // we'd like for users to allocate fewer, larger buffers if possible.

    JDirectArray da;
    if (jdbb != NULL) {
      da = JDirectArray::wrapDirectByteBuffer(jniEnv, jdbb);
    }
    return da;
  }

  // Only movable
  JDirectArray(const JDirectArray &) = delete;
  JDirectArray &operator=(const JDirectArray &) = delete;
  JDirectArray(JDirectArray &&other) = default;
  JDirectArray &operator=(JDirectArray &&other) = default;

  JDirectArray()
    : unsafe_data_(nullptr),
      unsafe_size_(0)
  { }

protected:
  friend struct ::djinnix::jni::JDirectArrayTranslator;

  // Wrap the djinnix.DirectArray instance `j` on the JVM heap
  // and return the created wrapper.
  inline static JDirectArray wrapDirectByteBuffer(JNIEnv *jniEnv, jobject j) {
    JDirectArray da;
    da.jdbb_ = djinni::GlobalRef<jobject>(jniEnv, j);
      // Don't let the JVM GC this array until the wrapper expires.
    return da;
  }

  struct UnsafeArrayInfo {
    const djinni::GlobalRef<jclass> clazz {
      djinni::jniFindClass("com/dropbox/djinnix/UnsafeArray")
    };

    /*
     * NB: For JNI symbols, try
     *   $ javap -classpath build/classes/ -s com.dropbox.djinnix.DirectArray
     */

    const jfieldID field_address {
      djinni::jniGetFieldID(clazz.get(), "address", "J")
    };
    const jfieldID field_size {
      djinni::jniGetFieldID(clazz.get(), "size", "J")
    };
  };


  // Wrap the (address, size) from a `djinnix.UnsafeArray` and
  // return the created wrapper.  *Never owns the array*.
  inline static JDirectArray wrapUnsafeArray(JNIEnv *jniEnv, jobject j) {
    JDirectArray da;

    const auto& unsafe_data = djinni::JniClass<UnsafeArrayInfo>::get();
    da.unsafe_data_ = jniEnv->GetLongField(j, unsafe_data.field_address);
    da.unsafe_size_ = jniEnv->GetLongField(j, unsafe_data.field_size);
    djinni::jniExceptionCheck(jniEnv);

    return da;
  }

  // Return the GlobalRef pointer; users should not try to take ownership!
  inline jobject getDirectByteBufferRef() const noexcept { return jdbb_.get(); }

  inline void *getUnsafeData() const noexcept { return unsafe_data_; }
  inline size_t getUnsafeSize() const noexcept { return unsafe_size_; }

  struct ByteBufferInfo {
    const djinni::GlobalRef<jclass> clazz {
      djinni::jniFindClass("java/nio/ByteBuffer")
    };
    const jmethodID method_allocate_direct {
      djinni::jniGetStaticMethodID(
          clazz.get(),
          "allocateDirect",
          "(I)Ljava/nio/ByteBuffer;")
    };
  };

private:
  // The wrapped nio.DirectByteBuffer.  Can own this ref.
  djinni::GlobalRef<jobject> jdbb_;

  // Wrapped Unsafe array data
  void *unsafe_data_;   // Weak, *copyable*
  size_t unsafe_size_;
};


namespace jni {

// TODO: NOT inline, add docs
inline void freeUnsafeAllocated(void *addr) {
  if (!addr) { return; }

  JNIEnv *jniEnv = djinni::jniGetThreadEnv();
  DJINNI_ASSERT("Failed to obtain JNI env", jniEnv);

  struct UnsafeArrayUnsafeSupportInfo {
    const djinni::GlobalRef<jclass> clazz {
      djinni::jniFindClass("com/dropbox/djinnix/UnsafeArray$UnsafeSupport")
    };
    const jmethodID method_free_memory {
      djinni::jniGetStaticMethodID(
        clazz.get(),
        "freeMemory",
        "(J)V")
    };
  };

  const auto &unsafe_data =
    djinni::JniClass<UnsafeArrayUnsafeSupportInfo>::get();

  jniEnv->CallStaticObjectMethod(
    unsafe_data.clazz.get(),
    unsafe_data.method_free_memory,
    addr);
  djinni::jniExceptionCheck(jniEnv);
}

// djinni type translator for JDirectArray
struct JDirectArrayTranslator {
  using CppType = JDirectArray;
  using JniType = jobject;

  struct DirectArrayInfo {
    const djinni::GlobalRef<jclass> clazz {
      djinni::jniFindClass("com/dropbox/djinnix/DirectArray")
    };

    /*
     * NB: For JNI symbols, try
     *   $ javap -classpath build/classes/ -s com.dropbox.djinnix.DirectArray
     */

    const jmethodID method_wrap_byte_buffer {
      djinni::jniGetStaticMethodID(
        clazz.get(),
        "wrap",
        "(Ljava/nio/ByteBuffer;)Lcom/dropbox/djinnix/DirectArray;")
    };
    const jmethodID method_wrap_address_size {
      djinni::jniGetStaticMethodID(
        clazz.get(),
        "wrap",
        "(JJ)Lcom/dropbox/djinnix/DirectArray;")
    };
    const jmethodID method_get_byte_buffer {
      djinni::jniGetMethodID(
        clazz.get(),
        "getByteBuffer",
        "()Ljava/nio/ByteBuffer;")
    };
    const jmethodID method_unsafe_array {
      djinni::jniGetMethodID(
        clazz.get(),
        "getUnsafeArray",
        "()Lcom/dropbox/djinnix/UnsafeArray;")
    };
  };

  using Boxed = JDirectArrayTranslator;

  static CppType toCpp(JNIEnv* jniEnv, JniType j) {
    DJINNI_ASSERT_MSG(j, jniEnv, "Expected non-null Java instance.");
    const auto &data = djinni::JniClass<DirectArrayInfo>::get();
    DJINNI_ASSERT_MSG(
      jniEnv->IsInstanceOf(j, data.clazz.get()),
      jniEnv,
      "Instance is of wrong class type.");

    // Does the DirectArray wrap a DirectByteBuffer?
    jobject jdbb =
        jniEnv->CallObjectMethod(j, data.method_get_byte_buffer);
    djinni::jniExceptionCheck(jniEnv);
    if (jdbb != NULL) {
      return JDirectArray::wrapDirectByteBuffer(jniEnv, jdbb);
    }

    // Does DirectArray wrap an UnsafeArray?
    jobject jua =
        jniEnv->CallObjectMethod(j, data.method_unsafe_array);
    if (jua != NULL) {

    }

    return JDirectArray();
  }

  static djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) {
    const auto& data = djinni::JniClass<DirectArrayInfo>::get();
    djinni::LocalRef<jobject> j;

    // First check Unsafe, because otherwise we'll just create a DirectArray
    // with a null ByteBuffer
    if (c.getUnsafeData()) {
      j = djinni::LocalRef<jobject>(
                jniEnv,
                jniEnv->CallStaticObjectMethod(
                  data.clazz.get(),
                  data.method_wrap_address_size,
                  c.getUnsafeData(),
                  c.getUnsafeSize()));
    } else {
      j = djinni::LocalRef<jobject>(
                jniEnv,
                jniEnv->CallStaticObjectMethod(
                  data.clazz.get(),
                  data.method_wrap_byte_buffer,
                  c.getDirectByteBufferRef()));
    }

    djinni::jniExceptionCheck(jniEnv);
    return j;
  }
};

} /* namespace jni */
} /* namespace djinnix */

#endif /* DJINNIX_JDIRECTARRAY_HPP_ */

