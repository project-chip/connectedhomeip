/**
 *    Copyright (c) 2020-2026 Project CHIP Authors
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
#include <app/clusters/groups-server/GroupsCluster.h>

#include <app/clusters/scenes-server/Constants.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/GroupKeyManagement/Ids.h>
#include <clusters/Groups/Attributes.h>
#include <clusters/Groups/Commands.h>
#include <clusters/Groups/Metadata.h>
#include <clusters/Groups/Structs.h>
#include <tracing/macros.h>

using namespace chip::app::Clusters::Groups;

using chip::Credentials::GroupDataProvider;
using chip::Protocols::InteractionModel::Status;

// The macro LogErrorOnFailure generally logs file:line which makes the macro larger. We use here a smaller
// macro (i.e. no line & file info) since the underlying error would have the original cause and logs would contain
// what call is being processed. This is to save about 100 bytes or so of flash.
#define LogIfFailure(expr)                                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        CHIP_ERROR __err = (expr);                                                                                                 \
        if (__err != CHIP_NO_ERROR)                                                                                                \
        {                                                                                                                          \
            ChipLogError(Zcl, "Error: %" CHIP_ERROR_FORMAT, __err.Format());                                                       \
        }                                                                                                                          \
    } while (false)

namespace chip::app::Clusters {
namespace {

constexpr AttributePathParams kGroupKeyGroupTableAttributePath{ kRootEndpointId, GroupKeyManagement::Id,
                                                                GroupKeyManagement::Attributes::GroupTable::Id };

void NotifyGroupTableChanged(ServerClusterContext * context)
{
    // TODO: This seems a bit coupled: we are notifying in this cluster that ANOTHER cluster
    //       has changed. We should support only one cluster or another really...
    VerifyOrReturn(context != nullptr);
    context->interactionContext.dataModelChangeListener.MarkDirty(kGroupKeyGroupTableAttributePath);
}

class AutoReleaseIterator
{
public:
    AutoReleaseIterator(GroupDataProvider & provider, FabricIndex fabricIndex) : mIterator(provider.IterateGroupKeys(fabricIndex))
    {}
    ~AutoReleaseIterator()
    {
        if (mIterator != nullptr)
        {
            mIterator->Release();
        }
    }
    bool Valid() const { return mIterator != nullptr; }
    GroupDataProvider::GroupKeyIterator * operator->() { return mIterator; }

private:
    GroupDataProvider::GroupKeyIterator * mIterator;
};

/**
 * @brief Checks if there are key set associated with the given GroupId
 */
bool KeyExists(GroupDataProvider & provider, FabricIndex fabricIndex, GroupId groupId)
{
    AutoReleaseIterator it(provider, fabricIndex);
    VerifyOrReturnValue(it.Valid(), false);

    GroupDataProvider::GroupKey key;
    while (it->Next(key))
    {
        Credentials::GroupDataProvider::KeySet keys;
        if ((key.group_id == groupId) && (provider.GetKeySet(fabricIndex, key.keyset_id, keys) == CHIP_NO_ERROR))
        {
            return true;
        }
    }

    return false;
}

struct GroupMembershipResponse
{
    // A null capacity means that it is unknown if any further groups MAY be added.
    const DataModel::Nullable<uint8_t> kCapacityUnknown;

    // Use GetCommandId instead of commandId directly to avoid naming conflict with CommandIdentification in ExecutionOfACommand
    static constexpr CommandId GetCommandId() { return Commands::GetGroupMembershipResponse::Id; }
    static constexpr ClusterId GetClusterId() { return Groups::Id; }

    GroupMembershipResponse(const Commands::GetGroupMembership::DecodableType & data, EndpointId endpoint,
                            GroupDataProvider::EndpointIterator * iter) :
        mCommandData(data),
        mEndpoint(endpoint), mIterator(iter)
    {}

    const Commands::GetGroupMembership::DecodableType & mCommandData;
    EndpointId mEndpoint                            = kInvalidEndpointId;
    GroupDataProvider::EndpointIterator * mIterator = nullptr;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
    {
        TLV::TLVType outer;

        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));

        ReturnErrorOnFailure(
            DataModel::Encode(writer, TLV::ContextTag(Commands::GetGroupMembershipResponse::Fields::kCapacity), kCapacityUnknown));
        {
            TLV::TLVType type;
            ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(Commands::GetGroupMembershipResponse::Fields::kGroupList),
                                                       TLV::kTLVType_Array, type));
            {
                GroupDataProvider::GroupEndpoint mapping;
                size_t requestedCount = 0;
                ReturnErrorOnFailure(mCommandData.groupList.ComputeSize(&requestedCount));

                auto accept_group = [&](GroupId groupId) {
                    // 1.3.6.3.1. If the GroupList field is empty, the entity SHALL respond with all group identifiers of which the
                    // entity is a member.
                    VerifyOrReturnValue(0 != requestedCount, true);
                    auto iter = mCommandData.groupList.begin();
                    while (iter.Next())
                    {
                        VerifyOrReturnValue(groupId != iter.GetValue(), true);
                    }
                    return false;
                };

                while (mIterator && mIterator->Next(mapping))
                {
                    if (mapping.endpoint_id != mEndpoint)
                    {
                        continue;
                    }
                    if (!accept_group(mapping.group_id))
                    {
                        continue;
                    }
                    ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag(), mapping.group_id));
                    ChipLogDetail(Zcl, " 0x%02x", mapping.group_id);
                }
                ChipLogDetail(Zcl, "]");
            }
            ReturnErrorOnFailure(writer.EndContainer(type));
        }
        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
};

} // namespace

CHIP_ERROR GroupsCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Attributes::kMandatoryMetadata, {});
}

CHIP_ERROR GroupsCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                           ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    using namespace Commands;

    static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
        AddGroup::kMetadataEntry,              //
        ViewGroup::kMetadataEntry,             //
        GetGroupMembership::kMetadataEntry,    //
        RemoveGroup::kMetadataEntry,           //
        RemoveAllGroups::kMetadataEntry,       //
        AddGroupIfIdentifying::kMetadataEntry, //
    };

    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR GroupsCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    using namespace Commands;

    static constexpr CommandId kGeneratedCommands[] = {
        AddGroupResponse::Id,           //
        ViewGroupResponse::Id,          //
        GetGroupMembershipResponse::Id, //
        RemoveGroupResponse::Id,        //
    };

    return builder.ReferenceExisting(kGeneratedCommands);
}

DataModel::ActionReturnStatus GroupsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                           AttributeValueEncoder & encoder)
{
    using namespace Attributes;

    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case FeatureMap::Id:
        // Group names is hardcoded (feature is M conformance in the spec)
        return encoder.Encode(Feature::kGroupNames);
    case NameSupport::Id:
        // According to the spec, kGroupNames must be set (M conformance in the spec)
        return encoder.Encode(NameSupportBitmap::kGroupNames);
    default:
        return Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus>
GroupsCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    using namespace Commands;

    const FabricIndex fabricIndex = request.GetAccessingFabricIndex();

    switch (request.path.mCommandId)
    {
    case AddGroup::Id: {
        MATTER_TRACE_SCOPE("AddGroup", "Groups");
        AddGroup::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, fabricIndex));

        Groups::Commands::AddGroupResponse::Type response;
        response.groupID = request_data.groupID;
        response.status  = to_underlying(AddGroup(request_data.groupID, request_data.groupName, fabricIndex));
        handler->AddResponse(request.path, response);

        return std::nullopt;
    }
    case ViewGroup::Id: {
        MATTER_TRACE_SCOPE("ViewGroup", "Groups");
        ViewGroup::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, fabricIndex));

        const GroupId groupId = request_data.groupID;

        Credentials::GroupDataProvider::GroupInfo info;
        const Status status = [&]() {
            VerifyOrReturnError(IsValidGroupId(groupId), Status::ConstraintError);
            VerifyOrReturnError(mGroupDataProvider.HasEndpoint(fabricIndex, groupId, mPath.mEndpointId), Status::NotFound);

            if (mGroupDataProvider.GetGroupInfo(fabricIndex, groupId, info) != CHIP_NO_ERROR)
            {
                return Status::NotFound;
            }

            return Status::Success;
        }();

        if (status != Status::Success)
        {
            info.name[0] = 0;
        }

        handler->AddResponse(request.path,
                             ViewGroupResponse::Type{
                                 .status    = to_underlying(status),
                                 .groupID   = request_data.groupID,
                                 .groupName = CharSpan(info.name, strnlen(info.name, GroupDataProvider::GroupInfo::kGroupNameMax)),
                             });
        return std::nullopt;
    }
    case GetGroupMembership::Id: {
        MATTER_TRACE_SCOPE("GetGroupMembership", "Groups");
        GetGroupMembership::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, fabricIndex));

        GroupDataProvider::EndpointIterator * iter = mGroupDataProvider.IterateEndpoints(fabricIndex);
        VerifyOrReturnError(nullptr != iter, Status::Failure);

        handler->AddResponse(request.path, GroupMembershipResponse(request_data, mPath.mEndpointId, iter));
        iter->Release();
        return std::nullopt;
    }
    case RemoveGroup::Id: {
        MATTER_TRACE_SCOPE("RemoveGroup", "Groups");
        RemoveGroup::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, fabricIndex));

        const GroupId groupId = request_data.groupID;

        handler->AddResponse(
            request.path,
            Groups::Commands::RemoveGroupResponse::Type{
                .status  = to_underlying([&]() {
                    VerifyOrReturnError(IsValidGroupId(groupId), Status::ConstraintError);
                    VerifyOrReturnError(mGroupDataProvider.HasEndpoint(fabricIndex, groupId, mPath.mEndpointId), Status::NotFound);

                    if (CHIP_ERROR err = mGroupDataProvider.RemoveEndpoint(fabricIndex, groupId, mPath.mEndpointId);
                        err != CHIP_NO_ERROR)
                    {
                        ChipLogDetail(Zcl, "ERR: Failed to remove mapping (end:%d, group:0x%x), err:%" CHIP_ERROR_FORMAT,
                                       mPath.mEndpointId, groupId, err.Format());
                        return Status::NotFound;
                    }

                    if (mScenesIntegration != nullptr)
                    {
                        // If a group is removed the scenes associated with that group SHOULD be removed.
                        LogIfFailure(mScenesIntegration->GroupWillBeRemoved(fabricIndex, groupId));
                    }

                    NotifyGroupTableChanged(mContext);
                    return Status::Success;
                }()),
                .groupID = request_data.groupID,
            });
        return std::nullopt;
    }
    case RemoveAllGroups::Id: {
        MATTER_TRACE_SCOPE("RemoveAllGroups", "Groups");

        if (mScenesIntegration != nullptr)
        {
            GroupDataProvider::EndpointIterator * iter = mGroupDataProvider.IterateEndpoints(fabricIndex);
            GroupDataProvider::GroupEndpoint mapping;

            VerifyOrReturnError(nullptr != iter, Status::Failure);
            while (iter->Next(mapping))
            {
                if (mPath.mEndpointId == mapping.endpoint_id)
                {
                    LogIfFailure(mScenesIntegration->GroupWillBeRemoved(fabricIndex, mapping.group_id));
                }
            }
            iter->Release();
            LogIfFailure(mScenesIntegration->GroupWillBeRemoved(fabricIndex, scenes::kGlobalSceneGroupId));
        }

        LogIfFailure(mGroupDataProvider.RemoveEndpoint(fabricIndex, mPath.mEndpointId));
        NotifyGroupTableChanged(mContext);
        return Status::Success;
    }
    case AddGroupIfIdentifying::Id: {
        MATTER_TRACE_SCOPE("AddGroupIfIdentifying", "Groups");
        AddGroupIfIdentifying::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments, fabricIndex));

        // skip with success if we are not identifying
        VerifyOrReturnValue((mIdentifyIntegration != nullptr) && mIdentifyIntegration->IsIdentifying(), Status::Success);

        // AddGroupIfIdentifying is response `Y` in the spec: we return the status (not a structure, as opposed to AddGroup)
        return AddGroup(request_data.groupID, request_data.groupName, fabricIndex);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

Status GroupsCluster::AddGroup(GroupId groupID, CharSpan groupName, FabricIndex fabricIndex)
{
    VerifyOrReturnError(IsValidGroupId(groupID), Status::ConstraintError);
    VerifyOrReturnError(groupName.size() <= GroupDataProvider::GroupInfo::kGroupNameMax, Status::ConstraintError);

    VerifyOrReturnError(KeyExists(mGroupDataProvider, fabricIndex, groupID), Status::UnsupportedAccess);

    // Add a new entry to the GroupTable
    if (CHIP_ERROR err = mGroupDataProvider.SetGroupInfo(fabricIndex, GroupDataProvider::GroupInfo(groupID, groupName));
        err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "ERR: Failed to add mapping (end:%d, group:0x%x), err:%" CHIP_ERROR_FORMAT, mPath.mEndpointId, groupID,
                      err.Format());
        return Status::ResourceExhausted;
    }

    if (CHIP_ERROR err = mGroupDataProvider.AddEndpoint(fabricIndex, groupID, mPath.mEndpointId); err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "ERR: Failed to add mapping (end:%d, group:0x%x), err:%" CHIP_ERROR_FORMAT, mPath.mEndpointId, groupID,
                      err.Format());
        // best-effort undo the mapping we added above, to try to be a bit more atomic.
        LogIfFailure(mGroupDataProvider.RemoveGroupInfo(fabricIndex, groupID));
        return Status::ResourceExhausted;
    }
    NotifyGroupTableChanged(mContext);
    return Status::Success;
}

} // namespace chip::app::Clusters
