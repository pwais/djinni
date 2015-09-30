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

package com.dropbox.djinni;

import java.nio.ByteBuffer;
import java.util.logging.Level;
import java.util.logging.Logger;
import sun.nio.ch.DirectBuffer;;

/**
 * TODO
 */
public class NativeBufferFactory {
    
  public class BufferView {
    public long baseAddress = 0;
    public long size = 0;
    public byte[] jbuffer = null;
    
    public void reset() {
      baseAddress = 0;
      size = 0;
      jbuffer = null;
    }
    
    public void fascadeFor(ByteBuffer buf) {
      reset();
      if (buf.isDirect()) {
        baseAddress = ((DirectBuffer)buf).address();
        size = buf.capacity();
      } else if (buf.hasArray()) {
        jbuffer = buf.array();
      }
    }
    
    public void fascadeFor(byte[] buf) {
      reset();
      jbuffer = buf;
    }
    
    // Notes about Unsafe, jvm can move ..
    public void fascadeFor(long baseAddress, long size) {
      reset();
      this.baseAddress = baseAddress;
      this.size = size;
    }
  }
  
  public class RefCountedBufferView extends BufferView {
    // override finalize to tell dij / factory to release count ?
  }
  
  
  /**
   * use cases:
   * 
   * byte[] myarr;
   * val bv = new BufferView()
   * bv.fascadeFor(myarr);
   * 
   * native.my_method(bv)
   * 
   * cpp side:
   *   
   *   struct bufview_handle final {
   *     
   *     bufview Get();
   *       // do whatever is necessary, even copy, to get a ptr
   *     
   *     void ApplyCritical(std::function<()(bufview)> func);
   *       // run `func` on a bufview using GetCritical() jni call
   *      
   *     
   *   }
   * 
   * 
   * 
   *   struct bufview final {
   *     uint8_t *data;
   *     size_t size;
   *     Owner *owner;
   *     
   *     bufview()
   *       : data(nullptr), size(0), owner(nullptr)
   *     { }
   *     
   *     ~bufview() {
   *       if (owner) {
   *         owner->Dispose();
   *       }
   *       delete owner;
   *     }
   *   };
   *   
   *   Owners:
   *    * JavaOwner {
   *        virtual void Dispose() {
   *          // invokes JNI method ...
   *        }
   *      }
   *    * vector/string owner
   *    
   *    
   *  C++ => Java r/w:
   *    Direct Byte Buffer (or could be ptr, size for unsafe ...)
   *      to get byte[] need to copy thru byte buffer iface
   *  
   *  Java => C++ r/w:
   *    interface:
   *      * use critical (ptr, size)
   *      * get direct (ptr, size)
   *  
   *  C++ => Java o:
   *    interface:
   *      * get (ptr, size) or direct byte buffer
   *      * dispose() (called in finalize)
   *      (share should inc a refcount ?)
   *  
   *  Java => C++ o:
   *    heap:
   *      * use critical (ptr, size)
   *      * dispose() to null the byte[]
   *    off-heap:
   *      * get direct (ptr, size)
   *      * dispose() to do abitrary (null byte buffer, free pool)
   *    
   *   
   *   Use cases:
   *    * what if we want a buffer of buffers?  or array of buffers? very 
   *       expensive as above!   
   *       
   *         unless we can make the buffer fascade like a pointer.
   *         for java: direct byte buffers can emit pieces
   *         for c++: use use pointers..
   *   
   *   The practices to contrib:
   *    * a use critical interface for on-heap java to c++
   *    * a use direct interface for off-heap java to c++
   *    * a disposer in each lang, so that ownership is transferable
   *       and can be shared
   *   
   *   Use cases to support:
   *    * share a large buffer
   *    * share a small buffer with high r/w
   *   
   *   For cpp Tungsten-sort-- where records are protobufs
   *     * if we can pass a socket to native, that helps!
   *     * otherwise consumer byte[] in java and have cpp size load them
   *        into memory first; bench starts once all recs to be sorted
   *        are loaded *into linked list* -> sort record has prefix and 
   *        pointer to record (which could be to arb. place)
   *     * the bench ends when order of buffers is esablished, i.e. ready
   *        to send segments over the wire
   *     * support a buffer comparator that can do a JNI call (that will 
   *       do comparison in java-- ideally using the buffer data w/out deser
   *       but the comp could potentially do that. and perhaps keep a cache).
   *     
   *     In this case, 
   *      * CPP just needs a use critical for incoming byte[].  or a way to
   *          get ownership of a direct byte buffer of records.  getting
   *          ownership of individ records would suck.
   *      * if CPP doesn't send segments, CPP needs a way to expose
   *          direct byte buffers (one per seg?) to java and perhaps
   *          release after each emit.  in this case, want to re-use
   *          the direct byte buffer object and just re-point each emit ...
   *          
   *     For this problem:
   *      * want to flesh out who does the send/recieve stuff.  is it in java
   *          or cpp?  if in java, does input / output come as
   *          direct byte buf?
   * 
   * 
   *   For cpp map onto Rows-- where Rows are hashmaps of key -> object
   *     * for byte[] vals, we really just want a use critical
   *     * for Strings, dij xlate might be slow...
   *     * 
   */
}
