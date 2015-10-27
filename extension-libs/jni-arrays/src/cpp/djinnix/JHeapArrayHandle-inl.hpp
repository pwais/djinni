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

#ifndef DJINNIX_JHEAPARRAYHANDLE_INL_HPP_
#define DJINNIX_JHEAPARRAYHANDLE_INL_HPP_

#ifndef DJINNIX_JHEAPARRAYHANDLE_HPP_
#error "Include JHeapArrayHandle.hpp instead"
#endif

#pragma once

namespace djinnix {

inline
CriticalArrayRef JHeapArrayHandle::getCritical() const {
  JNIEnv *jniEnv = djinni::jniGetThreadEnv();
  DJINNI_ASSERT(jniEnv, jniEnv);
  return CriticalArrayRef::create(jniEnv, jarr_.get());
}

inline
ArrayElementsRef JHeapArrayHandle::getElements() const {
  JNIEnv *jniEnv = djinni::jniGetThreadEnv();
  DJINNI_ASSERT(jniEnv, jniEnv);
  return ArrayElementsRef::create(jniEnv, jarr_.get());
}

inline
JHeapArrayHandle JHeapArrayHandle::create(int32_t size) {
  JNIEnv *jniEnv = djinni::jniGetThreadEnv();
  DJINNI_ASSERT("Failed to obtain JNI env", jniEnv);

  JHeapArrayHandle ha;
  if (size == 0) { return ha; }

  ha.jarr_ =
    djinni::GlobalRef<jbyteArray>(
      jniEnv,
      jniEnv->NewByteArray(static_cast<jsize>(size)));
  djinni::jniExceptionCheck(jniEnv);

  return ha;
}



inline
void CriticalArrayRef::release() {
  if (jarr_) {
    jniEnv_->ReleasePrimitiveArrayCritical(jarr_, data_, JNI_ABORT);
    djinni::jniExceptionCheck(jniEnv_);
  }

  data_ = nullptr;
  size_ = 0;
  is_direct_ = false;
  jarr_ = nullptr;
  jniEnv_ = nullptr;
}

inline
CriticalArrayRef::CriticalArrayRef(CriticalArrayRef &&other)
  : jarr_(other.jarr_),
    jniEnv_(other.jniEnv_) {

  other.jarr_ = nullptr;
  other.jniEnv_ = nullptr;
  JArrayRef(std::move(other));
}

inline
CriticalArrayRef &CriticalArrayRef::operator=(CriticalArrayRef &&other) {
  if (jarr_ == other.jarr_) {
    // Don't let `other` release() our data
    other.jarr_ = nullptr;
  } else {
    if (jarr_) { release(); }
    std::swap(jarr_, other.jarr_);
  }
  
  std::swap(jniEnv_, other.jniEnv_);
  JArrayRef::operator=(std::move(other));
  return *this;
}

inline
int32_t CriticalArrayRef::writeToIndirect(
      int32_t start,
      const void * src,
      int32_t length) {

  jniEnv_->SetByteArrayRegion(
      jarr_,
      start,
      length,
      reinterpret_cast<const jbyte*>(src));
  djinni::jniExceptionCheck(jniEnv_);

  return length;
}

inline
CriticalArrayRef CriticalArrayRef::create(JNIEnv *jniEnv, jbyteArray jarr) {

  CriticalArrayRef arr;

  if (!jarr) { return arr; }

  const jsize arr_length = jniEnv->GetArrayLength(jarr);
  if (arr_length == 0) { return arr; }

  jboolean is_copy = true;
  void *data = jniEnv->GetPrimitiveArrayCritical(jarr, &is_copy);
  djinni::jniExceptionCheck(jniEnv); // Note any JVM exceptions
  if (!data) { return arr; }

  arr.data_ = data;
  arr.size_ = arr_length;
  arr.is_direct_ = !is_copy;
  arr.jarr_ = jarr;
  arr.jniEnv_ = jniEnv;

  return arr;
}



inline
void ArrayElementsRef::release() {
  if (jarr_) {
    jniEnv_->ReleaseByteArrayElements(jarr_, (jbyte *)data_, 0);
      /*
       * Mode `0`:
       *  * direct: data_ is simply un-pinned; no need to free data_
       *  * non-direct: JVM copies from data_ to the managed heap array
       *      and frees data_ (which is a JVM-allocated temporary)
       *
       * FMI other modes are JNI_{COMMIT,ABORT}
       */
    djinni::jniExceptionCheck(jniEnv_);
  }

  data_ = nullptr;
  size_ = 0;
  is_direct_ = false;
  jarr_ = nullptr;
  jniEnv_ = nullptr;
}

inline
ArrayElementsRef::ArrayElementsRef(ArrayElementsRef &&other)
  : jarr_(other.jarr_),
    jniEnv_(other.jniEnv_) {
  
  other.jarr_ = nullptr;
  other.jniEnv_ = nullptr;
  JArrayRef(std::move(other));
}

inline
ArrayElementsRef &ArrayElementsRef::operator=(ArrayElementsRef &&other) {
  if (jarr_ == other.jarr_) {
    // Don't let `other` release() our data
    other.jarr_ = nullptr;
  } else {
    if (jarr_) { release(); }
    std::swap(jarr_, other.jarr_);
  }
  
  std::swap(jniEnv_, other.jniEnv_);
  JArrayRef::operator=(std::move(other));
  return *this;
}

inline
ArrayElementsRef ArrayElementsRef::create(JNIEnv *jniEnv, jbyteArray jarr) {
  
  ArrayElementsRef arr;

  if (!jarr) { return arr; }

  const jsize arr_length = jniEnv->GetArrayLength(jarr);
  if (arr_length == 0) { return arr; }

  jboolean is_copy = true;
  void *data = jniEnv->GetByteArrayElements(jarr, &is_copy);
  djinni::jniExceptionCheck(jniEnv); // Note any JVM exceptions
  if (!data) { return arr; }

  arr.data_ = data;
  arr.size_ = arr_length;
  arr.is_direct_ = !is_copy;
  arr.jarr_ = jarr;
  arr.jniEnv_ = jniEnv;

  return arr;
}



namespace jni {

/**
 * Djinni type translator for JHeapArrayHandle
 */
struct JHeapArrayHandleTranslator {
  using CppType = JHeapArrayHandle;
  using JniType = jbyteArray;

  // TODO: see if we can / want to support ... Byte[] ?
  struct Boxed {
    using JniType = jobjectArray;
    static CppType toCpp(JNIEnv* jniEnv, JniType j) {
      assert(false); // TODO
      return JHeapArrayHandle();
    }

    static djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, CppType c) {
      assert(false); // TODO
      return nullptr;
    }
  };



  static CppType toCpp(JNIEnv* jniEnv, JniType jarr) {
    DJINNI_ASSERT_MSG(j, jniEnv, "Expected non-null Java instance.");

    JHeapArrayHandle ha;
    ha.jarrRef() = djinni::GlobalRef<jbyteArray>(jniEnv, jarr);
      // Don't let the JVM GC this array until the wrapper expires.
    return ha;
  }

  static djinni::LocalRef<JniType> fromCpp(JNIEnv* jniEnv, const CppType& c) {
    auto jarrp = c.jarr().get();
    djinni::LocalRef<JniType> j;
    if (jarrp) {
      j = djinni::LocalRef<JniType>{ (jbyteArray)jniEnv->NewLocalRef(jarrp)) };
    }
    djinni::jniExceptionCheck(jniEnv);
    return j;
  }

};

} /* namespace jni */
} /* namespace djinnix */

#endif /* DJINNIX_JHEAPARRAYHANDLE_INL_HPP_ */
