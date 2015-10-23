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

import sun.misc.Unsafe;

// TODO: Unsafe support
// NB: DirectByteBuffer is a private class and is not necessarily a part of all JVMs
public class DirectArray {
  protected ByteBuffer mDirectByteBuffer = null;
  protected UnsafeArray mUnsafeArray = null;
  
  public boolean isEmpty() {
    return mDirectByteBuffer == null && mUnsafeArray == null;
  }
  
  ///
  /// Factories
  ///
  
  public static DirectArray wrap(ByteBuffer b) {
    DirectArray da = new DirectArray();
    if (b != null && b.isDirect()) {
      da.mDirectByteBuffer = b;
    }
    return da;
  }
  
  public static DirectArray wrap(UnsafeArray a) {
    DirectArray da = new DirectArray();
    if (a != null) { da.mUnsafeArray = a; }
    return da;
  }
  
  // NB: used in native code
  public static DirectArray wrap(long address, long size) {
    DirectArray da = new DirectArray();
    UnsafeArray a = new UnsafeArray();
    da.mUnsafeArray = a;
    a.address = address;
    a.size = size;
    return da;
  }
  
  public static DirectArray allocate(int size) {
    DirectArray da = new DirectArray();
    da.mDirectByteBuffer = ByteBuffer.allocateDirect(size);
    return da;
  }
  
  public static DirectArray allocateUnsafe(long size) {
    DirectArray da = new DirectArray();
    da.mUnsafeArray = UnsafeArray.allocate(size);
    return da;
  }
  
  
  
  ///
  /// Accessors
  ///
  
  public ByteBuffer getByteBuffer() { return mDirectByteBuffer; }
  public UnsafeArray getUnsafeArray() { return mUnsafeArray; }

}
