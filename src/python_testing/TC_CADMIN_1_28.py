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

from mdns_discovery import mdns_discovery
from mobly import asserts
from support_modules.cadmin_support import CADMINBaseTest

import matter.clusters as Clusters
import matter.tlv
from matter import CertificateAuthority
from matter.storage import VolatileTemporaryPersistentStorage
from matter.testing.apps import AppServerSubprocess, JFAdministratorSubprocess, JFControllerSubprocess
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
        self.fabric_a_admin = None
        self.fabric_b_admin = None

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
            self.storage_directory_ecosystem_a = tempfile.mkdtemp(prefix=self.__class__.__name__+"_A_")
            self.storage_fabric_a = self.storage_directory_ecosystem_a
            log.info("Temporary storage directory: %s", self.storage_fabric_a)
        if self.storage_fabric_b is None:
            self.storage_directory_ecosystem_b = tempfile.mkdtemp(prefix=self.__class__.__name__+"_B_")
            self.storage_fabric_b = self.storage_directory_ecosystem_b
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
            TestStep("5", "TH_AAF2 starts a commissioning process to commission DUT_AJF on Fabric 2 using Joint Commissioning Method",
                     "DUT_AJF is successfuly commissioned into Fabric 2 by TH_AAF2 using Joint Commissioning Method"),
            TestStep("6", "Read DUT_AJF NOC at FabricIndex equal to the Joint Fabric",
                     "DUT_AJF NOC from JointFabric should contain Administrator CAT")
        ]

    def pics_TC_CADMIN_1_28(self):
        return ['CADMIN.S']

    @async_test_body
    async def test_TC_CADMIN_1_28(self):
        _devCtrlEcoA = None
        _devCtrlEcoB = None
        _fabric_a_persistent_storage = None
        _fabric_b_persistent_storage = None

        _devCtrlEcoA = None
        _devCtrlEcoB = None
        _fabric_a_persistent_storage = None
        _fabric_b_persistent_storage = None

        self.step("1")
        self.jfadmin_fabric_a_passcode = random.randint(20202021, 20202099)
        self.jfadmin_fabric_a_discriminator = random.randint(0, 4095)
        self.jfctrl_fabric_a_vid = random.randint(0x0001, 0xFFF0)

        # Start Fabric A JF-Administrator App
        self.fabric_a_admin = JFAdministratorSubprocess(
            self.jfa_server_app,
            prefix="JFA-A",
            storage_dir=self.storage_fabric_a,
            port=random.randint(5001, 5999),
            discriminator=self.jfadmin_fabric_a_discriminator,
            passcode=self.jfadmin_fabric_a_passcode,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", "33033"])
        self.fabric_a_admin.start(
            expected_output="Updating services using commissioning mode 1",
            timeout=30)

        # Start Fabric A JF-Controller App
        self.fabric_a_ctrl = JFControllerSubprocess(
            self.jfc_server_app,
            prefix="JFC-A",
            rpc_server_port=33033,
            storage_dir=self.storage_fabric_a,
            vendor_id=self.jfctrl_fabric_a_vid)
        self.fabric_a_ctrl.start(
            expected_output="CHIP task running",
            timeout=30)

        # Commission JF-ADMIN app with JF-Controller on Fabric A
        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork-long 1 {self.jfadmin_fabric_a_passcode} {self.jfadmin_fabric_a_discriminator} --anchor true",
            expected_output="[JF] Anchor Administrator (nodeId=1) commissioned with success",
            timeout=60)

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

        self.thserver_fabric_a_passcode = random.randint(20202021, 20202099)
        self.thserver_fabric_a_discriminator = random.randint(0, 4095)
        self.fabric_a_server_app = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage_fabric_a,
            port=random.randint(5001, 5999),
            discriminator=self.thserver_fabric_a_discriminator,
            passcode=self.thserver_fabric_a_passcode,
            extra_args=["--capabilities", "0x04"])
        self.fabric_a_server_app.start(
            expected_output="Updating services using commissioning mode 1",
            timeout=30)

        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork-long 2 {self.thserver_fabric_a_passcode} {self.thserver_fabric_a_discriminator}",
            expected_output="[CTL] Commissioning complete for node ID 0x0000000000000002: success",
            timeout=30)

        # Creating a Controller for Ecosystem A
        _fabric_a_persistent_storage = VolatileTemporaryPersistentStorage(
            self.ecoACtrlStorage['repl-config'], self.ecoACtrlStorage['sdk-config'])
        _certAuthorityManagerA = CertificateAuthority.CertificateAuthorityManager(
            chipStack=self.matter_stack._chip_stack,
            persistentStorage=_fabric_a_persistent_storage)
        _certAuthorityManagerA.LoadAuthoritiesFromStorage()
        _devCtrlEcoA = _certAuthorityManagerA.activeCaList[0].adminList[0].NewController(
            nodeId=101,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
            catTags=[int(self.ecoACATs, 16), int('fffe0001', 16)])

        self.step("2")
        self.jfadmin_fabric_b_passcode = random.randint(20202021, 20202099)
        self.jfadmin_fabric_b_discriminator = random.randint(0, 4095)
        self.jfctrl_fabric_b_vid = random.randint(0x0001, 0xFFF0)

        # Start Fabric B JF-Administrator App
        self.fabric_b_admin = JFAdministratorSubprocess(
            self.jfa_server_app,
            prefix="JFA-B",
            storage_dir=self.storage_fabric_b,
            port=random.randint(5001, 5999),
            discriminator=self.jfadmin_fabric_b_discriminator,
            passcode=self.jfadmin_fabric_b_passcode,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", "33055"])
        self.fabric_b_admin.start(
            expected_output="Updating services using commissioning mode 1",
            timeout=30)

        # Start Fabric B JF-Administrator App
        self.fabric_b_ctrl = JFControllerSubprocess(
            self.jfc_server_app,
            prefix="JFC-B",
            rpc_server_port=33055,
            storage_dir=self.storage_fabric_b,
            vendor_id=self.jfctrl_fabric_b_vid)
        self.fabric_b_ctrl.start(
            expected_output="CHIP task running",
            timeout=30)

        # Commission JF-ADMIN app with JF-Controller on Fabric B
        self.fabric_b_ctrl.send(
            message=f"pairing onnetwork-long 11 {self.jfadmin_fabric_b_passcode} {self.jfadmin_fabric_b_discriminator} --anchor true",
            expected_output="[JF] Anchor Administrator (nodeId=11) commissioned with success",
            timeout=60)

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

        self.thserver_fabric_b_passcode = random.randint(20202021, 20202099)
        self.thserver_fabric_b_discriminator = random.randint(0, 4095)
        self.fabric_b_server_app = AppServerSubprocess(
            self.th_server_app,
            storage_dir=self.storage_fabric_b,
            port=random.randint(5001, 5999),
            discriminator=self.thserver_fabric_b_discriminator,
            passcode=self.thserver_fabric_b_passcode,
            extra_args=["--capabilities", "0x04"])
        self.fabric_b_server_app.start(
            expected_output="Updating services using commissioning mode 1",
            timeout=30)

        self.fabric_b_ctrl.send(
            message=f"pairing onnetwork-long 22 {self.thserver_fabric_b_passcode} {self.thserver_fabric_b_discriminator}",
            expected_output="[CTL] Commissioning complete for node ID 0x0000000000000016: success",
            timeout=30)

        # Creating a Controller for Ecosystem B
        _fabric_b_persistent_storage = VolatileTemporaryPersistentStorage(
            self.ecoBCtrlStorage['repl-config'], self.ecoBCtrlStorage['sdk-config'])
        _certAuthorityManagerB = CertificateAuthority.CertificateAuthorityManager(
            chipStack=self.matter_stack._chip_stack,
            persistentStorage=_fabric_b_persistent_storage)
        _certAuthorityManagerB.LoadAuthoritiesFromStorage()
        _devCtrlEcoB = _certAuthorityManagerB.activeCaList[0].adminList[0].NewController(
            nodeId=201,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
            catTags=[int(self.ecoBCATs, 16)])

        self.step("3")
        response = None  # Initialize the response variable to be used outside of the try block
        discriminator = random.randint(0, 4095)

        try:
            response = await _devCtrlEcoB.OpenJointCommissioningWindow(
                nodeId=11,
                endpointId=1,
                timeout=400,
                iteration=random.randint(1000, 100000),
                discriminator=discriminator
            )

        except Exception as e:
            asserts.assert_true(False, f'Exception {e} occured during OJCW')

        self.step("4")
        service_found = None
        discovery = mdns_discovery.MdnsDiscovery()
        raw_services = await discovery.get_commissionable_services(discovery_timeout_sec=240, log_output=True)
        services = [self.ParsedService(service) for service in raw_services]
        for parsed_service in services:
            if parsed_service.cm == 3:
                service_found = parsed_service
                break
        asserts.assert_is_not_none(
            service_found, "Failed to find a commissionable service with CM=3 (Joint Commissioning Method) being advertised.")
        log.info(f"Successfully found service with CM={service_found.cm}, D={service_found.d}")

        self.step("5")
        log.info("Setup event on fabric_a_admin for JCM completion message")
        self.fabric_a_admin.set_output_match("[JF] Joint Commissioning Method (nodeId=15) success")
        self.fabric_a_admin.event.clear()

        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork-long 15 {response.setupPinCode} {discriminator} --jcm true",
            expected_output="[CTL] Commissioning complete for node ID 0x000000000000000F: success",
            timeout=60)

        log.info("Waiting for transfer of ownership from the commissioner(controller) to the administrator and completion of commissioning")
        if self.fabric_a_admin.event.wait(30) is False:
            raise TimeoutError("Timed out waiting for commissioning to complete")
        log.info("JCM commissioning complete")

        self.step("6")
        response = await _devCtrlEcoA.ReadAttribute(
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

        if _devCtrlEcoA is not None:
            _devCtrlEcoA.Shutdown()
        if _devCtrlEcoB is not None:
            _devCtrlEcoB.Shutdown()
        if _fabric_a_persistent_storage is not None:
            _fabric_a_persistent_storage.Shutdown()
        if _fabric_b_persistent_storage is not None:
            _fabric_b_persistent_storage.Shutdown()


if __name__ == "__main__":
    default_matter_test_main()
