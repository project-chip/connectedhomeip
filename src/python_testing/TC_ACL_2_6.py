#    Copyright (c) 2025 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may o+btain a copy of the License at
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
#       --endpoint 0
# === END CI TEST ARGUMENTS ===

import logging
import random

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from chip.testing.event_attribute_reporting import EventSubscriptionHandler
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_ACL_2_6(MatterBaseTest):
    async def get_latest_event_number(self, acec_event: Clusters.AccessControl.Events.AccessControlEntryChanged) -> int:
        event_path = [(self.matter_test_config.endpoint, acec_event, 1)]
        events = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path)

        if not events:
            raise AssertionError(f"No events found for {acec_event} to determine latest event number.")

        return max(e.Header.EventNumber for e in events)

    # Compare events by their relevant fields instead of string representation
    def event_key(self, event):
        # Extract only the fields we care about for comparison
        return (
            str(event.latestValue.privilege),
            str(event.latestValue.authMode),
            str(event.latestValue.subjects),
            str(event.latestValue.targets),
            str(event.latestValue.fabricIndex)
        )

    async def write_attribute_with_encoding_option(self, controller, node_id, path, forceLegacyListEncoding):
        if forceLegacyListEncoding:
            return await controller.TestOnlyWriteAttributeWithLegacyList(node_id, path)
        else:
            return await controller.WriteAttribute(node_id, path)

    def desc_TC_ACL_2_6(self) -> str:
        return "[TC-ACL-2.6] AccessControlEntryChanged event"

    async def internal_test_TC_ACL_2_6(self, force_legacy_encoding: bool):
        self.step(1)
        # Initialize TH1 controller
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)

        self.step(2)
        # Read CurrentFabricIndex for TH1
        oc_cluster = Clusters.OperationalCredentials
        cfi_attribute = oc_cluster.Attributes.CurrentFabricIndex
        f1 = await self.read_single_attribute_check_success(endpoint=0, cluster=oc_cluster, attribute=cfi_attribute)

        self.step(3)
        # Set up event subscription for future steps
        acec_event = Clusters.AccessControl.Events.AccessControlEntryChanged
        self.events_callback = EventSubscriptionHandler(expected_cluster=Clusters.AccessControl)
        await self.events_callback.start(self.default_controller, self.dut_node_id, 0)

        # Read initial events
        events_response = await self.th1.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event, 1)],
            fabricFiltered=True
        )
        # Getting the initial event from commissioning, validating it is the one
        # we are expecting as it adds the admin entry for our controller for
        # access control.
        events_response = [events_response[0]]
        logging.info(f"Events response: {events_response}")

        # If we found events via read, verify them
        expected_event = Clusters.AccessControl.Events.AccessControlEntryChanged(
            adminNodeID=NullValue,
            adminPasscodeID=0,
            changeType=Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded,
            latestValue=Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            fabricIndex=f1
        )
        asserts.assert_equal(len(events_response), 1, "Expected 1 event")

        # Verify read events match expectations
        found = False
        for event in events_response:
            if event.Data == expected_event:
                found = True
                break
        asserts.assert_true(found, "Expected event not found in read response")

        latest_event_number = await self.get_latest_event_number(acec_event)

        self.step(4)
        # Write ACL attribute
        acl_entries = [
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
                fabricIndex=f1
            ),
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=NullValue,
                targets=NullValue,
                fabricIndex=f1
            )
        ]

        acl_attr = Clusters.AccessControl.Attributes.Acl
        result = await self.write_attribute_with_encoding_option(
            self.th1,
            self.dut_node_id,
            [(0, acl_attr(value=acl_entries))],
            forceLegacyListEncoding=force_legacy_encoding
        )
        asserts.assert_equal(result[0].Status, Status.Success, "Write should have succeeded")

        self.step(5)
        # Wait for subscription events
        received_subscription_events = []
        expected_event_count = 3 if force_legacy_encoding else 2
        for _ in range(expected_event_count):
            event_data = self.events_callback.wait_for_event_report(acec_event, timeout_sec=15)
            logging.info(f"Received subscription event: {event_data}")
            received_subscription_events.append(event_data)

        # Read events
        events_response = await self.th1.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True,
            eventNumberFilter=latest_event_number + 1
        )
        logging.info(f"Read events response: {events_response}")

        asserts.assert_true(events_response, "Did not receive a response when calling ReadEvents")
        read_events = sorted([e.Data for e in events_response],
                             key=lambda x: next(e.Header.EventNumber for e in events_response if e.Data == x))

        if force_legacy_encoding:
            asserts.assert_true(len(read_events) == 3, f"Expected 3 events from read, but got {len(read_events)}")
            e1, e2, e3 = read_events

            # Event 1: Removed (admin)
            asserts.assert_equal(
                e1.changeType,
                Clusters.AccessControl.Enums.ChangeTypeEnum.kRemoved,
                "Expected Removed change type")
            asserts.assert_equal(e1.adminNodeID, self.default_controller.nodeId, "AdminNodeID should be the controller node ID")
            asserts.assert_in('chip.clusters.Types.Nullable', str(type(e1.adminPasscodeID)), "AdminPasscodeID should be Null")
            asserts.assert_equal(e1.latestValue, acl_entries[0], "LatestValue should match admin ACL entry")
            asserts.assert_equal(e1.latestValue.fabricIndex, f1, "LatestValue.FabricIndex should be the current fabric index")
            asserts.assert_equal(e1.fabricIndex, f1, "FabricIndex should be the current fabric index")

            # Event 2: Added (admin)
            asserts.assert_equal(e2.changeType, Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded, "Expected Added change type")
            asserts.assert_equal(e2.adminNodeID, self.default_controller.nodeId, "AdminNodeID should be the controller node ID")
            asserts.assert_in('chip.clusters.Types.Nullable', str(type(e2.adminPasscodeID)), "AdminPasscodeID should be Null")
            asserts.assert_equal(e2.latestValue, acl_entries[0], "LatestValue should match admin ACL entry")
            asserts.assert_equal(e2.latestValue.fabricIndex, f1, "LatestValue.FabricIndex should be the current fabric index")
            asserts.assert_equal(e2.fabricIndex, f1, "FabricIndex should be the current fabric index")

            # Event 3: Added (operate/group)
            asserts.assert_equal(e3.changeType, Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded, "Expected Added change type")
            asserts.assert_equal(e3.adminNodeID, self.default_controller.nodeId, "AdminNodeID should be the controller node ID")
            asserts.assert_in('chip.clusters.Types.Nullable', str(type(e3.adminPasscodeID)), "AdminPasscodeID should be Null")
            asserts.assert_equal(e3.latestValue, acl_entries[1], "LatestValue should match operate/group ACL entry")
            asserts.assert_equal(e3.latestValue.fabricIndex, f1, "LatestValue.FabricIndex should be the current fabric index")
            asserts.assert_equal(e3.fabricIndex, f1, "FabricIndex should be the current fabric index")

        else:
            asserts.assert_true(len(read_events) == 2, f"Expected 2 events from read, but got {len(read_events)}")

            e0, e1 = read_events
            # First event: changed for admin
            asserts.assert_equal(e0.changeType,
                                 Clusters.AccessControl.Enums.ChangeTypeEnum.kChanged,
                                 "Expected Changed change type for first event")
            asserts.assert_equal(e0.adminNodeID, self.default_controller.nodeId, "AdminNodeID should be the controller node ID")
            asserts.assert_in('chip.clusters.Types.Nullable', str(type(e0.adminPasscodeID)), "AdminPasscodeID should be Null")
            asserts.assert_equal(e0.latestValue, acl_entries[0], "First event LatestValue should match admin ACL entry")
            asserts.assert_equal(e0.latestValue.fabricIndex, f1, "LatestValue.FabricIndex should be the current fabric index")
            asserts.assert_equal(e0.fabricIndex, f1, "FabricIndex should be the current fabric index")

            # Second event: added for operate/group
            asserts.assert_equal(e1.changeType,
                                 Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded,
                                 "Expected Added change type for second event")
            asserts.assert_equal(e1.adminNodeID, self.default_controller.nodeId, "AdminNodeID should be the controller node ID")
            asserts.assert_in('chip.clusters.Types.Nullable', str(type(e1.adminPasscodeID)), "AdminPasscodeID should be Null")
            asserts.assert_equal(e1.latestValue, acl_entries[1], "Second event LatestValue should match operate/group ACL entry")
            asserts.assert_equal(e1.latestValue.fabricIndex, f1, "LatestValue.FabricIndex should be the current fabric index")
            asserts.assert_equal(e1.fabricIndex, f1, "FabricIndex should be the current fabric index")

        # Set comparison for debugging
        subscription_event_set = set(self.event_key(e) for e in received_subscription_events)
        read_event_set = set(self.event_key(e) for e in read_events)
        if subscription_event_set != read_event_set:
            sub_only = subscription_event_set - read_event_set
            read_only = read_event_set - subscription_event_set
            if sub_only:
                logging.error(f"Events only in subscription: {sub_only}")
            if read_only:
                logging.error(f"Events only in read: {read_only}")
        asserts.assert_equal(subscription_event_set, read_event_set, "Subscription and read events should match")

        # Only fail if read events are missing expected entries
        expected_event_keys = set(self.event_key(Clusters.AccessControl.Events.AccessControlEntryChanged(
            adminNodeID=NullValue,
            adminPasscodeID=0,
            changeType=Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded,
            latestValue=acl_entry,
            fabricIndex=f1
        )) for acl_entry in acl_entries)
        read_has_expected = expected_event_keys.issubset(read_event_set)
        asserts.assert_equal(
            read_has_expected,
            True,
            f"Read events missing expected entries: {expected_event_keys - read_event_set}")

        self.step(6)
        # Write invalid ACL attribute
        invalid_acl_entries = [
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
            ),
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=[0],
                targets=NullValue,
            )
        ]

        result = await self.write_attribute_with_encoding_option(
            self.th1,
            self.dut_node_id,
            [(0, acl_attr(value=invalid_acl_entries))],
            forceLegacyListEncoding=force_legacy_encoding
        )
        asserts.assert_equal(result[0].Status, Status.ConstraintError, "Write should have failed with CONSTRAINT_ERROR")

        self.step(7)
        # Verify no events for invalid entry via read as well
        latest_event_number = await self.get_latest_event_number(acec_event)
        events_response = await self.th1.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True,
            eventNumberFilter=latest_event_number + 1
        )

        # Check if any of the read events correspond to the invalid entry
        invalid_entry = invalid_acl_entries[1]
        for event in events_response:
            if (event.Data.latestValue.authMode == invalid_entry.authMode and
                    event.Data.latestValue.subjects == invalid_entry.subjects):
                asserts.fail(f"Found event for invalid entry in read response: {event.Data}")

        logging.info("No events found for invalid entry, as expected")

        self.step(8)
        if force_legacy_encoding:
            logging.info("Rerunning test with new list method")

    def steps_TC_ACL_2_6(self) -> list[TestStep]:
        steps = [
            TestStep(
                1,
                "TH1 commissions DUT using admin node ID N1",
                "DUT is commissioned on TH1 fabric",
                is_commissioning=True),
            TestStep(
                2,
                "TH1 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                "Result is SUCCESS, value is stored as F1"),
            TestStep(
                3,
                "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event",
                "Result is SUCCESS value is list of AccessControlEntryChangedEvent events containing 1 element"),
            TestStep(
                4,
                "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is list of AccessControlEntryStruct containing 2 elements",
                "Result is SUCCESS"),
            TestStep(
                5,
                "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event",
                "Result is SUCCESS, value is list of AccessControlEntryChanged events containing 2 new elements if new write list method is used, else then the legacy list method is used there should be 3 new elements"),
            TestStep(
                6,
                "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is list of AccessControlEntryStruct containing 2 elements. The first item is valid, the second item is invalid due to group ID 0 being used, which is illegal.",
                "Result is CONSTRAINT_ERROR"),
            TestStep(
                7,
                "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event",
                "value MUST NOT contain an AccessControlEntryChanged entry corresponding to the second invalid entry in step 6."),
            TestStep(
                8,
                "Rerunning test steps with new list method",
                "Rerunning test steps with new list method"),
        ]
        return steps

    @async_test_body
    async def test_TC_ACL_2_6(self):
        # First run with new list encoding
        await self.internal_test_TC_ACL_2_6(force_legacy_encoding=True)

        # Reset step counter and run second test with legacy list encoding
        self.current_step_index = 0
        logging.info("Starting second test run with new encoding")
        await self.internal_test_TC_ACL_2_6(force_legacy_encoding=False)


if __name__ == "__main__":
    default_matter_test_main()
