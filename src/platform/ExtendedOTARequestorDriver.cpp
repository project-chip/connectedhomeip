/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <platform/ExtendedOTARequestorDriver.h>

namespace chip {
namespace DeviceLayer {

namespace {

constexpr System::Clock::Seconds32 kUserConsentPollInterval = System::Clock::Seconds32(30);

} // namespace

bool ExtendedOTARequestorDriver::CanConsent()
{
    bool localConfigDisabled = false;
    VerifyOrdo(DeviceLayer::ConfigurationMgr().GetLocalConfigDisabled(localConfigDisabled) == CHIP_NO_ERROR,
               ChipLogProgress(SoftwareUpdate, "Failed to get local config disabled, using as false"));

    // If local config is disabled, we can't consent.
    return localConfigDisabled == false;
}

void ExtendedOTARequestorDriver::UpdateAvailable(const UpdateDescription & update, System::Clock::Seconds32 delay)
{
    VerifyOrDie(mRequestor != nullptr);

    if (update.userConsentNeeded == true && mUserConsentDelegate)
    {
        chip::ota::UserConsentSubject subject;
        CHIP_ERROR err = GetUserConsentSubject(subject, update);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Failed to get user consent subject");
            HandleError(UpdateFailureState::kDelayedOnUserConsent, err);
            return;
        }

        mDelayedActionTime = delay;
        HandleUserConsentState(mUserConsentDelegate->GetUserConsentState(subject));
        return;
    }

    GenericOTARequestorDriver::UpdateAvailable(update, delay);
}

void ExtendedOTARequestorDriver::PollUserConsentState()
{
    HandleUserConsentState(mUserConsentDelegate->CheckDeferredUserConsentState());
}

CHIP_ERROR ExtendedOTARequestorDriver::GetUserConsentSubject(chip::ota::UserConsentSubject & subject,
                                                             const UpdateDescription & update)
{
    // mLastUsedProvider has the provider fabric index and endpoint id
    subject.fabricIndex        = mLastUsedProvider.fabricIndex;
    subject.providerEndpointId = mLastUsedProvider.endpoint;

    // TODO: As we cannot use the src/app/Server.h in here so, figure out a way to get the node id.

    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetVendorId(subject.requestorVendorId));
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetProductId(subject.requestorProductId));
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSoftwareVersion(subject.requestorCurrentVersion));
    subject.requestorTargetVersion = update.softwareVersion;
    subject.metadata               = update.metadataForRequestor;

    return CHIP_NO_ERROR;
}

void ExtendedOTARequestorDriver::HandleUserConsentState(chip::ota::UserConsentState userConsentState)
{
    ChipLogDetail(SoftwareUpdate, "User consent state: %s", mUserConsentDelegate->UserConsentStateToString(userConsentState));

    switch (userConsentState)
    {
    case chip::ota::UserConsentState::kGranted:
        ScheduleDelayedAction(
            mDelayedActionTime,
            [](System::Layer *, void * context) {
                static_cast<ExtendedOTARequestorDriver *>(context)->mRequestor->DownloadUpdate();
            },
            this);
        break;

    case chip::ota::UserConsentState::kDenied:
        ScheduleDelayedAction(
            mDelayedActionTime,
            [](System::Layer *, void * context) {
                static_cast<ExtendedOTARequestorDriver *>(context)->mRequestor->CancelImageUpdate();
            },
            this);
        break;

    case chip::ota::UserConsentState::kObtaining:
        SystemLayer().ScheduleWork(
            [](System::Layer *, void * context) {
                static_cast<ExtendedOTARequestorDriver *>(context)->mRequestor->DownloadUpdateDelayedOnUserConsent();
            },
            this);

        ScheduleDelayedAction(
            kUserConsentPollInterval,
            [](System::Layer *, void * context) { static_cast<ExtendedOTARequestorDriver *>(context)->PollUserConsentState(); },
            this);
        break;

    default:
        break;
    }
}

} // namespace DeviceLayer
} // namespace chip
