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

#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/command-id.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <credentials/GroupDataProvider.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace chip::app::Clusters;

//
// Attributes
//

namespace {

struct GroupKeyCodec
{
    static constexpr TLV::Tag TagFabric()
    {
        return TLV::ContextTag(to_underlying(GroupKeyManagement::Structs::GroupKey::Fields::kFabricIndex));
    }
    static constexpr TLV::Tag TagGroup()
    {
        return TLV::ContextTag(to_underlying(GroupKeyManagement::Structs::GroupKey::Fields::kGroupId));
    }
    static constexpr TLV::Tag TagKeyset()
    {
        return TLV::ContextTag(to_underlying(GroupKeyManagement::Structs::GroupKey::Fields::kGroupKeySetID));
    }

    chip::FabricIndex mFabric = 0;
    GroupDataProvider::GroupKey mMapping;

    GroupKeyCodec() = default;
    GroupKeyCodec(chip::FabricIndex fabric_index, const GroupDataProvider::GroupKey & mapping) :
        mFabric(fabric_index), mMapping(mapping)
    {}

    CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
    {
        TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));

        // FabricIndex
        ReturnErrorOnFailure(DataModel::Encode(writer, TagFabric(), mFabric));
        // GroupId
        ReturnErrorOnFailure(DataModel::Encode(writer, TagGroup(), mMapping.group_id));
        // GroupKeySetID
        ReturnErrorOnFailure(DataModel::Encode(writer, TagKeyset(), mMapping.keyset_id));

        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Decode(TLV::TLVReader & reader)
    {
        TLV::TLVType outer;

        VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
        ReturnErrorOnFailure(reader.EnterContainer(outer));

        // FabricIndex
        ReturnErrorOnFailure(reader.Next(TagFabric()));
        ReturnErrorOnFailure(reader.Get(mFabric));
        // GroupId
        ReturnErrorOnFailure(reader.Next(TagGroup()));
        ReturnErrorOnFailure(reader.Get(mMapping.group_id));
        // GroupKeySetID
        ReturnErrorOnFailure(reader.Next(TagKeyset()));
        ReturnErrorOnFailure(reader.Get(mMapping.keyset_id));

        ReturnErrorOnFailure(reader.ExitContainer(outer));
        return CHIP_NO_ERROR;
    }
};

struct GroupTableCodec
{
    static constexpr TLV::Tag TagFabric()
    {
        return TLV::ContextTag(to_underlying(GroupKeyManagement::Structs::GroupInfo::Fields::kFabricIndex));
    }
    static constexpr TLV::Tag TagGroup()
    {
        return TLV::ContextTag(to_underlying(GroupKeyManagement::Structs::GroupInfo::Fields::kGroupId));
    }
    static constexpr TLV::Tag TagEndpoints()
    {
        return TLV::ContextTag(to_underlying(GroupKeyManagement::Structs::GroupInfo::Fields::kEndpoints));
    }
    static constexpr TLV::Tag TagGroupName()
    {
        return TLV::ContextTag(to_underlying(GroupKeyManagement::Structs::GroupInfo::Fields::kGroupName));
    }

    GroupDataProvider * mProvider = nullptr;
    chip::FabricIndex mFabric;
    GroupDataProvider::GroupInfo mInfo;

    GroupTableCodec(GroupDataProvider * provider, chip::FabricIndex fabric_index, GroupDataProvider::GroupInfo & info) :
        mProvider(provider), mFabric(fabric_index), mInfo(info)
    {}

    CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
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
        auto iter = mProvider->IterateEndpoints(mFabric);
        if (nullptr != iter)
        {
            while (iter->Next(mapping))
            {
                if (mapping.group_id == mInfo.group_id)
                {
                    ReturnErrorOnFailure(writer.Put(TLV::AnonymousTag(), static_cast<uint16_t>(mapping.endpoint_id)));
                }
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

class GroupKeyManagementAttributeAccess : public AttributeAccessInterface
{
public:
    // Register for the GroupKeyManagement cluster on all endpoints.
    GroupKeyManagementAttributeAccess() : AttributeAccessInterface(Optional<EndpointId>(0), GroupKeyManagement::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        VerifyOrDie(aPath.mClusterId == GroupKeyManagement::Id);

        switch (aPath.mAttributeId)
        {
        case GroupKeyManagement::Attributes::ClusterRevision::Id:
            return ReadClusterRevision(aPath.mEndpointId, aEncoder);
        case GroupKeyManagement::Attributes::GroupKeyMap::Id:
            return ReadGroupKeyMap(aPath.mEndpointId, aEncoder);
        case GroupKeyManagement::Attributes::GroupTable::Id:
            return ReadGroupTable(aPath.mEndpointId, aEncoder);
        case GroupKeyManagement::Attributes::MaxGroupsPerFabric::Id:
            return ReadMaxGroupsPerFabric(aPath.mEndpointId, aEncoder);
        case GroupKeyManagement::Attributes::MaxGroupKeysPerFabric::Id:
            return ReadMaxGroupKeysPerFabric(aPath.mEndpointId, aEncoder);
        default:
            break;
        }
        return CHIP_ERROR_READ_FAILED;
    }

    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override
    {

        if (GroupKeyManagement::Attributes::GroupKeyMap::Id == aPath.mAttributeId)
        {
            return WriteGroupKeyMap(aPath.mEndpointId, aDecoder);
        }
        return CHIP_ERROR_WRITE_FAILED;
    }

private:
    static constexpr uint16_t kClusterRevision = 1;

    CHIP_ERROR ReadClusterRevision(EndpointId endpoint, AttributeValueEncoder & aEncoder)
    {
        return aEncoder.Encode(kClusterRevision);
    }
    CHIP_ERROR ReadGroupKeyMap(EndpointId endpoint, AttributeValueEncoder & aEncoder)
    {
        auto fabric_index = aEncoder.AccessingFabricIndex();
        auto provider     = GetGroupDataProvider();
        VerifyOrReturnError(nullptr != provider, CHIP_ERROR_INTERNAL);

        CHIP_ERROR err = aEncoder.EncodeList([provider, fabric_index](const auto & encoder) -> CHIP_ERROR {
            auto iter = provider->IterateGroupKeys(fabric_index);
            VerifyOrReturnError(nullptr != iter, CHIP_ERROR_NO_MEMORY);

            GroupDataProvider::GroupKey mapping;
            while (iter->Next(mapping))
            {
                encoder.Encode(GroupKeyCodec(fabric_index, mapping));
            }
            iter->Release();
            return CHIP_NO_ERROR;
        });
        return err;
    }

    CHIP_ERROR WriteGroupKeyMap(EndpointId endpoint, AttributeValueDecoder & aDecoder)
    {
        auto fabric_index = aDecoder.AccessingFabricIndex();
        auto provider     = GetGroupDataProvider();
        DataModel::DecodableList<GroupKeyCodec> list;
        size_t new_count;

        VerifyOrReturnError(nullptr != provider, CHIP_ERROR_INTERNAL);
        ReturnErrorOnFailure(aDecoder.Decode(list));
        ReturnErrorOnFailure(list.ComputeSize(&new_count));

        // Remove existing keys
        ReturnErrorOnFailure(provider->RemoveGroupKeys(fabric_index));

        // Add the new keys
        auto iter = list.begin();
        size_t i  = 0;
        while (iter.Next())
        {
            const GroupKeyCodec & value = iter.GetValue();
            VerifyOrReturnError(fabric_index == value.mFabric, CHIP_ERROR_INVALID_FABRIC_ID);
            ReturnErrorOnFailure(provider->SetGroupKeyAt(value.mFabric, i++, value.mMapping));
        }
        ReturnErrorOnFailure(iter.GetStatus());
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ReadGroupTable(EndpointId endpoint, AttributeValueEncoder & aEncoder)
    {
        auto fabric_index = aEncoder.AccessingFabricIndex();
        auto provider     = GetGroupDataProvider();
        VerifyOrReturnError(nullptr != provider, CHIP_ERROR_INTERNAL);

        CHIP_ERROR err = aEncoder.EncodeList([provider, fabric_index](const auto & encoder) -> CHIP_ERROR {
            auto iter = provider->IterateGroupInfo(fabric_index);
            VerifyOrReturnError(nullptr != iter, CHIP_ERROR_NO_MEMORY);

            GroupDataProvider::GroupInfo info;
            while (iter->Next(info))
            {
                encoder.Encode(GroupTableCodec(provider, fabric_index, info));
            }
            iter->Release();
            return CHIP_NO_ERROR;
        });
        return err;
    }
    CHIP_ERROR ReadMaxGroupsPerFabric(EndpointId endpoint, AttributeValueEncoder & aEncoder)
    {
        auto * provider = GetGroupDataProvider();
        VerifyOrReturnError(nullptr != provider, CHIP_ERROR_INTERNAL);
        return aEncoder.Encode(provider->GetMaxGroupsPerFabric());
    }
    CHIP_ERROR ReadMaxGroupKeysPerFabric(EndpointId endpoint, AttributeValueEncoder & aEncoder)
    {
        auto * provider = GetGroupDataProvider();
        VerifyOrReturnError(nullptr != provider, CHIP_ERROR_INTERNAL);
        return aEncoder.Encode(provider->GetMaxGroupKeysPerFabric());
    }
};

constexpr uint16_t GroupKeyManagementAttributeAccess::kClusterRevision;

GroupKeyManagementAttributeAccess gAttribute;

} // anonymous namespace

void MatterGroupKeyManagementPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttribute);
}

//
// Commands
//

void emberAfGroupKeyManagementClusterServerInitCallback(chip::EndpointId endpoint) {}

bool emberAfGroupKeyManagementClusterKeySetWriteCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::GroupKeyManagement::Commands::KeySetWrite::DecodableType & commandData)
{
    auto fabric     = commandObj->GetAccessingFabricIndex();
    auto * provider = GetGroupDataProvider();

    if (nullptr == provider)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return true;
    }

    if (commandData.groupKeySet.epochKey0.empty() || (0 == commandData.groupKeySet.epochStartTime0))
    {
        // If the EpochKey0 field is null or its associated EpochStartTime0 field is null,
        // then this command SHALL fail with an INVALID_COMMAND
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
        return true;
    }

    GroupDataProvider::KeySet keyset(commandData.groupKeySet.groupKeySetID, commandData.groupKeySet.securityPolicy, 0);

    // Epoch Key 0
    keyset.epoch_keys[0].start_time = commandData.groupKeySet.epochStartTime0;
    memcpy(keyset.epoch_keys[0].key, commandData.groupKeySet.epochKey0.data(), GroupDataProvider::EpochKey::kLengthBytes);
    keyset.num_keys_used++;

    // Epoch Key 1
    if (!commandData.groupKeySet.epochKey1.empty())
    {
        if (commandData.groupKeySet.epochStartTime1 <= commandData.groupKeySet.epochStartTime0)
        {
            // If the EpochKey1 field is not null, its associated EpochStartTime1 field SHALL contain
            // a later epoch start time than the epoch start time found in the EpochStartTime0 field.
            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
            return true;
        }
        keyset.epoch_keys[1].start_time = commandData.groupKeySet.epochStartTime1;
        memcpy(keyset.epoch_keys[1].key, commandData.groupKeySet.epochKey1.data(), GroupDataProvider::EpochKey::kLengthBytes);
        keyset.num_keys_used++;
    }

    // Epoch Key 2
    if (!commandData.groupKeySet.epochKey2.empty())
    {
        keyset.num_keys_used++;
        if (commandData.groupKeySet.epochStartTime2 <= commandData.groupKeySet.epochStartTime1)
        {
            // If the EpochKey1 field is not null, its associated EpochStartTime1 field SHALL contain
            // a later epoch start time than the epoch start time found in the EpochStartTime0 field.
            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
            return true;
        }
        keyset.epoch_keys[2].start_time = commandData.groupKeySet.epochStartTime2;
        memcpy(keyset.epoch_keys[2].key, commandData.groupKeySet.epochKey2.data(), GroupDataProvider::EpochKey::kLengthBytes);
        keyset.num_keys_used++;
    }

    // Set KeySet
    CHIP_ERROR err = provider->SetKeySet(fabric, keyset);
    if (CHIP_NO_ERROR == err)
    {
        ChipLogDetail(Zcl, "GroupKeyManagementCluster: KeySetWrite OK");
    }
    else
    {
        ChipLogDetail(Zcl, "GroupKeyManagementCluster: KeySetWrite: %s", err.AsString());
    }

    // Send response
    EmberStatus status =
        emberAfSendImmediateDefaultResponse(CHIP_NO_ERROR == err ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);
    if (EMBER_SUCCESS != status)
    {
        ChipLogDetail(Zcl, "GroupKeyManagementCluster: KeySetWrite failed: 0x%x", status);
    }
    return true;
}

bool emberAfGroupKeyManagementClusterKeySetReadCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::GroupKeyManagement::Commands::KeySetRead::DecodableType & commandData)
{
    auto fabric     = commandObj->GetAccessingFabricIndex();
    auto * provider = GetGroupDataProvider();

    if (nullptr == provider)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return true;
    }

    GroupDataProvider::KeySet keyset;
    if (CHIP_NO_ERROR != provider->GetKeySet(fabric, commandData.groupKeySetID, keyset))
    {
        // KeySet ID not found
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
        return true;
    }

    GroupKeyManagement::Commands::KeySetReadResponse::Type response;
    response.groupKeySet.groupKeySetID  = keyset.keyset_id;
    response.groupKeySet.securityPolicy = keyset.policy;
    // Keyset 0
    if (keyset.num_keys_used > 0)
    {
        response.groupKeySet.epochStartTime0 = keyset.epoch_keys[0].start_time;
        response.groupKeySet.epochKey0       = chip::ByteSpan(keyset.epoch_keys[0].key, GroupDataProvider::EpochKey::kLengthBytes);
    }
    else
    {
        response.groupKeySet.epochStartTime0 = 0;
        response.groupKeySet.epochKey0       = chip::ByteSpan(nullptr, 0);
    }
    // Keyset 1
    if (keyset.num_keys_used > 1)
    {
        response.groupKeySet.epochStartTime1 = keyset.epoch_keys[1].start_time;
        response.groupKeySet.epochKey1       = chip::ByteSpan(keyset.epoch_keys[1].key, GroupDataProvider::EpochKey::kLengthBytes);
    }
    else
    {
        response.groupKeySet.epochStartTime1 = 0;
        response.groupKeySet.epochKey1       = chip::ByteSpan(nullptr, 0);
    }
    // Keyset 2
    if (keyset.num_keys_used > 2)
    {
        response.groupKeySet.epochStartTime2 = keyset.epoch_keys[2].start_time;
        response.groupKeySet.epochKey2       = chip::ByteSpan(keyset.epoch_keys[2].key, GroupDataProvider::EpochKey::kLengthBytes);
    }
    else
    {
        response.groupKeySet.epochStartTime2 = 0;
        response.groupKeySet.epochKey2       = chip::ByteSpan(nullptr, 0);
    }

    CHIP_ERROR err = commandObj->AddResponseData(commandPath, response);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogDetail(Zcl, "GroupKeyManagementCluster: KeySetRead failed: %s", ErrorStr(err));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
    return true;
}

bool emberAfGroupKeyManagementClusterKeySetRemoveCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::GroupKeyManagement::Commands::KeySetRemove::DecodableType & commandData)

{
    auto fabric          = commandObj->GetAccessingFabricIndex();
    auto * provider      = GetGroupDataProvider();
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;

    if (nullptr != provider)
    {
        // Remove keyset
        CHIP_ERROR err = provider->RemoveKeySet(fabric, commandData.groupKeySetID);
        if (CHIP_ERROR_KEY_NOT_FOUND == err)
        {
            status = EMBER_ZCL_STATUS_NOT_FOUND;
        }
        else if (CHIP_NO_ERROR == err)
        {
            status = EMBER_ZCL_STATUS_SUCCESS;
        }
    }

    // Send response
    EmberStatus send_status = emberAfSendImmediateDefaultResponse(status);
    if (EMBER_SUCCESS != send_status)
    {
        ChipLogDetail(Zcl, "GroupKeyManagementCluster: KeySetRemove failed: 0x%x", send_status);
    }
    return true;
}

struct KeySetReadAllIndicesResponse
{
    static constexpr CommandId GetCommandId() { return GroupKeyManagement::Commands::KeySetReadAllIndicesResponse::Id; }
    static constexpr ClusterId GetClusterId() { return GroupKeyManagement::Id; }

    GroupDataProvider::KeySetIterator * mIterator = nullptr;

    KeySetReadAllIndicesResponse(GroupDataProvider::KeySetIterator * iter) : mIterator(iter) {}

    CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
    {
        TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));

        TLV::TLVType array;
        ReturnErrorOnFailure(writer.StartContainer(
            TLV::ContextTag(to_underlying(GroupKeyManagement::Commands::KeySetReadAllIndicesResponse::Fields::kGroupKeySetIDs)),
            TLV::kTLVType_Array, array));

        GroupDataProvider::KeySet keyset;
        while (mIterator && mIterator->Next(keyset))
        {
            ReturnErrorOnFailure(app::DataModel::Encode(writer, TLV::AnonymousTag(), keyset.keyset_id));
        }

        ReturnErrorOnFailure(writer.EndContainer(array));
        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
};

bool emberAfGroupKeyManagementClusterKeySetReadAllIndicesCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::GroupKeyManagement::Commands::KeySetReadAllIndices::DecodableType & commandData)
{
    auto fabric     = commandObj->GetAccessingFabricIndex();
    auto * provider = GetGroupDataProvider();

    if (nullptr == provider)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return true;
    }

    auto keysIt = provider->IterateKeySets(fabric);
    if (nullptr == keysIt)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return true;
    }

    CHIP_ERROR err = commandObj->AddResponseData(commandPath, KeySetReadAllIndicesResponse(keysIt));
    if (CHIP_NO_ERROR != err)
    {
        ChipLogDetail(Zcl, "GroupKeyManagementCluster: KeySetReadAllIndices failed: %s", ErrorStr(err));
    }
    keysIt->Release();
    return true;
}
