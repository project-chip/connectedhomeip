#!/usr/bin/env -S python3 -B
#
#    Copyright (c) 2024 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import sys

from common_icdm_data import ICDMData, c, run_tests, uat

long_string = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut e"
too_long_string = long_string + "1"

TEST_CASES = [

    # ==============================
    # ICDM 2.1 Test cases
    # ==============================
    # --------
    # Test cases to validate IdleModeDuration
    # --------
    # IdleModeDuration under minimum (< 1)
    ICDMData(0, 0, 0, 100, [], 0, 2, 0, "",
             c.Enums.OperatingModeEnum.kSit, 64800, False),
    # IdleModeDuration at minimum
    ICDMData(0, 1, 0, 100, [], 0, 2, 0, "",
             c.Enums.OperatingModeEnum.kSit, 64800, True),
    # IdleModeDuration at maximum
    ICDMData(0, 64800, 100, 100, [], 0, 2, 0, "",
             c.Enums.OperatingModeEnum.kSit,  64800, True),
    # IdleModeDuration over maximum (>64800)
    ICDMData(0, 64801, 100, 100, [], 0, 2, 0, "",
             c.Enums.OperatingModeEnum.kSit, 64800, False),
    # IdleModeDuration < ActiveModeDuration
    ICDMData(0, 1, 1001, 100, [], 0, 2, 0, "",
             c.Enums.OperatingModeEnum.kSit, 64800, False),
    # --------
    # Test cases to validate ActiveModeDuration
    # --------
    # ActiveModeDuration under minimum
    ICDMData(0, 100, -1, 100, [], 0, 2, 0, "",
             c.Enums.OperatingModeEnum.kSit, 64800, False),
    # ActiveModeDuration at minimum
    ICDMData(0, 100, 0, 100, [], 0, 2, 0, "",
             c.Enums.OperatingModeEnum.kSit, 64800, True),
    # ActiveModeDuration at maximum - value is max IdleModeDuration value - 1
    ICDMData(0, 64800, 0x3DCC4FF, 100, [], 0, 2, 0, "",
             c.Enums.OperatingModeEnum.kSit, 64800, True),
    # --------
    # Test cases to validate ActiveModeThreshold
    # --------
    # ActiveModeThreshold < minimum
    ICDMData(0, 1, 0, -1, [], 0, 2, 0, "",
             c.Enums.OperatingModeEnum.kSit, 64800, False),
    # ActiveModeThreshold at SIT minimum
    ICDMData(0, 1, 0, 0, [], 0, 2, 0, "",
             c.Enums.OperatingModeEnum.kSit, 64800, True),
    # ActiveModeThreshold under LIT minimum
    ICDMData(0x7, 1, 0, 4999, [], 0, 2, 0, "",
             c.Enums.OperatingModeEnum.kLit, 64800, False),
    # ActiveModeThreshold at LIT minimum
    ICDMData(0x7, 1, 0, 5000, [], 0, 2, 0, "",
             c.Enums.OperatingModeEnum.kLit, 64800, True),
    # ActiveModeThreshold at Maximum
    ICDMData(0, 1, 0, 0xFFFF, [], 0, 2, 0, "",
             c.Enums.OperatingModeEnum.kSit, 64800, True),
    # ActiveModeThreshold over Maximum
    ICDMData(0, 1, 0, 0x10000, [], 0, 2, 0, "",
             c.Enums.OperatingModeEnum.kSit, 64800, False),
    # --------
    # Test cases to validate ClientsSupportedPerFabric
    # --------
    # ClientsSupportedPerFabric under minimum (< 1)
    ICDMData(0, 1, 0, 100, [], 0, 0, 0, "",
             c.Enums.OperatingModeEnum.kLit, 64800, False),
    # ClientsSupportedPerFabric at minimum
    ICDMData(0, 1, 0, 100, [], 0, 1, 0, "",
             c.Enums.OperatingModeEnum.kLit, 64800, True),
    # ClientsSupportedPerFabric at maximum
    ICDMData(0, 1, 0, 100, [], 0, 255, 0, "",
             c.Enums.OperatingModeEnum.kLit, 64800, True),
    # ClientsSupportedPerFabric > maximum
    ICDMData(0, 1, 0, 100, [], 0, 256, 0, "",
             c.Enums.OperatingModeEnum.kLit, 64800, True),
    # --------
    # Test cases to validate RegisteredClients
    # --------
    # Incorrect type
    ICDMData(0, 1, 0, 100, 0, 0, 1, 0, "",
             c.Enums.OperatingModeEnum.kLit, 64800, False),
    # Correct type
    ICDMData(0, 1, 0, 100, [], 0, 1, 0, "",
             c.Enums.OperatingModeEnum.kLit, 64800, True),
    # --------
    # Test cases to validate ICDCounter
    # --------
    # ICDCounter under minimum (< 0)
    ICDMData(0, 1, 0, 100, [], -1, 1, 0, "",
             c.Enums.OperatingModeEnum.kLit, 64800, False),
    # ICDCounter at minimum
    ICDMData(0, 1, 0, 100, [], 0, 1, 0, "",
             c.Enums.OperatingModeEnum.kLit, 64800, True),
    # ICDCounter at maximum
    ICDMData(0, 1, 0, 100, [], 0xFFFFFFFF, 1, 0, "",
             c.Enums.OperatingModeEnum.kLit, 64800, True),
    # ICDCounter over maximum
    ICDMData(0, 1, 0, 100, [], 0x100000000, 1, 0, "",
             c.Enums.OperatingModeEnum.kLit, 64800, False),
    # --------
    # Test cases to validate UserActiveModeTriggerHint
    # --------
    # UserActiveModeTriggerHint outsite valid range
    ICDMData(0, 1, 0, 100, [], 0, 1, 0x1FFFF, "",
             c.Enums.OperatingModeEnum.kLit, 64800, False),
    # UserActiveModeTriggerHint outsite valid range
    ICDMData(0, 1, 0, 100, [], 0, 1, -1, "",
             c.Enums.OperatingModeEnum.kLit, 64800, False),
    # UserActiveModeTriggerHint with no hints
    ICDMData(0, 1, 0, 100, [], 0, 1, 0, "",
             c.Enums.OperatingModeEnum.kLit, 64800, True),
    # UserActiveModeTriggerHint wiht two instruction depedent bits set
    ICDMData(0, 1, 0, 100, [], 0, 1, uat.kCustomInstruction | uat.kActuateSensorSeconds, "",
             c.Enums.OperatingModeEnum.kLit, 64800, False),
    # --------
    # Test cases to validate UserActiveModeTriggerInstruction
    # --------
    # UserActiveModeTriggerInstruction with wrong encoding
    ICDMData(0, 1, 0, 100, [], 0, 1, uat.kCustomInstruction, "Hello\uD83D\uDE00World",
             c.Enums.OperatingModeEnum.kLit, 64800, False),
    # UserActiveModeTriggerInstruction with empty string
    ICDMData(0, 1, 0, 100, [], 0, 1, uat.kCustomInstruction, "",
             c.Enums.OperatingModeEnum.kLit, 64800, True),
    # UserActiveModeTriggerInstruction with empty string
    ICDMData(0, 1, 0, 100, [], 0, 1, uat.kCustomInstruction, "",
             c.Enums.OperatingModeEnum.kLit, 64800, True),
    # UserActiveModeTriggerInstruction with max string length
    ICDMData(0, 1, 0, 100, [], 0, 1, uat.kCustomInstruction, long_string,
             c.Enums.OperatingModeEnum.kLit, 64800, True),
    # UserActiveModeTriggerInstruction > max string length
    ICDMData(0, 1, 0, 100, [], 0, 1, uat.kCustomInstruction, too_long_string,
             c.Enums.OperatingModeEnum.kLit, 64800, False),
    # UserActiveModeTriggerInstruction invalid number - Trailing 0s
    ICDMData(0, 1, 0, 100, [], 0, 1, uat.kActuateSensorSeconds, "001",
             c.Enums.OperatingModeEnum.kLit, 64800, False),
    # UserActiveModeTriggerInstruction invalid number - Letters
    ICDMData(0, 1, 0, 100, [], 0, 1,  uat.kActuateSensorSeconds, "not a number",
             c.Enums.OperatingModeEnum.kLit, 64800, False),
    # UserActiveModeTriggerInstruction Valid number
    ICDMData(0, 1, 0, 100, [], 0, 1,  uat.kActuateSensorSeconds, "100000",
             c.Enums.OperatingModeEnum.kLit, 64800, True),
    ICDMData(0, 1, 0, 100, [], 0, 1, uat.kActuateSensorLightsBlink, "", c.Enums.OperatingModeEnum.kLit, 64800, False),
    ICDMData(0, 1, 0, 100, [], 0, 1, uat.kActuateSensorLightsBlink, "AAAAAAA", c.Enums.OperatingModeEnum.kLit, 64800, False),
    ICDMData(0, 1, 0, 100, [], 0, 1, uat.kActuateSensorLightsBlink, "AAAAA", c.Enums.OperatingModeEnum.kLit, 64800, False),
    ICDMData(0, 1, 0, 100, [], 0, 1, uat.kActuateSensorLightsBlink, "AAAAAK", c.Enums.OperatingModeEnum.kLit, 64800, False),
    ICDMData(0, 1, 0, 100, [], 0, 1, uat.kActuateSensorLightsBlink, "012345", c.Enums.OperatingModeEnum.kLit, 64800, True),
    # --------
    # Test cases to validate OpertingMode
    # --------
    # OpertingMode with negative value
    ICDMData(0, 1, 0, 100, [], 0, 1,  uat.kActuateSensorSeconds, "100000",
             -1, 64800, False),
    # OpertingMode with Accepted value
    ICDMData(0, 1, 0, 100, [], 0, 1,  uat.kActuateSensorSeconds, "100000",
             c.Enums.OperatingModeEnum.kLit, 64800, True),
    # OpertingMode with unkown value
    ICDMData(0, 1, 0, 100, [], 0, 1,  uat.kActuateSensorSeconds, "100000",
             c.Enums.OperatingModeEnum.kUnknownEnumValue, 64800, False),
    # --------
    # Test cases to validate MaximumCheckInBackOff
    # --------
    ICDMData(0, 1, 0, 100, [], 0, 1,  uat.kActuateSensorSeconds, "100000",
             c.Enums.OperatingModeEnum.kUnknownEnumValue, 0, False),
    ICDMData(0, 1, 0, 100, [], 0, 1,  uat.kActuateSensorSeconds, "100000",
             c.Enums.OperatingModeEnum.kSit, 1, True),
    ICDMData(0, 1, 0, 100, [], 0, 1,  uat.kActuateSensorSeconds, "100000",
             c.Enums.OperatingModeEnum.kSit, 64800, True),
    ICDMData(0, 1, 0, 100, [], 0, 1,  uat.kActuateSensorSeconds, "100000",
             c.Enums.OperatingModeEnum.kSit, 64801, False),

]


def main():
    pics = {"ICDM.S.A0000": True, "ICDM.S.A0001": True, "ICDM.S.A0002": True, "ICDM.S.A0003": True, "ICDM.S.A0004": True,
            "ICDM.S.A0005": True, "ICDM.S.A0006": True, "ICDM.S.A0007": True, "ICDM.S.A0008": True, "ICDM.S.A0009": True, }

    return run_tests(pics, 'TC_ICDM_2_1', TEST_CASES, 'test_TC_ICDM_2_1')


if __name__ == "__main__":
    sys.exit(main())
