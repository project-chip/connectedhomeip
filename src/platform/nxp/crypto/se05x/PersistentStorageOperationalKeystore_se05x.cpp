/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "PersistentStorageOperationalKeystore_se05x.h"
#include "CHIPCryptoPALHsm_se05x_utils.h"
#include <lib/support/DefaultStorageKeyAllocator.h>

namespace chip {

using namespace chip::Crypto;

#define CHIP_SE05x_NODE_OP_KEY_INDEX 0x7E000000
#define CHIP_SE05x_NODE_OP_REF_KEY_TEMPLATE                                                                                        \
    {                                                                                                                              \
        0xA5, 0xA6, 0xB5, 0xB6, 0xA5, 0xA6, 0xB5, 0xB6, 0x7E, 0x00, 0x00, 0x00                                                     \
    }
#define CHIP_SE05x_NODE_OP_KEY_ID_INDEX 11

CHIP_ERROR PersistentStorageOpKeystorese05x::NewOpKeypairForFabric(FabricIndex fabricIndex,
                                                                   MutableByteSpan & outCertificateSigningRequest)
{
    P256SerializedKeypair serializedKeypair;
    uint8_t privatekey[32] = CHIP_SE05x_NODE_OP_REF_KEY_TEMPLATE;
    uint8_t publickey[65]  = {
        0x00,
    };
    size_t privatekey_len = sizeof(privatekey);
    size_t pubkey_len     = sizeof(publickey);
    uint32_t hsmKeyId     = 0;

    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // If a key is pending, we cannot generate for a different fabric index until we commit or revert.
    if ((mPendingFabricIndex != kUndefinedFabricIndex) && (fabricIndex != mPendingFabricIndex))
    {
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }

    VerifyOrReturnError(outCertificateSigningRequest.size() >= Crypto::kMIN_CSR_Buffer_Size, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Replace previous pending key pair, if any was previously allocated
    ResetPendingKey();

    mPendingKeypair = Platform::New<Crypto::P256Keypair>();
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_NO_MEMORY);

    hsmKeyId                                        = CHIP_SE05x_NODE_OP_KEY_INDEX + fabricIndex;
    privatekey[CHIP_SE05x_NODE_OP_KEY_ID_INDEX - 3] = (hsmKeyId >> 24) & 0xFF;
    privatekey[CHIP_SE05x_NODE_OP_KEY_ID_INDEX - 2] = (hsmKeyId >> 16) & 0xFF;
    privatekey[CHIP_SE05x_NODE_OP_KEY_ID_INDEX - 1] = (hsmKeyId >> 8) & 0xFF;
    privatekey[CHIP_SE05x_NODE_OP_KEY_ID_INDEX]     = (hsmKeyId >> 0) & 0xFF;

    memcpy(serializedKeypair.Bytes(), &publickey, pubkey_len);
    memcpy(serializedKeypair.Bytes() + pubkey_len, &privatekey[0], privatekey_len);
    serializedKeypair.SetLength(privatekey_len + pubkey_len);

    // This is required to ensure we pass the key id (mapping to fabric id) to CHIPCryptoPALHsm_se05x_p256.cpp NIST256 class.
    ReturnErrorOnFailure(mPendingKeypair->Deserialize(serializedKeypair));

    ChipLogDetail(Crypto,
                  "PersistentStorageOpKeystorese05x::NewOpKeypairForFabric ::Create NIST256 key in SE05x (at id = 0x%" PRIx32 ")",
                  hsmKeyId);
    mPendingKeypair->Initialize(Crypto::ECPKeyTarget::ECDSA);
    size_t csrLength = outCertificateSigningRequest.size();
    CHIP_ERROR err   = mPendingKeypair->NewCertificateSigningRequest(outCertificateSigningRequest.data(), csrLength);
    if (err != CHIP_NO_ERROR)
    {
        ResetPendingKey();
        return err;
    }

    outCertificateSigningRequest.reduce_size(csrLength);
    mPendingFabricIndex = fabricIndex;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOpKeystorese05x::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    uint32_t keyId = CHIP_SE05x_NODE_OP_KEY_INDEX + fabricIndex;

    ChipLogDetail(
        Crypto, "PersistentStorageOpKeystorese05x::RemoveOpKeypairForFabric ::Delete NIST256 key in SE05x (at id = 0x%" PRIx32 ")",
        keyId);
    Se05x_API_DeleteSecureObject(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, keyId);

    // remove key from secure element
    if ((mPendingKeypair != nullptr) && (fabricIndex == mPendingFabricIndex))
    {
        RevertPendingKeypair();
    }

    CHIP_ERROR err = mStorage->SyncDeleteKeyValue(DefaultStorageKeyAllocator::FabricOpKey(fabricIndex).KeyName());
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_ERROR_INVALID_FABRIC_INDEX;
    }

    return err;
}

} // namespace chip
