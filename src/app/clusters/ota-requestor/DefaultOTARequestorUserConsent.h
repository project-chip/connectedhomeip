/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <app/clusters/ota-requestor/OTARequestorUserConsentDelegate.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace ota {

/**
 * @brief Default implementation of OTARequestorUserConsentDelegate interface.
 *
 * This class provides API to set the user consent state, and this state is then used by
 * OTA Requestor Driver to decide whether to continue with the OTA process.
 */

class DefaultOTARequestorUserConsent : public OTARequestorUserConsentDelegate
{
public:
    DefaultOTARequestorUserConsent() = default;

    ~DefaultOTARequestorUserConsent() = default;

    // This method returns kGranted unless explicitly set by the user by calling SetUserConsentState()
    UserConsentState GetUserConsentState(const UserConsentSubject & subject) override
    {
        UserConsentState curUserConsentState = mUserConsentState;

        subject.Log();
        mUserConsentState = chip::ota::UserConsentState::kGranted;

        return curUserConsentState;
    }

    UserConsentState CheckDeferredUserConsentState() override { return mUserConsentState; }

    void SetUserConsentState(UserConsentState state) { mUserConsentState = state; }

private:
    UserConsentState mUserConsentState = UserConsentState::kGranted;
};

} // namespace ota
} // namespace chip
