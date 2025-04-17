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
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_infrastructure.chip.testing.matter_testing import (AttributeValue, ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep, default_matter_test_main,
                                                                       has_cluster, run_if_endpoint_matches)
from mobly import asserts


class TC_FAN_2_4(MatterBaseTest):
    def desc_TC_FAN_4_1(self) -> str:
        return "[TC-FAN-4.1] Fan interaction with On/Off cluster"

    def steps_TC_FAN_4_1(self):
        return [TestStep(1, "TH Commissions DUT.", is_commissioning=True),
                TestStep(2, "TH subscribes to the Fan control cluster"),

                TestStep(3, "TH sends the On command to the On/Off cluster", "DUT returns SUCCESS"),
                TestStep(4, "TH sets the FanMode attribute on the Fan Control cluster to High", "DUT returns SUCCESS"),

                TestStep(5, "TH reads the PercentSetting attribute from the Fan Control cluster and saves as `percent_setting_original`", "SUCCESS"),
                TestStep(6, "If the SPD feature is supported, TH reads the SpeedSetting attribute from the Fan Control cluster and saves as `speed_setting_original`", "SUCCESS"),
                TestStep(7, "TH reads the PercentCurrent attribute from the Fan Control cluster", "SUCCESS"),
                TestStep(8, "If PercentCurrent is not equal to PercentSetting, TH awaits an attribute report for the PercentCurrent matching PercentSetting", "Report is received"),

                TestStep(9, "TH sends the Off command to the On/Off cluster", "DUT returns SUCCESS"),
                TestStep(10, "TH awaits the following attribute reports (order does not matter): PercentCurrent is 0, SpeedCurrent is 0 (if SPD feature is supported)", "Report(s) are received"),
                TestStep(11, "TH reads the FanMode attribute", "FanMode is set to HIGH"),
                TestStep(12, "TH reads the PercentSetting attribute", "PercentSetting matches `percent_setting_original"),
                TestStep(13, "If the SPD feature is supported, TH reads the SpeedSetting",
                         "SpeedSetting matches `speed_setting_original`"),

                TestStep(14, "TH sends On command to the On/Off cluster", "DUT returns SUCCESS"),
                TestStep(15, "TH awaits the following attribute reports (order does not matter): PercentCurrent is `percent_setting_original`, SpeedCurrent is `speed_setting_original` (if SPD feature is supported)", "Reports are received"),

                ]

    def pics_TC_FAN_4_1(self) -> list[str]:
        return ["FAN.S", "OO.S"]

    @run_if_endpoint_matches(has_cluster(Clusters.FanControl) and has_cluster(Clusters.OnOff))
    async def test_TC_FAN_4_1(self):
        self.step(1)
        fan = Clusters.FanControl
        features = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.FeatureMap)
        has_spd = (features & fan.Bitmaps.Feature.kMultiSpeed) != 0
        # Wait for the entire duration of the test because this valve may be slow. The test will time out before this does. That's fine.
        timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout

        self.step(2)
        sub = ClusterAttributeChangeAccumulator(Clusters.FanControl)
        await sub.start(self.default_controller, node_id=self.dut_node_id, endpoint=self.get_endpoint())

        self.step(3)
        await self.send_single_cmd(cmd=Clusters.OnOff.Commands.On())

        self.step(4)
        attr = Clusters.FanControl.Attributes.FanMode(Clusters.FanControl.Enums.FanModeEnum.kHigh)
        resp = await self.default_controller.WriteAttribute(nodeid=self.dut_node_id, attributes=[(self.get_endpoint(), attr)])
        asserts.assert_equal(resp[0].Status, Status.Success, "Unexpected error writing FanMode attribute")

        self.step(5)
        percent_setting_original = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.PercentSetting)

        self.step(6)
        if await self.feature_guard(self.get_endpoint(), cluster=fan, feature_int=fan.Bitmaps.Feature.kMultiSpeed):
            speed_setting_original = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.SpeedSetting)

        self.step(7)
        percent_current = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.PercentCurrent)

        self.step(8)
        if percent_current != percent_setting_original:
            sub.await_all_final_values_reported(
                expected_final_values=[fan.Attributes.PercentCurrent(percent_setting_original)], timeout_sec=timeout)
        else:
            self.mark_current_step_skipped()

        self.step(9)
        await self.send_single_cmd(cmd=Clusters.OnOff.Commands.Off())

        self.step(10)
        awaiting = [AttributeValue(endpoint_id=self.get_endpoint(), attribute=fan.Attributes.PercentCurrent, value=0)]
        if has_spd:
            awaiting.append(AttributeValue(endpoint_id=self.get_endpoint(), attribute=fan.Attributes.SpeedCurrent, value=0))
        sub.await_all_final_values_reported(expected_final_values=awaiting, timeout_sec=timeout)
        sub.reset()

        self.step(11)
        mode = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.FanMode)
        asserts.assert_equal(mode, fan.Enums.FanModeEnum.kHigh, "FanMode was changed when on/off cluster was changed")

        self.step(12)
        percent_setting_new = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.PercentSetting)
        asserts.assert_equal(percent_setting_new, percent_setting_original,
                             "PercentSetting was changed when on/off cluster was changed")

        self.step(13)
        if await self.feature_guard(self.get_endpoint(), cluster=fan, feature_int=fan.Bitmaps.Feature.kMultiSpeed):
            speed_setting_new = await self.read_single_attribute_check_success(cluster=fan, attribute=fan.Attributes.SpeedSetting)
            asserts.assert_equal(speed_setting_new, speed_setting_original,
                                 "SpeedSetting was changed when on/off cluster was changed")

        # Changing the percent while the fan is off should change the percent setting, but not the percent current or speed current
        attr = Clusters.FanControl.Attributes.PercentSetting(1)
        resp = await self.default_controller.WriteAttribute(nodeid=self.dut_node_id, attributes=[(self.get_endpoint(), attr)])

        self.step(14)
        await self.send_single_cmd(cmd=Clusters.OnOff.Commands.On())

        self.step(15)
        awaiting = [AttributeValue(endpoint_id=self.get_endpoint(
        ), attribute=fan.Attributes.PercentCurrent, value=percent_setting_original)]
        if has_spd:
            awaiting.append(AttributeValue(endpoint_id=self.get_endpoint(),
                            attribute=fan.Attributes.SpeedCurrent, value=speed_setting_original))
        sub.await_all_final_values_reported(expected_final_values=awaiting, timeout_sec=timeout)

        sub.reset()

        # Make sure that we can still adjust the Percent values now that it's back on
        attr = Clusters.FanControl.Attributes.PercentSetting(1)
        resp = await self.default_controller.WriteAttribute(nodeid=self.dut_node_id, attributes=[(self.get_endpoint(), attr)])
        # we want to see a change on percent setting and percent current to 1
        sub.await_all_final_values_reported()


if __name__ == "__main__":
    default_matter_test_main()
