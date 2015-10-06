//
// Copyright 2014 Dropbox, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef DJINNIX_DISPOSER_IMPL_HPP_
#define DJINNIX_DISPOSER_IMPL_HPP_

#pragma once

#include <functional>

#include "DjinnixGenDisposer.hpp"

namespace djinnix {

class DisposerImpl : public Disposer {
public:
  DisposerImpl() { }
  virtual ~DisposerImpl() { }

  virtual void dispose() override;

private:
  std::function<void()> f_;

};

} /* namespace djinnix */

#endif /* DJINNIX_DISPOSER_IMPL_HPP_ */
