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

#include <lib/core/CHIPError.h>
#include <ota-provider-common/UserConsentDelegate.h>

namespace chip {
namespace ota {

class DefaultUserConsentProvider : public UserConsentDelegate
{
public:
    DefaultUserConsentProvider() = default;

    ~DefaultUserConsentProvider() = default;

    // This method returns kGranted unless explicitly denied by the user by calling RevokeUserConsent()
    UserConsentState GetUserConsentState(const UserConsentSubject & subject) override;

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

    void LogUserConsentSubject(const UserConsentSubject & subject);
};

} // namespace ota
} // namespace chip
