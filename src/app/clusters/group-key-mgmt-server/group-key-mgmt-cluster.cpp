/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/group-key-mgmt-server/group-key-mgmt-cluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server/Server.h>
#include <clusters/GroupKeyManagement/ClusterId.h>
#include <clusters/GroupKeyManagement/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::GroupKeyManagement;
using namespace chip::app::Clusters::GroupKeyManagement::Attributes;
using namespace chip::DeviceLayer;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace {
constexpr DataModel::AttributeEntry kMandatoryAttributes[] = {
    GroupKeyManagement::Attributes::GroupKeyMap::kMetadataEntry,
    GroupKeyManagement::Attributes::GroupTable::kMetadataEntry,
    GroupKeyManagement::Attributes::MaxGroupsPerFabric::kMetadataEntry,
    GroupKeyManagement::Attributes::MaxGroupKeysPerFabric::kMetadataEntry,
};
}

struct GroupTableCodec
{
    static constexpr TLV::Tag TagFabric()
    {
        return TLV::ContextTag(GroupKeyManagement::Structs::GroupInfoMapStruct::Fields::kFabricIndex);
    }
    static constexpr TLV::Tag TagGroup()
    {
        return TLV::ContextTag(GroupKeyManagement::Structs::GroupInfoMapStruct::Fields::kGroupId);
    }
    static constexpr TLV::Tag TagEndpoints()
    {
        return TLV::ContextTag(GroupKeyManagement::Structs::GroupInfoMapStruct::Fields::kEndpoints);
    }
    static constexpr TLV::Tag TagGroupName()
    {
        return TLV::ContextTag(GroupKeyManagement::Structs::GroupInfoMapStruct::Fields::kGroupName);
    }

    GroupDataProvider * mProvider = nullptr;
    chip::FabricIndex mFabric;
    GroupDataProvider::GroupInfo mInfo;

    GroupTableCodec(GroupDataProvider * provider, chip::FabricIndex fabric_index, GroupDataProvider::GroupInfo & info) :
        mProvider(provider), mFabric(fabric_index), mInfo(info)
    {}

    static constexpr bool kIsFabricScoped = true;

    auto GetFabricIndex() const { return mFabric; }

    CHIP_ERROR EncodeForRead(TLV::TLVWriter & writer, TLV::Tag tag, FabricIndex accessingFabricIndex) const
    {
        TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));

        // FabricIndex
        ReturnErrorOnFailure(DataModel::Encode(writer, TagFabric(), mFabric));
        // GroupId
        ReturnErrorOnFailure(DataModel::Encode(writer, TagGroup(), mInfo.group_id));
        // Endpoints
        TLV::TLVType inner;
        ReturnErrorOnFailure(writer.StartContainer(TagEndpoints(), TLV::kTLVType_Array, inner));
        GroupDataProvider::GroupEndpoint mapping;
        auto iter = mProvider->IterateEndpoints(mFabric, std::make_optional(mInfo.group_id));
        if (nullptr != iter)
        {
            while (iter->Next(mapping))
            {
                ReturnErrorOnFailure(writer.Put(TLV::AnonymousTag(), static_cast<uint16_t>(mapping.endpoint_id)));
            }
            iter->Release();
        }
        ReturnErrorOnFailure(writer.EndContainer(inner));
        // GroupName
        uint32_t name_size = static_cast<uint32_t>(strnlen(mInfo.name, GroupDataProvider::GroupInfo::kGroupNameMax));
        ReturnErrorOnFailure(writer.PutString(TagGroupName(), mInfo.name, name_size));

        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
};

std::optional<DataModel::ActionReturnStatus> GroupKeyManagementCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) 
{
    switch (request.path.mCommandId)
    {
    case GroupKeyManagement::Commands::KeySetWrie::Id:
        return 
    case GroupKeyManagement::Commands::KeySetRead::Id:
        return 
    case GroupKeyManagement::Commands::KeySetRemove::Id:
        return
    case GroupKeyManagement::Commands::KeySetReadAllIndices::Id:
        return
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus GroupKeyManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                       AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
        case GroupKeyManagement::Attributes::ClusterRevision::Id:
            return encoder.Encode(GroupKeyManagement::kRevision);
        case Attributes::FeatureMap::Id: {
            uint32_t features = 0;
            if (IsMCSPSupported())
            {
                // TODO: Once there is MCSP support, this will need to add the
                // right feature bit.
            }
            return encoder.Encode(features);
        }
        case GroupKeyManagement::Attributes::GroupKeyMap::Id:
            return ReadGroupKeyMap(encoder);
        case GroupKeyManagement::Attributes::GroupTable::Id:
            return ReadGroupTable(encoder);
        case GroupKeyManagement::Attributes::MaxGroupsPerFabric::Id:
            return ReadMaxGroupsPerFabric(encoder);
        case GroupKeyManagement::Attributes::MaxGroupKeysPerFabric::Id:
            return ReadMaxGroupKeysPerFabric(encoder);
        default:
            return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus GroupKeyManagementCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) {
    

}

CHIP_ERROR GroupKeyManagementCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) {
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(kMandatoryAttributes), Span<const DataModel::AttributeEntry>());
}

CHIP_ERROR GroupKeyManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) {
    static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = { 
        Commands::KeySetWrite::kMetadataEntry,
        Commands::KeySetRead::kMetadataEntry,
        Commands::KeySetRemove::kMetadataEntry,
        Commands::KeySetReadAllIndices::kMetadataEntry,
    };
    return builder.ReferenceExisting(kAcceptedCommands);
}
    
CHIP_ERROR GroupKeyManagementCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) {
    static constexpr CommandId kGeneratedCommands[] = {
        Commands::KeySetReadAllIndicesResponse::Id,
        Commands::KeySetReadResponse::Id,
    };
    return builder.ReferenceExisting(kGeneratedCommands);
}

CHIP_ERROR GroupKeyManagementCluster::ReadGroupKeyMap(AttributeValueEncoder & aEncoder)
{
    auto provider = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != provider, CHIP_ERROR_INTERNAL);

    CHIP_ERROR err = aEncoder.EncodeList([provider](const auto & encoder) -> CHIP_ERROR {
        CHIP_ERROR encodeStatus = CHIP_NO_ERROR;

        for (auto & fabric : Server::GetInstance().GetFabricTable())
        {
            auto fabric_index = fabric.GetFabricIndex();
            auto iter         = provider->IterateGroupKeys(fabric_index);
            VerifyOrReturnError(nullptr != iter, CHIP_ERROR_NO_MEMORY);

            GroupDataProvider::GroupKey mapping;
            while (iter->Next(mapping))
            {
                GroupKeyManagement::Structs::GroupKeyMapStruct::Type key = {
                    .groupId       = mapping.group_id,
                    .groupKeySetID = mapping.keyset_id,
                    .fabricIndex   = fabric_index,
                };
                encodeStatus = encoder.Encode(key);
                if (encodeStatus != CHIP_NO_ERROR)
                {
                    break;
                }
            }
            iter->Release();
            if (encodeStatus != CHIP_NO_ERROR)
            {
                break;
            }
        }
        return encodeStatus;
    });
    return err;
}

CHIP_ERROR GroupKeyManagementCluster::ReadGroupTable(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    auto provider = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != provider, CHIP_ERROR_INTERNAL);

    CHIP_ERROR err = aEncoder.EncodeList([provider](const auto & encoder) -> CHIP_ERROR {
        CHIP_ERROR encodeStatus = CHIP_NO_ERROR;

        for (auto & fabric : Server::GetInstance().GetFabricTable())
        {
            auto fabric_index = fabric.GetFabricIndex();
            auto iter         = provider->IterateGroupInfo(fabric_index);
            VerifyOrReturnError(nullptr != iter, CHIP_ERROR_NO_MEMORY);

            GroupDataProvider::GroupInfo info;
            while (iter->Next(info))
            {
                encodeStatus = encoder.Encode(GroupTableCodec(provider, fabric_index, info));
                if (encodeStatus != CHIP_NO_ERROR)
                {
                    break;
                }
            }
            iter->Release();
            if (encodeStatus != CHIP_NO_ERROR)
            {
                break;
            }
        }
        return encodeStatus;
    });
    return err;
}

CHIP_ERROR GroupKeyManagementCluster::ReadMaxGroupsPerFabric(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    auto * provider = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != provider, CHIP_ERROR_INTERNAL);
    return aEncoder.Encode(provider->GetMaxGroupsPerFabric());
}

CHIP_ERROR GroupKeyManagementCluster::ReadMaxGroupKeysPerFabric(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    auto * provider = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != provider, CHIP_ERROR_INTERNAL);
    return aEncoder.Encode(provider->GetMaxGroupKeysPerFabric());
}

}
}
}