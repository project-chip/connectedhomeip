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
import random

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
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
                     "Result is SUCCESS, value is list of AccessControlEntryChanged events containing at least 3 new elements"),
            TestStep(6, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is list of AccessControlEntryStruct containing 2 elements. The first item is valid, the second item is invalid due to group ID 0 being used, which is illegal.", "Result is CONSTRAINT_ERROR"),
            TestStep(7, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlEntryChanged event",
                     "value is empty list (received ReportData Message should have no/empty EventReportIB list) since the entire list of Test Step 6 was rejected."),
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
        # Read AccessControlEntryChanged event
        acec_event = Clusters.AccessControl.Events.AccessControlEntryChanged
        events_response = await self.th1.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True
        )
        asserts.assert_equal(len(events_response), 1, "Should have exactly one initial event")

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
        # Read AccessControlEntryChanged event again
        events_response = await self.th1.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True
        )
        asserts.assert_true(len(events_response) >= 3, "Should have at least 3 events")

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

        try:
            result = await self.th1.WriteAttribute(
                self.dut_node_id,
                [(0, acl_attr(value=invalid_acl_entries))]
            )
            self.print_step("Write should have failed with CONSTRAINT_ERROR", str(result))
            asserts.fail("Write should have failed with CONSTRAINT_ERROR")
        except Exception as e:
            self.print_step("Expected CONSTRAINT_ERROR", str(e))
            asserts.assert_true("CONSTRAINT_ERROR" in str(e), "Expected CONSTRAINT_ERROR")

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
