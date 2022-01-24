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

DefaultUserConsentProvider::DefaultUserConsentProvider()
{
    for (uint8_t i = 0; i < kMaxUserConsentEntries; i++)
    {
        mUserConsentEntries[i].nodeId = chip::kUndefinedNodeId;
    }
}

UserConsentState DefaultUserConsentProvider::GetUserConsentState(chip::NodeId nodeId, chip::EndpointId endpoint,
                                                                 uint32_t currentVersion, uint32_t newVersion)
{
    for (uint8_t i = 0; i < kMaxUserConsentEntries; i++)
    {
        if (mUserConsentEntries[i].nodeId == nodeId && mUserConsentEntries[i].endpoint == endpoint)
        {
            return mUserConsentEntries[i].state;
        }
    }
    return UserConsentState::kGranted;
}

CHIP_ERROR DefaultUserConsentProvider::SetUserConsentState(chip::NodeId nodeId, chip::EndpointId endpoint, UserConsentState state)
{
    for (uint8_t i = 0; i < kMaxUserConsentEntries; i++)
    {
        if (mUserConsentEntries[i].nodeId == nodeId && mUserConsentEntries[i].endpoint == endpoint)
        {
            mUserConsentEntries[i].state = state;
            return CHIP_NO_ERROR;
        }
    }

    for (uint8_t i = 0; i < kMaxUserConsentEntries; i++)
    {
        if (mUserConsentEntries[i].nodeId == chip::kUndefinedNodeId)
        {
            mUserConsentEntries[i].nodeId   = nodeId;
            mUserConsentEntries[i].endpoint = endpoint;
            mUserConsentEntries[i].state    = state;
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR DefaultUserConsentProvider::GrantUserConsent(chip::NodeId nodeId, chip::EndpointId endpoint)
{
    return SetUserConsentState(nodeId, endpoint, UserConsentState::kGranted);
}

CHIP_ERROR DefaultUserConsentProvider::RevokeUserConsent(chip::NodeId nodeId, chip::EndpointId endpoint)
{
    return SetUserConsentState(nodeId, endpoint, UserConsentState::kDenied);
}

} // namespace ota
} // namespace chip
