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

from TC_AUDIOCONTROLTestBase import AUDIOCONTROLTestBase

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.decorators import has_attribute, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AUDIOCONTROL_2_6(MatterBaseTest, AUDIOCONTROLTestBase):

    # This test reboots the DUT three times (steps 5, 10, 16). Disable the background
    # wildcard subscription so reads after each reboot are not cross-checked against a
    # now-stale subscription cache.
    disable_wildcard_subscription = True
    
    def desc_TC_AUDIOCONTROL_2_6(self) -> str:
        return "[TC-AUDIOCONTROL-2.6] Startup behavior with DUT as Server"

    def pics_TC_AUDIOCONTROL_2_6(self) -> list[str]:
        return ["AUDIOCONTROL.S", "AUDIOCONTROL.S.A000c", "AUDIOCONTROL.S.A000d"]

    def steps_TC_AUDIOCONTROL_2_6(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep("2a", "TH reads MinDeviceVolume attribute.", "Record as minDeviceVolume."),
            TestStep("2b", "TH reads MaxDeviceVolume attribute.", "Record as maxDeviceVolume."),
            TestStep("2c", "TH reads MaxUserVolume attribute.",
                     "Record as maxUserVolume, or fall back to maxDeviceVolume when unsupported."),
            TestStep("2d", "TH reads StartUpMuted attribute.", "Record as startUpMutedOld."),
            TestStep("2e", "TH reads StartUpVolume attribute.", "Record as startUpVolumeOld."),
            TestStep(3, "TH writes StartUpMuted with FALSE.", "Verify the write request was successful."),
            TestStep(4, "TH writes StartUpVolume with minDeviceVolume.", "Verify the write request was successful."),
            TestStep(5, "TH triggers a reboot of the DUT and waits for it to reconnect to the fabric."),
            TestStep(6, "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            TestStep(7, "TH reads Volume attribute.", "DUT replies with minDeviceVolume."),
            TestStep(8, "TH writes StartUpMuted with TRUE.", "Verify the write request was successful."),
            TestStep(9, "TH writes StartUpVolume with minOf(maxDeviceVolume, maxUserVolume).",
                     "Verify the write request was successful."),
            TestStep(10, "TH triggers a reboot of the DUT and waits for it to reconnect to the fabric."),
            TestStep(11, "TH reads SoftMuted attribute.", "DUT replies with TRUE."),
            TestStep(12, "TH reads Volume attribute.", "DUT replies with minOf(maxDeviceVolume, maxUserVolume)."),
            TestStep(13, "TH sends an Unmute command and sets Volume to minDeviceVolume via SetVolume, to establish "
                     "a known pre-reboot state.", "Verify both commands return SUCCESS status responses."),
            TestStep(14, "TH writes StartUpMuted with null.", "Verify the write request was successful."),
            TestStep(15, "TH writes StartUpVolume with null.", "Verify the write request was successful."),
            TestStep(16, "TH triggers a reboot of the DUT and waits for it to reconnect to the fabric."),
            TestStep(17, "TH reads SoftMuted attribute.",
                     "DUT replies with FALSE, the pre-reboot value retained across the restart."),
            TestStep(18, "TH reads Volume attribute.",
                     "DUT replies with minDeviceVolume, the pre-reboot value retained across the restart."),
            TestStep(19, "TH writes StartUpMuted with the value startUpMutedOld.",
                     "Verify the write request was successful."),
            TestStep(20, "TH writes StartUpVolume with the value startUpVolumeOld.",
                     "Verify the write request was successful."),
        ]

    @run_if_endpoint_matches(has_attribute(Clusters.AudioControl.Attributes.StartUpMuted))
    async def test_TC_AUDIOCONTROL_2_6(self):
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
        if self.supports_attribute(attributes.MaxUserVolume):
            max_user_volume = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.MaxUserVolume)
        else:
            max_user_volume = max_device_volume
            self.mark_current_step_skipped()
        effective_max = min(max_device_volume, max_user_volume)

        supports_start_up_volume = self.supports_attribute(attributes.StartUpVolume)

        self.step("2d")
        start_up_muted_old = await self.read_audiocontrol_attribute_expect_success(endpoint, attributes.StartUpMuted)

        self.step("2e")
        start_up_volume_old = None
        if supports_start_up_volume:
            start_up_volume_old = await self.read_audiocontrol_attribute_expect_success(
                endpoint, attributes.StartUpVolume)
        else:
            self.mark_current_step_skipped()

        # First reboot: StartUpMuted FALSE, StartUpVolume at the floor
        self.step(3)
        await self.write_audiocontrol_attribute_expect_success(endpoint, attributes.StartUpMuted, False)

        self.step(4)
        if supports_start_up_volume:
            await self.write_audiocontrol_attribute_expect_success(
                endpoint, attributes.StartUpVolume, min_device_volume)
        else:
            self.mark_current_step_skipped()

        self.step(5)
        await self.request_device_reboot()

        self.step(6)
        await self._verify_soft_muted_or_skip(endpoint, False)

        self.step(7)
        if supports_start_up_volume:
            await self._verify_volume_or_skip(endpoint, min_device_volume)
        else:
            self.mark_current_step_skipped()

        # Second reboot: StartUpMuted TRUE, StartUpVolume at the ceiling
        self.step(8)
        await self.write_audiocontrol_attribute_expect_success(endpoint, attributes.StartUpMuted, True)

        self.step(9)
        if supports_start_up_volume:
            await self.write_audiocontrol_attribute_expect_success(endpoint, attributes.StartUpVolume, effective_max)
        else:
            self.mark_current_step_skipped()

        self.step(10)
        await self.request_device_reboot()

        self.step(11)
        await self._verify_soft_muted_or_skip(endpoint, True)

        self.step(12)
        if supports_start_up_volume:
            await self._verify_volume_or_skip(endpoint, effective_max)
        else:
            self.mark_current_step_skipped()

        # Third reboot: both null, so the pre-reboot state must be retained
        self.step(13)
        await self.send_unmute(endpoint)
        await self.send_set_volume(endpoint, min_device_volume)

        self.step(14)
        await self.write_audiocontrol_attribute_expect_success(endpoint, attributes.StartUpMuted, NullValue)

        self.step(15)
        if supports_start_up_volume:
            await self.write_audiocontrol_attribute_expect_success(endpoint, attributes.StartUpVolume, NullValue)
        else:
            self.mark_current_step_skipped()

        self.step(16)
        await self.request_device_reboot()

        self.step(17)
        await self._verify_soft_muted_or_skip(endpoint, False)

        self.step(18)
        await self._verify_volume_or_skip(endpoint, min_device_volume)

        self.step(19)
        await self.write_audiocontrol_attribute_expect_success(endpoint, attributes.StartUpMuted, start_up_muted_old)

        self.step(20)
        if supports_start_up_volume:
            await self.write_audiocontrol_attribute_expect_success(
                endpoint, attributes.StartUpVolume, start_up_volume_old)
        else:
            self.mark_current_step_skipped()

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
