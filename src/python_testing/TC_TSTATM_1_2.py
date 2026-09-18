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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from modebase_cluster_check import ModeBaseClusterChecks

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

cluster_tstat_mode = Clusters.ThermostatMode


class TC_TSTATM_1_2(MatterBaseTest, ModeBaseClusterChecks):

    def __init__(self, *args):
        MatterBaseTest.__init__(self, *args)
        ModeBaseClusterChecks.__init__(self, modebase_derived_cluster=cluster_tstat_mode)

    def desc_TC_TSTATM_1_2(self) -> str:
        return "[TC-TSTATM-1.2] Cluster attributes with DUT as Server"

    def steps_TC_TSTATM_1_2(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "TH reads SupportedModes attribute from DUT"),
            TestStep("2b", "TH validates ModeTags field entries in SupportedModes"),
            TestStep("2c", "TH validates mode tag conditions for Thermostat Mode"),
            TestStep(3, "TH reads CurrentMode attribute from DUT"),
            TestStep(5, "TH reads StartUpMode attribute from DUT"),
            TestStep(6, "TH reads CoreModeTags attribute from DUT"),
        ]

    def pics_TC_TSTATM_1_2(self) -> list[str]:
        return [
            "TSTATM.S"
        ]

    @property
    def default_endpoint(self) -> int:
        return 1

    def check_thermostat_mode_tag_conditions(self, supported_modes: list[Clusters.ThermostatMode.Structs.ModeOptionStruct]) -> None:
        """Validates the Thermostat Mode specific conditions on SupportedModes.

        Conditions defined in mode_thermostat.adoc:
        1) Verify that each entry contains exactly one of Off, Cool, Heat, or Auto mode tag.
        2) Verify that the Off tag appears in only one SupportedModes entry.
        3) If the EmergencyHeat tag is included in a list:
           3a) Verify that the Heat tag appears in the same list as the EmergencyHeat tag.
           3b) One or more manufacturer tags MAY be included in the same list as the EmergencyHeat tag.
           3c) The Heat tag SHALL appear without the EmergencyHeat tag in a separate list from the list which includes the Heat and EmergencyHeat tags.
        4) The Heat, Cool and Auto tags MAY appear in a list by themselves or with one or more manufacturer tag.
        5) The Heat, Cool or Auto SHALL NOT appear in a list together or in a list with any two of these tags.
        6) If a manufacturer tag appears in a list, a standard tag SHALL also appear in the same list.
        7) Verify that each ModeTags field contains at most 8 entries.
        8) Verify that each Label field has a length of at most 64.
        """
        mode_tags_enum = Clusters.ThermostatMode.Enums.ModeTag
        core_tags = {
            mode_tags_enum.kOff,
            mode_tags_enum.kCool,
            mode_tags_enum.kHeat,
            mode_tags_enum.kAuto,
        }
        standard_tags = core_tags | {mode_tags_enum.kEmergencyHeat}

        off_entries_count = 0
        has_emergency_heat = False
        heat_without_emergency_heat_count = 0

        for entry in supported_modes:
            tag_values = [tag.value for tag in entry.modeTags]

            # 7) Verify that each ModeTags field contains at most 8 entries.
            asserts.assert_less_equal(
                len(entry.modeTags), 8,
                f"ModeTags field for mode {entry.mode} has {len(entry.modeTags)} entries, maximum allowed is 8",
            )

            # 8) Verify that each Label field has a length of at most 64.
            asserts.assert_less_equal(
                len(entry.label), 64,
                f"Label for mode {entry.mode} has length {len(entry.label)}, maximum allowed is 64",
            )

            # 1) Verify that each entry contains exactly one of Off, Cool, Heat, or Auto mode tag.
            core_tags_in_entry = [t for t in tag_values if t in core_tags]
            asserts.assert_equal(
                len(core_tags_in_entry), 1,
                f"Entry for mode {entry.mode} must contain exactly one of Off, Cool, Heat, or Auto tag; found: {core_tags_in_entry}",
            )

            if mode_tags_enum.kOff in tag_values:
                off_entries_count += 1

            if mode_tags_enum.kEmergencyHeat in tag_values:
                has_emergency_heat = True
                # 3a) Verify that the Heat tag appears in the same list as the EmergencyHeat tag.
                asserts.assert_in(
                    mode_tags_enum.kHeat, tag_values,
                    f"Heat tag must appear in the same list as EmergencyHeat tag in mode {entry.mode}",
                )
                # 3b) One or more manufacturer tags MAY be included with EmergencyHeat.
                for t in tag_values:
                    if t not in (mode_tags_enum.kHeat, mode_tags_enum.kEmergencyHeat):
                        is_mfg = (0x8000 <= t <= 0xBFFF)
                        asserts.assert_true(
                            is_mfg,
                            f"Tag {hex(t)} in EmergencyHeat mode {entry.mode} is neither Heat, EmergencyHeat, nor a manufacturer tag",
                        )
            elif mode_tags_enum.kHeat in tag_values:
                heat_without_emergency_heat_count += 1

            # Validate that every tag is either a standard tag (Auto, Off, Cool, Heat, EmergencyHeat) or in the manufacturer range.
            for t in tag_values:
                is_mfg = (0x8000 <= t <= 0xBFFF)
                is_std = (t in standard_tags)
                asserts.assert_true(
                    is_mfg or is_std,
                    f"Tag {hex(t)} in mode {entry.mode} is neither a standard tag nor a manufacturer tag",
                )

            # 6) If a manufacturer tag appears in a list, a standard tag SHALL also appear in the same list.
            has_mfg = any(0x8000 <= t <= 0xBFFF for t in tag_values)
            if has_mfg:
                has_std = any(t in standard_tags for t in tag_values)
                asserts.assert_true(
                    has_std,
                    f"Mode {entry.mode} contains a manufacturer tag but no standard tag",
                )

        # 2) Verify that the Off tag appears in only one SupportedModes entry.
        asserts.assert_equal(
            off_entries_count, 1,
            f"Off tag must appear in exactly one SupportedModes entry; found in {off_entries_count} entries",
        )

        # 3c) The Heat SHALL appear without the EmergencyHeat tag in a separate list from the list which includes Heat and EmergencyHeat.
        if has_emergency_heat:
            asserts.assert_greater_equal(
                heat_without_emergency_heat_count, 1,
                "Heat tag must appear without EmergencyHeat tag in at least one separate SupportedModes entry",
            )

    @async_test_body
    async def test_TC_TSTATM_1_2(self):
        endpoint = self.get_endpoint()

        # Step 1: Commissioning, already done
        self.step(1)

        # Step 2a: Validate SupportedModes
        self.step("2a")
        supported_modes = await self.check_supported_modes_and_labels(endpoint=endpoint)
        supported_mode_tags_dut: list[int] = []
        for m in supported_modes:
            for t in m.modeTags:
                supported_mode_tags_dut.append(t.value)

        # Step 2b: Validate ModeTags entries
        self.step("2b")
        self.check_tags_in_lists(supported_modes=supported_modes)

        # Step 2c: Validate Thermostat Mode conditions
        self.step("2c")
        self.check_thermostat_mode_tag_conditions(supported_modes=supported_modes)

        # Step 3: CurrentMode attribute
        self.step(3)
        mode_attr = self.cluster.Attributes.CurrentMode
        current_mode = await self.read_and_check_mode(endpoint=endpoint, mode=mode_attr, supported_modes=supported_modes)
        logger.info("Current mode: %s", current_mode)

        # Step 5: StartUpMode attribute (optional)
        self.step(5)
        if await self.attribute_guard(endpoint=endpoint, attribute=self.cluster.Attributes.StartUpMode):
            startup_mode = await self.read_and_check_mode(
                endpoint=endpoint, mode=self.cluster.Attributes.StartUpMode, supported_modes=supported_modes, is_nullable=True
            )
            logger.info("StartUpMode: %s", startup_mode)

        # Step 6: CoreModeTags attribute (CoreModes feature)
        self.step(6)
        if await self.attribute_guard(endpoint=endpoint, attribute=self.cluster.Attributes.CoreModeTags):
            core_mode_tags = await self.read_single_attribute_check_success(
                endpoint=endpoint,
                cluster=self.cluster,
                attribute=self.cluster.Attributes.CoreModeTags,
            )
            logger.info("CoreModeTags: %s", core_mode_tags)
            asserts.assert_greater_equal(len(core_mode_tags), 1, "CoreModeTags must have at least 1 entry")
            asserts.assert_less_equal(len(core_mode_tags), 16, "CoreModeTags must have at most 16 entries")
            asserts.assert_equal(len(core_mode_tags), len(set(core_mode_tags)), "CoreModeTags contains duplicate values")

            allowed_core_mode_tags = [
                cluster_tstat_mode.Enums.ModeTag.kOff,
                cluster_tstat_mode.Enums.ModeTag.kCool,
                cluster_tstat_mode.Enums.ModeTag.kHeat,
                cluster_tstat_mode.Enums.ModeTag.kAuto,
            ]
            for tag in core_mode_tags:
                asserts.assert_in(
                    tag, supported_mode_tags_dut,
                    f"Core mode tag {tag} is not in supported_mode_tags_dut: {supported_mode_tags_dut}",
                )
                asserts.assert_in(
                    tag, allowed_core_mode_tags,
                    f"Core mode tag {tag} is not one of allowedCoreModeTags: {allowed_core_mode_tags}",
                )


if __name__ == "__main__":
    default_matter_test_main()
