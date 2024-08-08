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
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_VALCC_3_3(MatterBaseTest):
    async def read_valcc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ValveConfigurationAndControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_VALCC_3_3(self) -> str:
        return "[TC-VALCC-3.3] DefaultOpenLevel functionality with DUT as Server"

    def steps_TC_VALCC_3_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read AttributeList attribute"),
            TestStep(3, "Read DefaultOpenLevel attribute, if supported"),
            TestStep(4, "Send Open command"),
            TestStep(5, "Read TargetLevel attribute"),
            TestStep(6, "Read CurrentLevel attribute"),
            TestStep(7, "Send Close command"),
            TestStep(8, "Read TargetLevel attribute"),
            TestStep(9, "Read CurrentLevel attribute"),
        ]
        return steps

    def pics_TC_VALCC_3_3(self) -> list[str]:
        pics = [
            "VALCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_VALCC_3_3(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.ValveConfigurationAndControl.Attributes

        self.step(2)
        attribute_list = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        self.step(3)
        if attributes.DefaultOpenLevel.attribute_id in attribute_list:
            defaultOpenLevel = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.DefaultOpenLevel)
        else:
            logging.info("Test step skipped")

        self.step(4)
        if attributes.DefaultOpenLevel.attribute_id in attribute_list:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Open(), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step(5)
        if attributes.DefaultOpenLevel.attribute_id in attribute_list:
            target_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetLevel)

            asserts.assert_true(target_level_dut is not NullValue, "TargetLevel is null")
            asserts.assert_equal(target_level_dut, defaultOpenLevel, "TargetLevel is not the expected value")
        else:
            logging.info("Test step skipped")

        self.step(6)
        if attributes.DefaultOpenLevel.attribute_id in attribute_list:
            current_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentLevel)
            asserts.assert_true(current_level_dut is not NullValue, "CurrentLevel is null")

            while current_level_dut != defaultOpenLevel:
                time.sleep(1)

                current_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentLevel)
                asserts.assert_true(current_level_dut is not NullValue, "CurrentLevel is null")

            asserts.assert_equal(current_level_dut, defaultOpenLevel, "CurrentLevel is not the expected value")
        else:
            logging.info("Test step skipped")

        self.step(7)
        if attributes.DefaultOpenLevel.attribute_id in attribute_list:
            try:
                await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Close(), endpoint=endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
                pass
        else:
            logging.info("Test step skipped")

        self.step(8)
        if attributes.DefaultOpenLevel.attribute_id in attribute_list:
            target_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetLevel)

            asserts.assert_true(target_level_dut is not NullValue, "TargetLevel is null")
            asserts.assert_equal(target_level_dut, 0, "TargetLevel is not the expected value")
        else:
            logging.info("Test step skipped")

        self.step(9)
        if attributes.DefaultOpenLevel.attribute_id in attribute_list:
            current_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentLevel)
            asserts.assert_true(current_level_dut is not NullValue, "CurrentLevel is null")

            while current_level_dut is Clusters.Objects.ValveConfigurationAndControl.Enums.ValveStateEnum.kTransitioning:
                time.sleep(1)

                current_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentLevel)
                asserts.assert_true(current_level_dut is not NullValue, "CurrentLevel is null")

            asserts.assert_equal(current_level_dut, 0, "CurrentLevel is not the expected value")
        else:
            logging.info("Test step skipped")


if __name__ == "__main__":
    default_matter_test_main()
