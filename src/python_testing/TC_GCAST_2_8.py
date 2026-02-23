#
#    Copyright (c) 2025 Project CHIP Authors
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
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
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
#       --endpoint 0
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_GCAST_common import generate_fabric_under_test_matcher, get_feature_map

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_GCAST_2_8(MatterBaseTest):
    def desc_TC_GCAST_2_8(self):
        return "[TC-GCAST-2.8] GroupcastTesting command effect with DUT as Server - PROVISIONAL"

    def steps_TC_GCAST_2_8(self):
        return [
            TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep("1b", "TH subscribes to FabricUnderTest attribute with min interval 0s and max interval 30s"),
            TestStep("1c", "TH reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute"),
            TestStep(2, "TH reads FabricUnderTest attribute"),
            TestStep(3, "Enable a test operation. GroupcastTesting (TestOperation='see notes')"),
            TestStep(4, "TH awaits subscription report of new FabricUnderTest attribute. (value == F1)"),
            TestStep(5, "Disable testing. GroupcastTesting (TestOperation=DisableTesting)"),
            TestStep(6, "TH awaits subscription report of new FabricUnderTest attribute. (value == 0)"),
            TestStep(7, "Enable a test operation for 10 seconds. GroupcastTesting (TestOperation='see notes', DurationSeconds=10)"),
            TestStep(8, "TH awaits subscription report of new FabricUnderTest attribute. (value == F1)"),
            TestStep(9, "TH awaits subscription report of new FabricUnderTest attribute after DurationSeconds of step 7 has elapsed. (value == 0)"),
        ]

    def pics_TC_GCAST_2_8(self) -> list[str]:
        return ["GCAST.S"]

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GCAST_2_8(self):
        groupcast_cluster = Clusters.Objects.Groupcast
        fabricUnderTest_attribute = Clusters.Groupcast.Attributes.FabricUnderTest

        self.step("1a")
        ln_enabled, sd_enabled, pga_enabled = await get_feature_map(self)

        self.step("1b")
        sub = AttributeSubscriptionHandler(groupcast_cluster, fabricUnderTest_attribute)
        await sub.start(self.default_controller, self.dut_node_id, self.get_endpoint(), min_interval_sec=0, max_interval_sec=30)

        self.step("1c")
        currentFabricIndex = await self.read_single_attribute_check_success(
            cluster=Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        )
        F1 = currentFabricIndex

        self.step(2)
        fabricUnderTest = await self.read_single_attribute_check_success(groupcast_cluster, fabricUnderTest_attribute)
        asserts.assert_equal(fabricUnderTest, 0, "FabricUnderTest attribute should be 0 (testing disabled)")

        self.step(3)
        if ln_enabled:
            testOperation = Clusters.Groupcast.Enums.GroupcastTestingEnum.kEnableListenerTesting
        else:
            testOperation = Clusters.Groupcast.Enums.GroupcastTestingEnum.kEnableSenderTesting

        await self.send_single_cmd(Clusters.Groupcast.Commands.GroupcastTesting(
            testOperation=testOperation)
        )

        self.step(4)
        sub.reset()
        fabric_matcher = generate_fabric_under_test_matcher(F1)
        sub.await_all_expected_report_matches(expected_matchers=[fabric_matcher], timeout_sec=60)

        self.step(5)
        await self.send_single_cmd(Clusters.Groupcast.Commands.GroupcastTesting(
            testOperation=Clusters.Groupcast.Enums.GroupcastTestingEnum.kDisableTesting)
        )

        self.step(6)
        sub.reset()
        fabric_matcher = generate_fabric_under_test_matcher(0)
        sub.await_all_expected_report_matches(expected_matchers=[fabric_matcher], timeout_sec=60)

        self.step(7)
        durationSeconds = 10
        await self.send_single_cmd(Clusters.Groupcast.Commands.GroupcastTesting(
            testOperation=testOperation,
            durationSeconds=durationSeconds)
        )

        self.step(8)
        sub.reset()
        fabric_matcher = generate_fabric_under_test_matcher(F1)
        sub.await_all_expected_report_matches(expected_matchers=[fabric_matcher], timeout_sec=60)

        self.step(9)
        sub.reset()
        fabric_matcher = generate_fabric_under_test_matcher(0)
        sub.await_all_expected_report_matches(expected_matchers=[fabric_matcher], timeout_sec=60)


if __name__ == "__main__":
    default_matter_test_main()
