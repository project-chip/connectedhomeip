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

_UINT16_MAX = 0xFFFF


class TC_AUDIOCONTROL_2_4(MatterBaseTest, AUDIOCONTROLTestBase):

    def desc_TC_AUDIOCONTROL_2_4(self) -> str:
        return "[TC-AUDIOCONTROL-2.4] SetVolume command with DUT as Server"

    def pics_TC_AUDIOCONTROL_2_4(self) -> list[str]:
        return ["AUDIOCONTROL.S", "AUDIOCONTROL.S.C03.Rsp"]

    def steps_TC_AUDIOCONTROL_2_4(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "TH reads MinDeviceVolume attribute.", "Record as minDeviceVolume."),
            TestStep("2b", "TH reads MaxDeviceVolume attribute.", "Record as maxDeviceVolume."),
            TestStep("2c", "TH reads MaxUserVolume attribute.",
                     "Record as maxUserVolume, or fall back to maxDeviceVolume when unsupported."),
            TestStep("2d", "TH reads SetVolumeUnmutePolicy attribute.", "Record as setVolumeUnmutePolicy."),
            TestStep(3, "TH sends a SetVolume command with NewVolume outside the range 0 to "
                     "minOf(maxDeviceVolume, maxUserVolume).", "Verify DUT responds with a CONSTRAINT_ERROR status."),
            TestStep(4, "TH sends an Unmute command to the DUT.", "Verify DUT responds with a SUCCESS status."),
            TestStep(5, "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            TestStep(6, "TH sends a SetVolume command with NewVolume set to 0.",
                     "Verify DUT responds with a SUCCESS status."),
            TestStep(7, "TH reads SoftMuted attribute.", "DUT replies with TRUE."),
            TestStep(8, "TH sends an Unmute command to the DUT.", "Verify DUT responds with a SUCCESS status."),
            TestStep(9, "TH sends a SetVolume command with NewVolume set to minDeviceVolume.",
                     "Verify DUT responds with a SUCCESS status."),
            TestStep(10, "TH reads Volume attribute.", "DUT replies with minDeviceVolume."),
            TestStep(11, "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            TestStep(12, "TH sends a SetVolume command with NewVolume set to minOf(maxDeviceVolume, maxUserVolume).",
                     "Verify DUT responds with a SUCCESS status."),
            TestStep(13, "TH reads Volume attribute.", "DUT replies with minOf(maxDeviceVolume, maxUserVolume)."),
            TestStep(14, "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            TestStep(15, "TH writes SetVolumeUnmutePolicy with UnmuteOrChangeVolume (0).",
                     "Verify the write request was successful."),
            TestStep(16, "TH sends a Mute command to the DUT.", "Verify DUT responds with a SUCCESS status."),
            TestStep(17, "TH sends a SetVolume command with NewVolume set to minDeviceVolume.",
                     "Verify DUT responds with a SUCCESS status."),
            TestStep(18, "TH reads Volume attribute.", "DUT replies with minDeviceVolume."),
            TestStep(19, "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            TestStep(20, "TH writes SetVolumeUnmutePolicy with UnmuteOrDoNotChangeVolume (1).",
                     "Verify the write request was successful."),
            TestStep(21, "TH sends a Mute command to the DUT.", "Verify DUT responds with a SUCCESS status."),
            TestStep(22, "TH sends a SetVolume command with NewVolume set to minDeviceVolume.",
                     "Verify DUT responds with a SUCCESS status."),
            TestStep(23, "TH reads Volume attribute.", "DUT replies with minDeviceVolume."),
            TestStep(24, "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            # The test plan has no step 25; its numbering is preserved here for traceability.
            TestStep(26, "TH writes SetVolumeUnmutePolicy with DoNotUnmuteAndChangeVolume (2).",
                     "Verify the write request was successful."),
            TestStep(27, "TH sends a Mute command to the DUT.", "Verify DUT responds with a SUCCESS status."),
            TestStep(28, "TH sends a SetVolume command with NewVolume set to minDeviceVolume.",
                     "Verify DUT responds with a SUCCESS status."),
            TestStep(29, "TH reads Volume attribute.", "DUT replies with minDeviceVolume."),
            TestStep(30, "TH reads SoftMuted attribute.", "DUT replies with TRUE."),
            TestStep(31, "TH writes SetVolumeUnmutePolicy with DoNotUnmuteAndDoNotChangeVolume (3).",
                     "Verify the write request was successful."),
            TestStep(32, "TH reads Volume attribute.", "Record as volumeBeforeSet."),
            TestStep(33, "TH sends a SetVolume command with NewVolume set to minOf(maxDeviceVolume, maxUserVolume).",
                     "Verify DUT responds with an INVALID_IN_STATE status."),
            TestStep(34, "TH reads Volume attribute.", "DUT replies with volumeBeforeSet."),
            TestStep(35, "TH reads SoftMuted attribute.", "DUT replies with TRUE."),
            TestStep(36, "TH writes SetVolumeUnmutePolicy with DoNotUnmuteAndDoNotChangeVolume (3).",
                     "Verify the write request was successful."),
            TestStep(37, "TH sends a SetVolume command with NewVolume set to minDeviceVolume and the UnmutePolicy "
                     "field set to UnmuteOrChangeVolume (0).", "Verify DUT responds with a SUCCESS status."),
            TestStep(38, "TH reads Volume attribute.", "DUT replies with minDeviceVolume."),
            TestStep(39, "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            TestStep(40, "TH writes SetVolumeUnmutePolicy with the value setVolumeUnmutePolicy.",
                     "Verify the write request was successful."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AudioControl))
    async def test_TC_AUDIOCONTROL_2_4(self):
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
        set_volume_unmute_policy = await self.read_audiocontrol_attribute_expect_success(
            endpoint, attributes.SetVolumeUnmutePolicy)

        self.step(3)
        if effective_max < _UINT16_MAX:
            await self.send_set_volume(endpoint, effective_max + 1, expected_status=Status.ConstraintError)
        else:
            # The ceiling is already the largest uint16, so no out-of-range value can be encoded.
            log.info("Effective maximum volume is 0xFFFF; no out-of-range NewVolume can be expressed")
            self.mark_current_step_skipped()

        self.step(4)
        await self.send_unmute(endpoint)

        self.step(5)
        await self._verify_soft_muted_or_skip(endpoint, False)

        self.step(6)
        # NewVolume=0 is the documented shorthand for muting the DUT.
        await self.send_set_volume(endpoint, 0)

        self.step(7)
        await self._verify_soft_muted_or_skip(endpoint, True)

        self.step(8)
        await self.send_unmute(endpoint)

        self.step(9)
        await self.send_set_volume(endpoint, min_device_volume)

        self.step(10)
        await self._verify_volume_or_skip(endpoint, min_device_volume)

        self.step(11)
        await self._verify_soft_muted_or_skip(endpoint, False)

        self.step(12)
        await self.send_set_volume(endpoint, effective_max)

        self.step(13)
        await self._verify_volume_or_skip(endpoint, effective_max)

        self.step(14)
        await self._verify_soft_muted_or_skip(endpoint, False)

        # Each policy below is written, the DUT is muted, and SetVolume is issued so that the
        # resulting Volume/SoftMuted pair demonstrates that policy's documented behaviour.
        self.step(15)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.SetVolumeUnmutePolicy, policies.kUnmuteOrChangeVolume)

        self.step(16)
        await self.send_mute(endpoint)

        self.step(17)
        await self.send_set_volume(endpoint, min_device_volume)

        self.step(18)
        await self._verify_volume_or_skip(endpoint, min_device_volume)

        self.step(19)
        await self._verify_soft_muted_or_skip(endpoint, False)

        self.step(20)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.SetVolumeUnmutePolicy, policies.kUnmuteOrDoNotChangeVolume)

        self.step(21)
        await self.send_mute(endpoint)

        self.step(22)
        await self.send_set_volume(endpoint, min_device_volume)

        self.step(23)
        await self._verify_volume_or_skip(endpoint, min_device_volume)

        self.step(24)
        await self._verify_soft_muted_or_skip(endpoint, False)

        self.step(26)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.SetVolumeUnmutePolicy, policies.kDoNotUnmuteAndChangeVolume)

        self.step(27)
        await self.send_mute(endpoint)

        self.step(28)
        await self.send_set_volume(endpoint, min_device_volume)

        self.step(29)
        # The volume changes even though the DUT stays muted.
        await self._verify_volume_or_skip(endpoint, min_device_volume)

        self.step(30)
        await self._verify_soft_muted_or_skip(endpoint, True)

        self.step(31)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.SetVolumeUnmutePolicy, policies.kDoNotUnmuteAndDoNotChangeVolume)

        self.step(32)
        volume_before_set = None
        if self.supports_attribute(attributes.Volume):
            volume_before_set = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.Volume)
        else:
            self.mark_current_step_skipped()

        self.step(33)
        # DoNotUnmuteAndDoNotChangeVolume means the DUT must reject the command outright.
        await self.send_set_volume(endpoint, effective_max, expected_status=Status.InvalidInState)

        self.step(34)
        if volume_before_set is not None:
            current_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.Volume)
            asserts.assert_equal(current_volume, volume_before_set,
                                 "Volume must be unchanged after a rejected SetVolume")
        else:
            self.mark_current_step_skipped()

        self.step(35)
        await self._verify_soft_muted_or_skip(endpoint, True)

        self.step(36)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.SetVolumeUnmutePolicy, policies.kDoNotUnmuteAndDoNotChangeVolume)

        self.step(37)
        # The per-command UnmutePolicy field overrides the attribute set in step 36.
        await self.send_set_volume(endpoint, min_device_volume, unmute_policy=policies.kUnmuteOrChangeVolume)

        self.step(38)
        await self._verify_volume_or_skip(endpoint, min_device_volume)

        self.step(39)
        await self._verify_soft_muted_or_skip(endpoint, False)

        self.step(40)
        await self.write_audiocontrol_attribute_expect_success(
            endpoint, attributes.SetVolumeUnmutePolicy, set_volume_unmute_policy)

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
