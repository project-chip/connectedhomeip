/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
