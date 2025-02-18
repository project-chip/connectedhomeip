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

import asyncio
import enum
import logging
import time
from enum import Enum
from typing import Any

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
    def steps_TC_FAN_3_1(self):
        return [TestStep(1, "[FC] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[FC] TH reads the FanModeSequence attribute to retreive the available fan modes.",
                         "Save the result for future reference."),
                TestStep(3, "[FC] TH reads the FeatureMap attribute from the DUT.",
                         "Check for the existence of the SpeedSetting feature. If the SpeedSetting feature is supported, read the SpeedMax attribute and save the result for future reference."),
                TestStep(4, "[FC] TH subscribes to the DUT's FanControl cluster.", "Enables the TH to receive attribute updates."),
                TestStep(5, "[FC] TH configures the testing setup for the following scenario: Updating the PercentSetting attribute in ascending order and monitoring the FanMode (and SpeedSetting, if supported) attributes.",
                         "Initialize attribute values: - FanMode: 0 (Off). - PercentSetting: 0. - SpeedSetting (if supported): 0. Read the attribute values back and verify that they match the written values."),
                TestStep(6, "[FC] TH updates the value of the PercentSetting attribute of the DUT iteratively, in ascending order (from 1 to 100). For each iteration, the TH reads the value of the FanMode attribute (and SpeedSetting, if supported) from the DUT.",
                         "For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. Verify the following for the FanMode attribute value (and SpeedSetting, if supported): - For iterations where the DUT returns a SUCCESS status code after the PercentSetting attribute value update, and a change in the attribute value is detected, verify that the current value is greater than the previous one. - For iterations where the DUT returns an INVALID_IN_STATE status code after the PercentSetting attribute value update, which means that no update operation occurred, verify that the current value is the same as the previous one."),
                TestStep(7, "[FC] TH configures the testing setup for the following scenario: Updating the PercentSetting attribute in descending order and monitoring the FanMode (and SpeedSetting, if supported) attributes.",
                         "Initialize attribute values: - FanMode: 3 (High). - PercentSetting: 100. - SpeedSetting (if supported): SpeedMax attribute value. Read the attribute values back and verify that they match the written values."),
                TestStep(8, "[FC] TH updates the value of the PercentSetting attribute of the DUT iteratively, in descending order (from 99 to 0). For each iteration, the TH reads the value of the FanMode attribute (and SpeedSetting, if supported) from the DUT.",
                         "For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. Verify the following for the FanMode attribute value (and SpeedSetting, if supported): - For iterations where the DUT returns a SUCCESS status code after the PercentSetting attribute value update, and a change in the attribute value is detected, verify that the current value is less than the previous one. - For iterations where the DUT returns an INVALID_IN_STATE status code after the PercentSetting attribute value update, which means that no update operation occurred, verify that the current value is the same as the previous one."),
                TestStep(9, "[FC] TH updates the DUT's FanMode attribute value iteratively, in accordance with the available fan modes provided by the FanModeSequence attribute, in ascending order from the fan mode subsequent to 0 (Off) to 3 (High). For each iteration, the TH reads the value of the PercentSetting attribute (and SpeedSetting, if supported) from the DUT.",
                         "Initialize attribute values: - FanMode: 0 (Off). - PercentSetting: 0. - SpeedSetting (if supported): 0. Read the attribute values back and verify that they match the written values."),
                TestStep(10, "[FC] TH updates the value of the FanMode attribute of the DUT iteratively, in ascending order (from 0 (Off) to 3 (High)). For each iteration, the TH reads the value of the PercentSetting attribute (and SpeedSetting, if supported) from the DUT.",
                         "For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. Verify the following for the PercentSetting attribute value (and SpeedSetting, if supported): - For iterations where the DUT returns a SUCCESS status code after the FanMode attribute value update, and a change in the attribute value is detected, verify that the current value is greater than the previous one. - For iterations where the DUT returns an INVALID_IN_STATE status code after the FanMode attribute value update, which means that no update operation occurred, verify that the current value is the same as the previous one."),
                TestStep(11, "[FC] TH updates the DUT's FanMode attribute value iteratively, in accordance with the available fan modes provided by the FanModeSequence attribute, in descending order from the fan mode prior to 3 (High) to 0 (Off). For each iteration, the TH reads the value of the PercentSetting attribute (and SpeedSetting, if supported) from the DUT.",
                         "Initialize attribute values: - FanMode: 3 (High). - PercentSetting: 100. - SpeedSetting (if supported): SpeedMax attribute value. Read the attribute values back and verify that they match the written values."),
                TestStep(12, "[FC] TH updates the value of the FanMode attribute of the DUT iteratively, in descending order (from 3 (High) to 0 (Off)) For each iteration, the TH reads the value of the PercentSetting attribute (and SpeedSetting, if supported) from the DUT.",
                         "For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. Verify the following for the PercentSetting attribute value (and SpeedSetting, if supported): - For iterations where the DUT returns a SUCCESS status code after the FanMode attribute value update, and a change in the attribute value is detected, verify that the current value is less than the previous one. - For iterations where the DUT returns an INVALID_IN_STATE status code after the FanMode attribute value update, which means that no update operation occurred, verify that the current value is the same as the previous one."),
                ]

    @staticmethod
    async def get_attribute_value_from_queue(queue, attribute, timeout_sec: float) -> Any:
        start_time = time.time()
        while time.time() - start_time < timeout_sec:
            for q in list(queue):
                if q.attribute == attribute:
                    return q.value
            await asyncio.sleep(0.01)
        return None

    @staticmethod
    def get_enum_value(value) -> int:
        """
        Retrieves the numeric value of an Enum instance.

        - If the input is an Enum instance, it returns its corresponding numeric value.
        - If the input is already a numeric value, it returns it unchanged.

        Args:
            value (enum.Enum or int): The input value to be processed.

        Returns:
            int: The numeric value of the Enum instance or the original integer.
        """
        if isinstance(value, enum.Enum):
            enum_type = type(value)
            value = enum_type(value)
        return value

    async def read_setting(self, endpoint, attribute):
        cluster = Clusters.Objects.FanControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def write_setting(self, endpoint, attribute, value) -> Status:
        value = self.get_enum_value(value)
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, attribute(value))])
        write_status = result[0].Status
        write_status_success = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_success,
                            f"[FC] {attribute.__name__} write did not return a result of either SUCCESS or INVALID_IN_STATE ({write_status.name})")
        return write_status

    async def write_and_verify_attribute(self, endpoint, attribute, value) -> Any:
        await self.write_setting(endpoint, attribute, value)
        value_read = await self.read_setting(endpoint, attribute)
        asserts.assert_equal(value_read, value,
                             f"[FC] Mismatch between written and read attribute value ({attribute.__name__})")
        return value_read

    async def get_fan_modes(self, endpoint, exclude_auto: bool = False) -> list[int]:
        # Read FanModeSequence attribute value
        fan_mode_sequence_attr = Clusters.FanControl.Attributes.FanModeSequence
        fm_en = Clusters.FanControl.Enums.FanModeEnum
        fan_mode_sequence = await self.read_setting(endpoint, fan_mode_sequence_attr)

        # There are only 6 valid FanModeSequence values (0, 1, 2, 3, 4, 5)
        # Verifying read FanModeSequence attribute value is valid
        asserts.assert_in(fan_mode_sequence, range(0, 5), f"Unsupported FanModeSequence: {fan_mode_sequence}")

        fan_mode_sequence_enum_value = self.get_enum_value(fan_mode_sequence)
        logging.info(f"[FC] Supported fan modes: {fan_mode_sequence_enum_value.name}")

        fan_modes = None
        if fan_mode_sequence == 0:
            fan_modes = [fm_en.kOff.value, fm_en.kLow.value, fm_en.kMedium.value, fm_en.kHigh.value]
        elif fan_mode_sequence == 1:
            fan_modes = [fm_en.kOff.value, fm_en.kLow.value, fm_en.kHigh.value]
        elif fan_mode_sequence == 2:
            fan_modes = [fm_en.kOff.value, fm_en.kLow.value, fm_en.kMedium.value, fm_en.kHigh.value, fm_en.kAuto.value]
        elif fan_mode_sequence == 3:
            fan_modes = [fm_en.kOff.value, fm_en.kLow.value, fm_en.kHigh.value, fm_en.kAuto.value]
        elif fan_mode_sequence == 4:
            fan_modes = [fm_en.kOff.value, fm_en.kHigh.value, fm_en.kAuto.value]
        elif fan_mode_sequence == 5:
            fan_modes = [fm_en.kOff.value, fm_en.kHigh.value]

        if exclude_auto and fm_en.kAuto.value in fan_modes:
            fan_modes.remove(fm_en.kAuto.value)

        return fan_modes

    async def get_initialization_parametes(self, attr_to_update, order):
        cluster = Clusters.FanControl
        fan_mode_attr = cluster.Attributes.FanMode
        percent_setting_attr = cluster.Attributes.PercentSetting
        fan_mode_off = cluster.Enums.FanModeEnum.kOff
        fan_mode_high = cluster.Enums.FanModeEnum.kHigh
        percent_setting_max_value = 100  # PercentSetting max value is 100 as per the spec

        # Sets the number of iterations for attribute updates
        # Sets the update order (ascending or descending)
        if attr_to_update == percent_setting_attr:
            attr_to_verify = fan_mode_attr
            iteration_range = range(1, percent_setting_max_value +
                                    1) if order == OrderEnum.Ascending else range(percent_setting_max_value - 1, -1, -1)
            value_init_verify = fan_mode_off if order == OrderEnum.Ascending else fan_mode_high
            value_init_update = 0 if order == OrderEnum.Ascending else percent_setting_max_value
        elif attr_to_update == fan_mode_attr:
            attr_to_verify = percent_setting_attr
            iteration_range = range(1, len(self.fan_modes)) if order == OrderEnum.Ascending else range(
                len(self.fan_modes) - 2, -1, -1)
            value_init_verify = 0 if order == OrderEnum.Ascending else percent_setting_max_value
            value_init_update = fan_mode_off if order == OrderEnum.Ascending else fan_mode_high

        return attr_to_verify, value_init_verify, value_init_update, iteration_range

    def verify_attribute_change(self, attr_to_verify, value_current, value_previous, order) -> None:
        if order == OrderEnum.Ascending:
            # Verify that the current attribute value is greater than the previous attribute value
            asserts.assert_greater(value_current, value_previous,
                                   f"[FC] Current {attr_to_verify.__name__} must be greater than previous {attr_to_verify.__name__}")
        else:
            # Verify that the current attribute value is less than the previous attribute value
            asserts.assert_less(value_current, value_previous,
                                f"[FC] Current {attr_to_verify.__name__} must be less than previous {attr_to_verify.__name__}")

        # Logging attribute value change details
        sub_text = f"({value_previous}) to ({value_current})"
        if attr_to_verify == Clusters.FanControl.Attributes.FanMode:
            sub_text = f"({value_previous}:{value_previous.name}) to ({value_current}:{value_current.name})"
        logging.info(f"\t\t[FC] {attr_to_verify.__name__} changed from {sub_text}")

    async def log_scenario(self, endpoint, attr_to_update, attr_to_verify, speed_setting_read, order):
        cluster = Clusters.FanControl
        fan_mode_attr = cluster.Attributes.FanMode
        percent_setting_attr = cluster.Attributes.PercentSetting

        # Logging the scenario being tested
        sub_text = " and SpeedSetting" if self.supports_speed else ""
        logging.info(
            f"[FC] Update {attr_to_update.__name__} {order.name}, verify {attr_to_verify.__name__}{sub_text}")

        # Logging initialization values (FanMode, PercentSetting, and SpeedSetting (if supported))
        fan_mode_log = cluster.Enums.FanModeEnum(await self.read_setting(endpoint, fan_mode_attr))
        percent_setting_log = await self.read_setting(endpoint, percent_setting_attr)
        sub_text = f" SpeedSetting({speed_setting_read})" if self.supports_speed else ""
        logging.info(
            f"[FC] Initialization values: FanMode({fan_mode_log}:{fan_mode_log.name}) PercentSetting({percent_setting_log}){sub_text}")

    async def verify_fan_control_attribute_progression(self, endpoint, attr_to_update, order) -> None:
        # Setup
        speed_setting_attr = Clusters.FanControl.Attributes.SpeedSetting
        timeout_sec = 0.1  # Timeout given for item retreival from the attribute queue

        # *** NEXT STEP ***
        # TH configures the testing setup for the one of the following scenarios:
        #   - Updating the PercentSetting attribute in ascending order and monitoring the FanMode (and SpeedSetting, if supported) attributes
        #   - Updating the PercentSetting attribute in descending order and monitoring the FanMode (and SpeedSetting, if supported) attributes
        #   - Updating the FanMode attribute in aescending order and monitoring the PercentSetting (and SpeedSetting, if supported) attributes
        #   - Updating the FanMode attribute in descending order and monitoring the PercentSetting (and SpeedSetting, if supported) attributes
        self.step(self.current_step_index + 1)

        # Get initialization parameters
        attr_to_verify, value_init_verify, value_init_update, iteration_range = \
            await self.get_initialization_parametes(attr_to_update, order)

        # Initializatization of the attribute to update (Write and read back verification)
        await self.write_and_verify_attribute(endpoint, attr_to_update, value_init_update)

        # Initializatization of the attribute to verify (Write and read back verification)
        attr_value_read = await self.write_and_verify_attribute(endpoint, attr_to_verify, value_init_verify)

        # Initializatization of the SpeedSetting attribute, if supported (Write and read back verification)
        if self.supports_speed:
            speed_setting_init = 0 if order == OrderEnum.Ascending else self.speed_max
            speed_setting_read = await self.write_and_verify_attribute(endpoint, speed_setting_attr, speed_setting_init)

        # *** NEXT STEP ***
        # TH performs the testing scenario defined in the previous step
        self.step(self.current_step_index + 1)

        # Logging the scenario being tested
        await self.log_scenario(endpoint, attr_to_update, attr_to_verify, speed_setting_read, order)

        # Write to attribute iteratively
        attr_value_current = attr_value_read
        attr_value_previous = attr_value_read
        speed_setting_current = speed_setting_read
        speed_setting_previous = speed_setting_read
        for value_to_write in iteration_range:
            # Clear the attribute report queue before each update to avoid duplicates
            self.attribute_subscription.get_last_report()

            # Write to attribute
            write_status = await self.write_setting(endpoint, attr_to_update, value_to_write)
            logging.info(f"[FC] {attr_to_update.__name__} written: {value_to_write}")

            # If the write status is SUCCESS, it means the write operation occurred
            # Verify that the current attribute value is greater than the previous
            # one if order was set to Ascending, or less if order was set to Descending
            if write_status == Status.Success:
                # Get current attribute value and verify correct increment/decrement
                queue = self.attribute_subscription.attribute_queue.queue
                attr_value_current = await self.get_attribute_value_from_queue(queue, attr_to_verify, timeout_sec)
                if attr_value_current is not None:
                    self.verify_attribute_change(attr_to_verify, attr_value_current, attr_value_previous, order)
                    attr_value_previous = attr_value_current

                # Get current SpeedSetting attribute value and verify correct increment/decrement (if supported)
                if self.supports_speed:
                    speed_setting_current = await self.get_attribute_value_from_queue(queue, speed_setting_attr, timeout_sec)
                    if speed_setting_current is not None:
                        self.verify_attribute_change(speed_setting_attr, speed_setting_current, speed_setting_previous, order)
                        speed_setting_previous = speed_setting_current

            # If the write status is INVALID_IN_STATE, it means no write operation occurred
            # Verify that the current attribute value is equal to the previous one
            elif write_status == Status.InvalidInState:
                # Get current attribute value and verify it's equal to the previous value
                attr_value_current = await self.read_setting(endpoint, attr_to_verify)
                asserts.assert_equal(attr_value_current, attr_value_previous,
                                     f"[FC] Current {attr_to_verify.__name__} attribute value must be equal to the previous one")

                # Get current SpeedSetting attribute value and verify it's equal to the previous value (if supported)
                if self.supports_speed:
                    speed_setting_current = await self.read_setting(endpoint, speed_setting_attr)
                    asserts.assert_equal(speed_setting_current, speed_setting_previous,
                                         "Current SpeedSetting attribute value must be equal to the previous one")

    def pics_TC_FAN_3_1(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_1(self):
        # Setup
        ep = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attributes = cluster.Attributes

        # *** STEP 1 ***
        # Commissioning already done
        self.step(1)

        # *** STEP 2 ***
        # TH reads the FanModeSequence attribute to retreive the available fan modes
        self.step(2)
        self.fan_modes = await self.get_fan_modes(ep, exclude_auto=True)

        # *** STEP 3 ***
        # TH reads the FeatureMap attribute to check for support of the SpeedSetting feature
        self.step(3)
        feature_map = await self.read_setting(ep, attributes.FeatureMap)
        self.supports_speed = bool(feature_map & cluster.Bitmaps.Feature.kMultiSpeed)
        if self.supports_speed:
            self.speed_max = await self.read_setting(ep, attributes.SpeedMax)

        # *** STEP 4 ***
        # TH subscribes to the DUT's FanControl cluster
        self.step(4)
        self.attribute_subscription = ClusterAttributeChangeAccumulator(cluster)
        await self.attribute_subscription.start(self.default_controller, self.dut_node_id, ep)

        # TH updates the DUT's PercentSetting attribute value iteratively, in ascending order (from 1 to 100)
        # and monitors the FanMode (and SpeedSetting, if supported) attribute value's correct progression
        # (current value greater than previous value)
        await self.verify_fan_control_attribute_progression(ep, attributes.PercentSetting, OrderEnum.Ascending)

        # TH updates the DUT's PercentSetting attribute value iteratively, in descending order (from 99 to 0)
        # and monitors the FanMode (and SpeedSetting, if supported) attribute value's correct progression
        # (current value less than previous value)
        await self.verify_fan_control_attribute_progression(ep, attributes.PercentSetting, OrderEnum.Descending)

        # TH updates the DUT's FanMode attribute value iteratively, in accordance with the
        # available fan modes provided by the FanModeSequence attribute, in ascending order
        # from the fan mode subsequent to 0 (Off) to 3 (High) and monitors the PercentSetting
        # (and SpeedSetting, if supported) attribute value's correct progression
        # (current value greater than previous value)
        await self.verify_fan_control_attribute_progression(ep, attributes.FanMode, OrderEnum.Ascending)

        # TH updates the DUT's FanMode attribute value iteratively, in accordance with the
        # available fan modes provided by the FanModeSequence attribute, in descending order
        # from the fan mode prior to 3 (High) to 0 (Off) and monitors the PercentSetting
        # (and SpeedSetting, if supported) attribute value's correct progression
        # (current value less than previous value)
        await self.verify_fan_control_attribute_progression(ep, attributes.FanMode, OrderEnum.Descending)


if __name__ == "__main__":
    default_matter_test_main()
