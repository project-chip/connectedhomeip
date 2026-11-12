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

#include <app/CommandResponseHelper.h>
#include <clusters/KeypadInput/Commands.h>
#include <clusters/KeypadInput/Enums.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/TypeTraits.h>

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
    /// Handles a SendKey command.
    ///
    /// The delegate is expected to answer through `helper` (either with a SendKeyResponse via
    /// `helper.Success(response)` or with a status via `helper.Failure(...)`) before returning.
    /// If no answer is sent, the cluster replies with a `Failure` status.
    virtual void HandleSendKey(CommandResponseHelper<Commands::SendKeyResponse::Type> & helper, const CECKeyCodeEnum & keyCode) = 0;

    bool HasFeature(chip::EndpointId endpoint, Feature feature)
    {
        return (GetFeatureMap(endpoint) & chip::to_underlying(feature)) != 0;
    }

    /// Returns the value reported by the FeatureMap attribute of the cluster on `endpoint`.
    ///
    /// The returned value is read on every FeatureMap read. Changes are not reported to subscribers.
    /// Implementations must not read the FeatureMap attribute of the Keypad Input cluster through
    /// the data model, as that read calls back into this method.
    virtual uint32_t GetFeatureMap(chip::EndpointId endpoint) = 0;

    virtual ~Delegate() = default;
};

} // namespace KeypadInput
} // namespace Clusters
} // namespace app
} // namespace chip
