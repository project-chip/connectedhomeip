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

import time
import logging

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_VCC_4_3(MatterBaseTest):
    async def read_vcc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ValveConfigurationAndControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_VCC_4_3(self) -> str:
        return "[TC-VCC-4.2] AutoCloseTime functionality with DUT as Server"

    def steps_TC_VCC_4_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read FeatureMap attribute"),
            TestStep(3, "Read UTCTime attribute from TimeSync cluster"),
            TestStep(4, "Send Open command with duration set to 60"),
            TestStep(5, "Read AutoCloseTime attribute"),
            TestStep(6, "Send Close command"),
            TestStep(7, "Read AutoCloseTime attribute"),
            TestStep(8, "Read DefaultOpenDuration attribute"),
            TestStep(9, "Read UTCTime attribute from TimeSync cluster"),
            TestStep(10, "Send Open command"),
            TestStep(11, "Read AutoCloseTime attribute"),
            TestStep(12, "Send Close command"),
            TestStep(13, "Read AutoCloseTime attribute"),
        ]
        return steps

    def pics_TC_VCC_4_3(self) -> list[str]:
        pics = [
            "VCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_VCC_4_3(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.ValveConfigurationAndControl.Attributes

        self.step(2)
        feature_map = await self.read_vcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)

        is_ts_feature_supported = feature_map & Clusters.ValveConfigurationAndControl.Bitmaps.Feature.kTimeSync

        self.step(3)
        if is_ts_feature_supported:
            utcTime = await self.read_single_attribute_check_success(endpoint=0, cluster=Clusters.Objects.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.UTCTime)

            asserts.assert_true(utcTime is not NullValue, "OpenDuration is null")
        else:
            logging.info("Test step skipped")

        self.step(4)
        if is_ts_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Open(openDuration=60), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step(5)
        if is_ts_feature_supported:
            auto_close_time_dut = await self.read_vcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.AutoCloseTime)

            asserts.assert_true(auto_close_time_dut is not NullValue, "AutoCloseTime is null")
            asserts.assert_greater_equal(auto_close_time_dut, (utcTime + 55000000),
                                         "AutoCloseTime is not in the expected range")
            asserts.assert_less_equal(auto_close_time_dut, (utcTime + 60000000), "AutoCloseTime is not in the expected range")
        else:
            logging.info("Test step skipped")

        self.step(6)
        if is_ts_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Close(), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step(7)
        if is_ts_feature_supported:
            auto_close_time_dut = await self.read_vcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.AutoCloseTime)

            print(auto_close_time_dut)

            asserts.assert_true(auto_close_time_dut is NullValue, "AutoCloseTime is not null")
        else:
            logging.info("Test step skipped")

        self.step(8)
        defaultOpenDuration = 0

        if is_ts_feature_supported:
            defaultOpenDuration = await self.read_vcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.OpenDuration)

            print(auto_close_time_dut)

            asserts.assert_true(auto_close_time_dut is NullValue, "AutoCloseTime is not null")
        else:
            logging.info("Test step skipped")

        self.step(9)
        if is_ts_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Open(openDuration=60), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step(10)
        if is_ts_feature_supported:
            utcTime = await self.read_single_attribute_check_success(endpoint=0, cluster=Clusters.Objects.TimeSynchronization, attribute=Clusters.TimeSynchronization.Attributes.UTCTime)

            asserts.assert_true(utcTime is not NullValue, "OpenDuration is null")
        else:
            logging.info("Test step skipped")

        self.step(11)
        if is_ts_feature_supported:
            auto_close_time_dut = await self.read_vcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.AutoCloseTime)

            asserts.assert_true(auto_close_time_dut is not NullValue, "AutoCloseTime is null")
            asserts.assert_greater_equal(auto_close_time_dut, (utcTime + ((defaultOpenDuration - 5) * 1000000)),
                                         "AutoCloseTime is not in the expected range")
            asserts.assert_less_equal(auto_close_time_dut, (utcTime + (defaultOpenDuration * 1000000)),
                                      "AutoCloseTime is not in the expected range")
        else:
            logging.info("Test step skipped")

        self.step(12)
        if is_ts_feature_supported:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Close(), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step(13)
        if is_ts_feature_supported:
            auto_close_time_dut = await self.read_vcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.AutoCloseTime)

            asserts.assert_true(auto_close_time_dut is NullValue, "AutoCloseTime is not null")
        else:
            logging.info("Test step skipped")


if __name__ == "__main__":
    default_matter_test_main()
