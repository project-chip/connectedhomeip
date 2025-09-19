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
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/DLLUtil.h>

namespace chip {
namespace Controller {

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

    kInternalError = 200,
};

class DeviceCommissioner;

enum TrustVerificationStage : uint8_t
{
    kIdle,
    kVerifyingAdministratorInformation,
    kPerformingVendorIDVerification,
    kAskingUserForConsent,
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
    case kComplete:
        return "COMPLETE";
    case kError:
        return "ERROR";

    default:
        return "UNKNOWN";
    }
}

typedef void (*TrustVerificationCompleteCallback)(void * context, TrustVerificationInfo & info, TrustVerificationError result);

/**
 * A delegate that can be notified of progress as the JCM Trust Verification check proceeds.
 */
class DLL_EXPORT TrustVerificationDelegate
{
public:
    virtual ~TrustVerificationDelegate() = default;

    virtual void OnProgressUpdate(DeviceCommissioner & commissioner, TrustVerificationStage stage, TrustVerificationInfo & info,
                                  TrustVerificationError error)                                       = 0;
    virtual void OnAskUserForConsent(DeviceCommissioner & commissioner, TrustVerificationInfo & info) = 0;
    virtual void OnVerifyVendorId(DeviceCommissioner & commissioner, TrustVerificationInfo & info)    = 0;
};

} // namespace JCM
} // namespace Controller
} // namespace chip
