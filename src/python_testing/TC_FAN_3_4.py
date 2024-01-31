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

    def pics_TC_FAN_3_4(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_4(self):
        if not self.check_pics("FAN.S.F03"):
            logger.info("Test skipped because PICS FAN.S.F03 is not set")
            return

        endpoint = self.user_params.get("endpoint", 1)

        self.print_step(1, "Commissioning, already done")

        self.print_step(2, "Read from the DUT the WindSupport attribute and store")
        wind_support = await self.read_wind_support(endpoint=endpoint)

        if wind_support & Clusters.FanControl.Bitmaps.WindBitmap.kSleepWind:
            self.print_step(3, "SleepWind is supported, so write 0x01 to WindSetting")
            await self.write_wind_setting(endpoint=endpoint, wind_setting=Clusters.FanControl.Bitmaps.WindBitmap.kSleepWind)
            time.sleep(1)

            self.print_step(4, "Read from the DUT the WindSetting attribute")
            wind_setting = await self.read_wind_setting(endpoint=endpoint)
            asserts.assert_equal(wind_setting, Clusters.FanControl.Bitmaps.WindBitmap.kSleepWind, "WindSetting is not 0x01")

        if wind_support & Clusters.FanControl.Bitmaps.WindBitmap.kNaturalWind:
            self.print_step(5, "NaturalWind is supported, so write 0x02 to WindSetting")
            await self.write_wind_setting(endpoint=endpoint, wind_setting=Clusters.FanControl.Bitmaps.WindBitmap.kNaturalWind)
            time.sleep(1)

            self.print_step(6, "Read from the DUT the WindSetting attribute")
            wind_setting = await self.read_wind_setting(endpoint=endpoint)
            asserts.assert_equal(wind_setting, Clusters.FanControl.Bitmaps.WindBitmap.kNaturalWind, "WindSetting is not 0x02")

        self.print_step(9, "Write WindSetting to 0x00")
        await self.write_wind_setting(endpoint=endpoint, wind_setting=0x00)


if __name__ == "__main__":
    default_matter_test_main()
