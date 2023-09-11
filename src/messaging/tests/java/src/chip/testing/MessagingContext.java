/*
 * SPDX-FileCopyrightText: 2020-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
