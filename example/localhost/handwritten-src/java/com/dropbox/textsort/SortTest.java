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

    public static int kNumTests = 10000;
    
    public static int kStringsPerTest = 100;
  
    public static int kStringLength = 100;
    
    private static SecureRandom random = new SecureRandom();
    
    private static final Logger log =
        Logger.getLogger(SortTest.class.getName());
    
    public static String randomString() {
        return new BigInteger(130, random).toString(kStringLength);
    }
    
    public static void main(String[] args) throws Exception {  
        NativeLibLoader.loadLibs();
      
        // Create some random strings to sort below
        ArrayList<String> strs = new ArrayList<String>();
        for (int i = 0; i < kStringsPerTest; ++i) {
            strs.add(randomString());
        }
        
        
        /*
         * The microbenchmark below:
         *  * Allocates a new array of strings and deep-copies all test strings
         *  * Sorts them in-place
         * We explicitly deep copy strings in order to make results more
         * comparable with the tests below.
         */
        log.log(Level.INFO, "\n\n");
        log.log(Level.INFO, "Running java sort test ...");
        Instant startJava = Instant.now();
        for (int i = 0; i < kNumTests; ++i) {
            ArrayList<String> temp = new ArrayList<String>();
            for (String s : strs) {
              temp.add(s + ""); // Force allocation
            }
            Collections.sort(temp);
        }
        Instant endJava = Instant.now();
        log.log(
            Level.INFO,
            "... done in " + Duration.between(startJava, endJava) + ".");
        log.log(Level.INFO, "\n\n");
        
        /*
         * Now we run a microbenchmark similar to the one above, but using
         * STL's std::sort().  The below test:
         *  * Allocates a new temp std::vector of strings
         *  * Deep copies Java String data to std::string data.  This
         *     translation is expensive (!!) and dominates runtime.
         *  * Sorts the temp vector in-place.
         */
        log.log(Level.INFO, "Running native string sort test ...");
        Instant startNativeStr = Instant.now();
        for (int i = 0; i < kNumTests; ++i) {
            SortItems.runSortItems(new ItemList(strs));
        }
        Instant endNativeStr = Instant.now();
        log.log(
            Level.INFO,
            "... done in " +
                Duration.between(startNativeStr, endNativeStr) + ".");
        log.log(Level.INFO, "\n\n");
        
        /*
         * Now we run a microbenchmark similar to the one above, but 
         * we'll sort on String buffer data directly to avoid the
         * translation cost; the OS can do a fast memcpy. The below test:
         *  * Allocates a new std::vector of vector-buffers
         *  * Deep copies Java String data to the vector-buffers.  Since
         *      no transformation is done, this step is fast.
         *  * Sorts the temp vector in-place.
         */
        log.log(Level.INFO, "Running native buffer sort test ...");
        Instant startNativeBuf = Instant.now();
        for (int i = 0; i < kNumTests; ++i) {
            ArrayList<byte[]> bs = new ArrayList<byte[]>(); 
            for (String s : strs) {
                bs.add(s.getBytes());
            }
            SortItems.runSortBuffers(new BufferList(bs));
        }
        Instant endNativeBuf = Instant.now();
        log.log(
            Level.INFO,
            "... done in " +
                Duration.between(startNativeBuf, endNativeBuf) + ".");
    }

}

