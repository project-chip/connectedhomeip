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
};

class UserConsentDelegate
{
public:
    virtual ~UserConsentDelegate() = default;

    virtual UserConsentState GetUserConsentState(chip::NodeId nodeId, chip::EndpointId endpoint, uint32_t currentVersion,
                                                 uint32_t newVersion) = 0;
};

} // namespace ota
} // namespace chip
