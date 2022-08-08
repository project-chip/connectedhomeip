/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "PersistentStorageOperationalKeystoreHSM.h"
#include "CHIPCryptoPALHsm_SE05X_utils.h"
#include <crypto/hsm/CHIPCryptoPALHsm.h>

#if ENABLE_HSM_GENERATE_EC_KEY

namespace chip {

using namespace chip::Crypto;

#define MAX_KEYID_SLOTS_FOR_FABRICS 32
#define FABRIC_SE05X_KEYID_START 0x7D100000

struct keyidFabIdMapping_t
{
    uint32_t keyId;
    FabricIndex fabricIndex;
    bool isPending;
    Crypto::P256KeypairHSM * pkeyPair;
} keyidFabIdMapping[MAX_KEYID_SLOTS_FOR_FABRICS] = {
    0,
};

uint8_t getEmpytSlotId()
{
    uint8_t i = 0;
    while (i < MAX_KEYID_SLOTS_FOR_FABRICS)
    {
        if (keyidFabIdMapping[i].keyId == 0 && keyidFabIdMapping[i].isPending == 0)
        {
            break;
        }
        i++;
    }
    return i;
}

bool PersistentStorageOperationalKeystoreHSM::HasOpKeypairForFabric(FabricIndex fabricIndex) const
{
    uint8_t i = 0;
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), false);

    // If there was a pending keypair, then there's really a usable key
    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex) && (mPendingKeypair != nullptr))
    {
        ChipLogProgress(Crypto, "SE05x: HasOpKeypairForFabric ==> mPendingKeypair found");
        return true;
    }

    while (i < MAX_KEYID_SLOTS_FOR_FABRICS)
    {
        if (keyidFabIdMapping[i].fabricIndex == fabricIndex)
        {
            ChipLogProgress(Crypto, "SE05x: HasOpKeypairForFabric ==> stored keyPair found");
            return true;
        }
        i++;
    }

    ChipLogProgress(Crypto, "SE05x: HasOpKeypairForFabric ==> No key found");
    return false;
}

CHIP_ERROR PersistentStorageOperationalKeystoreHSM::NewOpKeypairForFabric(FabricIndex fabricIndex,
                                                                          MutableByteSpan & outCertificateSigningRequest)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t slotId = getEmpytSlotId();

    VerifyOrReturnError(slotId < MAX_KEYID_SLOTS_FOR_FABRICS, CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    ChipLogProgress(Crypto, "SE05x: New Op Keypair for Fabric %02x", fabricIndex);

    // Replace previous pending keypair, if any was previously allocated
    ResetPendingKey();

    mPendingKeypair = Platform::New<Crypto::P256KeypairHSM>();
    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_NO_MEMORY);

    // Key id is created as slotid + start offset of ops key id
    mPendingKeypair->SetKeyId(FABRIC_SE05X_KEYID_START + slotId);

    err = mPendingKeypair->Initialize();
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_NO_MEMORY);

    mPendingFabricIndex = fabricIndex;

    keyidFabIdMapping[slotId].isPending = 1;

    size_t csrLength = outCertificateSigningRequest.size();
    err              = mPendingKeypair->NewCertificateSigningRequest(outCertificateSigningRequest.data(), csrLength);
    if (err != CHIP_NO_ERROR)
    {
        ResetPendingKey();
        return err;
    }
    outCertificateSigningRequest.reduce_size(csrLength);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystoreHSM::ActivateOpKeypairForFabric(FabricIndex fabricIndex,
                                                                               const Crypto::P256PublicKey & nocPublicKey)
{
    ChipLogProgress(Crypto, "SE05x: ActivateOpKeypair for Fabric %02x", fabricIndex);

    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Validate public key being activated matches last generated pending keypair
    VerifyOrReturnError(mPendingKeypair->Pubkey().Matches(nocPublicKey), CHIP_ERROR_INVALID_PUBLIC_KEY);

    mIsPendingKeypairActive = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystoreHSM::CommitOpKeypairForFabric(FabricIndex fabricIndex)
{
    uint8_t i       = 0;

    VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mIsPendingKeypairActive == true, CHIP_ERROR_INCORRECT_STATE);

    uint32_t slotId = mPendingKeypair->GetKeyId() - FABRIC_SE05X_KEYID_START;

    VerifyOrReturnError(slotId < MAX_KEYID_SLOTS_FOR_FABRICS, CHIP_ERROR_NO_MEMORY);

    ChipLogProgress(Crypto, "SE05x: CommitOpKeypair for Fabric %02x", fabricIndex);

    while (i < MAX_KEYID_SLOTS_FOR_FABRICS)
    {
        if (keyidFabIdMapping[i].fabricIndex == fabricIndex)
        {
            // Delete the previous keyPair associated with the fabric
            keyidFabIdMapping[i].isPending = 0;
            Platform::Delete<Crypto::P256KeypairHSM>(keyidFabIdMapping[i].pkeyPair);
            keyidFabIdMapping[i].pkeyPair    = NULL;
            keyidFabIdMapping[i].keyId       = 0;
            keyidFabIdMapping[i].fabricIndex = 0;
        }
        i++;
    }

    keyidFabIdMapping[slotId].pkeyPair    = mPendingKeypair;
    keyidFabIdMapping[slotId].keyId       = mPendingKeypair->GetKeyId();
    keyidFabIdMapping[slotId].fabricIndex = mPendingFabricIndex;
    keyidFabIdMapping[slotId].isPending   = 0;

    // If we got here, we succeeded and can reset the pending key: next `SignWithOpKeypair` will use the stored key.
    mPendingKeypair           = nullptr;
    mIsPendingKeypairActive   = false;
    mPendingFabricIndex       = kUndefinedFabricIndex;
    
    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystoreHSM::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
{
    uint8_t i = 0;
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    ChipLogProgress(Crypto, "SE05x: RemoveOpKeypair for Fabric %02x", fabricIndex);

    // Remove pending state if matching
    if ((mPendingKeypair != nullptr) && (fabricIndex == mPendingFabricIndex))
    {
        RevertPendingKeypair();
    }

    while (i < MAX_KEYID_SLOTS_FOR_FABRICS)
    {
        if (keyidFabIdMapping[i].fabricIndex == fabricIndex)
        {
            // Delete the keyPair associated with the fabric
            keyidFabIdMapping[i].isPending = 0;
            Platform::Delete<Crypto::P256KeypairHSM>(keyidFabIdMapping[i].pkeyPair);
            keyidFabIdMapping[i].pkeyPair    = NULL;
            keyidFabIdMapping[i].keyId       = 0;
            keyidFabIdMapping[i].fabricIndex = 0;
        }
        i++;
    }

    return CHIP_NO_ERROR;
}

void PersistentStorageOperationalKeystoreHSM::RevertPendingKeypair()
{
    ChipLogProgress(Crypto, "SE05x: RevertPendingKeypair");
    // Just reset the pending key, we never stored anything
    ResetPendingKey();
}

CHIP_ERROR PersistentStorageOperationalKeystoreHSM::SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                                                      Crypto::P256ECDSASignature & outSignature) const
{
    uint8_t i = 0;
    ChipLogProgress(Crypto, "SE05x: SignWithOpKeypair");

    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex))
    {
        VerifyOrReturnError(mPendingKeypair != nullptr, CHIP_ERROR_INTERNAL);
        // We have an override key: sign with it!
        ChipLogProgress(Crypto, "SE05x: SignWithOpKeypair ==> using mPendingKeypair");
        return mPendingKeypair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
    }
    else
    {
        while (i < MAX_KEYID_SLOTS_FOR_FABRICS)
        {
            if ((keyidFabIdMapping[i].fabricIndex == fabricIndex) && (keyidFabIdMapping[i].isPending == 0))
            {
                ChipLogProgress(Crypto, "SE05x: SignWithOpKeypair ==> using stored keyPair");
                return keyidFabIdMapping[i].pkeyPair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
            }
            i++;
        }
    }

    ChipLogProgress(Crypto, "SE05x: SignWithOpKeypair ==> No keyPair found");
    return CHIP_ERROR_INVALID_FABRIC_INDEX;
}

Crypto::P256Keypair * PersistentStorageOperationalKeystoreHSM::AllocateEphemeralKeypairForCASE()
{
    ChipLogProgress(Crypto, "SE05x: AllocateEphemeralKeypairForCASE using se05x");
    Crypto::P256KeypairHSM * pkeyPair = Platform::New<Crypto::P256KeypairHSM>();

    if (pkeyPair != nullptr){
        pkeyPair->SetKeyId(kKeyId_case_ephemeral_keyid);
    }
    
    return pkeyPair;
}

void PersistentStorageOperationalKeystoreHSM::ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair)
{
    ChipLogProgress(Crypto, "SE05x: ReleaseEphemeralKeypair using se05x");
    Platform::Delete(static_cast<Crypto::P256KeypairHSM*>(keypair));
}

} // namespace chip

#endif //#if ENABLE_HSM_GENERATE_EC_KEY
