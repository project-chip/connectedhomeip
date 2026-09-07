/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      Low-level primitives for a Matter operational (NOC) key held entirely inside
 *      ESP-TEE secure storage. The private key never leaves the secure world: it is
 *      generated in the TEE, and CSR creation and CASE signing are performed there.
 *
 *      These are the building blocks for a TEE-backed OperationalKeystore. Each key is
 *      addressed by a secure-storage id string.
 */
#pragma once

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/// Generate a fresh SECP256R1 keypair inside TEE secure storage under @p keyId.
/// Overwrites any existing key with the same id.
CHIP_ERROR ESP32TEEOpKeyGenerate(const char * keyId);

/// Read the public key of the TEE-stored key @p keyId (uncompressed, 65 bytes).
CHIP_ERROR ESP32TEEOpKeyGetPublicKey(const char * keyId, Crypto::P256PublicKey & outPublicKey);

/// Build a PKCS#10 CSR for the TEE-stored key @p keyId. The CSR is signed inside the TEE.
/// @p outCsr is resized to the actual CSR length on success.
CHIP_ERROR ESP32TEEOpKeyNewCSR(const char * keyId, MutableByteSpan & outCsr);

/// Sign @p message with the TEE-stored key @p keyId (SHA256 + ECDSA, inside the TEE).
CHIP_ERROR ESP32TEEOpKeySign(const char * keyId, ByteSpan message, Crypto::P256ECDSASignature & outSignature);

/// Delete the TEE-stored key @p keyId from secure storage. Not-found is treated as success.
CHIP_ERROR ESP32TEEOpKeyRemove(const char * keyId);

/// Return whether a key with id @p keyId currently exists in TEE secure storage.
CHIP_ERROR ESP32TEEOpKeyExists(const char * keyId, bool & outExists);

/// Self-check: generate a throwaway key, derive its public key, build+verify a CSR,
/// sign+verify a message, then delete the key. Returns CHIP_NO_ERROR only if every
/// step and cryptographic verification passes. Safe to call at boot for bring-up.
CHIP_ERROR ESP32TEEOpKeySelfTest();

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
