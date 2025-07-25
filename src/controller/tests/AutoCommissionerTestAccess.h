/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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

#include <controller/AutoCommissioner.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMemString.h>

#include <app/AttributePathParams.h>
#include <controller/CommissioneeDeviceProxy.h>
#include <controller/CommissioningDelegate.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/ScopedBuffer.h>
#include <protocols/secure_channel/RendezvousParameters.h>

namespace chip {

namespace Test {
// Provides access to private/protected members of AutoCommissioner class for testing
class AutoCommissionerTestAccess
{
public:
    AutoCommissionerTestAccess() = delete;
    AutoCommissionerTestAccess(Controller::AutoCommissioner * commissioner) : mCommissioner(commissioner) {}

    Controller::CommissioningStage AccessGetNextCommissioningStageInternal(Controller::CommissioningStage currentStage,
                                                                           CHIP_ERROR & lastErr)
    {
        return mCommissioner->GetNextCommissioningStageInternal(currentStage, lastErr);
    }
    void SetBreadcrumb(uint64_t value) { mCommissioner->mDeviceCommissioningInfo.general.breadcrumb = value; }
    void SetRequiresUTC(bool requiresUTC) { mCommissioner->mDeviceCommissioningInfo.requiresUTC = requiresUTC; }
    void SetRequiresTimeZone(bool requiresTimeZone)
    {
        mCommissioner->mDeviceCommissioningInfo.requiresTimeZone = requiresTimeZone;
    }

    void AccessSetTrustedTimeSource(
        app::DataModel::Nullable<app::Clusters::TimeSynchronization::Structs::FabricScopedTrustedTimeSourceStruct::Type>
            trustedTimeSource)
    {
        mCommissioner->mParams.SetTrustedTimeSource(trustedTimeSource);
    }
    void AccessSetTimeZone(app::DataModel::List<app::Clusters::TimeSynchronization::Structs::TimeZoneStruct::Type> timeZone)
    {
        mCommissioner->mParams.SetTimeZone(timeZone);
    }
    void SetNeedIcdRegistration(bool NeedIcdRegistration) { mCommissioner->mNeedIcdRegistration = NeedIcdRegistration; }
    void SetNeedsDST(bool NeedsDST) { mCommissioner->mNeedsDST = NeedsDST; }
    void AccessSetDSTOffsets(app::DataModel::List<app::Clusters::TimeSynchronization::Structs::DSTOffsetStruct::Type> dstOffset)
    {
        mCommissioner->mParams.SetDSTOffsets(dstOffset);
    }
    void SetRequiresDefaultNTP(bool requiresDefaultNTP)
    {
        mCommissioner->mDeviceCommissioningInfo.requiresDefaultNTP = requiresDefaultNTP;
    }
    void AccessSetDefaultNTP(app::DataModel::Nullable<CharSpan> defaultNTP) { mCommissioner->mParams.SetDefaultNTP(defaultNTP); }

    void SetRequiresTrustedTimeSource(bool requiresTrustedTimeSource)
    {
        mCommissioner->mDeviceCommissioningInfo.requiresTrustedTimeSource = requiresTrustedTimeSource;
    }

private:
    Controller::AutoCommissioner * mCommissioner = nullptr;
};

} // namespace Test
} // namespace chip
