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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${AIR_PURIFIER_APP}
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
import math
from typing import Any

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.interaction_model import Status
from chip.testing.matter_asserts import is_valid_uint_value
from chip.testing.matter_testing import (AttributeValue, ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep,
                                         async_test_body, default_matter_test_main)
from mobly import asserts

logger = logging.getLogger(__name__)


class Uint8Type(int):
    pass


class TC_FAN_3_2(MatterBaseTest):
    def desc_TC_FAN_3_2(self) -> str:
        return "[TC-FAN-3.2] Optional speed functionality with DUT as Server"

    def steps_TC_FAN_3_2(self):
        return [TestStep(1, "[FC] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[FC] TH checks the DUT for support of the MultiSpeed feature.",
                         "If the MultiSpeed feature is unsupported, fail the test."),
                TestStep(3, "[FC] TH reads from the DUT the initial SpeedMax, SpeedSetting, SpeedCurrent, PercentSetting, PercentCurrent, and FanMode attributes and stores.",
                         "Verify that the SpeedMax DUT response contains a uint8 with value between 1 and 100 inclusive. Verify that the SpeedSetting DUT response contains a uint8 with value between 0 and SpeedMax inclusive. Verify that the SpeedCurrent DUT response contains a uint8 with value between 0 and SpeedMax inclusive. Verify that the PercentSeting DUT response contains a uint8 with value between 1 and 100 inclusive. Verify that the PercentCurrent DUT response contains a uint8 with value between 1 and 100 inclusive. Verify that the FanMode DUT response contains a FanModeEnum with value between 1 and 5 inclusive, excluding 4."),
                TestStep(4, "[FC] TH subscribes to the DUT's FanControl cluster.", "Enables the TH to receive attribute updates."),
                TestStep(5, "[FC] TH writes to the DUT a SpeedSetting value less than or equal to the SpeedMax value.",
                         "Device shall return either SUCCESS or CONSTRAINT_ERROR. If the write operation returned SUCCESS, verify that the SpeedSetting, SpeedCurrent, PercentSetting, PercentCurrent, and FanMode attributes are updated to their expected values. If the write operation returned CONSTRAINT_ERROR, verify that the SpeedSetting, SpeedCurrent, PercentSetting, PercentCurrent, and FanMode attributes are the same as their initial values."),
                ]

    async def read_setting(self, attribute: Any) -> Any:
        """
        Asynchronously reads a specified attribute from the FanControl cluster at a given endpoint.

        Args:
            endpoint (int): The endpoint identifier for the fan device.
            attribute (Any): The attribute to be read.

        Returns:
            Any: The value of the specified attribute if the read operation is successful.

        Raises:
            AssertionError: If the read operation fails.
        """
        cluster = Clusters.Objects.FanControl
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    async def write_setting(self, attribute, value) -> Status:
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(self.endpoint, attribute(value))])
        write_status = result[0].Status
        write_status_success = (write_status == Status.Success) or (write_status == Status.ConstraintError)
        asserts.assert_true(write_status_success,
                            f"[FC] {attribute.__name__} write did not return a result of either SUCCESS or CONSTRAINT_ERROR ({write_status.name})")
        return write_status

    async def read_verify_setting(self, attribute, type, range):
        # Read attribute value
        value = await self.read_setting(attribute)

        # Verify response is of expected type
        if type is Uint8Type:
            asserts.assert_true(is_valid_uint_value(value),
                                f"[FC] {attribute.__name__} result ({value}) isn't of type {type.__name__}")
        else:
            asserts.assert_is_instance(value, type,
                                       f"[FC] {attribute.__name__} result ({value}) isn't of type {type.__name__}")

        # Verify response is valid (value is within expected range)
        asserts.assert_in(value, range, f"[FC] {attribute.__name__} result ({value}) is invalid")

        return value

    async def verify_attribute_constraint_error(self, attribute, init_value) -> None:
        value_current = await self.read_setting(attribute)
        asserts.assert_equal(value_current, init_value,
                             f"[FC] Current {attribute.__name__} value ({value_current}) must be equal to its initial value ({init_value}).")

    def pics_TC_FAN_3_2(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_2(self):
        # Setup
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attributes = cluster.Attributes
        fm_enum = cluster.Enums.FanModeEnum

        # *** STEP 1 ***
        # Commissioning already done
        self.step(1)

        # *** STEP 2 ***
        # TH checks the DUT for support of the MultiSpeed feature
        # If the MultiSpeed feature is unsupported, fail the test
        self.step(2)
        feature_map = await self.read_setting(attributes.FeatureMap)
        self.supports_multispeed = bool(feature_map & cluster.Bitmaps.Feature.kMultiSpeed)
        if not self.supports_multispeed:
            asserts.fail("[FC] This test case is only valid if the MultiSpeed feature is supported by the DUT.")

        # *** STEP 3 ***
        # TH reads from the DUT the initial SpeedMax, SpeedSetting, SpeedCurrent, PercentSetting,
        # PercentCurrent, and FanMode attributes and stores
        #   - Verify that the SpeedMax DUT response contains a uint8 with value between 1 and 100 inclusive
        #   - Verify that the SpeedSetting DUT response contains a uint8 with value between 0 and SpeedMax inclusive
        #   - Verify that the SpeedCurrent DUT response contains a uint8 with value between 0 and SpeedMax inclusive
        #   - Verify that the PercentSeting DUT response contains a uint8 with value between 1 and 100 inclusive
        #   - Verify that the PercentCurrent DUT response contains a uint8 with value between 1 and 100 inclusive
        #   - Verify that the FanMode DUT response contains a FanModeEnum with value between 1 and 5 inclusive, excluding 4
        self.step(3)
        init_speed_max = await self.read_verify_setting(attributes.SpeedMax, Uint8Type, range(1, 101))
        init_speed_setting = await self.read_verify_setting(attributes.SpeedSetting, Uint8Type, range(0, init_speed_max + 1))
        init_speed_current = await self.read_verify_setting(attributes.SpeedCurrent, Uint8Type, range(0, init_speed_max + 1))
        init_percent_setting = await self.read_verify_setting(attributes.PercentSetting, Uint8Type, range(0, 101))
        init_percent_current = await self.read_verify_setting(attributes.PercentCurrent, Uint8Type, range(0, 101))
        init_fan_mode = await self.read_verify_setting(attributes.FanMode, fm_enum, [0, 1, 2, 3, 5])
        logging.info(
            f"[FC] Initial state - SpeedSetting: {init_speed_setting}, SpeedCurrent: {init_speed_current}, PercentSetting: {init_percent_setting}, PercentCurrent: {init_percent_current}, FanMode: {init_fan_mode}")

        # *** STEP 4 ***
        # TH subscribes to the DUT's FanControl cluster
        self.step(4)
        self.attribute_subscription = ClusterAttributeChangeAccumulator(cluster)
        await self.attribute_subscription.start(self.default_controller, self.dut_node_id, self.endpoint)

        # *** STEP 5 ***
        # TH writes to the DUT a SpeedSetting value less than or equal to the SpeedMax value
        # Device shall return either SUCCESS or CONSTRAINT_ERROR
        self.step(5)
        # A SpeedSetting value update of 1 in any initial fan conditions should always result in the
        # FanMode attribute being set to kLow, regardless of the available fan modes specified by
        # the FanModeSequence attribute value. This makes the expected FanMode value deterministic
        # in this particular case. Using this approach as we don't know the manufacturer's mappings.
        speed_setting_expected = 1
        fan_mode_expected = fm_enum.kLow

        # The expected PercentSetting value can be calculated from the SpeedSetting
        # and SpeedMax values. The formula is provided by the Fan Control spec.
        percent_setting_expected = math.floor((speed_setting_expected / init_speed_max) * 100)

        # Write to the SpeedSetting attribute
        write_status = await self.write_setting(attributes.SpeedSetting, speed_setting_expected)

        # If the write operation returned SUCCESS, verify that the SpeedSetting, SpeedCurrent,
        # PercentSetting, PercentCurrent, and FanMode attributes are updated to their expected values
        if write_status == Status.Success:
            expected_values = [
                AttributeValue(self.endpoint, attributes.SpeedSetting, speed_setting_expected),
                AttributeValue(self.endpoint, attributes.SpeedCurrent, speed_setting_expected),
                AttributeValue(self.endpoint, attributes.PercentSetting, percent_setting_expected),
                AttributeValue(self.endpoint, attributes.PercentCurrent, percent_setting_expected),
                AttributeValue(self.endpoint, attributes.FanMode, fan_mode_expected)
            ]
            self.attribute_subscription.await_all_final_values_reported(
                expected_final_values=expected_values, timeout_sec=1)

        # If the write operation returned CONSTRAINT_ERROR, verify that the SpeedSetting, SpeedCurrent,
        # PercentSetting, PercentCurrent, and FanMode attributes are the same as their initial values
        elif write_status == Status.ConstraintError:
            await self.verify_attribute_constraint_error(attributes.FanMode, init_fan_mode)
            await self.verify_attribute_constraint_error(attributes.SpeedSetting, init_speed_setting)
            await self.verify_attribute_constraint_error(attributes.SpeedCurrent, init_speed_current)
            await self.verify_attribute_constraint_error(attributes.PercentSetting, init_percent_setting)
            await self.verify_attribute_constraint_error(attributes.PercentCurrent, init_percent_current)


if __name__ == "__main__":
    default_matter_test_main()
