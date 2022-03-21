/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

// This file contains an implementation of the OTARequestorDriver interface class.
// Individual platforms may supply their own implementations of OTARequestorDriver
// or use this one. There are no requirements or assumptions on the implementation other
// than adherence to the OTA Image Update Requestor part of the Matter specification; the
// aspects of the functionality not mandated by the specification are considered implementation choices.
//
// This particular implementation of the OTARequestorDriver makes the following choices:
// - Only a single timer can be active at any given moment
// - The periodic query timer is running if and only if there is no update in progress (the OTARequestor
//   UpdateState is kIdle)
// - AnnounceOTAProviders command is ignored if an update is in progress
// - The provider location passed in AnnounceOTAProviders is used in a single query (possibly retried) and then discarded
// - Explicitly triggering a query through TriggerImmediateQuery() cancels any in-progress update
// - A QueryImage call results in the driver iterating through the list of default OTA providers, from beginning to the end, until a
//   provider successfully transfers the OTA image. If a provider is busy, it will be retried a set number of times before moving
//   to the next available one. If all else fails, the periodic query timer is kicked off again.

#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>

#include "GenericOTARequestorDriver.h"
#include "OTARequestorInterface.h"

namespace chip {
namespace DeviceLayer {
namespace {

using namespace app::Clusters::OtaSoftwareUpdateRequestor;
using namespace app::Clusters::OtaSoftwareUpdateRequestor::Structs;

constexpr uint32_t kDelayQueryUponCommissioningSec = 30; // Delay before sending the initial image query after commissioning
constexpr uint32_t kImmediateStartDelaySec         = 1;  // Delay before sending a query in response to UrgentUpdateAvailable
constexpr System::Clock::Seconds32 kDefaultDelayedActionTime = System::Clock::Seconds32(120);

GenericOTARequestorDriver * ToDriver(void * context)
{
    return static_cast<GenericOTARequestorDriver *>(context);
}

} // namespace

void GenericOTARequestorDriver::Init(OTARequestorInterface * requestor, OTAImageProcessorInterface * processor)
{
    mRequestor          = requestor;
    mImageProcessor     = processor;
    mProviderRetryCount = 0;

    if (mImageProcessor->IsFirstImageRun())
    {
        SystemLayer().ScheduleLambda([this] {
            CHIP_ERROR error = mImageProcessor->ConfirmCurrentImage();

            if (error != CHIP_NO_ERROR)
            {
                ChipLogError(SoftwareUpdate, "Failed to confirm image: %" CHIP_ERROR_FORMAT, error.Format());
                return;
            }

            mRequestor->NotifyUpdateApplied();
        });
    }
}

bool GenericOTARequestorDriver::CanConsent()
{
    return false;
}

uint16_t GenericOTARequestorDriver::GetMaxDownloadBlockSize()
{
    return 1024;
}

void StartDelayTimerHandler(System::Layer * systemLayer, void * appState)
{
    static_cast<GenericOTARequestorDriver *>(appState)->SendQueryImage();
}

bool GenericOTARequestorDriver::ProviderLocationsEqual(const ProviderLocationType & a, const ProviderLocationType & b)
{
    if ((a.fabricIndex == b.fabricIndex) && (a.providerNodeID == b.providerNodeID) && (a.endpoint == b.endpoint))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void GenericOTARequestorDriver::HandleError(UpdateFailureState state, CHIP_ERROR error) {}

void GenericOTARequestorDriver::HandleIdleState(IdleStateReason reason)
{
    switch (reason)
    {
    case IdleStateReason::kUnknown:
        ChipLogProgress(SoftwareUpdate, "Unknown idle state reason so set the periodic timer for a next attempt");
        StartDefaultProviderTimer();
        break;
    case IdleStateReason::kIdle:
        // There is no current OTA update in progress so start the periodic query timer
        StartDefaultProviderTimer();
        break;
    case IdleStateReason::kInvalidSession:
        // An invalid session is detected which may be temporary so try to query the same provider again
        SendQueryImage();
        break;
    }
}

void GenericOTARequestorDriver::UpdateAvailable(const UpdateDescription & update, System::Clock::Seconds32 delay)
{
    // IMPLEMENTATION CHOICE:
    // This implementation unconditionally downloads an available update

    VerifyOrDie(mRequestor != nullptr);
    ScheduleDelayedAction(
        delay, [](System::Layer *, void * context) { ToDriver(context)->mRequestor->DownloadUpdate(); }, this);
}

void GenericOTARequestorDriver::UpdateNotFound(UpdateNotFoundReason reason, System::Clock::Seconds32 delay)
{
    VerifyOrDie(mRequestor != nullptr);

    ProviderLocationType providerLocation;
    bool willTryAnotherQuery = false;

    switch (reason)
    {
    case UpdateNotFoundReason::kUpToDate:
        willTryAnotherQuery = false;
        break;
    case UpdateNotFoundReason::kBusy:
        willTryAnotherQuery = true;
        if (mProviderRetryCount <= kMaxBusyProviderRetryCount)
        {
            break;
        }
        ChipLogProgress(SoftwareUpdate, "Max Busy Provider retries reached. Attempting to get next Provider.");
        __attribute__((fallthrough)); // fallthrough
    case UpdateNotFoundReason::kNotAvailable: {
        // IMPLEMENTATION CHOICE:
        // This implementation schedules a query only if a different provider is available
        // Note that the "listExhausted" being set to TRUE, implies that the entire list of
        // defaultOTAProviders has been traversed. On bootup, the last provider is reset
        // which ensures that every QueryImage call will ensure that the list is traversed from
        // start to end, until an OTA is successfully completed.
        bool listExhausted = false;
        if ((GetNextProviderLocation(providerLocation, listExhausted) != true) || (listExhausted == true))
        {
            willTryAnotherQuery = false;
        }
        else
        {
            willTryAnotherQuery = true;
            mRequestor->SetCurrentProviderLocation(providerLocation);
        }
        break;
    }
    }

    if (delay < kDefaultDelayedActionTime)
    {
        delay = kDefaultDelayedActionTime;
    }

    if (willTryAnotherQuery == true)
    {
        ChipLogProgress(SoftwareUpdate, "UpdateNotFound, scheduling a retry");
        ScheduleDelayedAction(delay, StartDelayTimerHandler, this);
    }
    else
    {
        ChipLogProgress(SoftwareUpdate, "UpdateNotFound, not scheduling further retries");
        StartDefaultProviderTimer();
    }
}

void GenericOTARequestorDriver::UpdateDownloaded()
{
    VerifyOrDie(mRequestor != nullptr);
    mRequestor->ApplyUpdate();
}

void GenericOTARequestorDriver::UpdateConfirmed(System::Clock::Seconds32 delay)
{
    VerifyOrDie(mImageProcessor != nullptr);
    ScheduleDelayedAction(
        delay, [](System::Layer *, void * context) { ToDriver(context)->mImageProcessor->Apply(); }, this);
}

void GenericOTARequestorDriver::UpdateSuspended(System::Clock::Seconds32 delay)
{
    VerifyOrDie(mRequestor != nullptr);

    if (delay < kDefaultDelayedActionTime)
    {
        delay = kDefaultDelayedActionTime;
    }

    ScheduleDelayedAction(
        delay, [](System::Layer *, void * context) { ToDriver(context)->mRequestor->ApplyUpdate(); }, this);
}

void GenericOTARequestorDriver::UpdateDiscontinued()
{
    VerifyOrDie(mImageProcessor != nullptr);
    mImageProcessor->Abort();

    // Cancel all update timers
    UpdateCancelled();

    // Restart the periodic default provider timer
    StartDefaultProviderTimer();
}

// Cancel all OTA update timers
void GenericOTARequestorDriver::UpdateCancelled()
{
    // Cancel all OTA Update timers started by  OTARequestorDriver regardless of whether thery are running or not
    CancelDelayedAction([](System::Layer *, void * context) { ToDriver(context)->mRequestor->DownloadUpdate(); }, this);
    CancelDelayedAction(StartDelayTimerHandler, this);
    CancelDelayedAction([](System::Layer *, void * context) { ToDriver(context)->mImageProcessor->Apply(); }, this);
    CancelDelayedAction([](System::Layer *, void * context) { ToDriver(context)->mRequestor->ApplyUpdate(); }, this);
}

void GenericOTARequestorDriver::ScheduleDelayedAction(System::Clock::Seconds32 delay, System::TimerCompleteCallback action,
                                                      void * aAppState)
{
    VerifyOrDie(SystemLayer().StartTimer(std::chrono::duration_cast<System::Clock::Timeout>(delay), action, aAppState) ==
                CHIP_NO_ERROR);
}

void GenericOTARequestorDriver::CancelDelayedAction(System::TimerCompleteCallback action, void * aAppState)
{
    SystemLayer().CancelTimer(action, aAppState);
}

// Device commissioning has completed, schedule a provider query
void GenericOTARequestorDriver::OTACommissioningCallback()
{
    // Schedule a query. At the end of this query/update process the Default Provider timer is started
    ScheduleDelayedAction(
        System::Clock::Seconds32(kDelayQueryUponCommissioningSec),
        [](System::Layer *, void * context) { static_cast<OTARequestorDriver *>(context)->SendQueryImage(); }, this);
}

void GenericOTARequestorDriver::ProcessAnnounceOTAProviders(
    const ProviderLocationType & providerLocation,
    app::Clusters::OtaSoftwareUpdateRequestor::OTAAnnouncementReason announcementReason)
{
    // If reason is URGENT_UPDATE_AVAILABLE, we start OTA immediately. Otherwise, respect the timer value set in mOtaStartDelaySec.
    // This is done to exemplify what a real-world OTA Requestor might do while also being configurable enough to use as a test app.
    uint32_t secToStart = 0;
    switch (announcementReason)
    {
    case OTAAnnouncementReason::kSimpleAnnouncement:
    case OTAAnnouncementReason::kUpdateAvailable:
        secToStart = mOtaStartDelaySec;
        break;
    case OTAAnnouncementReason::kUrgentUpdateAvailable:
        // TODO: Implement random delay per spec
        secToStart = kImmediateStartDelaySec;
        break;
    default:
        ChipLogError(SoftwareUpdate, "Unexpected announcementReason: %u", static_cast<uint8_t>(announcementReason));
        return;
    }

    // IMPLEMENTATION CHOICE:
    // This implementation of the OTARequestor driver ignores the announcement if an update is in progress,
    // otherwise it queries the provider passed in the announcement

    if (mRequestor->GetCurrentUpdateState() != OTAUpdateStateEnum::kIdle)
    {
        ChipLogProgress(SoftwareUpdate, "State is not kIdle, ignoring the AnnounceOTAProviders. State: %d",
                        (int) mRequestor->GetCurrentUpdateState());
        return;
    }

    // Point the OTARequestor to the announced provider
    mRequestor->SetCurrentProviderLocation(providerLocation);

    ScheduleDelayedAction(System::Clock::Seconds32(secToStart), StartDelayTimerHandler, this);
}

void GenericOTARequestorDriver::SendQueryImage()
{
    Optional<ProviderLocationType> lastUsedProvider;
    mRequestor->GetProviderLocation(lastUsedProvider);
    if (!lastUsedProvider.HasValue())
    {
        ProviderLocationType providerLocation;
        bool listExhausted = false;
        if (GetNextProviderLocation(providerLocation, listExhausted) == true)
        {
            mRequestor->SetCurrentProviderLocation(providerLocation);
        }
        else
        {
            ChipLogProgress(SoftwareUpdate, "No provider available");
            return;
        }
    }
    // IMPLEMENTATION CHOICE
    // In this implementation explicitly triggering a query cancels any in-progress update.
    UpdateCancelled();

    // Default provider timer only runs when there is no ongoing query/update; must stop it now.
    // TriggerImmediateQueryInternal() will cause the state to change from kIdle
    StopDefaultProviderTimer();

    mProviderRetryCount++;

    DeviceLayer::SystemLayer().ScheduleLambda([this] { mRequestor->TriggerImmediateQueryInternal(); });
}

void GenericOTARequestorDriver::DefaultProviderTimerHandler(System::Layer * systemLayer, void * appState)
{
    ChipLogProgress(SoftwareUpdate, "Default Provider timer handler is invoked");

    // Determine which provider to query next
    ProviderLocationType providerLocation;
    bool listExhausted = false;
    if (GetNextProviderLocation(providerLocation, listExhausted) != true)
    {
        StartDefaultProviderTimer();
        return;
    }

    mRequestor->SetCurrentProviderLocation(providerLocation);

    SendQueryImage();
}

void GenericOTARequestorDriver::StartDefaultProviderTimer()
{
    ChipLogProgress(SoftwareUpdate, "Starting the Default Provider timer, timeout: %u seconds",
                    (unsigned int) mPeriodicQueryTimeInterval);
    ScheduleDelayedAction(
        System::Clock::Seconds32(mPeriodicQueryTimeInterval),
        [](System::Layer *, void * context) {
            (static_cast<GenericOTARequestorDriver *>(context))->DefaultProviderTimerHandler(nullptr, context);
        },
        this);
}

void GenericOTARequestorDriver::StopDefaultProviderTimer()
{
    ChipLogProgress(SoftwareUpdate, "Stopping the Default Provider timer");
    CancelDelayedAction(
        [](System::Layer *, void * context) {
            (static_cast<GenericOTARequestorDriver *>(context))->DefaultProviderTimerHandler(nullptr, context);
        },
        this);
}

/**
 * Returns the next available Provider location. The algorithm is to simply loop through the list of DefaultOtaProviders as a
 * circular list and return the next value (based on the last used provider). If the list of DefaultOtaProviders is empty, FALSE is
 * returned.
 */
bool GenericOTARequestorDriver::GetNextProviderLocation(ProviderLocationType & providerLocation, bool & listExhausted)
{
    Optional<ProviderLocationType> lastUsedProvider;
    mRequestor->GetProviderLocation(lastUsedProvider);
    mProviderRetryCount = 0; // Reset provider retry count
    listExhausted       = false;

    // Iterate through the default providers list and find the last used provider. If found, return the provider after it
    auto iterator = mRequestor->GetDefaultOTAProviderListIterator();
    while (lastUsedProvider.HasValue() && iterator.Next())
    {
        if (ProviderLocationsEqual(iterator.GetValue(), lastUsedProvider.Value()))
        {
            if (iterator.Next())
            {
                providerLocation = iterator.GetValue();
                return true;
            }
        }
    }

    // If no suitable candidate found, return the first element of the default providers list or an error
    iterator = mRequestor->GetDefaultOTAProviderListIterator();
    if (iterator.Next())
    {
        providerLocation = iterator.GetValue();
        listExhausted    = true;
        return true;
    }
    else
    {
        ChipLogError(SoftwareUpdate, "No suitable OTA Provider candidate found");
        return false;
    }
    return false;
}

} // namespace DeviceLayer
} // namespace chip
