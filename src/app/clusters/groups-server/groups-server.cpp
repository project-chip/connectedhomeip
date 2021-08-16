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

#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app/CommandHandler.h>
#include <app/util/af.h>

#include <app/clusters/scenes/scenes.h>
#include <platform/KeyValueStoreManager.h>
#include <stdio.h>
#include <utility>

using namespace chip;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::Groups;

#define STORAGE_KEY_SIZE_MAX 32

//------------------------------------------------------------------------------
// Local functions
//------------------------------------------------------------------------------

static bool isGroupIdValid(GroupId id)
{
    return id > 0;
}

static void buildStoreKey(char key[STORAGE_KEY_SIZE_MAX], EndpointId endpoint)
{
    snprintf(key, STORAGE_KEY_SIZE_MAX, "groups_table:%04x", endpoint);
}

static void printGroups(EndpointId endpoint)
{
    char store_key[STORAGE_KEY_SIZE_MAX];
    GroupId count = 0;

    buildStoreKey(store_key, endpoint);

    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(store_key, &count, sizeof(GroupId));
    if (CHIP_NO_ERROR == err)
    {
        GroupId groups[count + 1];

        emberAfGroupsClusterPrint("GROUPS, end:%u, count:%u, err:%d", endpoint, count, err);

        err = PersistedStorage::KeyValueStoreMgr().Get(store_key, groups, sizeof(groups));
        if (CHIP_NO_ERROR == err)
        {
            for (uint16_t i = 1; i <= count; ++i)
            {
                emberAfGroupsClusterPrint("  group[%u]:%04x", i - 1, groups[i]);
            }
        }
    }
}

static EmberAfStatus isGroupMember(EndpointId endpoint, GroupId groupId)
{
    printGroups(endpoint);

    if (!isGroupIdValid(groupId))
    {
        return EMBER_ZCL_STATUS_INVALID_VALUE;
    }
    else
    {
        char store_key[STORAGE_KEY_SIZE_MAX];
        GroupId count = 0;

        buildStoreKey(store_key, endpoint);

        CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(store_key, &count, sizeof(GroupId));
        if (CHIP_NO_ERROR == err)
        {
            GroupId groups[count + 1];
            err = PersistedStorage::KeyValueStoreMgr().Get(store_key, groups, sizeof(groups));
            if (CHIP_NO_ERROR == err)
            {
                for (uint16_t i = 1; i <= count; ++i)
                {
                    if (groups[i] == groupId)
                    {
                        return EMBER_ZCL_STATUS_SUCCESS;
                    }
                }
            }
        }
        return EMBER_ZCL_STATUS_NOT_FOUND;
    }
}

static EmberAfStatus addGroup(EndpointId endpoint, GroupId groupId, uint8_t * groupName)
{
    if (!isGroupIdValid(groupId))
    {
        return EMBER_ZCL_STATUS_INVALID_VALUE;
    }

    char store_key[STORAGE_KEY_SIZE_MAX];
    GroupId count = 0;

    buildStoreKey(store_key, endpoint);

    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(store_key, &count, sizeof(GroupId));
    if (CHIP_NO_ERROR == err)
    {
        // Existing endpoint
        GroupId groups[count + 2]; // group_count + n_groups + new_group
        err = PersistedStorage::KeyValueStoreMgr().Get(store_key, groups, (count + 1) * sizeof(GroupId));
        if (CHIP_NO_ERROR != err)
        {
            // Cannot get endpoint's groups
            return EMBER_ZCL_STATUS_FAILURE;
        }

        // Check endpoint's groups
        for (uint16_t i = 1; i <= count; ++i)
        {
            if (groups[i] == groupId)
            {
                return EMBER_ZCL_STATUS_DUPLICATE_EXISTS;
            }
        }
        groups[0]         = static_cast<GroupId>(count + 1);
        groups[count + 1] = groupId;
        err               = PersistedStorage::KeyValueStoreMgr().Put(store_key, groups, sizeof(groups));
    }
    else
    {
        // New endpoint
        GroupId groups[2] = { 1, groupId };
        err               = PersistedStorage::KeyValueStoreMgr().Put(store_key, groups, sizeof(groups));
    }

    if (CHIP_NO_ERROR != err)
    {
        // Cannot store endpoint's groups
        return EMBER_ZCL_STATUS_FAILURE;
    }

    printGroups(endpoint);
    return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus removeGroup(EndpointId endpoint, GroupId groupId)
{
    if (!isGroupIdValid(groupId))
    {
        return EMBER_ZCL_STATUS_INVALID_VALUE;
    }

    char store_key[STORAGE_KEY_SIZE_MAX];
    GroupId count = 0;

    buildStoreKey(store_key, endpoint);

    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(store_key, &count, sizeof(GroupId));
    if (CHIP_NO_ERROR != err)
    {
        return EMBER_ZCL_STATUS_NOT_FOUND;
    }
    else
    {
        // Existing endpoint
        GroupId groups[count + 1];
        err = PersistedStorage::KeyValueStoreMgr().Get(store_key, groups, sizeof(groups));
        if (CHIP_NO_ERROR != err)
        {
            // Cannot get endpoint's groups
            return EMBER_ZCL_STATUS_FAILURE;
        }

        // Remove group
        bool found = false;
        for (uint16_t i = 1; i <= count; ++i)
        {
            if (found)
            {
                // Shift-down remaining groups
                groups[i - 1] = groups[i];
            }
            else if (groups[i] == groupId)
            {
                found = true;
                // EMAPPFWKV2-1414: if we remove a group, we should remove any scene
                // associated with it. ZCL6: 3.6.2.3.5: "Note that if a group is
                // removed the scenes associated with that group SHOULD be removed."
                emberAfScenesClusterRemoveScenesInGroupCallback(endpoint, groupId);
            }
        }
        if (!found)
        {
            return EMBER_ZCL_STATUS_NOT_FOUND;
        }
        groups[0] = --count;
        err       = PersistedStorage::KeyValueStoreMgr().Put(store_key, groups, (count + 1) * sizeof(GroupId));
    }

    printGroups(endpoint);
    return CHIP_NO_ERROR == err ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
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
    uint8_t nameSupport = 0x00;

    EmberAfStatus status = Attributes::SetNameSupport(endpoint, nameSupport);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfGroupsClusterPrint("ERR: writing name support %x", status);
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

        EmberAfStatus status = isGroupMember(endpoint, groupId);

        emberAfGroupsClusterPrint("ViewGroup, end:%u, group:0x%2x \"%s\"", endpoint, groupId, groupName);

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
    uint8_t count      = 0;

    if (!groupList)
    {
        status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    }
    else
    {
        char store_key[STORAGE_KEY_SIZE_MAX];
        GroupId stored_count = 0;

        buildStoreKey(store_key, endpoint);
        err = PersistedStorage::KeyValueStoreMgr().Get(store_key, &stored_count, sizeof(GroupId));
        if (CHIP_NO_ERROR != err)
        {
            status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
        }
        else
        {
            // For all networks, Get Group Membership commands may be sent either
            // unicast or broadcast (removing the ZLL-specific limitation to unicast).
            GroupId groups[stored_count + 1];

            err = PersistedStorage::KeyValueStoreMgr().Get(store_key, groups, (stored_count + 1) * sizeof(GroupId));
            if (CHIP_NO_ERROR != err)
            {
                status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
            }
            else
            {
                uint8_t list[stored_count * 2];
                uint8_t listLen = 0;
                uint8_t i;
                if (0 == groupCount)
                {
                    // When Group Count is zero, respond with a list of all active groups.
                    for (i = 1; i <= stored_count; ++i)
                    {
                        list[listLen]     = EMBER_LOW_BYTE(groups[i]);
                        list[listLen + 1] = EMBER_HIGH_BYTE(groups[i + 1]);
                        listLen           = static_cast<uint8_t>(listLen + 2);
                        count++;
                    }
                }
                else
                {
                    uint8_t j;
                    // Otherwise, respond with a list of matches.
                    for (i = 0; i < groupCount; ++i)
                    {
                        GroupId groupId = emberAfGetInt16u(groupList + (i << 1), 0, 2);
                        for (j = 1; j <= stored_count; ++j)
                        {
                            if (groups[j] == groupId)
                            {
                                list[listLen]     = EMBER_LOW_BYTE(groupId);
                                list[listLen + 1] = EMBER_HIGH_BYTE(groupId);
                                listLen           = static_cast<uint8_t>(listLen + 2);
                                count++;
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
                    app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, ZCL_GROUPS_CLUSTER_ID,
                                                         ZCL_GET_GROUP_MEMBERSHIP_RESPONSE_COMMAND_ID,
                                                         (app::CommandPathFlags::kEndpointIdValid) };
                    TLV::TLVWriter * writer          = nullptr;
                    SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
                    VerifyOrExit((writer = commandObj->GetCommandDataElementTLVWriter()) != nullptr,
                                 err = CHIP_ERROR_INCORRECT_STATE);
                    SuccessOrExit(err = writer->Put(TLV::ContextTag(0), static_cast<uint8_t>(0xff)));
                    SuccessOrExit(err = writer->Put(TLV::ContextTag(1), count));
                    SuccessOrExit(err = writer->PutBytes(TLV::ContextTag(2), list, listLen));
                    SuccessOrExit(err = commandObj->FinishCommand());
                }
                else
                {
                    status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
                }
            }
        }
    }

    if (EMBER_SUCCESS != status)
    {
        emberAfGroupsClusterPrint("Groups: failed to send %s: 0x%x",
                                  (groupCount == 0 || count != 0) ? "get_group_membership response" : "default_response", status);
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
    emberAfGroupsClusterPrint("RemoveGroup, end:%u, gid:%u", endpoint, groupId);
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

    char store_key[STORAGE_KEY_SIZE_MAX];
    GroupId count = 0;

    emberAfGroupsClusterPrint("RemoveAllGroups, end:%u", endpoint);

    buildStoreKey(store_key, endpoint);

    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(store_key, &count, sizeof(GroupId));
    if (CHIP_NO_ERROR == err)
    {
        GroupId groups[count + 1];

        err = PersistedStorage::KeyValueStoreMgr().Get(store_key, groups, (count + 1) * sizeof(GroupId));
        if (CHIP_NO_ERROR == err)
        {
            for (uint16_t i = 1; i <= count; ++i)
            {
                // EMAPPFWKV2-1414: if we remove a group, we should remove any scene
                // associated with it. ZCL6: 3.6.2.3.5: "Note that if a group is
                // removed the scenes associated with that group SHOULD be removed."
                emberAfScenesClusterRemoveScenesInGroupCallback(endpoint, groups[i]);
            }
        }
        PersistedStorage::KeyValueStoreMgr().Delete(store_key);
    }

    emberAfScenesClusterRemoveScenesInGroupCallback(endpoint, ZCL_SCENES_GLOBAL_SCENE_GROUP_ID);

    sendStatus = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfGroupsClusterPrint("Groups: failed to send %s: 0x%x", "default_response", sendStatus);
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
        emberAfGroupsClusterPrint("Groups: failed to send %s: 0x%x", "default_response", sendStatus);
    }

    return true;
}

//------------------------------------------------------------------------------
// Groups Server Plugin
//------------------------------------------------------------------------------

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
    return isGroupMember(endpoint, groupId);
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
