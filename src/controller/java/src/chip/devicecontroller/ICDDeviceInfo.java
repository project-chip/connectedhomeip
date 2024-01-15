/*
 *   Copyright (c) 2020-2023 Project CHIP Authors
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

import java.util.HashSet;
import java.util.Set;

/** Class for holding ICD Device information. */
public class ICDDeviceInfo {
  /** List of methods to trigger the ICD to switch to Active mode. */
  public enum UserActiveModeTriggerBitmap {
    PowerCycle(0),
    SettingsMenu(1),
    CustomInstruction(2),
    DeviceManual(3),
    ActuateSensor(4),
    ActuateSensorSeconds(5),
    ActuateSensorTimes(6),
    ActuateSensorLightsBlink(7),
    ResetButton(8),
    ResetButtonLightsBlink(9),
    ResetButtonSeconds(10),
    ResetButtonTimes(11),
    SetupButton(12),
    SetupButtonSeconds(13),
    SetupButtonLightsBlink(14),
    SetupButtonTimes(15),
    AppDefinedButton(16);

    private final int bitIndex;

    UserActiveModeTriggerBitmap(int bitIndex) {
      this.bitIndex = bitIndex;
    }

    public int getBitIndex() {
      return bitIndex;
    }
  }

  private final byte[] symmetricKey;
  private final Set<UserActiveModeTriggerBitmap> userActiveModeTriggerHint;
  private final String userActiveModeTriggerInstruction;
  private final long icdNodeId;
  private final long icdCounter;
  private final long monitoredSubject;
  private final long fabricId;
  private final int fabricIndex;

  ICDDeviceInfo(
      byte[] symmetricKey,
      Set<UserActiveModeTriggerBitmap> userActiveModeTriggerHint,
      String userActiveModeTriggerInstruction,
      long icdNodeId,
      long icdCounter,
      long monitoredSubject,
      long fabricId,
      int fabricIndex) {
    this.symmetricKey = symmetricKey;
    this.userActiveModeTriggerHint = userActiveModeTriggerHint;
    this.userActiveModeTriggerInstruction = userActiveModeTriggerInstruction;
    this.icdNodeId = icdNodeId;
    this.icdCounter = icdCounter;
    this.monitoredSubject = monitoredSubject;
    this.fabricId = fabricId;
    this.fabricIndex = fabricIndex;
  }

  ICDDeviceInfo(
      byte[] symmetricKey,
      int userActiveModeTriggerHintRaw,
      String userActiveModeTriggerInstruction,
      long icdNodeId,
      long icdCounter,
      long monitoredSubject,
      long fabricId,
      int fabricIndex) {
    this.symmetricKey = symmetricKey;
    this.userActiveModeTriggerInstruction = userActiveModeTriggerInstruction;
    this.icdNodeId = icdNodeId;
    this.icdCounter = icdCounter;
    this.monitoredSubject = monitoredSubject;
    this.fabricId = fabricId;
    this.fabricIndex = fabricIndex;

    this.userActiveModeTriggerHint = new HashSet<>();
    for (UserActiveModeTriggerBitmap mode : UserActiveModeTriggerBitmap.values()) {
      int bitmask = 1 << mode.getBitIndex();
      if ((userActiveModeTriggerHintRaw & bitmask) != 0) {
        userActiveModeTriggerHint.add(mode);
      }
    }
  }

  /** Returns the 16 bytes ICD symmetric key. */
  public byte[] getSymmetricKey() {
    return symmetricKey;
  }

  /** Returns the Set of UserActiveModeTriggerHint. */
  public Set<UserActiveModeTriggerBitmap> getUserActiveModeTriggerHint() {
    return userActiveModeTriggerHint;
  }

  /** Returns the UserActiveModeTriggerInstruction. */
  public String getUserActiveModeTriggerInstruction() {
    return userActiveModeTriggerInstruction;
  }

  /** Returns the ICD Node Id. */
  public long getIcdNodeId() {
    return icdNodeId;
  }

  /** Returns the ICD Counter. */
  public long getIcdCounter() {
    return icdCounter;
  }

  /** Returns the Monitored Subject. */
  public long getMonitoredSubject() {
    return monitoredSubject;
  }

  /** Returns the Fabric Id */
  public long getFabricId() {
    return fabricId;
  }

  /** Returns the Fabric Index */
  public int getFabricIndex() {
    return fabricIndex;
  }

  @Override
  public String toString() {
    return "ICDDeviceInfo : {"
        + "\n\tuserActiveModeTriggerHint : "
        + userActiveModeTriggerHint
        + "\n\tuserActiveModeTriggerInstruction : "
        + userActiveModeTriggerInstruction
        + "\n\ticdNodeId : "
        + icdNodeId
        + "\n\ticdCounter : "
        + icdCounter
        + "\n\tmonitoredSubject : "
        + monitoredSubject
        + "\n\tfabricId : "
        + fabricId
        + "\n\tfabricIndex : "
        + fabricIndex
        + "\n}";
  }
}
