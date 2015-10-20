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

#include <vector>

#include "test_direct_array_impl.hpp"

#include "djinnix_test/util.hpp"

namespace djinnix_test {

std::shared_ptr<TestDirectArray>
TestDirectArray::allocateDirect(int32_t size) {
  TestDirectArrayImpl *tda = new TestDirectArrayImpl();
  std::shared_ptr<TestDirectArray> tdap(tda);

  auto da = ::djinnix::JDirectArray::allocateDirectBB(size);
  assert(da.hasArray()); // Failed to create a Direct Byte Buffer
  auto array_ref = da.getArray();
  assert(array_ref.size() == size);
    // Allocated ByteBuffer has wrong size

  tda->setDirectArray(std::move(da));
  return tdap;
}

std::shared_ptr<TestDirectArray>
TestDirectArray::allocateFascade(int32_t size) {
  TestDirectArrayImpl *tda = new TestDirectArrayImpl();
  std::shared_ptr<TestDirectArray> tdap(tda);

  auto &tda_buf = tda->getNativeBufRef();
  tda_buf.resize(size);

  auto da =
    ::djinnix::JDirectArray::createDirectFascadeFor(
        tda_buf.data(),
        tda_buf.size());
  assert(da.hasArray()); // Failed to create a Direct Byte Buffer
  auto array_ref = da.getArray();
  assert(array_ref.size() == size);
    // Allocated ByteBuffer has wrong size

  tda->setDirectArray(std::move(da));
  return tdap;
}

std::shared_ptr<TestDirectArray> TestDirectArray::create() {
  TestDirectArrayImpl *tda = new TestDirectArrayImpl();
  return std::shared_ptr<TestDirectArray>(tda);
}

bool TestDirectArray::check_null_direct_array(::djinnix::JDirectArray da) {
  return !da.hasArray();
}

}  /* namespace djinnix_test */
