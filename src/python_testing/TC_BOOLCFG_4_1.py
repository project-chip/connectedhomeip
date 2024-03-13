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

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_BOOLCFG_4_1(MatterBaseTest):
    async def read_boolcfg_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.BooleanStateConfiguration
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_BOOLCFG_4_1(self) -> str:
        return "[TC-BOOLCFG-4.1] AlarmsSupported attribute with DUT as Server"

    def steps_TC_BOOLCFG_4_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read FeatureMap attribute"),
            TestStep(3, "Read AlarmsSupported attribute"),
            TestStep("4a", "Verify AlarmsSupported attribute bit 0"),
            TestStep("4b", "Verify AlarmsSupported attribute bit 0"),
            TestStep("5a", "Verify AlarmsSupported attribute bit 1"),
            TestStep("5b", "Verify AlarmsSupported attribute bit 1"),
        ]
        return steps

    def pics_TC_BOOLCFG_4_1(self) -> list[str]:
        pics = [
            "BOOLCFG.S",
        ]
        return pics

    @async_test_body
    async def test_TC_BOOLCFG_4_1(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.BooleanStateConfiguration.Attributes

        self.step(2)
        feature_map = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        is_vis_feature_supported = feature_map & Clusters.BooleanStateConfiguration.Bitmaps.Feature.kVisual
        is_aud_feature_supported = feature_map & Clusters.BooleanStateConfiguration.Bitmaps.Feature.kAudible

        self.step(3)
        if is_vis_feature_supported or is_aud_feature_supported:
            supportedAlarms = await self.read_boolcfg_attribute_expect_success(endpoint=endpoint, attribute=attributes.AlarmsSupported)
        else:
            logging.info("Test step skipped")

        self.step("4a")
        if is_vis_feature_supported:
            asserts.assert_not_equal((supportedAlarms & Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kVisual), 0,
                                     "Bit 0 in AlarmsSupported does not match feature map value")
        else:
            logging.info("Test step skipped")

        self.step("4b")
        if not is_vis_feature_supported:
            asserts.assert_equal((supportedAlarms & Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kVisual), 0,
                                 "Bit 0 in AlarmsSupported does not match feature map value")
        else:
            logging.info("Test step skipped")

        self.step("5a")
        if is_aud_feature_supported:
            asserts.assert_not_equal((supportedAlarms & Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kAudible), 0,
                                     "Bit 1 in AlarmsSupported does not match feature map value")
        else:
            logging.info("Test step skipped")

        self.step("5b")
        if not is_aud_feature_supported:
            asserts.assert_equal((supportedAlarms & Clusters.BooleanStateConfiguration.Bitmaps.AlarmModeBitmap.kAudible), 0,
                                 "Bit 1 in AlarmsSupported does not match feature map value")
        else:
            logging.info("Test step skipped")


if __name__ == "__main__":
    default_matter_test_main()
