#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.hkdf import HKDF

from . import p256keypair


def generate_compressed_fabric_id(root_public_key: bytes, fabric_id: int) -> int:
    """Generates compressed fabric id from Root CA's public key and fabric id.

    Returns:
      Compressed fabric id as a int
    """
    if len(root_public_key) != p256keypair.P256_PUBLIC_KEY_LENGTH and root_public_key[0] != b'\x04':
        raise ValueError("Root public key must be an uncompressed P256 point.")

    return int.from_bytes(HKDF(
        algorithm=hashes.SHA256(),
        length=8,
        salt=fabric_id.to_bytes(length=8, byteorder="big", signed=False),
        info=b"CompressedFabric",
    ).derive(key_material=root_public_key[1:]), byteorder="big")
