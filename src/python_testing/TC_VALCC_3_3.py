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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.decorators import async_test_body
from chip.testing.matter_base_test import MatterBaseTest
from chip.testing.models import TestStep
from chip.testing.runner import default_matter_test_main
from chip.testing.utilities import AttributeValue, ClusterAttributeChangeAccumulator
from mobly import asserts


class TC_VALCC_3_3(MatterBaseTest):
    async def read_valcc_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.ValveConfigurationAndControl
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_VALCC_3_3(self) -> str:
        return "[TC-VALCC-3.3] DefaultOpenLevel functionality with DUT as Server"

    def steps_TC_VALCC_3_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT if required", is_commissioning=True),
            TestStep(2, "Read AttributeList attribute", "Verify that the DUT response contains the AttributeList attribute."),
            TestStep(3, "If the DefaultOpenLevel is not supported, skip all remaining steps in this test"),
            TestStep(4, "TH reads from the DUT the DefaultOpenLevel attribute. Store the value as defaultOpenLevel."),
            TestStep(5, "Set up a subscription to all attributes on the DUT", "Subscription is successful"),
            TestStep(6, "Send a close command to the DUT and wait until the CurrentState is reported as closed", "DUT returns SUCCESS"),
            # TODO: this test should probably SET the default open attribute as well and un-set it at the end, so we're not testing against the default.
            TestStep(7, "Send Open command with no fields populated", "DUT returns SUCCESS"),
            TestStep(8, "Wait until TH receives the following reports (ordering does not matter): TargetState set to NULL, TargetLevel set to NULL, CurrentState set to Open, CurrentLevel set to defaultOpenLevel",
                     "Expected attribute reports are received"),
            TestStep(9, "Read CurrentState and TargetState attribute", "CurrentState is Open, TargetState is NULL"),
            TestStep(10, "Read CurrentLevel and TargetLevel attribute", "CurrentLevel is defaultOpenLevel, TargetLevel is NULL"),
            TestStep(11, "Send Close command", "DUT returns SUCCESS"),
            TestStep(12, "Wait until TH receives the following reports (ordering does not matter): TargetState set to NULL, TargetLevel set to NULL, CurrentState set to Closed, CurrentLevel set to 0",
                     "Expected attribute reports are received"),
            TestStep(13, "Read CurrentState and TargetState attribute", "CurrentState is Closed, TargetState is NULL"),
            TestStep(14, "Read CurrentLevel and TargetLevel attribute", "CurrentLevel is 0, TargetLevel is NULL"),
        ]
        return steps

    def pics_TC_VALCC_3_3(self) -> list[str]:
        pics = [
            "VALCC.S",
        ]
        return pics

    @async_test_body
    async def test_TC_VALCC_3_3(self):

        endpoint = self.get_endpoint(default=1)

        self.step(1)
        attributes = Clusters.ValveConfigurationAndControl.Attributes

        self.step(2)
        attribute_list = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        self.step(3)
        if attributes.DefaultOpenLevel.attribute_id not in attribute_list:
            asserts.skip('Endpoint does not match test requirements')

        self.step(4)
        default_open_level = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.DefaultOpenLevel)

        self.step(5)
        cluster = Clusters.ValveConfigurationAndControl
        attributes = cluster.Attributes
        attribute_subscription = ClusterAttributeChangeAccumulator(cluster)
        await attribute_subscription.start(self.default_controller, self.dut_node_id, endpoint)

        self.step(6)
        timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout
        await self.send_single_cmd(cmd=cluster.Commands.Close(), endpoint=endpoint)
        current_state_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
        if current_state_dut != cluster.Enums.ValveStateEnum.kClosed:
            current_state_closed = AttributeValue(
                endpoint_id=endpoint, attribute=attributes.CurrentState, value=cluster.Enums.ValveStateEnum.kClosed)
            attribute_subscription.await_all_final_values_reported(
                expected_final_values=[current_state_closed], timeout_sec=timeout)

        self.step(7)
        attribute_subscription.reset()
        await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Open(), endpoint=endpoint)

        self.step(8)
        expected_final_state = [AttributeValue(endpoint_id=endpoint, attribute=attributes.TargetState, value=NullValue),
                                AttributeValue(endpoint_id=endpoint, attribute=attributes.CurrentState,
                                               value=cluster.Enums.ValveStateEnum.kOpen),
                                AttributeValue(endpoint_id=endpoint, attribute=attributes.TargetLevel, value=NullValue),
                                AttributeValue(endpoint_id=endpoint, attribute=attributes.CurrentLevel, value=default_open_level)]
        attribute_subscription.await_all_final_values_reported(expected_final_values=expected_final_state, timeout_sec=timeout)

        self.step(9)
        target_state_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
        current_state_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
        asserts.assert_equal(current_state_dut, cluster.Enums.ValveStateEnum.kOpen, "CurrentState is not open")
        asserts.assert_equal(target_state_dut, NullValue, "TargetState is not null")

        self.step(10)
        target_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetLevel)
        current_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentLevel)
        asserts.assert_equal(current_level_dut, default_open_level, "CurrentLevel is not defaultOpenLevel")
        asserts.assert_equal(target_level_dut, NullValue, "TargetLevel is not null")

        self.step(11)
        attribute_subscription.reset()
        await self.send_single_cmd(cmd=Clusters.Objects.ValveConfigurationAndControl.Commands.Close(), endpoint=endpoint)

        self.step(12)
        expected_final_state = [AttributeValue(endpoint_id=endpoint, attribute=attributes.TargetState, value=NullValue),
                                AttributeValue(endpoint_id=endpoint, attribute=attributes.CurrentState,
                                               value=cluster.Enums.ValveStateEnum.kClosed),
                                AttributeValue(endpoint_id=endpoint, attribute=attributes.TargetLevel, value=NullValue),
                                AttributeValue(endpoint_id=endpoint, attribute=attributes.CurrentLevel, value=0)]
        attribute_subscription.await_all_final_values_reported(expected_final_values=expected_final_state, timeout_sec=timeout)

        self.step(13)
        target_state_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetState)
        current_state_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentState)
        asserts.assert_equal(current_state_dut, cluster.Enums.ValveStateEnum.kClosed, "CurrentState is not open")
        asserts.assert_equal(target_state_dut, NullValue, "TargetState is not null")

        self.step(14)
        target_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.TargetLevel)
        current_level_dut = await self.read_valcc_attribute_expect_success(endpoint=endpoint, attribute=attributes.CurrentLevel)
        asserts.assert_equal(current_level_dut, 0, "CurrentLevel is not 0")
        asserts.assert_equal(target_level_dut, NullValue, "TargetLevel is not null")


if __name__ == "__main__":
    default_matter_test_main()
