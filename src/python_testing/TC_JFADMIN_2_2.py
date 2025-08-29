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
import time
from configparser import ConfigParser

from mobly import asserts

import matter.clusters as Clusters
from matter import CertificateAuthority
from matter.interaction_model import InteractionModelError
from matter.storage import VolatileTemporaryPersistentStorage
from matter.testing.apps import AppServerSubprocess, JFControllerSubprocess
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_JFADMIN_2_2(MatterBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.fabric_a_ctrl = None
        self.storage_fabric_a = self.user_params.get("fabric_a_storage", None)
        self.fabric_a_server_app = None

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
            logging.info("Temporary storage directory: %s", self.storage_fabric_a)

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

        # # Commission JF-ADMIN app with JF-Controller on Fabric A
        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork 1 {self.jfadmin_fabric_a_passcode} --anchor true",
            expected_output="[JF] Anchor Administrator (nodeId=1) commissioned with success",
            timeout=10)

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

    def teardown_class(self):
        # Stop all Subprocesses that were started in this test case
        if self.fabric_a_admin is not None:
            self.fabric_a_admin.terminate()
        if self.fabric_a_ctrl is not None:
            self.fabric_a_ctrl.terminate()

        super().teardown_class()

    def steps_TC_JFADMIN_2_2(self) -> list[TestStep]:
        return [
            TestStep("2", "TH sends ICACCSRRequest command to DUT.",
                     "DUT response contains status code FAILSAFE_REQUIRED."),
            TestStep("3", "TH sends ArmFailSafe command to DUT with ExpiryLengthSeconds set to 10 and Breadcrumb 1.",
                     "DUT respond with ArmFailSafeResponse Command."),
            TestStep("4", "TH sends ICACCSRRequest command to DUT.",
                     "DUT response contains an ICACCSR."),
            TestStep("5", "Wait for ArmFailSafe to expire."),
            TestStep("6", "TH sends AddICAC command to DUT using icac1 as parameter.",
                     "DUT response contains status code FAILSAFE_REQUIRED."),
            TestStep("7", "TH sends ArmFailSafe command to DUT with ExpiryLengthSeconds set to 10 and Breadcrumb 1.",
                     "DUT respond with ArmFailSafeResponse Command."),
            # TestStep("8", "[SKIP Missing SDK Implementation] TH sends AddICAC command to DUT using icac1 as parameter.",
            #          "DUT ICACResponse contains status 2 (InvalidICAC).")
        ]

    @async_test_body
    async def test_TC_JFADMIN_2_2(self):

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

        self.step("2")
        try:
            await self.send_single_cmd(
                dev_ctrl=devCtrlEcoA,
                node_id=1,
                endpoint=1,
                cmd=Clusters.JointFabricAdministrator.Commands.ICACCSRRequest())
        except InteractionModelError as e:
            asserts.assert_in('FailsafeRequired (0xca)',
                              str(e), f'Expected FailsafeRequired error, but got {str(e)}')
        else:
            asserts.assert_true(False, 'Expected InteractionModelError with FailsafeRequired, but no exception occurred.')

        self.step("3")
        await self.send_single_cmd(
            dev_ctrl=devCtrlEcoA,
            node_id=1,
            cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=10, breadcrumb=1))

        self.step("4")
        response = await self.send_single_cmd(
            dev_ctrl=devCtrlEcoA,
            node_id=1,
            endpoint=1,
            cmd=Clusters.JointFabricAdministrator.Commands.ICACCSRRequest())
        asserts.assert_not_equal(response.icaccsr, b'', "No ICACSR was returned!")

        self.step("5")
        # Wait for ArmFailSafe timer to expire
        time.sleep(11)

        self.step("6")
        # Get the ICAC from JF-Admin
        response = await devCtrlEcoA.ReadAttribute(
            nodeid=1, attributes=[(0, Clusters.OperationalCredentials.Attributes.NOCs)],
            returnClusterObject=True)
        _icac = response[0][Clusters.OperationalCredentials].NOCs[0].icac
        cmd = Clusters.JointFabricAdministrator.Commands.AddICAC(_icac)
        try:
            await self.send_single_cmd(dev_ctrl=devCtrlEcoA, node_id=1, cmd=cmd, endpoint=1)
        except InteractionModelError as e:
            asserts.assert_in('FailsafeRequired (0xca)',
                              str(e), f'Expected FailsafeRequired error, but got {str(e)}')
        else:
            asserts.assert_true(False, 'Expected InteractionModelError with FailsafeRequired, but no exception occurred.')

        self.step("7")
        await self.send_single_cmd(
            dev_ctrl=devCtrlEcoA,
            node_id=1,
            cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=60, breadcrumb=1))

        # TODO SDK Functionality is not finished. Uncomment when implementation is ready
        # self.step("8")
        # cmd = Clusters.OperationalCredentials.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=True)
        # csr_update = await self.send_single_cmd(dev_ctrl=devCtrlEcoA, node_id=1, cmd=cmd)
        # new_noc_chain = await devCtrlEcoA.IssueNOCChain(csr_update, 1)
        # cmd = Clusters.JointFabricAdministrator.Commands.AnnounceJointFabricAdministrator(1)
        # await self.send_single_cmd(dev_ctrl=devCtrlEcoA, node_id=1, cmd=cmd, endpoint=1)
        # cmd = Clusters.JointFabricAdministrator.Commands.ICACCSRRequest()
        # await self.send_single_cmd(dev_ctrl=devCtrlEcoA, node_id=1, cmd=cmd, endpoint=1)
        # cmd = Clusters.JointFabricAdministrator.Commands.AddICAC(new_noc_chain.icacBytes)
        # try:
        #     await self.send_single_cmd(dev_ctrl=devCtrlEcoA, node_id=1, cmd=cmd, endpoint=1)
        # except InteractionModelError as e:
        #     asserts.assert_in('InvalidICAC (0x02)',
        #                       str(e), f'Expected InvalidICAC error, but got {str(e)}')
        # else:
        #     asserts.assert_true(False, 'Expected InteractionModelError with InvalidICAC, but no exception occurred.')

        # Shutdown the Python Controllers started at the beginning of this script
        devCtrlEcoA.Shutdown()


if __name__ == "__main__":
    default_matter_test_main()
