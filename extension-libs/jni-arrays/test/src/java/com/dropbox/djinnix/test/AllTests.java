package com.dropbox.djinnix.test;

import com.dropbox.djinni.NativeLibLoader;

import junit.framework.Test;
import junit.framework.TestSuite;
import org.junit.runner.JUnitCore;

public class AllTests extends TestSuite {

    public static Test suite() {
        TestSuite mySuite = new TestSuite("Djinni JNI-Arrays Tests");
        mySuite.addTestSuite(JDirectArrayTest.class);
        mySuite.addTestSuite(JHeapArrayTest.class);
        return mySuite;
    }

    public static void main(String[] args) throws Exception {     
       NativeLibLoader.loadLibs();
       JUnitCore.main("com.dropbox.djinnix.test.AllTests");            
    }
}
