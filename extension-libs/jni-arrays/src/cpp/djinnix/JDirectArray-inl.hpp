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

#ifndef DJINNIX_JDIRECTARRAY_INL_HPP_
#define DJINNIX_JDIRECTARRAY_INL_HPP_

#ifndef DJINNIX_JDIRECTARRAY_HPP_
#error "Include JDirectArray.hpp instead"
#endif

#pragma once

namespace djinnix {

inline
JArrayRef JDirectArray::getArray() const {
  if (jdbb_) {
    JNIEnv *jniEnv = djinni::jniGetThreadEnv();
    DJINNI_ASSERT(jniEnv, jniEnv);

    void *addr = jniEnv->GetDirectBufferAddress(jdbb_.get());
    jlong capacity = jniEnv->GetDirectBufferCapacity(jdbb_.get());
    djinni::jniExceptionCheck(jniEnv);

    return JArrayRef(addr, capacity, true);
  } else if (unsafe_data_) {
    return JArrayRef(unsafe_data_, unsafe_size_, true);
  } else {
    return JArrayRef(nullptr, 0, true); // make `isDirect()` consistent
  }
}

inline 
JDirectArray JDirectArray::createDirectFascadeFor(void *data, size_t size) {
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

inline
JDirectArray JDirectArray::allocateDirectBB(int32_t size) {
  if (size < 0) { return JDirectArray(); } // Otherwise, Java will throw

  JNIEnv *jniEnv = djinni::jniGetThreadEnv();
  DJINNI_ASSERT(jniEnv, jniEnv);

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
    da.byteBufferRef() = djinni::GlobalRef<jobject>(jniEnv, jdbb);
  }
  return da;
}

inline JDirectArray::JDirectArray(JDirectArray &&other)
  : jdbb_(std::move(other.jdbb_)),
    unsafe_data_(nullptr),
    unsafe_size_(0) {
  std::swap(unsafe_data_, other.unsafe_data_);
  std::swap(unsafe_size_, other.unsafe_size_);
    // Move the unsafe_data_ member to avoid accidental user double-free
}

inline JDirectArray &JDirectArray::operator=(JDirectArray &&other) {
  if (jdbb_ != other.jdbb_) { jdbb_ = std::move(other.jdbb_); }
  
  unsafe_data_ = other.unsafe_data_;
  unsafe_size_ = other.unsafe_size_;
    // NB: May leak `this`'s data! But user is responsible
    // for ownership management; we don't know if `this` owns
    // `unsafe_data_`.
  
  other.unsafe_data_ = nullptr;
  other.unsafe_size_ = 0;
    // Do try to prevent accidental user double-free
  
  return *this;
}

namespace jni {

inline
void freeUnsafeAllocated(void *addr) {
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

inline
void freeUnsafeAllocated(JDirectArray &jda) {
  if (jda.isUnsafe()) {
    auto array_ref = jda.getArray();
    freeUnsafeAllocated(array_ref.data());
  }
}



/**
 * Djinni type translator for JDirectArray
 */
struct JDirectArrayTranslator {
  using CppType = JDirectArray;
  using JniType = jobject;

  ///
  /// Java Element Data
  ///   NB: For JNI symbols, try
  ///   $ javap -classpath build/classes/ -s com.dropbox.djinnix.DirectArray
  ///
    
  struct DirectArrayInfo {
    const djinni::GlobalRef<jclass> clazz {
      djinni::jniFindClass("com/dropbox/djinnix/DirectArray")
    };
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

  struct UnsafeArrayInfo {
    const djinni::GlobalRef<jclass> clazz {
      djinni::jniFindClass("com/dropbox/djinnix/UnsafeArray")
    };
    const jfieldID field_address {
      djinni::jniGetFieldID(clazz.get(), "address", "J")
    };
    const jfieldID field_size {
      djinni::jniGetFieldID(clazz.get(), "size", "J")
    };
  };
  
  
  
  ///
  /// Translator Impl
  ///
  
  using Boxed = JDirectArrayTranslator;

  static CppType toCpp(JNIEnv* jniEnv, JniType j) {
    DJINNI_ASSERT_MSG(j, jniEnv, "Expected non-null Java instance.");
    const auto &data = djinni::JniClass<DirectArrayInfo>::get();
    DJINNI_ASSERT_MSG(
      jniEnv->IsInstanceOf(j, data.clazz.get()),
      jniEnv,
      "Instance is of wrong class type.");

    // Does the DirectArray wrap a DirectByteBuffer?
    jobject jdbb = jniEnv->CallObjectMethod(j, data.method_get_byte_buffer);
    djinni::jniExceptionCheck(jniEnv);
    if (jdbb != NULL) {
      JDirectArray da;
      da.byteBufferRef() = djinni::GlobalRef<jobject>(jniEnv, j);
        // Don't let the JVM GC the ByteBuffer `j` until the wrapper expires.
      return da;
    }

    // Does DirectArray wrap an UnsafeArray?
    jobject jua = jniEnv->CallObjectMethod(j, data.method_unsafe_array);
    if (jua != NULL) {
      const auto& unsafe_data = djinni::JniClass<UnsafeArrayInfo>::get();
      
      JDirectArray da;
      da.unsafeDataRef() =
        (void *) jniEnv->GetLongField(j, unsafe_data.field_address);
      da.unsafeSizeRef() = jniEnv->GetLongField(j, unsafe_data.field_size);
      djinni::jniExceptionCheck(jniEnv);
      
      return da;
    }

    return JDirectArray();
  }

  static djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) {
    const auto& data = djinni::JniClass<DirectArrayInfo>::get();
    djinni::LocalRef<jobject> j;

    // First check Unsafe, because otherwise we'll just create a DirectArray
    // that *may* have a null ByteBuffer
    if (c.unsafeData()) {
      j = djinni::LocalRef<jobject>(
            jniEnv,
            jniEnv->CallStaticObjectMethod(
              data.clazz.get(),
              data.method_wrap_address_size,
              c.unsafeData(),
              c.unsafeSize()));
    } else {
      j = djinni::LocalRef<jobject>(
            jniEnv,
            jniEnv->CallStaticObjectMethod(
              data.clazz.get(),
              data.method_wrap_byte_buffer,
              c.byteBuffer().get()));
                // NB: `c` is empty <=> `j` is empty too, null ByteBuffer
                // is safe for `method_wrap_byte_buffer`
    }

    djinni::jniExceptionCheck(jniEnv);
    return j;
  }
};

} /* namespace jni */
} /* namespace djinnix */

#endif /* DJINNIX_JDIRECTARRAY_INL_HPP_ */