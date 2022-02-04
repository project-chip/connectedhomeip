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

static bool GroupExists(FabricIndex fabricIndex, EndpointId endpointId, GroupId groupId)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != provider, false);

    return provider->HasEndpoint(fabricIndex, groupId, endpointId);
}

static EmberAfStatus GroupAdd(FabricIndex fabricIndex, EndpointId endpointId, GroupId groupId, const CharSpan & groupName)
{
    VerifyOrReturnError(IsFabricGroupId(groupId), EMBER_ZCL_STATUS_INVALID_VALUE);

    GroupDataProvider * provider = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != provider, EMBER_ZCL_STATUS_NOT_FOUND);

    CHIP_ERROR err = provider->SetGroupInfo(fabricIndex, GroupDataProvider::GroupInfo(groupId, groupName));
    if (CHIP_NO_ERROR == err)
    {
        err = provider->AddEndpoint(fabricIndex, groupId, endpointId);
    }
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

    CHIP_ERROR err = provider->RemoveEndpoint(fabricIndex, groupId, endpointId);
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
    // The most significant bit of the NameSupport attribute indicates whether or not group names are supported
    static constexpr uint8_t nameSupport = 0x80;
    EmberAfStatus status                 = Attributes::NameSupport::Set(endpointId, nameSupport);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogDetail(Zcl, "ERR: writing name support %x", status);
    }
}

bool emberAfGroupsClusterAddGroupCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                          const Commands::AddGroup::DecodableType & commandData)
{
    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    Groups::Commands::AddGroupResponse::Type response;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // For all networks, Add Group commands are only responded to when
    // they are addressed to a single device.
    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }

    response.groupId = commandData.groupId;
    response.status  = GroupAdd(fabricIndex, commandPath.mEndpointId, commandData.groupId, commandData.groupName);
    err              = commandObj->AddResponseData(commandPath, response);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogDetail(Zcl, "GroupsCluster: AddGroup failed: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
    return true;
}

bool emberAfGroupsClusterViewGroupCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                           const Commands::ViewGroup::DecodableType & commandData)
{
    auto fabricIndex             = commandObj->GetAccessingFabricIndex();
    auto groupId                 = commandData.groupId;
    GroupDataProvider * provider = GetGroupDataProvider();
    GroupDataProvider::GroupInfo info;
    Groups::Commands::ViewGroupResponse::Type response;
    CHIP_ERROR err       = CHIP_NO_ERROR;
    EmberAfStatus status = EMBER_ZCL_STATUS_NOT_FOUND;

    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }

    VerifyOrExit(IsFabricGroupId(groupId), status = EMBER_ZCL_STATUS_INVALID_VALUE);
    VerifyOrExit(nullptr != provider, status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(provider->HasEndpoint(fabricIndex, groupId, commandPath.mEndpointId), status = EMBER_ZCL_STATUS_NOT_FOUND);

    err = provider->GetGroupInfo(fabricIndex, groupId, info);
    VerifyOrExit(CHIP_NO_ERROR == err, status = EMBER_ZCL_STATUS_NOT_FOUND);

    response.groupName = CharSpan(info.name, strnlen(info.name, GroupDataProvider::GroupInfo::kGroupNameMax));
    status             = EMBER_ZCL_STATUS_SUCCESS;
exit:
    response.groupId = groupId;
    response.status  = status;
    err              = commandObj->AddResponseData(commandPath, response);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogDetail(Zcl, "GroupsCluster: ViewGroup failed: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
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

    GroupMembershipResponse(const Commands::GetGroupMembership::DecodableType & data, chip::EndpointId endpoint,
                            GroupDataProvider::EndpointIterator * iter) :
        mCommandData(data),
        mEndpoint(endpoint), mIterator(iter)
    {}

    const Commands::GetGroupMembership::DecodableType & mCommandData;
    chip::EndpointId mEndpoint                      = kInvalidEndpointId;
    GroupDataProvider::EndpointIterator * mIterator = nullptr;

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
                GroupDataProvider::GroupEndpoint mapping;
                size_t requestedCount = 0;
                size_t matchCount     = 0;
                ReturnErrorOnFailure(mCommandData.groupList.ComputeSize(&requestedCount));

                if (0 == requestedCount)
                {
                    // 1.3.6.3.1. If the GroupList field is empty, the entity SHALL respond with all group identifiers of which the
                    // entity is a member.
                    while (mIterator && mIterator->Next(mapping))
                    {
                        if (mapping.endpoint_id == mEndpoint)
                        {
                            ReturnErrorOnFailure(app::DataModel::Encode(writer, TLV::AnonymousTag(), mapping.group_id));
                            matchCount++;
                            ChipLogDetail(Zcl, " 0x%02" PRIx16, mapping.group_id);
                        }
                    }
                }
                else
                {
                    while (mIterator && mIterator->Next(mapping))
                    {
                        auto iter = mCommandData.groupList.begin();
                        while (iter.Next())
                        {
                            if (mapping.endpoint_id == mEndpoint && mapping.group_id == iter.GetValue())
                            {
                                ReturnErrorOnFailure(app::DataModel::Encode(writer, TLV::AnonymousTag(), mapping.group_id));
                                matchCount++;
                                ChipLogDetail(Zcl, " 0x%02" PRIx16, mapping.group_id);
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
    auto fabricIndex     = commandObj->GetAccessingFabricIndex();
    auto * provider      = GetGroupDataProvider();
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;

    VerifyOrExit(nullptr != provider, status = EMBER_ZCL_STATUS_FAILURE);

    {
        GroupDataProvider::EndpointIterator * iter = nullptr;
        CHIP_ERROR err                             = CHIP_NO_ERROR;

        iter = provider->IterateEndpoints(fabricIndex);
        VerifyOrExit(nullptr != iter, status = EMBER_ZCL_STATUS_FAILURE);

        err = commandObj->AddResponseData(commandPath, GroupMembershipResponse(commandData, commandPath.mEndpointId, iter));
        iter->Release();
        status = (CHIP_NO_ERROR == err) ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    }

exit:
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogDetail(Zcl, "GroupsCluster: GetGroupMembership failed: failed: 0x%x", status);
        emberAfSendImmediateDefaultResponse(status);
    }
    return true;
}

bool emberAfGroupsClusterRemoveGroupCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                             const Commands::RemoveGroup::DecodableType & commandData)
{
    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    Groups::Commands::RemoveGroupResponse::Type response;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // For all networks, Remove Group commands are only responded to when
    // they are addressed to a single device.
    if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST && emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return true;
    }

#ifdef EMBER_AF_PLUGIN_SCENES
    // If a group is, removed the scenes associated with that group SHOULD be removed.
    emberAfScenesClusterRemoveScenesInGroupCallback(commandPath.mEndpointId, commandData.groupId);
#endif
    response.groupId = commandData.groupId;
    response.status  = GroupRemove(fabricIndex, commandPath.mEndpointId, commandData.groupId);

    err = commandObj->AddResponseData(commandPath, response);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogDetail(Zcl, "GroupsCluster: RemoveGroup failed: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
    return true;
}

bool emberAfGroupsClusterRemoveAllGroupsCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::RemoveAllGroups::DecodableType & commandData)
{
    auto fabricIndex     = commandObj->GetAccessingFabricIndex();
    auto * provider      = GetGroupDataProvider();
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;

    VerifyOrExit(nullptr != provider, status = EMBER_ZCL_STATUS_FAILURE);

#ifdef EMBER_AF_PLUGIN_SCENES
    {
        GroupDataProvider::EndpointIterator * iter = provider->IterateEndpoints(fabricIndex);
        GroupDataProvider::GroupEndpoint mapping;

        VerifyOrExit(nullptr != iter, status = EMBER_ZCL_STATUS_FAILURE);
        while (iter->Next(mapping))
        {
            if (commandPath.mEndpointId == mapping.endpoint_id)
            {
                emberAfScenesClusterRemoveScenesInGroupCallback(mapping.endpoint_id, mapping.group_id);
            }
        }
        iter->Release();
        emberAfScenesClusterRemoveScenesInGroupCallback(commandPath.mEndpointId, ZCL_SCENES_GLOBAL_SCENE_GROUP_ID);
    }
#endif

    provider->RemoveEndpoint(fabricIndex, commandPath.mEndpointId);
    status = EMBER_ZCL_STATUS_SUCCESS;

exit:
    emberAfSendImmediateDefaultResponse(status);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        ChipLogDetail(Zcl, "GroupsCluster: RemoveAllGroups failed: 0x%x", status);
    }
    return true;
}

bool emberAfGroupsClusterAddGroupIfIdentifyingCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::AddGroupIfIdentifying::DecodableType & commandData)
{
    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    auto groupId     = commandData.groupId;
    auto groupName   = commandData.groupName;
    auto endpointId  = commandPath.mEndpointId;

    EmberAfStatus status;
    EmberStatus sendStatus = EMBER_SUCCESS;

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
