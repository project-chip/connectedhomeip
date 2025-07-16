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
#       --string-arg th_server_app:${ALL_CLUSTERS_APP}
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

import chip.clusters as Clusters
import chip.tlv
from chip import CertificateAuthority
from chip.storage import PersistentStorage
from chip.testing.apps import AppServerSubprocess, JFControllerSubprocess
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_JCM_1_2(MatterBaseTest):

    ANCHOR_CAT = 0xFFFE0001

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.fabric_a_ctrl = None
        self.storage_fabric_a = self.user_params.get("fabric_a_storage", None)
        self.fabric_b_ctrl = None
        self.storage_fabric_b = self.user_params.get("fabric_b_storage", None)
        self.fabric_a_server_app = None
        self.fabric_b_server_app = None

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

        self.th_server_app = self.user_params.get("th_server_app", None)
        if not self.th_server_app:
            asserts.fail("This test requires a TH Server app. Specify app path with --string-arg th_server_app:<path_to_app>")
        if not os.path.exists(self.th_server_app):
            asserts.fail(f"The path {self.th_server_app} does not exist")

        # Create a temporary storage directory for both ecosystems to keep KVS files if not already provided by user.
        if self.storage_fabric_a is None:
            self.storage_directory_ecosystem_a = tempfile.TemporaryDirectory(prefix=self.__class__.__name__+"_A_")
            self.storage_fabric_a = self.storage_directory_ecosystem_a.name
            logging.info("Temporary storage directory: %s", self.storage_fabric_a)
        if self.storage_fabric_b is None:
            self.storage_directory_ecosystem_b = tempfile.TemporaryDirectory(prefix=self.__class__.__name__+"_B_")
            self.storage_fabric_b = self.storage_directory_ecosystem_b.name
            logging.info("Temporary storage directory: %s", self.storage_fabric_b)

        #####################################################################################################################################
        #
        # Initialize Ecosystem A
        #
        #####################################################################################################################################
        self.jfadmin_fabric_a_passcode = random.randint(110220011, 110220999)
        self.jfadmin_fabric_a_discriminator = random.randint(0, 4095)
        self.jfctrl_fabric_a_vid = random.randint(0x0001, 0xFFF0)

        # Start Fabric A JF-Administrator App
        self.fabric_a_admin = AppServerSubprocess(
            jfa_server_app,
            storage_dir=self.storage_fabric_a,
            port=random.randint(5001, 5999),
            discriminator=self.jfadmin_fabric_a_discriminator,
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
            expected_output="[JF] Anchor Administrator commissioned with sucess")

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

        #####################################################################################################################################
        #
        # Initialize Ecosystem B
        #
        #####################################################################################################################################
        self.jfadmin_fabric_b_passcode = random.randint(110220011, 110220999)
        self.jfadmin_fabric_b_discriminator = random.randint(0, 4095)
        self.jfctrl_fabric_b_vid = random.randint(0x0001, 0xFFF0)

        # Start Fabric B JF-Administrator App
        self.fabric_b_admin = AppServerSubprocess(
            jfa_server_app,
            storage_dir=self.storage_fabric_b,
            port=random.randint(5001, 5999),
            discriminator=self.jfadmin_fabric_b_discriminator,
            passcode=self.jfadmin_fabric_b_passcode,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", "33055"])
        self.fabric_b_admin.start(
            expected_output="Server initialization complete",
            timeout=10)

        # Start Fabric B JF-Administrator App
        self.fabric_b_ctrl = JFControllerSubprocess(
            jfc_server_app,
            rpc_server_port=33055,
            storage_dir=self.storage_fabric_b,
            vendor_id=self.jfctrl_fabric_b_vid)
        self.fabric_b_ctrl.start(
            expected_output="CHIP task running",
            timeout=10)

        # Commission JF-ADMIN app with JF-Controller on Fabric B
        self.fabric_b_ctrl.send(
            message=f"pairing onnetwork 11 {self.jfadmin_fabric_b_passcode} --anchor true",
            expected_output="[JF] Anchor Administrator commissioned with sucess")

        # Extract the Ecosystem B certificates and inject them in the storage that will be provided to a new Python Controller later
        jfcStorage = ConfigParser()
        jfcStorage.read(self.storage_fabric_b+'/chip_tool_config.alpha.ini')
        self.ecoBCtrlStorage = {
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
                            "vendorId": self.jfctrl_fabric_b_vid
                        }
                    ]
                }
            }
        }
        # Extract CATs to be provided to the Python Controller later
        self.ecoBCATs = base64.b64decode(jfcStorage.get("Default", "CommissionerCATs"))[::-1].hex().strip('0')

    def teardown_class(self):
        # Stop all Subprocesses that were started in this test case
        if self.fabric_a_admin is not None:
            self.fabric_a_admin.terminate()
        if self.fabric_b_admin is not None:
            self.fabric_b_admin.terminate()
        if self.fabric_a_ctrl is not None:
            self.fabric_a_ctrl.terminate()
        if self.fabric_b_ctrl is not None:
            self.fabric_b_ctrl.terminate()
        if self.fabric_a_server_app is not None:
            self.fabric_a_server_app.terminate()
        if self.fabric_b_server_app is not None:
            self.fabric_b_server_app.terminate()

        super().teardown_class()

    def steps_TC_JCM_1_2(self) -> list[TestStep]:
        return [
            TestStep("1", "On Ecosystem B, use jfc-app for opening a commissioning window in jfa-app using Python Controller"
                     "Check this Commissioning Window opens successfully with correct parameters"),
            TestStep("2", "On Ecosystem A, use jfc-app for commissioning jfa-app at EcosystemB using Python Controller"
                     "Verify Joint Commissioning completes successfully with --execute-jcm functionality"),
            TestStep("3", "On jfc-app@EcoB used a non-filtered fabric read for reading the NOC from Fabric Index=2"
                     "Parse the NOC bytes and Checked that it contains the Administrator CAT")
        ]

    async def _open_commissioning_window_in_EcoB(self, devCtrl, nodeid, discriminator, endpoint):
        """Open commissioning window on jfa-app in Ecosystem B using Python Controller"""

        # Open enhanced commissioning window using Administrator Commissioning cluster of jfa-app@EcoB
        # Using devCtrlEcoB Python Controller
        # Parameters matching: pairing open-commissioning-window 11 0 400 1000 {jfa-app--discriminator@EcoB}
        _ocw_timeout = 400  # 400 seconds ~ 6.5 minutes
        _ocw_iteration = 1000

        try:
            # Open Enhanced Commissioning Window using Administrator Commissioning Cluster
            params = await devCtrl.OpenCommissioningWindow(
                nodeid=nodeid,
                timeout=_ocw_timeout,
                iteration=_ocw_iteration,
                discriminator=discriminator,
                option=1
            )

            logging.info(f"Successfully opened Enhanced Commissioning Window on node {nodeid}")
            logging.info(f"Discriminator: {discriminator}, Timeout: {_ocw_timeout}s, setupPinCode: {params.setupPinCode}")

            # Verify commissioning window is open by reading WindowStatus attribute
            window_status = await devCtrl.ReadAttribute(
                nodeid=nodeid,
                attributes=[(endpoint, Clusters.AdministratorCommissioning.Attributes.WindowStatus)]
            )

            expected_WStatus = Clusters.AdministratorCommissioning.Enums.CommissioningWindowStatusEnum.kEnhancedWindowOpen
            actual_WStatus_value = window_status[endpoint][Clusters.AdministratorCommissioning][Clusters.AdministratorCommissioning.Attributes.WindowStatus]

            asserts.assert_equal(
                expected_WStatus,
                actual_WStatus_value,
                f"Commissioning window not properly opened. Expected: {expected_WStatus}, Got: {actual_WStatus_value}"
            )

            return params

        except Exception as e:
            logging.error(f"Failed to open commissioning window: {e}")
            asserts.fail(f"Could not open commissioning window on node {nodeid}: {e}")

    async def _joint_commission_EcoB_admin(self, devCtrl, nodeid, passcode, endpoint):
        """Commission jfa-app from Ecosystem B into Ecosystem A using Joint Commissioning"""

        # Parameters matching: pairing onnetwork {nodeid} {passcode} --execute-jcm true

        try:
            # Perform Joint Commissioning using Ecosystem A jfc-app
            logging.info(f"Starting Joint Commissioning of Ecosystem B jfa-app with node ID {nodeid}")

            devCtrl.send(
                message=f"pairing onnetwork {nodeid} {passcode} --execute-jcm true",
                expected_output="")

            logging.info(f"Joint Commissioning completed successfully for node {nodeid}")

            return True

        except Exception as e:
            logging.error(f"Joint Commissioning failed: {e}")
            asserts.fail(f"Could not perform Joint Commissioning on node {nodeid}: {e}")

    @async_test_body
    async def test_TC_JCM_1_2(self):

        # Creating a Controller for Ecosystem A
        _fabric_a_persistent_storage = PersistentStorage(jsonData=self.ecoACtrlStorage)
        _certAuthorityManagerA = CertificateAuthority.CertificateAuthorityManager(
            chipStack=self.matter_stack._chip_stack,
            persistentStorage=_fabric_a_persistent_storage)
        _certAuthorityManagerA.LoadAuthoritiesFromStorage()
        _devCtrlEcoA = _certAuthorityManagerA.activeCaList[0].adminList[0].NewController(
            nodeId=101,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
            catTags=[int(self.ecoACATs, 16)])

        # Creating a Controller for Ecosystem B
        _fabric_b_persistent_storage = PersistentStorage(jsonData=self.ecoBCtrlStorage)
        _certAuthorityManagerB = CertificateAuthority.CertificateAuthorityManager(
            chipStack=self.matter_stack._chip_stack,
            persistentStorage=_fabric_b_persistent_storage)
        _certAuthorityManagerB.LoadAuthoritiesFromStorage()
        devCtrlEcoB = _certAuthorityManagerB.activeCaList[0].adminList[0].NewController(
            nodeId=201,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
            catTags=[int(self.ecoBCATs, 16)])

        self.step("1")
        logging.info("On Ecosystem B, use jfc-app for opening a commissioning window in jfa-app using Python Controller")

        # Parameters matching: pairing open-commissioning-window 11 0 400 1000 {jfa-app--discriminator@EcoB}
        _ocw_nodeid = 11     # jfa-app node ID in Ecosystem B
        _ocw_endpoint = 0
        _ocw_discriminator = self.jfadmin_fabric_b_discriminator  # jfa-app discriminator in Ecosystem B

        # Open enhanced commissioning window using Administrator Commissioning cluster of jfa-app@EcoB
        _ocw_params = await self._open_commissioning_window_in_EcoB(devCtrlEcoB, _ocw_nodeid, _ocw_discriminator, _ocw_endpoint)

        self.step("2")
        logging.info("On Ecosystem A, use jfc-app for commissioning jfa-app at EcosystemB using Python Controller")

        # Perform Joint Commissioning: Ecosystem A commissions Ecosystem B jfa-app
        # Using devCtrlEcoA Python Controller

        # Parameters matching: pairing onnetwork 15 passcode --execute-jcm true
        _jcm_nodeid = 15     # Node ID for jfa-app from Ecosystem B in Ecosystem A fabric
        _jcm_endpoint = 0

        # Perform Joint Commissioning using Ecosystem A jfc-app
        await self._joint_commission_EcoB_admin(self.fabric_a_ctrl, _jcm_nodeid, _ocw_params.setupPinCode, _jcm_endpoint)

        self.step("3")
        logging.info("On jfc-app@EcoB used a non-filtered fabric read for reading the NOC from Fabric Index=2")

        # Read JF-Admin NOC on Ecoystem B using jfc-app@EcoB
        response = await devCtrlEcoB.ReadAttribute(
            nodeid=11, attributes=[(0, Clusters.OperationalCredentials.Attributes.NOCs)], fabricFiltered=False,
            returnClusterObject=True)

        # Search Administrator CAT (FFFF0001) and Anchor CAT (FFFE0001) in JF-Admin NOC on Ecoystem B
        noc_tlv_data = chip.tlv.TLVReader(response[0][Clusters.OperationalCredentials].NOCs[0].noc).get()
        _admin_cat_found = False
        _anchor_cat_found = False
        for _tag, _value in noc_tlv_data['Any'][6]:
            if _tag == 22 and _value == int(self.ecoBCATs, 16):
                # Administrator CAT present inside the Subject Field of the NOC
                _admin_cat_found = True
            elif _tag == 22 and _value == self.ANCHOR_CAT:
                # Anchor CAT present inside the Subject Field of the NOC
                _anchor_cat_found = True
            if _admin_cat_found and _anchor_cat_found:
                break
        asserts.assert_true(_admin_cat_found, "Administrator CAT not found in Admin App NOC on Ecosystem B")
        asserts.assert_true(_anchor_cat_found, "Anchor CAT not found in Admin App NOC on Ecosystem B")


if __name__ == "__main__":
    default_matter_test_main()
