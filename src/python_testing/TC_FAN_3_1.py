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
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
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
                TestStep(4, "[FC] TH tests the following scenario: - Attribute to update: PercentSetting - Attribute to verify: PercentSetting, FanMode and SpeedSetting (if present) - Update order: Ascending. Actions: * Initialize the DUT to `FanMode` Off and read back the value to verify written value. * Individually subscribe to the `PercentSetting`, `FanMode`, and `SpeedSetting` (if supported) attributes * Update the value of the `PercentSetting` attribute iteratively, in ascending order, from 1 to 100.",
                         "For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. After all updates have been performed, verify that the value of the attribute reports from the subscription of each attribute came in sequencially in ascending order (each new value greater than the previous one)."),
                TestStep(5, "[FC] TH tests the following scenario: - Attribute to update: PercentSetting - Attribute to verify: PercentSetting, FanMode and SpeedSetting (if present) - Update order: Descending. Actions: * Initialize the DUT to `FanMode` High and read back the value to verify written value. * Individually subscribe to the `PercentSetting`, `FanMode`, and `SpeedSetting` (if supported) attributes * Update the value of the `PercentSetting` attribute iteratively, in descending order, from 99 to 0.",
                         "For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. After all updates have been performed, verify that the value of the attribute reports from the subscription of each attribute came in sequencially in descending order (each new value less than the previous one)."),
                TestStep(6, "[FC] TH tests the following scenario: - Attribute to update: FanMode - Attribute to verify: FanMode, PercentSetting and SpeedSetting (if present) - Update order: Ascending. Actions: * Initialize the DUT to `FanMode` Off and read back the value to verify written value. * Individually subscribe to the `PercentSetting`, `FanMode`, and `SpeedSetting` (if supported) attributes * Update the value of the `FanMode` attribute iteratively, in ascending order, from 0 (Off) to the number of available fan modes specified by the `FanModeSequence` attribute, excluding modes beyond 3 (High).",
                         "For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. After all updates have been performed, verify that the value of the attribute reports from the subscription of each attribute came in sequencially in ascending order (each new value greater than the previous one). Verify that the number of FanMode reports matches the number of PercentSetting reports"),
                TestStep(7, "[FC] TH tests the following scenario: - Attribute to update: FanMode - Attribute to verify: FanMode, PercentSetting and SpeedSetting (if present) - Update order: Descending. Actions: * Initialize the DUT to `FanMode` High and read back the value to verify written value. * Individually subscribe to the `PercentSetting`, `FanMode`, and `SpeedSetting` (if supported) attributes * Update the value of the `FanMode` attribute iteratively, in ascending order, from the number of available fan modes specified by the `FanModeSequence` attribute, excluding modes beyond 3 (High), to 0 (Off).",
                         "For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. After all updates have been performed, verify that the value of the attribute reports from the subscription of each attribute came in sequencially in descending order (each new value less than the previous one). Verify that the number of FanMode reports matches the number of PercentSetting reports"),
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

    def get_value_range(self, attr_to_update, order) -> range:
        # Setup
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        percent_setting_max_value = 100
        fan_modes_qty = len(self.fan_modes)

        # Compute value range tro write
        if attr_to_update == attr.PercentSetting:
            value_range = range(1, percent_setting_max_value +
                                1) if order == OrderEnum.Ascending else range(percent_setting_max_value - 1, -1, -1)
        elif attr_to_update == attr.FanMode:
            value_range = range(1, fan_modes_qty) if order == OrderEnum.Ascending else range(
                fan_modes_qty - 2, -1, -1)

        return value_range

    def log_scenario(self, attr_to_update, value_range, order) -> None:
        # Logging support info
        logging.info("[FC] ====================================================================")
        logging.info(f"[FC] *** Supported fan modes: {self.fan_mode_sequence.name}")
        logging.info(f"[FC] *** MultiSpeed feature supported: {self.supports_multispeed}")

        # Logging initial FanMode state
        init_fan_mode = "Off" if order == OrderEnum.Ascending else "High"
        logging.info(f"[FC] *** Initial FanMode: {init_fan_mode}")

        # Logging the scenario being tested
        attr_to_verify = "FanMode" if attr_to_update == Clusters.FanControl.Attributes.PercentSetting else "PercentSetting, PercentCurrent"
        speed_setting_scenario = ", SpeedSetting, and SpeedCurrent" if self.supports_multispeed else ""
        logging.info(f"[FC] *** Update {attr_to_update.__name__} {order.name}, verify {attr_to_verify}{speed_setting_scenario}")
        logging.info(f"[FC] *** Value range to update: {value_range[0]} - {value_range[-1]}")
        logging.info("[FC]")

    async def subscribe_to_attributes(self) -> None:
        cluster = Clusters.FanControl
        attr = cluster.Attributes

        self.subscriptions = [
            ClusterAttributeChangeAccumulator(cluster, attr.PercentSetting),
            ClusterAttributeChangeAccumulator(cluster, attr.PercentCurrent),
            ClusterAttributeChangeAccumulator(cluster, attr.FanMode)
        ]

        if self.supports_multispeed:
            self.subscriptions.extend([
                ClusterAttributeChangeAccumulator(cluster, attr.SpeedSetting),
                ClusterAttributeChangeAccumulator(cluster, attr.SpeedCurrent)
            ])

        for sub in self.subscriptions:
            await sub.start(self.default_controller, self.dut_node_id, self.endpoint)

    def log_results(self) -> None:
        for sub in self.subscriptions:
            logging.info(f"[FC] - {sub._expected_attribute.__name__} Sub -")
            for q in sub.attribute_queue.queue:
                logging.info(f"[FC] {q.attribute.__name__}: {q.value}")
            logging.info("[FC]")

    def verify_attribute_progression(self, order) -> None:
        # Setup
        comp = operator.le if order == OrderEnum.Ascending else operator.ge
        comp_str = "greater" if order == OrderEnum.Ascending else "less"
        shared_str = f"not all attribute values progressed in {order.name.lower()} order (current value {comp_str} than previous value)."

        for sub in self.subscriptions:
            values = [q.value for q in sub.attribute_queue.queue]
            correct_progression = all(comp(a, b) for a, b in zip(values, values[1:]))
            asserts.assert_true(correct_progression, f"[FC] {sub._expected_attribute.__name__}: {shared_str}")

    def verify_number_of_fan_mode_reports(self) -> None:
        fan_mode_report_qty = 0
        percent_setting_report_qty = 0

        for sub in self.subscriptions:
            if sub._expected_attribute == Clusters.FanControl.Attributes.FanMode:
                fan_mode_report_qty = len(sub.attribute_queue.queue)
            if sub._expected_attribute == Clusters.FanControl.Attributes.PercentSetting:
                percent_setting_report_qty = len(sub.attribute_queue.queue)

        asserts.assert_equal(fan_mode_report_qty, percent_setting_report_qty,
                             "[FC] Number of FanMode reports doesn't match the number of PercentSetting reports")

    async def testing_scenario(self, attr_to_update, order) -> None:
        # Setup
        cluster = Clusters.FanControl
        attr = cluster.Attributes
        fm_enum = cluster.Enums.FanModeEnum

        # *** NEXT STEP ***
        # TH performs the requested testing scenario (attr_to_update, order)
        self.step(self.current_step_index + 1)

        # Initialize FanMode to Off or High based on the order
        init_fan_mode = fm_enum.kOff if order == OrderEnum.Ascending else fm_enum.kHigh
        await self.write_and_verify_attribute(attr.FanMode, init_fan_mode)

        # Subscribe to the PercentSetting, PercentCurrent, FanMode, and if supported, SpeedSetting and SpeedCurrent attributes
        await self.subscribe_to_attributes()

        # Get the range of values to write
        value_range = self.get_value_range(attr_to_update, order)

        # Logging the scenario being tested
        self.log_scenario(attr_to_update, value_range, order)

        # Write value to attribute and read back to verify the result
        for value_to_write in value_range:
            await self.write_and_verify_attribute(attr_to_update, value_to_write)

        # Log results of attribute reports per subscription
        self.log_results()

        # Veirfy attribute progression
        self.verify_attribute_progression(order)

        # When updating FanMode, verify that the number of FanMode
        # reports matches the number of PercentSetting report
        if attr_to_update == attr.FanMode:
            self.verify_number_of_fan_mode_reports()

        # Cancel subscriptions
        for sub in self.subscriptions:
            sub.cancel()

    def pics_TC_FAN_3_1(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_1(self):
        # Setup
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attr = cluster.Attributes

        # *** STEP 1 ***
        # Commissioning already done
        self.step(1)

        # *** STEP 2 ***
        # TH reads the FanModeSequence attribute from the DUT
        self.step(2)
        await self.get_fan_modes(remove_auto=True)

        # *** STEP 3 ***
        # TH checks the DUT for support of the MultiSpeed feature
        self.step(3)
        feature_map = await self.read_setting(attr.FeatureMap)
        self.supports_multispeed = bool(feature_map & cluster.Bitmaps.Feature.kMultiSpeed)

        # *** NEXT STEPS ***
        # TH tests the following scenarios and verifies the correct progression of the PercentSetting,
        # PercentCurrent, FanMode, and if supported, SpeedSetting and SpeedCurrent attributes
        await self.testing_scenario(attr_to_update=attr.PercentSetting, order=OrderEnum.Ascending)
        await self.testing_scenario(attr_to_update=attr.PercentSetting, order=OrderEnum.Descending)
        await self.testing_scenario(attr_to_update=attr.FanMode, order=OrderEnum.Ascending)
        await self.testing_scenario(attr_to_update=attr.FanMode, order=OrderEnum.Descending)


if __name__ == "__main__":
    default_matter_test_main()
