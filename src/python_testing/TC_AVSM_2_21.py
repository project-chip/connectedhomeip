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
#     app-ready-pattern: "APP STATUS: Starting event loop"
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
import random

from mobly import asserts
from TC_AVSMTestBase import AVSMTestBase

import matter.clusters as Clusters
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVSM_2_21(MatterBaseTest, AVSMTestBase):

    def desc_TC_AVSM_2_21(self) -> str:
        return "[TC-AVSM-2.21] Validate persistence of stream usage priorities with DUT"

    def steps_TC_AVSM_2_21(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on DUT. Verify F_VDO is supported."),
            TestStep(3, "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT. Verify the number of allocated video streams in the list is 0."),
            TestStep(4, "TH reads SupportedStreamUsages attribute from CameraAVStreamManagement Cluster on DUT. Store this value in aSupportedStreamUsages."),
            TestStep(5, "TH sends the SetStreamPriorities command with StreamPriorities set as a subset of aSupportedStreamUsages."),
            TestStep(6, "TH reads StreamUsagePriorities attribute from CameraAVStreamManagement Cluster on DUT. Store this value in aStreamUsagePriorities."),
            TestStep(7, "TH reboots the DUT."),
            TestStep(8, "TH waits for the DUT to come back online."),
            TestStep(9, "TH reads StreamUsagePriorities attribute from CameraAVStreamManagement Cluster on DUT. Verify this value is same as aStreamUsagePriorities."),
        ]

    def pics_TC_AVSM_2_21(self) -> list[str]:
        return ["AVSM.S", "AVSM.S.F_VDO"]

    @run_if_endpoint_matches(
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kVideo)
    )
    async def test_TC_AVSM_2_21(self):
        endpoint = self.get_endpoint()
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step(1)

        self.step(2)
        feature_map = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        has_f_vdo = (feature_map & cluster.Bitmaps.Feature.kVideo) != 0
        asserts.assert_true(has_f_vdo, "FeatureMap F_VDO is not set")

        self.step(3)
        allocated_video_streams = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams)
        asserts.assert_equal(len(allocated_video_streams), 0, "AllocatedVideoStreams should be empty")

        self.step(4)
        supported_stream_usages = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.SupportedStreamUsages)
        asserts.assert_greater(len(supported_stream_usages), 0, "SupportedStreamUsages should not be empty")

        self.step(5)
        # Send a shuffled subset of supported usages
        new_priorities = random.sample(supported_stream_usages, k=random.randint(1, len(supported_stream_usages)))
        await self.send_single_cmd(cmd=commands.SetStreamPriorities(streamPriorities=new_priorities), endpoint=endpoint)

        self.step(6)
        stream_usage_priorities = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities)
        asserts.assert_equal(stream_usage_priorities, new_priorities, "StreamUsagePriorities not set correctly")

        self.step(7)
        await self.request_device_reboot()
        self.step(8)

        self.step(9)
        stream_usage_priorities_after_reboot = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.StreamUsagePriorities)
        asserts.assert_equal(stream_usage_priorities_after_reboot, new_priorities,
                             "StreamUsagePriorities did not persist after reboot")


if __name__ == "__main__":
    default_matter_test_main()
