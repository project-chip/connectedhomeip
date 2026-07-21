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

#pragma once

#include <cstdint>
#include <string>

#include <app-common/zap-generated/cluster-objects.h>
#include <credentials/CHIPCert.h>
#include <credentials/FabricTable.h>
#include <credentials/jcm/TrustVerification.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/DLLUtil.h>
#include <messaging/ExchangeMgr.h>

#include <functional>
#include <memory>
#include <utility>

namespace chip {
namespace Credentials {
namespace JCM {

/**
 * A client that handles Vendor ID verification
 */
class DLL_EXPORT VendorIdVerificationClient
{
public:
    VendorIdVerificationClient()          = default;
    virtual ~VendorIdVerificationClient() = default;

    // We forbid moving or copying because they could cause old GuardWithLiveness callbacks to execute with a stale `this`.
    // VendorIdVerificationClients are not currently copied or moved, but this is a good defensive change.
    VendorIdVerificationClient(const VendorIdVerificationClient &)             = delete;
    VendorIdVerificationClient & operator=(const VendorIdVerificationClient &) = delete;
    VendorIdVerificationClient(VendorIdVerificationClient &&)                  = delete;
    VendorIdVerificationClient & operator=(VendorIdVerificationClient &&)      = delete;

    // Used to obtain SessionHandles from VerifyVendorId callers. SessionHandles cannot be stored, so we must retrieve them
    // dynamically with a callback.
    using SessionGetterFunc = std::function<Optional<SessionHandle>()>;

    CHIP_ERROR VerifyVendorId(Messaging::ExchangeManager * exchangeMgr, const SessionGetterFunc getSession,
                              TrustVerificationInfo * info);

protected:
    virtual CHIP_ERROR OnLookupOperationalTrustAnchor(VendorId vendorID, CertificateKeyId & subjectKeyId,
                                                      ByteSpan & globallyTrustedRootSpan) = 0;
    virtual void OnVendorIdVerificationComplete(const CHIP_ERROR & err)                   = 0;

    /**
     * Liveness guard for async callbacks. Returns a function that only executes the input function if `this` is still in scope.
     *
     * Intended for async callbacks that might outlive `this`.
     *
     * @param [in] f The function to invoke if `this` is alive
     *
     * @return A new function, `f'`, which invokes `f` if and only if `this` is alive
     */
    template <typename F>
    auto GuardWithLiveness(F && f)
    {
        std::weak_ptr<int> weak = mLivenessToken;
        return [weak, f = std::forward<F>(f)](auto &&... args) {
            // If we're able to lock the pointer to the liveness token, `this` is still alive and the function is safe to execute.
            if (auto strong = weak.lock())
            {
                f(std::forward<decltype(args)>(args)...);
            }
        };
    }

private:
    CHIP_ERROR VerifyNOCCertificateChain(const ByteSpan & nocSpan, const ByteSpan & icacSpan, const ByteSpan & rcacSpan);

    CHIP_ERROR
    Verify(TrustVerificationInfo * info, const ByteSpan clientChallengeSpan, ByteSpan attestationChallengeSpan,
           const app::Clusters::OperationalCredentials::Commands::SignVIDVerificationResponse::DecodableType responseData);

    /**
     * Token whose lifetime tracks this object; see GuardWithLiveness().
     */
    std::shared_ptr<int> mLivenessToken = std::make_shared<int>(0);
};

} // namespace JCM
} // namespace Credentials
} // namespace chip
