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
    void SetUTCRequirements(bool requiresUTC) { mCommissioner->mDeviceCommissioningInfo.requiresUTC = requiresUTC; }

private:
    Controller::AutoCommissioner * mCommissioner = nullptr;
};

} // namespace Test
} // namespace chip
