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
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts
from TC_CHIMETestBase import CHIMETestBase


class TC_CHIME_2_4(MatterBaseTest, CHIMETestBase):

    def desc_TC_CHIME_2_4(self) -> str:
        return "[TC-CHIME-2.4] Verify functionality of the PlayChimeSound command"

    def steps_TC_CHIME_2_4(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Write the value of False to the Enabled attribute."),
            TestStep(3, "Invoke the PlayChimeSound command. Verify a success response, and no chime is played."),
            TestStep(4, "Write the value of True to the Enabled attribute."),
            TestStep(5, "Invoke the PlayChimeSound command. Verify a success response, and a chime is played."),
        ]
        return steps

    def pics_TC_CHIME_2_4(self) -> list[str]:
        pics = [
            "CHIME.S",
        ]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.Chime))
    async def test_TC_CHIME_2_4(self):
        cluster = Clusters.Objects.Chime
        attributes = cluster.Attributes
        endpoint = self.get_endpoint(default=1)
        self.is_ci = self.check_pics("PICS_SDK_CI_ONLY")

        self.step(1)  # Already done, immediately go to step 2

        self.step(2)
        await self.write_chime_attribute_expect_success(endpoint, attributes.Enabled, False)

        self.step(3)
        await self.send_play_chime_sound_command(endpoint)
        if not self.is_ci:
            user_response = self.wait_for_user_input(prompt_msg="A chime sound should not have been played, is this correct? Enter 'y' or 'n'",
                                                     prompt_msg_placeholder="y",
                                                     default_value="y")
            asserts.assert_equal(user_response.lower(), "y")

        self.step(4)
        await self.write_chime_attribute_expect_success(endpoint, attributes.Enabled, True)

        self.step(5)
        await self.send_play_chime_sound_command(endpoint)
        if not self.is_ci:
            user_response = self.wait_for_user_input(prompt_msg="A chime sound should have been played, is this correct? Enter 'y' or 'n'",
                                                     prompt_msg_placeholder="y",
                                                     default_value="y")
            asserts.assert_equal(user_response.lower(), "y")


if __name__ == "__main__":
    default_matter_test_main()
