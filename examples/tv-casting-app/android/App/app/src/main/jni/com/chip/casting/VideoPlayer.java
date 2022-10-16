/*
 *   Copyright (c) 2022 Project CHIP Authors
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
package com.chip.casting;

import java.util.List;
import java.util.Objects;

public class VideoPlayer {
  private long nodeId;
  private byte fabricIndex;
  private String deviceName;
  private int vendorId;
  private int productId;
  private int deviceType;
  private List<ContentApp> contentApps;
  private boolean isConnected = false;

  private boolean isInitialized = false;

  public VideoPlayer(
      long nodeId,
      byte fabricIndex,
      String deviceName,
      int vendorId,
      int productId,
      int deviceType,
      List<ContentApp> contentApps,
      boolean isConnected) {
    this.nodeId = nodeId;
    this.fabricIndex = fabricIndex;
    this.deviceName = deviceName;
    this.vendorId = vendorId;
    this.productId = productId;
    this.deviceType = deviceType;
    this.contentApps = contentApps;
    this.isConnected = isConnected;
    this.isInitialized = true;
  }

  public boolean equals(Object object) {
    if (this == object) return true;
    if (object == null || getClass() != object.getClass()) return false;
    if (!super.equals(object)) return false;
    VideoPlayer that = (VideoPlayer) object;
    return nodeId == that.nodeId && fabricIndex == that.fabricIndex;
  }

  public int hashCode() {
    return Objects.hash(super.hashCode(), nodeId, fabricIndex);
  }

  @java.lang.Override
  public java.lang.String toString() {
    return "VideoPlayer{"
        + "nodeId="
        + nodeId
        + ", fabricIndex="
        + fabricIndex
        + ", deviceName='"
        + deviceName
        + '\''
        + ", vendorId="
        + vendorId
        + ", productId="
        + productId
        + ", isConnected="
        + isConnected
        + '}';
  }

  public long getNodeId() {
    return nodeId;
  }

  public byte getFabricIndex() {
    return fabricIndex;
  }

  public boolean isConnected() {
    return isConnected;
  }

  public List<ContentApp> getContentApps() {
    return contentApps;
  }

  public String getDeviceName() {
    return deviceName;
  }

  public int getVendorId() {
    return vendorId;
  }

  public int getProductId() {
    return productId;
  }

  public int getDeviceType() {
    return deviceType;
  }

  public boolean isInitialized() {
    return isInitialized;
  }
}
