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

#ifndef DJINNIX_JNI_ARRAYS_TEST_DIRECT_ARRAY_IMPL_HPP_
#define DJINNIX_JNI_ARRAYS_TEST_DIRECT_ARRAY_IMPL_HPP_

#pragma once

#include "test_direct_array.hpp"

#include "djinnix_test/util.hpp"

namespace djinnix_test {

class TestDirectArrayImpl : public TestDirectArray {
public:
  virtual ~TestDirectArrayImpl() { }
  TestDirectArrayImpl() { }
  
  inline
  bool wrap(::djinnix::JDirectArray da) override {
    return setDirectArray(std::move(da));
  }
  
  inline
  bool setDirectArray(::djinnix::JDirectArray &&da) {
    da_ = std::move(da);
    return true;
  }

  inline
  bool write(const std::vector<uint8_t> &contents) override {
    if (!da_.hasArray()) {
      DJXT_LOG_ERROR("JDirectArray does not have a buffer");
      return false;
    }
    
    auto array_ref = da_.getArray();
    if (array_ref.size() != contents.size()) {
      DJXT_LOG_ERROR(
        "Can't write " << contents.size() <<
        " bytes to JDirectArray with size " << array_ref.size());
      return false;
    }
    
    memcpy(array_ref.data(), contents.data(), array_ref.size());
    return true;
  }
  
  inline
  bool check(const std::vector<uint8_t> & expected_contents) override {
    if (!da_.hasArray()) {
      DJXT_LOG_ERROR("JDirectArray does not have a buffer");
      return false;
    }
    
    auto array_ref = da_.getArray();
    return djinnix_test::ArrayCompare(array_ref, expected_contents);
  }

  inline
  ::djinnix::JDirectArray takeArray() override {
    return std::move(da_);
  }
  
  inline
  std::vector<uint8_t> &getNativeBufRef() {
    return buf_;
  }
  
protected:
  ::djinnix::JDirectArray da_;
  std::vector<uint8_t> buf_; // for fascade tests
};

}  /* namespace djinnix_test */

#endif /* DJINNIX_JNI_ARRAYS_TEST_DIRECT_ARRAY_IMPL_HPP_ */