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
from TC_AUDIOCONTROLTestBase import AUDIOCONTROLTestBase

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AUDIOCONTROL_2_2(MatterBaseTest, AUDIOCONTROLTestBase):

    def desc_TC_AUDIOCONTROL_2_2(self) -> str:
        return "[TC-AUDIOCONTROL-2.2] Write cluster attributes with DUT as Server"

    def pics_TC_AUDIOCONTROL_2_2(self) -> list[str]:
        return ["AUDIOCONTROL.S"]

    def steps_TC_AUDIOCONTROL_2_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "TH reads MinDeviceVolume attribute.", "Record as minDeviceVolume."),
            TestStep("2b", "TH reads MaxDeviceVolume attribute.", "Record as maxDeviceVolume."),
            TestStep("2c", "TH reads MaxUserVolume attribute.",
                     "Record as maxUserVolume, or fall back to maxDeviceVolume when unsupported."),
            TestStep("2d", "TH reads MinCorrection attribute.", "Record as minCorrection when supported."),
            TestStep("2e", "TH reads MaxCorrection attribute.", "Record as maxCorrection when supported."),
            TestStep("3a", "TH reads Volume attribute.", "Record as volume."),
            TestStep("3b", "TH reads MaxUserVolume attribute.", "Record as maxUserVolumeOld."),
            TestStep("3c", "TH writes MaxUserVolume with a value different from maxUserVolumeOld, within "
                     "minDeviceVolume to maxDeviceVolume, and greater than or equal to volume.",
                     "Verify the write request was successful."),
            TestStep("3d", "TH reads MaxUserVolume attribute.", "DUT replies with the value previously written."),
            TestStep("3e", "TH reads Volume attribute.",
                     "Verify the value is less than or equal to the value written in step 3c."),
            TestStep("3f", "TH writes MaxUserVolume with the value maxUserVolumeOld.",
                     "Verify the write request was successful."),
            TestStep("3g", "TH sets Volume to maxUserVolumeOld via SetVolume, then sets StartUpVolume to "
                     "maxUserVolumeOld if supported.", "Verify the requests were successful."),
            TestStep("3h", "TH writes MaxUserVolume with minDeviceVolume, a value below both the current Volume "
                     "and StartUpVolume.", "Verify the write request was successful."),
            TestStep("3i", "TH reads Volume attribute.",
                     "DUT replies with minDeviceVolume, clipped down to the new MaxUserVolume."),
            TestStep("3j", "TH reads StartUpVolume attribute.",
                     "If not null, DUT replies with minDeviceVolume, clipped down to the new MaxUserVolume."),
            TestStep("3k", "TH writes MaxUserVolume with the value maxUserVolumeOld.",
                     "Verify the write request was successful."),
            TestStep("4a", "TH reads DefaultStepSize attribute.", "Record as defaultStepSizeOld."),
            TestStep("4b", "TH writes DefaultStepSize with a value different from defaultStepSizeOld, within 1 to "
                     "(minOf(maxDeviceVolume, maxUserVolume) - minDeviceVolume).",
                     "Verify the write request was successful."),
            TestStep("4c", "TH reads DefaultStepSize attribute.", "DUT replies with the value previously written."),
            TestStep("4d", "TH writes DefaultStepSize with the value defaultStepSizeOld.",
                     "Verify the write request was successful."),
        ]

        enum_attributes = (
            ("5", "SetVolumeUnmutePolicy", "UnmutePolicyEnum"),
            ("6", "IncreaseVolumeUnmutePolicy", "UnmutePolicyEnum"),
            ("7", "IncreaseVolumeUnmuteVolume", "UnmuteVolumeEnum"),
            ("8", "DecreaseVolumeUnmutePolicy", "UnmutePolicyEnum"),
        )
        for prefix, name, enum_name in enum_attributes:
            steps.extend([
                TestStep(f"{prefix}a", f"TH reads {name} attribute.", f"Record as {name}Old."),
                TestStep(f"{prefix}b", f"TH writes {name} with a valid {enum_name} value different from {name}Old.",
                         "Verify the write request was successful."),
                TestStep(f"{prefix}c", f"TH reads {name} attribute.", "DUT replies with the value previously written."),
                TestStep(f"{prefix}d", f"TH writes {name} with the value {name}Old.",
                         "Verify the write request was successful."),
            ])

        steps.extend([
            TestStep("9a", "TH reads StartUpMuted attribute.", "Record as startUpMutedOld."),
            TestStep("9b", "TH writes StartUpMuted with a value different from startUpMutedOld (null, TRUE or FALSE).",
                     "Verify the write request was successful."),
            TestStep("9c", "TH reads StartUpMuted attribute.", "DUT replies with the value previously written."),
            TestStep("9d", "TH writes StartUpMuted with the value startUpMutedOld.",
                     "Verify the write request was successful."),
            TestStep("10a", "TH reads StartUpVolume attribute.", "Record as startUpVolumeOld."),
            TestStep("10b", "TH writes StartUpVolume with a value different from startUpVolumeOld, within "
                     "minDeviceVolume to minOf(maxDeviceVolume, maxUserVolume), or null.",
                     "Verify the write request was successful."),
            TestStep("10c", "TH reads StartUpVolume attribute.", "DUT replies with the value previously written."),
            TestStep("10d", "TH writes StartUpVolume with the value startUpVolumeOld.",
                     "Verify the write request was successful."),
        ])

        for prefix, name in (("11", "Bass"), ("12", "Mid"), ("13", "Treble")):
            steps.extend([
                TestStep(f"{prefix}a", f"TH reads {name} attribute.", f"Record as {name}Old."),
                TestStep(f"{prefix}b", f"TH writes {name} with a value different from {name}Old, within minCorrection "
                         "to maxCorrection.", "Verify the write request was successful."),
                TestStep(f"{prefix}c", f"TH reads {name} attribute.", "DUT replies with the value previously written."),
                TestStep(f"{prefix}d", f"TH writes {name} with the value {name}Old.",
                         "Verify the write request was successful."),
            ])

        return steps

    async def _exercise_enum_attribute(self, endpoint, prefix: str, attribute, enum_type):
        """Read/modify/verify/restore cycle shared by the four enum policy attributes."""
        self.step(f"{prefix}a")
        if not self.supports_attribute(attribute):
            self.mark_current_step_skipped()
            for suffix in ("b", "c", "d"):
                self.step(f"{prefix}{suffix}")
                self.mark_current_step_skipped()
            return

        old_value = await self.read_audiocontrol_attribute_expect_success(endpoint, attribute)

        self.step(f"{prefix}b")
        new_value = self.pick_different_enum(old_value, enum_type)
        await self.write_audiocontrol_attribute_expect_success(endpoint, attribute, new_value)

        self.step(f"{prefix}c")
        read_back = await self.read_audiocontrol_attribute_expect_success(endpoint, attribute)
        asserts.assert_equal(read_back, new_value, f"{attribute.__name__} did not retain the written value")

        self.step(f"{prefix}d")
        await self.write_audiocontrol_attribute_expect_success(endpoint, attribute, old_value)

    async def _exercise_tone_attribute(self, endpoint, prefix: str, attribute, min_correction, max_correction):
        """Read/modify/verify/restore cycle shared by Bass, Mid and Treble."""
        self.step(f"{prefix}a")
        if not self.supports_attribute(attribute) or min_correction is None or max_correction is None:
            self.mark_current_step_skipped()
            for suffix in ("b", "c", "d"):
                self.step(f"{prefix}{suffix}")
                self.mark_current_step_skipped()
            return

        old_value = await self.read_audiocontrol_attribute_expect_success(endpoint, attribute)

        self.step(f"{prefix}b")
        new_value = self.pick_different_in_range(old_value, min_correction, max_correction)
        if new_value is None:
            # MinCorrection == MaxCorrection leaves no alternative value to write.
            log.info("Correction range is a single value; skipping the %s write cycle", attribute.__name__)
            self.mark_current_step_skipped()
            for suffix in ("c", "d"):
                self.step(f"{prefix}{suffix}")
                self.mark_current_step_skipped()
            return
        await self.write_audiocontrol_attribute_expect_success(endpoint, attribute, new_value)

        self.step(f"{prefix}c")
        read_back = await self.read_audiocontrol_attribute_expect_success(endpoint, attribute)
        asserts.assert_equal(read_back, new_value, f"{attribute.__name__} did not retain the written value")

        self.step(f"{prefix}d")
        await self.write_audiocontrol_attribute_expect_success(endpoint, attribute, old_value)

    @run_if_endpoint_matches(has_cluster(Clusters.AudioControl))
    async def test_TC_AUDIOCONTROL_2_2(self):
        cluster = Clusters.AudioControl
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()

        self.step(1)
        await self.read_audiocontrol_capabilities(endpoint)

        self.step("2a")
        min_device_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MinDeviceVolume)

        self.step("2b")
        max_device_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MaxDeviceVolume)

        self.step("2c")
        supports_max_user_volume = self.supports_attribute(attributes.MaxUserVolume)
        if supports_max_user_volume:
            max_user_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MaxUserVolume)
        else:
            max_user_volume = max_device_volume
            self.mark_current_step_skipped()
        effective_max = min(max_device_volume, max_user_volume)

        self.step("2d")
        min_correction = None
        if self.supports_attribute(attributes.MinCorrection):
            min_correction = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MinCorrection)
        else:
            self.mark_current_step_skipped()

        self.step("2e")
        max_correction = None
        if self.supports_attribute(attributes.MaxCorrection):
            max_correction = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MaxCorrection)
        else:
            self.mark_current_step_skipped()

        # Step 3: MaxUserVolume, including its clipping effect on Volume and StartUpVolume
        supports_start_up_volume = self.supports_attribute(attributes.StartUpVolume)

        self.step("3a")
        volume = None
        if supports_max_user_volume:
            volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.Volume)
        else:
            self.mark_current_step_skipped()

        self.step("3b")
        max_user_volume_old = None
        if supports_max_user_volume:
            max_user_volume_old = await self.read_audiocontrol_attribute_expect_success(
                endpoint, attributes.MaxUserVolume)
        else:
            self.mark_current_step_skipped()

        # The new MaxUserVolume must sit at or above the current Volume so the write is a pure
        # ceiling change; step 3h separately covers the case where the ceiling drops below Volume.
        new_max_user_volume = None
        if supports_max_user_volume:
            new_max_user_volume = self.pick_different_in_range(
                max_user_volume_old, max(min_device_volume, volume), max_device_volume)

        self.step("3c")
        if new_max_user_volume is not None:
            await self.write_audiocontrol_attribute_expect_success(
                endpoint, attributes.MaxUserVolume, new_max_user_volume)
        else:
            log.info("No alternative MaxUserVolume value available at or above the current Volume")
            self.mark_current_step_skipped()

        self.step("3d")
        if new_max_user_volume is not None:
            read_back = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MaxUserVolume)
            asserts.assert_equal(read_back, new_max_user_volume, "MaxUserVolume did not retain the written value")
        else:
            self.mark_current_step_skipped()

        self.step("3e")
        if new_max_user_volume is not None:
            current_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.Volume)
            asserts.assert_less_equal(current_volume, new_max_user_volume,
                                      "Volume must not exceed the newly written MaxUserVolume")
        else:
            self.mark_current_step_skipped()

        self.step("3f")
        if new_max_user_volume is not None:
            await self.write_audiocontrol_attribute_expect_success(
                endpoint, attributes.MaxUserVolume, max_user_volume_old)
        else:
            self.mark_current_step_skipped()

        self.step("3g")
        if supports_max_user_volume:
            await self.send_set_volume(endpoint, max_user_volume_old)
            if supports_start_up_volume:
                await self.write_audiocontrol_attribute_expect_success(
                    endpoint, attributes.StartUpVolume, max_user_volume_old)
        else:
            self.mark_current_step_skipped()

        # Dropping the ceiling to minDeviceVolume must clip both Volume and StartUpVolume down.
        can_test_clipping = supports_max_user_volume and max_user_volume_old > min_device_volume

        self.step("3h")
        if can_test_clipping:
            await self.write_audiocontrol_attribute_expect_success(
                endpoint, attributes.MaxUserVolume, min_device_volume)
        else:
            log.info("MaxUserVolume is already at MinDeviceVolume; clipping behaviour cannot be exercised")
            self.mark_current_step_skipped()

        self.step("3i")
        if can_test_clipping:
            await self.verify_volume(endpoint, min_device_volume)
        else:
            self.mark_current_step_skipped()

        self.step("3j")
        if can_test_clipping and supports_start_up_volume:
            start_up_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.StartUpVolume)
            if start_up_volume is not NullValue:
                asserts.assert_equal(start_up_volume, min_device_volume,
                                     "StartUpVolume should have been clipped to the new MaxUserVolume")
        else:
            self.mark_current_step_skipped()

        self.step("3k")
        if can_test_clipping:
            await self.write_audiocontrol_attribute_expect_success(
                endpoint, attributes.MaxUserVolume, max_user_volume_old)
        else:
            self.mark_current_step_skipped()

        # Step 4: DefaultStepSize
        self.step("4a")
        default_step_size_old = await self.read_audiocontrol_attribute_expect_success(
            endpoint, attributes.DefaultStepSize)

        step_size_ceiling = effective_max - min_device_volume
        new_step_size = self.pick_different_in_range(default_step_size_old, 1, step_size_ceiling)

        self.step("4b")
        if new_step_size is not None:
            await self.write_audiocontrol_attribute_expect_success(
                endpoint, attributes.DefaultStepSize, new_step_size)
        else:
            log.info("DefaultStepSize range admits no alternative value")
            self.mark_current_step_skipped()

        self.step("4c")
        if new_step_size is not None:
            read_back = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.DefaultStepSize)
            asserts.assert_equal(read_back, new_step_size, "DefaultStepSize did not retain the written value")
        else:
            self.mark_current_step_skipped()

        self.step("4d")
        if new_step_size is not None:
            await self.write_audiocontrol_attribute_expect_success(
                endpoint, attributes.DefaultStepSize, default_step_size_old)
        else:
            self.mark_current_step_skipped()

        # Steps 5-8: the unmute policy enums
        await self._exercise_enum_attribute(endpoint, "5", attributes.SetVolumeUnmutePolicy,
                                            cluster.Enums.UnmutePolicyEnum)
        await self._exercise_enum_attribute(endpoint, "6", attributes.IncreaseVolumeUnmutePolicy,
                                            cluster.Enums.UnmutePolicyEnum)
        await self._exercise_enum_attribute(endpoint, "7", attributes.IncreaseVolumeUnmuteVolume,
                                            cluster.Enums.UnmuteVolumeEnum)
        await self._exercise_enum_attribute(endpoint, "8", attributes.DecreaseVolumeUnmutePolicy,
                                            cluster.Enums.UnmutePolicyEnum)

        # Step 9: StartUpMuted
        self.step("9a")
        supports_start_up_muted = self.supports_attribute(attributes.StartUpMuted)
        start_up_muted_old = None
        if supports_start_up_muted:
            start_up_muted_old = await self.read_audiocontrol_attribute_expect_success(
                endpoint, attributes.StartUpMuted)
        else:
            self.mark_current_step_skipped()

        self.step("9b")
        new_start_up_muted = None
        if supports_start_up_muted:
            # StartUpMuted is nullable, so the three distinct values are null, TRUE and FALSE.
            new_start_up_muted = False if start_up_muted_old is not False else True
            await self.write_audiocontrol_attribute_expect_success(
                endpoint, attributes.StartUpMuted, new_start_up_muted)
        else:
            self.mark_current_step_skipped()

        self.step("9c")
        if supports_start_up_muted:
            read_back = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.StartUpMuted)
            asserts.assert_equal(read_back, new_start_up_muted, "StartUpMuted did not retain the written value")
        else:
            self.mark_current_step_skipped()

        self.step("9d")
        if supports_start_up_muted:
            await self.write_audiocontrol_attribute_expect_success(
                endpoint, attributes.StartUpMuted, start_up_muted_old)
        else:
            self.mark_current_step_skipped()

        # Step 10: StartUpVolume
        self.step("10a")
        start_up_volume_old = None
        if supports_start_up_volume:
            start_up_volume_old = await self.read_audiocontrol_attribute_expect_success(
                endpoint, attributes.StartUpVolume)
        else:
            self.mark_current_step_skipped()

        self.step("10b")
        new_start_up_volume = None
        if supports_start_up_volume:
            new_start_up_volume = self.pick_different_in_range(start_up_volume_old, min_device_volume, effective_max)
            if new_start_up_volume is None:
                # The volume range is a single value, so null is the only distinct alternative.
                new_start_up_volume = NullValue if start_up_volume_old is not NullValue else min_device_volume
            await self.write_audiocontrol_attribute_expect_success(
                endpoint, attributes.StartUpVolume, new_start_up_volume)
        else:
            self.mark_current_step_skipped()

        self.step("10c")
        if supports_start_up_volume:
            read_back = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.StartUpVolume)
            asserts.assert_equal(read_back, new_start_up_volume, "StartUpVolume did not retain the written value")
        else:
            self.mark_current_step_skipped()

        self.step("10d")
        if supports_start_up_volume:
            await self.write_audiocontrol_attribute_expect_success(
                endpoint, attributes.StartUpVolume, start_up_volume_old)
        else:
            self.mark_current_step_skipped()

        # Steps 11-13: the Basic Equalizer tone controls
        await self._exercise_tone_attribute(endpoint, "11", attributes.Bass, min_correction, max_correction)
        await self._exercise_tone_attribute(endpoint, "12", attributes.Mid, min_correction, max_correction)
        await self._exercise_tone_attribute(endpoint, "13", attributes.Treble, min_correction, max_correction)


if __name__ == "__main__":
    default_matter_test_main()
