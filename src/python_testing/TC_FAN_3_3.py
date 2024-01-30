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

import logging
import time

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_FAN_3_3(MatterBaseTest):

    async def read_fc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.FanControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_rock_support(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.RockSupport)

    async def read_rock_setting(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.RockSetting)

    async def write_rock_setting(self, endpoint, rock_setting):
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.FanControl.Attributes.RockSetting(rock_setting))])
        asserts.assert_equal(result[0].Status, Status.Success, "RockSetting write failed")

    def pics_TC_FAN_3_3(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_3(self):
        if not self.check_pics("FAN.S.F02"):
            logger.info("Test skipped because PICS FAN.S.F02 is not set")
            return

        endpoint = self.user_params.get("endpoint", 1)

        self.print_step(1, "Commissioning, already done")

        self.print_step(2, "Read from the DUT the RockSupport attribute and store")
        rock_support = await self.read_rock_support(endpoint=endpoint)

        if rock_support & Clusters.FanControl.Bitmaps.RockBitmap.kRockLeftRight:
            self.print_step(3, "RockLeftRight is supported, so write 0x01 to RockSetting")
            await self.write_rock_setting(endpoint=endpoint, rock_setting=Clusters.FanControl.Bitmaps.RockBitmap.kRockLeftRight)
            time.sleep(1)

            self.print_step(4, "Read from the DUT the RockSetting attribute")
            rock_setting = await self.read_rock_setting(endpoint=endpoint)
            asserts.assert_equal(rock_setting, Clusters.FanControl.Bitmaps.RockBitmap.kRockLeftRight, "RockSetting is not 0x01")

        if rock_support & Clusters.FanControl.Bitmaps.RockBitmap.kRockUpDown:
            self.print_step(5, "RockUpDown is supported, so write 0x02 to RockSetting")
            await self.write_rock_setting(endpoint=endpoint, rock_setting=Clusters.FanControl.Bitmaps.RockBitmap.kRockUpDown)
            time.sleep(1)

            self.print_step(6, "Read from the DUT the RockSetting attribute")
            rock_setting = await self.read_rock_setting(endpoint=endpoint)
            asserts.assert_equal(rock_setting, Clusters.FanControl.Bitmaps.RockBitmap.kRockUpDown, "RockSetting is not 0x02")

        if rock_support & Clusters.FanControl.Bitmaps.RockBitmap.kRockRound:
            self.print_step(7, "RockRound is supported, so write 0x04 to RockSetting")
            await self.write_rock_setting(endpoint=endpoint, rock_setting=Clusters.FanControl.Bitmaps.RockBitmap.kRockRound)
            time.sleep(1)

            self.print_step(8, "Read from the DUT the RockSetting attribute")
            rock_setting = await self.read_rock_setting(endpoint=endpoint)
            asserts.assert_equal(rock_setting, Clusters.FanControl.Bitmaps.RockBitmap.kRockRound, "RockSetting is not 0x04")

        self.print_step(9, "Write RockSetting to 0x00")
        await self.write_rock_setting(endpoint=endpoint, rock_setting=0x00)


if __name__ == "__main__":
    default_matter_test_main()
