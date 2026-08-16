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
from matter.interaction_model import Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AUDIOCONTROL_2_5(MatterBaseTest, AUDIOCONTROLTestBase):

    def desc_TC_AUDIOCONTROL_2_5(self) -> str:
        return "[TC-AUDIOCONTROL-2.5] IncreaseVolume and DecreaseVolume commands with DUT as Server"

    def pics_TC_AUDIOCONTROL_2_5(self) -> list[str]:
        return ["AUDIOCONTROL.S", "AUDIOCONTROL.S.C04.Rsp", "AUDIOCONTROL.S.C05.Rsp"]

    def steps_TC_AUDIOCONTROL_2_5(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "TH reads MinDeviceVolume attribute.", "Record as minDeviceVolume."),
            TestStep("2b", "TH reads MaxDeviceVolume attribute.", "Record as maxDeviceVolume."),
            TestStep("2c", "TH reads MaxUserVolume attribute.",
                     "Record as maxUserVolume, or fall back to maxDeviceVolume when unsupported."),
            TestStep("2d", "TH reads DefaultStepSize attribute.", "Record as defaultStepSize."),
            TestStep("2e", "TH reads IncreaseVolumeUnmutePolicy attribute.",
                     "Record as increaseVolumeUnmutePolicy for restoration in the final steps."),
            TestStep("2f", "TH reads IncreaseVolumeUnmuteVolume attribute.",
                     "Record as increaseVolumeUnmuteVolume for restoration in the final steps."),
            TestStep("2g", "TH reads DecreaseVolumeUnmutePolicy attribute.",
                     "Record as decreaseVolumeUnmutePolicy for restoration in the final steps."),
            TestStep(3, "TH sends an Unmute command to the DUT.", "DUT replies with a SUCCESS status."),
            TestStep(4, "TH sends a SetVolume command with NewVolume set to minDeviceVolume.",
                     "DUT replies with a SUCCESS status."),
            TestStep(5, "TH reads Volume attribute.", "DUT replies with minDeviceVolume."),
            TestStep(6, "TH sends an IncreaseVolume command with StepSize set to 1.",
                     "DUT replies with a SUCCESS status."),
            TestStep(7, "TH reads Volume attribute.", "DUT replies with minDeviceVolume + 1."),
            TestStep(8, "TH sends an IncreaseVolume command with StepSize omitted (default step size applies).",
                     "DUT replies with a SUCCESS status."),
            TestStep(9, "TH reads Volume attribute.", "DUT replies with minDeviceVolume + 1 + defaultStepSize."),
            TestStep(10, "TH sends a SetVolume command with NewVolume set to maxUserVolume.",
                     "DUT replies with a SUCCESS status."),
            TestStep(11, "TH sends an IncreaseVolume command with StepSize set to 1.",
                     "DUT replies with a SUCCESS status."),
            TestStep(12, "TH reads Volume attribute.", "DUT replies with maxUserVolume, clamped at the ceiling."),
            TestStep(13, "TH sends a SetVolume command with NewVolume set to minDeviceVolume + 1.",
                     "DUT replies with a SUCCESS status."),
            TestStep(14, "TH sends a DecreaseVolume command with StepSize set to 1.",
                     "DUT replies with a SUCCESS status."),
            TestStep(15, "TH reads Volume attribute.", "DUT replies with minDeviceVolume."),
            TestStep(16, "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            TestStep(17, "TH sends a DecreaseVolume command with StepSize omitted (default step size applies).",
                     "DUT replies with a SUCCESS status."),
            TestStep(18, "TH reads Volume attribute.", "DUT replies with minDeviceVolume, clamped at the floor."),
            TestStep(19, "TH reads SoftMuted attribute.", "DUT replies with TRUE."),
            TestStep(20, "TH writes IncreaseVolumeUnmutePolicy with UnmuteOrChangeVolume (0).",
                     "Verify the write request was successful."),
            TestStep(21, "TH sends a SetVolume command with NewVolume set to minDeviceVolume.",
                     "DUT replies with a SUCCESS status."),
            TestStep(22, "TH sends a Mute command to the DUT.", "DUT replies with a SUCCESS status."),
            TestStep(23, "TH sends an IncreaseVolume command with StepSize set to 1.",
                     "DUT replies with a SUCCESS status."),
            TestStep(24, "TH reads Volume attribute.", "DUT replies with minDeviceVolume + 1."),
            TestStep(25, "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            TestStep(26, "TH writes IncreaseVolumeUnmutePolicy with DoNotUnmuteAndChangeVolume (2).",
                     "Verify the write request was successful."),
            TestStep(27, "TH sends a Mute command to the DUT.", "DUT replies with a SUCCESS status."),
            TestStep(28, "TH sends an IncreaseVolume command with StepSize set to 1.",
                     "DUT replies with a SUCCESS status."),
            TestStep(29, "TH reads Volume attribute.", "DUT replies with minDeviceVolume + 2."),
            TestStep(30, "TH reads SoftMuted attribute.", "DUT replies with TRUE."),
            TestStep(31, "TH writes IncreaseVolumeUnmutePolicy with DoNotUnmuteAndDoNotChangeVolume (3).",
                     "Verify the write request was successful."),
            TestStep(32, "TH reads Volume attribute.", "Record as volumeBeforeIncrease."),
            TestStep(33, "TH sends an IncreaseVolume command with StepSize set to 1.",
                     "DUT replies with an INVALID_IN_STATE status."),
            TestStep(34, "TH reads Volume attribute.", "DUT replies with volumeBeforeIncrease."),
            TestStep(35, "TH reads SoftMuted attribute.", "DUT replies with TRUE."),
            TestStep(36, "TH writes IncreaseVolumeUnmutePolicy with DoNotUnmuteAndDoNotChangeVolume (3).",
                     "Verify the write request was successful."),
            TestStep(37, "TH sends an IncreaseVolume command with StepSize set to 1 and the UnmutePolicy field set "
                     "to UnmuteOrChangeVolume (0).", "DUT replies with a SUCCESS status."),
            TestStep(38, "TH reads Volume attribute.", "DUT replies with volumeBeforeIncrease + 1."),
            TestStep(39, "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            TestStep(40, "TH writes DecreaseVolumeUnmutePolicy with DoNotUnmuteAndChangeVolume (2).",
                     "Verify the write request was successful."),
            TestStep(41, "TH sends a SetVolume command with NewVolume set to minDeviceVolume + 2.",
                     "DUT replies with a SUCCESS status."),
            TestStep(42, "TH sends a Mute command to the DUT.", "DUT replies with a SUCCESS status."),
            TestStep(43, "TH sends a DecreaseVolume command with StepSize set to 1.",
                     "DUT replies with a SUCCESS status."),
            TestStep(44, "TH reads Volume attribute.", "DUT replies with minDeviceVolume + 1."),
            TestStep(45, "TH reads SoftMuted attribute.", "DUT replies with TRUE."),
            TestStep(46, "TH writes DecreaseVolumeUnmutePolicy with DoNotUnmuteAndDoNotChangeVolume (3).",
                     "Verify the write request was successful."),
            TestStep(47, "TH reads Volume attribute.", "Record as volumeBeforeDecrease."),
            TestStep(48, "TH sends a DecreaseVolume command with StepSize set to 1.",
                     "DUT replies with an INVALID_IN_STATE status."),
            TestStep(49, "TH reads Volume attribute.", "DUT replies with volumeBeforeDecrease."),
            TestStep(50, "TH reads SoftMuted attribute.", "DUT replies with TRUE."),
            TestStep(51, "TH writes DecreaseVolumeUnmutePolicy with DoNotUnmuteAndDoNotChangeVolume (3).",
                     "Verify the write request was successful."),
            TestStep(52, "TH sends a DecreaseVolume command with StepSize set to 1 and the UnmutePolicy field set "
                     "to UnmuteOrChangeVolume (0).", "DUT replies with a SUCCESS status."),
            TestStep(53, "TH reads Volume attribute.", "DUT replies with volumeBeforeDecrease - 1."),
            TestStep(54, "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            TestStep(55, "TH writes IncreaseVolumeUnmutePolicy with the recorded value.",
                     "Verify the write request was successful."),
            TestStep(56, "TH writes IncreaseVolumeUnmuteVolume with the recorded value.",
                     "Verify the write request was successful."),
            TestStep(57, "TH writes DecreaseVolumeUnmutePolicy with the recorded value.",
                     "Verify the write request was successful."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AudioControl))
    async def test_TC_AUDIOCONTROL_2_5(self):
        cluster = Clusters.AudioControl
        attributes = cluster.Attributes
        policies = cluster.Enums.UnmutePolicyEnum
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
        effective_max = min(max_device_volume, max_user_volume)

        self.step("2d")
        default_step_size = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.DefaultStepSize)

        self.step("2e")
        increase_unmute_policy = await self.read_audiocontrol_attribute_expect_success(
            endpoint, attributes.IncreaseVolumeUnmutePolicy)

        self.step("2f")
        increase_unmute_volume = await self.read_audiocontrol_attribute_expect_success(
            endpoint, attributes.IncreaseVolumeUnmuteVolume)

        self.step("2g")
        decrease_unmute_policy = await self.read_audiocontrol_attribute_expect_success(
            endpoint, attributes.DecreaseVolumeUnmutePolicy)

        # Several steps step the volume up to two levels above the floor. A DUT whose usable
        # range is narrower than that cannot demonstrate the behaviour under test.
        headroom = effective_max - min_device_volume
        if headroom < 2:
            log.info("Usable volume range is %d level(s); the stepping steps require at least 2", headroom)
            self.mark_all_remaining_steps_skipped(3)
            return

        self.step(3)
        await self.send_unmute(endpoint)

        self.step(4)
        await self.send_set_volume(endpoint, min_device_volume)

        self.step(5)
        await self._verify_volume_or_skip(endpoint, min_device_volume)

        self.step(6)
        await self.send_increase_volume(endpoint, step_size=1)

        self.step(7)
        await self._verify_volume_or_skip(endpoint, min_device_volume + 1)

        self.step(8)
        await self.send_increase_volume(endpoint)

        self.step(9)
        # The DUT clamps at the ceiling, so a large DefaultStepSize caps the expected result.
        expected_after_default_step = min(min_device_volume + 1 + default_step_size, effective_max)
        if expected_after_default_step != min_device_volume + 1 + default_step_size:
            log.info("DefaultStepSize overshoots the ceiling; expecting the clamped value %d",
                     expected_after_default_step)
        await self._verify_volume_or_skip(endpoint, expected_after_default_step)

        self.step(10)
        await self.send_set_volume(endpoint, effective_max)

        self.step(11)
        await self.send_increase_volume(endpoint, step_size=1)

        self.step(12)
        await self._verify_volume_or_skip(endpoint, effective_max)

        self.step(13)
        await self.send_set_volume(endpoint, min_device_volume + 1)

        self.step(14)
        await self.send_decrease_volume(endpoint, step_size=1)

        self.step(15)
        await self._verify_volume_or_skip(endpoint, min_device_volume)

        self.step(16)
        await self._verify_soft_muted_or_skip(endpoint, False)

        self.step(17)
        await self.send_decrease_volume(endpoint)

        self.step(18)
        await self._verify_volume_or_skip(endpoint, min_device_volume)

        self.step(19)
        # Decreasing while already at the floor mutes the DUT rather than going below it.
        await self._verify_soft_muted_or_skip(endpoint, True)

        self.step(20)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.IncreaseVolumeUnmutePolicy, policies.kUnmuteOrChangeVolume)

        self.step(21)
        await self.send_set_volume(endpoint, min_device_volume)

        self.step(22)
        await self.send_mute(endpoint)

        self.step(23)
        await self.send_increase_volume(endpoint, step_size=1)

        self.step(24)
        await self._verify_volume_or_skip(endpoint, min_device_volume + 1)

        self.step(25)
        await self._verify_soft_muted_or_skip(endpoint, False)

        self.step(26)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.IncreaseVolumeUnmutePolicy, policies.kDoNotUnmuteAndChangeVolume)

        self.step(27)
        await self.send_mute(endpoint)

        self.step(28)
        await self.send_increase_volume(endpoint, step_size=1)

        self.step(29)
        await self._verify_volume_or_skip(endpoint, min_device_volume + 2)

        self.step(30)
        await self._verify_soft_muted_or_skip(endpoint, True)

        self.step(31)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.IncreaseVolumeUnmutePolicy, policies.kDoNotUnmuteAndDoNotChangeVolume)

        self.step(32)
        volume_before_increase = None
        if self.supports_attribute(attributes.Volume):
            volume_before_increase = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.Volume)
        else:
            self.mark_current_step_skipped()

        self.step(33)
        await self.send_increase_volume(endpoint, step_size=1, expected_status=Status.InvalidInState)

        self.step(34)
        if volume_before_increase is not None:
            current_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.Volume)
            asserts.assert_equal(current_volume, volume_before_increase,
                                 "Volume must be unchanged after a rejected IncreaseVolume")
        else:
            self.mark_current_step_skipped()

        self.step(35)
        await self._verify_soft_muted_or_skip(endpoint, True)

        self.step(36)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.IncreaseVolumeUnmutePolicy, policies.kDoNotUnmuteAndDoNotChangeVolume)

        self.step(37)
        # The per-command UnmutePolicy field overrides the attribute set in step 36.
        await self.send_increase_volume(endpoint, step_size=1, unmute_policy=policies.kUnmuteOrChangeVolume)

        self.step(38)
        if volume_before_increase is not None:
            await self._verify_volume_or_skip(endpoint, volume_before_increase + 1)
        else:
            self.mark_current_step_skipped()

        self.step(39)
        await self._verify_soft_muted_or_skip(endpoint, False)

        self.step(40)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.DecreaseVolumeUnmutePolicy, policies.kDoNotUnmuteAndChangeVolume)

        self.step(41)
        await self.send_set_volume(endpoint, min_device_volume + 2)

        self.step(42)
        await self.send_mute(endpoint)

        self.step(43)
        await self.send_decrease_volume(endpoint, step_size=1)

        self.step(44)
        await self._verify_volume_or_skip(endpoint, min_device_volume + 1)

        self.step(45)
        await self._verify_soft_muted_or_skip(endpoint, True)

        self.step(46)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.DecreaseVolumeUnmutePolicy, policies.kDoNotUnmuteAndDoNotChangeVolume)

        self.step(47)
        volume_before_decrease = None
        if self.supports_attribute(attributes.Volume):
            volume_before_decrease = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.Volume)
        else:
            self.mark_current_step_skipped()

        self.step(48)
        await self.send_decrease_volume(endpoint, step_size=1, expected_status=Status.InvalidInState)

        self.step(49)
        if volume_before_decrease is not None:
            current_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.Volume)
            asserts.assert_equal(current_volume, volume_before_decrease,
                                 "Volume must be unchanged after a rejected DecreaseVolume")
        else:
            self.mark_current_step_skipped()

        self.step(50)
        await self._verify_soft_muted_or_skip(endpoint, True)

        self.step(51)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.DecreaseVolumeUnmutePolicy, policies.kDoNotUnmuteAndDoNotChangeVolume)

        self.step(52)
        await self.send_decrease_volume(endpoint, step_size=1, unmute_policy=policies.kUnmuteOrChangeVolume)

        self.step(53)
        if volume_before_decrease is not None:
            await self._verify_volume_or_skip(endpoint, volume_before_decrease - 1)
        else:
            self.mark_current_step_skipped()

        self.step(54)
        await self._verify_soft_muted_or_skip(endpoint, False)

        self.step(55)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.IncreaseVolumeUnmutePolicy, increase_unmute_policy)

        self.step(56)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.IncreaseVolumeUnmuteVolume, increase_unmute_volume)

        self.step(57)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.DecreaseVolumeUnmutePolicy, decrease_unmute_policy)

    async def _verify_soft_muted_or_skip(self, endpoint, expected: bool):
        if self.supports_attribute(Clusters.AudioControl.Attributes.SoftMuted):
            await self.verify_soft_muted(endpoint, expected)
        else:
            self.mark_current_step_skipped()

    async def _verify_volume_or_skip(self, endpoint, expected: int):
        if self.supports_attribute(Clusters.AudioControl.Attributes.Volume):
            await self.verify_volume(endpoint, expected)
        else:
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()
