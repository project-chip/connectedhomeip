#
#    Copyright (c) 2023 Project CHIP Authors
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

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

#import time


logger = logging.getLogger(__name__)


class TC_FAN_3_4(MatterBaseTest):

    async def read_fc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.FanControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_wind_support(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.WindSupport)

    async def read_wind_setting(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.WindSetting)

    async def write_wind_setting(self, endpoint, wind_setting):
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.FanControl.Attributes.WindSetting(wind_setting))])
        asserts.assert_equal(result[0].Status, Status.Success, "WindSetting write failed")

    async def write_wind_setting_expect_failure(self, endpoint, wind_setting):
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.FanControl.Attributes.WindSetting(wind_setting))])
        asserts.assert_equal(result[0].Status, Status.ConstraintError, "Expected ConstraintError but received a different error.")

    def desc_TC_FAN_3_4(self) -> str:
        return "[TC-FAN-3.4] Optional wind functionality with DUT as Server"

    def steps_TC_FAN_3_4(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read from the DUT the WindSupport attribute and store"),
            TestStep(3, "SleepWind is supported, so write 0x01 to WindSetting"),
            TestStep(4, "Read from the DUT the WindSetting attribute"),
            TestStep(5, "SleepWind is not supported, so write 0x01 to WindSetting to check for constraint error"),
            TestStep(6, "NaturalWind is supported, so write 0x02 to WindSetting"),
            TestStep(7, "Read from the DUT the WindSetting attribute"),
            TestStep(8, "NaturalWind is not supported, so write 0x02 to WindSetting to check for constraint error"),
            TestStep(9, "Write WindSetting to 0x00"),
        ]
        return steps

    def pics_TC_FAN_3_4(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_4(self):
        if not self.check_pics("FAN.S.F03"):
            self.skip_all_remaining_steps(1)
            logger.info("Test skipped because PICS FAN.S.F03 is not set")
            return

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)

        self.step(2)
        wind_support = await self.read_wind_support(endpoint=endpoint)

        if wind_support & Clusters.FanControl.Bitmaps.WindBitmap.kSleepWind:
            self.step(3)
            await self.write_wind_setting(endpoint=endpoint, wind_setting=Clusters.FanControl.Bitmaps.WindBitmap.kSleepWind)
            # time.sleep(1)

            self.step(4)
            wind_setting = await self.read_wind_setting(endpoint=endpoint)
            asserts.assert_equal(wind_setting, Clusters.FanControl.Bitmaps.WindBitmap.kSleepWind, "WindSetting is not 0x01")

            self.skip_step(5)
        else:
            self.skip_step(3)
            self.skip_step(4)

            self.step(5)
            await self.write_wind_setting_expect_failure(endpoint=endpoint, wind_setting=Clusters.FanControl.Bitmaps.WindBitmap.kSleepWind)

        if wind_support & Clusters.FanControl.Bitmaps.WindBitmap.kNaturalWind:
            self.step(6)
            await self.write_wind_setting(endpoint=endpoint, wind_setting=Clusters.FanControl.Bitmaps.WindBitmap.kNaturalWind)
            # time.sleep(1)

            self.step(7)
            wind_setting = await self.read_wind_setting(endpoint=endpoint)
            asserts.assert_equal(wind_setting, Clusters.FanControl.Bitmaps.WindBitmap.kNaturalWind, "WindSetting is not 0x02")

            self.skip_step(8)
        else:
            self.skip_step(6)
            self.skip_step(7)

            self.step(8)
            await self.write_wind_setting_expect_failure(endpoint=endpoint, wind_setting=Clusters.FanControl.Bitmaps.WindBitmap.kNaturalWind)

        self.step(9)
        await self.write_wind_setting(endpoint=endpoint, wind_setting=0x00)


if __name__ == "__main__":
    default_matter_test_main()
