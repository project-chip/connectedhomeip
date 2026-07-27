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
#       --bool-arg PIXIT.CANBEMADEBUSY:False
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import random

from mobly import asserts
from TC_AVSUMTestBase import AVSUMTestBase

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main


class TC_AVSUM_2_9(MatterBaseTest, AVSUMTestBase):

    def desc_TC_AVSUM_2_9(self) -> str:
        return "[TC-AVSUM-2.9] MptzSetPosition Command Validation"

    def steps_TC_AVSUM_2_9(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read and verify the MPTZPosition attribute."),
            TestStep(3, "Establish a subscription to MovementState"),
            TestStep(4, "If Pan is supported read and verify the PanMin attribute. Store in myPanMin."),
            TestStep(5, "If Pan is supported read and verify the PanMax attribute. Store in myPanMax."),
            TestStep(6, "Set newPan to be a value in the range myPanMin to myPanMax different from initialPan."),
            TestStep(7, "If Tilt is supported read and verify TiltMin attribute. Store in myTiltMin."),
            TestStep(8, "If Tilt is supported read and verify TiltMax attribute. Store in myTiltMax."),
            TestStep(9, "Set newTilt to be a value in the range myTiltMin to myTiltMax different from initialTilt."),
            TestStep(10, "If Zoom is supported read and verify ZoomMax attribute. Store in myZoomMax."),
            TestStep(11, "Set newZoom to be a value in the range 2 to myZoomMax different from initialZoom."),
            TestStep(12, "Send MPTZSetPosition command with Pan set to newPan, Tilt set to newTilt, Zoom set to newZoom as appropriate."),
            TestStep(13, "Once the MovementState has returned to _Idle_, read and verify the MPTZPosition attribute."),
            TestStep(14, "If MaxPresets is supported, read the MaxPresets attribute. Store the returned value in myMaxPresets."),
            TestStep(15, "Read the MPTZPresets attribute.  Save this in myPresets."),
            TestStep(16, "Fail if the number of entries in myPresets is greater than myMaxPresets."),
            TestStep(17, "Send the SavePreset command with PresetID set to myMaxPresets, Name set to 'newpreset'."),
            TestStep(18, "Read} the MPTZPresets attribute. Verify the list includes a preset with PresetID equal to myMaxPresets."),
            TestStep(19, "If DPTZ is supported, read AVStreamManagement.Viewport and AVStreamManagement.VideoSensorParams. Store the viewport in myViewport."),
            TestStep(20, "If DPTZ is supported, send AVStreamManagement.VideoStreamAllocate command. Store the the allocated video stream ID."),
            TestStep(21, "If DPTZ is supported read DPTZStreams attribute. Verify for the entry with a VideoStreamID of myStreamID the Viewport is myViewport."),
            TestStep(22, "Reboot the DUT."),
            TestStep(23, "Wait for the DUT to come back online."),
            TestStep(24, "Read and verify the MPTZPosition attribute. Verify Pan, Tilt, and Zoom values are persisted."),
            TestStep(25, "If MPTZPresets is supported, read the MPTZPresets attribute. Verify the list includes a preset with PresetID equal to myMaxPresets."),
            TestStep(26, "If DPTZ is supported, read the DPTZStreams attribute. Verify for the entry with a VideoStreamID of myStreamID the Viewport is myViewport."),
        ]

    def pics_TC_AVSUM_2_9(self) -> list[str]:
        return [
            "AVSUM.S",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.CameraAvSettingsUserLevelManagement))
    async def test_TC_AVSUM_2_9(self):
        cluster = Clusters.Objects.CameraAvSettingsUserLevelManagement
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()
        # Allow some user input steps to be skipped if running under CI
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        self.step(1)  # Already done, immediately go to step 2

        self.step(2)

        feature_map = await self.read_avsum_attribute_expect_success(endpoint, attributes.FeatureMap)
        self.has_feature_mpan = (feature_map & cluster.Bitmaps.Feature.kMechanicalPan) != 0
        self.has_feature_mtilt = (feature_map & cluster.Bitmaps.Feature.kMechanicalTilt) != 0
        self.has_feature_mzoom = (feature_map & cluster.Bitmaps.Feature.kMechanicalZoom) != 0
        self.has_feature_mpresets = (feature_map & cluster.Bitmaps.Feature.kMechanicalPresets) != 0
        self.has_feature_dptz = (feature_map & cluster.Bitmaps.Feature.kDigitalPTZ) != 0

        attribute_list = await self.read_avsum_attribute_expect_success(endpoint, attributes.AttributeList)

        if not (self.has_feature_mpan | self.has_feature_mtilt | self.has_feature_mzoom):
            asserts.fail("One of MPAN, MTILT, or MZOOM is mandatory for command support")

        asserts.assert_in(attributes.MPTZPosition.attribute_id, attribute_list,
                          "MPTZPosition attribute is mandatory if the command is supported.")
        mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
        currentPan = mptzposition_dut.pan
        currentTilt = mptzposition_dut.tilt
        currentZoom = mptzposition_dut.zoom

        self.step(3)
        # Establish subscription to MovementState
        sub_handler = AttributeSubscriptionHandler(cluster, attributes.MovementState)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        # Create attribute matchers
        movement_state_match = AttributeMatcher.from_callable(
            "MovementState is IDLE",
            lambda report: report.value == cluster.Enums.PhysicalMovementEnum.kIdle)

        newPan = newTilt = newZoom = None

        if self.has_feature_mpan:
            self.step(4)
            pan_min_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.PanMin)
            self.step(5)
            pan_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.PanMax)

            self.step(6)
            # Create new Value for Pan
            while True:
                newPan = random.randint(pan_min_dut, pan_max_dut)
                if newPan != currentPan:
                    break
        else:
            self.skip_step(4)
            self.skip_step(5)
            self.skip_step(6)

        if self.has_feature_mtilt:
            self.step(7)
            tilt_min_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.TiltMin)
            self.step(8)
            tilt_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.TiltMax)

            self.step(9)
            # Create new Value for Tilt
            while True:
                newTilt = random.randint(tilt_min_dut, tilt_max_dut)
                if newTilt != currentTilt:
                    break
        else:
            self.skip_step(7)
            self.skip_step(8)
            self.skip_step(9)

        if self.has_feature_mzoom:
            self.step(10)
            zoom_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.ZoomMax)

            self.step(11)
            # Create new Value for Zoom
            while True:
                newZoom = random.randint(2, zoom_max_dut)
                if newZoom != currentZoom:
                    break
        else:
            self.skip_step(10)
            self.skip_step(11)

        self.step(12)
        sub_handler.reset()
        # Invoke the command with the new values
        await self.send_mptz_set_position_command(endpoint, newPan, newTilt, newZoom)

        self.step(13)
        # Once the MovementState has reset to Idle, read the attribute back and make sure it was set
        sub_handler.await_all_expected_report_matches([movement_state_match], timeout_sec=self.default_timeout)
        new_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)

        if newPan is not None:
            asserts.assert_equal(new_mptzposition_dut.pan, newPan, "Received Pan does not match set Pan")

        if newTilt is not None:
            asserts.assert_equal(new_mptzposition_dut.tilt, newTilt, "Received Tilt does not match set Tilt")

        if newZoom is not None:
            asserts.assert_equal(new_mptzposition_dut.zoom, newZoom, "Received Zoom does not match set Zoom")

        if self.has_feature_mpresets:
            self.step(14)
            max_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MaxPresets)
            self.step(15)
            mptz_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPresets)

            self.step(16)
            asserts.assert_less_equal(len(mptz_presets_dut), max_presets_dut,
                                      "MPTZPresets size is greater than the allowed max.")

            self.step(17)
            name = "newpreset"
            await self.send_save_preset_command(endpoint, name, presetID=max_presets_dut)

            self.step(18)
            mptz_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPresets)

            match_found = False
            for mptzpreset in mptz_presets_dut:
                if mptzpreset.presetID == max_presets_dut:
                    # verify that the values match
                    asserts.assert_equal(name, mptzpreset.name, "Preset name doesn't match that saved")
                    self.verify_preset_matches(mptzpreset, new_mptzposition_dut)
                    match_found = True
                    break

            if not match_found:
                asserts.assert_fail("No matching preset found for saved preset ID")
        else:
            self.skip_step(14)
            self.skip_step(15)
            self.skip_step(16)
            self.skip_step(17)
            self.skip_step(18)

        if self.has_feature_dptz:
            self.step(19)
            clusterAVSTR = Clusters.Objects.CameraAvStreamManagement
            attributesAVSTR = clusterAVSTR.Attributes

            # Ensure that the DUTs Viewport and Sensor Size are coherent, the viewport cannot be larger than the sensor
            sensordimensions = await self.read_avstr_attribute_expect_success(endpoint, attributesAVSTR.VideoSensorParams)
            viewport = await self.read_avstr_attribute_expect_success(endpoint, attributesAVSTR.Viewport)
            sensorSize = sensordimensions.sensorWidth * sensordimensions.sensorHeight
            viewportSize = (viewport.x2 - viewport.x1) * (viewport.y2 - viewport.y1)
            asserts.assert_less_equal(viewportSize, sensorSize, "Viewport is larger than the devices sensor max.")

            self.step(20)
            # Allocate the stream
            videoStreamID = await self.video_stream_allocate_command(endpoint)

            self.step(21)
            # Read DPTZStreams and verify that the stream and viewport are present
            if not await self.dptzstreamentryvalid(endpoint, videoStreamID, viewport):
                asserts.assert_fail("No matching stream id and viewport found in DPTZStreams for the allocated video stream")

        self.step(22)
        # Reboot DUT
        # Check if restart flag file is available (indicates test runner supports app restart)
        await self.request_device_reboot()

        self.step(23)
        self.step(24)
        new_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)

        if newPan is not None:
            asserts.assert_equal(new_mptzposition_dut.pan, newPan, "Received Pan does not match set Pan")

        if newTilt is not None:
            asserts.assert_equal(new_mptzposition_dut.tilt, newTilt, "Received Tilt does not match set Tilt")

        if newZoom is not None:
            asserts.assert_equal(new_mptzposition_dut.zoom, newZoom, "Received Zoom does not match set Zoom")

        if self.has_feature_mpresets:
            self.step(25)
            mptz_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPresets)

            match_found = False
            for mptzpreset in mptz_presets_dut:
                if mptzpreset.presetID == max_presets_dut:
                    # verify that the values match
                    asserts.assert_equal(name, mptzpreset.name, "Preset name doesn't match that saved")
                    self.verify_preset_matches(mptzpreset, new_mptzposition_dut)
                    match_found = True
                    break

            if not match_found:
                asserts.assert_fail("No matching preset found for saved preset ID")
        else:
            self.skip_step(25)

        if self.has_feature_dptz:
            self.step(26)
            # Read DPTZStreams and verify that the stream and viewport are present
            if not await self.dptzstreamentryvalid(endpoint, videoStreamID, viewport):
                asserts.assert_fail("No matching stream id and viewport found in DPTZStreams for the allocated video stream")
        else:
            self.skip_step(26)


if __name__ == "__main__":
    default_matter_test_main()
