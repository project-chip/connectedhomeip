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
                TestStep(4, "[FC] TH tests the following scenario: - Attribute to update: PercentSetting - Attribute to verify: PercentSetting, FanMode and SpeedSetting (if present) - Update order: Ascending. Actions: * Initialize the DUT to `FanMode` Off and read back the value to verify written value. * Individually subscribe to the `PercentSetting`, `FanMode`, and `SpeedSetting` (if supported) attributes * Update the value of the `PercentSetting` attribute iteratively, in ascending order, from 1 to 100.",
                         "For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. After all updates have been performed, verify that the value of the attribute reports from the subscription of each attribute came in sequencially in ascending order (each new value greater than the previous one)."),
                TestStep(5, "[FC] TH tests the following scenario: - Attribute to update: PercentSetting - Attribute to verify: PercentSetting, FanMode and SpeedSetting (if present) - Update order: Descending. Actions: * Initialize the DUT to `FanMode` High and read back the value to verify written value. * Individually subscribe to the `PercentSetting`, `FanMode`, and `SpeedSetting` (if supported) attributes * Update the value of the `PercentSetting` attribute iteratively, in descending order, from 99 to 0.",
                         "For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. After all updates have been performed, verify that the value of the attribute reports from the subscription of each attribute came in sequencially in descending order (each new value less than the previous one)."),
                TestStep(6, "[FC] TH tests the following scenario: - Attribute to update: FanMode - Attribute to verify: FanMode, PercentSetting and SpeedSetting (if present) - Update order: Ascending. Actions: * Initialize the DUT to `FanMode` Off and read back the value to verify written value. * Individually subscribe to the `PercentSetting`, `FanMode`, and `SpeedSetting` (if supported) attributes * Update the value of the `FanMode` attribute iteratively, in ascending order, from 0 (Off) to the number of available fan modes specified by the `FanModeSequence` attribute, excluding modes beyond 3 (High).",
                         "For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. After all updates have been performed, verify that the value of the attribute reports from the subscription of each attribute came in sequencially in ascending order (each new value greater than the previous one)."),
                TestStep(7, "[FC] TH tests the following scenario: - Attribute to update: FanMode - Attribute to verify: FanMode, PercentSetting and SpeedSetting (if present) - Update order: Descending. Actions: * Initialize the DUT to `FanMode` High and read back the value to verify written value. * Individually subscribe to the `PercentSetting`, `FanMode`, and `SpeedSetting` (if supported) attributes * Update the value of the `FanMode` attribute iteratively, in ascending order, from the number of available fan modes specified by the `FanModeSequence` attribute, excluding modes beyond 3 (High), to 0 (Off).",
                         "For each update, the DUT shall return either a SUCCESS or an INVALID_IN_STATE status code. After all updates have been performed, verify that the value of the attribute reports from the subscription of each attribute came in sequencially in descending order (each new value less than the previous one)."),
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

    def get_range(self, attr_to_update, order) -> range:
        # Setup
        cluster = Clusters.FanControl
        attribute = cluster.Attributes
        percent_setting_max_value = 100
        fan_modes_qty = len(self.fan_modes)

        # Compute value range tro write
        if attr_to_update == attribute.PercentSetting:
            value_range = range(1, percent_setting_max_value +
                                1) if order == OrderEnum.Ascending else range(percent_setting_max_value - 1, -1, -1)
        elif attr_to_update == attribute.FanMode:
            value_range = range(1, fan_modes_qty) if order == OrderEnum.Ascending else range(
                fan_modes_qty - 2, -1, -1)

        return value_range

    def log_scenario(self, attr_to_update, value_range, order) -> None:
        # Logging supported fan modes
        logging.info("[FC] ====================================================================")
        logging.info(f"[FC] *** Supported fan modes: {self.fan_mode_sequence.name}")
        init_fan_mode = "Off" if order == OrderEnum.Ascending else "High"

        # Logging MultiSpeed feature support
        logging.info(f"[FC] *** MultiSpeed feature supported: {self.supports_multispeed}")

        # Logging fan initial state
        logging.info(f"[FC] *** Initial FanMode: {init_fan_mode}")

        # Logging the scenario being tested
        attr_to_verify = "FanMode" if attr_to_update == Clusters.FanControl.Attributes.PercentSetting else "PercentSetting"
        speed_setting_scenario = " and SpeedSetting" if self.supports_multispeed else ""
        logging.info(f"[FC] *** Update {attr_to_update.__name__} {order.name}, verify {attr_to_verify}{speed_setting_scenario}")

        # Logging the range of values to write
        logging.info(f"[FC] *** Value range to write: {value_range[0]} - {value_range[-1]}")

    def log_results(self) -> None:
        logging.info("[FC]")
        logging.info("[FC] - PercentSetting Sub -")
        for q in self.sub_percent_setting.attribute_queue.queue:
            logging.info(f"[FC] {q.attribute.__name__}: {q.value}")
        logging.info("[FC]")
        logging.info("[FC] - FanMode Sub -")
        for q in self.sub_fan_mode.attribute_queue.queue:
            logging.info(f"[FC] {q.attribute.__name__}: {q.value}")
        logging.info("[FC]")
        if self.supports_multispeed:
            logging.info("[FC] - SpeedSetting Sub -")
            for q in self.sub_speed_setting.attribute_queue.queue:
                logging.info(f"[FC] {q.attribute.__name__}: {q.value}")
            logging.info("[FC]")

    async def subscribe_to_attributes(self) -> None:
        # Setup
        cluster = Clusters.FanControl
        attributes = cluster.Attributes

        # Subscribe to PercentSetting attribute changes
        self.sub_percent_setting = ClusterAttributeChangeAccumulator(cluster, attributes.PercentSetting)
        await self.sub_percent_setting.start(self.default_controller, self.dut_node_id, self.endpoint)

        # Subscribe to FanMode attribute changes
        self.sub_fan_mode = ClusterAttributeChangeAccumulator(cluster, attributes.FanMode)
        await self.sub_fan_mode.start(self.default_controller, self.dut_node_id, self.endpoint)

        # Subscribe to SpeedSetting attribute changes if MultiSpeed feature is supported
        if self.supports_multispeed:
            self.sub_speed_setting = ClusterAttributeChangeAccumulator(cluster, attributes.SpeedSetting)
            await self.sub_speed_setting.start(self.default_controller, self.dut_node_id, self.endpoint)

    def verify_attribute_progression(self, order) -> None:
        # Setup
        comp = operator.le if order == OrderEnum.Ascending else operator.ge
        shared_str = f"not all attribute values progressed in {order.name.lower()} order."

        # Verify that the PercentSetting attribute values progressed in the expected order
        ps_values = [q.value for q in self.sub_percent_setting.attribute_queue.queue]
        ps_correct_progression = all(comp(a, b) for a, b in zip(ps_values, ps_values[1:]))
        asserts.assert_true(ps_correct_progression, f"[FC] PercentSetting: {shared_str}")

        # Verify that the FanMode attribute values progressed in the expected order
        fm_values = [q.value for q in self.sub_fan_mode.attribute_queue.queue]
        fm_correct_progression = all(comp(a, b) for a, b in zip(fm_values, fm_values[1:]))
        asserts.assert_true(fm_correct_progression, f"[FC] FanMode: {shared_str}")

        # Verify that the SpeedSetting attribute (if present) values progressed in the expected order
        if self.supports_multispeed:
            ss_values = [q.value for q in self.sub_speed_setting.attribute_queue.queue]
            ss_correct_progression = all(comp for a, b in zip(ss_values, ss_values[1:]))
            asserts.assert_true(ss_correct_progression, f"[FC] SpeedSetting: {shared_str}")

    async def testing_scenario(self, attr_to_update, order) -> None:
        # Setup
        cluster = Clusters.FanControl
        attributes = cluster.Attributes
        fm_enum = cluster.Enums.FanModeEnum

        # *** NEXT STEP ***
        # TH performs the requested testing scenario (attr_to_update, order)
        self.step(self.current_step_index + 1)

        # Initialize fan
        init_fan_mode = fm_enum.kOff if order == OrderEnum.Ascending else fm_enum.kHigh
        await self.write_and_verify_attribute(attributes.PercentSetting, init_fan_mode)

        # Subscribe to the PercentSetting, FanMode, and SpeedSetting (if supported) attributes
        await self.subscribe_to_attributes()

        # Get the range of values to write
        value_range = self.get_range(attr_to_update, order)

        # Logging the scenario being tested
        self.log_scenario(attr_to_update, value_range, order)

        # Write values to attribute and verify the result
        for value_to_write in value_range:
            await self.write_and_verify_attribute(attr_to_update, value_to_write)

        # Log results of attribute reports per subscription
        self.log_results()

        # Veirfy attribute progression
        self.verify_attribute_progression(order)

        # Cancel subscriptions
        self.sub_percent_setting.cancel()
        self.sub_fan_mode.cancel()
        if self.supports_multispeed:
            self.sub_speed_setting.cancel()

    def pics_TC_FAN_3_1(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_1(self):
        # Setup
        self.endpoint = self.get_endpoint(default=1)
        cluster = Clusters.FanControl
        attributes = cluster.Attributes

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
        feature_map = await self.read_setting(attributes.FeatureMap)
        self.supports_multispeed = bool(feature_map & cluster.Bitmaps.Feature.kMultiSpeed)

        # TH tests the following scenarios and verifies the correct progression of
        # the PercentSetting, FanMode, and SpeedSetting (if present) attributes
        await self.testing_scenario(attr_to_update=attributes.PercentSetting, order=OrderEnum.Ascending)
        await self.testing_scenario(attr_to_update=attributes.PercentSetting, order=OrderEnum.Descending)
        await self.testing_scenario(attr_to_update=attributes.FanMode, order=OrderEnum.Ascending)
        await self.testing_scenario(attr_to_update=attributes.FanMode, order=OrderEnum.Descending)


if __name__ == "__main__":
    default_matter_test_main()
