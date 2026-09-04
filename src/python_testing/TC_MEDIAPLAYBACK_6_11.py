#
#    Copyright (c) 2026 Project CHIP Authors
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

import logging

from mobly import asserts
from TC_MEDIAPLAYBACKTestBase import MEDIAPLAYBACKTestBase

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing import matter_asserts
from matter.testing.decorators import has_attribute, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# Client => server command IDs defined for Media Playback. AvailableCommands reports the
# subset of these that the server can currently process, so any other value is invalid.
# PlaybackResponse (0x0A) is server => client and therefore never appears in the list.
_PLAYBACK_COMMAND_IDS = {
    Clusters.MediaPlayback.Commands.Play.command_id,
    Clusters.MediaPlayback.Commands.Pause.command_id,
    Clusters.MediaPlayback.Commands.Stop.command_id,
    Clusters.MediaPlayback.Commands.StartOver.command_id,
    Clusters.MediaPlayback.Commands.Previous.command_id,
    Clusters.MediaPlayback.Commands.Next.command_id,
    Clusters.MediaPlayback.Commands.Rewind.command_id,
    Clusters.MediaPlayback.Commands.FastForward.command_id,
    Clusters.MediaPlayback.Commands.SkipForward.command_id,
    Clusters.MediaPlayback.Commands.SkipBackward.command_id,
    Clusters.MediaPlayback.Commands.Seek.command_id,
    Clusters.MediaPlayback.Commands.ActivateAudioTrack.command_id,
    Clusters.MediaPlayback.Commands.ActivateTextTrack.command_id,
    Clusters.MediaPlayback.Commands.DeactivateTextTrack.command_id,
}


class TC_MEDIAPLAYBACK_6_11(MatterBaseTest, MEDIAPLAYBACKTestBase):

    def desc_TC_MEDIAPLAYBACK_6_11(self) -> str:
        return "[TC-MEDIAPLAYBACK-6.11] Available Commands Verification"

    def pics_TC_MEDIAPLAYBACK_6_11(self) -> list[str]:
        return ["MEDIAPLAYBACK.S", "MEDIAPLAYBACK.S.A000b"]

    def steps_TC_MEDIAPLAYBACK_6_11(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads the Presets attribute from the DUT.",
                     "DUT replies with a list of Content Preset structs in which all PresetIDs are unique and "
                     "all Names are unique."),
            TestStep(2, "TH sends a PlayPreset command to the DUT with the first value in the list from step 1.",
                     "DUT replies with a success response and starts playing some kind of content."),
            TestStep(3, "TH reads the AvailableCommands attribute from the DUT.",
                     "DUT replies with a list of uint32 values that are a strict subset of the possible values for "
                     "Media Playback Commands."),
            TestStep(4, "TH sends a PlayPreset command to the DUT with the second value in the list from step 1.",
                     "DUT replies with a success response and starts playing some other kind of content."),
            TestStep(5, "TH reads the AvailableCommands attribute from the DUT.",
                     "DUT replies with a list of uint32 values that are a strict subset of the possible values for "
                     "Media Playback Commands, and is different from the value received in step 3."),
        ]

    def _verify_available_commands(self, available_commands, accepted_commands, label: str):
        """Assert the reported list is a valid strict subset of the Media Playback commands."""
        asserts.assert_true(available_commands is not NullValue,
                            f"AvailableCommands must not be null {label}")
        matter_asserts.assert_list(available_commands, f"AvailableCommands {label}")
        for command_id in available_commands:
            matter_asserts.assert_valid_uint32(command_id, f"AvailableCommands entry {label}")
            asserts.assert_in(command_id, _PLAYBACK_COMMAND_IDS,
                              f"AvailableCommands entry 0x{command_id:02X} {label} is not a Media Playback command")
        asserts.assert_equal(len(set(available_commands)), len(available_commands),
                             f"AvailableCommands {label} must not contain duplicates")
        # A command can only be currently available if the server implements it at all.
        unsupported = set(available_commands) - set(accepted_commands)
        asserts.assert_equal(unsupported, set(),
                             f"AvailableCommands {label} lists commands missing from AcceptedCommandList: "
                             f"{sorted(unsupported)}")
        # "Strict subset" - the list cannot cover every possible Media Playback command.
        asserts.assert_true(set(available_commands) < _PLAYBACK_COMMAND_IDS,
                            f"AvailableCommands {label} must be a strict subset of the Media Playback commands")

    @run_if_endpoint_matches(has_attribute(Clusters.MediaPlayback.Attributes.AvailableCommands))
    async def test_TC_MEDIAPLAYBACK_6_11(self):
        cluster = Clusters.MediaPlayback
        endpoint = self.get_endpoint()

        self.step(0)
        accepted_commands = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AcceptedCommandList)

        self.step(1)
        presets = await self.read_content_presets(endpoint)
        if presets is not None:
            self.verify_presets_unique(presets)
        else:
            self.mark_current_step_skipped()

        self.step(2)
        await self.switch_content(endpoint, presets, 0)

        self.step(3)
        first_available = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AvailableCommands)
        self._verify_available_commands(first_available, accepted_commands, "for the first content")

        self.step(4)
        await self.switch_content(endpoint, presets, 1)

        self.step(5)
        second_available = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.AvailableCommands)
        self._verify_available_commands(second_available, accepted_commands, "for the second content")
        asserts.assert_not_equal(
            set(second_available), set(first_available),
            "AvailableCommands should differ between two content items with different available commands")


if __name__ == "__main__":
    default_matter_test_main()
