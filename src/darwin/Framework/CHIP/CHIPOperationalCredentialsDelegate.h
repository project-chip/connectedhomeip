/**
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include <memory>

#import <Foundation/Foundation.h>
#import <Security/Security.h>

#import "CHIPError_Internal.h"
#import "CHIPP256KeypairBridge.h"
#import "CHIPPersistentStorageDelegateBridge.h"

#include <controller/OperationalCredentialsDelegate.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CASEAuthTag.h>
#include <platform/Darwin/CHIPP256KeypairNativeBridge.h>

NS_ASSUME_NONNULL_BEGIN

class CHIPOperationalCredentialsDelegate : public chip::Controller::OperationalCredentialsDelegate {
public:
    using ChipP256KeypairPtr = std::unique_ptr<chip::Crypto::P256Keypair>;

    ~CHIPOperationalCredentialsDelegate() {}

    CHIP_ERROR init(CHIPPersistentStorageDelegateBridge * storage, ChipP256KeypairPtr nocSigner, NSData * ipk);

    CHIP_ERROR GenerateNOCChain(const chip::ByteSpan & csrElements, const chip::ByteSpan & csrNonce,
        const chip::ByteSpan & attestationSignature, const chip::ByteSpan & attestationChallenge, const chip::ByteSpan & DAC,
        const chip::ByteSpan & PAI, chip::Callback::Callback<chip::Controller::OnNOCChainGeneration> * onCompletion) override;

    void SetNodeIdForNextNOCRequest(chip::NodeId nodeId) override
    {
        mNextRequestedNodeId = nodeId;
        mNodeIdRequested = true;
    }

    void SetFabricIdForNextNOCRequest(chip::FabricId fabricId) override { mNextFabricId = fabricId; }

    void SetDeviceID(chip::NodeId deviceId) { mDeviceBeingPaired = deviceId; }
    void ResetDeviceID() { mDeviceBeingPaired = chip::kUndefinedNodeId; }

    CHIP_ERROR GenerateNOCChainAfterValidation(chip::NodeId nodeId, chip::FabricId fabricId, const chip::CATValues & cats,
        const chip::Crypto::P256PublicKey & pubkey, chip::MutableByteSpan & rcac, chip::MutableByteSpan & icac,
        chip::MutableByteSpan & noc);

    const chip::Crypto::AesCcm128KeySpan GetIPK() { return mIPK.Span(); }

private:
    bool ToChipEpochTime(uint32_t offset, uint32_t & epoch);

    ChipP256KeypairPtr mIssuerKey;
    uint64_t mIssuerId = 1234;

    chip::Crypto::AesCcm128Key mIPK;

    const uint32_t kCertificateValiditySecs = 365 * 24 * 60 * 60;

    CHIPPersistentStorageDelegateBridge * mStorage;

    chip::NodeId mDeviceBeingPaired = chip::kUndefinedNodeId;

    chip::NodeId mNextRequestedNodeId = 1;
    chip::FabricId mNextFabricId = 1;
    bool mNodeIdRequested = false;
};

NS_ASSUME_NONNULL_END
