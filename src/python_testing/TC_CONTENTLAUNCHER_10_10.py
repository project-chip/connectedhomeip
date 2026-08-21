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

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing import matter_asserts
from matter.testing.decorators import has_attribute, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

_PRESET_ID_MAX = 0xFF


class TC_CONTENTLAUNCHER_10_10(MatterBaseTest):

    def desc_TC_CONTENTLAUNCHER_10_10(self) -> str:
        return "[TC-CONTENTLAUNCHER-10.10] Content Presets"

    def pics_TC_CONTENTLAUNCHER_10_10(self) -> list[str]:
        return ["CONTENTLAUNCHER.S", "CONTENTLAUNCHER.S.A0003"]

    def steps_TC_CONTENTLAUNCHER_10_10(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads the Presets attribute from the DUT.",
                     "DUT replies with a list of Content Preset structs in which all PresetIDs are unique and "
                     "all Names are unique."),
            TestStep(2, "TH sends a PlayPreset command to the DUT with the first value in the list from step 1.",
                     "DUT replies with a success response and starts playing some kind of content."),
            TestStep(3, "TH sends a PlayPreset command to the DUT with the second value in the list from step 1.",
                     "DUT replies with a success response, stops playing the original content, and starts playing "
                     "some kind of new content."),
            TestStep(4, "TH sends a PlayPreset command to the DUT with a value that is not in the list from step 1.",
                     "DUT replies with a PresetNotFound (10) failure response and continues playing the content "
                     "from step 3."),
        ]

    @run_if_endpoint_matches(has_attribute(Clusters.ContentLauncher.Attributes.Presets))
    async def test_TC_CONTENTLAUNCHER_10_10(self):
        cluster = Clusters.ContentLauncher
        endpoint = self.get_endpoint()

        self.step(0)

        self.step(1)
        presets = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.Presets)
        matter_asserts.assert_list(presets, "Presets", min_length=1)
        matter_asserts.assert_list_element_type(presets, cluster.Structs.ContentPresetStruct, "Presets")
        preset_ids = [preset.presetID for preset in presets]
        preset_names = [preset.presetName for preset in presets]
        asserts.assert_equal(len(set(preset_ids)), len(preset_ids), "All PresetIDs in Presets must be unique")
        asserts.assert_equal(len(set(preset_names)), len(preset_names), "All Names in Presets must be unique")

        # The test plan notes that a conformant DUT should offer at least 2 presets so that
        # PlayPreset can be shown to work for more than one, even though the spec requires 1.
        if len(presets) < 2:
            log.info("DUT reports %d preset(s); steps 3 and 4 need at least 2", len(presets))

        self.step(2)
        await self.send_single_cmd(cmd=cluster.Commands.PlayPreset(presetID=preset_ids[0]), endpoint=endpoint)

        self.step(3)
        if len(presets) >= 2:
            await self.send_single_cmd(cmd=cluster.Commands.PlayPreset(presetID=preset_ids[1]), endpoint=endpoint)
        else:
            self.mark_current_step_skipped()

        self.step(4)
        unknown_preset_id = self._find_unused_preset_id(preset_ids)
        if unknown_preset_id is None:
            # Every uint8 value is in use, so no invalid PresetID can be constructed.
            log.info("All %d PresetID values are in use; cannot build an invalid PresetID", _PRESET_ID_MAX + 1)
            self.mark_current_step_skipped()
            return

        try:
            await self.send_single_cmd(
                cmd=cluster.Commands.PlayPreset(presetID=unknown_preset_id), endpoint=endpoint)
            asserts.fail(f"PlayPreset with unknown PresetID {unknown_preset_id} should not have succeeded")
        except InteractionModelError as e:
            # The spec requires the server to respond with the cluster-specific PresetNotFound
            # status and end processing without any other side-effects.
            asserts.assert_equal(e.status, Status.Failure,
                                 "PlayPreset with an unknown PresetID should return a cluster-specific failure")
            asserts.assert_equal(e.clusterStatus, cluster.Enums.StatusEnum.kPresetNotFound,
                                 f"Expected PresetNotFound (10), got cluster status {e.clusterStatus}")

    def _find_unused_preset_id(self, preset_ids) -> int | None:
        """Return a uint8 PresetID that the DUT did not report, or None if all are taken."""
        used = set(preset_ids)
        for candidate in range(_PRESET_ID_MAX, -1, -1):
            if candidate not in used:
                return candidate
        return None


if __name__ == "__main__":
    default_matter_test_main()
