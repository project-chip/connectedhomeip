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

import android.net.nsd.NsdServiceInfo;
import android.util.Log;
import java.net.InetAddress;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Objects;

public class DiscoveredNodeData {
  private static final String TAG = DiscoveredNodeData.class.getSimpleName();

  private static final int MAX_IP_ADDRESSES = 5;
  private static final int MAX_ROTATING_ID_LEN = 50;
  private static final String KEY_DEVICE_NAME = "DN";
  private static final String KEY_DEVICE_TYPE = "DT";
  private static final String KEY_VENDOR_PRODUCT = "VP";

  private String hostName;
  private String instanceName;
  private long longDiscriminator;
  private long vendorId;
  private long productId;
  private byte commissioningMode;
  private long deviceType;
  private String deviceName;
  private byte rotatingId[] = new byte[MAX_ROTATING_ID_LEN];
  private int rotatingIdLen;
  private short pairingHint;
  private String pairingInstruction;
  private int port;
  private int numIPs;
  private List<InetAddress> ipAddresses;

  private VideoPlayer connectableVideoPlayer;

  public DiscoveredNodeData(NsdServiceInfo serviceInfo) {
    Map<String, byte[]> attributes = serviceInfo.getAttributes();

    if (attributes != null) {
      if (attributes.get(KEY_DEVICE_NAME) != null) {
        this.deviceName = new String(attributes.get(KEY_DEVICE_NAME), StandardCharsets.UTF_8);
      } else {
        Log.e(TAG, "No device name (DN) found in DiscovoredNodeData");
      }

      if (attributes.get(KEY_DEVICE_TYPE) != null) {
        try {
          this.deviceType =
              Long.parseLong(new String(attributes.get(KEY_DEVICE_TYPE), StandardCharsets.UTF_8));
        } catch (NumberFormatException e) {
          Log.e(TAG, "Could not parse TXT record for DT: " + e.getMessage());
        }
      } else {
        Log.e(TAG, "TXT Record for DT was null");
      }

      if (attributes.get(KEY_VENDOR_PRODUCT) != null) {
        String vp = new String(attributes.get(KEY_VENDOR_PRODUCT), StandardCharsets.UTF_8);
        if (vp != null) {
          String[] vpArray = vp.split("\\+");
          try {
            if (vpArray.length > 0) {
              this.vendorId = Long.parseLong(vpArray[0]);
              if (vpArray.length == 2) {
                this.productId = Long.parseLong(vpArray[1]);
              }
            }
          } catch (NumberFormatException e) {
            Log.e(TAG, "Could not parse TXT record for VP: " + e.getMessage());
          }
        }
      } else {
        Log.e(TAG, "TXT Record for VP was null");
      }
    } else {
      Log.e(TAG, "NsdServiceInfo.attributes was null");
    }

    if (serviceInfo.getHost() != null) {
      this.hostName = serviceInfo.getHost().getHostName();
      this.ipAddresses = Arrays.asList(serviceInfo.getHost());
    } else {
      Log.e(TAG, "Host name was null");
    }
    this.port = serviceInfo.getPort();
    this.numIPs = 1;
  }

  public DiscoveredNodeData(VideoPlayer player) {
    this.connectableVideoPlayer = player;
    this.instanceName = player.getInstanceName();
    this.hostName = player.getHostName();
    this.deviceName = player.getDeviceName();
    this.deviceType = player.getDeviceType();
    this.vendorId = player.getVendorId();
    this.productId = player.getProductId();
    this.numIPs = player.getNumIPs();
    this.ipAddresses = player.getIpAddresses();
    this.port = player.getPort();
  }

  void setConnectableVideoPlayer(VideoPlayer videoPlayer) {
    this.connectableVideoPlayer = videoPlayer;
  }

  public boolean isPreCommissioned() {
    return connectableVideoPlayer != null;
  }

  public VideoPlayer toConnectableVideoPlayer() {
    return connectableVideoPlayer;
  }

  public String getHostName() {
    return hostName;
  }

  public String getInstanceName() {
    return instanceName;
  }

  public long getLongDiscriminator() {
    return longDiscriminator;
  }

  public long getVendorId() {
    return vendorId;
  }

  public long getProductId() {
    return productId;
  }

  public byte getCommissioningMode() {
    return commissioningMode;
  }

  public long getDeviceType() {
    return deviceType;
  }

  public String getDeviceName() {
    return deviceName;
  }

  public byte[] getRotatingId() {
    return rotatingId;
  }

  public int getRotatingIdLen() {
    return rotatingIdLen;
  }

  public short getPairingHint() {
    return pairingHint;
  }

  public String getPairingInstruction() {
    return pairingInstruction;
  }

  public int getPort() {
    return port;
  }

  public int getNumIPs() {
    return numIPs;
  }

  public List<InetAddress> getIpAddresses() {
    return ipAddresses;
  }

  @Override
  public String toString() {
    return "DiscoveredNodeData{"
        + "hostName='"
        + hostName
        + '\''
        + ", instanceName='"
        + instanceName
        + '\''
        + ", longDiscriminator="
        + longDiscriminator
        + ", vendorId="
        + vendorId
        + ", productId="
        + productId
        + ", commissioningMode="
        + commissioningMode
        + ", deviceType="
        + deviceType
        + ", deviceName='"
        + deviceName
        + '\''
        + ", rotatingId="
        + Arrays.toString(rotatingId)
        + ", rotatingIdLen="
        + rotatingIdLen
        + ", pairingHint="
        + pairingHint
        + ", pairingInstruction='"
        + pairingInstruction
        + '\''
        + ", port="
        + port
        + ", numIPs="
        + numIPs
        + ", ipAddresses="
        + ipAddresses
        + '}';
  }

  /**
   * Checks to see if a discovered node is "effectively equal" to another by comparing the
   * parameters that should not change.
   *
   * @param o the object to compare to.
   * @return true if the objects are from the same source, false otherwise.
   */
  public boolean discoveredNodeHasSameSource(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;
    DiscoveredNodeData that = (DiscoveredNodeData) o;
    return vendorId == that.vendorId
        && productId == that.productId
        && commissioningMode == that.commissioningMode
        && deviceType == that.deviceType
        && Objects.equals(hostName, that.hostName);
  }
}
