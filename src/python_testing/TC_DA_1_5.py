#
#    Copyright (c) 2023 Project CHIP Authors
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

import random

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.interaction_model import InteractionModelError, Status
from chip.testing.conversions import hex_from_bytes
from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main, type_matches
from chip.tlv import TLVReader
from cryptography import x509
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec, utils
from ecdsa.curves import curve_by_name
from mobly import asserts
from pyasn1.codec.der.decoder import decode as der_decoder
from pyasn1.error import PyAsn1Error
from pyasn1_modules import rfc2986, rfc3279, rfc5480


class TC_DA_1_5(MatterBaseTest):
    @async_test_body
    async def test_TC_DA_1_5(self):

        opcreds = Clusters.Objects.OperationalCredentials

        gcomm = Clusters.Objects.GeneralCommissioning

        self.print_step(1, "Commissioning, already done")

        self.print_step(2, "Save attestation challenge")
        proxy = self.default_controller.GetConnectedDeviceSync(self.dut_node_id, False)
        attestation_challenge = proxy.attestationChallenge

        self.print_step(3, "Send CertificateChainRequest for DAC")
        certtype = opcreds.Enums.CertificateChainTypeEnum.kDACCertificate
        dac_resp = await self.send_single_cmd(cmd=opcreds.Commands.CertificateChainRequest(certificateType=certtype))
        asserts.assert_true(type_matches(dac_resp, opcreds.Commands.CertificateChainResponse),
                            "Certificate request returned incorrect type")
        der_dac = dac_resp.certificate
        # This throws an exception for a non-x509 cert
        try:
            dac = x509.load_der_x509_certificate(der_dac)
        except ValueError:
            asserts.assert_true(False, "Unable to parse certificate from CertificateChainResponse")

        self.print_step(4, "Send ArmFailSafe")
        await self.send_single_cmd(cmd=gcomm.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=1))

        self.print_step(5, "Send CSRRequest")
        csr_nonce = random.randbytes(32)
        csr_resp = await self.send_single_cmd(cmd=opcreds.Commands.CSRRequest(CSRNonce=csr_nonce, isForUpdateNOC=False))
        nocsr_elements = csr_resp.NOCSRElements
        nocsr_attestation_signature_raw = csr_resp.attestationSignature

        self.print_step(6, "Extract TLV")
        decoded = TLVReader(nocsr_elements).get()["Any"]
        # CSR is field 1, nonce is field 2
        asserts.assert_in(1, decoded.keys(), "CSR is not present in the NOSCRElements")
        asserts.assert_in(2, decoded.keys(), "Nonce is not present in the NOSCRElements")
        csr_raw = decoded[1]
        csr_nonce_returned = decoded[2]
        vendor1 = None
        vendor2 = None
        vendor3 = None
        if 3 in decoded.keys():
            vendor1 = decoded[3]
        if 4 in decoded.keys():
            vendor2 = decoded[4]
        if 5 in decoded.keys():
            vendor3 = decoded[5]

        # Verify that length of nocsr_elements is <= 900
        asserts.assert_less_equal(len(nocsr_elements), 900, "NOCSRElements is more than 900 bytes")

        # Verify der encoded and PKCS #10 (rfc2986 is PKCS #10) - next two requirements
        try:
            temp, _ = der_decoder(csr_raw, asn1Spec=rfc2986.CertificationRequest())
        except PyAsn1Error:
            asserts.fail("Unable to decode CSR - improperly formatted DER file")

        layer1 = dict(temp)
        info = dict(layer1['certificationRequestInfo'])

        # Verify public key is id-ecPublicKey with prime256v1
        requested_pk_algo = dict(dict(info['subjectPKInfo'])['algorithm'])
        asserts.assert_equal(requested_pk_algo['algorithm'], rfc5480.id_ecPublicKey, "Incorrect public key algorithm")
        der_parameters = requested_pk_algo['parameters']
        temp, _ = der_decoder(bytes(der_parameters), asn1Spec=rfc3279.EcpkParameters())
        parameters = dict(temp)
        asserts.assert_in('namedCurve', parameters.keys(), "Unable to find namedCurve in EcpkParameters")
        asserts.assert_equal(parameters['namedCurve'], rfc3279.prime256v1, "Incorrect curve specified for public key algorithm")

        # Verify public key is 256 bytes
        csr = x509.load_der_x509_csr(csr_raw)
        csr_pubkey = csr.public_key()
        asserts.assert_equal(csr_pubkey.key_size, 256, "Incorrect key size")

        # Verify signature algorithm is ecdsa-with-SHA156
        signature_algorithm = dict(layer1['signatureAlgorithm'])['algorithm']
        asserts.assert_equal(signature_algorithm, rfc5480.ecdsa_with_SHA256, "CSR specifies incorrect signature key algorithm")

        # Verify signature is valid
        asserts.assert_true(csr.is_signature_valid, "Signature is invalid")

        # Verify csr_nonce_returned is octet string of length 32
        try:
            # csr_nonce_returned is an octet string if it can be converted to an int
            int(hex_from_bytes(csr_nonce_returned), 16)
        except ValueError:
            asserts.fail("Returned CSR nonce is not an octet string")

        # Verify returned nonce matches sent nonce
        asserts.assert_equal(csr_nonce_returned, csr_nonce, "Returned nonce is incorrect")

        nocsr_tbs = nocsr_elements + attestation_challenge

        self.print_step(7, "Verify signature")
        baselen = curve_by_name("NIST256p").baselen
        attestation_raw_r = int(hex_from_bytes(nocsr_attestation_signature_raw[:baselen]), 16)
        attestation_raw_s = int(hex_from_bytes(nocsr_attestation_signature_raw[baselen:]), 16)

        nocsr_attestation = utils.encode_dss_signature(attestation_raw_r, attestation_raw_s)

        dac.public_key().verify(signature=nocsr_attestation, data=nocsr_tbs, signature_algorithm=ec.ECDSA(hashes.SHA256()))

        self.print_step(8, "Verify that attestation challenge does not appear in the vendor fields")
        if vendor1:
            asserts.assert_not_in(attestation_challenge, vendor1, "Attestation challenge appears in vendor 1")
        if vendor2:
            asserts.assert_not_in(attestation_challenge, vendor2, "Attestation challenge appears in vendor 2")
        if vendor3:
            asserts.assert_not_in(attestation_challenge, vendor3, "Attestation challenge appears in vendor 3")

        self.print_step(9, "Disarm failsafe")
        await self.send_single_cmd(cmd=gcomm.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=1))

        self.print_step(10, "Arm failsafe to 900s")
        await self.send_single_cmd(cmd=gcomm.Commands.ArmFailSafe(expiryLengthSeconds=900, breadcrumb=1))

        self.print_step(11, "Send CSRRequest wtih 31-byte nonce")
        bad_nonce = random.randbytes(32)
        try:
            await self.send_single_cmd(cmd=opcreds.Commands.CSRRequest(CSRNonce=bad_nonce, isForUpdateNOC=False))
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand, "Received incorrect error from CSRRequest command with bad nonce")

        self.print_step(12, "Disarm failsafe")
        await self.send_single_cmd(cmd=gcomm.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=1))

        self.print_step(13, "Open commissioning window")
        params = await self.default_controller.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=600, iteration=10000, discriminator=1234, option=1)

        self.print_step(14, "Commission to TH2")
        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=2)
        TH2 = new_fabric_admin.NewController(nodeId=112233)

        await TH2.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=1234)

        self.print_step(15, "Read NOCs list for TH1")
        temp = await self.read_single_attribute_check_success(
            cluster=Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.NOCs)
        asserts.assert_equal(len(temp), 1, "Returned NOC list does not contain one entry")
        th1_noc = temp[0].noc

        self.print_step(16, "Read NOCs list for TH2")
        temp = await self.read_single_attribute_check_success(
            cluster=Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.NOCs, dev_ctrl=TH2)
        asserts.assert_equal(len(temp), 1, "Returned NOC list does not contain one entry")
        th2_noc = temp[0].noc

        self.print_step(17, "Extract the public keys")
        # NOCs are TLV encoded, public key is field 9
        th1_decoded = TLVReader(th1_noc).get()["Any"]
        th2_decoded = TLVReader(th2_noc).get()["Any"]

        th1_pk = th1_decoded[9]
        th2_pk = th2_decoded[9]
        asserts.assert_not_equal(th1_pk, th2_pk, "Publc keys are the same")

        self.print_step(17, "Read the fabric index for TH2")
        th2_idx = await self.read_single_attribute_check_success(
            Clusters.OperationalCredentials,
            attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex, dev_ctrl=TH2)

        self.print_step(18, "Remove TH2")
        await self.send_single_cmd(cmd=Clusters.OperationalCredentials.Commands.RemoveFabric(fabricIndex=th2_idx))


if __name__ == "__main__":
    default_matter_test_main()
