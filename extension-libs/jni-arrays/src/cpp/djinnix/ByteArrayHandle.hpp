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

#ifndef DJINNIX_BYTEARRAYHANDLE_HPP_
#define DJINNIX_BYTEARRAYHANDLE_HPP_

#pragma once

#include <cstdint>

#include "djinni_support.hpp"

#include "djinnix/ArrayRef.hpp"

namespace djinnix {

class ByteArrayHandle;

namespace jni {
  /**
   * Translates ByteArrayHandle <-> com.dropbox.djinnix.ByteArray.
   * Only useful to Djinni internals.
   */
  struct ByteArrayTranslator;
}

/**
 * TODO
 */
class ByteArrayHandle {
public:

  virtual bool hasArray() const noexcept { return false; }

  virtual ArrayRef getArray() const { return ArrayRef(); }

  ByteArrayHandle() { }

  virtual ~ByteArrayHandle() { }
};

class ByteBufferHandle final : public ByteArrayHandle {
public:

};



class JDirectArray;

namespace jni {

  /**
   * Free memory at `addr` that was allocated using sun.misc.Unsafe.
   * This method actually does a JNI call, which may in turn issue
   * a delete[] or free() (or free some other allocator); we use this
   * technique since the implementation of Unsafe is technically
   * JVM- (and platform-) dependent.
   *
   * Use this method if you own the memory that your `JDirectArray`
   * wraps (i.e. you have moved it from Java to CPP).
   */
  void freeUnsafeAllocated(void *addr);
  void freeUnsafeAllocated(const JDirectArray &jda);

  /**
   * Translates JDirectArray <-> com.dropbox.djinnix.DirectArray.
   * Only useful to Djinni internals.
   */
  struct ByteArrayTranslator;
}



/*
 * TODO: docs, mebbe split unsafe and direct bb into different subclasses..
 * unsafe move assign might be nicer
 */
class JDirectArray final {
public:

  inline bool hasArray() const noexcept {
    return jdbb_ != nullptr || unsafe_data_ != nullptr;
  }

  inline bool isUnsafe() const noexcept { return unsafe_data_ != nullptr; }
  inline bool isDirectBB() const noexcept { return jdbb_ != nullptr; }

  // Create and return an (unowning) reference to the underlying array
  JArrayRef getArray() const;


  /*
   * what about:
   *  * JArrayRef -- keep as-is
   *  * JArrayHandle -- iface that has hasArray() / getArray() / allocate()
   *  * Subclasses: J(Direct)ByteBufferHandle, JUnsafeArrayHandle,
   *       JHeapArrayHandle (has getCritical() and getElements(),
   *        default to safe getElements()).
   *  * freeUnsafeAllocated() for JUnsafeArrayHandle.
   *  * User can cast to get more specific interface, easy to add
   *     subclasses for different types, unified translator
   *
   *  Java-side:
   *    * DirectArray has subclasses for unsafe, byte buffer, use
   *        IsInstanceOf in xlator
   *    * byte[] still xlates as-is to JHeapArrayHandle
   *
   *  Ownership:
   *    * to move, just give across border and forget.  we use Java refs
   *    * to share cpp: i guess use shared ptr to handle ... that's what dij
   *       is doing, right? could we just impl copy ctor that creates new
   *       global ref ? maybe we call GC directly
   *    * to share java: yea using refs to java objects should be ok.
   *        can we unit test the java ref goin out of scope and then cpp
   *        uses the obj?
   *
   *  Future:
   *   * want a way to have a string of memory segments...
   */








  /**
   * Have the JVM wrap the native (off-JVM-heap) array at `data` of `size`
   * with a nio.DirectByteBuffer and return a JDirectArray that serves
   * as a handle to the nio.DirectByteBuffer.  Note that the JDirectArray
   * does NOT take ownership of the given `data` memory.
   */
  static JDirectArray createDirectFascadeFor(void * data, size_t size);

  // TODO: need a owned factory that includes disposer ..

  /**
   * Have the JVM allocate a (direct) ByteBuffer of `size` and create
   * a JDirectArray referencing the buffer.  Use this factory if you
   * want to move ownership of memory to the JVM and expose the
   * memory as a ByteBuffer.
   */
  static JDirectArray allocateDirectBB(int32_t size);

  JDirectArray() : unsafe_data_(nullptr), unsafe_size_(0) { }

  // Only movable
  JDirectArray(const JDirectArray &) = delete;
  JDirectArray &operator=(const JDirectArray &) = delete;
  JDirectArray(JDirectArray &&other);
  JDirectArray &operator=(JDirectArray &&other);

protected:

  ///
  /// Protected API for JDirectArrayTranslator
  ///

  friend struct ::djinnix::jni::JDirectArrayTranslator;

  // Return the GlobalRef pointer; users should not try to take ownership!
  inline djinni::GlobalRef<jobject> &byteBufferRef() { return jdbb_; }
  inline const djinni::GlobalRef<jobject> &byteBuffer() const {
    return jdbb_;
  }

  inline void *&unsafeDataRef() { return unsafe_data_; }
  inline void *unsafeData() const { return unsafe_data_; }

  inline size_t &unsafeSizeRef() { return unsafe_size_; }
  inline size_t unsafeSize() const { return unsafe_size_; }

private:
  // The wrapped nio.DirectByteBuffer.  Can own this ref.
  djinni::GlobalRef<jobject> jdbb_;

  // Wrapped Unsafe array data
  void *unsafe_data_;
  size_t unsafe_size_;
};

} /* namespace djinnix */

#include "JDirectArray-inl.hpp"

#endif /* DJINNIX_BYTEARRAYHANDLE_HPP_ */

