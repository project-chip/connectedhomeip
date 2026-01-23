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
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main


class TC_AVSUM_2_9(MatterBaseTest, AVSUMTestBase):
    has_feature_mpan = False
    has_feature_mtilt = False
    has_feature_mzoom = False

    def desc_TC_AVSUM_2_9(self) -> str:
        return "[TC-AVSUM-2.9] MptzSetPosition Command Validation"

    def steps_TC_AVSUM_2_9(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Do stuff"),
            TestStep(3, "More stuff"),
            TestStep(4, "Reboot"),
            TestStep(5, "Post reboot"),
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

        attribute_list = await self.read_avsum_attribute_expect_success(endpoint, attributes.AttributeList)

        if not (self.has_feature_mpan | self.has_feature_mtilt | self.has_feature_mzoom):
            asserts.fail("One of MPAN, MTILT, or MZOOM is mandatory for command support")

        asserts.assert_in(attributes.MPTZPosition.attribute_id, attribute_list,
                          "MPTZPosition attribute is mandatory if the command is supported.")
        mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)
        currentPan = mptzposition_dut.pan
        currentTilt = mptzposition_dut.tilt
        currentZoom = mptzposition_dut.zoom

        # Establish subscription to MovementState
        sub_handler = AttributeSubscriptionHandler(cluster, attributes.MovementState)
        await sub_handler.start(self.default_controller, self.dut_node_id, endpoint=endpoint, min_interval_sec=0, max_interval_sec=30, keepSubscriptions=False)

        # Create attribute matchers
        movement_state_match = AttributeMatcher.from_callable(
            "MovementState is IDLE",
            lambda report: report.value == cluster.Enums.PhysicalMovementEnum.kIdle)

        newPan = newTilt = newZoom = None

        if self.has_feature_mpan:
            pan_min_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.PanMin)
            pan_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.PanMax)

            # Create new Value for Pan
            while True:
                newPan = random.randint(pan_min_dut, pan_max_dut)
                if newPan != currentPan:
                    break

        if self.has_feature_mtilt:
            tilt_min_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.TiltMin)
            tilt_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.TiltMax)

            # Create new Value for Tilt
            while True:
                newTilt = random.randint(tilt_min_dut, tilt_max_dut)
                if newTilt != currentTilt:
                    break

        if self.has_feature_mzoom:
            zoom_max_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.ZoomMax)

            # Create new Value for Zoom
            while True:
                newZoom = random.randint(2, zoom_max_dut)
                if newZoom != currentZoom:
                    break

        sub_handler.reset()
        # Invoke the command with the new values
        await self.send_mptz_set_position_command(endpoint, newPan, newTilt, newZoom)

        # Once the MovementState has reset to Idle, read the attribute back and make sure it was set
        sub_handler.await_all_expected_report_matches([movement_state_match], timeout_sec=self.default_timeout)
        new_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)

        if newPan is not None:
            asserts.assert_equal(new_mptzposition_dut.pan, newPan, "Received Pan does not match set Pan")

        if newTilt is not None:
            asserts.assert_equal(new_mptzposition_dut.tilt, newTilt, "Received Tilt does not match set Tilt")

        if newZoom is not None:
            asserts.assert_equal(new_mptzposition_dut.zoom, newZoom, "Received Zoom does not match set Zoom")

        self.step(3)
        max_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MaxPresets)
        mptz_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPresets)

        asserts.assert_less_equal(len(mptz_presets_dut), max_presets_dut,
                                  "MPTZPresets size is greater than the allowed max.")

        name = "newpreset"
        await self.send_save_preset_command(endpoint, name, presetID=max_presets_dut)
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

        self.step(4)
        # Reboot DUT
        # Check if restart flag file is available (indicates test runner supports app restart)
        await self.request_device_reboot()

        self.step(5)
        new_mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)

        if newPan is not None:
            asserts.assert_equal(new_mptzposition_dut.pan, newPan, "Received Pan does not match set Pan")

        if newTilt is not None:
            asserts.assert_equal(new_mptzposition_dut.tilt, newTilt, "Received Tilt does not match set Tilt")

        if newZoom is not None:
            asserts.assert_equal(new_mptzposition_dut.zoom, newZoom, "Received Zoom does not match set Zoom")
            
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


if __name__ == "__main__":
    default_matter_test_main()
