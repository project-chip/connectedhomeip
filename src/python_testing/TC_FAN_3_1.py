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
from enum import Enum

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.interaction_model import Status
from chip.testing.matter_testing import (ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep, async_test_body,
                                         default_matter_test_main)
from mobly import asserts


class OrderEnum(Enum):
    Ascending = 1
    Descending = 2


logger = logging.getLogger(__name__)


class TC_FAN_3_1(MatterBaseTest):
    # def steps_TC_FAN_3_1(self):
    #     return [TestStep("1", "Commissioning already done."),
    #             TestStep("2", "Action", "Verification"),
    #             ]
    async def read_fc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.FanControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_setting(self, endpoint, attribute):
        return await self.read_fc_attribute_expect_success(endpoint, attribute)

    async def write_setting(self, endpoint, attr_to_write, value) -> Status:
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, attr_to_write(value))])
        return result[0].Status

    async def verify_fan_mode(self, endpoint, attr_to_write, order, timeout_sec):
        logging.info(f"[FANS] Monitoring FanMode by writing to {attr_to_write.__name__} in {order.name} order")

        # Setup
        cluster = Clusters.FanControl
        fan_mode_attr = cluster.Attributes.FanMode
        attribute_subscription = ClusterAttributeChangeAccumulator(cluster)
        fan_mode_off = cluster.Enums.FanModeEnum.kOff
        fan_mode_high = cluster.Enums.FanModeEnum.kHigh

        # As per the spec, the max values for PercentSetting and SpeedSetting are 100 and SpeedMax respectively
        max_value = 100
        if attr_to_write == cluster.Attributes.SpeedSetting:
            max_value = await self.read_setting(endpoint, cluster.Attributes.SpeedMax)

        # Determine if values will be written ascending or descending
        fan_mode_init = fan_mode_off if order == OrderEnum.Ascending else fan_mode_high
        range_loop = range(1, max_value + 1) if order == OrderEnum.Ascending else range(max_value, -1, -1)

        # Write initial FanMode
        write_result = await self.write_setting(endpoint, fan_mode_attr, fan_mode_init)
        write_status_success = (write_result == Status.Success) or (write_result == Status.InvalidInState)
        asserts.assert_true(write_status_success,
                            f"FanMode write did not return a result of either SUCCESS or INVALID_IN_STATE ({write_result.name})")

        # Read FanMode back and verify written value
        fan_mode_initial = await self.read_setting(endpoint, fan_mode_attr)
        asserts.assert_in(fan_mode_initial, cluster.Enums.FanModeEnum, "FanMode read response doesn't contain a FanModeEnum")

        await attribute_subscription.start(self.default_controller, self.dut_node_id, endpoint)

        # Write to attribute iteratively within a range of 100, one at a time
        fan_mode_current = fan_mode_initial
        fan_mode_previous = fan_mode_initial
        for value_to_write in range_loop:
            # Clear the queue
            attribute_subscription.get_last_report()

            # Write to attribute
            write_result = await self.write_setting(endpoint, attr_to_write, value_to_write)
            write_status_success = (write_result == Status.Success) or (write_result == Status.InvalidInState)
            asserts.assert_true(write_status_success,
                                "Attribute write did not return a result of either SUCCESS or INVALID_IN_STATE")
            logging.info(f"[FANS] Attribute value written: {value_to_write}")

            # Wait for the FanMode attribute to appear in the queue until the specified timeout
            start_time = time.time()
            elapsed = 0
            time_remaining = timeout_sec
            break_out = False
            while time_remaining > 0:
                q = attribute_subscription.attribute_queue

                # Iterate through the queue items to search for the FanMode attribute
                for q_item in list(q.queue):
                    if q_item.attribute == fan_mode_attr:
                        fan_mode_current = q_item.value

                        if order == OrderEnum.Ascending:
                            # Verify the current FanMode is greater than the previous FanMode
                            asserts.assert_greater(fan_mode_current, fan_mode_previous,
                                                   "Current FanMode must be greater than previous FanMode")
                        else:
                            # Verify the current FanMode is less than the previous FanMode
                            asserts.assert_less(fan_mode_current, fan_mode_previous,
                                                "Current FanMode must be less than previous FanMode")

                        logging.info(
                            f"[FANS] FanMode changed from {fan_mode_previous.name}({fan_mode_previous}) to {fan_mode_current.name}({fan_mode_current})")
                        fan_mode_previous = fan_mode_current
                        break_out = True
                        break

                elapsed = time.time() - start_time
                time_remaining = timeout_sec - elapsed

                if break_out:
                    break

        await attribute_subscription.cancel()

    def pics_TC_FAN_3_1(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_1(self):
        # Setup
        endpoint = self.get_endpoint(default=1)
        percent_setting_attr = Clusters.FanControl.Attributes.PercentSetting
        speed_setting_attr = Clusters.FanControl.Attributes.SpeedSetting
        timeout_sec = 0.2

        # TH writes to the DUT the PercentSetting iteratively within a range of 1 to 100 one at a time in ascending order
        await self.verify_fan_mode(endpoint, percent_setting_attr, OrderEnum.Ascending, timeout_sec)

        # TH writes to the DUT the PercentSetting iteratively within a range of 1 to 100 one at a time in descending order
        await self.verify_fan_mode(endpoint, percent_setting_attr, OrderEnum.Descending, timeout_sec)

        # TH writes to the DUT the SpeedSetting iteratively within a range of 1 to SpeedMax one at a time in ascending order
        await self.verify_fan_mode(endpoint, speed_setting_attr, OrderEnum.Ascending, timeout_sec)

        # TH writes to the DUT the SpeedSetting iteratively within a range of 1 to SpeedMax one at a time in descending order
        await self.verify_fan_mode(endpoint, speed_setting_attr, OrderEnum.Descending, timeout_sec)


if __name__ == "__main__":
    default_matter_test_main()
