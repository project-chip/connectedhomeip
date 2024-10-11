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

/**
 * Delegate for OTA Provider.
 *
 * <p>See detailed in {@link ChipDeviceController#startOTAProvider(OTAProviderDelegate)}
 */
public interface OTAProviderDelegate {
  /**
   * Called to handle a QueryImage command and is responsible for sending the response (if success)
   * or status (if error).
   *
   * @param vendorId Vendor ID applying to the OTA Requestor’s Node.
   * @param productId Product ID applying to the OTA Requestor’s Node.
   * @param softwareVersion Current version running on the OTA Requestor.
   * @param hardwareVersion OTA Requestor’s hardware version. Can be set null.
   * @param location OTA Requestor's current location. Can be set null.
   * @param requestorCanConsent Variable for whether user consent is required. Can be set null.
   * @param metadataForProvider Manufacturer-specific payload. Can be set null.
   * @return QueryImageResponse Send Firmware image information, See detailed in {@link
   *     OTAProviderDelegate#QueryImageResponse}
   */
  public QueryImageResponse handleQueryImage(
      int vendorId,
      int productId,
      long softwareVersion,
      @Nullable Integer hardwareVersion,
      @Nullable String location,
      @Nullable Boolean requestorCanConsent,
      @Nullable byte[] metadataForProvider);

  /**
   * The callback will be invoked when receiving or responding to a QueryImage, it is called with an
   * ErrorCode.
   */
  public void handleOTAQueryFailure(int error);

  /**
   * The callback will be invoked when BDX Transfer session is begined.
   *
   * @param nodeId OTA Requester's Node ID
   * @param fileDesignator a variable-length identifier chosen by the Initiator to identify the
   *     payload to be transferred.
   * @param offset image's offset
   */
  public void handleBDXTransferSessionBegin(long nodeId, String fileDesignator, long offset);

  /**
   * The callback will be invoked when a BDX Query message has been received for some node.
   *
   * @param nodeId OTA Requester's Node ID
   * @param blockSize Max blockSize that can be sent
   * @param blockIndex block Index value
   * @param bytesToSkip Bytes to skip
   * @return BDXData sending packet in image, See detailed in {@link OTAProviderDelegate#BDXData}
   */
  public BDXData handleBDXQuery(long nodeId, int blockSize, long blockIndex, long bytesToSkip);

  /**
   * The callback will be invoked when BDX Transfer session is ended.
   *
   * @param nodeId errorCode if transfer is failed.
   * @param nodeId OTA Requester's Node ID
   */
  public void handleBDXTransferSessionEnd(long errorCode, long nodeId);

  /**
   * Called to handle an ApplyUpdateRequest command and is responsible for sending the response (if
   * success) or status (if error).
   *
   * @param nodeId OTA Requester's Node ID
   * @param newVersion New version to update
   * @return ApplyUpdateResponse response for future action. See detailed in {@link
   *     OTAProvider#ApplyUpdateResponse}
   */
  public ApplyUpdateResponse handleApplyUpdateRequest(long nodeId, long newVersion);

  /**
   * Called to handle a NotifyUpdateApplied command and is responsible for sending the status.
   *
   * @param nodeId OTA Requester's Node ID
   */
  public default void handleNotifyUpdateApplied(long nodeId) {}

  /**
   * This is a class used when transmitting using the BDX Protocol. If isEOF value set true, OTA
   * Requester treats the data transfer as finished.
   */
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

  /** This is a class used to send Image information. */
  public class QueryImageResponse {
    @Nullable Long softwareVersion;
    @Nullable String softwareVersionString;
    @Nullable String filePath;
    QueryImageResponseStatusEnum status;
    @Nullable Boolean userConsentNeeded = null;
    @Nullable Long delayedActionTime = null;

    public QueryImageResponse(
        @Nullable Long softwareVersion,
        @Nullable String softwareVersionString,
        String filePath,
        Boolean userConsentNeeded) {
      this.status = QueryImageResponseStatusEnum.UpdateAvailable;
      this.softwareVersion = softwareVersion;
      this.softwareVersionString = softwareVersionString;
      this.filePath = filePath;
      this.userConsentNeeded = userConsentNeeded;
    }

    public QueryImageResponse(
        QueryImageResponseStatusEnum status, long delayedActionTime, Boolean userConsentNeeded) {
      this.status = status;
      this.delayedActionTime = delayedActionTime;
      this.userConsentNeeded = userConsentNeeded;
    }

    public QueryImageResponse(QueryImageResponseStatusEnum status, Boolean userConsentNeeded) {
      this.status = status;
      this.userConsentNeeded = userConsentNeeded;
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

    public int getStatus() {
      return status.getValue();
    }

    public Long getDelayedActionTime() {
      return delayedActionTime;
    }

    public Boolean getUserConsentNeeded() {
      return userConsentNeeded;
    }
  }

  // See Refer CompatEnumNames.h
  public enum QueryImageResponseStatusEnum {
    // Enum for QueryImageResponseStatusEnum
    UpdateAvailable(0x00),
    Busy(0x01),
    NotAvailable(0x02),
    DownloadProtocolNotSupported(0x03);

    private int value;

    QueryImageResponseStatusEnum(int value) {
      this.value = value;
    }

    public int getValue() {
      return value;
    }
  }

  /**
   * This class is used to inform whether the OTA Requester starts updating. Class can set Delay
   * Time and Enum value for next action. See {@link OTAProviderDelegate#ApplyUpdateActionEnum} for
   * more information.
   */
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
  /**
   * The Action field SHALL express the action that the OTA Provider requests from the OTA
   * Requestor.
   */
  public enum ApplyUpdateActionEnum {
    /** Apply the update. */
    Proceed(0),
    /** Wait at least the given delay time. */
    AwaitNextAction(1),
    /** The OTA Provider is conveying a desire to rescind a previously provided Software Image. */
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
