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
import tempfile
import base64
from configparser import ConfigParser

import asyncio
from mobly import asserts

import matter.clusters as Clusters
import matter.discovery as Discovery
from matter import CertificateAuthority
from matter.interaction_model import InteractionModelError, Status
from matter.storage import VolatileTemporaryPersistentStorage
from matter.testing.apps import JFAdministratorSubprocess, JFControllerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_JFPKI_2_4(MatterBaseTest):
    _JOINT_FABRIC_ADMINISTRATOR_ENDPOINT = 1
    _GENERAL_COMMISSIONING_ENDPOINT = 0

    def desc_TC_JFPKI_2_4(self) -> str:
        return "[TC-JFPKI-2.4] Validate OpenJointCommissioningWindow Command correct behavior"

    def pics_TC_JFPKI_2_4(self) -> list[str]:
        return ["JFPKI.S"]

    @property
    def default_timeout(self) -> int:
        """We extend the timeout to support steps where we wait for the commissioning timeout to expire."""
        return 270

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

        self.step("2")
        valid_97_byte_verifier = bytes.fromhex(
            "b96170aae803346884724fe9a3b287c30330c2a660375d17bb205a8cf1aecb350457f8ab79ee253ab6a8e46bb09e543ae422736de501e3db37d441fe344920d09548e4c18240630c4ff4913c53513839b7c07fcc0627a1b8573a149fcd1fa466cf")
        valid_16_byte_salt = b"SPAKE2P Key Salt"
        timeout_step_2 = 180
        discriminator_step_2 = 3840
        cmd = Clusters.JointFabricAdministrator.Commands.OpenJointCommissioningWindow(
            commissioningTimeout=timeout_step_2,
            PAKEPasscodeVerifier=valid_97_byte_verifier,
            discriminator=discriminator_step_2,
            iterations=2000,
            salt=valid_16_byte_salt,
        )
        await self.send_single_cmd(
            cmd=cmd,
            dev_ctrl=self.dev_ctrl_eco_a,
            node_id=self.dut_node_id,
            endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
            timedRequestTimeoutMs=5000,
        )

        self.step("3")
        responses = await self.dev_ctrl_eco_a.DiscoverCommissionableNodes(
            filterType=Discovery.FilterType.LONG_DISCRIMINATOR,
            filter=discriminator_step_2,
            stopOnFirst=True,
        )
        asserts.assert_greater_equal(
            len(responses), 1, "DUT should advertise commissioning service with discriminator 3840"
        )

        self.step("4")
        sleep_seconds = timeout_step_2 + 1
        log.info(f"Waiting {sleep_seconds} seconds for CommissioningTimeout to expire...")
        await asyncio.sleep(sleep_seconds)
        responses = await self.dev_ctrl_eco_a.DiscoverCommissionableNodes(
            filterType=Discovery.FilterType.LONG_DISCRIMINATOR,
            filter=discriminator_step_2,
            stopOnFirst=True,
        )
        asserts.assert_equal(
            len(responses), 0, "DUT should have stopped advertising commissioning service with discriminator 3840"
        )

        self.step("5")
        cmd = Clusters.JointFabricAdministrator.Commands.OpenJointCommissioningWindow(
            commissioningTimeout=0,
            PAKEPasscodeVerifier=valid_97_byte_verifier,
            discriminator=discriminator_step_2,
            iterations=2000,
            salt=valid_16_byte_salt,
        )
        with asserts.assert_raises(InteractionModelError, "Expected INVALID_COMMAND for CommissioningTimeout=0.") as cm:
            await self.send_single_cmd(
                cmd=cmd,
                dev_ctrl=self.dev_ctrl_eco_a,
                node_id=self.dut_node_id,
                endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
                timedRequestTimeoutMs=5000,
            )
        asserts.assert_equal(cm.exception.status, Status.InvalidCommand,
                             f"Expected {Status.InvalidCommand}, got {cm.exception.status}")

        self.step("6")
        cmd = Clusters.JointFabricAdministrator.Commands.OpenJointCommissioningWindow(
            commissioningTimeout=65535,
            PAKEPasscodeVerifier=valid_97_byte_verifier,
            discriminator=discriminator_step_2,
            iterations=2000,
            salt=valid_16_byte_salt,
        )
        with asserts.assert_raises(InteractionModelError, "Expected INVALID_COMMAND for CommissioningTimeout=65535.") as cm:
            await self.send_single_cmd(
                cmd=cmd,
                dev_ctrl=self.dev_ctrl_eco_a,
                node_id=self.dut_node_id,
                endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
                timedRequestTimeoutMs=5000,
            )
        asserts.assert_equal(cm.exception.status, Status.InvalidCommand,
                             f"Expected {Status.InvalidCommand}, got {cm.exception.status}")

        self.step("7")
        invalid_96_byte_verifier = valid_97_byte_verifier[:-1]
        cmd = Clusters.JointFabricAdministrator.Commands.OpenJointCommissioningWindow(
            commissioningTimeout=60,
            PAKEPasscodeVerifier=invalid_96_byte_verifier,
            discriminator=discriminator_step_2,
            iterations=2000,
            salt=valid_16_byte_salt,
        )
        with asserts.assert_raises(InteractionModelError, "Expected INVALID_COMMAND for PAKEPasscodeVerifier length=96.") as cm:
            await self.send_single_cmd(
                cmd=cmd,
                dev_ctrl=self.dev_ctrl_eco_a,
                node_id=self.dut_node_id,
                endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
                timedRequestTimeoutMs=5000,
            )
        asserts.assert_equal(cm.exception.status, Status.InvalidCommand,
                             f"Expected {Status.InvalidCommand}, got {cm.exception.status}")

        self.step("8")
        invalid_98_byte_verifier = valid_97_byte_verifier + bytes([0x00])
        cmd = Clusters.JointFabricAdministrator.Commands.OpenJointCommissioningWindow(
            commissioningTimeout=60,
            PAKEPasscodeVerifier=invalid_98_byte_verifier,
            discriminator=discriminator_step_2,
            iterations=2000,
            salt=valid_16_byte_salt,
        )
        with asserts.assert_raises(InteractionModelError, "Expected INVALID_COMMAND for PAKEPasscodeVerifier length=98.") as cm:
            await self.send_single_cmd(
                cmd=cmd,
                dev_ctrl=self.dev_ctrl_eco_a,
                node_id=self.dut_node_id,
                endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
                timedRequestTimeoutMs=5000,
            )
        asserts.assert_equal(cm.exception.status, Status.InvalidCommand,
                             f"Expected {Status.InvalidCommand}, got {cm.exception.status}")

        self.step("9")
        cmd = Clusters.JointFabricAdministrator.Commands.OpenJointCommissioningWindow(
            commissioningTimeout=60,
            PAKEPasscodeVerifier=valid_97_byte_verifier,
            discriminator=4096,
            iterations=2000,
            salt=valid_16_byte_salt,
        )
        with asserts.assert_raises(InteractionModelError, "Expected INVALID_COMMAND for Discriminator=4096.") as cm:
            await self.send_single_cmd(
                cmd=cmd,
                dev_ctrl=self.dev_ctrl_eco_a,
                node_id=self.dut_node_id,
                endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
                timedRequestTimeoutMs=5000,
            )
        asserts.assert_equal(cm.exception.status, Status.InvalidCommand,
                             f"Expected {Status.InvalidCommand}, got {cm.exception.status}")

        self.step("10")
        cmd = Clusters.JointFabricAdministrator.Commands.OpenJointCommissioningWindow(
            commissioningTimeout=60,
            PAKEPasscodeVerifier=valid_97_byte_verifier,
            discriminator=discriminator_step_2,
            iterations=999,
            salt=valid_16_byte_salt,
        )
        with asserts.assert_raises(InteractionModelError, "Expected INVALID_COMMAND for Iterations=999.") as cm:
            await self.send_single_cmd(
                cmd=cmd,
                dev_ctrl=self.dev_ctrl_eco_a,
                node_id=self.dut_node_id,
                endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
                timedRequestTimeoutMs=5000,
            )
        asserts.assert_equal(cm.exception.status, Status.InvalidCommand,
                             f"Expected {Status.InvalidCommand}, got {cm.exception.status}")

        self.step("11")
        cmd = Clusters.JointFabricAdministrator.Commands.OpenJointCommissioningWindow(
            commissioningTimeout=60,
            PAKEPasscodeVerifier=valid_97_byte_verifier,
            discriminator=discriminator_step_2,
            iterations=100001,
            salt=valid_16_byte_salt,
        )
        with asserts.assert_raises(InteractionModelError, "Expected INVALID_COMMAND for Iterations=100001.") as cm:
            await self.send_single_cmd(
                cmd=cmd,
                dev_ctrl=self.dev_ctrl_eco_a,
                node_id=self.dut_node_id,
                endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
                timedRequestTimeoutMs=5000,
            )
        asserts.assert_equal(cm.exception.status, Status.InvalidCommand,
                             f"Expected {Status.InvalidCommand}, got {cm.exception.status}")

        self.step("12")
        invalid_15_byte_salt = valid_16_byte_salt[:-1]
        cmd = Clusters.JointFabricAdministrator.Commands.OpenJointCommissioningWindow(
            commissioningTimeout=60,
            PAKEPasscodeVerifier=valid_97_byte_verifier,
            discriminator=discriminator_step_2,
            iterations=2000,
            salt=invalid_15_byte_salt,
        )
        with asserts.assert_raises(InteractionModelError, "Expected INVALID_COMMAND for Salt length=15.") as cm:
            await self.send_single_cmd(
                cmd=cmd,
                dev_ctrl=self.dev_ctrl_eco_a,
                node_id=self.dut_node_id,
                endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
                timedRequestTimeoutMs=5000,
            )
        asserts.assert_equal(cm.exception.status, Status.InvalidCommand,
                             f"Expected {Status.InvalidCommand}, got {cm.exception.status}")

        self.step("13")
        invalid_33_byte_salt = valid_16_byte_salt + bytes(range(17))
        cmd = Clusters.JointFabricAdministrator.Commands.OpenJointCommissioningWindow(
            commissioningTimeout=60,
            PAKEPasscodeVerifier=valid_97_byte_verifier,
            discriminator=discriminator_step_2,
            iterations=2000,
            salt=invalid_33_byte_salt,
        )
        with asserts.assert_raises(InteractionModelError, "Expected INVALID_COMMAND for Salt length=33.") as cm:
            await self.send_single_cmd(
                cmd=cmd,
                dev_ctrl=self.dev_ctrl_eco_a,
                node_id=self.dut_node_id,
                endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
                timedRequestTimeoutMs=5000,
            )
        asserts.assert_equal(cm.exception.status, Status.InvalidCommand,
                             f"Expected {Status.InvalidCommand}, got {cm.exception.status}")

        self.step("14")
        valid_32_byte_salt = valid_16_byte_salt + bytes(range(16))
        cmd = Clusters.JointFabricAdministrator.Commands.OpenJointCommissioningWindow(
            commissioningTimeout=60,
            PAKEPasscodeVerifier=valid_97_byte_verifier,
            discriminator=discriminator_step_2,
            iterations=1000,
            salt=valid_32_byte_salt,
        )
        await self.send_single_cmd(
            cmd=cmd,
            dev_ctrl=self.dev_ctrl_eco_a,
            node_id=self.dut_node_id,
            endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
            timedRequestTimeoutMs=5000,
        )

        self.step("15")
        responses = await self.dev_ctrl_eco_a.DiscoverCommissionableNodes(
            filterType=Discovery.FilterType.LONG_DISCRIMINATOR,
            filter=discriminator_step_2,
            stopOnFirst=True,
        )
        asserts.assert_greater_equal(
            len(responses), 1, "DUT should advertise commissioning service with discriminator 3840"
        )

        self.step("16")
        cmd = Clusters.JointFabricAdministrator.Commands.OpenJointCommissioningWindow(
            commissioningTimeout=60,
            PAKEPasscodeVerifier=valid_97_byte_verifier,
            discriminator=discriminator_step_2,
            iterations=1000,
            salt=valid_32_byte_salt,
        )
        with asserts.assert_raises(InteractionModelError, "Expected BUSY while commissioning window is already open.") as cm:
            await self.send_single_cmd(
                cmd=cmd,
                dev_ctrl=self.dev_ctrl_eco_a,
                node_id=self.dut_node_id,
                endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
                timedRequestTimeoutMs=5000,
            )
        asserts.assert_equal(cm.exception.status, Status.Busy,
                             f"Expected {Status.Busy}, got {cm.exception.status}")

        self.step("17")
        await asyncio.sleep(61)
        responses = await self.dev_ctrl_eco_a.DiscoverCommissionableNodes(
            filterType=Discovery.FilterType.LONG_DISCRIMINATOR,
            filter=discriminator_step_2,
            stopOnFirst=True,
        )
        asserts.assert_equal(
            len(responses), 0, "DUT should have stopped advertising commissioning service with discriminator 3840"
        )
        # TODO: Investigate whether the discriminator should be randomized to reduce the likelihood of flakiness due to multiple physical devices advertising
        # or can we do some other filtering?

        self.step("18")
        valid_24_byte_salt = valid_16_byte_salt + bytes(range(8))
        cmd = Clusters.JointFabricAdministrator.Commands.OpenJointCommissioningWindow(
            commissioningTimeout=60,
            PAKEPasscodeVerifier=valid_97_byte_verifier,
            discriminator=discriminator_step_2,
            iterations=50000,
            salt=valid_24_byte_salt,
        )
        await self.send_single_cmd(
            cmd=cmd,
            dev_ctrl=self.dev_ctrl_eco_a,
            node_id=self.dut_node_id,
            endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
            timedRequestTimeoutMs=5000,
        )

        self.step("19")
        responses = await self.dev_ctrl_eco_a.DiscoverCommissionableNodes(
            filterType=Discovery.FilterType.LONG_DISCRIMINATOR,
            filter=discriminator_step_2,
            stopOnFirst=True,
        )
        asserts.assert_greater_equal(
            len(responses), 1, "DUT should advertise commissioning service with discriminator 3840"
        )

        self.step("20")
        sleep_seconds = 61
        log.info(f"Waiting {sleep_seconds} seconds for CommissioningTimeout to expire...")
        await asyncio.sleep(sleep_seconds)
        responses = await self.dev_ctrl_eco_a.DiscoverCommissionableNodes(
            filterType=Discovery.FilterType.LONG_DISCRIMINATOR,
            filter=discriminator_step_2,
            stopOnFirst=True,
        )
        asserts.assert_equal(
            len(responses), 0, "DUT should have stopped advertising commissioning service with discriminator 3840"
        )


if __name__ == "__main__":
    default_matter_test_main()
