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

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts


class TC_CCTRL_2_1(MatterBaseTest):

    def steps_TC_CCTRL_2_1(self) -> list[TestStep]:
        steps = [TestStep(1, "Read MCORE.FS PICS code", is_commissioning=True),
                 TestStep(2, "Validate SupportedDeviceCategories is set accordingly based on MCORE.FS")]
        return steps

    @run_if_endpoint_matches(has_cluster(Clusters.CommissionerControl))
    async def test_TC_CCTRL_2_1(self):
        self.step(1)
        is_fabric_sync_pics_enabled = self.check_pics("MCORE.FS")

        self.step(2)
        supported_device_categories = await self.read_single_attribute_check_success(cluster=Clusters.CommissionerControl, attribute=Clusters.CommissionerControl.Attributes.SupportedDeviceCategories)
        is_fabric_sync_bit_set = bool(supported_device_categories &
                                      Clusters.CommissionerControl.Bitmaps.SupportedDeviceCategoryBitmap.kFabricSynchronization)
        asserts.assert_equal(is_fabric_sync_bit_set, is_fabric_sync_pics_enabled,
                             "Mismatch between PICS MCORE.FS value and what attribute indicates")


if __name__ == "__main__":
    default_matter_test_main()
