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
namespace DeviceLayer {
namespace {

GenericOTARequestorDriver * ToDriver(void * context)
{
    return static_cast<GenericOTARequestorDriver *>(context);
}

} // namespace

bool GenericOTARequestorDriver::CanConsent()
{
    return false;
}

uint16_t GenericOTARequestorDriver::GetMaxDownloadBlockSize()
{
    return 1024;
}

void GenericOTARequestorDriver::HandleError(UpdateStateEnum state, CHIP_ERROR error)
{
    // TODO: Schedule the next QueryImage
}

void GenericOTARequestorDriver::UpdateAvailable(const UpdateDescription & update, System::Clock::Seconds32 delay)
{
    VerifyOrDie(mRequestor != nullptr);
    ScheduleDelayedAction(UpdateStateEnum::DelayedOnQuery, delay,
                          [](System::Layer *, void * context) { ToDriver(context)->mRequestor->DownloadUpdate(); });
}

void GenericOTARequestorDriver::UpdateNotFound(UpdateNotFoundReason reason, System::Clock::Seconds32 delay)
{
    // TODO: Schedule the next QueryImage
}

void GenericOTARequestorDriver::UpdateDownloaded()
{
    VerifyOrDie(mRequestor != nullptr);
    mRequestor->ApplyUpdate();
}

void GenericOTARequestorDriver::UpdateConfirmed(System::Clock::Seconds32 delay)
{
    VerifyOrDie(mImageProcessor != nullptr);
    ScheduleDelayedAction(UpdateStateEnum::DelayedOnApply, delay,
                          [](System::Layer *, void * context) { ToDriver(context)->mImageProcessor->Apply(); });
}

void GenericOTARequestorDriver::UpdateSuspended(System::Clock::Seconds32 delay)
{
    VerifyOrDie(mRequestor != nullptr);
    ScheduleDelayedAction(UpdateStateEnum::DelayedOnApply, delay,
                          [](System::Layer *, void * context) { ToDriver(context)->mRequestor->ApplyUpdate(); });
}

void GenericOTARequestorDriver::UpdateDiscontinued()
{
    VerifyOrDie(mImageProcessor != nullptr);
    mImageProcessor->Abort();
}

void GenericOTARequestorDriver::ScheduleDelayedAction(UpdateStateEnum state, System::Clock::Seconds32 delay,
                                                      System::TimerCompleteCallback action)
{
    CHIP_ERROR error = SystemLayer().StartTimer(std::chrono::duration_cast<System::Clock::Timeout>(delay), action, this);

    if (error != CHIP_NO_ERROR)
    {
        HandleError(state, error);
    }
}

} // namespace DeviceLayer
} // namespace chip
