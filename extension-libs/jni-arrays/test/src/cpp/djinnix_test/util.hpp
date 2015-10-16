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

#ifndef DJINNIX_TEST_UTIL_HPP_
#define DJINNIX_TEST_UTIL_HPP_

#pragma once

#include <cstring>
#include <iostream>
#include <string.h>


#define DJXT_LOG_ERROR(msg) do { \
  std::cerr << \
  /* filename, and not full path (often unhelpful) */ \
  std::string( \
    strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__) << \
  /* func and line */ \
  ":" << __func__ << "(" << __LINE__ << "): " << \
  msg << std::endl; \
} while(0)


namespace djinnix_test {

template <typename ActualT, typename ExpectedT>
inline bool ArrayCompare(ActualT &actual, ExpectedT &expected) {

  if (actual.empty() != expected.empty()) {
    DJXT_LOG_ERROR(
      "Actual empty (not?), but expected is not empty (empty?)\n" <<
      "actual empty: " << actual.empty() << "(size: " << actual.size() << ")\n" <<
      "expected empty: " << expected.empty() << "(size: " << expected.size() << ")");
    return false;
  } else if (actual.empty() && expected.empty()) {
    return true;
  }

  if (actual.size() != expected.size()) {
    DJXT_LOG_ERROR(
      "Size mismatch:\n" <<
      "actual: " << actual.size() << "\n" <<
      "expected: " << expected.size());
    return false;
  }

  int res = memcmp(actual.data(), expected.data(), expected.size());
  if (res == 0) {
    return true;
  } else {
    DJXT_LOG_ERROR(
      "memcmp found a difference of " << res << " in some byte; " <<
      "arrays NOT identical");
    return false;
  }
}

} /* namespace djinnix_test */

#endif /* DJINNIX_TEST_UTIL_HPP_ */
