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

#include <system/SystemClock.h>
#include <system/SystemLayer.h>

#include "OTARequestorDriver.h"

namespace chip {

class OTARequestorInterface;
class OTAImageProcessorInterface;

namespace DeviceLayer {

class GenericOTARequestorDriver : public OTARequestorDriver
{
public:
    /**
     * Initialize OTA requestor driver.
     *
     * Set OTA requestor instance to be controlled by the driver, and OTA image processor, used to
     * apply/abort the downloaded image.
     *
     * Additionally, if the current image is executed for the first time, approve the current image
     * to make the update permanent, and send NotifyUpdateApplied command to the last OTA provider.
     */
    void Init(OTARequestorInterface * requestor, OTAImageProcessorInterface * processor);

    // Set the timeout (in seconds) for querying providers on the default OTA provider list; must be non-zero
    void SetPeriodicQueryTimeout(uint32_t timeout)
    {
        if (timeout != 0)
        {
            mPeriodicQueryTimeInterval = timeout;
        }
    }

    //// Virtual methods from OTARequestorDriver
    bool CanConsent() override;
    uint16_t GetMaxDownloadBlockSize() override;
    void HandleError(UpdateFailureState state, CHIP_ERROR error) override;
    void HandleIdleState(IdleStateReason reason) override;
    void UpdateAvailable(const UpdateDescription & update, System::Clock::Seconds32 delay) override;
    void UpdateNotFound(UpdateNotFoundReason reason, System::Clock::Seconds32 delay) override;
    void UpdateDownloaded() override;
    void UpdateConfirmed(System::Clock::Seconds32 delay) override;
    void UpdateSuspended(System::Clock::Seconds32 delay) override;
    void UpdateDiscontinued() override;
    void UpdateCancelled() override;
    void OTACommissioningCallback() override;
    void ProcessAnnounceOTAProviders(const ProviderLocationType & providerLocation,
                                     app::Clusters::OtaSoftwareUpdateRequestor::OTAAnnouncementReason announcementReason) override;
    void SendQueryImage() override;
    bool GetNextProviderLocation(ProviderLocationType & providerLocation, bool & listExhausted) override;

protected:
    void StartDefaultProviderTimer();
    void StopDefaultProviderTimer();
    void DefaultProviderTimerHandler(System::Layer * systemLayer, void * appState);
    void ScheduleDelayedAction(System::Clock::Seconds32 delay, System::TimerCompleteCallback action, void * aAppState);
    void CancelDelayedAction(System::TimerCompleteCallback action, void * aAppState);
    bool ProviderLocationsEqual(const ProviderLocationType & a, const ProviderLocationType & b);

    OTARequestorInterface * mRequestor           = nullptr;
    OTAImageProcessorInterface * mImageProcessor = nullptr;
    uint32_t mOtaStartDelaySec                   = 0;
    uint32_t mPeriodicQueryTimeInterval = (24 * 60 * 60); // Timeout for querying providers on the default OTA provider list
    // Maximum number of times to retry a BUSY OTA provider before moving to the next available one
    static constexpr uint8_t kMaxBusyProviderRetryCount = 3;
    uint8_t mProviderRetryCount; // Track retry count for the current provider
};

} // namespace DeviceLayer
} // namespace chip
