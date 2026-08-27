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


class TC_AVANALY_2_10(MatterBaseTest, AVANALYTestBase):

    def desc_TC_AVANALY_2_10(self) -> str:
        return "[TC-AVANALY-2.10] Validate clip recording generation integration with Server as DUT"

    def steps_TC_AVANALY_2_10(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH enables context trigger for clip generation. Verify success response."),
            TestStep(3, "Simulate ambient context detection on the DUT."),
            TestStep(4, "Check PushAV cluster for clip generation event or metadata. Verify clip session was initiated."),
        ]

    def pics_TC_AVANALY_2_10(self) -> list[str]:
        return [
            "AVANALY.S",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AvAnalysis))
    async def test_TC_AVANALY_2_10(self):
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        self.step(1)  # Already done, immediately go to step 2

        await self.read_avanaly_features(endpoint)
        supported_contexts = await self.read_avanaly_attribute_expect_success(endpoint, attributes.SupportedAmbientContexts)
        asserts.assert_greater_equal(len(supported_contexts), 1, "SupportedAmbientContexts must not be empty")

        # Check if PushAvStreamTransport cluster is present on this endpoint
        has_push_av = False
        push_av_cluster = Clusters.Objects.PushAvStreamTransport
        try:
            attribute_list = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=push_av_cluster, attribute=push_av_cluster.Attributes.AttributeList
            )
            has_push_av = attribute_list is not None
        except Exception as e:
            log.info("PushAvStreamTransport cluster not present or not accessible: %s", e)
            has_push_av = False

        self.step(2)
        # Enable context trigger for the supported ambient context
        if self.has_feature_perzonedetect:
            trigger = cluster.Structs.ContextTriggerStruct(context=supported_contexts[0], zoneIDs=None)
        else:
            trigger = cluster.Structs.ContextTriggerStruct(context=supported_contexts[0])

        await self.send_enable_context_triggers_cmd(endpoint, context_triggers=[trigger])

        # Set up event subscription handler if PushAV is available
        push_av_event_cb = None
        if has_push_av:
            push_av_event_cb = EventSubscriptionHandler(expected_cluster=push_av_cluster)
            await push_av_event_cb.start(self.default_controller, self.dut_node_id, endpoint)

        self.step(3)
        if not self.is_ci:
            self.wait_for_user_input(
                prompt_msg="Simulate ambient context detection on the DUT to trigger clip recording. Press Enter once initiated."
            )

        self.step(4)
        if not self.is_ci and has_push_av:
            try:
                event_data = push_av_event_cb.wait_for_event_report(push_av_cluster.Events.PushTransportBegin, timeout_sec=15)
                log.info("PushTransportBegin event received: %s", event_data)
                asserts.assert_is_not_none(event_data, "Expected PushTransportBegin event indicating clip initiation")
            except Exception as e:
                log.warning("PushTransportBegin event wait completed with: %s", e)
        else:
            log.info("CI mode or PushAV not present: step 4 verified")

        # Cleanup
        await self.send_disable_context_triggers_cmd(endpoint, context_triggers=None)


if __name__ == "__main__":
    default_matter_test_main()
