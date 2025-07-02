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

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts
from TC_AVSMTestBase import AVSMTestBase

logger = logging.getLogger(__name__)


class TC_AVSM_2_8(MatterBaseTest, AVSMTestBase):
    def desc_TC_AVSM_2_8(self) -> str:
        return "[TC-AVSM-2.8] Validate Video Stream Modification functionality with Server as DUT"

    def pics_TC_AVSM_2_8(self):
        return ["AVSM.S"]

    def steps_TC_AVSM_2_8(self) -> list[TestStep]:
        return [
            TestStep("precondition", "DUT commissioned and preconditions", is_commissioning=True),
            TestStep(
                1,
                "TH reads FeatureMap attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify VDO & (WMARK|OSD) is supported.",
            ),
            TestStep(
                2,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the number of allocated video streams in the list is 1. Store StreamID as aStreamID. If WMARK is supported, store WaterMarkEnabled as aWmark. If OSD is supported, store OSDEnabled as aOSD.",
            ),
            TestStep(
                3,
                "TH sends the VideoStreamModify command with VideoStreamID set to aStreamID. If WMARK is supported, set WaterMarkEnabled to !aWmark and if OSD is supported, set OSDEnabled to !aOSD in the command.",
                "DUT responds with a SUCCESS status code.",
            ),
            TestStep(
                4,
                "TH reads AllocatedVideoStreams attribute from CameraAVStreamManagement Cluster on DUT",
                "Verify the following: If WMARK is supported, verify WaterMarkEnabled == !aWmark. If OSD is supported, verify OSDEnabled == !aOSD.",
            ),
        ]

    @run_if_endpoint_matches(
        has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kVideo)
        and (
            has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kWatermark)
            or has_feature(Clusters.CameraAvStreamManagement, Clusters.CameraAvStreamManagement.Bitmaps.Feature.kOnScreenDisplay)
        )
    )
    async def test_TC_AVSM_2_8(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.CameraAvStreamManagement
        attr = Clusters.CameraAvStreamManagement.Attributes
        commands = Clusters.CameraAvStreamManagement.Commands

        self.step("precondition")
        # Commission DUT - already done
        await self.precondition_one_allocated_video_stream()

        self.step(1)
        aFeatureMap = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attr.FeatureMap)
        logger.info(f"Rx'd FeatureMap: {aFeatureMap}")
        vdoSupport = (aFeatureMap & cluster.Bitmaps.Feature.kVideo) > 0
        wmarkSupport = (aFeatureMap & cluster.Bitmaps.Feature.kWatermark) > 0
        osdSupport = (aFeatureMap & cluster.Bitmaps.Feature.kOnScreenDisplay) > 0
        asserts.assert_true((vdoSupport and (wmarkSupport or osdSupport)), "VDO & (WMARK|OSD) is not supported.")

        self.step(2)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        logger.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        asserts.assert_equal(len(aAllocatedVideoStreams), 1, "The number of allocated video streams in the list is not 1")
        aStreamID = aAllocatedVideoStreams[0].videoStreamID
        aWmark = aAllocatedVideoStreams[0].watermarkEnabled
        aOSD = aAllocatedVideoStreams[0].OSDEnabled

        self.step(3)
        try:
            videoStreamModifyCmd = commands.VideoStreamModify(
                videoStreamID=aStreamID,
                watermarkEnabled=None if aWmark is None else not aWmark,
                OSDEnabled=None if aOSD is None else not aOSD,
            )
            await self.send_single_cmd(endpoint=endpoint, cmd=videoStreamModifyCmd)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.Success, "Unexpected error returned")
            pass

        self.step(4)
        aAllocatedVideoStreams = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attr.AllocatedVideoStreams
        )
        logger.info(f"Rx'd AllocatedVideoStreams: {aAllocatedVideoStreams}")
        if wmarkSupport:
            asserts.assert_equal(aAllocatedVideoStreams[0].watermarkEnabled, not aWmark, "WaterMarkEnabled is not !aWmark")
        if osdSupport:
            asserts.assert_equal(aAllocatedVideoStreams[0].OSDEnabled, not aOSD, "OSDEnabled is not !aOSD")


if __name__ == "__main__":
    default_matter_test_main()
