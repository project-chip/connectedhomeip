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

from mobly import asserts
from TC_AVSUMTestBase import AVSUMTestBase

import matter.clusters as Clusters
from matter.clusters import Globals
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches


class TC_AVSUM_2_7(MatterBaseTest, AVSUMTestBase):

    def desc_TC_AVSUM_2_7(self) -> str:
        return "[TC-AVSUM-2.7] DPTZSetViewport command validation"

    def steps_TC_AVSUM_2_7(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the viewport and sensorparms from the device. Ensure the viewport is less than or equal to the sensor size"),
            TestStep(3, "Create a viewport smaller than the supported minimum"),
            TestStep(4, "Send DPTZSetVieport with an unknown stream ID, verify NotFound response"),
            TestStep(5, "Send a VideoStreamAllocate command to AVStreamManagement to allocate a video stream ID. Record the returned ID"),
            TestStep(6, "Read the DPTZStreams attribute, verify that the response contains an entry with the allocated stream ID and the device viewport"),
            TestStep(7, "Send DPTZSetVieport with a the viewport created in Step 3. Verify ConstraintError response"),
            TestStep(8, "Create a viewport with valid aspect ratio that is larger than the camera sensor"),
            TestStep(9, "Send DPTZSetVieport with a the viewport created in Step 6. Verify ConstraintError response"),
            TestStep(10, "Create a valid viewport give the dimensions of the sensor and the device resolution"),
            TestStep(11, "Send DPTZSetVieport with a the viewport created in Step 9. Verify success"),
            TestStep(12, "Read the DPTZStreams attribute, verify that the response contains an entry with the allocated stream ID and the viewport from step 10"),
            TestStep(13, "Modify the valid viewport so that the aspect ratio is invalid"),
            TestStep(14, "Send DPTZSetVieport with a the viewport created in Step 10. Verify ConstraintError response"),
        ]
        return steps

    def pics_TC_AVSUM_2_7(self) -> list[str]:
        pics = [
            "AVSUM.S", "AVSUM.S.F00", "AVSM.S"
        ]
        return pics

    @run_if_endpoint_matches(has_feature(Clusters.CameraAvSettingsUserLevelManagement,
                                         Clusters.CameraAvSettingsUserLevelManagement.Bitmaps.Feature.kDigitalPTZ) and
                             has_feature(Clusters.CameraAvStreamManagement,
                                         Clusters.CameraAvStreamManagement.Bitmaps.Feature.kVideo))
    async def test_TC_AVSUM_2_7(self):
        clusterAVSTR = Clusters.Objects.CameraAvStreamManagement
        attributesAVSTR = clusterAVSTR.Attributes
        endpoint = self.get_endpoint(default=1)

        self.step(1)  # Already done, immediately go to step 2

        self.step(2)
        # Ensure that the DUTs Viewport and Sensor Size are coherent, the viewport cannot be larger than the sensor
        sensordimensions = await self.read_avstr_attribute_expect_success(endpoint, attributesAVSTR.VideoSensorParams)
        viewport = await self.read_avstr_attribute_expect_success(endpoint, attributesAVSTR.Viewport)
        sensorSize = sensordimensions.sensorWidth * sensordimensions.sensorHeight
        viewportSize = (viewport.x2 - viewport.x1) * (viewport.y2 - viewport.y1)
        asserts.assert_less_equal(viewportSize, sensorSize, "Viewport is larger than the devices sensor max.")

        self.step(3)
        # Create a dummy stream ID, initially it will fail. And a new viewport, that will also fail
        failingvideostreamID = 5
        smallfailingviewport = Globals.Structs.ViewportStruct(x1=1, y1=10, x2=1, y2=10)

        self.step(4)
        # Send a dptzsetviewport for the dodgy stream
        await self.send_dptz_set_viewport_command(endpoint, failingvideostreamID, smallfailingviewport, expected_status=Status.NotFound)

        self.step(5)
        # Allocate the stream
        videoStreamID = await self.video_stream_allocate_command(endpoint)

        self.step(6)
        # Read DPTZStreams and verify that the stream and viewport are present
        if not self.dptzstreamentryvalid(endpoint, videoStreamID, viewport):
            asserts.assert_fail("No matching stream id and viewport found in DPTZStreams for the allocated video stream")

        self.step(7)
        # Send a dptzsetviewport for the correct stream but with an invalid viewport small dimension viewport
        await self.send_dptz_set_viewport_command(endpoint, videoStreamID, smallfailingviewport, expected_status=Status.ConstraintError)

        self.step(8)
        # Send a dptzsetviewport for the correct stream but with an invalid viewport larger than the sensor call deal with dimension viewport
        largefailingviewport = Globals.Structs.ViewportStruct(
            x1=0, y1=0, x2=sensordimensions.sensorWidth+16, y2=sensordimensions.sensorHeight+9)

        self.step(9)
        await self.send_dptz_set_viewport_command(endpoint, videoStreamID, largefailingviewport, expected_status=Status.ConstraintError)

        # Get the current viewport
        # Get the sensor dimensions
        # define viewport that has x1 at sensor width - viewport width, y1 at 0
        viewportwidth = viewport.x2 - viewport.x1
        viewportheight = viewport.y2 - viewport.y1
        x1 = sensordimensions.sensorWidth - viewportwidth

        self.step(10)
        passingviewport = Globals.Structs.ViewportStruct(x1=x1, y1=0, x2=sensordimensions.sensorWidth, y2=viewportheight)
        self.step(11)
        await self.send_dptz_set_viewport_command(endpoint, videoStreamID, passingviewport)

        self.step(12)
        # Verify the viewport has been updated in DPTZStreams
        if not self.dptzstreamentryvalid(endpoint, videoStreamID, passingviewport):
            asserts.assert_fail("Viewport has not been updated in DPTZStreams")

        self.step(13)
        # Deliberately mess with the aspect ratio, ensure that the viewport setting fails.
        failingviewport = Globals.Structs.ViewportStruct(
            x1=x1, y1=viewportheight//2, x2=sensordimensions.sensorWidth, y2=viewportheight)
        self.step(14)
        await self.send_dptz_set_viewport_command(endpoint, videoStreamID, failingviewport, expected_status=Status.ConstraintError)


if __name__ == "__main__":
    default_matter_test_main()
