/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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
