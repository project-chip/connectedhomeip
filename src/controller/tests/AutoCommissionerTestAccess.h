/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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

namespace Testing {
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

    Controller::CommissioningParameters & AccessParams() { return mCommissioner->mParams; }

    void CleanupCommissioning() { mCommissioner->CleanupCommissioning(); }

    CommissioneeDeviceProxy * GetCommissioneeDeviceProxy() { return mCommissioner->GetCommissioneeDeviceProxy(); }

    Optional<System::Clock::Timeout> GetCommandTimeout(DeviceProxy * device, Controller::CommissioningStage stage) const
    {
        return mCommissioner->GetCommandTimeout(device, stage);
    }

    const ByteSpan GetDAC() { return mCommissioner->GetDAC(); }

    Controller::ReadCommissioningInfo & GetDeviceCommissioningInfo() { return mCommissioner->mDeviceCommissioningInfo; }

    EndpointId GetEndpoint(const Controller::CommissioningStage & stage) const { return mCommissioner->GetEndpoint(stage); }

    bool GetNeedsDST() { return mCommissioner->mNeedsDST; }

    Controller::CommissioningStage GetNextCommissioningStageNetworkSetup(Controller::CommissioningStage currentStage,
                                                                         CHIP_ERROR & lastErr)
    {
        return mCommissioner->GetNextCommissioningStageNetworkSetup(currentStage, lastErr);
    }

    OperationalDeviceProxy GetOperationalDeviceProxy() { return mCommissioner->mOperationalDeviceProxy; }

    const ByteSpan GetPAI() { return mCommissioner->GetPAI(); }

    bool IsScanNeeded() { return mCommissioner->IsScanNeeded(); }

    bool IsSecondaryNetworkSupported() const { return mCommissioner->IsSecondaryNetworkSupported(); }

    CHIP_ERROR NOCChainGenerated(ByteSpan noc, ByteSpan icac, ByteSpan rcac, Crypto::IdentityProtectionKeySpan ipk,
                                 NodeId adminSubject)
    {
        return mCommissioner->NOCChainGenerated(noc, icac, rcac, ipk, adminSubject);
    }

    void ResetNetworkAttemptType() { mCommissioner->ResetNetworkAttemptType(); }

    void SetBreadcrumb(uint64_t value) { mCommissioner->mDeviceCommissioningInfo.general.breadcrumb = value; }

    void SetCommissioner(Controller::DeviceCommissioner * commissioner) { mCommissioner->mCommissioner = commissioner; }

    void SetCommissioneeDeviceProxy(CommissioneeDeviceProxy * proxy) { mCommissioner->mCommissioneeDeviceProxy = proxy; }

    void SetUTCRequirements(bool requiresUTC) { mCommissioner->mDeviceCommissioningInfo.requiresUTC = requiresUTC; }

    void TrySecondaryNetwork() { mCommissioner->TrySecondaryNetwork(); }

    bool TryingSecondaryNetwork() const { return mCommissioner->TryingSecondaryNetwork(); }

private:
    Controller::AutoCommissioner * mCommissioner = nullptr;
};

} // namespace Testing
} // namespace chip
