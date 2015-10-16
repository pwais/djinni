package com.dropbox.djinnix.test;

import java.nio.ByteBuffer;
import junit.framework.TestCase;
import com.dropbox.djinnix.DirectArray;

public class JDirectArrayTest extends TestCase {

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
    byte[] expected = { 0xd, 0xe, 0xa, 0xd, 0xb, 0xe, 0xe, 0xf };
    ByteBuffer actual = ByteBuffer.allocateDirect(expected.length);
    actual.put(expected);
    assertTrue(
      TestDirectArray.checkDirectBbArrayContents(
        DirectArray.wrap(actual),
        expected));
  }

}
