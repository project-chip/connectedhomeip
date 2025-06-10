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
#       --endpoint 0
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import enum
import hashlib
import inspect
import logging
import re
import sys
from binascii import hexlify, unhexlify
from typing import Optional

import chip.clusters as Clusters
import nest_asyncio
from chip.interaction_model import InteractionModelError, Status
from chip.testing.matter_testing import (AttributeMatcher, AttributeValue, ClusterAttributeChangeAccumulator, MatterBaseTest,
                                         TestStep, default_matter_test_main, has_command, run_if_endpoint_matches)
from chip.testing.pics import accepted_cmd_pics_str
from chip.tlv import TLVReader
from chip.utils import CommissioningBuildingBlocks
from ecdsa import NIST256p, VerifyingKey
from ecdsa.keys import BadSignatureError
from mobly import asserts

nest_asyncio.apply()


def to_octet_string(input: bytes) -> str:
    """Takes `input` bytes and convert to a colon-separated hex octet string representation."""
    return ":".join(["%02x" % b for b in input])


class MatterCertParser:
    SUBJECT_TAG = 6
    SUBJECT_PUBLIC_KEY_TAG = 9
    SUBJECT_FABRIC_ID_TAG = 21

    def __init__(self, matter_cert_bytes: bytes):
        try:
            full_parsed_tlv = TLVReader(matter_cert_bytes).get()
        except (ValueError, TypeError, KeyError, IndexError) as e:
            raise ValueError(f"Failed to parse Matter TLV cert: {str(e)}. TLV cert bytes: {hexlify(matter_cert_bytes)}")

        if (not isinstance(full_parsed_tlv, dict)) or ("Any" not in full_parsed_tlv) or (not isinstance(full_parsed_tlv['Any'], dict)):
            raise ValueError(f"Failed to find top-level anonymous struct in Matter TLV cert: (TLV: {full_parsed_tlv}).")

        self.parsed_tlv = full_parsed_tlv["Any"]

    def get_subject_names(self) -> dict[int, object]:
        if self.SUBJECT_TAG not in self.parsed_tlv:
            raise ValueError(f"Did not find Subject tag in Matter TLV certificate: {self.parsed_tlv}")
        return {tag: value for tag, value in self.parsed_tlv[self.SUBJECT_TAG]}

    def get_public_key_bytes(self) -> bytes:
        if self.SUBJECT_PUBLIC_KEY_TAG not in self.parsed_tlv:
            raise ValueError(f"Did not find Subject Public Key tag in Matter TLV certificate: {self.parsed_tlv}")

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
    return unhexlify(re.sub(r'(\s|:)', "", hex))


# From Matter spec src/crypto_primitives/vid_verify_payload_test_vector.py
# ECDSA-with-SHA256 using NIST P-256
FABRIC_BINDING_VERSION_1 = 0x01
STATEMENT_VERSION_1 = 0x21
VID_VERIFICATION_CLIENT_CHALLENGE_SIZE_BYTES = 32
ATTESTATION_CHALLENGE_SIZE_BYTES = MappingsV1.CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES
VID_VERIFICATION_STATEMENT_SIZE_BYTES_V1 = 85


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
    # Valid fabric indices are [1..254]. 255 is forbidden.
    assert fabric_index >= 1 and fabric_index <= 254
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


def get_entry_for_fabric(fabric_index: int, entries: list[object]) -> object:
    for entry in entries:
        if not hasattr(entry, "fabricIndex"):
            return None
        if entry.fabricIndex == fabric_index:
            return entry
    else:
        return None


def make_vid_matcher(fabric_index: int, expected_vid: int) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.OperationalCredentials.Attributes.Fabrics or report.endpoint_id != 0 or not isinstance(report.value, list):
            return False
        for entry in report.value:
            if entry.fabricIndex == fabric_index and entry.vendorID == expected_vid:
                return True
        else:
            return False
    return AttributeMatcher.from_callable(description=f"Fabrics list entry report for FabricIndex {fabric_index} has VendorID field set to 0x{expected_vid:04x}", matcher=predicate)


def make_vvs_matcher(fabric_index: int, expected_vvs: bytes) -> AttributeMatcher:
    def predicate(report: AttributeValue) -> bool:
        if report.attribute != Clusters.OperationalCredentials.Attributes.Fabrics or report.endpoint_id != 0 or not isinstance(report.value, list):
            return False
        for entry in report.value:
            if entry.fabricIndex == fabric_index and entry.VIDVerificationStatement == expected_vvs:
                return True
        else:
            return False
    return AttributeMatcher.from_callable(description=f"Fabrics list entry report for FabricIndex {fabric_index} has VIDVerificationStatement field set to correct VIDVerificationStatement value just set", matcher=predicate)


class TestStepBlockPassException(Exception):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
    pass


class test_step(object):
    """Context manager for `with test_tesp(...) as step` that allows for aggregating step descriptions automatically.

    Use like:

      with test_step("1b", "Read attribute Blah from cluster Gizmo", "Verify that attribute value is set") as step:
          # NOTE: ===== THIS CANNOT BE A `pass` ====
          do_some_work_here()

    This expects a MatterBaseTest instance to be the caller, since it interacts with pieces
    that are in there for the purposes of accumulating the steps list.

    If the calling `MatterTestInstance`'s `is_aggregating_steps` field is set to True, the block
    body WILL BE SKIPPED.

    When the calling `MatterTestInstance`'s `is_aggregating_steps` field is set to False, the
    block is executed after an automatic call to `step(...)` method.

    TODO: Port back to matter_testing.py once this whole test suite is complete.
    """

    def __init__(self, id=None, description="", verification=None, is_commissioning=False):
        caller = inspect.currentframe().f_back.f_locals.get('self', None)
        if isinstance(caller, MatterBaseTest):
            self._test_instance = caller
        else:
            raise RuntimeError("Can only use `test_step` inside a MatterBaseTest-derived class")

        if id is None:
            id = self._test_instance.current_step_id
            next_step_id = self._test_instance.get_next_step_id(self._test_instance.current_step_id)
            self._test_instance.current_step_id = next_step_id
        else:
            self._test_instance.current_step_id = id

        self._id = id
        self._description = description
        self._verification = verification
        self._is_commissioning = is_commissioning

    def trace(self, frame, event, arg):
        raise TestStepBlockPassException("If you see this, you have a `pass` in a `with test_step(...):` block!")

    def __enter__(self):
        if self._test_instance.is_aggregating_steps:
            self._test_instance.aggregated_steps.append(
                TestStep(self._id, self._description, self._verification, self._is_commissioning))

            # Let's manipulate the callstack via tracing to skip the entire `with` block in steps extraction mode.
            sys.settrace(lambda *args, **keys: None)
            frame = sys._getframe(1)
            frame.f_trace = self.trace

            # NO MORE CALLS CAN BE PAST HERE IN __enter__(). Otherwise, the "skip block" feature doesn't work.
        else:
            self._test_instance.step(self._id)

        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if type is None:
            return  # No exception
        if isinstance(exc_value, TestStepBlockPassException):
            return True  # Suppress special exception we expect to see.

    @property
    def id(self):
        return self._id

    def skip(self):
        self._test_instance.mark_current_step_skipped()


class TC_OPCREDS_VidVerify(MatterBaseTest):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.is_aggregating_steps: bool = False
        self.aggregated_steps: list[TestStep] = []
        self.current_step_id = 0
        self.reset_steps()

    def reset_steps(self):
        self.aggregated_steps = []
        self.is_aggregating_steps = False
        self.current_step_id = 0

    def get_next_step_id(self, current_step_id) -> object:
        if isinstance(current_step_id, int):
            return current_step_id + 1
        elif isinstance(current_step_id, str):
            match = re.search(r"^(?P<step_number>\d+)", current_step_id)
            if match:
                return int(match.group('step_number')) + 1

        raise ValueError(f"Can't find correct next step for {current_step_id}")

    async def read_updated_fabrics(self, dev_ctrl, node_id):
        updated_fabrics = await self.read_single_attribute_check_success(
            dev_ctrl=dev_ctrl,
            node_id=node_id,
            cluster=Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.Fabrics,
            fabric_filtered=False
        )

        updated_nocs = await self.read_single_attribute_check_success(
            dev_ctrl=dev_ctrl,
            node_id=node_id,
            cluster=Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.NOCs,
            fabric_filtered=False
        )

        return updated_fabrics, updated_nocs

    def desc_TC_OPCREDS_3_8(self):
        return "[DUTServer] TC-OPCREDS-3.8 test case."

    def steps_TC_OPCREDS_3_8(self) -> list[TestStep]:
        try:
            self.current_step_id = 0
            self.is_aggregating_steps = True
            self.aggregated_steps = []
            self.test_TC_OPCREDS_3_8()
        finally:
            self.is_aggregating_steps = False

        self.current_step_id = 0
        return self.aggregated_steps

    def pics_TC_OPCREDS_3_8(self) -> list[str]:
        return [accepted_cmd_pics_str('OPCREDS', Clusters.OperationalCredentials.Commands.SetVIDVerificationStatement.command_id)]

    @run_if_endpoint_matches(has_command(Clusters.OperationalCredentials.Commands.SetVIDVerificationStatement))
    async def test_TC_OPCREDS_3_8(self):
        # TODO(test_plans#5046): actually make the test follow final test plan. For now
        # it functionally validates the VID Verification parts of Operational Credentials Cluster

        with test_step(0, description="Commission DUT in TH1's fabric. Cert chain must include ICAC.", is_commissioning=True):
            # Commissioning with ICAC was implicit due to the commissioning method passed on command line.

            opcreds = Clusters.OperationalCredentials
            dev_ctrl = self.default_controller
            th1_dev_ctrl = self.default_controller
            th1_dut_node_id = self.dut_node_id

            # Read fabric index for TH1 after commissioning it.
            th1_fabric_index = await self.read_single_attribute_check_success(
                dev_ctrl=th1_dev_ctrl,
                node_id=th1_dut_node_id,
                cluster=opcreds,
                attribute=opcreds.Attributes.CurrentFabricIndex
            )

            root_certs = await self.read_single_attribute_check_success(
                dev_ctrl=th1_dev_ctrl,
                node_id=th1_dut_node_id,
                cluster=opcreds,
                attribute=opcreds.Attributes.TrustedRootCertificates,
                fabric_filtered=True
            )
            asserts.assert_true(len(root_certs) == 1,
                                f"Expecting exactly one root from TrustedRootCertificates (TH1's), got {len(root_certs)}")

            logging.info("Parsing root certificate for TH1's fabric")
            try:
                th1_root_parser = MatterCertParser(root_certs[0])
                th1_root_public_key = th1_root_parser.get_public_key_bytes()
            except (ValueError, IndexError, KeyError, TypeError) as e:
                asserts.fail(f"Failed to parse root certificate for TH1's fabric: {str(e)}")
            logging.info("Parsed TH1's RCAC successfully.")
            logging.info(f"  -> Root public key bytes: {to_octet_string(th1_root_public_key)}")

        with test_step(1, description="Commission DUT in TH2's fabric. Cert chain must NOT include ICAC"):
            new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
            new_certificate_authority.alwaysOmitIcac = True
            th2_vid = 0xFFF2
            th2_fabricId = 0x2222
            th2_new_fabric_admin = new_certificate_authority.NewFabricAdmin(
                vendorId=th2_vid, fabricId=th2_fabricId)
            th2_nodeid = th1_dev_ctrl.nodeId + 1
            th2_dut_node_id = th1_dut_node_id + 1

            th2_dev_ctrl = th2_new_fabric_admin.NewController(
                nodeId=th2_nodeid)
            success, nocResp, chain = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
                commissionerDevCtrl=th1_dev_ctrl, newFabricDevCtrl=th2_dev_ctrl,
                existingNodeId=th1_dut_node_id, newNodeId=th2_dut_node_id
            )
            asserts.assert_true(success, "Commissioning DUT into TH2's fabrics must succeed.")

            rcacResp = chain.rcacBytes

            th2_fabric_index = nocResp.fabricIndex

            logging.info("Parsing root certificate for TH2's fabric")
            try:
                th2_rcac = MatterCertParser(rcacResp)
                th2_root_public_key = th2_rcac.get_public_key_bytes()
            except (ValueError, IndexError, KeyError, TypeError) as e:
                asserts.fail(f"Failed to parse root certificate for TH2's fabric: {str(e)}")
            logging.info("Parsed TH2's RCAC successfully.")
            logging.info(f"  -> Root public key bytes: {to_octet_string(th2_root_public_key)}")

        # Read NOCs and validate that both the entry for TH1 and TH2 are readable
        # and have the right expected fabricId
        with test_step(2, description="Read DUT's NOCs attribute and validate both fabrics have expected values extractable from their NOC. The NOCs attribute must show the NOC for every fabric."):
            nocs_list = await self.read_single_attribute_check_success(
                dev_ctrl=th1_dev_ctrl,
                node_id=th1_dut_node_id,
                cluster=opcreds,
                attribute=opcreds.Attributes.NOCs,
                fabric_filtered=False
            )

            fabric_ids_from_certs = {}
            noc_public_keys_from_certs = {}

            fabric_indices = {
                "TH1": th1_fabric_index,
                "TH2": th2_fabric_index
            }

            fabric_ids = {
                "TH1": self.default_controller.fabricId,
                "TH2": th2_fabricId
            }

            found_fabric_indices = set()

            for controller_name, fabric_index in fabric_indices.items():
                for noc_struct in nocs_list:
                    if noc_struct.fabricIndex != fabric_index:
                        continue

                    found_fabric_indices.add(noc_struct.fabricIndex)
                    asserts.assert_true(noc_struct.noc is not None and len(
                        noc_struct.noc) > 0, "`noc` field in NOCs attribute entry not found for fabric index {fabric_index}! Ensure you are running a Matter stack for >= 1.4.2 where NOCStruct fields are not fabric-sensitive.")

                    try:
                        logging.info(f"Trying to parse NOC for fabric index {fabric_index}")
                        noc_cert = MatterCertParser(noc_struct.noc)
                        for tag, value in noc_cert.get_subject_names().items():
                            if tag == noc_cert.SUBJECT_FABRIC_ID_TAG:
                                fabric_ids_from_certs[controller_name] = value
                        noc_public_keys_from_certs[controller_name] = noc_cert.get_public_key_bytes()
                    except (ValueError, IndexError, KeyError, TypeError) as e:
                        asserts.fail(f"Failed to parse NOC for fabric index {fabric_index}: {str(e)}")

                    logging.info(f"Succeeded in parsing NOC for fabric index {fabric_index}.")
                    logging.info(f"  -> NOC public key bytes: {to_octet_string(noc_public_keys_from_certs[controller_name])}")

            logging.info(f"Fabric IDs found: {fabric_ids_from_certs}")

            asserts.assert_true(th1_fabric_index in found_fabric_indices,
                                f"Expected to have seen entry for TH1's fabric (fabric Index {th1_fabric_index}) in NOCs attribute, but did not find it!")
            asserts.assert_true(th2_fabric_index in found_fabric_indices,
                                f"Expected to have seen entry for TH2's fabric (fabric Index {th2_fabric_index}) in NOCs attribute, but did not find it!")

            for controller_name, fabric_id in fabric_ids.items():
                asserts.assert_in(controller_name, fabric_ids_from_certs.keys(),
                                  f"Did not find {controller_name}'s fabric the NOCs attribute")
                asserts.assert_equal(fabric_ids_from_certs[controller_name], fabric_id,
                                     f"Did not find {controller_name}'s fabric ID in the correct NOC")

        with test_step(3, description="Read DUT's Fabrics attribute and validate both fabrics have expected values."):
            fabric_roots = {
                "TH1": th1_root_public_key,
                "TH2": th2_root_public_key
            }

            fabric_vendor_ids = {
                "TH1": 0xfff1,
                "TH2": th2_vid
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

        with test_step(4, description="TH1 sends SignVIDVerificationRequest for TH2's fabric. Verify the response and signature."):

            client_challenge = bytes_from_hex(
                "a1:a2:a3:a4:a5:a6:a7:a8:a9:aa:ab:ac:ad:ae:af:b0:b1:b2:b3:b4:b5:b6:b7:b8:b9:ba:bb:bc:bd:be:bf:c0")
            sign_vid_verification_response = await self.send_single_cmd(cmd=opcreds.Commands.SignVIDVerificationRequest(fabricIndex=th2_fabric_index, clientChallenge=client_challenge))

            asserts.assert_equal(sign_vid_verification_response.fabricIndex, th2_fabric_index,
                                 "FabricIndex in SignVIDVerificationResponse must match request.")

            # Locally generate the vendor_id_verification_tbs to check the signature.
            expected_vendor_fabric_binding_message = generate_vendor_fabric_binding_message(
                root_public_key_bytes=th2_root_public_key, fabric_id=th2_fabricId, vendor_id=th2_vid)
            attestation_challenge = dev_ctrl.GetConnectedDeviceSync(self.dut_node_id, allowPASE=False).attestationChallenge
            vendor_id_verification_tbs = generate_vendor_id_verification_tbs(sign_vid_verification_response.fabricBindingVersion, attestation_challenge,
                                                                             client_challenge, sign_vid_verification_response.fabricIndex, expected_vendor_fabric_binding_message, vid_verification_statement=b"")

            # Check signature against vendor_id_verification_tbs
            asserts.assert_true(verify_signature(public_key=noc_public_keys_from_certs["TH2"], message=vendor_id_verification_tbs,
                                signature=sign_vid_verification_response.signature), "VID Verification Signature must validate using DUT's NOC public key")

        with test_step(5, description="Send bad SignVIDVerificationRequest commands and verify failures"):
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
                                 "Expected CONSTRAINT_ERROR from SignVIDVerificationRequest against fabricIndex 0")

            # Must fail with client challenge different than expected length
            CHALLENGE_TOO_SMALL = b"\x01" * (VID_VERIFICATION_CLIENT_CHALLENGE_SIZE_BYTES - 1)
            CHALLENGE_TOO_BIG = b"\x02" * (VID_VERIFICATION_CLIENT_CHALLENGE_SIZE_BYTES + 1)

            with asserts.assert_raises(InteractionModelError) as exception_context:
                await self.send_single_cmd(cmd=opcreds.Commands.SignVIDVerificationRequest(fabricIndex=th2_fabric_index, clientChallenge=CHALLENGE_TOO_SMALL))

            asserts.assert_equal(exception_context.exception.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR from SignVIDVerificationRequest with ClientChallenge of length {len(CHALLENGE_TOO_SMALL)}")

            with asserts.assert_raises(InteractionModelError) as exception_context:
                await self.send_single_cmd(cmd=opcreds.Commands.SignVIDVerificationRequest(fabricIndex=th2_fabric_index, clientChallenge=CHALLENGE_TOO_BIG))

            asserts.assert_equal(exception_context.exception.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR from SignVIDVerificationRequest with ClientChallenge of length {len(CHALLENGE_TOO_BIG)}")

        with test_step(6, description="Invoke SetVIDVerificationStatement with VVSC, VIDVerificationStatement and VID fields all missing, outside fail-safe. Expect INVALID_COMMAND"):
            with asserts.assert_raises(InteractionModelError) as exception_context:
                await self.send_single_cmd(cmd=opcreds.Commands.SetVIDVerificationStatement())
            asserts.assert_equal(exception_context.exception.status, Status.InvalidCommand,
                                 "Expected INVALID_COMMAND for SetVIDVerificationStatement with no arguments present")

        with test_step(7, description="Invoke SetVIDVerificationStatement using TH2's fabric (no ICAC present) with VVSC field set to a size > 400 bytes, outside fail-safe. Expect CONSTRAINT_ERROR"):
            with asserts.assert_raises(InteractionModelError) as exception_context:
                await self.send_single_cmd(dev_ctrl=th2_dev_ctrl, node_id=th2_dut_node_id, cmd=opcreds.Commands.SetVIDVerificationStatement(vvsc=(b"\x01" * 401)))
            asserts.assert_equal(exception_context.exception.status, Status.ConstraintError,
                                 "Expected CONSTRAINT_ERROR for SetVIDVerificationStatement with VVSC too large")

        with test_step(8, description="Invoke SetVIDVerificationStatement with VIDVerificationStatement field set to a size > 85 bytes, outside fail-safe. Expect CONSTRAINT_ERROR"):
            with asserts.assert_raises(InteractionModelError) as exception_context:
                await self.send_single_cmd(cmd=opcreds.Commands.SetVIDVerificationStatement(VIDVerificationStatement=(b"\x01" * (VID_VERIFICATION_STATEMENT_SIZE_BYTES_V1 + 1))))
            asserts.assert_equal(exception_context.exception.status, Status.ConstraintError,
                                 "Expected CONSTRAINT_ERROR for SetVIDVerificationStatement with VIDVerificationStatement too large")

        with test_step(9, description="Establish a subscription to Operational Credentials cluster on endpoint 0 from TH1 fabric client, with MinIntervalFloor=0, MaxIntervalCeiling=30"):
            attrib_listener = ClusterAttributeChangeAccumulator(opcreds)
            await attrib_listener.start(th1_dev_ctrl, th1_dut_node_id, endpoint=0, min_interval_sec=0, max_interval_sec=30)

        with test_step(10, description="Invoke SetVIDVerificationStatement with maximum-sized VVSC and VIDVerificationStatement present and setting VID to 0x6a01 on TH2's fabric, outside fail-safe. Verify VIDVerificationStatement, VVSC and VID updates are correct. Verify subscription received the updated values."):
            vvsc = b"\xaa" * 400
            VIDVerificationStatement = b"\x01" * VID_VERIFICATION_STATEMENT_SIZE_BYTES_V1
            vendorID = 0x6a01

            attrib_listener.reset()
            await self.send_single_cmd(dev_ctrl=th2_dev_ctrl, node_id=th2_dut_node_id, cmd=opcreds.Commands.SetVIDVerificationStatement(VIDVerificationStatement=VIDVerificationStatement, vvsc=vvsc, vendorID=vendorID))
            attrib_listener.await_all_expected_report_matches(expected_matchers=[make_vid_matcher(
                th2_fabric_index, vendorID), make_vvs_matcher(th2_fabric_index, VIDVerificationStatement)], timeout_sec=30.0)

            updated_fabrics, updated_nocs = await self.read_updated_fabrics(dev_ctrl=th1_dev_ctrl, node_id=th1_dut_node_id)

            th2_fabrics_entry = get_entry_for_fabric(th2_fabric_index, updated_fabrics)
            asserts.assert_is_not_none(
                th2_fabrics_entry, f"Could not find Fabrics list entry for TH2's fabric index {th2_fabric_index}")

            th2_nocs_entry = get_entry_for_fabric(th2_fabric_index, updated_nocs)
            asserts.assert_is_not_none(th2_nocs_entry, f"Could not find NOCs list entry for TH2's fabric index {th2_fabric_index}")

            asserts.assert_equal(th2_nocs_entry.vvsc, vvsc,
                                 "Did not get the expected value set for VVSC field of NOCs list for TH2's fabric.")
            asserts.assert_equal(th2_fabrics_entry.VIDVerificationStatement, VIDVerificationStatement,
                                 "Did not get the expected value set for VIDVerificationStatement field of Fabrics list for TH2's fabric.")
            asserts.assert_equal(th2_fabrics_entry.vendorID, vendorID,
                                 "Did not get the expected value set for VendorID field of Fabrics list for TH2's fabric.")

        with test_step(11, description="Invoke SetVIDVerificationStatement with maximum-sized VVSC on TH1's fabric, outside fail-safe. Verify INVALID_COMMAND due to presence of ICAC."):
            vvsc = b"\xaa" * 400

            with asserts.assert_raises(InteractionModelError) as exception_context:
                await self.send_single_cmd(dev_ctrl=th1_dev_ctrl, node_id=th1_dut_node_id, cmd=opcreds.Commands.SetVIDVerificationStatement(vvsc=vvsc))
            asserts.assert_equal(exception_context.exception.status, Status.InvalidCommand,
                                 "Expected INVALID_COMMAND for SetVIDVerificationStatement with VVSC present against DUT on TH1's fabric due to presence of ICAC.")

        with test_step(12, description="Invoke SetVIDVerificationStatement with setting VID to 0xFFF1 on TH2's fabric, outside fail-safe. Verify VID is now 0xFFF1."):
            vendorID = 0xFFF1

            attrib_listener.reset()
            await self.send_single_cmd(dev_ctrl=th2_dev_ctrl, node_id=th2_dut_node_id, cmd=opcreds.Commands.SetVIDVerificationStatement(vendorID=vendorID))
            attrib_listener.await_all_expected_report_matches(
                expected_matchers=[make_vid_matcher(th2_fabric_index, vendorID)], timeout_sec=30.0)

            updated_fabrics, _ = await self.read_updated_fabrics(dev_ctrl=th1_dev_ctrl, node_id=th1_dut_node_id)

            th2_fabrics_entry = get_entry_for_fabric(th2_fabric_index, updated_fabrics)
            asserts.assert_is_not_none(
                th2_fabrics_entry, f"Could not find Fabrics list entry for TH2's fabric index {th2_fabric_index}")
            asserts.assert_equal(th2_fabrics_entry.vendorID, vendorID,
                                 "Did not get the expected value set for VendorID field of Fabrics list for TH2's fabric.")

        with test_step(13, description="Arm a fail safe for 300s from TH2 on dut_node_id2 with breadcrumb set to 10."):
            await self.send_single_cmd(dev_ctrl=th2_dev_ctrl, node_id=th2_dut_node_id, cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=60, breadcrumb=10))

        with test_step(14, description="Invoke SetVIDVerificationStatement with VVSC and VIDVerificationStatement to empty, and VID set to 0xFFF3 on TH2's fabric, inside fail-safe. Verify VVSC, VIDVerificationStatement are now empty and VID is 0xFFF3 for that fabric."):
            vvsc = b""
            VIDVerificationStatement = b""
            vendorID = 0xFFF3

            attrib_listener.reset()
            await self.send_single_cmd(dev_ctrl=th2_dev_ctrl, node_id=th2_dut_node_id, cmd=opcreds.Commands.SetVIDVerificationStatement(VIDVerificationStatement=VIDVerificationStatement, vvsc=vvsc, vendorID=vendorID))
            attrib_listener.await_all_expected_report_matches(expected_matchers=[make_vid_matcher(
                th2_fabric_index, vendorID), make_vvs_matcher(th2_fabric_index, None)], timeout_sec=30.0)

            updated_fabrics, updated_nocs = await self.read_updated_fabrics(dev_ctrl=th2_dev_ctrl, node_id=th2_dut_node_id)

            th2_fabrics_entry = get_entry_for_fabric(th2_fabric_index, updated_fabrics)
            asserts.assert_is_not_none(
                th2_fabrics_entry, f"Could not find Fabrics list entry for TH2's fabric index {th2_fabric_index}")

            th2_nocs_entry = get_entry_for_fabric(th2_fabric_index, updated_nocs)
            asserts.assert_is_not_none(th2_nocs_entry, f"Could not find NOCs list entry for TH2's fabric index {th2_fabric_index}")

            asserts.assert_equal(th2_nocs_entry.vvsc, None,
                                 "Expected VVSC to be missing")
            asserts.assert_equal(th2_fabrics_entry.VIDVerificationStatement, None,
                                 "Did not get the expected value set for VIDVerificationStatement field of Fabrics list for TH2's fabric.")
            asserts.assert_equal(th2_fabrics_entry.vendorID, 0xFFF3,
                                 "Did not get the expected value set for VendorID field of Fabrics list for TH2's fabric.")

        with test_step(15, description="Disarm fail safe with ArmFailSafe(0s). Verify VVS, VendorID are not reverted, since there had been no AddNOC/UpdateNOC"):
            await self.send_single_cmd(dev_ctrl=th2_dev_ctrl, node_id=th2_dut_node_id, cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=11))
            updated_fabrics, updated_nocs = await self.read_updated_fabrics(dev_ctrl=th2_dev_ctrl, node_id=th2_dut_node_id)

            th2_fabrics_entry = get_entry_for_fabric(th2_fabric_index, updated_fabrics)
            asserts.assert_is_not_none(
                th2_fabrics_entry, f"Could not find Fabrics list entry for TH2's fabric index {th2_fabric_index}")

            th2_nocs_entry = get_entry_for_fabric(th2_fabric_index, updated_nocs)
            asserts.assert_is_not_none(th2_nocs_entry, f"Could not find NOCs list entry for TH2's fabric index {th2_fabric_index}")

            asserts.assert_equal(th2_nocs_entry.vvsc, None,
                                 "Did not get the expected value set for VVSC field of NOCs list for TH2's fabric.")
            asserts.assert_equal(th2_fabrics_entry.VIDVerificationStatement, None,
                                 "Did not get the expected value set for VIDVerificationStatement field of Fabrics list for TH2's fabric.")
            asserts.assert_equal(th2_fabrics_entry.vendorID, 0xFFF3,
                                 "Did not get the expected value set for VendorID field of Fabrics list for TH2's fabric.")

        with test_step(16, description="Create a new fabric under TH2's root with fabric ID 0x3333, node ID 0x33333333, VendorID 0xFFF2, by invoking ArmFailSafe(600s), CSRRequest, AddTrustedRootCertificate and AddNOC. Do not disarm failsafe, do not execute commissioning complete."):
            th3_dut_node_id = 0x33333333

            th3_vid = 0xFFF2
            th3_fabricId = 0x3333
            th3_new_fabric_admin = new_certificate_authority.NewFabricAdmin(
                vendorId=th3_vid, fabricId=th3_fabricId)
            th3_nodeid = th2_nodeid + 1
            th3_dut_node_id = th2_dut_node_id + 1

            th3_dev_ctrl = th3_new_fabric_admin.NewController(
                nodeId=th3_nodeid)

            success, nocResp, chain = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
                commissionerDevCtrl=th2_dev_ctrl, newFabricDevCtrl=th3_dev_ctrl,
                existingNodeId=th2_dut_node_id, newNodeId=th3_dut_node_id,
                omitCommissioningComplete=True, failSafeDurationSeconds=600
            )

            th3_fabric_index = nocResp.fabricIndex
            asserts.assert_true(success, "Commissioning DUT into TH3's fabrics must succeed.")

        with test_step(17, description="Invoke SetVIDVerificationStatement with VVSC and VIDVerificationStatement present and setting VID to 0xFFF1 on fabric ID 0x3333 under TH2's root, inside fail-safe. Verify VIDVerificationStatement, VVSC and VID values match values set."):
            vvsc = b"\xcc" * 400
            VIDVerificationStatement = b"\x01" * VID_VERIFICATION_STATEMENT_SIZE_BYTES_V1
            vendorID = 0xFFF1

            attrib_listener.reset()
            await self.send_single_cmd(dev_ctrl=th3_dev_ctrl, node_id=th3_dut_node_id, cmd=opcreds.Commands.SetVIDVerificationStatement(VIDVerificationStatement=VIDVerificationStatement, vvsc=vvsc, vendorID=vendorID))
            attrib_listener.await_all_expected_report_matches(expected_matchers=[make_vid_matcher(
                th3_fabric_index, vendorID), make_vvs_matcher(th3_fabric_index, VIDVerificationStatement)], timeout_sec=30.0)

            updated_fabrics, updated_nocs = await self.read_updated_fabrics(dev_ctrl=th3_dev_ctrl, node_id=th3_dut_node_id)

            th3_fabrics_entry = get_entry_for_fabric(th3_fabric_index, updated_fabrics)
            asserts.assert_is_not_none(
                th3_fabrics_entry, f"Could not find Fabrics list entry for TH3's fabric index {th3_fabric_index}")

            th3_nocs_entry = get_entry_for_fabric(th3_fabric_index, updated_nocs)
            asserts.assert_is_not_none(th3_nocs_entry, f"Could not find NOCs list entry for TH3's fabric index {th3_fabric_index}")

            asserts.assert_equal(th3_nocs_entry.vvsc, vvsc,
                                 "Did not get the expected value set for VVSC field of NOCs list for TH3.")
            asserts.assert_equal(th3_fabrics_entry.VIDVerificationStatement, VIDVerificationStatement,
                                 "Did not get the expected value set for VIDVerificationStatement field of Fabrics list for TH3's fabric.")
            asserts.assert_equal(th3_fabrics_entry.vendorID, vendorID,
                                 "Did not get the expected value set for VendorID field of Fabrics list for TH3's fabric.")

        with test_step(18, description="Disarm failsafe with ArmFailSafe(0s) from TH3 client. Verify that fabric table no longer has VVSC and VIDVerificationStatement for the pending fabric that was dropped."):
            attrib_listener.reset()

            await self.send_single_cmd(dev_ctrl=th3_dev_ctrl, node_id=th3_dut_node_id, cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=12))

            updated_fabrics, updated_nocs = await self.read_updated_fabrics(dev_ctrl=th2_dev_ctrl, node_id=th2_dut_node_id)

            th3_fabrics_entry = get_entry_for_fabric(th3_fabric_index, updated_fabrics)
            asserts.assert_is_none(
                th3_fabrics_entry, f"Expected to see no more data for TH3's fabric in Fabrics attribute, but found some anyway: {th3_fabrics_entry}")

            th3_nocs_entry = get_entry_for_fabric(th3_fabric_index, updated_nocs)
            asserts.assert_is_none(
                th3_nocs_entry, f"Expected to see no more data for TH3's fabric in NOCs attribute, but found some anyway: {th3_nocs_entry}")

        with test_step(19, description="SetVIDVerificationStatement against TH1's fabric with VendorID set to 0xFFF5. Expect a CONSTRAINT_ERROR."):
            with asserts.assert_raises(InteractionModelError) as exception_context:
                await self.send_single_cmd(cmd=opcreds.Commands.SetVIDVerificationStatement(vendorID=0xFFF5))
            asserts.assert_equal(exception_context.exception.status, Status.ConstraintError,
                                 "Expected CONSTRAINT_ERROR for SetVIDVerificationStatement with invalid VendorID 0xFFF5")

        with test_step(20, description="Invoke SetVIDVerificationStatement with maximum-sized VVSC and VIDVerificationStatement present and setting VID to 0x6a01 on TH2's fabric, outside fail-safe. Expect success."):
            vvsc = b"\x5a" * 400
            VIDVerificationStatement = b"\x01" * VID_VERIFICATION_STATEMENT_SIZE_BYTES_V1
            th2_vid = 0x6a01

            await self.send_single_cmd(dev_ctrl=th2_dev_ctrl, node_id=th2_dut_node_id, cmd=opcreds.Commands.SetVIDVerificationStatement(VIDVerificationStatement=VIDVerificationStatement, vvsc=vvsc, vendorID=th2_vid))

        with test_step(21, description="TH1 sends SignVIDVerificationRequest for TH2's fabric (which has a VIDVerificationStatement). Verify the response and signature."):
            client_challenge = bytes_from_hex(
                "a1:a2:a3:a4:a5:a6:a7:a8:a9:aa:ab:ac:ad:ae:af:b0:b1:b2:b3:b4:b5:b6:b7:b8:b9:ba:bb:bc:bd:be:bf:c1")
            sign_vid_verification_response = await self.send_single_cmd(cmd=opcreds.Commands.SignVIDVerificationRequest(fabricIndex=th2_fabric_index, clientChallenge=client_challenge))

            asserts.assert_equal(sign_vid_verification_response.fabricIndex, th2_fabric_index,
                                 "FabricIndex in SignVIDVerificationResponse must match request.")

            # Locally generate the vendor_id_verification_tbs to check the signature.
            expected_vendor_fabric_binding_message = generate_vendor_fabric_binding_message(
                root_public_key_bytes=th2_root_public_key, fabric_id=th2_fabricId, vendor_id=th2_vid)
            attestation_challenge = dev_ctrl.GetConnectedDeviceSync(self.dut_node_id, allowPASE=False).attestationChallenge
            vendor_id_verification_tbs = generate_vendor_id_verification_tbs(sign_vid_verification_response.fabricBindingVersion, attestation_challenge,
                                                                             client_challenge, sign_vid_verification_response.fabricIndex, expected_vendor_fabric_binding_message, vid_verification_statement=VIDVerificationStatement)

            # Check signature against vendor_id_verification_tbs
            asserts.assert_true(verify_signature(public_key=noc_public_keys_from_certs["TH2"], message=vendor_id_verification_tbs,
                                signature=sign_vid_verification_response.signature), "VID Verification Signature must validate using DUT's NOC public key")

        # Cover having done UpdateNOC in a Fail-safe, overriding the VVS and VVSC, and then expiring failsafe and seeing the prior values are returned.
        with test_step(22, description="Start updating fabric under TH2's root with fabric ID 0x2222, node ID 0x22222222, by invoking ArmFailSafe(600s), CSRRequest and UpdateNOC. Do not disarm failsafe, do not execute commissioning complete."):
            # fabricId is already 0x2222
            new_th2_nodeId = 0x22222222
            success = await CommissioningBuildingBlocks.UpdateNOC(
                devCtrl=th2_dev_ctrl, existingNodeId=th2_dut_node_id, newNodeId=new_th2_nodeId,
                omitCommissioningComplete=True, failSafeDurationSeconds=600
            )

            asserts.assert_true(success, "UpdateNOC into TH2's fabric must succeed.")

        with test_step(23, description="Invoke SetVIDVerificationStatement with different VVSC and VIDVerificationStatement present and setting VID back to 0xFFF2 under TH2's fabric, inside fail-safe. Verify VIDVerificationStatement, VVSC and VID values match values set."):
            new_vvsc = b"\xcc" * 400
            new_VIDVerificationStatement = b"\x01" + (b"\x02" * (VID_VERIFICATION_STATEMENT_SIZE_BYTES_V1 - 1))
            new_vendorID = 0xFFF2

            attrib_listener.reset()
            await self.send_single_cmd(dev_ctrl=th2_dev_ctrl, node_id=new_th2_nodeId, cmd=opcreds.Commands.SetVIDVerificationStatement(VIDVerificationStatement=new_VIDVerificationStatement, vvsc=new_vvsc, vendorID=new_vendorID))
            attrib_listener.await_all_expected_report_matches(expected_matchers=[make_vid_matcher(
                th2_fabric_index, new_vendorID), make_vvs_matcher(th2_fabric_index, new_VIDVerificationStatement)], timeout_sec=30.0)

            updated_fabrics, updated_nocs = await self.read_updated_fabrics(dev_ctrl=th2_dev_ctrl, node_id=new_th2_nodeId)

            th2_fabrics_entry = get_entry_for_fabric(th2_fabric_index, updated_fabrics)
            asserts.assert_is_not_none(
                th2_fabrics_entry, f"Could not find Fabrics list entry for TH2's fabric index {th2_fabric_index}")

            th2_nocs_entry = get_entry_for_fabric(th2_fabric_index, updated_nocs)
            asserts.assert_is_not_none(th2_nocs_entry, f"Could not find NOCs list entry for TH2's fabric index {th2_fabric_index}")

            asserts.assert_equal(th2_nocs_entry.vvsc, new_vvsc,
                                 "Did not get the expected value set for VVSC field of NOCs list for TH2.")
            asserts.assert_equal(th2_fabrics_entry.VIDVerificationStatement, new_VIDVerificationStatement,
                                 "Did not get the expected value set for VIDVerificationStatement field of Fabrics list for TH2's fabric.")
            asserts.assert_equal(th2_fabrics_entry.vendorID, new_vendorID,
                                 "Did not get the expected value set for VendorID field of Fabrics list for TH2's fabric.")

        with test_step(24, description="Disarm failsafe with ArmFailSafe(0s) from TH2 client. Verify that fabric table has previous VIDVerificationStatement and VVSC due to the pending fabric update that was dropped."):
            attrib_listener.reset()

            await self.send_single_cmd(dev_ctrl=th2_dev_ctrl, node_id=new_th2_nodeId, cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=24))

            updated_fabrics, updated_nocs = await self.read_updated_fabrics(dev_ctrl=th2_dev_ctrl, node_id=th2_dut_node_id)

            attrib_listener.await_all_expected_report_matches(expected_matchers=[make_vid_matcher(
                th2_fabric_index, th2_vid), make_vvs_matcher(th2_fabric_index, VIDVerificationStatement)], timeout_sec=30.0)

            th2_fabrics_entry = get_entry_for_fabric(th2_fabric_index, updated_fabrics)
            asserts.assert_is_not_none(
                th2_fabrics_entry, f"Could not find Fabrics list entry for TH2's fabric index {th2_fabric_index}")

            th2_nocs_entry = get_entry_for_fabric(th2_fabric_index, updated_nocs)
            asserts.assert_is_not_none(th2_nocs_entry, f"Could not find NOCs list entry for TH2's fabric index {th2_fabric_index}")

            asserts.assert_equal(th2_nocs_entry.vvsc, vvsc,
                                 "Did not get the expected value set for VVSC field of NOCs list for TH2's fabric.")
            asserts.assert_equal(th2_fabrics_entry.VIDVerificationStatement, VIDVerificationStatement,
                                 "Did not get the expected value set for VIDVerificationStatement field of Fabrics list for TH2's fabric.")
            asserts.assert_equal(th2_fabrics_entry.vendorID, th2_vid,
                                 "Did not get the expected value set for VendorID field of Fabrics list for TH2's fabric.")

        # Cover having done UpdateNOC in a Fail-safe, overriding the VVS and VVSC, and then commissioning complete, and seeing the updated values are seen.
        with test_step(25, description="Start updating fabric under TH2's root with fabric ID 0x2222, node ID 0x77777777, by invoking ArmFailSafe(600s), CSRRequest and UpdateNOC. Do not disarm failsafe, do not execute commissioning complete."):
            # fabricId is already 0x2222
            new_th2_nodeId = 0x77777777
            success = await CommissioningBuildingBlocks.UpdateNOC(
                devCtrl=th2_dev_ctrl, existingNodeId=th1_dut_node_id + 1, newNodeId=new_th2_nodeId,
                omitCommissioningComplete=True, failSafeDurationSeconds=600
            )

            asserts.assert_true(success, "UpdateNOC into TH2's fabric must succeed.")

        with test_step(26, description="Invoke SetVIDVerificationStatement with VVSC and VIDVerificationStatement present and setting VID to 0xFFF4 under TH2's fabric, inside fail-safe. Verify VIDVerificationStatement, VVSC and VID values match values set."):
            vvsc = b"\xcd" * 400
            VIDVerificationStatement = b"\x01" + (b"\x03" * (VID_VERIFICATION_STATEMENT_SIZE_BYTES_V1 - 1))
            vendorID = 0xFFF4

            attrib_listener.reset()
            await self.send_single_cmd(dev_ctrl=th2_dev_ctrl, node_id=new_th2_nodeId, cmd=opcreds.Commands.SetVIDVerificationStatement(VIDVerificationStatement=VIDVerificationStatement, vvsc=vvsc, vendorID=vendorID))
            attrib_listener.await_all_expected_report_matches(expected_matchers=[make_vid_matcher(
                th2_fabric_index, vendorID), make_vvs_matcher(th2_fabric_index, VIDVerificationStatement)], timeout_sec=30.0)

            updated_fabrics, updated_nocs = await self.read_updated_fabrics(dev_ctrl=th2_dev_ctrl, node_id=new_th2_nodeId)

            th2_fabrics_entry = get_entry_for_fabric(th2_fabric_index, updated_fabrics)
            asserts.assert_is_not_none(
                th2_fabrics_entry, f"Could not find Fabrics list entry for TH2's fabric index {th2_fabric_index}")

            th2_nocs_entry = get_entry_for_fabric(th2_fabric_index, updated_nocs)
            asserts.assert_is_not_none(th2_nocs_entry, f"Could not find NOCs list entry for TH2's fabric index {th2_fabric_index}")

            asserts.assert_equal(th2_nocs_entry.vvsc, vvsc,
                                 "Did not get the expected value set for VVSC field of NOCs list for TH2.")
            asserts.assert_equal(th2_fabrics_entry.VIDVerificationStatement, VIDVerificationStatement,
                                 "Did not get the expected value set for VIDVerificationStatement field of Fabrics list for TH2's fabric.")
            asserts.assert_equal(th2_fabrics_entry.vendorID, vendorID,
                                 "Did not get the expected value set for VendorID field of Fabrics list for TH2's fabric.")

        with test_step(27, description="Invoke CommissioningComplete on the pending fabric under TH2 root. Verify VIDVerificationStatement, VVSC and VID values match the values that were set inside the fail-safe period."):
            resp = await self.send_single_cmd(dev_ctrl=th2_dev_ctrl, node_id=new_th2_nodeId, cmd=Clusters.GeneralCommissioning.Commands.CommissioningComplete())
            if resp.errorCode is not Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk:
                # Expiring the failsafe timer in an attempt to clean up.
                await self.send_single_cmd(dev_ctrl=th2_dev_ctrl, node_id=new_th2_nodeId, cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=27))
                asserts.assert_equal(resp.errorCode, Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                                     "Expected success of CommissioningComplete")

            th2_dut_node_id = new_th2_nodeId

            updated_fabrics, updated_nocs = await self.read_updated_fabrics(dev_ctrl=th2_dev_ctrl, node_id=th2_dut_node_id)

            th2_fabrics_entry = get_entry_for_fabric(th2_fabric_index, updated_fabrics)
            asserts.assert_is_not_none(
                th2_fabrics_entry, f"Could not find Fabrics list entry for TH2's fabric index {th2_fabric_index}")

            th2_nocs_entry = get_entry_for_fabric(th2_fabric_index, updated_nocs)
            asserts.assert_is_not_none(th2_nocs_entry, f"Could not find NOCs list entry for TH2's fabric index {th2_fabric_index}")

            asserts.assert_equal(th2_nocs_entry.vvsc, vvsc,
                                 "Did not get the expected value set for VVSC field of NOCs list for TH2.")
            asserts.assert_equal(th2_fabrics_entry.VIDVerificationStatement, VIDVerificationStatement,
                                 "Did not get the expected value set for VIDVerificationStatement field of Fabrics list for TH2's fabric.")
            asserts.assert_equal(th2_fabrics_entry.vendorID, vendorID,
                                 "Did not get the expected value set for VendorID field of Fabrics list for TH2's fabric.")

        with test_step(28, description="Remove TH2 and TH3's fabric"):
            cmd = opcreds.Commands.RemoveFabric(th2_fabric_index)
            resp = await self.send_single_cmd(cmd=cmd)
            asserts.assert_equal(
                resp.statusCode, opcreds.Enums.NodeOperationalCertStatusEnum.kOk, "Expected removal of TH2's fabric to succeed")


if __name__ == "__main__":
    default_matter_test_main()
