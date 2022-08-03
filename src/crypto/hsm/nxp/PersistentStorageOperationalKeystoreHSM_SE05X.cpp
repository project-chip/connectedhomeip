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

#include <crypto/OperationalKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#include "CHIPCryptoPALHsm_SE05X_utils.h"
#include "PersistentStorageOperationalKeystoreHSM_SE05X.h"


namespace chip {

using namespace chip::Crypto;

namespace {

// Tags for our operational keypair storage.
constexpr TLV::Tag kOpKeyVersionTag = TLV::ContextTag(0);
constexpr TLV::Tag kOpKeyDataTag    = TLV::ContextTag(1);

// If this version grows beyond UINT16_MAX, adjust OpKeypairTLVMaxSize
// accordingly.
constexpr uint16_t kOpKeyVersion = 1;

constexpr size_t OpKeyTLVMaxSize()
{
    // Version and serialized key
    return TLV::EstimateStructOverhead(sizeof(uint16_t), Crypto::P256SerializedKeypair::Capacity());
}

} // namespace

#define MAX_KEYID_SLOTS_FOR_FABRICS 32
#define FABRIC_SE05X_KEYID_START 0x56780000

struct keyidFabIdMapping_t {
    uint32_t keyId;
    FabricIndex fabricIndex;
    bool isActive;
    Crypto::P256KeypairHSM *pkeyPair;
} keyidFabIdMapping[MAX_KEYID_SLOTS_FOR_FABRICS] = {0,};


uint8_t getEmpytSlotId()
{
    uint8_t i = 0;
    while(i++ < MAX_KEYID_SLOTS_FOR_FABRICS){
        if (keyidFabIdMapping[i].keyId == 0){
            break;
        }
    }
    return i;
}

bool PersistentStorageOperationalKeystoreHSM::HasOpKeypairForFabric(FabricIndex fabricIndex) const
{
    uint8_t i = 0; 
    ChipLogProgress(Crypto,"Se05x: HasOpKeypairForFabric");
    while(i++ < MAX_KEYID_SLOTS_FOR_FABRICS){
        if (keyidFabIdMapping[i].fabricIndex == fabricIndex){
            return 1;
        }
    }
    return 0;
}

CHIP_ERROR PersistentStorageOperationalKeystoreHSM::NewOpKeypairForFabric(FabricIndex fabricIndex,
                                                                       MutableByteSpan & outCertificateSigningRequest)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t slotId = getEmpytSlotId();
    VerifyOrReturnError(slotId != 0, CHIP_ERROR_NO_MEMORY);

    ChipLogProgress(Crypto,"Se05x: New OPS key for Fabric %02x",fabricIndex);

    keyidFabIdMapping[slotId].pkeyPair = Platform::New<Crypto::P256KeypairHSM>();
    VerifyOrReturnError(keyidFabIdMapping[slotId].pkeyPair != nullptr, CHIP_ERROR_NO_MEMORY);

    // Key id is created as slotid + start offset of ops key id
    keyidFabIdMapping[slotId].keyId = FABRIC_SE05X_KEYID_START + slotId;
    keyidFabIdMapping[slotId].pkeyPair->SetKeyId(FABRIC_SE05X_KEYID_START + slotId);
    keyidFabIdMapping[slotId].isActive = 0; // Not yet
    keyidFabIdMapping[slotId].fabricIndex = fabricIndex;
    
    err = keyidFabIdMapping[slotId].pkeyPair->Initialize();
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_ERROR_NO_MEMORY);
    
    size_t csrLength = outCertificateSigningRequest.size();
    err   = keyidFabIdMapping[slotId].pkeyPair->NewCertificateSigningRequest(outCertificateSigningRequest.data(), csrLength);
    if (err != CHIP_NO_ERROR)
    {
        //ResetPendingKey();
        return err;
    }
    outCertificateSigningRequest.reduce_size(csrLength);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystoreHSM::ActivateOpKeypairForFabric(FabricIndex fabricIndex,
                                                                            const Crypto::P256PublicKey & nocPublicKey)
{
    ChipLogProgress(Crypto,"Se05x: ActivateOpKeypair for Fabric %02x",fabricIndex);
    //TODO - Compare public key with public key of fabricIndex (not active)
    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystoreHSM::CommitOpKeypairForFabric(FabricIndex fabricIndex)
{
    uint8_t i = 0;
    
    while(i++ < MAX_KEYID_SLOTS_FOR_FABRICS){
        if (keyidFabIdMapping[i].fabricIndex == fabricIndex){
            
            if(keyidFabIdMapping[i].isActive == 1){
                // Delete the previous keyPair associated with the fabric
                keyidFabIdMapping[i].isActive = 0;
                Platform::Delete<Crypto::P256KeypairHSM>(keyidFabIdMapping[i].pkeyPair);
                keyidFabIdMapping[i].pkeyPair = NULL;
                keyidFabIdMapping[i].keyId = 0;
                keyidFabIdMapping[i].fabricIndex = 0;
            }

            if(keyidFabIdMapping[i].isActive == 0){
                // Activate the new keyPair associated with the fabric
                keyidFabIdMapping[i].isActive = 1;
                ChipLogProgress(Crypto,"Se05x: CommitOpKeypair for Fabric %02x",fabricIndex);
            }
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystoreHSM::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
{
    printf("Se05x HSM - RemoveOpKeypairForFabric \n");
    return CHIP_NO_ERROR;
}

void PersistentStorageOperationalKeystoreHSM::RevertPendingKeypair()
{
    uint8_t i = 0;
    ChipLogProgress(Crypto,"Se05x: RevertPendingKeypair");
    
    while(i++ < MAX_KEYID_SLOTS_FOR_FABRICS){
        if(keyidFabIdMapping[i].isActive == 0){
            // Just reset the pending key
            Platform::Delete<Crypto::P256KeypairHSM>(keyidFabIdMapping[i].pkeyPair);
            keyidFabIdMapping[i].pkeyPair = NULL;
            keyidFabIdMapping[i].keyId = 0;
            keyidFabIdMapping[i].fabricIndex = 0;
        }
    }
}

CHIP_ERROR PersistentStorageOperationalKeystoreHSM::SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                                                   Crypto::P256ECDSASignature & outSignature) const
{
    uint8_t i = 0;
    ChipLogProgress(Crypto,"Se05x: RevertPendingKeypair");
    
    while(i++ < MAX_KEYID_SLOTS_FOR_FABRICS){
        if (keyidFabIdMapping[i].fabricIndex == fabricIndex){ // && keyidFabIdMapping[i].isActive){
            return keyidFabIdMapping[i].pkeyPair->ECDSA_sign_msg(message.data(), message.size(), outSignature);
        }
    }

    return CHIP_ERROR_INTERNAL;
}

Crypto::P256Keypair * PersistentStorageOperationalKeystoreHSM::AllocateEphemeralKeypairForCASE()
{
    printf("AllocateEphemeralKeypairForCASE using se05x \n");
    Crypto::P256KeypairHSM *pkeyPair = Platform::New<Crypto::P256KeypairHSM>();
    pkeyPair->SetKeyId(kKeyId_case_ephemeral_keyid);
    return pkeyPair;
}

void PersistentStorageOperationalKeystoreHSM::ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair)
{
    Platform::Delete<Crypto::P256Keypair>(keypair);
}

} // namespace chip
