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

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   ml-dsa-44:
#     app: ${ALL_DEVICES_APP}
#     app-args: >
#       --device on-off-light:1
#       --discriminator 1234
#       --KVS kvs-TC_OPCREDS_3_9-ml-dsa-44
#       --vendor-id 0xFFF1
#       --product-id 0x8001
#       --dac_provider credentials/development/attestation/TestCredentials-FFF1-8001-ML-DSA-44.json
#       --trace-to json:${TRACE_APP}-ml-dsa-44.json
#     script-args: >
#       --storage-path admin_storage-TC_OPCREDS_3_9-ml-dsa-44.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values-pqc
#       --paa-trust-store-path credentials/development/paa-root-certs
#       --trace-to json:${TRACE_TEST_JSON}-ml-dsa-44.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}-ml-dsa-44.perfetto
#     factory-reset: true
#     quiet: true
#   ml-dsa-65:
#     app: ${ALL_DEVICES_APP}
#     app-args: >
#       --device on-off-light:1
#       --discriminator 1234
#       --KVS kvs-TC_OPCREDS_3_9-ml-dsa-65
#       --vendor-id 0xFFF1
#       --product-id 0x8001
#       --dac_provider credentials/development/attestation/TestCredentials-FFF1-8001-ML-DSA-65.json
#       --trace-to json:${TRACE_APP}-ml-dsa-65.json
#     script-args: >
#       --storage-path admin_storage-TC_OPCREDS_3_9-ml-dsa-65.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values-pqc
#       --paa-trust-store-path credentials/development/paa-root-certs
#       --trace-to json:${TRACE_TEST_JSON}-ml-dsa-65.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}-ml-dsa-65.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts
from support_modules.pqc_support import (AttestationCryptoProfile, CertificateAlgorithms, OperationalCredentialsFeature,
                                         assert_initial_certificate_segment, assert_profile_advertised,
                                         candidate_attestation_chains, kCertificateSegmentSize, parse_certificate_algorithms,
                                         request_first_available_pai, retrieve_segmented_document)

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

# A MaxSegmentSize that no Matter transport can carry within a single message (test step 9).
kOversizedMaxSegmentSize = 0xFFFF

# CertificateType values outside CertificateChainTypeEnum (test step 10).
kInvalidCertificateType = 0x03


class TC_OPCREDS_3_9(MatterTestCommissionedDevice):
    """PQC Device Attestation CertificateChainRequest behavior [DUT-Server]."""

    def desc_TC_OPCREDS_3_9(self) -> str:
        return "[TC-OPCREDS-3.9] PQC Device Attestation CertificateChainRequest behavior [DUT-Server]"

    def pics_TC_OPCREDS_3_9(self) -> list[str]:
        return ["OPCREDS.S", "OPCREDS.S.F00", "OPCREDS.S.C02.Rsp", "OPCREDS.S.C03.Tx"]

    def steps_TC_OPCREDS_3_9(self) -> list[TestStep]:
        return [
            TestStep(0, "Commission DUT to TH"),
            TestStep(1, "TH reads the DUT FeatureMap.",
                     "TH verifies DUT has PQCDA set."),
            TestStep(2, "TH reads PQCDeviceAttestationProfile.",
                     "TH obtains the advertised profile bitmap set for PAA, PAI and DAC. PAA is stored as "
                     "PAAProfileSupported, PAI is stored as PAIProfileSupported and DAC is stored as "
                     "DACProfileSupported."),
            TestStep(3, "TH independently selects the highest-security profiles advertised in PAAProfileSupported "
                        "and PAIProfileSupported as selectedPAAProfile and selectedPAIProfile. TH verifies that "
                        "DACProfileSupported advertises EcdsaMatterLegacy and stores it as selectedDACProfile. "
                        "TH constructs selectedCryptoProfile as a CryptoProfileChainStruct with PAACrypto set to "
                        "selectedPAAProfile, PAICrypto set to selectedPAIProfile, and DACCrypto set to selectedDACProfile.",
                     "Each PAA/PAI selection prefers MlDsa65, then MlDsa44, then EcdsaMatterLegacy. "
                     "selectedDACProfile is EcdsaMatterLegacy."),
            TestStep(4, "TH sends CertificateChainRequest for PAICertificate with CryptoProfile set to "
                        "selectedCryptoProfile. TH sets MaxSegmentSize "
                        "to a value that requires more than one response segment.",
                     "DUT returns TotalDocumentSize and NextSegmentID as required for segmented retrieval."),
            TestStep(5, "TH continues to request the remaining segments for PAI using the returned NextSegmentID "
                        "values until completion.",
                     "Segment numbering is monotonic from zero; TotalDocumentSize is stable across all segments; "
                     "NextSegmentID is present until the final segment and absent on the final segment; the "
                     "reassembled PAI payload exactly matches the declared TotalDocumentSize; the reassembled PAI "
                     "payload parses successfully; its subjectPublicKeyInfo algorithm matches the selected PAI "
                     "profile; and its signature matches the selected PAA profile."),
            TestStep(6, "TH sends CertificateChainRequest for DACCertificate with CryptoProfile set to "
                        "selectedCryptoProfile and the same MaxSegmentSize used for the PAI request.",
                     "If the PAI public-key profile is EcdsaMatterLegacy, DUT returns the certificate in a single "
                     "response and omits NextSegmentID. Otherwise DUT returns TotalDocumentSize and NextSegmentID "
                     "as required for segmented retrieval."),
            TestStep(7, "TH continues to request the remaining segments for DAC with CryptoProfile set to "
                        "selectedCryptoProfile, using the returned NextSegmentID "
                        "values until completion.",
                     "Segment numbering is monotonic from zero; TotalDocumentSize is stable across all segments; "
                     "NextSegmentID is present until the final segment and absent on the final segment; the "
                     "reassembled DAC payload exactly matches the declared TotalDocumentSize; the reassembled DAC "
                     "payload parses successfully; its subjectPublicKeyInfo algorithm matches the selected DAC "
                     "profile; the DAC signature matches selectedPAIProfile; the PAI signature matches "
                     "selectedPAAProfile; and the DAC public key uses EcdsaMatterLegacy for the Device Attestation signature."),
            TestStep(8, "TH sends a CertificateChainRequest with CryptoProfile set to selectedCryptoProfile "
                        "and a SegmentID that points beyond the available data.",
                     "DUT rejects the request with INVALID_COMMAND."),
            TestStep(9, "TH sends a CertificateChainRequest with CryptoProfile set to selectedCryptoProfile "
                        "and a MaxSegmentSize value that cannot fit within the "
                        "message space on the transport under test.",
                     "DUT rejects the request with INVALID_COMMAND."),
            TestStep(10, "TH sends CertificateChainRequest with an invalid CertificateType value.",
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
        self.step(3)
        # The two bitmaps do not say which PAA/PAI pairs the DUT holds, so step 4 requests each
        # advertised pairing, highest security first, and keeps the first one the DUT serves.
        candidate_chains = candidate_attestation_chains(attestation_profile.PAASupportedProfiles,
                                                        attestation_profile.PAISupportedProfiles)
        assert_profile_advertised(attestation_profile.DACSupportedProfiles,
                                  AttestationCryptoProfile.kEcdsaMatterLegacy, "DAC", "Device Attestation signature")
        # Exercise the Phase 1 DAC profile even when the DUT advertises additional profiles.
        selected_dac_profile = AttestationCryptoProfile.kEcdsaMatterLegacy

        self.step(4)
        # kCertificateSegmentSize is the smallest MaxSegmentSize the spec allows, so it is the value
        # that forces the largest number of response segments.
        selected_crypto_profile, pai_first_segment = await request_first_available_pai(
            self._send_certificate_chain_request, candidate_chains, kCertificateSegmentSize)
        selected_paa_profile = selected_crypto_profile.PAACrypto
        selected_pai_profile = selected_crypto_profile.PAICrypto
        assert_initial_certificate_segment(pai_first_segment, "PAI", kCertificateSegmentSize)
        asserts.assert_is_not_none(pai_first_segment.nextSegmentID,
                                   "The selected PQC PAI must return NextSegmentID for segmented retrieval")

        self.step(5)
        pai_document = await retrieve_segmented_document(
            self._send_certificate_chain_request, certificate_type.kPAICertificate, selected_crypto_profile, "PAI",
            kCertificateSegmentSize, first_response=pai_first_segment)
        asserts.assert_greater(pai_document.segment_count, 1,
                               "The selected PQC PAI certificate must require more than one response segment")
        pai_algorithms = parse_certificate_algorithms(pai_document.der, "PAI")
        self._assert_certificate_profiles(
            pai_algorithms, "PAI", selected_pai_profile, selected_paa_profile)

        self.step(6)
        dac_first_segment = await self._send_certificate_chain_request(
            certificate_type.kDACCertificate, selected_crypto_profile, 0, kCertificateSegmentSize)
        assert_initial_certificate_segment(dac_first_segment, "DAC", kCertificateSegmentSize)
        if selected_pai_profile == AttestationCryptoProfile.kEcdsaMatterLegacy:
            asserts.assert_is_none(dac_first_segment.nextSegmentID, "A legacy-signed DAC must fit in one response")
        else:
            asserts.assert_is_not_none(dac_first_segment.nextSegmentID, "A PQC-signed DAC must return NextSegmentID")

        self.step(7)
        dac_document = await retrieve_segmented_document(
            self._send_certificate_chain_request, certificate_type.kDACCertificate, selected_crypto_profile, "DAC",
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

        self.step(8)
        # Valid SegmentIDs run from 0 to segment_count - 1, so segment_count is the first
        # one whose offset lands at or past the end of the PAI document.
        await self._expect_invalid_command(
            opcreds.Commands.CertificateChainRequest(certificateType=certificate_type.kPAICertificate,
                                                     cryptoProfile=selected_crypto_profile,
                                                     segmentID=pai_document.segment_count,
                                                     maxSegmentSize=kCertificateSegmentSize),
            f"a SegmentID ({pai_document.segment_count}) past the end of a "
            f"{pai_document.segment_count}-segment PAI document")

        self.step(9)
        await self._expect_invalid_command(
            opcreds.Commands.CertificateChainRequest(certificateType=certificate_type.kPAICertificate,
                                                     cryptoProfile=selected_crypto_profile,
                                                     segmentID=0,
                                                     maxSegmentSize=kOversizedMaxSegmentSize),
            f"a MaxSegmentSize ({kOversizedMaxSegmentSize}) that cannot fit in a single message on this transport")

        self.step(10)
        await self._expect_invalid_command(
            opcreds.Commands.CertificateChainRequest(certificateType=kInvalidCertificateType,
                                                     cryptoProfile=selected_crypto_profile,
                                                     segmentID=0,
                                                     maxSegmentSize=kCertificateSegmentSize),
            f"an invalid CertificateType ({kInvalidCertificateType})")
        logger.info("Retrieved the PAI in %d segment(s) and the DAC in %d segment(s)", pai_document.segment_count,
                    dac_document.segment_count)


if __name__ == "__main__":
    default_matter_test_main()
