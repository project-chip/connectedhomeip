#
#    Copyright (c) 2024 Project CHIP Authors
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
#
#  There are CI issues to be followed up for the test cases below that implements manually controlling sensor device for
#  the occupancy state ON/OFF change.
#  [TC-OCC-3.1] test procedure step 4
#  [TC-OCC-3.2] test precedure step 3a, 3c

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

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_AVSUM_2_1(MatterBaseTest):
    async def read_avsum_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.CameraAvSettingsUserLevelManagement
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    def desc_TC_AVSUM_2_1(self) -> str:
        return "[TC-AVSUM-2.1] Attributes with DUT as Server"

    def steps_TC_AVSUM_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read MPTZPosition attribute, if supported."),
            TestStep(3, "Read MaxPresets attribute, if supported."),
            TestStep(4, "Read MPTZPresets attribute, if supported."),
            TestStep(5, "Read DPTZRelativeMove attribute, if supported"),
            TestStep(6, "Read ZoomMax attribute, if supported"),
            TestStep(7, "Read TiltMin attribute, if supported"),
            TestStep(8, "Read TiltMax attribute, if supported"),
            TestStep(9, "Read PanMin attribute, if supported"),
            TestStep(10, "Read PanMax attribute, if supported"),
        ]
        return steps

    def pics_TC_AVSUM_2_1(self) -> list[str]:
        pics = [
            "AVSUM.S",
        ]
        return pics

    @async_test_body
    async def test_TC_AVSUM_2_1(self):
        endpoint = self.matter_test_config.endpoint
        cluster = Clusters.Objects.CameraAvSettingsUserLevelManagement
        attributes = cluster.Attributes

        self.step(1)  # Already done, immediately go to step 2

        self.step(2)

        feature_map = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute=attributes.FeatureMap)
        has_feature_dptz = (feature_map & cluster.Bitmaps.Feature.kDigitalPTZ) != 0
        has_feature_mpan = (feature_map & cluster.Bitmaps.Feature.kMechanicalPan) != 0
        has_feature_mtilt = (feature_map & cluster.Bitmaps.Feature.kMechanicalTilt) != 0
        has_feature_mzoom = (feature_map & cluster.Bitmaps.Feature.kMechanicalZoom) != 0
        has_feature_mpresets = (feature_map & cluster.Bitmaps.Feature.kMechanicalPresets) != 0

        logging.info(
            f"Feature map: 0x{feature_map:x}. MPAN: {has_feature_mpan}, MTILT:{has_feature_mtilt}, MZOOM:{has_feature_mzoom}")

        attribute_list = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute=attributes.AttributeList)

        if not(has_feature_mpan | has_feature_mtilt | has_feature_mzoom):
            logging.info("One of MPAN, MTILT, or MZOOM is mandatory")
            self.skip_all_remaining_steps(3)

        if has_feature_mpan | has_feature_mtilt | has_feature_mzoom:
            asserts.assert_in(attributes.MPTZPosition.attribute_id, attribute_list, "MPTZPosition attribute is mandatory if MPAN or MTILT or MZOOM")
            mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute=attributes.MptzPosition)

        if has_feature_mpresets:
            self.step(3)
            asserts.assert_in(attributes.MaxPresets.attribute_id, attribute_list,
                          "MaxPresets attribute is a mandatory attribute if MPRESETS.")

            max_presets_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute=attributes.MaxPresets)

            self.step(4)
            asserts.assert_in(attributes.MPtzPresets.attribute_id, attribute_list,
                          "MPTZPresets attribute is a mandatory attribut if MPRESETS.")

            mptz_presets_bitmap_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute=attributes.MptzPresets)
        else:
            logging.info("MPRESETS Feature not supported. Test steps skipped")
            self.skip_step(3)
            self.skip_step(4)

        self.step(5)
        if has_feature_dptz:
            asserts.assert_in(attributes.DPtzRelativeMove.attribute_id, attribute_list,
                          "DPTZRelativeMove attribute is a mandatory attribute if DPTZ.")

            dptz_relative_move_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute=attributes.DptzRelativeMove)
        else:
            logging.info("DPTZ Feature not supported. Test step skipped")
            self.mark_current_step_skipped()

        self.step(6)
        if has_feature_mzoom:
            asserts.assert_in(attributes.ZoomMax.attribute_id, attribute_list, "ZoomMax attribute is a mandatory attribute if MZOOM.")
            zoom_max_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute=attributes.ZoomMax)
            asserts.assert_less_equal(zoom_max_dut.zoomMax, 0x0001, "ZoomMaz is not in valid range.")
            asserts.assert_greater_equal(zoom_max_dut.zoomMax, 0x0065, "ZoomMax is not in valid range.")
        else:
            logging.info("MZOOM Feature not supported. Test step skipped")
            self.mark_current_step_skipped()

        self.step(7)
        if has_feature_mtilt:
            asserts.assert_in(attributes.TiltMin.attribute_id, attribute_list,
                              "TiltMin attribute is a mandatory attribute if MTILT.")
            tilt_min_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute=attributes.TiltMin)

            asserts.assert_in(attributes.TiltMax.attribute_id, attribute_list,
                              "TiltMax attribute is a mandatory attribute if MTILT.")
            tilt_max_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute=attributes.TiltMax)

        else:
            logging.info("MTILT feature not supported. Test steps skipped")
            self.mark_current_step_skipped()

        self.step(9)
        if has_feature_mpan:
            asserts.assert_in(attributes.PanMin.attribute_id, attribute_list,
                              "PanMin attribute is a mandatory attribute if MPAN.")
            pan_min_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute=attributes.PanMin)

            asserts.assert_in(attributes.PanMax.attribute_id, attribute_list,
                              "PanMax attribute is a mandatory attribute if MPAN.")
            pan_max_dut = await self.read_avsum_attribute_expect_success(endpoint=endpoint, attribute=attributes.PanMax)

        else:
            logging.info("MPAN feature not supported. Test steps skipped")
            self.mark_current_step_skipped()



if __name__ == "__main__":
    default_matter_test_main()
