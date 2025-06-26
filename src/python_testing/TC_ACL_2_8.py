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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     factory-reset: true
#     quiet: true
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
# === END CI TEST ARGUMENTS ===

import logging
import random

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_ACL_2_8(MatterBaseTest):
    async def get_latest_event_number(self, acec_event: Clusters.AccessControl.Events.AccessControlEntryChanged) -> int:
        event_path = [(self.matter_test_config.endpoint, acec_event, 1)]
        events = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path)
        if not events:
            raise AssertionError(f"No events found for {acec_event} to determine the latest event number.")
        return max([e.Header.EventNumber for e in events])

    def _get_relevant_acl_events(self, all_events: list, expected_add_subject_node_id: int, expected_change_subject_node_ids: list) -> tuple:
        """
        Extracts the most recent 'added' and 'changed' events for a specific node from all events.

        Args:
            all_events: List of all AccessControlEntryChanged events
            expected_add_subject_node_id: Node ID expected in the 'added' event subjects
            expected_change_subject_node_ids: List of node IDs expected in the 'changed' event subjects

        Returns:
            Tuple containing the relevant 'added' and 'changed' events in chronological order
        """
        # First find the most recent "added" event
        added_events = [e for e in all_events if (
            e.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
            e.Data.latestValue.subjects == [expected_add_subject_node_id]
        )]
        asserts.assert_true(len(added_events) > 0, f"Expected 'added' event for node {expected_add_subject_node_id} not found")
        added_event = sorted(added_events, key=lambda e: e.Header.EventNumber)[-1]

        # Guarantee we have a valid added_event
        asserts.assert_is_not_none(added_event, f"Added event for node {expected_add_subject_node_id} must not be None")

        # Then find the most recent "changed" event that occurred after the "added" event
        changed_events = [e for e in all_events if (
            e.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kChanged and
            e.Data.latestValue.subjects == expected_change_subject_node_ids and
            e.Header.EventNumber > added_event.Header.EventNumber
        )]
        asserts.assert_true(len(changed_events) > 0,
                            f"Expected 'changed' event for node {expected_add_subject_node_id} not found after the 'added' event")
        changed_event = sorted(changed_events, key=lambda e: e.Header.EventNumber)[-1]

        # Guarantee we have a valid changed_event
        asserts.assert_is_not_none(changed_event, f"Changed event for node {expected_add_subject_node_id} must not be None")

        return (added_event, changed_event)

    def _verify_acl_event(
            self,
            event,
            admin_node_id,
            admin_passcode_id,
            change_type,
            subjects,
            fabric_index):
        """Verifies ACL event contents"""
        data = event.Data

        # Verify admin IDs
        asserts.assert_equal(data.adminNodeID, admin_node_id)
        asserts.assert_equal(data.adminPasscodeID, admin_passcode_id)

        # Verify event data
        asserts.assert_equal(data.changeType, change_type)
        asserts.assert_equal(data.latestValue.privilege, 5)
        asserts.assert_equal(data.latestValue.authMode, 2)
        asserts.assert_equal(
            data.latestValue.subjects,
            [subjects] if not isinstance(
                subjects,
                list) else subjects)
        asserts.assert_equal(data.latestValue.targets, NullValue)
        asserts.assert_equal(data.fabricIndex, fabric_index)

    def desc_TC_ACL_2_8(self) -> str:
        return "[TC-ACL-2.8] ACL multi-fabric"

    def steps_TC_ACL_2_8(self) -> list[TestStep]:
        steps = [
            TestStep(1, "TH1 commissions DUT using admin node ID N1",
                     "DUT is commissioned on TH1 fabric", is_commissioning=True),
            TestStep(2, "TH1 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                     "Result is SUCCESS, value is stored as F1"),
            TestStep(3, "TH1 puts DUT into commissioning mode, TH2 commissions DUT using admin node ID N2",
                     "DUT is commissioned on TH2 fabric"),
            TestStep(4, "TH2 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                     "Result is SUCCESS, value is stored as F2"),
            TestStep(5, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is list of AccessControlEntryStruct containing 1 element",
                     "Result is SUCCESS"),
            TestStep(6, "TH2 writes DUT Endpoint 0 AccessControl cluster ACL attribute value is list of AccessControlEntryStruct containing 1 element",
                     "Result is SUCCESS"),
            TestStep(7, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value is list of AccessControlEntryStruct containing 1 element, and MUST NOT contain an element with FabricIndex F2"),
            TestStep(8, "TH2 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value is list of AccessControlEntryStruct containing 1 element, and MUST NOT contain an element with FabricIndex F1"),
            TestStep(9, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event",
                     "Result is SUCCESS, value is list of AccessControlEntryChanged containing 2 elements, and MUST NOT contain any element with FabricIndex F2"),
            TestStep(10, "TH2 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event",
                     "Result is SUCCESS, value is list of AccessControlEntryChanged containing 2 elements, and MUST NOT contain any element with FabricIndex F1"),
        ]
        return steps

    @async_test_body
    async def test_TC_ACL_2_8(self):
        self.step(1)
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)

        self.step(2)
        # Read CurrentFabricIndex for TH1
        oc_cluster = Clusters.OperationalCredentials
        cfi_attribute = oc_cluster.Attributes.CurrentFabricIndex
        f1 = await self.read_single_attribute_check_success(endpoint=0, cluster=oc_cluster, attribute=cfi_attribute)
        logging.info(f"CurrentFabricIndex F1 {str(f1)}")

        self.step(3)
        # Commission TH2
        params = await self.th1.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=900, iteration=10000, discriminator=self.discriminator, option=1)
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(
            vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(
            nodeId=2, useTestCommissioner=True)
        setupPinCode = params.setupPinCode
        await self.th2.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.discriminator)

        self.step(4)
        # Read CurrentFabricIndex for TH2
        f2 = await self.read_single_attribute_check_success(dev_ctrl=self.th2, endpoint=0, cluster=oc_cluster, attribute=cfi_attribute)
        logging.info(f"CurrentFabricIndex F2 {str(f2)}")

        self.step(5)
        # TH1 writes ACL attribute
        acl_struct = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=5,
            authMode=2,
            subjects=[self.th1.nodeId, 1111]
        )
        acl_attr = Clusters.AccessControl.Attributes.Acl
        acl_list = [acl_struct]
        result = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attr(value=acl_list))]
        )
        asserts.assert_equal(
            result[0].Status,
            Status.Success,
            "Write should have succeeded")

        self.step(6)
        # TH2 writes ACL attribute
        acl_struct_th2 = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=5,
            authMode=2,
            subjects=[self.th2.nodeId, 2222]
        )
        acl_list = [acl_struct_th2]
        result = await self.th2.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attr(value=acl_list))]
        )
        asserts.assert_equal(
            result[0].Status,
            Status.Success,
            "Write should have succeeded")

        self.step(7)
        # TH1 reads ACL attribute
        ac_cluster = Clusters.AccessControl
        acl_attr = Clusters.AccessControl.Attributes.Acl
        acl_list = await self.read_single_attribute_check_success(dev_ctrl=self.th1, endpoint=0, cluster=ac_cluster, attribute=acl_attr)
        logging.info(f"ACL list {str(acl_list)}")

        asserts.assert_equal(
            len(acl_list), 1, "Should have exactly one ACL entry")
        entry = acl_list[0]

        asserts.assert_equal(
            entry.privilege, 5, "Privilege should be Administer (5)")
        asserts.assert_equal(
            entry.authMode, 2, "AuthMode should be CASE (2)")
        asserts.assert_equal(entry.subjects, [self.th1.nodeId, 1111])
        asserts.assert_equal(
            entry.targets,
            NullValue,
            "Targets should be NullValue")
        asserts.assert_equal(entry.fabricIndex, f1)

        for entry in acl_list:
            asserts.assert_not_equal(
                entry.fabricIndex, f2, "Should not contain entry with FabricIndex F2")

        self.step(8)
        # TH2 reads ACL attribute
        acl_list = await self.read_single_attribute_check_success(dev_ctrl=self.th2, endpoint=0, cluster=ac_cluster, attribute=acl_attr)
        logging.info(f"ACL list {str(acl_list)}")

        asserts.assert_equal(
            len(acl_list), 1, "Should have exactly one ACL entry")
        entry = acl_list[0]

        # Verify entry contents
        asserts.assert_equal(
            entry.privilege, 5, "Privilege should be Administer (5)")
        asserts.assert_equal(
            entry.authMode, 2, "AuthMode should be CASE (2)")
        asserts.assert_equal(entry.subjects, [self.th2.nodeId, 2222])
        asserts.assert_equal(
            entry.targets,
            NullValue,
            "Targets should be NullValue")
        asserts.assert_equal(entry.fabricIndex, f2)

        for entry in acl_list:
            asserts.assert_not_equal(
                entry.fabricIndex, f1, "Should not contain entry with FabricIndex F1")

        self.step(9)
        # TH1 reads AccessControlEntryChanged events
        events = await self.th1.ReadEvent(
            self.dut_node_id,
            [(0, Clusters.AccessControl.Events.AccessControlEntryChanged)],
            fabricFiltered=True
        )

        # Below event filtering and parsing is currently required in the event that the DUT is not reset before running this test.
        added_event, changed_event = self._get_relevant_acl_events(events, self.th1.nodeId, [self.th1.nodeId, 1111])
        logging.info(f"TH1 Events: added_event={added_event}, changed_event={changed_event}")

        # Verify event contents match expected sequence
        self._verify_acl_event(
            added_event,
            NullValue,
            0,
            Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded,
            self.th1.nodeId,
            f1)
        self._verify_acl_event(
            changed_event,
            self.th1.nodeId,
            NullValue,
            Clusters.AccessControl.Enums.ChangeTypeEnum.kChanged,
            [self.th1.nodeId, 1111],
            f1)

        for event in [added_event, changed_event]:
            asserts.assert_not_equal(
                event.Data.fabricIndex,
                f2,
                "Should not contain event with FabricIndex F2")

        self.step(10)
        # TH2 reads AccessControlEntryChanged events
        events = await self.th2.ReadEvent(
            self.dut_node_id,
            [(0, Clusters.AccessControl.Events.AccessControlEntryChanged)],
            fabricFiltered=True
        )

        added_event, changed_event = self._get_relevant_acl_events(events, self.th2.nodeId, [self.th2.nodeId, 2222])
        logging.info(f"TH2 Events: added_event={added_event}, changed_event={changed_event}")

        # Verify event contents match expected sequence
        self._verify_acl_event(
            added_event,
            NullValue,
            0,
            Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded,
            self.th2.nodeId,
            f2)

        self._verify_acl_event(
            changed_event,
            self.th2.nodeId,
            NullValue,
            Clusters.AccessControl.Enums.ChangeTypeEnum.kChanged,
            [self.th2.nodeId, 2222],
            f2)

        for event in [added_event, changed_event]:
            asserts.assert_not_equal(
                event.Data.fabricIndex,
                f1,
                "Should not contain event with FabricIndex F1")


if __name__ == "__main__":
    default_matter_test_main()
