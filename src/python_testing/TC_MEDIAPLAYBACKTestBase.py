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

"""Shared helpers for the Media Playback test cases that switch between content items.

TC-MEDIAPLAYBACK-6.11 and 6.12 both need the DUT to play two different pieces of
content so that a per-content attribute can be compared across the change. The test
plan describes doing this with the Content Launcher Presets feature but explicitly
allows any other method, so ``switch_content`` falls back to an operator prompt when
Presets is not implemented.
"""

import logging

from mobly import asserts

import matter.clusters as Clusters

log = logging.getLogger(__name__)

_CONTENT_LAUNCHER = Clusters.ContentLauncher


class MEDIAPLAYBACKTestBase:

    async def read_content_presets(self, endpoint):
        """Return the Content Launcher Presets list, or None when the feature is absent."""
        server_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.ServerList)
        if _CONTENT_LAUNCHER.id not in server_list:
            log.info("Content Launcher cluster is not present on endpoint %d", endpoint)
            return None
        attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=_CONTENT_LAUNCHER, attribute=_CONTENT_LAUNCHER.Attributes.AttributeList)
        if _CONTENT_LAUNCHER.Attributes.Presets.attribute_id not in attribute_list:
            log.info("Content Launcher Presets attribute is not implemented on endpoint %d", endpoint)
            return None
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=_CONTENT_LAUNCHER, attribute=_CONTENT_LAUNCHER.Attributes.Presets)

    def verify_presets_unique(self, presets):
        """Assert that every PresetID and every Name in the Presets list is unique."""
        asserts.assert_true(len(presets) > 0, "Presets attribute must not be empty when implemented")
        preset_ids = [preset.presetID for preset in presets]
        preset_names = [preset.presetName for preset in presets]
        asserts.assert_equal(len(set(preset_ids)), len(preset_ids), "All PresetIDs in Presets must be unique")
        asserts.assert_equal(len(set(preset_names)), len(preset_names), "All Names in Presets must be unique")

    def confirm_playback_started(self, ordinal: str):
        """Ask the operator to confirm that the DUT is actually playing the content just started.

        Playback is otherwise only observed through attribute reads that follow immediately,
        which an operator cannot check. A 'n' answer fails the test rather than being skipped.
        Not asked in the CI simulator, which has no operator.
        """
        if self.is_pics_sdk_ci_only:
            return
        response = self.wait_for_user_input(
            prompt_msg=f"Confirm the DUT is playing the {ordinal} content now (audio or on-screen).\n"
            "Enter 'y' if playback started, or 'n' if it did not.\n",
            prompt_msg_placeholder="y", default_value="y")
        asserts.assert_is_not_none(response, f"No response to the {ordinal} playback prompt, expected 'y' or 'n'")
        response = response.lower()
        asserts.assert_in(response, ["y", "n"], f"Invalid response to the {ordinal} playback prompt, expected 'y' or 'n'")
        asserts.assert_equal(response, "y", f"Operator reports the {ordinal} content did not start playing")

    async def switch_content(self, endpoint, presets, index: int):
        """Start playing the content at ``index`` in the Presets list.

        Falls back to prompting the operator when Presets is unavailable or too short,
        which is the substitution the test plan permits.
        """
        ordinal = "first" if index == 0 else "second"
        if presets is not None and len(presets) > index:
            preset = presets[index]
            log.info("Playing preset %s (PresetID %d)", preset.presetName, preset.presetID)
            await self.send_single_cmd(
                cmd=_CONTENT_LAUNCHER.Commands.PlayPreset(presetID=preset.presetID), endpoint=endpoint)
            return preset

        self.wait_for_user_input(
            prompt_msg=f"Start playing the {ordinal} piece of content on the DUT using any available method, "
            "then press Enter.\n")
        return None
