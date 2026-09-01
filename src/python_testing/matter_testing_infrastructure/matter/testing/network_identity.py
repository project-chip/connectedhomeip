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

"""Generation of Per-Device Credentials (PDC) material for the Network Identity
Management cluster (0x0450).

Two artifacts are produced here:

* A **Network (Client) Identity** in the ``compact-pdc-identity`` TLV format
  accepted by the ``AddClient`` command and validated by the DUT via
  ``Credentials::ValidateChipNetworkIdentity`` (which recomputes the fixed
  X.509 TBSCertificate template from the embedded public key and verifies the
  self-signature). The Python encoder mirrors
  ``Credentials::EncodeNetworkIdentityTBSCert`` in
  ``src/credentials/GenerateChipX509Cert.cpp`` byte-for-byte so that the
  signature we compute verifies against the template the DUT reconstructs.

* A **Network Administrator Shared Secret (NASS)** in the TLV format accepted by
  the ``ImportAdminSecret`` command (see
  ``src/app/clusters/network-identity-management-server/NetworkAdministratorSecret.cpp``).

The encoders are exercised against the C++ known-answer vectors in
``test_network_identity.py``.
"""

from __future__ import annotations

import hashlib
import os
import time

from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.asymmetric.utils import decode_dss_signature

# Length of the raw key material in a Network Administrator Shared Secret.
NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH = 32
# Length of the 20-byte identifier (truncated SHA-256 of the public key) reported
# in the ActiveNetworkIdentities/Clients attributes.
NETWORK_IDENTITY_IDENTIFIER_LENGTH = 20
# Length of a compact-pdc-identity: anonymous TLV struct { pubkey[65], signature[64] }.
COMPACT_IDENTITY_LENGTH = 137
# Unix time of the Matter/CHIP epoch (2000-01-01T00:00:00 UTC).
MATTER_EPOCH_OFFSET_SECONDS = 946684800

# TLV tags of the compact-pdc-identity structure (Matter cert TLV tags).
_TLV_TAG_EC_PUBLIC_KEY = 0x09
_TLV_TAG_ECDSA_SIGNATURE = 0x0B

# ---------------------------------------------------------------------------
# Minimal DER encoding helpers for the Network Identity TBSCertificate.
# ---------------------------------------------------------------------------


def _der_len(length: int) -> bytes:
    if length < 0x80:
        return bytes([length])
    encoded = b""
    while length:
        encoded = bytes([length & 0xFF]) + encoded
        length >>= 8
    return bytes([0x80 | len(encoded)]) + encoded


def _der(tag: int, content: bytes) -> bytes:
    return bytes([tag]) + _der_len(len(content)) + content


# Pre-encoded DER OBJECT IDENTIFIERs.
_OID_ECDSA_WITH_SHA256 = _der(0x06, bytes([0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02]))
_OID_EC_PUBLIC_KEY = _der(0x06, bytes([0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01]))
_OID_PRIME256V1 = _der(0x06, bytes([0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07]))
_OID_COMMON_NAME = _der(0x06, bytes([0x55, 0x04, 0x03]))
_OID_BASIC_CONSTRAINTS = _der(0x06, bytes([0x55, 0x1D, 0x13]))
_OID_KEY_USAGE = _der(0x06, bytes([0x55, 0x1D, 0x0F]))
_OID_EXT_KEY_USAGE = _der(0x06, bytes([0x55, 0x1D, 0x25]))
_OID_KEY_PURPOSE_CLIENT_AUTH = _der(0x06, bytes([0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x02]))
_OID_KEY_PURPOSE_SERVER_AUTH = _der(0x06, bytes([0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x01]))

# DER BOOLEAN TRUE.
_DER_TRUE = _der(0x01, b"\xff")


def _network_identity_name() -> bytes:
    # RDNSequence with a single CN="*" attribute, encoded as a UTF8String (as
    # done by InitNetworkIdentitySubject; "*" is not a valid PrintableString).
    common_name = _der(0x0C, b"*")
    attribute = _der(0x30, _OID_COMMON_NAME + common_name)
    return _der(0x30, _der(0x31, attribute))  # SEQUENCE { SET { ... } }


def _subject_public_key_info(public_key: bytes) -> bytes:
    algorithm = _der(0x30, _OID_EC_PUBLIC_KEY + _OID_PRIME256V1)
    subject_public_key = _der(0x03, b"\x00" + public_key)  # BIT STRING, 0 unused bits
    return _der(0x30, algorithm + subject_public_key)


def _network_identity_extensions() -> bytes:
    # basicConstraints (critical): cA absent -> empty SEQUENCE.
    basic_constraints = _der(0x30, _OID_BASIC_CONSTRAINTS + _DER_TRUE + _der(0x04, _der(0x30, b"")))
    # keyUsage (critical): digitalSignature only -> BIT STRING with 7 unused bits, 0x80.
    key_usage = _der(0x30, _OID_KEY_USAGE + _DER_TRUE + _der(0x04, _der(0x03, b"\x07\x80")))
    # extKeyUsage (critical): id-kp-clientAuth, id-kp-serverAuth.
    ext_key_usage = _der(
        0x30, _OID_EXT_KEY_USAGE + _DER_TRUE + _der(0x04, _der(0x30, _OID_KEY_PURPOSE_CLIENT_AUTH + _OID_KEY_PURPOSE_SERVER_AUTH)))
    return _der(0xA3, _der(0x30, basic_constraints + key_usage + ext_key_usage))  # [3] EXPLICIT


def _encode_network_identity_tbs(public_key: bytes) -> bytes:
    """Encodes the X.509 TBSCertificate for a Network Identity with the given
    65-byte uncompressed P-256 public key. Mirrors EncodeNetworkIdentityTBSCert."""
    version = _der(0xA0, _der(0x02, b"\x02"))  # [0] EXPLICIT Version v3
    serial_number = _der(0x02, b"\x01")
    signature_algorithm = _der(0x30, _OID_ECDSA_WITH_SHA256)
    issuer = subject = _network_identity_name()
    # notBefore = CHIP epoch 1 (UTCTime); notAfter = 99991231235959Z (GeneralizedTime).
    validity = _der(0x30, _der(0x17, b"000101000001Z") + _der(0x18, b"99991231235959Z"))
    return _der(
        0x30,
        version + serial_number + signature_algorithm + issuer + validity + subject
        + _subject_public_key_info(public_key) + _network_identity_extensions())


# ---------------------------------------------------------------------------
# Compact TLV encoding of the identity.
# ---------------------------------------------------------------------------


def _encode_compact_identity(public_key: bytes, signature: bytes) -> bytes:
    if len(public_key) != 65:
        raise ValueError(f"public key must be 65 bytes, got {len(public_key)}")
    if len(signature) != 64:
        raise ValueError(f"signature must be 64 bytes, got {len(signature)}")
    # Anonymous TLV structure { [9] pubkey (octstr), [11] signature (octstr) }.
    return (bytes([0x15])
            + bytes([0x30, _TLV_TAG_EC_PUBLIC_KEY, len(public_key)]) + public_key
            + bytes([0x30, _TLV_TAG_ECDSA_SIGNATURE, len(signature)]) + signature
            + bytes([0x18]))


def _raw_public_key(private_key: ec.EllipticCurvePrivateKey) -> bytes:
    return private_key.public_key().public_bytes(
        encoding=serialization.Encoding.X962,
        format=serialization.PublicFormat.UncompressedPoint)


def generate_network_client_identity() -> tuple[ec.EllipticCurvePrivateKey, bytes]:
    """Generates a fresh Network (Client) Identity.

    Returns a tuple of (private_key, compact_identity) where compact_identity is
    the 137-byte compact-pdc-identity accepted by the AddClient command.
    """
    private_key = ec.generate_private_key(ec.SECP256R1())
    public_key = _raw_public_key(private_key)

    tbs = _encode_network_identity_tbs(public_key)
    der_signature = private_key.sign(tbs, ec.ECDSA(hashes.SHA256()))
    r, s = decode_dss_signature(der_signature)
    raw_signature = r.to_bytes(32, "big") + s.to_bytes(32, "big")

    return private_key, _encode_compact_identity(public_key, raw_signature)


def compact_identity_public_key(compact_identity: bytes) -> bytes:
    """Extracts the 65-byte uncompressed public key from a compact identity."""
    if len(compact_identity) < 4 + 65:
        raise ValueError("compact identity is too short")
    if compact_identity[1:3] != bytes([0x30, _TLV_TAG_EC_PUBLIC_KEY]) or compact_identity[3] != 65:
        raise ValueError("unexpected compact identity encoding")
    return compact_identity[4:4 + 65]


def network_identity_identifier(public_key_or_compact: bytes) -> bytes:
    """Computes the 20-byte identifier for a Network (Client) Identity.

    Accepts either a 65-byte uncompressed public key or a full compact identity.
    The identifier is the first 20 bytes of SHA-256 over the public key, matching
    ``CalculateKeyIdentifierSha256`` in the SDK.
    """
    if len(public_key_or_compact) == 65:
        public_key = public_key_or_compact
    else:
        public_key = compact_identity_public_key(public_key_or_compact)
    return hashlib.sha256(public_key).digest()[:NETWORK_IDENTITY_IDENTIFIER_LENGTH]


# ---------------------------------------------------------------------------
# Network Administrator Shared Secret (NASS) encoding.
# ---------------------------------------------------------------------------


def matter_epoch_now() -> int:
    """Current time expressed in Matter epoch seconds (seconds since 2000-01-01)."""
    return int(time.time()) - MATTER_EPOCH_OFFSET_SECONDS


def encode_network_administrator_secret(created: int, raw_secret: bytes | None = None) -> bytes:
    """Encodes a NASS in the TLV format accepted by ImportAdminSecret.

    Args:
        created: Creation timestamp in Matter epoch seconds. The DUT reports this
            as the ActiveNetworkIdentities CreatedTimestamp and enforces that it
            increases monotonically across imports.
        raw_secret: 32 bytes of raw key material. A random value is generated when
            omitted.
    """
    if raw_secret is None:
        raw_secret = os.urandom(NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH)
    if len(raw_secret) != NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH:
        raise ValueError(f"raw secret must be {NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH} bytes, got {len(raw_secret)}")
    if not 0 <= created <= 0xFFFFFFFF:
        raise ValueError("created must fit in a uint32")

    # Anonymous TLV struct { [1] version=0 (u8), [2] created (u32), [3] raw-secret (octstr[32]) }.
    return (bytes([0x15])
            + bytes([0x24, 0x01, 0x00])
            + bytes([0x26, 0x02]) + created.to_bytes(4, "little")
            + bytes([0x30, 0x03, NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH]) + raw_secret
            + bytes([0x18]))
