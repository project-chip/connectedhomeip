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

from mobly import asserts

import matter.clusters as Clusters
import matter.tlv
from matter import CertificateAuthority
from matter.storage import VolatileTemporaryPersistentStorage
from matter.testing.apps import AppServerSubprocess, JFControllerSubprocess
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_JCM_1_1(MatterBaseTest):

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

        #####################################################################################################################################
        #
        # Initialize Ecosystem B
        #
        #####################################################################################################################################
        self.jfadmin_fabric_b_passcode = random.randint(110220011, 110220999)
        self.jfctrl_fabric_b_vid = random.randint(0x0001, 0xFFF0)

        # Start Fabric B JF-Administrator App
        self.fabric_b_admin = AppServerSubprocess(
            jfa_server_app,
            storage_dir=self.storage_fabric_b,
            port=random.randint(5001, 5999),
            discriminator=random.randint(0, 4095),
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
            expected_output="[JF] Anchor Administrator (nodeId=11) commissioned with success",
            timeout=10)

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

    def steps_TC_JCM_1_1(self) -> list[TestStep]:
        return [
            TestStep("1", "Verify VID on the JFAdmin app on both Ecosystems",
                     "Expect the VID to be the one configured by the jfc-app on each fabric"),
            TestStep("2", "Verify NOCs of both JF-Admin apps from Ecosystem A and Ecosystem B have the Anchor CAT and Administrator CAT",
                     "Expect each Admin app to generate the correct CATs"),
            TestStep("3", "Commission a server app in Ecosystem A",
                     "Check the correct Vendor ID and Administrator CATs have been installed on the server app"),
            TestStep("4", "Commission a server app in Ecosystem B",
                     "Check the correct Vendor ID and Administrator CATs have been installed on the server app")
        ]

    @async_test_body
    async def test_TC_JCM_1_1(self):

        # Creating a Controller for Ecosystem A
        _fabric_a_persistent_storage = VolatileTemporaryPersistentStorage(
            self.ecoACtrlStorage['repl-config'], self.ecoACtrlStorage['sdk-config'])
        _certAuthorityManagerA = CertificateAuthority.CertificateAuthorityManager(
            chipStack=self.matter_stack._chip_stack,
            persistentStorage=_fabric_a_persistent_storage)
        _certAuthorityManagerA.LoadAuthoritiesFromStorage()
        devCtrlEcoA = _certAuthorityManagerA.activeCaList[0].adminList[0].NewController(
            nodeId=101,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
            catTags=[int(self.ecoACATs, 16)])

        # Creating a Controller for Ecosystem B
        _fabric_b_persistent_storage = VolatileTemporaryPersistentStorage(
            self.ecoBCtrlStorage['repl-config'], self.ecoBCtrlStorage['sdk-config'])
        _certAuthorityManagerB = CertificateAuthority.CertificateAuthorityManager(
            chipStack=self.matter_stack._chip_stack,
            persistentStorage=_fabric_b_persistent_storage)
        _certAuthorityManagerB.LoadAuthoritiesFromStorage()
        devCtrlEcoB = _certAuthorityManagerB.activeCaList[0].adminList[0].NewController(
            nodeId=201,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
            catTags=[int(self.ecoBCATs, 16)])

        self.step("1")
        response = await devCtrlEcoA.ReadAttribute(
            nodeid=1, attributes=[(0, Clusters.OperationalCredentials.Attributes.Fabrics)],
            returnClusterObject=True)
        asserts.assert_equal(
            self.jfctrl_fabric_a_vid,
            response[0][Clusters.OperationalCredentials].fabrics[0].vendorID,
            "JF-Admin App on Ecosystem A doesn't have the correct VID")

        response = await devCtrlEcoB.ReadAttribute(
            nodeid=11, attributes=[(0, Clusters.OperationalCredentials.Attributes.Fabrics)],
            returnClusterObject=True)
        asserts.assert_equal(
            self.jfctrl_fabric_b_vid,
            response[0][Clusters.OperationalCredentials].fabrics[0].vendorID,
            "JF-Admin App on Ecosystem A doesn't have the correct VID")

        self.step("2")
        response = await devCtrlEcoA.ReadAttribute(
            nodeid=1, attributes=[(0, Clusters.OperationalCredentials.Attributes.NOCs)],
            returnClusterObject=True)
        # Search Administrator CAT (FFFF0001) and Anchor CAT (FFFD0001) in JF-Admin NOC on Ecoystem A
        noc_tlv_data = matter.tlv.TLVReader(response[0][Clusters.OperationalCredentials].NOCs[0].noc).get()
        _admin_cat_found = False
        _anchor_cat_found = False
        for _tag, _value in noc_tlv_data['Any'][6]:
            if _tag == 22 and _value == int(self.ecoACATs, 16):
                _admin_cat_found = True
            elif _tag == 22 and _value == int("FFFE0001", 16):
                _anchor_cat_found = True
            if _admin_cat_found and _anchor_cat_found:
                break
        asserts.assert_true(_admin_cat_found, "Administrator CAT not found in Admin App NOC on Ecosystem A")
        asserts.assert_true(_anchor_cat_found, "Anchor CAT not found in Admin App NOC on Ecosystem A")
        # Search jf-anchor-cat in Subject field of JF-Admin ICAC on Ecoystem A
        icac_tlv_data = matter.tlv.TLVReader(response[0][Clusters.OperationalCredentials].NOCs[0].icac).get()
        _found = False
        for _tag, _value in icac_tlv_data['Any'][6]:
            if _tag == 8 and _value == 'jf-anchor-icac':
                _found = True
                break
        asserts.assert_true(_found, "Anchor ICAC (jf-anchor-icac) not found in Admin App ICAC Subject field on Ecosystem A")
        response = await devCtrlEcoB.ReadAttribute(
            nodeid=11, attributes=[(0, Clusters.OperationalCredentials.Attributes.NOCs)],
            returnClusterObject=True)
        # Search Administrator CAT (FFFF0001) and Anchor CAT (FFFD0001) in JF-Admin NOC on Ecoystem A
        noc_tlv_data = matter.tlv.TLVReader(response[0][Clusters.OperationalCredentials].NOCs[0].noc).get()
        _admin_cat_found = False
        _anchor_cat_found = False
        for _tag, _value in noc_tlv_data['Any'][6]:
            if _tag == 22 and _value == int(self.ecoBCATs, 16):
                _admin_cat_found = True
            elif _tag == 22 and _value == int("FFFE0001", 16):
                _anchor_cat_found = True
            if _admin_cat_found and _anchor_cat_found:
                break
        asserts.assert_true(_admin_cat_found, "Administrator CAT not found in Admin App NOC on Ecosystem A")
        asserts.assert_true(_anchor_cat_found, "Anchor CAT not found in Admin App NOC on Ecosystem A")
        # Search jf-anchor-cat in Subject field of JF-Admin ICAC on Ecoystem A
        icac_tlv_data = matter.tlv.TLVReader(response[0][Clusters.OperationalCredentials].NOCs[0].icac).get()
        _found = False
        for _tag, _value in icac_tlv_data['Any'][6]:
            if _tag == 8 and _value == 'jf-anchor-icac':
                _found = True
                break
        asserts.assert_true(_found, "Anchor ICAC (jf-anchor-icac) not found in Admin App ICAC Subject field on Ecosystem A")

        self.step("3")
        self.thserver_fabric_a_passcode = random.randint(110220011, 110220999)
        self.fabric_a_server_app = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage_fabric_a,
            port=random.randint(5001, 5999),
            discriminator=random.randint(0, 4095),
            passcode=self.thserver_fabric_a_passcode,
            extra_args=["--capabilities", "0x04"])
        self.fabric_a_server_app.start(
            expected_output="Server initialization complete",
            timeout=10)

        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork 2 {self.thserver_fabric_a_passcode}",
            expected_output="[CTL] Commissioning complete for node ID 0x0000000000000002: success",
            timeout=10)

        response = await devCtrlEcoA.ReadAttribute(
            nodeid=2, attributes=[(0, Clusters.OperationalCredentials.Attributes.Fabrics)],
            returnClusterObject=True)
        asserts.assert_equal(
            self.jfctrl_fabric_a_vid,
            response[0][Clusters.OperationalCredentials].fabrics[0].vendorID,
            "JF-Admin App on Ecosystem A doesn't have the correct VID")

        response = await devCtrlEcoA.ReadAttribute(
            nodeid=2, attributes=[(0, Clusters.AccessControl.Attributes.Acl)],
            returnClusterObject=True)
        asserts.assert_equal(
            int('0xFFFFFFFD'+self.ecoACATs, 16),
            response[0][Clusters.AccessControl].acl[0].subjects[0],
            "EcoA Server App Subjects field has wrong value")

        self.step("4")
        self.thserver_fabric_b_passcode = random.randint(110220011, 110220999)
        self.fabric_b_server_app = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage_fabric_b,
            port=random.randint(5001, 5999),
            discriminator=random.randint(0, 4095),
            passcode=self.thserver_fabric_b_passcode,
            extra_args=["--capabilities", "0x04"])
        self.fabric_b_server_app.start(
            expected_output="Server initialization complete",
            timeout=10)

        self.fabric_b_ctrl.send(
            message=f"pairing onnetwork 22 {self.thserver_fabric_b_passcode}",
            expected_output="[CTL] Commissioning complete for node ID 0x0000000000000016: success",
            timeout=10)

        response = await devCtrlEcoB.ReadAttribute(
            nodeid=22, attributes=[(0, Clusters.OperationalCredentials.Attributes.Fabrics)],
            returnClusterObject=True)
        asserts.assert_equal(
            self.jfctrl_fabric_b_vid,
            response[0][Clusters.OperationalCredentials].fabrics[0].vendorID,
            "JF-Admin App on Ecosystem A doesn't have the correct VID")

        response = await devCtrlEcoB.ReadAttribute(
            nodeid=22, attributes=[(0, Clusters.AccessControl.Attributes.Acl)],
            returnClusterObject=True)
        asserts.assert_equal(
            int('0xFFFFFFFD'+self.ecoBCATs, 16),
            response[0][Clusters.AccessControl].acl[0].subjects[0],
            "EcoA Server App Subjects field has wrong value")

        # Shutdown the Python Controllers started at the beginning of this script
        devCtrlEcoA.Shutdown()
        devCtrlEcoB.Shutdown()


if __name__ == "__main__":
    default_matter_test_main()
