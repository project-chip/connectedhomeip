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
// - The periodic query timer is running if and only if there is no update in progress (the core logic
//   UpdateState is kIdle)
// - AnnounceOTAProviders command is ignored if an update is in progress
// - The provider location passed in AnnounceOTAProviders is used in a single query (possibly retried) and then discarded
// - Explicitly triggering a query through TriggerImmediateQuery() cancels any in-progress update
// - A QueryImage call results in the driver iterating through the list of default OTA providers, from beginning to the end, until a
//   provider successfully transfers the OTA image. If a provider is busy, it will be retried a set number of times before moving
//   to the next available one. If all else fails, the periodic query timer is kicked off again.

#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>

#include "DefaultOTARequestorDriver.h"
#include "OTARequestorInterface.h"

namespace chip {
namespace DeviceLayer {
namespace {

using namespace app::Clusters::OtaSoftwareUpdateRequestor;
using namespace app::Clusters::OtaSoftwareUpdateRequestor::Structs;

constexpr uint8_t kMaxInvalidSessionRetries        = 1;  // Max # of query image retries to perform on invalid session error
constexpr uint32_t kDelayQueryUponCommissioningSec = 30; // Delay before sending the initial image query after commissioning
constexpr uint32_t kImmediateStartDelaySec         = 1;  // Delay before sending a query in response to UrgentUpdateAvailable
constexpr System::Clock::Seconds32 kDefaultDelayedActionTime = System::Clock::Seconds32(120);

DefaultOTARequestorDriver * ToDriver(void * context)
{
    return static_cast<DefaultOTARequestorDriver *>(context);
}

} // namespace

void DefaultOTARequestorDriver::Init(OTARequestorInterface * requestor, OTAImageProcessorInterface * processor)
{
    mRequestor                = requestor;
    mImageProcessor           = processor;
    mProviderRetryCount       = 0;
    mInvalidSessionRetryCount = 0;

    if (mImageProcessor->IsFirstImageRun())
    {
        SystemLayer().ScheduleLambda([this] {
            CHIP_ERROR error = mImageProcessor->ConfirmCurrentImage();

            if (error != CHIP_NO_ERROR)
            {
                ChipLogError(SoftwareUpdate, "Failed to confirm image: %" CHIP_ERROR_FORMAT, error.Format());
                mRequestor->Reset();
                return;
            }

            mRequestor->NotifyUpdateApplied();
        });
    }
    else if ((mRequestor->GetCurrentUpdateState() != OTAUpdateStateEnum::kIdle))
    {
        // Not running a new image for the first time but also not in the idle state may indicate there is a problem
        mRequestor->Reset();
    }
    else
    {
        // Start the first periodic query timer
        StartSelectedTimer(SelectedTimer::kPeriodicQueryTimer);
    }
}

bool DefaultOTARequestorDriver::CanConsent()
{
    return false;
}

uint16_t DefaultOTARequestorDriver::GetMaxDownloadBlockSize()
{
    return maxDownloadBlockSize;
}

void DefaultOTARequestorDriver::SetMaxDownloadBlockSize(uint16_t blockSize)
{
    maxDownloadBlockSize = blockSize;
}

void StartDelayTimerHandler(System::Layer * systemLayer, void * appState)
{
    ToDriver(appState)->SendQueryImage();
}

bool DefaultOTARequestorDriver::ProviderLocationsEqual(const ProviderLocationType & a, const ProviderLocationType & b)
{
    if ((a.fabricIndex == b.fabricIndex) && (a.providerNodeID == b.providerNodeID) && (a.endpoint == b.endpoint))
    {
        return true;
    }

    return false;
}

void DefaultOTARequestorDriver::HandleIdleStateExit()
{
    // Start watchdog timer to monitor new Query Image session
    StartSelectedTimer(SelectedTimer::kWatchdogTimer);
}

void DefaultOTARequestorDriver::HandleIdleStateEnter(IdleStateReason reason)
{
    if (reason != IdleStateReason::kInvalidSession)
    {
        mInvalidSessionRetryCount = 0;
    }

    switch (reason)
    {
    case IdleStateReason::kUnknown:
        ChipLogProgress(SoftwareUpdate, "Unknown idle state reason so set the periodic timer for a next attempt");
        StartSelectedTimer(SelectedTimer::kPeriodicQueryTimer);
        break;
    case IdleStateReason::kIdle:
        // There is no current OTA update in progress so start the periodic query timer
        StartSelectedTimer(SelectedTimer::kPeriodicQueryTimer);
        break;
    case IdleStateReason::kInvalidSession:
        if (mInvalidSessionRetryCount < kMaxInvalidSessionRetries)
        {
            // An invalid session is detected which may be temporary (such as provider being restarted)
            // so try to query the same provider again. Since the session has already been disconnected prior to
            // getting here, this new query should trigger an attempt to re-establish CASE. If that subsequently fails,
            // we conclusively know the provider is not available, and will fall into the else clause below on that attempt.
            SendQueryImage();
            mInvalidSessionRetryCount++;
        }
        else
        {
            mInvalidSessionRetryCount = 0;
            StartSelectedTimer(SelectedTimer::kPeriodicQueryTimer);
        }
        break;
    }
}

void DefaultOTARequestorDriver::DownloadUpdateTimerHandler(System::Layer * systemLayer, void * appState)
{
    DefaultOTARequestorDriver * driver = ToDriver(appState);

    VerifyOrDie(driver->mRequestor != nullptr);
    driver->mRequestor->DownloadUpdate();
}

void DefaultOTARequestorDriver::ApplyUpdateTimerHandler(System::Layer * systemLayer, void * appState)
{
    DefaultOTARequestorDriver * driver = ToDriver(appState);

    VerifyOrDie(driver->mRequestor != nullptr);
    driver->mRequestor->ApplyUpdate();
}

void DefaultOTARequestorDriver::ApplyTimerHandler(System::Layer * systemLayer, void * appState)
{
    DefaultOTARequestorDriver * driver = ToDriver(appState);

    VerifyOrDie(driver->mImageProcessor != nullptr);
    driver->mImageProcessor->Apply();
}

void DefaultOTARequestorDriver::UpdateAvailable(const UpdateDescription & update, System::Clock::Seconds32 delay)
{
    // IMPLEMENTATION CHOICE:
    // This implementation unconditionally downloads an available update

    VerifyOrDie(mRequestor != nullptr);
    ScheduleDelayedAction(delay, DownloadUpdateTimerHandler, this);
}

CHIP_ERROR DefaultOTARequestorDriver::UpdateNotFound(UpdateNotFoundReason reason, System::Clock::Seconds32 delay)
{
    CHIP_ERROR status = CHIP_NO_ERROR;

    switch (reason)
    {
    case UpdateNotFoundReason::kUpToDate:
        break;
    case UpdateNotFoundReason::kBusy: {
        status = ScheduleQueryRetry(true, chip::max(kDefaultDelayedActionTime, delay));
        if (status == CHIP_ERROR_MAX_RETRY_EXCEEDED)
        {
            // If max retry exceeded with current provider, try a different provider
            status = ScheduleQueryRetry(false, chip::max(kDefaultDelayedActionTime, delay));
        }
        break;
    }
    case UpdateNotFoundReason::kNotAvailable: {
        // Schedule a query only if a different provider is available
        status = ScheduleQueryRetry(false, chip::max(kDefaultDelayedActionTime, delay));
        break;
    }
    }
    return status;
}

void DefaultOTARequestorDriver::UpdateDownloaded()
{
    VerifyOrDie(mRequestor != nullptr);
    mRequestor->ApplyUpdate();
}

void DefaultOTARequestorDriver::UpdateConfirmed(System::Clock::Seconds32 delay)
{
    VerifyOrDie(mImageProcessor != nullptr);
    ScheduleDelayedAction(delay, ApplyTimerHandler, this);
}

void DefaultOTARequestorDriver::UpdateSuspended(System::Clock::Seconds32 delay)
{
    VerifyOrDie(mRequestor != nullptr);

    if (delay < kDefaultDelayedActionTime)
    {
        delay = kDefaultDelayedActionTime;
    }

    ScheduleDelayedAction(delay, ApplyUpdateTimerHandler, this);
}

void DefaultOTARequestorDriver::UpdateDiscontinued()
{
    VerifyOrDie(mImageProcessor != nullptr);
    mImageProcessor->Abort();

    // Cancel all update timers
    UpdateCancelled();
}

// Cancel all OTA update timers
void DefaultOTARequestorDriver::UpdateCancelled()
{
    // Cancel all OTA Update timers started by OTARequestorDriver regardless of whether thery are running or not
    CancelDelayedAction(DownloadUpdateTimerHandler, this);
    CancelDelayedAction(StartDelayTimerHandler, this);
    CancelDelayedAction(ApplyTimerHandler, this);
    CancelDelayedAction(ApplyUpdateTimerHandler, this);
}

void DefaultOTARequestorDriver::ScheduleDelayedAction(System::Clock::Seconds32 delay, System::TimerCompleteCallback action,
                                                      void * aAppState)
{
    VerifyOrDie(SystemLayer().StartTimer(std::chrono::duration_cast<System::Clock::Timeout>(delay), action, aAppState) ==
                CHIP_NO_ERROR);
}

void DefaultOTARequestorDriver::CancelDelayedAction(System::TimerCompleteCallback action, void * aAppState)
{
    SystemLayer().CancelTimer(action, aAppState);
}

// Device commissioning has completed, schedule a provider query
void DefaultOTARequestorDriver::OTACommissioningCallback()
{
    // Schedule a query. At the end of this query/update process the Default Provider timer is started
    ScheduleDelayedAction(System::Clock::Seconds32(kDelayQueryUponCommissioningSec), StartDelayTimerHandler, this);
}

void DefaultOTARequestorDriver::ProcessAnnounceOTAProviders(
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

    // Point to the announced provider
    mRequestor->SetCurrentProviderLocation(providerLocation);

    ScheduleDelayedAction(System::Clock::Seconds32(secToStart), StartDelayTimerHandler, this);
}

void DefaultOTARequestorDriver::SendQueryImage()
{
    OTAUpdateStateEnum currentUpdateState;
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

    currentUpdateState = mRequestor->GetCurrentUpdateState();
    if ((currentUpdateState == OTAUpdateStateEnum::kIdle) || (currentUpdateState == OTAUpdateStateEnum::kDelayedOnQuery))
    {
        mProviderRetryCount++;
        DeviceLayer::SystemLayer().ScheduleLambda([this] { mRequestor->TriggerImmediateQueryInternal(); });
    }
    else
    {
        ChipLogProgress(SoftwareUpdate, "Query already in progress");
    }
}

void DefaultOTARequestorDriver::PeriodicQueryTimerHandler(System::Layer * systemLayer, void * appState)
{
    ChipLogProgress(SoftwareUpdate, "Default Provider timer handler is invoked");

    DefaultOTARequestorDriver * driver = ToDriver(appState);

    // Determine which provider to query next
    ProviderLocationType providerLocation;
    bool listExhausted = false;
    if (driver->GetNextProviderLocation(providerLocation, listExhausted) != true)
    {
        driver->StartSelectedTimer(SelectedTimer::kPeriodicQueryTimer);
        return;
    }

    driver->mRequestor->SetCurrentProviderLocation(providerLocation);

    driver->SendQueryImage();
}

void DefaultOTARequestorDriver::StartPeriodicQueryTimer()
{
    ChipLogProgress(SoftwareUpdate, "Starting the periodic query timer, timeout: %u seconds",
                    (unsigned int) mPeriodicQueryTimeInterval);
    ScheduleDelayedAction(System::Clock::Seconds32(mPeriodicQueryTimeInterval), PeriodicQueryTimerHandler, this);
}

void DefaultOTARequestorDriver::StopPeriodicQueryTimer()
{
    ChipLogProgress(SoftwareUpdate, "Stopping the Periodic Query timer");
    CancelDelayedAction(PeriodicQueryTimerHandler, this);
}

void DefaultOTARequestorDriver::RekickPeriodicQueryTimer(void)
{
    ChipLogProgress(SoftwareUpdate, "Rekicking the Periodic Query timer");
    StopPeriodicQueryTimer();
    StartPeriodicQueryTimer();
}

void DefaultOTARequestorDriver::WatchdogTimerHandler(System::Layer * systemLayer, void * appState)
{
    DefaultOTARequestorDriver * driver = ToDriver(appState);

    ChipLogError(SoftwareUpdate, "Watchdog timer detects state stuck at %u. Cancelling download and resetting state.",
                 to_underlying(driver->mRequestor->GetCurrentUpdateState()));

    // Something went wrong and OTA requestor is stuck in a non-idle state for too long.
    // Let's just cancel download, reset state, and re-start periodic query timer.
    driver->UpdateDiscontinued();
    driver->mRequestor->CancelImageUpdate();
    driver->StartPeriodicQueryTimer();
}

void DefaultOTARequestorDriver::StartWatchdogTimer()
{
    ChipLogProgress(SoftwareUpdate, "Starting the watchdog timer, timeout: %u seconds", (unsigned int) mWatchdogTimeInterval);
    ScheduleDelayedAction(System::Clock::Seconds32(mWatchdogTimeInterval), WatchdogTimerHandler, this);
}

void DefaultOTARequestorDriver::StopWatchdogTimer()
{
    ChipLogProgress(SoftwareUpdate, "Stopping the watchdog timer");
    CancelDelayedAction(WatchdogTimerHandler, this);
}

void DefaultOTARequestorDriver::StartSelectedTimer(SelectedTimer timer)
{
    switch (timer)
    {
    case SelectedTimer::kPeriodicQueryTimer:
        StopWatchdogTimer();
        StartPeriodicQueryTimer();
        break;
    case SelectedTimer::kWatchdogTimer:
        StopPeriodicQueryTimer();
        StartWatchdogTimer();
        break;
    }
}

/**
 * Returns the next available Provider location. The algorithm is to simply loop through the list of DefaultOtaProviders as a
 * circular list and return the next value (based on the last used provider). If the list of DefaultOtaProviders is empty, FALSE is
 * returned.
 */
bool DefaultOTARequestorDriver::GetNextProviderLocation(ProviderLocationType & providerLocation, bool & listExhausted)
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

    ChipLogError(SoftwareUpdate, "No suitable OTA Provider candidate found");
    return false;
}

CHIP_ERROR DefaultOTARequestorDriver::ScheduleQueryRetry(bool trySameProvider, System::Clock::Seconds32 delay)
{
    CHIP_ERROR status = CHIP_NO_ERROR;

    if (trySameProvider == false)
    {
        VerifyOrDie(mRequestor != nullptr);

        ProviderLocationType providerLocation;
        bool listExhausted = false;

        // Note that the "listExhausted" being set to TRUE, implies that the entire list of
        // defaultOTAProviders has been traversed. On bootup, the last provider is reset
        // which ensures that every QueryImage call will ensure that the list is traversed from
        // start to end, until an OTA is successfully completed.
        if ((GetNextProviderLocation(providerLocation, listExhausted) != true) || (listExhausted == true))
        {
            status = CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        else
        {
            mRequestor->SetCurrentProviderLocation(providerLocation);
        }
    }

    if (mProviderRetryCount > kMaxBusyProviderRetryCount)
    {
        ChipLogProgress(SoftwareUpdate, "Max retry of %u exceeded.  Will not retry", kMaxBusyProviderRetryCount);
        status = CHIP_ERROR_MAX_RETRY_EXCEEDED;
    }

    if (status == CHIP_NO_ERROR)
    {
        ChipLogProgress(SoftwareUpdate, "Scheduling a retry");
        ScheduleDelayedAction(delay, StartDelayTimerHandler, this);
    }

    return status;
}

} // namespace DeviceLayer
} // namespace chip
