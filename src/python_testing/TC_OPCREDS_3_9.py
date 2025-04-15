#
#    Copyright (c) 2025 Project CHIP Authors
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from chip.interaction_model import InteractionModelError, Status
from chip.tlv import TLVReader
from chip.utils import CommissioningBuildingBlocks
import enum
import hashlib
from binascii import unhexlify
from ecdsa import NIST256p, VerifyingKey
from ecdsa.keys import BadSignatureError
from typing import Optional
from mobly import asserts
import logging


class MatterCertParser:
    SUBJECT_TAG = 6
    SUBJECT_PUBLIC_KEY_TAG = 9
    SUBJECT_FABRIC_ID_TAG = 21

    def __init__(self, matter_cert_bytes: bytes):
        self.parsed_tlv = TLVReader(matter_cert_bytes).get()["Any"]

    def get_subject_names(self) -> dict[int, object]:
        return {tag: value for tag, value in self.parsed_tlv[self.SUBJECT_TAG]}

    def get_public_key_bytes(self) -> bytes:
        public_key_bytes = self.parsed_tlv[self.SUBJECT_PUBLIC_KEY_TAG]
        return public_key_bytes


# From Matter spec src/crypto_primitives/crypto_primitives.py
class MappingsV1(enum.IntEnum):
    CHIP_CRYPTO_HASH_LEN_BITS = 256
    CHIP_CRYPTO_HASH_LEN_BYTES = 32
    CHIP_CRYPTO_HASH_BLOCK_LEN_BYTES = 64
    CHIP_CRYPTO_GROUP_SIZE_BITS = 256
    CHIP_CRYPTO_GROUP_SIZE_BYTES = 32
    CHIP_CRYPTO_PUBLIC_KEY_SIZE_BYTES = (2 * CHIP_CRYPTO_GROUP_SIZE_BYTES) + 1
    CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BITS = 128
    CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES = 16
    CHIP_CRYPTO_AEAD_MIC_LENGTH_BITS = 128
    CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES = 16
    CHIP_CRYPTO_AEAD_NONCE_LENGTH_BYTES = 13


# From Matter spec src/crypto_primitives/crypto_primitives.py
def bytes_from_hex(hex: str) -> bytes:
    """Converts any `hex` string representation including `01:ab:cd` to bytes

    Handles any whitespace including newlines, which are all stripped.
    """
    return unhexlify("".join(hex.replace(":", "").split()))


# From Matter spec src/crypto_primitives/vid_verify_payload_test_vector.py
# ECDSA-with-SHA256 using NIST P-256
FABRIC_BINDING_VERSION_1 = 0x01
STATEMENT_VERSION_1 = 0x21
SIGNATURE_SIZE = (2 * MappingsV1.CHIP_CRYPTO_GROUP_SIZE_BYTES)
SKID_SIZE = 20
VID_VERIFICATION_CLIENT_CHALLENGE_SIZE_BYTES = 32
ATTESTATION_CHALLENGE_SIZE_BYTES = MappingsV1.CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES


def verify_signature(public_key: bytes, message: bytes, signature: bytes) -> bool:
    """Verify a `signature` on the given `message` using `public_key`. Returns True on success."""

    verifying_key: VerifyingKey = VerifyingKey.from_string(public_key, curve=NIST256p)
    assert verifying_key.curve == NIST256p

    try:
        return verifying_key.verify(signature, message, hashfunc=hashlib.sha256)
    except BadSignatureError:
        return False


# From Matter spec src/crypto_primitives/vid_verify_payload_test_vector.py
def generate_vendor_fabric_binding_message(
        root_public_key_bytes: bytes,
        fabric_id: int,
        vendor_id: int) -> bytes:

    assert len(root_public_key_bytes) == MappingsV1.CHIP_CRYPTO_PUBLIC_KEY_SIZE_BYTES
    assert fabric_id > 0 and fabric_id <= 0xFFFF_FFFF_FFFF_FFFF
    assert vendor_id > 0 and vendor_id <= 0xFFF4

    fabric_id_bytes = fabric_id.to_bytes(length=8, byteorder='big')
    vendor_id_bytes = vendor_id.to_bytes(length=2, byteorder='big')
    vendor_fabric_binding_message = FABRIC_BINDING_VERSION_1.to_bytes(
        length=1) + root_public_key_bytes + fabric_id_bytes + vendor_id_bytes
    return vendor_fabric_binding_message

# From Matter spec src/crypto_primitives/vid_verify_payload_test_vector.py


def generate_vendor_id_verification_tbs(fabric_binding_version: int,
                                        attestation_challenge: bytes,
                                        client_challenge: bytes,
                                        fabric_index: int,
                                        vendor_fabric_binding_message: bytes,
                                        vid_verification_statement: Optional[bytes] = None) -> bytes:
    assert len(attestation_challenge) == ATTESTATION_CHALLENGE_SIZE_BYTES
    assert len(client_challenge) == VID_VERIFICATION_CLIENT_CHALLENGE_SIZE_BYTES
    assert fabric_index > 0 and fabric_index < 255
    assert vendor_fabric_binding_message
    assert fabric_binding_version == FABRIC_BINDING_VERSION_1

    vendor_id_verification_tbs = fabric_binding_version.to_bytes(length=1)
    vendor_id_verification_tbs += client_challenge + attestation_challenge + fabric_index.to_bytes(length=1)
    vendor_id_verification_tbs += vendor_fabric_binding_message
    if vid_verification_statement is not None:
        vendor_id_verification_tbs += vid_verification_statement

    return vendor_id_verification_tbs


def get_unassigned_fabric_index(fabric_indices: list[int]) -> int:
    for fabric_index in range(1, 255):
        if fabric_index not in fabric_indices:
            return fabric_index
    else:
        asserts.fail(f"Somehow could not find an unallocated fabric index in {fabric_indices}")


class TC_OPCREDS_3_9(MatterBaseTest):
    def desc_TC_OPCREDS_3_9(self):
        return "[DUTServer] Pre-release TC-OPCREDS-3.9 test case."

    @async_test_body
    async def test_TC_OPCREDS_3_9(self):
        # TODO(test_plans#5046): actually make the test follow final test plan. For now
        # it functionally validates the VID Verification parts of Operational Credentials Cluster

        opcreds = Clusters.OperationalCredentials
        dev_ctrl = self.default_controller

        self.print_step(0, "Commission DUT in TH1's fabric.")

        # Read fabric index for CR1 after commissioning it.
        cr1_fabric_index = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=opcreds,
            attribute=opcreds.Attributes.CurrentFabricIndex
        )

        root_certs = await self.read_single_attribute_check_success(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            cluster=opcreds,
            attribute=opcreds.Attributes.TrustedRootCertificates,
            fabric_filtered=True
        )
        asserts.assert_true(len(root_certs) == 1, "Could not read TH1's root cert from TrustedRootCertificates")
        th1_root_parser = MatterCertParser(root_certs[0])
        cr1_root_public_key = th1_root_parser.get_public_key_bytes()

        self.print_step(1, "Commission DUT in TH2's fabric.")

        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        cr2_vid = 0xFFF2
        cr2_fabricId = 2222
        cr2_new_fabric_admin = new_certificate_authority.NewFabricAdmin(
            vendorId=cr2_vid, fabricId=cr2_fabricId)
        cr2_nodeid = dev_ctrl.nodeId + 1
        cr2_dut_node_id = self.dut_node_id + 1

        cr2_new_admin_ctrl = cr2_new_fabric_admin.NewController(
            nodeId=cr2_nodeid)
        success, nocResp, chain = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
            commissionerDevCtrl=dev_ctrl, newFabricDevCtrl=cr2_new_admin_ctrl,
            existingNodeId=self.dut_node_id, newNodeId=cr2_dut_node_id
        )
        asserts.assert_true(success, "Commissioning DUT into CR2's fabrics must succeed.")

        rcacResp = chain.rcacBytes

        cr2_fabric_index = nocResp.fabricIndex
        cr2_rcac = MatterCertParser(rcacResp)
        cr2_root_public_key = cr2_rcac.get_public_key_bytes()

        # Read NOCs and validate that both the entry for CR1 and CR2 are readable
        # and have the right expected fabricId
        self.print_step(2, "Read DUT's NOCs attribute and validate both fabrics have expected values extractable from their NOC.")
        nocs_list = await self.read_single_attribute_check_success(
            cluster=opcreds,
            attribute=opcreds.Attributes.NOCs,
            fabric_filtered=False
        )

        fabric_ids_from_certs = {}
        noc_public_keys_from_certs = {}

        fabric_indices = {
            "CR1": cr1_fabric_index,
            "CR2": cr2_fabric_index
        }

        fabric_ids = {
            "CR1": self.default_controller.fabricId,
            "CR2": cr2_fabricId
        }

        for controller_name, fabric_index in fabric_indices.items():
            for noc_struct in nocs_list:
                if noc_struct.fabricIndex != fabric_index:
                    continue

                noc_cert = MatterCertParser(noc_struct.noc)
                for tag, value in noc_cert.get_subject_names().items():
                    if tag == noc_cert.SUBJECT_FABRIC_ID_TAG:
                        fabric_ids_from_certs[controller_name] = value
                noc_public_keys_from_certs[controller_name] = noc_cert.get_public_key_bytes()

        logging.info(f"Fabric IDs found: {fabric_ids_from_certs}")

        for controller_name, fabric_id in fabric_ids.items():
            asserts.assert_in(controller_name, fabric_ids_from_certs.keys(),
                              f"Did not find {controller_name}'s fabric the NOCs attribute")
            asserts.assert_equal(fabric_ids_from_certs[controller_name], fabric_id,
                                 f"Did not find {controller_name}'s fabric ID in the correct NOC")

        self.print_step(3, "Read DUT's Fabrics attribute and validate both fabrics have expected values.")

        fabric_roots = {
            "CR1": cr1_root_public_key,
            "CR2": cr2_root_public_key
        }

        fabric_vendor_ids = {
            "CR1": 0xfff1,
            "CR2": cr2_vid
        }

        fabrics_list = await self.read_single_attribute_check_success(
            cluster=opcreds,
            attribute=opcreds.Attributes.Fabrics,
            fabric_filtered=False
        )

        for controller_name, fabric_index in fabric_indices.items():
            for fabric_struct in fabrics_list:
                if fabric_struct.fabricIndex != fabric_index:
                    continue

                asserts.assert_equal(fabric_struct.rootPublicKey, fabric_roots[controller_name],
                                     f"Did not find matching root public key in Fabrics attribute for {controller_name}")
                asserts.assert_equal(
                    fabric_struct.vendorID, fabric_vendor_ids[controller_name], f"Did not find matching VendorID in Fabrics attribute for {controller_name}")
                asserts.assert_equal(
                    fabric_struct.fabricID, fabric_ids[controller_name], f"Did not find matching FabricID in Fabrics attribute for {controller_name}")

        self.print_step(4, "TH1 sends SignVIDVerificationRequest for TH2's fabric. Verify the response and signature.")

        client_challenge = bytes_from_hex(
            "a1:a2:a3:a4:a5:a6:a7:a8:a9:aa:ab:ac:ad:ae:af:b0:b1:b2:b3:b4:b5:b6:b7:b8:b9:ba:bb:bc:bd:be:bf:c0")
        sign_vid_verification_response = await self.send_single_cmd(cmd=opcreds.Commands.SignVIDVerificationRequest(fabricIndex=cr2_fabric_index, clientChallenge=client_challenge))

        asserts.assert_equal(sign_vid_verification_response.fabricIndex, cr2_fabric_index,
                             "FabricIndex in SignVIDVerificationResponse must match request.")

        # Locally generate the vendor_id_verification_tbs to check the signature.
        expected_vendor_fabric_binding_message = generate_vendor_fabric_binding_message(
            root_public_key_bytes=cr2_root_public_key, fabric_id=cr2_fabricId, vendor_id=cr2_vid)
        attestation_challenge = dev_ctrl.GetConnectedDeviceSync(self.dut_node_id, allowPASE=False).attestationChallenge
        vendor_id_verification_tbs = generate_vendor_id_verification_tbs(sign_vid_verification_response.fabricBindingVersion, attestation_challenge,
                                                                         client_challenge, sign_vid_verification_response.fabricIndex, expected_vendor_fabric_binding_message, vid_verification_statement=b"")

        # Check signature against vendor_id_verification_tbs
        noc_cert = MatterCertParser(noc_struct.noc)
        asserts.assert_true(verify_signature(public_key=noc_public_keys_from_certs["CR2"], message=vendor_id_verification_tbs,
                            signature=sign_vid_verification_response.signature), "VID Verification Signature must validate using DUT's NOC public key")

        self.print_step(5, "Send bad SignVIDVerificationRequest commands and verify failures")

        # Must fail with correct client challenge but non-existent fabric.
        unassigned_fabric_index = get_unassigned_fabric_index(fabric_indices.values())
        with asserts.assert_raises(InteractionModelError) as exception_context:
            await self.send_single_cmd(cmd=opcreds.Commands.SignVIDVerificationRequest(fabricIndex=unassigned_fabric_index, clientChallenge=client_challenge))

        asserts.assert_equal(exception_context.exception.status, Status.ConstraintError,
                             f"Expected CONSTRAINT_ERROR from SignVIDVerificationRequest against fabricIndex {unassigned_fabric_index}")

        # Must fail with correct client challenge but fabricIndex 0.
        with asserts.assert_raises(InteractionModelError) as exception_context:
            await self.send_single_cmd(cmd=opcreds.Commands.SignVIDVerificationRequest(fabricIndex=0, clientChallenge=client_challenge))

        asserts.assert_equal(exception_context.exception.status, Status.ConstraintError,
                             f"Expected CONSTRAINT_ERROR from SignVIDVerificationRequest against fabricIndex 0")

        # Must fail with client challenge different than expected length
        CHALLENGE_TOO_SMALL = b"\x01" * (VID_VERIFICATION_CLIENT_CHALLENGE_SIZE_BYTES - 1)
        CHALLENGE_TOO_BIG = b"\x02" * (VID_VERIFICATION_CLIENT_CHALLENGE_SIZE_BYTES + 1)

        with asserts.assert_raises(InteractionModelError) as exception_context:
            await self.send_single_cmd(cmd=opcreds.Commands.SignVIDVerificationRequest(fabricIndex=cr2_fabric_index, clientChallenge=CHALLENGE_TOO_SMALL))

        asserts.assert_equal(exception_context.exception.status, Status.ConstraintError,
                             f"Expected CONSTRAINT_ERROR from SignVIDVerificationRequest with ClientChallenge of length {len(CHALLENGE_TOO_SMALL)}")

        with asserts.assert_raises(InteractionModelError) as exception_context:
            await self.send_single_cmd(cmd=opcreds.Commands.SignVIDVerificationRequest(fabricIndex=cr2_fabric_index, clientChallenge=CHALLENGE_TOO_BIG))

        asserts.assert_equal(exception_context.exception.status, Status.ConstraintError,
                             f"Expected CONSTRAINT_ERROR from SignVIDVerificationRequest with ClientChallenge of length {len(CHALLENGE_TOO_BIG)}")

        self.print_step(6, "Remove TH2's fabric")
        cmd = opcreds.Commands.RemoveFabric(cr2_fabric_index)
        resp = await self.send_single_cmd(cmd=cmd)
        asserts.assert_equal(
            resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kOk)


if __name__ == "__main__":
    default_matter_test_main()
