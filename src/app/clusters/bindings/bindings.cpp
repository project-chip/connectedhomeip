/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Implementation for the Binding Server Cluster
 ***************************************************************************/

#include <app/util/af.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/binding/BindingTable.h>
#include <app/util/binding-table.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app::Clusters::Binding;
using chip::app::BindingTable;

namespace {
// TODO: Add peer fabric index to command path
constexpr FabricIndex kPeerFabricIndex = 1;

EmberAfStatus MapBindingErrorToStatus(CHIP_ERROR error)
{
    if (error == CHIP_ERROR_NO_MEMORY)
    {
        return EMBER_ZCL_STATUS_LIMIT_REACHED;
    }
    if (error == CHIP_NO_ERROR)
    {
        return EMBER_ZCL_STATUS_SUCCESS;
    }
    if (error == CHIP_ERROR_KEY_NOT_FOUND)
    {
        return EMBER_ZCL_STATUS_NOT_FOUND;
    }
    return EMBER_ZCL_STATUS_FAILURE;
}

} // namespace

bool emberAfBindingClusterBindCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                       const Commands::Bind::DecodableType & commandData)
{
    auto & nodeId             = commandData.nodeId;
    auto & endpointId         = commandData.endpointId;
    auto & clusterId          = commandData.clusterId;
    EndpointId selfEndpointId = commandPath.mEndpointId;

    ChipLogProgress(Zcl, "RX: BindCallback to node %" PRIu64 "\n", nodeId);
    CHIP_ERROR error = BindingTable::GetInstance().Add(selfEndpointId, nodeId, endpointId, kPeerFabricIndex, clusterId);
    emberAfSendImmediateDefaultResponse(MapBindingErrorToStatus(error));
    return true;
}

bool emberAfBindingClusterUnbindCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                         const Commands::Unbind::DecodableType & commandData)
{
    auto & nodeId             = commandData.nodeId;
    auto & endpointId         = commandData.endpointId;
    auto & clusterId          = commandData.clusterId;
    EndpointId selfEndpointId = commandPath.mEndpointId;

    ChipLogProgress(Zcl, "RX: UnbindCallback to node %" PRIu64 "\n", nodeId);
    CHIP_ERROR error = BindingTable::GetInstance().Remove(selfEndpointId, nodeId, endpointId, kPeerFabricIndex, clusterId);
    emberAfSendImmediateDefaultResponse(MapBindingErrorToStatus(error));

    return true;
}

void MatterBindingPluginServerInitCallback() {}
