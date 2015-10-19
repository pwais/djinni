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

package com.dropbox.djinnix.test;

import java.nio.ByteBuffer;
import static org.junit.Assert.*;
import java.util.Arrays;

public class Util {

  public static void checkBBs(ByteBuffer actual, ByteBuffer expected) {
    assertEquals(actual == null, expected == null);
    assertEquals(actual.capacity(), expected.capacity());
    
    byte[] actualArray = new byte[actual.capacity()];
    actual.get(actualArray);
    actual.rewind();
    
    byte[] expectedArray = new byte[expected.capacity()];
    expected.get(expectedArray);
    expected.rewind();
    
    assertArrayEquals(actualArray, expectedArray);
  }
  
  public static void checkBBs(ByteBuffer actual, byte[] expected) {
    checkBBs(actual, ByteBuffer.wrap(expected));
  }

}