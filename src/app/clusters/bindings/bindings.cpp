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
#include <app/clusters/bindings/BindingManager.h>
#include <app/util/binding-table.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app::Clusters::Binding;

// TODO: add binding table to the persistent storage

static EmberStatus getBindingIndex(EmberBindingTableEntry & newEntry, uint8_t * bindingIndex)
{
    EmberBindingTableEntry currentEntry;
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        emberGetBinding(i, &currentEntry);
        if (currentEntry.type != EMBER_UNUSED_BINDING && currentEntry == newEntry)
        {
            *bindingIndex = i;
            return EMBER_SUCCESS;
        }
    }

    return EMBER_NOT_FOUND;
}

static EmberStatus getUnusedBindingIndex(uint8_t * bindingIndex)
{
    EmberBindingTableEntry currentEntry;
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        emberGetBinding(i, &currentEntry);
        if (currentEntry.type == EMBER_UNUSED_BINDING)
        {
            *bindingIndex = i;
            return EMBER_SUCCESS;
        }
    }

    return EMBER_NOT_FOUND;
}

bool emberAfBindingClusterBindCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                       const Commands::Bind::DecodableType & commandData)
{
    NodeId nodeId             = commandData.nodeId;
    GroupId groupId           = commandData.groupId;
    ClusterId clusterId       = commandData.clusterId;
    EndpointId remoteEndpoint = commandData.endpointId;
    EndpointId localEndpoint  = commandPath.mEndpointId;
    FabricIndex fabricIndex   = commandObj->GetAccessingFabricIndex();
    EmberBindingTableEntry bindingEntry;

    ChipLogDetail(Zcl, "RX: BindCallback");

    if ((groupId != 0 && nodeId != 0) || (groupId == 0 && nodeId == 0) || (groupId != 0 && remoteEndpoint != 0))
    {
        ChipLogError(Zcl, "Binding: Invalid request");
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_MALFORMED_COMMAND);
        return true;
    }

    if (groupId)
    {
        bindingEntry = EmberBindingTableEntry::ForGroup(fabricIndex, groupId, localEndpoint, clusterId);
    }
    else
    {
        bindingEntry = EmberBindingTableEntry::ForNode(fabricIndex, nodeId, localEndpoint, remoteEndpoint, clusterId);
    }

    uint8_t bindingIndex;
    if (getBindingIndex(bindingEntry, &bindingIndex) != EMBER_NOT_FOUND)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_DUPLICATE_EXISTS);
        return true;
    }

    if (getUnusedBindingIndex(&bindingIndex) != EMBER_SUCCESS)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INSUFFICIENT_SPACE);
        return true;
    }

    emberSetBinding(bindingIndex, &bindingEntry);
    if (nodeId)
    {
        CHIP_ERROR err = BindingManager::GetInstance().UnicastBindingCreated(bindingEntry);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogProgress(
                Zcl, "Binding: Failed to create session for unicast binding to device " ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                ChipLogValueX64(nodeId), err.Format());
        }
    }

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfBindingClusterUnbindCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                         const Commands::Unbind::DecodableType & commandData)
{
    NodeId nodeId             = commandData.nodeId;
    GroupId groupId           = commandData.groupId;
    ClusterId clusterId       = commandData.clusterId;
    EndpointId remoteEndpoint = commandData.endpointId;
    EndpointId localEndpoint  = commandPath.mEndpointId;
    FabricIndex fabricIndex   = commandObj->GetAccessingFabricIndex();
    EmberBindingTableEntry bindingEntry;

    ChipLogDetail(Zcl, "RX: UnbindCallback");

    if ((groupId != 0 && nodeId != 0) || (groupId == 0 && nodeId == 0))
    {
        ChipLogError(Zcl, "Binding: Invalid request");
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_MALFORMED_COMMAND);
        return true;
    }
    if (groupId)
    {
        bindingEntry = EmberBindingTableEntry::ForGroup(fabricIndex, groupId, localEndpoint, clusterId);
    }
    else
    {
        bindingEntry = EmberBindingTableEntry::ForNode(fabricIndex, nodeId, localEndpoint, remoteEndpoint, clusterId);
    }

    uint8_t bindingIndex;
    if (getBindingIndex(bindingEntry, &bindingIndex) != EMBER_SUCCESS)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
        return true;
    }

    CHIP_ERROR err = BindingManager::GetInstance().UnicastBindingRemoved(bindingIndex);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Binding: Failed to remove pending notification for unicast binding" ChipLogFormatX64 ": %s",
                     ChipLogValueX64(nodeId), err.AsString());
    }

    emberDeleteBinding(bindingIndex);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

void MatterBindingPluginServerInitCallback() {}
