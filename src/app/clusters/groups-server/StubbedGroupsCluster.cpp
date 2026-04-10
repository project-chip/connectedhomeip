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
#include <app/clusters/groups-server/StubbedGroupsCluster.h>

#include <app/clusters/scenes-server/Constants.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/Groups/Attributes.h>
#include <clusters/Groups/Commands.h>
#include <clusters/Groups/Metadata.h>
#include <clusters/Groups/Structs.h>
#include <lib/core/GroupId.h>
#include <tracing/macros.h>

using namespace chip::app::Clusters::Groups;
using chip::Protocols::InteractionModel::Status;

namespace chip::app::Clusters {

static_assert(Groups::kRevision >= 5, "This Stubbed Groups cluster implementation is only valid for cluster revision 5 and above");

CHIP_ERROR StubbedGroupsCluster::Attributes(const ConcreteClusterPath & path,
                                            ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Attributes::kMandatoryMetadata, {});
}

CHIP_ERROR StubbedGroupsCluster::AcceptedCommands(const ConcreteClusterPath & path,
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

CHIP_ERROR StubbedGroupsCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
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

DataModel::ActionReturnStatus StubbedGroupsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
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

std::optional<DataModel::ActionReturnStatus> StubbedGroupsCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                 TLV::TLVReader & input_arguments,
                                                                                 CommandHandler * handler)
{
    using namespace Commands;

    const FabricIndex fabricIndex = request.GetAccessingFabricIndex();

    switch (request.path.mCommandId)
    {
    case AddGroup::Id: {
        MATTER_TRACE_SCOPE("AddGroup", "Groups");
        return Status::InvalidInState;
    }
    case ViewGroup::Id: {
        MATTER_TRACE_SCOPE("ViewGroup", "Groups");
        return Status::InvalidInState;
    }
    case GetGroupMembership::Id: {
        MATTER_TRACE_SCOPE("GetGroupMembership", "Groups");
        return Status::InvalidInState;
    }
    case AddGroupIfIdentifying::Id: {
        MATTER_TRACE_SCOPE("AddGroupIfIdentifying", "Groups");
        return Status::InvalidInState;
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
                        return Status::NotFound;
                    }

                    return Status::Success;
                }()),
                .groupID = request_data.groupID,
            });
        return std::nullopt;
    }
    case RemoveAllGroups::Id: {
        MATTER_TRACE_SCOPE("RemoveAllGroups", "Groups");

        VerifyOrReturnError(mGroupDataProvider.RemoveEndpoint(fabricIndex, mPath.mEndpointId) == CHIP_NO_ERROR, Status::Failure);
        return Status::Success;
    }

    default:
        return Status::UnsupportedCommand;
    }
}

} // namespace chip::app::Clusters
