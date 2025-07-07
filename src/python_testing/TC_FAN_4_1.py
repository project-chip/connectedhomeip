#
#    Copyright (c) 2025 Project CHIP Authors
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
#       --int-arg pixit_fan_start_time:1
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import time
from typing import Optional

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.event_attribute_reporting import ClusterAttributeChangeAccumulator
from chip.testing.matter_testing import (AttributeValue, MatterBaseTest, TestStep, default_matter_test_main, has_cluster,
                                         run_if_endpoint_matches)
from mobly import asserts


class TC_FAN_4_1(MatterBaseTest):
    def desc_TC_FAN_4_1(self) -> str:
        return "[TC-FAN-4.1] Fan interaction with On/Off cluster"

    def get_fan_modes(self, fan_mode_sequence: Clusters.FanControl.Enums.FanModeSequenceEnum):
        # TODO: put this in a common location once all the fan stuff is landed
        # https://github.com/project-chip/matter-test-scripts/issues/550
        sequence = Clusters.FanControl.Enums.FanModeSequenceEnum
        mode = Clusters.FanControl.Enums.FanModeEnum

        if fan_mode_sequence == sequence.kOffLowMedHigh:
            return [mode.kOff, mode.kLow, mode.kMedium, mode.kHigh]
        if fan_mode_sequence == sequence.kOffLowHigh:
            return [mode.kOff, mode.kLow, mode.kHigh]
        if fan_mode_sequence == sequence.kOffLowMedHighAuto:
            return [mode.kOff, mode.kLow, mode.kMedium, mode.kHigh, mode.kAuto]
        if fan_mode_sequence == sequence.kOffLowHighAuto:
            return [mode.kOff, mode.kLow, mode.kHigh, mode.kAuto]
        if fan_mode_sequence == sequence.kOffHighAuto:
            return [mode.kOff, mode.kHigh, mode.kAuto]
        if fan_mode_sequence == sequence.kOffHigh:
            return [mode.kOff, mode.kHigh]

        asserts.fail(f"Unknown FanModeSequence {fan_mode_sequence}")

    def _sub_step(self, start_step: int, attribute_to_set: str, value: str, verify_mode: str, verify_percent: str, verify_speed: str, spd_check: bool = False):
        spd = ""
        if spd_check:
            spd = "If SPD is not supported, skip this step and the next 6 steps. "
        return [TestStep(start_step, f"{spd}Set the {attribute_to_set} to {value}. If the write returns INVALID_IN_STATE, skip the next six steps", "INVALID_IN_STATE or SUCCESS"),
                TestStep(start_step + 1, "Read the FanMode", f"Verify FanMode is set to {verify_mode}"),
                TestStep(start_step + 2, "Read PercentSetting", f"Verify PercentSetting is {verify_percent}"),
                TestStep(start_step + 3, "Wait for PIXIT.FanStartTime seconds"),
                TestStep(start_step + 4, "Read PercentCurrent", "PercentCurrent is 0"),
                TestStep(start_step + 5, "If SPD is supported, Read SpeedSetting", f"Verify SpeedSetting is {verify_speed}"),
                TestStep(start_step + 6, "If SPD is supported, Read SpeedCurrent", "SpeedCurrent is 0")
                ]

    def steps_TC_FAN_4_1(self):
        steps = [TestStep(1, "TH Commissions DUT.", is_commissioning=True),
                 TestStep(2, "TH subscribes to the Fan control cluster", "SUCCESS"),
                 TestStep(3, "TH reads the supported fan modes", "SUCCESS"),
                 TestStep(4, "If SPD is supported, TH reads SpeedMax", "SUCCESS"),

                 TestStep(5, "TH sends the On command to the On/Off cluster", "DUT returns SUCCESS"),
                 TestStep(6, "TH sets the FanMode attribute on the Fan Control cluster to High", "DUT returns SUCCESS"),

                 TestStep(7, "TH reads the PercentSetting attribute from the Fan Control cluster and saves as `percent_setting_original`", "SUCCESS"),
                 TestStep(8, "If the SPD feature is supported, TH reads the SpeedSetting attribute from the Fan Control cluster and saves as `speed_setting_original`", "SUCCESS"),
                 TestStep(9, "TH reads the PercentCurrent attribute from the Fan Control cluster", "SUCCESS"),
                 TestStep(10, "If PercentCurrent is not equal to PercentSetting, TH awaits an attribute report for the PercentCurrent matching PercentSetting", "Report is received"),

                 TestStep(11, "TH sends the Off command to the On/Off cluster", "DUT returns SUCCESS"),
                 TestStep(12, "TH awaits the following attribute reports (order does not matter): PercentCurrent is 0, SpeedCurrent is 0 (if SPD feature is supported)", "Report(s) are received"),
                 TestStep(13, "TH reads the FanMode attribute", "FanMode is set to HIGH"),
                 TestStep(14, "TH reads the PercentSetting attribute", "PercentSetting matches `percent_setting_original"),
                 TestStep(15, "If the SPD feature is supported, TH reads the SpeedSetting",
                          "SpeedSetting matches `speed_setting_original`"),
                 ]
        num = 16
        num_substeps = 7
        steps.extend(self._sub_step(num, attribute_to_set="PercentSetting", value="1",
                     verify_mode="lowest supported mode above Off", verify_percent="1", verify_speed="1"))
        num += num_substeps
        steps.extend(self._sub_step(num, attribute_to_set="PercentSetting", value="0",
                     verify_mode="Off", verify_percent="0", verify_speed="0"))
        num += num_substeps
        steps.extend(self._sub_step(num, attribute_to_set="FanMode", value="High",
                     verify_mode="High", verify_percent="not 0", verify_speed="not 0"))
        num += num_substeps
        steps.extend(self._sub_step(num, attribute_to_set="FanMode", value="Off",
                     verify_mode="Off", verify_percent="0", verify_speed="0"))
        num += num_substeps
        steps.extend(self._sub_step(num, attribute_to_set="SpeedSetting", value="SpeedMax",
                     verify_mode="High", verify_percent="not 0", verify_speed="SpeedMax", spd_check=True))
        num += num_substeps
        steps.extend(self._sub_step(num, attribute_to_set="SpeedSetting", value="0",
                     verify_mode="Off", verify_percent="0", verify_speed="0", spd_check=True))
        num += num_substeps
        steps.extend(self._sub_step(num, attribute_to_set="PercentSetting", value="100",
                     verify_mode="High", verify_percent="100", verify_speed="SpeedMax"))
        num += num_substeps

        steps.append(TestStep(
            num, "TH reads the PercentSetting and SpeedSetting (if supported) values and saves as `percent_setting_before_on` and `speed_setting_before_on`"))
        steps.append(TestStep(num + 1, "TH sends On command to the On/Off cluster", "DUT returns SUCCESS"))
        steps.append(TestStep(num + 2, "TH awaits the following attribute reports (order does not matter): PercentCurrent is `percent_setting_before_on`, SpeedCurrent is `speed_setting_before_on` (if SPD feature is supported)", "Reports are received"))
        steps.append(TestStep(num + 3, "TH sets PercentSetting to 50", "Response is SUCCESS or INVALID_IN_STATE"))
        steps.append(TestStep(num + 4, "If the response was SUCCESS, TH awaits the following attribute reports (order does not matter): PercentCurrent is 50, PercentSetting is 50", "Report(s) are received"))
        return steps

    def pics_TC_FAN_4_1(self) -> list[str]:
        return ["FAN.S", "OO.S"]

    @run_if_endpoint_matches(has_cluster(Clusters.FanControl) and has_cluster(Clusters.OnOff))
    async def test_TC_FAN_4_1(self):
        self.step(1)
        fan = Clusters.FanControl
        features = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.FeatureMap)
        has_spd = (features & fan.Bitmaps.Feature.kMultiSpeed) != 0
        # Wait for the entire duration of the test because this fan may be slow. The test will time out before this does. That's fine.
        timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout

        wait_s = self.user_params.get('pixit_fan_start_time', 5)

        self.step(2)
        sub = ClusterAttributeChangeAccumulator(fan)
        await sub.start(self.default_controller, node_id=self.dut_node_id, endpoint=self.get_endpoint())

        self.step(3)
        supported_fan_modes = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.FanModeSequence)

        self.step(4)
        if has_spd:
            speed_max = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.SpeedMax)
        else:
            speed_max = None
            self.mark_current_step_skipped()

        self.step(5)
        await self.send_single_cmd(cmd=Clusters.OnOff.Commands.On())

        self.step(6)
        attr = fan.Attributes.FanMode(fan.Enums.FanModeEnum.kHigh)
        resp = await self.default_controller.WriteAttribute(nodeid=self.dut_node_id, attributes=[(self.get_endpoint(), attr)])
        asserts.assert_equal(resp[0].Status, Status.Success, "Unexpected error writing FanMode attribute")

        self.step(7)
        percent_setting_original = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.PercentSetting)

        self.step(8)
        if await self.feature_guard(self.get_endpoint(), cluster=fan, feature_int=fan.Bitmaps.Feature.kMultiSpeed):
            speed_setting_original = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.SpeedSetting)

        self.step(9)
        percent_current = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.PercentCurrent)

        self.step(10)
        if percent_current != percent_setting_original:
            sub.await_all_final_values_reported(
                expected_final_values=[fan.Attributes.PercentCurrent(percent_setting_original)], timeout_sec=timeout)
        else:
            self.mark_current_step_skipped()

        self.step(11)
        await self.send_single_cmd(cmd=Clusters.OnOff.Commands.Off())

        self.step(12)
        awaiting = [AttributeValue(endpoint_id=self.get_endpoint(), attribute=fan.Attributes.PercentCurrent, value=0)]
        if has_spd:
            awaiting.append(AttributeValue(endpoint_id=self.get_endpoint(), attribute=fan.Attributes.SpeedCurrent, value=0))
        sub.await_all_final_values_reported(expected_final_values=awaiting, timeout_sec=timeout)
        sub.reset()

        self.step(13)
        mode = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.FanMode)
        asserts.assert_equal(mode, fan.Enums.FanModeEnum.kHigh, "FanMode was changed when on/off cluster was changed")

        self.step(14)
        percent_setting_new = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.PercentSetting)
        asserts.assert_equal(percent_setting_new, percent_setting_original,
                             "PercentSetting was changed when on/off cluster was changed")

        self.step(15)
        if await self.feature_guard(self.get_endpoint(), cluster=fan, feature_int=fan.Bitmaps.Feature.kMultiSpeed):
            speed_setting_new = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.SpeedSetting)
            asserts.assert_equal(speed_setting_new, speed_setting_original,
                                 "SpeedSetting was changed when on/off cluster was changed")
        step_num = 16
        num_substeps = 7

        async def verify_onoff_off(attr: Clusters.ClusterObjects.ClusterAttributeDescriptor, expected_mode: Clusters.FanControl.Enums.FanModeEnum, expected_percent_setting: Optional[int], expected_speed_setting: Optional[int]):
            """ Writes specified attribute and checks expected results for On/Off cluster in Off mode
                None on PercentSetting or SpeedSetting just verifies the values are not 0.
            """
            nonlocal step_num
            self.step(step_num)
            resp = await self.default_controller.WriteAttribute(nodeid=self.dut_node_id, attributes=[(self.get_endpoint(), attr)])
            asserts.assert_in(resp[0].Status, [Status.Success, Status.InvalidInState], "Unexpected status returned")
            if resp[0].Status != Status.Success:
                self.skip_step(step_num + 1)
                self.skip_step(step_num + 2)
                self.skip_step(step_num + 3)
                self.skip_step(step_num + 4)
                self.skip_step(step_num + 5)
                self.skip_step(step_num + 6)
                step_num += num_substeps
                return

            self.step(step_num + 1)
            fan_mode = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.FanMode)
            asserts.assert_equal(fan_mode, expected_mode, "Incorrect FanMode")

            self.step(step_num + 2)
            percent_setting = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.PercentSetting)
            if expected_percent_setting is not None:
                asserts.assert_equal(percent_setting, expected_percent_setting, "Incorrect percent setting")
            else:
                asserts.assert_not_equal(percent_setting, 0, "Incorrect percent setting")

            self.step(step_num + 3)
            logging.info(f"Waiting for {wait_s} seconds to give the fan a chance to respond")
            time.sleep(wait_s)

            self.step(step_num + 4)
            percent_current = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.PercentCurrent)
            asserts.assert_equal(percent_current, 0, "Fan turned on while On/Off cluster was set to off")

            if has_spd:
                self.step(step_num + 5)
                speed_setting = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.SpeedSetting)
                if expected_speed_setting is not None:
                    asserts.assert_equal(speed_setting, expected_speed_setting,
                                         "Speed was not adjusted when percent was adjusted while fan was off")
                else:
                    asserts.assert_not_equal(speed_setting, 0, "Incorrect speed setting")

                self.step(step_num + 6)
                speed_current = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.SpeedCurrent)
                asserts.assert_equal(speed_current, 0, "Current speed is not 0 even though the fan is off")
            else:
                self.skip_step(step_num + 5)
                self.skip_step(step_num + 6)
            step_num += num_substeps

        lowest_mode = self.get_fan_modes(supported_fan_modes)[1]
        await verify_onoff_off(attr=fan.Attributes.PercentSetting(1), expected_mode=lowest_mode, expected_percent_setting=1, expected_speed_setting=1)
        await verify_onoff_off(attr=fan.Attributes.PercentSetting(0), expected_mode=fan.Enums.FanModeEnum.kOff, expected_percent_setting=0, expected_speed_setting=0)
        await verify_onoff_off(attr=fan.Attributes.FanMode(fan.Enums.FanModeEnum.kHigh), expected_mode=fan.Enums.FanModeEnum.kHigh, expected_percent_setting=None, expected_speed_setting=None)
        await verify_onoff_off(attr=fan.Attributes.FanMode(fan.Enums.FanModeEnum.kOff), expected_mode=fan.Enums.FanModeEnum.kOff, expected_percent_setting=0, expected_speed_setting=0)
        if has_spd:
            await verify_onoff_off(attr=fan.Attributes.SpeedSetting(speed_max), expected_mode=fan.Enums.FanModeEnum.kHigh, expected_percent_setting=None, expected_speed_setting=speed_max)
            await verify_onoff_off(attr=fan.Attributes.SpeedSetting(0), expected_mode=fan.Enums.FanModeEnum.kOff, expected_percent_setting=0, expected_speed_setting=0)
        else:
            for i in range(2*num_substeps + 1):
                self.skip_step(step_num + i)
            step_num += 2 * num_substeps

        await verify_onoff_off(attr=fan.Attributes.PercentSetting(100), expected_mode=fan.Enums.FanModeEnum.kHigh, expected_percent_setting=100, expected_speed_setting=speed_max)

        self.step(step_num)
        percent_setting_before_on = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.PercentSetting)
        if has_spd:
            speed_setting_before_on = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.SpeedSetting)
        step_num += 1

        self.step(step_num)
        await self.send_single_cmd(cmd=Clusters.OnOff.Commands.On())
        step_num += 1

        self.step(step_num)
        awaiting = [AttributeValue(endpoint_id=self.get_endpoint(
        ), attribute=fan.Attributes.PercentCurrent, value=percent_setting_before_on)]
        if has_spd:
            awaiting.append(AttributeValue(endpoint_id=self.get_endpoint(),
                            attribute=fan.Attributes.SpeedCurrent, value=speed_setting_before_on))
        sub.await_all_final_values_reported(expected_final_values=awaiting, timeout_sec=timeout)
        sub.reset()
        step_num += 1

        # Make sure that we can still adjust the Percent values now that it's back on
        self.step(step_num)
        attr = fan.Attributes.PercentSetting(50)
        resp = await self.default_controller.WriteAttribute(nodeid=self.dut_node_id, attributes=[(self.get_endpoint(), attr)])
        asserts.assert_in(resp[0].Status, [Status.Success, Status.InvalidInState], "Invalid response from writing PercentSetting")
        step_num += 1

        # we want to see a change on percent setting and percent current to 1
        self.step(step_num)
        if resp[0].Status == Status.Success:
            sub.await_all_final_values_reported([AttributeValue(self.get_endpoint(), fan.Attributes.PercentSetting, 50), AttributeValue(
                self.get_endpoint(), fan.Attributes.PercentCurrent, 50)], timeout_sec=timeout)
        else:
            self.mark_current_step_skipped()


# TODO: need to add tests and probably handlers to check what happens when the step command is given when the on/off cluster is off
# Also what happens when the on/off cluster is turned off while the step is a-steppin'

if __name__ == "__main__":
    default_matter_test_main()
