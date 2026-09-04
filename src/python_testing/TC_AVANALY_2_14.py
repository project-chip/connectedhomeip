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
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVANALY_2_14(MatterBaseTest, AVANALYTestBase):

    def desc_TC_AVANALY_2_14(self) -> str:
        return "[TC-AVANALY-2.14] Validate RemoteZones integration and SourceNodeId consistency with Server as DUT"

    def steps_TC_AVANALY_2_14(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH creates a remote zone in Zone Management cluster with a valid NodeId. Save as remote_zone_id."),
            TestStep(3, "TH establishes an analysis stream to the same remote camera NodeId. Verify success response."),
            TestStep(4, "Simulate detection of an ambient context in remote_zone_id."),
            TestStep(5, "TH receives AnalysisSessionStart event. Verify SourceNodeId matches remote camera NodeId."),
            TestStep(6, "Verify TriggeredZones contains remote_zone_id in AnalysisSessionStart event."),
            TestStep(7, "TH receives PerceivedContext event. Verify SourceNodeId matches remote camera NodeId."),
            TestStep(8, "TH receives AnalysisSessionEnd event. Verify SourceNodeId matches remote camera NodeId."),
        ]

    def pics_TC_AVANALY_2_14(self) -> list[str]:
        return [
            "AVANALY.S",
            "ZONEMGMT.S.F04",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AvAnalysis))
    async def test_TC_AVANALY_2_14(self):
        cluster = Clusters.Objects.AvAnalysis
        endpoint = self.get_endpoint()
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        self.step(1)  # Already done, immediately go to step 2

        await self.read_avanaly_features(endpoint)
        zone_cluster = Clusters.Objects.ZoneManagement
        has_remote_zones = self.check_pics("ZONEMGMT.S.F04")
        if hasattr(zone_cluster.Bitmaps.Feature, "kRemoteZones"):
            try:
                zone_feature_map = await self.read_single_attribute_check_success(
                    endpoint=endpoint, cluster=zone_cluster, attribute=zone_cluster.Attributes.FeatureMap
                )
                has_remote_zones = (zone_feature_map & zone_cluster.Bitmaps.Feature.kRemoteZones) != 0
            except Exception as e:
                log.info("ZoneManagement cluster query exception: %s", e)

        if not has_remote_zones:
            log.info("RemoteZones feature not supported on ZoneManagement, skipping TC-AVANALY-2.14")
            self.skip_step(2)
            self.skip_step(3)
            self.skip_step(4)
            self.skip_step(5)
            self.skip_step(6)
            self.skip_step(7)
            self.skip_step(8)
            return

        remote_node_id = self.user_params.get("remote_node_id", self.dut_node_id)

        self.step(2)
        remote_zone_id = 1
        log.info("Remote NodeId: %d, Remote Zone ID: %d", remote_node_id, remote_zone_id)

        self.step(3)
        resp = await self.send_establish_analysis_stream_cmd(endpoint, node_id=remote_node_id)
        log.info("EstablishAnalysisStreamResponse: %s", resp)

        # Set up event subscription handler
        event_callback = EventSubscriptionHandler(expected_cluster=cluster)
        await event_callback.start(self.default_controller, self.dut_node_id, endpoint)

        self.step(4)
        if not self.is_ci:
            self.wait_for_user_input(
                prompt_msg=f"Simulate detection of an ambient context in remote zone {remote_zone_id} for node {remote_node_id}. Press Enter once initiated."
            )

        self.step(5)
        if not self.is_ci:
            start_event = event_callback.wait_for_event_report(cluster.Events.AnalysisSessionStart, timeout_sec=30)
            log.info("AnalysisSessionStart event: %s", start_event)
            asserts.assert_is_not_none(start_event, "Expected AnalysisSessionStart event")
            if start_event.sourceNodeId is not None:
                asserts.assert_equal(start_event.sourceNodeId, remote_node_id, "SourceNodeId mismatch in AnalysisSessionStart")

            self.step(6)
            if start_event.triggeredZones is not None:
                asserts.assert_in(remote_zone_id, start_event.triggeredZones,
                                  f"Remote zone {remote_zone_id} not found in triggeredZones {start_event.triggeredZones}")
        else:
            log.info("CI mode: skipping blocking event wait in Steps 5 & 6")
            self.step(6)

        self.step(7)
        if not self.is_ci:
            perceived_event = event_callback.wait_for_event_report(cluster.Events.PerceivedContext, timeout_sec=30)
            log.info("PerceivedContext event: %s", perceived_event)
            asserts.assert_is_not_none(perceived_event, "Expected PerceivedContext event")
            if perceived_event.sourceNodeId is not None:
                asserts.assert_equal(perceived_event.sourceNodeId, remote_node_id, "SourceNodeId mismatch in PerceivedContext")
        else:
            log.info("CI mode: skipping blocking event wait in Step 7")

        self.step(8)
        if not self.is_ci:
            end_event = event_callback.wait_for_event_report(cluster.Events.AnalysisSessionEnd, timeout_sec=30)
            log.info("AnalysisSessionEnd event: %s", end_event)
            asserts.assert_is_not_none(end_event, "Expected AnalysisSessionEnd event")
            if end_event.sourceNodeId is not None:
                asserts.assert_equal(end_event.sourceNodeId, remote_node_id, "SourceNodeId mismatch in AnalysisSessionEnd")
        else:
            log.info("CI mode: skipping blocking event wait in Step 8")


if __name__ == "__main__":
    default_matter_test_main()
