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

from TC_OPCREDS_3_9 import (  # noqa: E402
    AttestationCryptoProfile,
    TC_OPCREDS_3_9,
    _certificate_algorithms_for_oids,
    _profile_mask,
    kOidEcPublicKey,
    kOidEcdsaWithSha256,
    kOidMlDsa44,
    kOidMlDsa65,
    parse_certificate_algorithms,
)


class TestTCOPCREDS39CertificateProfiles(unittest.TestCase):
    def setUp(self):
        self.test_case = TC_OPCREDS_3_9.__new__(TC_OPCREDS_3_9)

    def test_mixed_dac_algorithms_map_to_independent_profiles(self):
        algorithms = _certificate_algorithms_for_oids(kOidMlDsa65, kOidEcPublicKey)

        self.assertEqual(algorithms.subject_key_profile, AttestationCryptoProfile.kEcdsaMatterLegacy)
        self.assertEqual(algorithms.signature_profile, AttestationCryptoProfile.kMlDsa65)

    def test_profile_bitmap_is_union_of_certificate_algorithms(self):
        mixed_dac_profiles = (
            _profile_mask(AttestationCryptoProfile.kEcdsaMatterLegacy)
            | _profile_mask(AttestationCryptoProfile.kMlDsa65)
        )

        self.test_case._assert_profile_advertised(
            mixed_dac_profiles, AttestationCryptoProfile.kEcdsaMatterLegacy, "DAC", "subjectPublicKeyInfo")
        self.test_case._assert_profile_advertised(
            mixed_dac_profiles, AttestationCryptoProfile.kMlDsa65, "DAC", "signatureAlgorithm")

        with self.assertRaises(signals.TestFailure):
            self.test_case._assert_profile_advertised(
                _profile_mask(AttestationCryptoProfile.kMlDsa65),
                AttestationCryptoProfile.kEcdsaMatterLegacy,
                "DAC",
                "subjectPublicKeyInfo",
            )

    def test_strongest_paa_profile_is_selected(self):
        selected_profile = self.test_case._select_strongest_profile(0x0007, "PAA")

        self.assertEqual(selected_profile, AttestationCryptoProfile.kMlDsa65)

    def test_legacy_certificate_algorithms_parse(self):
        dac_path = _CHIP_ROOT / "credentials/test/attestation/Chip-Test-DAC-FFF1-8000-0000-Cert.der"

        algorithms = parse_certificate_algorithms(dac_path.read_bytes())

        self.assertEqual(algorithms.subject_key_profile, AttestationCryptoProfile.kEcdsaMatterLegacy)
        self.assertEqual(algorithms.signature_profile, AttestationCryptoProfile.kEcdsaMatterLegacy)
        self.assertEqual(algorithms.subject_public_key_algorithm_oid, kOidEcPublicKey)
        self.assertEqual(algorithms.signature_algorithm_oid, kOidEcdsaWithSha256)

    def test_unknown_algorithm_oid_is_rejected(self):
        with self.assertRaises(signals.TestFailure):
            _certificate_algorithms_for_oids("1.2.3.4", kOidEcPublicKey)

        with self.assertRaises(signals.TestFailure):
            _certificate_algorithms_for_oids(kOidMlDsa44, "1.2.3.4")


if __name__ == "__main__":
    unittest.main()
