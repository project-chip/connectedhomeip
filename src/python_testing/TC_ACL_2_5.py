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

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import has_attribute, run_if_endpoint_matches
from matter.testing.event_attribute_reporting import EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_ACL_2_5(MatterBaseTest):
    async def write_attribute_with_encoding_option(self, controller, node_id, path, forceLegacyListEncoding):
        if forceLegacyListEncoding:
            return await controller.TestOnlyWriteAttributeWithLegacyList(node_id, path)
        return await controller.WriteAttribute(node_id, path)

    async def internal_test_TC_ACL_2_5(self, force_legacy_encoding: bool):
        self.step(1)
        self.th1 = self.default_controller
        self.endpoint = self.get_endpoint()

        self.step(2)
        oc_cluster = Clusters.OperationalCredentials
        cfi_attribute = oc_cluster.Attributes.CurrentFabricIndex
        f1 = await self.read_single_attribute_check_success(endpoint=0, cluster=oc_cluster, attribute=cfi_attribute)
        extension_attr = Clusters.AccessControl.Attributes.Extension

        self.step(3)
        # Read initial AccessControlClusterExtension
        acec_event = Clusters.AccessControl.Events.AccessControlExtensionChanged
        events_response = await self.th1.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True
        )
        log.info(f"Initial events response {str(events_response)}")

        # Extract events from the response
        log.info(f"Found {len(events_response)} initial events")
        if not force_legacy_encoding:
            # if new list method is used we will have 0 events, however since we created events during prior run with new list method then we would have events already
            asserts.assert_equal(len(events_response), 0, "Expected 0 events")

        # Set up event subscription before making change
        log.info("Setting up event subscription...")
        events_callback = EventSubscriptionHandler(expected_cluster=Clusters.AccessControl)
        await events_callback.start(self.default_controller, self.dut_node_id, 0)

        self.step(4)
        # Create an extension with a test string (test string specified by test plan)
        # 1718 = octstr of length 2 containing valid TLC for a top level anonymous list (empty)
        D_OK_EMPTY = bytes.fromhex('1718')
        extension = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_OK_EMPTY)

        # Write the extension to the device - properly wrap the extensions list
        log.info(f"Writing extension with data {D_OK_EMPTY.hex()}")
        extensions_list = [extension]
        result = await self.write_attribute_with_encoding_option(
            self.default_controller,
            self.dut_node_id,
            [(0, extension_attr(value=extensions_list))],
            forceLegacyListEncoding=force_legacy_encoding
        )
        log.info(f"Write result {str(result)}")
        asserts.assert_equal(
            result[0].Status, Status.Success, "Write should have succeeded")

        self.step(5)
        # Wait for and verify the event from subscription
        log.info("Waiting for AccessControlExtensionChanged event from subscription...")
        subscription_event = events_callback.wait_for_event_report(acec_event, timeout_sec=15)

        # Read the event directly
        log.info("Reading event directly...")
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

        # Verify both methods return the same event data
        log.info(f"Comparing subscription event: {subscription_event} with direct event: {direct_event}")
        asserts.assert_equal(subscription_event, direct_event.Data, "Subscription event should be in direct event")

        asserts.assert_equal(subscription_event.changeType,
                             Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded,
                             "Expected Added change type")

        asserts.assert_equal(subscription_event.latestValue.data,
                             D_OK_EMPTY,
                             "LatestValue.Data should be D_OK_EMPTY")
        asserts.assert_in('matter.clusters.Types.Nullable', str(type(subscription_event.adminPasscodeID)),
                          "AdminPasscodeID should be Null")
        asserts.assert_equal(subscription_event.adminNodeID,
                             self.default_controller.nodeId,
                             "AdminNodeID should be the controller node ID")
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
        log.info(f"Writing new extension with data {D_OK_SINGLE.hex()}")
        extensions_list = [new_extension]
        result = await self.write_attribute_with_encoding_option(
            self.default_controller,
            self.dut_node_id,
            [(0, extension_attr(value=extensions_list))],
            forceLegacyListEncoding=force_legacy_encoding
        )
        log.info(f"Write result: {result}")
        asserts.assert_equal(
            result[0].Status, Status.Success, "Write should have succeeded")

        self.step(7)
        # Wait for and verify the event
        log.info("Waiting for AccessControlExtensionChanged event...")
        event_data1 = events_callback.wait_for_event_report(acec_event, timeout_sec=15)

        if not force_legacy_encoding:
            # Verify event data
            asserts.assert_equal(event_data1.changeType,
                                 Clusters.AccessControl.Enums.ChangeTypeEnum.kChanged,
                                 "Expected Changed change type")
            asserts.assert_in('matter.clusters.Types.Nullable', str(type(event_data1.adminPasscodeID)),
                              "AdminPasscodeID should be Null")
            asserts.assert_equal(event_data1.adminNodeID,
                                 self.default_controller.nodeId,
                                 "AdminNodeID should be the controller node ID")
            asserts.assert_equal(event_data1.latestValue.data,
                                 D_OK_SINGLE,
                                 "LatestValue.Data should match D_OK_SINGLE")
            asserts.assert_equal(event_data1.latestValue.fabricIndex,
                                 f1,
                                 "LatestValue.FabricIndex should be the current fabric index")
            asserts.assert_equal(event_data1.fabricIndex,
                                 f1,
                                 "FabricIndex should be the current fabric index")

        if force_legacy_encoding:
            # Verify event data 1 struct
            asserts.assert_equal(event_data1.changeType,
                                 Clusters.AccessControl.Enums.ChangeTypeEnum.kRemoved,
                                 "Expected Removed change type")
            asserts.assert_in('matter.clusters.Types.Nullable', str(type(event_data1.adminPasscodeID)),
                              "AdminPasscodeID should be Null")
            asserts.assert_equal(event_data1.adminNodeID,
                                 self.default_controller.nodeId,
                                 "AdminNodeID should be the controller node ID")
            asserts.assert_equal(event_data1.latestValue.data,
                                 D_OK_EMPTY,
                                 "LatestValue.Data should match D_OK_EMPTY")
            asserts.assert_equal(event_data1.latestValue.fabricIndex,
                                 f1,
                                 "LatestValue.FabricIndex should be the current fabric index")
            asserts.assert_equal(event_data1.fabricIndex,
                                 f1,
                                 "FabricIndex should be the current fabric index")

            event_data2 = events_callback.wait_for_event_report(acec_event, timeout_sec=15)
            # Verify event data 2 struct
            asserts.assert_equal(event_data2.changeType,
                                 Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded,
                                 "Expected Added change type")
            asserts.assert_in('matter.clusters.Types.Nullable', str(type(event_data2.adminPasscodeID)),
                              "AdminPasscodeID should be Null")
            asserts.assert_equal(event_data2.adminNodeID,
                                 self.default_controller.nodeId,
                                 "AdminNodeID should be the controller node ID")
            asserts.assert_equal(event_data2.latestValue.data,
                                 D_OK_SINGLE,
                                 "LatestValue.Data should match D_OK_SINGLE")
            asserts.assert_equal(event_data2.latestValue.fabricIndex,
                                 f1,
                                 "LatestValue.FabricIndex should be the current fabric index")
            asserts.assert_equal(event_data2.fabricIndex,
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
        log.info("Attempting to write extension that exceeds max length (should fail)")
        extensions_list = [too_long_extension]
        a = await self.write_attribute_with_encoding_option(
            self.default_controller,
            self.dut_node_id,
            [(0, extension_attr(value=extensions_list))],
            forceLegacyListEncoding=force_legacy_encoding
        )
        log.info(f"Write result {str(a)}")
        asserts.assert_equal(a[0].Status, Status.ConstraintError,
                             "Write should have failed with CONSTRAINT_ERROR 135")

        self.step(9)
        # Verify no event was generated for the failed write
        log.info("Reading events after failed write (too long extension)...")

        latest_event_num = await self.get_latest_event_number(acec_event)

        # Try to read events directly
        events_response2 = await self.default_controller.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True,
            eventNumberFilter=latest_event_num + 1
        )
        log.info(f"Events response {str(events_response2)}")

        # Extract events from the response
        log.info(f"Found {len(events_response2)} events")
        if not force_legacy_encoding:
            asserts.assert_equal(len(events_response2), 0, "There should be no events found")

        if force_legacy_encoding:
            event_data3 = events_callback.wait_for_event_report(acec_event, timeout_sec=15)
            # Verify event data 1 struct
            asserts.assert_equal(event_data3.changeType,
                                 Clusters.AccessControl.Enums.ChangeTypeEnum.kRemoved,
                                 "Expected Removed change type")
            asserts.assert_in('matter.clusters.Types.Nullable', str(type(event_data3.adminPasscodeID)),
                              "AdminPasscodeID should be Null")
            asserts.assert_equal(event_data3.adminNodeID,
                                 self.default_controller.nodeId,
                                 "AdminNodeID should be the controller node ID")
            asserts.assert_equal(event_data3.latestValue.data,
                                 D_OK_SINGLE,
                                 "LatestValue.Data should match D_OK_SINGLE")
            asserts.assert_equal(event_data3.latestValue.fabricIndex,
                                 f1,
                                 "LatestValue.FabricIndex should be the current fabric index")
            asserts.assert_equal(event_data3.fabricIndex,
                                 f1,
                                 "FabricIndex should be the current fabric index")

        self.step(10)
        # This should fail with CONSTRAINT_ERROR
        log.info("Attempting to write multiple extensions (should fail)")
        extensions_list = [extension, new_extension]
        b = await self.write_attribute_with_encoding_option(
            self.default_controller,
            self.dut_node_id,
            [(0, extension_attr(value=extensions_list))],
            forceLegacyListEncoding=force_legacy_encoding
        )
        log.info(f"Write result {str(b)}")
        asserts.assert_equal(b[0].Status, Status.ConstraintError,
                             "Write should have failed with CONSTRAINT_ERROR")

        self.step(11)
        # Verify no event was generated at all, since the whole extensions list was rejected.
        log.info("Reading events after failed write (multiple extensions)...")

        latest_event_num2 = await self.get_latest_event_number(acec_event)

        events_response3 = await self.default_controller.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True,
            eventNumberFilter=latest_event_num2 + 1
        )
        log.info(f"Events response {str(events_response3)}")

        # Extract events from the response
        log.info(f"Found {len(events_response3)} events")
        if not force_legacy_encoding:
            asserts.assert_equal(len(events_response3), 0, "There should be no events found")
        else:
            event_data4 = events_callback.wait_for_event_report(acec_event, timeout_sec=15)
            # Verify event data 2 struct
            asserts.assert_equal(event_data4.changeType,
                                 Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded,
                                 "Expected Added change type")
            asserts.assert_in('matter.clusters.Types.Nullable', str(type(event_data4.adminPasscodeID)),
                              "AdminPasscodeID should be Null")
            asserts.assert_equal(event_data4.adminNodeID,
                                 self.default_controller.nodeId,
                                 "AdminNodeID should be the controller node ID")
            asserts.assert_equal(event_data4.latestValue.data,
                                 D_OK_EMPTY,
                                 "LatestValue.Data should match D_OK_EMPTY")
            asserts.assert_equal(event_data4.latestValue.fabricIndex,
                                 f1,
                                 "LatestValue.FabricIndex should be the current fabric index")
            asserts.assert_equal(event_data4.fabricIndex,
                                 f1,
                                 "FabricIndex should be the current fabric index")

        self.step(12)
        # Write an empty list to clear all extensions
        log.info("Writing empty extension list to clear all extensions...")
        extensions_list2 = []
        result = await self.write_attribute_with_encoding_option(
            self.default_controller,
            self.dut_node_id,
            [(0, extension_attr(value=extensions_list2))],
            forceLegacyListEncoding=force_legacy_encoding
        )
        log.info(f"Write result {str(result)}")
        asserts.assert_equal(
            result[0].Status, Status.Success, "Write should have succeeded")

        self.step(13)
        log.info("Waiting for AccessControlExtensionChanged event...")
        event_data5 = events_callback.wait_for_event_report(acec_event, timeout_sec=15)

        if not force_legacy_encoding:
            # Verify event data
            asserts.assert_equal(event_data5.latestValue.data,
                                 D_OK_SINGLE,
                                 "LatestValue.Data should match D_OK_SINGLE")

        if force_legacy_encoding:
            # Verify event data 1 struct
            asserts.assert_equal(event_data5.latestValue.data,
                                 D_OK_EMPTY,
                                 "LatestValue.Data should match D_OK_EMPTY")

        asserts.assert_in('matter.clusters.Types.Nullable', str(type(event_data5.adminPasscodeID)),
                          "AdminPasscodeID should be Null")

        asserts.assert_equal(event_data5.changeType,
                             Clusters.AccessControl.Enums.ChangeTypeEnum.kRemoved,
                             "Expected Removed change type")

        asserts.assert_equal(event_data5.adminNodeID,
                             self.default_controller.nodeId,
                             "AdminNodeID should be the controller node ID")

        asserts.assert_equal(event_data5.latestValue.fabricIndex,
                             f1,
                             "LatestValue.FabricIndex should be the current fabric index")

        asserts.assert_equal(event_data5.fabricIndex,
                             f1,
                             "FabricIndex should be the current fabric index")

        # Rerunning test using the legacy list writing mechanism
        if not force_legacy_encoding:
            self.step(14)
            log.info("*** Rerunning test using the legacy list writing mechanism now ***")
        else:
            self.skip_step(14)

    async def get_latest_event_number(self, acec_event: Clusters.AccessControl.Events.AccessControlExtensionChanged) -> int:
        event_path = [(self.matter_test_config.endpoint, acec_event, 1)]
        events = await self.default_controller.ReadEvent(nodeId=self.dut_node_id, events=event_path)
        return max([e.Header.EventNumber for e in events])

    def pics_TC_ACL_2_5(self) -> list[str]:
        return ['ACL.S.A0001']

    def desc_TC_ACL_2_5(self) -> str:
        return "[TC-ACL-2.5]  AccessControlExtensionChanged event"

    def steps_TC_ACL_2_5(self) -> list[TestStep]:
        return [
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
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing 1 element if new list method is used; If legacy list method is used, the event should be REMOVED and ADDED"),
            TestStep(8, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element",
                     "Result is 0x87 (CONSTRAINT_ERROR)-Data value exceeds maximum length."),
            TestStep(9, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event",
                     "Result is SUCCESS, value is empty list (received ReportData Message should have no/empty EventReportIB list) if new list method is used; If legacy list method is used, the event should be REMOVED"),
            TestStep(10, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 2 elements",
                     "Result is 0x87 (CONSTRAINT_ERROR)-as there are more than 1 entry associated with the given accessing fabric index in the extension list"),
            TestStep(11, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event",
                     "Result is SUCCESS, value is empty list (received ReportData Message should have no/empty EventReportIB list) since the entire list of Test Step 10 was rejected if new write list method is used, else then the legacy list method is used and the event ADDED."),
            TestStep(12, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is an empty list", "Result is SUCCESS"),
            TestStep(13, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event",
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing at least 1 new element if new write list method is used LatestValue Field should be D_OK_EMPTY, else then the legacy list method is used value should be D_OK_SINGLE."),
            TestStep(14, "Re-run the test using the legacy list writing mechanism, where the client issues a series of AttributeDataIBs, with the first containing a path to the list itself and Data that is empty array, which signals clearing the list, and subsequent AttributeDataIBs containing updates.",
                     "Test succeeds with legacy list encoding mechanism"),
        ]

    @run_if_endpoint_matches(has_attribute(Clusters.AccessControl.Attributes.Extension))
    async def test_TC_ACL_2_5(self):
        await self.internal_test_TC_ACL_2_5(force_legacy_encoding=False)
        self.current_step_index = 0
        await self.internal_test_TC_ACL_2_5(force_legacy_encoding=True)


if __name__ == "__main__":
    default_matter_test_main()
