#
#    Copyright (c) 2026 Project CHIP Authors
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_AVANALYTestBase import AVANALYTestBase

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVANALY_2_4(MatterBaseTest, AVANALYTestBase):

    def desc_TC_AVANALY_2_4(self) -> str:
        return "[TC-AVANALY-2.4] Validate EnableContextTriggers and DisableContextTriggers functionality with remote context with Server as DUT"

    def steps_TC_AVANALY_2_4(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH sends EnableContextTriggers command without an analysis stream established. Verify INVALID_IN_STATE error."),
            TestStep(3, "TH sends EstablishAnalysisStream command with a valid NodeID. Verify success response."),
            TestStep(4, "TH reads SupportedAmbientContexts attribute. Save as supported_contexts."),
            TestStep(5, "TH sends EnableContextTriggers command with valid subset of supported_contexts. Verify success."),
            TestStep(6, "TH reads ActiveAmbientContextTriggers attribute. Verify it matches the provided list."),
            TestStep(7, "TH sends DisableContextTriggers command with ContextTriggers set to null. Verify success."),
            TestStep(8, "TH reads ActiveAmbientContextTriggers attribute. Verify it is empty."),
        ]

    def pics_TC_AVANALY_2_4(self) -> list[str]:
        return [
            "AVANALY.S",
            "AVANALY.S.F01",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AvAnalysis))
    async def test_TC_AVANALY_2_4(self):
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()

        self.step(1)  # Already done, immediately go to step 2

        await self.read_avanaly_features(endpoint)
        log.info("Features - LCLCONDETECT: %s, REMCONDETECT: %s, PERZONEDETECT: %s",
                 self.has_feature_lclcondetect, self.has_feature_remcondetect, self.has_feature_perzonedetect)

        if not self.has_feature_remcondetect:
            log.info("REMCONDETECT not supported, skipping TC-AVANALY-2.4")
            self.skip_step(2)
            self.skip_step(3)
            self.skip_step(4)
            self.skip_step(5)
            self.skip_step(6)
            self.skip_step(7)
            self.skip_step(8)
            return

        self.step(2)
        # TH sends EnableContextTriggers command before establishing analysis stream
        # Expect INVALID_IN_STATE because no stream is active
        try:
            await self.send_enable_context_triggers_cmd(endpoint, context_triggers=NullValue)
            asserts.fail("EnableContextTriggers should fail with INVALID_IN_STATE when no stream is active")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidInState,
                                 f"Expected INVALID_IN_STATE status, received {e.status}")

        self.step(3)
        # Establish analysis stream with TH node id or valid NodeID
        node_id = self.dut_node_id
        resp = await self.send_establish_analysis_stream_cmd(endpoint, node_id=node_id)
        log.info("EstablishAnalysisStream response: %s", resp)

        self.step(4)
        supported_contexts = await self.read_avanaly_attribute_expect_success(endpoint, attributes.SupportedAmbientContexts)
        asserts.assert_greater_equal(len(supported_contexts), 1, "SupportedAmbientContexts must not be empty")

        self.step(5)
        valid_subset = [supported_contexts[0]]
        if self.has_feature_perzonedetect:
            triggers = [cluster.Structs.ContextTriggerStruct(context=sc, zoneIDs=NullValue) for sc in valid_subset]
        else:
            triggers = [cluster.Structs.ContextTriggerStruct(context=sc) for sc in valid_subset]

        await self.send_enable_context_triggers_cmd(endpoint, context_triggers=triggers)

        self.step(6)
        active_triggers = await self.read_avanaly_attribute_expect_success(endpoint, attributes.ActiveAmbientContextTriggers)
        active_tags = {(t.context.namespaceID, t.context.tag) for t in active_triggers}
        asserts.assert_in((valid_subset[0].namespaceID, valid_subset[0].tag), active_tags,
                          "Enabled context not found in ActiveAmbientContextTriggers")

        self.step(7)
        await self.send_disable_context_triggers_cmd(endpoint, context_triggers=NullValue)

        self.step(8)
        active_triggers = await self.read_avanaly_attribute_expect_success(endpoint, attributes.ActiveAmbientContextTriggers)
        asserts.assert_equal(len(active_triggers), 0,
                             "ActiveAmbientContextTriggers should be empty after null disable")


if __name__ == "__main__":
    default_matter_test_main()
