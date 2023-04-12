/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *  @file
 *    This file contains class definition of an example operational certificate
 *    issuer for CHIP devices. The class can be used as a guideline on how to
 *    construct your own certificate issuer. It can also be used in tests and tools
 *    if a specific signing authority is not required.
 *
 *    NOTE: This class stores the encryption key in clear storage. This is not suited
 *          for production use. This should only be used in test tools.
 */

#pragma once

#include <controller/AutoCommissioner.h>
#include <controller/OperationalCredentialsDelegate.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CASEAuthTag.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

#include <jni.h>

namespace chip {
namespace Controller {

class DLL_EXPORT AndroidOperationalCredentialsIssuer : public OperationalCredentialsDelegate
{
public:
    virtual ~AndroidOperationalCredentialsIssuer() {}

    CHIP_ERROR GenerateNOCChain(const ByteSpan & csrElements, const ByteSpan & csrNonce, const ByteSpan & attestationSignature,
                                const ByteSpan & attestationChallenge, const ByteSpan & DAC, const ByteSpan & PAI,
                                Callback::Callback<OnNOCChainGeneration> * onCompletion) override;

    CHIP_ERROR NOCChainGenerated(CHIP_ERROR status, const ByteSpan & noc, const ByteSpan & icac, const ByteSpan & rcac,
                                 Optional<Crypto::IdentityProtectionKeySpan> ipk, Optional<NodeId> adminSubject);

    void SetUseJavaCallbackForNOCRequest(bool useJavaCallbackForNOCRequest)
    {
        mUseJavaCallbackForNOCRequest = useJavaCallbackForNOCRequest;
    }

    void SetNodeIdForNextNOCRequest(NodeId nodeId) override
    {
        mNextRequestedNodeId = nodeId;
        mNodeIdRequested     = true;
    }

    void SetFabricIdForNextNOCRequest(FabricId fabricId) override { mNextFabricId = fabricId; }

    /**
     * @brief Initialize the issuer with the keypair in the storage.
     *        If the storage doesn't have one, it'll create one, and it to the storage.
     *
     * @param[in] storage  A reference to the storage, where the keypair is stored.
     *                     The object of ExampleOperationalCredentialsIssuer doesn't hold
     *                     on the reference of storage.
     *
     * @return Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise
     **/
    CHIP_ERROR Initialize(PersistentStorageDelegate & storage, AutoCommissioner * autoCommissioner, jobject javaObjectRef);

    void SetIssuerId(uint32_t id) { mIssuerId = id; }

    void SetCurrentEpoch(uint32_t epoch) { mNow = epoch; }

    void SetCertificateValidityPeriod(uint32_t validity) { mValidity = validity; }

    /**
     * This is a utility method that generates a operational certificate chain for the given public key.
     * This method is expected to be called once all the checks (e.g. device attestation, CSR verification etc)
     * have been completed, or not required (e.g. for self trusted devices such as commissioner apps).
     */
    CHIP_ERROR GenerateNOCChainAfterValidation(NodeId nodeId, FabricId fabricId, const CATValues & cats,
                                               const Crypto::P256PublicKey & pubkey, MutableByteSpan & rcac, MutableByteSpan & icac,
                                               MutableByteSpan & noc);

private:
    CHIP_ERROR CallbackGenerateNOCChain(const ByteSpan & csrElements, const ByteSpan & csrNonce,
                                        const ByteSpan & attestationSignature, const ByteSpan & attestationChallenge,
                                        const ByteSpan & DAC, const ByteSpan & PAI,
                                        Callback::Callback<OnNOCChainGeneration> * onCompletion);

    CHIP_ERROR LocalGenerateNOCChain(const ByteSpan & csrElements, const ByteSpan & csrNonce, const ByteSpan & attestationSignature,
                                     const ByteSpan & attestationChallenge, const ByteSpan & DAC, const ByteSpan & PAI,
                                     Callback::Callback<OnNOCChainGeneration> * onCompletion);

    Crypto::P256Keypair mIssuer;
    bool mInitialized  = false;
    uint32_t mIssuerId = 0;
    uint32_t mNow      = 0;

    // By default, let's set validity to 10 years
    uint32_t mValidity = 365 * 24 * 60 * 60 * 10;

    NodeId mNextAvailableNodeId          = 1;
    PersistentStorageDelegate * mStorage = nullptr;
    AutoCommissioner * mAutoCommissioner = nullptr;

    NodeId mNextRequestedNodeId = 1;
    FabricId mNextFabricId      = 1;
    bool mNodeIdRequested       = false;

    jobject mJavaObjectRef = nullptr;

    bool mUseJavaCallbackForNOCRequest                                  = false;
    Callback::Callback<OnNOCChainGeneration> * mOnNOCCompletionCallback = nullptr;
};

} // namespace Controller
} // namespace chip
