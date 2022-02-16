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

#pragma once

#include <platform/OTARequestorDriver.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>

namespace chip {

class OTARequestorInterface;
class OTAImageProcessorInterface;

namespace DeviceLayer {

class GenericOTARequestorDriver : public OTARequestorDriver
{
public:
    /**
     * Called to perform some initialization including:
     *   - Set the OTA requestor instance used to direct download progress
     *   - Set the OTA image processor instance used to apply/abort the downloaded image
     */
    void Init(OTARequestorInterface * requestor, OTAImageProcessorInterface * processor)
    {
        mRequestor      = requestor;
        mImageProcessor = processor;
    }

    bool CanConsent() override;
    uint16_t GetMaxDownloadBlockSize() override;

    void HandleError(UpdateFailureState state, CHIP_ERROR error) override;
    void UpdateAvailable(const UpdateDescription & update, System::Clock::Seconds32 delay) override;
    void UpdateNotFound(UpdateNotFoundReason reason, System::Clock::Seconds32 delay) override;
    void UpdateDownloaded() override;
    void UpdateConfirmed(System::Clock::Seconds32 delay) override;
    void UpdateSuspended(System::Clock::Seconds32 delay) override;
    void UpdateDiscontinued() override;
    void UpdateCancelled() override;

protected:
    void ScheduleDelayedAction(UpdateFailureState state, System::Clock::Seconds32 delay, System::TimerCompleteCallback action);

    OTARequestorInterface * mRequestor           = nullptr;
    OTAImageProcessorInterface * mImageProcessor = nullptr;
};

} // namespace DeviceLayer
} // namespace chip
