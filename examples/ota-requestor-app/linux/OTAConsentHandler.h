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

#ifndef OTA_CONSENT_HANDLER_H
#define OTA_CONSENT_HANDLER_H

#include <app/OTAUserConsentCommon.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h> // Ensure this is the correct include path

namespace chip {

class OTAConsentHandler
{
public:
    OTAConsentHandler() = default;

    /**
     * FIX: Remove the 'chip::' prefix because we are already
     * inside the chip namespace.
     */
    void SetRequestorCanConsent(Optional<bool> consent);

    void SetRequestorCanConsent(bool consent);

    /**
     * FIX: Just use Optional<bool> here as well.
     */
    Optional<bool> GetRequestorConsent() const;

    bool HasValue() const;

    void SetUserConsentState(chip::ota::UserConsentState state);

    chip::ota::UserConsentState GetUserConsentState() const;

private:
    Optional<bool> mRequestorCanConsent;
    chip::ota::UserConsentState mUserConsentState = chip::ota::UserConsentState::kUnknown;
};

extern OTAConsentHandler gConsentHandler;

} // namespace chip

#endif // OTA_CONSENT_HANDLER_H
