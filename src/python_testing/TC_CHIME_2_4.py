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
#   run2:
#     app: ${ALL_DEVICES_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --device chime:1 --device speaker:2,parent=1
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
from TC_CHIMETestBase import CHIMETestBase

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_CHIME_2_4(MatterBaseTest, CHIMETestBase):

    def desc_TC_CHIME_2_4(self) -> str:
        return "[TC-CHIME-2.4] Verify functionality of the PlayChimeSound command"

    def steps_TC_CHIME_2_4(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "On the child Speaker endpoint, write OnOff attribute of OnOff cluster to True (Unmuted) if Speaker is present"),
            TestStep(3, "On the child Speaker endpoint, write CurrentLevel attribute to a high level (e.g. 200) if Speaker is present and Level Control is supported"),
            TestStep(4, "Write the value of False to the Enabled attribute of the Chime"),
            TestStep(5, "Invoke PlayChimeSound command. Verify success response and that no chime sound is heard"),
            TestStep(6, "Write the value of True to the Enabled attribute of the Chime"),
            TestStep(7, "Invoke PlayChimeSound command. Verify success response and that a chime sound is heard"),
            TestStep(8, "Ensure that the SelectedChime is the longest chime available on the DUT"),
            TestStep(9, "Invoke PlayChimeSound three (3) times in rapid succession. Ensure success responses. Ensure no more than two were audible"),
            TestStep(10, "If there is more than one chime sound supported, proceed to step 11, otherwise end the test case"),
            TestStep(11, "Invoke PlayChimeSound on the DUT. Verify success"),
            TestStep(12, "Write a new supported chime sound to SelectedChime"),
            TestStep(13, "Obtain manual verification that the chime sound from step 11 is complete"),
            TestStep(14, "Invoke PlayChimeSound on the DUT. Verify that a different sound from the one played in step 11 is heard"),
            TestStep(15, "Check if child Speaker endpoint is present. If not, end test case"),
            TestStep(16, "On the child Speaker endpoint, write OnOff attribute of OnOff cluster to False (Muted)"),
            TestStep(17, "Invoke PlayChimeSound on the parent Chime endpoint. Verify success response and that no chime sound is heard"),
            TestStep(18, "On the child Speaker endpoint, write OnOff attribute of OnOff cluster to True (Unmuted)"),
            TestStep(19, "Invoke PlayChimeSound on the parent Chime endpoint. Verify success response and that a chime sound is heard"),
            TestStep(20, "Check if Level Control cluster is supported on the child Speaker endpoint. If not, end test case"),
            TestStep(21, "On the child Speaker endpoint, write CurrentLevel attribute to a low level (10)"),
            TestStep(22, "Invoke PlayChimeSound on the parent Chime endpoint. Verify success response and that sound is heard at a low volume"),
            TestStep(23, "On the child Speaker endpoint, write CurrentLevel attribute to a high level (200)"),
            TestStep(24, "Invoke PlayChimeSound on the parent Chime endpoint. Verify success response and that sound is heard at a high volume"),
        ]

    def pics_TC_CHIME_2_4(self) -> list[str]:
        return [
            "CHIME.S",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.Chime))
    async def test_TC_CHIME_2_4(self):
        cluster = Clusters.Objects.Chime
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        self.step(1)  # Already done, immediately go to step 2

        # Step 2: Unmute child Speaker if present
        self.step(2)
        parts_list = await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.Objects.Descriptor,
            attribute=Clusters.Objects.Descriptor.Attributes.PartsList
        )

        speaker_endpoint = None
        for child_endpoint in parts_list:
            device_type_list = await self.read_single_attribute_check_success(
                endpoint=child_endpoint,
                cluster=Clusters.Objects.Descriptor,
                attribute=Clusters.Objects.Descriptor.Attributes.DeviceTypeList
            )
            for dt in device_type_list:
                if dt.deviceType == 0x0022:  # Speaker device type ID
                    speaker_endpoint = child_endpoint
                    break
            if speaker_endpoint is not None:
                break

        if speaker_endpoint is not None:
            await self.send_single_cmd(
                cmd=Clusters.OnOff.Commands.On(),
                endpoint=speaker_endpoint
            )
        else:
            log.info("No child Speaker endpoint found, skipping unmute initialization step")

        # Step 3: Set Level Control to high if supported on child Speaker
        self.step(3)
        if speaker_endpoint is not None:
            server_list = await self.read_single_attribute_check_success(
                endpoint=speaker_endpoint,
                cluster=Clusters.Objects.Descriptor,
                attribute=Clusters.Objects.Descriptor.Attributes.ServerList
            )
            has_level_control = 0x0008 in server_list
            if has_level_control:
                await self.send_single_cmd(
                    cmd=Clusters.LevelControl.Commands.MoveToLevel(level=200, transitionTime=0, optionsMask=0, optionsOverride=0),
                    endpoint=speaker_endpoint
                )
            else:
                log.info("Level Control not supported on Speaker endpoint, skipping volume initialization step")
        else:
            log.info("No child Speaker endpoint found, skipping volume initialization step")

        # Step 4: Write Enabled = False
        self.step(4)
        await self.write_chime_attribute_expect_success(endpoint, attributes.Enabled, False)

        # Step 5: PlayChimeSound (no sound heard)
        self.step(5)
        await self.send_play_chime_sound_command(endpoint)
        if not self.is_ci:
            user_response = self.wait_for_user_input(prompt_msg="A chime sound should not have been played, is this correct? Enter 'y' or 'n'",
                                                     prompt_msg_placeholder="y",
                                                     default_value="y")
            if user_response is not None:
                log.info("TC-CHIME-2.4: response '%s' received on confirmation of no chime sound", user_response)
                asserts.assert_equal(user_response.lower(), "y")
            else:
                log.info("TC-CHIME-2.4: No response received for no chime sound played user prompt")

        # Step 6: Write Enabled = True
        self.step(6)
        await self.write_chime_attribute_expect_success(endpoint, attributes.Enabled, True)

        # Step 7: PlayChimeSound (sound heard)
        self.step(7)
        await self.send_play_chime_sound_command(endpoint)
        if not self.is_ci:
            user_response = self.wait_for_user_input(prompt_msg="A chime sound should have been played, is this correct? Enter 'y' or 'n'",
                                                     prompt_msg_placeholder="y",
                                                     default_value="y")
            if user_response is not None:
                log.info("TC-CHIME-2.4: response '%s' received on confirmation of chime sound", user_response)
                asserts.assert_equal(user_response.lower(), "y")
            else:
                log.info("TC-CHIME-2.4: No response received for chime sound played user prompt")

        # Step 8: Ensure SelectedChime is the longest chime
        self.step(8)
        longestChimeDurationChime = await self.read_chime_attribute_expect_success(endpoint, attributes.SelectedChime)

        if not self.is_ci:
            user_response = self.wait_for_user_input(prompt_msg="Please enter the ChimeID of the longest duration chime",
                                                     prompt_msg_placeholder=str(longestChimeDurationChime),
                                                     default_value=str(longestChimeDurationChime))

            if user_response is not None:
                chosenChimeID = int(user_response)
                # Make sure the selected ID is valid
                myChimeSounds = await self.read_chime_attribute_expect_success(endpoint, attributes.InstalledChimeSounds)
                found_id = False
                for chime in myChimeSounds:
                    if chime.chimeID == chosenChimeID:
                        found_id = True
                        break

                if not found_id:
                    asserts.assert_fail(f"Unknown ChimeID selected: {chosenChimeID}")
                else:
                    log.info("TC-CHIME-2.4: selected chime id for longest chime: %s", chosenChimeID)

                longestChimeDurationChime = chosenChimeID
            else:
                log.info("TC-CHIME-2.4: No response received for longest ChimeID user prompt")

        await self.write_chime_attribute_expect_success(endpoint, attributes.SelectedChime, longestChimeDurationChime)

        # Step 9: PlayChimeSound 3 times in rapid succession
        self.step(9)
        if not self.is_ci:
            self.wait_for_user_input(prompt_msg="About to play multiple chimes on the DUT. Hit ENTER once ready.")

        await self.send_play_chime_sound_command(endpoint)
        await self.send_play_chime_sound_command(endpoint)
        await self.send_play_chime_sound_command(endpoint)

        if not self.is_ci:
            user_response = self.wait_for_user_input(prompt_msg="No more than two chime sounds should have been played, is this correct? Enter 'y' or 'n'",
                                                     prompt_msg_placeholder="y",
                                                     default_value="y")
            if user_response is not None:
                log.info("TC-CHIME-2.4: response '%s' received on confirmation of no more than two chime sounds", user_response)
                asserts.assert_equal(user_response.lower(), "y")
            else:
                log.info("TC-CHIME-2.4: No response received for no more than two chime sounds played user prompt")

        # Step 10: Check if >1 chime sounds supported
        self.step(10)
        myChimeSounds = await self.read_chime_attribute_expect_success(endpoint, attributes.InstalledChimeSounds)
        if len(myChimeSounds) > 1:

            if not self.is_ci:
                self.wait_for_user_input(prompt_msg="About to play a single chime on the DUT. Hit ENTER once ready.")

            # Step 11: PlayChimeSound
            self.step(11)
            await self.send_play_chime_sound_command(endpoint)

            # Step 12: Write new supported chime sound
            self.step(12)
            newSelectedChime = longestChimeDurationChime
            for chime in myChimeSounds:
                if chime.chimeID != longestChimeDurationChime:
                    newSelectedChime = chime.chimeID
                    break

            await self.write_chime_attribute_expect_success(endpoint, attributes.SelectedChime, newSelectedChime)

            # Step 13: Obtain manual verification that the chime sound is complete
            self.step(13)
            if not self.is_ci:
                self.wait_for_user_input(prompt_msg="Hit ENTER once the chime has completed playing.")

            # Step 14: PlayChimeSound and verify different sound
            self.step(14)
            await self.send_play_chime_sound_command(endpoint)
            if not self.is_ci:
                user_response = self.wait_for_user_input(prompt_msg="A different chime sound should have just been played, is this correct? Enter 'y' or 'n'",
                                                         prompt_msg_placeholder="y",
                                                         default_value="y")
                if user_response is not None:
                    log.info("TC-CHIME-2.4: response '%s' received on confirmation of different chime sound", user_response)
                    asserts.assert_equal(user_response.lower(), "y")
                else:
                    log.info("TC-CHIME-2.4: No response received for different chime sound played user prompt")

        else:
            self.skip_step(11)
            self.skip_step(12)
            self.skip_step(13)
            self.skip_step(14)

        # Step 15: Find Speaker child endpoint
        self.step(15)
        if speaker_endpoint is None:
            log.info("No child Speaker endpoint found. Ending test case.")
            self.skip_step(16)
            self.skip_step(17)
            self.skip_step(18)
            self.skip_step(19)
            self.skip_step(20)
            self.skip_step(21)
            self.skip_step(22)
            self.skip_step(23)
            self.skip_step(24)
            return

        # Step 16: On the child Speaker endpoint, invoke Off command (Muted)
        self.step(16)
        await self.send_single_cmd(
            cmd=Clusters.OnOff.Commands.Off(),
            endpoint=speaker_endpoint
        )

        # Step 17: PlayChimeSound and verify no chime sound is heard
        self.step(17)
        await self.send_play_chime_sound_command(endpoint)
        if not self.is_ci:
            user_response = self.wait_for_user_input(
                prompt_msg="A chime sound should NOT have been played (Speaker is muted), is this correct? Enter 'y' or 'n'",
                prompt_msg_placeholder="y",
                default_value="y"
            )
            if user_response is not None:
                log.info("TC-CHIME-2.4: response '%s' received on confirmation of no chime sound (muted)", user_response)
                asserts.assert_equal(user_response.lower(), "y")
            else:
                log.info("TC-CHIME-2.4: No response received for muted chime sound user prompt")

        # Step 18: On the child Speaker endpoint, invoke On command (Unmuted)
        self.step(18)
        await self.send_single_cmd(
            cmd=Clusters.OnOff.Commands.On(),
            endpoint=speaker_endpoint
        )

        # Step 19: PlayChimeSound and verify chime sound is heard
        self.step(19)
        await self.send_play_chime_sound_command(endpoint)
        if not self.is_ci:
            user_response = self.wait_for_user_input(
                prompt_msg="A chime sound should have been played (Speaker is unmuted), is this correct? Enter 'y' or 'n'",
                prompt_msg_placeholder="y",
                default_value="y"
            )
            if user_response is not None:
                log.info("TC-CHIME-2.4: response '%s' received on confirmation of chime sound (unmuted)", user_response)
                asserts.assert_equal(user_response.lower(), "y")
            else:
                log.info("TC-CHIME-2.4: No response received for unmuted chime sound user prompt")

        # Step 20: Check if Level Control is supported
        self.step(20)
        server_list = await self.read_single_attribute_check_success(
            endpoint=speaker_endpoint,
            cluster=Clusters.Objects.Descriptor,
            attribute=Clusters.Objects.Descriptor.Attributes.ServerList
        )
        has_level_control = 0x0008 in server_list
        if not has_level_control:
            log.info("Level Control not supported on Speaker endpoint. Ending test case.")
            self.skip_step(21)
            self.skip_step(22)
            self.skip_step(23)
            self.skip_step(24)
            return

        # Step 21: On the child Speaker endpoint, invoke MoveToLevel command to low level (10)
        self.step(21)
        await self.send_single_cmd(
            cmd=Clusters.LevelControl.Commands.MoveToLevel(level=10, transitionTime=0, optionsMask=0, optionsOverride=0),
            endpoint=speaker_endpoint
        )

        # Step 22: PlayChimeSound and verify low volume sound
        self.step(22)
        await self.send_play_chime_sound_command(endpoint)
        if not self.is_ci:
            user_response = self.wait_for_user_input(
                prompt_msg="A chime sound should have been played at a LOW volume, is this correct? Enter 'y' or 'n'",
                prompt_msg_placeholder="y",
                default_value="y"
            )
            if user_response is not None:
                log.info("TC-CHIME-2.4: response '%s' received on confirmation of low volume chime", user_response)
                asserts.assert_equal(user_response.lower(), "y")
            else:
                log.info("TC-CHIME-2.4: No response received for low volume chime user prompt")

        # Step 23: On the child Speaker endpoint, invoke MoveToLevel command to high level (200)
        self.step(23)
        await self.send_single_cmd(
            cmd=Clusters.LevelControl.Commands.MoveToLevel(level=200, transitionTime=0, optionsMask=0, optionsOverride=0),
            endpoint=speaker_endpoint
        )

        # Step 24: PlayChimeSound and verify high volume sound
        self.step(24)
        await self.send_play_chime_sound_command(endpoint)
        if not self.is_ci:
            user_response = self.wait_for_user_input(
                prompt_msg="A chime sound should have been played at a HIGH volume, is this correct? Enter 'y' or 'n'",
                prompt_msg_placeholder="y",
                default_value="y"
            )
            if user_response is not None:
                log.info("TC-CHIME-2.4: response '%s' received on confirmation of high volume chime", user_response)
                asserts.assert_equal(user_response.lower(), "y")
            else:
                log.info("TC-CHIME-2.4: No response received for high volume chime user prompt")


if __name__ == "__main__":
    default_matter_test_main()
