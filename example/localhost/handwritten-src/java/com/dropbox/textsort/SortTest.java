package com.dropbox.textsort;

import java.math.BigInteger;
import java.security.SecureRandom;
import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.Collections;
import java.util.logging.Level;
import java.util.logging.Logger;

import com.dropbox.djinni.NativeLibLoader;

public class SortTest {

    public static int kNumTests = 100;
    
    public static int kStringsPerTest = 1000;
  
    public static int kStringLength = 1000;
    
    private static SecureRandom random = new SecureRandom();
    
    private static final Logger log =
        Logger.getLogger(SortTest.class.getName());
    
    public static String randomString() {
        return new BigInteger(130, random).toString(kStringLength);
    }
    
    private static class TextboxListenerDummyImpl extends TextboxListener {
      @Override
      public void update(ItemList items) { }
    }
    
    public static void main(String[] args) throws Exception {  
        NativeLibLoader.loadLibs();
      
        ArrayList<String> strs = new ArrayList<String>();
        for (int i = 0; i < kStringsPerTest; ++i) {
            strs.add(randomString());
        }
        
        log.log(Level.INFO, "Running java sort test ...");
        Instant startJava = Instant.now();
        for (int i = 0; i < kNumTests; ++i) {
            ArrayList<String> temp = new ArrayList<String>();
            for (String s : strs) {
              temp.add(new String(s));
            }
            Collections.sort(temp);
        }
        Instant endJava = Instant.now();
        log.log(
            Level.INFO,
            "... done in " + Duration.between(startJava, endJava) + ".");
      
        SortItems sortItemsInterface = 
            SortItems.createWithListener(new TextboxListenerDummyImpl());
        log.log(Level.INFO, "Running native sort test ...");
        Instant startNative = Instant.now();
        for (int i = 0; i < kNumTests; ++i) {
            sortItemsInterface.sort(SortOrder.DESCENDING, new ItemList(strs));
        }
        Instant endNative = Instant.now();
        log.log(
            Level.INFO,
            "... done in " + Duration.between(startNative, endNative) + ".");
    }

}

