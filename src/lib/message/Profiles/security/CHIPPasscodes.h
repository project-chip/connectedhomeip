/*
 *
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      This file defines types, classes and interfaces associated with
 *      CHIP passcodes.
 *
 */

#ifndef CHIPPASSCODES_H_
#define CHIPPASSCODES_H_

#include <core/CHIPCore.h>
#include <support/DLLUtil.h>
#include <support/crypto/AESBlockCipher.h>

/**
 *   @namespace chip::Profiles::Security::Passcodes
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for the CHIP
 *     passcodes library within the CHIP security profile.
 */

namespace chip {
namespace Profiles {
namespace Security {
namespace Passcodes {

using chip::Profiles::Security::AppKeys::GroupKeyStoreBase;

/**
 *  @brief
 *    CHIP passcode encryption configuration types.
 */
enum
{
    kPasscode_Config1_TEST_ONLY = 0x01, /**< Passcode encryption configuration #1 is not using encryption and secret
                                             keys to authenticate and fingerprint passcode. This configuration SHOULD
                                             be used for TEST purposes only. */
    kPasscode_Config2 = 0x02,           /**< Passcode encryption configuration #2 is using AES-128-ECB encryption
                                             with 8 byte HMAC-SHA-1 integrity and 8 byte HMAC-SHA-1 fingerprint. */

    kPasscodeConfig1_KeyId = ChipKeyId::kNone, /**< Dummy key id used for test passcode configuration #1. */
};

/**
 *  @brief
 *    Key diversifier used for CHIP passcode encryption key derivation. This value
 *    represents first 4 bytes of the SHA-1 HASH of "Nest Passcode EK and AK" phrase.
 */
extern const uint8_t kPasscodeEncKeyDiversifier[4];

/**
 *  @brief
 *    Key diversifier used for CHIP passcode encryption key derivation. This value
 *    represents first 4 bytes of the SHA-1 HASH of "Nest Passcode EK and AK" phrase.
 */
extern const uint8_t kPasscodeFingerprintKeyDiversifier[4];

/**
 *  @brief
 *    CHIP passcode encryption protocol parameter sizes.
 */
enum
{
    /** Passcode encryption (AES-128) key length. */
    kPasscodeEncryptionKeyLen = chip::Platform::Security::AES128BlockCipher::kKeyLength,
    /** Passcode authentication (SHA-1) key length. */
    kPasscodeAuthenticationKeyLen = chip::Platform::Security::SHA1::kHashLength,
    /** Passcode fingerprint (SHA-1) key length. */
    kPasscodeFingerprintKeyLen = chip::Platform::Security::SHA1::kHashLength,
    /** Passcode total (encryption & integrity) key length. */
    kPasscodeTotalDerivedKeyLen = kPasscodeEncryptionKeyLen + kPasscodeAuthenticationKeyLen,
    /** Passcode length (padded to the AES-128 block length). */
    kPasscodePaddedLen = chip::Platform::Security::AES128BlockCipher::kBlockLength,
    /** Passcode authenticator length. */
    kPasscodeAuthenticatorLen = 8,
    /** Passcode fingerprint length. */
    kPasscodeFingerprintLen = 8,
    /** Max encrypted passcode length. */
    kPasscodeMaxEncryptedLen =
        sizeof(uint8_t) + 2 * sizeof(uint32_t) + kPasscodePaddedLen + kPasscodeAuthenticatorLen + kPasscodeFingerprintLen,
    /** Max unencrytped passcode length */
    kPasscodeMaxLen = kPasscodePaddedLen,
    /** Passcode encryption application key diversifier size. */
    kPasscodeEncKeyDiversifierSize = sizeof(kPasscodeEncKeyDiversifier) + sizeof(uint8_t),
    /** Passcode fingerprint key diversifier size. */
    kPasscodeFingerprintKeyDiversifierSize = sizeof(kPasscodeFingerprintKeyDiversifier),
};

// Encrypt CHIP passcode.
extern CHIP_ERROR EncryptPasscode(uint8_t config, uint32_t keyId, uint32_t nonce, const uint8_t * passcode, size_t passcodeLen,
                                  uint8_t * encPasscode, size_t encPasscodeBufSize, size_t & encPasscodeLen,
                                  GroupKeyStoreBase * groupKeyStore);
extern CHIP_ERROR EncryptPasscode(uint8_t config, uint32_t keyId, uint32_t nonce, const uint8_t * passcode, size_t passcodeLen,
                                  const uint8_t * encKey, const uint8_t * authKey, const uint8_t * fingerprintKey,
                                  uint8_t * encPasscode, size_t encPasscodeBufSize, size_t & encPasscodeLen);

// Decrypt CHIP passcode.
extern CHIP_ERROR DecryptPasscode(const uint8_t * encPasscode, size_t encPasscodeLen, uint8_t * passcodeBuf, size_t passcodeBufSize,
                                  size_t & passcodeLen, GroupKeyStoreBase * groupKeyStore);
extern CHIP_ERROR DecryptPasscode(const uint8_t * encPasscode, size_t encPasscodeLen, const uint8_t * encKey,
                                  const uint8_t * authKey, const uint8_t * fingerprintKey, uint8_t * passcodeBuf,
                                  size_t passcodeBufSize, size_t & passcodeLen);

// Utility functions for interacting with encrypted passcodes.
extern CHIP_ERROR GetEncryptedPasscodeConfig(const uint8_t * encPasscode, size_t encPasscodeLen, uint8_t & config);
extern CHIP_ERROR GetEncryptedPasscodeKeyId(const uint8_t * encPasscode, size_t encPasscodeLen, uint32_t & keyId);
extern CHIP_ERROR GetEncryptedPasscodeNonce(const uint8_t * encPasscode, size_t encPasscodeLen, uint32_t & nonce);
extern CHIP_ERROR GetEncryptedPasscodeFingerprint(const uint8_t * encPasscode, size_t encPasscodeLen, uint8_t * fingerprintBuf,
                                                  size_t fingerprintBufSize, size_t & fingerprintLen);
extern bool IsSupportedPasscodeEncryptionConfig(uint8_t config);

} // namespace Passcodes
} // namespace Security
} // namespace Profiles
} // namespace chip

#endif /* CHIPPASSCODES_H_ */
