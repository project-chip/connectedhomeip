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

import java.net.InetAddress;
import java.util.List;
import java.util.Objects;

/**
 * A Matter Casting Player represents a Matter commissioner that is able to play media to a physical
 * output or to a display screen which is part of the device (e.g. TV). It is discovered on the
 * local network using Matter Commissioner discovery over DNS. It contains all the information about
 * the service discovered/resolved.
 */
public class MatterCastingPlayer implements CastingPlayer {
  private boolean connected;
  private String deviceId;
  private String deviceName;
  private String hostName;
  private String instanceName;
  private List<InetAddress> ipAddresses;
  private int port;
  private int productId;
  private int vendorId;
  private long deviceType;

  public MatterCastingPlayer(
      boolean connected,
      String deviceId,
      String hostName,
      String deviceName,
      String instanceName,
      List<InetAddress> ipAddresses,
      int port,
      int productId,
      int vendorId,
      long deviceType) {
    this.connected = connected;
    this.deviceId = deviceId;
    this.hostName = hostName;
    this.deviceName = deviceName;
    this.instanceName = instanceName;
    this.ipAddresses = ipAddresses;
    this.port = port;
    this.productId = productId;
    this.vendorId = vendorId;
    this.deviceType = deviceType;
  }

  /**
   * @return a boolean indicating whether a Casting Player instance is connected to the TV Casting
   *     App.
   */
  @Override
  public boolean isConnected() {
    return this.connected;
  }

  /**
   * @return a String representing the Casting Player device ID which is a concatenation of the
   *     device's IP address and port number.
   */
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

  /** @return a list of valid IP addresses for this Casting PLayer. */
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
  public long getDeviceType() {
    return this.deviceType;
  }

  @Override
  public String toString() {
    return this.deviceId;
  }

  @Override
  public int hashCode() {
    return this.deviceId.hashCode();
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;
    MatterCastingPlayer that = (MatterCastingPlayer) o;
    return Objects.equals(this.deviceId, that.deviceId);
  }
}
