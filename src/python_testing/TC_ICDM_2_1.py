#
#    Copyright (c) 2023 Project CHIP Authors
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
import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger('PythonMatterControllerTEST')
logger.setLevel(logging.INFO)


class TC_ICDM_2_1(MatterBaseTest):
    async def read_icdm_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.IcdManagement
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def pics_TC_ICDM_2_1(self) -> list[str]:
        return ["ICDM.S"]

    @async_test_body
    async def test_TC_ICDM_2_1(self):

        if not self.check_pics("ICDM.S"):
            logger.info("Test skipped because PICS ICDM.S is not set")
            return

        endpoint = self.user_params.get("endpoint", 0)

        self.print_step(1, "Commissioning, already done")
        attributes = Clusters.IcdManagement.Attributes
        userActiveModeTriggerHintBitmapMatch = False
        uint16Max = 65535
        uint32Max = 4294967295
        feature_map = await self.read_icdm_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        isCIP = feature_map & Clusters.IcdManagement.Bitmaps.Feature.kCheckInProtocolSupport
        # isUAT = feature_map & Clusters.IcdManagement.Bitmaps.Feature.kUserActiveModeTrigger
        # isLITS = feature_map & Clusters.IcdManagement.Bitmaps.Feature.kLongIdleTimeSupport

        # if isLITS:
        #     asserts.assert_true(isCIP and self.check_pics("ICDM.S.F00"), "CheckInProtocolSupport is a mandatory feature if LongIdleTimeSupport is set and must be present in the PICS file")
        #     asserts.assert_true(isUAT and self.check_pics("ICDM.S.F01"), "UserActiveModeTrigger is a mandatory feature if LongIdleTimeSupport is set and must be present in the PICS file")
        #     asserts.assert_true(self.check_pics("ICDM.S.F02"), "LongIdleTimeSupport is a mandatory feature if LongIdleTimeSupport is set and must be present in the PICS file")

        # Active Mode Threshold attribute test
        self.print_step(2, "Read activeModeThreshold Attribute")
        asserts.assert_true(self.check_pics("ICDM.S.A0002"), "ActiveModeThreshold is a mandatory attribute and must be present in the PICS file")
        ActiveModeThresholdMin = 0  # 5 if isLITS else 0
        activeModeThreshold = await self.read_icdm_attribute_expect_success(endpoint=endpoint, attribute=attributes.ActiveModeThreshold)
        logger.info(f"ActiveModeThreshold: {activeModeThreshold}")
        asserts.assert_true(ActiveModeThresholdMin <= activeModeThreshold <= uint16Max, "ActiveModeThreshold attribute does not fit in a uint16.")

        # Active Mode Duration attribute test
        self.print_step(3, "Read ActiveModeDuration Attribute")
        asserts.assert_true(self.check_pics("ICDM.S.A0001"), "ActiveModeDuration is a mandatory attribute and must be present in the PICS file")
        activeModeDuration = await self.read_icdm_attribute_expect_success(endpoint=endpoint, attribute=attributes.ActiveModeDuration)
        logger.info(f"ActiveModeDuration: {activeModeDuration}")
        asserts.assert_true(0 <= activeModeDuration <= uint32Max, "ActiveModeDuration attribute does not fit in a uint32.")
        asserts.assert_not_equal(activeModeDuration, activeModeThreshold, "ActiveModeDuration includes the ActiveModeThreshold.")

        # Idle Mode Duration attribute test
        self.print_step(4, "Read IdleModeDuration Attribute")
        asserts.assert_true(self.check_pics("ICDM.S.A0000"), "IdleModeDuration is a mandatory attribute and must be present in the PICS file")
        idleModeDuration = await self.read_icdm_attribute_expect_success(endpoint=endpoint, attribute=attributes.IdleModeDuration)
        logger.info(f"IdleModeDuration: {idleModeDuration}")
        asserts.assert_true(0 <= activeModeDuration <= uint32Max, "IdleModeDuration attribute does not fit in a uint32.")
        asserts.assert_greater_equal(idleModeDuration, 1, "IdleModeDuration attribute is smaller than minimum value (1).")
        asserts.assert_less_equal(idleModeDuration, 64800, "IdleModeDuration attribute is greater than maximum value (64800).")
        asserts.assert_greater_equal(idleModeDuration, activeModeDuration, "IdleModeDuration attribute is less than ActiveModeDuration.")

        if isCIP:
            # ClientsSupportedPerFabric attribute test
            self.print_step(5, "Read ClientsSupportedPerFabric Attribute")
            asserts.assert_true(self.check_pics("ICDM.S.A0005"), "ClientsSupportedPerFabric is a mandatory attribute if CheckInProtocolSupport feature is set and must be present in the PICS file")
            clientsSupportedPerFabric = await self.read_icdm_attribute_expect_success(endpoint=endpoint, attribute=attributes.ClientsSupportedPerFabric)
            logger.info(f"ClientsSupportedPerFabric: {clientsSupportedPerFabric}")
            asserts.assert_true(0 <= clientsSupportedPerFabric <= uint16Max, "ClientsSupportedPerFabric attribute does not fit in a uint16.")
            asserts.assert_greater_equal(clientsSupportedPerFabric, 1, "ClientsSupportedPerFabric attribute is smaller than minimum value (1).")

            # RegisteredClients attribute test
            self.print_step(6, "Read RegisteredClients Attribute")
            asserts.assert_true(self.check_pics("ICDM.S.A0003"), "RegisteredClients is a mandatory attribute if CheckInProtocolSupport feature is set and must be present in the PICS file")
            registeredClients = await self.read_icdm_attribute_expect_success(endpoint=endpoint, attribute=attributes.RegisteredClients)
            logger.info(f"RegisteredClients: {registeredClients}")
            asserts.assert_less_equal(len(registeredClients), clientsSupportedPerFabric, "Number of RegisteredClients is greater than ClientsSupportedPerFabric.")

            # ICDCounter attribute test
            self.print_step(7, "Read ICDCounter Attribute")
            asserts.assert_true(self.check_pics("ICDM.S.A0004"), "ICDCounter is a mandatory attribute if CheckInProtocolSupport feature is set and must be present in the PICS file")
            ICDCounter = await self.read_icdm_attribute_expect_success(endpoint=endpoint, attribute=attributes.ICDCounter)
            logger.info(f"ICDCounter: {ICDCounter}")
            asserts.assert_true(0 <= ICDCounter <= uint32Max, "ICDCounter attribute does not fit in a uint32.")
        #
        # if isUAT:
        #     # UserActiveModeTriggerHint attribute test
        #     self.print_step(8, "Read UserActiveModeTriggerHint Attribute")
        #     asserts.assert_true(self.check_pics("ICDM.S.A0006"), "UserActiveModeTriggerHint is a mandatory attribute if UserActiveModeTrigger feature is set and must be present in the PICS file")
        #     userActiveModeTriggerHint = await self.read_icdm_attribute_expect_success(endpoint=endpoint, attribute=attributes.UserActiveModeTriggerHint)
        #     logger.info(f"UserActiveModeTriggerHint: {userActiveModeTriggerHint}")
        #     asserts.assert_true(0 <= userActiveModeTriggerHint <= uint32Max, "UserActiveModeTriggerHint attribute does not fit in a bitmap32.")
        #
        #     userActiveModeTriggerInstructionList = [
        #         Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kCustomInstruction,
        #         Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kActuateSensorSeconds,
        #         Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kActuateSensorTimes,
        #         Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kActuateSensorLightsBlink,
        #         Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kResetButtonLightsBlink,
        #         Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kResetButtonSeconds,
        #         Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kResetButtonTimes,
        #         Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kSetupButtonSeconds,
        #         Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kSetupButtonLightsBlink,
        #         Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kSetupButtonTimes,
        #         Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kAppDefinedButton,
        #
        #     ]
        #     for bitmap in userActiveModeTriggerInstructionList:
        #         if userActiveModeTriggerHint & bitmap:
        #             asserts.assert_false(userActiveModeTriggerHintBitmapMatch, "More than one UserActiveModeTriggerHint bit depending on UserActiveModeTriggerInstruction is set: \
        #             bit 2, 5, 6, 7, 9, 10, 11, 13, 14, 15, 16.")
        #         else:
        #             userActiveModeTriggerHintBitmapMatch = True
        #
        #     # UserActiveModeTriggerInstruction attribute test
        #     self.print_step(9, "Read UserActiveModeTriggerInstruction Attribute")
        #     asserts.assert_true(self.check_pics("ICDM.S.A0009"), "UserActiveModeTriggerHint is a mandatory attribute if UserActiveModeTrigger feature is set and must be present in the PICS file")
        #     userActiveModeTriggerInstruction = await self.read_icdm_attribute_expect_success(endpoint=endpoint, attribute=attributes.UserActiveModeTriggerInstruction)
        #     logger.info(f"UserActiveModeTriggerInstruction: {userActiveModeTriggerInstruction}")
        #     asserts.assert_true(userActiveModeTriggerInstruction != NullValue, "UserActiveModeTriggerInstruction attribute is a null value.")
        #     asserts.assert_less_equal(len(str(userActiveModeTriggerInstruction)), 128, "UserActiveModeTriggerInstruction attribute is longer than 128 characters.")
        #     if userActiveModeTriggerHintBitmapMatch:
        #         userActiveModeTriggerInstructionTypes = {
        #             'digit': [
        #                 Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kActuateSensorSeconds,
        #                 Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kActuateSensorTimes,
        #                 Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kResetButtonSeconds,
        #                 Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kResetButtonTimes,
        #                 Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kSetupButtonSeconds,
        #                 Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kSetupButtonTimes,
        #                 Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kAppDefinedButton,
        #             ],
        #             'string': [
        #                 Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kCustomInstruction,
        #             ],
        #             'color': [
        #                 Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kActuateSensorLightsBlink,
        #                 Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kResetButtonLightsBlink,
        #                 Clusters.IcdManagement.Bitmaps.UserActiveModeTriggerBitmap.kSetupButtonLightsBlink,
        #             ]
        #         }
        #
        #         if userActiveModeTriggerInstruction in userActiveModeTriggerInstructionTypes['digit']:
        #             asserts.assert_true(str(userActiveModeTriggerInstruction).isdigit() and
        #                                 not len(str(userActiveModeTriggerInstruction)) - len(str(userActiveModeTriggerInstruction).lstrip('0')),
        #                                 "userActiveModeTriggerInstruction attribute is not decimal unsigned integer using the ASCII digits 0-9, and without leading zeros.")
        #         elif userActiveModeTriggerInstruction in userActiveModeTriggerInstructionTypes['string']:
        #             asserts.assert_false(str(userActiveModeTriggerInstruction).isdigit(), "userActiveModeTriggerInstruction attribute is not a text string.")
        #         elif userActiveModeTriggerInstruction in userActiveModeTriggerInstructionTypes['color']:
        #             asserts.assert_false(str(userActiveModeTriggerInstruction).isdigit(), "userActiveModeTriggerInstruction attribute is not a text string (color of LED).")
        #             # TODO: use semi-manual prompt to verify color of LEDs.
        #             # colorList = ['white', 'red', 'green', 'blue', 'orange', 'yellow', 'purple']
        #             # asserts.assert_true(userActiveModeTriggerInstruction.lower() in colorList, "userActiveModeTriggerInstruction attribute is not in the range of allowed colors")
        #
        # if isLITS:
        #     # OperatingMode attribute test
        #     self.print_step(10, "Read OperatingMode Attribute")
        #     asserts.assert_true(self.check_pics("ICDM.S.A0008"), "OperatingMode is a mandatory attribute if LongIdleTimeSupport feature is set and must be present in the PICS file")
        #     operatingMode = await self.read_icdm_attribute_expect_success(endpoint=endpoint, attribute=attributes.OperatingMode)
        #     logger.info(f"OperatingMode: {operatingMode}")
        #     asserts.assert_true(operatingMode in [0, 1], "OperatingMode attribute does not fit in an enum of 0 or 1.")


if __name__ == "__main__":
    default_matter_test_main()
