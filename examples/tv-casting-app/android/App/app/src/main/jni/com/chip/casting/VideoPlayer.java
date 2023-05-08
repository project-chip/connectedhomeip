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

import java.net.InetAddress;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

public class VideoPlayer {
  private static final String TAG = VideoPlayer.class.getSimpleName();

  private long nodeId;
  private byte fabricIndex;
  private String deviceName;
  private int vendorId;
  private int productId;
  private int deviceType;
  private List<ContentApp> contentApps;
  private boolean isConnected = false;

  private int numIPs;
  private List<InetAddress> ipAddresses;
  private String hostName;

  private boolean isInitialized = false;

  public VideoPlayer(
      long nodeId,
      byte fabricIndex,
      String deviceName,
      int vendorId,
      int productId,
      int deviceType,
      List<ContentApp> contentApps,
      int numIPs,
      List<InetAddress> ipAddresses,
      String hostName,
      boolean isConnected) {
    this.nodeId = nodeId;
    this.fabricIndex = fabricIndex;
    this.deviceName = deviceName;
    this.vendorId = vendorId;
    this.productId = productId;
    this.deviceType = deviceType;
    this.contentApps = contentApps;
    this.isConnected = isConnected;
    this.numIPs = numIPs;
    this.ipAddresses = ipAddresses;
    this.hostName = hostName;
    this.isInitialized = true;
  }

  public boolean isSameAs(DiscoveredNodeData discoveredNodeData) {
    // return false because 'this' VideoPlayer is not null
    if (discoveredNodeData == null) {
      return false;
    }

    // return true if hostNames match
    if (Objects.equals(hostName, discoveredNodeData.getHostName())) {
      return true;
    }

    // return false because deviceNames are different
    if (Objects.equals(deviceName, discoveredNodeData.getDeviceName()) == false) {
      return false;
    }

    // return false because not even a single IP Address matches
    if (ipAddresses != null) {
      boolean matchFound = false;
      Set<InetAddress> discoveredNodeDataIpAddressSet =
          new HashSet<InetAddress>(discoveredNodeData.getIpAddresses());
      for (InetAddress videoPlayerIpAddress : ipAddresses) {
        if (discoveredNodeDataIpAddressSet.contains(videoPlayerIpAddress)) {
          matchFound = true;
          break;
        }
      }

      if (!matchFound) {
        return false;
      }
    }

    // otherwise, return true
    return true;
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
        + ", deviceType="
        + deviceType
        + ", contentApps="
        + contentApps
        + ", isConnected="
        + isConnected
        + ", numIPs="
        + numIPs
        + ", ipAddresses="
        + ipAddresses
        + ", isInitialized="
        + ", hostName='"
        + hostName
        + '\''
        + isInitialized
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
