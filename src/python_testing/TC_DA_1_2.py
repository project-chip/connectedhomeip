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

import logging
import os
import random
import re

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.testing.basic_composition import BasicCompositionTests
from chip.testing.conversions import hex_from_bytes
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches
from chip.tlv import TLVReader
from cryptography import x509
from cryptography.exceptions import InvalidSignature
from cryptography.hazmat._oid import ExtensionOID
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec, utils
from ecdsa.curves import curve_by_name
from mobly import asserts
from pyasn1.codec.der.decoder import decode as der_decoder
from pyasn1.error import PyAsn1Error
from pyasn1.type import univ
from pyasn1_modules import rfc5652


def get_value_for_oid(oid_dotted_str: str, cert: x509.Certificate) -> str:
    rdn = list(filter(lambda rdn: oid_dotted_str in rdn.oid.dotted_string, cert.subject))
    if len(rdn) != 1:
        return None

    return rdn[0].value.strip()


def parse_ids_from_subject(cert: x509.Certificate) -> tuple([str, str]):
    vid_str = get_value_for_oid('1.3.6.1.4.1.37244.2.1', cert)
    pid_str = get_value_for_oid('1.3.6.1.4.1.37244.2.2', cert)

    return vid_str, pid_str


def parse_single_vidpid_from_common_name(commonName: str, tag_str: str) -> str:
    sp = commonName.split(tag_str)
    if (len(sp)) != 2:
        return None

    s = sp[1][:4]
    if re.match("[0-9A-F]{4}", s) is None:
        asserts.fail(f"Improperly encoded PID or VID when using fallback encoding {tag_str}:{s}")
        return None

    return s


def parse_ids_from_common_name(cert: x509.Certificate) -> tuple([str, str]):
    common = get_value_for_oid('2.5.4.3', cert)
    vid_str = parse_single_vidpid_from_common_name(common, 'Mvid:')
    pid_str = parse_single_vidpid_from_common_name(common, 'Mpid:')

    return vid_str, pid_str


def parse_ids_from_certs(dac: x509.Certificate, pai: x509.Certificate) -> tuple([int, int, int, int]):
    dac_vid_str, dac_pid_str = parse_ids_from_subject(dac)
    pai_vid_str, pai_pid_str = parse_ids_from_subject(pai)

    # Fallback methods - parse from commonName
    if dac_vid_str is None and dac_pid_str is None:
        dac_vid_str, dac_pid_str = parse_ids_from_common_name(dac)

    if pai_vid_str is None and pai_pid_str is None:
        pai_vid_str, pai_pid_str = parse_ids_from_common_name(pai)

    # PID is not required in the PAI
    asserts.assert_true(dac_vid_str is not None, "VID must be present in the DAC")
    asserts.assert_true(dac_pid_str is not None, "PID must be present in the DAC")
    asserts.assert_true(pai_vid_str is not None, "VID must be present in the PAI")

    dac_vid = int(dac_vid_str, 16)
    dac_pid = int(dac_pid_str, 16)
    pai_vid = int(pai_vid_str, 16)
    if pai_pid_str:
        pai_pid = int(pai_pid_str, 16)
    else:
        pai_pid = None

    return dac_vid, dac_pid, pai_vid, pai_pid

# To set the directory for the CD certificates use
# --string-arg cd_cert_dir:'your_directory_name'
# ex. --string-arg cd_cert_dir:'credentials/development/cd-certs'
# default is 'credentials/development/cd-certs'.


class TC_DA_1_2(MatterBaseTest, BasicCompositionTests):
    def desc_TC_DA_1_2(self):
        return "Device Attestation Request Validation [DUT - Commissionee]"

    def steps_TC_DA_1_2(self):
        return [TestStep(0, "Commission DUT if not done", is_commissioning=True),
                TestStep(1, "TH1 generates 32-byte AttestationNonce and saves as `nonce", ""),
                TestStep(2, "TH1 sends AttestationRequest Command to the DUT with AttestationNonce set to `nonce`",
                         "Verify AttestationResponse is received"),
                TestStep("3a", "TH1 sends CertificateChainRequest Command with CertificateType field set to DACCertificate (1) to DUT to obtain DAC",
                         "DUT responds with CertificateChainResponse the DAC certificate in X.509v3 format with size ⇐ 600 bytes"),
                TestStep("3b", "TH1 sends CertificateChainRequest Command with CertificateType field set to PAICertificate (2) to DUT to obtain PAI",
                         "DUT responds with CertificateChainResponse the PAI certificate in X.509v3 format with size ⇐ 600 bytes"),
                TestStep("4a", "TH1 Reads the VendorID attribute of the Basic Information cluster and saves it as `basic_info_vendor_id`"),
                TestStep("4b", "TH1 Reads the ProductID attribute of the Basic Information cluster and saves it as `basic_info_product_id`"),
                TestStep(5, "Extract the attestation_elements_message structure fields from the AttestationResponse"),
                TestStep("5.1", "Verify AttestationElements field size",
                         "AttestationElements field size should not be greater than RESP_MAX(900 bytes)"),
                TestStep("5.2", "Verify certification_declaration format",
                         "certification_declaration is present and is an octet string representation CMS-format certification declaration, as described in section 6.3.1"),
                TestStep("6.1", "Verify CD format_version", "format_version = 1"),
                TestStep("6.2", "Verify CD vendor_id",
                         "vendor_id field matches `basic_info_vendor_id` and is in the standard vendor ID range"),
                TestStep("6.3", "Verify CD product_id_array", "product_id_array field contains `basic_info_product_id`"),
                TestStep("6.4", "Verify CD device_type_id", "device_type_id has a value between 0 and (2^31 - 1)"),
                TestStep("6.5", "Verify CD certificate_id", "certificate_id has a length of 19"),
                TestStep("6.6", "Verify CD security level", "security level = 0"),
                TestStep("6.7", "Verify CD security_information", "security_information = 0"),
                TestStep("6.8", "Verify CD version_number", "version_number is an integer in range 0..65535"),
                TestStep("6.9", "Verify CD certification_type", "certification_type has a value between 1..2"),
                TestStep("7.0", "Extract the Vendor ID (VID) and Product ID (PID) from the DAC. Extract the VID from the PAI. Extract the PID from the PAI, if present",
                         "VID and PID are present and properly encoded in the DAC. VID is present and properly encoded in the PAI. If the PID is present in the PAI, it is properly encoded"),
                TestStep("7.1", "", "If the dac_origin_vendor_id is present in the CD, confirm the dac_origin_product_id is also present. If the dac_origin_vendor_id is not present in the CD, confirm the dac_origin_product_id is also not present."),
                TestStep("7.2", "If the Certification Declaration has both the dac_origin_vendor_id and the dac_origin_product_id fields, verify dac_origin fields",
                         ("* The Vendor ID (VID) in the DAC subject and PAI subject are the same as the dac_origin_vendor_id field in the Certification Declaration.\n"
                          "* The Product ID (PID) in the DAC subject is same as the dac_origin_product_id field in the Certification Declaration.\n"
                          "* If it is present in the PAI certificate, the Product ID (PID) in the subject is same as the dac_origin_product_id field in the Certification Declaration.\n")),
                TestStep("7.3", "If the Certification Declaration has neither the dac_origin_vendor_id nor the dac_origin_product_id fields, verify the vendor_id and product_id_array fields",
                         ("* The Vendor ID (VID) in the DAC subject and PAI subject are the same as the vendor_id field in the Certification Declaration.\n"
                          "* The Product ID (PID) subject DN in the DAC is contained in the product_id_array field in the Certification Declaration.\n"
                          "* If it is present in the PAI certificate, the Product ID (PID) in the subject is contained in the product_id_array field in the Certification Declaration.\n")),
                TestStep(8, "If the Certification Declaration has authorized_paa_list, check that the authority_key_id extension of the PAI matches one found in the authorized_paa_list",
                         "PAA from PAI authority_key_id extension matches one found in authorized_paa_list"),
                TestStep(9, "Verify that the certification_declaration CMS enveloped can be verified with the well-known Certification Declaration public key used to originally sign the Certification Declaration", "Signature verification passes"),
                TestStep(10, "Verify attestation_nonce", ("* attestation_nonce is present in the attestation_elements_message structure\n"
                                                          "* attestation_nonce value matches the AttestationNonce field value sent in the AttestationRequest Command sent by the commissioner\n"
                                                          "* attestation_nonce is a 32 byte-long octet string\n")),
                TestStep(11, "If firmware_information is present, verify firmware information type",
                         "firmware_information is an octet string"),
                TestStep(12, "Using Crypto_Verify cryptographic primitive, validate that the AttestationSignature from the AttestationResponse Command is valid if verified against a message constructed by concatenating AttestationElements with the attestation challenge associated with the secure session over which the AttestationResponse was obtained, using the subject public key found in the DAC.", "Signature is valid"),
                TestStep(13, "TH1 sends AttestationRequest Command with Invalid AttestationNonce (size > 32 bytes) as the field to the DUT.",
                         "Verify DUT responds w/ status INVALID_COMMAND(0x85)"),
                TestStep(14, "TH1 sends AttestationRequest Command with invalid AttestationNonce (size < 32 bytes) as the field to the DUT.",
                         "Verify that the DUT reports an INVALID_COMMAND error"),
                ]

    @async_test_body
    async def test_TC_DA_1_2(self):
        cd_cert_dir = self.user_params.get("cd_cert_dir", 'credentials/development/cd-certs')
        post_cert_test = self.user_params.get("post_cert_test", False)

        do_test_over_pase = self.user_params.get("use_pase_only", False)
        if do_test_over_pase:
            self.connect_over_pase(self.default_controller)

        # Commissioning - done
        self.step(0)

        opcreds = Clusters.Objects.OperationalCredentials
        basic = Clusters.Objects.BasicInformation

        self.step(1)
        nonce = random.randbytes(32)

        self.step(2)
        attestation_resp = await self.send_single_cmd(cmd=opcreds.Commands.AttestationRequest(attestationNonce=nonce))
        asserts.assert_true(type_matches(attestation_resp, opcreds.Commands.AttestationResponse),
                            "DUT returned invalid response to AttestationRequest")

        self.step("3a")
        type = opcreds.Enums.CertificateChainTypeEnum.kDACCertificate
        dac_resp = await self.send_single_cmd(cmd=opcreds.Commands.CertificateChainRequest(certificateType=type))
        asserts.assert_true(type_matches(dac_resp, opcreds.Commands.CertificateChainResponse),
                            "DUT returned invalid response to CertificateChainRequest")
        der_dac = dac_resp.certificate
        asserts.assert_less_equal(len(der_dac), 600, "Returned DAC is > 600 bytes")
        # This throws an exception for a non-x509 cert
        try:
            parsed_dac = x509.load_der_x509_certificate(der_dac)
        except ValueError:
            asserts.assert_true(False, "Unable to parse certificate from CertificateChainResponse")
        asserts.assert_equal(parsed_dac.version, x509.Version.v3, "DUT returned incorrect certificate type")

        self.step("3b")
        type = opcreds.Enums.CertificateChainTypeEnum.kPAICertificate
        pai_resp = await self.send_single_cmd(cmd=opcreds.Commands.CertificateChainRequest(certificateType=type))
        asserts.assert_true(type_matches(pai_resp, opcreds.Commands.CertificateChainResponse),
                            "DUT returned invalid response to CertificateChainRequest")
        der_pai = pai_resp.certificate
        asserts.assert_less_equal(len(der_pai), 600, "Returned PAI is > 600 bytes")
        # This throws an exception for a non-x509 cert
        try:
            parsed_pai = x509.load_der_x509_certificate(der_pai)
        except ValueError:
            asserts.assert_true(False, "Unable to parse certificate from CertificateChainResponse")
        asserts.assert_equal(parsed_pai.version, x509.Version.v3, "DUT returned incorrect certificate type")

        self.step("4a")
        basic_info_vendor_id = await self.read_single_attribute_check_success(basic, basic.Attributes.VendorID)

        self.step("4b")
        basic_info_product_id = await self.read_single_attribute_check_success(basic, basic.Attributes.ProductID)

        self.step(5)
        elements = attestation_resp.attestationElements
        signature_attestation_raw = attestation_resp.attestationSignature

        self.step("5.1")
        asserts.assert_less_equal(len(elements), 900, "AttestationElements field is more than 900 bytes")

        self.step("5.2")
        decoded = TLVReader(elements).get()["Any"]
        # Certification declaration is tag 1
        asserts.assert_in(1, decoded.keys(), "CD is not present in the attestation elements")
        cd_der = decoded[1]

        try:
            temp, _ = der_decoder(cd_der, asn1Spec=rfc5652.ContentInfo())
        except PyAsn1Error:
            asserts.fail("Unable to decode CD - improperly encoded DER")

        # turn this into a dict so I don't have to keep parsing tuples
        layer1 = dict(temp)

        id_sha256 = univ.ObjectIdentifier('2.16.840.1.101.3.4.2.1')

        asserts.assert_equal(layer1['contentType'], rfc5652.id_signedData, "Incorrect object type")

        # uh, is this actually right? Doesn't the spec say enveloped data?
        temp, _ = der_decoder(layer1['content'].asOctets(), asn1Spec=rfc5652.SignedData())

        signed_data = dict(temp)

        asserts.assert_equal(signed_data['version'], 3, "Signed data version is not 3")
        asserts.assert_equal(len(signed_data['digestAlgorithms']), 1, "More than one digest algorithm listed")

        # DigestAlgorithmIdentifier
        algo_id = dict(signed_data['digestAlgorithms'][0])
        asserts.assert_equal(algo_id['algorithm'], id_sha256, "Reported digest algorithm is not SHA256")

        encap_content_info = dict(signed_data['encapContentInfo'])

        id_pkcs7_data = univ.ObjectIdentifier('1.2.840.113549.1.7.1')
        asserts.assert_equal(encap_content_info['eContentType'], id_pkcs7_data, "Incorrect encapsulated content type")

        cd_tlv = bytes(encap_content_info['eContent'])

        # Check the signer info
        # There should be only one signer info
        asserts.assert_equal(len(signed_data['signerInfos']), 1, "Too many signer infos provided")

        # version should be 3
        signer_info = dict(signed_data['signerInfos'][0])
        asserts.assert_equal(signer_info['version'], 3, "Incorrect version on signer info")

        # subject key identifier needs to match the connectivity standards aliance key
        subject_key_identifier = bytes(dict(signer_info['sid'])['subjectKeyIdentifier'])

        # digest algorithm is sha256, only one allowed
        algo_id = dict(signer_info['digestAlgorithm'])
        asserts.assert_equal(algo_id['algorithm'], id_sha256, "Incorrect digest algorithm for the signer info")

        # signature algorithm is ecdsa-with-sha256
        id_ecdsa_with_sha256 = univ.ObjectIdentifier('1.2.840.10045.4.3.2')
        algo_id = dict(signer_info['signatureAlgorithm'])
        asserts.assert_equal(algo_id['algorithm'], id_ecdsa_with_sha256, "Incorrect signature algorithm")

        self.step("6.1")
        # First, lets parse it
        cd = TLVReader(cd_tlv).get()["Any"]
        format_version = cd[0]
        vendor_id = cd[1]
        product_id_array = cd[2]
        device_type_id = cd[3]
        certificate_id = cd[4]
        security_level = cd[5]
        security_info = cd[6]
        version_number = cd[7]
        certification_type = cd[8]

        asserts.assert_equal(format_version, 1, "Format version is incorrect")
        self.step("6.2")
        asserts.assert_equal(vendor_id, basic_info_vendor_id, "Vendor ID is incorrect")
        if not self.is_pics_sdk_ci_only:
            asserts.assert_in(vendor_id, range(1, 0xfff0), "Vendor ID is out of range")
        self.step("6.3")
        asserts.assert_true(basic_info_product_id in product_id_array, "Product ID not found in CD product array")
        self.step("6.4")
        asserts.assert_in(device_type_id, range(0, (2**31)-1), "Device type ID is out of range")
        self.step("6.5")
        asserts.assert_equal(len(certificate_id), 19, "Certificate id is the incorrect length")
        self.step("6.6")
        asserts.assert_equal(security_level, 0, "Incorrect value for security level")
        self.step("6.7")
        asserts.assert_equal(security_info, 0, "Incorrect value for security information")
        self.step("6.8")
        asserts.assert_in(version_number, range(0, 65535), "Version number out of range")
        self.step("6.9")
        if post_cert_test:
            asserts.assert_equal(certification_type, 2, "Certification declaration is not marked as production.")
        elif self.is_pics_sdk_ci_only:
            asserts.assert_in(certification_type, [0, 1, 2], "Certification type is out of range")
        else:
            asserts.assert_in(certification_type, [1, 2], "Certification type is out of range")

        self.step("7.0")
        dac_vid, dac_pid, pai_vid, pai_pid = parse_ids_from_certs(parsed_dac, parsed_pai)

        self.step("7.1")
        has_origin_vid = 9 in cd.keys()
        has_origin_pid = 10 in cd.keys()
        if has_origin_pid != has_origin_vid:
            asserts.fail("Found one of origin PID or VID in CD but not both")

        self.step("7.2")
        if has_origin_vid:
            origin_vid = cd[9]
            origin_pid = cd[10]

            asserts.assert_equal(dac_vid, origin_vid, "Origin Vendor ID in the CD does not match the Vendor ID in the DAC")
            asserts.assert_equal(pai_vid, origin_vid, "Origin Vendor ID in the CD does not match the Vendor ID in the PAI")
            asserts.assert_equal(dac_pid, origin_pid, "Origin Product ID in the CD does not match the Product ID in the DAC")
            if pai_pid:
                asserts.assert_equal(pai_pid, origin_pid, "Origin Product ID in the CD does not match the Product ID in the PAI")
        else:
            self.mark_current_step_skipped()

        self.step("7.3")
        if not has_origin_vid:
            asserts.assert_equal(dac_vid, vendor_id, "Vendor ID in the CD does not match the Vendor ID in the DAC")
            asserts.assert_equal(pai_vid, vendor_id, "Vendor ID in the CD does not match the Vendor ID in the PAI")
            asserts.assert_in(dac_pid, product_id_array, "Product ID from the DAC is not present in the PID list in the CD")
            if pai_pid:
                asserts.assert_in(pai_pid, product_id_array, "Product ID from the PAI is not present in the PID list in the CD")
        else:
            self.mark_current_step_skipped()

        self.step(8)
        has_paa_list = 11 in cd.keys()

        if has_paa_list:
            akids = [ext.value.key_identifier for ext in parsed_pai.extensions if ext.oid == ExtensionOID.AUTHORITY_KEY_IDENTIFIER]
            asserts.assert_equal(len(akids), 1, "PAI requires exactly one AuthorityKeyIdentifier")
            paa_authority_list = cd[11]
            asserts.assert_in(akids[0], paa_authority_list, "PAI AKID not found in the authority list")
        else:
            self.mark_current_step_skipped()

        self.step(9)
        signature_cd = bytes(signer_info['signature'])
        certs = {}
        for filename in os.listdir(cd_cert_dir):
            if '.der' not in filename:
                continue
            with open(os.path.join(cd_cert_dir, filename), 'rb') as f:
                logging.info(f'Parsing CD signing certificate file: {filename}')
                try:
                    cert = x509.load_der_x509_certificate(f.read())
                except ValueError:
                    logging.info(f'File {filename} is not a valid certificate, skipping')
                    pass
                pub = cert.public_key()
                ski = x509.SubjectKeyIdentifier.from_public_key(pub).digest
                certs[ski] = pub

        asserts.assert_true(subject_key_identifier in certs.keys(), "Subject key identifier not found in CD certs")
        try:
            certs[subject_key_identifier].verify(signature=signature_cd, data=cd_tlv,
                                                 signature_algorithm=ec.ECDSA(hashes.SHA256()))
        except InvalidSignature:
            asserts.fail("Failed to verify CD signature against known CD public key")

        self.step(10)
        asserts.assert_in(2, decoded.keys(), "Attestation nonce is not present in the attestation elements")
        returned_nonce = decoded[2]
        asserts.assert_equal(returned_nonce, nonce, "Returned attestation nonce does not match request nonce")
        asserts.assert_equal(len(returned_nonce), 32, "Returned nonce is incorrect size")

        self.step(11)
        has_firmware_information = 4 in decoded.keys()
        if has_firmware_information:
            try:
                int(decoded[4], 16)
            except ValueError:
                asserts.fail("Firmware is not an octet string")
        else:
            self.mark_current_step_skipped()

        self.step(12)
        proxy = self.default_controller.GetConnectedDeviceSync(self.dut_node_id, do_test_over_pase)
        asserts.assert_equal(len(proxy.attestationChallenge), 16, "Attestation challenge is the wrong length")
        attestation_tbs = elements + proxy.attestationChallenge

        # signature is a struct of r and s - see 3.5.3
        # Actual curve is secp256r1 / NIST P-256 per 2.7
        baselen = curve_by_name("NIST256p").baselen
        signature_attestation_raw_r = int(hex_from_bytes(signature_attestation_raw[:baselen]), 16)
        signature_attestation_raw_s = int(hex_from_bytes(signature_attestation_raw[baselen:]), 16)

        signature_attestation = utils.encode_dss_signature(signature_attestation_raw_r, signature_attestation_raw_s)

        parsed_dac.public_key().verify(signature=signature_attestation, data=attestation_tbs,
                                       signature_algorithm=ec.ECDSA(hashes.SHA256()))

        self.step(13)
        nonce = random.randbytes(33)
        try:
            await self.send_single_cmd(cmd=opcreds.Commands.AttestationRequest(attestationNonce=nonce))
            asserts.fail("Received Success response when an INVALID_COMMAND was expected")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand, "Received incorrect error from AttestationRequest command")

        self.step(14)
        nonce = random.randbytes(31)
        try:
            await self.send_single_cmd(cmd=opcreds.Commands.AttestationRequest(attestationNonce=nonce))
            asserts.fail("Received Success response when an INVALID_COMMAND was expected")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand, "Received incorrect error from AttestationRequest command")


if __name__ == "__main__":
    default_matter_test_main()
