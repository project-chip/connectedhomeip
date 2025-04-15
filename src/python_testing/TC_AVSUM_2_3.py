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
#     app: ${ALL_CLUSTERS_APP}
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
import random

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_AVSUMTestBase import AVSUMTestBase

class TC_AVSUM_2_3(MatterBaseTest, AVSUMTestBase):
    has_feature_mpan = False
    has_feature_mtilt = False
    has_feature_mzoom = False

    def ptz_range_validation(self, mptz, tiltmin, tiltmax, panmin, panmax, zoommax):
        if self.has_feature_mpan:
            if mptz.pan == None:
                asserts.fail("Pan value missing when MPAN feature set")
            else: 
                apan = mptz.pan
                asserts.assert_greater_equal(apan, panmin, "Pan out of range of DUT defined values")
                asserts.assert_less_equal(apan, panmax, "Pan out of range of DUT defined values")
            
        if self.has_feature_mtilt:
            if mptz.tilt == None:
                asserts.fail("Tilt value missing when MTILT feature set")
            else: 
                atilt = mptz.tilt
                asserts.assert_greater_equal(atilt, tiltmin, "Tilt out of range of DUT defined values")
                asserts.assert_less_equal(atilt, tiltmax, "Tilt out of range of DUT defined values")

        if self.has_feature_mzoom:
            if mptz.zoom == None:
                asserts.fail("Zoom value missing when MZOOM feature set")
            else: 
                azoom = mptz.zoom
                asserts.assert_greater_equal(azoom, 1, "Zoom out of range of DUT defined values")
                asserts.assert_less_equal(azoom, zoommax, "Zoom out of range of DUT defined values")

    def desc_TC_AVSUM_2_3(self) -> str:
        return "[TC-AVSUM-2.3] MptzRelativeMove Command Validation"

    def steps_TC_AVSUM_2_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read and verify MPTZPosition attribute."),
            TestStep(3, "Validate changes to the Pan value, if supported."),
            TestStep(4, "Validate changes to the Tilt value, if supported."),
            TestStep(5, "Validate changes to the Zoom value, if supported"),
        ]
        return steps

    def pics_TC_AVSUM_2_3(self) -> list[str]:
        pics = [
            "AVSUM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_AVSUM_2_3(self, endpoint=1):
        cluster = Clusters.Objects.CameraAvSettingsUserLevelManagement
        attributes = cluster.Attributes

        self.step(1)  # Already done, immediately go to step 2

        feature_map = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute="FeatureMap")
        self.has_feature_mpan = (feature_map & cluster.Bitmaps.Feature.kMechanicalPan) != 0
        self.has_feature_mtilt = (feature_map & cluster.Bitmaps.Feature.kMechanicalTilt) != 0
        self.has_feature_mzoom = (feature_map & cluster.Bitmaps.Feature.kMechanicalZoom) != 0

        attribute_list = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute="AttributeList")

        if not(self.has_feature_mpan | self.has_feature_mtilt | self.has_feature_mzoom):
            asserts.fail("One of MPAN, MTILT, or MZOOM is mandatory")
            self.skip_all_remaining_steps(2)

        self.step(2)
        asserts.assert_in(attributes.MPTZPosition.attribute_id, attribute_list, "MPTZPosition attribute is mandatory.")
        mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute="MPTZPosition")
        initialPan = mptzposition_dut.pan
        initialTilt = mptzposition_dut.tilt
        initialZoom = mptzposition_dut.zoom

        if self.has_feature_mpan:
            self.step(3)
            asserts.assert_in(attributes.PanMin.attribute_id, attribute_list,
                              "PanMin attribute is a mandatory attribute if MPAN.")
            pan_min_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute="PanMin")
            asserts.assert_less_equal(pan_min_dut, 179, "PanMin is not in valid range.")
            asserts.assert_greater_equal(pan_min_dut, -180, "PanMin is not in valid range.")

            asserts.assert_in(attributes.PanMax.attribute_id, attribute_list,
                              "PanMax attribute is a mandatory attribute if MPAN.")
            pan_max_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute="PanMax")    
            asserts.assert_less_equal(pan_max_dut, 180, "PanMax is not in valid range.")
            asserts.assert_greater_equal(pan_max_dut, -179, "PanMax is not in valid range.")

            # Create new Value for Pan
            while True: 
                newPan = random.randint(pan_min_dut, pan_max_dut)
                if newPan != initialPan:
                    break
            
            # Calulate the difference, this is the relative move
            
            relativePan = abs(initialPan - newPan) if (initialPan < newPan) else -abs(newPan - initialPan)
            logging.info("***Current Pan value %s", initialPan)
            logging.info("***Target new Pan value %s", newPan)
            logging.info("***RelativePan to be sent %s", relativePan)

            # Invoke the command with the new pan value
            await self.send_mptz_relative_move_pan_command(endpoint, relativePan)

            # Read the attribute back and make sure it was set
            newpan_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute="MPTZPosition")
            asserts.assert_equal(newpan_mptzposition_dut.pan, newPan, "Received Pan does not match set Pan")

            # Create an out of range value for Pan, verify it's clipped to PanMax
            newPanFail = abs(pan_max_dut - newPan) + 10
            await self.send_mptz_relative_move_pan_command(endpoint, newPanFail)

            newpan_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute="MPTZPosition")
            asserts.assert_equal(newpan_mptzposition_dut.pan, pan_max_dut, "Received Pan does not match PanMax")
        else:
            self.skip_step(3)

        if self.has_feature_mtilt:
            self.step(4)
            asserts.assert_in(attributes.TiltMin.attribute_id, attribute_list,
                              "TiltMin attribute is a mandatory attribute if MTILT.")
            tilt_min_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute="TiltMin")
            asserts.assert_less_equal(tilt_min_dut, 179, "TiltMin is not in valid range.")
            asserts.assert_greater_equal(tilt_min_dut, -180, "TiltMin is not in valid range.")

            asserts.assert_in(attributes.TiltMax.attribute_id, attribute_list,
                              "TiltMax attribute is a mandatory attribute if MTILT.")
            tilt_max_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute="TiltMax")    
            asserts.assert_less_equal(tilt_max_dut, 180, "TiltMax is not in valid range.")
            asserts.assert_greater_equal(tilt_max_dut, -179, "TiltMax is not in valid range.")

            # Create new Value for Tilt
            while True: 
                newTilt = random.randint(tilt_min_dut, tilt_max_dut)
                if newTilt != initialTilt:
                    break
            
            relativeTilt = abs(initialTilt - newTilt) if (initialTilt < newTilt) else -abs(newTilt - initialTilt)

            # Invoke the command with the new tilt value
            await self.send_mptz_relative_move_tilt_command(endpoint, relativeTilt)

            # Read the attribute back and make sure it was set
            newtilt_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute="MPTZPosition")
            asserts.assert_equal(newtilt_mptzposition_dut.tilt, newTilt, "Received Tilt does not match set Tilt")

            # Create an out of range value for Tilt, verify it's clipped to TiltMax
            newTiltFail = abs(tilt_max_dut - newTilt) + 10
            await self.send_mptz_relative_move_tilt_command(endpoint, newTiltFail)

            newtilt_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute="MPTZPosition")
            asserts.assert_equal(newtilt_mptzposition_dut.tilt, tilt_max_dut, "Received Tilt does not match TiltMax")
        else:
            self.skip_step(4)

        if self.has_feature_mzoom:
            self.step(5)
            asserts.assert_in(attributes.ZoomMax.attribute_id, attribute_list,
                              "ZoomMax attribute is a mandatory attribute if MZOOM.")
            zoom_max_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute="ZoomMax")    
            asserts.assert_less_equal(zoom_max_dut, 100, "ZoomMax is not in valid range.")
            asserts.assert_greater_equal(zoom_max_dut, 2, "ZoomMax is not in valid range.")

            # Create new Value for Zoom
            while True: 
                newZoom = random.randint(2, zoom_max_dut)
                if newZoom != initialZoom:
                    break
            
            relativeZoom = abs(initialZoom - newZoom) if (initialZoom < newZoom) else -abs(newZoom - initialZoom)

            # Invoke the command with the new pan value
            await self.send_mptz_relative_move_zoom_command(endpoint, relativeZoom)

            # Read the attribute back and make sure it was set
            newzoom_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute="MPTZPosition")
            asserts.assert_equal(newzoom_mptzposition_dut.zoom, newZoom, "Received Zoom does not match set Zoom")

            # Create an out of range value for Zoom, verify it's clipped to ZoomMax
            newZoomFail = abs(zoom_max_dut - newZoom) + 10
            await self.send_mptz_relative_move_zoom_command(endpoint, newZoomFail)

            newzoom_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute="MPTZPosition")
            asserts.assert_equal(newzoom_mptzposition_dut.zoom, zoom_max_dut, "Received Zoom does not match ZoomMax")
        else:
            self.skip_step(5)

if __name__ == "__main__":
    default_matter_test_main()
