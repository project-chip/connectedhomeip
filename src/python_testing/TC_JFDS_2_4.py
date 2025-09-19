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

# This test requires a TH_SERVER application. Please specify with --string-arg th_server_app_path:<path_to_app>

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     script-args: >
#       --string-arg jfa_server_app:${JF_ADMIN_APP}
#       --string-arg jfc_server_app:${JF_CONTROL_APP}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===


import base64
import logging
import os
import random
import tempfile
from configparser import ConfigParser

from mobly import asserts

import matter.clusters as Clusters
from matter import CertificateAuthority
from matter.interaction_model import InteractionModelError
from matter.storage import VolatileTemporaryPersistentStorage
from matter.testing.apps import AppServerSubprocess, JFControllerSubprocess
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_JFDS_2_4(MatterBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.fabric_a_ctrl = None
        self.storage_fabric_a = self.user_params.get("fabric_a_storage", None)
        self.fabric_a_server_app = None

        jfc_server_app = self.user_params.get("jfc_server_app", None)
        if not jfc_server_app:
            asserts.fail("This test requires a Joint Fabric Controller app. Specify app path with --string-arg jfc_server_app:<path_to_app>")
        if not os.path.exists(jfc_server_app):
            asserts.fail(f"The path {jfc_server_app} does not exist")

        jfa_server_app = self.user_params.get("jfa_server_app", None)
        if not jfa_server_app:
            asserts.fail("This test requires a Joint Fabrics Admin app. Specify app path with --string-arg jfa_server_app:<path_to_app>")
        if not os.path.exists(jfa_server_app):
            asserts.fail(f"The path {jfa_server_app} does not exist")

        # Create a temporary storage directory for both ecosystems to keep KVS files if not already provided by user.
        if self.storage_fabric_a is None:
            self.storage_directory_ecosystem_a = tempfile.TemporaryDirectory(prefix=self.__class__.__name__+"_A_")
            self.storage_fabric_a = self.storage_directory_ecosystem_a.name
            logging.info("Temporary storage directory: %s", self.storage_fabric_a)

        #####################################################################################################################################
        #
        # Initialize Ecosystem A
        #
        #####################################################################################################################################
        self.jfadmin_fabric_a_passcode = random.randint(110220011, 110220999)
        self.jfctrl_fabric_a_vid = random.randint(0x0001, 0xFFF0)

        # Start Fabric A JF-Administrator App
        self.fabric_a_admin = AppServerSubprocess(
            jfa_server_app,
            storage_dir=self.storage_fabric_a,
            port=random.randint(5001, 5999),
            discriminator=random.randint(0, 4095),
            passcode=self.jfadmin_fabric_a_passcode,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", "33033"])
        self.fabric_a_admin.start(
            expected_output="Server initialization complete",
            timeout=10)

        # Start Fabric A JF-Controller App
        self.fabric_a_ctrl = JFControllerSubprocess(
            jfc_server_app,
            rpc_server_port=33033,
            storage_dir=self.storage_fabric_a,
            vendor_id=self.jfctrl_fabric_a_vid)
        self.fabric_a_ctrl.start(
            expected_output="CHIP task running",
            timeout=10)

        # Commission JF-ADMIN app with JF-Controller on Fabric A
        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork 1 {self.jfadmin_fabric_a_passcode} --anchor true",
            expected_output="[JF] Anchor Administrator (nodeId=1) commissioned with success",
            timeout=10)

        # Extract the Ecosystem A certificates and inject them in the storage that will be provided to a new Python Controller later
        jfcStorage = ConfigParser()
        jfcStorage.read(self.storage_fabric_a+'/chip_tool_config.alpha.ini')
        self.ecoACtrlStorage = {
            "sdk-config": {
                "ExampleOpCredsCAKey1": jfcStorage.get("Default", "ExampleOpCredsCAKey0"),
                "ExampleOpCredsICAKey1": jfcStorage.get("Default", "ExampleOpCredsICAKey0"),
                "ExampleCARootCert1": jfcStorage.get("Default", "ExampleCARootCert0"),
                "ExampleCAIntermediateCert1": jfcStorage.get("Default", "ExampleCAIntermediateCert0"),
            },
            "repl-config": {
                "caList": {
                    "1": [
                        {
                            "fabricId": 1,
                            "vendorId": self.jfctrl_fabric_a_vid
                        }
                    ]
                }
            }
        }
        # Extract CATs to be provided to the Python Controller later
        self.ecoACATs = base64.b64decode(jfcStorage.get("Default", "CommissionerCATs"))[::-1].hex().strip('0')

    def teardown_class(self):
        # Stop all Subprocesses that were started in this test case
        if self.fabric_a_admin is not None:
            self.fabric_a_admin.terminate()
        if self.fabric_a_ctrl is not None:
            self.fabric_a_ctrl.terminate()
        if self.fabric_a_server_app is not None:
            self.fabric_a_server_app.terminate()

        super().teardown_class()

    def steps_TC_JFDS_2_4(self) -> list[TestStep]:
        return [
            TestStep(
                '1',
                'TH reads AdminList attribute from DUT',
                'Verify that at least one entry is returned. Verify that an entry with NodeID of DUT exists in the list.'
            ),
            TestStep(
                '2',
                'TH sends AddAdmin command to DUT with NodeId=0x0000_0000_0000_000a. '
                'Node fields are as follows: * NodeID: 0x0000_0000_0000_000a * FriendlyName: "tc-jf-2.4" '
                '* VendorID: 0x000c * ICAC: 0x0001_0002',
                'Verify that the DUT responds with Status as SUCCESS'
            ),
            TestStep(
                '3',
                'TH reads AdminList attribute from DUT',
                'Verify that one entry has been added. Verify that the new entry has NodeId=0x0000_0000_0000_000a '
                'and values matching those added in step 2.'
            ),
            TestStep(
                '4',
                'TH sends UpdateAdmin command to DUT with NodeId=0x0000_0000_0000_000a and other values different '
                'from values used in step 2. Node fields are as follows: * NodeID: 0x0000_0000_0000_000a '
                '* FriendlyName: "tc-jf-2.4-update" * VendorID: 0x000d * ICAC: 0x0001_0003',
                'Verify that the DUT responds with Status as SUCCESS'
            ),
            TestStep(
                '5',
                'TH reads AdminList attribute from DUT',
                'Verify that the entry with NodeId=0xA has values matching those added in step 4.'
            ),
            TestStep(
                '6',
                'TH sends AddAdmin command to DUT with NodeId=0x0000_0000_0000_000a (duplicate). '
                'Node fields are as follows: * NodeID: 0x0000_0000_0000_000a * FriendlyName: "tc-jf-2.4" '
                '* VendorID: 0x000c * ICAC: 0x0001_0002',
                'Verify that the DUT responds with Status code CONSTRAINT_ERROR.'
            ),
            TestStep(
                '7',
                'TH sends RemoveAdmin command to DUT with NodeId=0x0000_0000_0000_000a',
                'Verify that the DUT responds with Status as SUCCESS'
            ),
            TestStep(
                '8',
                'TH reads AdminList attribute from DUT',
                'Verify that no entry with NodeId=0x0000_0000_0000_000a exists in the list.'
            )
        ]

    @async_test_body
    async def test_TC_JFDS_2_4(self):
        # Create a Controller for Ecosystem A
        _fabric_a_persistent_storage = VolatileTemporaryPersistentStorage(
            self.ecoACtrlStorage['repl-config'], self.ecoACtrlStorage['sdk-config'])
        _certAuthorityManagerA = CertificateAuthority.CertificateAuthorityManager(
            chipStack=self.matter_stack._chip_stack,
            persistentStorage=_fabric_a_persistent_storage)
        _certAuthorityManagerA.LoadAuthoritiesFromStorage()
        devCtrlEcoA = _certAuthorityManagerA.activeCaList[0].adminList[0].NewController(
            nodeId=101,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
            catTags=[int(self.ecoACATs, 16)]
        )

        # Step 1
        self.step('1')
        response = await devCtrlEcoA.ReadAttribute(
            nodeid=1,
            attributes=[(1, Clusters.JointFabricDatastore.Attributes.AdminList)],
            returnClusterObject=True
        )
        admin_list = response[1][Clusters.JointFabricDatastore].adminList
        asserts.assert_greater_equal(len(admin_list), 1, 'AdminList should have at least one entry')
        asserts.assert_true(
            self.dut_node_id in [entry.nodeID for entry in admin_list],
            f'AdminList should contain an entry with NodeID of DUT: {self.dut_node_id}'
        )

        # Step 2
        self.step('2')
        add_admin_cmd = Clusters.JointFabricDatastore.Commands.AddAdmin(
            nodeID=0xA,
            friendlyName="tc-jf-2.4",
            vendorID=0x000C,
            icac=0x00010002
        )
        await self.send_single_cmd(cmd=add_admin_cmd, dev_ctrl=devCtrlEcoA, node_id=1, endpoint=1)

        # Step 3
        self.step('3')
        response = await devCtrlEcoA.ReadAttribute(
            nodeid=1,
            attributes=[(1, Clusters.JointFabricDatastore.Attributes.AdminList)],
            returnClusterObject=True
        )
        admin_list = response[1][Clusters.JointFabricDatastore].adminList
        target = next((entry for entry in admin_list if entry.nodeID == 0xA), None)
        asserts.assert_is_not_none(target, "Newly added Admin (0xA) should exist")
        asserts.assert_equal(target.friendlyName, "tc-jf-2.4")
        asserts.assert_equal(target.vendorID, 0x000C)
        asserts.assert_equal(target.icac, 0x00010002)

        # Step 4
        self.step('4')
        update_admin_cmd = Clusters.JointFabricDatastore.Commands.UpdateAdmin(
            nodeID=0xA,
            friendlyName="tc-jf-2.4-update",
            vendorID=0x000D,
            icac=0x00010003
        )
        await self.send_single_cmd(cmd=update_admin_cmd, dev_ctrl=devCtrlEcoA, node_id=1, endpoint=1)

        # Step 5
        self.step('5')
        response = await devCtrlEcoA.ReadAttribute(
            nodeid=1,
            attributes=[(1, Clusters.JointFabricDatastore.Attributes.AdminList)],
            returnClusterObject=True
        )
        admin_list = response[1][Clusters.JointFabricDatastore].adminList
        target = next((entry for entry in admin_list if entry.nodeID == 0xA), None)
        asserts.assert_is_not_none(target, "Updated Admin (0xA) should exist")
        asserts.assert_equal(target.friendlyName, "tc-jf-2.4-update")
        asserts.assert_equal(target.vendorID, 0x000D)
        asserts.assert_equal(target.icac, 0x00010003)

        # Step 6
        self.step('6')
        dup_admin_cmd = Clusters.JointFabricDatastore.Commands.AddAdmin(
            nodeID=0xA,
            friendlyName="tc-jf-2.4",
            vendorID=0x000C,
            icac=0x00010002
        )
        try:
            await self.send_single_cmd(cmd=dup_admin_cmd, dev_ctrl=devCtrlEcoA, node_id=1, endpoint=1)
        except InteractionModelError as e:
            asserts.assert_in("CONSTRAINT_ERROR", str(e), f'Expected CONSTRAINT_ERROR, got {e}')
        else:
            asserts.fail("Expected CONSTRAINT_ERROR, but got success")

        # Step 7
        self.step('7')
        remove_admin_cmd = Clusters.JointFabricDatastore.Commands.RemoveAdmin(nodeID=0xA)
        await self.send_single_cmd(cmd=remove_admin_cmd, dev_ctrl=devCtrlEcoA, node_id=1, endpoint=1)

        # Step 8
        self.step('8')
        response = await devCtrlEcoA.ReadAttribute(
            nodeid=1,
            attributes=[(1, Clusters.JointFabricDatastore.Attributes.AdminList)],
            returnClusterObject=True
        )
        admin_list = response[1][Clusters.JointFabricDatastore].adminList
        target = next((entry for entry in admin_list if entry.nodeID == 0xA), None)
        asserts.assert_is_none(target, "Admin (0xA) should no longer exist in AdminList")


if __name__ == "__main__":
    default_matter_test_main()
