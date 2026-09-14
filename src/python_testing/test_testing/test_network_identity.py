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

import base64
import os
import sys
import unittest

from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.asymmetric.utils import encode_dss_signature

# The helper under test lives in src/python_testing/support_modules, which is only on
# sys.path when a test script is launched from src/python_testing. This file is run by
# path from the repository root, so add the parent directory explicitly.
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from support_modules import network_identity as ni  # noqa: E402 isort:skip

# Known-answer vector taken from the SDK C++ tests. This is a PDCID01 Network
# Identity: the full X.509 DER (sTestCert_PDCID01_DER) and its 20-byte identifier
# (sTestCert_PDCID01_KeyId) from src/credentials/tests/CHIPCert_test_vectors.cpp.
_PDCID01_DER = bytes([
    0x30, 0x82, 0x01, 0x4a, 0x30, 0x81, 0xf1, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x01, 0x01, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86,
    0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x0c, 0x31, 0x0a, 0x30, 0x08, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x01, 0x2a, 0x30,
    0x20, 0x17, 0x0d, 0x30, 0x30, 0x30, 0x31, 0x30, 0x31, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x5a, 0x18, 0x0f, 0x39, 0x39, 0x39,
    0x39, 0x31, 0x32, 0x33, 0x31, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5a, 0x30, 0x0c, 0x31, 0x0a, 0x30, 0x08, 0x06, 0x03, 0x55,
    0x04, 0x03, 0x0c, 0x01, 0x2a, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a,
    0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x39, 0xcc, 0xd4, 0xac, 0x7e, 0x59, 0x68, 0xbd, 0xf1, 0xb0,
    0x80, 0x11, 0x1d, 0x92, 0x53, 0x64, 0x94, 0xfc, 0x51, 0x62, 0x4c, 0x70, 0xaa, 0x6d, 0x73, 0x08, 0xda, 0xed, 0xf3, 0xa1, 0x5e,
    0x38, 0x69, 0x17, 0x7b, 0x1b, 0xf3, 0xd0, 0x90, 0x47, 0xeb, 0xf0, 0x6b, 0xe8, 0xdd, 0x17, 0xbe, 0x23, 0xf2, 0xfb, 0x3d, 0x63,
    0x90, 0xc6, 0xcf, 0x82, 0x80, 0x2f, 0x62, 0xd0, 0x53, 0x62, 0xc0, 0x08, 0xa3, 0x42, 0x30, 0x40, 0x30, 0x0c, 0x06, 0x03, 0x55,
    0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x02, 0x30, 0x00, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04,
    0x03, 0x02, 0x07, 0x80, 0x30, 0x20, 0x06, 0x03, 0x55, 0x1d, 0x25, 0x01, 0x01, 0xff, 0x04, 0x16, 0x30, 0x14, 0x06, 0x08, 0x2b,
    0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x02, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x01, 0x30, 0x0a, 0x06, 0x08,
    0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 0x20, 0x38, 0x4c, 0x1b, 0xd0, 0x8f, 0xe4,
    0xca, 0xa0, 0x46, 0x07, 0x91, 0x66, 0x0d, 0x82, 0x14, 0x5d, 0xfb, 0xfe, 0x97, 0xd3, 0x14, 0xd1, 0x5e, 0x00, 0x0d, 0x75, 0xc0,
    0x73, 0xaf, 0x5d, 0x7c, 0x7e, 0x02, 0x21, 0x00, 0xcc, 0x1a, 0x01, 0x85, 0x21, 0x26, 0x18, 0x4a, 0xd9, 0xeb, 0xec, 0x80, 0x9b,
    0x4c, 0x78, 0xff, 0xf3, 0x81, 0xfe, 0x32, 0x4b, 0xaf, 0x88, 0x1c, 0xc8, 0x24, 0x9e, 0x16, 0x92, 0x59, 0xbd, 0x5a,
])
_PDCID01_PUBLIC_KEY = bytes([
    0x04, 0x39, 0xcc, 0xd4, 0xac, 0x7e, 0x59, 0x68, 0xbd, 0xf1, 0xb0, 0x80, 0x11, 0x1d, 0x92, 0x53, 0x64,
    0x94, 0xfc, 0x51, 0x62, 0x4c, 0x70, 0xaa, 0x6d, 0x73, 0x08, 0xda, 0xed, 0xf3, 0xa1, 0x5e, 0x38, 0x69,
    0x17, 0x7b, 0x1b, 0xf3, 0xd0, 0x90, 0x47, 0xeb, 0xf0, 0x6b, 0xe8, 0xdd, 0x17, 0xbe, 0x23, 0xf2, 0xfb,
    0x3d, 0x63, 0x90, 0xc6, 0xcf, 0x82, 0x80, 0x2f, 0x62, 0xd0, 0x53, 0x62, 0xc0, 0x08,
])
_PDCID01_KEY_ID = bytes([
    0x3a, 0x0e, 0x71, 0xe2, 0x09, 0x9a, 0x49, 0xda, 0xc9, 0x74, 0xfe, 0xd0, 0x5e, 0xa5, 0x3e, 0xba, 0xce, 0x29, 0x33, 0x90,
])

# The tbs_certificate portion of the DER above (bytes 4..247), i.e. the exact
# input EncodeNetworkIdentityTBSCert produces and signs.
_PDCID01_TBS = _PDCID01_DER[4:4 + 244]

# NASS spec vector from src/app/clusters/network-identity-management-server/tests/NASS_test_vectors.h
# (kNASSTestVector1_*), also used by the C++ DeriveNASSSpecVector test. The raw secret is chosen so
# the first derivation candidate is out of range and the second is accepted.
_NASS_VECTOR_PEM = "FSQBACYCgHXoMDADIAAA+TEST+VECTOR+AoPhgYAAAAAUxWnjRzQJQUERWcGGA=="
_NASS_VECTOR_COMPACT_IDENTITY = bytes([
    0x15, 0x30, 0x09, 0x41, 0x04, 0xbc, 0x41, 0x0b, 0xc2, 0x97, 0x40, 0xfc, 0x44, 0xdc, 0x5b, 0xc5, 0x46, 0xe4, 0x95, 0x30,
    0x97, 0x66, 0xed, 0x00, 0xf7, 0x44, 0xb2, 0x6e, 0x1b, 0x06, 0x48, 0x32, 0x3c, 0x45, 0x10, 0x74, 0xc0, 0xd2, 0xcf, 0x1a,
    0x6e, 0xb0, 0x4e, 0x4c, 0xe8, 0xce, 0xa6, 0x7d, 0x1e, 0x01, 0xe8, 0xbb, 0xb0, 0xf2, 0x95, 0xdc, 0xca, 0x1c, 0x2c, 0x84,
    0x74, 0xb5, 0xe3, 0x3a, 0x40, 0x12, 0x7f, 0x1b, 0x0a, 0x30, 0x0b, 0x40, 0xba, 0x5d, 0x50, 0x0e, 0x92, 0x9f, 0x36, 0x4c,
    0x06, 0xb9, 0xe2, 0x2d, 0xc7, 0xaf, 0xf3, 0x5d, 0x78, 0x05, 0xd2, 0xfe, 0xc5, 0x15, 0xbc, 0x0f, 0xa5, 0xa6, 0x99, 0x50,
    0xb7, 0xbc, 0x55, 0x51, 0x00, 0x5d, 0x66, 0xa2, 0xda, 0xc6, 0x2f, 0x7c, 0x84, 0xe4, 0x56, 0x17, 0x20, 0x29, 0xe7, 0xba,
    0x32, 0xab, 0x94, 0x6e, 0xde, 0x7f, 0x64, 0x5f, 0x60, 0x02, 0x0d, 0x4f, 0x54, 0xef, 0xbb, 0x6b, 0x18,
])
_NASS_VECTOR_KEY_IDENTIFIER = bytes([
    0xef, 0xa8, 0xce, 0x87, 0x39, 0xf9, 0xcf, 0xfd, 0x6d, 0xfb, 0xd2, 0x08, 0xa4, 0x06, 0xf7, 0x7b, 0x9b, 0x5b, 0xa8, 0xce,
])
# Raw secret extracted from the NASS TLV above (context tag 3, a 32-byte octet string).
_NASS_VECTOR_RAW_SECRET = base64.b64decode(_NASS_VECTOR_PEM)[-1 - 32:-1]


class TestNetworkIdentityTbsEncoding(unittest.TestCase):
    """Validates the Python TBS encoder against the SDK C++ known-answer vector."""

    def test_tbs_is_byte_exact(self):
        self.assertEqual(ni._encode_network_identity_tbs(_PDCID01_PUBLIC_KEY), _PDCID01_TBS)

    def test_tbs_length(self):
        # Matches kNetworkIdentityTBSLength in CHIPCert_Internal.h.
        self.assertEqual(len(ni._encode_network_identity_tbs(_PDCID01_PUBLIC_KEY)), 244)

    def test_key_identifier_matches_vector(self):
        self.assertEqual(ni.network_identity_identifier(_PDCID01_PUBLIC_KEY), _PDCID01_KEY_ID)


class TestGeneratedClientIdentity(unittest.TestCase):
    """Validates freshly generated identities round-trip and self-verify."""

    def test_length_and_structure(self):
        _, compact = ni.generate_network_client_identity()
        self.assertEqual(len(compact), ni.COMPACT_IDENTITY_LENGTH)
        self.assertEqual(compact[0], 0x15)
        self.assertEqual(compact[-1], 0x18)

    def test_identifier_derivation_is_consistent(self):
        private_key, compact = ni.generate_network_client_identity()
        public_key = ni.compact_identity_public_key(compact)
        self.assertEqual(public_key, private_key.public_key().public_bytes(
            encoding=serialization.Encoding.X962, format=serialization.PublicFormat.UncompressedPoint))
        self.assertEqual(ni.network_identity_identifier(compact), ni.network_identity_identifier(public_key))

    def test_self_signature_verifies_over_reconstructed_tbs(self):
        # The DUT reconstructs the TBS from the embedded public key and verifies
        # the signature; mimic that to prove the identity would be accepted.
        _, compact = ni.generate_network_client_identity()
        public_key = ni.compact_identity_public_key(compact)
        raw_signature = compact[4 + 65 + 3:4 + 65 + 3 + 64]
        r = int.from_bytes(raw_signature[:32], "big")
        s = int.from_bytes(raw_signature[32:], "big")

        loaded = ec.EllipticCurvePublicKey.from_encoded_point(ec.SECP256R1(), public_key)
        tbs = ni._encode_network_identity_tbs(public_key)
        loaded.verify(encode_dss_signature(r, s), tbs, ec.ECDSA(hashes.SHA256()))

    def test_verification_fails_for_tampered_tbs(self):
        _, compact = ni.generate_network_client_identity()
        public_key = ni.compact_identity_public_key(compact)
        raw_signature = compact[4 + 65 + 3:4 + 65 + 3 + 64]
        r = int.from_bytes(raw_signature[:32], "big")
        s = int.from_bytes(raw_signature[32:], "big")

        loaded = ec.EllipticCurvePublicKey.from_encoded_point(ec.SECP256R1(), public_key)
        tampered = bytearray(ni._encode_network_identity_tbs(public_key))
        tampered[-1] ^= 0xFF
        with self.assertRaises(InvalidSignature):
            loaded.verify(encode_dss_signature(r, s), bytes(tampered), ec.ECDSA(hashes.SHA256()))


def _verify_compact_self_signature(compact: bytes) -> None:
    """Reconstructs the TBS from the embedded public key and verifies the signature,
    mirroring the DUT's ValidateChipNetworkIdentity. Raises InvalidSignature on failure."""
    public_key = ni.compact_identity_public_key(compact)
    raw_signature = compact[4 + 65 + 3:4 + 65 + 3 + 64]
    r = int.from_bytes(raw_signature[:32], "big")
    s = int.from_bytes(raw_signature[32:], "big")
    loaded = ec.EllipticCurvePublicKey.from_encoded_point(ec.SECP256R1(), public_key)
    loaded.verify(encode_dss_signature(r, s), ni._encode_network_identity_tbs(public_key), ec.ECDSA(hashes.SHA256()))


class TestCollidingAndInvalidIdentities(unittest.TestCase):
    """Validates the helpers used to build collision and invalid-identity test inputs."""

    def test_regenerated_identity_collides_but_differs(self):
        private_key, original = ni.generate_network_client_identity()
        colliding = ni.regenerate_network_client_identity(private_key)
        # Same identifier (same public key) but different bytes, and still self-verifies.
        self.assertEqual(ni.network_identity_identifier(colliding), ni.network_identity_identifier(original))
        self.assertNotEqual(colliding, original)
        _verify_compact_self_signature(colliding)

    def test_corrupted_identity_keeps_structure_but_fails_verification(self):
        _, original = ni.generate_network_client_identity()
        corrupted = ni.corrupt_network_client_identity(original)
        self.assertEqual(len(corrupted), len(original))
        self.assertEqual(corrupted[0], 0x15)
        self.assertEqual(corrupted[-1], 0x18)
        # Public key (and thus identifier) is untouched, but the signature no longer verifies.
        self.assertEqual(ni.compact_identity_public_key(corrupted), ni.compact_identity_public_key(original))
        with self.assertRaises(InvalidSignature):
            _verify_compact_self_signature(corrupted)


class TestEcdsaNetworkIdentityDerivation(unittest.TestCase):
    """Validates the NASS -> ECDSA Network Identity derivation against the C++ spec vector."""

    def test_compact_identity_matches_vector(self):
        _, compact = ni.derive_ecdsa_network_identity(_NASS_VECTOR_RAW_SECRET)
        self.assertEqual(compact, _NASS_VECTOR_COMPACT_IDENTITY)

    def test_identifier_matches_vector(self):
        _, compact = ni.derive_ecdsa_network_identity(_NASS_VECTOR_RAW_SECRET)
        self.assertEqual(ni.network_identity_identifier(compact), _NASS_VECTOR_KEY_IDENTIFIER)

    def test_derivation_is_deterministic(self):
        self.assertEqual(ni.derive_ecdsa_network_identity(_NASS_VECTOR_RAW_SECRET)[1],
                         ni.derive_ecdsa_network_identity(_NASS_VECTOR_RAW_SECRET)[1])

    def test_rejects_bad_secret_length(self):
        with self.assertRaises(ValueError):
            ni.derive_ecdsa_network_identity(b"\x00" * 16)


class TestNetworkAdministratorSecret(unittest.TestCase):
    def test_encoding_layout(self):
        raw_secret = bytes(range(ni.NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH))
        encoded = ni.encode_network_administrator_secret(0x01020304, raw_secret)
        expected = (bytes([0x15, 0x24, 0x01, 0x00, 0x26, 0x02, 0x04, 0x03, 0x02, 0x01,
                           0x30, 0x03, 0x20]) + raw_secret + bytes([0x18]))
        self.assertEqual(encoded, expected)

    def test_random_secret_length(self):
        encoded = ni.encode_network_administrator_secret(ni.matter_epoch_now())
        # struct(1) + version(3) + created(6) + raw-secret(3+32) + end(1)
        self.assertEqual(len(encoded), 1 + 3 + 6 + 3 + ni.NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH + 1)

    def test_rejects_bad_secret_length(self):
        with self.assertRaises(ValueError):
            ni.encode_network_administrator_secret(0, b"\x00" * 16)


class TestNetworkAdministratorSecretDecoding(unittest.TestCase):
    def test_round_trips_encoded_secret(self):
        raw_secret = bytes(range(ni.NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH))
        created = 0x01020304
        decoded = ni.decode_network_administrator_secret(
            ni.encode_network_administrator_secret(created, raw_secret))
        self.assertEqual(decoded.version, 0)
        self.assertEqual(decoded.created, created)
        self.assertEqual(decoded.raw_secret, raw_secret)

    def test_accepts_narrow_created_encodings(self):
        # The C++ TLV writer emits the smallest unsigned encoding that fits, so a small
        # timestamp comes back as a 1- or 2-byte element rather than the 4 bytes the
        # Python encoder always writes.
        raw_secret = bytes(range(ni.NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH))
        for control, value_bytes, expected in ((0x24, b"\x07", 7), (0x25, b"\x34\x12", 0x1234)):
            encoded = (bytes([0x15, 0x24, 0x01, 0x00, control, 0x02]) + value_bytes
                       + bytes([0x30, 0x03, 0x20]) + raw_secret + bytes([0x18]))
            self.assertEqual(ni.decode_network_administrator_secret(encoded).created, expected)

    def test_rejects_malformed_secrets(self):
        raw_secret = bytes(ni.NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH)
        valid = ni.encode_network_administrator_secret(0x01020304, raw_secret)
        # Each case names the rejection it must produce. Matching the message keeps two cases
        # from silently landing in the same branch and leaving another one unreached: dropping
        # bytes off the end of a valid NASS, for instance, takes the closing 0x18 with them and
        # is rejected as a bad structure long before any truncation check runs.
        cases = {
            "not a structure": (valid[1:], "anonymous TLV structure"),
            "no closing tag": (valid[:-1], "anonymous TLV structure"),
            # Ends between two fields, with less than a control-and-tag pair left.
            "ends after the version": (bytes([0x15, 0x24, 0x01, 0x00, 0x18]), "truncated before field 2"),
            # Ends right after the octet string's tag, so its length prefix is off the end.
            "ends after the raw secret tag": (bytes([0x15, 0x24, 0x01, 0x00, 0x26, 0x02, 0x04, 0x03, 0x02, 0x01,
                                                     0x30, 0x03, 0x18]),
                                              "truncated before the length of field 3"),
            # Declares 32 bytes of raw secret but carries 28, so the field runs off the end.
            "raw secret shorter than its length prefix": (valid[:-5] + bytes([0x18]), "truncated inside field 3"),
            "trailing field": (valid[:-1] + bytes([0x24, 0x04, 0x00, 0x18]), "unexpected trailing fields"),
            "nonzero version": (bytes([0x15, 0x24, 0x01, 0x01]) + valid[4:], "version must be 0"),
            "short raw secret": (bytes([0x15, 0x24, 0x01, 0x00, 0x26, 0x02, 0x04, 0x03, 0x02, 0x01,
                                        0x30, 0x03, 0x10]) + raw_secret[:16] + bytes([0x18]),
                                 "raw secret must be 32 bytes"),
            "out of tag order": (bytes([0x15, 0x26, 0x02, 0x04, 0x03, 0x02, 0x01, 0x24, 0x01, 0x00,
                                        0x30, 0x03, 0x20]) + raw_secret + bytes([0x18]),
                                 "out of tag order"),
            # Each field's TLV type is enforced, so callers can rely on `created` being an
            # integer they can do arithmetic on rather than getting bytes back.
            "version as an octet string": (bytes([0x15, 0x30, 0x01, 0x01, 0x00, 0x26, 0x02, 0x04, 0x03, 0x02, 0x01,
                                                  0x30, 0x03, 0x20]) + raw_secret + bytes([0x18]),
                                           "field 1 must be an unsigned integer"),
            "created as an octet string": (bytes([0x15, 0x24, 0x01, 0x00, 0x30, 0x02, 0x04, 0x04, 0x03, 0x02, 0x01,
                                                  0x30, 0x03, 0x20]) + raw_secret + bytes([0x18]),
                                           "field 2 must be an unsigned integer"),
            "raw secret as an integer": (bytes([0x15, 0x24, 0x01, 0x00, 0x26, 0x02, 0x04, 0x03, 0x02, 0x01,
                                                0x26, 0x03, 0x04, 0x03, 0x02, 0x01, 0x18]),
                                         "field 3 must be an octet string"),
        }
        for name, (encoded, expected_message) in cases.items():
            with self.subTest(name=name):
                with self.assertRaises(ValueError) as caught:
                    ni.decode_network_administrator_secret(encoded)
                self.assertIn(expected_message, str(caught.exception))


if __name__ == "__main__":
    unittest.main()
