/*
 *   Copyright (c) 2024 Project CHIP Authors
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

public enum PairingHintBitmap {
  PowerCycle(0, false),
  DeviceManufacturerURL(1, false),
  Administrator(2, false),
  SettingsMenuOnTheNode(3, false),
  CustomInstruction(4, true),
  DeviceManual(5, false),
  PressResetButton(6, false),
  PressResetButtonWithApplicationOfPower(7, false),
  PressResetButtonForNseconds(8, true),
  PressResetButtonUntilLightBlinks(9, true),
  PressResetButtonForNsecondsWithApplicationOfPower(10, true),
  PressResetButtonUntilLightBlinksWithApplicationOfPower(11, true),
  PressResetButtonNTimes(12, true),
  PressSetupButton(13, false),
  PressSetupButtonWithApplicationOfPower(14, false),
  PressSetupButtonForNseconds(15, true),
  PressSetupButtonUntilLightBlinks(16, true),
  PressSetupButtonForNsecondsWithApplicationOfPower(17, true),
  PressSetupButtonUntilLightBlinksWithApplicationOfPower(18, true),
  PressSetupButtonNtimes(19, true);

  private final int bitIndex;
  private final boolean isRequirePairingInstruction;

  PairingHintBitmap(int bitIndex, boolean isRequirePairingInstruction) {
    this.bitIndex = bitIndex;
    this.isRequirePairingInstruction = isRequirePairingInstruction;
  }

  public int getBitIndex() {
    return bitIndex;
  }

  public boolean getRequirePairingInstruction() {
    return isRequirePairingInstruction;
  }
}
