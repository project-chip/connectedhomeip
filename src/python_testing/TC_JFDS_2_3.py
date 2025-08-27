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


class TC_JFDS_2_3(MatterBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.fabric_a_ctrl = None
        self.storage_fabric_a = self.user_params.get("fabric_a_storage", None)
        self.fabric_a_server_app = None

        jfc_server_app = self.user_params.get("jfc_server_app", None)
        if not jfc_server_app or not os.path.exists(jfc_server_app):
            asserts.fail("This test requires a Joint Fabric Controller app via --string-arg jfc_server_app:<path_to_app>")

        jfa_server_app = self.user_params.get("jfa_server_app", None)
        if not jfa_server_app or not os.path.exists(jfa_server_app):
            asserts.fail("This test requires a Joint Fabric Admin app via --string-arg jfa_server_app:<path_to_app>")

        if self.storage_fabric_a is None:
            self.storage_directory_ecosystem_a = tempfile.TemporaryDirectory(prefix=self.__class__.__name__+"_A_")
            self.storage_fabric_a = self.storage_directory_ecosystem_a.name
            logging.info("Temporary storage directory: %s", self.storage_fabric_a)

        self.jfadmin_fabric_a_passcode = random.randint(110220011, 110220999)
        self.jfctrl_fabric_a_vid = random.randint(0x0001, 0xFFF0)

        # Start JF-Administrator App on Fabric A
        self.fabric_a_admin = AppServerSubprocess(
            jfa_server_app,
            storage_dir=self.storage_fabric_a,
            port=random.randint(5001, 5999),
            discriminator=random.randint(0, 4095),
            passcode=self.jfadmin_fabric_a_passcode,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", "33033"])
        self.fabric_a_admin.start("Server initialization complete", timeout=10)

        # Start JF-Controller App on Fabric A
        self.fabric_a_ctrl = JFControllerSubprocess(
            jfc_server_app,
            rpc_server_port=33033,
            storage_dir=self.storage_fabric_a,
            vendor_id=self.jfctrl_fabric_a_vid)
        self.fabric_a_ctrl.start("CHIP task running", timeout=10)

        # Commission Admin with Controller on Fabric A
        self.fabric_a_ctrl.send(
            f"pairing onnetwork 1 {self.jfadmin_fabric_a_passcode} --anchor true",
            expected_output="[JF] Anchor Administrator (nodeId=1) commissioned with success",
            timeout=10)

        # Extract certificates and CATs
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
                        {"fabricId": 1, "vendorId": self.jfctrl_fabric_a_vid}
                    ]
                }
            }
        }
        self.ecoACATs = base64.b64decode(jfcStorage.get("Default", "CommissionerCATs"))[::-1].hex().strip('0')

    def teardown_class(self):
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
                     "Verify Admin CAT and Anchor CAT entries exist, note their GroupIDs"),
            TestStep("2", "TH sends AddGroup with GroupID=0x000a", "DUT responds SUCCESS"),
            TestStep("3", "TH reads GroupList, verifies new entry with expected values"),
            TestStep("4", "TH sends UpdateGroup for GroupID=0x000a with modified values", "DUT responds SUCCESS"),
            TestStep("5", "TH reads GroupList, verifies updated values"),
            TestStep("6", "TH sends RemoveGroup for GroupID=0x000a", "DUT responds SUCCESS"),
            TestStep("7", "TH reads GroupList, verifies entry 0x000a is absent"),
            TestStep("8", "TH sends AddGroup with GroupCAT=0xFFFF_0001 (Admin)", "Expect CONSTRAINT_ERROR"),
            TestStep("9", "TH sends UpdateGroup with GroupID=admin_cat_group_id", "Expect CONSTRAINT_ERROR"),
            TestStep("10", "TH sends RemoveGroup with GroupID=admin_cat_group_id", "Expect CONSTRAINT_ERROR"),
            TestStep("11", "TH sends AddGroup with GroupCAT=0xFFFE_0001 (Anchor)", "Expect CONSTRAINT_ERROR"),
            TestStep("12", "TH sends UpdateGroup with GroupID=anchor_cat_group_id", "Expect CONSTRAINT_ERROR"),
            TestStep("13", "TH sends RemoveGroup with GroupID=anchor_cat_group_id", "Expect CONSTRAINT_ERROR"),
        ]

    @async_test_body
    async def test_TC_JFDS_2_3(self):
        # Setup Controller
        storage = VolatileTemporaryPersistentStorage(
            self.ecoACtrlStorage['repl-config'], self.ecoACtrlStorage['sdk-config'])
        caMgr = CertificateAuthority.CertificateAuthorityManager(
            chipStack=self.matter_stack._chip_stack, persistentStorage=storage)
        caMgr.LoadAuthoritiesFromStorage()
        devCtrl = caMgr.activeCaList[0].adminList[0].NewController(
            nodeId=102,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
            catTags=[int(self.ecoACATs, 16)])

        self.step("1")
        resp = await devCtrl.ReadAttribute(
            nodeid=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.GroupList)],
            returnClusterObject=True)
        groupList = resp[1][Clusters.JointFabricDatastore].groupList
        asserts.assert_greater_equal(len(groupList), 2, "Expected at least Admin and Anchor CAT groups")
        # Must keep around for steps 12, 13
        admin_cat_group_id = None
        anchor_cat_group_id = None
        for g in groupList:
            if g.groupCAT == 0xFFFF0001:
                admin_cat_group_id = g.groupID
            if g.groupCAT == 0xFFFE0001:
                anchor_cat_group_id = g.groupID
        asserts.assert_is_not_none(admin_cat_group_id, "Admin CAT entry missing")
        asserts.assert_is_not_none(anchor_cat_group_id, "Anchor CAT entry missing")

        self.step("2")
        addGroup = Clusters.JointFabricDatastore.Structs.DatastoreGroupStruct(
            groupID=0x000a,
            friendlyName="tc-jf-2.3",
            groupKeySetID=0x000b,
            groupCAT=0x00010001,
            groupCATVersion=0x0001,
            groupPermission=Clusters.JointFabricDatastore.Enums.DatastoreGroupPermissionEnum.kView)
        cmd = Clusters.JointFabricDatastore.Commands.AddGroup(addGroup)
        await self.send_single_cmd(cmd=cmd, dev_ctrl=devCtrl, node_id=1, endpoint=1)

        self.step("3")
        resp = await devCtrl.ReadAttribute(
            nodeid=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.GroupList)],
            returnClusterObject=True)
        groupList = resp[1][Clusters.JointFabricDatastore].groupList
        found = any(g == addGroup for g in groupList)
        asserts.assert_true(found, "Group from step2 not found in DUT")

        self.step("4")
        updateGroup = Clusters.JointFabricDatastore.Structs.DatastoreGroupStruct(
            groupID=0x000a,
            friendlyName="tc-jf-2.3-update",
            groupKeySetID=0x000c,
            groupCAT=0x00010002,
            groupCATVersion=0x0002,
            groupPermission=Clusters.JointFabricDatastore.Enums.DatastoreGroupPermissionEnum.kOperate)
        cmd = Clusters.JointFabricDatastore.Commands.UpdateGroup(updateGroup)
        await self.send_single_cmd(cmd=cmd, dev_ctrl=devCtrl, node_id=1, endpoint=1)

        self.step("5")
        resp = await devCtrl.ReadAttribute(
            nodeid=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.GroupList)],
            returnClusterObject=True)
        groupList = resp[1][Clusters.JointFabricDatastore].groupList
        found = any(g == updateGroup for g in groupList)
        asserts.assert_true(found, "Updated group not found in DUT")

        self.step("6")
        cmd = Clusters.JointFabricDatastore.Commands.RemoveGroup(0x000a)
        await self.send_single_cmd(cmd=cmd, dev_ctrl=devCtrl, node_id=1, endpoint=1)

        self.step("7")
        resp = await devCtrl.ReadAttribute(
            nodeid=1, attributes=[(1, Clusters.JointFabricDatastore.Attributes.GroupList)],
            returnClusterObject=True)
        groupList = resp[1][Clusters.JointFabricDatastore].groupList
        asserts.assert_false(any(g.groupID == 0x000a for g in groupList),
                             "GroupID 0x000a still present after removal")

        # for CONSTRAINT_ERROR checks (steps 8 - 13)
        async def expect_constraint(cmd):
            try:
                await self.send_single_cmd(cmd=cmd, dev_ctrl=devCtrl, node_id=1, endpoint=1)
            except InteractionModelError as e:
                asserts.assert_in("CONSTRAINT_ERROR", str(e), f'Expected CONSTRAINT_ERROR, got {e}')
            else:
                asserts.fail("Expected CONSTRAINT_ERROR, but got success")

        self.step("8")
        invalidGroupAdminAdd = Clusters.JointFabricDatastore.Structs.DatastoreGroupStruct(
            groupID=0x000a,
            friendlyName="tc-jf-2.3",
            groupKeySetID=0x000b,
            groupCAT=0xFFFF0001,  # Admin CAT
            groupCATVersion=0x0001,
            groupPermission=Clusters.JointFabricDatastore.Enums.DatastoreGroupPermissionEnum.kView)
        await expect_constraint(Clusters.JointFabricDatastore.Commands.AddGroup(invalidGroupAdminAdd))

        self.step("9")
        invalidGroupAdminUpdate = Clusters.JointFabricDatastore.Structs.DatastoreGroupStruct(
            groupID=admin_cat_group_id,
            friendlyName="tc-jf-2.3-update",
            groupKeySetID=0x000c,
            groupCAT=0x00030002,
            groupCATVersion=0x0002,
            groupPermission=Clusters.JointFabricDatastore.Enums.DatastoreGroupPermissionEnum.kAdministrate)
        await expect_constraint(Clusters.JointFabricDatastore.Commands.UpdateGroup(invalidGroupAdminUpdate))

        self.step("10")
        await expect_constraint(Clusters.JointFabricDatastore.Commands.RemoveGroup(admin_cat_group_id))

        self.step("11")
        invalidGroupAnchorAdd = Clusters.JointFabricDatastore.Structs.DatastoreGroupStruct(
            groupID=0x000a,
            friendlyName="tc-jf-2.3",
            groupKeySetID=0x000b,
            groupCAT=0xFFFE0001,  # Anchor CAT
            groupCATVersion=0x0001,
            groupPermission=Clusters.JointFabricDatastore.Enums.DatastoreGroupPermissionEnum.kView)
        await expect_constraint(Clusters.JointFabricDatastore.Commands.AddGroup(invalidGroupAnchorAdd))

        self.step("12")
        invalidGroupAnchorUpdate = Clusters.JointFabricDatastore.Structs.DatastoreGroupStruct(
            groupID=anchor_cat_group_id,
            friendlyName="tc-jf-2.3-update",
            groupKeySetID=0x000c,
            groupCAT=0x00030002,
            groupCATVersion=0x0002,
            groupPermission=Clusters.JointFabricDatastore.Enums.DatastoreGroupPermissionEnum.kAdministrate)
        await expect_constraint(Clusters.JointFabricDatastore.Commands.UpdateGroup(invalidGroupAnchorUpdate))

        self.step("13")
        await expect_constraint(Clusters.JointFabricDatastore.Commands.RemoveGroup(anchor_cat_group_id))

        devCtrl.Shutdown()


if __name__ == "__main__":
    default_matter_test_main()
