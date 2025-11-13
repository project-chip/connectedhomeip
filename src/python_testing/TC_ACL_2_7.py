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

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import Nullable
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

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

    def _validate_events(self, events, expected_fabric_index, expected_node_id, other_fabric_index, controller_name, is_filtered):
        """Helper method to validate events for a TH"""
        logging.info(f"Found {len(events)} events for {controller_name}")

        found_valid_events = 0
        found_other_event = False

        for event in events:
            logging.info(f"Examining event: {str(event)}")
            if hasattr(event, 'Data') and hasattr(event.Data, 'changeType'):
                if expected_node_id == self.th1.nodeId:
                    # Check for expected field values
                    if (event.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
                        event.Data.adminNodeID == self.th1.nodeId and
                        isinstance(event.Data.adminPasscodeID, Nullable) and
                        event.Data.latestValue.data == D_OK_EMPTY and
                        event.Data.latestValue.fabricIndex == expected_fabric_index and
                            event.Data.fabricIndex == expected_fabric_index):
                        found_valid_events += 1

                if expected_node_id == self.th2.nodeId:
                    # Check for expected field values
                    if (event.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
                        event.Data.adminNodeID == self.th2.nodeId and
                        isinstance(event.Data.adminPasscodeID, Nullable) and
                        event.Data.latestValue.data == D_OK_SINGLE and
                        event.Data.latestValue.fabricIndex == expected_fabric_index and
                            event.Data.fabricIndex == expected_fabric_index):
                        found_valid_events += 1

                # If this is an event for the other fabric
                if event.Data.fabricIndex == other_fabric_index:
                    found_other_event = True

        if is_filtered:
            other_controller = "TH1" if controller_name == "TH2" else "TH2"
            asserts.assert_false(
                found_other_event, f"{controller_name} should not see any events from {other_controller}'s fabric when fabric filtered")

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
            TestStep(7, "TH1 reads DUT Endpoint 0 AccessControl cluster Extension attribute with both fabricFiltered True and False",
                     "Result is SUCCESS, value is list of AccessControlExtensionStruct containing 1 element when fabric filtered"),
            TestStep(8, "TH2 reads DUT Endpoint 0 AccessControl cluster Extension attribute with both fabricFiltered True and False",
                     "Result is SUCCESS, value is list of AccessControlExtensionStruct containing 1 element when fabric filtered"),
            TestStep(9, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event with both fabricFiltered True and False",
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing 1 element when fabric filtered"),
            TestStep(10, "TH2 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event with both fabricFiltered True and False",
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing 1 element when fabric filtered"),
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
            nodeId=self.dut_node_id, timeout=900, iteration=10000, discriminator=self.discriminator, option=1)
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
        # TH1 reads Extension attribute with both fabricFiltered True and False
        ac_cluster = Clusters.AccessControl
        extension_attr = Clusters.AccessControl.Attributes.Extension

        # Read with fabric_filtered=True
        result_filtered = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1, endpoint=0, cluster=ac_cluster, attribute=extension_attr,
        )
        logging.info("TH1 read result (fabricFiltered=True): %s", str(result_filtered))
        asserts.assert_equal(len(result_filtered), 1, "Should have exactly one extension when fabric filtered")

        endpoint_data = [r for r in result_filtered if r.fabricIndex == f1]
        asserts.assert_equal(
            endpoint_data[0].data, D_OK_EMPTY, "Extension data should match D_OK_EMPTY")
        asserts.assert_equal(endpoint_data[0].fabricIndex,
                             f1, "FabricIndex should match F1")

        # Read with fabric_filtered=False
        result_unfiltered = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1, endpoint=0, cluster=ac_cluster, attribute=extension_attr,
            fabric_filtered=False
        )
        logging.info("TH1 read result (fabricFiltered=False): %s", str(result_unfiltered))
        asserts.assert_greater(len(result_unfiltered), 1, "Should have at least two extensions when not fabric filtered")
        # Check that the TH2 extension data is empty
        endpoint_data_th2 = [r for r in result_unfiltered if r.fabricIndex == f2]
        asserts.assert_equal(
            endpoint_data_th2[0].data, b'', "Extension data should be empty")

        self.step(8)
        # TH2 reads Extension attribute with both fabricFiltered True and False
        result2_filtered = await self.read_single_attribute_check_success(
            dev_ctrl=self.th2, endpoint=0, cluster=ac_cluster, attribute=extension_attr,
        )
        logging.info("TH2 read result (fabricFiltered=True): %s", str(result2_filtered))
        asserts.assert_equal(len(result2_filtered), 1, "Should have exactly one extension when fabric filtered")

        endpoint_data2 = [r for r in result2_filtered if r.fabricIndex == f2]
        asserts.assert_equal(
            endpoint_data2[0].data, D_OK_SINGLE, "Extension data should match D_OK_SINGLE")
        asserts.assert_equal(endpoint_data2[0].fabricIndex, f2, "FabricIndex should match F2")

        # Read with fabric_filtered=False
        result2_unfiltered = await self.read_single_attribute_check_success(
            dev_ctrl=self.th2, endpoint=0, cluster=ac_cluster, attribute=extension_attr,
            fabric_filtered=False
        )
        logging.info("TH2 read result (fabricFiltered=False): %s", str(result2_unfiltered))
        asserts.assert_greater(len(result2_unfiltered), 1, "Should have at least two extensions when not fabric filtered")
        # Check that the TH1 extension data is empty
        endpoint_data_th1 = [r for r in result2_unfiltered if r.fabricIndex == f1]
        asserts.assert_equal(
            endpoint_data_th1[0].data, b'', "Extension data should be empty")

        self.step(9)
        # TH1 reads AccessControlExtensionChanged event with both fabricFiltered True and False
        acec_event = Clusters.AccessControl.Events.AccessControlExtensionChanged

        # Read with fabricFiltered=True
        events_response_filtered = await self.th1.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True
        )
        self._validate_events(events_response_filtered, f1, self.th1.nodeId, f2, "TH1", True)

        # Read with fabricFiltered=False
        events_response_unfiltered = await self.th1.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=False
        )
        self._validate_events(events_response_unfiltered, f1, self.th1.nodeId, f2, "TH1", False)

        self.step(10)
        # TH2 reads AccessControlExtensionChanged event with both fabricFiltered True and False
        # Read with fabricFiltered=True
        events_response2_filtered = await self.th2.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=True
        )
        self._validate_events(events_response2_filtered, f2, self.th2.nodeId, f1, "TH2", True)

        # Read with fabricFiltered=False
        events_response2_unfiltered = await self.th2.ReadEvent(
            self.dut_node_id,
            events=[(0, acec_event)],
            fabricFiltered=False
        )
        self._validate_events(events_response2_unfiltered, f2, self.th2.nodeId, f1, "TH2", False)

        self.step(11)
        # TH_CR1 sends RemoveFabric for TH2 fabric command to DUT_CE
        fabric_idx_cr2_2 = await self.read_currentfabricindex(th=self.th2)
        removeFabricCmd2 = Clusters.OperationalCredentials.Commands.RemoveFabric(fabric_idx_cr2_2)
        result = await self.th1.SendCommand(nodeId=self.dut_node_id, endpoint=0, payload=removeFabricCmd2)
        logging.info("RemoveFabric command result: %s", str(result))
        asserts.assert_equal(
            result.statusCode, 0, "RemoveFabric command should have succeeded")


if __name__ == "__main__":
    default_matter_test_main()
