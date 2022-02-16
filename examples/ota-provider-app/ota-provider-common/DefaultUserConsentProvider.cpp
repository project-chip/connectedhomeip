/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <lib/support/logging/CHIPLogging.h>
#include <ota-provider-common/DefaultUserConsentProvider.h>

namespace chip {
namespace ota {

void DefaultUserConsentProvider::LogUserConsentSubject(const UserConsentSubject & subject)
{
    ChipLogDetail(SoftwareUpdate, "User consent request for:");
    ChipLogDetail(SoftwareUpdate, ":  FabricIndex: %u", subject.fabricIndex);
    ChipLogDetail(SoftwareUpdate, ":  RequestorNodeId: " ChipLogFormatX64, ChipLogValueX64(subject.requestorNodeId));
    ChipLogDetail(SoftwareUpdate, ":  ProviderEndpointId: %" PRIu16, subject.providerEndpointId);
    ChipLogDetail(SoftwareUpdate, ":  RequestorVendorId: %" PRIu16, subject.requestorVendorId);
    ChipLogDetail(SoftwareUpdate, ":  RequestorProductId: %" PRIu16, subject.requestorProductId);
    ChipLogDetail(SoftwareUpdate, ":  RequestorCurrentVersion: %" PRIu32, subject.requestorCurrentVersion);
    ChipLogDetail(SoftwareUpdate, ":  RequestorTargetVersion: %" PRIu32, subject.requestorTargetVersion);
    ChipLogDetail(SoftwareUpdate, ":  Metadata:");
    ChipLogByteSpan(SoftwareUpdate, subject.metadata);
}

UserConsentState DefaultUserConsentProvider::GetUserConsentState(const UserConsentSubject & subject)
{
    LogUserConsentSubject(subject);

    if (mUseGlobalConsent)
    {
        return mGlobalConsentState;
    }

    return UserConsentState::kGranted;
}

} // namespace ota
} // namespace chip
