/**
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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
 * @brief Routines for the Groups Server plugin, the
 *server implementation of the Groups cluster.
 *******************************************************************************
 ******************************************************************************/

// *******************************************************************
// * groups-server.c
// *
// *
// * Copyright 2010 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************
#include "groups-server.h"

#include <app/Command.h>
#include <app/common/gen/att-storage.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/util/af.h>
#include <app/util/binding-table.h>

#ifdef EMBER_AF_PLUGIN_SCENES
#include <app/clusters/scenes/scenes.h>
#endif // EMBER_AF_PLUGIN_SCENES

using namespace chip;

static bool isGroupPresent(EndpointId endpoint, GroupId groupId);

static bool bindingGroupMatch(EndpointId endpoint, GroupId groupId, EmberBindingTableEntry * entry);

static uint8_t findGroupIndex(EndpointId endpoint, GroupId groupId);

void emberAfGroupsClusterServerInitCallback(EndpointId endpoint)
{
    // The high bit of Name Support indicates whether group names are supported.
    // Group names are not supported by this plugin.
    EmberAfStatus status;
    uint8_t nameSupport = (uint8_t) emberAfPluginGroupsServerGroupNamesSupportedCallback(endpoint);
    status = emberAfWriteAttribute(endpoint, ZCL_GROUPS_CLUSTER_ID, ZCL_GROUP_NAME_SUPPORT_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                   (uint8_t *) &nameSupport, ZCL_BITMAP8_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfGroupsClusterPrintln("ERR: writing name support %x", status);
    }
}

// --------------------------
// Internal functions used to maintain the group table within the context
// of the binding table.
//
// In the binding:
// The first two bytes of the identifier is set to the groupId
// The local endpoint is set to the endpoint that is mapped to this group
// --------------------------
static EmberAfStatus addEntryToGroupTable(EndpointId endpoint, GroupId groupId, uint8_t * groupName)
{
    uint8_t i;

    // Check for duplicates.
    if (isGroupPresent(endpoint, groupId))
    {
        // Even if the group already exists, tell the application about the name,
        // so it can cope with renames.
        emberAfPluginGroupsServerSetGroupNameCallback(endpoint, groupId, groupName);
        return EMBER_ZCL_STATUS_DUPLICATE_EXISTS;
    }

    // Look for an empty binding slot.
    for (i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        EmberBindingTableEntry binding;
        if (emberGetBinding(i, &binding) == EMBER_SUCCESS && binding.type == EMBER_UNUSED_BINDING)
        {
            EmberStatus status;
            binding.type    = EMBER_MULTICAST_BINDING;
            binding.groupId = groupId;
            binding.local   = endpoint;

            status = emberSetBinding(i, &binding);
            if (status == EMBER_SUCCESS)
            {
                // Set the group name, if supported
                emberAfPluginGroupsServerSetGroupNameCallback(endpoint, groupId, groupName);
                return EMBER_ZCL_STATUS_SUCCESS;
            }
            else
            {
                emberAfGroupsClusterPrintln("ERR: Failed to create binding (0x%x)", status);
            }
        }
    }
    emberAfGroupsClusterPrintln("ERR: Binding table is full");
    return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
}

static EmberAfStatus removeEntryFromGroupTable(EndpointId endpoint, GroupId groupId)
{
    if (isGroupPresent(endpoint, groupId))
    {
        uint8_t bindingIndex = findGroupIndex(endpoint, groupId);
        EmberStatus status   = emberDeleteBinding(bindingIndex);
        if (status == EMBER_SUCCESS)
        {
            uint8_t groupName[ZCL_GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH + 1] = { 0 };
            emberAfPluginGroupsServerSetGroupNameCallback(endpoint, groupId, groupName);
            return EMBER_ZCL_STATUS_SUCCESS;
        }
        else
        {
            emberAfGroupsClusterPrintln("ERR: Failed to delete binding (0x%x)", status);
            return EMBER_ZCL_STATUS_FAILURE;
        }
    }
    return EMBER_ZCL_STATUS_NOT_FOUND;
}

bool emberAfGroupsClusterAddGroupCallback(chip::app::Command * commandObj, GroupId groupId, uint8_t * groupName)
{
    EmberAfStatus status;
    CHIP_ERROR err = CHIP_NO_ERROR;

    emberAfGroupsClusterPrint("RX: AddGroup 0x%2x, \"", groupId);
    emberAfGroupsClusterPrintString(groupName);
    emberAfGroupsClusterPrintln("\"");

    status = addEntryToGroupTable(emberAfCurrentEndpoint(), groupId, groupName);

    // For all networks, Add Group commands are only responded to when
    // they are addressed to a single device.
    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }

    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_GROUPS_CLUSTER_ID,
                                             ZCL_ADD_GROUP_RESPONSE_COMMAND_ID, (chip::app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(1), groupId));
        SuccessOrExit(err = commandObj->FinishCommand());
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfGroupsClusterViewGroupCallback(chip::app::Command * commandObj, GroupId groupId)
{
    EmberAfStatus status                                          = EMBER_ZCL_STATUS_NOT_FOUND;
    CHIP_ERROR err                                                = CHIP_NO_ERROR;
    uint8_t groupName[ZCL_GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH + 1] = { 0 };

    // Get the group name, if supported
    emberAfPluginGroupsServerGetGroupNameCallback(emberAfCurrentEndpoint(), groupId, groupName);

    emberAfGroupsClusterPrintln("RX: ViewGroup 0x%2x", groupId);

    // For all networks, View Group commands can only be addressed to a
    // single device.
    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }

    if (isGroupPresent(emberAfCurrentEndpoint(), groupId))
    {
        status = EMBER_ZCL_STATUS_SUCCESS;
    }

    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_GROUPS_CLUSTER_ID,
                                             ZCL_VIEW_GROUP_RESPONSE_COMMAND_ID, (chip::app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(1), groupId));
        SuccessOrExit(err = writer->PutString(TLV::ContextTag(2), reinterpret_cast<char *>(&groupName[1]), groupName[0]));
        SuccessOrExit(err = commandObj->FinishCommand());
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfGroupsClusterGetGroupMembershipCallback(chip::app::Command * commandObj, uint8_t groupCount, uint8_t * groupList)
{
    EmberStatus status = EMBER_ZCL_STATUS_FAILURE;
    uint8_t i, j;
    uint8_t count = 0;
    uint8_t list[EMBER_BINDING_TABLE_SIZE << 1];
    uint8_t listLen = 0;
    CHIP_ERROR err  = CHIP_NO_ERROR;

    emberAfGroupsClusterPrint("RX: GetGroupMembership 0x%x,", groupCount);
    for (i = 0; i < groupCount; i++)
    {
        emberAfGroupsClusterPrint(" [0x%2x]", emberAfGetInt16u(groupList + (i << 1), 0, 2));
    }
    emberAfGroupsClusterPrintln("");

    // For all networks, Get Group Membership commands may be sent either
    // unicast or broadcast (removing the ZLL-specific limitation to unicast).

    // When Group Count is zero, respond with a list of all active groups.
    // Otherwise, respond with a list of matches.
    if (groupCount == 0)
    {
        for (i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
        {
            EmberBindingTableEntry entry;
            status = emberGetBinding(i, &entry);
            if ((status == EMBER_SUCCESS) && (entry.type == EMBER_MULTICAST_BINDING) && (entry.local == emberAfCurrentEndpoint()))
            {
                list[listLen]     = EMBER_LOW_BYTE(entry.groupId);
                list[listLen + 1] = EMBER_HIGH_BYTE(entry.groupId);
                listLen           = static_cast<uint8_t>(listLen + 2);
                count++;
            }
        }
    }
    else
    {
        for (i = 0; i < groupCount; i++)
        {
            GroupId groupId = emberAfGetInt16u(groupList + (i << 1), 0, 2);
            for (j = 0; j < EMBER_BINDING_TABLE_SIZE; j++)
            {
                EmberBindingTableEntry entry;
                status = emberGetBinding(j, &entry);
                if ((status == EMBER_SUCCESS) && (entry.type == EMBER_MULTICAST_BINDING))
                {
                    if (entry.local == emberAfCurrentEndpoint() && entry.groupId == groupId)
                    {
                        list[listLen]     = EMBER_LOW_BYTE(groupId);
                        list[listLen + 1] = EMBER_HIGH_BYTE(groupId);
                        listLen           = static_cast<uint8_t>(listLen + 2);
                        count++;
                    }
                }
            }
        }
    }

    // Only send a response if the Group Count was zero or if one or more active
    // groups matched.  Otherwise, a Default Response is sent.
    if (groupCount == 0 || count != 0)
    {
        // A capacity of 0xFF means that it is unknown if any further groups may be
        // added.  Each group requires a binding and, because the binding table is
        // used for other purposes besides groups, we can't be sure what the
        // capacity will be in the future.
        {
            app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_GROUPS_CLUSTER_ID,
                                                 ZCL_GET_GROUP_MEMBERSHIP_RESPONSE_COMMAND_ID,
                                                 (chip::app::CommandPathFlags::kEndpointIdValid) };
            TLV::TLVWriter * writer          = nullptr;
            SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
            VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
            SuccessOrExit(err = writer->Put(TLV::ContextTag(0), static_cast<uint8_t>(0xff)));
            SuccessOrExit(err = writer->Put(TLV::ContextTag(1), count));
            SuccessOrExit(err = writer->PutBytes(TLV::ContextTag(2), list, listLen));
            SuccessOrExit(err = commandObj->FinishCommand());
        }
    }
    else
    {
        status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    }
    if (EMBER_SUCCESS != status)
    {
        emberAfGroupsClusterPrintln("Groups: failed to send %s: 0x%x",
                                    (groupCount == 0 || count != 0) ? "get_group_membership response" : "default_response", status);
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfGroupsClusterRemoveGroupCallback(chip::app::Command * commandObj, GroupId groupId)
{
    EmberAfStatus status;
    CHIP_ERROR err = CHIP_NO_ERROR;

    emberAfGroupsClusterPrintln("RX: RemoveGroup 0x%2x", groupId);

    status = removeEntryFromGroupTable(emberAfCurrentEndpoint(), groupId);

    // For all networks, Remove Group commands are only responded to when
    // they are addressed to a single device.
    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }

    // EMAPPFWKV2-1414: if we remove a group, we should remove any scene
    // associated with it. ZCL6: 3.6.2.3.5: "Note that if a group is
    // removed the scenes associated with that group SHOULD be removed."
    emberAfScenesClusterRemoveScenesInGroupCallback(emberAfCurrentEndpoint(), groupId);
    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_GROUPS_CLUSTER_ID,
                                             ZCL_REMOVE_GROUP_RESPONSE_COMMAND_ID,
                                             (chip::app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(1), groupId));
        SuccessOrExit(err = commandObj->FinishCommand());
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

bool emberAfGroupsClusterRemoveAllGroupsCallback(chip::app::Command * commandObj)
{
    EmberStatus sendStatus = EMBER_SUCCESS;
    uint8_t i;
    EndpointId endpoint = emberAfCurrentEndpoint();
    bool success        = true;

    emberAfGroupsClusterPrintln("RX: RemoveAllGroups");

    for (i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        EmberBindingTableEntry binding;
        if (emberGetBinding(i, &binding) == EMBER_SUCCESS)
        {
            if (binding.type == EMBER_MULTICAST_BINDING && endpoint == binding.local)
            {
                EmberStatus status = emberDeleteBinding(i);
                if (status != EMBER_SUCCESS)
                {
                    success = false;
                    emberAfGroupsClusterPrintln("ERR: Failed to delete binding (0x%x)", status);
                }
                else
                {
                    uint8_t groupName[ZCL_GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH + 1] = { 0 };
                    GroupId groupId                                               = binding.groupId;
                    emberAfPluginGroupsServerSetGroupNameCallback(endpoint, groupId, groupName);
                    success = true && success;

                    // EMAPPFWKV2-1414: if we remove a group, we should remove any scene
                    // associated with it. ZCL6: 3.6.2.3.5: "Note that if a group is
                    // removed the scenes associated with that group SHOULD be removed."
                    emberAfScenesClusterRemoveScenesInGroupCallback(endpoint, groupId);
                }
            }
        }
    }

    emberAfScenesClusterRemoveScenesInGroupCallback(emberAfCurrentEndpoint(), ZCL_SCENES_GLOBAL_SCENE_GROUP_ID);

    sendStatus = emberAfSendImmediateDefaultResponse(success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfGroupsClusterPrintln("Groups: failed to send %s: 0x%x", "default_response", sendStatus);
    }
    return true;
}

bool emberAfGroupsClusterAddGroupIfIdentifyingCallback(chip::app::Command * commandObj, GroupId groupId, uint8_t * groupName)
{
    EmberAfStatus status;
    EmberStatus sendStatus = EMBER_SUCCESS;

    emberAfGroupsClusterPrint("RX: AddGroupIfIdentifying 0x%2x, \"", groupId);
    emberAfGroupsClusterPrintString(groupName);
    emberAfGroupsClusterPrintln("\"");

    if (!emberAfIsDeviceIdentifying(emberAfCurrentEndpoint()))
    {
        // If not identifying, ignore add group -> success; not a failure.
        status = EMBER_ZCL_STATUS_SUCCESS;
    }
    else
    {
        status = addEntryToGroupTable(emberAfCurrentEndpoint(), groupId, groupName);
    }

    sendStatus = emberAfSendImmediateDefaultResponse(status);
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfGroupsClusterPrintln("Groups: failed to send %s: 0x%x", "default_response", sendStatus);
    }
    return true;
}

bool emberAfGroupsClusterEndpointInGroupCallback(EndpointId endpoint, GroupId groupId)
{
    return isGroupPresent(endpoint, groupId);
}

void emberAfGroupsClusterClearGroupTableCallback(EndpointId endpoint)
{
    uint8_t i, networkIndex = 0 /* emberGetCurrentNetwork() */;
    for (i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        EmberBindingTableEntry binding;
        if (emberGetBinding(i, &binding) == EMBER_SUCCESS && binding.type == EMBER_MULTICAST_BINDING &&
            (endpoint == binding.local || (endpoint == EMBER_BROADCAST_ENDPOINT && networkIndex == binding.networkIndex)))
        {
            EmberStatus status = emberDeleteBinding(i);
            if (status != EMBER_SUCCESS)
            {
                emberAfGroupsClusterPrintln("ERR: Failed to delete binding (0x%x)", status);
            }
        }
    }
}

static bool isGroupPresent(EndpointId endpoint, GroupId groupId)
{
    uint8_t i;

    for (i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        EmberBindingTableEntry binding;
        if (emberGetBinding(i, &binding) == EMBER_SUCCESS)
        {
            if (bindingGroupMatch(endpoint, groupId, &binding))
            {
                return true;
            }
        }
    }

    return false;
}

static bool bindingGroupMatch(EndpointId endpoint, GroupId groupId, EmberBindingTableEntry * entry)
{
    return (entry->type == EMBER_MULTICAST_BINDING && entry->groupId == groupId && entry->local == endpoint);
}

static uint8_t findGroupIndex(EndpointId endpoint, GroupId groupId)
{
    EmberStatus status;
    uint8_t i;

    for (i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        EmberBindingTableEntry entry;
        status = emberGetBinding(i, &entry);
        if ((status == EMBER_SUCCESS) && bindingGroupMatch(endpoint, groupId, &entry))
        {
            return i;
        }
    }
    return EMBER_AF_GROUP_TABLE_NULL_INDEX;
}

void emberAfPluginGroupsServerGetGroupNameCallback(EndpointId endpoint, GroupId groupId, uint8_t * groupName) {}

bool emberAfPluginGroupsServerGroupNamesSupportedCallback(EndpointId endpoint)
{
    return false;
}

void emberAfPluginGroupsServerSetGroupNameCallback(EndpointId endpoint, GroupId groupId, uint8_t * groupName) {}
