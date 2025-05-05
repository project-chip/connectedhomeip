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

import random

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts
from TC_AVSUMTestBase import AVSUMTestBase


class TC_AVSUM_2_3(MatterBaseTest, AVSUMTestBase):

    def desc_TC_AVSUM_2_3(self) -> str:
        return "[TC-AVSUM-2.3] MptzRelativeMove Command Validation"

    def steps_TC_AVSUM_2_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read and verify MPTZPosition attribute."),
            TestStep(3, "If Pan is supported, read and verify PanMin and PanMax attributes. If not skip to step 10"),
            TestStep(4, "Create a valid value for a Pan, calculate the relative distance from the current Pan."),
            TestStep(5, "Set via MPTZRelativeMove command the relative Pan. Verify success response."),
            TestStep(6, "Read MPTZPosition. Verify the Pan value is that set in Step 5."),
            TestStep(7, "Create an invalid value for a relative Pan that would exceed PanMax."),
            TestStep(8, "Set via MPTZSetRelativeMove command the relative Pan. Verify success response."),
            TestStep(9, "Read MPTZPosition. Verify the Pan value is set to PanMax."),
            TestStep(10, "If Tilt is supported, read and verify TiltMin and TiltMax attributes. If not skip to step 17"),
            TestStep(11, "Create a valid value for a Tilt, calculate the relative distance from the current Tilt."),
            TestStep(12, "Set via MPTZRelativeMove command the relative Tilt. Verify success response."),
            TestStep(13, "Read MPTZPosition. Verify the Tilt value is that set in Step 12."),
            TestStep(14, "Create an invalid value for a relative Tilt that would exceed TiltMax."),
            TestStep(15, "Set via MPTZSetRelativeMove command the relative Tilt. Verify success response."),
            TestStep(16, "Read MPTZPosition. Verify the Tilt value is set to TiltMax."),
            TestStep(17, "If Zoom is supported, read and verify the ZoomMax attribute."),
            TestStep(18, "Create a valid value for a Zoom, calculate the relative distance from the current Zoom."),
            TestStep(19, "Set via MPTZRelativeMove command the relative Zoom. Verify success response."),
            TestStep(20, "Read MPTZPosition. Verify the Zoom value is that set in Step 19."),
            TestStep(21, "Create an invalid value for a relative Zoom that would exceed ZoomMax."),
            TestStep(22, "Set via MPTZSetRelativeMove command the relative Zoom. Verify success response."),
            TestStep(23, "Read MPTZPosition. Verify the Zoom value is set to ZoomMax."),
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
            asserts.fail("One of MPAN, MTILT, or MZOOM is mandatory")

        self.step(2)
        asserts.assert_in(attributes.MPTZPosition.attribute_id, attribute_list, "MPTZPosition attribute is mandatory.")
        mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
        initialPan = mptzposition_dut.pan
        initialTilt = mptzposition_dut.tilt
        initialZoom = mptzposition_dut.zoom

        if self.has_feature_mpan:
            self.step(3)
            asserts.assert_in(attributes.PanMin.attribute_id, attribute_list,
                              "PanMin attribute is a mandatory attribute if MPAN.")
            pan_min_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.PanMin)
            asserts.assert_less_equal(pan_min_dut, 179, "PanMin is not in valid range.")
            asserts.assert_greater_equal(pan_min_dut, -180, "PanMin is not in valid range.")

            asserts.assert_in(attributes.PanMax.attribute_id, attribute_list,
                              "PanMax attribute is a mandatory attribute if MPAN.")
            pan_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.PanMax)
            asserts.assert_less_equal(pan_max_dut, 180, "PanMax is not in valid range.")
            asserts.assert_greater_equal(pan_max_dut, -179, "PanMax is not in valid range.")

            self.step(4)
            # Create new Value for Pan
            while True:
                newPan = random.randint(pan_min_dut+1, pan_max_dut)
                if newPan != initialPan:
                    break

            # Calulate the difference, this is the relative move
            relativePan = abs(initialPan - newPan) if (initialPan < newPan) else -abs(newPan - initialPan)

            self.step(5)
            # Invoke the command with the new Pan value
            await self.send_mptz_relative_move_pan_command(endpoint, relativePan)

            self.step(6)
            # Read the attribute back and make sure it was set
            newpan_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newpan_mptzposition_dut.pan, newPan, "Received Pan does not match set Pan")

            self.step(7)
            # Create an out of range value for Pan, verify it's clipped to PanMax
            newPanFail = abs(pan_max_dut - pan_min_dut)

            self.step(8)
            await self.send_mptz_relative_move_pan_command(endpoint, newPanFail)

            self.step(9)
            newpan_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newpan_mptzposition_dut.pan, pan_max_dut, "Received Pan does not match PanMax")
        else:
            self.skip_step(3)
            self.skip_step(4)
            self.skip_step(5)
            self.skip_step(6)
            self.skip_step(7)
            self.skip_step(8)
            self.skip_step(9)

        if self.has_feature_mtilt:
            self.step(10)
            asserts.assert_in(attributes.TiltMin.attribute_id, attribute_list,
                              "TiltMin attribute is a mandatory attribute if MTILT.")
            tilt_min_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.TiltMin)
            asserts.assert_less_equal(tilt_min_dut, 179, "TiltMin is not in valid range.")
            asserts.assert_greater_equal(tilt_min_dut, -180, "TiltMin is not in valid range.")

            asserts.assert_in(attributes.TiltMax.attribute_id, attribute_list,
                              "TiltMax attribute is a mandatory attribute if MTILT.")
            tilt_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.TiltMax)
            asserts.assert_less_equal(tilt_max_dut, 180, "TiltMax is not in valid range.")
            asserts.assert_greater_equal(tilt_max_dut, -179, "TiltMax is not in valid range.")

            self.step(11)
            # Create new Value for Tilt
            while True:
                newTilt = random.randint(tilt_min_dut+1, tilt_max_dut)
                if newTilt != initialTilt:
                    break

            relativeTilt = abs(initialTilt - newTilt) if (initialTilt < newTilt) else -abs(newTilt - initialTilt)

            self.step(12)
            # Invoke the command with the new Tilt value
            await self.send_mptz_relative_move_tilt_command(endpoint, relativeTilt)

            self.step(13)
            # Read the attribute back and make sure it was set
            newtilt_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newtilt_mptzposition_dut.tilt, newTilt, "Received Tilt does not match set Tilt")

            self.step(14)
            # Create an out of range value for Tilt, verify it's clipped to TiltMax
            newTiltFail = abs(tilt_max_dut - tilt_min_dut)

            self.step(15)
            await self.send_mptz_relative_move_tilt_command(endpoint, newTiltFail)

            self.step(16)
            newtilt_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newtilt_mptzposition_dut.tilt, tilt_max_dut, "Received Tilt does not match TiltMax")
        else:
            self.skip_step(10)
            self.skip_step(11)
            self.skip_step(12)
            self.skip_step(13)
            self.skip_step(14)
            self.skip_step(15)
            self.skip_step(16)

        if self.has_feature_mzoom:
            self.step(17)
            asserts.assert_in(attributes.ZoomMax.attribute_id, attribute_list,
                              "ZoomMax attribute is a mandatory attribute if MZOOM.")
            zoom_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.ZoomMax)
            asserts.assert_less_equal(zoom_max_dut, 100, "ZoomMax is not in valid range.")
            asserts.assert_greater_equal(zoom_max_dut, 2, "ZoomMax is not in valid range.")

            self.step(18)
            # Create new Value for Zoom
            while True:
                newZoom = random.randint(2, zoom_max_dut)
                if newZoom != initialZoom:
                    break

            relativeZoom = abs(initialZoom - newZoom) if (initialZoom < newZoom) else -abs(newZoom - initialZoom)

            self.step(19)
            # Invoke the command with the new Zoom value
            await self.send_mptz_relative_move_zoom_command(endpoint, relativeZoom)

            self.step(20)
            # Read the attribute back and make sure it was set
            newzoom_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newzoom_mptzposition_dut.zoom, newZoom, "Received Zoom does not match set Zoom")

            self.step(21)
            # Create an out of range value for Zoom, verify it's clipped to ZoomMax
            newZoomFail = zoom_max_dut - 1

            self.step(22)
            await self.send_mptz_relative_move_zoom_command(endpoint, newZoomFail)

            self.step(23)
            newzoom_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newzoom_mptzposition_dut.zoom, zoom_max_dut, "Received Zoom does not match ZoomMax")
        else:
            self.skip_step(17)
            self.skip_step(18)
            self.skip_step(19)
            self.skip_step(20)
            self.skip_step(21)
            self.skip_step(22)
            self.skip_step(23)


if __name__ == "__main__":
    default_matter_test_main()
