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
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVANALY_2_5(MatterBaseTest, AVANALYTestBase):

    def desc_TC_AVANALY_2_5(self) -> str:
        return "[TC-AVANALY-2.5] Validate EstablishAnalysisStream functionality with Server as DUT"

    def steps_TC_AVANALY_2_5(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads MaxAnalysisStreamCount attribute. Save as max_streams."),
            TestStep(3, "TH reads CurrentAnalysisStreamCount attribute. Save as current_streams."),
            TestStep(4, "TH sends EstablishAnalysisStream command with a valid NodeID. Verify EstablishAnalysisStreamResponse with AnalysisStreamID."),
            TestStep(5, "TH reads CurrentAnalysisStreamCount attribute. Value is current_streams + 1."),
            TestStep(6, "TH reads AnalysisStreams attribute. Verify new entry exists with state PendingInitiation."),
            TestStep(7, "TH sends ActivateAnalysisStream command for the new stream. Verify success response."),
            TestStep(8, "TH reads AnalysisStreams attribute. Verify state is WebRTCActive or PushAVActive."),
            TestStep(9, "TH sends DeactivateAnalysisStream command. Verify success response."),
            TestStep(10, "TH reads AnalysisStreams attribute. Verify state returns to PendingInitiation."),
            TestStep(11, "TH sends RemoveAnalysisStream command. Verify success response."),
            TestStep(12, "TH reads CurrentAnalysisStreamCount attribute. Value is current_streams."),
        ]

    def pics_TC_AVANALY_2_5(self) -> list[str]:
        return [
            "AVANALY.S",
            "AVANALY.S.F01",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AvAnalysis))
    async def test_TC_AVANALY_2_5(self):
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        enums = cluster.Enums
        endpoint = self.get_endpoint()

        self.step(1)  # Already done, immediately go to step 2

        await self.read_avanaly_features(endpoint)
        log.info("Features - LCLCONDETECT: %s, REMCONDETECT: %s, PERZONEDETECT: %s",
                 self.has_feature_lclcondetect, self.has_feature_remcondetect, self.has_feature_perzonedetect)

        if not self.has_feature_remcondetect:
            log.info("REMCONDETECT not supported, skipping TC-AVANALY-2.5")
            self.skip_step(2)
            self.skip_step(3)
            self.skip_step(4)
            self.skip_step(5)
            self.skip_step(6)
            self.skip_step(7)
            self.skip_step(8)
            self.skip_step(9)
            self.skip_step(10)
            self.skip_step(11)
            self.skip_step(12)
            return

        self.step(2)
        max_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.MaxAnalysisStreamCount)
        log.info("MaxAnalysisStreamCount: %d", max_streams)
        asserts.assert_greater_equal(max_streams, 1, "MaxAnalysisStreamCount must be at least 1")

        self.step(3)
        current_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.CurrentAnalysisStreamCount)
        log.info("CurrentAnalysisStreamCount: %d", current_streams)

        self.step(4)
        node_id = self.dut_node_id
        resp = await self.send_establish_analysis_stream_cmd(endpoint, node_id=node_id)
        log.info("EstablishAnalysisStreamResponse: %s", resp)
        asserts.assert_is_not_none(resp, "Expected EstablishAnalysisStreamResponse")
        stream_id = resp.analysisStreamID
        asserts.assert_is_not_none(stream_id, "Response must contain analysisStreamID")

        self.step(5)
        new_current_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.CurrentAnalysisStreamCount)
        asserts.assert_equal(new_current_streams, current_streams + 1,
                             f"Expected CurrentAnalysisStreamCount to be {current_streams + 1}, got {new_current_streams}")

        self.step(6)
        analysis_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.AnalysisStreams)
        matching_streams = [s for s in analysis_streams if s.analysisStreamID == stream_id]
        asserts.assert_equal(len(matching_streams), 1, f"AnalysisStream with ID {stream_id} not found in AnalysisStreams")
        asserts.assert_equal(matching_streams[0].analysisStreamState, enums.AnalysisStreamStateEnum.kPendingInitiation,
                             "Expected stream state to be PendingInitiation")

        self.step(7)
        await self.send_activate_analysis_stream_cmd(endpoint, analysis_stream_id=stream_id)

        self.step(8)
        analysis_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.AnalysisStreams)
        matching_streams = [s for s in analysis_streams if s.analysisStreamID == stream_id]
        asserts.assert_equal(len(matching_streams), 1, f"AnalysisStream with ID {stream_id} not found in AnalysisStreams")
        valid_active_states = [enums.AnalysisStreamStateEnum.kWebRTCActive, enums.AnalysisStreamStateEnum.kPushAVActive]
        asserts.assert_in(matching_streams[0].analysisStreamState, valid_active_states,
                          f"Expected stream state to be WebRTCActive or PushAVActive, got {matching_streams[0].analysisStreamState}")

        self.step(9)
        await self.send_deactivate_analysis_stream_cmd(endpoint, analysis_stream_id=stream_id)

        self.step(10)
        analysis_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.AnalysisStreams)
        matching_streams = [s for s in analysis_streams if s.analysisStreamID == stream_id]
        asserts.assert_equal(len(matching_streams), 1, f"AnalysisStream with ID {stream_id} not found in AnalysisStreams")
        asserts.assert_equal(matching_streams[0].analysisStreamState, enums.AnalysisStreamStateEnum.kPendingInitiation,
                             "Expected stream state to return to PendingInitiation")

        self.step(11)
        await self.send_remove_analysis_stream_cmd(endpoint, analysis_stream_id=stream_id)

        self.step(12)
        final_current_streams = await self.read_avanaly_attribute_expect_success(endpoint, attributes.CurrentAnalysisStreamCount)
        asserts.assert_equal(final_current_streams, current_streams,
                             f"Expected CurrentAnalysisStreamCount to return to {current_streams}, got {final_current_streams}")


if __name__ == "__main__":
    default_matter_test_main()
