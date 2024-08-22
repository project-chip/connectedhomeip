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
# pylint: disable=invalid-name

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ENERGY_MANAGEMENT_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --enable-key 000102030405060708090a0b0c0d0e0f --featureSet 0x7f
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --hex-arg enableKey:000102030405060708090a0b0c0d0e0f --endpoint 1 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

"""Define Matter test case TC_DEM_1_1."""


import logging

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_DEMTestBase import DEMTestBase

logger = logging.getLogger(__name__)


class TC_DEM_1_1(MatterBaseTest, DEMTestBase):
    """Implementation of test case TC_DEM_1_1."""

    def desc_TC_DEM_1_1(self) -> str:
        """Return a description of this test."""
        return "4.1.3. [TC-DEM-2.5] Forecast Adjustment with Power Forecast Reporting feature functionality with DUT as Server"

    def pics_TC_DEM_1_1(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            # Depends on Feature 05 (ForecastAdjustment) & Feature 01 (PowerForecastReporting)
            "DEM.S.F05", "DEM.S.F01"
        ]
        return pics

    def steps_TC_DEM_1_1(self) -> list[TestStep]:
        """Execute the test steps."""
        steps = [
            TestStep("1", "Commission DUT to TH (can be skipped if done in a preceding test)."),
            TestStep("2", "TH reads from the DUT the ClusterRevision attribute.",
                     "Verify that the DUT response contains the ClusterRevision attribute and has the value 3."),
            TestStep("3", "TH reads from the DUT the FeatureMap attribute.",
                     "Verify that the DUT response contains the FeatureMap attribute and have the following bit set:\n- bit 0: SHALL be 1 if and only if DEM.S.F00(PowerAdjustment).\n- bit 1: SHALL be 1 if and only if DEM.S.F01(PowerForecastReporting).\n- bit 2: SHALL be 1 if and only if DEM.S.F02(StateForecastReporting).\n- bit 3: SHALL be 1 if and only if DEM.S.F03(StartTimeAdjustment).\n- bit 4: SHALL be 1 if and only if DEM.S.F04(Pausable).\nAll remaining bits SHALL be 0. Provisional features DEM.S.F05(ForecastAdjustment) and DEM.S.F06(ConstraintBasedAdjustment) must not be supported."),
            TestStep("4", "TH reads from the DUT the AttributeList attribute.",
                     "Verify that the DUT response contains the AttributeList attribute and have the list of supported attributes:\nThe list SHALL include all the mandatory entries: +\n0x0000, 0x0001, 0x0002, 0x0003, 0x0004\n0xfff8, 0xfff9, 0xfffb, 0xfffc and 0xfffd.\nThe list includes entries based on feature support: +\n- 0x0005: SHALL be included if and only if DEM.S.F00(PowerAdjustment).\n- 0x0006: SHALL be included if and only if DEM.S.F01(PowerForecastReporting) | DEM.S.F02(StateForecastReporting).\n- 0x0007: SHALL be included if and only if DEM.S.F00(PowerAdjustment) | DEM.S.F03(StartTimeAdjustment) | DEM.S.F04(Pausable) | DEM.S.F05(ForecastAdjustment) | DEM.S.F06(ConstraintBasedAdjustment)."),
            TestStep("5", "TH reads from the DUT the EventList attribute.",
                     "Verify that the DUT response contains the EventList attribute and have the list of supported events:\nThe list includes entries based on feature support: +\n- 0x00, 0x01: SHALL be included if and only if DEM.S.F00(PowerAdjustment).\n- 0x02, 0x03: SHALL be included if and only if DEM.S.F04(Pausable)."),
            TestStep("6", "TH reads from the DUT the AcceptedCommandList attribute.",
                     "Verify that the DUT response contains the AcceptedCommandList attribute and have the list of Accepted Command:\nThe list includes entries based on feature support: +\n- 0x00, 0x01: SHALL be included if and only if DEM.S.F00(PowerAdjustment).\n- 0x02: SHALL be included if and only if DEM.S.F03(StartTimeAdjustment).\n- 0x03, 0x04: SHALL be included if and only if DEM.S.F04(Pausable).\n- 0x05: SHALL be included if and only if DEM.S.F05(ForecastAdjustment).\n- 0x06: SHALL be included if and only if DEM.S.F06(ConstraintBasedAdjustment).\n- 0x07: SHALL be included if and only if DEM.S.F03(StartTimeAdjustment)|DEM.S.F05(ForecastAdjustment)|DEM.S.F06(ConstraintBasedAdjustment)"),
            TestStep("7", "TH reads from the DUT the GeneratedCommandList attribute.",
                     "Verify that the DUT response contains the GeneratedCommandList attribute and have the list of Generated Command:\nThis cluster has no entries in the standard or scoped range. +"),
        ]

        return steps

    @async_test_body
    async def test_TC_DEM_1_1(self):
        # pylint: disable=too-many-locals, too-many-statements
        """Run the test steps."""
        self.step("1")
        # Commission DUT - already done

        self.step("2")
        await self.check_dem_attribute("ClusterRevision", 4)

        self.step("3")
        await self.validate_feature_map([Clusters.DeviceEnergyManagement.Bitmaps.Feature.kForecastAdjustment,
                                         Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerForecastReporting,
                                         Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerAdjustment,
                                         Clusters.DeviceEnergyManagement.Bitmaps.Feature.kStateForecastReporting,
                                         Clusters.DeviceEnergyManagement.Bitmaps.Feature.kStartTimeAdjustment,
                                         Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPausable,
                                         Clusters.DeviceEnergyManagement.Bitmaps.Feature.kConstraintBasedAdjustment],
                                        [])

        self.step("4")
        attribute_list = await self.read_dem_attribute_expect_success(attribute="AttributeList")
        logging.info(attribute_list)

        mandatory_attributes = [Clusters.DeviceEnergyManagement.Attributes.ESAType.attribute_id,
                                Clusters.DeviceEnergyManagement.Attributes.ESACanGenerate.attribute_id,
                                Clusters.DeviceEnergyManagement.Attributes.ESAState.attribute_id,
                                Clusters.DeviceEnergyManagement.Attributes.AbsMinPower.attribute_id,
                                Clusters.DeviceEnergyManagement.Attributes.AbsMaxPower.attribute_id]

        for mandatory_attribute in mandatory_attributes:
            asserts.assert_true(mandatory_attribute in attribute_list,
                                f"Expected to find mandatory attribute {mandatory_attribute} in attribute_list {attribute_list}")

            # Remove the enrty from the list
            attribute_list.remove(mandatory_attribute)

        # Now check feature dependant attributes
        feature_map = await self.read_dem_attribute_expect_success(attribute="FeatureMap")

        attribute_id = Clusters.DeviceEnergyManagement.Attributes.PowerAdjustmentCapability.attribute_id
        required_features = Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerAdjustment
        self.check_attribute_valid_and_remove_from_list(attribute_id, feature_map, attribute_list, required_features)

        attribute_id = Clusters.DeviceEnergyManagement.Attributes.Forecast.attribute_id
        required_features = (Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerForecastReporting |
                             Clusters.DeviceEnergyManagement.Bitmaps.Feature.kStateForecastReporting)
        self.check_attribute_valid_and_remove_from_list(attribute_id, feature_map, attribute_list, required_features)

        attribute_id = Clusters.DeviceEnergyManagement.Attributes.OptOutState.attribute_id
        required_features = (Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerAdjustment |
                             Clusters.DeviceEnergyManagement.Bitmaps.Feature.kStartTimeAdjustment |
                             Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPausable |
                             Clusters.DeviceEnergyManagement.Bitmaps.Feature.kForecastAdjustment |
                             Clusters.DeviceEnergyManagement.Bitmaps.Feature.kConstraintBasedAdjustment)

        self.check_attribute_valid_and_remove_from_list(attribute_id, feature_map, attribute_list, required_features)

        global_attributes = [Clusters.DeviceEnergyManagement.Attributes.GeneratedCommandList.attribute_id,
                             Clusters.DeviceEnergyManagement.Attributes.AcceptedCommandList.attribute_id,
                             Clusters.DeviceEnergyManagement.Attributes.AttributeList.attribute_id,
                             Clusters.DeviceEnergyManagement.Attributes.FeatureMap.attribute_id,
                             Clusters.DeviceEnergyManagement.Attributes.ClusterRevision.attribute_id]

        for global_attribute in global_attributes:
            logging.info(f"global_attribute {global_attribute} global_attributes {global_attributes}")
            asserts.assert_true(global_attribute in attribute_list,
                                f"Expected to find global attribute {global_attribute} in attribute_list {attribute_list}")

            # Remove the enrty from the list
            attribute_list.remove(global_attribute)

        # Check there are no unknown attributes left in the list
        asserts.assert_equal(len(attribute_list), 0)

        self.step("5")
        logging.info(f"Not supported/skipped")

        self.step("6")
        accepted_command_list = await self.read_dem_attribute_expect_success(attribute="AcceptedCommandList")
        logging.info(f"accepted_command_list {accepted_command_list}")

        required_features = Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPowerAdjustment
        expected_commands = [Clusters.DeviceEnergyManagement.Commands.PowerAdjustRequest.command_id,
                             Clusters.DeviceEnergyManagement.Commands.CancelPowerAdjustRequest.command_id]
        self.check_expected_commands_in_list(expected_commands, feature_map, accepted_command_list, required_features)

        required_features = Clusters.DeviceEnergyManagement.Bitmaps.Feature.kStartTimeAdjustment
        expected_commands = [Clusters.DeviceEnergyManagement.Commands.StartTimeAdjustRequest.command_id]
        self.check_expected_commands_in_list(expected_commands, feature_map, accepted_command_list, required_features)

        required_features = Clusters.DeviceEnergyManagement.Bitmaps.Feature.kPausable
        expected_commands = [Clusters.DeviceEnergyManagement.Commands.PauseRequest.command_id,
                             Clusters.DeviceEnergyManagement.Commands.ResumeRequest.command_id]
        self.check_expected_commands_in_list(expected_commands, feature_map, accepted_command_list, required_features)

        required_features = Clusters.DeviceEnergyManagement.Bitmaps.Feature.kForecastAdjustment
        expected_commands = [Clusters.DeviceEnergyManagement.Commands.ModifyForecastRequest.command_id]
        self.check_expected_commands_in_list(expected_commands, feature_map, accepted_command_list, required_features)

        required_features = Clusters.DeviceEnergyManagement.Bitmaps.Feature.kConstraintBasedAdjustment
        expected_commands = [Clusters.DeviceEnergyManagement.Commands.RequestConstraintBasedForecast.command_id]
        self.check_expected_commands_in_list(expected_commands, feature_map, accepted_command_list, required_features)

        required_features = (Clusters.DeviceEnergyManagement.Bitmaps.Feature.kStartTimeAdjustment |
                             Clusters.DeviceEnergyManagement.Bitmaps.Feature.kForecastAdjustment |
                             Clusters.DeviceEnergyManagement.Bitmaps.Feature.kConstraintBasedAdjustment)
        expected_commands = [Clusters.DeviceEnergyManagement.Commands.CancelRequest.command_id]
        self.check_expected_commands_in_list(expected_commands, feature_map, accepted_command_list, required_features)

        self.step("7")
        generated_command_list = await self.read_dem_attribute_expect_success(attribute="GeneratedCommandList")
        asserts.assert_equal(len(generated_command_list), 0)

    def check_attribute_valid_and_remove_from_list(self, attribute_id, feature_map, attribute_list, required_features):
        if feature_map & required_features:
            asserts.assert_true(attribute_id in attribute_list,
                                 f"Expected to find attribute {attribute_id} in attribute_list {attribute_list} given feature_map 0x{feature_map:x}")
            attribute_list.remove(attribute_id)

        else:
            asserts.assert_false(attribute_id in attribute_list,
                                 f"Did not expect to find attribute {attribute_id} in attribute_list {attribute_list} given feature_map 0x{feature_map:x}")

    def check_expected_commands_in_list(self, expected_commands, feature_map, accepted_command_list, required_features):
        if (feature_map & required_features) != 0:
            for command in expected_commands:
                asserts.assert_true(command in accepted_command_list,
                                    f"Expected command {command} to be in accepted_command_list {accepted_command_list} given required_features {required_features} in feature_map 0x{feature_map:x}")

        else:
            for command in expected_commands:
                asserts.assert_false(command in accepted_command_list,
                                    f"Did not expect command {command} to be in accepted_command_list {accepted_command_list} given required_features {required_features} in feature_map 0x{feature_map:x}")
if __name__ == "__main__":
    default_matter_test_main()
