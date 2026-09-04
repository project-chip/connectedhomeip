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
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AUDIOCONTROL_2_3(MatterBaseTest, AUDIOCONTROLTestBase):

    def desc_TC_AUDIOCONTROL_2_3(self) -> str:
        return "[TC-AUDIOCONTROL-2.3] Mute, Unmute and ToggleMuted commands with DUT as Server"

    def pics_TC_AUDIOCONTROL_2_3(self) -> list[str]:
        return ["AUDIOCONTROL.S"]

    def steps_TC_AUDIOCONTROL_2_3(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH sends a Mute command to the DUT.", "Verify DUT responds with a SUCCESS status response."),
            TestStep(3, "TH reads SoftMuted attribute.", "DUT replies with TRUE."),
            TestStep(4, "TH sends an Unmute command to the DUT.", "Verify DUT responds with a SUCCESS status response."),
            TestStep(5, "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            TestStep(6, "TH sends a ToggleMuted command to the DUT.",
                     "Verify DUT responds with a SUCCESS status response."),
            TestStep(7, "TH reads SoftMuted attribute.", "DUT replies with TRUE."),
            TestStep(8, "TH sends a ToggleMuted command to the DUT.",
                     "Verify DUT responds with a SUCCESS status response."),
            TestStep(9, "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            TestStep(10, "TH sends a Mute command to the DUT.", "Verify DUT responds with a SUCCESS status response."),
            TestStep(11, "TH reads SoftMuted attribute.", "DUT replies with TRUE."),
            TestStep(12, "TH sends a Mute command to the DUT again (already muted).",
                     "Verify DUT responds with a SUCCESS status response."),
            TestStep(13, "TH reads SoftMuted attribute.", "DUT replies with TRUE."),
            TestStep(14, "TH sends an Unmute command to the DUT.",
                     "Verify DUT responds with a SUCCESS status response."),
            TestStep(15, "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
            TestStep(16, "TH sends an Unmute command to the DUT again (already unmuted).",
                     "Verify DUT responds with a SUCCESS status response."),
            TestStep(17, "TH reads SoftMuted attribute.", "DUT replies with FALSE."),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AudioControl))
    async def test_TC_AUDIOCONTROL_2_3(self):
        endpoint = self.get_endpoint()

        self.step(1)
        await self.read_audiocontrol_capabilities(endpoint)

        # (command to send, expected SoftMuted value after it)
        sequence = [
            (self.send_mute, True),
            (self.send_unmute, False),
            (self.send_toggle_muted, True),
            (self.send_toggle_muted, False),
            (self.send_mute, True),
            # Repeating Mute while already muted must still succeed and leave the state unchanged.
            (self.send_mute, True),
            (self.send_unmute, False),
            # Likewise for a redundant Unmute.
            (self.send_unmute, False),
        ]

        step_number = 2
        for send_command, expected_soft_muted in sequence:
            self.step(step_number)
            await send_command(endpoint)

            self.step(step_number + 1)
            if self.supports_attribute(Clusters.AudioControl.Attributes.SoftMuted):
                await self.verify_soft_muted(endpoint, expected_soft_muted)
            else:
                self.mark_current_step_skipped()

            step_number += 2


if __name__ == "__main__":
    default_matter_test_main()
