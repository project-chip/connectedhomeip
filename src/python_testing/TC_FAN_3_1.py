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

import enum
import logging
import time
from enum import Enum
from typing import Any

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.interaction_model import Status
from chip.testing.matter_testing import (ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep, async_test_body,
                                         default_matter_test_main, has_feature)
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

    async def _supports_speed(self) -> bool:
        # TODO: Implement
        time.sleep(0.1)
        return True

    async def verify_fan_control_attribute_values(self, endpoint, attr_to_write, order, timeout_sec):
        # Setup
        cluster = Clusters.FanControl
        fan_mode_attr = cluster.Attributes.FanMode
        percent_setting_attr = cluster.Attributes.PercentSetting
        speed_setting_attr = cluster.Attributes.SpeedSetting
        speed_max_attr = cluster.Attributes.SpeedMax
        attribute_subscription = ClusterAttributeChangeAccumulator(cluster)
        fan_mode_off = cluster.Enums.FanModeEnum.kOff
        fan_mode_high = cluster.Enums.FanModeEnum.kHigh
        percent_setting_max_value = 100 # PercentSetting max value is 100 as per the spec

        # Sets the 'for loop' range based on the attribute that will be written to
        # and the specified order (ascending or descending)
        # Sets which mandatory attribute is to be verified (FanMode or PercentSetting)
        speed_max = await self.read_setting(endpoint, speed_max_attr)
        speed_setting_init = 0 if order == OrderEnum.Ascending else speed_max
        range_loop = range(1, percent_setting_max_value + 1) if order == OrderEnum.Ascending else range(percent_setting_max_value -1, -1, -1)
        value_init = fan_mode_off if order == OrderEnum.Ascending else fan_mode_high
        attr_to_verify = fan_mode_attr
        if attr_to_write == fan_mode_attr:
            range_loop = range(1, fan_mode_high.value + 1) if order == OrderEnum.Ascending else range(fan_mode_high.value -1, -1, -1)
            value_init = 0 if order == OrderEnum.Ascending else percent_setting_max_value
            attr_to_verify = percent_setting_attr

        logging.info(f"\n[F] Updating {attr_to_write.__name__} {order.name}, verifying {attr_to_verify.__name__} and SpeedSetting (if supported)")

        # Write and read back the initialization value for the attribute to verify
        # Verify that the write status is either SUCCESS or INVALID_IN_STATE
        # Verify written and read value match
        attr_value_read = await self.write_and_verify_attribute(endpoint, attr_to_verify, value_init)

        # Write and read back the initialization value for SpeedSetting (if supported)
        # Verify that the write status is either SUCCESS or INVALID_IN_STATE
        # Verify written and read value match
        if self.supports_speed:
            speed_setting_read = await self.write_and_verify_attribute(endpoint, speed_setting_attr, speed_setting_init)

        # Start subscription
        await attribute_subscription.start(self.default_controller, self.dut_node_id, endpoint)

        # Log FanMode, PercentSetting, and SpeedSetting initialization values
        fm = await self.read_setting(endpoint, fan_mode_attr)
        ps = await self.read_setting(endpoint, percent_setting_attr)
        logging.info(f"\n[F] Initialization state: FanMode({fm}) PercentSetting({ps}) SpeedSetting({speed_setting_read})")

        # Write to attribute iteratively within a range of 100, one at a time
        attr_value_current = attr_value_read
        attr_value_previous = attr_value_read
        speed_setting_current = speed_setting_read
        speed_setting_previous = speed_setting_read
        for value_to_write in range_loop:
            # Clear the queue
            attribute_subscription.get_last_report()

            # Write to attribute
            value_to_write = self.get_enum(value_to_write)
            write_result = await self.write_setting(endpoint, attr_to_write, value_to_write)
            write_status_success = (write_result == Status.Success) or (write_result == Status.InvalidInState)
            asserts.assert_true(write_status_success,
                                f"{attr_to_write.__name__} write did not return a result of either SUCCESS or INVALID_IN_STATE")
            logging.info(f"\n[F] {attr_to_write.__name__} written: {value_to_write}")

            # Get the subscription queue
            queue = attribute_subscription.attribute_queue.queue

            # Verifying PercentSetting
            attr_value_current = await self.get_attribute_value_from_queue(queue, attr_to_verify, timeout_sec)
            if attr_value_current is not None:
                self.verify_attribute_progression(attr_to_verify, attr_value_current, attr_value_previous, order)
                attr_value_previous = attr_value_current

            # Verifying SpeedSetting (if supported)
            if self.supports_speed:
                speed_setting_current = await self.get_attribute_value_from_queue(queue, speed_setting_attr, timeout_sec)
                if speed_setting_current is not None:
                    self.verify_attribute_progression(speed_setting_attr, speed_setting_current, speed_setting_previous, order)
                    speed_setting_previous = speed_setting_current

        await attribute_subscription.cancel()

    async def write_and_verify_attribute(self, endpoint, attribute, value_init):
        value_init = self.get_enum(value_init)

        write_result = await self.write_setting(endpoint, attribute, value_init)
        write_status_success = (write_result == Status.Success) or (write_result == Status.InvalidInState)
        asserts.assert_true(write_status_success,
                            f"{attribute.__name__} write did not return a result of either SUCCESS or INVALID_IN_STATE ({write_result.name})")
        value_read = await self.read_setting(endpoint, attribute)
        asserts.assert_equal(value_read, value_init,
                             f"Mismatch between written and read {attribute.__name__}")
        return value_read

    def verify_attribute_progression(self, attribute, value_current, value_previous, order):
        value_current = self.get_enum(value_current)
        if order == OrderEnum.Ascending:
            # Verify the current FanMode is greater than the previous FanMode
            asserts.assert_greater(value_current, value_previous,
                                f"Current {attribute.__name__} must be greater than previous {attribute.__name__}")
        else:
            # Verify the current FanMode is less than the previous FanMode
            asserts.assert_less(value_current, value_previous,
                                f"Current {attribute.__name__} must be less than previous {attribute.__name__}")
        logging.info(
            f"\n\t[F] {attribute.__name__} changed from ({value_previous}) to ({value_current})")

    @staticmethod
    async def get_attribute_value_from_queue(queue, attribute, timeout_sec) -> Any:
        start_time = time.time()
        elapsed = 0
        time_remaining = timeout_sec
        break_out = False
        value = None

        # Wait for the attribute to appear in the queue until the specified timeout
        while time_remaining > 0:
            for q in list(queue):
                if q.attribute == attribute:
                    value = q.value
                    break_out = True
                    break

            elapsed = time.time() - start_time
            time_remaining = timeout_sec - elapsed

            if break_out:
                break

        return value

    @staticmethod
    def get_enum(value):
        if isinstance(value, enum.Enum):
            enum_type = type(value)
            value = enum_type(value)
        return value

    def pics_TC_FAN_3_1(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_1(self):
        # Setup
        endpoint = self.get_endpoint(default=1)
        percent_setting_attr = Clusters.FanControl.Attributes.PercentSetting
        fan_mode_attr = Clusters.FanControl.Attributes.FanMode
        self.supports_speed = await self._supports_speed()
        timeout_sec = 0.333

        # TH writes to the DUT the PercentSetting attribute iteratively within
        # a range of 1 to 100 one at a time in ascending order
        # Verifies that FanMode and SpeedSetting values (if supported) are being
        # updated accordingly (greater or less than the previous values)
        await self.verify_fan_control_attribute_values(
            endpoint=endpoint,
            attr_to_write=percent_setting_attr,
            order=OrderEnum.Ascending,
            timeout_sec=timeout_sec)

        # TH writes to the DUT the PercentSetting attribute iteratively within
        # a range of 1 to 100 one at a time in descending order
        # Verifies that FanMode and SpeedSetting values (if supported) are being
        # updated accordingly (greater or less than the previous values)
        await self.verify_fan_control_attribute_values(
            endpoint=endpoint,
            attr_to_write=percent_setting_attr,
            order=OrderEnum.Descending,
            timeout_sec=timeout_sec)

        # TH writes to the DUT the FanMode attribute iteratively within a range of
        # the number of available fan modes one at a time in ascending order
        # Verifies that PercentSetting and SpeedSetting values (if supported) are being
        # updated accordingly (greater or less than the previous values)
        await self.verify_fan_control_attribute_values(
            endpoint=endpoint,
            attr_to_write=fan_mode_attr,
            order=OrderEnum.Ascending,
            timeout_sec=timeout_sec)

        # TH writes to the DUT the FanMode attribute iteratively within a range of
        # the number of available fan modes one at a time in descending order
        # Verifies that PercentSetting and SpeedSetting values (if supported) are being
        # updated accordingly (greater or less than the previous values)
        await self.verify_fan_control_attribute_values(
            endpoint=endpoint,
            attr_to_write=fan_mode_attr,
            order=OrderEnum.Descending,
            timeout_sec=timeout_sec)


if __name__ == "__main__":
    default_matter_test_main()
