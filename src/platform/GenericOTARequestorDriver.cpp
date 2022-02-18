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

namespace chip {

extern void StartDelayTimerHandler(System::Layer * systemLayer, void * appState);

namespace DeviceLayer {
namespace {

constexpr uint32_t kImmediateStartDelayMs = 1; // Start the timer with this value when starting OTA "immediately"

using namespace app::Clusters::OtaSoftwareUpdateRequestor;

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


void GenericOTARequestorDriver::HandleError(UpdateFailureState state, CHIP_ERROR error)
{
    // Any error encountered by the OTARequestor results a call to this function. The OTARequestor enters the 
    // kIdle state, the default providers timer must be started

    StartDefaultProvidersTimer();
}

void GenericOTARequestorDriver::UpdateAvailable(const UpdateDescription & update, System::Clock::Seconds32 delay)
{
    VerifyOrDie(mRequestor != nullptr);
    ScheduleDelayedAction(UpdateFailureState::kDownloading, delay,
                          [](System::Layer *, void * context) { ToDriver(context)->mRequestor->DownloadUpdate(); }, this);
}

void GenericOTARequestorDriver::UpdateNotFound(UpdateNotFoundReason reason, System::Clock::Seconds32 delay)
{
    VerifyOrDie(mRequestor != nullptr);

    if (delay < kDefaultDelayedActionTime)
    {
        delay = kDefaultDelayedActionTime;
    }

    // IMPLEMENTATION CHOICE:
    // This implementation chooses to schedule another query with the same provider

    // TODO: Should keep count of retries and stop after 3. 
    // SL TODO: Add logic to select a different provider. 


    ScheduleDelayedAction(UpdateFailureState::kQuerying, delay,
                          [](System::Layer *, void * context) { ToDriver(context)->mRequestor->ConnectToProvider(OTARequestorInterface::kQueryImage); }, this);
}

void GenericOTARequestorDriver::UpdateDownloaded()
{
    VerifyOrDie(mRequestor != nullptr);
    mRequestor->ApplyUpdate();
}

void GenericOTARequestorDriver::UpdateConfirmed(System::Clock::Seconds32 delay)
{
    VerifyOrDie(mImageProcessor != nullptr);
    ScheduleDelayedAction(UpdateFailureState::kApplying, delay,
                          [](System::Layer *, void * context) { ToDriver(context)->mImageProcessor->Apply(); }, this);
}

void GenericOTARequestorDriver::UpdateSuspended(System::Clock::Seconds32 delay)
{
    VerifyOrDie(mRequestor != nullptr);

    if (delay < kDefaultDelayedActionTime)
    {
        delay = kDefaultDelayedActionTime;
    }

    ScheduleDelayedAction(UpdateFailureState::kAwaitingNextAction, delay,
                          [](System::Layer *, void * context) { ToDriver(context)->mRequestor->ApplyUpdate(); }, this);
}

void GenericOTARequestorDriver::UpdateDiscontinued()
{
    VerifyOrDie(mImageProcessor != nullptr);
    mImageProcessor->Abort();

    // Cancel all update timers
    UpdateCancelled();

    // Restart the periodic default providers timer
    StartDefaultProvidersTimer();
}

// Cancel all OTA update timers 
void GenericOTARequestorDriver::UpdateCancelled()
{
    // Cancel all OTA Update timers started by  OTARequestorDriver regardless of whether thery are running or not
    CancelDelayedAction([](System::Layer *, void * context) { ToDriver(context)->mRequestor->DownloadUpdate(); }, this);
    CancelDelayedAction([](System::Layer *, void * context) { ToDriver(context)->mRequestor->TriggerImmediateQuery(); }, this);
    CancelDelayedAction([](System::Layer *, void * context) { ToDriver(context)->mImageProcessor->Apply(); }, this);
    CancelDelayedAction([](System::Layer *, void * context) { ToDriver(context)->mRequestor->ApplyUpdate(); }, this);
}

void GenericOTARequestorDriver::ScheduleDelayedAction(UpdateFailureState state, System::Clock::Seconds32 delay,
                                                      System::TimerCompleteCallback action, void * aAppState)
{
    CHIP_ERROR error = SystemLayer().StartTimer(std::chrono::duration_cast<System::Clock::Timeout>(delay), action, aAppState);

    if (error != CHIP_NO_ERROR)
    {
        HandleError(state, error);
    }
}

void GenericOTARequestorDriver::CancelDelayedAction(System::TimerCompleteCallback action, void * aAppState)
{
    SystemLayer().CancelTimer(action, aAppState);
}


// void StartDelayTimerHandler(System::Layer * systemLayer, void * appState)
// {
//     ChipLogDetail(SoftwareUpdate, "LISS StartDelayTimerHandler appState %p", appState);

//     VerifyOrReturn(appState != nullptr);

//  ChipLogDetail(SoftwareUpdate, "LISS StartDelayTimerHandler appState %p", appState);

//     static_cast<GenericOTARequestorDriver *>(appState)->mRequestor->ConnectToProvider(OTARequestorInterface::kQueryImage);
// }


void GenericOTARequestorDriver::ProcessAnnounceOTAProviders(const ProviderLocationType &providerLocation, 
                                                          app::Clusters::OtaSoftwareUpdateRequestor::OTAAnnouncementReason announcementReason)
{
    // If reason is URGENT_UPDATE_AVAILABLE, we start OTA immediately. Otherwise, respect the timer value set in mOtaStartDelayMs.
    // This is done to exemplify what a real-world OTA Requestor might do while also being configurable enough to use as a test app.
    uint32_t msToStart = 0;
    switch (announcementReason)
    {
    case OTAAnnouncementReason::kSimpleAnnouncement:
    case OTAAnnouncementReason::kUpdateAvailable:
        msToStart = mOtaStartDelayMs;
        break;
    case OTAAnnouncementReason::kUrgentUpdateAvailable:
        msToStart = kImmediateStartDelayMs;
        break;
    default:
        ChipLogError(SoftwareUpdate, "Unexpected announcementReason: %u", static_cast<uint8_t>(announcementReason));
        return;
    }

    // IMPLEMENTATION CHOICE:
    // This implementation of the OTARequestor driver chooses to unconditionally start the query using the Provider specified in this command. 

    // Point  mProviderNodeId to the announced node and cancel the default providers timer
    mRequestor->SetCurrentProviderLocation(providerLocation);
    StopDefaultProvidersTimer();

    ScheduleDelayedAction(UpdateFailureState::kQuerying, System::Clock::Seconds32(msToStart/1000), StartDelayTimerHandler, mRequestor);
}

OTARequestorAction GenericOTARequestorDriver::GetRequestorAction(OTARequestorIncomingEvent input)
{

    OTAUpdateStateEnum state  = mRequestor->GetCurrentUpdateState();
    OTARequestorAction action = OTARequestorAction::DoNotProceed;

    switch(input)
        {
            case OTARequestorIncomingEvent::AnnouncedOTAProviderReceived:
                {
                    if(state != OTAUpdateStateEnum::kIdle) {
                        action = OTARequestorAction::DoNotProceed;
                    } else {
                        action = OTARequestorAction::Proceed;
                    }

                    break;
                }
            case OTARequestorIncomingEvent::TriggerImmediateQueryInvoked:
                {
                    if(state != OTAUpdateStateEnum::kIdle) {
                        action = OTARequestorAction::DoNotProceed;
                    } else {
                        action = OTARequestorAction::Proceed;
                    }
                    break;
                }
            case OTARequestorIncomingEvent::DefaultProvidersAttrSet:
                {
                    if(state != OTAUpdateStateEnum::kIdle) {
                        action = OTARequestorAction::CancelCurrentUpdateAndProceed;
                    } else {
                        action = OTARequestorAction::Proceed;
                    }
                    break;
                }
            case OTARequestorIncomingEvent::DefaultProvidersTimerExpiry:
                {
                    action = OTARequestorAction:: Proceed;
                    break;
                }
            default:
                {
                    action = OTARequestorAction::Proceed;
                }
        }

    return action;
}

void GenericOTARequestorDriver::DefaultProviderTimerHandler(System::Layer * systemLayer, void * appState)
{
    VerifyOrReturn(appState != nullptr);
    //OTARequestorInterface *requestorCore =  static_cast<OTARequestor *>(appState);

    //  SL TODO -- implement better API here
    //    TestModeSetProviderParameters(mTestingProviderNodeId, 0 , 0);
    mRequestor->ConnectToProvider(OTARequestorInterface::kQueryImage);
}


void GenericOTARequestorDriver::StartDefaultProvidersTimer()
{
    //  SL TODO: This has to be a method: PickNextDefaultProvider()
    //    mProviderNodeId = mTestingProviderNodeId;
    ScheduleDelayedAction(UpdateFailureState::kIdle,
                                               System::Clock::Seconds32(),
                                               [](System::Layer *, void * context){ (static_cast<GenericOTARequestorDriver *>(context))->DefaultProviderTimerHandler(nullptr, context); },
                                               this);
}

void GenericOTARequestorDriver::StopDefaultProvidersTimer()
{
    CancelDelayedAction([](System::Layer *, void * context){ (static_cast<GenericOTARequestorDriver *>(context))->DefaultProviderTimerHandler(nullptr, context); },
                                               this);
}

} // namespace DeviceLayer
} // namespace chip
