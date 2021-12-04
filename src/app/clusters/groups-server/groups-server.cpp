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
    GroupDataProvider * provider = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != provider, false);

    GroupDataProvider::GroupMapping mapping(endpointId, groupId);

    return provider->GroupMappingExists(fabricIndex, mapping);
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

    GroupDataProvider * provider = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != provider, EMBER_ZCL_STATUS_NOT_FOUND);
    GroupDataProvider::GroupMapping mapping(endpointId, groupId, groupName);

    CHIP_ERROR err = provider->AddGroupMapping(fabricIndex, mapping);
    if (CHIP_NO_ERROR == err)
    {
        return EMBER_ZCL_STATUS_SUCCESS;
    }
    else
    {
        ChipLogDetail(Zcl, "ERR: Failed to add mapping (end:%d, group:0x%x), err:%" CHIP_ERROR_FORMAT, endpointId, groupId,
                      err.Format());
        return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
    }
}

static EmberAfStatus GroupRemove(FabricIndex fabricIndex, EndpointId endpointId, GroupId groupId)
{
    VerifyOrReturnError(IsFabricGroupId(groupId), EMBER_ZCL_STATUS_INVALID_VALUE);
    VerifyOrReturnError(GroupExists(fabricIndex, endpointId, groupId), EMBER_ZCL_STATUS_NOT_FOUND);

    GroupDataProvider * provider = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != provider, EMBER_ZCL_STATUS_NOT_FOUND);
    GroupDataProvider::GroupMapping mapping(endpointId, groupId);

    CHIP_ERROR err = provider->RemoveGroupMapping(fabricIndex, mapping);
    if (CHIP_NO_ERROR == err)
    {
        return EMBER_ZCL_STATUS_SUCCESS;
    }
    else
    {
        ChipLogDetail(Zcl, "ERR: Failed to remove mapping (end:%d, group:0x%x), err:%" CHIP_ERROR_FORMAT, endpointId, groupId,
                      err.Format());
        return EMBER_ZCL_STATUS_NOT_FOUND;
    }
}

void emberAfGroupsClusterServerInitCallback(EndpointId endpointId)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    uint8_t nameSupport          = (provider && provider->HasGroupNamesSupport()) ? 0x80 : 0x00;

    EmberAfStatus status = Attributes::NameSupport::Set(endpointId, nameSupport);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogDetail(Zcl, "ERR: writing name support %x", status);
    }
}

bool emberAfGroupsClusterAddGroupCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                          const Commands::AddGroup::DecodableType & commandData)
{
    auto fabricIndex = GetFabricIndex(commandObj);
    auto groupId     = commandData.groupId;
    auto groupName   = commandData.groupName;
    auto endpointId  = commandPath.mEndpointId;

    EmberAfStatus status;
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(Zcl, "RX: AddGroup 0x%2x, \"%.*s\"", groupId, static_cast<int>(groupName.size()), groupName.data());

    status = GroupAdd(fabricIndex, endpointId, groupId, groupName);

    // For all networks, Add Group commands are only responded to when
    // they are addressed to a single device.
    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }

    {
        app::ConcreteCommandPath path = { emberAfCurrentEndpoint(), Groups::Id, Commands::AddGroupResponse::Id };
        TLV::TLVWriter * writer       = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(path));
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
    auto groupId     = commandData.groupId;
    auto endpointId  = commandPath.mEndpointId;

    GroupDataProvider::GroupMapping mapping;
    EmberAfStatus status = EMBER_ZCL_STATUS_NOT_FOUND;
    CHIP_ERROR err       = CHIP_NO_ERROR;
    size_t nameSize      = 0;

    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }

    if (IsFabricGroupId(groupId))
    {
        GroupDataProvider * provider = GetGroupDataProvider();
        VerifyOrReturnError(nullptr != provider, false);

        auto * groupIt = provider->IterateGroupMappings(fabricIndex, endpointId);
        VerifyOrReturnError(nullptr != groupIt, false);

        while (groupIt->Next(mapping))
        {
            if (mapping.group == groupId)
            {
                nameSize = strnlen(mapping.name, GroupDataProvider::GroupMapping::kGroupNameMax);
                status   = EMBER_ZCL_STATUS_SUCCESS;
                break;
            }
        }
        groupIt->Release();
    }
    else
    {
        status = EMBER_ZCL_STATUS_INVALID_VALUE;
    }

    {
        app::ConcreteCommandPath path = { endpointId, Groups::Id, Commands::ViewGroupResponse::Id };
        TLV::TLVWriter * writer       = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(path));
        VerifyOrExit((writer = commandObj->GetCommandDataIBTLVWriter()) != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = writer->Put(TLV::ContextTag(0), status));
        SuccessOrExit(err = writer->Put(TLV::ContextTag(1), groupId));
        SuccessOrExit(err = writer->PutString(TLV::ContextTag(2), mapping.name, static_cast<uint32_t>(nameSize)));
        SuccessOrExit(err = commandObj->FinishCommand());
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to encode response command.");
    }
    return true;
}

struct GroupMembershipResponse
{
    // A capacity of 0xFF means that it is unknown if any further groups MAY be added.
    static constexpr uint8_t kCapacityUnknown = 0xff;

    // Use GetCommandId instead of commandId directly to avoid naming conflict with CommandIdentification in ExecutionOfACommand
    static constexpr CommandId GetCommandId() { return Commands::GetGroupMembershipResponse::Id; }
    static constexpr ClusterId GetClusterId() { return Groups::Id; }

    GroupMembershipResponse(const Commands::GetGroupMembership::DecodableType & data,
                            GroupDataProvider::GroupMappingIterator * iter) :
        mCommandData(data),
        mIterator(iter)
    {}

    const Commands::GetGroupMembership::DecodableType & mCommandData;
    GroupDataProvider::GroupMappingIterator * mIterator = nullptr;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
    {
        TLV::TLVType outer;

        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));

        ReturnErrorOnFailure(app::DataModel::Encode(
            writer, TLV::ContextTag(to_underlying(Commands::GetGroupMembershipResponse::Fields::kCapacity)), kCapacityUnknown));
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

                ChipLogDetail(Zcl, "RX: GetGroupMembership [");
                if (0 == requestedCount)
                {
                    // 1.3.6.3.1. If the GroupList field is empty, the entity SHALL respond with all group identifiers of which the
                    // entity is a member.
                    while (mIterator->Next(mapping))
                    {
                        ReturnErrorOnFailure(app::DataModel::Encode(writer, TLV::AnonymousTag, mapping.group));
                        matchCount++;
                        ChipLogDetail(Zcl, " 0x%02" PRIx16, mapping.group);
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
                                ChipLogDetail(Zcl, " 0x%02" PRIx16, mapping.group);
                                break;
                            }
                        }
                        ReturnErrorOnFailure(iter.GetStatus());
                    }
                }
                ChipLogDetail(Zcl, "]");
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
    auto * provider    = GetGroupDataProvider();
    EmberStatus status = EMBER_ZCL_STATUS_FAILURE;
    CHIP_ERROR err     = CHIP_NO_ERROR;

    if (nullptr == provider)
    {
        status = emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
    else
    {
        auto iter = provider->IterateGroupMappings(fabricIndex, commandPath.mEndpointId);
        VerifyOrExit(nullptr != iter, err = CHIP_ERROR_NO_MEMORY);

        err = commandObj->AddResponseData(commandPath, GroupMembershipResponse(commandData, iter));
        iter->Release();
        if (CHIP_NO_ERROR == err)
        {
            status = EMBER_SUCCESS;
        }
    }
exit:
    if (EMBER_SUCCESS != status)
    {
        ChipLogDetail(Zcl, "Groups: failed to send get_group_membership response: 0x%x", status);
    }
    return true;
}

bool emberAfGroupsClusterRemoveGroupCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                             const Commands::RemoveGroup::DecodableType & commandData)
{
    auto fabricIndex = GetFabricIndex(commandObj);
    auto groupId     = commandData.groupId;
    auto endpointId  = commandPath.mEndpointId;
    EmberAfStatus status;
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(Zcl, "RX: RemoveGroup 0x%2x", groupId);

    status = GroupRemove(fabricIndex, endpointId, groupId);

    // For all networks, Remove Group commands are only responded to when
    // they are addressed to a single device.
    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }
#ifdef EMBER_AF_PLUGIN_SCENES
    // If a group is, removed the scenes associated with that group SHOULD be removed.
    emberAfScenesClusterRemoveScenesInGroupCallback(endpointId, groupId);
#endif
    {
        app::ConcreteCommandPath path = { endpointId, Groups::Id, Commands::RemoveGroupResponse::Id };
        TLV::TLVWriter * writer       = nullptr;
        SuccessOrExit(err = commandObj->PrepareCommand(path));
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
    FabricIndex fabricIndex      = GetFabricIndex(commandObj);
    GroupDataProvider * provider = GetGroupDataProvider();
    EndpointId endpointId        = commandPath.mEndpointId;
    EmberStatus sendStatus       = EMBER_SUCCESS;
    CHIP_ERROR err               = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;

    ChipLogDetail(Zcl, "RX: RemoveAllGroups");
    VerifyOrReturnError(nullptr != provider, true);

#ifdef EMBER_AF_PLUGIN_SCENES
    auto groupIter = provider->IterateGroupMappings(fabricIndex, endpointId);
    VerifyOrReturnError(nullptr != groupIter, true);

    GroupDataProvider::GroupMapping mapping;
    while (groupIter->Next(mapping))
    {
        emberAfScenesClusterRemoveScenesInGroupCallback(endpointId, mapping.group);
    }
    groupIter->Release();

    emberAfScenesClusterRemoveScenesInGroupCallback(endpointId, ZCL_SCENES_GLOBAL_SCENE_GROUP_ID);
#endif
    err = provider->RemoveAllGroupMappings(fabricIndex, endpointId);

    sendStatus = emberAfSendImmediateDefaultResponse(CHIP_NO_ERROR == err ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);
    if (EMBER_SUCCESS != sendStatus)
    {
        ChipLogDetail(Zcl, "Groups: failed to send %s: 0x%x", "default_response", sendStatus);
    }
    return true;
}

bool emberAfGroupsClusterAddGroupIfIdentifyingCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::AddGroupIfIdentifying::DecodableType & commandData)
{
    auto fabricIndex = GetFabricIndex(commandObj);
    auto groupId     = commandData.groupId;
    auto groupName   = commandData.groupName;
    auto endpointId  = commandPath.mEndpointId;

    EmberAfStatus status;
    EmberStatus sendStatus = EMBER_SUCCESS;

    ChipLogDetail(Zcl, "RX: AddGroupIfIdentifying 0x%2x, \"%.*s\"", groupId, static_cast<int>(groupName.size()), groupName.data());

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
        ChipLogDetail(Zcl, "Groups: failed to send %s: 0x%x", "default_response", sendStatus);
    }
    return true;
}

bool emberAfGroupsClusterEndpointInGroupCallback(chip::FabricIndex fabricIndex, EndpointId endpointId, GroupId groupId)
{
    return GroupExists(fabricIndex, endpointId, groupId);
}

void emberAfPluginGroupsServerSetGroupNameCallback(EndpointId endpoint, GroupId groupId, const CharSpan & groupName) {}

void MatterGroupsPluginServerInitCallback() {}
