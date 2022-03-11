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
#include <platform/OTARequestorUserConsentDelegate.h>

namespace chip {
namespace ota {

class DefaultOTARequestorUserConsentProvider : public OTARequestorUserConsentDelegate
{
public:
    DefaultOTARequestorUserConsentProvider() = default;

    ~DefaultOTARequestorUserConsentProvider() = default;

    // This method returns kGranted unless explicitly set by the user by calling SetUserConsentState()
    UserConsentState GetUserConsentState(const UserConsentSubject & subject) override
    {
        UserConsentState curUserConsentState = mUserConsentState;

        subject.Log();

        if (mUserConsentStateCount > 0)
        {
            mUserConsentStateCount--;
            mUserConsentState = chip::ota::UserConsentState::kGranted;
        }

        return curUserConsentState;
    }

    UserConsentState CheckDeferredUserConsentState() override { return mUserConsentState; }

    void SetUserConsentState(UserConsentState state) { mUserConsentState = state; }

    void SetUserConsentStateCount(uint8_t count) { mUserConsentStateCount = count; }

private:
    UserConsentState mUserConsentState = UserConsentState::kGranted;
    uint8_t mUserConsentStateCount = 0; // # of times to respond with value of mGlobalConsentState before resorting to the success response.
};

} // namespace ota
} // namespace chip
