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

import javax.annotation.Nonnull;

/** An interface for receiving download log response. */
public interface DownloadLogCallback {
  /**
   * OnError will be called when the log download fails.
   *
   * @param fabricIndex The fabric index of to which the node belongs
   * @param nodeId The Node ID of the target device.
   * @param errorCode Error code on attestation failure.
   */
  public void onError(int fabricIndex, long nodeId, long errorCode);

  /**
   * onSuccess will be called when the log download is complete.
   *
   * @param fabricIndex The fabric index of to which the node belongs
   * @param nodeId The Node ID of the target device.
   */
  public void onSuccess(int fabricIndex, long nodeId);

  /**
   * onTransferData will be called when data is being received.
   *
   * @param fabricIndex The fabric index of to which the node belongs
   * @param nodeId The Node ID of the target device.
   * @param data Received log data.
   */
  public boolean onTransferData(int fabricIndex, long nodeId, @Nonnull byte[] data);
}
