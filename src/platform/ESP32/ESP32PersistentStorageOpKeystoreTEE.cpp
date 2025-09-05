/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <crypto/CHIPCryptoPAL.h>
#include <cstdio>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h> // if you want to reuse allocator style
#include <lib/support/logging/CHIPLogging.h>
#include <string>

#include "ESP32PersistentStorageOpKeystoreTEE.h"
#include "nvs.h"

// Maximum length of TEE secure storage key ID (including null terminator)
#define KEY_ID_MAX_SZ (NVS_KEY_NAME_MAX_SIZE)

using namespace chip;
using namespace chip::Crypto;

static const char * TAG = "PersistentStorageOpKeystoreTEE";

// Helper
std::string PersistentStorageOpKeystoreTEE::GetKeyId(FabricIndex fabricIndex) const
{
    char buf[KEY_ID_MAX_SZ];
    int written = snprintf(buf, sizeof(buf), "fabric_%u_key", fabricIndex);
    if (written < 0 || written >= static_cast<int>(sizeof(buf)))
    {
        buf[sizeof(buf) - 1] = '\0';
    }
    return std::string(buf);
}

CHIP_ERROR PersistentStorageOpKeystoreTEE::Init()
{
    VerifyOrReturnError(!mInitialized, CHIP_ERROR_INCORRECT_STATE);

    ResetPendingKey();

    mInitialized = true;
    return CHIP_NO_ERROR;
}

void PersistentStorageOpKeystoreTEE::Finish()
{
    if (!mInitialized)
    {
        return;
    }

    ResetPendingKey();

    mInitialized = false;
}

void PersistentStorageOpKeystoreTEE::ResetPendingKey()
{
    if (mPendingKeypair != nullptr)
    {
        Platform::Delete(mPendingKeypair);
        mPendingKeypair = nullptr;
    }
    mIsPendingKeypairActive = false;
    mPendingFabricIndex     = kUndefinedFabricIndex;
}

// API implementations
bool PersistentStorageOpKeystoreTEE::HasOpKeypairForFabric(FabricIndex fabricIndex) const
{
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), false);

    // Try to fetch the public-key from the TEE secure storage for the given fabric. If it succeeds, the key exists.
    std::string keyId                         = GetKeyId(fabricIndex);
    esp_tee_sec_storage_key_cfg_t cfg         = { .id = keyId.c_str(), .type = ESP_SEC_STG_KEY_ECDSA_SECP256R1 };
    esp_tee_sec_storage_ecdsa_pubkey_t pubkey = {};
    esp_err_t err                             = esp_tee_sec_storage_ecdsa_get_pubkey(&cfg, &pubkey);

    return (err == ESP_OK);
}

CHIP_ERROR PersistentStorageOpKeystoreTEE::NewOpKeypairForFabric(FabricIndex fabricIndex, MutableByteSpan & outCSR)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(outCSR.size() >= Crypto::kMIN_CSR_Buffer_Size, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Ensure we don't have a pending key for a different fabric
    if ((mPendingFabricIndex != kUndefinedFabricIndex) && (fabricIndex != mPendingFabricIndex))
    {
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }

    // Clear any previously allocated pending wrapper
    ResetPendingKey();

    std::string keyId                 = GetKeyId(fabricIndex);
    esp_tee_sec_storage_key_cfg_t cfg = { .id = keyId.c_str(), .type = ESP_SEC_STG_KEY_ECDSA_SECP256R1 };
    esp_err_t err                     = esp_tee_sec_storage_gen_key(&cfg);
    VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_INTERNAL);

    mPendingKeypair = Platform::New<Crypto::ESP32P256Keypair>();
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR chipErr = mPendingKeypair->InitializeFromTEE(Crypto::ECPKeyTarget::ECDSA, GetKeyId(fabricIndex).c_str());
    if (chipErr != CHIP_NO_ERROR)
    {
        esp_tee_sec_storage_clear_key(GetKeyId(fabricIndex).c_str());
        ResetPendingKey();
        return chipErr;
    }

    size_t csrLen = outCSR.size();
    chipErr       = mPendingKeypair->NewCertificateSigningRequest(outCSR.data(), csrLen);
    if (chipErr != CHIP_NO_ERROR)
    {
        esp_tee_sec_storage_clear_key(GetKeyId(fabricIndex).c_str());
        ResetPendingKey();
        return chipErr;
    }
    outCSR.reduce_size(csrLen);

    mIsPendingKeypairActive = false;
    mPendingFabricIndex     = fabricIndex;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOpKeystoreTEE::ActivateOpKeypairForFabric(FabricIndex fabricIndex,
                                                                      const Crypto::P256PublicKey & nocPublicKey)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mPendingFabricIndex == fabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Validate public key being activated matches last generated pending keypair
    VerifyOrReturnError(mPendingKeypair->Pubkey().Matches(nocPublicKey), CHIP_ERROR_INVALID_PUBLIC_KEY);
    mIsPendingKeypairActive = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOpKeystoreTEE::CommitOpKeypairForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mPendingFabricIndex == fabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mIsPendingKeypairActive, CHIP_ERROR_INCORRECT_STATE);

    // If we got here, we succeeded and can reset the pending key: next `SignWithOpKeypair` will use the stored key.
    ResetPendingKey();
    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOpKeystoreTEE::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // If this is the pending key, revert it
    if (mPendingFabricIndex != kUndefinedFabricIndex && fabricIndex == mPendingFabricIndex)
    {
        RevertPendingKeypair();
        return CHIP_NO_ERROR;
    }

    std::string keyId = GetKeyId(fabricIndex);
    esp_err_t err     = esp_tee_sec_storage_clear_key(keyId.c_str());
    if (err == ESP_OK)
    {
        return CHIP_NO_ERROR;
    }
    else if (err == ESP_ERR_NOT_FOUND)
    {
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }

    return CHIP_ERROR_INTERNAL;
}

void PersistentStorageOpKeystoreTEE::RevertPendingKeypair()
{
    VerifyOrReturn(mInitialized);

    if (mPendingFabricIndex == kUndefinedFabricIndex)
    {
        return;
    }

    std::string keyId = GetKeyId(mPendingFabricIndex);
    esp_err_t err     = esp_tee_sec_storage_clear_key(keyId.c_str());
    if (err != ESP_OK && err != ESP_ERR_NOT_FOUND)
    {
        ChipLogError(Crypto, "Failed to revert pending keypair for fabric %u (err=%d)", mPendingFabricIndex, err);
    }

    // Clean local wrapper and pending state
    ResetPendingKey();
}

CHIP_ERROR PersistentStorageOpKeystoreTEE::SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                                             Crypto::P256ECDSASignature & outSignature) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    if (mPendingFabricIndex == fabricIndex && mIsPendingKeypairActive && mPendingKeypair != nullptr)
    {
        return mPendingKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
    }

    Crypto::ESP32P256Keypair mKeypair;
    ReturnErrorOnFailure(mKeypair.InitializeFromTEE(Crypto::ECPKeyTarget::ECDSA, GetKeyId(fabricIndex).c_str()));

    return mKeypair.ECDSA_sign_msg(message.data(), message.size(), outSignature);
}
