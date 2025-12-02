#
#    Copyright (c) 2024 Project CHIP Authors
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
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${LIGHTING_APP_NO_UNIQUE_ID}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import secrets
from mobly import asserts
import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, has_cluster, run_if_endpoint_matches
from matter.interaction_model import InteractionModelError, Status

from src.python_testing.TC_GCAST_common import get_feature_map, valid_endpoints_list

logger = logging.getLogger(__name__)


class TC_GCAST_2_7(MatterBaseTest):
    def desc_TC_GCAST_2_7(self):
        return "[TC-GCAST-2.7] Capacity & MaxMembershipCount enforcement with DUT as Server - PROVISIONAL"

    def steps_TC_GCAST_2_7(self):
        return [
            TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep("1b", "TH removes any existing group and KeyID on the DUT"),
            TestStep(2, "Iteratively Join group until the Group count = M_max using a new GroupId and KeyId every time: "
                        "TH sends command JoinGroup (GroupID=Gn, Endpoints='see notes', KeyID=Kn, Key=InputKeyn) until the Group count = M_max"),
            TestStep(3, "Attempt to join 1 additional group: TH sends command JoinGroup (GroupID=Gn+1, Endpoints='see notes', KeyID=Kn+1, Key=InputKeyn+1)"),
            TestStep(4, "Leave one group: TH sends command LeaveGroup (GroupID=Gn)"),
            TestStep(5, "Repeat Step 2: TH sends command JoinGroup (GroupID=Gn+1, Endpoints='see notes', KeyID=Kn+1, Key=InputKeyn+1)"),
        ]

    def pics_TC_GCAST_2_7(self) -> list[str]:
        pics = ["GCAST.S"]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GCAST_2_7(self):
        if self.matter_test_config.endpoint is None:
            self.matter_test_config.endpoint = 0
        groupcast_cluster = Clusters.Objects.Groupcast
        max_membership_count_attribute = Clusters.Groupcast.Attributes.MaxMembershipCount

        self.step("1a")
        ln_enabled, sd_enabled = await get_feature_map(self)
        endpoints_list = await valid_endpoints_list(self, ln_enabled)
        if len(endpoints_list) > 1:
            endpoints_list = endpoints_list[:2]

        M_max = await self.read_single_attribute_check_success(groupcast_cluster, max_membership_count_attribute)
        asserts.assert_true(M_max >= 10, "MaxMembershipCount attribute should be >= 10")

        self.step("1b")
        groupID0 = 0
        await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=groupID0))

        self.step(2)
        for membership in range(M_max):
            groupID = membership + 1
            keyID = membership + 1
            inputKey = secrets.token_bytes(16)

            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID,
                endpoints=endpoints_list,
                keyID=keyID,
                key=inputKey)
            )

        self.step(3)
        groupIDExhausted = M_max + 1
        keyIDExhausted = M_max + 1
        inputKeyExhausted = secrets.token_bytes(16)
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupIDExhausted,
                endpoints=endpoints_list,
                keyID=keyIDExhausted,
                key=inputKeyExhausted)
            )
            asserts.fail("JoinGroup command should have failed with ResourceExhausted, but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ResourceExhausted,
                                 f"Send JoinGroup command error should be {Status.ResourceExhausted} instead of {e.status}")

        self.step(4)
        await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=M_max))

        self.step(5)
        groupIDLimit = M_max
        keyIDLimit = M_max
        inputKeyLimit = secrets.token_bytes(16)
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupIDLimit,
            endpoints=endpoints_list,
            keyID=keyIDLimit,
            key=inputKeyLimit)
        )


if __name__ == "__main__":
    default_matter_test_main()
