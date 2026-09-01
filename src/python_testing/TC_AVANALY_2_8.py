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
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVANALY_2_8(MatterBaseTest, AVANALYTestBase):

    def desc_TC_AVANALY_2_8(self) -> str:
        return "[TC-AVANALY-2.8] Validate IdentifiedContextID consistency when tracking is enabled with Server as DUT"

    def steps_TC_AVANALY_2_8(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH sets TrackingEnabled attribute to True. Verify success response."),
            TestStep(3, "Simulate detection of an entity (e.g., Person)."),
            TestStep(4, "TH receives PerceivedContext event. Save IdentifiedContextID as id1."),
            TestStep(5, "Simulate the entity moving and being detected again (e.g., across zones)."),
            TestStep(6, "TH receives PerceivedContext event. Verify IdentifiedContextID is equal to id1."),
        ]

    def pics_TC_AVANALY_2_8(self) -> list[str]:
        return [
            "AVANALY.S",
            "AVANALY.S.A0005",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AvAnalysis))
    async def test_TC_AVANALY_2_8(self):
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        self.step(1)  # Already done, immediately go to step 2

        await self.read_avanaly_features(endpoint)
        supported_contexts = await self.read_avanaly_attribute_expect_success(endpoint, attributes.SupportedAmbientContexts)
        asserts.assert_greater_equal(len(supported_contexts), 1, "SupportedAmbientContexts must not be empty")

        self.step(2)
        # Enable tracking
        result = await self.write_single_attribute(attributes.TrackingEnabled(True), endpoint_id=endpoint)
        asserts.assert_equal(result, Status.Success, "Writing TrackingEnabled to True failed")

        # Also ensure at least one context trigger is enabled
        if self.has_feature_perzonedetect:
            trigger = cluster.Structs.ContextTriggerStruct(context=supported_contexts[0], zoneIDs=NullValue)
        else:
            trigger = cluster.Structs.ContextTriggerStruct(context=supported_contexts[0])
        await self.send_enable_context_triggers_cmd(endpoint, context_triggers=[trigger])

        # Set up event subscription handler
        event_callback = EventSubscriptionHandler(expected_cluster=cluster)
        await event_callback.start(self.default_controller, self.dut_node_id, endpoint)

        self.step(3)
        if not self.is_ci:
            self.wait_for_user_input(
                prompt_msg="Simulate detection of a tracked entity (e.g., Person) on the DUT. Press Enter once detected."
            )

        self.step(4)
        id1 = None
        if not self.is_ci:
            event1 = event_callback.wait_for_event_report(cluster.Events.PerceivedContext, timeout_sec=30)
            log.info("PerceivedContext event 1: %s", event1)
            asserts.assert_is_not_none(event1, "Expected PerceivedContext event")
            asserts.assert_is_not_none(event1.newIdentifiedContexts, "newIdentifiedContexts must be present in event 1")
            asserts.assert_greater_equal(len(event1.newIdentifiedContexts), 1, "Expected at least 1 TrackedContext entry")
            id1 = event1.newIdentifiedContexts[0].identifiedContextID
            log.info("Saved IdentifiedContextID id1: %s", id1)
            asserts.assert_is_not_none(id1, "IdentifiedContextID must not be None")
        else:
            log.info("CI mode: skipping blocking event wait in Step 4")
            id1 = 1

        self.step(5)
        if not self.is_ci:
            self.wait_for_user_input(
                prompt_msg="Simulate the entity moving and being detected again (e.g., across zones). Press Enter once detected."
            )

        self.step(6)
        if not self.is_ci:
            event2 = event_callback.wait_for_event_report(cluster.Events.PerceivedContext, timeout_sec=30)
            log.info("PerceivedContext event 2: %s", event2)
            asserts.assert_is_not_none(event2, "Expected PerceivedContext event")
            tracked_list = event2.newIdentifiedContexts or event2.currentIdentifiedContexts
            asserts.assert_is_not_none(tracked_list, "Tracked contexts list must not be None")
            matching_ids = [tc.identifiedContextID for tc in tracked_list if tc.identifiedContextID == id1]
            asserts.assert_greater_equal(len(matching_ids), 1,
                                         f"IdentifiedContextID {id1} not found in second PerceivedContext event: {tracked_list}")
        else:
            log.info("CI mode: skipping blocking event wait in Step 6")

        # Cleanup: reset TrackingEnabled and disable context triggers
        await self.write_single_attribute(attributes.TrackingEnabled(False), endpoint_id=endpoint)
        await self.send_disable_context_triggers_cmd(endpoint, context_triggers=NullValue)


if __name__ == "__main__":
    default_matter_test_main()
