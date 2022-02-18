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

public class NetworkInterface {

  public static final int INTERFACE_TYPE_UNSPECIFIED = 0x00;
  public static final int INTERFACE_TYPE_WI_FI = 0x01;
  public static final int INTERFACE_TYPE_ETHERNET = 0x02;
  public static final int INTERFACE_TYPE_CELLULAR = 0x03;
  public static final int INTERFACE_TYPE_THREAD = 0x04;

  public static final int REACHABLE_UNKNOWN = 0;
  public static final int REACHABLE_YES = 1;
  public static final int REACHABLE_NO = 2;

  public String name;
  public boolean isOperational;
  public int offPremiseServicesReachableIPv4;
  public int offPremiseServicesReachableIPv6;
  public byte[] hardwareAddress;
  public int type;
}
