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

import logging
import time

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_FAN_3_2(MatterBaseTest):
    async def read_fc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.FanControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def read_speed_setting(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.SpeedSetting)

    async def read_speed_current(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.SpeedCurrent)

    async def read_speed_max(self, endpoint):
        return await self.read_fc_attribute_expect_success(endpoint, Clusters.FanControl.Attributes.SpeedMax)

    async def write_speed_setting(self, endpoint, speed_setting) -> Status:
        result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, Clusters.FanControl.Attributes.SpeedSetting(speed_setting))])
        return result[0].Status

    def pics_TC_FAN_3_2(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_3_2(self):
        if not self.check_pics("FAN.S.F00"):
            logger.info("Test skipped because PICS FAN.S.F00 is not set")
            return

        endpoint = self.user_params.get("endpoint", 1)

        self.print_step(1, "Commissioning, already done")

        self.print_step(2, "Read from the DUT the SpeedSetting attribute and store")
        existing_speed_setting = await self.read_speed_setting(endpoint=endpoint)

        self.print_step(3, "Read from the DUT the SpeedMax attribute and store")
        speed_max = await self.read_speed_max(endpoint=endpoint)

        self.print_step(4, "Write to the DUT the SpeedSetting attribute with value SpeedMax")
        status = await self.write_speed_setting(endpoint=endpoint, speed_setting=speed_max)
        status_ok = (status == Status.Success) or (status == Status.InvalidInState)
        asserts.assert_true(status_ok, "SpeedSetting write did not return a value of Success or InvalidInState")

        self.print_step(5, "After a few seconds, read from the DUT the SpeedSetting attribute")
        time.sleep(3)

        new_speed_setting = await self.read_speed_setting(endpoint=endpoint)

        if status == Status.Success:
            asserts.assert_equal(new_speed_setting, speed_max, "SpeedSetting is not equal to SpeedMax")
        else:
            asserts.assert_equal(new_speed_setting, existing_speed_setting, "SpeedSetting is not unchanged")

        self.print_step(6, "Read from the DUT the SpeedCurrent attribute")
        speed_current = await self.read_speed_current(endpoint=endpoint)

        if status == Status.Success:
            asserts.assert_equal(speed_current, speed_max, "SpeedCurrent is not equal to SpeedMax")
        else:
            asserts.assert_equal(speed_current, existing_speed_setting, "SpeedCurrent is not unchanged")


if __name__ == "__main__":
    default_matter_test_main()
