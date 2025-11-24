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
#     app-ready-pattern: "Server initialization complete"
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 0
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import random

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

# These below variables are used to test the AccessControl cluster
# Extension attribute and come from the test plan here:
# https://github.com/CHIP-Specifications/chip-test-plans/blob/59e8c45b8e7c24d5ce130b166520ff4f7bd935b6/src/cluster/AccessControl.adoc#tc-acl-2-10-persistence
D_OK_EMPTY = bytes.fromhex('1718')
D_OK_SINGLE = bytes.fromhex(
    '17D00000F1FF01003D48656C6C6F20576F726C642E205468697320697320612073696E676C6520656C656D656E74206C6976696E6720617320612063686172737472696E670018')


class TC_ACL_2_10(MatterBaseTest):
    def desc_TC_ACL_2_10(self) -> str:
        return "[TC-ACL-2.10] Persistence"

    def steps_TC_ACL_2_10(self) -> list[TestStep]:
        steps = [
            TestStep(1, "TH1 commissions DUT using admin node ID",
                     is_commissioning=True),
            TestStep(2, "TH1 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                     "Result is SUCCESS, value is stored as F1"),
            TestStep(3, "TH1 puts DUT into commissioning mode, TH2 commissions DUT using admin node ID",
                     "DUT is commissioned on TH2 fabric"),
            TestStep(4, "TH2 reads DUT Endpoint 0 OperationalCredentials cluster CurrentFabricIndex attribute",
                     "Result is SUCCESS, value is stored as F2"),
            TestStep(5, "TH1 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is list of AccessControlExtensionStruct containing 2 elements",
                     "Result is SUCCESS"),
            TestStep(6, "TH2 writes DUT Endpoint 0 AccessControl cluster ACL attribute, value is list of AccessControlExtensionStruct containing 2 elements",
                     "Result is SUCCESS"),
            TestStep(7, "TH1 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 elements; element data field is D_OK_EMPTY",
                     "Result is SUCCESS"),
            TestStep(8, "TH2 writes DUT Endpoint 0 AccessControl cluster Extension attribute, value is list of AccessControlExtensionStruct containing 1 element; element data field is D_OK_SINGLE",
                     "Result is SUCCESS"),
            TestStep(9, "Reboot the DUT",
                     "DUT is rebooted"),
            TestStep(10, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value is list of AccessControlExtensionStruct containing 2 elements; must not contain an element with fabricIndex F2"),
            TestStep(11, "TH1 reads DUT Endpoint 0 AccessControl cluster Extension attribute",
                     "Result is SUCCESS, value is list of AccessControlExtensionStruct containing 1 element; MUST NOT contain an element with FabricIndex `F2` or Data `D_OK_SINGLE`"),
            TestStep(12, "TH2 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value is list of AccessControlExtensionStruct containing 2 elements; must not contain an element with fabricIndex F1"),
            TestStep(13, "TH2 reads DUT Endpoint 0 AccessControl cluster Extension attribute",
                     "Result is SUCCESS, value is list of AccessControlExtensionStruct containing 1 element; MUST NOT contain an element with FabricIndex `F1` or Data `D_OK_EMPTY`"),
            TestStep(14, "TH1 removes fabric `F2` from DUT",
                     "Result is SUCCESS"),
            TestStep(15, "TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute",
                     "Result is SUCCESS, value is list of AccessControlEntryStruct containing 2 elements, and MUST NOT contain an element with FabricIndex `F2`"),
            TestStep(16, "TH1 reads DUT Endpoint 0 AccessControl cluster Extension attribute",
                     "Result is SUCCESS, value is list of AccessControlExtensionStruct containing 1 element; MUST NOT contain an element with FabricIndex `F2` or Data `D_OK_SINGLE`"),
            TestStep(17, "TH1 writes DUT Endpoint 0 AccessControl cluster back to admin-only ACL entry",
                     "Result is SUCCESS"),
        ]
        return steps

    @async_test_body
    async def test_TC_ACL_2_10(self):
        self.step(1)
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)
        self.endpoint = self.get_endpoint()
        extension_attr = Clusters.AccessControl.Attributes.Extension

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
        # Read TH2's fabric index
        f2 = await self.read_single_attribute_check_success(dev_ctrl=self.th2, endpoint=0, cluster=oc_cluster, attribute=cfi_attribute)
        logging.info("CurrentFabricIndex F2: %s", str(f2))

        self.step(5)
        # Saving initial ACL to use later during test step 17
        acl_attribute = Clusters.AccessControl.Attributes.Acl
        acl_cluster = Clusters.AccessControl
        original_acl = await self.read_single_attribute_check_success(dev_ctrl=self.th1, endpoint=0, cluster=acl_cluster, attribute=acl_attribute)

        # TH1 writes ACL attribute with 2 elements
        two_element_acl_th1 = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th1.nodeId, 1111],
                targets=NullValue,
                fabricIndex=f1
            ),
            # Operate entry
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=[3333],
                targets=NullValue,
                fabricIndex=f1
            )
        ]
        result = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=two_element_acl_th1))]
        )
        logging.info("Write result: %s", str(result))
        asserts.assert_equal(
            result[0].Status, Status.Success, "Write should have succeeded")

        self.step(6)
        # TH2 writes ACL attribute with 2 elements
        two_element_acl_th2 = [
            # Admin entry (unchanged)
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[self.th2.nodeId, 2222],
                targets=NullValue,
                fabricIndex=f2
            ),
            # Operate entry
            Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kOperate,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kGroup,
                subjects=[4444],
                targets=NullValue,
                fabricIndex=f2
            )
        ]

        result = await self.th2.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=two_element_acl_th2))]
        )
        logging.info("Write result: %s", str(result))
        asserts.assert_equal(
            result[0].Status, Status.Success, "Write should have succeeded")

        self.step(7)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=extension_attr):
            # TH1 writes Extension attribute with D_OK_EMPTY
            extension1 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
                data=D_OK_EMPTY)
            logging.info(f"Writing extension with data {D_OK_EMPTY.hex()}")
            extensions_list1 = [extension1]
            result = await self.th1.WriteAttribute(
                self.dut_node_id,
                [(0, extension_attr(value=extensions_list1))]
            )
            logging.info("TH1 write result: %s", str(result))
            asserts.assert_equal(
                result[0].Status, Status.Success, "Write should have succeeded")

        self.step(8)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=extension_attr):
            # TH2 writes Extension attribute with D_OK_SINGLE
            extension2 = Clusters.AccessControl.Structs.AccessControlExtensionStruct(
                data=D_OK_SINGLE)
            logging.info(f"Writing extension with data {D_OK_SINGLE.hex()}")
            extensions_list2 = [extension2]

            result = await self.th2.WriteAttribute(
                self.dut_node_id,
                [(0, extension_attr(value=extensions_list2))]
            )
            logging.info("TH2 write result: %s", str(result))
            asserts.assert_equal(
                result[0].Status, Status.Success, "Write should have succeeded")

        self.step(9)
        # Reboot DUT
        # Check if restart flag file is available (indicates test runner supports app restart)
        restart_flag_file = self.get_restart_flag_file()

        if not restart_flag_file:
            # No restart flag file: ask user to manually reboot
            self.wait_for_user_input(prompt_msg="Reboot the DUT. Press Enter when ready.\n")

            # After manual reboot, expire previous sessions so that we can re-establish connections
            logging.info("Expiring sessions after manual device reboot")
            self.th1.ExpireSessions(self.dut_node_id)
            self.th2.ExpireSessions(self.dut_node_id)
            logging.info("Manual device reboot completed")

        else:
            try:
                # Create the restart flag file to signal the test runner
                with open(restart_flag_file, "w") as f:
                    f.write("restart")
                logging.info("Created restart flag file to signal app restart")

                # The test runner will automatically wait for the app-ready-pattern before continuing
                # Waiting 1 second after the app-ready-pattern is detected as we need to wait a tad longer for the app to be ready and stable, otherwise TH2 connection fails later on in test step 14.
                await asyncio.sleep(1)

                # Expire sessions and re-establish connections
                self.th1.ExpireSessions(self.dut_node_id)
                self.th2.ExpireSessions(self.dut_node_id)

                logging.info("App restart completed successfully")

            except Exception as e:
                logging.error(f"Failed to restart app: {e}")
                asserts.fail(f"App restart failed: {e}")

        self.step(10)
        # TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute
        # Result is SUCCESS, value is list of AccessControlExtensionStruct
        # containing 2 elements; must not contain an element with fabricIndex F2
        # acl_cluster = Clusters.AccessControl
        result1 = await self.read_single_attribute_check_success(dev_ctrl=self.th1, endpoint=0, cluster=acl_cluster, attribute=acl_attribute)
        logging.info("TH1 read result: %s", str(result1))
        asserts.assert_equal(len(result1), 2,
                             "Should have exactly two ACL entries")
        for entry in result1:
            asserts.assert_not_equal(
                entry.fabricIndex, f2, "Should not contain entry with FabricIndex F2")

        self.step(11)
        # TH1 reads DUT Endpoint 0 AccessControl cluster Extension attribute
        # Result is SUCCESS, value is list of AccessControlExtensionStruct
        # containing 1 element; MUST NOT contain an element with FabricIndex `F2`
        # or Data `D_OK_SINGLE`
        if await self.attribute_guard(endpoint=self.endpoint, attribute=extension_attr):
            th1_extension_attr = await self.read_single_attribute_check_success(dev_ctrl=self.th1, endpoint=0, cluster=acl_cluster, attribute=extension_attr)
            logging.info("TH1 read extension result: %s", str(th1_extension_attr))
            asserts.assert_equal(len(th1_extension_attr), 1, "Expected exactly one extension attribute")

            # Verify the actual values
            entry = th1_extension_attr[0]
            asserts.assert_equal(entry.data, D_OK_EMPTY, "Data should be D_OK_EMPTY")
            asserts.assert_equal(entry.fabricIndex, f1, "FabricIndex should be F1")

        self.step(12)
        # TH2 reads DUT Endpoint 0 AccessControl cluster ACL attribute
        # Result is SUCCESS, value is list of AccessControlExtensionStruct
        # containing 2 elements; must not contain an element with fabricIndex F1
        result2 = await self.read_single_attribute_check_success(dev_ctrl=self.th2, endpoint=0, cluster=acl_cluster, attribute=acl_attribute)
        logging.info("TH2 read result: %s", str(result2))
        asserts.assert_equal(len(result2), 2,
                             "Should have exactly two ACL entries")
        for entry in result2:
            asserts.assert_not_equal(
                entry.fabricIndex, f1, "Should not contain entry with FabricIndex F1")

        self.step(13)
        # TH2 reads DUT Endpoint 0 AccessControl cluster Extension attribute
        # Result is SUCCESS, value is list of AccessControlExtensionStruct
        # containing 1 element; MUST NOT contain an element with FabricIndex `F1`
        # or Data `D_OK_EMPTY`
        if await self.attribute_guard(endpoint=self.endpoint, attribute=extension_attr):
            th2_extension_attr = await self.read_single_attribute_check_success(dev_ctrl=self.th2, endpoint=0, cluster=acl_cluster, attribute=extension_attr)
            logging.info("TH2 read extension result: %s", str(th2_extension_attr))
            asserts.assert_equal(len(th2_extension_attr), 1, "Expected exactly one extension attribute")

            # Verify the actual values
            entry2 = th2_extension_attr[0]
            asserts.assert_equal(entry2.data, D_OK_SINGLE, "Data should be D_OK_SINGLE")
            asserts.assert_equal(entry2.fabricIndex, f2, "FabricIndex should be F2")

        self.step(14)
        # TH1 removes fabric `F2` from DUT
        removeFabricCmd2 = Clusters.OperationalCredentials.Commands.RemoveFabric(f2)
        await self.th1.SendCommand(nodeId=self.dut_node_id, endpoint=0, payload=removeFabricCmd2)

        self.step(15)
        # TH1 reads DUT Endpoint 0 AccessControl cluster ACL attribute
        # Result is SUCCESS, value is list of AccessControlEntryStruct containing
        # 2 elements, and MUST NOT contain an element with FabricIndex `F2`
        result3 = await self.read_single_attribute_check_success(dev_ctrl=self.th1, endpoint=0, cluster=acl_cluster, attribute=acl_attribute)
        logging.info("TH1 read result: %s", str(result3))
        asserts.assert_equal(len(result3), 2,
                             "Should have exactly two ACL entries")

        # Verify the ACL entries, should not contain an entry with FabricIndex F2
        for entry in result3:
            asserts.assert_not_equal(
                entry.fabricIndex, f2, "Should not contain entry with FabricIndex F2")

        self.step(16)
        if await self.attribute_guard(endpoint=self.endpoint, attribute=extension_attr):
            # TH1 reads DUT Endpoint 0 AccessControl cluster Extension attribute
            # Result is SUCCESS, value is list of AccessControlExtensionStruct
            # containing 1 element; MUST NOT contain an element with FabricIndex `F2`
            # or Data `D_OK_SINGLE`
            th1_extension_attr2 = await self.read_single_attribute_check_success(dev_ctrl=self.th1, endpoint=0, cluster=acl_cluster, attribute=extension_attr)
            logging.info("TH1 read extension result: %s", str(th1_extension_attr2))
            asserts.assert_equal(len(th1_extension_attr2), 1, "Expected exactly one extension attribute")

            # Verify the attribute, should not contain an entry with FabricIndex F2 or Data D_OK_SINGLE
            entry3 = th1_extension_attr2[0]
            asserts.assert_equal(entry3.data, D_OK_EMPTY, "Data should be D_OK_EMPTY")
            asserts.assert_equal(entry3.fabricIndex, f1, "FabricIndex should be F1")

        # Step 17: Write minimum required ACL (admin only)
        self.step(17)
        result = await self.th1.WriteAttribute(
            self.dut_node_id,
            [(0, acl_attribute(value=original_acl))]
        )
        asserts.assert_equal(result[0].Status, Status.Success,
                             "Write admin-only ACL should succeed")
        logging.info("Successfully reset ACL to admin-only entry")

        # Final verification that ACL contains only admin entry
        final_acl = await self.read_single_attribute_check_success(
            endpoint=0,
            cluster=acl_cluster,
            attribute=acl_attribute
        )
        asserts.assert_equal(len(final_acl), 1,
                             "ACL should contain exactly one admin entry")
        asserts.assert_equal(final_acl[0].privilege,
                             Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                             "Remaining entry should have admin privilege")
        asserts.assert_equal(final_acl[0].subjects, [self.th1.nodeId],
                             "Entry should have admin node ID")


if __name__ == "__main__":
    default_matter_test_main()
