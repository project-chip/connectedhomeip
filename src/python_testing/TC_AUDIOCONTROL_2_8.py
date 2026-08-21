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
import math

from mobly import asserts
from TC_AUDIOCONTROLTestBase import AUDIOCONTROLTestBase

import matter.clusters as Clusters
from matter.testing import matter_asserts
from matter.testing.decorators import EndpointCheckFunction, has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# Device type IDs are not emitted into the cluster codegen, so they are declared here as
# module constants, matching the _DEVICE_TYPE_* pattern used by the other python tests.
_SPEAKER_DEVICE_TYPE_ID = 0x0022
_SPEAKER_MIN_REVISION = 2

# Fallbacks defined by the Level Control cluster when these optional attributes are absent.
_MIN_LEVEL_FALLBACK = 1
_MAX_LEVEL_FALLBACK = 254


def has_speaker_audio_clusters() -> EndpointCheckFunction:
    """Accept endpoints carrying Audio Control alongside On/Off and Level Control.

    The dependencies under test only exist when all three clusters sit on the same
    endpoint, which is the Revision 2 Speaker device type composition.
    """
    checks = (has_cluster(Clusters.AudioControl),
              has_cluster(Clusters.OnOff),
              has_cluster(Clusters.LevelControl))

    def accept(wildcard, endpoint) -> bool:
        return all(check(wildcard, endpoint) for check in checks)

    return accept


class TC_AUDIOCONTROL_2_8(MatterBaseTest, AUDIOCONTROLTestBase):

    def desc_TC_AUDIOCONTROL_2_8(self) -> str:
        return "[TC-AUDIOCONTROL-2.8] On/Off and Level Control Cluster Interaction with DUT as Server"

    def pics_TC_AUDIOCONTROL_2_8(self) -> list[str]:
        return ["AUDIOCONTROL.S", "OO.S", "LVL.S"]

    def steps_TC_AUDIOCONTROL_2_8(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "TH reads MinDeviceVolume attribute.", "Record as minDeviceVolume."),
            TestStep("2b", "TH reads MaxDeviceVolume attribute.", "Record as maxDeviceVolume."),
            TestStep("2c", "TH reads MaxUserVolume attribute.",
                     "Record as maxUserVolume, or fall back to maxDeviceVolume when unsupported. Record "
                     "minOf(maxDeviceVolume, maxUserVolume) as effectiveMaxVolume."),
            TestStep("2d", "TH reads MinLevel attribute from the Level Control cluster.",
                     "Record as minLevel, or fall back to 1 when the attribute is absent."),
            TestStep("2e", "TH reads MaxLevel attribute from the Level Control cluster.",
                     "Record as maxLevel, or fall back to 254 when the attribute is absent. Verify maxLevel is "
                     "greater than or equal to minLevel."),
            TestStep("2f", "TH reads DeviceTypeList attribute from the Descriptor cluster on the endpoint under test.",
                     "Verify the list contains an entry with DeviceType 0x0022 (Speaker) and a Revision of 2 or "
                     "greater."),
        ]

        # Step 3: Audio Control -> On/Off. Each command is followed by a SoftMuted and an OnOff read.
        for step_id, command, soft_muted in (("3a", "Unmute", False), ("3d", "Mute", True),
                                             ("3g", "ToggleMuted", False)):
            reads = chr(ord(step_id[1]) + 1), chr(ord(step_id[1]) + 2)
            steps.extend([
                TestStep(step_id, f"TH sends a {command} command to the DUT.",
                         "Verify DUT responds with a SUCCESS status response."),
                TestStep(f"3{reads[0]}", "TH reads SoftMuted attribute.",
                         f"DUT replies with {str(soft_muted).upper()}."),
                TestStep(f"3{reads[1]}", "TH reads OnOff attribute from the On/Off cluster.",
                         f"DUT replies with {str(not soft_muted).upper()}, the inverse of SoftMuted."),
            ])

        # Step 4: On/Off -> Audio Control. Each command is followed by an OnOff and a SoftMuted read.
        for step_id, command, on_off in (("4a", "Off", False), ("4d", "On", True), ("4g", "Toggle", False)):
            reads = chr(ord(step_id[1]) + 1), chr(ord(step_id[1]) + 2)
            steps.extend([
                TestStep(step_id, f"TH sends an {command} command to the On/Off cluster of the DUT.",
                         "Verify DUT responds with a SUCCESS status response."),
                TestStep(f"4{reads[0]}", "TH reads OnOff attribute from the On/Off cluster.",
                         f"DUT replies with {str(on_off).upper()}."),
                TestStep(f"4{reads[1]}", "TH reads SoftMuted attribute.",
                         f"DUT replies with {str(not on_off).upper()}, the inverse of OnOff."),
            ])

        steps.extend([
            TestStep("5a", "TH reads PhysicallyMuted attribute.", "Record as physicallyMutedOld."),
            TestStep("5b", "TH sends an Unmute command to the DUT.",
                     "Verify DUT responds with a SUCCESS status response."),
            TestStep("5c", "The DUT is physically muted as per manufacturer documentation. TH reads "
                     "PhysicallyMuted attribute.", "DUT replies with TRUE."),
            TestStep("5d", "TH reads SoftMuted attribute.", "DUT replies with FALSE, unchanged by the physical mute."),
            TestStep("5e", "TH reads OnOff attribute from the On/Off cluster.",
                     "DUT replies with TRUE, unchanged by the physical mute, as PhysicallyMuted has no corresponding "
                     "representation in the On/Off cluster."),
            TestStep("5f", "The DUT is restored to its original physical mute state as per manufacturer "
                     "documentation. TH reads PhysicallyMuted attribute.", "DUT replies with physicallyMutedOld."),
            TestStep("6a", "TH sends an Unmute command to the DUT, so that the mute state does not confound the "
                     "level comparisons.", "Verify DUT responds with a SUCCESS status response."),
            TestStep("6b", "TH sends a SetVolume command with NewVolume set to minDeviceVolume.",
                     "Verify DUT responds with a SUCCESS status response."),
            TestStep("6c", "TH reads Volume attribute.", "DUT replies with minDeviceVolume."),
            TestStep("6d", "TH reads CurrentLevel attribute from the Level Control cluster.",
                     "DUT replies with minLevel, the value minDeviceVolume maps to."),
            TestStep("6e", "TH sends a SetVolume command with NewVolume set to effectiveMaxVolume.",
                     "Verify DUT responds with a SUCCESS status response."),
            TestStep("6f", "TH reads Volume attribute.", "DUT replies with effectiveMaxVolume."),
            TestStep("6g", "TH reads CurrentLevel attribute from the Level Control cluster.",
                     "DUT replies with maxLevel, the value effectiveMaxVolume maps to."),
            TestStep("6h", "TH sends a SetVolume command with NewVolume set to midVolume = minDeviceVolume + "
                     "floor((effectiveMaxVolume - minDeviceVolume) / 2).",
                     "Verify DUT responds with a SUCCESS status response."),
            TestStep("6i", "TH reads CurrentLevel attribute from the Level Control cluster.",
                     "DUT replies with the level midVolume maps to, within the documented tolerance."),
            TestStep("7a", "TH sends a MoveToLevel command to the Level Control cluster with Level set to minLevel "
                     "and TransitionTime, OptionsMask and OptionsOverride set to 0.",
                     "Verify DUT responds with a SUCCESS status response."),
            TestStep("7b", "TH reads CurrentLevel attribute from the Level Control cluster.",
                     "DUT replies with minLevel."),
            TestStep("7c", "TH reads Volume attribute.",
                     "DUT replies with minDeviceVolume, the volume minLevel maps to."),
            TestStep("7d", "TH sends a MoveToLevel command to the Level Control cluster with Level set to maxLevel "
                     "and TransitionTime, OptionsMask and OptionsOverride set to 0.",
                     "Verify DUT responds with a SUCCESS status response."),
            TestStep("7e", "TH reads CurrentLevel attribute from the Level Control cluster.",
                     "DUT replies with maxLevel."),
            TestStep("7f", "TH reads Volume attribute.",
                     "DUT replies with effectiveMaxVolume, the volume maxLevel maps to."),
            TestStep("7g", "TH sends a MoveToLevel command to the Level Control cluster with Level set to midLevel = "
                     "minLevel + floor((maxLevel - minLevel) / 2) and TransitionTime, OptionsMask and "
                     "OptionsOverride set to 0.", "Verify DUT responds with a SUCCESS status response."),
            TestStep("7h", "TH reads Volume attribute.",
                     "DUT replies with the volume midLevel maps to, within the documented tolerance."),
            TestStep("8a", "TH reads MaxUserVolume attribute.", "Record as maxUserVolumeOld."),
            TestStep("8b", "TH writes MaxUserVolume with a value reducedMax, strictly greater than minDeviceVolume "
                     "and strictly less than minOf(maxDeviceVolume, maxUserVolumeOld).",
                     "Verify the write request was successful."),
            TestStep("8c", "TH sends a MoveToLevel command to the Level Control cluster with Level set to maxLevel "
                     "and TransitionTime, OptionsMask and OptionsOverride set to 0.",
                     "Verify DUT responds with a SUCCESS status response."),
            TestStep("8d", "TH reads Volume attribute.",
                     "DUT replies with reducedMax and not maxDeviceVolume, demonstrating that the MaxUserVolume "
                     "limit is respected when the level is set through the Level Control cluster."),
            TestStep("8e", "TH reads CurrentLevel attribute from the Level Control cluster.",
                     "DUT replies with maxLevel, since maxLevel now maps to the reduced ceiling reducedMax."),
            TestStep("8f", "TH writes MaxUserVolume with the value maxUserVolumeOld.",
                     "Verify the write request was successful."),
            TestStep(9, "TH sends a SetVolume command with NewVolume set to minDeviceVolume and an Unmute command "
                     "to the DUT, to leave it in a known state.",
                     "Verify both commands return SUCCESS status responses."),
        ])
        return steps

    @staticmethod
    def _map_value(value: int, source_min: int, source_max: int, target_min: int, target_max: int) -> int:
        """Scale value from [source_min, source_max] onto [target_min, target_max]."""
        span = source_max - source_min
        return target_min + round((value - source_min) * (target_max - target_min) / span)

    @staticmethod
    def _tolerance(source_min: int, source_max: int, target_min: int, target_max: int) -> int:
        """Accepted deviation when mapping between two ranges of differing granularity.

        One step of the coarser range spans several units of the finer one, so a
        conformant implementation's rounding can land up to half a source step away. The
        bound stays tight enough to fail a DUT that does not track the other attribute.
        """
        return max(1, math.ceil((target_max - target_min) / (2 * (source_max - source_min))))

    async def _read_on_off(self, endpoint) -> bool:
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.OnOff, attribute=Clusters.OnOff.Attributes.OnOff)

    async def _read_current_level(self, endpoint) -> int:
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.LevelControl,
            attribute=Clusters.LevelControl.Attributes.CurrentLevel)

    async def _move_to_level(self, endpoint, level: int) -> None:
        # MoveToLevel rather than MoveToLevelWithOnOff: the latter would change OnOff, and so
        # SoftMuted via the dependency verified in step 4, confounding the mapping under test.
        await self.send_single_cmd(
            cmd=Clusters.LevelControl.Commands.MoveToLevel(
                level=level, transitionTime=0, optionsMask=0, optionsOverride=0),
            endpoint=endpoint)

    @run_if_endpoint_matches(has_speaker_audio_clusters())
    async def test_TC_AUDIOCONTROL_2_8(self):
        cluster = Clusters.AudioControl
        attributes = cluster.Attributes
        level_attributes = Clusters.LevelControl.Attributes
        endpoint = self.get_endpoint()

        self.step(1)
        await self.read_audiocontrol_capabilities(endpoint)

        self.step("2a")
        min_device_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MinDeviceVolume)

        self.step("2b")
        max_device_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MaxDeviceVolume)

        self.step("2c")
        if self.supports_attribute(attributes.MaxUserVolume):
            max_user_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MaxUserVolume)
        else:
            max_user_volume = max_device_volume
            self.mark_current_step_skipped()
        effective_max_volume = min(max_device_volume, max_user_volume)

        level_attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.LevelControl, attribute=level_attributes.AttributeList)

        self.step("2d")
        if level_attributes.MinLevel.attribute_id in level_attribute_list:
            min_level = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=Clusters.LevelControl, attribute=level_attributes.MinLevel)
        else:
            min_level = _MIN_LEVEL_FALLBACK
            log.info("MinLevel is not implemented; using the specification fallback of %d", min_level)
        matter_asserts.assert_valid_uint8(min_level, "MinLevel")

        self.step("2e")
        if level_attributes.MaxLevel.attribute_id in level_attribute_list:
            max_level = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=Clusters.LevelControl, attribute=level_attributes.MaxLevel)
        else:
            max_level = _MAX_LEVEL_FALLBACK
            log.info("MaxLevel is not implemented; using the specification fallback of %d", max_level)
        matter_asserts.assert_valid_uint8(max_level, "MaxLevel")
        asserts.assert_greater_equal(max_level, min_level, "MaxLevel must be at least MinLevel")

        self.step("2f")
        device_type_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=Clusters.Descriptor, attribute=Clusters.Descriptor.Attributes.DeviceTypeList)
        speaker_entries = [entry for entry in device_type_list if entry.deviceType == _SPEAKER_DEVICE_TYPE_ID]
        asserts.assert_true(
            speaker_entries,
            f"Endpoint {endpoint} must declare the Speaker device type (0x{_SPEAKER_DEVICE_TYPE_ID:04X}); "
            f"DeviceTypeList reported {[entry.deviceType for entry in device_type_list]}")
        asserts.assert_greater_equal(
            max(entry.revision for entry in speaker_entries), _SPEAKER_MIN_REVISION,
            "The Audio Control cluster is only allowed on the Speaker device type from Revision 2 onwards")

        # Step 3: a change made through the Audio Control cluster is reflected in the On/Off cluster.
        for step_id, send_command, expected_soft_muted in (("3a", self.send_unmute, False),
                                                           ("3d", self.send_mute, True),
                                                           ("3g", self.send_toggle_muted, False)):
            group, offset = step_id[0], ord(step_id[1])
            self.step(step_id)
            await send_command(endpoint)

            self.step(f"{group}{chr(offset + 1)}")
            await self.verify_soft_muted(endpoint, expected_soft_muted)

            self.step(f"{group}{chr(offset + 2)}")
            on_off = await self._read_on_off(endpoint)
            asserts.assert_equal(on_off, not expected_soft_muted,
                                 "OnOff must be the inverse of SoftMuted after an Audio Control command")

        # Step 4: a change made through the On/Off cluster is reflected in the Audio Control cluster.
        on_off_commands = (("4a", Clusters.OnOff.Commands.Off(), False),
                           ("4d", Clusters.OnOff.Commands.On(), True),
                           ("4g", Clusters.OnOff.Commands.Toggle(), False))
        for step_id, command, expected_on_off in on_off_commands:
            group, offset = step_id[0], ord(step_id[1])
            self.step(step_id)
            await self.send_single_cmd(cmd=command, endpoint=endpoint)

            self.step(f"{group}{chr(offset + 1)}")
            on_off = await self._read_on_off(endpoint)
            asserts.assert_equal(on_off, expected_on_off,
                                 f"OnOff should be {expected_on_off} after {type(command).__name__}")

            self.step(f"{group}{chr(offset + 2)}")
            await self.verify_soft_muted(endpoint, not expected_on_off)

        # Step 5: PhysicallyMuted has no representation in the On/Off cluster.
        await self._verify_physically_muted_independence(endpoint)

        # Steps 6-8 scale between two ranges, so both must span more than a single value.
        volume_span = effective_max_volume - min_device_volume
        level_span = max_level - min_level
        if volume_span < 1 or level_span < 1:
            log.info("Volume span is %d and level span is %d; a proportional mapping needs both to be non-zero",
                     volume_span, level_span)
            self.mark_all_remaining_steps_skipped("6a")
            return

        volume_to_level_tolerance = self._tolerance(min_device_volume, effective_max_volume, min_level, max_level)
        level_to_volume_tolerance = self._tolerance(min_level, max_level, min_device_volume, effective_max_volume)

        # Step 6: a volume change made through Audio Control is reflected proportionally in CurrentLevel.
        self.step("6a")
        await self.send_unmute(endpoint)

        self.step("6b")
        await self.send_set_volume(endpoint, min_device_volume)

        self.step("6c")
        await self.verify_volume(endpoint, min_device_volume)

        self.step("6d")
        # The bottom of each range maps exactly, so no tolerance is allowed here.
        current_level = await self._read_current_level(endpoint)
        asserts.assert_equal(current_level, min_level,
                             "CurrentLevel must sit at MinLevel when Volume is at MinDeviceVolume")

        self.step("6e")
        await self.send_set_volume(endpoint, effective_max_volume)

        self.step("6f")
        await self.verify_volume(endpoint, effective_max_volume)

        self.step("6g")
        # The top of each range maps exactly as well.
        current_level = await self._read_current_level(endpoint)
        asserts.assert_equal(current_level, max_level,
                             "CurrentLevel must sit at MaxLevel when Volume is at the effective maximum")

        mid_volume = min_device_volume + volume_span // 2

        self.step("6h")
        await self.send_set_volume(endpoint, mid_volume)

        self.step("6i")
        expected_level = self._map_value(mid_volume, min_device_volume, effective_max_volume, min_level, max_level)
        current_level = await self._read_current_level(endpoint)
        self._assert_within(current_level, expected_level, volume_to_level_tolerance,
                            f"CurrentLevel for Volume {mid_volume}")

        # Step 7: a level change made through Level Control is reflected proportionally in Volume.
        self.step("7a")
        await self._move_to_level(endpoint, min_level)

        self.step("7b")
        current_level = await self._read_current_level(endpoint)
        asserts.assert_equal(current_level, min_level, "CurrentLevel should be MinLevel after MoveToLevel")

        self.step("7c")
        await self.verify_volume(endpoint, min_device_volume)

        self.step("7d")
        await self._move_to_level(endpoint, max_level)

        self.step("7e")
        current_level = await self._read_current_level(endpoint)
        asserts.assert_equal(current_level, max_level, "CurrentLevel should be MaxLevel after MoveToLevel")

        self.step("7f")
        await self.verify_volume(endpoint, effective_max_volume)

        mid_level = min_level + level_span // 2

        self.step("7g")
        await self._move_to_level(endpoint, mid_level)

        self.step("7h")
        expected_volume = self._map_value(mid_level, min_level, max_level, min_device_volume, effective_max_volume)
        volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.Volume)
        self._assert_within(volume, expected_volume, level_to_volume_tolerance,
                            f"Volume for CurrentLevel {mid_level}")

        # Step 8: the MaxUserVolume limit is respected when the level is changed via Level Control.
        await self._verify_max_user_volume_limit(endpoint, min_device_volume, max_device_volume, max_level)

        self.step(9)
        await self.send_set_volume(endpoint, min_device_volume)
        await self.send_unmute(endpoint)

    @staticmethod
    def _assert_within(actual: int, expected: int, tolerance: int, description: str) -> None:
        asserts.assert_less_equal(
            abs(actual - expected), tolerance,
            f"{description} was {actual}, expected {expected} within a tolerance of {tolerance}")

    async def _verify_physically_muted_independence(self, endpoint) -> None:
        """Steps 5a-5f: a physical mute must not disturb SoftMuted or OnOff."""
        attributes = Clusters.AudioControl.Attributes
        supported = self.supports_attribute(attributes.PhysicallyMuted)

        self.step("5a")
        physically_muted_old = None
        if supported:
            physically_muted_old = await self.read_audiocontrol_attribute_expect_success(
                endpoint, attributes.PhysicallyMuted)
            matter_asserts.assert_valid_bool(physically_muted_old, "PhysicallyMuted")
        else:
            log.info("PhysicallyMuted is not implemented; the step 5 dependency does not apply")
            self.mark_current_step_skipped()

        self.step("5b")
        if supported:
            await self.send_unmute(endpoint)
        else:
            self.mark_current_step_skipped()

        # PhysicallyMuted is read-only, so only an operator can change it. A DUT with no such
        # control cannot exercise this dependency, which is distinct from failing it.
        muted = False
        if supported:
            response = self.wait_for_user_input(
                prompt_msg="Physically mute the DUT, per the manufacturer's documentation.\n"
                           "Enter 'y' once done, or 'n' if the DUT provides no way to physically mute it.\n",
                prompt_msg_placeholder="y", default_value="y")
            muted = response is not None and response.lower() == "y"
            if not muted:
                log.info("Operator reported that the DUT cannot be physically muted; skipping steps 5c-5f")

        self.step("5c")
        if muted:
            physically_muted = await self.read_audiocontrol_attribute_expect_success(
                endpoint, attributes.PhysicallyMuted)
            asserts.assert_true(physically_muted,
                                "PhysicallyMuted should be TRUE once the DUT has been physically muted")
        else:
            self.mark_current_step_skipped()

        self.step("5d")
        if muted:
            await self.verify_soft_muted(endpoint, False)
        else:
            self.mark_current_step_skipped()

        self.step("5e")
        if muted:
            on_off = await self._read_on_off(endpoint)
            asserts.assert_true(on_off,
                                "OnOff must stay TRUE while only PhysicallyMuted is set, because PhysicallyMuted "
                                "has no corresponding representation in the On/Off cluster")
        else:
            self.mark_current_step_skipped()

        self.step("5f")
        if muted:
            self.wait_for_user_input(
                prompt_msg="Restore the DUT to its original physical mute state, then press Enter.\n")
            physically_muted = await self.read_audiocontrol_attribute_expect_success(
                endpoint, attributes.PhysicallyMuted)
            asserts.assert_equal(physically_muted, physically_muted_old,
                                 "PhysicallyMuted should return to its original value")
        else:
            self.mark_current_step_skipped()

    async def _verify_max_user_volume_limit(self, endpoint, min_device_volume: int, max_device_volume: int,
                                            max_level: int) -> None:
        """Steps 8a-8f: driving the level to MaxLevel must respect the MaxUserVolume ceiling."""
        attributes = Clusters.AudioControl.Attributes
        supported = self.supports_attribute(attributes.MaxUserVolume)

        self.step("8a")
        max_user_volume_old = None
        if supported:
            max_user_volume_old = await self.read_audiocontrol_attribute_expect_success(
                endpoint, attributes.MaxUserVolume)
        else:
            log.info("MaxUserVolume is not implemented; there is no user ceiling to enforce")
            self.mark_current_step_skipped()

        # reducedMax must leave headroom on both sides so that reaching it is a visible change
        # and is distinguishable from MaxDeviceVolume.
        reduced_max = None
        if supported:
            ceiling = min(max_device_volume, max_user_volume_old)
            if ceiling - min_device_volume >= 2:
                reduced_max = min_device_volume + (ceiling - min_device_volume) // 2
            else:
                log.info("MaxUserVolume range [%d, %d] is too narrow to pick a distinguishable reduced ceiling",
                         min_device_volume, ceiling)

        self.step("8b")
        if reduced_max is not None:
            await self.write_audiocontrol_attribute_expect_success(
                endpoint, attributes.MaxUserVolume, reduced_max)
        else:
            self.mark_current_step_skipped()

        self.step("8c")
        if reduced_max is not None:
            await self._move_to_level(endpoint, max_level)
        else:
            self.mark_current_step_skipped()

        self.step("8d")
        if reduced_max is not None:
            volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.Volume)
            asserts.assert_equal(
                volume, reduced_max,
                f"Volume should be capped at the reduced MaxUserVolume ({reduced_max}) after moving the level to "
                f"MaxLevel, not raised towards MaxDeviceVolume ({max_device_volume})")
        else:
            self.mark_current_step_skipped()

        self.step("8e")
        if reduced_max is not None:
            current_level = await self._read_current_level(endpoint)
            asserts.assert_equal(current_level, max_level,
                                 "CurrentLevel should remain at MaxLevel, which now maps to the reduced ceiling")
        else:
            self.mark_current_step_skipped()

        self.step("8f")
        if reduced_max is not None:
            await self.write_audiocontrol_attribute_expect_success(
                endpoint, attributes.MaxUserVolume, max_user_volume_old)
        else:
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
