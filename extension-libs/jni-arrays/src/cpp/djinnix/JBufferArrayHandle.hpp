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

#ifndef DJINNIX_JBUFFERARRAYHANDLE_HPP_
#define DJINNIX_JBUFFERARRAYHANDLE_HPP_

#pragma once

#include <cstdint>

#include "djinni_support.hpp"

#include "djinnix/JArrayRef.hpp"

namespace djinnix {

class JBufferArrayHandle;

namespace jni {

/**
   * Translates JBufferArrayHandle <-> java.nio.ByteBuffer.
   * Only useful to Djinni internals.
   */
  struct JBufferArrayHandleTranslator;
}

/**
 * Wraps a (direct) java.nio.ByteBuffer-- the ByteBuffer is an object
 * on the JVM heap, but the ByteBuffer references (and owns) an
 * off-JVM-heap data.
 */
class JBufferArrayHandle final {
public:

  inline bool empty() const noexcept { return jdbb_ == nullptr; }

  // Create and return an (unowning) reference to the underlying array
  // NB: if the wrapped ByteBuffer is NOT direct, may return an empty array.
  JArrayRef getArray() const;

  /**
   * Have the JVM reference the native (off-JVM-heap) array at `data` of `size`
   * with a nio.DirectByteBuffer and return a JBufferArrayHandle that serves
   * as a handle to the nio.DirectByteBuffer.  Note that the JBufferArrayHandle
   * does NOT take ownership of the given `data`.
   */
  static JBufferArrayHandle createDirectFascadeFor(void *data, size_t size);

  // TODO: so i guess we have a java struct with a BB and an owner

  /**
   * Have the JVM allocate a (direct) ByteBuffer of `size` and create
   * a JBufferArrayHandle referencing the buffer.  Use this factory if you
   * want to move ownership of memory to the JVM and expose the
   * memory as a ByteBuffer.
   */
  static JBufferArrayHandle allocateDirectBB(int32_t size);

  inline JBufferArrayHandle() noexcept { }

  // Only movable
  JBufferArrayHandle(const JBufferArrayHandle &) = delete;
  JBufferArrayHandle &operator=(const JBufferArrayHandle &) = delete;
  JBufferArrayHandle(JBufferArrayHandle &&other) = default;
  JBufferArrayHandle &operator=(JBufferArrayHandle &&other) = default;

protected:

  ///
  /// Protected API for JBufferArrayHandleTranslator
  ///

  friend struct ::djinnix::jni::JBufferArrayHandleTranslator;

  // Return the GlobalRef pointer; users should not try to take ownership!
  inline djinni::GlobalRef<jobject> &byteBufferRef() { return jdbb_; }
  inline const djinni::GlobalRef<jobject> &byteBuffer() const {
    return jdbb_;
  }

private:
  // The wrapped nio.DirectByteBuffer.  Can own this ref.
  djinni::GlobalRef<jobject> jdbb_;
};

} /* namespace djinnix */

#include "JBufferArrayHandle-inl.hpp"

#endif /* DJINNIX_JBUFFERARRAYHANDLE_HPP_ */

