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
from matter.testing.decorators import all_of, has_command, has_feature, run_if_endpoint_matches
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
            TestStep(5, "Create a viewport with the aspect ratio of the allocated stream, at least half the stream width but not smaller than MinViewportResolution. Set this via DPTZSetViewport"),
            TestStep(6, "Setup deltaX and deltaY to move beyond the Cartesian plane, send via DPTZRelativeMove.",
                        "Verify the viewport clamps at the sensor bounds for negative and positive deltas"),
            TestStep(7, "Setup DeltaX and DeltaY to move the viewport back to the top left of the sensor plane"),
            TestStep(8, "Read the DPTZStreams attribute, store the viewport associated with the allocated stream id"),
            TestStep(9, "Setup deltaX to move the viewport to the right, send via DPTZRelativeMove. Verify success"),
            TestStep(10, "Read the DPTZStreams attribute, verify that the new viewport is the old viewport + deltaX"),
            TestStep(11, "Setup deltaY to move the viewport down, send via DPTZRelativeMove. Verify success"),
            TestStep(12, "Read the DPTZStreams attribute, verify that the new viewport is the old viewport + deltaY"),
            TestStep(13, "Move the viewport to the bottom right corner of the sensor, send DPTZRelativeMove with a Zoom Delta of +100%, verify the viewport is at least MinViewportResolution and remains within the sensor bounds. Send a second Zoom Delta of +100% and verify the viewport is unchanged"),
            TestStep(14, "Repeatedly invoke DPTZRelativeMove with a Zoom Delta of -100% until the viewport stops changing, verify it fills the sensor in at least one dimension and remains within the sensor bounds"),
            TestStep(15, "Create values of deltaX and deltaY that would move the viewport beyond the sensor's Cartesian plane"),
            TestStep(16, "Via DPTZRelativeMove send the created values of deltaX and deltaY. Verify success"),
            TestStep(17, "Read the DPTZStreams attribute, verify that the viewport moved by deltaX and deltaY, clamped inside the sensor with its size unchanged"),
        ]

    def pics_TC_AVSUM_2_8(self) -> list[str]:
        return [
            "AVSUM.S", "AVSUM.S.F00", "AVSUM.S.C06.Rsp", "AVSM.S"
        ]

    @run_if_endpoint_matches(all_of(has_feature(Clusters.CameraAvSettingsUserLevelManagement,
                                                Clusters.CameraAvSettingsUserLevelManagement.Bitmaps.Feature.kDigitalPTZ),
                                    has_feature(Clusters.CameraAvStreamManagement,
                                                Clusters.CameraAvStreamManagement.Bitmaps.Feature.kVideo),
                                    has_command(Clusters.CameraAvSettingsUserLevelManagement.Commands.DPTZRelativeMove)))
    async def test_TC_AVSUM_2_8(self):
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
        # Set a viewport with the stream's aspect ratio that is smaller than the sensor, so the relative
        # moves in the following steps have room to move within the sensor bounds. Use at least half
        # the stream width, but never below the minimum viewport resolution.
        sensordimensions = await self.read_avstr_attribute_expect_success(endpoint, attributesAVSTR.VideoSensorParams)
        minviewport = await self.read_avstr_attribute_expect_success(endpoint, attributesAVSTR.MinViewportResolution)

        # DPTZSetViewport requires the viewport aspect ratio to match the stream's maxResolution,
        # which can differ from the sensor size when a stream was already allocated before this
        # test ran.
        allocatedstreams = await self.read_avstr_attribute_expect_success(endpoint, attributesAVSTR.AllocatedVideoStreams)
        stream = next((s for s in allocatedstreams if s.videoStreamID == videoStreamID), None)
        asserts.assert_is_not_none(stream, f"Stream {videoStreamID} not found in AllocatedVideoStreams")
        streamwidth = stream.maxResolution.width
        streamheight = stream.maxResolution.height

        viewportwidth = max(streamwidth // 2, minviewport.width)
        viewportheight = round(viewportwidth * streamheight / streamwidth)
        if viewportheight < minviewport.height:
            viewportheight = minviewport.height
            viewportwidth = round(viewportheight * streamwidth / streamheight)

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
        asserts.assert_equal(await self.read_dptz_viewport(endpoint, videoStreamID), homeViewport,
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
        asserts.assert_equal(await self.read_dptz_viewport(endpoint, videoStreamID), clampedViewport,
                             "Positive delta move did not clamp the viewport to the bottom right of the sensor")

        self.step(7)
        # Move the viewport back to the top left for the following steps, which rely on an unclamped move
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, deltaX=-deltaXP, deltaY=-deltaYP)
        asserts.assert_equal(await self.read_dptz_viewport(endpoint, videoStreamID), homeViewport,
                             "Viewport did not return to the top left of the sensor")

        self.step(8)
        myViewport = await self.read_dptz_viewport(endpoint, videoStreamID)

        self.step(9)
        # Move at most 100 pixels, limited to the space left before the sensor edge
        shiftX = min(100, sensordimensions.sensorWidth - myViewport.x2)
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, deltaX=shiftX)
        myViewport.x1 = myViewport.x1+shiftX
        myViewport.x2 = myViewport.x2+shiftX

        self.step(10)
        asserts.assert_equal(await self.read_dptz_viewport(endpoint, videoStreamID), myViewport,
                             "Viewport not moved to the right as requested by DPTZRelativeMove")

        self.step(11)
        # Move at most 100 pixels, limited to the space left before the sensor edge
        shiftY = min(100, sensordimensions.sensorHeight - myViewport.y2)
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, deltaY=shiftY)
        myViewport.y1 = myViewport.y1+shiftY
        myViewport.y2 = myViewport.y2+shiftY

        self.step(12)
        asserts.assert_equal(await self.read_dptz_viewport(endpoint, videoStreamID), myViewport,
                             "Viewport not moved down as requested by DPTZRelativeMove")

        self.step(13)
        # Move the viewport to the bottom right corner of the sensor, then zoom in. The zoom-in must
        # not shrink the viewport below the minimum resolution and must stay within the sensor bounds.
        # No aspect-ratio check until the MinViewportResolution and aspect-ratio rules are reconciled.
        shiftX = sensordimensions.sensorWidth - myViewport.x2
        shiftY = sensordimensions.sensorHeight - myViewport.y2
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, deltaX=shiftX, deltaY=shiftY)
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, zoomDelta=100)

        zoomedInViewport = await self.read_dptz_viewport(endpoint, videoStreamID)
        asserts.assert_true(zoomedInViewport.x2 - zoomedInViewport.x1 >= minviewport.width,
                            "Zoomed-in viewport is smaller than MinViewportResolution width")
        asserts.assert_true(zoomedInViewport.y2 - zoomedInViewport.y1 >= minviewport.height,
                            "Zoomed-in viewport is smaller than MinViewportResolution height")
        asserts.assert_true(zoomedInViewport.x2 <= sensordimensions.sensorWidth,
                            "Zoomed-in viewport exceeded the sensor width")
        asserts.assert_true(zoomedInViewport.y2 <= sensordimensions.sensorHeight,
                            "Zoomed-in viewport exceeded the sensor height")

        # A zoom-in of +100% saturates at the minimum viewport size, so a second zoom-in must
        # leave the viewport unchanged. This catches a per-call drift of the viewport origin
        # when the size is recomputed at the minimum.
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, zoomDelta=100)
        asserts.assert_equal(await self.read_dptz_viewport(endpoint, videoStreamID), zoomedInViewport,
                             "Repeated zoom-in at the minimum size moved the viewport")

        self.step(14)
        # Zoom out until a further zoom-out leaves the viewport unchanged. The largest viewport keeps the
        # stream's aspect ratio, so it fills the sensor in at least one dimension but not necessarily both.
        maxviewport = await self.read_dptz_viewport(endpoint, videoStreamID)
        for _ in range(20):
            await self.send_dptz_relative_move_command(endpoint, videoStreamID, zoomDelta=-100)
            previousviewport = maxviewport
            maxviewport = await self.read_dptz_viewport(endpoint, videoStreamID)
            if maxviewport == previousviewport:
                break
        else:
            asserts.assert_fail("Viewport kept changing after repeated zoom-out")

        maxwidth = maxviewport.x2 - maxviewport.x1
        maxheight = maxviewport.y2 - maxviewport.y1
        asserts.assert_true(maxwidth == sensordimensions.sensorWidth or maxheight == sensordimensions.sensorHeight,
                            f"Zoomed-out viewport {maxviewport} does not fill the sensor in either dimension")
        asserts.assert_true(maxviewport.x2 <= sensordimensions.sensorWidth and maxviewport.y2 <= sensordimensions.sensorHeight,
                            f"Zoomed-out viewport {maxviewport} exceeds the sensor")

        self.step(15)
        # The viewport is at its largest size, so it fills the sensor in at least one dimension and any
        # movement right and down leaves the sensor's Cartesian plane. Use positive deltas so the direction
        # matches the reported issue, where a positive move at the full-sensor viewport pushed the viewport
        # outside of the sensor.
        deltaX = sensordimensions.sensorWidth // 10
        deltaY = sensordimensions.sensorHeight // 10

        self.step(16)
        await self.send_dptz_relative_move_command(endpoint, videoStreamID, deltaX, deltaY)

        self.step(17)
        # The move is clamped, so the viewport keeps its size and stays inside the sensor.
        expectedx1 = min(maxviewport.x1 + deltaX, sensordimensions.sensorWidth - maxwidth)
        expectedy1 = min(maxviewport.y1 + deltaY, sensordimensions.sensorHeight - maxheight)
        expectedviewport = Globals.Structs.ViewportStruct(x1=expectedx1,
                                                          y1=expectedy1,
                                                          x2=expectedx1 + maxwidth,
                                                          y2=expectedy1 + maxheight)
        asserts.assert_equal(await self.read_dptz_viewport(endpoint, videoStreamID), expectedviewport,
                             "Viewport not clamped inside the sensor after the final relative move")


if __name__ == "__main__":
    default_matter_test_main()
