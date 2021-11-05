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
#include <inttypes.h>
#include <lib/support/CodeUtils.h>

#ifdef EMBER_AF_PLUGIN_SCENES
#include <app/clusters/scenes/scenes.h>
#endif // EMBER_AF_PLUGIN_SCENES

using namespace chip;
using namespace app::Clusters;
using namespace app::Clusters::Groups;
using namespace chip::Credentials;

static FabricIndex GetFabricIndex(app::CommandHandler * commandObj)
{
    VerifyOrReturnError(nullptr != commandObj, 0);
    VerifyOrReturnError(nullptr != commandObj->GetExchangeContext(), 0);
    return commandObj->GetExchangeContext()->GetSessionHandle().GetFabricIndex();
}

static bool GroupExists(FabricIndex fabricIndex, EndpointId endpointId, GroupId groupId)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != groups, false);

    GroupDataProvider::GroupMapping mapping(endpointId, groupId);

    return groups->GroupMappingExists(fabricIndex, mapping);
}

static bool GroupFind(FabricIndex fabricIndex, EndpointId endpointId, GroupId groupId, CharSpan & name)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != groups, false);

    auto * groupIt = groups->IterateGroupMappings(fabricIndex, endpointId);
    VerifyOrReturnError(nullptr != groupIt, false);

    GroupDataProvider::GroupMapping mapping;
    bool found = false;
    while (!found && groupIt->Next(mapping))
    {
        if (mapping.group == groupId)
        {
            name  = mapping.name;
            found = true;
        }
    }
    groupIt->Release();
    return found;
}

static EmberAfStatus GroupAdd(FabricIndex fabricIndex, EndpointId endpointId, GroupId groupId, const CharSpan & groupName)
{
    VerifyOrReturnError(IsFabricGroupId(groupId), EMBER_ZCL_STATUS_INVALID_VALUE);

    // Check for duplicates.
    if (GroupExists(fabricIndex, endpointId, groupId))
    {
        // Even if the group already exists, tell the application about the name,
        // so it can cope with renames.
        return EMBER_ZCL_STATUS_DUPLICATE_EXISTS;
    }

    GroupDataProvider * groups = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != groups, EMBER_ZCL_STATUS_NOT_FOUND);
    GroupDataProvider::GroupMapping mapping(endpointId, groupId, groupName);

    CHIP_ERROR err = groups->AddGroupMapping(fabricIndex, mapping);
    if (CHIP_NO_ERROR == err)
    {
        return EMBER_ZCL_STATUS_SUCCESS;
    }
    else
    {
        emberAfGroupsClusterPrint("ERR: Failed to add mapping (end:%d, group:0x%x), err:%" CHIP_ERROR_FORMAT, endpointId, groupId,
                                  err.Format());
        return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
    }
}

static EmberAfStatus GroupRemove(FabricIndex fabricIndex, EndpointId endpointId, GroupId groupId)
{
    VerifyOrReturnError(IsFabricGroupId(groupId), EMBER_ZCL_STATUS_INVALID_VALUE);

    if (GroupExists(fabricIndex, endpointId, groupId))
    {
        GroupDataProvider * groups = GetGroupDataProvider();
        VerifyOrReturnError(nullptr != groups, EMBER_ZCL_STATUS_NOT_FOUND);
        GroupDataProvider::GroupMapping mapping(endpointId, groupId);

        CHIP_ERROR err = groups->RemoveGroupMapping(fabricIndex, mapping);
        if (CHIP_NO_ERROR == err)
        {
            return EMBER_ZCL_STATUS_SUCCESS;
        }
        else
        {
            emberAfGroupsClusterPrint("ERR: Failed to remove mapping (end:%d, group:0x%x), err:%" CHIP_ERROR_FORMAT, endpointId,
                                      groupId, err.Format());
            return EMBER_ZCL_STATUS_FAILURE;
        }
    }
    return EMBER_ZCL_STATUS_NOT_FOUND;
}

void emberAfGroupsClusterServerInitCallback(EndpointId endpointId)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    uint8_t nameSupport        = (groups && groups->HasGroupNamesSupport()) ? 0x80 : 0x00;

    EmberAfStatus status = Attributes::NameSupport::Set(endpointId, nameSupport);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfGroupsClusterPrint("ERR: writing name support %x", status);
    }
}

bool emberAfGroupsClusterAddGroupCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                          const Commands::AddGroup::DecodableType & commandData)
{
    auto fabricIndex = GetFabricIndex(commandObj);
    auto & groupId   = commandData.groupId;
    auto & groupName = commandData.groupName;
    auto endpointId  = commandPath.mEndpointId;

    EmberAfStatus status;
    CHIP_ERROR err = CHIP_NO_ERROR;

    emberAfGroupsClusterPrintln("RX: AddGroup 0x%2x, \"%.*s\"", groupId, static_cast<int>(groupName.size()), groupName.data());

    status = GroupAdd(fabricIndex, endpointId, groupId, groupName);

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
    auto fabricIndex = GetFabricIndex(commandObj);
    auto & groupId   = commandData.groupId;
    auto endpointId  = commandPath.mEndpointId;

    EmberAfStatus status = EMBER_ZCL_STATUS_NOT_FOUND;
    CHIP_ERROR err       = CHIP_NO_ERROR;
    CharSpan groupName;

    emberAfGroupsClusterPrintln("RX: ViewGroup 0x%02x", groupId);

    // For all networks, View Group commands can only be addressed to a
    // single device.
    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }

    if (!IsFabricGroupId(groupId))
    {
        status = EMBER_ZCL_STATUS_INVALID_VALUE;
    }
    else if (GroupFind(fabricIndex, endpointId, groupId, groupName))
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
        SuccessOrExit(err = writer->PutString(TLV::ContextTag(2), groupName.data(), static_cast<uint32_t>(groupName.size())));
        SuccessOrExit(err = commandObj->FinishCommand());
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

struct GetGroupMembershipResponse
{
public:
    // Use GetCommandId instead of commandId directly to avoid naming conflict with CommandIdentification in ExecutionOfACommand
    static constexpr CommandId GetCommandId() { return Commands::GetGroupMembershipResponse::Id; }
    static constexpr ClusterId GetClusterId() { return Groups::Id; }

    GetGroupMembershipResponse(const Commands::GetGroupMembership::DecodableType & data,
                               GroupDataProvider::GroupMappingIterator * iter) :
        mCommandData(data),
        mIterator(iter)
    {}

    const Commands::GetGroupMembership::DecodableType & mCommandData;
    GroupDataProvider::GroupMappingIterator * mIterator = nullptr;

    uint8_t mCapacity = 0xff;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
    {
        TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
        // A capacity of 0xFF means that it is unknown if any further groups may be
        // added.  Each group requires a binding and, because the binding table is
        // used for other purposes besides groups, we can't be sure what the
        // capacity will be in the future.
        ReturnErrorOnFailure(app::DataModel::Encode(
            writer, TLV::ContextTag(to_underlying(Commands::GetGroupMembershipResponse::Fields::kCapacity)), mCapacity));
        {
            TLV::TLVType type;
            ReturnErrorOnFailure(
                writer.StartContainer(TLV::ContextTag(to_underlying(Commands::GetGroupMembershipResponse::Fields::kGroupList)),
                                      TLV::kTLVType_Array, type));
            {
                GroupDataProvider::GroupMapping mapping;
                size_t requestedCount = 0;
                size_t matchCount     = 0;
                ReturnErrorOnFailure(mCommandData.groupList.ComputeSize(&requestedCount));

                emberAfGroupsClusterPrint("RX: GetGroupMembership [");
                if (0 == requestedCount)
                {
                    // 1.3.6.3.1. If the GroupList field is empty, the entity SHALL respond with all group identifiers of which the
                    // entity is a member.
                    while (mIterator->Next(mapping))
                    {
                        ReturnErrorOnFailure(app::DataModel::Encode(writer, TLV::AnonymousTag, mapping.group));
                        matchCount++;
                        emberAfGroupsClusterPrint(" 0x%02" PRIx16, mapping.group);
                    }
                }
                else
                {
                    while (mIterator->Next(mapping))
                    {
                        auto iter = mCommandData.groupList.begin();
                        while (iter.Next())
                        {
                            if (mapping.group == iter.GetValue())
                            {
                                ReturnErrorOnFailure(app::DataModel::Encode(writer, TLV::AnonymousTag, mapping.group));
                                matchCount++;
                                emberAfGroupsClusterPrint(" 0x%02" PRIx16, mapping.group);
                                break;
                            }
                        }
                        ReturnErrorOnFailure(iter.GetStatus());
                    }
                }
                emberAfGroupsClusterPrintln("]");

                // 1.3.6.3.1: If the GroupList field contains one or more group-id’s but does not contain any group
                // of which the entity is a member, the entity SHALL only respond if the command is unicast
                VerifyOrReturnError((0 == requestedCount) || (matchCount > 0) ||
                                        (emberAfCurrentCommand()->type == EMBER_INCOMING_UNICAST),
                                    CHIP_ERROR_INVALID_DATA_LIST);
            }
            ReturnErrorOnFailure(writer.EndContainer(type));
        }
        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
};

bool emberAfGroupsClusterGetGroupMembershipCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::GetGroupMembership::DecodableType & commandData)
{
    auto fabricIndex   = GetFabricIndex(commandObj);
    auto * groups      = GetGroupDataProvider();
    EmberStatus status = EMBER_ZCL_STATUS_FAILURE;
    CHIP_ERROR err     = CHIP_NO_ERROR;

    if (nullptr == groups)
    {
        status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
    else
    {
        auto iter = groups->IterateGroupMappings(fabricIndex, commandPath.mEndpointId);
        VerifyOrExit(nullptr != iter, err = CHIP_ERROR_NO_MEMORY);

        err = commandObj->AddResponseData(commandPath, GetGroupMembershipResponse(commandData, iter));
        iter->Release();
        if (CHIP_NO_ERROR == err)
        {
            status = EMBER_SUCCESS;
        }
    }
exit:
    if (EMBER_SUCCESS != status)
    {
        emberAfGroupsClusterPrint("Groups: failed to send get_group_membership response: 0x%x", status);
    }
    return true;
}

bool emberAfGroupsClusterRemoveGroupCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                             const Commands::RemoveGroup::DecodableType & commandData)
{
    auto fabricIndex = GetFabricIndex(commandObj);
    auto & groupId   = commandData.groupId;
    auto endpointId  = commandPath.mEndpointId;
    EmberAfStatus status;
    CHIP_ERROR err = CHIP_NO_ERROR;

    emberAfGroupsClusterPrintln("RX: RemoveGroup 0x%2x", groupId);

    status = GroupRemove(fabricIndex, endpointId, groupId);

    // For all networks, Remove Group commands are only responded to when
    // they are addressed to a single device.
    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }
#ifdef EMBER_AF_PLUGIN_SCENES
    // EMAPPFWKV2-1414: if we remove a group, we should remove any scene
    // associated with it. ZCL6: 3.6.2.3.5: "Note that if a group is
    // removed the scenes associated with that group SHOULD be removed."
    emberAfScenesClusterRemoveScenesInGroupCallback(endpointId, groupId);
#endif
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
    FabricIndex fabricIndex    = GetFabricIndex(commandObj);
    GroupDataProvider * groups = GetGroupDataProvider();
    EndpointId endpointId      = commandPath.mEndpointId;
    EmberStatus sendStatus     = EMBER_SUCCESS;
    CHIP_ERROR err             = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;

    emberAfGroupsClusterPrintln("RX: RemoveAllGroups");

    if (groups)
    {
#ifdef EMBER_AF_PLUGIN_SCENES
        auto groupIter = groups->IterateGroupMappings(fabricIndex, endpointId);
        if (nullptr != groupIter)
        {
            GroupDataProvider::GroupMapping mapping;
            while (groupIter->Next(mapping))
            {
                // EMAPPFWKV2-1414: if we remove a group, we should remove any scene
                // associated with it. ZCL6: 3.6.2.3.5: "Note that if a group is
                // removed the scenes associated with that group SHOULD be removed."
                emberAfScenesClusterRemoveScenesInGroupCallback(endpointId, mapping.group);
            }
            groupIter->Release();
        }
        emberAfScenesClusterRemoveScenesInGroupCallback(endpointId, ZCL_SCENES_GLOBAL_SCENE_GROUP_ID);
#endif
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
    auto fabricIndex = GetFabricIndex(commandObj);
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
        status = GroupAdd(fabricIndex, endpointId, groupId, groupName);
    }

    sendStatus = emberAfSendImmediateDefaultResponse(status);
    if (EMBER_SUCCESS != sendStatus)
    {
        emberAfGroupsClusterPrintln("Groups: failed to send %s: 0x%x", "default_response", sendStatus);
    }
    return true;
}

bool emberAfGroupsClusterEndpointInGroupCallback(chip::FabricIndex fabricIndex, EndpointId endpointId, GroupId groupId)
{
    return GroupExists(fabricIndex, endpointId, groupId);
}

bool emberAfPluginGroupsServerGroupNamesSupportedCallback(EndpointId endpointId)
{
    return false;
}

void emberAfPluginGroupsServerSetGroupNameCallback(EndpointId endpoint, GroupId groupId, const CharSpan & groupName) {}

void MatterGroupsPluginServerInitCallback() {}
