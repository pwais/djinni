package com.dropbox.djinnix.test;

import junit.framework.TestCase;
import com.dropbox.djinnix.DirectArray;

public class JDirectArrayTest extends TestCase {

  public void testNull() {
    assertTrue(TestDirectArray.checkNullDirectArray(DirectArray.wrap(null)));
  }

  public void testEmpty() {
  }

  public void testNonEmpty() {
  }

}

