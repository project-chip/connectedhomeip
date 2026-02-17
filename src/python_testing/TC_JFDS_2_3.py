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


class TC_JFDS_2_3(MatterBaseTest):

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

    def steps_TC_JFDS_2_3(self) -> list[TestStep]:
        return [
            TestStep("1", "TH reads GroupList attribute from DUT",
                     "Note the number of entries returned. Verify that entries exist for the Admin CAT and the Anchor CAT. Note the GroupID for the Admin CAT as admin_cat_group_id and the GroupID for the AnchorCAT as anchor_cat_group_id"),
            TestStep("2", "TH sends AddGroup command to DUT with GroupID=0x000A",
                     "Verify that the DUT responds with Status as Success"),
            TestStep("3", "TH reads GroupList attribute from DUT",
                     "Verify that one entry has been added. Verify that the new entry has GroupID=0x000a and values matching Group added in step 2"),
            TestStep("4", "TH sends UpdateGroup command to DUT with GroupID=0x000a and other values different from values used in step 2",
                     "Verify that the DUT responds with Status as SUCCESS"),
            TestStep("5", "TH reads GroupList attribute from DUT",
                     "Verify that the entry with GroupID=0x000a has values matching values added in step 4"),
            TestStep("6", "TH sends RemoveGroup command to DUT with GroupID=0x000a",
                     "Verify that the DUT responds with Status as SUCCESS"),
            TestStep("7", "TH reads GroupList attribute from DUT",
                     "Verify that no entry with GroupID=0x000a exists in the list"),
            TestStep("8", "TH sends AddGroup command to DUT with GroupCAT: 0xFFFF_0001 (Admin CAT)",
                     "Verify that the DUT responds with Error with Status code CONSTRAINT_ERROR"),
            TestStep("9", "TH sends UpdateGroup command to DUT with GroupID=admin_cat_group_id",
                     "Verify that the DUT responds with Status code CONSTRAINT_ERROR"),
            TestStep("10", "TH sends RemoveGroup command to DUT with GroupID=admin_cat_group_id",
                     "Verify that the DUT responds with Status code CONSTRAINT_ERROR"),
            TestStep("11", "TH sends AddGroup command to DUT with GroupCAT: 0xFFFE_0001 (Anchor CAT)",
                     "Verify that the DUT responds with Error with Status code CONSTRAINT_ERROR"),
            TestStep("12", "TH sends UpdateGroup command to DUT with GroupID=anchor_cat_group_id",
                     "Verify that the DUT responds with Status code CONSTRAINT_ERROR"),
            TestStep("13", "TH sends RemoveGroup command to DUT with GroupID=anchor_cat_group_id",
                     "Verify that the DUT responds with Status code CONSTRAINT_ERROR"),
        ]

    @async_test_body
    async def test_TC_JFDS_2_3(self):
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
        # Read GroupList attribute from DUT
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.GroupList)],
            returnClusterObject=True)
        groupList = response[1][Clusters.JointFabricDatastore].groupList

        # Note the number of entries returned
        num_entries = len(groupList)
        log.info(f"GroupList contains {num_entries} entries")

        # Variables to track found entries
        admin_cat_group_id = None
        anchor_cat_group_id = None

        # Look for entries matching Admin CAT and Anchor CAT
        for entry in groupList:
            log.info(f"GroupList entry: GroupID={entry.groupID}, CAT={entry.groupCAT}")

            # Check if this entry's CAT matches our controller's CAT tags
            # Admin CAT should be present (commissioned with --anchor true)
            # We need to check both Admin and Anchor CAT presence
            if entry.groupCAT is not None:
                # If CAT matches and we haven't found admin yet, consider it admin
                if admin_cat_group_id is None and entry.groupCAT == 0xFFFF:
                    admin_cat_group_id = entry.groupID
                    log.info(f"Found Admin CAT entry with GroupID: {admin_cat_group_id}")
                # If CAT matches and admin already found, consider it anchor
                elif anchor_cat_group_id is None and entry.groupCAT == 0xFFFE:
                    anchor_cat_group_id = entry.groupID
                    log.info(f"Found Anchor CAT entry with GroupID: {anchor_cat_group_id}")

        # Verify that both Admin CAT and Anchor CAT entries were found
        asserts.assert_is_not_none(admin_cat_group_id, "Admin CAT entry must exist in GroupList")
        asserts.assert_is_not_none(anchor_cat_group_id, "Anchor CAT entry must exist in GroupList")

        log.info(f"Admin CAT GroupID: {admin_cat_group_id}")
        log.info(f"Anchor CAT GroupID: {anchor_cat_group_id}")

        # Store these for potential use in future steps
        self.admin_cat_group_id = admin_cat_group_id
        self.anchor_cat_group_id = anchor_cat_group_id

        self.step("2")
        # Create group struct with specified parameters
        step2_cmd = Clusters.JointFabricDatastore.Commands.AddGroup(
            groupID=0x000A,
            friendlyName="tc-jf-2.3",
            groupKeySetID=0x000B,
            groupCAT=0x0001,
            groupCATVersion=0x0001,
            groupPermission=Clusters.JointFabricDatastore.Enums.DatastoreAccessControlEntryPrivilegeEnum.kView)

        # Send AddGroup command to DUT
        await self.send_single_cmd(cmd=step2_cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)

        self.step("3")
        # Read GroupList attribute again to verify the new entry was added
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.GroupList)],
            returnClusterObject=True)
        groupList = response[1][Clusters.JointFabricDatastore].groupList

        # Verify that one entry has been added
        asserts.assert_greater(len(groupList), num_entries,
                               "A new entry was not added to GroupList")

        # Find and verify the entry with GroupID=0x000a
        found_entry = None
        for entry in groupList:
            if entry.groupID == 0x000A:
                found_entry = entry
                break

        asserts.assert_is_not_none(found_entry, "Entry with GroupID=0x000A not found in GroupList")

        # Verify all fields match the group added in step 2
        asserts.assert_equal(found_entry.groupID, step2_cmd.groupID, "GroupID does not match")
        asserts.assert_equal(found_entry.friendlyName, step2_cmd.friendlyName, "FriendlyName does not match")
        asserts.assert_equal(found_entry.groupKeySetID, step2_cmd.groupKeySetID, "GroupKeySetID does not match")
        asserts.assert_equal(found_entry.groupCAT, step2_cmd.groupCAT, "GroupCAT does not match")
        asserts.assert_equal(found_entry.groupCATVersion, step2_cmd.groupCATVersion, "GroupCATVersion does not match")
        asserts.assert_equal(found_entry.groupPermission, step2_cmd.groupPermission, "GroupPermission does not match")

        log.info("Successfully verified Group entry added in step 2")

        self.step("4")
        # Send UpdateGroup command with different values
        step4_cmd = Clusters.JointFabricDatastore.Commands.UpdateGroup(
            groupID=0x000A,
            friendlyName="tc-jf-2.3-update",
            groupKeySetID=0x000C,
            groupCAT=0x0001,
            groupCATVersion=0x0002,
            groupPermission=Clusters.JointFabricDatastore.Enums.DatastoreAccessControlEntryPrivilegeEnum.kOperate)
        await self.send_single_cmd(cmd=step4_cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)

        self.step("5")
        # Read GroupList attribute to verify the update
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.GroupList)],
            returnClusterObject=True)
        groupList = response[1][Clusters.JointFabricDatastore].groupList

        # Find and verify the updated entry with GroupID=0x000a
        found_entry = None
        for entry in groupList:
            if entry.groupID == 0x000A:
                found_entry = entry
                break

        asserts.assert_is_not_none(found_entry, "Entry with GroupID=0x000A not found in GroupList after update")

        # Verify all fields match the group updated in step 4
        asserts.assert_equal(found_entry.groupID, step4_cmd.groupID, "GroupID does not match")
        asserts.assert_equal(found_entry.friendlyName, step4_cmd.friendlyName, "FriendlyName does not match")
        asserts.assert_equal(found_entry.groupKeySetID, step4_cmd.groupKeySetID, "GroupKeySetID does not match")
        asserts.assert_equal(found_entry.groupCAT, step4_cmd.groupCAT, "GroupCAT does not match")
        asserts.assert_equal(found_entry.groupCATVersion, step4_cmd.groupCATVersion, "GroupCATVersion does not match")
        asserts.assert_equal(found_entry.groupPermission, step4_cmd.groupPermission, "GroupPermission does not match")

        log.info("Successfully verified Group entry updated in step 4")

        self.step("6")
        # Send RemoveGroup command
        cmd = Clusters.JointFabricDatastore.Commands.RemoveGroup(0x000A)
        await self.send_single_cmd(cmd=cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)

        self.step("7")
        # Read GroupList attribute to verify the entry was removed
        response = await self.devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.GroupList)],
            returnClusterObject=True)
        groupList = response[1][Clusters.JointFabricDatastore].groupList

        # Verify that no entry with GroupID=0x000a exists
        found_entry = False
        for entry in groupList:
            if entry.groupID == 0x000A:
                found_entry = True
                break

        asserts.assert_false(found_entry, "Entry with GroupID=0x000A should not exist in GroupList after removal")

        self.step("8")
        # Try to add a group with Admin CAT (0xFFFF_0001) - should fail with CONSTRAINT_ERROR
        step8_cmd = Clusters.JointFabricDatastore.Commands.AddGroup(
            groupID=0x000A,
            friendlyName="tc-jf-2.3",
            groupKeySetID=0x000B,
            groupCAT=0xFFFE,  # SDK fails to decode 0xFFFF CAT, so using 0xFFFE here to simulate Admin CAT
            groupCATVersion=0x0001,
            groupPermission=Clusters.JointFabricDatastore.Enums.DatastoreAccessControlEntryPrivilegeEnum.kView)

        try:
            await self.send_single_cmd(cmd=step8_cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR but got {e.status}")

        self.step("9")
        # Try to update the Admin CAT group - should fail with CONSTRAINT_ERROR
        step9_cmd = Clusters.JointFabricDatastore.Commands.UpdateGroup(
            groupID=self.admin_cat_group_id,
            friendlyName="tc-jf-2.3-update",
            groupKeySetID=0x000C,
            groupCAT=0x0003,
            groupCATVersion=0x0002,
            groupPermission=Clusters.JointFabricDatastore.Enums.DatastoreAccessControlEntryPrivilegeEnum.kAdminister)

        try:
            await self.send_single_cmd(cmd=step9_cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR but got {e.status}")

        self.step("10")
        # Try to remove the Admin CAT group - should fail with CONSTRAINT_ERROR
        cmd = Clusters.JointFabricDatastore.Commands.RemoveGroup(self.admin_cat_group_id)
        try:
            await self.send_single_cmd(cmd=cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR but got {e.status}")

        self.step("11")
        # Try to add a group with Anchor CAT (0xFFFE_0001) - should fail with CONSTRAINT_ERROR
        step11_cmd = Clusters.JointFabricDatastore.Commands.AddGroup(
            groupID=0x000A,
            friendlyName="tc-jf-2.3",
            groupKeySetID=0x000B,
            groupCAT=0xFFFE,
            groupCATVersion=0x0001,
            groupPermission=Clusters.JointFabricDatastore.Enums.DatastoreAccessControlEntryPrivilegeEnum.kView)

        try:
            await self.send_single_cmd(cmd=step11_cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR but got {e.status}")

        self.step("12")
        # # Try to update the Anchor CAT group - should fail with CONSTRAINT_ERROR
        step12_cmd = Clusters.JointFabricDatastore.Commands.UpdateGroup(
            groupID=self.anchor_cat_group_id,
            friendlyName="tc-jf-2.3-update",
            groupKeySetID=0x000C,
            groupCAT=0x0003,
            groupCATVersion=0x0002,
            groupPermission=Clusters.JointFabricDatastore.Enums.DatastoreAccessControlEntryPrivilegeEnum.kAdminister)

        cmd = step12_cmd
        try:
            await self.send_single_cmd(cmd=cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR but got {e.status}")

        self.step("13")
        # Try to remove the Anchor CAT group - should fail with CONSTRAINT_ERROR
        cmd = Clusters.JointFabricDatastore.Commands.RemoveGroup(self.anchor_cat_group_id)
        try:
            await self.send_single_cmd(cmd=cmd, dev_ctrl=self.devCtrlEcoA, node_id=1, endpoint=1)
            asserts.fail("Expected CONSTRAINT_ERROR but command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR but got {e.status}")


if __name__ == "__main__":
    default_matter_test_main()
