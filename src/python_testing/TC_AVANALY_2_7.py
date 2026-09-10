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
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json --app-pipe /tmp/avanaly_2_7_fifo
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#       --app-pipe /tmp/avanaly_2_7_fifo
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_AVANALYTestBase import AVANALYTestBase

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVANALY_2_7(MatterBaseTest, AVANALYTestBase):

    def desc_TC_AVANALY_2_7(self) -> str:
        return "[TC-AVANALY-2.7] Validate PerceivedContext event generation and context list handling with Server as DUT"

    def steps_TC_AVANALY_2_7(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH enables multiple context triggers (e.g., Context A and Context B). Verify success response."),
            TestStep(3, "Simulate detection of Context A (e.g. Person)."),
            TestStep(
                4,
                "TH waits for PerceivedContext event. Verify NewIdentifiedContexts contains Context A, SessionID matches active session, and conditional SourceNodeID/SourceStartTimestamp.",
            ),
            TestStep(5, "Simulate detection of Context B (e.g. Package) while Context A is still present."),
            TestStep(
                6,
                "TH waits for PerceivedContext event. Verify NewIdentifiedContexts contains Context B, CurrentIdentifiedContexts contains Context A, and SessionID matches active session.",
            ),
            TestStep(7, "Simulate Context A leaving/expiring."),
            TestStep(
                8,
                "TH waits for PerceivedContext event. Verify ExpiredContexts contains Context A, CurrentIdentifiedContexts contains Context B, and SessionID matches active session.",
            ),
            TestStep(
                9,
                "TH disables context triggers by sending DisableContextTriggers command with ContextTriggers set to null. Verify success response.",
            ),
        ]

    def pics_TC_AVANALY_2_7(self) -> list[str]:
        return [
            "AVANALY.S",
            "AVANALY.S.E02",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AvAnalysis))
    async def test_TC_AVANALY_2_7(self):
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        self.step(1)  # Already done, immediately go to step 2

        await self.read_avanaly_features(endpoint)
        supported_contexts = await self.read_avanaly_attribute_expect_success(endpoint, attributes.SupportedAmbientContexts)
        asserts.assert_greater_equal(len(supported_contexts), 1, "SupportedAmbientContexts must not be empty")

        if len(supported_contexts) < 2:
            log.info("DUT does not support at least 2 distinct ambient contexts, skipping TC-AVANALY-2.7")
            for s in range(2, 10):
                self.skip_step(s)
            return

        self.step(2)
        context_a = supported_contexts[0]
        context_b = supported_contexts[1]

        if self.has_feature_perzonedetect:
            triggers = [
                cluster.Structs.ContextTriggerStruct(context=context_a, zoneIDs=NullValue),
                cluster.Structs.ContextTriggerStruct(context=context_b, zoneIDs=NullValue)
            ]
        else:
            triggers = [
                cluster.Structs.ContextTriggerStruct(context=context_a),
                cluster.Structs.ContextTriggerStruct(context=context_b)
            ]

        await self.send_enable_context_triggers_cmd(endpoint, context_triggers=triggers)

        # Set up event subscription handler
        event_callback = EventSubscriptionHandler(expected_cluster=cluster)
        await event_callback.start(self.default_controller, self.dut_node_id, endpoint)

        self.step(3)
        pipe_payload_1 = {
            "Name": "AvAnalysisPerceivedContext",
            "NewContexts": [
                {"NamespaceId": context_a.namespaceID, "Tag": context_a.tag, "IdentifiedContextId": 1}
            ]
        }
        if self.has_feature_remcondetect:
            pipe_payload_1["SourceNodeId"] = self.dut_node_id
        if self.matter_test_config.pipe_name:
            self.write_to_app_pipe(pipe_payload_1)
        elif not self.is_ci:
            self.wait_for_user_input(
                prompt_msg=f"Simulate detection of Context A (namespace=0x{context_a.namespaceID:02X}, tag=0x{context_a.tag:04X}) on the DUT. Press Enter once detected."
            )

        self.step(4)
        if self.matter_test_config.pipe_name or not self.is_ci:
            event1 = event_callback.wait_for_event_report(cluster.Events.PerceivedContext, timeout_sec=30)
            log.info("PerceivedContext event 1: %s", event1)
            asserts.assert_is_not_none(event1, "Expected PerceivedContext event")
            active_session_id = event1.sessionID
            asserts.assert_is_not_none(active_session_id, "sessionID must be present in event 1")
            asserts.assert_greater_equal(active_session_id, 0, "sessionID must be a valid uint16")
            asserts.assert_less_equal(active_session_id, 0xFFFF, "sessionID must be a valid uint16")
            if self.has_feature_remcondetect:
                asserts.assert_is_not_none(event1.sourceNodeId,
                                           "SourceNodeId must be present when REMCONDETECT is supported")
                asserts.assert_equal(event1.sourceNodeId, self.dut_node_id,
                                     f"SourceNodeId ({event1.sourceNodeId}) must match source camera NodeID ({self.dut_node_id})")
                if event1.sourceStartTimestamp is not None:
                    log.info("SourceStartTimestamp present in event 1: %s", event1.sourceStartTimestamp)
                else:
                    log.warning("SourceStartTimestamp not provided by DUT in event 1")
            asserts.assert_is_not_none(event1.newIdentifiedContexts, "newIdentifiedContexts must be present in event 1")
            new_tags_1 = [(tc.identifiedContext.namespaceID, tc.identifiedContext.tag) for tc in event1.newIdentifiedContexts]
            asserts.assert_in((context_a.namespaceID, context_a.tag), new_tags_1,
                              "Context A not found in newIdentifiedContexts of event 1")
        else:
            log.info("CI mode: skipping blocking event wait in Step 4")
            active_session_id = 0

        self.step(5)
        pipe_payload_2 = {
            "Name": "AvAnalysisPerceivedContext",
            "SessionId": active_session_id,
            "NewContexts": [
                {"NamespaceId": context_b.namespaceID, "Tag": context_b.tag, "IdentifiedContextId": 2}
            ]
        }
        if self.has_feature_remcondetect:
            pipe_payload_2["SourceNodeId"] = self.dut_node_id
        if self.matter_test_config.pipe_name:
            self.write_to_app_pipe(pipe_payload_2)
        elif not self.is_ci:
            self.wait_for_user_input(
                prompt_msg=f"Simulate detection of Context B (namespace=0x{context_b.namespaceID:02X}, tag=0x{context_b.tag:04X}) on the DUT while Context A remains present. Press Enter once detected."
            )

        self.step(6)
        if self.matter_test_config.pipe_name or not self.is_ci:
            event2 = event_callback.wait_for_event_report(cluster.Events.PerceivedContext, timeout_sec=30)
            log.info("PerceivedContext event 2: %s", event2)
            asserts.assert_is_not_none(event2, "Expected PerceivedContext event")
            asserts.assert_equal(event2.sessionID, active_session_id,
                                 f"SessionID in event 2 ({event2.sessionID}) does not match active session ({active_session_id})")
            if self.has_feature_remcondetect:
                asserts.assert_is_not_none(event2.sourceNodeId,
                                           "SourceNodeId must be present when REMCONDETECT is supported")
                asserts.assert_equal(event2.sourceNodeId, self.dut_node_id,
                                     f"SourceNodeId ({event2.sourceNodeId}) must match source camera NodeID ({self.dut_node_id})")
                if event2.sourceStartTimestamp is not None:
                    log.info("SourceStartTimestamp present in event 2: %s", event2.sourceStartTimestamp)
                else:
                    log.warning("SourceStartTimestamp not provided by DUT in event 2")
            asserts.assert_is_not_none(event2.newIdentifiedContexts, "newIdentifiedContexts must be present in event 2")
            new_tags_2 = [(tc.identifiedContext.namespaceID, tc.identifiedContext.tag) for tc in event2.newIdentifiedContexts]
            asserts.assert_in((context_b.namespaceID, context_b.tag), new_tags_2,
                              "Context B not found in newIdentifiedContexts of event 2")
            asserts.assert_is_not_none(event2.currentIdentifiedContexts,
                                       "currentIdentifiedContexts must be present in event 2")
            current_tags_2 = [(tc.identifiedContext.namespaceID, tc.identifiedContext.tag)
                              for tc in event2.currentIdentifiedContexts]
            asserts.assert_in((context_a.namespaceID, context_a.tag), current_tags_2,
                              "Context A not found in currentIdentifiedContexts of event 2")
        else:
            log.info("CI mode: skipping blocking event wait in Step 6")

        self.step(7)
        pipe_payload_3 = {
            "Name": "AvAnalysisPerceivedContext",
            "SessionId": active_session_id,
            "ExpiredContexts": [
                {"NamespaceId": context_a.namespaceID, "Tag": context_a.tag, "IdentifiedContextId": 1}
            ]
        }
        if self.has_feature_remcondetect:
            pipe_payload_3["SourceNodeId"] = self.dut_node_id
        if self.matter_test_config.pipe_name:
            self.write_to_app_pipe(pipe_payload_3)
        elif not self.is_ci:
            self.wait_for_user_input(
                prompt_msg=f"Simulate Context A (namespace=0x{context_a.namespaceID:02X}, tag=0x{context_a.tag:04X}) leaving/expiring while Context B remains. Press Enter once expired."
            )

        self.step(8)
        if self.matter_test_config.pipe_name or not self.is_ci:
            event3 = event_callback.wait_for_event_report(cluster.Events.PerceivedContext, timeout_sec=30)
            log.info("PerceivedContext event 3: %s", event3)
            asserts.assert_is_not_none(event3, "Expected PerceivedContext event")
            asserts.assert_equal(event3.sessionID, active_session_id,
                                 f"SessionID in event 3 ({event3.sessionID}) does not match active session ({active_session_id})")
            if self.has_feature_remcondetect:
                asserts.assert_is_not_none(event3.sourceNodeId,
                                           "SourceNodeId must be present when REMCONDETECT is supported")
                asserts.assert_equal(event3.sourceNodeId, self.dut_node_id,
                                     f"SourceNodeId ({event3.sourceNodeId}) must match source camera NodeID ({self.dut_node_id})")
                if event3.sourceStartTimestamp is not None:
                    log.info("SourceStartTimestamp present in event 3: %s", event3.sourceStartTimestamp)
                else:
                    log.warning("SourceStartTimestamp not provided by DUT in event 3")
            asserts.assert_is_not_none(event3.expiredContexts, "expiredContexts must be present in event 3")
            expired_tags_3 = [(tc.identifiedContext.namespaceID, tc.identifiedContext.tag) for tc in event3.expiredContexts]
            asserts.assert_in((context_a.namespaceID, context_a.tag), expired_tags_3,
                              "Context A not found in expiredContexts of event 3")
            asserts.assert_is_not_none(event3.currentIdentifiedContexts,
                                       "currentIdentifiedContexts must be present in event 3")
            current_tags_3 = [(tc.identifiedContext.namespaceID, tc.identifiedContext.tag)
                              for tc in event3.currentIdentifiedContexts]
            asserts.assert_in((context_b.namespaceID, context_b.tag), current_tags_3,
                              "Context B not found in currentIdentifiedContexts of event 3")
        else:
            log.info("CI mode: skipping blocking event wait in Step 8")

        self.step(9)
        # Cleanup: end analysis session and disable all context triggers
        if self.matter_test_config.pipe_name:
            cleanup_payload = {"Name": "AvAnalysisSessionEnd", "SessionId": active_session_id}
            if self.has_feature_remcondetect:
                cleanup_payload["SourceNodeId"] = self.dut_node_id
            self.write_to_app_pipe(cleanup_payload)
        await self.send_disable_context_triggers_cmd(endpoint, context_triggers=NullValue)


if __name__ == "__main__":
    default_matter_test_main()
