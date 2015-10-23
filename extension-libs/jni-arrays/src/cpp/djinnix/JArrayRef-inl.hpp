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

#ifndef DJINNIX_JARRAYREF_INL_HPP_
#define DJINNIX_JARRAYREF_INL_HPP_

#ifndef DJINNIX_JARRAYREF_HPP_
#error "Include JArrayRef.hpp instead"
#endif

namespace djinnix {

inline
int32_t JArrayRef::writeTo(int32_t start, const void *src, int32_t length) {
  if (length <= 0) { return 0; }
    // Skip empty writes
  
  if (src == nullptr) { return 0; }
    // Short-circuit null
  
  if (empty()) { return 0; }
    // Don't write if this wrapper is null / invalid

  if (start + length > size_) { length = size_ - start; }
    // Don't write past the end of the array

  if (isDirect()) {
    std::memcpy((uint8_t *)data_ + start, src, length);
    return length;
  } else {
    return writeToIndirect(start, src, length);
  }
}

inline JArrayRef::JArrayRef(JArrayRef &&other)
  : data_(other.data_),
    size_(other.size_),
    is_direct_(other.is_direct_) {
  
  other.data_ = nullptr;
  other.size_ = 0;
  other.is_direct_ = false;
}

inline JArrayRef &JArrayRef::operator=(JArrayRef &&other) {
  std::swap(data_, other.data_);
  std::swap(size_, other.size_);
  std::swap(is_direct_, other.is_direct_);
  return *this;
}

} /* namespace djinnix */

#endif /* DJINNIX_JARRAYREF_INL_HPP_ */
