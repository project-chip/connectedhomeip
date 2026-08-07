/*
 *
 *    Copyright (c) 2021-2026 Project CHIP Authors
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
#include "OTAConsentHandler.h"

namespace chip {

// Define the global instance here
OTAConsentHandler gConsentHandler;

void OTAConsentHandler::SetRequestorCanConsent(chip::Optional<bool> consent)
{
    mRequestorCanConsent = consent;
}

void OTAConsentHandler::SetRequestorCanConsent(bool consent)
{
    mRequestorCanConsent.SetValue(consent);
}

chip::Optional<bool> OTAConsentHandler::GetRequestorConsent() const
{
    return mRequestorCanConsent;
}

bool OTAConsentHandler::HasValue() const
{
    return mRequestorCanConsent.HasValue();
}

void OTAConsentHandler::SetUserConsentState(chip::ota::UserConsentState state)
{
    mUserConsentState = state;
}

chip::ota::UserConsentState OTAConsentHandler::GetUserConsentState() const
{
    return mUserConsentState;
}

} // namespace chip
