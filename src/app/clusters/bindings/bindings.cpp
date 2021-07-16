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

#include <app/CommandHandler.h>
#include <app/util/binding-table.h>
#include <support/logging/CHIPLogging.h>

using namespace chip;

EmberStatus prepareBinding(EmberBindingTableEntry & binding, NodeId nodeId, GroupId groupId, EndpointId endpointId,
                           ClusterId clusterId)
{
    if (groupId && nodeId)
    {
        return EMBER_BAD_ARGUMENT;
    }

    binding.clusterId    = clusterId;
    binding.local        = emberAfCurrentCommand()->apsFrame->destinationEndpoint;
    binding.networkIndex = 0;

    if (groupId)
    {
        binding.type    = EMBER_MULTICAST_BINDING;
        binding.groupId = groupId;
        binding.remote  = 0;
    }
    else
    {
        binding.type   = EMBER_UNICAST_BINDING;
        binding.nodeId = nodeId;
        binding.remote = endpointId;
    }

    return EMBER_SUCCESS;
}

EmberStatus getBindingIndex(EmberBindingTableEntry & newEntry, uint8_t * bindingIndex)
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

EmberStatus getUnusedBindingIndex(uint8_t * bindingIndex)
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

bool emberAfBindingClusterBindCallback(chip::app::CommandHandler * commandObj, NodeId nodeId, GroupId groupId,
                                       EndpointId endpointId, ClusterId clusterId)
{
    ChipLogDetail(Zcl, "RX: BindCallback");

    EmberBindingTableEntry bindingEntry;
    if (prepareBinding(bindingEntry, nodeId, groupId, endpointId, clusterId) != EMBER_SUCCESS)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_MALFORMED_COMMAND);
        return true;
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
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfBindingClusterUnbindCallback(chip::app::CommandHandler * commandObj, NodeId nodeId, GroupId groupId,
                                         EndpointId endpointId, ClusterId clusterId)
{
    ChipLogDetail(Zcl, "RX: UnbindCallback");

    EmberBindingTableEntry bindingEntry;
    if (prepareBinding(bindingEntry, nodeId, groupId, endpointId, clusterId) != EMBER_SUCCESS)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_MALFORMED_COMMAND);
        return true;
    }

    uint8_t bindingIndex;
    if (getBindingIndex(bindingEntry, &bindingIndex) != EMBER_SUCCESS)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
        return true;
    }

    emberDeleteBinding(bindingIndex);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
