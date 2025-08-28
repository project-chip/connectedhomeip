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

from mobly import asserts
from TC_AVSUMTestBase import AVSUMTestBase

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches


class TC_AVSUM_2_1(MatterBaseTest, AVSUMTestBase):

    def desc_TC_AVSUM_2_1(self) -> str:
        return "[TC-AVSUM-2.1] Attributes with DUT as Server"

    def steps_TC_AVSUM_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read and verify that one of MTILT, MPAN, MZOOM, or DPTZ is supported"),
            TestStep(3, "Read and verify ZoomMax attribute, if supported"),
            TestStep(4, "Read and verify TiltMin attribute, if supported"),
            TestStep(5, "Read and verify TiltMax attribute, if supported"),
            TestStep(6, "Read and verify PanMin attribute, if supported"),
            TestStep(7, "Read and verify PanMax attribute, if supported"),
            TestStep(8, "Read and verify MPTZPosition attribute, if supported."),
            TestStep(9, "Read and verify MaxPresets attribute, if supported."),
            TestStep(10, "Read and verify MPTZPresets attribute, if supported."),
            TestStep(11, "Verify the DPTZStreams attribute is present if the DPTZ feature is supported"),
            TestStep(12, "Ensure that a video stream has been allocated, store the streamID"),
            TestStep(13, "Read the DPTZStreams attribute. Verify the streamIDs are unique, and the allocated streamID is present"),
        ]
        return steps

    def pics_TC_AVSUM_2_1(self) -> list[str]:
        pics = [
            "AVSUM.S",
        ]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.CameraAvSettingsUserLevelManagement))
    async def test_TC_AVSUM_2_1(self):
        cluster = Clusters.Objects.CameraAvSettingsUserLevelManagement
        attributes = cluster.Attributes
        endpoint = self.get_endpoint(default=1)

        tilt_min_dut = tilt_max_dut = pan_min_dut = pan_max_dut = zoom_max_dut = None

        self.step(1)  # Already done, immediately go to step 2

        feature_map = await self.read_avsum_attribute_expect_success(endpoint, attributes.FeatureMap)
        self.has_feature_dptz = (feature_map & cluster.Bitmaps.Feature.kDigitalPTZ) != 0
        self.has_feature_mpan = (feature_map & cluster.Bitmaps.Feature.kMechanicalPan) != 0
        self.has_feature_mtilt = (feature_map & cluster.Bitmaps.Feature.kMechanicalTilt) != 0
        self.has_feature_mzoom = (feature_map & cluster.Bitmaps.Feature.kMechanicalZoom) != 0
        self.has_feature_mpresets = (feature_map & cluster.Bitmaps.Feature.kMechanicalPresets) != 0

        logging.info(
            f"Feature map: 0x{feature_map:x}. MPAN: {self.has_feature_mpan}, MTILT:{self.has_feature_mtilt}, MZOOM:{self.has_feature_mzoom}, DPTZ:{self.has_feature_dptz}")

        attribute_list = await self.read_avsum_attribute_expect_success(endpoint, attributes.AttributeList)

        self.step(2)
        if not (self.has_feature_mpan | self.has_feature_mtilt | self.has_feature_mzoom | self.has_feature_dptz):
            asserts.fail("One of MPAN, MTILT, MZOOM, or DPTZ is mandatory")

        if self.has_feature_mzoom:
            self.step(3)
            asserts.assert_in(attributes.ZoomMax.attribute_id, attribute_list,
                              "ZoomMax attribute is a mandatory attribute if MZOOM.")
            zoom_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.ZoomMax)
            asserts.assert_less_equal(zoom_max_dut, 100, "ZoomMax is not in valid range.")
            asserts.assert_greater_equal(zoom_max_dut, 2, "ZoomMax must be at least 2.")
        else:
            logging.info("MZOOM Feature not supported. Test step skipped")
            self.skip_step(3)

        if self.has_feature_mtilt:
            self.step(4)
            asserts.assert_in(attributes.TiltMin.attribute_id, attribute_list,
                              "TiltMin attribute is a mandatory attribute if MTILT.")
            tilt_min_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.TiltMin)
            asserts.assert_less_equal(tilt_min_dut, 179, "TiltMin is not in valid range.")
            asserts.assert_greater_equal(tilt_min_dut, -180, "TiltMin is not in valid range.")

            self.step(5)
            asserts.assert_in(attributes.TiltMax.attribute_id, attribute_list,
                              "TiltMax attribute is a mandatory attribute if MTILT.")
            tilt_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.TiltMax)
            asserts.assert_less_equal(tilt_max_dut, 180, "TiltMin is not in valid range.")
            asserts.assert_greater_equal(tilt_max_dut, -179, "TiltMin is not in valid range.")
        else:
            logging.info("MTILT Feature not supported. Test steps skipped")
            self.skip_step(4)
            self.skip_step(5)

        if self.has_feature_mpan:
            self.step(6)
            asserts.assert_in(attributes.PanMin.attribute_id, attribute_list,
                              "PanMin attribute is a mandatory attribute if MPAN.")
            pan_min_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.PanMin)
            asserts.assert_less_equal(pan_min_dut, 179, "PanMin is not in valid range.")
            asserts.assert_greater_equal(pan_min_dut, -180, "PanMin is not in valid range.")

            self.step(7)
            asserts.assert_in(attributes.PanMax.attribute_id, attribute_list,
                              "PanMax attribute is a mandatory attribute if MPAN.")
            pan_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.PanMax)
            asserts.assert_less_equal(pan_max_dut, 180, "PanMax is not in valid range.")
            asserts.assert_greater_equal(pan_max_dut, -179, "PanMax is not in valid range.")
        else:
            logging.info("MPAN Feature not supported. Test steps skipped")
            self.skip_step(6)
            self.skip_step(7)

        if self.has_feature_mpan | self.has_feature_mtilt | self.has_feature_mzoom:
            self.step(8)
            asserts.assert_in(attributes.MPTZPosition.attribute_id, attribute_list,
                              "MPTZPosition attribute is mandatory if one of MPAN, MTILT, or MZOOM.")
            mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
            self.ptz_range_validation(mptzposition_dut, tilt_min_dut, tilt_max_dut, pan_min_dut, pan_max_dut, zoom_max_dut)
        else:
            self.skip_step(8)

        if self.has_feature_mpresets:
            self.step(9)
            asserts.assert_in(attributes.MaxPresets.attribute_id, attribute_list,
                              "MaxPresets attribute is a mandatory attribute if MPRESETS.")

            # For now force a preset to be present so there is something to read
            await self.send_save_preset_command(endpoint, name="newpreset")

            max_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MaxPresets)

            self.step(10)
            asserts.assert_in(attributes.MPTZPresets.attribute_id, attribute_list,
                              "MPTZPresets attribute is a mandatory attribut if MPRESETS.")

            mptz_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPresets)
            if mptz_presets_dut is not None:
                asserts.assert_less_equal(len(mptz_presets_dut), max_presets_dut,
                                          "MPTZPresets size is greater than the allowed max.")
                for mptzpreset in mptz_presets_dut:
                    asserts.assert_less_equal(mptzpreset.presetID, max_presets_dut, "PresetID is out of range")
                    asserts.assert_less_equal(len(mptzpreset.name), 32, "Preset name is too long")
                    self.ptz_range_validation(mptzpreset.settings, tilt_min_dut, tilt_max_dut,
                                              pan_min_dut, pan_max_dut, zoom_max_dut)
            else:
                asserts.fail("MPTZPresets is empty, even after saving at least one entry.")
        else:
            logging.info("MPRESETS Feature not supported. Test steps skipped")
            self.skip_step(9)
            self.skip_step(10)

        if self.has_feature_dptz:
            self.step(11)
            asserts.assert_in(attributes.DPTZStreams.attribute_id, attribute_list,
                              "DPTZStreams attribute is a mandatory attribute if DPTZ.")

            self.step(12)
            # Make sure we have at least one video stream
            allocatedstream = await self.video_stream_allocate_command(endpoint)

            self.step(13)
            dptz_streams_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.DPTZStreams)
            if dptz_streams_dut is not None:
                # Verify that all elements in the list are unique
                foundids = []

                for streams in dptz_streams_dut:
                    asserts.assert_greater_equal(streams.videoStreamID, 0, "Provided video stream id is out of range")
                    foundids.append(streams.videoStreamID)

                asserts.assert_equal(len(foundids), len(set(foundids)), "DPTZStreams has non-unique values")
                if allocatedstream not in foundids:
                    asserts.assert_fail("DPTZStreams does not contain known allocated video stream id")
            else:
                asserts.assert_fail("DPTZStreams is empty, even though a stream has been allocated")
        else:
            logging.info("DPTZ Feature not supported. Test step skipped")
            self.skip_step(11)
            self.skip_step(12)
            self.skip_step(13)


if __name__ == "__main__":
    default_matter_test_main()
