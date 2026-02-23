#
#    Copyright (c) 2024 Project CHIP Authors
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
from matter.interaction_model import InteractionModelError, Status
# from matter.interaction_model import InteractionModelError
from matter.storage import VolatileTemporaryPersistentStorage
from matter.testing.apps import AppServerSubprocess, JFControllerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_JFDS_2_2(MatterBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.fabric_a_ctrl = None
        self.storage_fabric_a = self.user_params.get("fabric_a_storage", None)
        self.fabric_a_server_app = None
        self.devCtrlEcoA = None
        self.certAuthorityManagerA = None
        self.fabric_a_persistent_storage = None

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
            log.info("Temporary storage directory: %s", self.storage_fabric_a)

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
            "JFC_A",  # Name of the controller instance, used for logging purposes in the JF-Controller app:w
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
        if self.fabric_a_admin is not None:
            self.fabric_a_admin.terminate()
        if self.fabric_a_ctrl is not None:
            self.fabric_a_ctrl.terminate()
        if self.fabric_a_server_app is not None:
            self.fabric_a_server_app.terminate()

        super().teardown_class()

    def steps_TC_JFDS_2_2(self) -> list[TestStep]:
        return [
            TestStep("1", "TH reads KeySetList attribute from DUT",
                     "Verify that at least one entry is returned and an entry with GroupKeySetID=0 exists in the list."),
            TestStep("2", "TH sends AddKeySet command to DUT with GroupKeySetID=0x000a.",
                     "Verify that the DUT responds with Status as SUCCESS"),
            TestStep("3", "TH reads KeySetList attribute from DUT",
                     "Verify that one entry has been added and has values matching values added in step 2."),
            TestStep("4", "TH sends AddKeySet command to DUT with GroupKeySetID=0x000a.",
                     "Verify that the DUT responds with Status as CONSTRAINT_ERROR"),
            TestStep("5", "TH sends UpdateKeySet command to DUT with GroupKeySetID=0x000a.",
                     "Verify that the DUT responds with Status as SUCCESS and the respective DatastoreNodeKeySetEntry entry is set to Committed."),
            TestStep("6", "TH sends UpdateKeySet command to DUT with GroupKeySetID=0x000a and wrong GroupKeySecurityPolicy value.",
                     "Verify that the DUT responds with Status as CONSTRAINT_ERROR and the respective DatastoreNodeKeySetEntry entry is set to CommitFailed."),
            TestStep("7", "TH reads KeySetList attribute from DUT",
                     "Verify that the entry with GroupKeySetID=0x000a exists in the list and matches values added in step 5."),
            TestStep("8", "TH sends UpdateKeySet command to DUT with GroupKeySetID=0x0FFF.",
                     "Verify that the DUT responds with Status code NOT_FOUND."),
            TestStep("9", "TH sends RemoveKeySet command to DUT with GroupKeySetID=0x0FFF.",
                     "Verify that the DUT responds with Status code NOT_FOUND."),
            TestStep("10", "TH sends RemoveKeySet command to DUT with GroupKeySetID=0x0000.",
                     "Verify that the DUT responds with Status code CONSTRAINT_ERROR."),
            TestStep("11", "TH sends RemoveKeySet command to DUT with GroupKeySetID=0x000a.",
                     "Verify that the DUT responds with Status as SUCCESS."),
            TestStep("12", "TH reads KeySetList attribute from DUT",
                     "Verify that the entry with GroupKeySetID=0x000a no longer exists in the list."),
            TestStep("13", "TH sends AddKeySet command to DUT with GroupKeySetID=0x0000.",
                     "Verify that the DUT responds with Status code CONSTRAINT_ERROR."),
            TestStep("14", "TH sends RemoveKeySet command to DUT with GroupKeySetID=0x0000.",
                     "Verify that the DUT responds with Status code CONSTRAINT_ERROR."),
        ]

    @async_test_body
    async def test_TC_JFDS_2_2(self):
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

        self.step("1")
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.GroupKeySetList)],
            returnClusterObject=True)
        _groupKeySetList = response[1][Clusters.JointFabricDatastore].groupKeySetList
        step1_groupKeySetListLength = len(_groupKeySetList)
        asserts.assert_greater_equal(step1_groupKeySetListLength, 1, "GroupKeySetList must contain at least one entry!")
        _found = False
        for _entry in _groupKeySetList:
            if _entry.groupKeySetID == 0:
                _found = True
                break
        asserts.assert_true(_found, "No GroupKeySetID=0 entry found!")

        self.step("2")
        step2_groupKeySet = Clusters.JointFabricDatastore.Structs.DatastoreGroupKeySetStruct(
            groupKeySetID=0x000a,
            groupKeySecurityPolicy=Clusters.JointFabricDatastore.Enums.DatastoreGroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=b'00000000000000000000000000000000',
            epochStartTime0=2220000,
            epochKey1=b'11111111111111111111111111111111',
            epochStartTime1=2220001,
            epochKey2=b'22222222222222222222222222222222',
            epochStartTime2=2220002)
        cmd = Clusters.JointFabricDatastore.Commands.AddKeySet(step2_groupKeySet)
        await self.send_single_cmd(cmd=cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)

        self.step("3")
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.GroupKeySetList)],
            returnClusterObject=True)
        _groupKeySetList = response[1][Clusters.JointFabricDatastore].groupKeySetList
        asserts.assert_greater(len(_groupKeySetList), step1_groupKeySetListLength,
                               "An new entry was not added in groupKeySetList")
        _found = False
        for _item in _groupKeySetList:
            if _item == step2_groupKeySet:
                _found = True
                break
        asserts.assert_true(_found, "GroupKeySet from step2 was not found on DUT!")

        self.step("4")
        step4_groupKeySet = Clusters.JointFabricDatastore.Structs.DatastoreGroupKeySetStruct(
            groupKeySetID=0x000a,
            groupKeySecurityPolicy=Clusters.JointFabricDatastore.Enums.DatastoreGroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=b'00000000000000000000000000000000',
            epochStartTime0=2220000,
            epochKey1=b'11111111111111111111111111111111',
            epochStartTime1=2220001,
            epochKey2=b'99999999999999999999999999999999',
            epochStartTime2=2220002)
        cmd = Clusters.JointFabricDatastore.Commands.AddKeySet(step4_groupKeySet)
        # Verify that the DUT responds with Status as CONSTRAINT_ERROR
        try:
            await self.send_single_cmd(cmd=cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR but got {e.status}")

        self.step("5")
        step5_groupKeySet = Clusters.JointFabricDatastore.Structs.DatastoreGroupKeySetStruct(
            groupKeySetID=0x000a,
            groupKeySecurityPolicy=Clusters.JointFabricDatastore.Enums.DatastoreGroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=b'00000000000000000000000000000000',
            epochStartTime0=2220001,
            epochKey1=b'11111111111111111111111111111111',
            epochStartTime1=2220002,
            epochKey2=b'99999999999999999999999999999999',
            epochStartTime2=2220003)
        cmd = Clusters.JointFabricDatastore.Commands.UpdateKeySet(step5_groupKeySet)
        await self.send_single_cmd(cmd=cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)

        self.step("6")
        step6_groupKeySet = Clusters.JointFabricDatastore.Structs.DatastoreGroupKeySetStruct(
            groupKeySetID=0x000a,
            groupKeySecurityPolicy=2,
            epochKey0=b'00000000000000000000000000000000',
            epochStartTime0=2220001,
            epochKey1=b'11111111111111111111111111111111',
            epochStartTime1=2220002,
            epochKey2=b'99999999999999999999999999999999',
            epochStartTime2=2220003)
        cmd = Clusters.JointFabricDatastore.Commands.UpdateKeySet(step6_groupKeySet)
        try:
            await self.send_single_cmd(cmd=cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR but got {e.status}")

        self.step("7")
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.GroupKeySetList)],
            returnClusterObject=True)
        _groupKeySetList = response[1][Clusters.JointFabricDatastore].groupKeySetList

        _found = False
        for _item in _groupKeySetList:
            if _item == step5_groupKeySet:
                _found = True
                break
        asserts.assert_true(_found, "GroupKeySet from step5 was not found on DUT!")

        self.step("8")
        step8_groupKeySet = Clusters.JointFabricDatastore.Structs.DatastoreGroupKeySetStruct(
            groupKeySetID=0x0FFF,
            groupKeySecurityPolicy=Clusters.JointFabricDatastore.Enums.DatastoreGroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=b'00000000000000000000000000000000',
            epochStartTime0=2220001,
            epochKey1=b'11111111111111111111111111111111',
            epochStartTime1=2220002,
            epochKey2=b'99999999999999999999999999999999',
            epochStartTime2=2220003)
        cmd = Clusters.JointFabricDatastore.Commands.UpdateKeySet(step8_groupKeySet)
        try:
            await self.send_single_cmd(cmd=cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)
            asserts.fail("Expected NOT_FOUND but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Expected NOT_FOUND but got {e.status}")

        self.step("9")
        cmd = Clusters.JointFabricDatastore.Commands.RemoveKeySet(0x0fff)
        try:
            await self.send_single_cmd(cmd=cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)
            asserts.fail("Expected NOT_FOUND but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 f"Expected NOT_FOUND but got {e.status}")

        self.step("10")
        cmd = Clusters.JointFabricDatastore.Commands.RemoveKeySet(0x0000)
        try:
            await self.send_single_cmd(cmd=cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR but got {e.status}")

        self.step("11")
        cmd = Clusters.JointFabricDatastore.Commands.RemoveKeySet(0x000a)
        await self.send_single_cmd(cmd=cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)

        self.step("12")
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.GroupKeySetList)],
            returnClusterObject=True)
        _groupKeySetList = response[1][Clusters.JointFabricDatastore].groupKeySetList
        _found = False
        for _item in _groupKeySetList:
            if _item.groupKeySetID == 0x000a:
                _found = True
                break
        asserts.assert_false(_found, "GroupKeySetID=0x000a should not exist in the list")

        self.step("13")
        _groupKeySet = Clusters.JointFabricDatastore.Structs.DatastoreGroupKeySetStruct(
            groupKeySetID=0x0000,
            groupKeySecurityPolicy=Clusters.JointFabricDatastore.Enums.DatastoreGroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=b'00000000000000000000000000000000',
            epochStartTime0=2220000,
            epochKey1=b'11111111111111111111111111111111',
            epochStartTime1=2220001,
            epochKey2=b'22222222222222222222222222222222',
            epochStartTime2=2220002)
        cmd = Clusters.JointFabricDatastore.Commands.AddKeySet(_groupKeySet)
        try:
            await self.send_single_cmd(cmd=cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR but got {e.status}")

        self.step("14")
        cmd = Clusters.JointFabricDatastore.Commands.RemoveKeySet(0x0000)
        try:
            await self.send_single_cmd(cmd=cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR but got {e.status}")


if __name__ == "__main__":
    default_matter_test_main()
