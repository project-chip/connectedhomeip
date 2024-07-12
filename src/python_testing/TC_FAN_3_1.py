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
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import time

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_FAN_3_1(MatterBaseTest):

    async def read_fc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.FanControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_fan_mode(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.FanMode)

    async def read_percent_setting(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.PercentSetting)

    async def read_percent_current(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.PercentCurrent)

    async def write_fan_mode(self, endpoint, fan_mode) -> Status:
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.FanControl.Attributes.FanMode(fan_mode))])
        return result[0].Status

    async def write_percent_setting(self, endpoint, percent_setting) -> Status:
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.FanControl.Attributes.PercentSetting(percent_setting))])
        return result[0].Status

    def pics_TC_FAN_3_1(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_1(self):
        endpoint = self.user_params.get("endpoint", 1)

        self.print_step(1, "Commissioning, already done")

        self.print_step("2a", "Read from the DUT the FanMode attribute and store")
        existing_fan_mode = await self.read_fan_mode(endpoint=endpoint)

        self.print_step("2b", "Write High to FanMode")
        status = await self.write_fan_mode(endpoint=endpoint, fan_mode=Clusters.FanControl.Enums.FanModeEnum.kHigh)
        status_ok = (status == Status.Success) or (status == Status.InvalidInState)
        asserts.assert_true(status_ok, "FanMode write did not return a value of Success or InvalidInState")

        self.print_step("2c", "After a few seconds, read from the DUT the FanMode attribute")
        time.sleep(3)

        new_fan_mode = await self.read_fan_mode(endpoint=endpoint)

        if status == Status.Success:
            asserts.assert_equal(new_fan_mode, Clusters.FanControl.Enums.FanModeEnum.kHigh, "FanMode is not High")
        else:
            asserts.assert_equal(new_fan_mode, existing_fan_mode, "FanMode is not unchanged")

        self.print_step("3a", "Read from the DUT the PercentSetting attribute and store")
        existing_percent_setting = await self.read_percent_setting(endpoint=endpoint)

        self.print_step("3b", "Write Off to Fan Mode")
        status = await self.write_fan_mode(endpoint=endpoint, fan_mode=Clusters.FanControl.Enums.FanModeEnum.kOff)
        status_ok = (status == Status.Success) or (status == Status.InvalidInState)
        asserts.assert_true(status_ok, "FanMode write did not return a value of Success or InvalidInState")

        self.print_step("3c", "After a few seconds, read from the DUT the PercentSetting attribute")
        time.sleep(3)

        new_percent_setting = await self.read_percent_setting(endpoint=endpoint)

        if status == Status.Success:
            asserts.assert_equal(new_percent_setting, Clusters.FanControl.Enums.FanModeEnum.kOff, "PercentSetting is not Off")
        else:
            asserts.assert_equal(new_percent_setting, existing_percent_setting, "PercentSetting is not unchanged")

        self.print_step("3d", "Read from the DUT the PercentCurrent attribute")
        percent_current = await self.read_percent_current(endpoint=endpoint)

        if status == Status.Success:
            asserts.assert_equal(percent_current, 0, "PercentCurrent is not 0")
        else:
            asserts.assert_equal(percent_current, existing_percent_setting, "PercentCurrent is not unchanged")

        self.print_step("4a", "Read from the DUT the PercentSetting attribute and store")
        existing_percent_setting = await self.read_percent_setting(endpoint=endpoint)

        self.print_step("4b", "Write PercentSetting to 30")
        status = await self.write_percent_setting(endpoint=endpoint, percent_setting=30)
        status_ok = (status == Status.Success) or (status == Status.InvalidInState)
        asserts.assert_true(status_ok, "PercentSetting write did not return a value of Success or InvalidInState")

        self.print_step("4c", "After a few seconds, read from the DUT the PercentSetting attribute")
        time.sleep(3)

        new_percent_setting = await self.read_percent_setting(endpoint=endpoint)

        if status == Status.Success:
            asserts.assert_equal(new_percent_setting, 30, "PercentSetting is not 30")
        else:
            asserts.assert_equal(new_percent_setting, existing_percent_setting, "PercentSetting is not unchanged")

        self.print_step("4d", "Read from the DUT the PercentCurrent attribute")
        percent_current = await self.read_percent_current(endpoint=endpoint)

        if status == Status.Success:
            asserts.assert_equal(percent_current, 30, "PercentCurrent is not 30")
        else:
            asserts.assert_equal(percent_current, existing_percent_setting, "PercentCurrent is not unchanged")

        self.print_step("5a", "Read from the DUT the FanMode attribute and store")
        existing_fan_mode = await self.read_fan_mode(endpoint=endpoint)

        self.print_step("5b", "Write PercentSetting to 0")
        status = await self.write_percent_setting(endpoint=endpoint, percent_setting=0)
        status_ok = (status == Status.Success) or (status == Status.InvalidInState)
        asserts.assert_true(status_ok, "PercentSetting write did not return a value of Success or InvalidInState")

        self.print_step("5c", "After a few seconds, read from the DUT the FanMode attribute")
        time.sleep(3)

        new_fan_mode = await self.read_fan_mode(endpoint=endpoint)

        if status == Status.Success:
            asserts.assert_equal(new_fan_mode, Clusters.FanControl.Enums.FanModeEnum.kOff, "FanMode is not Off")
        else:
            asserts.assert_equal(new_fan_mode, existing_fan_mode, "FanMode is not unchanged")


if __name__ == "__main__":
    default_matter_test_main()
