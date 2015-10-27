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

import java.lang.reflect.Field;
import java.util.logging.Level;
import java.util.logging.Logger;
import sun.misc.Unsafe;

public class UnsafeArray extends ByteArray {
  public long address = 0;
  public long size = 0;
  
  public boolean isEmpty() { return size == 0; }
  
  ///
  /// Factories
  ///
  
  public static UnsafeArray allocate(long size) {
    UnsafeArray a = new UnsafeArray();
    a.address = UnsafeSupport.allocateMemory(size);
    a.size = a.address != 0 ? size : 0;
    return a;
  }

  ///
  /// Accessors
  ///
  
  public byte get(long i) {
    assert !isEmpty() : "Empty array";
    assert i < size : "Index " + i + " out of bounds (size: " + size + ")";
    return UnsafeSupport.get(address + i);
  }
  
  public void put(long i, byte v) {
    assert !isEmpty() : "Empty array";
    assert i < size : "Index " + i + " out of bounds (size: " + size + ")";
    UnsafeSupport.put(address + i, v);
  }
  
  public void free() {
    if (!isEmpty()) { UnsafeSupport.freeMemory(address); }
  }
  
  
  
  ///
  /// Utils
  ///

  // TODO: a way to convert to ByteBuffer (interface) would be nice...
  // OpenJDK's DirectByteBuffer is a candidate but very very complicated :(
  
  /**
   * A small adapter around sun.misc.Unsafe, which is expected to undergo
   * (small) modifications in future JDK releases.
   * 
   * Notes:
   *  * Allocations / frees are assert-free and safe if Unsafe is not supported
   *  * get/put should be JIT-friendly with asserts off
   */
  public static class UnsafeSupport {
    private static sun.misc.Unsafe mUnsafe = null;
 
    private static final Logger log =
       Logger.getLogger(UnsafeArray.UnsafeSupport.class.getName());
   
    static {
      try {
        Field f = Unsafe.class.getDeclaredField("theUnsafe");
        f.setAccessible(true);
        mUnsafe = (sun.misc.Unsafe) f.get(null);
      } catch (Throwable e) {
        log.log(Level.SEVERE, "Could not instantiate Unsafe: " + e.toString());
      }
    }
    
    public static long allocateMemory(long size) {
      if (mUnsafe == null) {
        log.log(Level.SEVERE, "Unsafe not supported");
        return 0;
      }
      return mUnsafe.allocateMemory(size);
    }
 
    public static void freeMemory(long addr) {
      if (mUnsafe == null) { 
        log.log(
          Level.SEVERE,
          "Unsafe not supported; memory will leak: " + Long.toHexString(addr));
        return;
      }
      mUnsafe.freeMemory(addr);
    }
    
    public static byte get(long address) {
      assert mUnsafe != null : "Unsafe not supported";
      assert address >= 0 : "Invalid address"; // Unsafe may chop sign bits
      return mUnsafe.getByte(address);
    }
    
    public static void put(long address, byte value) {
      assert mUnsafe != null : "Unsafe not supported";
      assert address >= 0 : "Invalid address"; // Unsafe may chop sign bits
      mUnsafe.putByte(address, value);
    }
  }
}
