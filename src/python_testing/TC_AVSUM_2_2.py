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
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts
from TC_AVSUMTestBase import AVSUMTestBase


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
            TestStep(3, "If Pan is supported, read and verify PanMin and PanMax attributes. If not skip to step 8"),
            TestStep(4, "Create a valid value for a Pan, set via MPTZSetPosition command. Verify success response."),
            TestStep(5, "Read MPTZPosition. Verify the Pan value is that set in Step 4."),
            TestStep(6, "Create an invalid value for a Pan, set via MPTZSetPosition command. Verify failure response."),
            TestStep(7, "Read MPTZPosition. Verify the Pan value is that set in Step 4."),
            TestStep(8, "If Tilt is supported, read and verify TiltMin and TiltMax attributes. If not skip to step 13"),
            TestStep(9, "Create a valid value for a Tilt, set via MPTZSetPosition command. Verify success response."),
            TestStep(10, "Read MPTZPosition. Verify the Tilt value is that set in Step 9."),
            TestStep(11, "Create an invalid value for a Tilt, set via MPTZSetPosition command. Verify failure response."),
            TestStep(12, "Read MPTZPosition. Verify the Tilt value is that set in Step 9."),
            TestStep(13, "If Zoom is supported, read and verify the ZoomMax attribute."),
            TestStep(14, "Create a valid value for Zoom, set via MPTZSetPosition command. Verify success response."),
            TestStep(15, "Read MPTZPosition. Verify the Zoom value is that set in Step 14."),
            TestStep(16, "Create an invalid value for a Zoom, set via MPTZSetPosition command. Verify failure response."),
            TestStep(17, "Read MPTZPosition. Verify the Tilt value is that set in Step 14."),
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
                newPan = random.randint(pan_min_dut, pan_max_dut)
                if newPan != initialPan:
                    break

            # Invoke the command with the new Pan value
            await self.send_mptz_set_pan_position_command(endpoint, newPan)

            self.step(5)
            # Read the attribute back and make sure it was set
            newpan_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newpan_mptzposition_dut.pan, newPan, "Received Pan does not match set Pan")

            self.step(6)
            # Create an out of range value for Pan, verify failure
            newPanFail = pan_max_dut + 10
            await self.send_mptz_set_pan_position_command(endpoint, newPanFail, expected_status=Status.ConstraintError)

            self.step(7)
            # Verify no change in the Pan value
            newpan_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newpan_mptzposition_dut.pan, newPan, "Received Pan does not match set Pan")
        else:
            self.skip_step(3)
            self.skip_step(4)
            self.skip_step(5)
            self.skip_step(6)
            self.skip_step(7)

        if self.has_feature_mtilt:
            self.step(8)
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

            self.step(9)
            # Create new Value for Tilt
            while True:
                newTilt = random.randint(tilt_min_dut, tilt_max_dut)
                if newTilt != initialTilt:
                    break

            # Invoke the command with the new Tilt value
            await self.send_mptz_set_tilt_position_command(endpoint, newTilt)

            self.step(10)
            # Read the attribute back and make sure it was set
            newtilt_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newtilt_mptzposition_dut.tilt, newTilt, "Received Tilt does not match set Tilt")

            self.step(11)
            # Create an out of range value for Tilt, verify failure
            newTiltFail = tilt_max_dut + 10
            await self.send_mptz_set_tilt_position_command(endpoint, newTiltFail, expected_status=Status.ConstraintError)

            self.step(12)
            # Verify no change in the Tilt value
            newtilt_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newtilt_mptzposition_dut.tilt, newTilt, "Received Tilt does not match set Tilt")
        else:
            self.skip_step(8)
            self.skip_step(9)
            self.skip_step(10)
            self.skip_step(11)
            self.skip_step(12)

        if self.has_feature_mzoom:
            self.step(13)
            asserts.assert_in(attributes.ZoomMax.attribute_id, attribute_list,
                              "ZoomMax attribute is a mandatory attribute if MZOOM.")
            zoom_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.ZoomMax)
            asserts.assert_less_equal(zoom_max_dut, 100, "ZoomMax is not in valid range.")
            asserts.assert_greater_equal(zoom_max_dut, 2, "ZoomMax is not in valid range.")

            self.step(14)
            # Create new Value for Zoom
            while True:
                newZoom = random.randint(2, zoom_max_dut)
                if newZoom != initialZoom:
                    break

            # Invoke the command with the new Zoom value
            await self.send_mptz_set_zoom_position_command(endpoint, newZoom)

            self.step(15)
            # Read the attribute back and make sure it was set
            newzoom_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newzoom_mptzposition_dut.zoom, newZoom, "Received Zoom does not match set Zoom")

            self.step(16)
            # Create an out of range value for Zoom, verify failure
            newZoomFail = zoom_max_dut + 10
            await self.send_mptz_set_zoom_position_command(endpoint, newZoomFail, expected_status=Status.ConstraintError)

            self.step(17)
            # Verify no change in the Zoom value
            newzoom_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            asserts.assert_equal(newzoom_mptzposition_dut.zoom, newZoom, "Received Zoom does not match set Zoom")
        else:
            self.skip_step(13)
            self.skip_step(14)
            self.skip_step(15)
            self.skip_step(16)
            self.skip_step(17)


if __name__ == "__main__":
    default_matter_test_main()
