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
#include <ota-provider-common/DefaultUserConsentProvider.h>

namespace chip {
namespace ota {

UserConsentState DefaultUserConsentProvider::GetUserConsentState(const UserConsentSubject & subject)
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

} // namespace ota
} // namespace chip
