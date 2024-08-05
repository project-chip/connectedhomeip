/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include "OperationalCredentialsDelegate.h"

namespace chip {
namespace Controller {

namespace {

// Version to have a default placeholder so the getter never
// returns `nullptr` by default.
class UnimplementedOperationalCredentialsDelegate : public OperationalCredentialsDelegate
{
public:
    CHIP_ERROR GenerateNOCChain(const ByteSpan & csrElements, const ByteSpan & csrNonce, const ByteSpan & attestationSignature,
                                const ByteSpan & attestationChallenge, const ByteSpan & DAC, const ByteSpan & PAI,
                                Callback::Callback<OnNOCChainGeneration> * onCompletion) override
    {
        (void) csrElements;
        (void) csrNonce;
        (void) attestationSignature;
        (void) attestationChallenge;
        (void) DAC;
        (void) PAI;
        (void) onCompletion;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR SignNOCIssuer(const ByteSpan & icaCsr, Callback::Callback<OnNOCChainGeneration> * onCompletion) override
    {
        (void) icaCsr;
        (void) onCompletion;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    void SetNodeIdForNextNOCRequest(NodeId nodeId) override { (void) nodeId; }

    void SetFabricIdForNextNOCRequest(FabricId fabricId) override { (void) fabricId; }

    CHIP_ERROR ObtainCsrNonce(MutableByteSpan & csrNonce) override
    {
        (void) CSRNonce;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR ObtainIcaCsr(const ByteSpan & icac, MutableByteSpan & icaCsr) override
    {
        (void) icac;
        (void) icaCsr;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

// Default to avoid nullptr on getter and cleanly handle new products/clients before
// they provide their own.
UnimplementedOperationalCredentialsDelegate gDefaultOperationalCredentialsDelegate;

OperationalCredentialsDelegate * gOperationalCredentialsDelegate = &gDefaultOperationalCredentialsDelegate;

} // namespace

OperationalCredentialsDelegate * GetOperationalCredentialsDelegate()
{
    return gOperationalCredentialsDelegate;
}

void SetOperationalCredentialsDelegate(OperationalCredentialsDelegate * provider)
{
    if (provider == nullptr)
    {
        return;
    }

    gOperationalCredentialsDelegate = provider;
}

bool IsOperationalCredentialsDelegateSet()
{
    return (gOperationalCredentialsDelegate != &gDefaultOperationalCredentialsDelegate);
}

} // namespace Controller
} // namespace chip
