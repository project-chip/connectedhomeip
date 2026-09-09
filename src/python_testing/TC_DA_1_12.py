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

# This test requires a DUT that signals the PQCDA feature of the
# Operational Credentials cluster. There is no CI test arguments block because no PQC-capable DAC
# provider fixture is checked in yet; run it manually against a Linux example app started with:
#
#   ./chip-all-clusters-app --dac_provider <pqc_dac_provider.json>
#
# where the JSON carries the dac_cert_ml_dsa_44/65 and pai_cert_ml_dsa_44/65 keys read by
# TestHarnessDACProvider. Generate the chains with chip-cert gen-att-cert.
#
# Steps 10 and 11 validate the SAME AttestationResponse against both the profile-selected and the
# legacy chain, so every DAC the DUT serves has to carry the same P-256 subject key: there is only
# one Device Attestation signature, and TestHarnessDACProvider always produces it with the
# dac_private_key from the provider JSON. Generate one DAC key and reuse it across the legacy,
# ML-DSA-44 and ML-DSA-65 chains by passing the same `chip-cert gen-att-cert --key` to each.
#
# Steps 10 and 11 also validate both chains against externally obtained PAA certificates, so
# --paa-trust-store-path must hold the DER PAA for the negotiated PQC profile alongside the legacy
# ECDSA PAA.
#
# All certificate and signature validation runs in Python (see support_modules/pqc_support.py)
# rather than through the SDK's C++ crypto, so the harness is an independent check on the stack.
# Verifying ML-DSA signatures therefore needs a cryptography build providing
# hazmat.primitives.asymmetric.mldsa (46.0.0 or newer, linked against OpenSSL 3.5 or newer).
#
#   python3 src/python_testing/TC_DA_1_12.py --commissioning-method on-network \
#       --discriminator 1234 --passcode 20202021 \
#       --paa-trust-store-path <directory holding the legacy and ML-DSA PAA .der files>
#
# Test CDs use the bundled development signer certificates by default. Supply
# --string-arg cd_cert_dir:<directory> for another test signer set. A provisional CD signed by a
# test key additionally requires --bool-arg override_provisional_cd_check_warning:true; official
# CDs and provisional CDs without that override are always checked against the CSA production keys.

import logging
import random
from pathlib import Path

from mobly import asserts
from support_modules.pqc_support import (AttestationCryptoProfile, OperationalCredentialsFeature, assert_attestation_nonce,
                                         assert_authorized_paa, assert_profile_advertised, assert_profile_supported_by_test_harness,
                                         find_issuing_paa, is_pqc_profile, kAttestationNonceLength, kCertificateSegmentSize,
                                         load_paa_certificates, parse_attestation_elements, parse_certificate,
                                         retrieve_segmented_document, select_strongest_profile, validate_attestation_chain,
                                         validate_certification_declaration, verify_attestation_signature)

import matter.clusters as Clusters
from matter.testing.credentials import CredentialSource
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_DA_1_12(MatterBaseTest):
    """ML-DSA Device Attestation Request Validation Scenario [DUT-Commissionee]."""

    def desc_TC_DA_1_12(self) -> str:
        return "[TC-DA-1.12] ML-DSA Device Attestation Request Validation Scenario [DUT-Commissionee]"

    def pics_TC_DA_1_12(self) -> list[str]:
        return ["MCORE.ROLE.COMMISSIONEE", "OPCREDS.S", "OPCREDS.S.C00.Rsp", "OPCREDS.S.C01.Tx",
                "OPCREDS.S.C02.Rsp", "OPCREDS.S.C03.Tx"]

    def steps_TC_DA_1_12(self) -> list[TestStep]:
        return [
            TestStep(0, "Commission DUT to TH", is_commissioning=True),
            TestStep(1, "TH reads the Operational Credentials Cluster FeatureMap attribute from the DUT.",
                     "TH verifies that the PQCDA feature bit is set."),
            TestStep(2, "TH reads the PQCDeviceAttestationProfile attribute from the DUT.",
                     "TH stores the advertised profile sets as paaProfiles, paiProfiles and dacProfiles."),
            TestStep(3, "TH selects the highest-security profile in paaProfiles and stores it as selectedPAAProfile. "
                        "TH independently selects the highest-security profile in paiProfiles and stores it as "
                        "selectedPAIProfile.",
                     "For each profile set, TH selects MlDsa65 when it is advertised, otherwise MlDsa44 when it is "
                     "advertised, otherwise EcdsaMatterLegacy."),
            TestStep(4, "TH sends CertificateChainRequest for PAICertificate with CryptoProfile set to "
                        "selectedPAIProfile, SegmentID set to 0, and MaxSegmentSize set to the maximum certificate "
                        "data size that fits in one response on the transport under test. While NextSegmentID is "
                        "present, TH requests the remaining segments and reassembles the certificate as "
                        "profile_pai_cert.",
                     "If either selectedPAAProfile or selectedPAIProfile is a PQC profile, the PAI is returned in "
                     "multiple segments, including when an ECDSA PAI was signed by a PQC PAA. If both profiles are "
                     "EcdsaMatterLegacy, the complete PAI is returned in one response. Segment numbering is "
                     "monotonic from zero; TotalDocumentSize is stable across all segments; NextSegmentID is absent "
                     "on the final response; profile_pai_cert parses as an X.509 certificate; the PAI subject key "
                     "algorithm matches selectedPAIProfile; and the PAI signature algorithm "
                     "matches selectedPAAProfile."),
            TestStep(5, "TH verifies that dacProfiles advertises EcdsaMatterLegacy and stores it as selectedDACProfile.",
                     "selectedDACProfile is EcdsaMatterLegacy."),
            TestStep(6, "TH sends CertificateChainRequest for DACCertificate with CryptoProfile set to "
                        "selectedDACProfile, SegmentID set to 0, and MaxSegmentSize set to the value used for the PAI "
                        "request. While NextSegmentID is present, TH requests the remaining segments and reassembles "
                        "the certificate as profile_dac_cert.",
                     "If the PAI subject key is a PQC profile, the DAC is returned in multiple segments because of "
                     "the PQC signature. If the PAI subject key is EcdsaMatterLegacy, the complete DAC is returned "
                     "in one response. Segment numbering is monotonic from zero; TotalDocumentSize is stable across "
                     "all segments; NextSegmentID is absent on the final response; profile_dac_cert parses as an "
                     "X.509 certificate; the DAC public key algorithm is EcdsaMatterLegacy; and the DAC signature "
                     "algorithm matches the PAI subject key algorithm."),
            TestStep(7, "TH sends the legacy CertificateChainRequest for PAICertificate, including only the "
                        "CertificateType field and omitting CryptoProfile, SegmentID and MaxSegmentSize.",
                     "DUT returns the complete legacy PAI in one CertificateChainResponse. NextSegmentID is absent. "
                     "TH saves the certificate as legacy_pai_cert."),
            TestStep(8, "TH sends the legacy CertificateChainRequest for DACCertificate, including only the "
                        "CertificateType field and omitting CryptoProfile, SegmentID and MaxSegmentSize.",
                     "DUT returns the complete legacy DAC in one CertificateChainResponse. NextSegmentID is absent. "
                     "TH saves the certificate as legacy_dac_cert."),
            TestStep(9, "TH generates a random 32-byte nonce and saves it as attestation_nonce. TH sends "
                        "AttestationRequest with AttestationNonce set to attestation_nonce.",
                     "DUT returns AttestationResponse. TH saves the AttestationElements and AttestationSignature "
                     "fields."),
            TestStep(10, "TH validates the profile-selected certificate chain and the AttestationResponse using "
                         "profile_pai_cert, profile_dac_cert, and the externally obtained PAA certificate for "
                         "selectedPAAProfile.",
                     "The PAA, PAI and DAC form a valid three-certificate attestation chain; the PAI signature "
                     "validates with the PAA public key; the DAC signature validates with the PAI public key; the "
                     "DAC and PAI satisfy the device attestation VID and PID constraints; AttestationSignature "
                     "validates using the public key from profile_dac_cert; the attestation elements contain "
                     "attestation_nonce; and Certification Declaration validation succeeds."),
            TestStep(11, "TH validates the legacy certificate chain and the same AttestationResponse using "
                         "legacy_pai_cert, legacy_dac_cert, and the externally obtained legacy PAA certificate.",
                     "The legacy PAA, PAI and DAC form a valid three-certificate attestation chain; the PAI "
                     "signature validates with the PAA public key; the DAC signature validates with the PAI public "
                     "key; the DAC and PAI satisfy the device attestation VID and PID constraints; "
                     "AttestationSignature validates using the public key from legacy_dac_cert; the attestation "
                     "elements contain attestation_nonce; and Certification Declaration validation succeeds."),
            TestStep(12, "TH completes commissioning of the DUT.", "Commissioning succeeds."),
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
                               f"DUT returned NextSegmentID for the legacy {document_name}, which is always "
                               "returned in a single response")
        asserts.assert_greater(len(response.certificate), 0, f"DUT returned an empty legacy {document_name}")
        logger.info("Retrieved the legacy %s in one response, %d bytes", document_name, len(response.certificate))
        return response.certificate

    def _validate_chain_and_attestation(self, paa, pai, dac, chain_name: str, elements, signature: bytes,
                                        attestation_challenge: bytes, expected_nonce: bytes,
                                        basic_info_vendor_id: int, basic_info_product_id: int,
                                        test_cd_signer_source: CredentialSource | Path,
                                        allow_provisional_test_signer: bool) -> None:
        """Validate one certificate chain and the AttestationResponse against it.

        Shared by steps 10 and 11, which apply the identical set of checks to the profile-selected
        and the legacy chain.
        """
        validate_attestation_chain(paa, pai, dac, chain_name)
        verify_attestation_signature(dac, elements, signature, attestation_challenge)
        assert_attestation_nonce(elements, expected_nonce)
        declaration = validate_certification_declaration(
            elements.certification_declaration, dac, pai, basic_info_vendor_id, basic_info_product_id,
            test_cd_signer_source=test_cd_signer_source,
            allow_provisional_test_signer=allow_provisional_test_signer)
        assert_authorized_paa(declaration, pai)

    @async_test_body
    async def test_TC_DA_1_12(self):
        opcreds = Clusters.OperationalCredentials
        basic_information = Clusters.BasicInformation
        certificate_type = opcreds.Enums.CertificateChainTypeEnum
        # Operational Credentials only ever lives on the root endpoint.
        self.root_endpoint = 0

        # Steps 10 and 11 need PAA certificates the DUT never supplies, so resolve the trust store
        # up front rather than failing once the retrieval work is already done.
        paa_trust_store_path = self.matter_test_config.paa_trust_store_path
        asserts.assert_is_not_none(paa_trust_store_path,
                                   "--paa-trust-store-path is required: pre-condition 1 has the TH obtain the PAA "
                                   "certificates for both the profile-selected and the legacy chain externally")
        paa_candidates = load_paa_certificates(paa_trust_store_path)

        cd_cert_dir = self.user_params.get("cd_cert_dir")
        test_cd_signer_source = CredentialSource.kDevelopment if cd_cert_dir is None else Path(cd_cert_dir)
        allow_provisional_test_signer = self.user_params.get("override_provisional_cd_check_warning", False)

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
        selected_paa_profile = select_strongest_profile(paa_profiles, "PAA")
        selected_pai_profile = select_strongest_profile(pai_profiles, "PAI")

        # PAA and PAI subject-key profiles are negotiated independently.
        # Check the TH's crypto capabilities before retrieving either certificate.
        assert_profile_supported_by_test_harness(selected_paa_profile)
        assert_profile_supported_by_test_harness(selected_pai_profile)
        chain_name = f"{selected_paa_profile.name}/{selected_pai_profile.name}"

        self.step(4)
        # kCertificateSegmentSize is both the smallest MaxSegmentSize the spec allows and the
        # boundary the DUT segments on, so it forces the largest number of response segments.
        pai_document = await retrieve_segmented_document(
            self._send_certificate_chain_request, certificate_type.kPAICertificate, selected_pai_profile, "PAI",
            kCertificateSegmentSize)
        if is_pqc_profile(selected_paa_profile) or is_pqc_profile(selected_pai_profile):
            asserts.assert_greater(pai_document.segment_count, 1,
                                   "A PQC PAA or PAI profile makes the PAI too large for a single 600 byte segment")
        else:
            asserts.assert_equal(pai_document.segment_count, 1,
                                 "An EcdsaMatterLegacy chain must return the complete PAI in one response")

        profile_pai_cert = parse_certificate(pai_document.der, "profile_pai_cert")
        pai_algorithms = profile_pai_cert.algorithms

        # The PAI subject key follows its own profile; its signature follows the issuer's.
        asserts.assert_equal(pai_algorithms.subject_key_profile, selected_pai_profile,
                             "profile_pai_cert public key algorithm must match selectedPAIProfile "
                             f"{selected_pai_profile.name}")
        asserts.assert_equal(pai_algorithms.signature_profile, selected_paa_profile,
                             "profile_pai_cert must be signed with the selectedPAAProfile "
                             f"{selected_paa_profile.name} because the self-signed PAA issued it")

        self.step(5)
        # During PQC Phase 1 the DAC keeps a P-256 subject key so the Device Attestation signature
        # stays EcdsaMatterLegacy, which is what dacProfiles must advertise here.
        assert_profile_advertised(dac_profiles, AttestationCryptoProfile.kEcdsaMatterLegacy, "DAC",
                                  "Device Attestation signature")
        selected_dac_profile = AttestationCryptoProfile.kEcdsaMatterLegacy

        self.step(6)
        # CryptoProfile describes the DAC subject key. The DUT selects the stored chain
        # separately, so an explicit ECDSA request still retrieves the PQC-issued DAC.
        dac_document = await retrieve_segmented_document(
            self._send_certificate_chain_request, certificate_type.kDACCertificate, selected_dac_profile, "DAC",
            kCertificateSegmentSize)

        # What drives the DAC size is the signature the PAI generated over it, so the segmentation
        # expectation follows the PAI subject key rather than the chain profile. A PQC chain may
        # legitimately carry a P-256 PAI, and the DAC it issues then still fits one segment: the
        # smallest ML-DSA signature is 2420 bytes, while an ECDSA-signed DAC is capped at 600.
        if is_pqc_profile(pai_algorithms.subject_key_profile):
            asserts.assert_greater(dac_document.segment_count, 1,
                                   f"A {pai_algorithms.subject_key_profile.name} PAI signature makes the DAC too "
                                   "large for a single 600 byte segment")
        else:
            asserts.assert_equal(dac_document.segment_count, 1,
                                 "A DAC signed by an EcdsaMatterLegacy PAI must be returned in one response")

        profile_dac_cert = parse_certificate(dac_document.der, "profile_dac_cert")
        dac_algorithms = profile_dac_cert.algorithms
        assert_profile_advertised(dac_profiles, dac_algorithms.subject_key_profile, "DAC", "subjectPublicKeyInfo")
        asserts.assert_equal(dac_algorithms.subject_key_profile, selected_dac_profile,
                             "profile_dac_cert must keep an ECDSA subject key so the Device Attestation signature "
                             "stays EcdsaMatterLegacy during PQC Phase 1")
        asserts.assert_equal(dac_algorithms.signature_profile, pai_algorithms.subject_key_profile,
                             "profile_dac_cert must be signed with the profile_pai_cert subject key profile "
                             f"{pai_algorithms.subject_key_profile.name}")

        self.step(7)
        legacy_pai_cert = parse_certificate(
            await self._retrieve_legacy_certificate(certificate_type.kPAICertificate, "PAI"), "legacy_pai_cert")

        self.step(8)
        legacy_dac_cert = parse_certificate(
            await self._retrieve_legacy_certificate(certificate_type.kDACCertificate, "DAC"), "legacy_dac_cert")

        # Steps 10 and 11 verify one AttestationSignature against both DACs, which is only possible
        # when the two carry the same P-256 subject key. Asserting it here reports the cause
        # directly instead of surfacing as an unexplained signature failure in one of those steps.
        profile_dac_public_key = profile_dac_cert.certificate.public_key().public_numbers()
        legacy_dac_public_key = legacy_dac_cert.certificate.public_key().public_numbers()
        asserts.assert_equal(profile_dac_public_key, legacy_dac_public_key,
                             "profile_dac_cert and legacy_dac_cert carry different public keys, so the single "
                             "AttestationSignature cannot validate against both. A DUT serving several DAC "
                             "documents must use one Device Attestation key across them.")

        self.step(9)
        attestation_nonce = random.randbytes(kAttestationNonceLength)
        attestation_response = await self.send_single_cmd(
            cmd=opcreds.Commands.AttestationRequest(attestationNonce=attestation_nonce),
            endpoint=self.root_endpoint)
        asserts.assert_true(isinstance(attestation_response, opcreds.Commands.AttestationResponse),
                            "DUT returned an invalid response to AttestationRequest")
        attestation_elements = parse_attestation_elements(attestation_response.attestationElements)
        attestation_signature = attestation_response.attestationSignature

        # The signature covers the elements plus the challenge from the session it was requested
        # over, so the challenge has to come from that same session.
        proxy = self.default_controller.GetConnectedDeviceSync(self.dut_node_id, False)
        attestation_challenge = proxy.attestationChallenge

        self.step(10)
        basic_info_vendor_id = await self.read_single_attribute_check_success(
            cluster=basic_information, attribute=basic_information.Attributes.VendorID, endpoint=self.root_endpoint)
        basic_info_product_id = await self.read_single_attribute_check_success(
            cluster=basic_information, attribute=basic_information.Attributes.ProductID, endpoint=self.root_endpoint)
        profile_paa_cert = find_issuing_paa(profile_pai_cert, paa_candidates, selected_paa_profile,
                                            chain_name)
        self._validate_chain_and_attestation(profile_paa_cert, profile_pai_cert, profile_dac_cert,
                                             chain_name, attestation_elements,
                                             attestation_signature, attestation_challenge, attestation_nonce,
                                             basic_info_vendor_id, basic_info_product_id, test_cd_signer_source,
                                             allow_provisional_test_signer)

        self.step(11)
        legacy_paa_cert = find_issuing_paa(legacy_pai_cert, paa_candidates,
                                           AttestationCryptoProfile.kEcdsaMatterLegacy, "legacy")
        self._validate_chain_and_attestation(legacy_paa_cert, legacy_pai_cert, legacy_dac_cert, "legacy",
                                             attestation_elements, attestation_signature, attestation_challenge,
                                             attestation_nonce, basic_info_vendor_id, basic_info_product_id,
                                             test_cd_signer_source, allow_provisional_test_signer)

        self.step(12)
        # The framework commissioned the DUT at step 0, so what remains is confirming that
        # commissioning actually took effect: the DUT holds this TH's fabric and answers on the
        # resulting operational session.
        commissioned_fabrics = await self.read_single_attribute_check_success(
            cluster=opcreds, attribute=opcreds.Attributes.CommissionedFabrics, endpoint=self.root_endpoint)
        asserts.assert_greater(commissioned_fabrics, 0, "DUT reports no commissioned fabrics")

        current_fabric_index = await self.read_single_attribute_check_success(
            cluster=opcreds, attribute=opcreds.Attributes.CurrentFabricIndex, endpoint=self.root_endpoint)
        fabrics = await self.read_single_attribute_check_success(
            cluster=opcreds, attribute=opcreds.Attributes.Fabrics, endpoint=self.root_endpoint)
        asserts.assert_in(current_fabric_index, [fabric.fabricIndex for fabric in fabrics],
                          "The fabric this TH commissioned is not present in the DUT Fabrics attribute")
        logger.info("Commissioning succeeded: DUT holds %d fabric(s) and answers on fabric index %d",
                    commissioned_fabrics, current_fabric_index)


if __name__ == "__main__":
    default_matter_test_main()
