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
import typing

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_MODESELECT(MatterBaseTest):
    """Tests for chef mode select device."""

    ENDPOINT = 1
    EXPECTED_MODES: typing.List[Clusters.Objects.ModeSelect.Structs.ModeOptionStruct] = [
        Clusters.Objects.ModeSelect.Structs.ModeOptionStruct(
            label="3.5s", mode=35, semanticTags=[Clusters.Objects.ModeSelect.Structs.SemanticTagStruct(mfgCode=0xFFF1, value=0)]),
        Clusters.Objects.ModeSelect.Structs.ModeOptionStruct(
            label="4s", mode=40, semanticTags=[Clusters.Objects.ModeSelect.Structs.SemanticTagStruct(mfgCode=0xFFF1, value=0)]),
        Clusters.Objects.ModeSelect.Structs.ModeOptionStruct(
            label="5s", mode=50, semanticTags=[Clusters.Objects.ModeSelect.Structs.SemanticTagStruct(mfgCode=0xFFF1, value=0)])
    ]

    def desc_TC_MODESELECT(self) -> str:
        return "[TC_MODESELECT] chef mode select functionality test."

    def steps_TC_MODESELECT(self):
        return [TestStep(1, "Commissioning already done.", is_commissioning=True),
                TestStep(2, "Read Description and check it is equal to 'Mode Select Sample'."),
                TestStep(3, "Read StandardNamespace and check it is equal to Null."),
                TestStep(4, "Read SupportedModes and check it matches default modes."),
                TestStep(5, "Read CurrentMode and check it is equal to 1."),
                TestStep(6, "Loop over modes 1, 2, 3 and run ChangeToMode command.")]

    async def _read_description(self):
        return await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT,
            cluster=Clusters.Objects.ModeSelect,
            attribute=Clusters.Objects.ModeSelect.Attributes.Description)

    async def _read_standard_namespace(self):
        return await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT,
            cluster=Clusters.Objects.ModeSelect,
            attribute=Clusters.Objects.ModeSelect.Attributes.StandardNamespace)

    async def _read_supported_modes(self):
        return await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT,
            cluster=Clusters.Objects.ModeSelect,
            attribute=Clusters.Objects.ModeSelect.Attributes.SupportedModes)

    async def _read_current_mode(self):
        return await self.read_single_attribute_check_success(
            endpoint=self.ENDPOINT,
            cluster=Clusters.Objects.ModeSelect,
            attribute=Clusters.Objects.ModeSelect.Attributes.CurrentMode)

    async def _send_change_to_mode_command(self, mode):
        return await self.send_single_cmd(
            cmd=Clusters.Objects.ModeSelect.Commands.ChangeToMode(newMode=mode),
            endpoint=self.ENDPOINT)

    @async_test_body
    async def test_TC_MODESELECT(self):
        """Test Mode Select cluster functionality."""

        self.step(1)
        # Commissioning already done.

        # Step 2: Read Description and check it is equal to "Mode Select Sample"
        self.step(2)
        description = await self._read_description()
        asserts.assert_equal(description, "Mode Select Sample", "Description should be 'Mode Select Sample'")

        # Step 3: Read StandardNamespace and check it is equal to Null
        self.step(3)
        standard_namespace = await self._read_standard_namespace()
        asserts.assert_equal(standard_namespace, NullValue, "StandardNamespace should be Null")

        # Step 4: Read SupportedModes and check it matches default modes
        self.step(4)
        supported_modes = await self._read_supported_modes()
        asserts.assert_equal(supported_modes, self.EXPECTED_MODES, "SupportedModes should match expected default modes")

        # Step 5: Read CurrentMode and check it is equal to the first mode.
        self.step(5)
        current_mode = await self._read_current_mode()
        asserts.assert_equal(current_mode, self.EXPECTED_MODES[0].mode,
                             f"Initial CurrentMode should be {self.EXPECTED_MODES[0].mode}")

        # Step 6: Loop over modes and run ChangeToMode command
        self.step(6)
        for mode_option in self.EXPECTED_MODES:
            mode = mode_option.mode
            await self._send_change_to_mode_command(mode)
            current_mode = await self._read_current_mode()
            asserts.assert_equal(current_mode, mode, f"CurrentMode should be {mode} after ChangeToMode({mode})")


if __name__ == "__main__":
    default_matter_test_main()
