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
#

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_CHIME(MatterBaseTest):
    """Tests for chef chime device implementation."""

    _CHIME_ENDPOINT = 1

    # Expected chime sounds from chef-chime-delegate.cpp
    _EXPECTED_CHIME_SOUNDS = [
        Clusters.Objects.Chime.Structs.ChimeSoundStruct(chimeID=1, name="Classic Ding Dong"),
        Clusters.Objects.Chime.Structs.ChimeSoundStruct(chimeID=2, name="Merry Melodies"),
        Clusters.Objects.Chime.Structs.ChimeSoundStruct(chimeID=3, name="Digital Alert"),
    ]

    async def _read_chime_attribute(self, attribute):
        return await self.read_single_attribute_check_success(
            endpoint=self._CHIME_ENDPOINT,
            cluster=Clusters.Objects.Chime,
            attribute=attribute
        )

    async def _write_chime_attribute(self, attribute, value):
        return await self.write_single_attribute(
            attribute_value=attribute(value),
            endpoint_id=self._CHIME_ENDPOINT
        )

    async def _send_play_chime_sound_command(self, chimeID=None):
        return await self.send_single_cmd(
            endpoint=self._CHIME_ENDPOINT,
            cmd=Clusters.Objects.Chime.Commands.PlayChimeSound(chimeID=chimeID)
        )

    def desc_TC_CHIME(self) -> str:
        return "[TC_CHIME] chef chime functionality test."

    def steps_TC_CHIME(self):
        return [
            TestStep(1, "Commissioning already done.", is_commissioning=True),
            TestStep(2, "Read InstalledChimeSounds attribute."),
            TestStep(3, "Verify default SelectedChime is valid and test read/write."),
            TestStep(4, "Read and write Enabled attribute."),
            TestStep(5, "Test PlayChimeSound command when Enabled is True and verify event."),
            TestStep(6, "Test PlayChimeSound command when Enabled is False."),
            TestStep(7, "Test PlayChimeSound command already playing (back-to-back).")
        ]

    @async_test_body
    async def test_TC_CHIME(self):
        """Run all steps."""

        self.step(1)
        # Commissioning already done.

        self.step(2)
        installed_sounds = await self._read_chime_attribute(Clusters.Objects.Chime.Attributes.InstalledChimeSounds)
        asserts.assert_equal(len(installed_sounds), len(self._EXPECTED_CHIME_SOUNDS), "Unexpected number of chime sounds.")
        for i in range(len(self._EXPECTED_CHIME_SOUNDS)):
            asserts.assert_equal(installed_sounds[i].chimeID, self._EXPECTED_CHIME_SOUNDS[i].chimeID)
            asserts.assert_equal(installed_sounds[i].name, self._EXPECTED_CHIME_SOUNDS[i].name)

        self.step(3)
        # Verify initial default SelectedChime is valid
        initial_selected_chime = await self._read_chime_attribute(Clusters.Objects.Chime.Attributes.SelectedChime)
        valid_chime_ids = [sound.chimeID for sound in self._EXPECTED_CHIME_SOUNDS]
        asserts.assert_in(initial_selected_chime, valid_chime_ids, "Initial SelectedChime ID is not valid.")

        # Test writing SelectedChime
        await self._write_chime_attribute(Clusters.Objects.Chime.Attributes.SelectedChime, 2)
        selected_chime = await self._read_chime_attribute(Clusters.Objects.Chime.Attributes.SelectedChime)
        asserts.assert_equal(selected_chime, 2, "SelectedChime was not updated correctly.")

        # Revert to default
        await self._write_chime_attribute(Clusters.Objects.Chime.Attributes.SelectedChime, initial_selected_chime)

        self.step(4)
        # Test writing Enabled
        await self._write_chime_attribute(Clusters.Objects.Chime.Attributes.Enabled, False)
        enabled = await self._read_chime_attribute(Clusters.Objects.Chime.Attributes.Enabled)
        asserts.assert_false(enabled, "Enabled attribute was not updated to False.")

        await self._write_chime_attribute(Clusters.Objects.Chime.Attributes.Enabled, True)
        enabled = await self._read_chime_attribute(Clusters.Objects.Chime.Attributes.Enabled)
        asserts.assert_true(enabled, "Enabled attribute was not updated to True.")

        self.step(5)
        # Play chime sound when enabled and verify ChimeStartedPlaying event
        events_callback = EventSubscriptionHandler(expected_cluster=Clusters.Objects.Chime)
        await events_callback.start(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._CHIME_ENDPOINT,
        )

        chime_id_to_play = 1
        await self._send_play_chime_sound_command(chimeID=chime_id_to_play)

        event_data = events_callback.wait_for_event_report(Clusters.Objects.Chime.Events.ChimeStartedPlaying)
        asserts.assert_equal(event_data.chimeID, chime_id_to_play, "Unexpected chimeID in ChimeStartedPlaying event.")

        events_callback.reset()
        events_callback.cancel()

        self.step(6)
        # Disable chime and play sound
        await self._write_chime_attribute(Clusters.Objects.Chime.Attributes.Enabled, False)
        await self._send_play_chime_sound_command(chimeID=1)

        # Re-enable
        await self._write_chime_attribute(Clusters.Objects.Chime.Attributes.Enabled, True)

        self.step(7)
        # Test already playing logic.
        # Play one sound
        await self._send_play_chime_sound_command(chimeID=1)
        # Immediately play another one. Delegate should ignore it and return success.
        await self._send_play_chime_sound_command(chimeID=2)


if __name__ == "__main__":
    default_matter_test_main()
