/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller;

/** JNI wrapper callback class for {@link WriteAttributesCallback}. */
public final class WriteAttributesCallbackJni {
  private final WriteAttributesCallback wrappedWriteAttributesCallback;
  private long callbackHandle;

  public WriteAttributesCallbackJni(WriteAttributesCallback wrappedWriteAttributesCallback) {
    this.wrappedWriteAttributesCallback = wrappedWriteAttributesCallback;
    this.callbackHandle = newCallback(wrappedWriteAttributesCallback);
  }

  long getCallbackHandle() {
    return callbackHandle;
  }

  private native long newCallback(WriteAttributesCallback wrappedCallback);

  private native void deleteCallback(long callbackHandle);

  // TODO(#8578): Replace finalizer with PhantomReference.
  @SuppressWarnings("deprecation")
  protected void finalize() throws Throwable {
    super.finalize();

    if (callbackHandle != 0) {
      deleteCallback(callbackHandle);
      callbackHandle = 0;
    }
  }
}
