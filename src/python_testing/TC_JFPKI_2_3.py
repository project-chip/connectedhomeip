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

# This test requires a Joint Fabric Administrator app and Joint Fabric Controller app.
# Please specify with:
#   --string-arg jfa_server_app:<path_to_app>
#   --string-arg jfc_server_app:<path_to_app>

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

import logging
import os
import random
import re
import tempfile
import base64
from configparser import ConfigParser

from mobly import asserts

import matter.clusters as Clusters
from matter import CertificateAuthority
from matter.storage import VolatileTemporaryPersistentStorage
from matter.testing.apps import JFAdministratorSubprocess, JFControllerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_JFPKI_2_3(MatterBaseTest):
    _JOINT_FABRIC_ADMINISTRATOR_ENDPOINT = 1
    _GENERAL_COMMISSIONING_ENDPOINT = 0

    def desc_TC_JFPKI_2_3(self) -> str:
        return "[TC-JFPKI-2.3] Validate ICACCSRResponse command"

    def pics_TC_JFPKI_2_3(self) -> list[str]:
        return ["JFPKI.S"]

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.fabric_a_ctrl = None
        self.fabric_a_admin = None
        self.cert_authority_manager_a = None
        self.dev_ctrl_eco_a = None
        self.storage_fabric_a = self.user_params.get("fabric_a_storage", None)
        self.fabric_a_persistent_storage = None

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

        # Create a temporary storage directory to keep KVS files if not provided by user.
        if self.storage_fabric_a is None:
            self.storage_directory_ecosystem_a = tempfile.mkdtemp(prefix=self.__class__.__name__+"_A_")
            self.storage_fabric_a = self.storage_directory_ecosystem_a
            log.info("Temporary storage directory: %s", self.storage_fabric_a)

        self.jfadmin_fabric_a_passcode = random.randint(20202021, 20202099)
        self.jfadmin_fabric_a_discriminator = random.randint(0, 4095)
        self.jfctrl_fabric_a_vid = random.randint(0x0001, 0xFFF0)

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

        self.fabric_a_ctrl = JFControllerSubprocess(
            self.jfc_server_app,
            prefix="JFC-A",
            rpc_server_port=33033,
            storage_dir=self.storage_fabric_a,
            vendor_id=self.jfctrl_fabric_a_vid)
        self.fabric_a_ctrl.start(
            expected_output="CHIP task running",
            timeout=30)

    def teardown_class(self):
        if self.fabric_a_persistent_storage is not None:
            self.fabric_a_persistent_storage.Shutdown()
        if self.fabric_a_admin is not None:
            self.fabric_a_admin.terminate()
        if self.fabric_a_ctrl is not None:
            self.fabric_a_ctrl.terminate()
        if self.cert_authority_manager_a is not None:
            self.cert_authority_manager_a.Shutdown()

        super().teardown_class()

    def steps_TC_JFPKI_2_3(self) -> list[TestStep]:
        return [
            TestStep("1", "Commission DUT to TH."),
            TestStep("2", "TH sends ArmFailSafe command to DUT with ExpiryLengthSeconds set to 10 and Breadcrumb 1.",
                     "DUT respond with ArmFailSafeResponse Command."),
            TestStep("3", "DUT performs Fabric Table VID Verification Procedure command against TH.",
                     "DUT performs with success Fabric Table VID Verification Procedure command against TH."),
            TestStep("4", "TH sends ICACCSRRequest command to DUT.",
                     "Verify DUT responds with ICACCSRResponse containing DER-encoded PKCS#10 CSR (max 600 bytes)."),
        ]

    @async_test_body
    async def test_TC_JFPKI_2_3(self):
        self.step("1")
        self.fabric_a_ctrl.send(
            message=f"pairing onnetwork-long {self.dut_node_id} {self.jfadmin_fabric_a_passcode} {self.jfadmin_fabric_a_discriminator} --anchor true",
            expected_output=f"[JF] Anchor Administrator (nodeId={self.dut_node_id}) commissioned with success",
            timeout=60)

        jfc_storage = ConfigParser()
        jfc_storage.read(os.path.join(self.storage_fabric_a, 'chip_tool_config.alpha.ini'))
        self.eco_a_ctrl_storage = {
            "sdk-config": {
                "ExampleOpCredsCAKey1": jfc_storage.get("Default", "ExampleOpCredsCAKey0"),
                "ExampleOpCredsICAKey1": jfc_storage.get("Default", "ExampleOpCredsICAKey0"),
                "ExampleCARootCert1": jfc_storage.get("Default", "ExampleCARootCert0"),
                "ExampleCAIntermediateCert1": jfc_storage.get("Default", "ExampleCAIntermediateCert0"),
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
        self.eco_a_cats = int(base64.b64decode(jfc_storage.get("Default", "CommissionerCATs"))[::-1].hex().strip('0'), 16)

        self.fabric_a_persistent_storage = VolatileTemporaryPersistentStorage(
            self.eco_a_ctrl_storage['repl-config'], self.eco_a_ctrl_storage['sdk-config'])
        self.cert_authority_manager_a = CertificateAuthority.CertificateAuthorityManager(
            chipStack=self.matter_stack._chip_stack,
            persistentStorage=self.fabric_a_persistent_storage)
        self.cert_authority_manager_a.LoadAuthoritiesFromStorage()
        self.dev_ctrl_eco_a = self.cert_authority_manager_a.activeCaList[0].adminList[0].NewController(
            nodeId=101,
            paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
            catTags=[self.eco_a_cats])

        self.step("2")
        resp = await self.send_single_cmd(
            dev_ctrl=self.dev_ctrl_eco_a,
            node_id=self.dut_node_id,
            endpoint=self._GENERAL_COMMISSIONING_ENDPOINT,
            cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=10, breadcrumb=1))
        asserts.assert_true(
            isinstance(resp, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
            f"Unexpected response type: {type(resp)}",
        )
        asserts.assert_equal(
            resp.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            f"ArmFailSafeResponse error code not OK: {resp.errorCode}",
        )

        self.step("3")
        # TODO: DUT performs Fabric Table VID Verification Procedure command against TH.
        # Expected: DUT performs with success Fabric Table VID Verification Procedure command against TH.

        self.step("4")
        # TODO: TH sends ICACCSRRequest command to DUT.
        # Expected: Verify DUT responds with ICACCSRResponse containing DER-encoded PKCS#10 CSR (max 600 bytes).


if __name__ == "__main__":
    default_matter_test_main()
