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
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.event_attribute_reporting import EventChangeCallback
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_ACL_2_5(MatterBaseTest):
    async def get_latest_event_number(self, acec_event: Clusters.AccessControl.Events.AccessControlExtensionChanged) -> int:
        event_path = [(self.matter_test_config.endpoint, acec_event, 1)]
        events = await self.default_controller.ReadEvent(nodeid=self.dut_node_id, events=event_path)
        return max([e.Header.EventNumber for e in events])

    def desc_TC_ACL_2_5(self) -> str:
        return "[TC-ACL-2.5] Cluster endpoint"

    def steps_TC_ACL_2_5(self) -> list[TestStep]:
        steps = [
            TestStep(1, "TH1 commissions DUT using admin node ID",
                     is_commissioning=True),
            TestStep(2, "TH1 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                     "Result is SUCCESS, value is a valid index"),
            TestStep(3, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged events and create subscription for new events",
                     "Result is SUCCESS, value is empty list, new event subscription is created"),
            TestStep(4, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element", "Result is SUCCESS"),
            TestStep(5, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged events",
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing 1 element"),
            TestStep(6, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element", "Result is SUCCESS"),
            TestStep(7, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged events",
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing 1 element"),
            TestStep(8, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element",
                     "Result is 0x87 (CONSTRAINT_ERROR)-Data value exceeds maximum length."),
            TestStep(9, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event",
                     "Result is SUCCESS, value is empty list (received ReportData Message should have no/empty EventReportIB list)"),
            TestStep(10, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 2 elements",
                     "Result is 0x87 (CONSTRAINT_ERROR)-as there are more than 1 entry associated with the given accessing fabric index in the extension list"),
            TestStep(11, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event",
                     "Result is SUCCESS, value is empty list (received ReportData Message should have no/empty EventReportIB list) since the entire list of Test Step 10 was rejected."),
            TestStep(12, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is an empty list", "Result is SUCCESS"),
            TestStep(13, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event",
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing at least 1 new element"),
        ]
        return steps

    @async_test_body
    async def test_TC_ACL_2_5(self):
        self.step(1)
        self.th1 = self.default_controller

        self.step(2)
        oc_cluster = Clusters.OperationalCredentials
        cfi_attribute = oc_cluster.Attributes.CurrentFabricIndex
        f1 = await self.read_single_attribute_check_success(endpoint=0, cluster=oc_cluster, attribute=cfi_attribute)

        self.step(3)
        # Read initial AccessControlClusterExtension
        acec_event = Clusters.AccessControl.Events.AccessControlExtensionChanged
        events_response = await self.th1.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True
        )
        logging.info(f"Initial events response {str(events_response)}")

        # Extract events from the response
        logging.info(f"Found {len(events_response)} initial events")
        asserts.assert_equal(len(events_response), 0, "Expected 0 events")

        # Set up event subscription before making changes
        logging.info("Setting up event subscription...")
        events_callback = EventChangeCallback(Clusters.AccessControl)
        await events_callback.start(self.default_controller, self.dut_node_id, 0)

        self.step(4)
        # Create an extension with a test string (test string specified by test plan)
        # 1718 = octstr of length 2 containing valid TLC for a top level anonymous list (empty)
        D_OK_EMPTY = bytes.fromhex('1718')
        extension = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_OK_EMPTY)

        # Write the extension to the device - properly wrap the extensions list
        logging.info(f"Writing extension with data {D_OK_EMPTY.hex()}")
        extension_attr = Clusters.AccessControl.Attributes.Extension
        extensions_list = [extension]
        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, extension_attr(value=extensions_list))]
        )
        logging.info(f"Write result {str(result)}")
        asserts.assert_equal(
            result[0].Status, Status.Success, "Write should have succeeded")

        self.step(5)
        # Wait for and verify the event from subscription
        logging.info("Waiting for AccessControlExtensionChanged event from subscription...")
        subscription_event = events_callback.wait_for_event_report(acec_event, timeout_sec=15)

        # Read the event directly
        logging.info("Reading event directly...")
        latest_event_num = await self.get_latest_event_number(acec_event)
        direct_events = await self.default_controller.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True,
            eventNumberFilter=latest_event_num
        )

        # There should be exactly one event
        asserts.assert_equal(len(direct_events), 1, "Expected exactly one event from direct read")
        direct_event = direct_events[0]

        # Log the event structures to help debug
        logging.info(f"direct event: {direct_event}")
        logging.info(f"Direct event structure: {dir(direct_event)}")
        logging.info(f"Subscription event structure: {dir(subscription_event)}")

        # Verify both methods return the same event data
        logging.info(f"Comparing subscription event: {subscription_event} with direct event: {direct_event}")
        asserts.assert_equal(subscription_event, direct_event.Data, "Subscription event should be in direct event")

        # Verify the actual values
        asserts.assert_equal(subscription_event.changeType,
                             Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded,
                             "Expected Added change type")
        asserts.assert_in('chip.clusters.Types.Nullable', str(type(subscription_event.adminPasscodeID)),
                          "AdminPasscodeID should be Null")
        asserts.assert_equal(subscription_event.adminNodeID,
                             self.default_controller.nodeId,
                             "AdminNodeID should be the controller node ID")
        asserts.assert_equal(subscription_event.latestValue.data,
                             b'\x17\x18',
                             "LatestValue.Data should be 1718")
        asserts.assert_equal(subscription_event.latestValue.fabricIndex,
                             f1,
                             "LatestValue.FabricIndex should be the current fabric index")
        asserts.assert_equal(subscription_event.fabricIndex,
                             f1,
                             "FabricIndex should be the current fabric index")

        self.step(6)
        # Create a new extension with different data to replace the existing one
        # Value is from test plan and is a valid top level anonymous list containing one element.
        D_OK_SINGLE = bytes.fromhex(
            '17D00000F1FF01003D48656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E74206C6976696E6720617320612063686172737472696E670018'
        )
        new_extension = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_OK_SINGLE)

        # Write the new extension
        logging.info(f"Writing new extension with data {D_OK_SINGLE.hex()}")
        extensions_list = [new_extension]
        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, extension_attr(value=extensions_list))]
        )
        logging.info(f"Write result: {result}")
        asserts.assert_equal(
            result[0].Status, Status.Success, "Write should have succeeded")

        self.step(7)
        # Wait for and verify the event
        logging.info("Waiting for AccessControlExtensionChanged event...")
        event_data = events_callback.wait_for_event_report(acec_event, timeout_sec=15)

        # Verify event data
        asserts.assert_equal(event_data.changeType,
                             Clusters.AccessControl.Enums.ChangeTypeEnum.kChanged,
                             "Expected Changed change type")
        asserts.assert_in('chip.clusters.Types.Nullable', str(type(event_data.adminPasscodeID)),
                          "AdminPasscodeID should be Null")
        asserts.assert_equal(event_data.adminNodeID,
                             self.default_controller.nodeId,
                             "AdminNodeID should be the controller node ID")
        asserts.assert_equal(event_data.latestValue.data,
                             D_OK_SINGLE,
                             "LatestValue.Data should match D_OK_SINGLE")
        asserts.assert_equal(event_data.latestValue.fabricIndex,
                             f1,
                             "LatestValue.FabricIndex should be the current fabric index")
        asserts.assert_equal(event_data.fabricIndex,
                             f1,
                             "FabricIndex should be the current fabric index")

        self.step(8)
        # Try to write an extension that exceeds max length (128 bytes)
        # Value is from test plan and is a valid top-level anonymous list with a length of 129 (too long)
        too_long_data = bytes.fromhex(
            "17D00000F1FF01003D48656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E74206C6976696E6720617320612063686172737472696E6700D00000F1FF02003248656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E7420616761696E2E2E2E2E2E2E0018"
        )
        too_long_extension = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=too_long_data)

        # This should fail with CONSTRAINT_ERROR
        logging.info("Attempting to write extension that exceeds max length (should fail)")
        extensions_list = [too_long_extension]
        a = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, extension_attr(value=extensions_list))]
        )
        logging.info(f"Write result {str(a)}")
        asserts.assert_equal(a[0].Status, Status.ConstraintError,
                             "Write should have failed with CONSTRAINT_ERROR 135")

        self.step(9)
        # Verify no event was generated for the failed write
        logging.info("Reading events after failed write (too long extension)...")

        latest_event_num = await self.get_latest_event_number(acec_event)

        # Try to read events directly
        events_response2 = await self.default_controller.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True,
            eventNumberFilter=latest_event_num + 1
        )
        logging.info(f"Events response {str(events_response2)}")

        # Extract events from the response
        logging.info(f"Found {len(events_response2)} events")
        asserts.assert_equal(len(events_response2), 0, "There should be no events found")

        self.step(10)
        # This should fail with CONSTRAINT_ERROR
        logging.info("Attempting to write multiple extensions (should fail)")
        extensions_list = [extension, new_extension]
        b = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, extension_attr(value=extensions_list))]
        )
        logging.info(f"Write result {str(b)}")
        asserts.assert_equal(b[0].Status, Status.ConstraintError,
                             "Write should have failed with CONSTRAINT_ERROR")

        self.step(11)
        # Verify no event was generated at all, since the whole extensions list was rejected.
        logging.info("Reading events after failed write (multiple extensions)...")

        latest_event_num2 = await self.get_latest_event_number(acec_event)

        events_response3 = await self.default_controller.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True,
            eventNumberFilter=latest_event_num2 + 1
        )
        logging.info(f"Events response {str(events_response3)}")

        # Extract events from the response
        logging.info(f"Found {len(events_response3)} events")
        asserts.assert_equal(len(events_response3), 0, "There should be no events found")

        self.step(12)
        # Write an empty list to clear all extensions
        logging.info("Writing empty extension list to clear all extensions...")
        extensions_list2 = []
        result = await self.default_controller.WriteAttribute(
            self.dut_node_id,
            [(0, extension_attr(value=extensions_list2))]
        )
        logging.info(f"Write result {str(result)}")
        asserts.assert_equal(
            result[0].Status, Status.Success, "Write should have succeeded")

        self.step(13)
        logging.info("Waiting for AccessControlExtensionChanged event...")
        event_data = events_callback.wait_for_event_report(acec_event, timeout_sec=15)

        # Verify event data
        asserts.assert_equal(event_data.changeType,
                             Clusters.AccessControl.Enums.ChangeTypeEnum.kRemoved,
                             "Expected Removed change type")
        asserts.assert_in('chip.clusters.Types.Nullable', str(type(event_data.adminPasscodeID)),
                          "AdminPasscodeID should be Null")
        asserts.assert_equal(event_data.adminNodeID,
                             self.default_controller.nodeId,
                             "AdminNodeID should be the controller node ID")
        asserts.assert_equal(event_data.latestValue.data,
                             D_OK_SINGLE,
                             "LatestValue.Data should match D_OK_SINGLE")
        asserts.assert_equal(event_data.latestValue.fabricIndex,
                             f1,
                             "LatestValue.FabricIndex should be the current fabric index")
        asserts.assert_equal(event_data.fabricIndex,
                             f1,
                             "FabricIndex should be the current fabric index")

        # After all attempts, check if we found the valid event
        asserts.assert_true(
            True, "Did not find the expected REMOVE event with specified fields")
        logging.info("Successfully verified the expected REMOVE event")


if __name__ == "__main__":
    default_matter_test_main()
