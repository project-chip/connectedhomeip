/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller;

/** JNI wrapper callback class for {@link InvokeCallback}. */
public final class InvokeCallbackJni {
  private final InvokeCallback wrappedInvokeCallback;
  private long callbackHandle;

  public InvokeCallbackJni(InvokeCallback wrappedInvokeCallback) {
    this.wrappedInvokeCallback = wrappedInvokeCallback;
    this.callbackHandle = newCallback(wrappedInvokeCallback);
  }

  long getCallbackHandle() {
    return callbackHandle;
  }

  private native long newCallback(InvokeCallback wrappedCallback);

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
