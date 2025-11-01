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

namespace chip {
namespace Credentials {
namespace JCM {

/**
 * A client that handles Vendor ID verification
 */
class DLL_EXPORT VendorIdVerificationClient
{
public:
    virtual ~VendorIdVerificationClient() = default;

    // Used to obtain SessionHandles from VerifyVendorId callers. SessionHandles cannot be stored, so we must retrieve them
    // dynamically with a callback.
    using SessionGetterFunc = std::function<Optional<SessionHandle>()>;

    CHIP_ERROR VerifyVendorId(Messaging::ExchangeManager * exchangeMgr, const SessionGetterFunc getSession,
                              TrustVerificationInfo * info);

protected:
    virtual CHIP_ERROR OnLookupOperationalTrustAnchor(VendorId vendorID, CertificateKeyId & subjectKeyId,
                                                      ByteSpan & globallyTrustedRootSpan) = 0;
    virtual void OnVendorIdVerificationComplete(const CHIP_ERROR & err)                   = 0;

private:
    CHIP_ERROR VerifyNOCCertificateChain(const ByteSpan & nocSpan, const ByteSpan & icacSpan, const ByteSpan & rcacSpan);

    CHIP_ERROR
    Verify(TrustVerificationInfo * info, const ByteSpan clientChallengeSpan, ByteSpan attestationChallengeSpan,
           const app::Clusters::OperationalCredentials::Commands::SignVIDVerificationResponse::DecodableType responseData);
};

} // namespace JCM
} // namespace Credentials
} // namespace chip
