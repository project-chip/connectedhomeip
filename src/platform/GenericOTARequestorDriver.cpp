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

#include "GenericOTARequestorDriver.h"

#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>
#include <platform/OTARequestorInterface.h>

// This implementation of the OTARequestorDriver operates according the following rules:
// - Only a single timer can be active at any given moment
// - The default provider timer is running if and only if there is no update in progress (the OTARequestor
//   UpdateState is kIdle)
// - AnnounceOTAProviders command is ignored if an update is in progress
// - The provider location passed in AnnounceOTAProviders is used in a single query (possibly retried) and then discarded
// - Explicitly triggering a query through TriggerImmediateQuery() or SendQuery() cancels any in-progress update

namespace chip {
namespace DeviceLayer {
namespace {

constexpr uint32_t kImmediateStartDelaySec = 1; // Start the timer with this value when starting OTA "immediately"

using namespace app::Clusters::OtaSoftwareUpdateRequestor;
using namespace app::Clusters::OtaSoftwareUpdateRequestor::Structs;

GenericOTARequestorDriver * ToDriver(void * context)
{
    return static_cast<GenericOTARequestorDriver *>(context);
}

constexpr System::Clock::Seconds32 kDefaultDelayedActionTime = System::Clock::Seconds32(120);

} // namespace

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
    static_cast<GenericOTARequestorDriver *>(appState)->DriverSendQuery();
}

bool ProviderLocationsEqual(const ProviderLocation::Type & a, const ProviderLocation::Type & b)
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

void GenericOTARequestorDriver::HandleIdleState()
{
    // Default provider timer runs if and only if the OTARequestor's update state is kIdle.
    // Must (re)start the timer every time we enter the kIdle state
    StartDefaultProviderTimer();
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

    ProviderLocation::Type providerLocation;
    bool willTryAnotherQuery = false;

    switch (reason)
    {
    case UpdateNotFoundReason::UpToDate:
        willTryAnotherQuery = false;
        break;

    case UpdateNotFoundReason::Busy:
        willTryAnotherQuery = true;
        break;

    case UpdateNotFoundReason::ConnectionFailed:
    case UpdateNotFoundReason::NotAvailable:
        // IMPLEMENTATION CHOICE:
        // This implementation schedules a query only if a different provider is available
        if ((DetermineProviderLocation(providerLocation) != true) || ProviderLocationsEqual(providerLocation, mLastUsedProvider))
        {
            willTryAnotherQuery = false;
        }
        else
        {
            willTryAnotherQuery = true;
        }
        mRequestor->SetCurrentProviderLocation(providerLocation);
        mLastUsedProvider = providerLocation;
        break;

    default:
        willTryAnotherQuery = false;
        break;
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
    SystemLayer().StartTimer(std::chrono::duration_cast<System::Clock::Timeout>(delay), action, aAppState);
}

void GenericOTARequestorDriver::CancelDelayedAction(System::TimerCompleteCallback action, void * aAppState)
{
    SystemLayer().CancelTimer(action, aAppState);
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

    if (mRequestor->GetCurrentUpdateState() != OTARequestorInterface::kStateIdle)
    {
        ChipLogProgress(SoftwareUpdate, "State is not kIdle, ignoring the AnnounceOTAProviders. State: %d",
                        (int) mRequestor->GetCurrentUpdateState());
        return;
    }

    // Point the OTARequestor to the announced provider
    mRequestor->SetCurrentProviderLocation(providerLocation);
    mLastUsedProvider = providerLocation;

    ScheduleDelayedAction(System::Clock::Seconds32(secToStart), StartDelayTimerHandler, this);
}

void GenericOTARequestorDriver::DriverSendQuery()
{

    // IMPLEMENTATION CHOICE
    // In this implementation explicitly triggering a query cancels any in-progress update.
    UpdateCancelled();

    // Default provider timer only runs when there is no ongoing query/update; must stop it now.
    // ConnectToProvider() will change the state from kIdle
    StopDefaultProviderTimer();

    DeviceLayer::SystemLayer().ScheduleLambda([this] { mRequestor->ConnectToProvider(OTARequestorInterface::kQueryImage); });
}

void GenericOTARequestorDriver::DefaultProviderTimerHandler(System::Layer * systemLayer, void * appState)
{
    ChipLogProgress(SoftwareUpdate, "Default Provider timer handler is invoked");

    // Determine which provider to query next
    ProviderLocation::Type providerLocation;
    if (DetermineProviderLocation(providerLocation) != true)
    {
        StartDefaultProviderTimer();
        return;
    }

    mRequestor->SetCurrentProviderLocation(providerLocation);
    mLastUsedProvider = providerLocation;

    DriverSendQuery();
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

// Determines the next available Provider location and sets it in the OTARequestor
void GenericOTARequestorDriver::DetermineAndSetProviderLocation()
{
    ProviderLocation::Type providerLocation;
    if (DetermineProviderLocation(providerLocation) != true)
    {
        return;
    }

    mRequestor->SetCurrentProviderLocation(providerLocation);
    mLastUsedProvider = providerLocation;
}

// Returns the next available Provider location
bool GenericOTARequestorDriver::DetermineProviderLocation(ProviderLocation::Type & providerLocation)
{
    auto iterator = mRequestor->GetDefaultOTAProviderListIterator();
    while (iterator.Next())
    {
        // For now, just return the first one
        providerLocation = iterator.GetValue();
        return true;
    }

    return false;
}

} // namespace DeviceLayer
} // namespace chip
