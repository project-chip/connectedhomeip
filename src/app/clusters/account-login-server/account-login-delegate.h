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

#include <app-common/zap-generated/cluster-objects.h>

#include <app/CommandResponseHelper.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AccountLogin {

/** @brief
 *    Defines methods for implementing application-specific logic for the Account Login Cluster.
 */
class Delegate
{
public:
    // helper method to allow the platform to facilitate providing the pin
    virtual void SetSetupPin(char * setupPin) = 0;

    virtual bool HandleLogin(const chip::CharSpan & tempAccountIdentifierString, const chip::CharSpan & setupPinString,
                             const chip::Optional<NodeId> & nodeId)                                                    = 0;
    virtual bool HandleLogout(const chip::Optional<NodeId> & nodeId)                                                   = 0;
    virtual void HandleGetSetupPin(CommandResponseHelper<Commands::GetSetupPINResponse::Type> & helper,
                                   const chip::CharSpan & tempAccountIdentifierString)                                 = 0;
    virtual void GetSetupPin(char * setupPin, size_t setupPinSize, const chip::CharSpan & tempAccountIdentifierString) = 0;

    virtual uint16_t GetClusterRevision(chip::EndpointId endpoint) = 0;

    virtual ~Delegate() = default;
};

} // namespace AccountLogin
} // namespace Clusters
} // namespace app
} // namespace chip
