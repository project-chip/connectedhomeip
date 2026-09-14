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
#   ./chip-all-clusters-app --dac_provider <pqc_dac_provider.json>
#
# where the JSON carries the dac_cert_ml_dsa_44/65 and pai_cert_ml_dsa_44/65 keys
# read by TestHarnessDACProvider. The PQC DAC certificates retain P-256 subject keys for the
# legacy Device Attestation signature and are signed by the corresponding ML-DSA PAI.

import logging

from mobly import asserts
from support_modules.pqc_support import (AttestationCryptoProfile, CertificateAlgorithms, OperationalCredentialsFeature,
                                         assert_initial_certificate_segment, assert_profile_advertised,
                                         assert_profile_supported_by_test_harness, kCertificateSegmentSize,
                                         parse_certificate_algorithms, retrieve_segmented_document, select_strongest_profile)

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

# A MaxSegmentSize that no Matter transport can carry within a single message (test step 8).
kOversizedMaxSegmentSize = 0xFFFF

# CertificateType values outside CertificateChainTypeEnum (test step 9).
kInvalidCertificateType = 0x03


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
            TestStep(3, "TH selects the strongest profile advertised for the PAI and sends "
                        "CertificateChainRequest for PAICertificate using that profile. TH sets MaxSegmentSize "
                        "to a value that requires more than one response segment.",
                     "DUT returns TotalDocumentSize and NextSegmentID as required for segmented retrieval."),
            TestStep(4, "TH continues to request the remaining segments for PAI using the returned NextSegmentID "
                        "values until completion.",
                     "Segment numbering is monotonic from zero; TotalDocumentSize is stable across all segments; "
                     "NextSegmentID is present until the final segment and absent on the final segment; the "
                     "reassembled PAI payload exactly matches the declared TotalDocumentSize; the reassembled PAI "
                     "payload parses successfully; its subjectPublicKeyInfo algorithm matches the selected PAI "
                     "profile; and its signature matches the selected PAA profile."),
            TestStep(5, "TH selects the strongest profile advertised for the DAC and sends CertificateChainRequest "
                        "for DACCertificate using that profile.",
                     "If the PAI public-key profile is EcdsaMatterLegacy, DUT returns the certificate in a single "
                     "response and omits NextSegmentID. Otherwise DUT returns TotalDocumentSize and NextSegmentID "
                     "as required for segmented retrieval."),
            TestStep(6, "TH continues to request the remaining segments for DAC using the returned NextSegmentID "
                        "values until completion.",
                     "Segment numbering is monotonic from zero; TotalDocumentSize is stable across all segments; "
                     "NextSegmentID is present until the final segment and absent on the final segment; the "
                     "reassembled DAC payload exactly matches the declared TotalDocumentSize; the reassembled DAC "
                     "payload parses successfully; its subjectPublicKeyInfo algorithm matches the selected DAC "
                     "profile; the DAC signature matches the selected PAI profile; and the DAC public key uses "
                     "EcdsaMatterLegacy for the Device Attestation signature."),
            TestStep(7, "TH sends a CertificateChainRequest with a SegmentID that points beyond the available data.",
                     "DUT rejects the request with INVALID_COMMAND."),
            TestStep(8, "TH sends a CertificateChainRequest with a MaxSegmentSize value that cannot fit within the "
                        "message space on the transport under test.",
                     "DUT rejects the request with INVALID_COMMAND."),
            TestStep(9, "TH sends CertificateChainRequest with an invalid CertificateType value.",
                     "DUT rejects the request with INVALID_COMMAND."),
        ]

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

    def _assert_certificate_profiles(self, algorithms: CertificateAlgorithms, document_name: str,
                                     expected_subject_key_profile: AttestationCryptoProfile,
                                     expected_signature_profile: AttestationCryptoProfile) -> None:
        """Validate a certificate's key and signature against the independently selected profiles."""
        logger.info("Reassembled %s: signatureAlgorithm %s, subjectPublicKeyInfo algorithm %s",
                    document_name, algorithms.signature_algorithm_oid,
                    algorithms.subject_public_key_algorithm_oid)

        asserts.assert_equal(algorithms.subject_key_profile, expected_subject_key_profile,
                             f"{document_name} subjectPublicKeyInfo must use "
                             f"{expected_subject_key_profile.name}")
        asserts.assert_equal(algorithms.signature_profile, expected_signature_profile,
                             f"{document_name} signature profile must match its issuer's public-key profile "
                             f"{expected_signature_profile.name}")

    @async_test_body
    async def test_TC_OPCREDS_3_9(self):
        opcreds = Clusters.OperationalCredentials
        certificate_type = opcreds.Enums.CertificateChainTypeEnum
        # Operational Credentials only ever lives on the root endpoint.
        self.root_endpoint = 0

        self.step(0)

        self.step(1)
        feature_map = await self.read_single_attribute_check_success(
            cluster=opcreds, attribute=opcreds.Attributes.FeatureMap, endpoint=self.root_endpoint)
        asserts.assert_true(feature_map & OperationalCredentialsFeature.kPQCDeviceAttestation,
                            f"DUT FeatureMap (0x{feature_map:08X}) does not have PQCDA set")

        self.step(2)
        attestation_profile = await self.read_single_attribute_check_success(
            cluster=opcreds, attribute=opcreds.Attributes.PQCDeviceAttestationProfile, endpoint=self.root_endpoint)
        asserts.assert_is_not_none(attestation_profile,
                                   "DUT signals PQCDA but does not expose PQCDeviceAttestationProfile")
        selected_paa_profile = select_strongest_profile(attestation_profile.PAASupportedProfiles, "PAA")
        selected_pai_profile = select_strongest_profile(attestation_profile.PAISupportedProfiles, "PAI")
        assert_profile_advertised(attestation_profile.DACSupportedProfiles,
                                  AttestationCryptoProfile.kEcdsaMatterLegacy, "DAC", "Device Attestation signature")
        # Exercise the Phase 1 DAC profile even when the DUT advertises additional profiles.
        selected_dac_profile = AttestationCryptoProfile.kEcdsaMatterLegacy

        assert_profile_supported_by_test_harness(selected_paa_profile)
        assert_profile_supported_by_test_harness(selected_pai_profile)

        self.step(3)
        # kCertificateSegmentSize is the smallest MaxSegmentSize the spec allows, so it is the value
        # that forces the largest number of response segments.
        pai_first_segment = await self._send_certificate_chain_request(
            certificate_type.kPAICertificate, selected_pai_profile, 0, kCertificateSegmentSize)
        assert_initial_certificate_segment(pai_first_segment, "PAI", kCertificateSegmentSize)

        self.step(4)
        pai_document = await retrieve_segmented_document(
            self._send_certificate_chain_request, certificate_type.kPAICertificate, selected_pai_profile, "PAI",
            kCertificateSegmentSize, first_response=pai_first_segment)
        asserts.assert_greater(pai_document.segment_count, 1,
                               "The selected PQC PAI certificate must require more than one response segment")
        pai_algorithms = parse_certificate_algorithms(pai_document.der, "PAI")
        self._assert_certificate_profiles(
            pai_algorithms, "PAI", selected_pai_profile, selected_paa_profile)

        self.step(5)
        dac_first_segment = await self._send_certificate_chain_request(
            certificate_type.kDACCertificate, selected_dac_profile, 0, kCertificateSegmentSize)
        assert_initial_certificate_segment(dac_first_segment, "DAC", kCertificateSegmentSize)

        self.step(6)
        dac_document = await retrieve_segmented_document(
            self._send_certificate_chain_request, certificate_type.kDACCertificate, selected_dac_profile, "DAC",
            kCertificateSegmentSize, first_response=dac_first_segment)
        dac_algorithms = parse_certificate_algorithms(dac_document.der, "DAC")
        self._assert_certificate_profiles(
            dac_algorithms, "DAC", selected_dac_profile, selected_pai_profile)

        if selected_pai_profile == AttestationCryptoProfile.kEcdsaMatterLegacy:
            asserts.assert_equal(dac_document.segment_count, 1,
                                 "A DAC signed by an EcdsaMatterLegacy PAI must be returned in one response")
        else:
            asserts.assert_greater(dac_document.segment_count, 1,
                                   f"A {selected_pai_profile.name} PAI signature requires a segmented DAC response")

        self.step(7)
        # Valid SegmentIDs run from 0 to segment_count - 1, so segment_count is the first
        # one whose offset lands at or past the end of the PAI document.
        await self._expect_invalid_command(
            opcreds.Commands.CertificateChainRequest(certificateType=certificate_type.kPAICertificate,
                                                     cryptoProfile=selected_pai_profile,
                                                     segmentID=pai_document.segment_count,
                                                     maxSegmentSize=kCertificateSegmentSize),
            f"a SegmentID ({pai_document.segment_count}) past the end of a "
            f"{pai_document.segment_count}-segment PAI document")

        self.step(8)
        await self._expect_invalid_command(
            opcreds.Commands.CertificateChainRequest(certificateType=certificate_type.kPAICertificate,
                                                     cryptoProfile=selected_pai_profile,
                                                     segmentID=0,
                                                     maxSegmentSize=kOversizedMaxSegmentSize),
            f"a MaxSegmentSize ({kOversizedMaxSegmentSize}) that cannot fit in a single message on this transport")

        self.step(9)
        await self._expect_invalid_command(
            opcreds.Commands.CertificateChainRequest(certificateType=kInvalidCertificateType,
                                                     cryptoProfile=selected_dac_profile,
                                                     segmentID=0,
                                                     maxSegmentSize=kCertificateSegmentSize),
            f"an invalid CertificateType ({kInvalidCertificateType})")
        logger.info("Retrieved the PAI in %d segment(s) and the DAC in %d segment(s)", pai_document.segment_count,
                    dac_document.segment_count)


if __name__ == "__main__":
    default_matter_test_main()
