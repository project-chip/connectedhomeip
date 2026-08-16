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
from dataclasses import dataclass

from mobly import asserts
from TC_AUDIOCONTROLTestBase import AUDIOCONTROLTestBase

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing import matter_asserts
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


@dataclass
class _ToneControl:
    """One Basic Equalizer tone control and the steps that read and range-check it."""
    name: str
    attribute: type
    read_step: int
    range_step: int
    value: int | None = None


class TC_AUDIOCONTROL_2_1(MatterBaseTest, AUDIOCONTROLTestBase):

    def desc_TC_AUDIOCONTROL_2_1(self) -> str:
        return "[TC-AUDIOCONTROL-2.1] Attributes with DUT as Server"

    def pics_TC_AUDIOCONTROL_2_1(self) -> list[str]:
        return ["AUDIOCONTROL.S"]

    def steps_TC_AUDIOCONTROL_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads SoftMuted attribute.", "DUT replies with a bool value."),
            TestStep(3, "TH reads PhysicallyMuted attribute.", "DUT replies with a bool value."),
            TestStep(4, "TH reads Volume attribute.",
                     "DUT replies with a uint16 value. Record this value as volume."),
            TestStep(5, "TH reads MinDeviceVolume attribute.",
                     "DUT replies with a uint16 value greater than or equal to 1. Record as minDeviceVolume."),
            TestStep(6, "TH reads MaxDeviceVolume attribute.",
                     "DUT replies with a uint16 value greater than or equal to minDeviceVolume. Record as maxDeviceVolume."),
            TestStep(7, "Verify that volume is greater than or equal to minDeviceVolume."),
            TestStep(8, "Verify that volume is less than or equal to maxDeviceVolume (when MaxUserVolume is not supported)."),
            TestStep(9, "TH reads MaxDeviceVolumeDB attribute.", "DUT replies with a uint16 value."),
            TestStep(10, "TH reads MaxUserVolume attribute.",
                     "DUT replies with a uint16 value between minDeviceVolume and maxDeviceVolume. Record as maxUserVolume."),
            TestStep(11, "Verify that volume is less than or equal to maxUserVolume."),
            TestStep(12, "TH reads DefaultStepSize attribute.",
                     "DUT replies with a uint16 value in the range 1 to "
                     "(minOf(maxDeviceVolume, maxUserVolume) - minDeviceVolume)."),
            TestStep(13, "TH reads SetVolumeUnmutePolicy attribute.", "DUT replies with a valid UnmutePolicyEnum value."),
            TestStep(14, "TH reads IncreaseVolumeUnmutePolicy attribute.", "DUT replies with a valid UnmutePolicyEnum value."),
            TestStep(15, "TH reads IncreaseVolumeUnmuteVolume attribute.", "DUT replies with a valid UnmuteVolumeEnum value."),
            TestStep(16, "TH reads DecreaseVolumeUnmutePolicy attribute.", "DUT replies with a valid UnmutePolicyEnum value."),
            TestStep(17, "TH reads StartUpMuted attribute.", "DUT replies with either null or a bool value."),
            TestStep(18, "TH reads StartUpVolume attribute.",
                     "DUT replies with either null or a uint16 value in the range minDeviceVolume to "
                     "minOf(maxDeviceVolume, maxUserVolume)."),
            TestStep(19, "TH reads MinCorrection attribute.",
                     "DUT replies with an int16 value less than or equal to 0. Record as minCorrection."),
            TestStep(20, "TH reads MaxCorrection attribute.",
                     "DUT replies with an int16 value greater than or equal to 0. Record as maxCorrection."),
            TestStep(21, "TH reads Bass attribute.", "DUT replies with an int16 value. Record as bass."),
            TestStep(22, "TH reads Mid attribute.", "DUT replies with an int16 value. Record as mid."),
            TestStep(23, "TH reads Treble attribute.", "DUT replies with an int16 value. Record as treble."),
            TestStep(24, "Verify that bass is in the range minCorrection to maxCorrection."),
            TestStep(25, "Verify that mid is in the range minCorrection to maxCorrection."),
            TestStep(26, "Verify that treble is in the range minCorrection to maxCorrection."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AudioControl))
    async def test_TC_AUDIOCONTROL_2_1(self):
        cluster = Clusters.AudioControl
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()

        self.step(1)
        await self.read_audiocontrol_capabilities(endpoint)

        self.step(2)
        soft_muted = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.SoftMuted)
        matter_asserts.assert_valid_bool(soft_muted, "SoftMuted")

        self.step(3)
        if self.supports_attribute(attributes.PhysicallyMuted):
            physically_muted = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.PhysicallyMuted)
            matter_asserts.assert_valid_bool(physically_muted, "PhysicallyMuted")
        else:
            self.mark_current_step_skipped()

        self.step(4)
        volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.Volume)
        matter_asserts.assert_valid_uint16(volume, "Volume")

        self.step(5)
        min_device_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MinDeviceVolume)
        matter_asserts.assert_valid_uint16(min_device_volume, "MinDeviceVolume")
        asserts.assert_greater_equal(min_device_volume, 1, "MinDeviceVolume must be at least 1")

        self.step(6)
        max_device_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MaxDeviceVolume)
        matter_asserts.assert_valid_uint16(max_device_volume, "MaxDeviceVolume")
        asserts.assert_greater_equal(max_device_volume, min_device_volume,
                                     "MaxDeviceVolume must be at least MinDeviceVolume")

        self.step(7)
        asserts.assert_greater_equal(volume, min_device_volume, "Volume must be at least MinDeviceVolume")

        self.step(8)
        # Only meaningful when MaxUserVolume is absent; step 11 covers the ceiling otherwise.
        if not self.supports_attribute(attributes.MaxUserVolume):
            asserts.assert_less_equal(volume, max_device_volume, "Volume must not exceed MaxDeviceVolume")
        else:
            self.mark_current_step_skipped()

        self.step(9)
        if self.supports_attribute(attributes.MaxDeviceVolumeDB):
            max_device_volume_db = await self.read_audiocontrol_attribute_expect_success(
                endpoint, attributes.MaxDeviceVolumeDB)
            matter_asserts.assert_valid_uint16(max_device_volume_db, "MaxDeviceVolumeDB")
        else:
            self.mark_current_step_skipped()

        self.step(10)
        max_user_volume = None
        if self.supports_attribute(attributes.MaxUserVolume):
            max_user_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MaxUserVolume)
            matter_asserts.assert_valid_uint16(max_user_volume, "MaxUserVolume")
            asserts.assert_greater_equal(max_user_volume, min_device_volume,
                                         "MaxUserVolume must be at least MinDeviceVolume")
            asserts.assert_less_equal(max_user_volume, max_device_volume,
                                      "MaxUserVolume must not exceed MaxDeviceVolume")
        else:
            self.mark_current_step_skipped()

        self.step(11)
        if max_user_volume is not None:
            asserts.assert_less_equal(volume, max_user_volume, "Volume must not exceed MaxUserVolume")
        else:
            self.mark_current_step_skipped()

        effective_max = min(max_device_volume, max_user_volume) if max_user_volume is not None else max_device_volume

        self.step(12)
        default_step_size = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.DefaultStepSize)
        matter_asserts.assert_valid_uint16(default_step_size, "DefaultStepSize")
        step_size_ceiling = effective_max - min_device_volume
        if step_size_ceiling >= 1:
            matter_asserts.assert_int_in_range(default_step_size, 1, step_size_ceiling, "DefaultStepSize")
        else:
            # The DUT exposes a single usable volume level, so the spec range collapses to
            # an empty interval. Only the lower bound of the uint16 constraint is checkable.
            log.info("Volume range collapses to a single level; checking DefaultStepSize lower bound only")
            asserts.assert_greater_equal(default_step_size, 1, "DefaultStepSize must be at least 1")

        self.step(13)
        set_volume_unmute_policy = await self.read_audiocontrol_attribute_expect_success(
            endpoint, attributes.SetVolumeUnmutePolicy)
        matter_asserts.assert_valid_enum(set_volume_unmute_policy, "SetVolumeUnmutePolicy",
                                         cluster.Enums.UnmutePolicyEnum)

        self.step(14)
        increase_unmute_policy = await self.read_audiocontrol_attribute_expect_success(
            endpoint, attributes.IncreaseVolumeUnmutePolicy)
        matter_asserts.assert_valid_enum(increase_unmute_policy, "IncreaseVolumeUnmutePolicy",
                                         cluster.Enums.UnmutePolicyEnum)

        self.step(15)
        increase_unmute_volume = await self.read_audiocontrol_attribute_expect_success(
            endpoint, attributes.IncreaseVolumeUnmuteVolume)
        matter_asserts.assert_valid_enum(increase_unmute_volume, "IncreaseVolumeUnmuteVolume",
                                         cluster.Enums.UnmuteVolumeEnum)

        self.step(16)
        decrease_unmute_policy = await self.read_audiocontrol_attribute_expect_success(
            endpoint, attributes.DecreaseVolumeUnmutePolicy)
        matter_asserts.assert_valid_enum(decrease_unmute_policy, "DecreaseVolumeUnmutePolicy",
                                         cluster.Enums.UnmutePolicyEnum)

        self.step(17)
        if self.supports_attribute(attributes.StartUpMuted):
            start_up_muted = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.StartUpMuted)
            if start_up_muted is not NullValue:
                matter_asserts.assert_valid_bool(start_up_muted, "StartUpMuted")
        else:
            self.mark_current_step_skipped()

        self.step(18)
        if self.supports_attribute(attributes.StartUpVolume):
            start_up_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.StartUpVolume)
            if start_up_volume is not NullValue:
                matter_asserts.assert_valid_uint16(start_up_volume, "StartUpVolume")
                matter_asserts.assert_int_in_range(start_up_volume, min_device_volume, effective_max, "StartUpVolume")
        else:
            self.mark_current_step_skipped()

        self.step(19)
        min_correction = None
        if self.supports_attribute(attributes.MinCorrection):
            min_correction = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MinCorrection)
            matter_asserts.assert_valid_int16(min_correction, "MinCorrection")
            asserts.assert_less_equal(min_correction, 0, "MinCorrection must not be greater than 0")
        else:
            self.mark_current_step_skipped()

        self.step(20)
        max_correction = None
        if self.supports_attribute(attributes.MaxCorrection):
            max_correction = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MaxCorrection)
            matter_asserts.assert_valid_int16(max_correction, "MaxCorrection")
            asserts.assert_greater_equal(max_correction, 0, "MaxCorrection must not be less than 0")
        else:
            self.mark_current_step_skipped()

        # Steps 21-23 read each tone control; steps 24-26 range-check the same values.
        tone_controls = [
            _ToneControl(name="Bass", attribute=attributes.Bass, read_step=21, range_step=24),
            _ToneControl(name="Mid", attribute=attributes.Mid, read_step=22, range_step=25),
            _ToneControl(name="Treble", attribute=attributes.Treble, read_step=23, range_step=26),
        ]

        for tone_control in tone_controls:
            self.step(tone_control.read_step)
            if self.supports_attribute(tone_control.attribute):
                tone_control.value = await self.read_audiocontrol_attribute_expect_success(
                    endpoint, tone_control.attribute)
                matter_asserts.assert_valid_int16(tone_control.value, tone_control.name)
            else:
                self.mark_current_step_skipped()

        for tone_control in tone_controls:
            self.step(tone_control.range_step)
            if tone_control.value is not None and min_correction is not None and max_correction is not None:
                matter_asserts.assert_int_in_range(
                    tone_control.value, min_correction, max_correction, tone_control.name)
            else:
                self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
