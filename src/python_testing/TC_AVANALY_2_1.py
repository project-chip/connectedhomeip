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


class TC_AVANALY_2_1(MatterBaseTest, AVANALYTestBase):

    def desc_TC_AVANALY_2_1(self) -> str:
        return "[TC-AVANALY-2.1] Attributes with DUT as Server"

    def steps_TC_AVANALY_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads the SupportedAmbientContexts attribute. Verify that it is list of valid semantic structs",
                        "Verify that there are no more than 50 entries"),
            TestStep(3, "TH reads the ActiveAmbientContextTriggers attribute. Verify that this is a list with zero entries"),
            TestStep(4, "If RemoteContextDetection is supported, TH reads the MaxAnalysisStreamCount attribute",
                        "Verify that this is a uint8 value."),
            TestStep(5, "If RemoteContextDetection is supported, TH reads the CurrentAnalysisStreamCount attribute",
                        "Verify that this is zero."),
            TestStep(6, "If RemoteContextDetection is supported, TH reads the AnalysisStreams attribute. Verify that this is a list with zero entries"),
            TestStep(7, "TH reads the TrackingEnabled attribute.  Verify that this is a bool with a value of false."),
        ]

    def pics_TC_AVANALY_2_1(self) -> list[str]:
        return [
            "AVANALY.S",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AvAnalysis))
    async def test_TC_AVANALY_2_1(self):
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()

        self.step(1)  # Already done, immediately go to step 2

        feature_map = await self.read_avanaly_attribute_expect_success(endpoint, attributes.FeatureMap)
        self.has_feature_lclcondetect = (feature_map & cluster.Bitmaps.Feature.kLocalContextDetection) != 0
        self.has_feature_remcondetect = (feature_map & cluster.Bitmaps.Feature.kRemoteContextDetection) != 0
        self.has_feature_perzonedetect = (feature_map & cluster.Bitmaps.Feature.kPerZoneContextDetection) != 0

        log.info("Feature map: 0x%x. LCLCONDETECT: %s, REMCONDETECT:%s, PERZONEDETECT:%s",
                 feature_map, self.has_feature_lclcondetect, self.has_feature_remcondetect, self.has_feature_perzonedetect)

        attribute_list = await self.read_avanaly_attribute_expect_success(endpoint, attributes.AttributeList)

        self.step(2)
        if not (self.has_feature_lclcondetect or self.has_feature_remcondetect):
            asserts.fail("One of LCLCONDETECT or REMCONDETECT is mandatory")

        asserts.assert_in(attributes.SupportedAmbientContexts.attribute_id, attribute_list,
                          "SupportedAmbientContexts attribute is a mandatory attribute.")
        asserts.assert_in(attributes.ActiveAmbientContextTriggers.attribute_id, attribute_list,
                          "ActiveAmbientContextTriggers attribute is a mandatory attribute.")
        asserts.assert_in(attributes.TrackingEnabled.attribute_id, attribute_list,
                          "TrackingEnabled attribute is a mandatory attribute.")

        supported_ambient_contexts_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.SupportedAmbientContexts)

        # Make sure all provided contexts are in one of the supported namespaces
        asserts.assert_less_equal(len(supported_ambient_contexts_dut), self.SPEC_MAX_COUNT_SUPPORTEDAMBIENTCONTEXTS,
                                  "SupportedAmbientContexts size is greater than the allowed max.")
        asserts.assert_greater_equal(len(supported_ambient_contexts_dut), 1, "SupportedAmbientContexts cannot be empty.")

        for supportedcontext in supported_ambient_contexts_dut:
            asserts.assert_greater_equal(supportedcontext.namespaceID, 0x49, "NamespaceID is out of range")
            asserts.assert_less_equal(supportedcontext.namespaceID, 0x4b, "NamespaceID is out of range")

        self.step(3)
        # On fresh startup the active contexts should be empty
        active_ambient_context_triggers_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.ActiveAmbientContextTriggers)
        asserts.assert_equal(len(active_ambient_context_triggers_dut), 0,
                             "ActiveAmbientContextTriggers should be empty on initial startup.")

        if self.has_feature_remcondetect:
            self.step(4)
            max_analysis_stream_count_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.MaxAnalysisStreamCount)
            asserts.assert_less_equal(max_analysis_stream_count_dut, self.SPEC_MAX_COUNT_ANALYSIS_STREAMS)

            self.step(5)
            current_analysis_stream_count_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.CurrentAnalysisStreamCount)
            asserts.assert_equal(current_analysis_stream_count_dut, 0,
                                 "CurrentAnalysisStreamCount should be zero on initial startup")

            self.step(6)
            analysis_streams_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.AnalysisStreams)
            asserts.assert_equal(len(analysis_streams_dut), 0,
                                 "AnalysisStreams should be empty on initial startup.")
        else:
            self.skip_step(4)
            self.skip_step(5)
            self.skip_step(6)

        self.step(7)
        tracking_enabled_dut = await self.read_avanaly_attribute_expect_success(endpoint, attributes.TrackingEnabled)
        asserts.assert_false(tracking_enabled_dut, "TrackingEnabled should be false on startup")


if __name__ == "__main__":
    default_matter_test_main()
