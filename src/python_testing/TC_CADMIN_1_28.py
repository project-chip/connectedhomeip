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
# === END CI TEST ARGUMENTS ===

import base64
import logging
import os
import random
import tempfile
from configparser import ConfigParser

from mobly import asserts
from mdns_discovery import mdns_discovery

import matter.clusters as Clusters
from support_modules.cadmin_support import CADMINBaseTest
import matter.tlv
from matter import CertificateAuthority
from matter.storage import VolatileTemporaryPersistentStorage
from matter.testing.apps import AppServerSubprocess, JFControllerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_CADMIN_1_28(CADMINBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.fabric_a_ctrl = None
        self.storage_fabric_a = self.user_params.get("fabric_a_storage", None)
        self.fabric_b_ctrl = None
        self.storage_fabric_b = self.user_params.get("fabric_b_storage", None)
        self.fabric_a_server_app = None
        self.fabric_b_server_app = None

        self.jfc_server_app = self.user_params.get("jfc_server_app", None)
        if not self.jfc_server_app:
            asserts.fail("This test requires a Joint Fabric Controller app. Specify app path with --string-arg jfc_server_app:<path_to_app>")
        if not os.path.exists(self.jfc_server_app):
            asserts.fail(f"The path {self.jfc_server_app} does not exist")

        self.jfa_server_app = self.user_params.get("jfa_server_app", None)
        if not self.jfa_server_app:
            asserts.fail("This test requires a Joint Fabrics Admin app. Specify app path with --string-arg jfa_server_app:<path_to_app>")
        if not os.path.exists(self.jfa_server_app):
            asserts.fail(f"The path {self.jfa_server_app} does not exist")

        self.th_server_app = self.user_params.get("th_server_app", None)
        if not self.th_server_app:
            asserts.fail("This test requires a TH Server app. Specify app path with --string-arg th_server_app:<path_to_app>")
        if not os.path.exists(self.th_server_app):
            asserts.fail(f"The path {self.th_server_app} does not exist")

        # Create a temporary storage directory for both ecosystems to keep KVS files if not already provided by user.
        if self.storage_fabric_a is None:
            self.storage_directory_ecosystem_a = tempfile.TemporaryDirectory(prefix=self.__class__.__name__+"_A_")
            self.storage_fabric_a = self.storage_directory_ecosystem_a.name
            log.info("Temporary storage directory: %s", self.storage_fabric_a)
        if self.storage_fabric_b is None:
            self.storage_directory_ecosystem_b = tempfile.TemporaryDirectory(prefix=self.__class__.__name__+"_B_")
            self.storage_fabric_b = self.storage_directory_ecosystem_b.name
            log.info("Temporary storage directory: %s", self.storage_fabric_b)

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

    def steps_TC_CADMIN_1_28(self) -> list[TestStep]:
        # Steps 1 and 2 from Test Plan are done in setup class
        return [
            TestStep("1", "DUT_AJF starts a commissioning process to commission TH_DEV1 on Fabric 1",
                     "TH_DEV1 is commissioned by DUT_AJF on Fabric 1"),
            TestStep("2", "DUT_AJF starts a commissioning process to commission TH_DEV2 on Fabric 2",
                     "TH_DEV2 is commissioned by DUT_AJF on Fabric 2"),
            TestStep("3", "Open a Commissioning Window on DUT_AJF using OpenJointCommissioningWindow command",
                    "Verify DUT_AJF opens its Commissioning window to allow another commissioning"),
            TestStep("4", "DNS-SD records shows DUT_AJF advertising",
                     "Verify that the DNS-SD advertisement has JF key with a value between 1 and 15"),
            TestStep("5", "TH_AAF2 starts a commissioning process to commission DUT_AJF on Fabric 2 proceeding to step 19 (successful established CASE session)",
                     "TH_AAF2 complete with success step 19 (successful established CASE session) from standard commissioning flow for DUT_AJF"),
            TestStep("6", "DUT_AJF performs Fabric Table Vendor ID Verification procedure against the Fabric indicated by the AdministratorFabricIndex of the Joint Fabric Administrator Cluster on JointEndPointA",
                     "Fabric Table Vendor ID Verification procedure is successful"),
            TestStep("7", "DUT_AJF responds to ICACCSRRequest command issued by TH_AAF2",
                     "DUT_AJF sends ICACCSRResponse to TH_AAF2 with ICAC CSR certificate as a DER-encoded string"),
            TestStep("8", "DUT_AJF responds to the AddICAC command from TH_AAF2 with an ICACResponse",
                     "DUT_AJF sends ICACResponse to TH_AAF2"),
            TestStep("9", "Read DUT_AJF NOC at FabricIndex equal to the Joint Fabric",
                     "DUT_AJF NOC from JointFabric should contain Administrator CAT")
        ]

    @async_test_body
    async def test_TC_CADMIN_1_28(self):

        self.step("1")
        self.jfadmin_fabric_a_passcode = random.randint(110220011, 110220999)
        self.jfadmin_fabric_a_discriminator = random.randint(0, 4095)
        self.jfctrl_fabric_a_vid = random.randint(0x0001, 0xFFF0)

        # Start Fabric A JF-Administrator App
        self.fabric_a_admin = AppServerSubprocess(
            self.jfa_server_app,
            storage_dir=self.storage_fabric_a,
            port=random.randint(5001, 5999),
            discriminator=self.jfadmin_fabric_a_discriminator,
            passcode=self.jfadmin_fabric_a_passcode,
            capture_output=True,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", "33033"])
        self.fabric_a_admin.start(
            expected_output="Server initialization complete",
            timeout=10)

        # Start Fabric A JF-Controller App
        self.fabric_a_ctrl = JFControllerSubprocess(
            self.jfc_server_app,
            rpc_server_port=33033,
            storage_dir=self.storage_fabric_a,
            vendor_id=self.jfctrl_fabric_a_vid,
            capture_output=True)
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

        self.thserver_fabric_a_passcode = random.randint(110220011, 110220999)
        self.fabric_a_server_app = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage_fabric_a,
            port=random.randint(5001, 5999),
            discriminator=random.randint(0, 4095),
            passcode=self.thserver_fabric_a_passcode,
            capture_output=True,
            extra_args=["--capabilities", "0x04"])
        self.fabric_a_server_app.start(
            expected_output="Server initialization complete",
            timeout=10)

        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork 2 {self.thserver_fabric_a_passcode}",
            expected_output="[CTL] Commissioning complete for node ID 0x0000000000000002: success",
            timeout=10)

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
            catTags=[int(self.ecoACATs, 16), int('fffe0001', 16)])

        self.step("2")
        self.jfadmin_fabric_b_passcode = random.randint(110220011, 110220999)
        self.jfadmin_fabric_b_discriminator = random.randint(0, 4095)
        self.jfctrl_fabric_b_vid = random.randint(0x0001, 0xFFF0)

        # Start Fabric B JF-Administrator App
        self.fabric_b_admin = AppServerSubprocess(
            self.jfa_server_app,
            storage_dir=self.storage_fabric_b,
            port=random.randint(5001, 5999),
            discriminator=self.jfadmin_fabric_b_discriminator,
            passcode=self.jfadmin_fabric_b_passcode,
            capture_output=True,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", "33055"])
        self.fabric_b_admin.start(
            expected_output="Server initialization complete",
            timeout=10)

        # Start Fabric B JF-Administrator App
        self.fabric_b_ctrl = JFControllerSubprocess(
            self.jfc_server_app,
            rpc_server_port=33055,
            storage_dir=self.storage_fabric_b,
            vendor_id=self.jfctrl_fabric_b_vid,
            capture_output=True)
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

        self.thserver_fabric_b_passcode = random.randint(110220011, 110220999)
        self.fabric_b_server_app = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage_fabric_b,
            port=random.randint(5001, 5999),
            discriminator=random.randint(0, 4095),
            passcode=self.thserver_fabric_b_passcode,
            capture_output=True,
            extra_args=["--capabilities", "0x04"])
        self.fabric_b_server_app.start(
            expected_output="Server initialization complete",
            timeout=10)

        self.fabric_b_ctrl.send(
            message=f"pairing onnetwork 22 {self.thserver_fabric_b_passcode}",
            expected_output="[CTL] Commissioning complete for node ID 0x0000000000000016: success",
            timeout=10)

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

        self.step("3")
        self.fabric_a_admin.get_stdout()
        ojcw_discriminator = random.randint(0, 4095)
        try:
            response = await devCtrlEcoA.OpenJointCommissioningWindow(
                nodeId=1,
                endpointId=1,
                timeout=400,
                iteration=random.randint(1000, 100000),
                discriminator=random.randint(0, 4095)
            )
        except Exception as e:
            asserts.assert_true(False, f'Exception {e} occured during OJCW')


        self.step("4")
        service_found = [False, None]
        discovery = mdns_discovery.MdnsDiscovery()
        raw_services = await discovery.get_commissionable_services(discovery_timeout_sec=240, log_output=True)
        services = [self.ParsedService(service) for service in raw_services]
        for parsed_service in services:
            if parsed_service.cm == 3:
                service_found = [True, parsed_service]
                break
        asserts.assert_true(service_found[0], f"Successfully found service with CM={service_found[1].cm}, D={service_found[1].d}")

        self.step("5")
        self.fabric_a_admin.get_stdout()
        self.fabric_b_ctrl.get_stdout()
        _nodeID = 15
        self.fabric_b_ctrl.send(
            message=f"pairing onnetwork {_nodeID} {response.setupPinCode} --jcm true",
            expected_output=f"[JF] Joint Commissioning Method (nodeId={_nodeID}) success",
            timeout=30)

        log_fab_a_admin = self.fabric_a_admin.get_stdout()
        log_fab_b_ctrl = self.fabric_b_ctrl.get_stdout()
        asserts.assert_in(b"[SVR] Commissioning completed session establishment step", log_fab_a_admin,
                          "CASE session was established with success")

        self.step("6")
        asserts.assert_in(b"[CTL] JCM: Trust Verification Stage Finished: STORING_ENDPOINT_ID", log_fab_a_admin,
                          "Fabric Table Vendor ID Verification procedure is successful")
        asserts.assert_in(b"[CTL] JCM: Trust Verification Stage Finished: READING_COMMISSIONER_ADMIN_FABRIC_INDEX", log_fab_a_admin,
                          "Fabric Table Vendor ID Verification procedure is successful")

        self.step("7")
        # Command 0x0E is SignVIDVerificationResponse which contains the ICAC CSR response in JCM context
        asserts.assert_in(b"[CTL] Successfully received SignVIDVerificationResponse", log_fab_b_ctrl,
                          "DUT_AJF sends ICACCSRResponse to TH_AAF2 with ICAC CSR certificate as a DER-encoded string")

        self.step("8")
        # Command 0x05 is CSRResponse
        asserts.assert_in(b"[CTL] Received certificate signing request from the device", log_fab_b_ctrl,
                          "DUT_AJF sends ICACResponse to TH_AAF2")

        self.step("9")
        response = await devCtrlEcoA.ReadAttribute(
            nodeId=1, attributes=[(0, Clusters.OperationalCredentials.Attributes.NOCs)], fabricFiltered=False,
            returnClusterObject=True)

        fabricIndex2_noc = None
        for _nocs in response[0][Clusters.OperationalCredentials].NOCs:
            if _nocs.fabricIndex == 2:
                fabricIndex2_noc = _nocs.noc
        asserts.assert_is_not_none(fabricIndex2_noc, "No NOC on fabric index 2 found!")

        # Search Administrator CAT (FFFF0001) in JF-Admin NOC on Ecosystem A
        noc_tlv_data = matter.tlv.TLVReader(fabricIndex2_noc).get()
        _admin_cat_found = False
        for _tag, _value in noc_tlv_data['Any'][6]:
            if _tag == 22 and _value == int(self.ecoBCATs, 16):
                _admin_cat_found = True
                break
        asserts.assert_true(_admin_cat_found, "Administrator CAT not found in Admin App NOC on Ecosystem B")

        devCtrlEcoA.Shutdown()
        devCtrlEcoB.Shutdown()


if __name__ == "__main__":
    default_matter_test_main()
