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
from TC_GCAST_common import (generate_membership_entry_matcher, generate_usedMcastAddrCount_entry_matcher, get_feature_map,
                             valid_endpoints_list)

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_GCAST_2_7(MatterBaseTest):
    def desc_TC_GCAST_2_7(self):
        return "[TC-GCAST-2.7] Multicast address policy and UsedMcastAddrCount with DUT as Server - PROVISIONAL"

    def steps_TC_GCAST_2_7(self):
        return [
            TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test). This fabric is F1", is_commissioning=True),
            TestStep("1b", "TH removes any existing group and KeySetID on the DUT"),
            TestStep("1c", "TH subscribes to Membership attribute with min interval 0s and max interval 30s"),
            TestStep("1d", "TH subscribes to UsedMcastAddrCount attribute with min interval 0s and max interval 30s"),
            TestStep(2, "TH reads MaxMcastAddrCount attribute"),
            TestStep(3, "TH reads UsedMcastAddrCount attribute"),
            TestStep("4a", "Join Group G1 with a new key and PerGroup multicast address policy. JoinGroup (GroupID=G1, Endpoints='see notes', KeySetID=K1, Key=InputKey1, McastAddrPolicy=PerGroup)"),
            TestStep("4b", "TH awaits subscription report of new Membership within max interval. (Entry for G1 shows McastAddrPolicy=PerGroup)"),
            TestStep("4c", "TH awaits subscription report of new UsedMcastAddrCount within max interval. (value == 1)"),
            TestStep("5a", "Join Group G2 with IanaAddr policy. JoinGroup (GroupID=G2, Endpoints='see notes', KeySetID=K2, Key=InputKey2, McastAddrPolicy=IanaAddr)"),
            TestStep("5b", "TH awaits subscription report of new Membership within max interval. (Entry for G2 shows McastAddrPolicy=IanaAddr)"),
            TestStep("5c", "TH awaits subscription report of new UsedMcastAddrCount within max interval. (value == 2)"),
            TestStep("6a", "If A_max < floor(M_max/2) skip to step 8"),
            TestStep("6b", "Commission a second fabric (F2) on the DUT using a second controller instance"),
            TestStep("7a", "On F2, Join a new PerGroup group generating a new key. JoinGroup (GroupID=G3, Endpoints='see notes', KeySetID=K3, Key=InputKey3, McastAddrPolicy=PerGroup). For subsequent joins, omit Key and reuse KeySetID=K3"),
            TestStep("7b", "On F2, iteratively Join PerGroup groups using a new GroupID every time until floor(M_max/2) groups have been joined. JoinGroup (GroupID=Gi, Endpoints='see notes', KeySetID=K3, McastAddrPolicy=PerGroup)"),
            TestStep(8, "On F1, iteratively Join additional PerGroup groups using a new GroupID every time until A_max groups have been joined. JoinGroup (GroupID=Gn, Endpoints='see notes', KeySetID=K1, McastAddrPolicy=PerGroup)"),
            TestStep(9, "TH awaits subscription report of new UsedMcastAddrCount within max interval. (value == A_max)"),
            TestStep(10, "Attempt to join 1 additional PerGroup group beyond A_max"),
        ]

    def pics_TC_GCAST_2_7(self) -> list[str]:
        return ["GCAST.S"]

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GCAST_2_7(self):
        groupcast_cluster = Clusters.Objects.Groupcast
        membership_attribute = Clusters.Groupcast.Attributes.Membership
        max_membership_count_attribute = Clusters.Groupcast.Attributes.MaxMembershipCount
        usedMcastAddrCount_attribute = Clusters.Groupcast.Attributes.UsedMcastAddrCount
        maxMcastAddrCount_attribute = Clusters.Groupcast.Attributes.MaxMcastAddrCount

        self.step("1a")
        ln_enabled, sd_enabled, pga_enabled = await get_feature_map(self)
        if not pga_enabled:
            logger.info("PerGroup feature is not enabled, skip remaining steps.")
            self.mark_all_remaining_steps_skipped("1b")
            return

        endpoints_list = await valid_endpoints_list(self, ln_enabled)
        if len(endpoints_list) > 1:
            endpoints_list = [endpoints_list[0]]

        self.step("1b")
        # Check if there are any groups on the DUT.
        membership = await self.read_single_attribute_check_success(groupcast_cluster, membership_attribute)
        if membership:
            # LeaveGroup with groupID 0 will leave all groups on the fabric.
            await self.send_single_cmd(Clusters.Groupcast.Commands.LeaveGroup(groupID=0))

        # remove any existing KeySetID on the DUT, except KeySetId 0 (IPK).
        resp: Clusters.GroupKeyManagement.Commands.KeySetReadAllIndicesResponse = await self.send_single_cmd(Clusters.GroupKeyManagement.Commands.KeySetReadAllIndices())

        read_group_key_ids: list[int] = resp.groupKeySetIDs
        for key_set_id in read_group_key_ids:
            if key_set_id != 0:
                await self.send_single_cmd(Clusters.GroupKeyManagement.Commands.KeySetRemove(key_set_id))

        self.step("1c")
        sub = AttributeSubscriptionHandler(groupcast_cluster, membership_attribute)
        await sub.start(self.default_controller, self.dut_node_id, self.get_endpoint(), min_interval_sec=0, max_interval_sec=30)

        self.step("1d")
        sub2 = AttributeSubscriptionHandler(groupcast_cluster, usedMcastAddrCount_attribute)
        await sub2.start(self.default_controller, self.dut_node_id, self.get_endpoint(), min_interval_sec=0, max_interval_sec=30)

        self.step(2)
        M_max = await self.read_single_attribute_check_success(groupcast_cluster, max_membership_count_attribute)
        asserts.assert_true(M_max >= 10, "MaxMembershipCount attribute should be >= 10")

        A_max = await self.read_single_attribute_check_success(groupcast_cluster, maxMcastAddrCount_attribute)
        asserts.assert_true(A_max >= 4, f"MaxMcastAddrCount attribute should be >= 4 but got {A_max} instead.")
        asserts.assert_true(A_max <= M_max, f"MaxMcastAddrCount ({A_max}) should be <= MaxMembershipCount ({M_max})")

        self.step(3)
        usedMcastAddrCount = await self.read_single_attribute_check_success(groupcast_cluster, usedMcastAddrCount_attribute)
        asserts.assert_equal(usedMcastAddrCount, 0, "UsedMcastAddrCount attribute should be 0")

        self.step("4a")
        groupID1 = 1
        keySetID1 = 1
        inputKey1 = secrets.token_bytes(16)
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID1,
            endpoints=endpoints_list,
            keySetID=keySetID1,
            key=inputKey1,
            mcastAddrPolicy=Clusters.Groupcast.Enums.MulticastAddrPolicyEnum.kPerGroup)
        )
        f1_current_group_count = 1

        self.step("4b")
        membership_matcher = generate_membership_entry_matcher(
            groupID1, mcastAddrPolicy=Clusters.Groupcast.Enums.MulticastAddrPolicyEnum.kPerGroup)
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step("4c")
        usedMcastAddrCount_matcher = generate_usedMcastAddrCount_entry_matcher(1)
        sub2.await_all_expected_report_matches(expected_matchers=[usedMcastAddrCount_matcher], timeout_sec=60)

        self.step("5a")
        groupID2 = 2
        keySetID2 = 2
        inputKey2 = secrets.token_bytes(16)
        await self.send_single_cmd(Clusters.Groupcast.Commands.JoinGroup(
            groupID=groupID2,
            endpoints=endpoints_list,
            keySetID=keySetID2,
            key=inputKey2,
            mcastAddrPolicy=Clusters.Groupcast.Enums.MulticastAddrPolicyEnum.kIanaAddr)
        )
        f1_current_group_count += 1

        self.step("5b")
        membership_matcher = generate_membership_entry_matcher(
            groupID2, mcastAddrPolicy=Clusters.Groupcast.Enums.MulticastAddrPolicyEnum.kIanaAddr)
        sub.await_all_expected_report_matches(expected_matchers=[membership_matcher], timeout_sec=60)

        self.step("5c")
        usedMcastAddrCount_matcher = generate_usedMcastAddrCount_entry_matcher(2)
        sub2.await_all_expected_report_matches(expected_matchers=[usedMcastAddrCount_matcher], timeout_sec=60)

        self.step("6a")
        if A_max < math.floor(M_max / 2):
            self.mark_step_range_skipped("6b", "8")
        else:
            self.step("6b")
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

            self.step("7a")
            groupID3 = 3
            keySetID3 = 3
            inputKey3 = secrets.token_bytes(16)
            await self.send_single_cmd(dev_ctrl=self.th2, cmd=Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupID3,
                endpoints=endpoints_list,
                keySetID=keySetID3,
                key=inputKey3,
                mcastAddrPolicy=Clusters.Groupcast.Enums.MulticastAddrPolicyEnum.kPerGroup)
            )
            f2_current_group_count = 1

            self.step("7b")
            f2_max_groups = math.floor(M_max / 2)
            for i in range(f2_current_group_count, f2_max_groups):
                groupID = i + 3
                await self.send_single_cmd(dev_ctrl=self.th2, cmd=Clusters.Groupcast.Commands.JoinGroup(
                    groupID=groupID,
                    endpoints=endpoints_list,
                    keySetID=keySetID3,
                    mcastAddrPolicy=Clusters.Groupcast.Enums.MulticastAddrPolicyEnum.kPerGroup)
                )
                f2_current_group_count += 1

            self.step(8)
            total_per_group_count = f1_current_group_count + f2_current_group_count
            for i in range(total_per_group_count, A_max):
                groupID = i + 1
                await self.send_single_cmd(cmd=Clusters.Groupcast.Commands.JoinGroup(
                    groupID=groupID,
                    endpoints=endpoints_list,
                    keySetID=keySetID1,
                    mcastAddrPolicy=Clusters.Groupcast.Enums.MulticastAddrPolicyEnum.kPerGroup)
                )
                f1_current_group_count += 1

        self.step("9")
        usedMcastAddrCount_matcher = generate_usedMcastAddrCount_entry_matcher(A_max)
        sub2.await_all_expected_report_matches(expected_matchers=[usedMcastAddrCount_matcher], timeout_sec=60)

        self.step("10")
        groupIDExhausted = A_max + 1
        try:
            await self.send_single_cmd(dev_ctrl=self.th2, cmd=Clusters.Groupcast.Commands.JoinGroup(
                groupID=groupIDExhausted,
                endpoints=endpoints_list,
                keySetID=keySetID3,
                mcastAddrPolicy=Clusters.Groupcast.Enums.MulticastAddrPolicyEnum.kPerGroup)
            )
            asserts.fail("JoinGroup command should have failed with ResourceExhausted, but it succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ResourceExhausted,
                                 f"Send JoinGroup command error should be {Status.ResourceExhausted} instead of {e.status}")


if __name__ == "__main__":
    default_matter_test_main()
