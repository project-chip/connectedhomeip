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
from matter_testing_support import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts


class TC_VALCC_3_2(MatterBaseTest):
    async def read_valcc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ValveConfigurationAndControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_VALCC_3_2(self) -> str:
        return "[TC-VALCC-3.2] Basic level functionality with DUT as Server"

    def steps_TC_VALCC_3_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read FeatureMap attribute"),
            TestStep(3, "Send Open command with TargetLevel set to 100"),
            TestStep(4, "Read TargetLevel attribute"),
            TestStep(5, "Read CurrentLevel attribute"),
            TestStep(6, "Send Close command"),
            TestStep(7, "Read TargetLevel attribute"),
            TestStep(8, "Read CurrentLevel attribute"),
        ]
        return steps

    @run_if_endpoint_matches(has_feature(Clusters.ValveConfigurationAndControl, Clusters.ValveConfigurationAndControl.Bitmaps.Feature.kLevel))
    async def test_TC_VALCC_3_2(self):

        endpoint = self.matter_test_config.endpoint

        self.step(1)
        attributes = Clusters.ValveConfigurationAndControl.Attributes

        self.step(2)
        # Done as part of the test initialization

        self.step(3)
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Open(targetLevel=100), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(4)
        target_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetLevel)

        asserts.assert_true(target_level_dut is not NullValue, "TargetLevel is null")
        asserts.assert_equal(target_level_dut, 100, "TargetLevel is not the expected value")

        self.step(5)
        current_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentLevel)
        asserts.assert_true(current_level_dut is not NullValue, "CurrentLevel is null")

        while current_level_dut != 100:
            time.sleep(1)

            current_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentLevel)
            asserts.assert_true(current_level_dut is not NullValue, "CurrentLevel is null")

        asserts.assert_equal(current_level_dut, 100, "CurrentLevel is not the expected value")

        self.step(6)
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Close(), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(7)
        target_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetLevel)

        asserts.assert_true(target_level_dut is not NullValue, "TargetLevel is null")
        asserts.assert_equal(target_level_dut, 0, "TargetLevel is not the expected value")

        self.step(8)
        current_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentLevel)
        asserts.assert_true(current_level_dut is not NullValue, "CurrentLevel is null")

        while current_level_dut is Clusters.Objects.ValveConfigurationAndControl.Enums.ValveStateEnum.kTransitioning:
            time.sleep(1)

            current_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentLevel)
            asserts.assert_true(current_level_dut is not NullValue, "CurrentLevel is null")

        asserts.assert_equal(current_level_dut, 0, "CurrentLevel is not the expected value")


if __name__ == "__main__":
    default_matter_test_main()
