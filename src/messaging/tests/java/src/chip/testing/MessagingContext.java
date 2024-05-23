/*
 *   Copyright (c) 2020-2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package chip.testing;

/** JNI wrapper for test utilities class. */
public final class MessagingContext {
  private long messagingContextHandle;

  static {
    System.loadLibrary("CHIPMessaging");
  }

  public MessagingContext(boolean initializeNodes) {
    this.messagingContextHandle = newMessagingContext(initializeNodes);
  }

  public MessagingContext() {
    this(true);
  }

  public long getMessagingContextHandle() {
    return messagingContextHandle;
  }

  private native long newMessagingContext(boolean initializeNodes);

  private native void deleteMessagingContext(long messagingContextHandle);

  // TODO(#8578): Replace finalizer with PhantomReference.
  @SuppressWarnings("deprecation")
  protected void finalize() throws Throwable {
    super.finalize();

    if (messagingContextHandle != 0) {
      deleteMessagingContext(messagingContextHandle);
      messagingContextHandle = 0;
    }
  }
}
