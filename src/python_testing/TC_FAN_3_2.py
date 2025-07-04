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
#     app-args: --discriminator 1234 --KVS kvs1 --trace_file json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --timeout 600
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import operator
from enum import Enum
from typing import Any

import chip.clusters as Clusters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.interaction_model import Status
from chip.testing.event_attribute_reporting import ClusterAttributeChangeAccumulator
from chip.testing.matter_asserts import assert_valid_uint8
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts


class OrderEnum(Enum):
    Ascending = 1
    Descending = 2


logger = logging.getLogger(__name__)


class TC_FAN_3_2(MatterBaseTest):
    def desc_TC_FAN_3_2(self) -> str:
        return "[TC-FAN-3.2] Optional speed functionality with DUT as Server"

    def steps_TC_FAN_3_2(self):
        return [TestStep(1, "[FC] Commissioning already done.", is_commissioning=True),
                TestStep(2, "[FC] TH reads the SpeedMax attribute from the DUT. This attribute specifies the the maximum value for SpeedSetting.",
                         "[FC] Verify that the DUT response contains a uint8 value no greater than 100 and store."),
                TestStep(3, "[FC] TH reads the FanModeSequence attribute from the DUT. This attribute specifies the available fan modes.",
                         "Verify that the DUT response contains a FanModeSequenceEnum and store."),
                TestStep(4, "[FC] Initialize the DUT to `FanMode` Off.",
                         "[FC] * Read back and verify the written value. * The DUT shall return either a SUCCESS or an INVALID_IN_STATE status code."),
                TestStep(5, "[FC] Individually subscribe to the PercentSetting, PercentCurrent, FanMode, SpeedSetting, and SpeedCurrent attributes.",
                         "[FC] This will receive updates for the attributes when the SpeedSetting attribute is updated."),
                TestStep(6, "[FC] Update the value of the `SpeedSetting` attribute iteratively, in ascending order, from 1 to SpeedMax.",
                         "[FC] For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. After all updates have been performed, verify: If no INVALID_IN_STATE write status was returned during the SpeedSetting updates: -- Verify that if the number of reports received for SpeedSetting is greater than or equal to the number of reports received for FanMode, then the number of reports received for FanMode should be equal to the number of available FanModes - 1 (since the first FanMode is Off due to initialization). -- Verify that the number of reports received for PercentSetting matches the number of reports received for SpeedSetting. * The value of the attribute reports from the subscription of each attribute came in sequencially in ascending order (each new value greater than the previous one)."),
                TestStep(7, "[FC] Initialize the DUT to `FanMode` High.",
                         "[FC] * Read back and verify the written value. * The DUT shall return either a SUCCESS or an INVALID_IN_STATE status code."),
                TestStep(8, "[FC] Individually subscribe to the PercentSetting, PercentCurrent, FanMode, SpeedSetting, and SpeedCurrent attributes.",
                         "[FC] This will receive updates for the attributes when the SpeedSetting attribute is updated."),
                TestStep(9, "[FC] Update the value of the `SpeedSetting` attribute iteratively, in descending order, from SpeedMax - 1 to 0.",
                         "[FC] For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. After all updates have been performed, verify: If no INVALID_IN_STATE write status was returned during the SpeedSetting updates: -- Verify that if the number of reports received for SpeedSetting is greater than or equal to the number of reports received for FanMode, then the number of reports received for FanMode should be equal to the number of available FanModes - 1 (since the first FanMode is High due to initialization). -- Verify that the number of reports received for PercentSetting matches the number of reports received for SpeedSetting. * The value of the attribute reports from the subscription of each attribute came in sequencially in descending order (each new value less than the previous one)."),
                ]

    async def read_setting(self, attribute: Any) -> Any:
        cluster = Clusters.Objects.FanControl
        return await self.read_single_attribute_check_success(endpoint=self.endpoint, cluster=cluster, attribute=attribute)

    async def write_and_verify_attribute(self, attribute, value) -> Status:
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(self.endpoint, attribute(value))])
        write_status = result[0].Status
        write_status_success = (write_status == Status.Success) or (write_status == Status.InvalidInState)
        asserts.assert_true(write_status_success,
                            f"[FC] {attribute.__name__} write did not return a result of either SUCCESS or INVALID_IN_STATE ({write_status.name})")

        if write_status == Status.Success:
            value_read = await self.read_setting(attribute)
            asserts.assert_equal(value_read, value,
                                 f"[FC] Mismatch between written and read attribute value ({attribute.__name__} - written: {value}, read: {value_read})")
        return write_status

    async def get_fan_modes(self, remove_auto: bool = False):
        # Read FanModeSequence attribute value
        fan_mode_sequence_attr = Clusters.FanControl.Attributes.FanModeSequence
        fm_enum = Clusters.FanControl.Enums.FanModeEnum
        fms_enum = Clusters.FanControl.Enums.FanModeSequenceEnum
        self.fan_mode_sequence = await self.read_setting(fan_mode_sequence_attr)

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

        self.fan_modes = [f for f in fan_modes if not (remove_auto and f == fm_enum.kAuto)]

    def log_scenario(self, value_range, order) -> None:
        # Logging support info
        logging.info("[FC] ====================================================================")

        # Logging initial FanMode state
        init_fan_mode = "Off" if order == OrderEnum.Ascending else "High"
        logging.info(f"[FC] *** Initial FanMode: {init_fan_mode}")

        # Logging the scenario being tested
        logging.info(
            f"[FC] *** Update SpeedSetting {order.name.lower()}, verify PercentSetting, PercentCurrent, FanMode, SpeedSetting, and SpeedCurrent")
        logging.info(f"[FC] *** Value range to update: {value_range[0]} - {value_range[-1]}")
        logging.info("[FC]")

    async def subscribe_to_attributes(self) -> None:
        cluster = Clusters.FanControl
        attr = cluster.Attributes

        self.subscriptions = [
            ClusterAttributeChangeAccumulator(cluster, attr.PercentSetting),
            ClusterAttributeChangeAccumulator(cluster, attr.PercentCurrent),
            ClusterAttributeChangeAccumulator(cluster, attr.FanMode),
            ClusterAttributeChangeAccumulator(cluster, attr.SpeedSetting),
            ClusterAttributeChangeAccumulator(cluster, attr.SpeedCurrent)
        ]

        for sub in self.subscriptions:
            await sub.start(self.default_controller, self.dut_node_id, self.endpoint)

    def log_results(self) -> None:
        for sub in self.subscriptions:
            logging.info(f"[FC] - {sub._expected_attribute.__name__} Sub -")
            for q in sub.attribute_queue.queue:
                logging.info(f"[FC] {q.attribute.__name__}: {q.value}")
            logging.info("[FC]")

    def verify_attribute_progression(self, order: OrderEnum, invalid_in_state_occurred: bool) -> None:
        # Setup
        comp = operator.le if order == OrderEnum.Ascending else operator.ge
        comp_str = "greater" if order == OrderEnum.Ascending else "less"
        shared_str = f"not all attribute values progressed in {order.name.lower()} order (current value {comp_str} than previous value)."

        # If no INVALID_IN_STATE write status was returned during the SpeedSetting updates:
        if not invalid_in_state_occurred:
            subs = {name: next(sub for sub in self.subscriptions if sub._expected_attribute.__name__ == name)
                    for name in ["SpeedSetting", "FanMode", "PercentSetting"]}
            speed_setting_sub = subs["SpeedSetting"]
            fan_mode_sub = subs["FanMode"]
            percent_setting_sub = subs["PercentSetting"]

            # Verify that if the number of reports received for SpeedSetting is greater than or equal to the
            # number of reports received for FanMode, then the number of reports received for FanMode should be
            # equal to the number of available FanModes - 1 (since the first FanMode is Off/High due to initialization)
            if len(speed_setting_sub.attribute_queue.queue) >= len(fan_mode_sub.attribute_queue.queue):
                expected_fan_mode_qty = len(self.fan_modes) - 1
                asserts.assert_equal(len(fan_mode_sub.attribute_queue.queue), expected_fan_mode_qty,
                                     f"[FC] FanMode attribute report count ({len(fan_mode_sub.attribute_queue.queue)}) does not match expected count ({expected_fan_mode_qty})")

            # Verify that the number of reports received for PercentSetting
            # matches the number of reports received for SpeedSetting
            asserts.assert_equal(len(percent_setting_sub.attribute_queue.queue), len(speed_setting_sub.attribute_queue.queue),
                                 f"[FC] PercentSetting attribute report count ({len(percent_setting_sub.attribute_queue.queue)}) does not match SpeedSetting attribute report count ({len(speed_setting_sub.attribute_queue.queue)})")

        # Verify the correct progression of attribute values
        for sub in self.subscriptions:
            values = [q.value for q in sub.attribute_queue.queue]
            correct_progression = all(comp(a, b) for a, b in zip(values, values[1:]))
            asserts.assert_true(correct_progression, f"[FC] {sub._expected_attribute.__name__}: {shared_str}")

        logging.info(
            f"[FC] All attribute values progressed as expected ({order.name.lower()} order - current value {comp_str} than previous value).")
        logging.info("[FC]")

    async def testing_scenario_update_speed_setting(self, order) -> None:
        # Setup
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        fm_enum = cluster.Enums.FanModeEnum
        invalid_in_state_occurred = False

        # *** NEXT STEP ***
        # Initialize FanMode to Off or High based on update order
        self.step(self.current_step_index + 1)
        init_fan_mode = fm_enum.kOff if order == OrderEnum.Ascending else fm_enum.kHigh
        await self.write_and_verify_attribute(attr.FanMode, init_fan_mode)

        # *** NEXT STEP ***
        # Individually subscribe to the PercentSetting, PercentCurrent, FanMode, SpeedSetting, and SpeedCurrent attributes
        self.step(self.current_step_index + 1)
        await self.subscribe_to_attributes()

        # Get the range of values to write
        value_range = range(1, self.speed_max + 1) if order == OrderEnum.Ascending else range(self.speed_max - 1, -1, -1)

        # Logging the scenario being tested
        self.log_scenario(value_range, order)

        # *** NEXT STEP ***
        # Update the value of the `SpeedSetting` attribute iteratively, in the specified order
        self.step(self.current_step_index + 1)
        for value_to_write in value_range:
            write_status = await self.write_and_verify_attribute(attr.SpeedSetting, value_to_write)
            if not invalid_in_state_occurred:
                if write_status == Status.InvalidInState:
                    invalid_in_state_occurred = True
                    logging.info(f"[FC] InvalidInState occurred for SpeedSetting attribute write ({value_to_write})")

        # Log results of attribute reports per subscription
        self.log_results()

        # After all updates have been performed, verify that the value of the
        # attribute reports from each subscription came in sequencially in the
        # specified order (each new value greater or less than the previous one)
        self.verify_attribute_progression(order, invalid_in_state_occurred)

        # Cancel subscriptions
        for sub in self.subscriptions:
            sub.cancel()

    def pics_TC_FAN_3_2(self) -> list[str]:
        return ["FAN.S.F00"]

    @run_if_endpoint_matches(has_feature(Clusters.FanControl, Clusters.FanControl.Bitmaps.Feature.kMultiSpeed))
    async def test_TC_FAN_3_2(self):
        # Setup
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attr = cluster.Attributes

        # *** STEP 1 ***
        # Commissioning already done
        self.step(1)

        # *** STEP 2 ***
        # TH reads the SpeedMax attribute from the DUT
        self.step(2)
        self.speed_max = await self.read_setting(attr.SpeedMax)
        assert_valid_uint8(self.speed_max, "SpeedMax")

        # *** STEP 3 ***
        # TH reads the FanModeSequence attribute from the DUT
        # to get the available fan modes
        self.step(3)
        await self.get_fan_modes(remove_auto=True)

        # *** NEXT STEPS ***
        # TH tests the following scenarios and verifies the correct progression of the PercentSetting,
        # PercentCurrent, FanMode, and if supported, SpeedSetting and SpeedCurrent attributes
        await self.testing_scenario_update_speed_setting(OrderEnum.Ascending)
        await self.testing_scenario_update_speed_setting(order=OrderEnum.Descending)


if __name__ == "__main__":
    default_matter_test_main()
