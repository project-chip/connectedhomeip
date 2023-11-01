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

import javax.annotation.Nullable;

public interface OTAProviderDelegate {
  public QueryImageResponse handleQueryImage(
      int vendorId,
      int productId,
      long softwareVersion,
      @Nullable Integer hardwareVersion,
      @Nullable String location,
      @Nullable Boolean requestorCanConsent,
      @Nullable byte[] metadataForProvider);

  public void handleOTAQueryFailure(int error);

  public void handleBDXTransferSessionBegin(long nodeId, String fileDesignator, long offset);

  public BDXData handleBDXQuery(long nodeId, int blockSize, long blockIndex, long bytesToSkip);

  public void handleBDXTransferSessionEnd(long errorCode, long nodeId);

  public ApplyUpdateResponse handleApplyUpdateRequest(long nodeId, long newVersion);

  public default void handleNotifyUpdateApplied(long nodeId) {}

  public class BDXData {
    @Nullable private byte[] data;
    private boolean isEOF;

    public BDXData(byte[] data, boolean isEOF) {
      this.data = data;
      this.isEOF = isEOF;
    }

    public byte[] getData() {
      return data;
    }

    public boolean isEOF() {
      return isEOF;
    }
  }

  public class QueryImageResponse {
    @Nullable Long softwareVersion;
    @Nullable String softwareVersionString;
    @Nullable String filePath;

    public QueryImageResponse(
        @Nullable Long softwareVersion, @Nullable String softwareVersionString, String filePath) {
      this.softwareVersion = softwareVersion;
      this.softwareVersionString = softwareVersionString;
      this.filePath = filePath;
    }

    public Long getSoftwareVersion() {
      return softwareVersion;
    }

    public String getSoftwareVersionString() {
      return softwareVersionString;
    }

    public String getFilePath() {
      return filePath;
    }
  }

  public class ApplyUpdateResponse {
    private ApplyUpdateActionEnum action;
    private long delayedActionTime;

    public ApplyUpdateResponse(ApplyUpdateActionEnum action, long delayedActionTime) {
      this.action = action;
      this.delayedActionTime = delayedActionTime;
    }

    public int getAction() {
      return action.getValue();
    }

    public long getDelayedActionTime() {
      return delayedActionTime;
    }
  }

  public enum ApplyUpdateActionEnum {
    Proceed(0),
    AwaitNextAction(1),
    Discontinue(2);

    private int value;

    ApplyUpdateActionEnum(int value) {
      this.value = value;
    }

    public int getValue() {
      return value;
    }
  }
}
