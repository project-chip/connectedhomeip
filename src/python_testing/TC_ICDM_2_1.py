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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${LIT_ICD_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import re

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)

kRootEndpointId = 0
kMaxUserActiveModeBitmap = 0x1FFFF
kMaxUserActiveModeTriggerInstructionByteLength = 128

cluster = Clusters.Objects.IcdManagement
uat = cluster.Bitmaps.UserActiveModeTriggerBitmap
modes = cluster.Enums.OperatingModeEnum
features = cluster.Bitmaps.Feature

# BitMask for all user active mode trigger hints that are depedent on the UserActiveModeTriggerInstruction
kUatInstructionDependentBitMask = uat.kCustomInstruction | uat.kActuateSensorSeconds | uat.kActuateSensorTimes | uat.kActuateSensorLightsBlink | uat.kResetButtonLightsBlink | uat.kResetButtonSeconds | uat.kResetButtonTimes | uat.kSetupButtonSeconds | uat.kSetupButtonLightsBlink | uat.kSetupButtonTimes | uat.kAppDefinedButton

# BitMask for UserActiveModeTriggerHint that REQUIRE the prescense of the UserActiveModeTriggerInstruction
kUatInstructionMandatoryBitMask = uat.kCustomInstruction | uat.kActuateSensorSeconds | uat.kActuateSensorTimes | uat.kResetButtonSeconds | uat.kResetButtonTimes | uat.kSetupButtonSeconds | uat.kSetupButtonTimes | uat.kAppDefinedButton

# BitMask for all user active mode trigger hints that have the UserActiveModeTriggerInstruction as an uint
kUatNumberInstructionBitMask = uat.kActuateSensorSeconds | uat.kActuateSensorTimes | uat.kResetButtonSeconds | uat.kResetButtonTimes | uat.kSetupButtonSeconds | uat.kSetupButtonTimes

# BitMask for all user active mode trigger hints that provide a color in the UserActiveModeTriggerInstruction
kUatColorInstructionBitMask = uat.kActuateSensorLightsBlink | uat.kResetButtonLightsBlink | uat.kSetupButtonLightsBlink


class TC_ICDM_2_1(MatterBaseTest):

    #
    # Class Helper functions
    #

    @staticmethod
    def is_valid_uint32_value(var):
        return isinstance(var, int) and 0 <= var <= 0xFFFFFFFF

    @staticmethod
    def is_valid_uint16_value(var):
        return isinstance(var, int) and 0 <= var <= 0xFFFF

    @staticmethod
    def is_valid_uint8_value(var):
        return isinstance(var, int) and 0 <= var <= 0xFF

    @staticmethod
    def set_bits_count(number):
        return bin(number).count("1")

    async def _read_icdm_attribute_expect_success(self, attribute):
        return await self.read_single_attribute_check_success(endpoint=kRootEndpointId, cluster=cluster, attribute=attribute)

    async def _wildcard_cluster_read(self):
        return await self.default_controller.ReadAttribute(self.dut_node_id, [(kRootEndpointId, cluster)])

    #
    # Test Harness Helpers
    #

    def desc_TC_ICDM_2_1(self) -> str:
        """Returns a description of this test"""
        return "[TC_ICDM_2_1]  attributes with DUT as Server"

    def steps_TC_ICDM_2_1(self) -> list[TestStep]:
        steps = [
            TestStep("1a", "Commissioning, already done", is_commissioning=True),
            TestStep("1b", "CTH reads from the DUT the FeatureMap attribute."),
            TestStep(2, "TH reads from the DUT the ActiveModeThreshold attribute."),
            TestStep(3, "TH reads from the DUT the ActiveModeDuration attribute."),
            TestStep(4, "TH reads from the DUT the IdleModeDuration attribute."),
            TestStep(
                5, "TH reads from the DUT the ClientsSupportedPerFabric attribute."),
            TestStep(6, "TH reads from the DUT the RegisteredClients attribute."),
            TestStep(7, "TH reads from the DUT the ICDCounter attribute."),
            TestStep(
                8, "TH reads from the DUT the UserActiveModeTriggerHint attribute."),
            TestStep(
                9, "TH reads from the DUT the UserActiveModeTriggerInstruction attribute"),
            TestStep(10, "TH reads from the DUT the OperatingMode attribute."),
            TestStep(11, "TH reads from the DUT the MaximumCheckInBackoff attribute."),
        ]
        return steps

    def pics_TC_ICDM_2_1(self) -> list[str]:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        pics = [
            "ICDM.S",
        ]
        return pics

    #
    # ICDM 2.1 Test Body
    #

    @async_test_body
    async def test_TC_ICDM_2_1(self):

        cluster = Clusters.Objects.IcdManagement
        attributes = cluster.Attributes

        # Commissioning
        self.step("1a")
        # Read feature map
        self.step("1b")
        featureMap = await self._read_icdm_attribute_expect_success(
            attributes.FeatureMap)

        # Validate ActiveModeThreshold
        self.step(2)
        if self.check_pics("ICDM.S.A0002"):

            activeModeThreshold = await self._read_icdm_attribute_expect_success(
                attributes.ActiveModeThreshold)
            # Verify ActiveModeThreshold is not bigger than uint16
            asserts.assert_true(self.is_valid_uint16_value(activeModeThreshold),
                                "ActiveModeThreshold attribute does not fit in a uint16.")

            if featureMap > 0 and features.kLongIdleTimeSupport in features(featureMap):
                asserts.assert_greater_equal(
                    activeModeThreshold, 5000, "Minimum ActiveModeThreshold is 5s for a LIT ICD.")

        else:
            asserts.assert_true(
                False, "ActiveModeThreshold is a mandatory attribute and must be present in the PICS file")

        # Validate ActiveModeDuration
        self.step(3)
        if self.check_pics("ICDM.S.A0001"):
            activeModeDuration = await self._read_icdm_attribute_expect_success(
                attributes.ActiveModeDuration)
            # Verify ActiveModeDuration is not bigger than uint32
            asserts.assert_true(self.is_valid_uint32_value(activeModeDuration),
                                "ActiveModeDuration attribute does not fit in a uint32")
        else:
            asserts.assert_true(
                False, "ActiveModeDuration is a mandatory attribute and must be present in the PICS file")

        # Validate IdleModeDuration
        self.step(4)
        if self.check_pics("ICDM.S.A0000"):
            idleModeDuration = await self._read_icdm_attribute_expect_success(
                attributes.IdleModeDuration)
            # Verify IdleModeDuration is not bigger than uint32
            asserts.assert_greater_equal(
                idleModeDuration, 1, "IdleModeDuration attribute is smaller than minimum value (1).")
            asserts.assert_less_equal(
                idleModeDuration, 64800, "IdleModeDuration attribute is greater than maximum value (64800).")
            asserts.assert_greater_equal(idleModeDuration * 1000, activeModeDuration,
                                         "ActiveModeDuration attribute is greater than the IdleModeDuration attrbiute.")
        else:
            asserts.assert_true(
                False, "IdleModeDuration is a mandatory attribute and must be present in the PICS file")

        # Validate ClientsSupportedPerFabric
        self.step(5)
        if self.pics_guard(self.check_pics("ICDM.S.A0005")):
            clientsSupportedPerFabric = await self._read_icdm_attribute_expect_success(
                attributes.ClientsSupportedPerFabric)

            # Verify ClientsSupportedPerFabric is not bigger than uint16
            asserts.assert_true(self.is_valid_uint16_value(clientsSupportedPerFabric),
                                "ClientsSupportedPerFabric attribute does not fit in a uint16.")

            asserts.assert_greater_equal(
                clientsSupportedPerFabric, 1, "ClientsSupportedPerFabric attribute is smaller than minimum value (1).")

        # Validate RegisteredClients
        self.step(6)
        if self.pics_guard(self.check_pics("ICDM.S.A0003")):
            registeredClients = await self._read_icdm_attribute_expect_success(
                attributes.RegisteredClients)

            asserts.assert_true(isinstance(
                registeredClients, list), "RegisteredClients is not a list.")

        # Validate ICDCounter
        self.step(7)
        if self.pics_guard(self.check_pics("ICDM.S.A0004")):

            icdCounter = await self._read_icdm_attribute_expect_success(
                attributes.ICDCounter)
            # Verify ICDCounter is not bigger than uint32
            asserts.assert_true(self.is_valid_uint32_value(icdCounter),
                                "ActiveModeDuration attribute does not fit in a uint32")

        # Validate UserActiveModeTriggerHint
        self.step(8)
        if self.pics_guard(self.check_pics("ICDM.S.A0006")):
            userActiveModeTriggerHint = await self._read_icdm_attribute_expect_success(
                attributes.UserActiveModeTriggerHint)

            # Verify that it is a bitmap32 - Only the first 16 bits are used
            asserts.assert_true(0 <= userActiveModeTriggerHint <= kMaxUserActiveModeBitmap,
                                "UserActiveModeTriggerHint attribute does not fit in a bitmap32")

            # Verify that only a single UserActiveModeTriggerInstruction dependent bit is set
            uatHintInstructionDepedentBitmap = uat(
                userActiveModeTriggerHint) & kUatInstructionDependentBitMask

            asserts.assert_less_equal(
                self.set_bits_count(uatHintInstructionDepedentBitmap), 1, "UserActiveModeTriggerHint has more than 1 bit that is dependent on the UserActiveModeTriggerInstruction")

        # Valdate UserActiveModeTriggerInstruction
        self.step(9)
        if self.check_pics("ICDM.S.A0007"):
            userActiveModeTriggerInstruction = await self._read_icdm_attribute_expect_success(
                attributes.UserActiveModeTriggerInstruction)

            # Verify that the UserActiveModeTriggerInstruction has the correct encoding
            try:
                encodedUATInstruction = userActiveModeTriggerInstruction.encode(
                    'utf-8')
            except Exception:
                asserts.assert_true(
                    False, "UserActiveModeTriggerInstruction is not encoded in the correct format (utf-8).")

            # Verify byte length of the UserActiveModeTirggerInstruction
            asserts.assert_less_equal(
                len(encodedUATInstruction), kMaxUserActiveModeTriggerInstructionByteLength, "UserActiveModeTriggerInstruction is longuer than the maximum allowed length (128).")

            if uatHintInstructionDepedentBitmap > 0 and uatHintInstructionDepedentBitmap in kUatNumberInstructionBitMask:
                # Validate Instruction is a decimal unsigned integer using the ASCII digits 0-9, and without leading zeros.
                asserts.assert_true((re.search(r'^(?!0)[0-9]*$', userActiveModeTriggerInstruction) is not None),
                                    "UserActiveModeTriggerInstruction is not in the correct format for the associated UserActiveModeTriggerHint")

            if uatHintInstructionDepedentBitmap > 0 and uatHintInstructionDepedentBitmap in kUatColorInstructionBitMask:
                pattern = re.compile(r'^[0-9A-F]{6}$')
                asserts.assert_true(pattern.match(userActiveModeTriggerInstruction),
                                    "UserActiveModeTriggerInstruction is not in the correct format for the associated UserActiveModeTriggerHint")
        elif self.check_pics("ICDM.S.A0006"):
            # Check if the UserActiveModeTriggerInstruction was required
            asserts.assert_false(uatHintInstructionDepedentBitmap in kUatInstructionMandatoryBitMask,
                                 "UserActiveModeTriggerHint requires the UserActiveModeTriggerInstruction")

        # Verify OperatingMode
        self.step(10)
        if self.pics_guard(self.check_pics("ICDM.S.A0008")):
            operatingMode = await self._read_icdm_attribute_expect_success(
                attributes.OperatingMode)

            asserts.assert_true(self.is_valid_uint8_value(operatingMode),
                                "OperatingMode does not fit in an enum8")

            asserts.assert_less(
                operatingMode, modes.kUnknownEnumValue, "OperatingMode can only have 0 and 1 as valid values")
        self.step(11)
        if self.pics_guard(self.check_pics("ICDM.S.A0009")):
            maximumCheckInBackOff = await self._read_icdm_attribute_expect_success(attributes.MaximumCheckInBackOff)

            asserts.assert_true(self.is_valid_uint32_value(maximumCheckInBackOff),
                                "MaximumCheckInBackOff attribute is not a valid uint32.")
            asserts.assert_greater_equal(maximumCheckInBackOff, idleModeDuration,
                                         "MaximumCheckInBack attribute is not greater or euqal to the IdleModeDuration")
            asserts.assert_less_equal(maximumCheckInBackOff, 64800,
                                      "MaximumCheckInBackOff attribute is greater than maximum value (64800).")


if __name__ == "__main__":
    default_matter_test_main()
