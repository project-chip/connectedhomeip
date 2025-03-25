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

#include "groups-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/reporting/reporting.h>
#include <app/util/config.h>
#include <credentials/GroupDataProvider.h>
#include <inttypes.h>
#include <lib/support/CodeUtils.h>
#include <tracing/macros.h>

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
#include <app/clusters/scenes-server/scenes-server.h>
#endif // MATTER_DM_PLUGIN_SCENES_MANAGEMENT

using namespace chip;
using namespace app::Clusters;
using namespace app::Clusters::Groups;
using namespace chip::Credentials;
using Protocols::InteractionModel::Status;

// Is the device identifying?
static bool emberAfIsDeviceIdentifying(EndpointId endpoint)
{
#ifdef ZCL_USING_IDENTIFY_CLUSTER_SERVER
    uint16_t identifyTime;
    Status status = app::Clusters::Identify::Attributes::IdentifyTime::Get(endpoint, &identifyTime);
    return (status == Status::Success && 0 < identifyTime);
#else
    return false;
#endif
}

/**
 * @brief Checks if group-endpoint association exist for the given fabric
 */
static bool GroupExists(FabricIndex fabricIndex, EndpointId endpointId, GroupId groupId)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != provider, false);

    return provider->HasEndpoint(fabricIndex, groupId, endpointId);
}

/**
 * @brief Checks if there are key set associated with the given GroupId
 */
static bool KeyExists(FabricIndex fabricIndex, GroupId groupId)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != provider, false);
    GroupDataProvider::GroupKey entry;

    auto it    = provider->IterateGroupKeys(fabricIndex);
    bool found = false;
    while (!found && it->Next(entry))
    {
        if (entry.group_id == groupId)
        {
            GroupDataProvider::KeySet keys;
            found = (CHIP_NO_ERROR == provider->GetKeySet(fabricIndex, entry.keyset_id, keys));
        }
    }
    it->Release();

    return found;
}

static Status GroupAdd(FabricIndex fabricIndex, EndpointId endpointId, GroupId groupId, const CharSpan & groupName)
{
    VerifyOrReturnError(IsValidGroupId(groupId), Status::ConstraintError);
    VerifyOrReturnError(groupName.size() <= GroupDataProvider::GroupInfo::kGroupNameMax, Status::ConstraintError);

    GroupDataProvider * provider = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != provider, Status::NotFound);
    VerifyOrReturnError(KeyExists(fabricIndex, groupId), Status::UnsupportedAccess);

    // Add a new entry to the GroupTable
    CHIP_ERROR err = provider->SetGroupInfo(fabricIndex, GroupDataProvider::GroupInfo(groupId, groupName));
    if (CHIP_NO_ERROR == err)
    {
        err = provider->AddEndpoint(fabricIndex, groupId, endpointId);
    }
    if (CHIP_NO_ERROR == err)
    {
        MatterReportingAttributeChangeCallback(kRootEndpointId, GroupKeyManagement::Id,
                                               GroupKeyManagement::Attributes::GroupTable::Id);
        return Status::Success;
    }

    ChipLogDetail(Zcl, "ERR: Failed to add mapping (end:%d, group:0x%x), err:%" CHIP_ERROR_FORMAT, endpointId, groupId,
                  err.Format());
    return Status::ResourceExhausted;
}

static Status GroupRemove(FabricIndex fabricIndex, EndpointId endpointId, GroupId groupId)
{
    VerifyOrReturnError(IsValidGroupId(groupId), Status::ConstraintError);
    VerifyOrReturnError(GroupExists(fabricIndex, endpointId, groupId), Status::NotFound);

    GroupDataProvider * provider = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != provider, Status::NotFound);

    CHIP_ERROR err = provider->RemoveEndpoint(fabricIndex, groupId, endpointId);
    if (CHIP_NO_ERROR == err)
    {
        MatterReportingAttributeChangeCallback(kRootEndpointId, GroupKeyManagement::Id,
                                               GroupKeyManagement::Attributes::GroupTable::Id);
        return Status::Success;
    }

    ChipLogDetail(Zcl, "ERR: Failed to remove mapping (end:%d, group:0x%x), err:%" CHIP_ERROR_FORMAT, endpointId, groupId,
                  err.Format());
    return Status::NotFound;
}

void emberAfGroupsClusterServerInitCallback(EndpointId endpointId)
{
    // According to spec, highest bit (Group Names) MUST match feature bit 0 (Group Names)
    Status status = Attributes::NameSupport::Set(endpointId, NameSupportBitmap::kGroupNames);
    if (status != Status::Success)
    {
        ChipLogDetail(Zcl, "ERR: writing NameSupport %x", to_underlying(status));
    }

    status = Attributes::FeatureMap::Set(endpointId, static_cast<uint32_t>(Feature::kGroupNames));
    if (status != Status::Success)
    {
        ChipLogDetail(Zcl, "ERR: writing group feature map %x", to_underlying(status));
    }
}

bool emberAfGroupsClusterAddGroupCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                          const Commands::AddGroup::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("AddGroup", "Groups");
    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    Groups::Commands::AddGroupResponse::Type response;

    response.groupID = commandData.groupID;
    response.status  = to_underlying(GroupAdd(fabricIndex, commandPath.mEndpointId, commandData.groupID, commandData.groupName));
    commandObj->AddResponse(commandPath, response);
    return true;
}

bool emberAfGroupsClusterViewGroupCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                           const Commands::ViewGroup::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("ViewGroup", "Groups");
    auto fabricIndex             = commandObj->GetAccessingFabricIndex();
    auto groupId                 = commandData.groupID;
    GroupDataProvider * provider = GetGroupDataProvider();
    GroupDataProvider::GroupInfo info;
    Groups::Commands::ViewGroupResponse::Type response;
    CHIP_ERROR err = CHIP_NO_ERROR;
    Status status  = Status::NotFound;

    VerifyOrExit(IsValidGroupId(groupId), status = Status::ConstraintError);
    VerifyOrExit(nullptr != provider, status = Status::Failure);
    VerifyOrExit(provider->HasEndpoint(fabricIndex, groupId, commandPath.mEndpointId), status = Status::NotFound);

    err = provider->GetGroupInfo(fabricIndex, groupId, info);
    VerifyOrExit(CHIP_NO_ERROR == err, status = Status::NotFound);

    response.groupName = CharSpan(info.name, strnlen(info.name, GroupDataProvider::GroupInfo::kGroupNameMax));
    status             = Status::Success;
exit:
    response.groupID = groupId;
    response.status  = to_underlying(status);
    commandObj->AddResponse(commandPath, response);
    return true;
}

struct GroupMembershipResponse
{
    // A null capacity means that it is unknown if any further groups MAY be added.
    const chip::app::DataModel::Nullable<uint8_t> kCapacityUnknown;

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
            writer, TLV::ContextTag(Commands::GetGroupMembershipResponse::Fields::kCapacity), kCapacityUnknown));
        {
            TLV::TLVType type;
            ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(Commands::GetGroupMembershipResponse::Fields::kGroupList),
                                                       TLV::kTLVType_Array, type));
            {
                GroupDataProvider::GroupEndpoint mapping;
                size_t requestedCount = 0;
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
                            ChipLogDetail(Zcl, " 0x%02x", mapping.group_id);
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
                                ChipLogDetail(Zcl, " 0x%02x", mapping.group_id);
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
    MATTER_TRACE_SCOPE("GetGroupMembership", "Groups");
    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    auto * provider  = GetGroupDataProvider();
    Status status    = Status::Failure;

    VerifyOrExit(nullptr != provider, status = Status::Failure);

    {
        GroupDataProvider::EndpointIterator * iter = nullptr;

        iter = provider->IterateEndpoints(fabricIndex);
        VerifyOrExit(nullptr != iter, status = Status::Failure);

        commandObj->AddResponse(commandPath, GroupMembershipResponse(commandData, commandPath.mEndpointId, iter));
        iter->Release();
        status = Status::Success;
    }

exit:
    if (Status::Success != status)
    {
        ChipLogDetail(Zcl, "GroupsCluster: GetGroupMembership failed: failed: 0x%x", to_underlying(status));
        commandObj->AddStatus(commandPath, status);
    }
    return true;
}

bool emberAfGroupsClusterRemoveGroupCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                             const Commands::RemoveGroup::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("RemoveGroup", "Groups");
    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    Groups::Commands::RemoveGroupResponse::Type response;

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    // If a group is removed the scenes associated with that group SHOULD be removed.
    ScenesManagement::ScenesServer::Instance().GroupWillBeRemoved(fabricIndex, commandPath.mEndpointId, commandData.groupID);
#endif
    response.groupID = commandData.groupID;
    response.status  = to_underlying(GroupRemove(fabricIndex, commandPath.mEndpointId, commandData.groupID));

    commandObj->AddResponse(commandPath, response);
    return true;
}

bool emberAfGroupsClusterRemoveAllGroupsCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::RemoveAllGroups::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("RemoveAllGroups", "Groups");
    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    auto * provider  = GetGroupDataProvider();
    Status status    = Status::Failure;

    VerifyOrExit(nullptr != provider, status = Status::Failure);

#ifdef MATTER_DM_PLUGIN_SCENES_MANAGEMENT
    {
        GroupDataProvider::EndpointIterator * iter = provider->IterateEndpoints(fabricIndex);
        GroupDataProvider::GroupEndpoint mapping;

        VerifyOrExit(nullptr != iter, status = Status::Failure);
        while (iter->Next(mapping))
        {
            if (commandPath.mEndpointId == mapping.endpoint_id)
            {
                ScenesManagement::ScenesServer::Instance().GroupWillBeRemoved(fabricIndex, mapping.endpoint_id, mapping.group_id);
            }
        }
        iter->Release();
        ScenesManagement::ScenesServer::Instance().GroupWillBeRemoved(fabricIndex, commandPath.mEndpointId,
                                                                      ScenesManagement::ScenesServer::kGlobalSceneGroupId);
    }
#endif

    provider->RemoveEndpoint(fabricIndex, commandPath.mEndpointId);
    status = Status::Success;
    MatterReportingAttributeChangeCallback(kRootEndpointId, GroupKeyManagement::Id, GroupKeyManagement::Attributes::GroupTable::Id);
exit:
    commandObj->AddStatus(commandPath, status);
    if (Status::Success != status)
    {
        ChipLogDetail(Zcl, "GroupsCluster: RemoveAllGroups failed: 0x%x", to_underlying(status));
    }
    return true;
}

bool emberAfGroupsClusterAddGroupIfIdentifyingCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const Commands::AddGroupIfIdentifying::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("AddGroupIfIdentifying", "Groups");
    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    auto groupId     = commandData.groupID;
    auto groupName   = commandData.groupName;
    auto endpointId  = commandPath.mEndpointId;

    Status status;
    if (!emberAfIsDeviceIdentifying(endpointId))
    {
        // If not identifying, ignore add group -> success; not a failure.
        status = Status::Success;
    }
    else
    {
        status = GroupAdd(fabricIndex, endpointId, groupId, groupName);
    }

    commandObj->AddStatus(commandPath, status);
    return true;
}

bool emberAfGroupsClusterEndpointInGroupCallback(chip::FabricIndex fabricIndex, EndpointId endpointId, GroupId groupId)
{
    return GroupExists(fabricIndex, endpointId, groupId);
}

void emberAfPluginGroupsServerSetGroupNameCallback(EndpointId endpoint, GroupId groupId, const CharSpan & groupName) {}

void MatterGroupsPluginServerInitCallback() {}
