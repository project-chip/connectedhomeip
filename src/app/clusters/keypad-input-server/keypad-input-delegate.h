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
namespace KeypadInput {

/** @brief
 *    Defines methods for implementing application-specific logic for the Keypad Input Cluster.
 */
class Delegate
{
public:
    virtual void HandleSendKey(CommandResponseHelper<Commands::SendKeyResponse::Type> & helper, const CECKeyCodeEnum & keyCode) = 0;

    bool HasFeature(chip::EndpointId endpoint, Feature feature);

    virtual uint32_t GetFeatureMap(chip::EndpointId endpoint) = 0;

    virtual ~Delegate() = default;
};

} // namespace KeypadInput
} // namespace Clusters
} // namespace app
} // namespace chip
