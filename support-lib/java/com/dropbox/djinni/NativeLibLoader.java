//                                                                                                                                                                                     
// Copyright 2015 Dropbox, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

package com.dropbox.djinni;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;

/**
 * Utilities for loading native libraries containing djinni interfaces
 * and records.  To load libraries in your application at startup,
 * simply place them:
 *  * inside the jar containing this code at the path `djinniNativeLibsJarPath`
 *  * somewhere on the host filesystem and use the `djinniNativeLibsSysProp`
 *      system property to provide path(s)
 * 
 * To *disable* automatic loading of native libs at startup (and require
 * manual loading), use the `djinniLoadNativeLibsAtStartupProp` system
 * property.
 */
class NativeLibLoader {
  
  /**
   * Canonical directory in a jar containing (djinni-adapted) native libraries
   */
  public static final String djinniNativeLibsJarPath =
      "resources/djinni_native_libs";
  
  /**
   * Load native libraries with djinni support from the (comma-separated)
   * path(s) provided in this system property 
   */
  public static final String djinniNativeLibsSysProp =
      "djinni.native_libs_dirs";
  
  /**
   * System property flag indicating if native libraries should be loaded
   * on program startup
   */
  public static final String djinniLoadNativeLibsAtStartupProp = 
      "djinni.load_native_libs_at_startup";
  
  private NativeLibLoader() { } 
  
  // Try to load libs at program startup
  static {
    String doLoad = System.getProperty(djinniLoadNativeLibsAtStartupProp);
    if (doLoad == null || Boolean.valueOf(doLoad)) {
      try {
        loadLibs();
      } catch (Exception e) {
        throw new RuntimeException(e);
      }
    }
  }
  
  // Load native libs from canonical locations
  public static void loadLibs() throws URISyntaxException, IOException {
    // Try to load from Jar
    URI libs =
        NativeLibLoader.class.getResource(djinniNativeLibsJarPath).toURI();
    if (libs.getScheme().equals("jar")) {
      loadLibFromJarPath(Paths.get(libs));
    } else {
      // Support running outside a jar
      loadLibsFromLocalPath(Paths.get(libs));
    }
    
    // Try to load from system
    String localPaths = System.getProperty(djinniNativeLibsSysProp);
    if (localPaths != null) {
      for (String localPath : localPaths.split(",")) {
        loadLibsFromLocalPath(Paths.get(localPath));
      }
    }
  }
  
  // Load native lib(s) from the given `localPath` - a file or directory
  public static void loadLibsFromLocalPath(Path localPath) throws IOException {
    File localFile = localPath.toFile();
    if (!localFile.exists()) { return; }
    if (localFile.isDirectory()) {
      Files
        .walk(localFile.toPath(), 1)
        .forEach(p -> System.load(p.toFile().getAbsolutePath()));
    } else {
      System.load(localFile.getAbsolutePath());
    }
  }
  
  // Load a single native lib from a jar resource with path `libPath`
  public static void loadLibFromJarPath(Path libPath) throws IOException {

    /*
     * System libraries *must* be loaded from the filesystem,
     * so copy the lib's data to a tempfile
     */ 
    InputStream libIn =
        NativeLibLoader.class.getResourceAsStream(libPath.toString());
    File tempLib =
        File.createTempFile(
            libPath.getName(libPath.getNameCount() - 1).toString(),
              // name tempfile after the lib to ease debugging
            null);
    tempLib.deleteOnExit();
    try {
      Files.copy(libIn, tempLib.toPath(), StandardCopyOption.REPLACE_EXISTING);
    } catch (SecurityException e) { 
      throw new RuntimeException(
          "SecurityException while trying to create tempfile: " +
            e.getMessage() + "\n\n If you cannot grant this process " +
            "permissions to create temporary files, you need to install " +
            "the native libraries manually and provide the installation " + 
            "path(s) using the system property " + djinniNativeLibsSysProp);
    }
    
    System.load(tempLib.getAbsolutePath());
  }  
}

