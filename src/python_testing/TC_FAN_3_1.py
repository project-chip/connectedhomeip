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
#       --timeout 600
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import time
from enum import Enum
from typing import Any, Optional

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
    def desc_TC_FAN_3_1(self) -> str:
        return "[TC-FAN-3.1] Mandatory functionality with DUT as Server"

    def steps_TC_FAN_3_1(self):
        return [TestStep(1, "[FC] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[FC] TH reads the FanModeSequence attribute from the DUT. This attribute specifies the available fan modes.",
                         "Verify that the DUT response contains a FanModeSequenceEnum and store."),
                TestStep(3, "[FC] TH checks the DUT for support of the MultiSpeed feature.",
                         "If the MultiSpeed feature is supported the SpeedSetting attribute will be present."),
                TestStep(4, "[FC] TH subscribes to the DUT's FanControl cluster.", "Enables the TH to receive attribute updates."),
                TestStep(5, "[FC] TH tests the following scenario: - Attribute to update: PercentSetting - Attribute to verify: PercentSetting, FanMode and SpeedSetting (if present) - Update order: Ascending",
                         "Update the value of the PercentSetting attribute iteratively, in ascending order, from 0 to 100. For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. If SUCCESS and a PercentSetting, FanMode (and/or SpeedSetting if present) attribute value change is triggered: Verify that the current value(s) is greater than the previous one. If INVALID_IN_STATE, no update operation occurred. Verify that the current value(s) is the same as the previous one"),
                TestStep(6, "[FC] TH tests the following scenario: - Attribute to update: PercentSetting - Attribute to verify: PercentSetting, FanMode and SpeedSetting (if present) - Update order: Descending",
                         "Update the value of the PercentSetting attribute iteratively, in descending order, from 100 to 0. For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. If SUCCESS and a PercentSetting, FanMode (and/or SpeedSetting if present) attribute value change is triggered. Verify that the current value(s) is less than the previous one. If INVALID_IN_STATE, no update operation occurred. Verify that the current value(s) is the same as the previous one"),
                TestStep(7, "[FC] TH tests the following scenario: - Attribute to update: FanMode - Attribute to verify: FanMode, PercentSetting and SpeedSetting (if present) - Update order: Ascending",
                         "Update the value of the FanMode attribute iteratively, in ascending order, from 0 (Off) to the number of available fan modes specified by the FanModeSequence attribute excluding modes beyond 3 (High). For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. If SUCCESS and a FanMode, PercentSetting (and/or SpeedSetting if present) attribute value change is triggered. Verify that the current value(s) is greater than the previous one. If INVALID_IN_STATE, no update operation occurred. Verify that the current value(s) is the same as the previous one"),
                TestStep(8, "[FC] TH tests the following scenario: - Attribute to update: FanMode - Attribute to verify: FanMode, PercentSetting and SpeedSetting (if present) - Update order: Descending",
                         "Update the value of the FanMode attribute iteratively, in descending order, from the number of available fan modes specified by the FanModeSequence attribute excluding modes beyond 3 (High) to 0 (Off). For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. If SUCCESS and a FanMode, PercentSetting (and/or SpeedSetting if present) attribute value change is triggered. Verify that the current value(s) is less than the previous one. If INVALID_IN_STATE, no update operation occurred. Verify that the current value(s) is the same as the previous one"),
                ]

    async def read_setting(self, endpoint: int, attribute: Any) -> Any:
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
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def write_setting(self, endpoint, attribute, value) -> Status:
        """
        Writes a specified attribute value.

        Args:
            endpoint (int): The endpoint identifier for the fan device.
            attribute (Callable): The attribute to be written.
            value (Any): The value to write to the attribute.

        Returns:
            Status: The status of the write operation.

        Raises:
            AssertionError: If the write status is neither SUCCESS nor INVALID_IN_STATE.
        """
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, attribute(value))])
        write_status = result[0].Status
        write_status_success = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_success,
                            f"[FC] {attribute.__name__} write did not return a result of either SUCCESS or INVALID_IN_STATE ({write_status.name})")
        return write_status

    async def write_and_verify_attribute(self, endpoint, attribute, value) -> Status:
        """
        Writes a value to a specified attribute on a given endpoint and verifies the write operation by reading back the value.

        Args:
            endpoint (Any): The endpoint identifier for the fan device.
            attribute (Any): The attribute to be written.
            value (Any): The value to write to the attribute.

        Returns:
            Status: The status of the write operation.

        Raises:
            AssertionError: If the value read back does not match the value written.
        """
        write_status = await self.write_setting(endpoint, attribute, value)
        if write_status == Status.Success:
            value_read = await self.read_setting(endpoint, attribute)
            asserts.assert_equal(value_read, value,
                                 f"[FC] Mismatch between written and read attribute value ({attribute.__name__} - written: {value}, read: {value_read})")
        return write_status

    async def get_attribute_value_from_queue(self, attribute) -> Optional[Any]:
        """
        Wait for the latest occurrence of a specific attribute update in the queue within a given timeout.

        Args:
            attribute (str): The attribute to wait for.

        Returns:
            Optional[Any]: The latest value of the attribute if found within the timeout, otherwise None.
        """
        queue = self.attribute_subscription.attribute_queue.queue
        start_time = time.time()

        while time.time() - start_time < self.timeout_sec:
            if queue:
                last_value = None
                index_to_remove = None

                # Iterate the queue in reverse order
                for i in range(len(queue) - 1, -1, -1):

                    # Find matching attribute in the queue
                    if queue[i].attribute == attribute:
                        last_value = queue[i].value

                        # Mark matching item for removal
                        index_to_remove = i
                        break

                # Remove matching item from the queue to
                # avoid false positives in subsequent runs
                if index_to_remove is not None:
                    del queue[index_to_remove]

                if last_value is not None:
                    # Return the latest match
                    return last_value

            await asyncio.sleep(self.timeout_sec / 4)

        # Return None if timeout expires
        return None

    async def get_fan_modes(self, endpoint, max_high: bool = False):
        """
        Gets the available fan modes provided by the FanModeSequence attribute.

        Args:
            endpoint (int): The endpoint identifier for the fan device.
            max_high (bool, optional): If True, limits the fan modes to those up to and including 'High'. Defaults to False.

        Raises:
            AssertionError: If the FanModeSequence attribute value is not an instance of FanModeSequenceEnum.

        Returns:
            None: The fan modes are stored in the instance variable `self.fan_modes`.
        """
        # Read FanModeSequence attribute value
        fan_mode_sequence_attr = Clusters.FanControl.Attributes.FanModeSequence
        fm_enum = Clusters.FanControl.Enums.FanModeEnum
        fms_enum = Clusters.FanControl.Enums.FanModeSequenceEnum
        self.fan_mode_sequence = await self.read_setting(endpoint, fan_mode_sequence_attr)

        # Verify response contains a FanModeSequenceEnum
        asserts.assert_is_instance(self.fan_mode_sequence, fms_enum,
                                   f"[FC] FanModeSequence result isn't of enum type {fms_enum.__name__}")

        fan_modes = None
        if self.fan_mode_sequence == 0:
            fan_modes = [fm_enum.kOff, fm_enum.kLow, fm_enum.kMedium, fm_enum.kHigh]
        elif self.fan_mode_sequence == 1:
            fan_modes = [fm_enum.kOff, fm_enum.kLow, fm_enum.kHigh]
        elif self.fan_mode_sequence == 2:
            fan_modes = [fm_enum.kOff, fm_enum.kLow, fm_enum.kMedium, fm_enum.kHigh, fm_enum.kAuto]
        elif self.fan_mode_sequence == 3:
            fan_modes = [fm_enum.kOff, fm_enum.kLow, fm_enum.kHigh, fm_enum.kAuto]
        elif self.fan_mode_sequence == 4:
            fan_modes = [fm_enum.kOff, fm_enum.kHigh, fm_enum.kAuto]
        elif self.fan_mode_sequence == 5:
            fan_modes = [fm_enum.kOff, fm_enum.kHigh]

        if max_high:
            if fm_enum.kHigh in fan_modes:
                high_index = fan_modes.index(fm_enum.kHigh)
                fan_modes = fan_modes[:high_index + 1]  # Keep only modes up to and including kHigh

        self.fan_modes = fan_modes

    async def get_initial_parametes(self, endpoint, attr_to_update, order) -> tuple:
        """
        This method gets the necessary initial parameters for updating fan control attributes
        such as PercentSetting, FanMode, and SpeedSetting (if present). It determines the
        attribute to verify, the iteration range for updates, and retrieves the initial state
        of the fan.

        Args:
            endpoint (int): The endpoint identifier for the fan device.
            attr_to_update (attribute): The attribute to be updated (PercentSetting or FanMode).
            order (OrderEnum): The order of updates (ascending or descending).

        Returns:
            tuple: A tuple containing:
                - attr_to_verify (attribute): The attribute to verify during updates.
                - iteration_range (range): The range of iterations for attribute updates.
                - init_fan_mode (int): The initial FanMode setting.
                - init_percent_setting (int): The initial PercentSetting value.
                - init_speed_setting (int or None): The initial SpeedSetting value if present, otherwise None.
        """
        cluster = Clusters.FanControl
        attribute = cluster.Attributes
        percent_setting_max_value = 100  # PercentSetting max value is 100 as per the spec

        # Sets the number of iterations for attribute updates
        # Sets the update order (ascending or descending)
        if attr_to_update == attribute.PercentSetting:
            attr_to_verify = attribute.FanMode
            iteration_range = range(0, percent_setting_max_value +
                                    1) if order == OrderEnum.Ascending else range(percent_setting_max_value, -1, -1)
        elif attr_to_update == attribute.FanMode:
            attr_to_verify = attribute.PercentSetting
            iteration_range = range(0, len(self.fan_modes)) if order == OrderEnum.Ascending else range(
                len(self.fan_modes) - 1, -1, -1)

        # Get fan initial state (FanMode, PercentSetting, SpeedSetting if present)
        init_fan_mode = await self.read_setting(endpoint, attribute.FanMode)
        init_percent_setting = await self.read_setting(endpoint, attribute.PercentSetting)
        init_speed_setting = await self.read_setting(endpoint, attribute.SpeedSetting) if self.supports_multispeed else None

        # Determine the initial values of the attributes to update and verify
        init_attr_to_update_value = init_percent_setting if attr_to_update == attribute.PercentSetting else init_fan_mode
        init_attr_to_verify_value = init_fan_mode if attr_to_verify == attribute.FanMode else init_percent_setting

        return attr_to_verify, iteration_range, init_fan_mode, init_percent_setting, init_speed_setting, init_attr_to_update_value, init_attr_to_verify_value

    @staticmethod
    def handle_iteration_one_edge_cases(attr_to_update, attr_to_verify, order, attr_to_verify_value_current, init_fan_mode, init_percent_setting, init_speed_setting) -> None:
        """
        Handles edge cases for the first iteration of fan control attribute updates and verifications.

        Parameters:
        attr_to_update (Attribute): The attribute that is being updated.
        attr_to_verify (Attribute): The attribute that is being verified.
        order (OrderEnum): The order of the update (Ascending or Descending).
        attr_to_verify_value_current (int/Enum): The current value of the attribute to verify.
        init_fan_mode (FanModeEnum): The initial fan mode.
        init_percent_setting (int): The initial percent setting.
        init_speed_setting (int): The initial speed setting.

        Returns:
        None

        Logs the changes in attribute values and asserts the correctness of the updated values based on the initial fan mode and the order of update.
        """
        cluster = Clusters.FanControl
        attribute = cluster.Attributes
        fm_enum = cluster.Enums.FanModeEnum

        # Determine initial value of attribute to verify
        if attr_to_verify == attribute.PercentSetting:
            init_setting = init_percent_setting
        elif attr_to_verify == attribute.SpeedSetting:
            init_setting = init_speed_setting
        elif attr_to_verify == attribute.FanMode:
            init_setting = init_fan_mode

        # Logging attribute value change details
        sub_text = f"({init_setting}) to ({attr_to_verify_value_current})"
        if attr_to_verify == attribute.FanMode:
            sub_text = f"({init_setting}:{init_setting.name}) to ({attr_to_verify_value_current}:{attr_to_verify_value_current.name})"
        logging.info(f"\t\t[FC] {attr_to_verify.__name__} changed from {sub_text}")

        # The fan's general state is Off
        if init_fan_mode == fm_enum.kOff:
            if attr_to_update == attribute.PercentSetting:
                if order == OrderEnum.Ascending:
                    asserts.assert_equal(attr_to_verify_value_current, init_setting,
                                         f"[FC] Current {attr_to_verify.__name__} attribute value must be equal to the previous value")
                elif order == OrderEnum.Descending:
                    asserts.assert_greater(attr_to_verify_value_current, init_setting,
                                           f"[FC] Current {attr_to_verify.__name__} attribute value must be greater than the previous value")
            if attr_to_update == attribute.FanMode:
                if order == OrderEnum.Ascending:
                    asserts.assert_equal(attr_to_verify_value_current, init_setting,
                                         f"[FC] Current {attr_to_verify.__name__} attribute value must be equal to the previous value")
                elif order == OrderEnum.Descending:
                    asserts.assert_greater(attr_to_verify_value_current, init_setting,
                                           f"[FC] Current {attr_to_verify.__name__} attribute value must be greater than the previous value")

        # The fan's general state is High
        elif init_fan_mode == fm_enum.kHigh:
            if attr_to_update == attribute.PercentSetting:
                if order == OrderEnum.Ascending:
                    asserts.assert_less(attr_to_verify_value_current, init_setting,
                                        f"[FC] Current {attr_to_verify.__name__} attribute value must be less than the previous value")
                elif order == OrderEnum.Descending:
                    asserts.assert_greater_equal(attr_to_verify_value_current, init_setting,
                                                 f"[FC] Current {attr_to_verify.__name__} attribute value must be greater or equal to the previous value")
            if attr_to_update == attribute.FanMode:
                # init_setting = init_speed_setting if attr_to_verify == attribute.SpeedSetting else init_percent_setting
                if order == OrderEnum.Ascending:
                    asserts.assert_less(attr_to_verify_value_current, init_setting,
                                        f"[FC] Current {attr_to_verify.__name__} attribute value must be less than the previous value")
                elif order == OrderEnum.Descending:
                    asserts.assert_equal(attr_to_verify_value_current, init_setting,
                                         f"[FC] Current {attr_to_verify.__name__} attribute value must be equal to the previous value")

        # The fan's general state is in between Off and High
        else:
            if attr_to_update == attribute.PercentSetting:
                if order == OrderEnum.Ascending:
                    asserts.assert_less(attr_to_verify_value_current, init_setting,
                                        f"[FC] Current {attr_to_verify.__name__} attribute value must be less than the previous value")
                elif order == OrderEnum.Descending:
                    asserts.assert_greater(attr_to_verify_value_current, init_setting,
                                           f"[FC] Current {attr_to_verify.__name__} attribute value must be greater than the previous value")
            if attr_to_update == attribute.FanMode:
                if order == OrderEnum.Ascending:
                    asserts.assert_less(attr_to_verify_value_current, init_setting,
                                        f"[FC] Current {attr_to_verify.__name__} attribute value must be less than the previous value")
                elif order == OrderEnum.Descending:
                    asserts.assert_greater(attr_to_verify_value_current, init_setting,
                                           f"[FC] Current {attr_to_verify.__name__} attribute value must be greater than the previous value")

    @staticmethod
    def verify_attribute_change(attr_to_verify, value_current, value_previous, order) -> None:
        """
        Verifies that an attribute value has changed in the expected order and logs the change details.

        Args:
            attr_to_verify (Attribute): The attribute to verify.
            value_current (Any): The current value of the attribute.
            value_previous (Any): The previous value of the attribute.
            order (OrderEnum): The expected order of the attribute change (Ascending or Descending).

        Raises:
            AssertionError: If the current attribute value does not follow the expected order compared to the previous value.

        Logs:
            Logs the attribute value change details
        """
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

    async def update_and_verify_attribute_progression(self, endpoint, attr_to_update, order) -> None:
        """
        Sets up the testing conditions for a given testing scenario.

        This method sets up the testing conditions for the different attribute
        update and verification scenarios. It retrieves the initial parameters
        for the fan control attributes and then proceeds to update the specified
        attribute in the given order. The method then monitors the corresponding
        attributes to ensure they change correctly.

        Args:
            endpoint (int): The endpoint identifier for the fan device.
            attr_to_update (Any): The attribute to be updated during the test.
            order (OrderEnum): The order in which the attribute should be updated (ascending or descending).

        Returns:
            None

        Raises:
            AssertionError: If the attribute values do not progress as expected.

        Test scenarios:
            - Updating the PercentSetting attribute in ascending order and monitoring the PercentSetting, FanMode, (and SpeedSetting, if present) attributes.
            - Updating the PercentSetting attribute in descending order and monitoring the PercentSetting, FanMode, (and SpeedSetting, if present) attributes.
            - Updating the FanMode attribute in ascending order and monitoring the FanMode, PercentSetting, (and SpeedSetting, if present) attributes.
            - Updating the FanMode attribute in descending order and monitoring the FanMode, PercentSetting, (and SpeedSetting, if present) attributes.
        """
        cluster = Clusters.FanControl
        attribute = cluster.Attributes

        # Get initial parameters
        attr_to_verify, iteration_range, init_fan_mode, init_percent_setting, init_speed_setting, init_attr_to_update_value, init_attr_to_verify_value = \
            await self.get_initial_parametes(endpoint, attr_to_update, order)

        # *** NEXT STEP ***
        # TH performs one of the testing scenarios previously defined
        self.step(self.current_step_index + 1)

        # Logging the scenario being tested
        await self.log_scenario(attr_to_update, attr_to_verify, order, init_fan_mode, init_percent_setting, init_speed_setting)

        # Flush the queue before starting the test
        self.attribute_subscription.flush_reports()

        # Update attribute iteratively
        attr_to_update_value_current = None
        attr_to_update_value_previous = None
        attr_to_verify_value_current = None
        attr_to_verify_value_previous = None
        speed_setting_current = None
        speed_setting_previous = None
        iteration = 0
        for value_to_write in iteration_range:
            iteration += 1

            # Write to attribute
            write_status = await self.write_and_verify_attribute(endpoint, attr_to_update, value_to_write)
            logging.info(f"[FC] {attr_to_update.__name__} written: {value_to_write}")

            # - If the write status is SUCCESS, it means the write operation
            #   occurred and it triggered an update in the corresponding attribute(s)
            # - Verify the correct progression of the attribute(s) value(s)
            if write_status == Status.Success:
                # queue = self.attribute_subscription.attribute_queue.queue

                # Attribute to update check
                attr_to_update_value_current, attr_to_update_value_previous = await self.verify_attribute_success(
                    attr_to_update, attr_to_update, order, init_fan_mode,
                    init_percent_setting, init_speed_setting, init_attr_to_update_value,
                    attr_to_update_value_current, attr_to_update_value_previous, iteration)

                # Attribute to verify check
                attr_to_verify_value_current, attr_to_verify_value_previous = await self.verify_attribute_success(
                    attr_to_update, attr_to_verify, order, init_fan_mode,
                    init_percent_setting, init_speed_setting, init_attr_to_verify_value,
                    attr_to_verify_value_current, attr_to_verify_value_previous, iteration)

                # SpeedSetting attribute check (if present)
                if self.supports_multispeed:
                    speed_setting_current, speed_setting_previous = await self.verify_attribute_success(
                        attr_to_update, attribute.SpeedSetting, order, init_fan_mode,
                        init_percent_setting, init_speed_setting, init_speed_setting,
                        speed_setting_current, speed_setting_previous, iteration)

            # If the write status is INVALID_IN_STATE, it means no write operation occurred
            # Verify that the current attribute value is equal to the previous value
            elif write_status == Status.InvalidInState:
                # Attribute to update check
                await self.verify_attribute_invalid_in_state(attr_to_update, init_attr_to_update_value, attr_to_update_value_current, attr_to_update_value_previous, iteration)

                # Attribute to verify check
                await self.verify_attribute_invalid_in_state(attr_to_verify, init_attr_to_verify_value, attr_to_verify_value_current, attr_to_verify_value_previous, iteration)

                # SpeedSetting attribute check (if present)
                if self.supports_multispeed:
                    await self.verify_attribute_invalid_in_state(attribute.SpeedSetting, init_speed_setting, speed_setting_current, speed_setting_previous, iteration)

        logging.info("[FC] *** Attribute(s) update and verification successful")

    async def verify_attribute_invalid_in_state(self, attr_to_verify, init_attr_to_verify_value, attr_to_verify_value_current, attr_to_verify_value_previous, iteration) -> None:
        """
        This method checks if the current value of the attribute-to-verify is equal to its previous value.
        This is done when the write operation returns an INVALID_IN_STATE status.

        Args:
            attr_to_verify (str): The attribute to verify.
            init_attr_to_verify_value (Any): The initial value of the attribute to verify.
            attr_to_verify_value_current (Any): The current value of the attribute to verify.
            attr_to_verify_value_previous (Any): The previous value of the attribute to verify.
            iteration (int): The current iteration number.

        Raises:
            AssertionError: If the current attribute value is not equal to the previous value.
        """
        # Get current attribute-to-update value and verify it's equal to the previous value
        attr_to_verify_value_previous = init_attr_to_verify_value if iteration == 1 else attr_to_verify_value_previous
        asserts.assert_equal(attr_to_verify_value_current, attr_to_verify_value_previous,
                             f"[FC] Current {attr_to_verify.__name__} attribute value ({attr_to_verify_value_current}) must be equal to the previous value ({attr_to_verify_value_previous})")

    async def verify_attribute_success(self, attr_to_update, attr_to_verify, order, init_fan_mode, init_percent_setting, init_speed_setting, init_attr_to_verify_value, attr_to_verify_value_current, attr_to_verify_value_previous, iteration) -> tuple[Any, Any]:
        """
        Verifies the correct progression of the current and previous values of the attribute to verify.

        Args:
            attr_to_update (str): The attribute that was updated.
            attr_to_verify (str): The attribute whose value needs to be verified.
            order (str): The order in which the attributes are processed.
            init_fan_mode (int): The initial fan mode setting.
            init_percent_setting (int): The initial percentage setting of the fan.
            init_speed_setting (int): The initial speed setting of the fan.
            init_attr_to_verify_value (Any): The initial value of the attribute to verify.
            attr_to_verify_value_current (Any): The current value of the attribute to verify.
            attr_to_verify_value_previous (Any): The previous value of the attribute to verify.
            iteration (int): The current iteration number.

        Returns:
            tuple: A tuple containing the current and previous values of the attribute to verify.
        """
        # Get the current attribute-to-update value from the attribute report queue
        attr_to_verify_value_current = await self.get_attribute_value_from_queue(attr_to_verify)

        # # Since this was a successful write operation, and the current and previous
        # # value are the same, it means that no attribute-to-verify value change was
        # # triggered, setting it to None to skip the verification step
        # if attr_to_verify_value_current == attr_to_verify_value_previous:
        #     attr_to_verify_value_current = None

        if attr_to_verify_value_current is not None:
            # Handle iteration 1 edge cases where:
            #   - The fan can be in any initial state
            #   - The attribute's previous-value becomes
            #     the attributes initial-state value
            #     (as opposed to the value written in the preceding iteration)
            if iteration == 1:
                self.handle_iteration_one_edge_cases(
                    attr_to_update, attr_to_verify, order, attr_to_verify_value_current, init_fan_mode, init_percent_setting, init_speed_setting)
            else:
                # The attribute's previous-value may be None in some cases, defaulting to the attribute's initial state value if so
                attr_to_verify_value_previous = init_attr_to_verify_value if attr_to_verify_value_previous is None else attr_to_verify_value_previous

                # Verify attribute change
                self.verify_attribute_change(attr_to_verify, attr_to_verify_value_current, attr_to_verify_value_previous, order)

            attr_to_verify_value_previous = attr_to_verify_value_current
        else:
            attr_to_verify_value_current = attr_to_verify_value_previous

        return attr_to_verify_value_current, attr_to_verify_value_previous

    async def log_scenario(self, attr_to_update, attr_to_verify, order, init_fan_mode, init_percent_setting, init_speed_setting) -> None:
        """
        Logs the scenario being tested, including the initial state of the fan and supported fan modes.

        Args:
            attr_to_update (Enum): The attribute to be updated.
            attr_to_verify (Enum): The attribute to be verified.
            order (Enum): The order of the update.
            init_fan_mode (Enum): The initial fan mode.
            init_percent_setting (int): The initial percent setting of the fan.
            init_speed_setting (int): The initial speed setting of the fan (if supported).

        Returns:
            None
        """
        # Logging the scenario being tested
        logging.info("[FC] ====================================================================")
        speed_setting_scenario = " and SpeedSetting" if self.supports_multispeed else ""
        logging.info(
            f"[FC] *** Update {attr_to_update.__name__} {order.name}, verify {attr_to_verify.__name__}{speed_setting_scenario}")

        # Logging fan initial state (FanMode, PercentSetting, SpeedSetting if present)
        speed_setting_log = f", SpeedSetting({init_speed_setting})" if self.supports_multispeed else ""
        logging.info(
            f"[FC] *** Fan initial state: FanMode ({init_fan_mode}:{init_fan_mode.name}), PercentSetting ({init_percent_setting}){speed_setting_log}")

        # Logging supported fan modes
        logging.info(f"[FC] *** Supported fan modes: {self.fan_mode_sequence.name}")

    def pics_TC_FAN_3_1(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_1(self):
        # Setup
        ep = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attributes = cluster.Attributes
        self.timeout_sec = 0.01  # Timeout given for item retreival from the attribute queue

        # *** STEP 1 ***
        # Commissioning already done
        self.step(1)

        # *** STEP 2 ***
        # TH reads the FanModeSequence attribute from the DUT
        self.step(2)
        await self.get_fan_modes(ep, max_high=True)

        # *** STEP 3 ***
        # TH checks the DUT for support of the MultiSpeed feature
        self.step(3)
        feature_map = await self.read_setting(ep, attributes.FeatureMap)
        self.supports_multispeed = bool(feature_map & cluster.Bitmaps.Feature.kMultiSpeed)

        # *** STEP 4 ***
        # TH subscribes to the DUT's FanControl cluster
        self.step(4)
        self.attribute_subscription = ClusterAttributeChangeAccumulator(cluster)
        await self.attribute_subscription.start(self.default_controller, self.dut_node_id, ep)

        # *** STEP 5 ***
        # TH tests the following scenario:
        #   - Attribute to update: PercentSetting
        #   - Attribute to verify: PercentSetting, FanMode and SpeedSetting (if present)
        #   - Update order: Ascending
        await self.update_and_verify_attribute_progression(ep, attributes.PercentSetting, OrderEnum.Ascending)

        # *** STEP 6 ***
        # TH tests the following scenario:
        #   - Attribute to update: PercentSetting
        #   - Attribute to verify: PercentSetting, FanMode and SpeedSetting (if present)
        #   - Update order: Descending
        await self.update_and_verify_attribute_progression(ep, attributes.PercentSetting, OrderEnum.Descending)

        # *** STEP 7 ***
        # TH tests the following scenario:
        #   - Attribute to update: FanMode
        #   - Attribute to verify: FanMode, PercentSetting and SpeedSetting (if present)
        #   - Update order: Ascending
        await self.update_and_verify_attribute_progression(ep, attributes.FanMode, OrderEnum.Ascending)

        # *** STEP 8 ***
        # TH tests the following scenario:
        #   - Attribute to update: FanMode
        #   - Attribute to verify: FanMode, PercentSetting and SpeedSetting (if present)
        #   - Update order: Descending
        await self.update_and_verify_attribute_progression(ep, attributes.FanMode, OrderEnum.Descending)


if __name__ == "__main__":
    default_matter_test_main()
