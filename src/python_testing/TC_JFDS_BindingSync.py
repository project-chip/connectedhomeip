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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     script-args: >
#       --string-arg jfa_server_app:${JF_ADMIN_APP}
#       --string-arg jfc_server_app:${JF_CONTROL_APP}
#       --string-arg th_server_app:${ALL_CLUSTERS_APP}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
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
from matter import CertificateAuthority, ChipDeviceCtrl
from matter.clusters.Types import NullValue
from matter.storage import VolatileTemporaryPersistentStorage
from matter.testing.apps import AppServerSubprocess, JFControllerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterTestCommissioner
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

JFDS = Clusters.JointFabricDatastore


class TC_JFDS_BindingSync(MatterTestCommissioner):
    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.fabric_a_ctrl = None
        self.fabric_a_admin = None
        self.target_app = None
        self.devCtrlEcoA = None
        self.certAuthorityManagerA = None
        self.fabric_a_persistent_storage = None

        jfc_server_app = self.user_params.get("jfc_server_app", None)
        jfa_server_app = self.user_params.get("jfa_server_app", None)
        th_server_app = self.user_params.get("th_server_app", None)
        for name, path in (("jfc_server_app", jfc_server_app), ("jfa_server_app", jfa_server_app),
                           ("th_server_app", th_server_app)):
            if not path:
                asserts.fail(f"This test requires --string-arg {name}:<path_to_app>")
            if not os.path.exists(path):
                asserts.fail(f"The path {path} does not exist")

        self.storage_directory = tempfile.TemporaryDirectory(prefix=self.__class__.__name__ + "_")
        self.storage = self.storage_directory.name
        log.info("Temporary storage directory: %s", self.storage)

        self.jfadmin_node_id = 1
        self.target_node_id = 2

        self.jfctrl_vid = random.randint(0x0001, 0xFFF0)
        jfadmin_passcode = random.randint(110220011, 110220999)
        rpc_server_port = str(self.get_random_port())
        self.fabric_a_admin = AppServerSubprocess(
            jfa_server_app,
            storage_dir=self.storage,
            port=self.get_random_port(),
            discriminator=random.randint(0, 4095),
            passcode=jfadmin_passcode,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", rpc_server_port])
        self.fabric_a_admin.start(expected_output="Server initialization complete", timeout=10)

        self.fabric_a_ctrl = JFControllerSubprocess(
            jfc_server_app,
            "JFC_A",
            rpc_server_port=rpc_server_port,
            storage_dir=self.storage,
            vendor_id=self.jfctrl_vid,
            extra_args=["--rpc-server-ip", "127.0.0.1"])
        self.fabric_a_ctrl.start(expected_output="CHIP task running", timeout=10)

        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork {self.jfadmin_node_id} {jfadmin_passcode} --anchor true",
            expected_output=f"[JF] Anchor Administrator (nodeId={self.jfadmin_node_id}) commissioned with success",
            timeout=10)

        jfcStorage = ConfigParser()
        jfcStorage.read(self.storage + '/chip_tool_config.alpha.ini')
        self.ecoACtrlStorage = {
            "sdk-config": {
                "ExampleOpCredsCAKey1": jfcStorage.get("Default", "ExampleOpCredsCAKey0"),
                "ExampleOpCredsICAKey1": jfcStorage.get("Default", "ExampleOpCredsICAKey0"),
                "ExampleCARootCert1": jfcStorage.get("Default", "ExampleCARootCert0"),
                "ExampleCAIntermediateCert1": jfcStorage.get("Default", "ExampleCAIntermediateCert0"),
            },
            "repl-config": {
                "caList": {
                    "1": [{"fabricId": 1, "vendorId": self.jfctrl_vid}]
                }
            }
        }
        self.ecoACATs = base64.b64decode(jfcStorage.get("Default", "CommissionerCATs"))[::-1].hex().strip('0')

        self.target_passcode = 20202021
        self.target_discriminator = random.randint(0, 4095)
        self.target_app = AppServerSubprocess(
            th_server_app,
            storage_dir=self.storage,
            port=self.get_random_port(),
            discriminator=self.target_discriminator,
            passcode=self.target_passcode)
        self.target_app.start(expected_output="Server initialization complete", timeout=30)

    def teardown_class(self):
        if self.devCtrlEcoA is not None:
            self.devCtrlEcoA.Shutdown()
        if self.certAuthorityManagerA is not None:
            self.certAuthorityManagerA.Shutdown()
        if self.fabric_a_persistent_storage is not None:
            self.fabric_a_persistent_storage.Shutdown()
        for app in (self.target_app, self.fabric_a_admin, self.fabric_a_ctrl):
            if app is not None:
                app.terminate()
        super().teardown_class()

    def _assert_admin_running(self):
        asserts.assert_is_none(self.fabric_a_admin.p.poll(), "JF Administrator app exited unexpectedly")

    async def _read_jfds(self, attribute):
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=self.jfadmin_node_id, attributes=[(self.jfds_endpoint, attribute)])
        return response[self.jfds_endpoint][JFDS][attribute]

    async def _wait_for_node_committed(self, timeout_s: float = 30):
        for _ in range(int(timeout_s * 2)):
            self._assert_admin_running()
            nodes = await self._read_jfds(JFDS.Attributes.NodeList)
            node = next((n for n in nodes if n.nodeID == self.target_node_id), None)
            if node is not None and node.commissioningStatusEntry.state == JFDS.Enums.DatastoreStateEnum.kCommitted:
                return
            await asyncio.sleep(0.5)
        asserts.fail("Target node did not reach the Committed state")

    async def _wait_for_acl_committed(self, timeout_s: float = 30):
        for _ in range(int(timeout_s * 2)):
            self._assert_admin_running()
            entries = await self._read_jfds(JFDS.Attributes.NodeACLList)
            if any(e.nodeID == self.target_node_id and e.statusEntry.state == JFDS.Enums.DatastoreStateEnum.kCommitted
                   for e in entries):
                return
            await asyncio.sleep(0.5)
        asserts.fail("ACL entry for the target node was not committed")

    async def _wait_for_target_binding_count(self, count: int, timeout_s: float = 30):
        for _ in range(int(timeout_s * 2)):
            self._assert_admin_running()
            bindings = await self.read_single_attribute(
                dev_ctrl=self.devCtrlEcoA, node_id=self.target_node_id, endpoint=0,
                attribute=Clusters.Binding.Attributes.Binding)
            if len(bindings) == count:
                return
            await asyncio.sleep(0.5)
        asserts.fail(f"Binding list on the target did not reach {count} entries")

    async def _add_binding(self, binding: JFDS.Structs.DatastoreBindingTargetStruct):
        await self.send_single_cmd(
            cmd=JFDS.Commands.AddBindingToEndpointForNode(nodeID=self.target_node_id, endpointID=1, binding=binding),
            dev_ctrl=self.devCtrlEcoA, node_id=self.jfadmin_node_id, endpoint=self.jfds_endpoint)

    @async_test_body
    async def test_TC_JFDS_BindingSync(self):
        """[TC-JFDS-BindingSync] Binding sync with a node whose Binding list is not empty"""
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

        descriptor_response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=self.jfadmin_node_id, attributes=[(Clusters.Descriptor)], returnClusterObject=True)
        self.jfds_endpoint = next((ep for ep, data in descriptor_response.items()
                                   if JFDS.id in data[Clusters.Descriptor].serverList), None)
        asserts.assert_is_not_none(self.jfds_endpoint, "JointFabricDatastore cluster not found on any endpoint")

        self.step(1, "Commission the target node onto the Joint Fabric and grant the JF Administrator Administer access")
        await self.devCtrlEcoA.CommissionOnNetwork(
            nodeId=self.target_node_id, setupPinCode=self.target_passcode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.target_discriminator)

        acl = await self.read_single_attribute(
            dev_ctrl=self.devCtrlEcoA, node_id=self.target_node_id, endpoint=0,
            attribute=Clusters.AccessControl.Attributes.Acl)
        acl.append(Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[self.jfadmin_node_id],
            targets=NullValue))
        await self.devCtrlEcoA.WriteAttribute(self.target_node_id, [(0, Clusters.AccessControl.Attributes.Acl(acl))])

        self.step(2, "Send AddPendingNode, AddACLToNode and RefreshNode for the target node",
                  expectation="Verify that the node reaches the Committed state")
        await self.send_single_cmd(
            cmd=JFDS.Commands.AddPendingNode(nodeID=self.target_node_id, friendlyName="binding-sync-target"),
            dev_ctrl=self.devCtrlEcoA, node_id=self.jfadmin_node_id, endpoint=self.jfds_endpoint)
        # RefreshNode writes the node's ACL back from the datastore and currently keeps only part of the node's existing
        # entries, so the access for the JF Administrator and this controller is registered in the datastore too.
        await self.send_single_cmd(
            cmd=JFDS.Commands.AddACLToNode(
                nodeID=self.target_node_id,
                ACLEntry=JFDS.Structs.DatastoreAccessControlEntryStruct(
                    privilege=JFDS.Enums.DatastoreAccessControlEntryPrivilegeEnum.kAdminister,
                    authMode=JFDS.Enums.DatastoreAccessControlEntryAuthModeEnum.kCase,
                    subjects=[self.jfadmin_node_id, self.devCtrlEcoA.nodeId],
                    targets=NullValue)),
            dev_ctrl=self.devCtrlEcoA, node_id=self.jfadmin_node_id, endpoint=self.jfds_endpoint)
        await self._wait_for_acl_committed()
        await self.send_single_cmd(
            cmd=JFDS.Commands.RefreshNode(nodeID=self.target_node_id),
            dev_ctrl=self.devCtrlEcoA, node_id=self.jfadmin_node_id, endpoint=self.jfds_endpoint)
        await self._wait_for_node_committed()

        self.step(3, "Send AddBindingToEndpointForNode with a first binding",
                  expectation="Verify that the target Binding list contains one entry")
        await self._add_binding(JFDS.Structs.DatastoreBindingTargetStruct(node=0x55, endpoint=1))
        await self._wait_for_target_binding_count(1)

        self.step(4, "Send AddBindingToEndpointForNode with a second binding",
                  expectation="Verify that the target Binding list contains two entries and the JF Administrator is still running")
        # The Binding list read back from the target is now non-empty, so this sync goes through the
        # path that converts each returned entry into a datastore entry.
        await self._add_binding(JFDS.Structs.DatastoreBindingTargetStruct(node=0x56, endpoint=1))
        await self._wait_for_target_binding_count(2)
        self._assert_admin_running()

        self.step(5, "Send RefreshNode for the target node",
                  expectation="Verify that the node reaches the Committed state and the JF Administrator is still running")
        await self.send_single_cmd(
            cmd=JFDS.Commands.RefreshNode(nodeID=self.target_node_id),
            dev_ctrl=self.devCtrlEcoA, node_id=self.jfadmin_node_id, endpoint=self.jfds_endpoint)
        await self._wait_for_node_committed()
        self._assert_admin_running()


if __name__ == "__main__":
    default_matter_test_main()
