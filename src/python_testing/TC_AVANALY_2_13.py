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


class TC_AVANALY_2_13(MatterBaseTest, AVANALYTestBase):

    def desc_TC_AVANALY_2_13(self) -> str:
        return "[TC-AVANALY-2.13] Validate SupportedAmbientContexts consistency with Ambient Context Sensing cluster with Server as DUT"

    def steps_TC_AVANALY_2_13(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads AmbientContextTypeSupported from Ambient Context Sensing cluster. Save as sensor_supported."),
            TestStep(3, "TH reads SupportedAmbientContexts from AV Analysis cluster. Save as analysis_supported."),
            TestStep(4, "Verify analysis_supported is a subset of sensor_supported."),
        ]

    def pics_TC_AVANALY_2_13(self) -> list[str]:
        return [
            "AVANALY.S",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AvAnalysis))
    async def test_TC_AVANALY_2_13(self):
        cluster = Clusters.Objects.AvAnalysis
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()

        self.step(1)  # Already done, immediately go to step 2

        # Check if AmbientContextSensing cluster exists on endpoint
        acs_cluster = Clusters.Objects.AmbientContextSensing
        acs_present = False
        try:
            acs_attr_list = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=acs_cluster, attribute=acs_cluster.Attributes.AttributeList
            )
            acs_present = acs_attr_list is not None
        except Exception as e:
            log.info("AmbientContextSensing cluster not present on endpoint %d: %s", endpoint, e)
            acs_present = False

        if not acs_present:
            log.info("AmbientContextSensing cluster not present, skipping TC-AVANALY-2.13 steps")
            self.skip_step(2)
            self.skip_step(3)
            self.skip_step(4)
            return

        self.step(2)
        sensor_supported = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=acs_cluster, attribute=acs_cluster.Attributes.AmbientContextTypeSupported
        )
        log.info("AmbientContextTypeSupported: %s", sensor_supported)

        self.step(3)
        analysis_supported = await self.read_avanaly_attribute_expect_success(endpoint, attributes.SupportedAmbientContexts)
        log.info("SupportedAmbientContexts: %s", analysis_supported)

        self.step(4)
        if sensor_supported is not None:
            sensor_tags = {(st.namespaceID, st.tag) for st in sensor_supported}
            for tag in analysis_supported:
                asserts.assert_in(
                    (tag.namespaceID, tag.tag), sensor_tags,
                    f"AVAnalysis context ({tag.namespaceID}, {tag.tag}) is not in AmbientContextSensing supported contexts {sensor_tags}"
                )


if __name__ == "__main__":
    default_matter_test_main()
