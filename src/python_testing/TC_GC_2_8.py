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
#     app: ${ALL_DEVICES_APP}
#     app-args: --device on-off-light:1 --discriminator 1234 --KVS kvs1 --groupcast
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

import asyncio
import logging

from mobly import asserts
from TC_GC_common import (find_colliding_epoch_key, generate_fabric_under_test_matcher, get_feature_map, get_iana_multicast_address,
                          get_operate_only_commands)

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_GC_2_8(MatterBaseTest):
    def desc_TC_GC_2_8(self):
        return "[TC-GC-2.8] GroupcastTesting command effect with DUT as Server - PROVISIONAL"

    def steps_TC_GC_2_8(self):
        return [
            TestStep("1a", "Commission DUT to TH (can be skipped if done in a preceding test)", is_commissioning=True),
            TestStep("1b", "TH subscribes to FabricUnderTest attribute with min interval 0s and max interval 30s"),
            TestStep("1c", "TH reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute (stored as F1)"),
            TestStep("1d", "TH subscribes to the GroupcastTesting event with min interval 0s and max interval 30s"),
            TestStep(2, "TH reads FabricUnderTest attribute. (value == 0)"),
            TestStep(3, "Enable a test operation. GroupcastTesting (TestOperation='see notes')"),
            TestStep(4, "TH awaits subscription report of new FabricUnderTest attribute. (value == F1)"),
            TestStep(5, "If LN is not supported, skip to step 17. Else TH establishes a Listener membership with GroupID G1, new KeySetID K1 using IANA address policy, then writes an ACL entry granting the Group (AuthMode=Group) Operate privilege to G1 on EP1"),
            TestStep(6, "TH sends an Operate-privilege command on EP1 as a group command using GroupID G1"),
            TestStep(7, "TH awaits a GroupcastTesting event for the successfull groupcast message: GroupID==G1, EndpointID==EP1, ClusterID/ElementID "
                     "matching the sent command, AccessAllowed==true, GroupcastTestResult==Success, DestinationIpAddress==IANA"),
            TestStep(8, "TH configures group information on the TH only for a second group G2 using a distinct KeySetID K2 and Key."),
            TestStep(9, "TH sends the same Operate-privilege command as a group command using GroupID G2 (DUT isn't joined to G2)"),
            TestStep(10, "TH awaits a GroupcastTesting event (unknown group): GroupcastTestResult==NoAvailableKey, "
                     "DestinationIpAddress==IANA"),
            TestStep(11, "TH revokes the G1 group ACL grant and re-sends the same group command to G1"),
            TestStep(12, "TH awaits a GroupcastTesting event (no access rights): GroupID==G1, AccessAllowed==false, "
                     "GroupcastTestResult==GeneralError, DestinationIpAddress==IANA"),
            TestStep(13, "TH sends the same Operate-privilege command as a group command using GroupID G1 but encrypting with a different KeySetID (K2)."),
            TestStep(14, "TH awaits a GroupcastTesting event (failed decryption): GroupcastTestResult==FailedAuth, "
                     "DestinationIpAddress==IANA"),
            TestStep(15, "TH clears the GroupKeyMap attribute (empty list) and re-sends the same group command to G1"),
            TestStep(16, "TH awaits a GroupcastTesting event (failed decryption): GroupcastTestResult==NoAvailableKey, "
                     "DestinationIpAddress==IANA"),
            TestStep(17, "Disable testing. GroupcastTesting (TestOperation=DisableTesting)"),
            TestStep(18, "TH awaits subscription report of new FabricUnderTest attribute. (value == 0)"),
            TestStep(19, "Enable a test operation for 10 seconds. GroupcastTesting (TestOperation='see notes', "
                     "DurationSeconds=10)"),
            TestStep(20, "TH awaits subscription report of new FabricUnderTest attribute. (value == F1)"),
            TestStep(21, "TH awaits subscription report of new FabricUnderTest attribute after DurationSeconds of step 19 "
                     "has elapsed. (value == 0)"),
            TestStep(22, "Test cleanup: leave G1, remove KeySetID K1, restore wildcard CASE-admin ACL"),
        ]

    def pics_TC_GC_2_8(self) -> list[str]:
        return ["GC.S"]

    def _admin_only_acl(self) -> list:
        """ACL entry granting only CASE admin access to the controller. The default ACL post commissioning setting for the controller."""
        return [
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.default_controller.nodeId],
                targets=NullValue),
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.Groupcast))
    async def test_TC_GC_2_8(self):
        groupcast_cluster = Clusters.Objects.Groupcast
        fabricUnderTest_attribute = Clusters.Groupcast.Attributes.FabricUnderTest
        groupcastTesting_event = Clusters.Groupcast.Events.GroupcastTesting
        dev_ctrl = self.default_controller
        node_id = self.dut_node_id

        self.step("1a")

        ln_enabled, sd_enabled, pga_enabled = await get_feature_map(self)
        if ln_enabled:
            testOperation = Clusters.Groupcast.Enums.GroupcastTestingEnum.kEnableListenerTesting
        else:
            testOperation = Clusters.Groupcast.Enums.GroupcastTestingEnum.kEnableSenderTesting

        self.step("1b")
        sub = AttributeSubscriptionHandler(groupcast_cluster, fabricUnderTest_attribute)
        await sub.start(dev_ctrl, node_id, self.get_endpoint(), min_interval_sec=0, max_interval_sec=30)

        self.step("1c")
        F1 = await self.read_single_attribute_check_success(
            cluster=Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        )

        self.step("1d")
        event_sub = EventSubscriptionHandler(
            expected_cluster=Clusters.Groupcast,
            expected_event_id=groupcastTesting_event.event_id)
        await event_sub.start(dev_ctrl, node_id, endpoint=0, min_interval_sec=0, max_interval_sec=30)

        self.step(2)
        fabricUnderTest = await self.read_single_attribute_check_success(groupcast_cluster, fabricUnderTest_attribute)
        asserts.assert_equal(fabricUnderTest, 0, "FabricUnderTest attribute should be 0 (testing disabled)")

        self.step(3)
        sub.reset()
        await self.send_single_cmd(Clusters.Groupcast.Commands.GroupcastTesting(testOperation=testOperation))

        self.step(4)
        sub.await_all_expected_report_matches(
            expected_matchers=[generate_fabric_under_test_matcher(F1)], timeout_sec=60)

        # DUT (Listener) joins G1 joins with KeySetId K1 and InputKey1
        GROUPID_G1 = 0x0103
        KEYSETID_K1 = 0x01a3
        INPUTKEY_1 = bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf")

        # GroupID G2 configured on the controller only with a KeySetId K2. DUT never joins G2 and doesn't know KeySetId K2.
        GROUPID_G2 = 0x0102
        KEYSETID_K2 = 0x01a2
        INPUTKEY_2 = bytes.fromhex("a0a1a2a3a4a5a6a7a8a9aaabacadaeaf")

        # SetGroupInfo flags value selecting the IANA multicast address policy with no auxiliary ACL.
        IANA_ADDR_POLICY = 0

        if ln_enabled:
            self.step(5)
            # Find a non-root endpoint exposing a command requiring only Operate privilege (EP1).
            operate_only_commands_dict = await get_operate_only_commands(dev_ctrl, node_id, exclude_ep0=True)
            asserts.assert_greater(len(operate_only_commands_dict), 0,
                                   "Listener feature is enabled but no non-root endpoint exposes an Operate-privilege command.")
            operate_ep = sorted(operate_only_commands_dict.keys())[0]
            operate_command = operate_only_commands_dict[operate_ep][0]
            logger.info("Using %s.%s on endpoint %d as the group command",
                        operate_command.cluster_object.__name__, operate_command.command_object.__name__, operate_ep)

            dev_ctrl.InitGroupTestingData()

            # Remove any pre-existing groups on this fabric to start from a clean state.
            membership = await self.read_single_attribute_check_success(
                cluster=groupcast_cluster, attribute=Clusters.Groupcast.Attributes.Membership, endpoint=0)
            if membership:
                await dev_ctrl.SendCommand(node_id, 0, Clusters.Groupcast.Commands.LeaveGroup(groupID=0))

            # Configure the controller's group state for G1 using the same key and IANA address policy).
            dev_ctrl.SetGroupKeySet(
                keyset_id=KEYSETID_K1,
                policy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
                num_keys=1,
                epoch_key0=INPUTKEY_1, epoch_start_time0=2220000,
                epoch_key1=None, epoch_start_time1=0,
                epoch_key2=None, epoch_start_time2=0)
            dev_ctrl.SetGroupKey(GROUPID_G1, KEYSETID_K1)
            dev_ctrl.SetGroupInfo(GROUPID_G1, "Group G1", IANA_ADDR_POLICY)

            # Join G1 as a Listener on the operate endpoint and provide K1 and InputKey1.
            await dev_ctrl.SendCommand(node_id, 0, Clusters.Groupcast.Commands.JoinGroup(
                groupID=GROUPID_G1, endpoints=[operate_ep], keySetID=KEYSETID_K1, key=INPUTKEY_1))

            # Grant a additional Group Operate access to the target cluster on EP1.
            acl = self._admin_only_acl()
            acl.append(Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=[GROUPID_G1],
                targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                    endpoint=operate_ep, cluster=operate_command.cluster_object.id)]))
            await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])

            # confirm that our test setup is correct.
            assert event_sub is not None and operate_command is not None and operate_ep is not None

            # function used as current_event_filter_func for wait_for_event_report_with_duplication
            def is_no_available_key(data) -> bool:
                return data.groupcastTestResult == Clusters.Groupcast.Enums.GroupcastTestResultEnum.kNoAvailableKey

            self.step(6)
            event_sub.reset()
            dev_ctrl.SendGroupCommand(GROUPID_G1, operate_command.command_object())
            await asyncio.sleep(3)

            self.step(7)
            event_data = event_sub.wait_for_event_report(groupcastTesting_event, timeout_sec=30)
            asserts.assert_equal(event_data.groupID, GROUPID_G1, "Incorrect GroupID in GroupcastTesting event")
            asserts.assert_equal(event_data.endpointID, operate_ep, "Incorrect EndpointID in GroupcastTesting event")
            asserts.assert_equal(event_data.clusterID, operate_command.cluster_object.id,
                                 "Incorrect ClusterID in GroupcastTesting event")
            asserts.assert_equal(event_data.elementID, operate_command.command_object.command_id,
                                 "Incorrect ElementID in GroupcastTesting event")
            asserts.assert_true(event_data.accessAllowed, "AccessAllowed should be true")
            asserts.assert_equal(event_data.groupcastTestResult,
                                 Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess,
                                 "GroupcastTesting event should report Success")
            asserts.assert_equal(event_data.destinationIpAddress, get_iana_multicast_address(),
                                 "Incorrect DestinationIpAddress in GroupcastTesting event")

            self.step(8)
            # Configure the controller for the unknown group G2 with a key the DUT never receives,
            # so messages sent to G2 cannot be decrypted by the DUT (NoAvailableKey).
            dev_ctrl.SetGroupKeySet(
                keyset_id=KEYSETID_K2,
                policy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
                num_keys=1,
                epoch_key0=INPUTKEY_2, epoch_start_time0=2220000,
                epoch_key1=None, epoch_start_time1=0,
                epoch_key2=None, epoch_start_time2=0)
            dev_ctrl.SetGroupKey(GROUPID_G2, KEYSETID_K2)
            dev_ctrl.SetGroupInfo(GROUPID_G2, "Group G2", IANA_ADDR_POLICY)

            self.step(9)
            event_sub.reset()
            dev_ctrl.SendGroupCommand(GROUPID_G2, operate_command.command_object())
            await asyncio.sleep(3)

            self.step(10)
            event_data = event_sub.wait_for_event_report_with_duplication(
                groupcastTesting_event, current_event_filter_func=is_no_available_key, timeout_sec=30)
            asserts.assert_equal(event_data.groupcastTestResult,
                                 Clusters.Groupcast.Enums.GroupcastTestResultEnum.kNoAvailableKey,
                                 "GroupcastTesting event for an unknown group should report NoAvailableKey")
            asserts.assert_equal(event_data.destinationIpAddress, get_iana_multicast_address(),
                                 "Incorrect DestinationIpAddress in GroupcastTesting event")

            self.step(11)
            await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.AccessControl.Attributes.Acl(self._admin_only_acl()))])
            event_sub.reset()
            dev_ctrl.SendGroupCommand(GROUPID_G1, operate_command.command_object())
            await asyncio.sleep(3)

            self.step(12)
            event_data = event_sub.wait_for_event_report(groupcastTesting_event, timeout_sec=30)
            asserts.assert_equal(event_data.groupID, GROUPID_G1, "Incorrect GroupID in GroupcastTesting event")
            asserts.assert_false(event_data.accessAllowed, "AccessAllowed should be false (no ACL grant)")
            asserts.assert_equal(event_data.groupcastTestResult,
                                 Clusters.Groupcast.Enums.GroupcastTestResultEnum.kSuccess,
                                 "GroupcastTesting event should report Success")
            asserts.assert_equal(event_data.destinationIpAddress, get_iana_multicast_address(),
                                 "Incorrect DestinationIpAddress in GroupcastTesting event")

            # Attempt to derive a distinct epoch key whose Group Session ID collides with K1 on this fabric.
            compressed_fabric_id = dev_ctrl.GetCompressedFabricId().to_bytes(8, byteorder="big")
            colliding_epoch_key = find_colliding_epoch_key(INPUTKEY_1, compressed_fabric_id)

            self.step(13)
            event_sub.reset()

            # Override KeySetID K2 on controller with the colliding epoch key
            dev_ctrl.SetGroupKeySet(
                keyset_id=KEYSETID_K2,
                policy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
                num_keys=1,
                epoch_key0=colliding_epoch_key, epoch_start_time0=2220000,
                epoch_key1=None, epoch_start_time1=0,
                epoch_key2=None, epoch_start_time2=0)

            # Step 13
            # Remap GroupID G1 to KeySetID K2 on controller so step 13 encrypts group message for G1 with K2 while the DUT only holds K1;
            # the collision makes the DUT select K1 for decryption, which fails and yields FailedAuth.
            dev_ctrl.SetGroupKey(GROUPID_G1, KEYSETID_K2)
            dev_ctrl.SendGroupCommand(GROUPID_G1, operate_command.command_object())
            await asyncio.sleep(3)

            self.step(14)
            event_data = event_sub.wait_for_event_report(groupcastTesting_event, timeout_sec=30)
            asserts.assert_equal(event_data.groupcastTestResult,
                                 Clusters.Groupcast.Enums.GroupcastTestResultEnum.kFailedAuth,
                                 "GroupcastTesting event should report FailedAuth")
            asserts.assert_equal(event_data.destinationIpAddress, get_iana_multicast_address(),
                                 "Incorrect DestinationIpAddress in GroupcastTesting event")

            self.step(15)
            await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap([]))])
            event_sub.reset()
            dev_ctrl.SendGroupCommand(GROUPID_G1, operate_command.command_object())
            await asyncio.sleep(3)

            self.step(16)
            event_data = event_sub.wait_for_event_report_with_duplication(
                groupcastTesting_event, current_event_filter_func=is_no_available_key, timeout_sec=30)
            asserts.assert_equal(event_data.groupcastTestResult,
                                 Clusters.Groupcast.Enums.GroupcastTestResultEnum.kNoAvailableKey,
                                 "GroupcastTesting event after clearing the key should report NoAvailableKey")
            asserts.assert_equal(event_data.destinationIpAddress, get_iana_multicast_address(),
                                 "Incorrect DestinationIpAddress in GroupcastTesting event")
        else:
            self.mark_step_range_skipped(5, 16)

        self.step(17)
        sub.reset()
        await self.send_single_cmd(Clusters.Groupcast.Commands.GroupcastTesting(
            testOperation=Clusters.Groupcast.Enums.GroupcastTestingEnum.kDisableTesting))

        self.step(18)
        sub.await_all_expected_report_matches(
            expected_matchers=[generate_fabric_under_test_matcher(0)], timeout_sec=60)

        self.step(19)
        sub.reset()
        await self.send_single_cmd(Clusters.Groupcast.Commands.GroupcastTesting(
            testOperation=testOperation, durationSeconds=10))

        self.step(20)
        sub.await_all_expected_report_matches(
            expected_matchers=[generate_fabric_under_test_matcher(F1)], timeout_sec=60)

        self.step(21)
        sub.reset()
        sub.await_all_expected_report_matches(
            expected_matchers=[generate_fabric_under_test_matcher(0)], timeout_sec=60)

        if ln_enabled:
            self.step(22)
            # Leave all groups created
            await dev_ctrl.SendCommand(node_id, 0, Clusters.Groupcast.Commands.LeaveGroup(groupID=0))
            # Remove the KeySetID K1 on the DUT
            await dev_ctrl.SendCommand(node_id, 0, Clusters.GroupKeyManagement.Commands.KeySetRemove(
                groupKeySetID=KEYSETID_K1))
            # Restore the default ACL post commissioning setting for the controller.
            await dev_ctrl.WriteAttribute(node_id, [(0, Clusters.AccessControl.Attributes.Acl(self._admin_only_acl()))])
        else:
            self.skip_step(22)


if __name__ == "__main__":
    default_matter_test_main()
