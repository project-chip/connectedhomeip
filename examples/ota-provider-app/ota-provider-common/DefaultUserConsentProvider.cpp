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

DefaultUserConsentProvider::DefaultUserConsentProvider()
{
    for (uint8_t i = 0; i < kMaxUserConsentEntries; i++)
    {
        mUserConsentEntries[i].isEntryValid = false;
    }
}

void DefaultUserConsentProvider::LogUserConsentSubject(const UserConsentSubject & subject)
{
    ChipLogDetail(SoftwareUpdate, "User consent request for:");
    ChipLogDetail(SoftwareUpdate, ":  FabricId: " ChipLogFormatX64, ChipLogValueX64(subject.fabricId));
    ChipLogDetail(SoftwareUpdate, ":  NodeId: " ChipLogFormatX64, ChipLogValueX64(subject.nodeId));
    ChipLogDetail(SoftwareUpdate, ":  EndpointId: %" PRIu16, subject.endpointId);
    ChipLogDetail(SoftwareUpdate, ":  VendorId: %" PRIu16, subject.vendorId);
    ChipLogDetail(SoftwareUpdate, ":  ProductId: %" PRIu16, subject.productId);
    ChipLogDetail(SoftwareUpdate, ":  CurrentVersion: %" PRIu32, subject.currentVersion);
    ChipLogDetail(SoftwareUpdate, ":  NewVersion: %" PRIu32, subject.targetVersion);
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

    for (uint8_t i = 0; i < kMaxUserConsentEntries; i++)
    {
        if (mUserConsentEntries[i].isEntryValid && mUserConsentEntries[i].subject == subject)
        {
            return mUserConsentEntries[i].state;
        }
    }
    return UserConsentState::kGranted;
}

CHIP_ERROR DefaultUserConsentProvider::SetUserConsentState(const UserConsentSubject & subject, UserConsentState state)
{
    for (uint8_t i = 0; i < kMaxUserConsentEntries; i++)
    {
        if (mUserConsentEntries[i].isEntryValid && mUserConsentEntries[i].subject == subject)
        {
            mUserConsentEntries[i].state = state;
            return CHIP_NO_ERROR;
        }
    }

    for (uint8_t i = 0; i < kMaxUserConsentEntries; i++)
    {
        if (mUserConsentEntries[i].isEntryValid == false)
        {
            mUserConsentEntries[i].subject      = subject;
            mUserConsentEntries[i].state        = state;
            mUserConsentEntries[i].isEntryValid = true;
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR DefaultUserConsentProvider::GrantUserConsent(const UserConsentSubject & subject)
{
    return SetUserConsentState(subject, UserConsentState::kGranted);
}

CHIP_ERROR DefaultUserConsentProvider::RevokeUserConsent(const UserConsentSubject & subject)
{
    return SetUserConsentState(subject, UserConsentState::kDenied);
}

CHIP_ERROR DefaultUserConsentProvider::DeferUserConsent(const UserConsentSubject & subject)
{
    return SetUserConsentState(subject, UserConsentState::kObtaining);
}

CHIP_ERROR DefaultUserConsentProvider::ClearUserConsentEntry(const UserConsentSubject & subject)
{
    for (uint8_t i = 0; i < kMaxUserConsentEntries; i++)
    {
        if (mUserConsentEntries[i].isEntryValid && mUserConsentEntries[i].subject == subject)
        {
            mUserConsentEntries[i].isEntryValid = false;
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_ERROR_NOT_FOUND;
}

} // namespace ota
} // namespace chip
