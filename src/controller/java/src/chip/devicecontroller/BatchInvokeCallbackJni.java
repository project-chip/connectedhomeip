/*
 *   Copyright (c) 2024 Project CHIP Authors
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

import chip.devicecontroller.model.InvokeResponseData;
import chip.devicecontroller.model.NoInvokeResponseData;
import java.lang.ref.Cleaner;
import java.util.Optional;
import javax.annotation.Nullable;

/** JNI wrapper callback class for {@link InvokeCallback}. */
public final class BatchInvokeCallbackJni {
  private final BatchInvokeCallbackJni wrappedBatchInvokeCallback;
  private long callbackHandle;

  private final Cleaner.Cleanable cleanable;

  public BatchInvokeCallbackJni(BatchInvokeCallback wrappedBatchInvokeCallback) {
    this.wrappedBatchInvokeCallback = wrappedBatchInvokeCallback;
    this.callbackHandle = newCallback();

    this.cleanable = Cleaner.create().register(this, () -> {
      if (chipClusterPtr != 0) {
        deleteCluster(chipClusterPtr);
        chipClusterPtr = 0;
      }
    });
  }

  long getCallbackHandle() {
    return callbackHandle;
  }

  private native long newCallback();

  private native void deleteCallback(long callbackHandle);

  private void onError(Exception e) {
    wrappedBatchInvokeCallback.onError(e);
  }

  private void onResponse(
      int endpointId,
      long clusterId,
      long commandId,
      @Nullable Integer commandRef,
      byte[] tlv,
      String jsonString) {
    wrappedExtendableInvokeCallback.onResponse(
        InvokeResponseData.newInstance(
            endpointId, clusterId, commandId, Optional.ofNullable(commandRef), tlv, jsonString));
  }

  private void onResponse(
      int endpointId,
      long clusterId,
      long commandId,
      @Nullable Integer commandRef,
      int status,
      @Nullable Integer clusterStatus) {
    wrappedExtendableInvokeCallback.onResponse(
        InvokeResponseData.newInstance(
            endpointId,
            clusterId,
            commandId,
            Optional.ofNullable(commandRef),
            status,
            Optional.ofNullable(clusterStatus)));
  }

  private void onNoResponse(int commandRef) {
    wrappedBatchInvokeCallback.onNoResponse(NoInvokeResponseData.newInstance(commandRef));
  }

  private void onDone() {
    wrappedBatchInvokeCallback.onDone();
  }
}
