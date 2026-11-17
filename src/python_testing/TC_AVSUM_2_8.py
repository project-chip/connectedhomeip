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
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import TestStep, default_matter_test_main


class TC_AVSUM_2_8(MatterTestCommissionedDevice, AVSUMTestBase):

    def desc_TC_AVSUM_2_8(self) -> str:
        return "[TC-AVSUM-2.8] DPTZRelativeMove command validation"

    def steps_TC_AVSUM_2_8(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done"),
            TestStep(2, "Send DPTZRelativeMove with an unknown stream ID, but valid Zoom Delta verify NotFound response"),
            TestStep(3, "Send a VideoStreamAllocate command to AVStreamManagement to allocate a video stream ID. Record the returned ID"),
            TestStep(4, "Send DPTZRelativeMove with the allocated stream ID, invalid Zoom Delta. Verify ConstraintError response"),
            TestStep(5, "Create a viewport with a valid AR that is half the sensor dimensions. Set this via DPTZSetViewport"),
            TestStep(6, "Setup deltaX and deltaY to move beyond the cartesian plane, send via DPTZRelativeMove.",
                        "Verify the viewport clamps at the sensor bounds for negative and positive deltas"),
            TestStep(7, "Setup DeltaX and DeltaY to move the viewport back to the top left of the sensor plane"),
            TestStep(8, "Read the DPTZStreams attribute, store the viewport associated with the allocated stream id"),
            TestStep(9, "Setup deltaX to move the viewport to the right, send via DPTZRelativeMove. Verify success"),
            TestStep(10, "Read the DPTZStreams attribute, verify that the new viewport is the old viewport + deltaX"),
            TestStep(11, "Setup deltaY to move the viewport down, send via DPTZRelativeMove. Verify success"),
            TestStep(12, "Read the DPTZStreams attribute, verify that the new viewport is the old viewport + deltaY"),
            TestStep(13, "Repeatedly invoke DPTZRelativeMove with a Zoom Delta of -100%, verify no error on max out of sensor size and that the viewport reaches the sensor max"),
            TestStep(14, "Create values of deltaX and deltaY that would move the viewport beyond the sensors cartesian plane"),
            TestStep(15, "Via DPTZRelativeMove send the created values of deltaX and deltaY. Verify success"),
            TestStep(16, "Read the DPTZStreams attribute, verify that the new viewport is set to the sensor max"),
        ]

    def pics_TC_AVSUM_2_8(self) -> list[str]:
        return [
            "AVSUM.S", "AVSUM.S.F00", "AVSM.S"
        ]

    @run_if_endpoint_matches(has_feature(Clusters.CameraAvSettingsUserLevelManagement,
                                         Clusters.CameraAvSettingsUserLevelManagement.Bitmaps.Feature.kDigitalPTZ) and
                             has_feature(Clusters.CameraAvStreamManagement,
                                         Clusters.CameraAvStreamManagement.Bitmaps.Feature.kVideo))
    async def test_TC_AVSUM_2_8(self):
        cluster = Clusters.Objects.CameraAvSettingsUserLevelManagement
        attributes = cluster.Attributes
        clusterAVSTR = Clusters.Objects.CameraAvStreamManagement
        attributesAVSTR = clusterAVSTR.Attributes
        endpoint = self.get_endpoint()

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
        # Set a viewport with a valid AR that is smaller than the sensor, so the relative moves
        # in the following steps have room to move within the sensor bounds
        sensordimensions = await self.read_avstr_attribute_expect_success(endpoint, attributesAVSTR.VideoSensorParams)

        viewportwidth = sensordimensions.sensorWidth // 2
        viewportheight = sensordimensions.sensorHeight // 2
        x1 = sensordimensions.sensorWidth - viewportwidth
        passingviewport = Globals.Structs.ViewportStruct(x1=x1, y1=0, x2=sensordimensions.sensorWidth, y2=viewportheight)
        await self.send_dptz_set_viewport_command(endpoint, videoStreamID, passingviewport)

        self.step(6)
        # Send a dptzrelativemove based on the set viewport, move to beyond cartesian plane
        deltaX = -x1 * 2
        deltaY = -viewportheight * 2
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, deltaX=deltaX, deltaY=deltaY)

        # The move is clamped, leaving the viewport pinned at the top left of the sensor
        homeViewport = Globals.Structs.ViewportStruct(x1=0, y1=0, x2=viewportwidth, y2=viewportheight)
        asserts.assert_true(await self.dptzstreamentryvalid(endpoint, videoStreamID, homeViewport),
                            "Negative delta move did not clamp the viewport to the top left of the sensor")

        # Send positive deltas that move beyond the right and bottom sensor edges. The move must clamp,
        # leaving the viewport pinned at the bottom right of the sensor with its size unchanged.
        deltaXP = sensordimensions.sensorWidth
        deltaYP = sensordimensions.sensorHeight
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, deltaX=deltaXP, deltaY=deltaYP)

        clampedViewport = Globals.Structs.ViewportStruct(x1=sensordimensions.sensorWidth - viewportwidth,
                                                         y1=sensordimensions.sensorHeight - viewportheight,
                                                         x2=sensordimensions.sensorWidth,
                                                         y2=sensordimensions.sensorHeight)
        asserts.assert_true(await self.dptzstreamentryvalid(endpoint, videoStreamID, clampedViewport),
                            "Positive delta move did not clamp the viewport to the bottom right of the sensor")

        self.step(7)
        # Move the viewport back to the top left for the following steps, which rely on an unclamped move
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, deltaX=-deltaXP, deltaY=-deltaYP)
        asserts.assert_true(await self.dptzstreamentryvalid(endpoint, videoStreamID, homeViewport),
                            "Viewport did not return to the top left of the sensor")

        self.step(8)
        dptz_streams_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.DPTZStreams)
        match_found = False
        myViewport = None
        if dptz_streams_dut is not None:
            for streams in dptz_streams_dut:
                if streams.videoStreamID == videoStreamID:
                    # Save the viewport
                    myViewport = streams.viewport
                    match_found = True
                    break
        else:
            asserts.assert_fail("DPTZStreams is empty, even though a stream has been allocated")

        if not match_found:
            asserts.assert_fail("Allocated stream not found in DPTZStreams")

        self.step(9)
        # Move at most 100 pixels, limited to the space left before the sensor edge
        shiftX = min(100, sensordimensions.sensorWidth - myViewport.x2)
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, deltaX=shiftX)
        myViewport.x1 = myViewport.x1+shiftX
        myViewport.x2 = myViewport.x2+shiftX

        self.step(10)
        asserts.assert_true(await self.dptzstreamentryvalid(endpoint, videoStreamID, myViewport),
                            "Viewport not moved to the right as requested by DPTZRelativeMove")

        self.step(11)
        # Move at most 100 pixels, limited to the space left before the sensor edge
        shiftY = min(100, sensordimensions.sensorHeight - myViewport.y2)
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, deltaY=shiftY)
        myViewport.y1 = myViewport.y1+shiftY
        myViewport.y2 = myViewport.y2+shiftY

        self.step(12)
        asserts.assert_true(await self.dptzstreamentryvalid(endpoint, videoStreamID, myViewport),
                            "Viewport not moved down as requested by DPTZRelativeMove")

        self.step(13)
        # Repeatedly zoom out until the viewport reaches the sensor max. The zoom results are
        # rounded, so the viewport is read after each command; a zoom can land just short of
        # the sensor bounds on the previous pass.
        maxviewport = Globals.Structs.ViewportStruct(x1=0,
                                                     y1=0,
                                                     x2=sensordimensions.sensorWidth,
                                                     y2=sensordimensions.sensorHeight)

        for _ in range(20):
            await self.send_dptz_relative_move_command(endpoint, videoStreamID, zoomDelta=-100)
            if await self.dptzstreamentryvalid(endpoint, videoStreamID, maxviewport):
                break
        else:
            asserts.assert_fail("Viewport did not reach the sensor max after repeated zoom-out")

        self.step(14)
        # Current viewport is at the sensor max, any movement will leave the cartesian plane
        deltaX = -(sensordimensions.sensorWidth // 2)
        deltaY = -(sensordimensions.sensorHeight // 2)

        self.step(15)
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, deltaX, deltaY)

        self.step(16)
        dptz_streams_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.DPTZStreams)
        match_found = False
        for streams in dptz_streams_dut:
            if streams.videoStreamID == videoStreamID:
                # verify the viewport dimensions
                viewportwidth = streams.viewport.x2 - streams.viewport.x1
                viewportheight = streams.viewport.y2 - streams.viewport.y1
                asserts.assert_equal(viewportwidth, sensordimensions.sensorWidth,
                                     "Viewport not set to the same width as the sensor")
                asserts.assert_equal(viewportheight, sensordimensions.sensorHeight,
                                     "Viewport not set to the same height as the sensor")
                match_found = True
                break

        if not match_found:
            asserts.assert_fail("No matching stream found in DPTZStreams")


if __name__ == "__main__":
    default_matter_test_main()
