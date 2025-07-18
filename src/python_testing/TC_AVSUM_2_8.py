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

import chip.clusters as Clusters
from chip.clusters import Globals
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from TC_AVSUMTestBase import AVSUMTestBase


class TC_AVSUM_2_8(MatterBaseTest, AVSUMTestBase):

    def desc_TC_AVSUM_2_8(self) -> str:
        return "[TC-AVSUM-2.8] DPTZRelativeMove command validation"

    def steps_TC_AVSUM_2_8(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Send DPTZRelativeMove with an unknown stream ID, but valid Zoom Delta verify NotFound response"),
            TestStep(3, "Send a VideoStreamAllocate command to AVStreamManagement to allocate a video stream ID. Record the returned ID"),
            TestStep(4, "Send DPTZRelativeMove with the allocated stream ID, invalid Zoom Delta. Verify ConstraintError response"),
            TestStep(5, "Create a viewport with a valid AR. Set this via DPTZSetViewport"),
            TestStep(6, "Setup deltaX and deltaY to move beyond the cartesian plan, send via DPTZRelativeMove. Verify success"),
            TestStep(7, "Setup deltaX to move the viewport to the right, send via DPTZRelativeMove. Verify success"),
            TestStep(8, "Setup deltaY to move the viewport down, send via DPTZRelativeMove. Verify success"),
            TestStep(9, "Repeatedly invoke DPTZRelativeMove with a Zoom Delta of 100%, verify no error on max out of sensor size"),
        ]
        return steps

    def pics_TC_AVSUM_2_8(self) -> list[str]:
        pics = [
            "AVSUM.S", "AVSUM.S.F00", "AVSM.S"
        ]
        return pics

    @run_if_endpoint_matches(has_feature(Clusters.CameraAvSettingsUserLevelManagement,
                                         Clusters.CameraAvSettingsUserLevelManagement.Bitmaps.Feature.kDigitalPTZ) and
                             has_feature(Clusters.CameraAvStreamManagement,
                                         Clusters.CameraAvStreamManagement.Bitmaps.Feature.kVideo))
    async def test_TC_AVSUM_2_8(self):
        clusterAVSTR = Clusters.Objects.CameraAvStreamManagement
        attributesAVSTR = clusterAVSTR.Attributes
        endpoint = self.get_endpoint(default=1)

        self.step(1)  # Already done, immediately go to step 2

        self.step(2)
        # Create a dummy stream ID, initially it will fail.
        failingvideostreamID = 5

        # Send a dptzrelativemove for the dodgy stream
        # Include a valid ZoomDelta
        await self.send_dptz_relative_move_command(endpoint, failingvideostreamID, zoomDelta=50, expected_status=Status.NotFound)

        self.step(3)
        # Allocate the stream
        videoStreamID = await self.video_stream_allocate_command(endpoint)

        self.step(4)
        # Send a dptzrelativemove for the correct stream but with an invalid ZoomDelta
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, zoomDelta=101, expected_status=Status.ConstraintError)

        self.step(5)
        # Set a viewport
        viewport = await self.read_avstr_attribute_expect_success(endpoint, attributesAVSTR.Viewport)
        sensordimensions = await self.read_avstr_attribute_expect_success(endpoint, attributesAVSTR.VideoSensorParams)

        viewportwidth = viewport.x2 - viewport.x1
        viewportheight = viewport.y2 - viewport.y1
        x1 = sensordimensions.sensorWidth - viewportwidth
        passingviewport = Globals.Structs.ViewportStruct(x1=x1, y1=0, x2=sensordimensions.sensorWidth, y2=viewportheight)
        await self.send_dptz_set_viewport_command(endpoint, videoStreamID, passingviewport)

        self.step(6)
        # Send a dptzrelativemove based on the set viewport, move to beyond cartesian plane
        deltaX = -x1 * 2
        deltaY = -viewportheight * 2
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, deltaX=deltaX, deltaY=deltaY)

        self.step(7)
        # Send a dptzrelativemove based on the current viewport, move to the right
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, deltaX=100)

        self.step(8)
        # Send a dptzrelativemove based on the new viewport, move down
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, deltaY=100)

        self.step(9)
        # Send a dptzrelativemove based on the new viewport, zoom to beyond sensor size
        currentsize = (viewport.x2-viewport.x1) * (viewport.y2-viewport.y1)
        sensorsize = sensordimensions.sensorWidth * sensordimensions.sensorHeight
        zoomedsize = currentsize * 2

        await self.send_dptz_relative_move_command(endpoint, videoStreamID, zoomDelta=100)

        while True:
            if zoomedsize < sensorsize:
                zoomedsize = zoomedsize * 2
                await self.send_dptz_relative_move_command(endpoint, videoStreamID, zoomDelta=100)
            else:
                break


if __name__ == "__main__":
    default_matter_test_main()
