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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
# === END CI TEST ARGUMENTS ===


import logging
import random

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.clusters.Types import Nullable
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

# These below variables are used to test the AccessControl cluster Extension attribute and come from the test plan here: https://github.com/CHIP-Specifications/chip-test-plans/blob/59e8c45b8e7c24d5ce130b166520ff4f7bd935b6/src/cluster/AccessControl.adoc#tc-acl-2-6-accesscontrolentrychanged-event:~:text=D_OK_EMPTY%3A%20%221718%22%20which%20is%20an%20octstr%20of%20length%202%20containing%20valid%20TLV%3A
D_OK_EMPTY = bytes.fromhex('1718')
D_OK_SINGLE = bytes.fromhex(
    '17D00000F1FF01003D48656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E74206C6976696E6720617320612063686172737472696E670018')


class TC_ACL_2_7(MatterBaseTest):
    async def read_currentfabricindex(self, th: ChipDeviceCtrl) -> int:
        cluster = Clusters.Objects.OperationalCredentials
        attribute = Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        current_fabric_index = await self.read_single_attribute_check_success(dev_ctrl=th, endpoint=0, cluster=cluster, attribute=attribute)
        return current_fabric_index

    def desc_TC_ACL_2_7(self) -> str:
        return "[TC-ACL-2.7] Multiple fabrics test"

    def steps_TC_ACL_2_7(self) -> list[TestStep]:
        steps = [
            TestStep(1, "TH1 commissions DUT using admin node ID",
                     is_commissioning=True),
            TestStep(2, "TH1 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                     "Result is SUCCESS, value is stored as F1"),
            TestStep(3, "TH1 puts DUT into commissioning mode, TH2 commissions DUT using admin node ID",
                     "DUT is commissioned on TH2 fabric"),
            TestStep(4, "TH2 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                     "Result is SUCCESS, value is stored as F2"),
            TestStep(5, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element", "Result is SUCCESS"),
            TestStep(6, "TH2 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element", "Result is SUCCESS"),
            TestStep(7, "TH1 reads DUT Endpoint 0 AccessControl cluster Extension attribute",
                     "Result is SUCCESS, value is list of AccessControlExtensionStruct containing 1 element"),
            TestStep(8, "TH2 reads DUT Endpoint 0 AccessControl cluster Extension attribute",
                     "Result is SUCCESS, value is list of AccessControlExtensionStruct containing 1 element"),
            TestStep(9, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event",
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing 1 element"),
            TestStep(10, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event",
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing 1 element"),
            TestStep(11, "TH_CR1 sends RemoveFabric for TH2 fabric command to DUT_CE",
                     "Verify DUT_CE responses with NOCResponse with a StatusCode OK"),
        ]
        return steps

    @async_test_body
    async def test_TC_ACL_2_7(self):
        self.step(1)
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)

        self.step(2)
        # Read CurrentFabricIndex for TH1
        oc_cluster = Clusters.OperationalCredentials
        cfi_attribute = oc_cluster.Attributes.CurrentFabricIndex
        f1 = await self.read_single_attribute_check_success(endpoint=0, cluster=oc_cluster, attribute=cfi_attribute)
        logging.info("CurrentFabricIndex F1: %s", str(f1))

        self.step(3)
        # TH1 puts DUT into commissioning mode, TH2 is created and commissions DUT using admin node ID
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
        # Read CurrentFabricIndex again after TH2 commissioning
        f2 = await self.read_single_attribute_check_success(dev_ctrl=self.th2, endpoint=0, cluster=oc_cluster, attribute=cfi_attribute)
        logging.info("CurrentFabricIndex F2: %s", str(f2))

        self.step(5)
        # TH1 writes Extension attribute with D_OK_EMPTY
        extension = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_OK_EMPTY)
        logging.info(f"TH1 writing extension with data {D_OK_EMPTY.hex()}")

        extension_attr = Clusters.AccessControl.Attributes.Extension
        extensions_list = [extension]
        result = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, extension_attr(value=extensions_list))]
        )
        logging.info("Write result: %s", str(result))
        asserts.assert_equal(
            result[0].Status, Status.Success, "Write should have succeeded")

        self.step(6)
        # TH2 writes Extension attribute with D_OK_SINGLE
        extension_th2 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_OK_SINGLE)
        logging.info(f"TH2 writing extension with data {D_OK_SINGLE.hex()}")

        extension_attr = Clusters.AccessControl.Attributes.Extension
        extensions_list = [extension_th2]
        result = await self.th2.WriteAttribute(
            self.dut_node_id,
            [(0, extension_attr(value=extensions_list))]
        )
        logging.info("Write result: %s", str(result))
        asserts.assert_equal(
            result[0].Status, Status.Success, "Write should have succeeded")

        self.step(7)
        # TH1 reads Extension attribute
        ac_cluster = Clusters.AccessControl
        extension_attr = Clusters.AccessControl.Attributes.Extension
        result = await self.read_single_attribute_check_success(dev_ctrl=self.th1, endpoint=0, cluster=ac_cluster, attribute=extension_attr)
        logging.info("TH1 read result: %s", str(result))
        asserts.assert_equal(len(result), 1,
                             "Should have exactly one extension")

        endpoint_data = result[0]
        logging.info("TH1 endpoint data: %s", str(endpoint_data))

        asserts.assert_equal(
            endpoint_data.data, D_OK_EMPTY, "Extension data should match D_OK_EMPTY")

        asserts.assert_equal(endpoint_data.fabricIndex,
                             f1, "FabricIndex should match F1")

        self.step(8)
        # TH2 reads Extension attribute
        extension_attr = Clusters.AccessControl.Attributes.Extension
        result2 = await self.read_single_attribute_check_success(dev_ctrl=self.th2, endpoint=0, cluster=ac_cluster, attribute=extension_attr)
        logging.info("TH2 read result: %s", str(result2))
        asserts.assert_equal(len(result2), 1,
                             "Should have exactly one extension")

        endpoint_data2 = result2[0]

        asserts.assert_equal(
            endpoint_data2.data, D_OK_SINGLE, "Extension data should match D_OK_SINGLE")
        asserts.assert_equal(endpoint_data2.fabricIndex, f2, "FabricIndex should match F2")

        self.step(9)
        # TH1 reads AccessControlExtensionChanged event
        acec_event = Clusters.AccessControl.Events.AccessControlExtensionChanged
        logging.info(
            "TH1 reading AccessControlExtensionChanged events...")

        events_response = await self.th1.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True
        )
        logging.info("Events response: %s", str(events_response))

        events = events_response
        logging.info("Found %d events", len(events))

        found_valid_event = False
        found_th2_event = False

        for event_data in events:
            logging.info("Examining event: %s", str(event_data))

            if hasattr(event_data, 'Data') and hasattr(event_data.Data, 'changeType'):
                # Check for valid TH1 event
                if (event_data.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
                    event_data.Data.adminNodeID == self.th1.nodeId and
                    isinstance(event_data.Data.adminPasscodeID, Nullable) and
                    event_data.Data.latestValue.data == D_OK_EMPTY and
                    event_data.Data.latestValue.fabricIndex == f1 and
                        event_data.Data.fabricIndex == f1):
                    found_valid_event = True
                    logging.info("Found valid event for TH1")

                # Check for TH2 events that shouldn't be visible to TH1
                if (event_data.Data.adminNodeID == self.th2.nodeId or
                    event_data.Data.latestValue.fabricIndex == f2 or
                        event_data.Data.fabricIndex == f2):
                    found_th2_event = True
                    logging.info("Found TH2 event visible to TH1, which violates fabric isolation")

        asserts.assert_true(
            found_valid_event, "Did not find the expected event with specified fields for TH1")
        asserts.assert_false(
            found_th2_event, "TH1 should not see any events from TH2's fabric due to fabric isolation")

        self.step(10)
        # TH2 reads AccessControlExtensionChanged event
        events_response = await self.th2.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True
        )
        logging.info("Events response: %s", str(events_response))

        events = events_response
        logging.info("Found %d events", len(events))

        found_valid_event = False
        found_th1_event = False

        for event_data in events:
            logging.info("Examining event: %s", str(event_data))

            if hasattr(event_data, 'Data') and hasattr(event_data.Data, 'changeType'):
                # Check for valid TH2 event
                if (event_data.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
                    event_data.Data.adminNodeID == self.th2.nodeId and
                    isinstance(event_data.Data.adminPasscodeID, Nullable) and
                    event_data.Data.latestValue.data == D_OK_SINGLE and
                    event_data.Data.latestValue.fabricIndex == f2 and
                        event_data.Data.fabricIndex == f2):
                    found_valid_event = True
                    logging.info("Found valid event for TH2")

                # Check for TH1 events that shouldn't be visible to TH2
                if (event_data.Data.adminNodeID == self.th1.nodeId or
                    event_data.Data.latestValue.fabricIndex == f1 or
                        event_data.Data.fabricIndex == f1):
                    found_th1_event = True
                    logging.info("Found TH1 event visible to TH2, which violates fabric isolation")

        asserts.assert_true(
            found_valid_event, "Did not find the expected event with specified fields for TH2")
        asserts.assert_false(
            found_th1_event, "TH2 should not see any events from TH1's fabric due to fabric isolation")

        self.step(11)
        # TH_CR1 sends RemoveFabric for TH2 fabric command to DUT_CE
        fabric_idx_cr2_2 = await self.read_currentfabricindex(th=self.th2)
        removeFabricCmd2 = Clusters.OperationalCredentials.Commands.RemoveFabric(fabric_idx_cr2_2)
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=removeFabricCmd2)


if __name__ == "__main__":
    default_matter_test_main()
