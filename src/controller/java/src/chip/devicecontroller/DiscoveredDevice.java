/*
 *   Copyright (c) 2020-2021 Project CHIP Authors
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

import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

public class DiscoveredDevice {
  public long discriminator;
  public String ipAddress;
  public int port;
  public long deviceType;
  public int vendorId;
  public int productId;
  public Set<PairingHintBitmap> pairingHint;
  public CommissioningWindowStatus commissioningMode;
  public byte[] rotatingId;
  public String instanceName;
  public String deviceName;
  public String pairingInstruction;

  // For use in JNI.
  private void setCommissioningMode(int value) {
    this.commissioningMode = CommissioningWindowStatus.value(value);
  }

  private void setPairingHint(int value) {
    this.pairingHint = new HashSet<>();
    for (PairingHintBitmap mode : PairingHintBitmap.values()) {
      int bitmask = 1 << mode.getBitIndex();
      if ((value & bitmask) != 0) {
        pairingHint.add(mode);
      }
    }
  }

  @Override
  public String toString() {
    return "DiscoveredDevice : {"
        + "\n\tdiscriminator : "
        + discriminator
        + "\n\tipAddress : "
        + ipAddress
        + "\n\tport : "
        + port
        + "\n\tdeviceType : "
        + deviceType
        + "\n\tvendorId : "
        + vendorId
        + "\n\tproductId : "
        + productId
        + "\n\tpairingHint : "
        + pairingHint
        + "\n\tcommissioningMode : "
        + commissioningMode
        + "\n\trotatingId : "
        + (rotatingId != null ? Arrays.toString(rotatingId) : "null")
        + "\n\tinstanceName : "
        + instanceName
        + "\n\tdeviceName : "
        + deviceName
        + "\n\tpairingInstruction : "
        + pairingInstruction
        + "\n}";
  }
}
