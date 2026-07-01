#
#    Copyright (c) 2026 Project CHIP Authors
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

# This test requires a TH2_SERVER application. Please specify with --string-arg th2_server_app_path:<path_to_app>

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     script-args: >
#       --string-arg jfa_server_app:${JF_ADMIN_APP}
#       --string-arg jfc_server_app:${JF_CONTROL_APP}
#       --string-arg th2_server_app:${ALL_CLUSTERS_APP}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import base64
import logging
import os
import random
import tempfile
from configparser import ConfigParser

from mobly import asserts

import matter.clusters as Clusters
from matter import CertificateAuthority
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.storage import VolatileTemporaryPersistentStorage
from matter.testing.apps import AppServerSubprocess, JFControllerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_JFDS_2_5(MatterBaseTest):

    @staticmethod
    def _enum_as_int(value):
        try:
            return int(value)
        except (TypeError, ValueError):
            return value

    @staticmethod
    def _enum_name_or_value(value):
        if hasattr(value, "name"):
            return value.name
        return TC_JFDS_2_5._enum_as_int(value)

    @staticmethod
    def _enum_equals(actual, expected):
        try:
            return type(expected)(actual) == expected
        except (TypeError, ValueError):
            return False

    @staticmethod
    def _assert_enum_equal(actual, expected, message):
        try:
            actual_enum = type(expected)(actual)
        except (TypeError, ValueError):
            asserts.fail(f"{message}. Expected enum {type(expected).__name__}.{expected.name}, got {actual!r}")
            return
        asserts.assert_equal(actual_enum, expected, message)

    @classmethod
    def _normalize_nullable(cls, value):
        if value is NullValue or value is None:
            return None
        return cls._enum_as_int(value)

    @classmethod
    def _normalize_acl_targets(cls, targets):
        if targets is NullValue or targets is None:
            return []

        normalized = []
        for target in targets:
            normalized.append((
                cls._normalize_nullable(target.cluster),
                cls._normalize_nullable(target.endpoint),
                cls._normalize_nullable(target.deviceType),
            ))
        return sorted(normalized)

    @classmethod
    def _normalize_acl_entry(cls, acl_entry):
        subjects = [] if acl_entry.subjects in (NullValue, None) else sorted(acl_entry.subjects)
        return {
            "privilege": cls._enum_name_or_value(acl_entry.privilege),
            "auth_mode": cls._enum_name_or_value(acl_entry.authMode),
            "subjects": subjects,
            "targets": cls._normalize_acl_targets(acl_entry.targets),
        }

    @classmethod
    def _normalize_binding_target(cls, binding):
        return {
            "node": cls._normalize_nullable(binding.node),
            "group": cls._normalize_nullable(binding.group),
            "endpoint": cls._normalize_nullable(binding.endpoint),
            "cluster": cls._normalize_nullable(binding.cluster),
        }

    @classmethod
    def _extract_datastore_bindings(cls, endpoint_binding_entry):
        if hasattr(endpoint_binding_entry, "bindingList"):
            return list(endpoint_binding_entry.bindingList)
        if hasattr(endpoint_binding_entry, "binding"):
            return [endpoint_binding_entry.binding]
        return []

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.fabric_a_ctrl = None
        self.storage_fabric_a = self.user_params.get("fabric_a_storage", None)
        self.storage_directory_ecosystem_a_admin = None
        self.storage_directory_ecosystem_a_ctrl = None
        self.storage_directory_ecosystem_a_th2 = None
        self.storage_fabric_a_admin = None
        self.storage_fabric_a_ctrl = None
        self.storage_fabric_a_th2 = None
        self.devCtrlEcoA = None
        self.certAuthorityManagerA = None
        self.fabric_a_persistent_storage = None
        self.th2_app = None

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

        # Get path to TH2 Server app from user parameters
        th2_server_app = self.user_params.get("th2_server_app", None)
        # Validate TH2 Server app
        if not th2_server_app:
            asserts.fail("This test requires a TH2 Server app. Specify app path with --string-arg th2_server_app:<path_to_app>")
        if not os.path.exists(th2_server_app):
            asserts.fail(f"The path {th2_server_app} does not exist")

        # Create distinct storage directories per app to avoid KVS collisions.
        if self.storage_fabric_a is None:
            self.storage_directory_ecosystem_a_admin = tempfile.TemporaryDirectory(
                prefix=self.__class__.__name__ + "_A_admin_")
            self.storage_directory_ecosystem_a_ctrl = tempfile.TemporaryDirectory(
                prefix=self.__class__.__name__ + "_A_ctrl_")
            self.storage_directory_ecosystem_a_th2 = tempfile.TemporaryDirectory(
                prefix=self.__class__.__name__ + "_A_th2_")
            self.storage_fabric_a_admin = self.storage_directory_ecosystem_a_admin.name
            self.storage_fabric_a_ctrl = self.storage_directory_ecosystem_a_ctrl.name
            self.storage_fabric_a_th2 = self.storage_directory_ecosystem_a_th2.name
            log.info("Temporary storage directories: admin=%s ctrl=%s th2=%s",
                     self.storage_fabric_a_admin, self.storage_fabric_a_ctrl, self.storage_fabric_a_th2)
        else:
            self.storage_fabric_a_admin = os.path.join(self.storage_fabric_a, "jf_admin")
            self.storage_fabric_a_ctrl = os.path.join(self.storage_fabric_a, "jf_ctrl")
            self.storage_fabric_a_th2 = os.path.join(self.storage_fabric_a, "th2_app")
            os.makedirs(self.storage_fabric_a_admin, exist_ok=True)
            os.makedirs(self.storage_fabric_a_ctrl, exist_ok=True)
            os.makedirs(self.storage_fabric_a_th2, exist_ok=True)

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
            storage_dir=self.storage_fabric_a_admin,
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
            "JFC_A",  # Name of the controller instance, used for logging purposes in the JF-Controller app
            rpc_server_port=33033,
            storage_dir=self.storage_fabric_a_ctrl,
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
        jfcStorage.read(os.path.join(self.storage_fabric_a_ctrl, 'chip_tool_config.alpha.ini'))
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

        # Setup TH2: Commission a second node on the same fabric
        self.th2_passcode = random.randint(110220011, 110220999)
        self.th2_node_id = 2
        self.th2_port = random.randint(6001, 6999)
        self.th2_kvs_path = os.path.join(self.storage_fabric_a_th2, "kvs-th2")

        # Start TH2 App (another admin node)
        self.th2_app = AppServerSubprocess(
            th2_server_app,
            storage_dir=self.storage_fabric_a_th2,
            port=self.th2_port,
            discriminator=random.randint(0, 4095),
            passcode=self.th2_passcode,
            extra_args=["--capabilities", "0x04"],
            kvs_path=self.th2_kvs_path)
        self.th2_app.start(
            expected_output="Server initialization complete",
            timeout=10)

        # Creating a Controller for Ecosystem A
        self.fabric_a_persistent_storage = VolatileTemporaryPersistentStorage(
            self.ecoACtrlStorage['repl-config'], self.ecoACtrlStorage['sdk-config'])
        self.certAuthorityManagerA = CertificateAuthority.CertificateAuthorityManager(
            chipStack=self.matter_stack._chip_stack,
            persistentStorage=self.fabric_a_persistent_storage)
        self.certAuthorityManagerA.LoadAuthoritiesFromStorage()
        self.devCtrlEcoA = self.certAuthorityManagerA.activeCaList[0].adminList[0].NewController(
            nodeId=101,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
            catTags=[int(self.ecoACATs, 16)])

        # Commission TH2 with JF-Controller on Fabric A
        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork {self.th2_node_id} {self.th2_passcode} --regular 1",
            expected_output="[CTL] Commissioning complete for node ID 0x0000000000000002: success",
            timeout=10)

        # Verify commissioning was successful
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=self.th2_node_id, attributes=[(0, Clusters.OperationalCredentials.Attributes.Fabrics)],
            returnClusterObject=True)
        asserts.assert_is_not_none(response, "TH2 commissioning failed")

        # Setup KS1: Add KeySet to datastore
        self.th_keyset_id = 0x0042
        # Store the original epoch key bytes locally; KeySetRead always returns Null for
        # epoch key material (write-only by spec), so this is the only way to know the value.
        self.th_keyset_epoch = bytes.fromhex('0123456789ABCDEFFEDCBA9876543210')

        # Wait for commissioning to complete before sending commands
        await asyncio.sleep(1)

        # Create KeySet with AddKeySet command
        keyset_struct = Clusters.JointFabricDatastore.Structs.DatastoreGroupKeySetStruct(
            groupKeySetID=self.th_keyset_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex('00112233445566778899AABBCCDDEEFF'),
            epochStartTime0=1,
            epochKey1=bytes.fromhex('FFEEDDCCBBAA99887766554433221100'),
            epochStartTime1=100000,
            epochKey2=self.th_keyset_epoch,
            epochStartTime2=200000
        )

        await self.send_single_cmd(
            cmd=Clusters.JointFabricDatastore.Commands.AddKeySet(keyset_struct),
            dev_ctrl=self.devCtrlEcoA,
            node_id=1,
            endpoint=1
        )

        # Setup G1: Add Group to datastore
        self.th_group_id = 0x0101
        self.th_group_cat = 0x0001
        self.th_group_cat_version = 0x0001

        add_group_cmd = Clusters.JointFabricDatastore.Commands.AddGroup(
            groupID=self.th_group_id,
            friendlyName="TestGroup",
            groupKeySetID=self.th_keyset_id,
            groupCAT=self.th_group_cat,
            groupCATVersion=self.th_group_cat_version,
            groupPermission=Clusters.JointFabricDatastore.Enums.DatastoreAccessControlEntryPrivilegeEnum.kView
        )

        await self.send_single_cmd(
            cmd=add_group_cmd,
            dev_ctrl=self.devCtrlEcoA,
            endpoint=1,
            node_id=1
        )

    def teardown_class(self):
        # Shutdown in the correct order: Controller -> CertificateAuthorityManager -> PersistentStorage
        if self.devCtrlEcoA is not None:
            self.devCtrlEcoA.Shutdown()
            self.devCtrlEcoA = None

        if self.certAuthorityManagerA is not None:
            self.certAuthorityManagerA.Shutdown()
            self.certAuthorityManagerA = None

        if self.fabric_a_persistent_storage is not None:
            self.fabric_a_persistent_storage.Shutdown()
            self.fabric_a_persistent_storage = None
        # Stop all Subprocesses that were started in this test case
        if self.th2_app is not None:
            self.th2_app.terminate()
        if self.fabric_a_admin is not None:
            self.fabric_a_admin.terminate()
        if self.fabric_a_ctrl is not None:
            self.fabric_a_ctrl.terminate()

        super().teardown_class()

    @async_test_body
    async def test_TC_JFDS_2_5(self):
        # Step 1: TH reads NodeList attribute from DUT
        self.step(1, "TH reads NodeList attribute from DUT")
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.NodeList)],
            returnClusterObject=True)
        node_list = response[1][Clusters.JointFabricDatastore].nodeList

        # Verify that th_node_id is not in the list
        th_node_in_list = any(node.nodeID == self.th2_node_id for node in node_list)
        asserts.assert_false(th_node_in_list, "th_node_id should not be in NodeList initially")

        # Step 2: TH sends AddPendingNode command to DUT
        self.step(2, "TH sends AddPendingNode command to DUT with NodeId=th_node_id, FriendlyName: 'tc-jf-2.5'")
        await self.send_single_cmd(
            cmd=Clusters.JointFabricDatastore.Commands.AddPendingNode(
                nodeID=self.th2_node_id,
                friendlyName="tc-jf-2.5",
            ),
            node_id=1,
            dev_ctrl=self.devCtrlEcoA,
            endpoint=1,
        )

        # Step 3: TH sends AddPendingNode command with duplicate NodeId
        self.step(3, "TH sends AddPendingNode command to DUT with NodeId=th_node_id (duplicate), FriendlyName: 'tc-jf-2.5'")
        try:
            await self.send_single_cmd(
                cmd=Clusters.JointFabricDatastore.Commands.AddPendingNode(
                    nodeID=self.th2_node_id,
                    friendlyName="tc-jf-2.5",
                ),
                dev_ctrl=self.devCtrlEcoA,
                node_id=1,
                endpoint=1,
            )
            asserts.fail("Expected CONSTRAINT_ERROR for duplicate node")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Duplicate node should return CONSTRAINT_ERROR")

        # Step 4: TH reads NodeList attribute from DUT
        self.step(4, "TH reads NodeList attribute from DUT")

        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.NodeList)],
            returnClusterObject=True)
        node_list_step4 = response[1][Clusters.JointFabricDatastore].nodeList

        # Find the newly added node
        step2_node = None
        for node in node_list_step4:
            if node.nodeID == self.th2_node_id:
                step2_node = node
                break

        asserts.assert_is_not_none(step2_node, "Node with th_node_id should be in NodeList")
        asserts.assert_equal(step2_node.friendlyName, "tc-jf-2.5", "FriendlyName should match step 2")
        self._assert_enum_equal(
            step2_node.commissioningStatusEntry.state,
            Clusters.JointFabricDatastore.Enums.DatastoreStateEnum.kPending,
            "Status should be Pending")

        # Step 5: TH sends UpdateNode command to DUT
        self.step(5, "TH sends UpdateNode command to DUT with NodeId=th_node_id and other values different from step 2, FriendlyName: 'tc-jf-2.5-update'")
        await self.send_single_cmd(
            cmd=Clusters.JointFabricDatastore.Commands.UpdateNode(
                nodeID=self.th2_node_id,
                friendlyName="tc-jf-2.5-update",
            ),
            dev_ctrl=self.devCtrlEcoA,
            node_id=1,
            endpoint=1,
        )

        # Step 6: TH reads NodeList attribute from DUT
        self.step(6, "TH reads NodeList attribute from DUT")
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.NodeList)],
            returnClusterObject=True)
        node_list_step6 = response[1][Clusters.JointFabricDatastore].nodeList

        # Find the updated node
        step5_node = None
        for node in node_list_step6:
            if node.nodeID == self.th2_node_id:
                step5_node = node
                break

        asserts.assert_is_not_none(step5_node, "Node with th_node_id should be in NodeList")
        asserts.assert_equal(step5_node.friendlyName, "tc-jf-2.5-update", "FriendlyName should match step 5 update")
        self._assert_enum_equal(
            step5_node.commissioningStatusEntry.state,
            Clusters.JointFabricDatastore.Enums.DatastoreStateEnum.kPending,
            "Status should be Pending")

        # Step 7: TH sends UpdateNode command with non-existent NodeId
        self.step(7, "TH sends UpdateNode command to DUT with NodeId=0x0000_0000_0000_000a (not found), FriendlyName: 'tc-jf-2.5-update'")
        try:
            await self.send_single_cmd(
                cmd=Clusters.JointFabricDatastore.Commands.UpdateNode(
                    nodeID=0x0000_0000_0000_000a,
                    friendlyName="tc-jf-2.5-update",
                ),
                dev_ctrl=self.devCtrlEcoA,
                endpoint=1,
                node_id=1
            )
            asserts.fail("Expected CONSTRAINT_ERROR for non-existent node")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError, "Non-existent node should return CONSTRAINT_ERROR")

        # Step 8: TH sends RefreshNode command to DUT
        self.step(8, "TH sends RefreshNode command to DUT with NodeId=th_node_id")

        await self.send_single_cmd(
            cmd=Clusters.JointFabricDatastore.Commands.RefreshNode(nodeID=self.th2_node_id),
            dev_ctrl=self.devCtrlEcoA,
            node_id=1,
            endpoint=1,
        )

        # Step 9: TH monitors NodeList attribute, waiting for Status to change to Committed
        self.step(9, "TH monitors NodeList attribute from DUT, waiting for the Status of entry with NodeId=th_node_id changes to Committed")

        # Poll NodeList until the node status changes to Committed (with timeout)
        max_retries = 30  # 30 seconds timeout
        node_committed = False
        committed_node = None

        for retry in range(max_retries):
            response = await self.devCtrlEcoA.ReadAttribute(
                nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.NodeList)],
                returnClusterObject=True)
            node_list_step10 = response[1][Clusters.JointFabricDatastore].nodeList

            for node in node_list_step10:
                if node.nodeID == self.th2_node_id:
                    if self._enum_equals(node.commissioningStatusEntry.state,
                                         Clusters.JointFabricDatastore.Enums.DatastoreStateEnum.kCommitted):
                        node_committed = True
                        committed_node = node
                        break

            if node_committed:
                break

            await asyncio.sleep(1)  # Wait 1 second before next poll

        asserts.assert_true(node_committed, "Node status should change to Committed")
        asserts.assert_is_not_none(committed_node, "Committed node entry should exist")

        # Step 10: Read the NodeList entry and verify NodeKeySetList
        self.step(10, "Read the NodeList entry for NodeId=th_node_id")

        # Get the NodeKeySetList from the datastore entry for th_node_id
        node_keyset_list_dut = committed_node.nodeKeySetList if hasattr(committed_node, 'nodeKeySetList') else []

        # Read the GroupKeyManagement cluster's KeySetList from TH2
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=self.th2_node_id, attributes=[(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap)],
            returnClusterObject=True)
        th2_keyset_list = response[0][Clusters.GroupKeyManagement].groupKeyMap

        # Verify 1:1 correspondence - same number of entries
        asserts.assert_equal(len(node_keyset_list_dut), len(th2_keyset_list),
                             "NodeKeySetList should have 1:1 correspondence with TH2's GroupKeyMap")

        # Verify each KeySet entry exists in both lists
        dut_keyset_ids = {keyset.groupKeySetID for keyset in node_keyset_list_dut}
        th2_keyset_ids = {keyset.groupKeySetID for keyset in th2_keyset_list}
        asserts.assert_equal(dut_keyset_ids, th2_keyset_ids,
                             "KeySet IDs should match between DUT and TH2")

        # Step 11: Read the NodeList entry and verify NodeACLList
        self.step(11, "Read the NodeList entry for NodeId=th_node_id")

        # Get NodeACLList from DUT for th_node_id
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.NodeACLList)],
            returnClusterObject=True)
        node_acl_list_step12 = response[1][Clusters.JointFabricDatastore].nodeACLList

        # Find the ACL entry for th_node_id
        th2_acl_entry_dut = None
        for acl_entry in node_acl_list_step12:
            if acl_entry.nodeID == self.th2_node_id:
                th2_acl_entry_dut = acl_entry
                break

        asserts.assert_is_not_none(th2_acl_entry_dut, "ACL entry for th_node_id should exist in DUT")

        # Read ACL from TH2
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=self.th2_node_id, attributes=[(0, Clusters.AccessControl.Attributes.Acl)],
            returnClusterObject=True)
        th2_acl_list = response[0][Clusters.AccessControl].acl

        # Compare only ACL entries mirrored for th_node_id.
        dut_acl_entries = [entry.ACLEntry for entry in node_acl_list_step12 if entry.nodeID == self.th2_node_id]
        asserts.assert_equal(len(dut_acl_entries), len(th2_acl_list),
                             "NodeACLList should have 1:1 correspondence with TH2's ACL entries")

        normalized_th2_acls = [self._normalize_acl_entry(acl) for acl in th2_acl_list]
        for dut_acl_entry in dut_acl_entries:
            normalized_dut_acl = self._normalize_acl_entry(dut_acl_entry)
            asserts.assert_in(
                normalized_dut_acl,
                normalized_th2_acls,
                f"Mirrored ACL entry should match TH2 ACL contents: {normalized_dut_acl}")

        # Step 12: Read the NodeList entry and verify NodeEndpointList
        self.step(12, "Read the NodeList entry for NodeId=th_node_id")

        # Get NodeEndpointList from DUT for th_node_id
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.NodeEndpointList)],
            returnClusterObject=True)
        node_endpoint_list = response[1][Clusters.JointFabricDatastore].nodeEndpointList

        # Find the endpoint entry for th_node_id
        th2_endpoint_entry = None
        for endpoint_entry in node_endpoint_list:
            if endpoint_entry.nodeID == self.th2_node_id:
                th2_endpoint_entry = endpoint_entry
                break

        asserts.assert_is_not_none(th2_endpoint_entry, "Endpoint entry for th_node_id should exist in DUT")

        # Read Descriptor Parts list from TH2 to get all endpoints
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=self.th2_node_id, attributes=[(0, Clusters.Descriptor.Attributes.PartsList)],
            returnClusterObject=True)
        th2_parts_list = response[0][Clusters.Descriptor].partsList

        # Extract endpoint IDs from DUT's NodeEndpointList
        dut_endpoint_ids = [ep.endpointID for ep in node_endpoint_list]

        # Verify 1:1 correspondence
        asserts.assert_equal(sorted(dut_endpoint_ids), sorted(th2_parts_list),
                             "NodeEndpointList should have 1:1 correspondence with TH2's endpoints")

        # Verify at least one non-zero endpoint exists and save it as th_app_endpoint_id
        non_zero_endpoints = [ep for ep in dut_endpoint_ids if ep != 0]
        asserts.assert_true(len(non_zero_endpoints) > 0, "Should have at least one application endpoint (non-zero)")
        self.th_app_endpoint_id = non_zero_endpoints[0]  # Save for potential future use

        # Step 13: Read the NodeList entry and verify EndpointGroupIDList
        self.step(13, "Read the NodeList entry for NodeId=th_node_id")

        # Get EndpointGroupIDList from DUT
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.EndpointGroupIDList)],
            returnClusterObject=True)
        endpoint_group_list = response[1][Clusters.JointFabricDatastore].endpointGroupIDList

        # Find entries for th_node_id
        th2_endpoint_groups = [entry for entry in endpoint_group_list if entry.nodeID == self.th2_node_id]

        # For each endpoint on TH2, verify the GroupList correspondence
        for th2_endpoint_id in th2_parts_list:
            # Read Groups cluster GroupTable from TH2 endpoint
            try:
                response = await self.devCtrlEcoA.ReadAttribute(
                    nodeId=self.th2_node_id, attributes=[(th2_endpoint_id, Clusters.Groups.Attributes.GroupTable)],
                    returnClusterObject=True)
                th2_group_table = response[th2_endpoint_id][Clusters.Groups].groupTable

                # Find corresponding entry in DUT's EndpointGroupIDList
                dut_endpoint_group_entry = None
                for entry in th2_endpoint_groups:
                    if entry.endpointID == th2_endpoint_id:
                        dut_endpoint_group_entry = entry
                        break

                if len(th2_group_table) > 0:
                    # If TH2 has groups on this endpoint, DUT should have a corresponding entry
                    asserts.assert_is_not_none(dut_endpoint_group_entry,
                                               f"DUT should have EndpointGroupIDList entry for endpoint {th2_endpoint_id}")

                    # Verify 1:1 correspondence of group IDs
                    th2_group_ids = {group.groupID for group in th2_group_table}
                    dut_group_ids = set(dut_endpoint_group_entry.groupIDList)
                    asserts.assert_equal(th2_group_ids, dut_group_ids,
                                         f"Group IDs should match for endpoint {th2_endpoint_id}")
            except Exception:
                # Groups cluster may not be supported on all endpoints, which is fine
                pass

        # Step 14: Read the NodeList entry and verify EndpointBindingList
        self.step(14, "Read the NodeList entry for NodeId=th_node_id")

        # Get EndpointBindingList from DUT
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.EndpointBindingList)],
            returnClusterObject=True)
        endpoint_binding_list = response[1][Clusters.JointFabricDatastore].endpointBindingList

        # Find entries for th_node_id
        th2_endpoint_bindings = [entry for entry in endpoint_binding_list if entry.nodeID == self.th2_node_id]

        # Aggregate DUT mirrored bindings per endpoint.
        dut_bindings_by_endpoint = {}
        for entry in th2_endpoint_bindings:
            endpoint_id = entry.endpointID
            if endpoint_id not in dut_bindings_by_endpoint:
                dut_bindings_by_endpoint[endpoint_id] = []
            for binding in self._extract_datastore_bindings(entry):
                dut_bindings_by_endpoint[endpoint_id].append(self._normalize_binding_target(binding))

        binding_capable_endpoints = set()

        # For each endpoint on TH2, verify the BindingList correspondence
        for th2_endpoint_id in th2_parts_list:
            # Determine whether this TH2 endpoint supports the Binding cluster.
            descriptor_response = await self.devCtrlEcoA.ReadAttribute(
                nodeId=self.th2_node_id,
                attributes=[(th2_endpoint_id, Clusters.Descriptor.Attributes.ServerList)],
                returnClusterObject=True)
            th2_server_list = descriptor_response[th2_endpoint_id][Clusters.Descriptor].serverList

            if Clusters.Binding.id not in th2_server_list:
                continue

            binding_capable_endpoints.add(th2_endpoint_id)

            # Read Binding cluster Binding attribute from TH2 endpoint.
            binding_response = await self.devCtrlEcoA.ReadAttribute(
                nodeId=self.th2_node_id,
                attributes=[(th2_endpoint_id, Clusters.Binding.Attributes.Binding)],
                returnClusterObject=True)
            asserts.assert_true(
                th2_endpoint_id in binding_response and Clusters.Binding in binding_response[th2_endpoint_id],
                f"TH2 endpoint {th2_endpoint_id} reports Binding in ServerList but Binding cluster read is unavailable")

            th2_binding_table = binding_response[th2_endpoint_id][Clusters.Binding].binding
            normalized_th2_bindings = [self._normalize_binding_target(binding) for binding in th2_binding_table]
            normalized_dut_bindings = dut_bindings_by_endpoint.get(th2_endpoint_id, [])

            asserts.assert_equal(
                len(normalized_dut_bindings),
                len(normalized_th2_bindings),
                f"Binding count should match for endpoint {th2_endpoint_id}")

            for normalized_dut_binding in normalized_dut_bindings:
                asserts.assert_in(
                    normalized_dut_binding,
                    normalized_th2_bindings,
                    f"Mirrored binding should match TH2 binding for endpoint {th2_endpoint_id}: {normalized_dut_binding}")

            for normalized_th2_binding in normalized_th2_bindings:
                asserts.assert_in(
                    normalized_th2_binding,
                    normalized_dut_bindings,
                    f"DUT should mirror TH2 binding for endpoint {th2_endpoint_id}: {normalized_th2_binding}")

        # DUT should not mirror bindings for endpoints that do not support Binding on TH2.
        for endpoint_id, normalized_dut_bindings in dut_bindings_by_endpoint.items():
            if endpoint_id in binding_capable_endpoints:
                continue
            asserts.assert_equal(
                len(normalized_dut_bindings),
                0,
                f"DUT should not contain bindings for TH2 endpoint {endpoint_id} that has no Binding cluster")

        # Step 15: TH sends AddACLToNode command with non-existent NodeId
        self.step(15, "TH sends AddACLToNode command to DUT with NodeId=0x0000_0000_0000_000a (not found)")

        acl_struct = Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kManage,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[self.th2_node_id],
            targets=[Clusters.AccessControl.Structs.AccessControlTargetStruct(
                cluster=0,
                endpoint=1,
                deviceType=0
            )]
        )

        try:
            await self.send_single_cmd(
                cmd=Clusters.JointFabricDatastore.Commands.AddACLToNode(
                    nodeID=0x0000_0000_0000_000a,
                    ACLEntry=acl_struct
                ),
                dev_ctrl=self.devCtrlEcoA,
                node_id=1,
                endpoint=1,
            )
            asserts.fail("Expected CONSTRAINT_ERROR for non-existent node")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 "AddACLToNode with non-existent node should return CONSTRAINT_ERROR")

        # Step 16: TH sends AddACLToNode command with valid NodeId
        self.step(
            16, "TH sends AddACLToNode command to DUT with NodeId=th_node_id, ACL fields: Privilege Manage (4), AuthMode CASE (2), Subjects [th_node_id2], Targets [{Cluster: null, Endpoint: 0, DeviceType: null}]")

        acl_to_add = Clusters.JointFabricDatastore.Structs.DatastoreAccessControlEntryStruct(
            privilege=Clusters.JointFabricDatastore.Enums.DatastoreAccessControlEntryPrivilegeEnum.kManage,
            authMode=Clusters.JointFabricDatastore.Enums.DatastoreAccessControlEntryAuthModeEnum.kCase,
            subjects=[self.th2_node_id],
            targets=[Clusters.JointFabricDatastore.Structs.DatastoreAccessControlTargetStruct(
                cluster=NullValue,
                endpoint=0,
                deviceType=NullValue
            )]
        )

        await self.send_single_cmd(
            cmd=Clusters.JointFabricDatastore.Commands.AddACLToNode(
                nodeID=self.th2_node_id,
                ACLEntry=acl_to_add
            ),
            dev_ctrl=self.devCtrlEcoA,
            node_id=1,
            endpoint=1,
        )

        # Step 17: TH reads NodeACLList and verifies the new ACL entry
        self.step(17, "TH reads NodeACLList entries NodeId=th_node_id from DUT and reads ACLList from TH2")

        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.NodeACLList)],
            returnClusterObject=True)
        node_acl_list_step18 = response[1][Clusters.JointFabricDatastore].nodeACLList

        # Find the ACL entry matching step 17
        matching_acl = None
        th_acllist_id = None
        expected_acl = self._normalize_acl_entry(acl_to_add)

        for acl in node_acl_list_step18:
            if acl.nodeID == self.th2_node_id and self._normalize_acl_entry(acl.ACLEntry) == expected_acl:
                matching_acl = acl
                # Get the ListID
                th_acllist_id = acl.listID

        asserts.assert_is_not_none(matching_acl, "NodeACLList should contain the ACL entry added in step 17")

        # Note the ListID
        self.th_acllist_id = th_acllist_id

        # Step 18: TH reads ACLList from TH2 and verifies the ACL entry exists there
        self.step(18, "TH reads NodeACLList entries NodeId=th_node_id from DUT and reads ACLList from TH2")

        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=self.th2_node_id, attributes=[(0, Clusters.AccessControl.Attributes.Acl)],
            returnClusterObject=True)
        th2_acl_list_step19 = response[0][Clusters.AccessControl].acl

        # Find the ACL entry matching step 17
        th2_normalized_acls = [self._normalize_acl_entry(acl) for acl in th2_acl_list_step19]
        asserts.assert_in(expected_acl, th2_normalized_acls,
                          "TH2's ACLList should contain the ACL entry added in step 17")

        # Step 19: TH reads ACLList from TH2 and verifies Admin CAT entry
        self.step(19, "TH reads ACLList from TH2")

        # We already have th2_acl_list_step19 from step 19
        admin_cat_acl = None
        th_admin_cat_version = None

        for acl in th2_acl_list_step19:
            if (acl.privilege == Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister and
                    acl.authMode == Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase):
                # Check if subjects contains Admin CAT (0xFFFF_xxxx pattern)
                for subject in acl.subjects:
                    if (subject & 0xFFFF_0000_0000_0000) == 0xFFFF_0000_0000_0000:
                        admin_cat_acl = acl
                        # Extract the CAT version (lower 16 bits of the identifier)
                        th_admin_cat_version = subject & 0xFFFF
                        break
                if admin_cat_acl:
                    break

        asserts.assert_is_not_none(admin_cat_acl, "TH2's ACLList should contain an Admin CAT entry with Administer privilege")
        has_empty_targets = (admin_cat_acl.targets == NullValue) or (len(admin_cat_acl.targets) == 0)
        asserts.assert_true(has_empty_targets, "Admin CAT ACL should have empty/null targets list")

        # Store the admin CAT version for potential future use
        self.th_admin_cat_version = th_admin_cat_version

        # Step 20: TH reads GroupList attribute from DUT
        self.step(20, "TH reads GroupList attribute from DUT")

        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.GroupList)],
            returnClusterObject=True)
        group_list_step21 = response[1][Clusters.JointFabricDatastore].groupList

        # Find the Admin CAT group entry
        admin_cat_group = None
        admin_cat_group_id = None
        admin_cat_version = None

        for group in group_list_step21:
            # Admin CAT group has groupCAT = 0xFFFF
            if group.groupCAT == 0xFFFF:
                admin_cat_group = group
                admin_cat_group_id = group.groupID
                admin_cat_version = group.groupCATVersion
                break

        asserts.assert_is_not_none(admin_cat_group, "GroupList should contain an Admin CAT group entry")

        # Store the admin CAT group ID and version
        self.admin_cat_group_id = admin_cat_group_id
        self.admin_cat_version = admin_cat_version

        # Keep expected "next version" values for the retained negative Admin CAT check.
        new_admin_cat_version = self.admin_cat_version + 1
        new_admin_cat_subject = 0xFFFF_0000_0000_0000 | new_admin_cat_version

        # Step 21: TH reads NodeACLList and verifies new Admin CAT version was not applied
        self.step(21, "TH reads NodeACLList entries NodeId=th_node_id from DUT")

        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.NodeACLList)],
            returnClusterObject=True)
        node_acl_list_step23 = response[1][Clusters.JointFabricDatastore].nodeACLList

        # Find the ACL entry for th_node_id
        th2_acl_entry_step23 = None
        for acl_entry in node_acl_list_step23:
            if acl_entry.nodeID == self.th2_node_id:
                th2_acl_entry_step23 = acl_entry
                break

        asserts.assert_is_not_none(th2_acl_entry_step23, "ACL entry for th_node_id should exist")

        # Verify no Admin CAT ACL with new version exists
        admin_cat_acl_pending = None

        if (self._enum_equals(th2_acl_entry_step23.ACLEntry.privilege,
                              Clusters.JointFabricDatastore.Enums.DatastoreAccessControlEntryPrivilegeEnum.kAdminister) and
            self._enum_equals(th2_acl_entry_step23.ACLEntry.authMode,
                              Clusters.JointFabricDatastore.Enums.DatastoreAccessControlEntryAuthModeEnum.kCase) and
                new_admin_cat_subject in th2_acl_entry_step23.ACLEntry.subjects and
                len(th2_acl_entry_step23.ACLEntry.targets) == 0):
            admin_cat_acl_pending = th2_acl_entry_step23

        asserts.assert_is_none(admin_cat_acl_pending,
                               f"NodeACLList should not contain Admin CAT ACL with new version {new_admin_cat_version}")

        # Step 22: TH sends RemoveACLFromNode command with non-existent NodeId
        self.step(22, "TH sends RemoveACLFromNode command to DUT with NodeId=0x0000_0000_0000_000a (not found), ListID=th_acllist_id")

        try:
            await self.send_single_cmd(
                cmd=Clusters.JointFabricDatastore.Commands.RemoveACLFromNode(
                    nodeID=0x0000_0000_0000_000a,
                    listID=self.th_acllist_id
                ),
                dev_ctrl=self.devCtrlEcoA,
                node_id=1,
                endpoint=1,
            )
            asserts.fail("Expected CONSTRAINT_ERROR for non-existent node")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 "RemoveACLFromNode with non-existent node should return CONSTRAINT_ERROR")

        # Step 23: TH sends RemoveACLFromNode command with valid NodeId
        self.step(23, "TH sends RemoveACLFromNode command to DUT with NodeId=th_node_id, ListID=th_acllist_id")

        await self.send_single_cmd(
            cmd=Clusters.JointFabricDatastore.Commands.RemoveACLFromNode(
                nodeID=self.th2_node_id,
                listID=self.th_acllist_id
            ),
            dev_ctrl=self.devCtrlEcoA,
            node_id=1,
            endpoint=1,
        )

        # Step 24: TH reads NodeACLList and ACLList from TH2 to verify removal
        self.step(24, "TH reads NodeACLList entries with NodeId=th_node_id from DUT and reads ACLList from TH2")

        removed_acl = Clusters.JointFabricDatastore.Structs.DatastoreAccessControlEntryStruct(
            privilege=Clusters.JointFabricDatastore.Enums.DatastoreAccessControlEntryPrivilegeEnum.kManage,
            authMode=Clusters.JointFabricDatastore.Enums.DatastoreAccessControlEntryAuthModeEnum.kCase,
            subjects=[self.th2_node_id],
            targets=[Clusters.JointFabricDatastore.Structs.DatastoreAccessControlTargetStruct(
                cluster=NullValue,
                endpoint=0,
                deviceType=NullValue,
            )],
        )

        acl_with_listid_found = True
        removed_acl_still_on_th2 = True
        th2_normalized_acls_step28 = []
        refresh_issued = False

        # Removal to datastore is immediate, but mirror to TH2 can lag.
        for _ in range(30):
            response = await self.devCtrlEcoA.ReadAttribute(
                nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.NodeACLList)],
                returnClusterObject=True)
            node_acl_list_step28 = response[1][Clusters.JointFabricDatastore].nodeACLList
            acl_with_listid_found = any(acl.listID == self.th_acllist_id for acl in node_acl_list_step28)

            response = await self.devCtrlEcoA.ReadAttribute(
                nodeId=self.th2_node_id, attributes=[(0, Clusters.AccessControl.Attributes.Acl)],
                returnClusterObject=True)
            th2_acl_list_step28 = response[0][Clusters.AccessControl].acl
            th2_normalized_acls_step28 = [self._normalize_acl_entry(acl) for acl in th2_acl_list_step28]

            removed_acl_normalized = self._normalize_acl_entry(removed_acl)
            removed_acl_still_on_th2 = removed_acl_normalized in th2_normalized_acls_step28

            if not acl_with_listid_found and not removed_acl_still_on_th2:
                break

            # If datastore is already clean but TH2 still stale, nudge sync once.
            if not acl_with_listid_found and removed_acl_still_on_th2 and not refresh_issued:
                await self.send_single_cmd(
                    cmd=Clusters.JointFabricDatastore.Commands.RefreshNode(nodeID=self.th2_node_id),
                    dev_ctrl=self.devCtrlEcoA,
                    node_id=1,
                    endpoint=1,
                )
                refresh_issued = True

            await asyncio.sleep(1)

        asserts.assert_false(acl_with_listid_found,
                             f"NodeACLList should NOT contain ACL entry with ListID={self.th_acllist_id}")

        # Verify the ACL matching step 17 is NOT in TH2's ACLList
        removed_acl_normalized = self._normalize_acl_entry(removed_acl)
        asserts.assert_not_in(
            removed_acl_normalized,
            th2_normalized_acls_step28,
            f"TH2's ACLList should NOT contain the ACL entry that was removed. Last observed ACLs: {th2_normalized_acls_step28}")

        # Step 25: TH sends UpdateEndpointForNode command with non-existent NodeId
        self.step(25, "TH sends UpdateEndpointForNode command to DUT with NodeId=0x0000_0000_0000_000a, EndpointID=th_app_endpoint_id")

        endpoint_struct = Clusters.JointFabricDatastore.Commands.UpdateEndpointForNode(
            nodeID=0x0000_0000_0000_000a,
            endpointID=self.th_app_endpoint_id,
            friendlyName="tc-jf-2.5-ep"
        )

        try:
            await self.send_single_cmd(
                cmd=endpoint_struct,
                dev_ctrl=self.devCtrlEcoA,
                node_id=1,
                endpoint=1,
            )
            asserts.fail("Expected CONSTRAINT_ERROR for non-existent node")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 "UpdateEndpointForNode with non-existent node should return CONSTRAINT_ERROR")

        # Step 26: TH sends UpdateEndpointForNode command with valid NodeId
        self.step(26, "TH sends UpdateEndpointForNode command to DUT with NodeId=th_node_id, EndpointID=th_app_endpoint_id")

        endpoint_to_update = Clusters.JointFabricDatastore.Commands.UpdateEndpointForNode(
            nodeID=self.th2_node_id,
            endpointID=self.th_app_endpoint_id,
            friendlyName="tc-jf-2.5-ep"
        )

        await self.send_single_cmd(
            cmd=endpoint_to_update,
            dev_ctrl=self.devCtrlEcoA,
            node_id=1,
            endpoint=1,
        )

        # Step 27: TH reads NodeEndpointList and verifies the updated endpoint
        self.step(27, "TH reads NodeEndpointList attribute from DUT")

        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.NodeEndpointList)],
            returnClusterObject=True)
        node_endpoint_list_step31 = response[1][Clusters.JointFabricDatastore].nodeEndpointList

        # Find the endpoint entry for th_node_id
        th2_endpoint_entry_step31 = None
        for endpoint_entry in node_endpoint_list_step31:
            if endpoint_entry.nodeID == self.th2_node_id and endpoint_entry.endpointID == self.th_app_endpoint_id and endpoint_entry.friendlyName == "tc-jf-2.5-ep":
                th2_endpoint_entry_step31 = endpoint_entry
                break

        asserts.assert_is_not_none(th2_endpoint_entry_step31, "Endpoint entry for th_node_id should exist")

        # Step 28: TH sends AddGroupIDToEndpointForNode command with non-existent NodeId
        self.step(28, "TH sends AddGroupIDToEndpointForNode command to DUT with NodeId=0x0000_0000_0000_000a (not found), EndpointID=th_app_endpoint_id, GroupID=th_group_id")

        try:
            await self.send_single_cmd(
                cmd=Clusters.JointFabricDatastore.Commands.AddGroupIDToEndpointForNode(
                    nodeID=0x0000_0000_0000_000a,
                    endpointID=self.th_app_endpoint_id,
                    groupID=self.th_group_id
                ),
                dev_ctrl=self.devCtrlEcoA,
                node_id=1,
                endpoint=1,
            )
            asserts.fail("Expected CONSTRAINT_ERROR for non-existent node")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 "AddGroupIDToEndpointForNode with non-existent node should return CONSTRAINT_ERROR")

        # Step 29: TH sends AddGroupIDToEndpointForNode command with valid NodeId
        self.step(29, "TH sends AddGroupIDToEndpointForNode command to DUT with NodeId=th_node_id, EndpointID=th_app_endpoint_id, GroupID=th_group_id")

        await self.send_single_cmd(
            cmd=Clusters.JointFabricDatastore.Commands.AddGroupIDToEndpointForNode(
                nodeID=self.th2_node_id,
                endpointID=self.th_app_endpoint_id,
                groupID=self.th_group_id
            ),
            dev_ctrl=self.devCtrlEcoA,
            node_id=1,
            endpoint=1,
        )

        await asyncio.sleep(2)  # Wait for the group addition to propagate

        # Step 30: TH reads EndpointGroupIDList and verifies the group entry
        self.step(30, "TH reads EndpointGroupIDList attribute from DUT")

        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.EndpointGroupIDList)],
            returnClusterObject=True)
        endpoint_group_list_step34 = response[1][Clusters.JointFabricDatastore].endpointGroupIDList

        # Find the entry for th_node_id and th_app_endpoint_id
        target_endpoint_group = None
        for entry in endpoint_group_list_step34:
            if entry.nodeID == self.th2_node_id and entry.endpointID == self.th_app_endpoint_id:
                target_endpoint_group = entry
                break

        asserts.assert_is_not_none(target_endpoint_group,
                                   f"EndpointGroupIDList should contain entry for NodeId={self.th2_node_id} and EndpointID={self.th_app_endpoint_id}")

        group_id = target_endpoint_group.groupID
        asserts.assert_equal(self.th_group_id, group_id, "GroupID should match the one added to the endpoint")

        # Verify Status is Committed
        self._assert_enum_equal(
            target_endpoint_group.statusEntry.state,
            Clusters.JointFabricDatastore.Enums.DatastoreStateEnum.kCommitted,
            "Status should be Committed")

        # Step 31: TH reads GroupList from the given endpoint on TH2
        self.step(31, "TH reads GroupList from the given endpoint on TH2")

        descriptor_response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=self.th2_node_id,
            attributes=[(self.th_app_endpoint_id, Clusters.Descriptor.Attributes.ServerList)],
            returnClusterObject=True)
        th2_server_list = descriptor_response[self.th_app_endpoint_id][Clusters.Descriptor].serverList
        asserts.assert_in(
            Clusters.Groups.id,
            th2_server_list,
            f"TH2 endpoint {self.th_app_endpoint_id} must support Groups cluster for step 35")

        group_response = await self.send_single_cmd(
            cmd=Clusters.Groups.Commands.GetGroupMembership(groupList=[self.th_group_id]),
            dev_ctrl=self.devCtrlEcoA,
            node_id=self.th2_node_id,
            endpoint=self.th_app_endpoint_id,
        )
        asserts.assert_in(
            self.th_group_id,
            group_response.groupList,
            f"Group {self.th_group_id} should be present on TH2 endpoint {self.th_app_endpoint_id}")
        # Note: The status "Committed" is implied by the group being present in the table

        # Step 32: TH reads KeySetList from the given endpoint on TH2
        self.step(32, "TH reads KeySetList from the given endpoint on TH2")

        # Read the actual KeySet for th_keyset_id using KeySetRead
        keyset_read_response = await self.send_single_cmd(
            cmd=Clusters.GroupKeyManagement.Commands.KeySetRead(groupKeySetID=self.th_keyset_id),
            dev_ctrl=self.devCtrlEcoA,
            node_id=self.th2_node_id,
            endpoint=0,
        )
        th2_keyset = keyset_read_response.groupKeySet

        asserts.assert_is_not_none(th2_keyset,
                                   f"KeySetRead should return KeySetID={self.th_keyset_id}")
        asserts.assert_equal(th2_keyset.groupKeySetID, self.th_keyset_id,
                             f"KeySet groupKeySetID should be {self.th_keyset_id}")
        # Note: epoch key bytes (epochKey0-2) are always returned as Null by KeySetRead per spec
        # (they are write-only). th_keyset_epoch was stored at setup time.

        # Step 33: TH reads NodeKeySetList and verifies Status Success
        self.step(33, "TH reads NodeKeySetList attribute from DUT for NodeId=th_node_id and KeySetID=th_keyset_id")

        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.NodeKeySetList)],
            returnClusterObject=True)
        node_keyset_list_step38 = response[1][Clusters.JointFabricDatastore].nodeKeySetList

        # Find the keyset entry for th_node_id and th_keyset_id
        th2_keyset_entry = None
        for entry in node_keyset_list_step38:
            if entry.nodeID == self.th2_node_id and entry.groupKeySetID == self.th_keyset_id:
                th2_keyset_entry = entry
                break

        asserts.assert_is_not_none(th2_keyset_entry, "NodeKeySetList should contain entry for th_node_id")
        self._assert_enum_equal(
            th2_keyset_entry.statusEntry.state,
            Clusters.JointFabricDatastore.Enums.DatastoreStateEnum.kCommitted,
            "StatusEntry should be Success")

        # Step 34: TH reads KeySetList from TH2 and verifies new EpochKey2
        self.step(34, "TH reads KeySetList from the given endpoint on TH2")

        # Read the actual KeySet for th_keyset_id using KeySetRead
        keyset_read_response = await self.send_single_cmd(
            cmd=Clusters.GroupKeyManagement.Commands.KeySetRead(groupKeySetID=self.th_keyset_id),
            dev_ctrl=self.devCtrlEcoA,
            node_id=self.th2_node_id,
            endpoint=0,
        )
        th2_keyset = keyset_read_response.groupKeySet

        asserts.assert_is_not_none(th2_keyset,
                                   f"KeySetRead should return KeySetID={self.th_keyset_id}")
        asserts.assert_equal(th2_keyset.groupKeySetID, self.th_keyset_id,
                             f"KeySet groupKeySetID should be {self.th_keyset_id}")
        # Note: epoch key bytes (epochKey0-2) are always returned as Null by KeySetRead per spec
        # (they are write-only). th_keyset_epoch was stored at setup time.

        # Step 35: TH sends RemoveGroupIDFromEndpointForNode command with non-existent NodeId
        self.step(35, "TH sends RemoveGroupIDFromEndpointForNode command to DUT with NodeId=0x0000_0000_0000_000a, EndpointID=th_app_endpoint_id, GroupID=th_group_id")

        try:
            await self.send_single_cmd(
                cmd=Clusters.JointFabricDatastore.Commands.RemoveGroupIDFromEndpointForNode(
                    nodeID=0x0000_0000_0000_000a,
                    endpointID=self.th_app_endpoint_id,
                    groupID=self.th_group_id
                ),
                dev_ctrl=self.devCtrlEcoA,
                node_id=1,
                endpoint=1,
            )
            asserts.fail("Expected CONSTRAINT_ERROR for non-existent node")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 "RemoveGroupIDFromEndpointForNode with non-existent node should return CONSTRAINT_ERROR")

        # Step 36: TH sends RemoveGroupIDFromEndpointForNode command with valid NodeId
        self.step(36, "TH sends RemoveGroupIDFromEndpointForNode command to DUT with NodeId=th_node_id, EndpointID=th_app_endpoint_id, GroupID=th_group_id")

        await self.send_single_cmd(
            cmd=Clusters.JointFabricDatastore.Commands.RemoveGroupIDFromEndpointForNode(
                nodeID=self.th2_node_id,
                endpointID=self.th_app_endpoint_id,
                groupID=self.th_group_id
            ),
            dev_ctrl=self.devCtrlEcoA,
            node_id=1,
            endpoint=1,
        )

        # Step 37: TH reads EndpointGroupIDList and verifies group removal
        self.step(37, "TH reads EndpointGroupIDList attribute from DUT")

        max_retries = 10
        removal_observed = False
        remaining_matches = []
        for _ in range(max_retries):
            response = await self.devCtrlEcoA.ReadAttribute(
                nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.EndpointGroupIDList)],
                returnClusterObject=True)
            endpoint_group_list_step43 = response[1][Clusters.JointFabricDatastore].endpointGroupIDList

            remaining_matches = [
                entry for entry in endpoint_group_list_step43
                if entry.nodeID == self.th2_node_id and entry.endpointID == self.th_app_endpoint_id and entry.groupID == self.th_group_id
            ]

            if len(remaining_matches) == 0:
                removal_observed = True
                break

            await asyncio.sleep(1)

        asserts.assert_true(
            removal_observed,
            f"EndpointGroupIDList still contains NodeID={self.th2_node_id}, EndpointID={self.th_app_endpoint_id}, "
            f"GroupID={self.th_group_id} after {max_retries}s; remaining entries={remaining_matches}")

        # Step 38: TH reads GroupList from TH2 and verifies group removal
        self.step(38, "TH reads from TH2 reads the GroupList from the given endpoint")

        descriptor_response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=self.th2_node_id,
            attributes=[(self.th_app_endpoint_id, Clusters.Descriptor.Attributes.ServerList)],
            returnClusterObject=True)
        th2_server_list_step44 = descriptor_response[self.th_app_endpoint_id][Clusters.Descriptor].serverList
        asserts.assert_in(
            Clusters.Groups.id,
            th2_server_list_step44,
            f"TH2 endpoint {self.th_app_endpoint_id} must support Groups cluster for step 44")

        get_group_cmd = Clusters.Groups.Commands.GetGroupMembership(
            groupList=[self.th_group_id]
        )

        group_resp = await self.send_single_cmd(
            cmd=get_group_cmd,
            dev_ctrl=self.devCtrlEcoA,
            node_id=self.th2_node_id,
            endpoint=self.th_app_endpoint_id,
        )

        asserts.assert_not_in(self.th_group_id, group_resp.groupList,
                              f"Group {self.th_group_id} should not be present on TH2")

        # Step 39: TH reads KeySetList from TH2 and verifies keyset does not exist
        self.step(39, "TH reads KeySetList from the given endpoint on TH2")

        # Read the actual KeySet for th_keyset_id using KeySetRead
        try:
            await self.send_single_cmd(
                cmd=Clusters.GroupKeyManagement.Commands.KeySetRead(groupKeySetID=self.th_keyset_id),
                dev_ctrl=self.devCtrlEcoA,
                node_id=self.th2_node_id,
                endpoint=0,
            )
            asserts.fail(f"Expected KeySetRead to fail with NotFound for KeySetID={self.th_keyset_id}")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Expected Status.NotFound for KeySetID={self.th_keyset_id}, got {e.status}")

        # Step 40: TH sends AddBindingToEndpointForNode command with non-existent NodeId
        self.step(40, "TH sends AddBindingToEndpointForNode command to DUT with NodeId=0x0000_0000_0000_000a and EndpointID=th_app_endpoint_id")

        binding_struct = Clusters.JointFabricDatastore.Structs.DatastoreBindingTargetStruct(
            node=self.th2_node_id,
            group=None,
            endpoint=0x01,
            cluster=None
        )

        try:
            await self.send_single_cmd(
                cmd=Clusters.JointFabricDatastore.Commands.AddBindingToEndpointForNode(
                    nodeID=0x0000_0000_0000_000a,
                    endpointID=self.th_app_endpoint_id,
                    binding=binding_struct
                ),
                dev_ctrl=self.devCtrlEcoA,
                node_id=1,
                endpoint=1,
            )
            asserts.fail("Expected CONSTRAINT_ERROR for non-existent node")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 "AddBindingToEndpointForNode with non-existent node should return CONSTRAINT_ERROR")

        # Step 41: TH sends AddBindingToEndpointForNode command with valid NodeId
        self.step(41, "TH sends AddBindingToEndpointForNode command to DUT with NodeId=th_node_id and EndpointID=th_app_endpoint_id")

        binding_to_add = Clusters.JointFabricDatastore.Structs.DatastoreBindingTargetStruct(
            node=self.th2_node_id,
            group=None,
            endpoint=0x01,
            cluster=None
        )

        await self.send_single_cmd(
            cmd=Clusters.JointFabricDatastore.Commands.AddBindingToEndpointForNode(
                nodeID=self.th2_node_id,
                endpointID=self.th_app_endpoint_id,
                binding=binding_to_add
            ),
            dev_ctrl=self.devCtrlEcoA,
            node_id=1,
            endpoint=1,
        )

        # Step 42: TH reads EndpointBindingList from DUT and BindingList from TH2
        self.step(42, "TH reads EndpointBindingList attribute from DUT and from TH2 reads the BindingList from the given endpoint")

        expected_binding = self._normalize_binding_target(binding_to_add)
        target_endpoint_binding = None
        binding_committed = False

        for _ in range(10):
            response = await self.devCtrlEcoA.ReadAttribute(
                nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.EndpointBindingList)],
                returnClusterObject=True)
            endpoint_binding_list_step48 = response[1][Clusters.JointFabricDatastore].endpointBindingList

            for entry in endpoint_binding_list_step48:
                if (entry.nodeID == self.th2_node_id and entry.endpointID == self.th_app_endpoint_id and
                        self._normalize_binding_target(entry.binding) == expected_binding):
                    target_endpoint_binding = entry
                    self.th_binding_list_id = entry.listID
                    if (entry.statusEntry.state ==
                            Clusters.JointFabricDatastore.Enums.DatastoreStateEnum.kCommitted):
                        binding_committed = True
                    break

            if binding_committed:
                break

            await asyncio.sleep(1)

        asserts.assert_is_not_none(target_endpoint_binding,
                                   "EndpointBindingList should contain the binding added in step 47")
        asserts.assert_true(binding_committed, "StatusEntry should change to Committed")

        asserts.assert_equal(self._normalize_binding_target(target_endpoint_binding.binding), expected_binding,
                             "DUT mirrored binding should exactly match the requested binding values")

        descriptor_response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=self.th2_node_id,
            attributes=[(self.th_app_endpoint_id, Clusters.Descriptor.Attributes.ServerList)],
            returnClusterObject=True)
        th2_server_list_step48 = descriptor_response[self.th_app_endpoint_id][Clusters.Descriptor].serverList
        asserts.assert_in(
            Clusters.Binding.id,
            th2_server_list_step48,
            f"TH2 endpoint {self.th_app_endpoint_id} must support Binding cluster for step 48")

        th2_matching_binding = None
        last_th2_binding_list = []
        for _ in range(10):
            th2_bindings_list_resp = await self.devCtrlEcoA.ReadAttribute(
                nodeId=self.th2_node_id,
                attributes=[(self.th_app_endpoint_id, Clusters.Binding.Attributes.Binding)],
                returnClusterObject=True
            )
            asserts.assert_true(
                self.th_app_endpoint_id in th2_bindings_list_resp and Clusters.Binding in th2_bindings_list_resp[
                    self.th_app_endpoint_id],
                f"TH2 endpoint {self.th_app_endpoint_id} reports Binding but Binding attribute is unavailable in step 48")

            th2_binding_list = th2_bindings_list_resp[self.th_app_endpoint_id][Clusters.Binding].binding
            last_th2_binding_list = [self._normalize_binding_target(binding) for binding in th2_binding_list]

            for binding in th2_binding_list:
                if self._normalize_binding_target(binding) == expected_binding:
                    th2_matching_binding = binding
                    break

            if th2_matching_binding is not None:
                break

            await asyncio.sleep(1)

        asserts.assert_is_not_none(
            th2_matching_binding,
            f"TH2's BindingList should contain the binding added in step 47. "
            f"Expected={expected_binding}, LastObserved={last_th2_binding_list}")

        # Step 43: TH sends RemoveBindingFromEndpointForNode command with non-existent NodeId
        self.step(43, "TH sends RemoveBindingFromEndpointForNode command to DUT with NodeId=0x0000_0000_0000_000a and EndpointID=th_app_endpoint_id, ListID=th_binding_list_id")

        try:
            await self.send_single_cmd(
                cmd=Clusters.JointFabricDatastore.Commands.RemoveBindingFromEndpointForNode(
                    nodeID=0x0000_0000_0000_000a,
                    endpointID=self.th_app_endpoint_id,
                    listID=self.th_binding_list_id
                ),
                dev_ctrl=self.devCtrlEcoA,
                node_id=1,
                endpoint=1,
            )
            asserts.fail("Expected CONSTRAINT_ERROR for non-existent node")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 "RemoveBindingFromEndpointForNode with non-existent node should return CONSTRAINT_ERROR")

        # Step 44: TH sends RemoveBindingFromEndpointForNode command with valid NodeId
        self.step(44, "TH sends RemoveBindingFromEndpointForNode command to DUT with NodeId=th_node_id and EndpointID=th_app_endpoint_id, ListID=th_binding_list_id")

        await self.send_single_cmd(
            cmd=Clusters.JointFabricDatastore.Commands.RemoveBindingFromEndpointForNode(
                nodeID=self.th2_node_id,
                endpointID=self.th_app_endpoint_id,
                listID=self.th_binding_list_id
            ),
            dev_ctrl=self.devCtrlEcoA,
            node_id=1,
            endpoint=1,
        )

        # Step 45: TH reads EndpointBindingList from DUT and BindingList from TH2 to verify removal
        self.step(45, "TH reads EndpointBindingList attribute from DUT and from TH2 reads the BindingList from the given endpoint")

        # Poll DUT EndpointBindingList until the removed entry is absent.
        max_retries = 10
        dut_binding_removed = False
        for _ in range(max_retries):
            response = await self.devCtrlEcoA.ReadAttribute(
                nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.EndpointBindingList)],
                returnClusterObject=True)
            endpoint_binding_list_step51 = response[1][Clusters.JointFabricDatastore].endpointBindingList

            matching_entries = [
                entry for entry in endpoint_binding_list_step51
                if entry.nodeID == self.th2_node_id and entry.endpointID == self.th_app_endpoint_id and entry.listID == self.th_binding_list_id
            ]

            if len(matching_entries) == 0:
                dut_binding_removed = True
                break

            await asyncio.sleep(1)

        asserts.assert_true(
            dut_binding_removed,
            f"EndpointBindingList should not contain ListID={self.th_binding_list_id} for NodeID={self.th2_node_id}, "
            f"EndpointID={self.th_app_endpoint_id} after {max_retries}s")

        descriptor_response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=self.th2_node_id,
            attributes=[(self.th_app_endpoint_id, Clusters.Descriptor.Attributes.ServerList)],
            returnClusterObject=True)
        th2_server_list_step51 = descriptor_response[self.th_app_endpoint_id][Clusters.Descriptor].serverList
        asserts.assert_in(
            Clusters.Binding.id,
            th2_server_list_step51,
            f"TH2 endpoint {self.th_app_endpoint_id} must support Binding cluster for step 51")

        # Poll TH2 BindingList until the removed binding is absent.
        th2_binding_removed = False
        for _ in range(max_retries):
            th2_bindings_list_resp = await self.devCtrlEcoA.ReadAttribute(
                nodeId=self.th2_node_id,
                attributes=[(self.th_app_endpoint_id, Clusters.Binding.Attributes.Binding)],
                returnClusterObject=True
            )
            asserts.assert_true(
                self.th_app_endpoint_id in th2_bindings_list_resp and Clusters.Binding in th2_bindings_list_resp[
                    self.th_app_endpoint_id],
                f"TH2 endpoint {self.th_app_endpoint_id} reports Binding but Binding attribute is unavailable in step 51")

            th2_binding_list = th2_bindings_list_resp[self.th_app_endpoint_id][Clusters.Binding].binding
            th2_matching_binding_found = any(
                self._normalize_binding_target(binding) == expected_binding for binding in th2_binding_list
            )

            if not th2_matching_binding_found:
                th2_binding_removed = True
                break

            await asyncio.sleep(1)

        asserts.assert_true(th2_binding_removed,
                            f"TH2 BindingList still contains removed binding after {max_retries}s: {expected_binding}")

        # Step 46: TH sends RemoveNode command with non-existent NodeId
        self.step(46, "TH sends RemoveNode command to DUT with NodeId=0x0000_0000_0000_000a")

        try:
            await self.send_single_cmd(
                cmd=Clusters.JointFabricDatastore.Commands.RemoveNode(nodeID=0x0000_0000_0000_000a),
                dev_ctrl=self.devCtrlEcoA,
                node_id=1,
                endpoint=1,
            )
            asserts.fail("Expected CONSTRAINT_ERROR for non-existent node")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 "RemoveNode with non-existent node should return CONSTRAINT_ERROR")

        # Step 47: TH sends RemoveNode command with valid NodeId
        self.step(47, "TH sends RemoveNode command to DUT with NodeId=th_node_id")

        await self.send_single_cmd(
            cmd=Clusters.JointFabricDatastore.Commands.RemoveNode(nodeID=self.th2_node_id),
            dev_ctrl=self.devCtrlEcoA,
            node_id=1,
            endpoint=1,
        )

        # Step 48: TH reads NodeList and verifies node removal
        self.step(48, "TH reads NodeList attribute from DUT")

        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.NodeList)],
            returnClusterObject=True)
        node_list_step54 = response[1][Clusters.JointFabricDatastore].nodeList

        # Verify th_node_id is NOT in the list
        node_found = False
        for node in node_list_step54:
            if node.nodeID == self.th2_node_id:
                node_found = True
                break

        asserts.assert_false(node_found, f"NodeList should NOT contain entry with NodeId={self.th2_node_id}")


if __name__ == "__main__":
    default_matter_test_main()
