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
package chip.platform;

import androidx.annotation.Nullable;

public class NetworkInterface {

  public static final int INTERFACE_TYPE_UNSPECIFIED = 0x00;
  public static final int INTERFACE_TYPE_WI_FI = 0x01;
  public static final int INTERFACE_TYPE_ETHERNET = 0x02;
  public static final int INTERFACE_TYPE_CELLULAR = 0x03;
  public static final int INTERFACE_TYPE_THREAD = 0x04;

  public String name;
  public boolean isOperational;

  @Nullable public Boolean offPremiseServicesReachableIPv4;
  @Nullable public Boolean offPremiseServicesReachableIPv6;
  public byte[] hardwareAddress;
  public byte[] ipv4Address;
  public byte[] ipv6Address;
  public int type;
}
