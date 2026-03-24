#
#    Copyright (c) 2025 Project CHIP Authors
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
#   run2:
#     app: ${ALL_DEVICES_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --device chime
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from TC_CHIMETestBase import CHIMETestBase

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_CHIME_2_5(MatterBaseTest, CHIMETestBase):

    def desc_TC_CHIME_2_5(self) -> str:
        return "[TC-CHIME-2.5] Verify functionality of the PlayChimeSound command-Release 1.5.1 or later"

    def steps_TC_CHIME_2_5(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Set the enabled attribute to False"),
            TestStep(3, "Send the PlayChimeSound command and verify that no chime sound is heard"),
            TestStep(4, "Set the enabled attribute to True"),
            TestStep(5, "Send the PlayChimeSound with a ChimeID field different to the currently SelectedChime"),
            TestStep(6, "Verify that a chime appropriate to the provided ID is heard."),
        ]

    def pics_TC_CHIME_2_5(self) -> list[str]:
        return [
            "CHIME.S",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.Chime))
    async def test_TC_CHIME_2_5(self):
        cluster = Clusters.Objects.Chime
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        # Pre-condition, make sure the ClusterRevision is at least 2
        clusterRevision = await self.read_chime_attribute_expect_success(endpoint=endpoint, attribute=attributes.ClusterRevision)
        if clusterRevision < 2:
            log.info("Chime 2.5: skipping as cluster revision is less than 2")
            self.mark_all_remaining_steps_skipped(6)
            return

        self.step(1)  # Already done, immediately go to step 2

        self.step(2)
        await self.write_chime_attribute_expect_success(endpoint, attributes.Enabled, False)

        self.step(3)
        await self.send_play_chime_sound_command(endpoint)
        if not self.is_ci:
            user_response = self.wait_for_user_input(prompt_msg="A chime sound should not have been played, is this correct? Enter 'y' or 'n'",
                                                     prompt_msg_placeholder="y",
                                                     default_value="y")
            if user_response is not None:
                log.info(f"CHIME 2_5: response '{user_response}' received on confirmation of no chime sound")
                asserts.assert_equal(user_response.lower(), "y")
            else:
                log.info("CHIME 2_5: No response received for no chime sound played user prompt")

        self.step(4)
        await self.write_chime_attribute_expect_success(endpoint, attributes.Enabled, True)

        myChimeSounds = await self.read_chime_attribute_expect_success(endpoint, attributes.InstalledChimeSounds)
        mySelectedChime = await self.read_chime_attribute_expect_success(endpoint, attributes.SelectedChime)
        if len(myChimeSounds) > 1:
            self.step(5)

            newSelectedChime = mySelectedChime
            for chime in myChimeSounds:
                if chime.chimeID != mySelectedChime:
                    newSelectedChime = chime.chimeID
                    break

            await self.send_play_chime_sound_command(endpoint, newSelectedChime)
            if not self.is_ci:
                self.step(6)
                user_response = self.wait_for_user_input(prompt_msg="A chime sound should have just been played, is this correct? Enter 'y' or 'n'",
                                                         prompt_msg_placeholder="y",
                                                         default_value="y")
                if user_response is not None:
                    log.info(f"CHIME 2_5: response '{user_response}' received on confirmation of different chime sound")
                    asserts.assert_equal(user_response.lower(), "y")
                else:
                    log.info("CHIME 2_5: No response received for different chime sound played user prompt")
            else:
                self.skip_step(6)

        else:
            self.skip_step(5)
            self.skip_step(6)


if __name__ == "__main__":
    default_matter_test_main()
