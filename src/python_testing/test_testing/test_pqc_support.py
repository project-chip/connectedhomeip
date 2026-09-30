#!/usr/bin/env -S python3 -B
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

"""Unit tests for the shared PQC device attestation helpers.

pqc_support validates attestation certificates in Python rather than through the SDK's C++
crypto, so that the test harness is an independent check on the stack. These tests are what makes
that independence worth having: they run the Python implementation over the same certificate
vectors the C++ suite uses (src/crypto/tests/MlDsaAttestationChain_test_vectors.h), including the
combinations that suite expects to be *rejected*, so the two implementations have to agree.

The fixtures are read out of the C++ header rather than copied, so the vectors cannot drift apart.
"""

import asyncio
import base64
import json
import re
import sys
import unittest
from dataclasses import dataclass
from pathlib import Path

from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.asymmetric.utils import decode_dss_signature
from mobly import signals

from matter.tlv import TLVWriter

_CHIP_ROOT = Path(__file__).resolve().parents[3]
sys.path.append(str(_CHIP_ROOT / "src/python_testing"))

from support_modules.pqc_support import (AttestationCertType, AttestationCryptoProfile,  # noqa: E402
                                         AttestationCryptoProfileBitmap, CertificateChainType, CertificationType,
                                         assert_attestation_certificate_format, assert_attestation_nonce, assert_authorized_paa,
                                         assert_certificate_currently_valid, assert_dac_and_pai_ids, assert_profile_advertised,
                                         certificate_algorithms_for_oids, find_issuing_paa, is_ml_dsa_supported, is_pqc_profile,
                                         kAttestationChallengeLength, kAttestationNonceLength, kCertificateSegmentSize,
                                         kMaxCertificateDocumentSize, kOidEcdsaWithSha256, kOidEcPublicKey, kOidMatterPid,
                                         kOidMatterVid, kOidMlDsa44, kOidMlDsa65, parse_attestation_elements, parse_certificate,
                                         profile_mask, retrieve_segmented_document, select_strongest_profile,
                                         validate_attestation_chain, validate_certification_declaration,
                                         verify_attestation_signature, verify_certificate_signature)

_ML_DSA_VECTORS = _CHIP_ROOT / "src/crypto/tests/MlDsaAttestationChain_test_vectors.h"
_DEV_VECTOR = (_CHIP_ROOT
               / "credentials/development/commissioner_dut/struct_dac_cert_version_v3/test_case_vector.json")
_OFFICIAL_TEST_VECTOR = (_CHIP_ROOT
                         / "credentials/development/commissioner_dut/struct_cd_official_cd/test_case_vector.json")
_PROVISIONAL_TEST_VECTOR = (_CHIP_ROOT
                            / "credentials/development/commissioner_dut/struct_cd_provisional_cd/test_case_vector.json")
_LEGACY_PAA = _CHIP_ROOT / "credentials/development/paa-root-certs/Chip-Test-PAA-FFF1-Cert.der"

# The development attestation certificates all belong to vendor FFF1, product 0x8000.
_kTestVid = 0xFFF1
_kTestPid = 0x8000

_kNeedsMlDsa = unittest.skipUnless(
    is_ml_dsa_supported(),
    "the installed cryptography build does not provide hazmat.primitives.asymmetric.mldsa")


def _load_pem_fixtures() -> dict[str, bytes]:
    """Extract every `constexpr char k...Pem[]` raw-string certificate from the C++ vectors."""
    text = _ML_DSA_VECTORS.read_text()
    fixtures = {}
    for match in re.finditer(r'constexpr char (\w+Pem)\[\] = R"\((.*?)\)"', text, re.DOTALL):
        name, body = match.group(1), match.group(2)
        encoded = "".join(line for line in body.splitlines() if line and not line.startswith("-----"))
        fixtures[name] = base64.b64decode(encoded)
    return fixtures


@dataclass(frozen=True)
class _FakeResponse:
    """Stands in for a CertificateChainResponse."""

    certificate: bytes
    totalDocumentSize: int | None = None
    nextSegmentID: int | None = None


class _FakeSegmentSource:
    """Serves a document the way the DUT does: fixed 600 byte slices indexed by SegmentID."""

    def __init__(self, document: bytes, segment_size: int = kCertificateSegmentSize):
        self.document = document
        self.segment_size = segment_size
        self.requests: list[tuple] = []

    async def __call__(self, certificate_type, crypto_profile, segment_id, max_segment_size):
        self.requests.append((certificate_type, crypto_profile, segment_id, max_segment_size))
        offset = segment_id * self.segment_size
        segment = self.document[offset:offset + self.segment_size]
        has_more = offset + len(segment) < len(self.document)
        return _FakeResponse(certificate=segment,
                             totalDocumentSize=len(self.document),
                             nextSegmentID=segment_id + 1 if has_more else None)


class FixtureTestCase(unittest.TestCase):
    """Base class giving access to the shared certificate vectors."""

    @classmethod
    def setUpClass(cls):
        cls.fixtures = _load_pem_fixtures()

    def certificate(self, fixture_name: str):
        self.assertIn(fixture_name, self.fixtures, f"{_ML_DSA_VECTORS.name} does not define {fixture_name}")
        return parse_certificate(self.fixtures[fixture_name], fixture_name)

    def legacy_paa(self):
        return parse_certificate(_LEGACY_PAA.read_bytes(), "legacy PAA")


class TestProfileSelection(unittest.TestCase):
    def test_selects_ml_dsa_65_over_weaker_profiles(self):
        bitmap = (AttestationCryptoProfileBitmap.kSupportsEcdsaMatterLegacy
                  | AttestationCryptoProfileBitmap.kSupportsMlDsa44
                  | AttestationCryptoProfileBitmap.kSupportsMlDsa65)
        self.assertEqual(select_strongest_profile(int(bitmap), "PAA"), AttestationCryptoProfile.kMlDsa65)

    def test_selects_ml_dsa_44_when_65_is_absent(self):
        bitmap = (AttestationCryptoProfileBitmap.kSupportsEcdsaMatterLegacy
                  | AttestationCryptoProfileBitmap.kSupportsMlDsa44)
        self.assertEqual(select_strongest_profile(int(bitmap), "PAI"), AttestationCryptoProfile.kMlDsa44)

    def test_selects_legacy_when_it_is_the_only_profile(self):
        bitmap = AttestationCryptoProfileBitmap.kSupportsEcdsaMatterLegacy
        self.assertEqual(select_strongest_profile(int(bitmap), "DAC"), AttestationCryptoProfile.kEcdsaMatterLegacy)

    def test_empty_bitmap_fails(self):
        with self.assertRaises(signals.TestFailure):
            select_strongest_profile(0, "PAA")

    def test_profile_mask_matches_the_generated_bitmap(self):
        self.assertEqual(profile_mask(AttestationCryptoProfile.kEcdsaMatterLegacy), 0x01)
        self.assertEqual(profile_mask(AttestationCryptoProfile.kMlDsa44), 0x02)
        self.assertEqual(profile_mask(AttestationCryptoProfile.kMlDsa65), 0x04)

    def test_only_ml_dsa_profiles_are_post_quantum(self):
        self.assertFalse(is_pqc_profile(AttestationCryptoProfile.kEcdsaMatterLegacy))
        self.assertTrue(is_pqc_profile(AttestationCryptoProfile.kMlDsa44))
        self.assertTrue(is_pqc_profile(AttestationCryptoProfile.kMlDsa65))

    def test_advertised_assertion_accepts_a_profile_in_the_bitmap(self):
        bitmap = int(AttestationCryptoProfileBitmap.kSupportsEcdsaMatterLegacy
                     | AttestationCryptoProfileBitmap.kSupportsMlDsa65)
        # Exercise membership for both bits in a synthetic bitmap. The field labels are diagnostic;
        # this does not imply that a DAC bitmap advertises its issuer's signature algorithm.
        assert_profile_advertised(bitmap, AttestationCryptoProfile.kEcdsaMatterLegacy, "DAC", "subjectPublicKeyInfo")
        assert_profile_advertised(bitmap, AttestationCryptoProfile.kMlDsa65, "DAC", "signatureAlgorithm")

    def test_advertised_assertion_rejects_a_profile_outside_the_bitmap(self):
        bitmap = int(AttestationCryptoProfileBitmap.kSupportsEcdsaMatterLegacy)
        with self.assertRaises(signals.TestFailure):
            assert_profile_advertised(bitmap, AttestationCryptoProfile.kMlDsa44, "PAI", "signatureAlgorithm")


class TestAlgorithmOidMapping(unittest.TestCase):
    def test_maps_key_and_signature_oids_independently(self):
        # The PQC Phase 1 DAC shape: a P-256 subject key under an ML-DSA-65 issuer signature.
        algorithms = certificate_algorithms_for_oids(kOidMlDsa65, kOidEcPublicKey)
        self.assertEqual(algorithms.subject_key_profile, AttestationCryptoProfile.kEcdsaMatterLegacy)
        self.assertEqual(algorithms.signature_profile, AttestationCryptoProfile.kMlDsa65)

    def test_maps_ml_dsa_44(self):
        algorithms = certificate_algorithms_for_oids(kOidMlDsa44, kOidMlDsa44)
        self.assertEqual(algorithms.subject_key_profile, AttestationCryptoProfile.kMlDsa44)
        self.assertEqual(algorithms.signature_profile, AttestationCryptoProfile.kMlDsa44)

    def test_maps_legacy_ecdsa(self):
        algorithms = certificate_algorithms_for_oids(kOidEcdsaWithSha256, kOidEcPublicKey)
        self.assertEqual(algorithms.subject_key_profile, AttestationCryptoProfile.kEcdsaMatterLegacy)
        self.assertEqual(algorithms.signature_profile, AttestationCryptoProfile.kEcdsaMatterLegacy)

    def test_rejects_an_unknown_signature_oid(self):
        with self.assertRaises(signals.TestFailure):
            certificate_algorithms_for_oids("1.2.3.4", kOidEcPublicKey)

    def test_rejects_an_unknown_public_key_oid(self):
        with self.assertRaises(signals.TestFailure):
            certificate_algorithms_for_oids(kOidMlDsa44, "1.2.3.4")


@_kNeedsMlDsa
class TestCertificateParsing(FixtureTestCase):
    def test_parses_the_algorithms_of_every_chain_shape(self):
        expected = {
            # fixture: (subject key profile, signature profile)
            "kMlDsa65PaaPem": (AttestationCryptoProfile.kMlDsa65, AttestationCryptoProfile.kMlDsa65),
            "kMlDsa65PaiPem": (AttestationCryptoProfile.kMlDsa65, AttestationCryptoProfile.kMlDsa65),
            "kMlDsa65PaiDacPem": (AttestationCryptoProfile.kEcdsaMatterLegacy, AttestationCryptoProfile.kMlDsa65),
            # The subject key and issuer signature can use different algorithms: the PAI's
            # subject-key profile is independent of the PAA's key profile used to sign it.
            "kMlDsa44PaiUnderMlDsa65PaaPem": (AttestationCryptoProfile.kMlDsa44, AttestationCryptoProfile.kMlDsa65),
            "kP256PaiUnderMlDsa65PaaPem": (AttestationCryptoProfile.kEcdsaMatterLegacy,
                                           AttestationCryptoProfile.kMlDsa65),
            "kMlDsa44PaaPem": (AttestationCryptoProfile.kMlDsa44, AttestationCryptoProfile.kMlDsa44),
            "kMlDsa44PaiPem": (AttestationCryptoProfile.kMlDsa44, AttestationCryptoProfile.kMlDsa44),
            "kMlDsa44PaiDacPem": (AttestationCryptoProfile.kEcdsaMatterLegacy, AttestationCryptoProfile.kMlDsa44),
            "kP256PaiUnderMlDsa44PaaPem": (AttestationCryptoProfile.kEcdsaMatterLegacy,
                                           AttestationCryptoProfile.kMlDsa44),
        }
        for fixture_name, (subject_key_profile, signature_profile) in expected.items():
            with self.subTest(fixture=fixture_name):
                algorithms = self.certificate(fixture_name).algorithms
                self.assertEqual(algorithms.subject_key_profile, subject_key_profile)
                self.assertEqual(algorithms.signature_profile, signature_profile)

    def test_parses_a_legacy_ecdsa_certificate(self):
        algorithms = self.legacy_paa().algorithms
        self.assertEqual(algorithms.subject_key_profile, AttestationCryptoProfile.kEcdsaMatterLegacy)
        self.assertEqual(algorithms.signature_profile, AttestationCryptoProfile.kEcdsaMatterLegacy)

    def test_identifies_self_issued_paa_certificates(self):
        self.assertTrue(self.certificate("kMlDsa65PaaPem").is_self_issued)
        self.assertTrue(self.certificate("kMlDsa44PaaPem").is_self_issued)
        self.assertFalse(self.certificate("kMlDsa65PaiPem").is_self_issued)
        self.assertFalse(self.certificate("kMlDsa65PaiDacPem").is_self_issued)

    def test_extracts_the_matter_vid_and_pid(self):
        dac = self.certificate("kMlDsa65PaiDacPem")
        self.assertEqual(dac.matter_id(kOidMatterVid), _kTestVid)
        self.assertEqual(dac.matter_id(kOidMatterPid), _kTestPid)

        # A PAI carries a VID but is not required to carry a PID.
        pai = self.certificate("kMlDsa65PaiPem")
        self.assertEqual(pai.matter_id(kOidMatterVid), _kTestVid)
        self.assertIsNone(pai.matter_id(kOidMatterPid))

    def test_rejects_trailing_bytes_after_the_certificate(self):
        with self.assertRaises(signals.TestFailure):
            parse_certificate(self.fixtures["kMlDsa44PaaPem"] + b"\x00", "padded PAA")

    def test_rejects_a_truncated_certificate(self):
        with self.assertRaises(signals.TestFailure):
            parse_certificate(self.fixtures["kMlDsa44PaaPem"][:512], "truncated PAA")

    def test_rejects_a_document_that_is_not_a_certificate(self):
        with self.assertRaises(signals.TestFailure):
            parse_certificate(b"\x02\x01\x00", "an INTEGER")

    def test_rejects_an_empty_document(self):
        with self.assertRaises(signals.TestFailure):
            parse_certificate(b"", "nothing")


class TestLegacyPaa(FixtureTestCase):
    """Validate the legacy PAA without requiring ML-DSA support."""

    def test_accepts_the_legacy_paa(self):
        assert_attestation_certificate_format(self.legacy_paa(), AttestationCertType.PAA)

    def test_validates_the_legacy_ecdsa_self_signature(self):
        paa = self.legacy_paa()
        verify_certificate_signature(paa, paa)


@_kNeedsMlDsa
class TestCertificateFormatRequirements(FixtureTestCase):
    """Cross-checks assert_attestation_certificate_format against the C++ suite's expectations."""

    def test_accepts_every_valid_chain_role(self):
        roles = (
            ("kMlDsa65PaaPem", AttestationCertType.PAA),
            ("kMlDsa44PaaPem", AttestationCertType.PAA),
            ("kMlDsa65PaiPem", AttestationCertType.PAI),
            ("kMlDsa44PaiPem", AttestationCertType.PAI),
            ("kMlDsa44PaiUnderMlDsa65PaaPem", AttestationCertType.PAI),
            ("kP256PaiUnderMlDsa65PaaPem", AttestationCertType.PAI),
            ("kP256PaiUnderMlDsa44PaaPem", AttestationCertType.PAI),
            ("kMlDsa65PaiDacPem", AttestationCertType.DAC),
            ("kMlDsa44PaiDacPem", AttestationCertType.DAC),
            ("kMlDsa44PaiUnderMlDsa65PaaDacPem", AttestationCertType.DAC),
            ("kP256PaiUnderMlDsa65PaaDacPem", AttestationCertType.DAC),
            ("kP256PaiUnderMlDsa44PaaDacPem", AttestationCertType.DAC),
        )
        for fixture_name, cert_type in roles:
            with self.subTest(fixture=fixture_name, role=cert_type.value):
                assert_attestation_certificate_format(self.certificate(fixture_name), cert_type)

    def test_rejects_a_key_stronger_than_its_issuer(self):
        # These two are the C++ suite's "must be rejected" vectors: a certificate whose key is
        # stronger than the algorithm that signed it. A weaker issuer cannot protect a stronger
        # key below it.
        for fixture_name in ("kMlDsa65PaiUnderMlDsa44PaaPem", "kMlDsa65PaiUnderP256PaaPem"):
            with self.subTest(fixture=fixture_name), self.assertRaises(signals.TestFailure):
                assert_attestation_certificate_format(self.certificate(fixture_name), AttestationCertType.PAI)

    def test_rejects_a_dac_that_does_not_carry_a_p256_key(self):
        # PQC Phase 1 keeps the Device Attestation signature on P-256, so an ML-DSA-keyed DAC is
        # invalid however it was signed.
        with self.assertRaises(signals.TestFailure):
            assert_attestation_certificate_format(self.certificate("kMlDsa44KeyedDacPem"), AttestationCertType.DAC)

    def test_rejects_a_ca_presented_as_a_dac(self):
        # A PAI sets basicConstraints CA and keyCertSign, which a DAC must not.
        with self.assertRaises(signals.TestFailure):
            assert_attestation_certificate_format(self.certificate("kMlDsa65PaiPem"), AttestationCertType.DAC)

    def test_rejects_a_dac_presented_as_a_ca(self):
        with self.assertRaises(signals.TestFailure):
            assert_attestation_certificate_format(self.certificate("kMlDsa65PaiDacPem"), AttestationCertType.PAI)

    def test_rejects_a_pai_presented_as_a_paa(self):
        # A PAA must be self-signed with its own key algorithm and carry pathlen absent or 1; the
        # PAI has pathlen 0.
        with self.assertRaises(signals.TestFailure):
            assert_attestation_certificate_format(self.certificate("kP256PaiUnderMlDsa65PaaPem"),
                                                  AttestationCertType.PAA)

    def test_every_fixture_is_currently_valid(self):
        for fixture_name in ("kMlDsa65PaaPem", "kMlDsa44PaaPem", "kMlDsa65PaiPem", "kMlDsa65PaiDacPem"):
            with self.subTest(fixture=fixture_name):
                assert_certificate_currently_valid(self.certificate(fixture_name))


@_kNeedsMlDsa
class TestChainValidation(FixtureTestCase):
    # Every attestation chain in the C++ vectors, as (PAA, PAI, DAC) fixture names.
    _kChains = (
        ("kMlDsa65PaaPem", "kMlDsa65PaiPem", "kMlDsa65PaiDacPem"),
        ("kMlDsa65PaaPem", "kMlDsa44PaiUnderMlDsa65PaaPem", "kMlDsa44PaiUnderMlDsa65PaaDacPem"),
        ("kMlDsa65PaaPem", "kP256PaiUnderMlDsa65PaaPem", "kP256PaiUnderMlDsa65PaaDacPem"),
        ("kMlDsa44PaaPem", "kMlDsa44PaiPem", "kMlDsa44PaiDacPem"),
        ("kMlDsa44PaaPem", "kP256PaiUnderMlDsa44PaaPem", "kP256PaiUnderMlDsa44PaaDacPem"),
    )

    def test_validates_every_attestation_chain(self):
        for paa_name, pai_name, dac_name in self._kChains:
            with self.subTest(chain=f"{paa_name}/{pai_name}/{dac_name}"):
                paa, pai, dac = (self.certificate(name) for name in (paa_name, pai_name, dac_name))
                validate_attestation_chain(paa, pai, dac, pai_name)

    def test_rejects_a_pai_signed_by_a_different_paa(self):
        # The ML-DSA-44 PAI belongs to the ML-DSA-44 PAA, so the ML-DSA-65 PAA must not validate it.
        with self.assertRaises(signals.TestFailure):
            verify_certificate_signature(self.certificate("kMlDsa44PaiPem"), self.certificate("kMlDsa65PaaPem"))

    def test_rejects_a_tampered_signature(self):
        dac_der = bytearray(self.fixtures["kMlDsa65PaiDacPem"])
        # Flipping a bit in the trailing signature octets leaves the DER structure intact.
        dac_der[-1] ^= 0x01
        dac = parse_certificate(bytes(dac_der), "tampered DAC")
        with self.assertRaises(signals.TestFailure):
            verify_certificate_signature(dac, self.certificate("kMlDsa65PaiPem"))

    def test_rejects_a_tampered_tbs_certificate(self):
        # Editing a digit of the UTCTime validity keeps the DER structurally valid and the same
        # length, so the certificate still parses but no longer matches its signature.
        der = bytearray(self.fixtures["kMlDsa65PaiDacPem"])
        validity = re.search(rb"\x17\x0d(\d{12})Z", bytes(der))
        self.assertIsNotNone(validity, "the fixture does not encode notBefore as a UTCTime")
        year_offset = validity.start(1)
        der[year_offset] = ord("3") if der[year_offset] != ord("3") else ord("2")

        dac = parse_certificate(bytes(der), "tampered DAC body")
        # The mutation must land in the signed body, not merely in the signature bytes.
        self.assertNotEqual(dac.certificate.tbs_certificate_bytes,
                            self.certificate("kMlDsa65PaiDacPem").certificate.tbs_certificate_bytes)
        with self.assertRaises(signals.TestFailure):
            verify_certificate_signature(dac, self.certificate("kMlDsa65PaiPem"))

    def test_rejects_a_chain_whose_dac_was_issued_by_another_pai(self):
        with self.assertRaises(signals.TestFailure):
            validate_attestation_chain(self.certificate("kMlDsa44PaaPem"), self.certificate("kMlDsa44PaiPem"),
                                       self.certificate("kMlDsa65PaiDacPem"), "mismatched")

    def test_rejects_a_chain_whose_root_is_not_self_issued(self):
        # The PAI is a CA but not a root, so it cannot stand in for the PAA.
        with self.assertRaises(signals.TestFailure):
            validate_attestation_chain(self.certificate("kMlDsa65PaiPem"), self.certificate("kMlDsa65PaiPem"),
                                       self.certificate("kMlDsa65PaiDacPem"), "non-root")


@_kNeedsMlDsa
class TestDacAndPaiIds(FixtureTestCase):
    def test_accepts_matching_vid_and_pid(self):
        assert_dac_and_pai_ids(self.certificate("kMlDsa65PaiDacPem"), self.certificate("kMlDsa65PaiPem"),
                               self.certificate("kMlDsa65PaaPem"), "ML-DSA-65")

    def test_rejects_a_paa_carrying_a_pid(self):
        # A DAC carries a PID, so standing it in for the PAA must be rejected.
        with self.assertRaises(signals.TestFailure):
            assert_dac_and_pai_ids(self.certificate("kMlDsa65PaiDacPem"), self.certificate("kMlDsa65PaiPem"),
                                   self.certificate("kMlDsa65PaiDacPem"), "bad PAA")


@_kNeedsMlDsa
class TestPaaResolution(FixtureTestCase):
    def test_resolves_the_paa_that_issued_the_pai(self):
        candidates = [self.certificate(name) for name in ("kMlDsa44PaaPem", "kMlDsa65PaaPem")]
        pai = self.certificate("kMlDsa65PaiPem")
        resolved = find_issuing_paa(pai, candidates, AttestationCryptoProfile.kMlDsa65, "ML-DSA-65")
        self.assertEqual(resolved.algorithms.subject_key_profile, AttestationCryptoProfile.kMlDsa65)
        verify_certificate_signature(pai, resolved)

    def test_resolves_a_p256_pai_to_its_pqc_paa(self):
        # The PAI key is P-256 while the PAA that signed it is ML-DSA-65, so resolution must follow
        # the requested PAA profile rather than the PAI key.
        candidates = [self.certificate(name) for name in ("kMlDsa44PaaPem", "kMlDsa65PaaPem")]
        pai = self.certificate("kP256PaiUnderMlDsa65PaaPem")
        resolved = find_issuing_paa(pai, candidates, AttestationCryptoProfile.kMlDsa65, "mixed")
        self.assertEqual(resolved.algorithms.subject_key_profile, AttestationCryptoProfile.kMlDsa65)

    def test_fails_when_no_candidate_uses_the_requested_profile(self):
        with self.assertRaises(signals.TestFailure):
            find_issuing_paa(self.certificate("kMlDsa65PaiPem"), [self.certificate("kMlDsa44PaaPem")],
                             AttestationCryptoProfile.kMlDsa65, "ML-DSA-65")

    def test_fails_when_no_candidate_matches_the_pai_issuer(self):
        with self.assertRaises(signals.TestFailure):
            find_issuing_paa(self.certificate("kMlDsa65PaiPem"), [self.legacy_paa()],
                             AttestationCryptoProfile.kEcdsaMatterLegacy, "legacy")


class TestSegmentedRetrieval(unittest.TestCase):
    def _retrieve(self, source, document_name="PAI", max_segment_size=kCertificateSegmentSize):
        return asyncio.run(retrieve_segmented_document(
            source, CertificateChainType.kPAICertificate, AttestationCryptoProfile.kMlDsa65, document_name,
            max_segment_size))

    def test_reassembles_a_multi_segment_document(self):
        document = bytes(range(256)) * 10  # 2560 bytes, so five 600 byte segments.
        source = _FakeSegmentSource(document)
        result = self._retrieve(source)

        self.assertEqual(result.der, document)
        self.assertEqual(result.total_document_size, len(document))
        self.assertEqual(result.segment_count, 5)
        # SegmentIDs must be requested in order starting at zero.
        self.assertEqual([request[2] for request in source.requests], [0, 1, 2, 3, 4])
        for certificate_type, crypto_profile, _, _ in source.requests:
            # Both enum values are 2; identity also catches swapped argument types.
            self.assertIs(certificate_type, CertificateChainType.kPAICertificate)
            self.assertIs(crypto_profile, AttestationCryptoProfile.kMlDsa65)

    def test_continues_from_an_already_received_first_segment(self):
        document = b"\xAA" * 1200
        source = _FakeSegmentSource(document)
        first_response = asyncio.run(source(
            CertificateChainType.kPAICertificate, AttestationCryptoProfile.kMlDsa65, 0, kCertificateSegmentSize))

        result = asyncio.run(retrieve_segmented_document(
            source, CertificateChainType.kPAICertificate, AttestationCryptoProfile.kMlDsa65, "PAI",
            first_response=first_response))

        self.assertEqual(result.der, document)
        self.assertEqual([request[2] for request in source.requests], [0, 1])
        for certificate_type, crypto_profile, _, _ in source.requests:
            self.assertIs(certificate_type, CertificateChainType.kPAICertificate)
            self.assertIs(crypto_profile, AttestationCryptoProfile.kMlDsa65)

    def test_returns_a_single_segment_document_without_further_requests(self):
        document = b"\xAA" * 400
        source = _FakeSegmentSource(document)
        result = self._retrieve(source, document_name="DAC")

        self.assertEqual(result.der, document)
        self.assertEqual(result.segment_count, 1)
        self.assertEqual(len(source.requests), 1)

    def test_accepts_a_single_segment_larger_than_the_default_segment_size(self):
        document = b"\xAA" * 700
        source = _FakeSegmentSource(document, segment_size=1000)
        result = self._retrieve(source, document_name="DAC", max_segment_size=1000)

        self.assertEqual(result.der, document)
        self.assertEqual(result.segment_count, 1)
        self.assertEqual(len(source.requests), 1)

    def test_reassembles_a_document_that_exactly_fills_its_segments(self):
        document = b"\xBB" * (2 * kCertificateSegmentSize)
        result = self._retrieve(_FakeSegmentSource(document))
        self.assertEqual(result.der, document)
        self.assertEqual(result.segment_count, 2)

    def test_reassembles_a_real_ml_dsa_65_certificate(self):
        # 5579 bytes, the size the DUT actually has to segment for an ML-DSA-65 PAI.
        document = _load_pem_fixtures()["kMlDsa65PaiPem"]
        result = self._retrieve(_FakeSegmentSource(document))
        self.assertEqual(result.der, document)
        self.assertEqual(result.segment_count, 10)

    def test_rejects_a_missing_total_document_size(self):
        async def send(certificate_type, crypto_profile, segment_id, max_segment_size):
            return _FakeResponse(certificate=b"\x01" * 100)

        with self.assertRaises(signals.TestFailure):
            self._retrieve(send)

    def test_rejects_an_empty_first_segment(self):
        async def send(certificate_type, crypto_profile, segment_id, max_segment_size):
            return _FakeResponse(certificate=b"", totalDocumentSize=100, nextSegmentID=1)

        with self.assertRaises(signals.TestFailure):
            self._retrieve(send)

    def test_rejects_a_document_larger_than_the_pqc_certificate_maximum(self):
        source = _FakeSegmentSource(b"\x01" * (kMaxCertificateDocumentSize + 1))

        with self.assertRaises(signals.TestFailure):
            self._retrieve(source)

        self.assertEqual(len(source.requests), 1)

    def test_rejects_a_next_segment_id_that_is_not_monotonic(self):
        async def send(certificate_type, crypto_profile, segment_id, max_segment_size):
            # Skips SegmentID 1 and jumps straight to 2.
            return _FakeResponse(certificate=b"\x01" * kCertificateSegmentSize, totalDocumentSize=1800,
                                 nextSegmentID=2)

        with self.assertRaises(signals.TestFailure):
            self._retrieve(send)

    def test_rejects_an_unstable_total_document_size(self):
        sizes = iter((1800, 1200))

        async def send(certificate_type, crypto_profile, segment_id, max_segment_size):
            return _FakeResponse(certificate=b"\x01" * kCertificateSegmentSize, totalDocumentSize=next(sizes),
                                 nextSegmentID=segment_id + 1)

        with self.assertRaises(signals.TestFailure):
            self._retrieve(send)

    def test_rejects_a_next_segment_id_on_a_single_segment_document(self):
        async def send(certificate_type, crypto_profile, segment_id, max_segment_size):
            return _FakeResponse(certificate=b"\x01" * 100, totalDocumentSize=100, nextSegmentID=1)

        with self.assertRaises(signals.TestFailure):
            self._retrieve(send)

    def test_rejects_a_missing_next_segment_id_on_a_multi_segment_document(self):
        async def send(certificate_type, crypto_profile, segment_id, max_segment_size):
            return _FakeResponse(certificate=b"\x01" * kCertificateSegmentSize, totalDocumentSize=1800)

        with self.assertRaises(signals.TestFailure):
            self._retrieve(send)

    def test_rejects_segments_that_overrun_the_total_document_size(self):
        async def send(certificate_type, crypto_profile, segment_id, max_segment_size):
            return _FakeResponse(certificate=b"\x01" * kCertificateSegmentSize, totalDocumentSize=700,
                                 nextSegmentID=segment_id + 1)

        with self.assertRaises(signals.TestFailure):
            self._retrieve(send)

    def test_rejects_a_segment_larger_than_the_requested_max_segment_size(self):
        requests = []

        async def send(certificate_type, crypto_profile, segment_id, max_segment_size):
            requests.append(segment_id)
            oversized = b"\x01" * (kCertificateSegmentSize + 1)
            return _FakeResponse(certificate=oversized, totalDocumentSize=4000,
                                 nextSegmentID=segment_id + 1 if segment_id == 0 else None)

        with self.assertRaises(signals.TestFailure):
            self._retrieve(send)

        self.assertEqual(requests, [0])

    def test_rejects_a_next_segment_id_that_cannot_be_requested(self):
        source = _FakeSegmentSource(b"\x01" * 102, segment_size=1)

        with self.assertRaises(signals.TestFailure):
            self._retrieve(source)

        self.assertEqual(source.requests[-1][2], 100)


class TestAttestationResponse(unittest.TestCase):
    """Exercises the AttestationResponse handling against the development credential vector.

    The vector carries a real CD, DAC, PAI and DAC private key, so an AttestationResponse can be
    built and then verified the same way the test verifies a DUT's.
    """

    @classmethod
    def setUpClass(cls):
        vector = json.loads(_DEV_VECTOR.read_text())
        cls.dac = parse_certificate(bytes.fromhex(vector["dac_cert"]), "vector DAC")
        cls.pai = parse_certificate(bytes.fromhex(vector["pai_cert"]), "vector PAI")
        cls.cd_der = bytes.fromhex(vector["certification_declaration"])
        cls.dac_private_key = ec.derive_private_key(int(vector["dac_private_key"], 16), ec.SECP256R1())
        cls.nonce = bytes(range(kAttestationNonceLength))
        cls.challenge = bytes(range(kAttestationChallengeLength))
        cls.elements_tlv = cls._build_elements(cls.cd_der, cls.nonce)

    @staticmethod
    def _build_elements(cd_der: bytes, nonce: bytes, timestamp: int = 0) -> bytes:
        """Encode an AttestationElements TLV the way a device would."""
        writer = TLVWriter()
        writer.put(None, {1: cd_der, 2: nonce, 3: timestamp})
        return bytes(writer.encoding)

    def _sign(self, elements: bytes, challenge: bytes) -> bytes:
        """Produce a raw r||s Device Attestation signature over elements || challenge."""
        der_signature = self.dac_private_key.sign(elements + challenge, ec.ECDSA(hashes.SHA256()))
        r, s = decode_dss_signature(der_signature)
        return r.to_bytes(32, "big") + s.to_bytes(32, "big")

    def test_parses_the_attestation_elements(self):
        elements = parse_attestation_elements(self.elements_tlv)
        self.assertEqual(elements.certification_declaration, self.cd_der)
        self.assertEqual(elements.attestation_nonce, self.nonce)
        self.assertIsNone(elements.firmware_information)

    def test_rejects_elements_missing_the_nonce(self):
        writer = TLVWriter()
        writer.put(None, {1: self.cd_der, 3: 0})
        with self.assertRaises(signals.TestFailure):
            parse_attestation_elements(bytes(writer.encoding))

    def test_rejects_oversized_elements(self):
        with self.assertRaises(signals.TestFailure):
            parse_attestation_elements(b"\x15" + b"\x00" * 1000)

    def test_rejects_empty_elements(self):
        with self.assertRaises(signals.TestFailure):
            parse_attestation_elements(b"")

    def test_accepts_a_matching_nonce(self):
        assert_attestation_nonce(parse_attestation_elements(self.elements_tlv), self.nonce)

    def test_rejects_a_mismatched_nonce(self):
        elements = parse_attestation_elements(self.elements_tlv)
        with self.assertRaises(signals.TestFailure):
            assert_attestation_nonce(elements, bytes(kAttestationNonceLength))

    def test_verifies_a_genuine_attestation_signature(self):
        elements = parse_attestation_elements(self.elements_tlv)
        verify_attestation_signature(self.dac, elements, self._sign(self.elements_tlv, self.challenge),
                                     self.challenge)

    def test_rejects_a_signature_over_a_different_challenge(self):
        elements = parse_attestation_elements(self.elements_tlv)
        other_challenge = bytes(kAttestationChallengeLength)
        signature = self._sign(self.elements_tlv, other_challenge)
        with self.assertRaises(signals.TestFailure):
            verify_attestation_signature(self.dac, elements, signature, self.challenge)

    def test_rejects_a_signature_over_different_elements(self):
        other_elements = self._build_elements(self.cd_der, bytes(kAttestationNonceLength))
        signature = self._sign(other_elements, self.challenge)
        with self.assertRaises(signals.TestFailure):
            verify_attestation_signature(self.dac, parse_attestation_elements(self.elements_tlv), signature,
                                         self.challenge)

    def test_rejects_a_wrong_length_challenge(self):
        elements = parse_attestation_elements(self.elements_tlv)
        with self.assertRaises(signals.TestFailure):
            verify_attestation_signature(self.dac, elements, self._sign(self.elements_tlv, self.challenge), b"short")

    def test_rejects_a_wrong_length_signature(self):
        elements = parse_attestation_elements(self.elements_tlv)
        with self.assertRaises(signals.TestFailure):
            verify_attestation_signature(self.dac, elements, b"\x00" * 63, self.challenge)

    def test_validates_the_certification_declaration(self):
        declaration = validate_certification_declaration(
            self.cd_der, self.dac, self.pai, _kTestVid, _kTestPid)
        self.assertEqual(declaration.format_version, 1)
        self.assertEqual(declaration.security_level, 0)
        self.assertEqual(declaration.security_information, 0)
        self.assertIn(declaration.certification_type, list(CertificationType))
        self.assertIn(self.dac.matter_id(kOidMatterPid), declaration.product_id_array)
        self.assertEqual(self.dac.matter_id(kOidMatterVid), declaration.vendor_id)

    def test_rejects_a_tampered_certification_declaration(self):
        tampered = bytearray(self.cd_der)
        # The last octets are the signature, so flipping one leaves the CMS structure intact.
        tampered[-1] ^= 0x01
        with self.assertRaises(signals.TestFailure):
            validate_certification_declaration(bytes(tampered), self.dac, self.pai, _kTestVid, _kTestPid)

    def test_rejects_a_declaration_that_is_not_cms(self):
        with self.assertRaises(signals.TestFailure):
            validate_certification_declaration(
                b"\x30\x03\x02\x01\x00", self.dac, self.pai, _kTestVid, _kTestPid)

    def test_rejects_a_basic_information_vendor_id_mismatch(self):
        with self.assertRaises(signals.TestFailure):
            validate_certification_declaration(
                self.cd_der, self.dac, self.pai, _kTestVid - 1, _kTestPid)

    def test_rejects_a_basic_information_product_id_mismatch(self):
        with self.assertRaises(signals.TestFailure):
            validate_certification_declaration(
                self.cd_der, self.dac, self.pai, _kTestVid, _kTestPid - 1)

    def test_rejects_an_official_declaration_signed_by_a_test_key(self):
        vector = json.loads(_OFFICIAL_TEST_VECTOR.read_text())
        dac = parse_certificate(bytes.fromhex(vector["dac_cert"]), "official vector DAC")
        pai = parse_certificate(bytes.fromhex(vector["pai_cert"]), "official vector PAI")
        with self.assertRaises(signals.TestFailure):
            validate_certification_declaration(
                bytes.fromhex(vector["certification_declaration"]), dac, pai, _kTestVid, vector["basic_info_pid"])

    def test_provisional_test_signer_requires_the_override(self):
        vector = json.loads(_PROVISIONAL_TEST_VECTOR.read_text())
        dac = parse_certificate(bytes.fromhex(vector["dac_cert"]), "provisional vector DAC")
        pai = parse_certificate(bytes.fromhex(vector["pai_cert"]), "provisional vector PAI")
        cd_der = bytes.fromhex(vector["certification_declaration"])

        with self.assertRaises(signals.TestFailure):
            validate_certification_declaration(cd_der, dac, pai, _kTestVid, vector["basic_info_pid"])

        declaration = validate_certification_declaration(
            cd_der, dac, pai, _kTestVid, vector["basic_info_pid"],
            allow_provisional_test_signer=True)
        self.assertEqual(declaration.certification_type, CertificationType.kProvisional)

    def test_authorized_paa_check_is_a_no_op_without_the_list(self):
        declaration = validate_certification_declaration(
            self.cd_der, self.dac, self.pai, _kTestVid, _kTestPid)
        if declaration.authorized_paa_list is None:
            assert_authorized_paa(declaration, self.pai)
        else:
            self.skipTest("the vector CD carries an authorized_paa_list")


if __name__ == "__main__":
    unittest.main()
