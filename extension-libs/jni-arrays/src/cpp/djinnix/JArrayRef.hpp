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

#ifndef DJINNIX_JARRAYREF_HPP_
#define DJINNIX_JARRAYREF_HPP_

namespace djinnix {

/**
 * A *non-owning* reference to a Java array. Provides a single read/write API
 * for a variety of Java array types.
 */
class JArrayRef {
public:

  // Is the array either empty or invalid?
  inline bool empty() const noexcept { return size_ == 0; }
  
  // Does this array provide direct access to the Java array?
  // If not (i.e. it wraps a copy), you MUST use the `writeTo()` API below,
  // else writing to `data()` would just write to a temporary copy of the
  // Java array data.  
  inline bool isDirect() const noexcept { return is_direct_; }
  
  ///
  /// Read Access
  ///
  
  inline void *data() const noexcept { return data_; }
  inline size_t size() const noexcept { return size_; }
  
  ///
  /// Write Access
  /// 

  // Write data from `src` into the `byte[]` and return the number of
  // bytes written.
  inline int32_t writeTo(const void *src, int32_t length) {
    return writeTo(0, src, length);
  }

  // Write data from `src` into `byte[]` starting at position `start`
  // and return the number of bytes written; returns 0 upon error.
  int32_t writeTo(int32_t start, const void *src, int32_t length);
  
  // Only movable; some subclasses are non-copyable
  JArrayRef(const JArrayRef &) = delete;
  JArrayRef &operator=(const JArrayRef &) = delete;
  JArrayRef(JArrayRef &&other) noexcept;
  JArrayRef &operator=(JArrayRef &&other) noexcept;

  inline JArrayRef() noexcept : data_(nullptr), size_(0), is_direct_(false) { }
  
  inline explicit JArrayRef(void *data, size_t size, bool is_direct) noexcept
    : data_(data), size_(size), is_direct_(is_direct)
  { }
  
  virtual ~JArrayRef() { }
  
protected:
  void *data_;
  size_t size_;
  bool is_direct_;

  // Allow subclasses to define indirect writing (e.g. through a JNI call).
  // Subclass may assume all arguments have already been properly validated.
  virtual int32_t writeToIndirect(
      int32_t start,
      const void *src,
      int32_t length) {
    
    // Elidable no-op for direct arrays
    return 0;
  }
  
};

} /* namespace djinnix */

#include "JArrayRef-inl.hpp"

#endif /* DJINNIX_JARRAYREF_HPP_ */
