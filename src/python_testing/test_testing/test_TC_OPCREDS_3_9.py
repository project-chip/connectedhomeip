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

from mobly import signals

_CHIP_ROOT = Path(__file__).resolve().parents[3]
sys.path.append(str(_CHIP_ROOT / "src/python_testing"))

from TC_OPCREDS_3_9 import TC_OPCREDS_3_9, AttestationCryptoProfile, CertificateAlgorithms  # noqa: E402


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


if __name__ == "__main__":
    unittest.main()
