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

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from mobly import asserts
from TC_AVSUMTestBase import AVSUMTestBase


class TC_AVSUM_2_5(MatterBaseTest, AVSUMTestBase):

    def desc_TC_AVSUM_2_5(self) -> str:
        return "[TC-AVSUM-2.5] MPTZSavePreset command validation"

    def steps_TC_AVSUM_2_5(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read and save the current value of MPTZPosition"),
            TestStep(3, "Read the value of MaxPresets, fail if unsupported."),
            TestStep(4, "Read the value of MPTZPresets, fail if unsupported"),
            TestStep(5, "If the size of MPTZPresets is greater than MaxPresets, fail"),
            TestStep(6, "Via the MPTZSavePreset command, create a new saved preset with PresetID of MaxPresets, name of 'newpreset'"),
            TestStep(7, "Read the value of MPTZPresets. Ensure it has an entry for a PresetID of MaxPresets with a name 'newpreset' that matches the saved MPTZPosition"),
            TestStep(8, "Verify there is space in the preset list, if not, skip to step 12"),
            TestStep(9, "Save a new Preset via the MPTZSavePreset command, name 'newpreset-2', do not provide a PresetID"),
            TestStep(10, "Read the value of MPTZPresets. Ensure it has an entry with a name 'newpreset-2' that matches the saved MPTZPosition"),
            TestStep(11, "Repeatedly send MPTZSavePreset commands till the number of presets equals MaxPresets"),
            TestStep(12, "Save a new Preset via the MPTZSavePreset command, name 'newpreset-fail', do not provide a PresetID. Verify Resource Exhausted failure. End the test case."),
        ]
        return steps

    def pics_TC_AVSUM_2_5(self) -> list[str]:
        pics = [
            "AVSUM.S",
        ]
        return pics

    @run_if_endpoint_matches(has_feature(Clusters.CameraAvSettingsUserLevelManagement,
                                         Clusters.CameraAvSettingsUserLevelManagement.Bitmaps.Feature.kMechanicalPresets))
    async def test_TC_AVSUM_2_5(self):
        cluster = Clusters.Objects.CameraAvSettingsUserLevelManagement
        attributes = cluster.Attributes
        endpoint = self.get_endpoint(default=1)

        self.step(1)  # Already done, immediately go to step 2

        self.step(2)
        mptzposition_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPosition)

        attribute_list = await self.read_avsum_attribute_expect_success(endpoint, attributes.AttributeList)

        self.step(3)
        asserts.assert_in(attributes.MaxPresets.attribute_id, attribute_list,
                          "MaxPresets attribute is a mandatory attribute if MPRESETS.")
        max_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MaxPresets)

        self.step(4)
        asserts.assert_in(attributes.MPTZPresets.attribute_id, attribute_list,
                          "MPTZPresets attribute is a mandatory attribut if MPRESETS.")
        mptz_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPresets)

        self.step(5)
        asserts.assert_less_equal(len(mptz_presets_dut), max_presets_dut,
                                  "MPTZPresets size is greater than the allowed max.")

        self.step(6)
        name = "newpreset"
        await self.send_save_preset_command(endpoint, name, presetID=max_presets_dut)
        mptz_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPresets)

        match_found = False
        self.step(7)
        for mptzpreset in mptz_presets_dut:
            if mptzpreset.presetID == max_presets_dut:
                # verify that the values match
                asserts.assert_equal(name, mptzpreset.name, "Preset name doesn't match that saved")
                self.verify_preset_matches(mptzpreset, mptzposition_dut)
                match_found = True
                break

        if not match_found:
            asserts.assert_fail("No matching preset found for saved preset ID")

        # make sure there is space is the preset list
        self.step(8)
        if len(mptz_presets_dut) < max_presets_dut:
            name = "newpreset-2"
            self.step(9)
            await self.send_save_preset_command(endpoint, name)

            self.step(10)
            mptz_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPresets)

            match_found = False
            for mptzpreset in mptz_presets_dut:
                if mptzpreset.name == name:
                    # verify that the values match
                    asserts.assert_less_equal(mptzpreset.presetID, max_presets_dut, "Allocated PresetID greater than allowed max")
                    self.verify_preset_matches(mptzpreset, mptzposition_dut)
                    match_found = True
                    break

            if not match_found:
                asserts.assert_fail("No matching preset found for saved preset")

            # fill the preset list to its max allowed number of values
            self.step(11)
            count = len(mptz_presets_dut)
            while count < max_presets_dut:
                name = "newpreset-"+str(count)
                await self.send_save_preset_command(endpoint, name)
                count += 1
        else:
            self.skip_step(9)
            self.skip_step(10)
            self.skip_step(11)

        # presets should now be full
        name = "newpreset-fail"
        self.step(12)
        await self.send_save_preset_command(endpoint, name, expected_status=Status.ResourceExhausted)


if __name__ == "__main__":
    default_matter_test_main()
