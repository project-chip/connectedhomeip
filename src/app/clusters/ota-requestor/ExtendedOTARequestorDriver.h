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
#pragma once
#include <app/clusters/ota-requestor/OTARequestorUserConsentDelegate.h>
#include <platform/CHIPDeviceLayer.h>

#include "DefaultOTARequestorDriver.h"

namespace chip {
namespace DeviceLayer {

/**
 * This extends the DefaultOTARequestorDriver and provides optional
 * features. For now, it adds support for the handling user consent.
 */
class ExtendedOTARequestorDriver : public DefaultOTARequestorDriver
{
public:
    bool CanConsent() override;

    void UpdateAvailable(const UpdateDescription & update, System::Clock::Seconds32 delay) override;

    /**
     * Set delegate for requesting user consent
     */
    void SetUserConsentDelegate(chip::ota::OTARequestorUserConsentDelegate * delegate) { mUserConsentDelegate = delegate; }

private:
    System::Clock::Seconds32 mDelayedActionTime;

    chip::ota::OTARequestorUserConsentDelegate * mUserConsentDelegate = nullptr;

    void PollUserConsentState();

    CHIP_ERROR GetUserConsentSubject(chip::ota::UserConsentSubject & subject, const UpdateDescription & update);

    void HandleUserConsentState(chip::ota::UserConsentState userConsentState);
};

} // namespace DeviceLayer
} // namespace chip
