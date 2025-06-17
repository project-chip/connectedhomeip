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


class TC_AVSUM_2_6(MatterBaseTest, AVSUMTestBase):

    def desc_TC_AVSUM_2_6(self) -> str:
        return "[TC-AVSUM-2.6] MPTZRemovePreset command validation"

    def steps_TC_AVSUM_2_6(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the value of MaxPresets, fail if unsupported."),
            TestStep(3, "Read the value of MPTZPresets, fail if unsupported"),
            TestStep(4, "Send a MPTZRemovePreset for a value larger than MaxPresets, verify failure"),
            TestStep(5, "If MPTZPresets is not empty, send MPTZRemovePreset for the first item in the list. Verify it has been removed. Otherwise skip to step 8."),
            TestStep(6, "Read MPTZPresets, verify the entry has been removed."),
            TestStep(7, "Repeat step 5, sending a MPTZRemovePreset command for the already removed preset. Verify failurre. End the test case."),
            TestStep(8, "If MPTZPresets is empty, via the MPTZSavePreset command, create a new saved preset with PresetID of MaxPresets, name of 'newpreset'"),
            TestStep(9, "Read the value of MPTZPresets. Ensure it has an entry for a PresetID of MaxPresets with a name 'newpreset' that matches the saved MPTZPosition"),
            TestStep(10, "Via MPTZRemovePreset remove the saved preset."),
            TestStep(11, "Read MPTZPresets, verify this is empty."),
            TestStep(12, "Repeat step 10, sending a MPTZRemovePreset command for a PresetID of MaxPresets. Verify failure."),
        ]
        return steps

    def pics_TC_AVSUM_2_6(self) -> list[str]:
        pics = [
            "AVSUM.S", "AVSUM.S.F04"
        ]
        return pics

    @run_if_endpoint_matches(has_feature(Clusters.CameraAvSettingsUserLevelManagement,
                                         Clusters.CameraAvSettingsUserLevelManagement.Bitmaps.Feature.kMechanicalPresets))
    async def test_TC_AVSUM_2_6(self):
        cluster = Clusters.Objects.CameraAvSettingsUserLevelManagement
        attributes = cluster.Attributes
        endpoint = self.get_endpoint(default=1)

        self.step(1)  # Already done, immediately go to step 2
        attribute_list = await self.read_avsum_attribute_expect_success(endpoint, attributes.AttributeList)

        self.step(2)
        asserts.assert_in(attributes.MaxPresets.attribute_id, attribute_list,
                          "MaxPresets attribute is a mandatory attribute if MPRESETS.")
        max_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MaxPresets)

        self.step(3)
        asserts.assert_in(attributes.MPTZPresets.attribute_id, attribute_list,
                          "MPTZPresets attribute is a mandatory attribut if MPRESETS.")
        mptz_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPresets)

        self.step(4)
        await self.send_remove_preset_command(endpoint, max_presets_dut+1, expected_status=Status.ConstraintError)

        if mptz_presets_dut:
            # Save the presetID of the first preset in the list
            presetID = mptz_presets_dut[0].presetID

            self.step(5)
            # Remove that preset
            await self.send_remove_preset_command(endpoint, presetID)

            # Re-read the attribute
            self.step(6)
            mptz_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPresets)
            notFound = True
            for mptzpreset in mptz_presets_dut:
                if mptzpreset.presetID == presetID:
                    notFound = False
                    break
            asserts.assert_true(notFound, "Preset not removed despite invocation of MPTZRemovePreset")

            # Repeat removal of already removed value, verify Not Found
            self.step(7)
            await self.send_remove_preset_command(endpoint, presetID, expected_status=Status.NotFound)

            self.mark_all_remaining_steps_skipped(8)
            return

        self.skip_step(5)
        self.skip_step(6)
        self.skip_step(7)
        self.step(8)
        # No existing values, create one and then make sure we can delete it
        await self.send_save_preset_command(endpoint, name="newpreset", presetID=max_presets_dut)
        mptz_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPresets)

        self.step(9)
        # Verify the first entry is what we sent
        asserts.assert_equal(mptz_presets_dut[0].presetID, max_presets_dut, "Saved Preset ID does not natch the provided preset")

        self.step(10)
        # Remove and verify empty
        await self.send_remove_preset_command(endpoint, max_presets_dut)
        mptz_presets_dut = await self.read_avsum_attribute_expect_success(endpoint, attributes.MPTZPresets)

        self.step(11)
        asserts.assert_true(not mptz_presets_dut, "Added Preset not removed despite invocation of MPTZRemovePreset")

        # Repeat removal of already removed value, verify Not Found
        self.step(12)
        await self.send_remove_preset_command(endpoint, max_presets_dut, expected_status=Status.NotFound)


if __name__ == "__main__":
    default_matter_test_main()
