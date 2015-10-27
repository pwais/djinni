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

#ifndef DJINNIX_JBUFFERARRAYHANDLE_INL_HPP_
#define DJINNIX_JBUFFERARRAYHANDLE_INL_HPP_

#ifndef DJINNIX_JBUFFERARRAYHANDLE_HPP_
#error "Include JBufferArray.hpp instead"
#endif

#pragma once

namespace djinnix {

namespace jni {

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

} /* namespace jni */

inline
JArrayRef JBufferArrayHandle::getArray() const {
  if (jdbb_) {
    JNIEnv *jniEnv = djinni::jniGetThreadEnv();
    DJINNI_ASSERT(jniEnv, jniEnv);

    void *addr = jniEnv->GetDirectBufferAddress(jdbb_.get());
    jlong capacity = jniEnv->GetDirectBufferCapacity(jdbb_.get());
    djinni::jniExceptionCheck(jniEnv);

    return JArrayRef(addr, capacity, true);
  } else {
    return JArrayRef(nullptr, 0, true); // make `isDirect()` consistent
  }
}

inline 
JBufferArrayHandle JBufferArrayHandle::createDirectFascadeFor(
    void *data,
    size_t size) {

  JNIEnv * jniEnv = djinni::jniGetThreadEnv();
  DJINNI_ASSERT(jniEnv, jniEnv);

  JBufferArrayHandle da;
  da.jdbb_ =
    djinni::GlobalRef<jobject>(
      jniEnv,
      jniEnv->NewDirectByteBuffer(data, size));
  djinni::jniExceptionCheck(jniEnv);

  return da;
}

inline
JBufferArrayHandle JBufferArrayHandle::allocateDirectBB(int32_t size) {
  if (size < 0) { return JBufferArrayHandle(); } // Otherwise, Java will throw

  JNIEnv *jniEnv = djinni::jniGetThreadEnv();
  DJINNI_ASSERT(jniEnv, jniEnv);
  
  const auto &data = djinni::JniClass<jni::ByteBufferInfo>::get();

  const jint capacity = size;
  jobject jdbb =
      jniEnv->CallStaticObjectMethod(
          data.clazz.get(),
          data.method_allocate_direct,
          capacity);
  djinni::jniExceptionCheck(jniEnv);
    // Discussion: the DirectByteBuffer class is *not* an official part of
    // a JRE; Android doesn't have it. OpenJDK uses sun.misc.Unsafe
    // to allocate the DirectByteBuffer, tho Unsafe may undergo API
    // changes.  We leverage ByteBuffer#allocateDirect() because it's
    // likely the most stable API.

  JBufferArrayHandle da;
  if (jdbb != NULL) {
    da.byteBufferRef() = djinni::GlobalRef<jobject>(jniEnv, jdbb);
  }
  return da;
}

namespace jni {

/**
 * Djinni type translator for JBufferArrayHandle
 */
struct JBufferArrayHandleTranslator {
  using CppType = JBufferArrayHandle;
  using JniType = jobject;
  
  ///
  /// Translator Impl
  ///
  
  using Boxed = JBufferArrayHandleTranslator;

  static CppType toCpp(JNIEnv* jniEnv, JniType j) {
    DJINNI_ASSERT_MSG(j, jniEnv, "Expected non-null Java instance.");

    JBufferArrayHandle jbb;
    jbb.byteBufferRef() = djinni::GlobalRef<jobject>(jniEnv, j);
      // Don't let the JVM GC the ByteBuffer `jdbb` until the wrapper expires
    return jbb;
  }

  static djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) {
    auto jbbp = c.byteBuffer().get();
    djinni::LocalRef<JniType> j;
    if (jbbp) {
      j = djinni::LocalRef<JniType>{ jniEnv->NewLocalRef(jbbp)) };
    }
    djinni::jniExceptionCheck(jniEnv);
    return j;
  }
};

} /* namespace jni */
} /* namespace djinnix */

#endif /* DJINNIX_JBUFFERARRAYHANDLE_INL_HPP_ */
