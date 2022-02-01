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
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>
#include <lib/support/Span.h>

namespace chip {
namespace ota {

enum UserConsentState
{
    // User consent is granted.
    kGranted,
    // Obtaining user consent is in progress, async implementations should return this state.
    kObtaining,
    // User consent is denied.
    kDenied,
    kUnknown,
};

/**
 * @brief User consent subject contains the information of the OTA requestor
 * that requires obtaining user consent for performing the OTA update.
 */
struct UserConsentSubject
{
    // Fabric Index
    FabricIndex fabricIndex;

    // Node ID of the OTA Requestor
    NodeId requestorNodeId;

    // Endpoint of the OTA Provider
    EndpointId providerEndpointId;

    // Vendor ID of the OTA Requestor
    uint16_t requestorVendorId;

    // Product ID of the OTA Requestor
    uint16_t requestorProductId;

    // Current software version of the OTA Requestor
    uint32_t requestorCurrentVersion;

    // Target software version available for the OTA Requestor
    uint32_t requestorTargetVersion;

    // This data is not owned by UserConsentSubject and therefore any user of this field
    // has to copy the data and own it if not immediately used from an argument having a UserConsentSubject
    ByteSpan metadata;
};

class UserConsentDelegate
{
public:
    virtual ~UserConsentDelegate() = default;

    virtual UserConsentState GetUserConsentState(const UserConsentSubject & subject) = 0;

    const char * UserConsentStateToString(UserConsentState state)
    {
        switch (state)
        {
        case kGranted:
            return "Granted";
        case kObtaining:
            return "Obtaining";
        case kDenied:
            return "Denied";
        default:
            return "Unknown";
        }
    }
};

} // namespace ota
} // namespace chip
