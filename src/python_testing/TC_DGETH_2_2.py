#
#  Copyright (c) 2025 Project CHIP Authors
#  All rights reserved.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

import logging

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 0
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#
import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_DGETH_2_2(MatterBaseTest):
    """
    [TC-DGETH-2.2] Command Received [DUT as Server]

    This test case verifies the behavior of the commands received by the DUT as a server.
    See the test plan steps for details on each command and expected outcome.
    """

    async def read_dgeth_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.EthernetNetworkDiagnostics
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def send_reset_counts_command(self):
        cluster = Clusters.Objects.EthernetNetworkDiagnostics
        return await self.send_single_cmd(cmd=cluster.Commands.ResetCounts(), endpoint=0)

    def desc_TC_DGETH_2_2(self) -> str:
        return "[TC-DGETH-2.2] Command Received [DUT as Server]"

    def pics_TC_DGETH_2_2(self) -> list[str]:
        return ["DGETH.S"]

    def steps_TC_DGETH_2_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read PHYRate 10 times to generate traffic"),
            TestStep(3, "Read initial PacketRxCount"),
            TestStep(4, "Read initial PacketTxCount"),
            TestStep(5, "Read initial TxErrCount"),
            TestStep(6, "Read initial CollisionCount"),
            TestStep(7, "Read initial OverrunCount"),
            TestStep(8, "Send ResetCounts command"),
            TestStep(9, "Verify PacketRxCount reset"),
            TestStep(10, "Verify PacketTxCount reset"),
            TestStep(11, "Verify TxErrCount reset"),
            TestStep(12, "Verify CollisionCount reset"),
            TestStep(13, "Verify OverrunCount reset")
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.Objects.EthernetNetworkDiagnostics))
    async def test_TC_DGETH_2_2(self):
        endpoint = self.get_endpoint()
        attributes = Clusters.EthernetNetworkDiagnostics.Attributes

        # Step 1: Commission DUT (already done)
        self.step(1)

        attribute_list = await self.read_dgeth_attribute_expect_success(endpoint, attributes.AttributeList)

        # Step 2: Generate traffic with PHYRate reads
        self.step(2)
        if self.pics_guard(attributes.PHYRate.attribute_id in attribute_list):
            for _ in range(10):
                await self.read_dgeth_attribute_expect_success(endpoint, attributes.AttributeList)

        # Steps 3-7: Read initial values
        initial_values = {}
        for step, attr in [(3, attributes.PacketRxCount),
                           (4, attributes.PacketTxCount),
                           (5, attributes.TxErrCount),
                           (6, attributes.CollisionCount),
                           (7, attributes.OverrunCount)]:
            self.step(step)
            if self.pics_guard(attr.attribute_id in attribute_list):
                initial_values[attr] = await self.read_dgeth_attribute_expect_success(endpoint, attr)
                logger.info(f"Initial {attr.__name__} value: {initial_values[attr]}")

        # Step 8: Send ResetCounts command
        self.step(8)
        await self.send_reset_counts_command()
        logger.info("Reset counts command sent")

        # Steps 9-13: Verify post-reset values
        verification_map = {
            9: (attributes.PacketRxCount, lambda a, b: a < b),
            10: (attributes.PacketTxCount, lambda a, b: a < b),
            11: (attributes.TxErrCount, lambda a, b: a <= b),
            12: (attributes.CollisionCount, lambda a, b: a <= b),
            13: (attributes.OverrunCount, lambda a, b: a <= b)
        }

        for step, (attr, comparator) in verification_map.items():
            self.step(step)
            if self.pics_guard(attr.attribute_id in attribute_list):
                post_value = await self.read_dgeth_attribute_expect_success(endpoint, attr)
                initial_value = initial_values.get(attr)
                logger.info(f"Post-reset {attr.__name__} value: {post_value} (initial: {initial_value})")

                asserts.assert_true(
                    comparator(post_value, initial_value),
                    f"{attr.__name__} post-reset value {post_value} failed check against initial {initial_value}"
                )


if __name__ == "__main__":
    default_matter_test_main()
