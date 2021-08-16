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
#include <app/CommandHandler.h>
#include <app/clusters/scenes/scenes.h>
#include <app/common/gen/att-storage.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/attributes/Accessors.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/util/af.h>
#include <map>
#include <set>
#include <utility>

using namespace chip;
using namespace chip::app::Clusters::Groups;

static std::map<EndpointId, std::set<GroupId>> sGroupTable;

//------------------------------------------------------------------------------
// Local functions
//------------------------------------------------------------------------------

static bool isGroupIdValid(GroupId id)
{
    return id > 0;
}

static void printGroups()
{
    emberAfGroupsClusterPrint("GROUP TABLE(%u)", sGroupTable.size());

    std::map<EndpointId, std::set<GroupId>>::iterator it;
    std::set<GroupId>::iterator it2;

    for (it = sGroupTable.begin(); it != sGroupTable.end(); ++it)
    {
        for (it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            emberAfGroupsClusterPrintln("  end:%u, group:%u", it->first, *it2);
        }
    }
}

static EmberAfStatus checkGroup(EndpointId endpoint, GroupId groupId)
{
    if(!isGroupIdValid(groupId))
    {
        return EMBER_ZCL_STATUS_INVALID_VALUE;
    }  

    std::map<EndpointId, std::set<GroupId>>::iterator it = sGroupTable.find(endpoint);
    // Check if the endpoint is on the table AND the groupId is associated with the endpoint
    if((it != sGroupTable.end()) && (it->second.find(groupId) != it->second.end()))
    {
        return EMBER_ZCL_STATUS_SUCCESS;
    }
    else
    {
        return EMBER_ZCL_STATUS_NOT_FOUND;
    }
}

static EmberAfStatus addGroup(EndpointId endpoint, GroupId groupId, uint8_t * groupName)
{
    if(!isGroupIdValid(groupId))
    {
        return EMBER_ZCL_STATUS_INVALID_VALUE;
    }   

    std::map<EndpointId, std::set<GroupId>>::iterator it = sGroupTable.find(endpoint);
    EmberAfStatus status                                 = EMBER_ZCL_STATUS_SUCCESS;

    if (it == sGroupTable.end())
    {
        // New endpoint entry
        sGroupTable[endpoint].insert(groupId);
        emberAfPluginGroupsServerSetGroupNameCallback(endpoint, groupId, groupName);
    }
    else
    {
        std::set<GroupId>::iterator it2 = it->second.find(groupId);
        if (it2 == it->second.end())
        {
            // New group for the given endpoint
            it->second.insert(groupId);
            emberAfPluginGroupsServerSetGroupNameCallback(endpoint, groupId, groupName);
        }
        else
        {
            // The endpoint is already associated with the given group,
            // tell the application about the name, so it can cope with renames.
            emberAfPluginGroupsServerSetGroupNameCallback(endpoint, groupId, groupName);
            status = EMBER_ZCL_STATUS_DUPLICATE_EXISTS;
        }
    }
    printGroups();
    return status;
}

static EmberAfStatus removeGroup(EndpointId endpoint, GroupId groupId)
{
    if(!isGroupIdValid(groupId))
    {
        return EMBER_ZCL_STATUS_INVALID_VALUE;
    }  

    std::map<EndpointId, std::set<GroupId>>::iterator it = sGroupTable.find(endpoint);
    EmberAfStatus status                                 = EMBER_ZCL_STATUS_NOT_FOUND;

    if (it != sGroupTable.end())
    {
        std::set<GroupId>::iterator it2 = it->second.find(groupId);
        if (it2 != it->second.end())
        {
            // Group found, removed it from the endpoint
            it->second.erase(it2);

            // Remove name
            emberAfPluginGroupsServerSetGroupNameCallback(endpoint, groupId, nullptr);

            // EMAPPFWKV2-1414: if we remove a group, we should remove any scene
            // associated with it. ZCL6: 3.6.2.3.5: "Note that if a group is
            // removed the scenes associated with that group SHOULD be removed."
            emberAfScenesClusterRemoveScenesInGroupCallback(endpoint, groupId);
            status = EMBER_ZCL_STATUS_SUCCESS;
        }
    }
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        emberAfGroupsClusterPrintln("Groups Cluster: Failed to delete entry (%u, %u)", endpoint, groupId);
    }

    printGroups();
    return status;
}

//------------------------------------------------------------------------------
// Group Cluster callbacks
//------------------------------------------------------------------------------

/** @brief Groups Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfGroupsClusterServerInitCallback(chip::EndpointId endpoint)
{
    uint8_t nameSupport = 0;
    if (emberAfPluginGroupsServerGroupNamesSupportedCallback(endpoint))
    {
        // The high bit of Name Support indicates whether group names are supported.
        nameSupport = 0x80;
    }

    EmberAfStatus status = Attributes::SetNameSupport(endpoint, nameSupport);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfGroupsClusterPrintln("ERR: writing name support %x", status);
    }
}

/**
 * @brief  Cluster AddGroup Command callback (from client)
 */
bool emberAfGroupsClusterAddGroupCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj, uint16_t groupId,
                                          uint8_t * groupName)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    emberAfGroupsClusterPrint("AddGroup, end:%u, group:0x%2x \"%s\"", endpoint, groupId, groupName);

    // For all networks, Add Group commands are only responded to when they are addressed to a single device.
    if (emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST || emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST_REPLY)
    {
        EmberAfStatus status = addGroup(endpoint, groupId, groupName);

        app::CommandPathParams cmdParams = { endpoint, /* group id */ 0, ZCL_GROUPS_CLUSTER_ID, ZCL_ADD_GROUP_RESPONSE_COMMAND_ID,
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

/**
 * @brief  Cluster ViewGroup Command callback (from client)
 */
bool emberAfGroupsClusterViewGroupCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj, uint16_t groupId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // For all networks, View Group commands can only be addressed to a single device.
    if (emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST || emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST_REPLY)
    {
        uint8_t groupName[ZCL_GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH + 1] = { 0 };

        EmberAfStatus status = checkGroup(endpoint, groupId);
        if (EMBER_ZCL_STATUS_SUCCESS == status)
        {
            // Get the group name, if supported
            emberAfPluginGroupsServerGetGroupNameCallback(endpoint, groupId, groupName);
        }
        emberAfGroupsClusterPrint("ViewGroup, ep:%u, group:0x%2x \"%s\"", endpoint, groupId, groupName);

        app::CommandPathParams cmdParams = { endpoint, /* group id */ 0, ZCL_GROUPS_CLUSTER_ID, ZCL_VIEW_GROUP_RESPONSE_COMMAND_ID,
                                             (chip::app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(1), groupId));
        SuccessOrExit(err = writer->PutString(TLV::ContextTag(2), reinterpret_cast<char *>(&groupName[0])));
        SuccessOrExit(err = commandObj->FinishCommand());
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

/**
 * @brief  Cluster GetGroupMembership Command callback (from client)
 * 
 * WARNING: This function is UNTESTED. The groupList parameter is an array of structures,
 * which is currently not supported. In consequence, the groupList parameter fails to decode,
 * and this function is not being called.
 */
bool emberAfGroupsClusterGetGroupMembershipCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                    uint8_t groupCount,
                                                    /* TYPE WARNING: array array defaults to */ uint8_t * groupList)
{
    EmberStatus status = EMBER_ZCL_STATUS_FAILURE;
    CHIP_ERROR err     = CHIP_NO_ERROR;
    uint8_t i          = 0;

    emberAfGroupsClusterPrint("GetGroupMembership, ep:%u, count:%u", endpoint, groupCount);

    for (i = 0; i < groupCount; i++)
    {
        emberAfGroupsClusterPrint(" [0x%2x]", emberAfGetInt16u(groupList + (i << 1), 0, 2));
    }

    // For all networks, Get Group Membership commands may be sent either
    // unicast or broadcast (removing the ZLL-specific limitation to unicast).

    std::map<EndpointId, std::set<GroupId>>::iterator it = sGroupTable.find(endpoint);
    if (it == sGroupTable.end())
    {
        // Endpoint not in the table
        status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
        if (EMBER_SUCCESS != status)
        {
            emberAfGroupsClusterPrintln("Groups: failed to send %s: 0x%x", "default_response", status);
        }
    }
    else
    {
        // Endpoint in table, iterate groups
        std::set<GroupId>::iterator it2;
        uint8_t list[sizeof(groupCount)];
        uint8_t count    = std::min(groupCount, (uint8_t) it->second.size());
        uint8_t capacity = 0xff;

        for (it2 = it->second.begin(), i = 0; it2 != it->second.end() && i < sizeof(list); ++it2, ++i)
        {
            list[i] = static_cast<uint8_t>(*it2);
        }
        // Send response
        app::CommandPathParams cmdParams = { endpoint, /* group id */ 0, ZCL_GROUPS_CLUSTER_ID,
                                             ZCL_GET_GROUP_MEMBERSHIP_RESPONSE_COMMAND_ID,
                                             (chip::app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), capacity));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(1), count));
        SuccessOrExit(err = writer->PutBytes(TLV::ContextTag(2), list, count));
        SuccessOrExit(err = commandObj->FinishCommand());
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

/**
 * @brief  Cluster RemoveGroup Command callback (from client)
 */
bool emberAfGroupsClusterRemoveGroupCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj, uint16_t groupId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // For all networks, Remove Group commands are only responded to when
    // they are addressed to a single device.
    if (emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST || emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST_REPLY)
    {
        EmberAfStatus status = removeGroup(endpoint, groupId);

        app::CommandPathParams cmdParams = { endpoint, /* group id */ 0, ZCL_GROUPS_CLUSTER_ID,
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

/**
 * @brief  Cluster RemoveAllGroups Command callback (from client)
 */
bool emberAfGroupsClusterRemoveAllGroupsCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj)
{
    EmberStatus sendStatus = EMBER_SUCCESS;

    emberAfGroupsClusterPrint("RemoveAllGroups, ep:%u", endpoint);

    std::map<EndpointId, std::set<GroupId>>::iterator it = sGroupTable.find(endpoint);
    if (it != sGroupTable.end())
    {
        // Endpoint in table, iterate groups
        std::set<GroupId>::iterator it2;
        for (it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            GroupId groupId = *it2;

            // Clear group name
            emberAfPluginGroupsServerSetGroupNameCallback(endpoint, groupId, nullptr);

            // EMAPPFWKV2-1414: if we remove a group, we should remove any scene
            // associated with it. ZCL6: 3.6.2.3.5: "Note that if a group is
            // removed the scenes associated with that group SHOULD be removed."
            emberAfScenesClusterRemoveScenesInGroupCallback(endpoint, groupId);
        }
        // Remove endpoint from table along with all its groups
        sGroupTable.erase(it);
    }

    emberAfScenesClusterRemoveScenesInGroupCallback(endpoint, ZCL_SCENES_GLOBAL_SCENE_GROUP_ID);

    sendStatus = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfGroupsClusterPrintln("Groups: failed to send %s: 0x%x", "default_response", sendStatus);
    }
    return true;
}
/**
 * @brief  Cluster AddGroupIfIdentifying Command callback (from client)
 */
bool emberAfGroupsClusterAddGroupIfIdentifyingCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                       uint16_t groupId, uint8_t * groupName)
{
    EmberAfStatus status;
    EmberStatus sendStatus = EMBER_SUCCESS;

    if (emberAfIsDeviceIdentifying(endpoint))
    {
        emberAfGroupsClusterPrint("AddGroup, end:%u, group:0x%2x \"%s\"", endpoint, groupId, groupName);
        status = addGroup(endpoint, groupId, groupName);
    }
    else
    {
        // If not identifying, ignore add group -> success; not a failure.
        status = EMBER_ZCL_STATUS_SUCCESS;
    }

    sendStatus = emberAfSendImmediateDefaultResponse(status);
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfGroupsClusterPrintln("Groups: failed to send %s: 0x%x", "default_response", sendStatus);
    }

    return true;
}

//------------------------------------------------------------------------------
// Groups Server Plugins
//------------------------------------------------------------------------------

/** @brief Get Group Name
 *
 * This function returns the name of a group with the provided group ID, should
 * it exist.
 *
 * @param endpoint Endpoint Ver.: always
 * @param groupId Group ID Ver.: always
 * @param groupName Group Name Ver.: always
 */
void emberAfPluginGroupsServerGetGroupNameCallback(chip::EndpointId endpoint, chip::GroupId groupId, uint8_t * groupName)
{
}

/** @brief Set Group Name
 *
 * This function sets the name of a group with the provided group ID.
 *
 * @param endpoint Endpoint Ver.: always
 * @param groupId Group ID Ver.: always
 * @param groupName Group Name Ver.: always
 */
void emberAfPluginGroupsServerSetGroupNameCallback(chip::EndpointId endpoint, chip::GroupId groupId, uint8_t * groupName)
{
}

/** @brief Group Names Supported
 *
 * This function returns whether or not group names are supported.
 *
 * @param endpoint Endpoint Ver.: always
 */
bool emberAfPluginGroupsServerGroupNamesSupportedCallback(chip::EndpointId endpoint)
{
    return false;
}

/** @brief Groups Cluster Endpoint In Group
 *
 * This function is called by the framework when it needs to determine if an
 * endpoint is a member of a group.  The application should return true if the
 * endpoint is a member of the group and false otherwise.
 *
 * @param endpoint The endpoint.  Ver.: always
 * @param groupId The group identifier.  Ver.: always
 */
bool emberAfGroupsClusterEndpointInGroupCallback(chip::EndpointId endpoint, chip::GroupId groupId)
{
    return checkGroup(endpoint, groupId);
}

/** @brief Scenes Cluster Remove Scenes In Group
 *
 * This function removes the scenes from a specified group. It is called
 * whenever a group is removed from the GroupsTable.
 *
 *
 * @param endpoint Endpoint  Ver.: always
 * @param groupId Group ID  Ver.: always
 */
void __attribute__((weak)) emberAfScenesClusterRemoveScenesInGroupCallback(EndpointId endpoint, chip::GroupId groupId) {}
