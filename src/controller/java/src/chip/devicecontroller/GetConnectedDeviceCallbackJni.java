/*
 *   Copyright (c) 2020-2021 Project CHIP Authors
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

/** JNI wrapper callback class for getting a connected device. */
public class GetConnectedDeviceCallbackJni {
  private GetConnectedDeviceCallback wrappedCallback;
  private long callbackHandle;

  public GetConnectedDeviceCallbackJni(GetConnectedDeviceCallback wrappedCallback) {
    this.wrappedCallback = wrappedCallback;
    this.callbackHandle = newCallback(wrappedCallback);
  }

  long getCallbackHandle() {
    return callbackHandle;
  }

  private native long newCallback(GetConnectedDeviceCallback wrappedCallback);

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

  /** Callbacks for getting a device connected with PASE or CASE, depending on the context. */
  public interface GetConnectedDeviceCallback {
    void onDeviceConnected(long devicePointer);

    void onConnectionFailure(long nodeId, Exception error);
  }
}
