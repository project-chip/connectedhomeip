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

namespace chip {
namespace app {
namespace Clusters {
namespace GroupKeyManagement {

struct GroupStateCodec
{
    static const TLV::Tag kTagFabric    = TLV::ContextTag(to_underlying(Structs::GroupState::Fields::kFabricIndex));
    static const TLV::Tag kTagGroup     = TLV::ContextTag(to_underlying(Structs::GroupState::Fields::kGroupId));
    static const TLV::Tag kTagKeySet    = TLV::ContextTag(to_underlying(Structs::GroupState::Fields::kGroupKeySetID));
    static const TLV::Tag kTagEndpoints = TLV::ContextTag(to_underlying(Structs::GroupState::Fields::kEndpoints));
    static const TLV::Tag kTagGroupName = TLV::ContextTag(to_underlying(Structs::GroupState::Fields::kGroupName));

    GroupDataProvider::GroupState mState;
    GroupDataProvider::GroupMappingIterator * mGroups = nullptr;

    GroupStateCodec(GroupDataProvider::GroupState state, GroupDataProvider::GroupMappingIterator * iter) :
        mState(state), mGroups(iter)
    {}

    CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
    {
        TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
        // FabricIndex
        ReturnErrorOnFailure(DataModel::Encode(writer, kTagFabric, mState.fabric_index));
        // GroupId
        ReturnErrorOnFailure(DataModel::Encode(writer, kTagGroup, mState.group));
        // GroupKeySetID
        ReturnErrorOnFailure(DataModel::Encode(writer, kTagKeySet, mState.keyset_id));
        // Endpoints
        char group_name[GroupDataProvider::GroupMapping::kGroupNameMax + 1] = { 0 };
        size_t name_size                                                    = 0;
        {
            GroupDataProvider::GroupMapping mapping;
            TLV::TLVType inner;
            ReturnErrorOnFailure(writer.StartContainer(kTagEndpoints, TLV::kTLVType_Array, inner));

            while (nullptr != mGroups && mGroups->Next(mapping))
            {
                if (mapping.group == mState.group)
                {
                    // Encode endpoint
                    ReturnErrorOnFailure(DataModel::Encode(writer, TLV::AnonymousTag, mapping.endpoint));
                    // Save group name
                    if ((0 == name_size) && (0 != mapping.name[0]))
                    {
                        name_size = strnlen(mapping.name, GroupDataProvider::GroupMapping::kGroupNameMax);
                        strncpy(group_name, mapping.name, name_size);
                        group_name[name_size] = 0;
                    }
                }
            }
            ReturnErrorOnFailure(writer.EndContainer(inner));
        }
        // GroupName
        ReturnErrorOnFailure(writer.PutString(kTagGroupName, group_name, static_cast<uint32_t>(name_size)));

        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
};

} // namespace GroupKeyManagement
} // namespace Clusters
} // namespace app
} // namespace chip

class GroupKeyManagementAttribute : public AttributeAccessInterface
{
public:
    GroupKeyManagementAttribute() : AttributeAccessInterface(Optional<EndpointId>(0), Clusters::GroupKeyManagement::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        VerifyOrReturnError(GroupKeyManagement::Attributes::Groups::Id == aPath.mAttributeId, CHIP_ERROR_INVALID_ARGUMENT);

        auto * provider = GetGroupDataProvider();
        VerifyOrReturnError(nullptr != provider, CHIP_ERROR_INTERNAL);

        auto statesIt = provider->IterateGroupStates();
        VerifyOrReturnError(nullptr != statesIt, CHIP_ERROR_INTERNAL);

        GroupDataProvider::GroupState state;

        CHIP_ERROR err = aEncoder.EncodeList([&](const TagBoundEncoder & encoder) -> CHIP_ERROR {
            while (statesIt->Next(state))
            {
                // Get groups associated with the state's fabric
                auto groupsIt = provider->IterateGroupMappings(state.fabric_index);
                VerifyOrReturnError(nullptr != groupsIt, CHIP_ERROR_INTERNAL);
                // Encode state
                GroupKeyManagement::GroupStateCodec codec(state, groupsIt);
                encoder.Encode(codec);
                groupsIt->Release();
            }
            return CHIP_NO_ERROR;
        });

        statesIt->Release();
        return err;
    }
};

GroupKeyManagementAttribute gAttribute;

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

    GroupDataProvider::KeySet keyset(commandData.groupKeySet.groupKeySetID, commandData.groupKeySet.groupKeySecurityPolicy, 0);

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
    CHIP_ERROR err = provider->SetKeySet(fabric, commandData.groupKeySet.groupKeySetID, keyset);
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
    response.groupKeySet.groupKeySetID          = keyset.keyset_id;
    response.groupKeySet.groupKeySecurityPolicy = keyset.policy;
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
        if (CHIP_NO_ERROR == provider->RemoveKeySet(fabric, commandData.groupKeySetID))
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
            ReturnErrorOnFailure(app::DataModel::Encode(writer, TLV::AnonymousTag, keyset.keyset_id));
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
