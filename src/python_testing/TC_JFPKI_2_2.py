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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
import os
import random
import tempfile

from mobly import asserts

import matter.clusters as Clusters
import matter.discovery as Discovery
from matter.clusters.Types import NullValue
from matter.exceptions import ChipStackError
from matter.interaction_model import InteractionModelError, Status
from matter.testing.apps import AppServerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main
from matter.tlv import TLVReader, TLVWriter

log = logging.getLogger(__name__)


class TC_JFPKI_2_2(MatterBaseTest):
    _JOINT_FABRIC_ADMINISTRATOR_ENDPOINT = 1
    _OPERATIONAL_CREDENTIALS_ENDPOINT = 0
    _GENERAL_COMMISSIONING_ENDPOINT = 0

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.fabric_storage = self.user_params.get("fabric_storage", None)
        self.storage_directory_ecosystem_a = None

        self.jfa_server_app = self.user_params.get("jfa_server_app", None)
        if not self.jfa_server_app:
            asserts.fail("This test requires a Joint Fabrics Admin app. Specify app path with --string-arg jfa_server_app:<path_to_app>")
        if not os.path.exists(self.jfa_server_app):
            asserts.fail(f"The path {self.jfa_server_app} does not exist")

        # Create a temporary storage directory to keep KVS files if not already provided by user.
        if self.fabric_storage is None:
            self.storage_directory_ecosystem_a = tempfile.TemporaryDirectory(prefix=self.__class__.__name__+"_A_")
            self.fabric_storage = self.storage_directory_ecosystem_a.name
            log.info("Temporary storage directory: %s", self.fabric_storage)

        self.admin_passcode = random.randint(20202021, 20202099)
        self.admin_discriminator = random.randint(0, 4095)

        # Start JF-Administrator App
        self.jf_admin = AppServerSubprocess(
            self.jfa_server_app,
            storage_dir=self.fabric_storage,
            port=random.randint(5001, 5999),
            discriminator=self.admin_discriminator,
            passcode=self.admin_passcode,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", "33033"])
        self.jf_admin.start(
            expected_output="Server initialization complete",
            timeout=10)

    def teardown_class(self):
        self.jf_admin.terminate()
        if self.storage_directory_ecosystem_a is not None:
            self.storage_directory_ecosystem_a.cleanup()
        super().teardown_class()

    def steps_TC_JFPKI_2_2(self) -> list[TestStep]:
        return [
            TestStep("1", "Commission DUT to TH."),
            TestStep("2", "TH sends ICACCSRRequest command to DUT.",
                     "DUT response contains status code FAILSAFEREQUIRED."),
            TestStep("3", "TH sends ArmFailSafe command to DUT with ExpiryLengthSeconds set to 10 and Breadcrumb 1.",
                     "DUT respond with ArmFailSafeResponse Command."),
            TestStep("4", "TH sends ICACCSRRequest command to DUT.",
                     "DUT response contains status code VIDNotVerified."),
            TestStep("5", "TH sends AddICAC command to DUT using icac1 as parameter.",
                     "DUT responds with SUCCESS status."),
            TestStep("6", "TH sends ICACCSRRequest command to DUT.",
                     "DUT responds with status code CONSTRAINTERROR."),
            TestStep("7", "Wait for ArmFailSafe to expire."),
            TestStep("8", "TH sends AddICAC command to DUT using icac1 as parameter.",
                     "DUT response contains status code FAILSAFEREQUIRED."),
            TestStep("9", "TH sends ArmFailSafe command to DUT with ExpiryLengthSeconds set to 20 and Breadcrumb 1.",
                     "DUT respond with ArmFailSafeResponse Command."),
            TestStep("10", "TH sends AddICAC command to DUT using icac1 as parameter.",
                     "DUT responds with SUCCESS status."),
            TestStep("11", "TH sends AddICAC command to DUT using icac1 as parameter.",
                     "DUT responds with status code CONSTRAINTERROR."),
            TestStep("12", "Wait for ArmFailSafe to expire."),
            TestStep("13", "TH sends ArmFailSafe command to DUT with ExpiryLengthSeconds set to 20 and Breadcrumb 1.",
                     "DUT respond with ArmFailSafeResponse Command."),
            TestStep("14", "TH sends AddICAC command to DUT using an ICAC that is not associated with the RCAC of TH as parameter.",
                     "DUT ICACResponse contains status 2 (InvalidICAC)."),
            TestStep("15", "TH sends AddICAC command to DUT using an ICAC with a different public key than the public key present in ICACCSRResponse",
                     "DUT ICACResponse contains status 1 (InvalidPublicKey)."),
            TestStep("16", "TH sends AddICAC command to DUT using a certificate that doesn't follow DN encoding for ICAC",
                     "DUT ICACResponse contains status 2 (InvalidICAC)."),
            TestStep("17", "TH sends OJCW command to DUT (check Precondition 1)",
                     "DUT responds with status code 0x06 (InvalidAdministratorFabricIndex)."),
        ]

    def desc_TC_JFPKI_2_2(self):
        return "[TC-JFPKI-2.2] Validate commands behavior outside Joint Commissioning Method"

    def pics_TC_JFPKI_2_2(self):
        return ['JFPKI.S']

    @async_test_body
    async def test_TC_JFPKI_2_2(self):
        self.step("1")

        await self.default_controller.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=self.admin_passcode,
            filterType=Discovery.FilterType.LONG_DISCRIMINATOR,
            filter=self.admin_discriminator,
        )
        admin_fabric_idx = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
            cluster=Clusters.JointFabricAdministrator,
            attribute=Clusters.JointFabricAdministrator.Attributes.AdministratorFabricIndex,
        )
        asserts.assert_true(
            admin_fabric_idx in (None, NullValue, 0),
            "Precondition failed: AdministratorFabricIndex must be null after commissioning"
            f" (expected None/NullValue/0, got {admin_fabric_idx!r})",
        )

        self.step("2")
        await self._assert_im_error(
            cmd=Clusters.JointFabricAdministrator.Commands.ICACCSRRequest(),
            expected_status=Status.FailsafeRequired,
            error_label="FailsafeRequired",
        )

        self.step("3")
        await self._arm_failsafe(10)

        self.step("4")
        await self._assert_im_error(
            cmd=Clusters.JointFabricAdministrator.Commands.ICACCSRRequest(),
            expected_status=Status.Failure,
            expected_cluster_status=Clusters.JointFabricAdministrator.Enums.StatusCodeEnum.kVIDNotVerified,
            error_label="VIDNotVerified",
        )

        self.step("5")
        response = await self.default_controller.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[(self._OPERATIONAL_CREDENTIALS_ENDPOINT, Clusters.OperationalCredentials.Attributes.NOCs)],
            returnClusterObject=True)
        icac1 = response[0][Clusters.OperationalCredentials].NOCs[0].icac
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
            cmd=Clusters.JointFabricAdministrator.Commands.AddICAC(icac1))

        self.step("6")
        await self._assert_im_error(
            cmd=Clusters.JointFabricAdministrator.Commands.ICACCSRRequest(),
            expected_status=Status.ConstraintError,
            error_label="ConstraintError",
        )

        self.step("7")
        await self._expire_failsafe()

        self.step("8")
        await self._assert_im_error(
            cmd=Clusters.JointFabricAdministrator.Commands.AddICAC(icac1),
            expected_status=Status.FailsafeRequired,
            error_label="FailsafeRequired",
        )

        self.step("9")

        await self._arm_failsafe(20)

        self.step("10")
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
            cmd=Clusters.JointFabricAdministrator.Commands.AddICAC(icac1))

        self.step("11")
        await self._assert_im_error(
            cmd=Clusters.JointFabricAdministrator.Commands.AddICAC(icac1),
            expected_status=Status.ConstraintError,
            error_label="ConstraintError",
        )

        self.step("12")
        await self._expire_failsafe()

        self.step("13")
        await self._arm_failsafe(20)

        self.step("14")

        # Create a second CA to generate an ICAC that is valid but chained to a different RCAC than the TH controller fabric RCAC.
        csr_nonce = random.randbytes(32)
        csr_response = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._OPERATIONAL_CREDENTIALS_ENDPOINT,
            cmd=Clusters.OperationalCredentials.Commands.CSRRequest(CSRNonce=csr_nonce, isForUpdateNOC=True))
        other_ca = self.certificate_authority_manager.NewCertificateAuthority()
        other_admin = other_ca.NewFabricAdmin(vendorId=0xFFF1, fabricId=0x1234)
        other_ctrl = other_admin.NewController(nodeId=102)
        other_chain = await other_ctrl.IssueNOCChain(csr_response, self.dut_node_id)
        asserts.assert_true(other_chain.icacBytes is not None, "Unable to generate ICAC from alternate RCAC")
        other_icac = other_chain.icacBytes

        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
            cmd=Clusters.JointFabricAdministrator.Commands.AddICAC(other_icac))
        asserts.assert_true(
            isinstance(resp, Clusters.JointFabricAdministrator.Commands.ICACResponse),
            f"Unexpected response type: {type(resp)}",
        )
        asserts.assert_equal(
            resp.statusCode,
            Clusters.JointFabricAdministrator.Enums.ICACResponseStatusEnum.kInvalidICAC,
            f"Expected InvalidICAC response status, but got {resp.statusCode}",
        )

        self.step("15")
        # Reset fail-safe context: AddICAC can only be invoked once per armed fail-safe session.
        await self._expire_failsafe()
        await self._arm_failsafe(20)

        # Generate a new ICAC using the old CSR Response, which will have the wrong public key for this fail-safe context.
        icac_wrong_public_key = await self.default_controller.IssueNOCChain(csr_response, self.dut_node_id)

        # Send AddICAC with the ICAC containing the wrong public key
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
            cmd=Clusters.JointFabricAdministrator.Commands.AddICAC(icac_wrong_public_key.icacBytes))
        asserts.assert_true(
            isinstance(resp, Clusters.JointFabricAdministrator.Commands.ICACResponse),
            f"Unexpected response type: {type(resp)}",
        )
        asserts.assert_equal(
            resp.statusCode,
            Clusters.JointFabricAdministrator.Enums.ICACResponseStatusEnum.kInvalidPublicKey,
            f"Expected InvalidPublicKey response status, but got {resp.statusCode}",
        )

        self.step("16")

        # Reset fail-safe context: AddICAC can only be invoked once per armed fail-safe session.
        await self._expire_failsafe()
        await self._arm_failsafe(20)

        # Create a valid ICAC for the current fail-safe context.
        csr_nonce = random.randbytes(32)
        csr_response_step16 = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._OPERATIONAL_CREDENTIALS_ENDPOINT,
            cmd=Clusters.OperationalCredentials.Commands.CSRRequest(CSRNonce=csr_nonce, isForUpdateNOC=True))
        valid_icac_chain = await self.default_controller.IssueNOCChain(csr_response_step16, self.dut_node_id)

        # Corrupt Subject DN encoding by changing the Subject container from a DN path to a structure.
        icac_tlv = TLVReader(valid_icac_chain.icacBytes).get()["Any"]
        icac_tlv[6] = dict(icac_tlv[6])
        writer = TLVWriter()
        writer.put(None, icac_tlv)
        invalid_dn_icac = bytes(writer.encoding)

        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
            cmd=Clusters.JointFabricAdministrator.Commands.AddICAC(invalid_dn_icac))
        asserts.assert_true(
            isinstance(resp, Clusters.JointFabricAdministrator.Commands.ICACResponse),
            f"Unexpected response type: {type(resp)}",
        )
        asserts.assert_equal(
            resp.statusCode,
            Clusters.JointFabricAdministrator.Enums.ICACResponseStatusEnum.kInvalidICAC,
            f"Expected InvalidICAC response status, but got {resp.statusCode}",
        )

        self.step("17")
        # Ensure OJCW does not fail with Busy due to an active fail-safe context from prior steps.
        await self._expire_failsafe()

        with asserts.assert_raises(ChipStackError) as cm:
            await self.default_controller.OpenJointCommissioningWindow(
                nodeId=self.dut_node_id,
                endpointId=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
                timeout=400,
                iteration=random.randint(1000, 100000),
                discriminator=random.randint(0, 4095),
            )
        # This is a packed error code, so we get the lowest 8 bits to check the cluster status code
        cluster_status = cm.exception.err & 0xFF
        asserts.assert_equal(
            cluster_status,
            Clusters.JointFabricAdministrator.Enums.StatusCodeEnum.kInvalidAdministratorFabricIndex,
            f'Expected InvalidAdministratorFabricIndex status code (0x06), but got 0x{(cluster_status):02x} ({str(cm.exception)})',
        )

    async def _arm_failsafe(self, expiry_length_seconds):
        """
        Sends the ArmFailSafe command to the General Commissioning Cluster of the DUT, with breadcrumb=1 and expiryLengthSeconds
        passed in as an argument.
        """
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self._GENERAL_COMMISSIONING_ENDPOINT,
            cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=expiry_length_seconds, breadcrumb=1))
        asserts.assert_true(
            isinstance(resp, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
            f"Unexpected response type: {type(resp)}",
        )
        asserts.assert_equal(
            resp.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            f"ArmFailSafeResponse error code not OK: {resp.errorCode}",
        )

    async def _expire_failsafe(self):
        """
        From the spec on ArmFailSafe (11.10.7.2):
        > If ExpiryLengthSeconds is 0 and the fail-safe timer was already armed and the accessing fabric matches the Fabric
        > currently associated with the fail-safe context, then the fail-safe timer SHALL be immediately expired (see further below
        > for side-effects of expiration).
        """
        await self._arm_failsafe(0)

    async def _assert_im_error(self, cmd, expected_status, expected_cluster_status=None, error_label=None):
        expected_error = error_label if error_label is not None else expected_status
        with asserts.assert_raises(InteractionModelError, f"Expected InteractionModelError with {expected_error}, but no exception occurred.") as cm:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self._JOINT_FABRIC_ADMINISTRATOR_ENDPOINT,
                cmd=cmd)
        asserts.assert_equal(
            cm.exception.status,
            expected_status,
            f"Expected {expected_status} status, but got {str(cm.exception)}",
        )
        if expected_cluster_status is not None:
            asserts.assert_equal(
                cm.exception.clusterStatus,
                expected_cluster_status,
                f"Expected {expected_cluster_status} cluster status, but got {str(cm.exception)}",
            )


if __name__ == "__main__":
    default_matter_test_main()
