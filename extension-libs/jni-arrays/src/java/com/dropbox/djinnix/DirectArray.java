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

package com.dropbox.djinnix;

import java.nio.ByteBuffer;

// TODO: Unsafe support
// NB: DirectByteBuffer is a private class and is not necessarily a part of all JVMs
class DirectArray {
  protected ByteBuffer mDirectByteBuffer = null;
  
  public bool isEmpty() { return mDirectByteBuffer != null; }
  
  ///
  /// Factories
  ///
  
  public DirectArray wrap(ByteBuffer b) {
    DirectArray da = new DirectArray();
    if (b.isDirect()) {
      da.mDirectByteBuffer = b;
    }
    return da;
  }
  
  public DirectArray allocate(long size) {
    DirectArray da = new DirectArray();
    da.mDirectByteBuffer = ByteBuffer.allocateDirect(size);
    return da;
  }
  
  ///
  /// Accessors (lso invoked from native code!)
  ///
  
  public ByteBuffer getAsDirectByteBuffer() {
    if (mDirectByteBuffer != null) {
      return mDirectByteBuffer;
    } else {
      return null;
    }
  }
}