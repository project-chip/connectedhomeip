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


class TC_VALCC_4_2(MatterBaseTest):
    async def read_valcc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ValveConfigurationAndControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_VALCC_4_2(self) -> str:
        return "[TC-VALCC-4.2] DefaultOpenDuration functionality with DUT as Server"

    def steps_TC_VALCC_4_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "Read DefaultOpenDuration attribute"),
            TestStep("2b", "Write DefaultOpenDuration attribute"),
            TestStep(3, "Send Open command"),
            TestStep(4, "Read OpenDuration attribute"),
            TestStep(5, "Read RemainingDuration attribute"),
            TestStep(6, "Wait for 5 seconds"),
            TestStep(7, "Read RemainingDuration attribute"),
            TestStep(8, "Send Close command"),
            TestStep(9, "Read OpenDuration attribute"),
            TestStep(10, "Read RemainingDuration attribute"),
        ]
        return steps

    def pics_TC_VALCC_4_2(self) -> list[str]:
        pics = [
            "VALCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_VALCC_4_2(self):

        endpoint = self.user_params.get("endpoint", 1)

        self.step(1)
        attributes = Clusters.ValveConfigurationAndControl.Attributes

        self.step("2a")
        defaultOpenDuration = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.DefaultOpenDuration)

        self.step("2b")
        if defaultOpenDuration is NullValue:
            defaultOpenDuration = 60

            result = await self.default_controller.WriteAttribute(self.dut_node_id, [(endpoint, attributes.DefaultOpenDuration(defaultOpenDuration))])
            asserts.assert_equal(result[0].Status, Status.Success, "DefaultOpenDuration write failed")
        else:
            logging.info("Test step skipped")

        self.step(3)
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Open(), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(4)
        open_duration_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.OpenDuration)
        asserts.assert_true(open_duration_dut is not NullValue, "OpenDuration is null")
        asserts.assert_equal(open_duration_dut, defaultOpenDuration, "OpenDuration is not the expected value")

        self.step(5)
        remaining_duration_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.RemainingDuration)
        asserts.assert_true(remaining_duration_dut is not NullValue, "RemainingDuration is null")
        asserts.assert_greater_equal(remaining_duration_dut, (defaultOpenDuration - 5),
                                     "RemainingDuration is not in the expected range")
        asserts.assert_less_equal(remaining_duration_dut, defaultOpenDuration, "RemainingDuration is not in the expected range")

        self.step(6)
        time.sleep(5)

        self.step(7)
        remaining_duration_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.RemainingDuration)
        asserts.assert_true(remaining_duration_dut is not NullValue, "RemainingDuration is null")
        asserts.assert_greater_equal(remaining_duration_dut, (defaultOpenDuration - 10),
                                     "RemainingDuration is not in the expected range")
        asserts.assert_less_equal(remaining_duration_dut, (defaultOpenDuration - 5),
                                  "RemainingDuration is not in the expected range")

        self.step(8)
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Close(), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(9)
        open_duration_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.OpenDuration)
        asserts.assert_true(open_duration_dut is NullValue, "OpenDuration is not null")

        self.step(10)
        remaining_duration_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.RemainingDuration)
        asserts.assert_true(remaining_duration_dut is NullValue, "RemainingDuration is not null")


if __name__ == "__main__":
    default_matter_test_main()
