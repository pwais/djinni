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
import junit.framework.TestCase;
import com.dropbox.djinnix.DirectArray;
import com.dropbox.djinnix.test.Util;
import static org.junit.Assert.*;

public class JDirectArrayTest extends TestCase {

  static byte[] kFixture = { 0xd, 0xe, 0xa, 0xd, 0xb, 0xe, 0xe, 0xf };
  
  public void testNull() {
    assertTrue(TestDirectArray.checkNullDirectArray(DirectArray.wrap(null)));
  }

  public void testEmpty() {
    assertTrue(
      TestDirectArray.checkDirectBbArrayContents(
        DirectArray.wrap(ByteBuffer.allocateDirect(0)),
        new byte[0]));
  }

  public void testNonEmpty() {
    ByteBuffer actual = ByteBuffer.allocateDirect(kFixture.length);
    actual.put(kFixture);
    assertTrue(
      TestDirectArray.checkDirectBbArrayContents(
        DirectArray.wrap(actual),
        kFixture));
  }

  public void testEmptyFascade() {
    DirectArray da = TestDirectArray.createDirectBbFascade(new byte[0]);
    ByteBuffer bb = da.getAsDirectByteBuffer();
    assertTrue(bb != null);
    assertEquals(0, bb.capacity());
  }
  
  public void testEmptyCPPAllocated() {
    DirectArray da = TestDirectArray.createDirectBb(new byte[0]);
    ByteBuffer bb = da.getAsDirectByteBuffer();
    assertTrue(bb != null);
    assertEquals(0, bb.capacity());
  }
  
  public void testFascade() {
    DirectArray da = TestDirectArray.createDirectBbFascade(kFixture);
    Util.checkBBs(da.getAsDirectByteBuffer(), kFixture);
  }
  
  public void testCPPAllocated() {
    DirectArray da = TestDirectArray.createDirectBb(kFixture);
    Util.checkBBs(da.getAsDirectByteBuffer(), kFixture);
  }
  
}
