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
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import time
from typing import Any

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
# from chip.testing.matter_testing import AttributeValue, ClusterAttributeChangeAccumulator, MatterBaseTest, async_test_body, default_matter_test_main
# from 

from mobly import asserts

from matter_testing_infrastructure.chip.testing.matter_testing import AttributeValueExpectation, ComparisonEnum, ClusterAttributeChangeAccumulator, MatterBaseTest, async_test_body, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_FAN_3_5(MatterBaseTest):
    
    async def read_setting(self, attribute: Any) -> Any:
        """
        Asynchronously reads a specified attribute from the FanControl cluster at a given endpoint.

        Args:
            attribute (Any): The attribute to be read.

        Returns:
            Any: The value of the specified attribute if the read operation is successful.

        Raises:
            AssertionError: If the read operation fails.
        """
        cluster = Clusters.Objects.FanControl
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)    

    async def write_setting(self, attribute, value) -> None:
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(self.endpoint, attribute(value))])
        asserts.assert_equal(result[0].Status, Status.Success, f"[FC] {attribute.__name__} attribute write faield.")

    async def send_step_command(self, direction: Clusters.Objects.FanControl.Enums.StepDirectionEnum,
                                wrap: bool = False, lowestOff: bool = False) -> None:
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.FanControl.Commands.Step(direction=direction, wrap=wrap, lowestOff=lowestOff), endpoint=self.endpoint)
            logger.info("[FC] send_single_cmd")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, f"Unexpected error returned ({e})")
            pass

    def verify_expected_value(self, attribute, value_expected, timeout_sec):
        value_current = self.attribute_subscription.get_last_attribute_report_value(self.endpoint, attribute, timeout_sec)
        asserts.assert_equal(value_current, value_expected, f"Current {attribute.__name__} attribute value ({value_current}) is not equal to the expected value ({value_expected})")

    def pics_TC_FAN_3_5(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_5(self):
        # Setup
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attributes = cluster.Attributes
        sd_enum = cluster.Enums.StepDirectionEnum
        fm_enum = cluster.Enums.FanModeEnum
        value_off = 0
        percent_setting_max = 100
        percent_setting = value_off
        percent_current = value_off
        fan_mode = fm_enum(value_off)
        timeout_sec = 1

        # *** STEP 1 ***
        # Commissioning already done
        # self.step(1)
        print(f"[FC] # self.step(1)")

        # *** STEP 2 ***
        # TH checks the DUT for support of the `Step` feature
        # self.step(2)
        print(f"[FC] # self.step(2)")
        feature_map = await self.read_setting(attributes.FeatureMap)
        self.supports_step = bool(feature_map & cluster.Bitmaps.Feature.kStep)
        if not self.supports_step:
            logger.info("Test skipped because the Step feature is not supported")
            return

        # *** STEP 3 ***
        # TH subscribes to the DUT's FanControl cluster
        # self.step(3)
        print(f"[FC] # self.step(3)")
        self.attribute_subscription = ClusterAttributeChangeAccumulator(cluster)
        await self.attribute_subscription.start(self.default_controller, self.dut_node_id, self.endpoint)

        # *** STEP 4 ***
        # TH reads from the DUT the FanMode attribute
        #   - If the FanMode attribute is different than 0 (Off), the
        #     TH writes to the DUT the FanMode attribute with 0 (Off),
        #     and verify that the PercentStting, PercentCurrent, and
        #     FanMode attributes are all 0, else, skip to the next step
        # self.step(4)
        print(f"[FC] # self.step(4)")
        fan_mode = await self.read_setting(attributes.FanMode)
        if fan_mode != fm_enum.kOff:
            await self.write_setting(attributes.FanMode, fan_mode)
            value_expectations = [
                AttributeValueExpectation(self.endpoint, attributes.PercentSetting, ComparisonEnum.Equal, percent_setting),
                AttributeValueExpectation(self.endpoint, attributes.PercentCurrent, ComparisonEnum.Equal, percent_current),
                AttributeValueExpectation(self.endpoint, attributes.FanMode, ComparisonEnum.Equal, fan_mode),
            ]
            percent_setting, percent_current, fan_mode = self.attribute_subscription.await_all_final_values_reported_threshold(
                value_expectations=value_expectations, timeout_sec=timeout_sec)
            self.attribute_subscription.reset()

        # *** STEP 5 ***
        # TH sends Step command to DUT with Direction set to Increase
        #   - Verify that the PercentStting, PercentCurrent, and
        #     FanMode attributes are greater than 0
        # self.step(5)
        print(f"[FC] # self.step(5)")
        await self.send_step_command(direction=sd_enum.kIncrease)
        value_expectations = [
            AttributeValueExpectation(self.endpoint, attributes.PercentSetting, ComparisonEnum.GreaterThan, percent_setting),
            AttributeValueExpectation(self.endpoint, attributes.PercentCurrent, ComparisonEnum.GreaterThan, percent_current),
            AttributeValueExpectation(self.endpoint, attributes.FanMode, ComparisonEnum.GreaterThan, fan_mode),
        ]
        self.attribute_subscription.await_all_final_values_reported_threshold(
            value_expectations=value_expectations, timeout_sec=timeout_sec)
        self.attribute_subscription.reset()

        # *** STEP 6 ***
        # TH writes to the DUT the FanMode attribute with 3 (High)
        #   - Verify that the PercentStting and PercentCurrent
        #     attributes are both less than 100
        #   - Verify that the FanMode attribute is set to High
        # self.step(6)
        print(f"[FC] # self.step(6)")
        await self.write_setting(attributes.FanMode, fm_enum.kHigh)
        value_expectations = [
            AttributeValueExpectation(self.endpoint, attributes.PercentSetting, ComparisonEnum.LessThan, percent_setting_max),
            AttributeValueExpectation(self.endpoint, attributes.PercentCurrent, ComparisonEnum.LessThan, percent_setting_max),
            AttributeValueExpectation(self.endpoint, attributes.FanMode, ComparisonEnum.Equal, fm_enum.kHigh),
        ]
        percent_setting, percent_current, fan_mode = self.attribute_subscription.await_all_final_values_reported_threshold(
            value_expectations=value_expectations, timeout_sec=timeout_sec)
        self.attribute_subscription.reset()

        # *** STEP 7 ***
        # TH sends Step command to DUT with Direction set to Decrease
        #   - Verify that the PercentStting and PercentCurrent attributes are less than 100
        #   - Verify that the FanMode attribute is set to a value lower than High
        # self.step(7)
        print(f"[FC] # self.step(7)")
        await self.send_step_command(direction=sd_enum.kDecrease)
        value_expectations = [
            AttributeValueExpectation(self.endpoint, attributes.PercentSetting, ComparisonEnum.LessThan, percent_setting),
            AttributeValueExpectation(self.endpoint, attributes.PercentCurrent, ComparisonEnum.LessThan, percent_current),
            AttributeValueExpectation(self.endpoint, attributes.FanMode, ComparisonEnum.LessThan, fan_mode),
        ]
        self.attribute_subscription.await_all_final_values_reported_threshold(
            value_expectations=value_expectations, timeout_sec=timeout_sec)
        self.attribute_subscription.reset()

        # *** STEP 8 ***
        # TH writes to the DUT the PercentSetting attribute with 100
        #   - Verify that the PercentStting and PercentCurrent
        #     attributes are both 100
        #   - Verify that the FanMode attribute is set to High
        # self.step(8)
        print(f"[FC] # self.step(8)")
        await self.write_setting(attributes.PercentSetting, percent_setting_max)
        value_expectations = [
            AttributeValueExpectation(self.endpoint, attributes.PercentSetting, ComparisonEnum.Equal, percent_setting_max),
            AttributeValueExpectation(self.endpoint, attributes.PercentCurrent, ComparisonEnum.Equal, percent_setting_max),
            AttributeValueExpectation(self.endpoint, attributes.FanMode, ComparisonEnum.Equal, fm_enum.kHigh),
        ]
        percent_setting, percent_current, fan_mode = self.attribute_subscription.await_all_final_values_reported_threshold(
            value_expectations=value_expectations, timeout_sec=timeout_sec)

        # *** STEP 9 ***
        # TH sends Step command to DUT with Direction set to Increase,
        # and Wrap set to False
        #   - Verify that the PercentStting and PercentCurrent
        #     attributes are both 100
        #   - Verify that the FanMode attribute is set to High
        # self.step(9)
        print(f"[FC] # self.step(9)")
        await self.send_step_command(direction=sd_enum.kIncrease, wrap=False)
        self.verify_expected_value(attributes.PercentSetting, percent_setting_max, timeout_sec)
        self.verify_expected_value(attributes.PercentCurrent, percent_setting_max, timeout_sec)
        self.verify_expected_value(attributes.FanMode, fm_enum.kHigh, timeout_sec)
        self.attribute_subscription.reset()

        # *** STEP 10 ***
        # TH sends Step command to DUT with Direction set to Increase,
        # Wrap set to True, and LowestOff set to False
        #   - Verify that the PercentStting, PercentCurrent, and FanMode
        #     attribute values are greater than 0
        # self.step(10)
        print(f"[FC] # self.step(10)")
        await self.send_step_command(direction=sd_enum.kIncrease, wrap=True, lowestOff=False)
        value_expectations = [
            AttributeValueExpectation(self.endpoint, attributes.PercentSetting, ComparisonEnum.GreaterThan, 0),
            AttributeValueExpectation(self.endpoint, attributes.PercentCurrent, ComparisonEnum.GreaterThan, 0),
            AttributeValueExpectation(self.endpoint, attributes.FanMode, ComparisonEnum.GreaterThan, fm_enum.kOff),
        ]
        percent_setting, percent_current, fan_mode = self.attribute_subscription.await_all_final_values_reported_threshold(
            value_expectations=value_expectations, timeout_sec=timeout_sec)
        self.attribute_subscription.reset()

        # *** STEP 11 ***
        # TH writes to the DUT the PercentSetting attribute with 100
        #   - Verify that the PercentStting and PercentCurrent
        #     attributes are both 100
        #   - Verify that the FanMode attribute is set to High
        # self.step(11)
        print(f"[FC] # self.step(11)")
        await self.write_setting(attributes.PercentSetting, percent_setting_max)
        value_expectations = [
            AttributeValueExpectation(self.endpoint, attributes.PercentSetting, ComparisonEnum.Equal, percent_setting_max),
            AttributeValueExpectation(self.endpoint, attributes.PercentCurrent, ComparisonEnum.Equal, percent_setting_max),
            AttributeValueExpectation(self.endpoint, attributes.FanMode, ComparisonEnum.Equal, fm_enum.kHigh),
        ]
        percent_setting, percent_current, fan_mode = self.attribute_subscription.await_all_final_values_reported_threshold(
            value_expectations=value_expectations, timeout_sec=timeout_sec)
        self.attribute_subscription.reset()

        # *** STEP 12 ***
        # TH sends Step command to DUT with Direction set to Increase,
        # Wrap set to True, and LowestOff set to True
        #   - Verify that the PercentStting, PercentCurrent, and FanMode
        #     attribute values are 0
        # self.step(12)
        print(f"[FC] # self.step(12)")
        await self.send_step_command(direction=sd_enum.kIncrease, wrap=True, lowestOff=True)
        value_expectations = [
            AttributeValueExpectation(self.endpoint, attributes.PercentSetting, ComparisonEnum.Equal, 0),
            AttributeValueExpectation(self.endpoint, attributes.PercentCurrent, ComparisonEnum.Equal, 0),
            AttributeValueExpectation(self.endpoint, attributes.FanMode, ComparisonEnum.Equal, fm_enum.kOff),
        ]
        percent_setting, percent_current, fan_mode = self.attribute_subscription.await_all_final_values_reported_threshold(
            value_expectations=value_expectations, timeout_sec=timeout_sec)
        self.attribute_subscription.reset()

        # *** STEP 13 ***
        # TH writes to the DUT the PercentSetting attribute with 1
        #   - Verify that the PercentStting and PercentCurrent
        #     attributes are both 1
        #   - Verify that the FanMode attribute is set to Low
        # self.step(13)
        print(f"[FC] # self.step(13)")
        await self.write_setting(attributes.PercentSetting, 1)
        value_expectations = [
            AttributeValueExpectation(self.endpoint, attributes.PercentSetting, ComparisonEnum.Equal, 1),
            AttributeValueExpectation(self.endpoint, attributes.PercentCurrent, ComparisonEnum.Equal, 1),
            AttributeValueExpectation(self.endpoint, attributes.FanMode, ComparisonEnum.Equal, fm_enum.kLow),
        ]
        percent_setting, percent_current, fan_mode = self.attribute_subscription.await_all_final_values_reported_threshold(
            value_expectations=value_expectations, timeout_sec=timeout_sec)

        # *** STEP 14 ***
        # TH sends Step command to DUT with Direction set to Decrease,
        # Wrap set to False, and LowestOff set to False
        #   - Verify that the PercentStting, PercentCurrent, and FanMode
        #     attribute values are 1
        # self.step(14)
        print(f"[FC] # self.step(14)")
        await self.send_step_command(direction=sd_enum.kDecrease, wrap=False, lowestOff=False)
        self.verify_expected_value(attributes.PercentSetting, percent_setting, timeout_sec)
        self.verify_expected_value(attributes.PercentCurrent, percent_current, timeout_sec)
        self.verify_expected_value(attributes.FanMode, fan_mode, timeout_sec)
        self.attribute_subscription.reset()

        # *** STEP 15 ***
        # TH sends Step command to DUT with Direction set to Decrease,
        # Wrap set to True, and LowestOff set to False
        #   - Verify that the PercentStting and PercentCurrent
        #     attributes are both 100
        #   - Verify that the FanMode attribute is set to High
        # self.step(15)
        print(f"[FC] # self.step(15)")
        await self.send_step_command(direction=sd_enum.kDecrease, wrap=True, lowestOff=False)
        value_expectations = [
            AttributeValueExpectation(self.endpoint, attributes.PercentSetting, ComparisonEnum.Equal, percent_setting_max),
            AttributeValueExpectation(self.endpoint, attributes.PercentCurrent, ComparisonEnum.Equal, percent_setting_max),
            AttributeValueExpectation(self.endpoint, attributes.FanMode, ComparisonEnum.Equal, fm_enum.kHigh),
        ]
        percent_setting, percent_current, fan_mode = self.attribute_subscription.await_all_final_values_reported_threshold(
            value_expectations=value_expectations, timeout_sec=timeout_sec)
        self.attribute_subscription.reset()
        
        # *** STEP 16 ***
        # TH writes to the DUT the PercentSetting attribute with 0
        #   - Verify that the PercentStting and PercentCurrent
        #     attributes are both 0
        #   - Verify that the FanMode attribute is set to Off
        # self.step(16)
        print(f"[FC] # self.step(16)")
        await self.write_setting(attributes.PercentSetting, 0)
        value_expectations = [
            AttributeValueExpectation(self.endpoint, attributes.PercentSetting, ComparisonEnum.Equal, 0),
            AttributeValueExpectation(self.endpoint, attributes.PercentCurrent, ComparisonEnum.Equal, 0),
            AttributeValueExpectation(self.endpoint, attributes.FanMode, ComparisonEnum.Equal, fm_enum.kOff),
        ]
        percent_setting, percent_current, fan_mode = self.attribute_subscription.await_all_final_values_reported_threshold(
            value_expectations=value_expectations, timeout_sec=timeout_sec)
        self.attribute_subscription.reset()
        
        # *** STEP 17 ***
        # TH sends Step command to DUT with Direction set to Decrease,
        # Wrap set to True, and LowestOff set to True
        #   - Verify that the PercentStting and PercentCurrent
        #     attributes are both 100
        #   - Verify that the FanMode attribute is set to High
        # self.step(17)
        print(f"[FC] # self.step(17)")
        await self.send_step_command(direction=sd_enum.kDecrease, wrap=True, lowestOff=True)
        value_expectations = [
            AttributeValueExpectation(self.endpoint, attributes.PercentSetting, ComparisonEnum.Equal, percent_setting_max),
            AttributeValueExpectation(self.endpoint, attributes.PercentCurrent, ComparisonEnum.Equal, percent_setting_max),
            AttributeValueExpectation(self.endpoint, attributes.FanMode, ComparisonEnum.Equal, fm_enum.kHigh),
        ]
        percent_setting, percent_current, fan_mode = self.attribute_subscription.await_all_final_values_reported_threshold(
            value_expectations=value_expectations, timeout_sec=timeout_sec)
        self.attribute_subscription.reset()        



if __name__ == "__main__":
    default_matter_test_main()
