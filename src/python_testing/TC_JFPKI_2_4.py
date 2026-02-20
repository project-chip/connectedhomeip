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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import base64
import logging
import os
import random
import tempfile
from configparser import ConfigParser

from mobly import asserts

import matter.clusters as Clusters
import matter.discovery as Discovery
from matter import CertificateAuthority
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.storage import VolatileTemporaryPersistentStorage
from matter.testing.apps import JFAdministratorSubprocess, JFControllerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_JFPKI_2_4(MatterBaseTest):
    _JOINT_FABRIC_ADMINISTRATOR_ENDPOINT = 1
    _MIN_COMMISSIONING_TIMEOUT = 60
    _DEFAULT_OJCW_TIMEOUT = 60
    # TODO: Seems like the OJCW Discriminator should be randomized, but the test plan explicitly requires this number.
    _DEFAULT_OJCW_DISCRIMINATOR = 3840
    _DEFAULT_OJCW_ITERATIONS = 2000
    _VALID_97_BYTE_VERIFIER = bytes.fromhex(
        "b96170aae803346884724fe9a3b287c30330c2a660375d17bb205a8cf1aecb350457f8ab79ee253ab6a8e46bb09e543ae422736de501e3db37d441fe344920d09548e4c18240630c4ff4913c53513839b7c07fcc0627a1b8573a149fcd1fa466cf"
    )
    _VALID_16_BYTE_SALT = b"SPAKE2P Key Salt"
    _REQUEST_TIMEOUT_MS = 5000
    _TIMEOUT_STEP_2 = 180

    def desc_TC_JFPKI_2_4(self) -> str:
        return "[TC-JFPKI-2.4] Validate OpenJointCommissioningWindow Command correct behavior"

    def pics_TC_JFPKI_2_4(self) -> list[str]:
        return ["JFPKI.S"]

    @property
    def default_timeout(self) -> int:
        """
        We override the test timeout to support steps where we wait for the commissioning timeout to expire.
        """

        return self._TIMEOUT_STEP_2 + (self._DEFAULT_OJCW_TIMEOUT * 2) + 30

    async def assert_ojcw(
        self,
        *,
        commissioning_timeout=_DEFAULT_OJCW_TIMEOUT,
        pake_passcode_verifier=_VALID_97_BYTE_VERIFIER,
        discriminator=_DEFAULT_OJCW_DISCRIMINATOR,
        iterations=_DEFAULT_OJCW_ITERATIONS,
        salt=_VALID_16_BYTE_SALT,
        expected_error_status=None,
        expected_cluster_status=None,
        expected_error_message=None,
    ):
        cmd = Clusters.JointFabricAdministrator.Commands.OpenJointCommissioningWindow(
            commissioningTimeout=commissioning_timeout,
            PAKEPasscodeVerifier=pake_passcode_verifier,
            discriminator=discriminator,
            iterations=iterations,
            salt=salt,
        )

        if expected_error_status is None:
            await self.send_single_cmd(
                cmd=cmd,
                dev_ctrl=self.dev_ctrl_eco_a,
                node_id=self.dut_node_id,
                endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
                timedRequestTimeoutMs=self._REQUEST_TIMEOUT_MS,
            )
            return

        assertion_message = expected_error_message or f"Expected {expected_error_status}."
        with asserts.assert_raises(InteractionModelError, assertion_message) as cm:
            await self.send_single_cmd(
                cmd=cmd,
                dev_ctrl=self.dev_ctrl_eco_a,
                node_id=self.dut_node_id,
                endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
                timedRequestTimeoutMs=self._REQUEST_TIMEOUT_MS,
            )
        asserts.assert_equal(cm.exception.status, expected_error_status,
                             f"Expected {expected_error_status}, got {cm.exception.status}")
        if expected_cluster_status is not None:
            asserts.assert_equal(cm.exception.clusterStatus, expected_cluster_status,
                                 f"Expected cluster status {expected_cluster_status}, got {cm.exception.clusterStatus}")

    async def discover_commissionable_nodes(self):
        return await self.dev_ctrl_eco_a.DiscoverCommissionableNodes(
            filterType=Discovery.FilterType.LONG_DISCRIMINATOR,
            filter=self._DEFAULT_OJCW_DISCRIMINATOR,
            stopOnFirst=True,
        )

    async def sleep(self, duration_sec):
        log.info(f"Sleeping for {duration_sec} seconds...")
        await asyncio.sleep(duration_sec)

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
            extra_args=["--capabilities", "0x04", "--rpc-server-port", "33033", "--min_commissioning_timeout", f"{self._MIN_COMMISSIONING_TIMEOUT}"])
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

    def steps_TC_JFPKI_2_4(self) -> list[TestStep]:
        return [
            TestStep("1", "Commission DUT to TH."),
            TestStep("2", "TH sends OJCW command to DUT with valid parameters: CommissioningTimeout=180, PAKEPasscodeVerifier=valid_97_byte_verifier, Discriminator=3840, Iterations=2000, Salt=valid_16_byte_salt.",
                     "DUT responds with SUCCESS status and opens its commissioning window."),
            TestStep("3", "Verify commissioning window is open by checking DUT advertisement.",
                     "DUT advertises commissioning service with correct discriminator."),
            TestStep("4", "Wait for CommissioningTimeout to expire.",
                     "DUT stops advertising commissioning service."),
            TestStep("5", "TH sends OJCW command to DUT with CommissioningTimeout=0.",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("6", "TH sends OJCW command to DUT with CommissioningTimeout=65535.",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("7", "TH sends OJCW command to DUT with PAKEPasscodeVerifier of incorrect length (96 bytes).",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("8", "TH sends OJCW command to DUT with PAKEPasscodeVerifier of incorrect length (98 bytes).",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("9", "TH sends OJCW command to DUT with Discriminator=4096 (out of range).",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("10", "TH sends OJCW command to DUT with Iterations=999 (below minimum).",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("11", "TH sends OJCW command to DUT with Iterations=100001 (above maximum).",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("12", "TH sends OJCW command to DUT with Salt of length 15 bytes (below minimum).",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("13", "TH sends OJCW command to DUT with Salt of length 33 bytes (above maximum).",
                     "DUT responds with INVALID_COMMAND status code."),
            TestStep("14", "TH sends OJCW command to DUT with valid parameters: CommissioningTimeout=60, Iterations=1000 (minimum valid), Salt=valid_32_byte_salt (maximum valid).",
                     "DUT responds with SUCCESS status and opens commissioning window."),
            TestStep("15", "Verify commissioning window behavior with minimum/maximum valid parameters.",
                     "DUT advertises commissioning service correctly."),
            TestStep("16", "While commissioning window is open, TH sends another OJCW command.",
                     "DUT responds with BUSY status code."),
            TestStep("17", "Wait for commissioning window to close.",
                     "DUT stops advertising commissioning service."),
            TestStep("18", "TH sends OJCW command to DUT with Iterations=50000 (mid-range valid), Salt=valid_24_byte_salt (mid-range valid).",
                     "DUT responds with SUCCESS status and opens commissioning window."),
            TestStep("19", "Verify commissioning window opens with mid-range parameters.",
                     "DUT advertises commissioning service correctly."),
            TestStep("20", "Wait for commissioning window to close.",
                     "DUT stops advertising commissioning service."),
        ]

    @async_test_body
    async def test_TC_JFPKI_2_4(self):
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

        admin_fabric_idx = await self.read_single_attribute_check_success(
            dev_ctrl=self.dev_ctrl_eco_a,
            node_id=self.dut_node_id,
            endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
            cluster=Clusters.JointFabricAdministrator,
            attribute=Clusters.JointFabricAdministrator.Attributes.AdministratorFabricIndex,
        )
        asserts.assert_true(
            admin_fabric_idx not in (None, NullValue, 0),
            "Precondition failed: AdministratorFabricIndex must be non-null after commissioning"
            f" (expected non-null value, got {admin_fabric_idx!r})",
        )
        asserts.assert_true(
            admin_fabric_idx in range(1, 255),
            f"Precondition failed: AdministratorFabricIndex={admin_fabric_idx!r} not in valid range [1..254]",
        )

        self.step("2")
        await self.assert_ojcw(commissioning_timeout=self._TIMEOUT_STEP_2)

        self.step("3")
        responses = await self.discover_commissionable_nodes()
        asserts.assert_greater_equal(
            len(responses), 1, "DUT should advertise commissioning service with discriminator 3840"
        )

        self.step("4")
        await self.sleep(self._TIMEOUT_STEP_2 + 1)
        responses = await self.discover_commissionable_nodes()
        asserts.assert_equal(
            len(responses), 0, "DUT should have stopped advertising commissioning service with discriminator 3840"
        )

        self.step("5")
        await self.assert_ojcw(
            commissioning_timeout=0,
            expected_error_status=Status.InvalidCommand,
            expected_error_message="Expected INVALID_COMMAND for CommissioningTimeout=0.",
        )

        self.step("6")
        await self.assert_ojcw(
            commissioning_timeout=65535,
            expected_error_status=Status.InvalidCommand,
            expected_error_message="Expected INVALID_COMMAND for CommissioningTimeout=65535.",
        )

        self.step("7")
        invalid_96_byte_verifier = self._VALID_97_BYTE_VERIFIER[:-1]
        await self.assert_ojcw(
            pake_passcode_verifier=invalid_96_byte_verifier,
            expected_error_status=Status.InvalidCommand,
            expected_error_message="Expected INVALID_COMMAND for PAKEPasscodeVerifier length=96.",
        )

        self.step("8")
        invalid_98_byte_verifier = self._VALID_97_BYTE_VERIFIER + bytes([0xff])
        await self.assert_ojcw(
            pake_passcode_verifier=invalid_98_byte_verifier,
            expected_error_status=Status.InvalidCommand,
            expected_error_message="Expected INVALID_COMMAND for PAKEPasscodeVerifier length=98.",
        )

        self.step("9")
        await self.assert_ojcw(
            discriminator=4096,
            expected_error_status=Status.InvalidCommand,
            expected_error_message="Expected INVALID_COMMAND for Discriminator=4096.",
        )

        self.step("10")
        await self.assert_ojcw(
            iterations=999,
            expected_error_status=Status.InvalidCommand,
            expected_error_message="Expected INVALID_COMMAND for Iterations=999.",
        )

        self.step("11")
        await self.assert_ojcw(
            iterations=100001,
            expected_error_status=Status.InvalidCommand,
            expected_error_message="Expected INVALID_COMMAND for Iterations=100001.",
        )

        self.step("12")
        invalid_15_byte_salt = self._VALID_16_BYTE_SALT[:-1]
        await self.assert_ojcw(
            salt=invalid_15_byte_salt,
            expected_error_status=Status.InvalidCommand,
            expected_error_message="Expected INVALID_COMMAND for Salt length=15.",
        )

        self.step("13")
        invalid_33_byte_salt = self._VALID_16_BYTE_SALT + bytes(range(17))
        await self.assert_ojcw(
            salt=invalid_33_byte_salt,
            expected_error_status=Status.InvalidCommand,
            expected_error_message="Expected INVALID_COMMAND for Salt length=33.",
        )

        self.step("14")
        valid_32_byte_salt = self._VALID_16_BYTE_SALT + bytes(range(16))
        await self.assert_ojcw(iterations=1000, salt=valid_32_byte_salt)

        self.step("15")
        responses = await self.discover_commissionable_nodes()
        asserts.assert_greater_equal(
            len(responses), 1, "DUT should advertise commissioning service with discriminator 3840"
        )

        self.step("16")
        await self.assert_ojcw(
            iterations=1000,
            salt=valid_32_byte_salt,
            expected_error_status=Status.Failure,
            expected_cluster_status=Clusters.JointFabricAdministrator.Enums.StatusCodeEnum.kBusy,
            expected_error_message="Expected BUSY cluster status while commissioning window is already open.",
        )

        self.step("17")
        await self.sleep(self._DEFAULT_OJCW_TIMEOUT + 1)
        responses = await self.discover_commissionable_nodes()
        asserts.assert_equal(
            len(responses), 0, "DUT should have stopped advertising commissioning service with discriminator 3840"
        )

        self.step("18")
        valid_24_byte_salt = self._VALID_16_BYTE_SALT + bytes(range(8))
        await self.assert_ojcw(iterations=50000, salt=valid_24_byte_salt)

        self.step("19")
        responses = await self.discover_commissionable_nodes()
        asserts.assert_greater_equal(
            len(responses), 1, "DUT should advertise commissioning service with discriminator 3840"
        )

        self.step("20")
        await self.sleep(self._DEFAULT_OJCW_TIMEOUT + 1)
        responses = await self.discover_commissionable_nodes()
        asserts.assert_equal(
            len(responses), 0, "DUT should have stopped advertising commissioning service with discriminator 3840"
        )


if __name__ == "__main__":
    default_matter_test_main()
