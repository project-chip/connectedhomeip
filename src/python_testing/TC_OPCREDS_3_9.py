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

# This test requires a DUT that signals the provisional PQCDA feature of the
# Operational Credentials cluster. There is no CI test arguments block because
# no PQC-capable DAC provider fixture is checked in yet; run it manually against
# a Linux example app started with:
#
#   ./chip-all-clusters-app --dac_provider <pqc_dac_provider.json> --dac_provider_pqc_ready
#
# where the JSON carries the dac_cert_ml_dsa_44/65 and pai_cert_ml_dsa_44/65 keys
# read by TestHarnessDACProvider.
#
# Steps 7 and 8 assert the INVALID_COMMAND outcome the test plan requires. The SDK server in
# src/app/clusters/operational-credentials-server does not produce it today: an out-of-range
# SegmentID surfaces CHIP_ERROR_INVALID_ARGUMENT from the provider, which the data model maps to
# FAILURE, and an oversized MaxSegmentSize is accepted because the only bound the handler checks is
# the 600-byte lower one. Both steps are expected to fail until the server is updated.

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

# Operational Credentials FeatureMap bit 0 (PQCDA). The ZAP Python code generator has not
# emitted Clusters.OperationalCredentials.Bitmaps.Feature yet, so the bit is spelled out here.
kFeaturePqcDeviceAttestation = 0x01

# Every segment the DUT emits is capped at this size, and the spec sets it as the floor for
# MaxSegmentSize because it is the payload a Matter transport is always guaranteed to carry.
kCertificateSegmentSize = 600

# A MaxSegmentSize that no Matter transport can carry within a single message (test step 8).
kOversizedMaxSegmentSize = 0xFFFF

# CertificateType values outside CertificateChainTypeEnum (test step 9).
kInvalidCertificateType = 0x03

# X.509 AlgorithmIdentifier OIDs for each attestation crypto profile. Per
# draft-ietf-lamps-dilithium-certificates an ML-DSA certificate carries the same OID in its
# signatureAlgorithm and in its subjectPublicKeyInfo algorithm.
kOidEcPublicKey = "1.2.840.10045.2.1"
kOidEcdsaWithSha256 = "1.2.840.10045.4.3.2"
kOidMlDsa44 = "2.16.840.1.101.3.4.3.17"
kOidMlDsa65 = "2.16.840.1.101.3.4.3.18"

# DER tags used while walking a certificate.
kDerTagInteger = 0x02
kDerTagBitString = 0x03
kDerTagObjectIdentifier = 0x06
kDerTagSequence = 0x30
kDerTagContextExplicit0 = 0xA0


def _der_read_element(data: bytes, offset: int) -> tuple[int, int, int, int]:
    """Read one DER TLV element starting at `offset`.

    Returns (tag, content_offset, content_length, next_offset). Only the single-byte tag and
    definite-length forms that X.509 uses are supported.
    """
    asserts.assert_less(offset, len(data), "Truncated DER element header")
    tag = data[offset]
    asserts.assert_not_equal(tag & 0x1F, 0x1F, "Multi-byte DER tags are not valid in X.509 certificates")

    length_offset = offset + 1
    asserts.assert_less(length_offset, len(data), "Truncated DER length")
    first_length_byte = data[length_offset]

    if first_length_byte < 0x80:
        length = first_length_byte
        content_offset = length_offset + 1
    else:
        length_byte_count = first_length_byte & 0x7F
        asserts.assert_not_equal(length_byte_count, 0, "Indefinite DER lengths are not valid in DER")
        asserts.assert_less_equal(length_offset + 1 + length_byte_count, len(data), "Truncated DER long-form length")
        length = int.from_bytes(data[length_offset + 1:length_offset + 1 + length_byte_count], byteorder="big")
        content_offset = length_offset + 1 + length_byte_count

    asserts.assert_less_equal(content_offset + length, len(data), "DER element extends past the end of the document")
    return tag, content_offset, length, content_offset + length


def _der_children(data: bytes, content_offset: int, content_length: int) -> list[tuple[int, int, int]]:
    """Return [(tag, content_offset, content_length)] for every direct child of a constructed element."""
    children = []
    offset = content_offset
    end = content_offset + content_length
    while offset < end:
        tag, child_offset, child_length, offset = _der_read_element(data, offset)
        children.append((tag, child_offset, child_length))
    asserts.assert_equal(offset, end, "DER children do not exactly fill their parent")
    return children


def _der_decode_oid(data: bytes, content_offset: int, content_length: int) -> str:
    """Decode a DER OBJECT IDENTIFIER body into its dotted-decimal representation."""
    asserts.assert_greater(content_length, 0, "Empty OBJECT IDENTIFIER")
    encoded = data[content_offset:content_offset + content_length]

    arcs = [str(encoded[0] // 40), str(encoded[0] % 40)]
    value = 0
    for index, byte in enumerate(encoded[1:], start=1):
        value = (value << 7) | (byte & 0x7F)
        if byte & 0x80:
            asserts.assert_less(index, content_length - 1, "OBJECT IDENTIFIER ends mid-arc")
            continue
        arcs.append(str(value))
        value = 0

    return ".".join(arcs)


def parse_certificate_algorithm_oids(der: bytes) -> tuple[str, str]:
    """Extract (signature_algorithm_oid, subject_public_key_algorithm_oid) from an X.509 certificate.

    The OIDs are pulled straight out of the DER rather than through the `cryptography` package
    because the installed version may not recognize the ML-DSA algorithm identifiers.
    """
    tag, content_offset, content_length, next_offset = _der_read_element(der, 0)
    asserts.assert_equal(tag, kDerTagSequence, "Certificate is not a DER SEQUENCE")
    asserts.assert_equal(next_offset, len(der), "Trailing bytes after the certificate SEQUENCE")

    certificate = _der_children(der, content_offset, content_length)
    asserts.assert_equal(len(certificate), 3,
                         "Certificate must hold tbsCertificate, signatureAlgorithm and signatureValue")

    tbs_tag, tbs_offset, tbs_length = certificate[0]
    signature_algorithm_tag, signature_algorithm_offset, signature_algorithm_length = certificate[1]
    asserts.assert_equal(tbs_tag, kDerTagSequence, "tbsCertificate is not a DER SEQUENCE")
    asserts.assert_equal(signature_algorithm_tag, kDerTagSequence, "signatureAlgorithm is not a DER SEQUENCE")
    asserts.assert_equal(certificate[2][0], kDerTagBitString, "signatureValue is not a DER BIT STRING")

    signature_algorithm = _der_children(der, signature_algorithm_offset, signature_algorithm_length)
    asserts.assert_greater_equal(len(signature_algorithm), 1, "signatureAlgorithm carries no algorithm OID")
    asserts.assert_equal(signature_algorithm[0][0], kDerTagObjectIdentifier,
                         "signatureAlgorithm does not start with an OBJECT IDENTIFIER")
    signature_algorithm_oid = _der_decode_oid(der, signature_algorithm[0][1], signature_algorithm[0][2])

    # tbsCertificate ::= [0] version DEFAULT v1, serialNumber, signature, issuer, validity,
    #                    subject, subjectPublicKeyInfo, ...
    tbs = _der_children(der, tbs_offset, tbs_length)
    subject_public_key_info_index = 6 if tbs[0][0] == kDerTagContextExplicit0 else 5
    if subject_public_key_info_index == 5:
        asserts.assert_equal(tbs[0][0], kDerTagInteger, "tbsCertificate must start with version or serialNumber")
    asserts.assert_greater(len(tbs), subject_public_key_info_index, "tbsCertificate has no subjectPublicKeyInfo")

    spki_tag, spki_offset, spki_length = tbs[subject_public_key_info_index]
    asserts.assert_equal(spki_tag, kDerTagSequence, "subjectPublicKeyInfo is not a DER SEQUENCE")
    spki = _der_children(der, spki_offset, spki_length)
    asserts.assert_greater_equal(len(spki), 2, "subjectPublicKeyInfo must hold an algorithm and a public key")
    asserts.assert_equal(spki[0][0], kDerTagSequence, "subjectPublicKeyInfo algorithm is not an AlgorithmIdentifier")

    spki_algorithm = _der_children(der, spki[0][1], spki[0][2])
    asserts.assert_greater_equal(len(spki_algorithm), 1, "subjectPublicKeyInfo AlgorithmIdentifier carries no OID")
    asserts.assert_equal(spki_algorithm[0][0], kDerTagObjectIdentifier,
                         "subjectPublicKeyInfo AlgorithmIdentifier does not start with an OBJECT IDENTIFIER")
    subject_public_key_algorithm_oid = _der_decode_oid(der, spki_algorithm[0][1], spki_algorithm[0][2])

    return signature_algorithm_oid, subject_public_key_algorithm_oid


class TC_OPCREDS_3_9(MatterBaseTest):
    """PQC Device Attestation CertificateChainRequest behavior [DUT-Server]."""

    def desc_TC_OPCREDS_3_9(self) -> str:
        return "[TC-OPCREDS-3.9] PQC Device Attestation CertificateChainRequest behavior [DUT-Server]"

    def pics_TC_OPCREDS_3_9(self) -> list[str]:
        return ["OPCREDS.S", "OPCREDS.S.C02.Rsp", "OPCREDS.S.C03.Tx"]

    def steps_TC_OPCREDS_3_9(self) -> list[TestStep]:
        return [
            TestStep(0, "Commission DUT to TH", is_commissioning=True),
            TestStep(1, "TH reads the DUT FeatureMap.",
                     "TH verifies DUT has PQCDA set."),
            TestStep(2, "TH reads PQCDeviceAttestationProfile.",
                     "TH obtains the advertised profile bitmap set for PAA, PAI and DAC. PAA is stored as "
                     "PAAProfileSupported, PAI is stored as PAIProfileSupported and DAC is stored as "
                     "DACProfileSupported."),
            TestStep(3, "TH sends CertificateChainRequest for PAICertificate and CryptoProfile set to "
                        "PAIProfileSupported. TH sets MaxSegmentSize to a value that requires more than one "
                        "response segment.",
                     "DUT returns TotalDocumentSize and NextSegmentID as required for segmented retrieval."),
            TestStep(4, "TH continues to request the remaining segments for PAI using the returned NextSegmentID "
                        "values until completion.",
                     "Segment numbering is monotonic from zero; TotalDocumentSize is stable across all segments; "
                     "NextSegmentID is present until the final segment and absent on the final segment; the "
                     "reassembled PAI payload exactly matches the declared TotalDocumentSize; the reassembled PAI "
                     "payload parses successfully; the subjectPublicKeyInfo algorithm matches the negotiated "
                     "profile; the PAI signature field matches the OID for the algorithm signaled by "
                     "PAAProfileSupported."),
            TestStep(5, "TH selects the expected profile and sends CertificateChainRequest for DACCertificate with "
                        "CryptoProfile set to DACProfileSupported.",
                     "If PAIProfileSupported is EcdsaMatterLegacy, DUT returns the certificate in a single response "
                     "and omits NextSegmentID. Otherwise DUT returns TotalDocumentSize and NextSegmentID as "
                     "required for segmented retrieval."),
            TestStep(6, "TH continues to request the remaining segments for DAC using the returned NextSegmentID "
                        "values until completion.",
                     "Segment numbering is monotonic from zero; TotalDocumentSize is stable across all segments; "
                     "NextSegmentID is present until the final segment and absent on the final segment; the "
                     "reassembled DAC payload exactly matches the declared TotalDocumentSize; the reassembled DAC "
                     "payload parses successfully; the subjectPublicKeyInfo algorithm matches the negotiated "
                     "profile; the DAC signature field matches the OID for the algorithm signaled by "
                     "PAIProfileSupported."),
            TestStep(7, "TH sends a CertificateChainRequest with a SegmentID that points beyond the available data.",
                     "DUT rejects the request with INVALID_COMMAND."),
            TestStep(8, "TH sends a CertificateChainRequest with a MaxSegmentSize value that cannot fit within the "
                        "message space on the transport under test.",
                     "DUT rejects the request with INVALID_COMMAND."),
            TestStep(9, "TH sends CertificateChainRequest with an invalid CertificateType value.",
                     "DUT rejects the request with INVALID_COMMAND."),
        ]

    def _negotiated_profile(self, supported_profiles: int, chain_element: str):
        """Pick the highest-security attestation profile the DUT advertises for one chain element.

        Pre-condition 2 of the test plan requires the TH to always negotiate for the highest
        security profile the DUT supports.
        """
        bitmap = Clusters.OperationalCredentials.Enums.AttestationCryptoProfileEnum

        # AttestationCryptoProfileBitmap: bit 0 EcdsaMatterLegacy, bit 1 MlDsa44, bit 2 MlDsa65.
        # Clusters.OperationalCredentials.Bitmaps is not generated, so the masks are spelled out.
        for mask, profile in ((0x04, bitmap.kMlDsa65), (0x02, bitmap.kMlDsa44), (0x01, bitmap.kEcdsaMatterLegacy)):
            if supported_profiles & mask:
                logger.info("Negotiated %s profile %s from bitmap 0x%04X", chain_element, profile.name, supported_profiles)
                return profile

        asserts.fail(f"{chain_element}SupportedProfiles (0x{supported_profiles:04X}) advertises no known profile")

    async def _send_certificate_chain_request(self, certificate_type, crypto_profile, segment_id: int,
                                              max_segment_size: int):
        opcreds = Clusters.OperationalCredentials
        response = await self.send_single_cmd(
            cmd=opcreds.Commands.CertificateChainRequest(certificateType=certificate_type,
                                                         cryptoProfile=crypto_profile,
                                                         segmentID=segment_id,
                                                         maxSegmentSize=max_segment_size),
            endpoint=self.root_endpoint)
        asserts.assert_true(isinstance(response, opcreds.Commands.CertificateChainResponse),
                            "DUT returned an invalid response to CertificateChainRequest")
        return response

    async def _expect_invalid_command(self, cmd, description: str):
        try:
            await self.send_single_cmd(cmd=cmd, endpoint=self.root_endpoint)
            asserts.fail(f"DUT accepted a CertificateChainRequest with {description}; INVALID_COMMAND was expected")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.InvalidCommand,
                                 f"DUT must reject a CertificateChainRequest with {description} using INVALID_COMMAND")

    async def _retrieve_remaining_segments(self, certificate_type, crypto_profile, first_response,
                                           document_name: str) -> tuple[bytes, int]:
        """Follow NextSegmentID from `first_response` to the final segment.

        Checks the segment numbering, TotalDocumentSize stability and NextSegmentID presence rules
        along the way. Returns the reassembled document and the number of segments it took.
        """
        total_document_size = first_response.totalDocumentSize
        document = bytearray(first_response.certificate)
        response = first_response
        # Segment 0 was already retrieved by the caller, so the next expected ID is 1.
        expected_segment_id = 1

        while response.nextSegmentID is not None:
            asserts.assert_equal(response.nextSegmentID, expected_segment_id,
                                 f"{document_name} NextSegmentID must be monotonic from zero")

            response = await self._send_certificate_chain_request(certificate_type, crypto_profile,
                                                                  expected_segment_id, kCertificateSegmentSize)
            asserts.assert_equal(response.totalDocumentSize, total_document_size,
                                 f"{document_name} TotalDocumentSize must be stable across all segments")
            asserts.assert_greater(len(response.certificate), 0,
                                   f"{document_name} segment {expected_segment_id} is empty")
            asserts.assert_less_equal(len(response.certificate), kCertificateSegmentSize,
                                      f"{document_name} segment {expected_segment_id} exceeds the requested "
                                      "MaxSegmentSize")
            document += response.certificate
            expected_segment_id += 1
            asserts.assert_less_equal(len(document), total_document_size,
                                      f"{document_name} segments returned more data than TotalDocumentSize")

        asserts.assert_is_none(response.nextSegmentID,
                               f"{document_name} final segment must omit NextSegmentID")
        asserts.assert_equal(len(document), total_document_size,
                             f"Reassembled {document_name} does not match the declared TotalDocumentSize")
        return bytes(document), expected_segment_id

    def _verify_certificate_algorithms(self, der: bytes, document_name: str, subject_profile, issuer_profile) -> None:
        """Check that a reassembled certificate parses and carries the negotiated algorithm OIDs.

        `subject_profile` is the profile negotiated for this document (it governs the
        subjectPublicKeyInfo algorithm) and `issuer_profile` is the profile of the certificate that
        signed it (it governs the signatureAlgorithm).
        """
        bitmap = Clusters.OperationalCredentials.Enums.AttestationCryptoProfileEnum
        expected_public_key_oids = {
            bitmap.kEcdsaMatterLegacy: kOidEcPublicKey,
            bitmap.kMlDsa44: kOidMlDsa44,
            bitmap.kMlDsa65: kOidMlDsa65,
        }
        expected_signature_oids = {
            bitmap.kEcdsaMatterLegacy: kOidEcdsaWithSha256,
            bitmap.kMlDsa44: kOidMlDsa44,
            bitmap.kMlDsa65: kOidMlDsa65,
        }

        signature_algorithm_oid, subject_public_key_algorithm_oid = parse_certificate_algorithm_oids(der)
        logger.info("Reassembled %s: signatureAlgorithm %s, subjectPublicKeyInfo algorithm %s",
                    document_name, signature_algorithm_oid, subject_public_key_algorithm_oid)

        asserts.assert_equal(subject_public_key_algorithm_oid, expected_public_key_oids[subject_profile],
                             f"{document_name} subjectPublicKeyInfo algorithm does not match the negotiated "
                             f"{subject_profile.name} profile")
        asserts.assert_equal(signature_algorithm_oid, expected_signature_oids[issuer_profile],
                             f"{document_name} signature field does not match the OID for the algorithm signaled by "
                             f"the issuer {issuer_profile.name} profile")

    @async_test_body
    async def test_TC_OPCREDS_3_9(self):
        opcreds = Clusters.OperationalCredentials
        certificate_type = opcreds.Enums.CertificateChainTypeEnum
        crypto_profile = opcreds.Enums.AttestationCryptoProfileEnum
        # Operational Credentials only ever lives on the root endpoint.
        self.root_endpoint = 0

        self.step(0)

        self.step(1)
        feature_map = await self.read_single_attribute_check_success(
            cluster=opcreds, attribute=opcreds.Attributes.FeatureMap, endpoint=self.root_endpoint)
        asserts.assert_true(feature_map & kFeaturePqcDeviceAttestation,
                            f"DUT FeatureMap (0x{feature_map:08X}) does not have PQCDA set")

        self.step(2)
        attestation_profile = await self.read_single_attribute_check_success(
            cluster=opcreds, attribute=opcreds.Attributes.PQCDeviceAttestationProfile, endpoint=self.root_endpoint)
        asserts.assert_is_not_none(attestation_profile,
                                   "DUT signals PQCDA but does not expose PQCDeviceAttestationProfile")
        paa_profile_supported = self._negotiated_profile(attestation_profile.PAASupportedProfiles, "PAA")
        pai_profile_supported = self._negotiated_profile(attestation_profile.PAISupportedProfiles, "PAI")
        dac_profile_supported = self._negotiated_profile(attestation_profile.DACSupportedProfiles, "DAC")

        self.step(3)
        # kCertificateSegmentSize is the smallest MaxSegmentSize the spec allows, so it is the value
        # that forces the largest number of response segments.
        pai_first_segment = await self._send_certificate_chain_request(
            certificate_type.kPAICertificate, pai_profile_supported, 0, kCertificateSegmentSize)
        asserts.assert_is_not_none(pai_first_segment.totalDocumentSize,
                                   "DUT omitted TotalDocumentSize from a profile-selected CertificateChainResponse")
        asserts.assert_greater(pai_first_segment.totalDocumentSize, 0, "DUT reported an empty PAI document")
        if pai_first_segment.totalDocumentSize > kCertificateSegmentSize:
            asserts.assert_is_not_none(pai_first_segment.nextSegmentID,
                                       "DUT omitted NextSegmentID on a PAI document that needs more than one segment")
        else:
            asserts.assert_is_none(pai_first_segment.nextSegmentID,
                                   "DUT returned NextSegmentID on a single-segment PAI document")

        self.step(4)
        pai_der, pai_segment_count = await self._retrieve_remaining_segments(
            certificate_type.kPAICertificate, pai_profile_supported, pai_first_segment, "PAI")
        # The PAI is issued by the PAA, so its signature carries the PAA's algorithm.
        self._verify_certificate_algorithms(pai_der, "PAI", pai_profile_supported, paa_profile_supported)

        self.step(5)
        dac_first_segment = await self._send_certificate_chain_request(
            certificate_type.kDACCertificate, dac_profile_supported, 0, kCertificateSegmentSize)
        asserts.assert_is_not_none(dac_first_segment.totalDocumentSize,
                                   "DUT omitted TotalDocumentSize from a profile-selected CertificateChainResponse")
        asserts.assert_greater(dac_first_segment.totalDocumentSize, 0, "DUT reported an empty DAC document")
        if pai_profile_supported == crypto_profile.kEcdsaMatterLegacy:
            asserts.assert_is_none(dac_first_segment.nextSegmentID,
                                   "A legacy PAI profile requires the DAC to be returned in a single response")
        elif dac_first_segment.totalDocumentSize > kCertificateSegmentSize:
            asserts.assert_is_not_none(dac_first_segment.nextSegmentID,
                                       "DUT omitted NextSegmentID on a DAC document that needs more than one segment")
        else:
            asserts.assert_is_none(dac_first_segment.nextSegmentID,
                                   "DUT returned NextSegmentID on a single-segment DAC document")

        self.step(6)
        dac_der, dac_segment_count = await self._retrieve_remaining_segments(
            certificate_type.kDACCertificate, dac_profile_supported, dac_first_segment, "DAC")
        # The DAC is issued by the PAI, so its signature carries the PAI's algorithm.
        self._verify_certificate_algorithms(dac_der, "DAC", dac_profile_supported, pai_profile_supported)

        self.step(7)
        # Valid SegmentIDs run from 0 to pai_segment_count - 1, so pai_segment_count is the first
        # one whose offset lands at or past the end of the PAI document.
        await self._expect_invalid_command(
            opcreds.Commands.CertificateChainRequest(certificateType=certificate_type.kPAICertificate,
                                                     cryptoProfile=pai_profile_supported,
                                                     segmentID=pai_segment_count,
                                                     maxSegmentSize=kCertificateSegmentSize),
            f"a SegmentID ({pai_segment_count}) past the end of a {pai_segment_count}-segment PAI document")

        self.step(8)
        await self._expect_invalid_command(
            opcreds.Commands.CertificateChainRequest(certificateType=certificate_type.kPAICertificate,
                                                     cryptoProfile=pai_profile_supported,
                                                     segmentID=0,
                                                     maxSegmentSize=kOversizedMaxSegmentSize),
            f"a MaxSegmentSize ({kOversizedMaxSegmentSize}) that cannot fit in a single message on this transport")

        self.step(9)
        await self._expect_invalid_command(
            opcreds.Commands.CertificateChainRequest(certificateType=kInvalidCertificateType,
                                                     cryptoProfile=dac_profile_supported,
                                                     segmentID=0,
                                                     maxSegmentSize=kCertificateSegmentSize),
            f"an invalid CertificateType ({kInvalidCertificateType})")
        logger.info("Retrieved the PAI in %d segment(s) and the DAC in %d segment(s)", pai_segment_count,
                    dac_segment_count)


if __name__ == "__main__":
    default_matter_test_main()
