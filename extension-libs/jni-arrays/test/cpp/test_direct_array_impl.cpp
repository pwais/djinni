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

#include "test_direct_array.hpp"

#include "djinnix_test/util.hpp"

namespace djinnix_test {

bool TestDirectArray::check_null_direct_array(::djinnix::JDirectArray da) {
  return da.empty();
}

bool TestDirectArray::check_direct_bb_array_contents(
    ::djinnix::JDirectArray da,
    const std::vector<uint8_t> &expected) {
  
  if (da.empty() != expected.empty()) {
    DJXT_LOG_ERROR(
      "JDirectArray empty (not?), but expected is not empty (empty?)\n" <<
      "actual empty: " << da.empty() << "\n" <<
      "expected empty: " << expected.empty());
    return false;
  } else if (da.empty() && expected.empty()) {
    return true;
  }
  
  auto array_ref = da.getArray();
  return djinnix_test::ArrayCompare(array_ref, expected);
}

}  /* namespace djinnix_test */
