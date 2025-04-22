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

import asyncio
import logging
import random

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_ACL_2_6(MatterBaseTest):
    def desc_TC_ACL_2_6(self) -> str:
        return "[TC-ACL-2.6] AccessControlEntryChanged event"

    def steps_TC_ACL_2_6(self) -> list[TestStep]:
        steps = [
            TestStep(1, "TH1 commissions DUT using admin node ID N1", "DUT is commissioned on TH1 fabric", is_commissioning=True),
            TestStep(2, "TH1 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                     "Result is SUCCESS, value is stored as F1"),
            TestStep(3, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event",
                     "Result is SUCCESS value is list of AccessControlEntryChangedEvent events containing 1 element"),
            TestStep(4, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is list of AccessControlEntryStruct containing 2 elements", "Result is SUCCESS"),
            TestStep(5, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event",
                     "Result is SUCCESS, value is list of AccessControlEntryChanged events containing 2 new elements"),
            TestStep(6, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is list of AccessControlEntryStruct containing 2 elements. The first item is valid, the second item is invalid due to group ID 0 being used, which is illegal.", "Result is CONSTRAINT_ERROR"),
            TestStep(7, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event",
                     "value MUST NOT contain an AccessControlEntryChanged entry corresponding to the second invalid entry in step 6."),
        ]
        return steps

    @async_test_body
    async def test_TC_ACL_2_6(self):
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
        acec_event = Clusters.AccessControl.Events.AccessControlEntryChanged
        events_response = await self.th1.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True
        )
        asserts.assert_equal(len(events_response), 1, "Expected 1 event")

        found_initial_event = False
        for event_data in events_response:
            logging.info(f"Examining initial event {str(event_data)}")

            if hasattr(event_data, 'Data') and hasattr(event_data.Data, 'changeType'):
                if (event_data.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
                    event_data.Data.adminNodeID is NullValue and
                    event_data.Data.adminPasscodeID == 0 and
                    event_data.Data.fabricIndex == f1 and
                        hasattr(event_data.Data, 'latestValue')):

                    latest_value = event_data.Data.latestValue
                    # Check for initial admin entry created during commissioning
                    if (latest_value.privilege == Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister and
                        latest_value.authMode == Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase and
                        self.th1.nodeId in latest_value.subjects and
                        latest_value.targets is NullValue and
                            latest_value.fabricIndex == f1):
                        found_initial_event = True
                        logging.info("Found initial admin ACL entry event")
                        break

        logging.info(f"Initial event search result Found: {found_initial_event}")
        asserts.assert_true(found_initial_event, "Did not find expected initial ACL event")

        event_path = [(self.matter_test_config.endpoint, acec_event, 1)]
        initial_events = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path)
        initial_event_num = [e.Header.EventNumber for e in initial_events]

        self.step(4)
        # Write ACL attribute
        acl_entries = [
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
            ),
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId],
                targets=NullValue,
            )
        ]

        acl_attr = Clusters.AccessControl.Attributes.Acl
        result = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attr(value=acl_entries))]
        )
        asserts.assert_equal(result[0].Status, Status.Success, "Write should have succeeded")

        await asyncio.sleep(1)

        self.step(5)
        # Create correct event path with endpoint 0
        events_response2 = await self.default_controller.ReadEvent(
            nodeid=self.dut_node_id,
            events=event_path,
            fabricFiltered=True,
            eventNumberFilter=max(initial_event_num)+1
        )

        logging.info(f"Events response: {events_response2}")
        logging.info(f"Event response length: {len(events_response2)}")
        asserts.assert_true(len(events_response2) == 2, "Expected 2 events")

        found_view_entry = False
        found_changed_entry = False

        for event_data in events_response2:
            logging.info(f"Examining event {str(event_data)}")
            # Check for changed admin entry (first event)
            if (event_data.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kChanged and
                event_data.Data.adminNodeID == self.th1.nodeId and
                event_data.Data.adminPasscodeID is NullValue and
                    hasattr(event_data.Data, 'latestValue')):

                latest_value = event_data.Data.latestValue
                if (latest_value.privilege == Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister and
                    latest_value.authMode == Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase and
                    self.th1.nodeId in latest_value.subjects and
                        latest_value.targets is NullValue):
                    found_changed_entry = True
                    logging.info("Found changed admin ACL entry event")

            # Check for added view entry (second event)
            elif (event_data.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
                    event_data.Data.adminNodeID == self.th1.nodeId and
                    event_data.Data.adminPasscodeID is NullValue and
                    hasattr(event_data.Data, 'latestValue')):

                latest_value = event_data.Data.latestValue
                if (latest_value.privilege == Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView and
                    latest_value.authMode == Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase and
                    self.th1.nodeId in latest_value.subjects and
                        latest_value.targets is NullValue):
                    found_view_entry = True
                    logging.info("Found view ACL entry event")

        asserts.assert_true(found_changed_entry, "Did not find changed admin ACL entry event")
        asserts.assert_true(found_view_entry, "Did not find view ACL entry event")
        asserts.assert_true(len(events_response2) == 2, f"Expected 2 events, found {len(events_response2)}")

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
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kView,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=[0],  # Invalid group ID
                targets=NullValue,
            )
        ]

        result = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attr(value=invalid_acl_entries))]
        )
        asserts.assert_equal(result[0].Status, Status.ConstraintError, "Write should have failed with CONSTRAINT_ERROR")

        self.step(7)
        # Read AccessControlEntryChanged event again
        events_response = await self.th1.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True
        )

        found_invalid_event = False
        for event in events_response:
            if (hasattr(event, 'Data') and
                hasattr(event.Data, 'subjects') and
                    0 in event.Data.subjects):
                found_invalid_event = True
                break
        asserts.assert_false(found_invalid_event, "Should not find event for invalid entry")


if __name__ == "__main__":
    default_matter_test_main()
