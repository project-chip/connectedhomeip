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

#include <app-common/zap-generated/cluster-objects.h>
#include <cstdint>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/DLLUtil.h>

namespace chip {
namespace Controller {

namespace JCM {

struct JCMTrustVerificationInfo
{
    EndpointId adminEndpointId   = kInvalidEndpointId;
    FabricIndex adminFabricIndex = kUndefinedFabricIndex;

    VendorId adminVendorId;
    FabricId adminFabricId;

    ByteSpan rootPublicKey;
    Platform::ScopedMemoryBufferWithSize<uint8_t> adminRCAC;
    Platform::ScopedMemoryBufferWithSize<uint8_t> adminICAC;
    Platform::ScopedMemoryBufferWithSize<uint8_t> adminNOC;

    void Cleanup()
    {
        adminEndpointId  = kInvalidEndpointId;
        adminFabricIndex = kUndefinedFabricIndex;
        adminVendorId    = VendorId::Common;
        adminFabricId    = kUndefinedFabricId;
        rootPublicKey    = MutableByteSpan{};
        adminNOC.Free();
        adminICAC.Free();
        adminRCAC.Free();
    }
};

enum class JCMTrustVerificationError : uint16_t
{
    kSuccess = 0,
    kAsync   = 1,

    kInvalidAdministratorEndpointId  = 100,
    kInvalidAdministratorFabricIndex = 101,
    kInvalidAdministratorCAT         = 102,
    kTrustVerificationDelegateNotSet = 103,
    kUserDeniedConsent               = 104,
    kVendorIdVerificationFailed      = 105,

    kInternalError = 200,
};

class JCMDeviceCommissioner;

enum JCMTrustVerificationStage : uint8_t
{
    kIdle,
    kVerifyingAdministratorInformation,
    kPerformingVendorIDVerification,
    kAskingUserForConsent,
    kComplete,
    kError,
};

/*
 * enumToString is a utility function that converts a JCMTrustVerificationError enum value
 * to its string representation for logging purposes.
 *
 * @param error The JCMTrustVerificationError to convert.
 * @return A string representation of the JCMTrustVerificationError.
 */
inline std::string enumToString(JCMTrustVerificationError error)
{
    switch (error)
    {
    case JCMTrustVerificationError::kSuccess:
        return "SUCCESS";
    case JCMTrustVerificationError::kAsync:
        return "ASYNC_OPERATION";
    case JCMTrustVerificationError::kInvalidAdministratorEndpointId:
        return "INVALID_ADMINISTRATOR_ENDPOINT_ID";
    case JCMTrustVerificationError::kInvalidAdministratorFabricIndex:
        return "INVALID_ADMINISTRATOR_FABRIC_INDEX";
    case JCMTrustVerificationError::kInvalidAdministratorCAT:
        return "INVALID_ADMINISTRATOR_CAT";
    case JCMTrustVerificationError::kTrustVerificationDelegateNotSet:
        return "TRUST_VERIFICATION_DELEGATE_NOT_SET";
    case JCMTrustVerificationError::kUserDeniedConsent:
        return "USER_DENIED_CONSENT";
    case JCMTrustVerificationError::kInternalError:
        return "INTERNAL_ERROR";

    default:
        return "UNKNOWN_ERROR";
    }
}

/*
 * enumToString is a utility function that converts a JCMTrustVerificationStage enum value
 * to its string representation for logging purposes.
 *
 * @param stage The JCMTrustVerificationStage to convert.
 * @return A string representation of the JCMTrustVerificationStage.
 */
inline std::string enumToString(JCMTrustVerificationStage stage)
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
    case kComplete:
        return "COMPLETE";
    case kError:
        return "ERROR";

    default:
        return "UNKNOWN";
    }
}

typedef void (*JCMTrustVerificationCompleteCallback)(void * context, JCMTrustVerificationInfo & info,
                                                     JCMTrustVerificationError result);

/**
 * A delegate that can be notified of progress as the JCM Trust Verification check proceeds.
 */
class DLL_EXPORT JCMTrustVerificationDelegate
{
public:
    virtual ~JCMTrustVerificationDelegate() = default;

    virtual void OnProgressUpdate(JCMDeviceCommissioner & commissioner, JCMTrustVerificationStage stage,
                                  JCMTrustVerificationInfo & info, JCMTrustVerificationError error)         = 0;
    virtual void OnAskUserForConsent(JCMDeviceCommissioner & commissioner, JCMTrustVerificationInfo & info) = 0;
    virtual void OnVerifyVendorId(JCMDeviceCommissioner & commissioner, JCMTrustVerificationInfo & info)    = 0;
};

} // namespace JCM
} // namespace Controller
} // namespace chip
