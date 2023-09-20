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

import os
import random

import chip.clusters as Clusters
from chip.interaction_model import InteractionModelError, Status
from chip.tlv import TLVReader
from cryptography import x509
from cryptography.exceptions import InvalidSignature
from cryptography.hazmat._oid import ExtensionOID
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec, utils
from ecdsa.curves import curve_by_name
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, hex_from_bytes, type_matches
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
    if not s.isupper() or len(s) != 4:
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


class TC_DA_1_2(MatterBaseTest):
    @async_test_body
    async def test_TC_DA_1_2(self):
        is_ci = self.check_pics('PICS_SDK_CI_ONLY')
        # These PICS will be ignored on the CI because we're going to test a bunch of combos
        pics_origin_pid = self.check_pics('MCORE.DA.CERTDECL_ORIGIN_PRODUCTID')
        pics_origin_vid = self.check_pics('MCORE.DA.CERTDECL_ORIGIN_VENDORID')
        pics_paa_list = self.check_pics('MCORE.DA.CERTDECL_AUTH_PAA')
        pics_firmware_info = self.check_pics('MCORE.DA.ATTESTELEMENT_FW_INFO')
        if pics_origin_pid != pics_origin_vid:
            asserts.fail("MCORE.DA.CERTDECL_ORIGIN_PRODUCTID and MCORE.DA.CERTDECL_ORIGIN_VENDORID PICS codes must match")

        cd_cert_dir = self.user_params.get("cd_cert_dir", 'credentials/development/cd-certs')

        self.print_step(0, "Commissioning, already done")

        opcreds = Clusters.Objects.OperationalCredentials
        basic = Clusters.Objects.BasicInformation

        self.print_step(1, "Generate 32-byte nonce")
        nonce = random.randbytes(32)

        self.print_step(2, "Send AttestationRequest")
        attestation_resp = await self.send_single_cmd(cmd=opcreds.Commands.AttestationRequest(attestationNonce=nonce))

        self.print_step("3a", "Verify AttestationResponse is correct type")
        asserts.assert_true(type_matches(attestation_resp, opcreds.Commands.AttestationResponse),
                            "DUT returned invalid response to AttestationRequest")

        self.print_step("3b", "Send CertificateChainRequest for DAC")
        type = opcreds.Enums.CertificateChainTypeEnum.kDACCertificate
        dac_resp = await self.send_single_cmd(cmd=opcreds.Commands.CertificateChainRequest(certificateType=type))

        self.print_step("3c", "Verify DAC is x509v3 and <= 600 bytes")
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

        self.print_step("3d", "Send CertificateChainRequest for PAI and verifies PAI is x509v3 and <= 600 bytes")
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

        self.print_step("3e", "TH1 saves PAI")
        # already saved above

        self.print_step("4a", "Read VendorID from basic info")
        basic_info_vendor_id = await self.read_single_attribute_check_success(basic, basic.Attributes.VendorID)

        self.print_step("4b", "Read ProductID from basic info")
        basic_info_product_id = await self.read_single_attribute_check_success(basic, basic.Attributes.ProductID)

        self.print_step(5, "Extract the attestation_elements_message")
        elements = attestation_resp.attestationElements

        self.print_step(6, "Verify the AttestationResponse has the following fields")
        # OK, it's a bit weird that we're doing this after extracting the elements already, but sure.
        # We type checked earlier, but let's grab the signature here.
        signature_attestation_raw = attestation_resp.attestationSignature

        self.print_step(7, "Read the attestation_elements_message structure fields")
        # Already done

        self.print_step(8, "Verify that the attestation_elements_message structure fields satisfy the following conditions")
        # Not sure why this is a separate step, but I'm ready...let's check.

        self.print_step("8.1", "Verify attestation elements size is < = 900 bytes")
        asserts.assert_less_equal(len(elements), 900, "AttestationElements field is more than 900 bytes")

        self.print_step("8.2", "Verify certification declaration is present and follows spec format")
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

        # version shoule be 3
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

        self.print_step("8.3", "Verify mandatory cd contents")
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
        asserts.assert_equal(vendor_id, basic_info_vendor_id, "Vendor ID is incorrect")
        if not is_ci:
            asserts.assert_in(vendor_id, range(1, 0xfff0), "Vendor ID is out of range")
        asserts.assert_true(basic_info_product_id in product_id_array, "Product ID not found in CD product array")
        asserts.assert_in(device_type_id, range(0, (2**31)-1), "Device type ID is out of range")
        asserts.assert_equal(len(certificate_id), 19, "Certificate id is the incorrect length")
        asserts.assert_equal(security_level, 0, "Incorrect value for security level")
        asserts.assert_equal(security_info, 0, "Incorrect value for security information")
        asserts.assert_in(version_number, range(0, 65535), "Version number out of range")
        if is_ci:
            asserts.assert_in(certification_type, [0, 1, 2], "Certification type is out of range")
        else:
            asserts.assert_in(certification_type, [1, 2], "Certification type is out of range")

        self.print_step("8.4", "Confirm that both dac_origin_vendor_id and dac_origin_product_id are present")
        if not is_ci and pics_origin_vid:
            asserts.assert_in(9, cd.keys(), "Origin vendor ID not found in cert")
            asserts.assert_in(10, cd.keys(), "Origin product ID not found in cert")

        self.print_step("8.5", "Confirm that neither dac_origin_vendor_id nor dac_origin_product_id are present")
        if not is_ci and not pics_origin_vid:
            asserts.assert_not_in(9, cd.keys(), "Origin vendor ID found in cert")
            asserts.assert_not_in(10, cd.keys(), "Origin product ID found in cert")

        dac_vid, dac_pid, pai_vid, pai_pid = parse_ids_from_certs(parsed_dac, parsed_pai)

        self.print_step("8.6", "Check origin PID/VID against DAC and PAI")
        has_origin_vid = 9 in cd.keys()
        has_origin_pid = 10 in cd.keys()
        if not is_ci and has_origin_vid != pics_origin_vid:
            asserts.fail("Origin VID in CD does not match PICS")
        if not is_ci and has_origin_pid and not pics_origin_pid:
            asserts.fail("Origin PID in CD does not match PICS")
        if has_origin_pid != has_origin_vid:
            asserts.fail("Found one of origin PID or VID in CD but not both")

        # If this is the CI, ignore the PICS, we're going to try many cases.
        if has_origin_vid:
            origin_vid = cd[9]
            origin_pid = cd[10]

            asserts.assert_equal(dac_vid, origin_vid, "Origin Vendor ID in the CD does not match the Vendor ID in the DAC")
            asserts.assert_equal(pai_vid, origin_vid, "Origin Vendor ID in the CD does not match the Vendor ID in the PAI")
            asserts.assert_equal(dac_pid, origin_pid, "Origin Product ID in the CD does not match the Product ID in the DAC")
            if pai_pid:
                asserts.assert_equal(pai_pid, origin_pid, "Origin Product ID in the CD does not match the Product ID in the PAI")

        self.print_step("8.7", "Check CD PID/VID against DAC and PAI")
        if not has_origin_vid:
            asserts.assert_equal(dac_vid, vendor_id, "Vendor ID in the CD does not match the Vendor ID in the DAC")
            asserts.assert_equal(pai_vid, vendor_id, "Vendor ID in the CD does not match the Vendor ID in the PAI")
            asserts.assert_in(dac_pid, product_id_array, "Product ID from the DAC is not present in the PID list in the CD")
            if pai_pid:
                asserts.assert_in(pai_pid, product_id_array, "Product ID from the PAI is not present in the PID list in the CD")

        self.print_step("8.8", "Check PAAs")
        has_paa_list = 11 in cd.keys()
        if not is_ci and pics_paa_list != has_paa_list:
            asserts.fail("PAA list does not match PICS")

        if has_paa_list:
            akids = [ext.value.key_identifier for ext in parsed_pai.extensions if ext.oid == ExtensionOID.AUTHORITY_KEY_IDENTIFIER]
            asserts.assert_equal(len(akids), 1, "PAI requires exactly one AuthorityKeyIdentifier")
            paa_authority_list = cd[11]
            asserts.assert_in(akids[0], paa_authority_list, "PAI AKID not found in the authority list")

        self.print_step("8.9", "Check signature")
        signature_cd = bytes(signer_info['signature'])
        certs = {}
        for filename in os.listdir(cd_cert_dir):
            if '.der' not in filename:
                continue
            with open(os.path.join(cd_cert_dir, filename), 'rb') as f:
                cert = x509.load_der_x509_certificate(f.read())
                pub = cert.public_key()
                ski = x509.SubjectKeyIdentifier.from_public_key(pub).digest
                certs[ski] = pub

        asserts.assert_true(subject_key_identifier in certs.keys(), "Subject key identifier not found in CD certs")
        try:
            certs[subject_key_identifier].verify(signature=signature_cd, data=cd_tlv,
                                                 signature_algorithm=ec.ECDSA(hashes.SHA256()))
        except InvalidSignature:
            asserts.fail("Failed to verify CD signature against known CD public key")

        self.print_step(9, "Verify nonce")
        asserts.assert_in(2, decoded.keys(), "Attestation nonce is not present in the attestation elements")
        returned_nonce = decoded[2]
        asserts.assert_equal(returned_nonce, nonce, "Returned attestation nonce does not match request nonce")
        asserts.assert_equal(len(returned_nonce), 32, "Returned nonce is incorrect size")

        self.print_step(10, "Verify firmware")
        has_firmware_information = 4 in decoded.keys()
        if not is_ci and has_firmware_information != pics_firmware_info:
            asserts.fail("PICS for firmware information does not match returned value")
        if has_firmware_information:
            try:
                int(decoded[4], 16)
            except ValueError:
                asserts.fail("Firmware is not an octet string")

        self.print_step(11, "Verify that the signature for the attestation response is valid")
        proxy = self.default_controller.GetConnectedDeviceSync(self.dut_node_id, False)
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

        self.print_step(12, "Send AttestationRequest with nonce > 32 bytes")
        nonce = random.randbytes(33)
        try:
            await self.send_single_cmd(cmd=opcreds.Commands.AttestationRequest(attestationNonce=nonce))
            asserts.fail("Received Success response when an INVALID_COMMAND was expected")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand, "Received incorrect error from AttestationRequest command")

        self.print_step(13, "Send AttestationRequest with nonce < 32 bytes")
        nonce = random.randbytes(31)
        try:
            await self.send_single_cmd(cmd=opcreds.Commands.AttestationRequest(attestationNonce=nonce))
            asserts.fail("Received Success response when an INVALID_COMMAND was expected")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand, "Received incorrect error from AttestationRequest command")


if __name__ == "__main__":
    default_matter_test_main()
