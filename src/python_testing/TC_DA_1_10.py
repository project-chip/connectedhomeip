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
#       --KVS kvs-TC_DA_1_10-ml-dsa-44
#       --vendor-id 0xFFF1
#       --product-id 0x8001
#       --dac_provider credentials/development/attestation/TestCredentials-FFF1-8001-ML-DSA-44.json
#       --trace-to json:${TRACE_APP}-ml-dsa-44.json
#     script-args: >
#       --storage-path admin_storage-TC_DA_1_10-ml-dsa-44.json
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
#       --KVS kvs-TC_DA_1_10-ml-dsa-65
#       --vendor-id 0xFFF1
#       --product-id 0x8001
#       --dac_provider credentials/development/attestation/TestCredentials-FFF1-8001-ML-DSA-65.json
#       --trace-to json:${TRACE_APP}-ml-dsa-65.json
#     script-args: >
#       --storage-path admin_storage-TC_DA_1_10-ml-dsa-65.json
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
from support_modules.pqc_support import (AttestationCryptoProfile, OperationalCredentialsFeature, assert_profile_advertised,
                                         candidate_attestation_chains, find_issuing_paa, is_pqc_profile, kCertificateSegmentSize,
                                         load_paa_certificates, parse_certificate, request_first_available_pai,
                                         retrieve_segmented_document, validate_attestation_chain)

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterTestCommissionedDevice
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_DA_1_10(MatterTestCommissionedDevice):
    """Validate CertificateChainRequest with PQC Profiles [DUT-Commissionee]."""

    def desc_TC_DA_1_10(self) -> str:
        return "[TC-DA-1.10] Validate CertificateChainRequest with PQC Profiles [DUT-Commissionee]"

    def pics_TC_DA_1_10(self) -> list[str]:
        return ["MCORE.ROLE.COMMISSIONEE", "OPCREDS.S", "OPCREDS.S.F00", "OPCREDS.S.C02.Rsp", "OPCREDS.S.C03.Tx"]

    def steps_TC_DA_1_10(self) -> list[TestStep]:
        return [
            TestStep(0, "Commission DUT to TH"),
            TestStep(1, "TH reads the Operational Credentials Cluster FeatureMap attribute from the DUT.",
                     "TH verifies that the PQCDA feature bit is set."),
            TestStep(2, "TH reads the PQCDeviceAttestationProfile attribute from the DUT.",
                     "TH stores the advertised profile sets as paaProfiles, paiProfiles and dacProfiles."),
            TestStep(3, "TH selects the highest-security profile in paaProfiles and stores it as selectedPAAProfile. "
                        "TH independently selects the highest-security profile in paiProfiles and stores it as "
                        "selectedPAIProfile.",
                     "For each profile set, TH selects MlDsa65 when it is advertised, otherwise MlDsa44 when it is "
                     "advertised, otherwise EcdsaMatterLegacy."),
            TestStep(4, "TH verifies that dacProfiles advertises EcdsaMatterLegacy and stores it as selectedDACProfile. "
                        "TH constructs selectedCryptoProfile as a CryptoProfileChainStruct with PAACrypto set to "
                        "selectedPAAProfile, PAICrypto set to selectedPAIProfile, and DACCrypto set to selectedDACProfile.",
                     "selectedDACProfile is EcdsaMatterLegacy."),
            TestStep(5, "TH sends CertificateChainRequest for PAICertificate with CryptoProfile set to "
                        "selectedCryptoProfile, SegmentID set to 0, and MaxSegmentSize set to >= 600.",
                     "If either selectedPAAProfile or selectedPAIProfile is a PQC profile, DUT returns the first "
                     "segment of the PAI. This includes the case where the PAI uses ECDSA but its signature was "
                     "generated by a PQC PAA. If both profiles are EcdsaMatterLegacy, DUT returns the complete PAI "
                     "in one CertificateChainResponse and omits NextSegmentID."),
            TestStep(6, "While NextSegmentID is present in the returned CertificateChainResponse, TH sends "
                        "CertificateChainRequest for PAICertificate with the same CryptoProfile and MaxSegmentSize, "
                        "and with SegmentID set to the previously returned NextSegmentID. TH reassembles and saves "
                        "the complete certificate as profile_pai_cert. If NextSegmentID was absent in step 5, "
                        "TH saves that response as profile_pai_cert.",
                     "Segment numbering is monotonic from zero; TotalDocumentSize is stable across all segments; "
                     "NextSegmentID is present until the final segment and absent on the final segment; the size of "
                     "profile_pai_cert matches TotalDocumentSize; profile_pai_cert parses as an X.509 certificate; "
                     "the PAI public key algorithm matches selectedPAIProfile; and the PAI "
                     "signature algorithm matches selectedPAAProfile."),
            TestStep(7, "TH sends CertificateChainRequest for DACCertificate with CryptoProfile set to "
                        "selectedCryptoProfile, SegmentID set to 0, and MaxSegmentSize set to the value used for the PAI "
                        "request.",
                     "If the PAI subject key is a PQC profile, DUT returns the first segment of the DAC and includes "
                     "NextSegmentID. If the PAI subject key is EcdsaMatterLegacy, DUT returns the complete DAC in one "
                     "CertificateChainResponse and omits NextSegmentID."),
            TestStep(8, "While NextSegmentID is present in the returned CertificateChainResponse, TH sends "
                        "CertificateChainRequest for DACCertificate with the same CryptoProfile and MaxSegmentSize, "
                        "and with SegmentID set to the previously returned NextSegmentID. TH reassembles and saves "
                        "the complete certificate as profile_dac_cert. If NextSegmentID was absent in step 7, "
                        "TH saves that response as profile_dac_cert.",
                     "Segment numbering is monotonic from zero when the response is segmented; TotalDocumentSize is "
                     "stable across all segments; NextSegmentID is absent on the final response; the size of "
                     "profile_dac_cert matches TotalDocumentSize; profile_dac_cert parses as an X.509 certificate; "
                     "the DAC public key algorithm is ECDSA; and the DAC signature algorithm matches the PAI subject "
                     "key algorithm."),
            TestStep(9, "TH sends the legacy CertificateChainRequest for PAICertificate, including only the "
                        "CertificateType field and omitting CryptoProfile, SegmentID and MaxSegmentSize.",
                     "DUT returns the complete legacy PAI in one CertificateChainResponse. NextSegmentID is absent. "
                     "TH saves the certificate as legacy_pai_cert."),
            TestStep(10, "TH sends the legacy CertificateChainRequest for DACCertificate, including only the "
                         "CertificateType field and omitting CryptoProfile, SegmentID and MaxSegmentSize.",
                     "DUT returns the complete legacy DAC in one CertificateChainResponse. NextSegmentID is absent. "
                     "TH saves the certificate as legacy_dac_cert."),
            TestStep(11, "TH validates profile_pai_cert and profile_dac_cert against the externally obtained PAA "
                         "certificate for selectedPAAProfile.",
                     "The PAA, PAI and DAC form a valid three-certificate attestation chain; the PAI signature "
                     "validates with the PAA public key; the DAC signature validates with the PAI public key; and "
                     "each certificate is valid according to the device attestation certificate requirements."),
            TestStep(12, "TH validates legacy_pai_cert and legacy_dac_cert against the externally obtained legacy "
                         "PAA certificate.",
                     "The legacy PAA, PAI and DAC form a valid three-certificate attestation chain; the PAI signature "
                     "validates with the PAA public key; the DAC signature validates with the PAI public key; and "
                     "each certificate is valid according to the device attestation certificate requirements."),
        ]

    async def _send_certificate_chain_request(self, certificate_type, crypto_profile, segment_id: int,
                                              max_segment_size: int):
        """Send a profile-selected CertificateChainRequest."""
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

    async def _retrieve_legacy_certificate(self, certificate_type, document_name: str) -> bytes:
        """Send a legacy CertificateChainRequest carrying only CertificateType.

        A DUT that signals PQCDA rejects a request that omits CryptoProfile but
        supplies SegmentID or MaxSegmentSize, so all three optional fields are left unset.
        """
        opcreds = Clusters.OperationalCredentials
        response = await self.send_single_cmd(
            cmd=opcreds.Commands.CertificateChainRequest(certificateType=certificate_type),
            endpoint=self.root_endpoint)
        asserts.assert_true(isinstance(response, opcreds.Commands.CertificateChainResponse),
                            "DUT returned an invalid response to a legacy CertificateChainRequest")
        asserts.assert_is_none(response.nextSegmentID,
                               f"DUT returned NextSegmentID for the legacy {document_name}, which is always returned "
                               "in a single response")
        asserts.assert_greater(len(response.certificate), 0, f"DUT returned an empty legacy {document_name}")
        logger.info("Retrieved the legacy %s in one response, %d bytes", document_name, len(response.certificate))
        return response.certificate

    @async_test_body
    async def test_TC_DA_1_10(self):
        opcreds = Clusters.OperationalCredentials
        certificate_type = opcreds.Enums.CertificateChainTypeEnum
        # Operational Credentials only ever lives on the root endpoint.
        self.root_endpoint = 0

        # Steps 11 and 12 need PAA certificates the DUT never supplies, so resolve the trust store
        # up front rather than failing once the retrieval work is already done.
        # The runner supplies a default path when --paa-trust-store-path is omitted.
        paa_candidates = load_paa_certificates(self.matter_test_config.paa_trust_store_path)

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
        paa_profiles = attestation_profile.PAASupportedProfiles
        pai_profiles = attestation_profile.PAISupportedProfiles
        dac_profiles = attestation_profile.DACSupportedProfiles

        self.step(3)
        # The two bitmaps do not say which PAA/PAI pairs the DUT holds, so step 5 requests each
        # advertised pairing, highest security first, and keeps the first one the DUT serves.
        candidate_chains = candidate_attestation_chains(paa_profiles, pai_profiles)

        self.step(4)
        # During PQC Phase 1 the DAC keeps a P-256 subject key so the Device Attestation signature
        # stays EcdsaMatterLegacy, which is what dacProfiles must advertise here.
        assert_profile_advertised(dac_profiles, AttestationCryptoProfile.kEcdsaMatterLegacy, "DAC",
                                  "Device Attestation signature")
        selected_dac_profile = AttestationCryptoProfile.kEcdsaMatterLegacy

        self.step(5)
        selected_crypto_profile, pai_first_segment = await request_first_available_pai(
            self._send_certificate_chain_request, candidate_chains, kCertificateSegmentSize)
        selected_paa_profile = selected_crypto_profile.PAACrypto
        selected_pai_profile = selected_crypto_profile.PAICrypto
        if is_pqc_profile(selected_paa_profile) or is_pqc_profile(selected_pai_profile):
            asserts.assert_is_not_none(pai_first_segment.nextSegmentID, "A PQC PAI must return NextSegmentID")
        else:
            asserts.assert_is_none(pai_first_segment.nextSegmentID, "A legacy PAI must fit in one response")

        self.step(6)
        pai_document = await retrieve_segmented_document(
            self._send_certificate_chain_request, certificate_type.kPAICertificate, selected_crypto_profile, "PAI",
            kCertificateSegmentSize, first_response=pai_first_segment)
        if is_pqc_profile(selected_paa_profile) or is_pqc_profile(selected_pai_profile):
            asserts.assert_greater(pai_document.segment_count, 1,
                                   "A PQC PAA or PAI profile makes the PAI too large for a single 600 byte segment")
        else:
            asserts.assert_equal(pai_document.segment_count, 1,
                                 "An EcdsaMatterLegacy chain must return the complete PAI in one response")

        # retrieve_segmented_document already asserted the segmentation rules and the reassembled
        # size, so what remains is parsing the document and checking its algorithms.
        profile_pai_cert = parse_certificate(pai_document.der, "profile_pai_cert")
        pai_algorithms = profile_pai_cert.algorithms

        # The PAA and PAI profiles are selected independently: the PAI's subject key follows
        # selectedPAIProfile, while its issuer signature follows selectedPAAProfile.
        asserts.assert_equal(pai_algorithms.subject_key_profile, selected_pai_profile,
                             "profile_pai_cert public key algorithm must match selectedPAIProfile "
                             f"{selected_pai_profile.name}")
        asserts.assert_equal(pai_algorithms.signature_profile, selected_paa_profile,
                             "profile_pai_cert signature algorithm must match selectedPAAProfile "
                             f"{selected_paa_profile.name} because the self-signed PAA issued it")

        self.step(7)
        dac_first_segment = await self._send_certificate_chain_request(
            certificate_type.kDACCertificate, selected_crypto_profile, 0, kCertificateSegmentSize)
        if is_pqc_profile(selected_pai_profile):
            asserts.assert_is_not_none(dac_first_segment.nextSegmentID, "A PQC-signed DAC must return NextSegmentID")
        else:
            asserts.assert_is_none(dac_first_segment.nextSegmentID, "A legacy-signed DAC must fit in one response")

        self.step(8)
        dac_document = await retrieve_segmented_document(
            self._send_certificate_chain_request, certificate_type.kDACCertificate, selected_crypto_profile, "DAC",
            kCertificateSegmentSize, first_response=dac_first_segment)

        if is_pqc_profile(selected_pai_profile):
            asserts.assert_greater(dac_document.segment_count, 1,
                                   f"A {selected_pai_profile.name} PAI signature makes the DAC too "
                                   "large for a single 600 byte segment")
        else:
            asserts.assert_equal(dac_document.segment_count, 1,
                                 "A DAC signed by an EcdsaMatterLegacy PAI must be returned in one response")

        profile_dac_cert = parse_certificate(dac_document.der, "profile_dac_cert")
        dac_algorithms = profile_dac_cert.algorithms

        asserts.assert_equal(dac_algorithms.subject_key_profile, selected_dac_profile,
                             "profile_dac_cert public key algorithm must match selectedDACProfile "
                             f"{selected_dac_profile.name}")
        asserts.assert_equal(dac_algorithms.signature_profile, selected_pai_profile,
                             "profile_dac_cert signature algorithm must match selectedPAIProfile "
                             f"{selected_pai_profile.name}")

        self.step(9)
        legacy_pai_cert = parse_certificate(
            await self._retrieve_legacy_certificate(certificate_type.kPAICertificate, "PAI"), "legacy_pai_cert")

        self.step(10)
        legacy_dac_cert = parse_certificate(
            await self._retrieve_legacy_certificate(certificate_type.kDACCertificate, "DAC"), "legacy_dac_cert")

        self.step(11)
        # The externally obtained PAA for selectedPAAProfile: the trust store entry whose key uses
        # that profile and whose signature over the retrieved PAI validates.
        profile_paa_cert = find_issuing_paa(profile_pai_cert, paa_candidates, selected_paa_profile,
                                            f"{selected_paa_profile.name}/{selected_pai_profile.name}")
        validate_attestation_chain(profile_paa_cert, profile_pai_cert, profile_dac_cert,
                                   f"{selected_paa_profile.name}/{selected_pai_profile.name}")

        self.step(12)
        legacy_paa_cert = find_issuing_paa(legacy_pai_cert, paa_candidates,
                                           AttestationCryptoProfile.kEcdsaMatterLegacy, "legacy")
        validate_attestation_chain(legacy_paa_cert, legacy_pai_cert, legacy_dac_cert, "legacy")

        logger.info("Validated the %s PAA / %s PAI chain (PAI %d segment(s), DAC %d segment(s)) and the legacy chain",
                    selected_paa_profile.name, selected_pai_profile.name,
                    pai_document.segment_count, dac_document.segment_count)


if __name__ == "__main__":
    default_matter_test_main()
