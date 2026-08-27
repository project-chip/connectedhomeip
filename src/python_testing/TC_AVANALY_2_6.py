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
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVANALY_2_6(MatterBaseTest, AVANALYTestBase):

    def desc_TC_AVANALY_2_6(self) -> str:
        return "[TC-AVANALY-2.6] Validate AnalysisSessionStart and AnalysisSessionEnd event generation with Server as DUT"

    def steps_TC_AVANALY_2_6(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH enables a context trigger. Verify success response."),
            TestStep(3, "Simulate the detection of the enabled ambient context on the DUT."),
            TestStep(4, "TH waits for AnalysisSessionStart event. Verify event received and save SessionID."),
            TestStep(5, "Simulate the end of the ambient context detection on the DUT."),
            TestStep(6, "TH waits for AnalysisSessionEnd event. Verify event received with matching SessionID."),
        ]

    def pics_TC_AVANALY_2_6(self) -> list[str]:
        return [
            "AVANALY.S",
            "AVANALY.S.E00",
            "AVANALY.S.E01",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AvAnalysis))
    async def test_TC_AVANALY_2_6(self):
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        self.step(1)  # Already done, immediately go to step 2

        await self.read_avanaly_features(endpoint)
        supported_contexts = await self.read_avanaly_attribute_expect_success(endpoint, attributes.SupportedAmbientContexts)
        asserts.assert_greater_equal(len(supported_contexts), 1, "SupportedAmbientContexts must not be empty")

        self.step(2)
        # Enable context trigger for the first supported ambient context
        context_to_enable = supported_contexts[0]
        if self.has_feature_perzonedetect:
            trigger = cluster.Structs.ContextTriggerStruct(context=context_to_enable, zoneIDs=None)
        else:
            trigger = cluster.Structs.ContextTriggerStruct(context=context_to_enable)

        await self.send_enable_context_triggers_cmd(endpoint, context_triggers=[trigger])

        # Set up event subscription handler
        event_callback = EventSubscriptionHandler(expected_cluster=cluster)
        await event_callback.start(self.default_controller, self.dut_node_id, endpoint)

        self.step(3)
        if not self.is_ci:
            self.wait_for_user_input(
                prompt_msg=f"Simulate detection of enabled ambient context (namespace=0x{context_to_enable.namespaceID:02X}, tag=0x{context_to_enable.tag:04X}) on the DUT. Press Enter once initiated."
            )

        self.step(4)
        if not self.is_ci:
            start_event_data = event_callback.wait_for_event_report(cluster.Events.AnalysisSessionStart, timeout_sec=30)
            log.info("AnalysisSessionStart event received: %s", start_event_data)
            asserts.assert_is_not_none(start_event_data, "Expected AnalysisSessionStart event")
            session_id = start_event_data.sessionID
            asserts.assert_is_not_none(session_id, "AnalysisSessionStart must contain sessionID")
        else:
            log.info("CI mode: skipping blocking event wait in Step 4")
            session_id = 0

        self.step(5)
        if not self.is_ci:
            self.wait_for_user_input(
                prompt_msg="Simulate the end of the ambient context detection on the DUT. Press Enter once completed."
            )

        self.step(6)
        if not self.is_ci:
            end_event_data = event_callback.wait_for_event_report(cluster.Events.AnalysisSessionEnd, timeout_sec=30)
            log.info("AnalysisSessionEnd event received: %s", end_event_data)
            asserts.assert_is_not_none(end_event_data, "Expected AnalysisSessionEnd event")
            asserts.assert_equal(end_event_data.sessionID, session_id,
                                 f"SessionID in AnalysisSessionEnd ({end_event_data.sessionID}) does not match AnalysisSessionStart ({session_id})")
        else:
            log.info("CI mode: skipping blocking event wait in Step 6")

        # Cleanup: disable context triggers
        await self.send_disable_context_triggers_cmd(endpoint, context_triggers=None)


if __name__ == "__main__":
    default_matter_test_main()
