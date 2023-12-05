/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <app/util/basic-types.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/PeerId.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/Span.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace Controller {

typedef void (*OnNOCChainGeneration)(void * context, CHIP_ERROR status, const ByteSpan & noc, const ByteSpan & icac,
                                     const ByteSpan & rcac, Optional<Crypto::IdentityProtectionKeySpan> ipk,
                                     Optional<NodeId> adminSubject);

inline constexpr uint32_t kMaxCHIPDERCertLength = 600;
inline constexpr size_t kCSRNonceLength         = 32;

/// Callbacks for CHIP operational credentials generation
class DLL_EXPORT OperationalCredentialsDelegate
{
public:
    virtual ~OperationalCredentialsDelegate() {}

    /**
     * @brief
     *   This function generates an operational certificate chain for a remote device that is being commissioned.
     *   The API generates the certificate in X.509 DER format.
     *
     *   The delegate is expected to use the certificate authority whose certificate
     *   is returned in `GetRootCACertificate()` API call.
     *
     *   The delegate will call `onCompletion` when the NOC certificate chain is ready.
     *
     * @param[in] csrElements          CSR elements as per specifications section 11.18.5.6. NOCSR Elements.
     * @param[in] csrNonce             CSR nonce as described in 6.4.6.1
     * @param[in] attestationSignature Attestation signature as per specifications section 11.22.7.6. CSRResponse Command.
     * @param[in] attestationChallenge Attestation challenge as per 11.18.5.7
     * @param[in] DAC                  Device attestation certificate received from the device being commissioned
     * @param[in] PAI                  Product Attestation Intermediate certificate
     * @param[in] onCompletion         Callback handler to provide generated NOC chain to the caller of GenerateNOCChain()
     *
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual CHIP_ERROR GenerateNOCChain(const ByteSpan & csrElements, const ByteSpan & csrNonce,
                                        const ByteSpan & attestationSignature, const ByteSpan & attestationChallenge,
                                        const ByteSpan & DAC, const ByteSpan & PAI,
                                        Callback::Callback<OnNOCChainGeneration> * onCompletion) = 0;

    /**
     *   This function sets the node ID for which the next NOC Chain would be requested. The node ID is
     *   provided as a hint, and the delegate implementation may chose to ignore it and pick node ID of
     *   their choice.
     */
    virtual void SetNodeIdForNextNOCRequest(NodeId nodeId) {}

    /**
     *   This function sets the fabric ID for which the next NOC Chain should be generated. This API is
     *   not required to be implemented if the delegate implementation has other mechanisms to find the
     *   fabric ID.
     */
    virtual void SetFabricIdForNextNOCRequest(FabricId fabricId) {}

    virtual CHIP_ERROR ObtainCsrNonce(MutableByteSpan & csrNonce)
    {
        VerifyOrReturnError(csrNonce.size() == kCSRNonceLength, CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(Crypto::DRBG_get_bytes(csrNonce.data(), csrNonce.size()));
        return CHIP_NO_ERROR;
    }
};

} // namespace Controller
} // namespace chip
