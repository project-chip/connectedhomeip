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

import asyncio
import logging
import random
import time

import chip.clusters as Clusters
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
from chip import ChipDeviceCtrl
from chip.clusters.Types import Nullable
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

# Add these constants at the top of the file after imports
D_OK_EMPTY = bytes.fromhex('1718')
D_OK_SINGLE = bytes.fromhex(
    '17D00000F1FF01003D48656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E74206C6976696E6720617320612063686172737472696E670018')


class TC_ACL_2_8(MatterBaseTest):
    def desc_TC_ACL_2_8(self) -> str:
        return "[TC-ACL-2.8] Multiple fabrics test"

    def steps_TC_ACL_2_8(self) -> list[TestStep]:
        steps = [
            TestStep(1, "TH1 commissions DUT using admin node ID N1", is_commissioning=True),
            TestStep(2, "TH1 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                     "Result is SUCCESS, value is stored as F1"),
            TestStep(3, "TH1 puts DUT into commissioning mode, TH2 commissions DUT using admin node ID N2",
                     "DUT is commissioned on TH2 fabric"),
            TestStep(4, "TH2 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                     "Result is SUCCESS, value is stored as F2"),
            TestStep(5, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element\n1. struct\nData field: D_OK_EMPTY",
                     "Result is SUCCESS"),
            TestStep(6, "TH2 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element\n1. struct\nData field: D_OK_SINGLE",
                     "Result is SUCCESS"),
            TestStep(7, "TH1 reads DUT Endpoint 0 AccessControl cluster Extension attribute",
                     "Result is SUCCESS, value is list of AccessControlExtensionStruct containing 1 element, and MUST NOT contain an element with FabricIndex F2 or Data D_OK_SINGLE"),
            TestStep(8, "TH2 reads DUT Endpoint 0 AccessControl cluster Extension attribute",
                     "Result is SUCCESS, value is list of AccessControlExtensionStruct containing 1 element, and MUST NOT contain an element with FabricIndex F1 or Data D_OK_EMPTY"),
            TestStep(9, "TH1 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event",
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing 1 element"),
            TestStep(10, "TH2 reads DUT Endpoint 0 AccessControl cluster AccessControlExtensionChanged event",
                     "Result is SUCCESS, value is list of AccessControlExtensionChanged containing 1 element"),
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
        result = await self.th1.ReadAttribute(
            self.dut_node_id,
            [(0, cfi_attribute)]
        )
        f1 = result[0][oc_cluster][cfi_attribute]
        self.print_step("CurrentFabricIndex F1", str(f1))

        self.step(3)
        # TH1 puts DUT into commissioning mode, TH2 is created and commissions DUT using admin node ID
        duration = await self.read_single_attribute_check_success(endpoint=0, cluster=Clusters.GeneralCommissioning, attribute=Clusters.GeneralCommissioning.Attributes.BasicCommissioningInfo)
        params = await self.th1.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=duration.maxCumulativeFailsafeSeconds, iteration=10000, discriminator=self.discriminator, option=1)
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
        result = await self.th2.ReadAttribute(
            self.dut_node_id,
            [(0, Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)]
        )
        f2 = result[0][oc_cluster][cfi_attribute]
        self.print_step("CurrentFabricIndex F2", str(f2))

        self.step(5)
        # TH1 writes Extension attribute with D_OK_EMPTY
        extension = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_OK_EMPTY)
        self.print_step("TH1 writing extension with data", D_OK_EMPTY.hex())

        try:
            extension_attr = Clusters.AccessControl.Attributes.Extension
            extensions_list = [extension]
            result = await self.th1.WriteAttribute(
                self.dut_node_id,
                [(0, extension_attr(value=extensions_list))]
            )
            self.print_step("Write result", str(result))
            asserts.assert_equal(
                result[0].Status, Status.Success, "Write should have succeeded")
        except Exception as e:
            self.print_step("Error writing extension", str(e))
            asserts.fail(f"Failed to write extension: {e}")

        self.step(6)
        # TH2 writes Extension attribute with D_OK_SINGLE
        extension_th2 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
            data=D_OK_SINGLE)
        self.print_step("TH2 writing extension with data", D_OK_SINGLE.hex())

        try:
            extension_attr = Clusters.AccessControl.Attributes.Extension
            extensions_list = [extension_th2]
            result = await self.th2.WriteAttribute(
                self.dut_node_id,
                [(0, extension_attr(value=extensions_list))]
            )
            self.print_step("Write result", str(result))
            asserts.assert_equal(
                result[0].Status, Status.Success, "Write should have succeeded")
        except Exception as e:
            self.print_step("Error writing extension", str(e))
            asserts.fail(f"Failed to write extension: {e}")

        self.step(7)
        # TH1 reads Access Control Extension attribute
        try:
            extension_attr = Clusters.AccessControl.Attributes.Extension
            result = await self.th1.ReadAttribute(
                self.dut_node_id,
                [(0, extension_attr)]
            )
            self.print_step("TH1 read result", str(result))

            endpoint_data = result[0]
            cluster_data = endpoint_data[Clusters.AccessControl]
            extension_list = cluster_data[extension_attr]

            asserts.assert_equal(len(extension_list), 1,
                                 "Should have exactly one extension")
            extension_struct = extension_list[0]

            asserts.assert_equal(
                extension_struct.data, D_OK_EMPTY, "Extension data should match D_OK_EMPTY")
            asserts.assert_equal(extension_struct.fabricIndex,
                                 f1, "FabricIndex should match F1")

            for ext in extension_list:
                self.print_step("extension data", ext.data)
                self.print_step("extension fabricIndex", ext.fabricIndex)
                self.print_step("extension list", extension_list)
                asserts.assert_not_equal(
                    ext.fabricIndex, f2,
                    "Should not contain element with FabricIndex F2")
                asserts.assert_not_equal(
                    ext.data, D_OK_SINGLE,
                    "Should not contain element with Data D_OK_SINGLE")

        except Exception as e:
            self.print_step("Error reading extension", str(e))
            asserts.fail(f"Failed to read extension: {e}")

        self.step(8)
        # TH2 reads Access Control Extension attribute
        try:
            extension_attr = Clusters.AccessControl.Attributes.Extension
            result2 = await self.th2.ReadAttribute(
                self.dut_node_id,
                [(0, extension_attr)]
            )
            self.print_step("TH2 read result", str(result2))

            endpoint_data2 = result2[0]
            cluster_data2 = endpoint_data2[Clusters.AccessControl]
            extension_list2 = cluster_data2[extension_attr]

            asserts.assert_equal(len(extension_list2), 1,
                                 "Should have exactly one extension")
            extension_struct2 = extension_list2[0]

            asserts.assert_equal(
                extension_struct2.data, D_OK_SINGLE, "Extension data should match D_OK_SINGLE")
            asserts.assert_equal(extension_struct2.fabricIndex,
                                 f2, "FabricIndex should match F2")

            for ext in extension_list2:
                asserts.assert_not_equal(
                    ext.fabricIndex, f1,
                    "Should not contain element with FabricIndex F1")
                asserts.assert_not_equal(
                    ext.data, D_OK_EMPTY,
                    "Should not contain element with Data D_OK_EMPTY")

        except Exception as e:
            self.print_step("Error reading extension", str(e))
            asserts.fail(f"Failed to read extension: {e}")

        self.step(9)
        # TH1 reads AccessControlExtensionChanged event
        acec_event = Clusters.AccessControl.Events.AccessControlExtensionChanged
        self.print_step(
            "TH1 reading AccessControlExtensionChanged events...", "")

        try:
            events_response = await self.th1.ReadEvent(
                self.dut_node_id,
                events=[(0, acec_event)],
                fabricFiltered=True
            )
            self.print_step("Events response", str(events_response))

            events = events_response
            self.print_step("Found {len(events)} events", "")

            found_valid_event = False

            for event_data in events:
                self.print_step("Examining event", str(event_data))

                if hasattr(event_data, 'Data') and hasattr(event_data.Data, 'changeType'):
                    if (event_data.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
                        event_data.Data.adminNodeID == self.th1.nodeId and
                        isinstance(event_data.Data.adminPasscodeID, Nullable) and
                        event_data.Data.latestValue.data == D_OK_EMPTY and
                        event_data.Data.latestValue.fabricIndex == f1 and
                            event_data.Data.fabricIndex == f1):
                        found_valid_event = True
                        self.print_step("Found valid event for TH1", "")
                        break

            asserts.assert_true(
                found_valid_event, "Did not find the expected event with specified fields for TH1")

        except Exception as e:
            self.print_step("Error reading events for TH1", str(e))
            asserts.fail(f"Failed to read events for TH1: {e}")

        self.step(10)
        # TH2 reads AccessControlExtensionChanged event
        self.print_step(
            "TH2 reading AccessControlExtensionChanged events...", "")

        try:
            events_response = await self.th2.ReadEvent(
                self.dut_node_id,
                events=[(0, acec_event)],
                fabricFiltered=True
            )
            self.print_step("Events response", str(events_response))

            events = events_response
            self.print_step("Found {len(events)} events", "")

            found_valid_event = False

            for event_data in events:
                self.print_step("Examining event", str(event_data))

                if hasattr(event_data, 'Data') and hasattr(event_data.Data, 'changeType'):
                    if (event_data.Data.changeType == Clusters.AccessControl.Enums.ChangeTypeEnum.kAdded and
                        event_data.Data.adminNodeID == self.th2.nodeId and  # Use TH2's node ID
                        isinstance(event_data.Data.adminPasscodeID, Nullable) and
                        event_data.Data.latestValue.data == D_OK_SINGLE and
                        event_data.Data.latestValue.fabricIndex == f2 and
                            event_data.Data.fabricIndex == f2):
                        found_valid_event = True
                        self.print_step("Found valid event for TH2", "")
                        break

            asserts.assert_true(
                found_valid_event, "Did not find the expected event with specified fields for TH2")

        except Exception as e:
            self.print_step("Error reading events for TH2", str(e))
            asserts.fail(f"Failed to read events for TH2: {e}")


if __name__ == "__main__":
    default_matter_test_main()
