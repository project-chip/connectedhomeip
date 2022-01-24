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
    DefaultUserConsentProvider();

    ~DefaultUserConsentProvider() = default;

    // This method returns kGranted unless explicitly denied by the user by calling RevokeUserConsent()
    UserConsentState GetUserConsentState(chip::NodeId nodeId, chip::EndpointId endpoint, uint32_t currentVersion,
                                         uint32_t newVersion);

    // Grant the user consent for the given node and endpoint for OTA updates
    CHIP_ERROR GrantUserConsent(chip::NodeId nodeId, chip::EndpointId endpoint);

    // Revoke the user consent for the given node and endpoint for OTA updates
    CHIP_ERROR RevokeUserConsent(chip::NodeId nodeId, chip::EndpointId endpoint);

private:
    CHIP_ERROR SetUserConsentState(chip::NodeId nodeId, chip::EndpointId endpoint, UserConsentState state);

    static constexpr uint8_t kMaxUserConsentEntries = 10;

    struct UserConsentEntry
    {
        EndpointId endpoint;
        NodeId nodeId;
        UserConsentState state;
    } mUserConsentEntries[kMaxUserConsentEntries];
};

} // namespace ota
} // namespace chip
