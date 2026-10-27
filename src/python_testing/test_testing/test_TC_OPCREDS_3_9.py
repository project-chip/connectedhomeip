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

import sys
import unittest
from pathlib import Path
from types import MethodType, SimpleNamespace

from mobly import signals

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status

_CHIP_ROOT = Path(__file__).resolve().parents[3]
sys.path.append(str(_CHIP_ROOT / "src/python_testing"))

from support_modules.pqc_support import (OperationalCredentialsFeature, is_ml_dsa_supported, kCertificateSegmentSize,  # noqa: E402
                                         profile_mask)
from TC_OPCREDS_3_9 import (TC_OPCREDS_3_9, AttestationCryptoProfile, CertificateAlgorithms, kInvalidCertificateType,  # noqa: E402
                            kOversizedMaxSegmentSize)
from test_pqc_support import _load_pem_fixtures  # noqa: E402


def _algorithms(subject_key_profile: AttestationCryptoProfile,
                signature_profile: AttestationCryptoProfile) -> CertificateAlgorithms:
    return CertificateAlgorithms(
        subject_key_profile=subject_key_profile,
        signature_profile=signature_profile,
        subject_public_key_algorithm_oid="subject-key-oid",
        signature_algorithm_oid="signature-oid",
    )


class TestTCOPCREDS39CertificateProfiles(unittest.TestCase):
    def setUp(self):
        self.test_case = TC_OPCREDS_3_9.__new__(TC_OPCREDS_3_9)

    def test_accepts_independently_selected_paa_and_pai_profiles(self):
        self.test_case._assert_certificate_profiles(
            _algorithms(AttestationCryptoProfile.kMlDsa44, AttestationCryptoProfile.kMlDsa65),
            "PAI",
            expected_subject_key_profile=AttestationCryptoProfile.kMlDsa44,
            expected_signature_profile=AttestationCryptoProfile.kMlDsa65,
        )

    def test_accepts_an_ecdsa_dac_signed_by_a_pqc_pai(self):
        self.test_case._assert_certificate_profiles(
            _algorithms(AttestationCryptoProfile.kEcdsaMatterLegacy, AttestationCryptoProfile.kMlDsa44),
            "DAC",
            expected_subject_key_profile=AttestationCryptoProfile.kEcdsaMatterLegacy,
            expected_signature_profile=AttestationCryptoProfile.kMlDsa44,
        )

    def test_rejects_a_subject_key_that_does_not_match_the_selected_profile(self):
        with self.assertRaises(signals.TestFailure):
            self.test_case._assert_certificate_profiles(
                _algorithms(AttestationCryptoProfile.kMlDsa65, AttestationCryptoProfile.kMlDsa44),
                "PAI",
                expected_subject_key_profile=AttestationCryptoProfile.kMlDsa44,
                expected_signature_profile=AttestationCryptoProfile.kMlDsa44,
            )

    def test_rejects_a_signature_that_does_not_match_the_issuer_profile(self):
        with self.assertRaises(signals.TestFailure):
            self.test_case._assert_certificate_profiles(
                _algorithms(AttestationCryptoProfile.kEcdsaMatterLegacy,
                            AttestationCryptoProfile.kEcdsaMatterLegacy),
                "DAC",
                expected_subject_key_profile=AttestationCryptoProfile.kEcdsaMatterLegacy,
                expected_signature_profile=AttestationCryptoProfile.kMlDsa65,
            )


@unittest.skipUnless(is_ml_dsa_supported(), "cryptography ML-DSA support is required")
class TestTCOPCREDS39DACSelection(unittest.IsolatedAsyncioTestCase):
    async def _retrieve(self, dac_profiles, paa_profile=AttestationCryptoProfile.kMlDsa65,
                        pai_profile=AttestationCryptoProfile.kMlDsa65, pai_name='kMlDsa65PaiPem',
                        dac_name='kMlDsa65PaiDacPem', extra_pai_profiles=0):
        """Run every step against a DUT holding only the (paa_profile, pai_profile) chain."""
        fixtures = _load_pem_fixtures()
        opcreds = Clusters.OperationalCredentials
        cert_type = opcreds.Enums.CertificateChainTypeEnum
        requests = self.requests = []
        steps = []

        async def read_attribute(*, cluster, attribute, endpoint):
            self.assertEqual((cluster, endpoint), (opcreds, 0))
            if attribute == opcreds.Attributes.FeatureMap:
                return OperationalCredentialsFeature.kPQCDeviceAttestation
            self.assertEqual(attribute, opcreds.Attributes.PQCDeviceAttestationProfile)
            return opcreds.Structs.PQCDeviceAttestationProfileStruct(
                PAASupportedProfiles=profile_mask(paa_profile),
                PAISupportedProfiles=profile_mask(pai_profile) | extra_pai_profiles,
                DACSupportedProfiles=dac_profiles)

        async def send_command(*, cmd, endpoint):
            self.assertEqual(endpoint, 0)
            encoded = cmd.ToTLV()
            self.assertEqual(type(cmd).FromTLV(encoded).cryptoProfile, cmd.cryptoProfile)
            requests.append(cmd)
            is_pai = cmd.certificateType == cert_type.kPAICertificate
            held = opcreds.Structs.CryptoProfileChainStruct(PAACrypto=paa_profile, PAICrypto=pai_profile)
            if steps[-1] == 4 and cmd.cryptoProfile != held:
                raise InteractionModelError(Status.InvalidCommand)
            self.assertEqual(cmd.cryptoProfile, held)
            if steps[-1] in (8, 9, 10):
                # These steps require rejection of an out-of-range segment, size, or certificate type.
                if steps[-1] == 8:
                    self.assertGreaterEqual(cmd.segmentID * kCertificateSegmentSize, len(fixtures[pai_name]))
                elif steps[-1] == 9:
                    self.assertEqual(cmd.maxSegmentSize, kOversizedMaxSegmentSize)
                else:
                    self.assertEqual(cmd.certificateType, kInvalidCertificateType)
                raise InteractionModelError(Status.InvalidCommand)
            self.assertEqual(steps[-1], (4 if cmd.segmentID == 0 else 5) if is_pai
                             else (6 if cmd.segmentID == 0 else 7))
            self.assertEqual(cmd.maxSegmentSize, kCertificateSegmentSize)
            document = fixtures[pai_name if is_pai else dac_name]
            offset = cmd.segmentID * kCertificateSegmentSize
            end = offset + kCertificateSegmentSize
            return opcreds.Commands.CertificateChainResponse(
                certificate=document[offset:end], totalDocumentSize=len(document),
                nextSegmentID=cmd.segmentID + 1 if end < len(document) else None)

        test_case = SimpleNamespace(step=steps.append, read_single_attribute_check_success=read_attribute,
                                    send_single_cmd=send_command)
        for name in ('_send_certificate_chain_request', '_assert_certificate_profiles', '_expect_invalid_command'):
            setattr(test_case, name, MethodType(getattr(TC_OPCREDS_3_9, name), test_case))
        await TC_OPCREDS_3_9.test_TC_OPCREDS_3_9.__wrapped__(test_case)
        self.assertEqual(steps, list(range(11)))
        self.assertEqual({cmd.certificateType for cmd in requests},
                         {cert_type.kPAICertificate, cert_type.kDACCertificate, kInvalidCertificateType})

    async def test_accepts_legacy_only_dac_profile(self):
        await self._retrieve(profile_mask(AttestationCryptoProfile.kEcdsaMatterLegacy))

    async def test_selects_legacy_when_additional_dac_profiles_are_advertised(self):
        for profile in (AttestationCryptoProfile.kMlDsa44, AttestationCryptoProfile.kMlDsa65):
            with self.subTest(profile=profile):
                await self._retrieve(profile_mask(AttestationCryptoProfile.kEcdsaMatterLegacy) | profile_mask(profile))

    async def test_falls_back_to_a_held_pairing_and_keeps_it_for_every_request(self):
        # The ML-DSA-65 PAA has no ML-DSA-65 PAI even though the PAI bitmap advertises one.
        await self._retrieve(profile_mask(AttestationCryptoProfile.kEcdsaMatterLegacy),
                             pai_profile=AttestationCryptoProfile.kMlDsa44, pai_name='kMlDsa44PaiUnderMlDsa65PaaPem',
                             dac_name='kMlDsa44PaiUnderMlDsa65PaaDacPem',
                             extra_pai_profiles=profile_mask(AttestationCryptoProfile.kMlDsa65))
        self.assertEqual(self.requests[0].cryptoProfile.PAICrypto, AttestationCryptoProfile.kMlDsa65)
        self.assertTrue(all(cmd.cryptoProfile.PAICrypto == AttestationCryptoProfile.kMlDsa44
                            for cmd in self.requests[1:]))

    async def test_rejects_bitmaps_without_a_valid_pairing(self):
        # An ML-DSA-65 PAI cannot be issued by an ML-DSA-44 PAA, so the bitmaps offer nothing to request.
        with self.assertRaisesRegex(signals.TestFailure, 'no valid PAA/PAI pairing'):
            await self._retrieve(profile_mask(AttestationCryptoProfile.kEcdsaMatterLegacy),
                                 paa_profile=AttestationCryptoProfile.kMlDsa44)
        self.assertEqual(self.requests, [])

    async def test_rejects_dac_profiles_without_legacy(self):
        # This Phase 1 test needs a P-256 DAC; advertising only ML-DSA cannot satisfy it.
        with self.assertRaisesRegex(signals.TestFailure, 'DAC'):
            await self._retrieve(profile_mask(AttestationCryptoProfile.kMlDsa44)
                                 | profile_mask(AttestationCryptoProfile.kMlDsa65))


if __name__ == "__main__":
    unittest.main()
