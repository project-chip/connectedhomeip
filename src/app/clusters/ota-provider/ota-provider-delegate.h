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
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/DecodableList.h>
#include <lib/core/Optional.h>

namespace chip {
namespace app {
namespace Clusters {

// TODO: Should this be OTAProvider::Delegate?
/** @brief
 *    Defines methods for implementing application-specific logic for the OTA Provider Cluster.
 */
class DLL_EXPORT OTAProviderDelegate
{
public:
    /**
     * Called to handle a QueryImage command and is responsible for sending the response (if success) or status (if error). The
     * caller is responsible for validating fields in the command.
     */
    virtual void HandleQueryImage(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                  const OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData) = 0;

    /**
     * Called to handle an ApplyUpdateRequest command and is responsible for sending the response (if success) or status (if error).
     * The caller is responsible for validating fields in the command.
     */
    virtual void
    HandleApplyUpdateRequest(CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                             const OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData) = 0;

    /**
     * Called to handle a NotifyUpdateApplied command and is responsible for sending the status. The caller is responsible for
     * validating fields in the command.
     */
    virtual void
    HandleNotifyUpdateApplied(CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                              const OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData) = 0;

    virtual ~OTAProviderDelegate() = default;
};

} // namespace Clusters
} // namespace app
} // namespace chip
