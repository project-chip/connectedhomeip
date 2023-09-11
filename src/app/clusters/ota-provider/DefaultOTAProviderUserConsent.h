/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <app/clusters/ota-provider/OTAProviderUserConsentDelegate.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace ota {

/**
 * @brief Default implementation of OTAProviderUserConsentDelegate interface.
 *
 * This class provides API to set the user consent state, and this state is then used by
 * OTA Provider to send an apporpriate response to QueryImage request.
 */

class DefaultOTAProviderUserConsent : public OTAProviderUserConsentDelegate
{
public:
    DefaultOTAProviderUserConsent() = default;

    ~DefaultOTAProviderUserConsent() = default;

    // This method returns kGranted unless explicitly set by the user by calling SetGlobalUserConsentState()
    UserConsentState GetUserConsentState(const UserConsentSubject & subject) override
    {
        subject.Log();

        if (mUseGlobalConsent)
        {
            // Reset mGlobalConsentState to success case after returning other possible values once.
            UserConsentState curGlobalConsentState = mGlobalConsentState;
            mGlobalConsentState                    = UserConsentState::kGranted;
            return curGlobalConsentState;
        }

        return UserConsentState::kGranted;
    }

    // If this is set to true, all the user consent requests will be replied with global consent.
    void SetGlobalUserConsentState(UserConsentState state)
    {
        mUseGlobalConsent   = true;
        mGlobalConsentState = state;
    }

    // state is only valid if isGlobalConsentSet is true
    void GetGlobalUserConsentState(bool & isGlobalConsentSet, UserConsentState & state)
    {
        isGlobalConsentSet = mUseGlobalConsent;
        state              = mGlobalConsentState;
    }

    // Clear the global user consent state
    void ClearGlobalUserConsentState() { mUseGlobalConsent = false; }

private:
    bool mUseGlobalConsent = false;

    UserConsentState mGlobalConsentState = UserConsentState::kGranted;
};

} // namespace ota
} // namespace chip
