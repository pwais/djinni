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

namespace jni {

  /**
   * Free memory at `addr` that was allocated using sun.misc.Unsafe.
   * This method actually does a JNI call, which may in turn issue
   * a delete[] or free() (or free some other allocator); we use this
   * technique since the implementation of Unsafe is technically
   * JVM-dependent.
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
  struct JDirectArrayTranslator;
}

/*
 * A (weak) reference to an array backed by off-JVM-heap
 * (i.e. unmanaged) memory.
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
 * TODO: docs
 */
class JDirectArray final {
public:

  inline bool hasArray() const noexcept {
    return jdbb_ != nullptr || unsafe_data_ != nullptr;
  }

  inline bool isUnsafe() const noexcept { return unsafe_data_ != nullptr; }

  // Create and return an (unowning) reference to the underlying array
  DirectArrayRef getArray() const;

  /**
   * Have the JVM wrap the native (off-JVM-heap) array at `data` of `size`
   * with a nio.DirectByteBuffer and return a JDirectArray that serves
   * as a handle to the nio.DirectByteBuffer.  Note that the JDirectArray
   * does NOT take ownership of the given `data` memory.
   */
  static JDirectArray createDirectFascadeFor(void * data, size_t size);

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
    : jdbb_(std::move(other.jdbb_)),
      unsafe_data_(nullptr),
      unsafe_size_(0) {
    std::swap(unsafe_data_, other.unsafe_data_);
    std::swap(unsafe_size_, other.unsafe_size_);
  }

  JDirectArray &operator=(JDirectArray &&other) = default;

protected:

  ///
  /// Protected API for JDirectArrayTranslator
  ///

  friend struct ::djinnix::jni::JDirectArrayTranslator;

  // Return the GlobalRef pointer; users should not try to take ownership!
  inline jobject getDirectByteBufferRef() const noexcept { return jdbb_.get(); }

  inline void *getUnsafeData() const noexcept { return unsafe_data_; }
  inline size_t getUnsafeSize() const noexcept { return unsafe_size_; }

private:
  // The wrapped nio.DirectByteBuffer.  Can own this ref.
  djinni::GlobalRef<jobject> jdbb_;

  // Wrapped Unsafe array data
  void *unsafe_data_;
  size_t unsafe_size_;
};

} /* namespace djinnix */

#include "JDirectArray-inl.hpp"

#endif /* DJINNIX_JDIRECTARRAY_HPP_ */

