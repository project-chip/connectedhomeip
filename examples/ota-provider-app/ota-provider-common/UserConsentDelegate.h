/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

namespace chip {
namespace ota {

class UserConsentDelegate
{

public:
    virtual ~UserConsentDelegate() {}

    // This function should be called when application has the user consent
    typedef void (*UserConsentCallback)(bool userConsent, EndpointId endpoint, NodeId nodeId);
    UserConsentCallback userConsentCallback;

    // TODO: According to the spec, we should provide all the available information
    // to the application related to the available OTA update.

    // OTA provider should call this API for obtaining user consent
    virtual void ObtainUserConsentAsync(NodeId nodeId, EndpointId endpoint, uint32_t currentVersion, uint32_t newVersion) = 0;
};

} // namespace ota
} // namespace chip
