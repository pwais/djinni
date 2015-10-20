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

  public static ByteBuffer directFixture() {
    ByteBuffer f = ByteBuffer.allocateDirect(kFixture.length);
    f.put(kFixture);
    f.rewind();
    return f;
  }
  
  public static byte[] heapFixture() {
    byte[] f = new byte[kFixture.length];
    System.arraycopy(kFixture, 0, f, 0, kFixture.length);
    return f;
  }
  
  public static void checkArrays(TestDirectArray tda, byte[] expected) {
    // Check byte-for-byte match in both CPP ... 
    assertTrue(tda.check(expected));
    
    // ... and Java-- by temporarily *moving* ownership of the test
    // object's array into Java ...
    DirectArray da = tda.takeArray();
    Util.checkBBs(da.getAsDirectByteBuffer(), expected);
    
    // ... and restoring it.
    assertTrue(tda.wrap(da));
  }

  
  
  ///
  /// Basic: test empty and null
  ///
  
  public void testNull() {
    assertTrue(TestDirectArray.checkNullDirectArray(DirectArray.wrap(null)));
  }
  
  public void testEmptyJava() {
    DirectArray da = DirectArray.wrap(ByteBuffer.allocateDirect(0));
    TestDirectArray tda = TestDirectArray.create();
    assertTrue(tda.wrap(da));
    
    checkArrays(tda, new byte[0]);
  }
  
  public void testEmptyCPPDirect() {
    TestDirectArray tda = TestDirectArray.allocateDirect(0);
    checkArrays(tda, new byte[0]);
  }
  
  public void testEmptyCPPFascade() {
    TestDirectArray tda = TestDirectArray.allocateFascade(0);
    checkArrays(tda, new byte[0]);
  }
  
  
  
  ///
  /// Test non-empty arrays
  ///
  
  public void testNonEmptyJava() {
    ByteBuffer actual = directFixture();
    
    DirectArray da = DirectArray.wrap(actual);
    TestDirectArray tda = TestDirectArray.create();
    assertTrue(tda.wrap(da));
    
    checkArrays(tda, kFixture);
  }
  
  public void testNonEmptyCPPDirect() {
    TestDirectArray tda = TestDirectArray.allocateDirect(kFixture.length);
    tda.write(kFixture);
    
    checkArrays(tda, kFixture);
  }
  
  public void testNonEmptyCPPFascade() {
    TestDirectArray tda = TestDirectArray.allocateFascade(kFixture.length);
    tda.write(kFixture);
    
    checkArrays(tda, kFixture);
    
  }
  
  
  
  ///
  /// Test multiple one-sided writes
  ///
  
  // Ensure writes to a Java-allocated direct ByteBuffer are visible to CPP
  // across many writes; CPP's pointer is valid every time
  public void testWriteJava() {
    byte[] expected = heapFixture();
    
    ByteBuffer actual = ByteBuffer.allocateDirect(expected.length);
    
    DirectArray da = DirectArray.wrap(actual);
    TestDirectArray tda = TestDirectArray.create();
    assertTrue(tda.wrap(da));
    
    for (int i = 0; i < kFixture.length; ++i) {
      expected[i] += 1;
      actual.put(expected);
      actual.rewind();
      
      checkArrays(tda, expected);
    }
  }

  // Ensure writes to a CPP-allocated direct ByteBuffer are visible to Java
  // across many writes; Java is able to ref the same ByteBuffer every time
  public void testWriteCPPDirect() {
    byte[] expected = heapFixture();
    
    TestDirectArray tda = TestDirectArray.allocateDirect(expected.length);
    
    for (int i = 0; i < kFixture.length; ++i) {
      expected[i] += 1;
      assertTrue(tda.write(expected));
      
      checkArrays(tda, expected);
    }
  }
  
  // Ensure writes to a CPP-allocated array are visible to Java across many
  // writes; Java is able to ref the same memory every time
  public void testWriteCPPFascade() {
    byte[] expected = heapFixture();
    
    TestDirectArray tda = TestDirectArray.allocateFascade(expected.length);
    
    for (int i = 0; i < kFixture.length; ++i) {
      expected[i] += 1;
      assertTrue(tda.write(expected));
      
      checkArrays(tda, expected);
    }
  }
}
