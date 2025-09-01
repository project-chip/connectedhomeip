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
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches


class TC_AVSUM_2_3(MatterBaseTest, AVSUMTestBase):

    def desc_TC_AVSUM_2_3(self) -> str:
        return "[TC-AVSUM-2.3] MptzRelativeMove Command Validation"

    def steps_TC_AVSUM_2_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read and verify MPTZPosition attribute."),
            TestStep(3, "Send an MPTZRelativeMove command with no fields. Verify failure response"),
            TestStep(4, "If Pan is supported, read and verify the PanMin attribute. If not skip to step 12"),
            TestStep(5, "Read and verify the PanMax attribute."),
            TestStep(6, "Create a valid value for a Pan, calculate the relative distance from the current Pan."),
            TestStep(7, "Set via MPTZRelativeMove command the relative Pan. Verify success response."),
            TestStep(8, "Read MPTZPosition. Verify the Pan value is that set in Step 7. Verify the Tilt and Zoom are unchanged."),
            TestStep(9, "Create an invalid value for a relative Pan that would exceed PanMax."),
            TestStep(10, "Set via MPTZSetRelativeMove command the invalid relative Pan. Verify success response."),
            TestStep(11, "Read MPTZPosition. Verify the Pan value is set to PanMax."),
            TestStep(12, "If Tilt is supported, read and verify the TiltMin attribute. If not skip to step 20"),
            TestStep(13, "Read and verify the TiltMax attribute."),
            TestStep(14, "Create a valid value for a Tilt, calculate the relative distance from the current Tilt."),
            TestStep(15, "Set via MPTZRelativeMove command the relative Tilt. Verify success response."),
            TestStep(16, "Read MPTZPosition. Verify the Tilt value is that set in Step 15. Verify that Pan and Zoom are unchanged."),
            TestStep(17, "Create an invalid value for a relative Tilt that would exceed TiltMax."),
            TestStep(18, "Set via MPTZSetRelativeMove command the relative Tilt. Verify success response."),
            TestStep(19, "Read MPTZPosition. Verify the Tilt value is set to TiltMax."),
            TestStep(20, "If Zoom is supported, read and verify the ZoomMax attribute."),
            TestStep(21, "Create a valid value for a Zoom, calculate the relative distance from the current Zoom."),
            TestStep(22, "Set via MPTZRelativeMove command the relative Zoom. Verify success response."),
            TestStep(23, "Read MPTZPosition. Verify the Zoom value is that set in Step 22. Verify the Pan and Tilt are unchanged"),
            TestStep(24, "Create an invalid value for a relative Zoom that would exceed ZoomMax."),
            TestStep(25, "Set via MPTZSetRelativeMove command the relative Zoom. Verify success response."),
            TestStep(26, "Read MPTZPosition. Verify the Zoom value is set to ZoomMax."),
            TestStep(27, "If PIXIT.CANBEMADEBUSY is set, place the DUT into a state where it cannot accept a command. Else end the test case."),
            TestStep(28, "Send an MPTZRelativeMove Command with any previously set relative value. Verify busy failure response."),
        ]
        return steps

    def pics_TC_AVSUM_2_3(self) -> list[str]:
        pics = [
            "AVSUM.S",
        ]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.CameraAvSettingsUserLevelManagement))
    async def test_TC_AVSUM_2_3(self):
        cluster = Clusters.Objects.CameraAvSettingsUserLevelManagement
        attributes = cluster.Attributes
        endpoint = self.get_endpoint(default=1)

        self.step(1)  # Already done, immediately go to step 2

        feature_map = await self.read_avsum_attribute_expect_success(endpoint, attributes.FeatureMap)
        self.has_feature_mpan = (feature_map & cluster.Bitmaps.Feature.kMechanicalPan) != 0
        self.has_feature_mtilt = (feature_map & cluster.Bitmaps.Feature.kMechanicalTilt) != 0
        self.has_feature_mzoom = (feature_map & cluster.Bitmaps.Feature.kMechanicalZoom) != 0

        attribute_list = await self.read_avsum_attribute_expect_success(endpoint, attributes.AttributeList)

        if not (self.has_feature_mpan | self.has_feature_mtilt | self.has_feature_mzoom):
            asserts.fail("One of MPAN, MTILT, or MZOOM is mandatory for command support")

        self.step(2)
        asserts.assert_in(attributes.MPTZPosition.attribute_id, attribute_list,
                          "MPTZPosition attribute is mandatory for command support.")
        mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
        currentPan = mptzposition_dut.pan
        currentTilt = mptzposition_dut.tilt
        currentZoom = mptzposition_dut.zoom

        self.step(3)
        await self.send_null_mptz_relative_move_command(endpoint, expected_status=Status.InvalidCommand)

        relativePan = relativeTilt = relativeZoom = None

        if self.has_feature_mpan:
            self.step(4)
            asserts.assert_in(attributes.PanMin.attribute_id, attribute_list,
                              "PanMin attribute is a mandatory attribute if MPAN.")
            pan_min_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.PanMin)
            asserts.assert_less_equal(pan_min_dut, 179, "PanMin is not in valid range.")
            asserts.assert_greater_equal(pan_min_dut, -180, "PanMin is not in valid range.")

            self.step(5)
            asserts.assert_in(attributes.PanMax.attribute_id, attribute_list,
                              "PanMax attribute is a mandatory attribute if MPAN.")
            pan_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.PanMax)
            asserts.assert_less_equal(pan_max_dut, 180, "PanMax is not in valid range.")
            asserts.assert_greater_equal(pan_max_dut, -179, "PanMax is not in valid range.")

            self.step(6)
            # Create new Value for Pan
            while True:
                newPan = random.randint(pan_min_dut+1, pan_max_dut)
                if newPan != currentPan:
                    break

            # Calulate the difference, this is the relative move
            relativePan = abs(currentPan - newPan) if (currentPan < newPan) else -abs(newPan - currentPan)

            self.step(7)
            # Invoke the command with the new Pan value
            await self.send_mptz_relative_move_pan_command(endpoint, relativePan)

            self.step(8)
            # Read the attribute back and make sure it was set
            newpan_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newpan_mptzposition_dut.pan, newPan, "Received Pan does not match set Pan")
            asserts.assert_equal(newpan_mptzposition_dut.tilt, currentTilt, "Tilt unexpectedly changed when only changing Pan")
            asserts.assert_equal(newpan_mptzposition_dut.zoom, currentZoom, "Zoom unexpectedly changed when only changing Pan")
            currentPan = newPan

            self.step(9)
            # Create an out of range value for Pan, verify it's clipped to PanMax
            newPanFail = abs(pan_max_dut - pan_min_dut)

            self.step(10)
            await self.send_mptz_relative_move_pan_command(endpoint, newPanFail)

            self.step(11)
            newpan_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newpan_mptzposition_dut.pan, pan_max_dut, "Received Pan does not match PanMax")
            currentPan = pan_max_dut

        else:
            self.skip_step(4)
            self.skip_step(5)
            self.skip_step(6)
            self.skip_step(7)
            self.skip_step(8)
            self.skip_step(9)
            self.skip_step(10)
            self.skip_step(11)

        if self.has_feature_mtilt:
            self.step(12)
            asserts.assert_in(attributes.TiltMin.attribute_id, attribute_list,
                              "TiltMin attribute is a mandatory attribute if MTILT.")
            tilt_min_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.TiltMin)
            asserts.assert_less_equal(tilt_min_dut, 179, "TiltMin is not in valid range.")
            asserts.assert_greater_equal(tilt_min_dut, -180, "TiltMin is not in valid range.")

            self.step(13)
            asserts.assert_in(attributes.TiltMax.attribute_id, attribute_list,
                              "TiltMax attribute is a mandatory attribute if MTILT.")
            tilt_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.TiltMax)
            asserts.assert_less_equal(tilt_max_dut, 180, "TiltMax is not in valid range.")
            asserts.assert_greater_equal(tilt_max_dut, -179, "TiltMax is not in valid range.")

            self.step(14)
            # Create new Value for Tilt
            while True:
                newTilt = random.randint(tilt_min_dut+1, tilt_max_dut)
                if newTilt != currentTilt:
                    break

            relativeTilt = abs(currentTilt - newTilt) if (currentTilt < newTilt) else -abs(newTilt - currentTilt)

            self.step(15)
            # Invoke the command with the new Tilt value
            await self.send_mptz_relative_move_tilt_command(endpoint, relativeTilt)

            self.step(16)
            # Read the attribute back and make sure it was set
            newtilt_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newtilt_mptzposition_dut.tilt, newTilt, "Received Tilt does not match set Tilt")
            asserts.assert_equal(newtilt_mptzposition_dut.pan, currentPan, "Pan unexpectedly changed when only changing Tilt")
            asserts.assert_equal(newtilt_mptzposition_dut.zoom, currentZoom, "Zoom unexpectedly changed when only changing Tilt")
            currentTilt = newTilt

            self.step(17)
            # Create an out of range value for Tilt, verify it's clipped to TiltMax
            newTiltFail = abs(tilt_max_dut - tilt_min_dut)

            self.step(18)
            await self.send_mptz_relative_move_tilt_command(endpoint, newTiltFail)

            self.step(19)
            newtilt_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newtilt_mptzposition_dut.tilt, tilt_max_dut, "Received Tilt does not match TiltMax")
            currentTilt = tilt_max_dut

        else:
            self.skip_step(12)
            self.skip_step(13)
            self.skip_step(14)
            self.skip_step(15)
            self.skip_step(16)
            self.skip_step(17)
            self.skip_step(18)
            self.skip_step(19)

        if self.has_feature_mzoom:
            self.step(20)
            asserts.assert_in(attributes.ZoomMax.attribute_id, attribute_list,
                              "ZoomMax attribute is a mandatory attribute if MZOOM.")
            zoom_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.ZoomMax)
            asserts.assert_less_equal(zoom_max_dut, 100, "ZoomMax is not in valid range.")
            asserts.assert_greater_equal(zoom_max_dut, 2, "ZoomMax is not in valid range.")

            self.step(21)
            # Create new Value for Zoom
            while True:
                newZoom = random.randint(2, zoom_max_dut)
                if newZoom != currentZoom:
                    break

            relativeZoom = abs(currentZoom - newZoom) if (currentZoom < newZoom) else -abs(newZoom - currentZoom)

            self.step(22)
            # Invoke the command with the new Zoom value
            await self.send_mptz_relative_move_zoom_command(endpoint, relativeZoom)

            self.step(23)
            # Read the attribute back and make sure it was set
            newzoom_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newzoom_mptzposition_dut.zoom, newZoom, "Received Zoom does not match set Zoom")
            asserts.assert_equal(newzoom_mptzposition_dut.pan, currentPan, "Pan unexpectedly changed when only changing Zoom")
            asserts.assert_equal(newzoom_mptzposition_dut.tilt, currentTilt, "Tilt unexpectedly changed when only changing Zoom")

            self.step(24)
            # Create an out of range value for Zoom, verify it's clipped to ZoomMax
            newZoomFail = zoom_max_dut - 1

            self.step(25)
            await self.send_mptz_relative_move_zoom_command(endpoint, newZoomFail)

            self.step(26)
            newzoom_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newzoom_mptzposition_dut.zoom, zoom_max_dut, "Received Zoom does not match ZoomMax")
        else:
            self.skip_step(20)
            self.skip_step(21)
            self.skip_step(22)
            self.skip_step(23)
            self.skip_step(24)
            self.skip_step(25)
            self.skip_step(26)

        self.step(27)
        # PIXIT check
        canbemadebusy = self.user_params.get("PIXIT.CANBEMADEBUSY", False)

        if canbemadebusy:
            self.step(28)
            # Busy response check
            if not self.is_pics_sdk_ci_only:
                self.wait_for_user_input(prompt_msg="Place device into a busy state. Hit ENTER once ready.")
                await self.send_mptz_relative_move_command(endpoint, relativePan, relativeTilt, relativeZoom, expected_status=Status.Busy)
        else:
            self.skip_step(28)


if __name__ == "__main__":
    default_matter_test_main()
