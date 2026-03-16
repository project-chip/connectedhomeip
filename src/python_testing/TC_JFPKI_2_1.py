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
#       --PICS src/app/tests/suites/certification/ci-pics-values
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
from matter.storage import VolatileTemporaryPersistentStorage
from matter.testing.apps import AppServerSubprocess, JFControllerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_JFPKI_2_1(MatterBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.fabric_a_ctrl = None
        self.fabric_a_admin = None
        self.storage_fabric_a = self.user_params.get("fabric_a_storage", None)

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

        # Create a temporary storage directory for both ecosystems to keep KVS files if not already provided by user.
        if self.storage_fabric_a is None:
            self.storage_directory_ecosystem_a = tempfile.TemporaryDirectory(prefix=self.__class__.__name__+"_A_")
            self.storage_fabric_a = self.storage_directory_ecosystem_a.name
            log.info("Temporary storage directory: %s", self.storage_fabric_a)

    def teardown_class(self):
        # Stop all Subprocesses that were started in this test case
        if self.fabric_a_admin is not None:
            self.fabric_a_admin.terminate()
        if self.fabric_a_ctrl is not None:
            self.fabric_a_ctrl.terminate()

        super().teardown_class()

    def steps_TC_JFPKI_2_1(self) -> list[TestStep]:
        return [
            TestStep("1", "TH starts a commissioning process with DUT.",
                     "DUT is commissioned by TH with success."),
            TestStep("2", "TH reads AdministratorFabricIndex attribute from DUT.",
                     "DUT response contains AdministratorFabricIndex with a value in range 1..254."),
            TestStep("3", "TH1 reads Fabrics attribute from Operation Cluster on EP0.",
                     "DUT response contains a FabricDescriptorStruct with FabricID equal to AdministratorFabricIndex from step 2.")
        ]

    def pics_TC_JFPKI_2_1(self):
        return ['JFPKI.S']

    @async_test_body
    async def test_TC_JFPKI_2_1(self):
        self.step("1")
        self.JFPKI_fabric_a_passcode = random.randint(110220011, 110220999)
        self.jfctrl_fabric_a_vid = random.randint(0x0001, 0xFFF0)

        # Start Fabric A JF-Administrator App
        self.fabric_a_admin = AppServerSubprocess(
            self.jfa_server_app,
            storage_dir=self.storage_fabric_a,
            port=random.randint(5001, 5999),
            discriminator=random.randint(0, 4095),
            passcode=self.JFPKI_fabric_a_passcode,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", "33033"])
        self.fabric_a_admin.start(
            expected_output="Server initialization complete",
            timeout=20)

        # Start Fabric A JF-Controller App
        self.fabric_a_ctrl = JFControllerSubprocess(
            self.jfc_server_app,
            "JFC-A",
            rpc_server_port=33033,
            storage_dir=self.storage_fabric_a,
            vendor_id=self.jfctrl_fabric_a_vid)
        self.fabric_a_ctrl.start(
            expected_output="CHIP task running",
            timeout=20)

        # Commission JF-ADMIN app with JF-Controller on Fabric A
        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork 1 {self.JFPKI_fabric_a_passcode} --anchor true",
            expected_output="[JF] Anchor Administrator (nodeId=1) commissioned with success",
            timeout=20)

        # Extract the Ecosystem A certificates and inject them in the storage that will be provided to a new Python Controller later
        jfcStorage = ConfigParser()
        jfcStorage.read(os.path.join(self.storage_fabric_a, 'chip_tool_config.alpha.ini'))
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

        if self.pics_guard(self.check_pics("JFPKI.S.A0000")):
            self.step("2")
            response = await devCtrlEcoA.ReadAttribute(
                nodeId=1, attributes=[(1, Clusters.JointFabricAdministrator.Attributes.AdministratorFabricIndex)],
                returnClusterObject=True)
            attributeAdminFabricIndex = response[1][Clusters.JointFabricAdministrator].administratorFabricIndex
            asserts.assert_true(attributeAdminFabricIndex in range(1, 255),
                                f"AdministratorFabricIndex={attributeAdminFabricIndex} not in expected range [1..254]")

            # Step 3 is under same PICS guard as Step 2 becasue it relies on attributeAdminFabricIndex
            self.step("3")
            response = await devCtrlEcoA.ReadAttribute(
                nodeId=1, attributes=[(0, Clusters.OperationalCredentials.Attributes.Fabrics)],
                returnClusterObject=True)
            fabricid_found = False
            for fabric in response[0][Clusters.OperationalCredentials].fabrics:
                log.info(f"Checking fabricIndex from fabricID={fabric.fabricID}")
                if fabric.fabricIndex == attributeAdminFabricIndex:
                    fabricid_found = True
                    log.info(f"Found matching fabricIndex={attributeAdminFabricIndex} on fabricID={fabric.fabricID}")
                    break
            asserts.assert_true(fabricid_found,
                                "No FabricDescriptorStruct with fabricIndex = AdministratorFabricIndex found in Operational Cluster Fabrics attribute on EP0")

        # Shutdown the Python Controllers started at the beginning of this script
        devCtrlEcoA.Shutdown()


if __name__ == "__main__":
    default_matter_test_main()
