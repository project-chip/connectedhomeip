/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/DecodableList.h>
#include <app/util/af.h>
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
