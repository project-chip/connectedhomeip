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
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import (AttributeValue, ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep,
                                         async_test_body, default_matter_test_main)
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
            TestStep(2, "Set up a subscription to all attributes on the DUT"),
            TestStep(3, "Send a close command to the DUT and wait until the CurrentState is closed", "DUT returns SUCCESS"),
            TestStep(4, "TH sends command Open command with TargetLevel field set to 100 and the remaining fields not populated.",
                     "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(5, "Wait until TH receives data reports for TargetState set to NULL, TargetLevel set to NULL, CurrentState set to Open and CurrentLevel set to 100 (ordering does not matter)",
                     "Expected attribute reports are received"),
            TestStep(6, "Read CurrentState, CurrentLevel, TargetState and TargetLevel attributes",
                     "CurrentState is Open, CurrentLevel is 100, TargetState is NULL and TargetLevel is NULL"),
            TestStep(7, "Send Close command", "DUT returns SUCCESS"),
            TestStep(8, "Wait until TH receives and data report for TargetState set to NULL and an attribute report for CurrentState set to Closed (ordering does not matter)",
                     "Expected attribute reports are received"),
            TestStep(9, "Read CurrentState and TargetState attribute", "CurrentState is Closed, TargetState is NULL"),
        ]
        return steps

    def pics_TC_VALCC_3_2(self) -> list[str]:
        pics = [
            "VALCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_VALCC_3_2(self):

        endpoint = self.get_endpoint(default=1)
        asserts.assert_is_not_none(
            endpoint, "Endpoint is required for this tests. The test endpoint is set using the --endpoint flag")

        self.step(1)
        attributes = Clusters.ValveConfigurationAndControl.Attributes
        # TODO: replace with top-level check using run_if_endpoint_matches
        feature_map = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        is_lvl_feature_supported = feature_map & Clusters.ValveConfigurationAndControl.Bitmaps.Feature.kLevel
        if not is_lvl_feature_supported:
            asserts.skip('Endpoint does not match test requirements')

        self.step(2)
        cluster = Clusters.ValveConfigurationAndControl
        attributes = cluster.Attributes
        attribute_subscription = ClusterAttributeChangeAccumulator(cluster)
        await attribute_subscription.start(self.default_controller, self.dut_node_id, endpoint)

        self.step(3)
        # Wait for the entire duration of the test because this valve may be slow. The test will time out before this does. That's fine.
        timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout
        await self.send_single_cmd(cmd=cluster.Commands.Close(), endpoint=endpoint)
        current_state_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
        if current_state_dut != cluster.Enums.ValveStateEnum.kClosed:
            current_state_closed = AttributeValue(
                endpoint_id=endpoint, attribute=attributes.CurrentState, value=cluster.Enums.ValveStateEnum.kClosed)
            attribute_subscription.await_all_final_values_reported(
                expected_final_values=[current_state_closed], timeout_sec=timeout)

        self.step(4)
        attribute_subscription.reset()
        try:
            await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Open(targetLevel=100), endpoint=endpoint)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(5)
        # Wait until the current state is open and the target state is Null.
        expected_final_state = [AttributeValue(endpoint_id=endpoint, attribute=attributes.TargetState, value=NullValue),
                                AttributeValue(endpoint_id=endpoint, attribute=attributes.CurrentState,
                                               value=cluster.Enums.ValveStateEnum.kOpen),
                                AttributeValue(endpoint_id=endpoint, attribute=attributes.TargetLevel, value=NullValue),
                                AttributeValue(endpoint_id=endpoint, attribute=attributes.CurrentLevel, value=100)]
        attribute_subscription.await_all_final_values_reported(expected_final_values=expected_final_state, timeout_sec=timeout)

        self.step(6)
        target_state_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
        current_state_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
        target_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetLevel)
        current_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentLevel)
        asserts.assert_equal(current_state_dut, cluster.Enums.ValveStateEnum.kOpen, "CurrentState is not open")
        asserts.assert_equal(target_state_dut, NullValue, "TargetState is not null")
        asserts.assert_equal(current_level_dut, 100, "CurrentLevel is not 100")
        asserts.assert_equal(target_level_dut, NullValue, "TargetLevel is not null")

        self.step(7)
        attribute_subscription.reset()
        await self.send_single_cmd(cmd=cluster.Commands.Close(), endpoint=endpoint)

        self.step(8)
        expected_final_state = [AttributeValue(endpoint_id=endpoint, attribute=attributes.TargetState, value=NullValue),
                                AttributeValue(endpoint_id=endpoint, attribute=attributes.CurrentState,
                                               value=cluster.Enums.ValveStateEnum.kClosed),
                                AttributeValue(endpoint_id=endpoint, attribute=attributes.TargetLevel, value=NullValue),
                                AttributeValue(endpoint_id=endpoint, attribute=attributes.CurrentLevel, value=0)]
        attribute_subscription.await_all_final_values_reported(expected_final_values=expected_final_state, timeout_sec=timeout)
        attribute_subscription.await_all_final_values_reported(expected_final_values=expected_final_state, timeout_sec=timeout)

        self.step(9)
        target_state_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
        current_state_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
        target_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetLevel)
        current_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentLevel)
        asserts.assert_equal(current_state_dut, cluster.Enums.ValveStateEnum.kClosed, "CurrentState is not closed")
        asserts.assert_equal(target_state_dut, NullValue, "TargetState is not null")
        asserts.assert_equal(current_level_dut, 0, "CurrentLevel is not 0")
        asserts.assert_equal(target_level_dut, NullValue, "TargetLevel is not null")


if __name__ == "__main__":
    default_matter_test_main()
