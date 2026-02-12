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
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import os
import random
import tempfile

from mobly import asserts

import matter.clusters as Clusters
import matter.discovery as Discovery
from matter.interaction_model import InteractionModelError, Status
from matter.testing.apps import JFAdministratorSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main
from matter.tlv import TLVReader, TLVWriter

log = logging.getLogger(__name__)


class TC_JFPKI_2_2(MatterBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()

        self.storage_fabric_a = self.user_params.get("fabric_a_storage", None)

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

        #####################################################################################################################################
        #
        # Initialize Ecosystem A
        #
        #####################################################################################################################################
        self.jfadmin_fabric_a_passcode = random.randint(20202021, 20202099)
        self.jfadmin_fabric_a_discriminator = random.randint(0, 4095)

        # Start Fabric A JF-Administrator App
        self.fabric_a_admin = JFAdministratorSubprocess(
            self.jfa_server_app,
            "JFA_A",
            storage_dir=self.storage_fabric_a,
            port=random.randint(5001, 5999),
            discriminator=self.jfadmin_fabric_a_discriminator,
            passcode=self.jfadmin_fabric_a_passcode,
            extra_args=["--capabilities", "0x04", "--rpc-server-port", "33033"])
        self.fabric_a_admin.start(
            expected_output="Server initialization complete",
            timeout=10)


    def teardown_class(self):
        # Stop all Subprocesses that were started in this test case
        if self.fabric_a_admin is not None:
            self.fabric_a_admin.terminate()

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

    @async_test_body
    async def test_TC_JFPKI_2_2(self):
        self.step("1")

        await self.default_controller.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=self.jfadmin_fabric_a_passcode,
            filterType=Discovery.FilterType.LONG_DISCRIMINATOR,
            filter=self.jfadmin_fabric_a_discriminator,
        )

        self.step("2")
        try:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=1,
                cmd=Clusters.JointFabricAdministrator.Commands.ICACCSRRequest())
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.FailsafeRequired,
                f'Expected FailsafeRequired error, but got {str(e)}',
            )
        else:
            asserts.assert_true(False, 'Expected InteractionModelError with FailsafeRequired, but no exception occurred.')

        # TODO: "TH sends ArmFailSafe command to DUT with ExpiryLengthSeconds set to 10 and Breadcrumb 1.", "DUT respond with ArmFailSafeResponse Command."
        self.step("3")
        expiry_failsafe_step_3 = 10
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=0,
            cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=expiry_failsafe_step_3, breadcrumb=1))
        asserts.assert_true(
            isinstance(resp, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
            f"Unexpected response type: {type(resp)}",
        )
        asserts.assert_equal(
            resp.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            f"ArmFailSafeResponse error code not OK: {resp.errorCode}",
        )

        self.step("4")
        try:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=1,
                cmd=Clusters.JointFabricAdministrator.Commands.ICACCSRRequest())
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.Failure,
                f'Expected Failure status for VIDNotVerified, but got {str(e)}',
            )
            asserts.assert_equal(
                e.clusterStatus,
                Clusters.JointFabricAdministrator.Enums.StatusCodeEnum.kVIDNotVerified,
                f'Expected VIDNotVerified error, but got {str(e)}',
            )
        else:
            asserts.assert_true(False, 'Expected InteractionModelError with VIDNotVerified, but no exception occurred.')

        self.step("5")
        response = await self.default_controller.ReadAttribute(
            nodeId=self.dut_node_id,
            attributes=[(0, Clusters.OperationalCredentials.Attributes.NOCs)],
            returnClusterObject=True)
        icac1 = response[0][Clusters.OperationalCredentials].NOCs[0].icac
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=1,
            cmd=Clusters.JointFabricAdministrator.Commands.AddICAC(icac1))

        self.step("6")
        try:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=1,
                cmd=Clusters.JointFabricAdministrator.Commands.ICACCSRRequest())
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                f'Expected ConstraintError status, but got {str(e)}',
            )
        else:
            asserts.assert_true(False, 'Expected InteractionModelError with ConstraintError, but no exception occurred.')

        # # Get the ICAC from JF-Admin
        # response = await self.default_controller.ReadAttribute(
        #     nodeId=self.dut_node_id, attributes=[(0, Clusters.OperationalCredentials.Attributes.NOCs)],
        #     returnClusterObject=True)
        # _icac = response[0][Clusters.OperationalCredentials].NOCs[0].icac
        # cmd = Clusters.JointFabricAdministrator.Commands.AddICAC(_icac)
        # try:
        #     await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd, endpoint=1)
        # except InteractionModelError as e:
        #     asserts.assert_equal(
        #         e.status,
        #         Status.FailsafeRequired,
        #         f'Expected FailsafeRequired error, but got {str(e)}',
        #     )
        # else:
        #     asserts.assert_true(False, 'Expected InteractionModelError with FailsafeRequired, but no exception occurred.')

        self.step("7")
        # Wait for ArmFailSafe timer to expire
        await asyncio.sleep(expiry_failsafe_step_3 + 1)
        # await self.send_single_cmd(
        #     dev_ctrl=self.default_controller,
        #     node_id=self.dut_node_id,
        #     cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=60, breadcrumb=1))

        self.step("8")
        try:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=1,
                cmd=Clusters.JointFabricAdministrator.Commands.AddICAC(icac1))
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.FailsafeRequired,
                f'Expected FailsafeRequired error, but got {str(e)}',
            )
        else:
            asserts.assert_true(False, 'Expected InteractionModelError with FailsafeRequired, but no exception occurred.')
        # cmd = Clusters.OperationalCredentials.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=True)
        # csr_update = await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd)
        # new_noc_chain = await self.default_controller.IssueNOCChain(csr_update, self.dut_node_id)
        # cmd = Clusters.JointFabricAdministrator.Commands.AnnounceJointFabricAdministrator(1)
        # await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd, endpoint=1)
        # cmd = Clusters.JointFabricAdministrator.Commands.ICACCSRRequest()
        # await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd, endpoint=1)
        # cmd = Clusters.JointFabricAdministrator.Commands.AddICAC(new_noc_chain.icacBytes)
        # try:
        #    await self.send_single_cmd(dev_ctrl=self.default_controller, node_id=self.dut_node_id, cmd=cmd, endpoint=1)
        # except InteractionModelError as e:
        #     asserts.assert_in('InvalidICAC (0x02)',
        #                       str(e), f'Expected InvalidICAC error, but got {str(e)}')
        # else:
        #     asserts.assert_true(False, 'Expected InteractionModelError with InvalidICAC, but no exception occurred.')
        self.step("9")

        expiry_failsafe_step_9 = 20

        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=0,
            cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=expiry_failsafe_step_9, breadcrumb=1))
        asserts.assert_true(
            isinstance(resp, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
            f"Unexpected response type: {type(resp)}",
        )
        asserts.assert_equal(
            resp.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            f"ArmFailSafeResponse error code not OK: {resp.errorCode}",
        )

        self.step("10")
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=1,
            cmd=Clusters.JointFabricAdministrator.Commands.AddICAC(icac1))

        self.step("11")
        try:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=1,
                cmd=Clusters.JointFabricAdministrator.Commands.AddICAC(icac1))
        except InteractionModelError as e:
            asserts.assert_equal(
                e.status,
                Status.ConstraintError,
                f'Expected ConstraintError status, but got {str(e)}',
            )
        else:
            asserts.assert_true(False, 'Expected InteractionModelError with ConstraintError, but no exception occurred.')

        self.step("12")
        # Wait for ArmFailSafe timer to expire
        await asyncio.sleep(expiry_failsafe_step_9 + 1)

        self.step("13")
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=0,
            cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=20, breadcrumb=1))
        asserts.assert_true(
            isinstance(resp, Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse),
            f"Unexpected response type: {type(resp)}",
        )
        asserts.assert_equal(
            resp.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            f"ArmFailSafeResponse error code not OK: {resp.errorCode}",
        )

        self.step("14")
        # Create a second CA to generate an ICAC that is valid but chained to a different RCAC than the TH controller fabric RCAC.
        csr_nonce = random.randbytes(32)
        csr_response = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=0,
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
            endpoint=1,
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
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=0,
            cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=1))
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=0,
            cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=20, breadcrumb=1))

        # Generate a new ICAC using the old CSR Response, which will have the wrong public key for this fail-safe context.
        icac_wrong_public_key = await self.default_controller.IssueNOCChain(csr_response, self.dut_node_id)

        # Send AddICAC with the ICAC containing the wrong public key
        resp = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=1,
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
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=0,
            cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=1))
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=0,
            cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=20, breadcrumb=1))

        # Create a valid ICAC for the current fail-safe context.
        csr_nonce = random.randbytes(32)
        csr_response_step16 = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=0,
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
            endpoint=1,
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
        # await self.send_single_cmd(
        #     dev_ctrl=self.default_controller,
        #     node_id=self.dut_node_id,
        #     endpoint=0,
        #     cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=1))

        # try:
        #     await self.default_controller.OpenJointCommissioningWindow(
        #         nodeId=self.dut_node_id,
        #         endpointId=1,
        #         timeout=400,
        #         iteration=random.randint(1000, 100000),
        #         discriminator=random.randint(0, 4095),
        #     )
        # except ChipStackError as e:  # chipstack-ok: OpenJointCommissioningWindow surfaces IM errors as ChipStackError
        #     asserts.assert_equal(
        #         e.err & 0xFF,
        #         Clusters.JointFabricAdministrator.Enums.StatusCodeEnum.kInvalidAdministratorFabricIndex,
        #         f'Expected InvalidAdministratorFabricIndex status code (0x06), but got 0x{(e.err & 0xFF):02x} ({str(e)})',
        #     )
        # else:
        #     asserts.assert_true(
        #         False,
        #         'Expected ChipStackError with InvalidAdministratorFabricIndex, but no exception occurred.',
        #     )

    async def arm_failsafe(self, expiry_length_seconds):
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=0,
            cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=expiry_length_seconds, breadcrumb=1))

    async def expire_failsafe(self):
        await self.arm_failsafe(0)


if __name__ == "__main__":
    default_matter_test_main()
