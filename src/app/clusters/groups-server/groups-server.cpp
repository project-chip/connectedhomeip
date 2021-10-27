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
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/command-id.h>
#include <app/CommandHandler.h>
#include <app/util/af.h>
#include <credentials/GroupDataProvider.h>
#include <lib/support/CodeUtils.h>

#ifdef EMBER_AF_PLUGIN_SCENES
#include <app/clusters/scenes/scenes.h>
#endif // EMBER_AF_PLUGIN_SCENES

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Groups;
using namespace chip::Credentials;

static FabricIndex GetFabricIndex()
{
    return emberAfCurrentCommand()->source->GetSessionHandle().GetFabricIndex();
}

static bool isGroupIdValid(chip::GroupId id)
{
    return id > 0;
}

static bool isGroupPresent(EndpointId endpointId, GroupId groupId)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != groups, false);
    FabricIndex fabricIndex                 = GetFabricIndex();
    GroupDataProvider::GroupMapping mapping = { .endpoint = endpointId, .group = groupId };

    return groups->GroupMappingExists(fabricIndex, mapping);
}

static EmberAfStatus addEntryToGroupTable(EndpointId endpointId, GroupId groupId, const CharSpan & groupName)
{
    VerifyOrReturnError(isGroupIdValid(groupId), EMBER_ZCL_STATUS_INVALID_VALUE);

    // Check for duplicates.
    if (isGroupPresent(endpointId, groupId))
    {
        // Even if the group already exists, tell the application about the name,
        // so it can cope with renames.
        emberAfPluginGroupsServerSetGroupNameCallback(endpointId, groupId, groupName);
        return EMBER_ZCL_STATUS_DUPLICATE_EXISTS;
    }

    GroupDataProvider * groups = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != groups, EMBER_ZCL_STATUS_NOT_FOUND);
    FabricIndex fabricIndex                 = GetFabricIndex();
    GroupDataProvider::GroupMapping mapping = { .endpoint = endpointId, .group = groupId };

    CHIP_ERROR err = groups->AddGroupMapping(fabricIndex, mapping, nullptr);
    return (CHIP_NO_ERROR == err) ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
}

static EmberAfStatus removeEntryFromGroupTable(EndpointId endpointId, GroupId groupId)
{
    VerifyOrReturnError(isGroupIdValid(groupId), EMBER_ZCL_STATUS_INVALID_VALUE);

    if (isGroupPresent(endpointId, groupId))
    {
        GroupDataProvider * groups = GetGroupDataProvider();
        VerifyOrReturnError(nullptr != groups, EMBER_ZCL_STATUS_NOT_FOUND);
        FabricIndex fabricIndex                 = GetFabricIndex();
        GroupDataProvider::GroupMapping mapping = { .endpoint = endpointId, .group = groupId };

        CHIP_ERROR err = groups->RemoveGroupMapping(fabricIndex, mapping);
        if (CHIP_NO_ERROR == err)
        {
            emberAfPluginGroupsServerSetGroupNameCallback(endpointId, groupId, CharSpan());
            return EMBER_ZCL_STATUS_SUCCESS;
        }
        else
        {
            emberAfGroupsClusterPrintln("ERR: Failed to remove mapping (end:0x%x, group:0x%x)", endpointId, groupId);
            return EMBER_ZCL_STATUS_FAILURE;
        }
    }
    return EMBER_ZCL_STATUS_NOT_FOUND;
}

static void printGroups(EndpointId endpointId)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    if (groups)
    {
        FabricIndex fabricIndex                            = GetFabricIndex();
        GroupDataProvider::GroupMappingIterator * group_it = groups->IterateGroupMappings(fabricIndex, endpointId);
        size_t count                                       = group_it ? group_it->Count() : 0;
        size_t i                                           = 0;
        GroupId gid                                        = 0;

        emberAfGroupsClusterPrint("GROUPS, end:%u, count:%zu", endpointId, count);
        while (group_it->Next(gid))
        {
            emberAfGroupsClusterPrint("  group[%zu]:%04x", i, gid);
        }
    }
}

void emberAfGroupsClusterServerInitCallback(chip::EndpointId endpointId)
{
    uint8_t nameSupport = 0x00;

    EmberAfStatus status = Attributes::NameSupport::Set(endpointId, nameSupport);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfGroupsClusterPrint("ERR: writing name support %x", status);
    }
}

bool emberAfGroupsClusterAddGroupCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                          const Commands::AddGroup::DecodableType & commandData)
{
    auto & groupId   = commandData.groupId;
    auto & groupName = commandData.groupName;
    auto endpointId  = commandPath.mEndpointId;

    EmberAfStatus status;
    CHIP_ERROR err = CHIP_NO_ERROR;

    emberAfGroupsClusterPrintln("RX: AddGroup 0x%2x, \"%.*s\"", groupId, static_cast<int>(groupName.size()), groupName.data());

    status = addEntryToGroupTable(endpointId, groupId, groupName);

    // For all networks, Add Group commands are only responded to when
    // they are addressed to a single device.
    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }

    {
        app::CommandPathParams cmdParams = { emberAfCurrentEndpoint(), /* group id */ 0, Groups::Id, Commands::AddGroupResponse::Id,
                                             (app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
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

bool emberAfGroupsClusterViewGroupCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                           const Commands::ViewGroup::DecodableType & commandData)
{
    auto & groupId  = commandData.groupId;
    auto endpointId = commandPath.mEndpointId;

    EmberAfStatus status                                          = EMBER_ZCL_STATUS_NOT_FOUND;
    CHIP_ERROR err                                                = CHIP_NO_ERROR;
    uint8_t groupName[ZCL_GROUPS_CLUSTER_MAXIMUM_NAME_LENGTH + 1] = { 0 };

    // Get the group name, if supported
    emberAfPluginGroupsServerGetGroupNameCallback(endpointId, groupId, groupName);

    emberAfGroupsClusterPrintln("RX: ViewGroup 0x%02x", groupId);

    // For all networks, View Group commands can only be addressed to a
    // single device.
    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }

    if (!isGroupIdValid(groupId))
    {
        status = EMBER_ZCL_STATUS_INVALID_VALUE;
    }
    else if (isGroupPresent(endpointId, groupId))
    {
        status = EMBER_ZCL_STATUS_SUCCESS;
    }

    {
        app::CommandPathParams cmdParams = { endpointId, /* group id */ 0, Groups::Id, Commands::ViewGroupResponse::Id,
                                             (app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
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

bool emberAfGroupsClusterGetGroupMembershipCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::GetGroupMembership::DecodableType & commandData)
{
    auto fabricIndex  = GetFabricIndex();
    auto endpointId   = commandPath.mEndpointId;
    auto & groupCount = commandData.groupCount;
    auto & groupList  = commandData.groupList;
    auto * groups     = GetGroupDataProvider();
    auto * groupIt    = groups ? groups->IterateGroupMappings(fabricIndex, endpointId) : nullptr;

    EmberStatus status = EMBER_ZCL_STATUS_FAILURE;
    uint8_t count      = 0;
    uint8_t list[EMBER_BINDING_TABLE_SIZE << 1];
    uint8_t listLen = 0;
    CHIP_ERROR err  = CHIP_NO_ERROR;

    emberAfGroupsClusterPrint("RX: GetGroupMembership [");

    if (nullptr != groupIt)
    {
        count = 0;
    }
    // For all networks, Get Group Membership commands may be sent either
    // unicast or broadcast (removing the ZLL-specific limitation to unicast).

    // When Group Count is zero, respond with a list of all active groups.
    // Otherwise, respond with a list of matches.
    // TODO: https://github.com/project-chip/connectedhomeip/issues/10335
    else if (groupCount == 0)
    {
        // When Group Count is zero, respond with a list of all active groups.
        GroupId groupId = 0;
        while (groupIt->Next(groupId))
        {
            list[listLen]     = EMBER_LOW_BYTE(groupId);
            list[listLen + 1] = EMBER_HIGH_BYTE(groupId);
            listLen           = static_cast<uint8_t>(listLen + 2);
            count++;
        }
    }
    else
    {
        auto iter = groupList.begin();
        while (iter.Next())
        {
            GroupId groupId  = iter.GetValue();
            GroupId groupId2 = 0;
            emberAfGroupsClusterPrint(" 0x%02" PRIx16, groupId);
            while (groupIt->Next(groupId2))
            {
                if (groupId2 == groupId)
                {
                    list[listLen]     = EMBER_LOW_BYTE(groupId);
                    list[listLen + 1] = EMBER_HIGH_BYTE(groupId);
                    listLen           = static_cast<uint8_t>(listLen + 2);
                    count++;
                }
            }
        }
        err = iter.GetStatus();
    }

    emberAfGroupsClusterPrintln("]");

    if (err != CHIP_NO_ERROR)
    {
        status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_MALFORMED_COMMAND);
        err    = CHIP_NO_ERROR;
        goto exit;
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
            app::CommandPathParams cmdParams = { endpointId, /* group id */ 0, Groups::Id, Commands::GetGroupMembershipResponse::Id,
                                                 (app::CommandPathFlags::kEndpointIdValid) };
            TLV::TLVWriter * writer          = nullptr;
            SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
            VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
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
        emberAfGroupsClusterPrint(
            "Groups: failed to send %s: 0x%x",
            (commandData.groupCount == 0 || count != 0) ? "get_group_membership response" : "default_response", status);
    }
exit:
    printGroups(endpointId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    if (groupIt)
    {
        delete groupIt;
    }
    return true;
}

bool emberAfGroupsClusterRemoveGroupCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                             const Commands::RemoveGroup::DecodableType & commandData)
{
    auto & groupId  = commandData.groupId;
    auto endpointId = commandPath.mEndpointId;
    EmberAfStatus status;
    CHIP_ERROR err = CHIP_NO_ERROR;

    emberAfGroupsClusterPrintln("RX: RemoveGroup 0x%2x", groupId);

    status = removeEntryFromGroupTable(endpointId, groupId);

    // For all networks, Remove Group commands are only responded to when
    // they are addressed to a single device.
    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }

    // EMAPPFWKV2-1414: if we remove a group, we should remove any scene
    // associated with it. ZCL6: 3.6.2.3.5: "Note that if a group is
    // removed the scenes associated with that group SHOULD be removed."
    emberAfScenesClusterRemoveScenesInGroupCallback(endpointId, groupId);
    {
        app::CommandPathParams cmdParams = { endpointId, /* group id */ 0, Groups::Id, Commands::RemoveGroupResponse::Id,
                                             (app::CommandPathFlags::kEndpointIdValid) };
        TLV::TLVWriter * writer          = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(cmdParams));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
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

bool emberAfGroupsClusterRemoveAllGroupsCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::RemoveAllGroups::DecodableType & commandData)
{
    FabricIndex fabricIndex    = GetFabricIndex();
    GroupDataProvider * groups = GetGroupDataProvider();
    EndpointId endpointId      = commandPath.mEndpointId;
    EmberStatus sendStatus     = EMBER_SUCCESS;
    CHIP_ERROR err             = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;

    emberAfGroupsClusterPrintln("RX: RemoveAllGroups");

    if (groups)
    {
        auto groupIter  = groups->IterateGroupMappings(fabricIndex, endpointId);
        GroupId groupId = 0;

        while (groupIter->Next(groupId))
        {
            emberAfPluginGroupsServerSetGroupNameCallback(endpointId, groupId, CharSpan());

            // EMAPPFWKV2-1414: if we remove a group, we should remove any scene
            // associated with it. ZCL6: 3.6.2.3.5: "Note that if a group is
            // removed the scenes associated with that group SHOULD be removed."
            emberAfScenesClusterRemoveScenesInGroupCallback(endpointId, groupId);
        }

        emberAfScenesClusterRemoveScenesInGroupCallback(endpointId, ZCL_SCENES_GLOBAL_SCENE_GROUP_ID);
        err = groups->RemoveAllGroupMappings(fabricIndex, endpointId);
    }

    sendStatus = emberAfSendImmediateDefaultResponse(CHIP_NO_ERROR == err ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfGroupsClusterPrintln("Groups: failed to send %s: 0x%x", "default_response", sendStatus);
    }
    return true;
}

bool emberAfGroupsClusterAddGroupIfIdentifyingCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::AddGroupIfIdentifying::DecodableType & commandData)
{
    auto & groupId   = commandData.groupId;
    auto & groupName = commandData.groupName;
    auto endpointId  = commandPath.mEndpointId;

    EmberAfStatus status;
    EmberStatus sendStatus = EMBER_SUCCESS;

    emberAfGroupsClusterPrintln("RX: AddGroupIfIdentifying 0x%2x, \"%.*s\"", groupId, static_cast<int>(groupName.size()),
                                groupName.data());

    if (!emberAfIsDeviceIdentifying(endpointId))
    {
        // If not identifying, ignore add group -> success; not a failure.
        status = EMBER_ZCL_STATUS_SUCCESS;
    }
    else
    {
        status = addEntryToGroupTable(endpointId, groupId, groupName);
    }

    sendStatus = emberAfSendImmediateDefaultResponse(status);
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfGroupsClusterPrintln("Groups: failed to send %s: 0x%x", "default_response", sendStatus);
    }
    return true;
}

bool emberAfGroupsClusterEndpointInGroupCallback(EndpointId endpointId, GroupId groupId)
{
    return isGroupPresent(endpointId, groupId);
}

void emberAfPluginGroupsServerGetGroupNameCallback(EndpointId endpointId, GroupId groupId, uint8_t * groupName) {}

bool emberAfPluginGroupsServerGroupNamesSupportedCallback(EndpointId endpointId)
{
    return false;
}

void emberAfPluginGroupsServerSetGroupNameCallback(EndpointId endpointId, GroupId groupId, const CharSpan & groupName) {}
