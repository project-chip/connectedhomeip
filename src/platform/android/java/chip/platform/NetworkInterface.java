/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.platform;

import androidx.annotation.Nullable;

public class NetworkInterface {

  public static final int INTERFACE_TYPE_ENUM_UNSPECIFIED = 0x00;
  public static final int INTERFACE_TYPE_ENUM_WI_FI = 0x01;
  public static final int INTERFACE_TYPE_ENUM_ETHERNET = 0x02;
  public static final int INTERFACE_TYPE_ENUM_CELLULAR = 0x03;
  public static final int INTERFACE_TYPE_ENUM_THREAD = 0x04;

  public String name;
  public boolean isOperational;

  @Nullable public Boolean offPremiseServicesReachableIPv4;
  @Nullable public Boolean offPremiseServicesReachableIPv6;
  public byte[] hardwareAddress;
  public byte[] ipv4Address;
  public byte[] ipv6Address;
  public int type;
}
