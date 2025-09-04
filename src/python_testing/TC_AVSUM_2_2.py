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


class TC_AVSUM_2_2(MatterBaseTest, AVSUMTestBase):
    has_feature_mpan = False
    has_feature_mtilt = False
    has_feature_mzoom = False

    def desc_TC_AVSUM_2_2(self) -> str:
        return "[TC-AVSUM-2.2] MptzSetPosition Command Validation"

    def steps_TC_AVSUM_2_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read and verify MPTZPosition attribute."),
            TestStep(3, "Send an MPTZSetPosition command with no fields. Verify failure response"),
            TestStep(4, "If Pan is supported, read and verify the PanMin attribute. If not skip to step 14"),
            TestStep(5, "Read and verify the PanMax attribute."),
            TestStep(6, "Create a valid value for a Pan"),
            TestStep(7, "Set the new Pan value via the MPTZSetPosition command. Verify success response."),
            TestStep(8, "Read MPTZPosition. Verify the Pan value is that set in Step 7. Verify the Tilt and Zoom are unchanged."),
            TestStep(9, "If PIXIT.CANBEMADEBUSY is set, place the DUT into a state where it cannot accept a command. Else jump to step 11."),
            TestStep(10, "Send an MPTZSetPositionCommand with the previously set Pan value. Verify failure response."),
            TestStep(11, "Create an invalid value for a Pan."),
            TestStep(12, "Set the invalid value via the MPTZSetPosition command. Verify failure response."),
            TestStep(13, "Read MPTZPosition. Verify the Pan value is that set in Step 7."),
            TestStep(14, "If Tilt is supported, read and verify the TiltMin and TiltMax attributes. If not skip to step 22"),
            TestStep(15, "Read and verify the TiltMax attribute."),
            TestStep(16, "Create a valid value for a Tilt different from the initial value."),
            TestStep(17, "Set the new Tilt value via the MPTZSetPosition command. Verify success response."),
            TestStep(18, "Read MPTZPosition. Verify the Tilt value is that set in Step 17. Verify that Pan and Zoom are unchanged"),
            TestStep(19, "Create an invalid value for a Tilt."),
            TestStep(20, "Set the invalid value via the MPTZSetPosition command. Verify failure response."),
            TestStep(21, "Read MPTZPosition. Verify the Tilt value is that set in Step 17."),
            TestStep(22, "If Zoom is supported, read and verify the ZoomMax attribute."),
            TestStep(23, "Create a valid value for Zoom."),
            TestStep(24, "Set the new Zoom value via the MPTZSetPosition command. Verify success response."),
            TestStep(25, "Read MPTZPosition. Verify the Zoom value is that set in Step 24. Verify Pan and Tilt are unchanged"),
            TestStep(26, "Create an invalid value for a Zoom."),
            TestStep(27, "Set the invalid value via the MPTZSetPosition command. Verify failure response."),
            TestStep(28, "Read MPTZPosition. Verify the Zoom value is that set in Step 24."),
        ]
        return steps

    def pics_TC_AVSUM_2_2(self) -> list[str]:
        pics = [
            "AVSUM.S",
        ]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.CameraAvSettingsUserLevelManagement))
    async def test_TC_AVSUM_2_2(self):
        cluster = Clusters.Objects.CameraAvSettingsUserLevelManagement
        attributes = cluster.Attributes
        endpoint = self.get_endpoint(default=1)
        # Allow some user input steps to be skipped if running under CI
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

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
                          "MPTZPosition attribute is mandatory if the command is supported.")
        mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
        currentPan = mptzposition_dut.pan
        currentTilt = mptzposition_dut.tilt
        currentZoom = mptzposition_dut.zoom

        self.step(3)
        await self.send_null_mptz_set_position_command(endpoint, expected_status=Status.InvalidCommand)

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
                newPan = random.randint(pan_min_dut, pan_max_dut)
                if newPan != currentPan:
                    break

            self.step(7)
            # Invoke the command with the new Pan value
            await self.send_mptz_set_pan_position_command(endpoint, newPan)

            self.step(8)
            # Read the attribute back and make sure it was set
            newpan_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newpan_mptzposition_dut.pan, newPan, "Received Pan does not match set Pan")
            asserts.assert_equal(newpan_mptzposition_dut.tilt, currentTilt, "Tilt unexpectedly changed when only changing Pan")
            asserts.assert_equal(newpan_mptzposition_dut.zoom, currentZoom, "Zoom unexpectedly changed when only changing Pan")
            currentPan = newPan

            self.step(9)
            # PIXIT check
            canbemadebusy = self.user_params.get("PIXIT.CANBEMADEBUSY", False)

            if canbemadebusy:
                self.step(10)
                # Busy response check
                if not self.is_pics_sdk_ci_only:
                    self.wait_for_user_input(prompt_msg="Place device into a busy state. Hit ENTER once ready.")
                    await self.send_mptz_set_pan_position_command(endpoint, newPan, expected_status=Status.Busy)
            else:
                self.skip_step(10)

            self.step(11)
            # Create an out of range value for Pan, verify failure
            newPanFail = pan_max_dut + 10

            self.step(12)
            await self.send_mptz_set_pan_position_command(endpoint, newPanFail, expected_status=Status.ConstraintError)

            self.step(13)
            # Verify no change in the Pan value
            newpan_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newpan_mptzposition_dut.pan, newPan, "Received Pan does not match set Pan")
        else:
            self.skip_step(4)
            self.skip_step(5)
            self.skip_step(6)
            self.skip_step(7)
            self.skip_step(8)
            self.skip_step(9)
            self.skip_step(10)
            self.skip_step(11)
            self.skip_step(12)
            self.skip_step(13)

        if self.has_feature_mtilt:
            self.step(14)
            asserts.assert_in(attributes.TiltMin.attribute_id, attribute_list,
                              "TiltMin attribute is a mandatory attribute if MTILT.")
            tilt_min_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.TiltMin)
            asserts.assert_less_equal(tilt_min_dut, 179, "TiltMin is not in valid range.")
            asserts.assert_greater_equal(tilt_min_dut, -180, "TiltMin is not in valid range.")

            self.step(15)
            asserts.assert_in(attributes.TiltMax.attribute_id, attribute_list,
                              "TiltMax attribute is a mandatory attribute if MTILT.")
            tilt_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.TiltMax)
            asserts.assert_less_equal(tilt_max_dut, 180, "TiltMax is not in valid range.")
            asserts.assert_greater_equal(tilt_max_dut, -179, "TiltMax is not in valid range.")

            self.step(16)
            # Create new Value for Tilt
            while True:
                newTilt = random.randint(tilt_min_dut, tilt_max_dut)
                if newTilt != currentTilt:
                    break

            self.step(17)
            # Invoke the command with the new Tilt value
            await self.send_mptz_set_tilt_position_command(endpoint, newTilt)

            self.step(18)
            # Read the attribute back and make sure it was set
            newtilt_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newtilt_mptzposition_dut.tilt, newTilt, "Received Tilt does not match set Tilt")
            asserts.assert_equal(newtilt_mptzposition_dut.pan, currentPan, "Pan unexpectedly changed when only changing Tilt")
            asserts.assert_equal(newtilt_mptzposition_dut.zoom, currentZoom, "Zoom unexpectedly changed when only changing Tilt")
            currentTilt = newTilt

            self.step(19)
            # Create an out of range value for Tilt, verify failure
            newTiltFail = tilt_max_dut + 10

            self.step(20)
            await self.send_mptz_set_tilt_position_command(endpoint, newTiltFail, expected_status=Status.ConstraintError)

            self.step(21)
            # Verify no change in the Tilt value
            newtilt_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newtilt_mptzposition_dut.tilt, newTilt, "Received Tilt does not match set Tilt")
        else:
            self.skip_step(14)
            self.skip_step(15)
            self.skip_step(16)
            self.skip_step(17)
            self.skip_step(18)
            self.skip_step(19)
            self.skip_step(20)
            self.skip_step(21)

        if self.has_feature_mzoom:
            self.step(22)
            asserts.assert_in(attributes.ZoomMax.attribute_id, attribute_list,
                              "ZoomMax attribute is a mandatory attribute if MZOOM.")
            zoom_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.ZoomMax)
            asserts.assert_less_equal(zoom_max_dut, 100, "ZoomMax is not in valid range.")
            asserts.assert_greater_equal(zoom_max_dut, 2, "ZoomMax is not in valid range.")

            self.step(23)
            # Create new Value for Zoom
            while True:
                newZoom = random.randint(2, zoom_max_dut)
                if newZoom != currentZoom:
                    break

            self.step(24)
            # Invoke the command with the new Zoom value
            await self.send_mptz_set_zoom_position_command(endpoint, newZoom)

            self.step(25)
            # Read the attribute back and make sure it was set
            newzoom_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newzoom_mptzposition_dut.zoom, newZoom, "Received Zoom does not match set Zoom")

            self.step(26)
            # Create an out of range value for Zoom, verify failure
            newZoomFail = zoom_max_dut + 10

            self.step(27)
            await self.send_mptz_set_zoom_position_command(endpoint, newZoomFail, expected_status=Status.ConstraintError)

            self.step(28)
            # Verify no change in the Zoom value
            newzoom_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newzoom_mptzposition_dut.zoom, newZoom, "Received Zoom does not match set Zoom")
            asserts.assert_equal(newzoom_mptzposition_dut.pan, currentPan, "Pan unexpectedly changed when only changing Zoom")
            asserts.assert_equal(newzoom_mptzposition_dut.tilt, currentTilt, "Tilt unexpectedly changed when only changing Zoom")
        else:
            self.skip_step(22)
            self.skip_step(23)
            self.skip_step(24)
            self.skip_step(25)
            self.skip_step(26)
            self.skip_step(27)
            self.skip_step(28)


if __name__ == "__main__":
    default_matter_test_main()
