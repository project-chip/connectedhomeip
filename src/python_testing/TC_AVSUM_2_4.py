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

import logging

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts
from TC_AVSUMTestBase import AVSUMTestBase


class TC_AVSUM_2_4(MatterBaseTest, AVSUMTestBase):
    has_feature_dptz = False
    has_feature_mpan = False
    has_feature_mtilt = False
    has_feature_mzoom = False
    has_feature_mpresets = False

    def ptz_range_validation(self, mptz, tiltmin, tiltmax, panmin, panmax, zoommax):
        if self.has_feature_mpan:
            if mptz.pan is None:
                asserts.fail("Pan value missing when MPAN feature set")
            else:
                apan = mptz.pan
                asserts.assert_greater_equal(apan, panmin, "Pan out of range of DUT defined values")
                asserts.assert_less_equal(apan, panmax, "Pan out of range of DUT defined values")

        if self.has_feature_mtilt:
            if mptz.tilt is None:
                asserts.fail("Tilt value missing when MTILT feature set")
            else:
                atilt = mptz.tilt
                asserts.assert_greater_equal(atilt, tiltmin, "Tilt out of range of DUT defined values")
                asserts.assert_less_equal(atilt, tiltmax, "Tilt out of range of DUT defined values")

        if self.has_feature_mzoom:
            if mptz.zoom is None:
                asserts.fail("Zoom value missing when MZOOM feature set")
            else:
                azoom = mptz.zoom
                asserts.assert_greater_equal(azoom, 1, "Zoom out of range of DUT defined values")
                asserts.assert_less_equal(azoom, zoommax, "Zoom out of range of DUT defined values")

    def verify_preset_matches(self, preset, position):
        if self.has_feature_mpan:
            asserts.assert_equal(preset.settings.pan, position.pan)

        if self.has_feature_mtilt:
            asserts.assert_equal(preset.settings.tilt, position.tilt)

        if self.has_feature_mzoom:
            asserts.assert_equal(preset.settings.zoom, position.zoom)

    def desc_TC_AVSUM_2_4(self) -> str:
        return "[TC-AVSUM-2.4] Attributes with DUT as Server"

    def steps_TC_AVSUM_2_4(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Verify that the MPRESETS feature is supported, exit otherwise"),
            TestStep(3, "Read the value of MaxPresets, fail if unsupported."),
            TestStep(4, "Read the value of MPTZPresets, fail if unsupported"),
            TestStep(5, "If MPTZPresets is empty, jump to step 9"),
            TestStep(6, "Verify that the size of the Presets List is not greater than MaxPresets"),
            TestStep(7, "Loop over the supported presets, for each verify the PresetID and Name are in range"),
            TestStep(8, "For each found preset, invoke MoveToPreset with the PresetID. Verify that the MPTZPosition is that from the Preset"),
            TestStep(9, "Exit the testcase"),
            TestStep(10,"Create a new saved preset with PresetID and MaxPresets"),
            TestStep(11,"Move to a new MPTZPosition that is the mid-point of all support PTZ attributes"),
            TestStep(12,"Move to the newly saved preset"),
            TestStep(13,"Verify that the MPTZPosition is that of the preset"),
        ]
        return steps

    def pics_TC_AVSUM_2_4(self) -> list[str]:
        pics = [
            "AVSUM.S",
        ]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.CameraAvSettingsUserLevelManagement))
    async def test_TC_AVSUM_2_4(self):
        cluster = Clusters.Objects.CameraAvSettingsUserLevelManagement
        attributes = cluster.Attributes
        tilt_min_dut = tilt_max_dut = pan_min_dut = pan_max_dut = zoom_max_dut = None

        self.step(1)  # Already done, immediately go to step 2

        feature_map = await self.read_avsum_attribute_expect_success(attribute="FeatureMap")
        self.has_feature_dptz = (feature_map & cluster.Bitmaps.Feature.kDigitalPTZ) != 0
        self.has_feature_mpan = (feature_map & cluster.Bitmaps.Feature.kMechanicalPan) != 0
        self.has_feature_mtilt = (feature_map & cluster.Bitmaps.Feature.kMechanicalTilt) != 0
        self.has_feature_mzoom = (feature_map & cluster.Bitmaps.Feature.kMechanicalZoom) != 0
        self.has_feature_mpresets = (feature_map & cluster.Bitmaps.Feature.kMechanicalPresets) != 0

        # Get the Max and Min values
        if self.has_feature_mzoom:
            zoom_max_dut = await self.read_avsum_attribute_expect_success(attribute="ZoomMax")

        if self.has_feature_mtilt:
            tilt_min_dut = await self.read_avsum_attribute_expect_success(attribute="TiltMin")
            tilt_max_dut = await self.read_avsum_attribute_expect_success(attribute="TiltMax")

        if self.has_feature_mpan:
            pan_min_dut = await self.read_avsum_attribute_expect_success(attribute="PanMin")
            pan_max_dut = await self.read_avsum_attribute_expect_success(attribute="PanMax")

        attribute_list = await self.read_avsum_attribute_expect_success(attribute="AttributeList")

        # If Presets is empty, change MPTZPosition to some new value, save that as a Preset.  Change to a different value. 
        # Check that.
        # Move to a discovered preset, or the created Preset.  Verify content.
        self.step(2) 
        if self.has_feature_mpresets:
            self.step(3) 
            asserts.assert_in(attributes.MaxPresets.attribute_id, attribute_list,
                              "MaxPresets attribute is a mandatory attribute if MPRESETS.")
            max_presets_dut = await self.read_avsum_attribute_expect_success(attribute="MaxPresets")

            self.step(4) 
            asserts.assert_in(attributes.MPTZPresets.attribute_id, attribute_list,
                              "MPTZPresets attribute is a mandatory attribut if MPRESETS.")

            mptz_presets_dut = await self.read_avsum_attribute_expect_success(attribute="MPTZPresets")

            self.step(5) 
            if mptz_presets_dut:
                self.step(6) 
                asserts.assert_less_equal(len(mptz_presets_dut), max_presets_dut,
                                          "MPTZPresets size is greater than the allowed max.")

                self.step(7)
                self.step(8)
                for mptzpreset in mptz_presets_dut:
                    asserts.assert_less_equal(mptzpreset.presetID, max_presets_dut, "PresetID is out of range")
                    asserts.assert_less_equal(len(mptzpreset.name), 32, "Preset name is too long")
                    self.ptz_range_validation(mptzpreset.settings, tilt_min_dut, tilt_max_dut,
                                              pan_min_dut, pan_max_dut, zoom_max_dut)
                    # Move to the Preset
                    await self.send_move_to_preset_command(mptzpreset.presetID)
                    mptzposition_dut = await self.read_avsum_attribute_expect_success(attribute="MPTZPosition")
                    self.verify_preset_matches(mptzpreset, mptzposition_dut)
                self.step(8)
                self.skip_all_remaining_tests(9)
            else:
                self.skip_step(6)
                self.skip_step(7)
                self.skip_step(8)
                self.skip_step(9)
                self.step(10)
                # For now force a preset to be present so there is something to read
                await self.send_save_presets_command(name="newpreset", presetID=max_presets_dut)
                stored_preset = await self.read_avsum_attribute_expect_success(attribute="MPTZPresets")
                newPan = newTilt = newZoom = None
                if self.has_feature_mpan:
                    newPan = (pan_max_dut - pan_min_dut)//2
                if self.has_feature_mtilt:
                    newTilt = (tilt_max_dut - tilt_min_dut)//2
                if self.has_feature_mzoom:
                    newZoom = (zoom_max_dut)//2

                self.step(11)
                await self.send_mptz_set_position_command(newPan, newTilt, newZoom)

                self.step(12)
                await self.send_move_to_preset_command(max_presets_dut)
                mptzposition_dut = await self.read_avsum_attribute_expect_success(attribute="MPTZPosition")

                self.step(13)
                self.verify_preset_matches(stored_preset[0], mptzposition_dut)

        else:
            logging.info("MPRESETS Feature not supported. Test steps skipped")


if __name__ == "__main__":
    default_matter_test_main()
