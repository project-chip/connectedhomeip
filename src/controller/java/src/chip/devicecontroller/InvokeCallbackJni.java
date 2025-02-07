/*
 *   Copyright (c) 2023 Project CHIP Authors
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
package chip.devicecontroller;

import chip.devicecontroller.model.InvokeElement;

/** JNI wrapper callback class for {@link InvokeCallback}. */
public final class InvokeCallbackJni {
  private final InvokeCallback wrappedInvokeCallback;
  private long callbackHandle;

  public InvokeCallbackJni(InvokeCallback wrappedInvokeCallback) {
    this.wrappedInvokeCallback = wrappedInvokeCallback;
    this.callbackHandle = newCallback();
  }

  long getCallbackHandle() {
    return callbackHandle;
  }

  private native long newCallback();

  private native void deleteCallback(long callbackHandle);

  private void onError(Exception e) {
    wrappedInvokeCallback.onError(e);
  }

  private void onResponse(
      int endpointId,
      long clusterId,
      long commandId,
      byte[] tlv,
      String jsonString,
      long successCode) {
    wrappedInvokeCallback.onResponse(
        InvokeElement.newInstance(endpointId, clusterId, commandId, tlv, jsonString), successCode);
  }

  private void onDone() {
    wrappedInvokeCallback.onDone();
  }

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
