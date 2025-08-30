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

from mobly import asserts
from TC_CHIMETestBase import CHIMETestBase

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches


class TC_CHIME_2_4(MatterBaseTest, CHIMETestBase):

    def desc_TC_CHIME_2_4(self) -> str:
        return "[TC-CHIME-2.4] Verify functionality of the PlayChimeSound command"

    def steps_TC_CHIME_2_4(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Write the value of False to the Enabled attribute."),
            TestStep(3, "Invoke the PlayChimeSound command. Verify a success response, and no chime is played."),
            TestStep(4, "Write the value of True to the Enabled attribute."),
            TestStep(5, "Invoke the PlayChimeSound command. Verify a success response, and a chime is played."),
            TestStep(6, "Ensure that the SelectedChime is the longest chime available on the DUT"),
            TestStep(7, "Invoke PlayChimeSound three (3) times in rapid succession. Ensure success responses. Ensure no more than two were audible"),
            TestStep(8, "If there is more than one chime sound supported, proceed to step 9, otherwise end the test case"),
            TestStep(9, "Invoke PlayChimeSound on the DUT. Verify success"),
            TestStep(10, "Write a new supported chime sound to SelectedChime"),
            TestStep(11, "Obtain manual verification that the chime sound from step 9 is complete"),
            TestStep(12, "Invoke PlayChimeSound on the DUT. Verify that a different sound from the one played in step 9 is heard"),
        ]
        return steps

    def pics_TC_CHIME_2_4(self) -> list[str]:
        pics = [
            "CHIME.S",
        ]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.Chime))
    async def test_TC_CHIME_2_4(self):
        cluster = Clusters.Objects.Chime
        attributes = cluster.Attributes
        endpoint = self.get_endpoint(default=1)
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        self.step(1)  # Already done, immediately go to step 2

        self.step(2)
        await self.write_chime_attribute_expect_success(endpoint, attributes.Enabled, False)

        self.step(3)
        await self.send_play_chime_sound_command(endpoint)
        if not self.is_ci:
            user_response = self.wait_for_user_input(prompt_msg="A chime sound should not have been played, is this correct? Enter 'y' or 'n'",
                                                     prompt_msg_placeholder="y",
                                                     default_value="y")
            asserts.assert_equal(user_response.lower(), "y")

        self.step(4)
        await self.write_chime_attribute_expect_success(endpoint, attributes.Enabled, True)

        self.step(5)
        await self.send_play_chime_sound_command(endpoint)
        if not self.is_ci:
            user_response = self.wait_for_user_input(prompt_msg="A chime sound should have been played, is this correct? Enter 'y' or 'n'",
                                                     prompt_msg_placeholder="y",
                                                     default_value="y")
            asserts.assert_equal(user_response.lower(), "y")

        self.step(6)
        # Use the current selected chime when in CI
        longestChimeDurationChime = await self.read_chime_attribute_expect_success(endpoint, attributes.SelectedChime)

        if not self.is_ci:
            user_response = self.wait_for_user_input(prompt_msg="Plesse enter the ChimeID of the longest duration chime",
                                                     prompt_msg_placeholder=str(longestChimeDurationChime),
                                                     default_value=str(longestChimeDurationChime))
            chosenChimeID = int(user_response)
            # Make sure the selected ID is valid
            myChimeSounds = await self.read_chime_attribute_expect_success(endpoint, attributes.InstalledChimeSounds)
            found_id = False
            for chime in myChimeSounds:
                if chime.chimeID == chosenChimeID:
                    found_id = True
                    break

            if not found_id:
                asserts.assert_fail("Unknown ChimeID selected")

            longestChimeDurationChime = chosenChimeID

        await self.write_chime_attribute_expect_success(endpoint, attributes.SelectedChime, longestChimeDurationChime)

        self.step(7)
        if not self.is_ci:
            self.wait_for_user_input(prompt_msg="About to play multiple chimes on the DUT. Hit ENTER once ready.")

        await self.send_play_chime_sound_command(endpoint)
        await self.send_play_chime_sound_command(endpoint)
        await self.send_play_chime_sound_command(endpoint)

        if not self.is_ci:
            user_response = self.wait_for_user_input(prompt_msg="No more than two chime sounds should have been played, is this correct? Enter 'y' or 'n'",
                                                     prompt_msg_placeholder="y",
                                                     default_value="y")
            asserts.assert_equal(user_response.lower(), "y")

        self.step(8)
        myChimeSounds = await self.read_chime_attribute_expect_success(endpoint, attributes.InstalledChimeSounds)
        if len(myChimeSounds) > 1:

            if not self.is_ci:
                self.wait_for_user_input(prompt_msg="About to play a single chime on the DUT. Hit ENTER once ready.")

            self.step(9)
            await self.send_play_chime_sound_command(endpoint)

            self.step(10)
            newSelectedChime = longestChimeDurationChime
            for chime in myChimeSounds:
                if chime.chimeID != longestChimeDurationChime:
                    newSelectedChime = chime.chimeID
                    break

            await self.write_chime_attribute_expect_success(endpoint, attributes.SelectedChime, newSelectedChime)

            self.step(11)
            if not self.is_ci:
                self.wait_for_user_input(prompt_msg="Hit ENTER once the chime has completed playing.")

            self.step(12)
            await self.send_play_chime_sound_command(endpoint)
            if not self.is_ci:
                user_response = self.wait_for_user_input(prompt_msg="A different chime sound should have just been played, is this correct? Enter 'y' or 'n'",
                                                         prompt_msg_placeholder="y",
                                                         default_value="y")
                asserts.assert_equal(user_response.lower(), "y")

        else:
            self.skip_step(9)
            self.skip_step(10)
            self.skip_step(11)
            self.skip_step(12)


if __name__ == "__main__":
    default_matter_test_main()
