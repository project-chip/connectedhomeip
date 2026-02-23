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
#
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 0
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import math
import random
import secrets

from mobly import asserts
from TC_GCAST_common import get_feature_map, valid_endpoints_list

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_GCAST_2_6(MatterBaseTest):
    def desc_TC_GCAST_2_6(self):
        return "[TC-GCAST-2.6] Capacity & MaxMembershipCount enforcement with DUT as Server - PROVISIONAL"

    def steps_TC_GCAST_2_6(self):
        return [
            TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep("1b", "Commission DUT to TH2 (can be skipped if done in a preceding test)"),
            TestStep("1c", "TH removes any existing group and KeySetID on the DUT"),
            TestStep(2, "Join group G1 generating a new key. JoinGroup (GroupID=G1, Endpoints='see notes', KeySetID=K1, Key=InputKey1)"),
            TestStep(3, "Iteratively Join groups, using a new GroupId and assigning KeySetID K1 every time until group count on fabric F1 = floor(M_max/2). JoinGroup (GroupID=Gn, Endpoints='see notes', KeySetID=K1)"),
            TestStep(4, "Attempt to join 1 additional group. JoinGroup (GroupID=Gn+1, Endpoints='see notes', KeySetID=K1)"),
            TestStep(5, "Leave one group. LeaveGroup (GroupID=Gn)"),
            TestStep(6, "Repeat Step 4. JoinGroup (GroupID=Gn+1, Endpoints='see notes', KeySetID=K1)"),
            TestStep(7, "On F2, join group G1 generating a new key. TH2 sends command JoinGroup (GroupID=G1, Endpoints='see notes', KeySetID=K1, Key=InputKey1)"),
            TestStep(8, "On F2, iteratively Join groups using a new GroupId and assigning KeySetID K1 every time until the total group count combining all DUT groups on both fabrics = M_max. TH2 sends command JoinGroup (GroupID=Gi, Endpoints='see notes', KeySetID=K1)"),
            TestStep(9, "On F2, attempt to join 1 additional group. TH2 sends command JoinGroup (GroupID=Gi+1, Endpoints='see notes', KeySetID=K1)"),
        ]

    def pics_TC_GCAST_2_6(self) -> list[str]:
        return ["GCAST.S"]

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GCAST_2_6(self):
        groupcast_cluster = Clusters.Objects.Groupcast
        max_membership_count_attribute = Clusters.Groupcast.Attributes.MaxMembershipCount

        self.step("1a")
        ln_enabled, sd_enabled, pga_enabled = await get_feature_map(self)
        endpoints_list = await valid_endpoints_list(self, ln_enabled)
        if len(endpoints_list) > 1:
            endpoints_list = endpoints_list[:2]

        M_max = await self.read_single_attribute_check_success(groupcast_cluster, max_membership_count_attribute)
        asserts.assert_true(M_max >= 10, "MaxMembershipCount attribute should be >= 10")

        self.step("1b")
        self.th1 = self.default_controller
        self.discriminatorTH2 = random.randint(0, 4095)
        # Create TH2 controller
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        # Open commissioning window on TH1
        params = await self.th1.OpenCommissioningWindow(
            nodeId=self.dut_node_id,
            timeout=900,
            iteration=1000,
            discriminator=self.discriminatorTH2,
            option=1
        )

        # Commission TH2
        await self.th2.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.discriminatorTH2
        )

        self.step("1c")
        # Check if there are any groups on the DUT.
        membership = await self.read_single_attribute_check_success(groupcast_cluster, Clusters.Groupcast.Attributes.Membership)
        if membership:
            # LeaveGroup with groupID 0 will leave all groups on the fabric.
            await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=0))

        # remove any existing KeySetID on the DUT, except KeySetId 0 (IPK).
        resp: Clusters.GroupKeyManagement.Commands.KeySetReadAllIndicesResponse = await self.send_single_cmd(Clusters.GroupKeyManagement.Commands.KeySetReadAllIndices())

        read_group_key_ids: list[int] = resp.groupKeySetIDs
        for key_set_id in read_group_key_ids:
            if key_set_id != 0:
                await self.send_single_cmd(Clusters.GroupKeyManagement.Commands.KeySetRemove(key_set_id))

        self.step(2)
        groupID1 = 1
        keySetID1 = 1
        inputKey1 = secrets.token_bytes(16)

        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID1,
            endpoints=endpoints_list,
            keySetID=keySetID1,
            key=inputKey1)
        )

        self.step(3)
        f1MaxMembershipCount = math.floor(M_max / 2)
        for membership in range(1, f1MaxMembershipCount):
            groupID = membership + 1

            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID,
                endpoints=endpoints_list,
                keySetID=keySetID1)
            )

        self.step(4)
        groupIDExhausted = f1MaxMembershipCount + 1
        try:
            await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupIDExhausted,
                endpoints=endpoints_list,
                keySetID=keySetID1)
            )
            asserts.fail("JoinGroup command should have failed with ResourceExhausted, but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ResourceExhausted,
                                 f"Send JoinGroup command error should be {Status.ResourceExhausted} instead of {e.status}")

        self.step(5)
        await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=f1MaxMembershipCount))

        self.step(6)
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=f1MaxMembershipCount,
            endpoints=endpoints_list,
            keySetID=keySetID1)
        )

        self.step(7)
        await self.send_single_cmd(dev_ctrl=self.th2, cmd=Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID1,
            endpoints=endpoints_list,
            keySetID=keySetID1,
            key=inputKey1)
        )

        self.step(8)
        for membership in range(f1MaxMembershipCount + 1, M_max):
            groupID = membership + 1

            await self.send_single_cmd(dev_ctrl=self.th2, cmd=Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID,
                endpoints=endpoints_list,
                keySetID=keySetID1)
            )

        self.step(9)
        groupIDExhausted = M_max + 1
        try:
            await self.send_single_cmd(dev_ctrl=self.th2, cmd=Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupIDExhausted,
                endpoints=endpoints_list,
                keySetID=keySetID1)
            )
            asserts.fail("JoinGroup command should have failed with ResourceExhausted, but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ResourceExhausted,
                                 f"Send JoinGroup command error should be {Status.ResourceExhausted} instead of {e.status}")


if __name__ == "__main__":
    default_matter_test_main()
