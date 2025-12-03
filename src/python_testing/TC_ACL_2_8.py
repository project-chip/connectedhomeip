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

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

log = logging.getLogger(__name__)


class TC_ACL_2_8(MatterBaseTest):
    async def get_latest_event_number(self, acec_event: Clusters.AccessControl.Events.AccessControlEntryChanged) -> int:
        event_path = [(self.matter_test_config.endpoint, acec_event, 1)]
        events = await self.default_controller.ReadEvent(nodeId=self.dut_node_id, events=event_path)
        if not events:
            raise AssertionError(f"No events found for {acec_event} to determine the latest event number.")
        return max([e.Header.EventNumber for e in events])

    def _get_relevant_acl_events(self, all_events: list, expected_add_subject_node_id: int,
                                 expected_change_subject_node_ids: list) -> tuple:
        """
        Extracts the most recent 'added' and 'changed' events for a specific node from all events.

        Args:
            all_events: List of all AccessControlEntryChanged events
            expected_add_subject_node_id: Node ID expected in the 'added' event subjects
            expected_change_subject_node_ids: List of node IDs expected in the 'changed' event subjects

        Returns:
            Tuple containing the relevant 'added' and 'changed' events in chronological order
        """
        # Find the most recent "added" event for the original subject
        added_events = [e for e in all_events if (
            e.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
            e.Data.latestValue.subjects == [expected_add_subject_node_id]
        )]
        asserts.assert_true(len(added_events) > 0, f"Expected 'added' event for node {expected_add_subject_node_id} not found")
        added_event = sorted(added_events, key=lambda e: e.Header.EventNumber)[-1]
        asserts.assert_is_not_none(added_event, f"Added event for node {expected_add_subject_node_id} must not be None")

        # Try to find a "changed" event (new encoding)
        changed_events = [e for e in all_events if (
            e.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kChanged and
            set(e.Data.latestValue.subjects) == set(expected_change_subject_node_ids) and
            e.Header.EventNumber > added_event.Header.EventNumber
        )]
        if changed_events:
            changed_event = sorted(changed_events, key=lambda e: e.Header.EventNumber)[-1]
            return (added_event, changed_event)

        # If no "changed", look for "removed" then "added" for new subjects (legacy encoding)
        removed_events = [e for e in all_events if (
            e.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kRemoved and
            e.Data.latestValue.subjects == [expected_add_subject_node_id] and
            e.Header.EventNumber > added_event.Header.EventNumber
        )]
        asserts.assert_true(len(removed_events) > 0,
                            f"Expected 'removed' event for node {expected_add_subject_node_id} not found after the 'added' event")
        removed_event = sorted(removed_events, key=lambda e: e.Header.EventNumber)[-1]

        added_new_events = [e for e in all_events if (
            e.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
            set(e.Data.latestValue.subjects) == set(expected_change_subject_node_ids) and
            e.Header.EventNumber > removed_event.Header.EventNumber
        )]
        asserts.assert_true(
            len(added_new_events) > 0,
            f"Expected 'added' event for new subjects {expected_change_subject_node_ids} not found after the 'removed' event")
        added_event2 = sorted(added_new_events, key=lambda e: e.Header.EventNumber)[-1]

        return (added_event, removed_event, added_event2)

    async def _shutdown_controller(self, controller_name: str):
        if hasattr(self, controller_name):
            try:
                controller = getattr(self, controller_name)
                logging.info(f"Shutting down {controller_name.upper()} controller")
                await controller.Shutdown()
                delattr(self, controller_name)
            except Exception as e:
                logging.warning(f"Error cleaning up {controller_name.upper()}: {e}")

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

    async def write_attribute_with_encoding_option(self, controller, node_id, path, forceLegacyListEncoding):
        if forceLegacyListEncoding:
            return await controller.TestOnlyWriteAttributeWithLegacyList(node_id, path)
        return await controller.WriteAttribute(node_id, path)

    def _validate_events(self, events, expected_fabric_index, expected_node_id, other_fabric_index, controller_name, is_filtered, force_legacy_encoding):
        """Helper method to validate events for a controller"""
        logging.info(f"Found {len(events)} events for {controller_name}")

        # We expect to see two events with the expected fabric index if not using legacy encoding
        # We expect to see one event with the expected fabric index if using legacy encoding
        expected_events_count = 1 if force_legacy_encoding else 2

        found_valid_events = 0
        found_other_event = False

        for event in events:
            logging.info(f"Examining event: {str(event)}")
            if hasattr(event, 'Data') and hasattr(event.Data, 'fabricIndex'):
                # If this is an event for the expected fabric
                if event.Data.fabricIndex == expected_fabric_index:
                    # Check for expected field values
                    if ((event.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
                        event.Data.adminNodeID == NullValue and
                        event.Data.adminPasscodeID == 0) or
                        (event.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kChanged and
                        event.Data.adminNodeID == expected_node_id and
                            event.Data.adminPasscodeID == NullValue)):
                        found_valid_events += 1

                # If this is an event for the other fabric
                if event.Data.fabricIndex == other_fabric_index:
                    found_other_event = True

        asserts.assert_equal(found_valid_events, expected_events_count,
                             f"Expected {expected_events_count} valid events for {controller_name}, found {found_valid_events}")

        if is_filtered:
            other_controller = "TH1" if controller_name == "TH2" else "TH2"
            asserts.assert_false(
                found_other_event, f"{controller_name} should not see any events from {other_controller}'s fabric when fabric filtered")

    async def internal_test_TC_ACL_2_8(self, force_legacy_encoding: bool):
        self.step(1)
        # Open commissioning window with default controller
        self.discriminator = random.randint(0, 4095)
        params = await self.default_controller.OpenCommissioningWindow(
            nodeId=self.dut_node_id,
            timeout=900,
            iteration=10000,
            discriminator=self.discriminator,
            option=1
        )

        # Create a new certificate authority and fabric admin for TH1
        th1_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th1_fabric_admin = th1_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=random.randint(1, 100000))
        self.th1 = th1_fabric_admin.NewController(nodeId=1, useTestCommissioner=True)

        # Commission TH1 through the commissioning window
        setupPinCode = params.setupPinCode
        await self.th1.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.discriminator
        )

        self.step(2)
        # Read CurrentFabricIndex for TH1
        oc_cluster = Clusters.OperationalCredentials
        cfi_attribute = oc_cluster.Attributes.CurrentFabricIndex
        f1 = await self.read_single_attribute_check_success(dev_ctrl=self.th1, endpoint=0, cluster=oc_cluster, attribute=cfi_attribute)
        logging.info(f"CurrentFabricIndex F1 {str(f1)}")

        self.step(3)
        # Open commissioning window with TH1
        params = await self.th1.OpenCommissioningWindow(
            nodeId=self.dut_node_id, timeout=900, iteration=10000, discriminator=self.discriminator, option=1)
        # Commission TH2
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
        acl_list = [acl_struct]
        acl_attr = Clusters.AccessControl.Attributes.Acl
        result = await self.write_attribute_with_encoding_option(
            self.th1,
            self.dut_node_id,
            [(0, acl_attr(value=acl_list))],
            forceLegacyListEncoding=force_legacy_encoding
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
        result = await self.write_attribute_with_encoding_option(
            self.th2,
            self.dut_node_id,
            [(0, acl_attr(value=acl_list))],
            forceLegacyListEncoding=force_legacy_encoding
        )
        asserts.assert_equal(
            result[0].Status,
            Status.Success,
            "Write should have succeeded")

        self.step(7)
        # TH1 reads ACL attribute with fabricFiltered=True (default)
        ac_cluster = Clusters.AccessControl
        acl_attr = Clusters.AccessControl.Attributes.Acl

        # Read with fabric_filtered=True (default)
        acl_list_filtered = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1, endpoint=0, cluster=ac_cluster, attribute=acl_attr
        )
        logging.info("TH1 read ACL result (fabricFiltered=True): %s", str(acl_list_filtered))

        asserts.assert_equal(
            len(acl_list_filtered), 1, "Should have exactly one ACL entry when fabric filtered")
        entry = acl_list_filtered[0]

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

        for entry in acl_list_filtered:
            asserts.assert_not_equal(
                entry.fabricIndex, f2, "Should not contain entry with FabricIndex F2")

        # Read with fabric_filtered=False
        acl_list_unfiltered = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1, endpoint=0, cluster=ac_cluster, attribute=acl_attr,
            fabric_filtered=False
        )
        logging.info("TH1 read ACL result (fabric_filtered=False): %s", str(acl_list_unfiltered))
        asserts.assert_greater(len(acl_list_unfiltered), 1, "Should have at least two ACL entries when not fabric filtered")
        # Check non-accessing fabric entry is empty because data leaks are bad
        for entry in acl_list_unfiltered:
            if entry.fabricIndex == f2:
                asserts.assert_equal(entry.privilege, 0)
                asserts.assert_equal(entry.authMode, 0)
                asserts.assert_true(
                    entry.subjects is None or entry.subjects == NullValue or entry.subjects == [],
                    "Non-accessing fabric: subjects should be empty"
                )
                asserts.assert_true(
                    entry.targets is None or entry.targets == NullValue,
                    "Non-accessing fabric: targets should be empty"
                )

        self.step(8)
        # TH2 reads ACL attribute with fabricFiltered=True (default)
        acl_list_filtered = await self.read_single_attribute_check_success(
            dev_ctrl=self.th2, endpoint=0, cluster=ac_cluster, attribute=acl_attr
        )
        logging.info("TH2 read ACL result (fabric_filtered=True): %s", str(acl_list_filtered))

        asserts.assert_equal(
            len(acl_list_filtered), 1, "Should have exactly one ACL entry when fabric filtered")
        entry = acl_list_filtered[0]

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

        for entry in acl_list_filtered:
            asserts.assert_not_equal(
                entry.fabricIndex, f1, "Should not contain entry with FabricIndex F1")

        # Read with fabric_filtered=False
        acl_list_unfiltered = await self.read_single_attribute_check_success(
            dev_ctrl=self.th2, endpoint=0, cluster=ac_cluster, attribute=acl_attr,
            fabric_filtered=False
        )
        logging.info("TH2 read ACL result (fabric_filtered=False): %s", str(acl_list_unfiltered))
        asserts.assert_greater(len(acl_list_unfiltered), 1, "Should have at least two ACL entries when not fabric filtered")
        # Check non-accessing fabric entry is empty because data leaks are bad
        for entry in acl_list_unfiltered:
            if entry.fabricIndex == f1:
                asserts.assert_equal(entry.privilege, 0)
                asserts.assert_equal(entry.authMode, 0)
                asserts.assert_true(
                    entry.subjects is None or entry.subjects == NullValue or entry.subjects == [],
                    "Non-accessing fabric: subjects should be empty"
                )
                asserts.assert_true(
                    entry.targets is None or entry.targets == NullValue,
                    "Non-accessing fabric: targets should be empty"
                )

        self.step(9)
        # TH1 reads AccessControlEntryChanged events with fabricFiltered=True
        events_filtered = await self.th1.ReadEvent(
            self.dut_node_id,
            [(0, Clusters.AccessControl.Events.AccessControlEntryChanged)],
            fabricFiltered=True
        )
        self._validate_events(events_filtered, f1, self.th1.nodeId, f2, "TH1", True, force_legacy_encoding)

        # Read with fabricFiltered=False
        events_unfiltered = await self.th1.ReadEvent(
            self.dut_node_id,
            [(0, Clusters.AccessControl.Events.AccessControlEntryChanged)],
            fabricFiltered=False
        )
        self._validate_events(events_unfiltered, f1, self.th1.nodeId, f2, "TH1", False, force_legacy_encoding)

        if force_legacy_encoding:
            asserts.assert_equal(len(events_filtered), 3, "Should have exactly 3 events")
        else:
            asserts.assert_equal(len(events_filtered), 2, "Should have exactly 2 events")

        # Unified event extraction
        result = self._get_relevant_acl_events(events_filtered, self.th1.nodeId, [self.th1.nodeId, 1111])
        added_event = result[0]
        self._verify_acl_event(
            added_event,
            NullValue,
            0,
            Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded,
            self.th1.nodeId,
            f1)
        relevant_events = [added_event]
        if force_legacy_encoding:
            removed_event, added_event2 = result[1], result[2]
            self._verify_acl_event(
                removed_event,
                self.th1.nodeId,
                NullValue,
                Clusters.AccessControl.Enums.ChangeTypeEnum.kRemoved,
                self.th1.nodeId,
                f1)
            self._verify_acl_event(
                added_event2,
                self.th1.nodeId,
                NullValue,
                Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded,
                [self.th1.nodeId, 1111],
                f1)
            relevant_events.extend([removed_event, added_event2])
        else:
            changed_event = result[1]
            self._verify_acl_event(
                changed_event,
                self.th1.nodeId,
                NullValue,
                Clusters.AccessControl.Enums.ChangeTypeEnum.kChanged,
                [self.th1.nodeId, 1111],
                f1)
            relevant_events.append(changed_event)
        for event in relevant_events:
            asserts.assert_not_equal(
                event.Data.fabricIndex,
                f2,
                "Should not contain event with FabricIndex F2")

        self.step(10)
        # TH2 reads AccessControlEntryChanged events with fabricFiltered=True
        events_filtered = await self.th2.ReadEvent(
            self.dut_node_id,
            [(0, Clusters.AccessControl.Events.AccessControlEntryChanged)],
            fabricFiltered=True
        )
        self._validate_events(events_filtered, f2, self.th2.nodeId, f1, "TH2", True, force_legacy_encoding)

        # Read with fabricFiltered=False
        events_unfiltered = await self.th2.ReadEvent(
            self.dut_node_id,
            [(0, Clusters.AccessControl.Events.AccessControlEntryChanged)],
            fabricFiltered=False
        )
        self._validate_events(events_unfiltered, f2, self.th2.nodeId, f1, "TH2", False, force_legacy_encoding)

        if force_legacy_encoding:
            asserts.assert_equal(len(events_filtered), 3, "Should have exactly 3 events")
        else:
            asserts.assert_equal(len(events_filtered), 2, "Should have exactly 2 events")

        result = self._get_relevant_acl_events(events_filtered, self.th2.nodeId, [self.th2.nodeId, 2222])
        added_event = result[0]
        self._verify_acl_event(
            added_event,
            NullValue,
            0,
            Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded,
            self.th2.nodeId,
            f2)
        relevant_events = [added_event]
        if force_legacy_encoding:
            removed_event, added_event2 = result[1], result[2]
            self._verify_acl_event(
                removed_event,
                self.th2.nodeId,
                NullValue,
                Clusters.AccessControl.Enums.ChangeTypeEnum.kRemoved,
                self.th2.nodeId,
                f2)
            self._verify_acl_event(
                added_event2,
                self.th2.nodeId,
                NullValue,
                Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded,
                [self.th2.nodeId, 2222],
                f2)
            relevant_events.extend([removed_event, added_event2])
        else:
            changed_event = result[1]
            self._verify_acl_event(
                changed_event,
                self.th2.nodeId,
                NullValue,
                Clusters.AccessControl.Enums.ChangeTypeEnum.kChanged,
                [self.th2.nodeId, 2222],
                f2)
            relevant_events.append(changed_event)
        for event in relevant_events:
            asserts.assert_not_equal(
                event.Data.fabricIndex,
                f1,
                "Should not contain event with FabricIndex F1")

         # Re-running test using the legacy list writing mechanism
        if not force_legacy_encoding:
            self.step(11)
            logging.info("*** Rerunning test using the legacy list writing mechanism now ***")
        else:
            self.skip_step(11)

    def desc_TC_ACL_2_8(self) -> str:
        return "[TC-ACL-2.8] ACL multi-fabric"

    def pics_TC_ACL_2_8(self) -> list[str]:
        return ['ACL.S']

    def steps_TC_ACL_2_8(self) -> list[TestStep]:
        return [
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
                     "Result is SUCCESS, value is list of AccessControlEntryChanged containing 2 elements if new list encoding is used, 3 elements if legacy list encoding is used, and MUST NOT contain any element with FabricIndex F2"),
            TestStep(10, "TH2 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event",
                     "Result is SUCCESS, value is list of AccessControlEntryChanged containing 2 elements if new list encoding is used, 3 elements if legacy list encoding is used, and MUST NOT contain any element with FabricIndex F1"),
            TestStep(11, "Re-run the test using the legacy list writing mechanism, where the client issues a series of AttributeDataIBs, with the first containing a path to the list itself and Data that is empty array, which signals clearing the list, and subsequent AttributeDataIBs containing updates.",
                     "Test succeeds with legacy list encoding mechanism"),
        ]

    @async_test_body
    async def test_TC_ACL_2_8(self):
        # First run with new list method
        await self.internal_test_TC_ACL_2_8(force_legacy_encoding=False)

        # --- Simplified cleanup between test runs ---
        logging.info("Cleaning up fabrics between test runs")

        # First, clean up TH1 and TH2 controllers
        await self._shutdown_controller('th1')
        await self._shutdown_controller('th2')

        # Reset step counter and run second test with legacy encoding
        self.current_step_index = 0
        logging.info("Starting second test run with legacy encoding")
        await self.internal_test_TC_ACL_2_8(force_legacy_encoding=True)


if __name__ == "__main__":
    default_matter_test_main()
