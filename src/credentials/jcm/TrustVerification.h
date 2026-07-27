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

struct TrustVerificationInfo
{
    EndpointId adminEndpointId   = kInvalidEndpointId;
    FabricIndex adminFabricIndex = kUndefinedFabricIndex;

    VendorId adminVendorId;
    FabricId adminFabricId;

    Platform::ScopedMemoryBufferWithSize<uint8_t> rootPublicKey;
    Platform::ScopedMemoryBufferWithSize<uint8_t> adminRCAC;
    Platform::ScopedMemoryBufferWithSize<uint8_t> adminICAC;
    Platform::ScopedMemoryBufferWithSize<uint8_t> adminNOC;

    void Cleanup()
    {
        adminEndpointId  = kInvalidEndpointId;
        adminFabricIndex = kUndefinedFabricIndex;
        adminVendorId    = VendorId::Common;
        adminFabricId    = kUndefinedFabricId;
        rootPublicKey.Free();
        adminNOC.Free();
        adminICAC.Free();
        adminRCAC.Free();
    }
};

enum class TrustVerificationError : uint16_t
{
    kSuccess = 0,
    kAsync   = 1,

    kInvalidAdministratorEndpointId  = 100,
    kInvalidAdministratorFabricIndex = 101,
    kInvalidAdministratorCAT         = 102,
    kTrustVerificationDelegateNotSet = 103,
    kUserDeniedConsent               = 104,
    kVendorIdVerificationFailed      = 105,
    kReadAdminAttributeFailed        = 106,
    kAdministratorIdMismatched       = 107,

    kInternalError = 200,
};

enum TrustVerificationStage : uint8_t
{
    kIdle,

    kVerifyingAdministratorInformation,
    kPerformingVendorIDVerification,
    kAskingUserForConsent,
    kStoringEndpointID,
    kReadingCommissionerAdminFabricIndex,
    kCrossCheckingAdministratorIds,

    kComplete,
    kError,
};

/*
 * EnumToString is a utility function that converts a TrustVerificationError enum value
 * to its string representation for logging purposes.
 *
 * @param error The TrustVerificationError to convert.
 * @return A string representation of the TrustVerificationError.
 */
inline std::string EnumToString(TrustVerificationError error)
{
    switch (error)
    {
    case TrustVerificationError::kSuccess:
        return "SUCCESS";
    case TrustVerificationError::kAsync:
        return "ASYNC_OPERATION";
    case TrustVerificationError::kInvalidAdministratorEndpointId:
        return "INVALID_ADMINISTRATOR_ENDPOINT_ID";
    case TrustVerificationError::kInvalidAdministratorFabricIndex:
        return "INVALID_ADMINISTRATOR_FABRIC_INDEX";
    case TrustVerificationError::kInvalidAdministratorCAT:
        return "INVALID_ADMINISTRATOR_CAT";
    case TrustVerificationError::kTrustVerificationDelegateNotSet:
        return "TRUST_VERIFICATION_DELEGATE_NOT_SET";
    case TrustVerificationError::kUserDeniedConsent:
        return "USER_DENIED_CONSENT";
    case TrustVerificationError::kVendorIdVerificationFailed:
        return "VENDOR_ID_VERIFICATION_FAILED";
    case TrustVerificationError::kReadAdminAttributeFailed:
        return "READ_ADMIN_ATTRIBUTE_FAILED";
    case TrustVerificationError::kAdministratorIdMismatched:
        return "ADMINISTRATOR_ID_MISMATCHED";
    case TrustVerificationError::kInternalError:
        return "INTERNAL_ERROR";

    default:
        return "UNKNOWN_ERROR";
    }
}

/*
 * EnumToString is a utility function that converts a TrustVerificationStage enum value
 * to its string representation for logging purposes.
 *
 * @param stage The TrustVerificationStage to convert.
 * @return A string representation of the TrustVerificationStage.
 */
inline std::string EnumToString(TrustVerificationStage stage)
{
    switch (stage)
    {
    case kIdle:
        return "IDLE";
    case kVerifyingAdministratorInformation:
        return "VERIFYING_ADMINISTRATOR_INFORMATION";
    case kPerformingVendorIDVerification:
        return "PERFORMING_VENDOR_ID_VERIFICATION_PROCEDURE";
    case kAskingUserForConsent:
        return "ASKING_USER_FOR_CONSENT";
    case kStoringEndpointID:
        return "STORING_ENDPOINT_ID";
    case kReadingCommissionerAdminFabricIndex:
        return "READING_COMMISSIONER_ADMIN_FABRIC_INDEX";
    case kCrossCheckingAdministratorIds:
        return "CROSS_CHECKING_ADMINISTRATOR_IDS";
    case kComplete:
        return "COMPLETE";
    case kError:
        return "ERROR";

    default:
        return "UNKNOWN";
    }
}

class TrustVerificationDelegate;

class DLL_EXPORT TrustVerificationStateMachine
{
public:
    virtual ~TrustVerificationStateMachine() = default;

    void RegisterTrustVerificationDelegate(TrustVerificationDelegate * trustVerificationDelegate);

    /*
     * ContinueAfterUserConsent is a method that continues the JCM trust verification process after the user has
     * provided consent or denied it. If the user grants consent, the trust verification process will continue;
     * otherwise, it will terminate with an error.
     *
     * @param consent A boolean indicating whether the user granted consent (true) or denied it (false).
     */
    virtual void ContinueAfterUserConsent(const bool & consent) {}

protected:
    virtual TrustVerificationStage GetNextTrustVerificationStage(const TrustVerificationStage & currentStage) = 0;
    virtual void PerformTrustVerificationStage(const TrustVerificationStage & nextStage)                      = 0;

    void StartTrustVerification();
    void TrustVerificationStageFinished(const TrustVerificationStage & completedStage, const TrustVerificationError & error);

    /*
     * OnTrustVerificationComplete is a callback method that is called when the JCM trust verification process is complete.
     * It will handle the result of the trust verification and report it to the commissioning delegate.
     *
     * @param result The result of the JCM trust verification process.
     */
    virtual void OnTrustVerificationComplete(TrustVerificationError error) {}

    // JCM trust verification info
    // This structure contains the information needed for JCM trust verification
    // such as the administrator fabric index, endpoint ID, and vendor ID
    // It is used to store the results of the trust verification process
    // and is passed to the JCM trust verification delegate
    // when the trust verification process is complete
    TrustVerificationInfo mInfo;

    // Trust verification delegate for the commissioning client
    TrustVerificationDelegate * mTrustVerificationDelegate = nullptr;
};

/**
 * A delegate that can be notified of progress as the JCM Trust Verification check proceeds.
 */
class DLL_EXPORT TrustVerificationDelegate
{
public:
    virtual ~TrustVerificationDelegate() = default;

    virtual void OnProgressUpdate(TrustVerificationStateMachine & stateMachine, TrustVerificationStage stage,
                                  TrustVerificationInfo & info, TrustVerificationError error)                    = 0;
    virtual void OnAskUserForConsent(TrustVerificationStateMachine & stateMachine, TrustVerificationInfo & info) = 0;
    virtual CHIP_ERROR OnLookupOperationalTrustAnchor(VendorId vendorID, CertificateKeyId & subjectKeyId,
                                                      ByteSpan & globallyTrustedRootSpan)                        = 0;
};

} // namespace JCM
} // namespace Credentials
} // namespace chip
