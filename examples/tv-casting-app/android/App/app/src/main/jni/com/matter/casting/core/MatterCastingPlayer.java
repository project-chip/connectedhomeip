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
 */
package com.matter.casting.core;

import android.util.Log;
import java.net.InetAddress;
import java.util.List;
import java.util.Objects;

/** The Matter Commissioner (e.g. TV) discovered using Matter Commissioner discovery. */
public class MatterCastingPlayer implements CastingPlayer {
  private static final String TAG = MatterCastingPlayer.class.getSimpleName();
  private boolean connected;
  private String deviceId;
  private String deviceName;
  private String hostName;
  private String instanceName;
  private int numberIPs;
  private List<InetAddress> ipAddresses;
  private int port;
  private int productId;
  private int vendorId;
  private int deviceType;

  public MatterCastingPlayer(
      boolean connected,
      String deviceId,
      String hostName,
      String deviceName,
      String instanceName,
      int numberIPs,
      List<InetAddress> ipAddresses,
      int port,
      int productId,
      int vendorId,
      int deviceType) {
    Log.d(TAG, "MatterCastingPlayer() constructor, building player with deviceId: " + deviceId);
    Log.d(TAG, "MatterCastingPlayer() constructor, building player with deviceName: " + deviceName);
    this.connected = connected;
    this.deviceId = deviceId;
    this.hostName = hostName;
    this.deviceName = deviceName;
    this.instanceName = instanceName;
    this.numberIPs = numberIPs;
    this.ipAddresses = ipAddresses;
    this.port = port;
    this.productId = productId;
    this.vendorId = vendorId;
    this.deviceType = deviceType;
  }

  @Override
  public boolean isConnected() {
    return this.connected;
  }

  @Override
  public String getDeviceId() {
    return this.deviceId;
  }

  @Override
  public String getHostName() {
    return this.hostName;
  }

  @Override
  public String getDeviceName() {
    return this.deviceName;
  }

  @Override
  public String getInstanceName() {
    return this.instanceName;
  }

  @Override
  public int getNumberIPs() {
    return this.numberIPs;
  }

  @Override
  public List<InetAddress> getIpAddresses() {
    return this.ipAddresses;
  }

  @Override
  public int getPort() {
    return this.port;
  }

  @Override
  public int getVendorId() {
    return this.vendorId;
  }

  @Override
  public int getProductId() {
    return this.productId;
  }

  @Override
  public int getDeviceType() {
    return this.deviceType;
  }

  @Override
  public boolean discoveredCastingPlayerHasSameSource(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;
    MatterCastingPlayer that = (MatterCastingPlayer) o;
    return Objects.equals(deviceId, that.deviceId) && vendorId == that.vendorId;
  }
}
