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

import chip.devicecontroller.model.ChipAttributePath;
import chip.devicecontroller.model.Status;
import javax.annotation.Nullable;

/** JNI wrapper callback class for {@link WriteAttributesCallback}. */
public final class WriteAttributesCallbackJni {
  private final WriteAttributesCallback wrappedWriteAttributesCallback;
  private long callbackHandle;

  public WriteAttributesCallbackJni(WriteAttributesCallback wrappedWriteAttributesCallback) {
    this.wrappedWriteAttributesCallback = wrappedWriteAttributesCallback;
    this.callbackHandle = newCallback();
  }

  long getCallbackHandle() {
    return callbackHandle;
  }

  private native long newCallback();

  private native void deleteCallback(long callbackHandle);

  // Called from native code only, which ignores access modifiers.
  private void onError(
      boolean isAttributePath, int endpointId, long clusterId, long attributeId, Exception e) {
    wrappedWriteAttributesCallback.onError(
        isAttributePath ? ChipAttributePath.newInstance(endpointId, clusterId, attributeId) : null,
        e);
  }

  private void onResponse(
      int endpointId,
      long clusterId,
      long attributeId,
      int status,
      @Nullable Integer clusterStatus) {
    wrappedWriteAttributesCallback.onResponse(
        ChipAttributePath.newInstance(endpointId, clusterId, attributeId),
        Status.newInstance(status, clusterStatus));
  }

  private void onDone() {
    wrappedWriteAttributesCallback.onDone();
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
