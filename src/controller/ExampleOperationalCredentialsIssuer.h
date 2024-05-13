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

#include <controller/OperationalCredentialsDelegate.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CASEAuthTag.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

namespace chip {
namespace Controller {

class DLL_EXPORT ExampleOperationalCredentialsIssuer : public OperationalCredentialsDelegate
{
public:
    //
    // Constructor to create an instance of this object that vends out operational credentials for a given fabric.
    //
    // An index should be provided to numerically identify this instance relative to others in a multi-fabric deployment. This is
    // needed given the interactions of this object with persistent storage. Consequently, the index is used to scope the entries
    // read/written to/from storage.
    //
    // It is recommended that this index track the fabric index within which this issuer is operating.
    //
    ExampleOperationalCredentialsIssuer(uint32_t index = 0) { mIndex = index; }
    ~ExampleOperationalCredentialsIssuer() override {}

    CHIP_ERROR GenerateNOCChain(const ByteSpan & csrElements, const ByteSpan & csrNonce, const ByteSpan & attestationSignature,
                                const ByteSpan & attestationChallenge, const ByteSpan & DAC, const ByteSpan & PAI,
                                Callback::Callback<OnNOCChainGeneration> * onCompletion) override;

    void SetNodeIdForNextNOCRequest(NodeId nodeId) override
    {
        mNextRequestedNodeId = nodeId;
        mNodeIdRequested     = true;
    }

    void SetMaximallyLargeCertsUsed(bool areMaximallyLargeCertsUsed) { mUseMaximallySizedCerts = areMaximallyLargeCertsUsed; }

    void SetFabricIdForNextNOCRequest(FabricId fabricId) override { mNextFabricId = fabricId; }

    void SetCATValuesForNextNOCRequest(CATValues cats) { mNextCATs = cats; }

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
    [[deprecated("This class stores the encryption key in clear storage. Don't use it for production code.")]] CHIP_ERROR
    Initialize(PersistentStorageDelegate & storage);

    void SetIssuerId(uint32_t id) { mIssuerId = id; }

    void SetCurrentEpoch(uint32_t epoch) { mNow = epoch; }

    void SetCertificateValidityPeriod(uint32_t validity) { mValidity = validity; }

    /**
     * Generate a random operational node id.
     *
     * @param[out] aNodeId where to place the generated id.
     *
     * On error no guarantees are made about the state of aNodeId.
     */
    static CHIP_ERROR GetRandomOperationalNodeId(NodeId * aNodeId);

    /**
     * This is a utility method that generates a operational certificate chain for the given public key.
     * This method is expected to be called once all the checks (e.g. device attestation, CSR verification etc)
     * have been completed, or not required (e.g. for self trusted devices such as commissioner apps).
     */
    CHIP_ERROR GenerateNOCChainAfterValidation(NodeId nodeId, FabricId fabricId, const CATValues & cats,
                                               const Crypto::P256PublicKey & pubkey, MutableByteSpan & rcac, MutableByteSpan & icac,
                                               MutableByteSpan & noc);

private:
    Crypto::P256Keypair mIssuer;
    Crypto::P256Keypair mIntermediateIssuer;
    bool mInitialized              = false;
    uint32_t mIssuerId             = 1;
    uint32_t mIntermediateIssuerId = 2;
    uint32_t mNow                  = 0;

    // By default, let's set validity to 10 years
    uint32_t mValidity = 365 * 24 * 60 * 60 * 10;

    NodeId mNextAvailableNodeId          = 1;
    PersistentStorageDelegate * mStorage = nullptr;
    bool mUseMaximallySizedCerts         = false;

    NodeId mNextRequestedNodeId = 1;
    FabricId mNextFabricId      = 1;
    CATValues mNextCATs         = kUndefinedCATs;
    bool mNodeIdRequested       = false;
    uint64_t mIndex             = 0;
};

} // namespace Controller
} // namespace chip
