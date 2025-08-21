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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/MessageDef/StatusIB.h>
#include <app/reporting/reporting.h>
#include <app/server/Server.h>
#include <app/util/config.h>
#include <inttypes.h>
#include <lib/support/CodeUtils.h>
#include <tracing/macros.h>

#include <credentials/GroupcastDataProvider.h>
#include <lib/support/BytesToHex.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Groupcast;
using Protocols::InteractionModel::Status;

namespace {

class GroupcastAttributeAccess : public AttributeAccessInterface
{
public:
    // Register for the Groupcast cluster on all endpoints.
    GroupcastAttributeAccess() : AttributeAccessInterface(Optional<EndpointId>(0), Clusters::Groupcast::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        VerifyOrDie(aPath.mClusterId == Clusters::Groupcast::Id);

        switch (aPath.mAttributeId)
        {
        case Attributes::Membership::Id:
            return ReadMembership(aPath.mEndpointId, aEncoder);
        case Attributes::MaxMembershipCount::Id:
            return ReadMaxMembershipCount(aPath.mEndpointId, aEncoder);
        default:
            break;
        }
        return CHIP_ERROR_READ_FAILED;
    }

    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override
    {
        return CHIP_ERROR_WRITE_FAILED;
    }

private:

    CHIP_ERROR ReadMembership(EndpointId endpoint, AttributeValueEncoder & aEncoder)
    {
        chip::Groupcast::DataProvider &provider = chip::Groupcast::DataProvider::Instance();

        CHIP_ERROR err = aEncoder.EncodeList([&provider](const auto & encoder) -> CHIP_ERROR {
            CHIP_ERROR status = CHIP_NO_ERROR;
            for (auto & fabric : Server::GetInstance().GetFabricTable())
            {
                auto fabric_index = fabric.GetFabricIndex();
                auto iter         = provider.IterateGroups(fabric_index);
                VerifyOrReturnError(nullptr != iter, CHIP_ERROR_NO_MEMORY);
                chip::Groupcast::Group entry;
                while (iter->Next(entry))
                {
                    Clusters::Groupcast::Structs::MembershipStruct::Type target;
                    target.fabricIndex = fabric_index;
                    target.groupId     = entry.group_id;
                    target.endpoints   = DataModel::List<const chip::EndpointId>(entry.endpoints, entry.endpoint_count);
                    status             = encoder.Encode(target);
                    if (status != CHIP_NO_ERROR)
                    {
                        break;
                    }
                }
                iter->Release();
                if (status != CHIP_NO_ERROR)
                {
                    break;
                }
            }
            return status;
        });
        return err;
    }

    CHIP_ERROR ReadMaxMembershipCount(EndpointId endpoint, AttributeValueEncoder & aEncoder)
    {
        chip::Groupcast::DataProvider &provider = chip::Groupcast::DataProvider::Instance();
        return aEncoder.Encode(provider.GetMaxMembershipCount());
    }
};

GroupcastAttributeAccess gAttribute;

} // namespace

void MatterGroupcastPluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&gAttribute);
}

void emberAfGroupcastClusterServerInitCallback(chip::EndpointId endpoint) {}

bool emberAfGroupcastClusterJoinGroupCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Groupcast::Commands::JoinGroup::DecodableType & commandData)
{
    static constexpr size_t kLengthBytes = Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES;
    Server & server                          = Server::GetInstance();
    const FabricInfo * fabric                = server.GetFabricTable().FindFabricWithIndex(commandObj->GetAccessingFabricIndex());
    uint8_t compressed_fabric_id_buffer[sizeof(uint64_t)];
    MutableByteSpan compressed_fabric_id(compressed_fabric_id_buffer);
    chip::Groupcast::DataProvider &provider = chip::Groupcast::DataProvider::Instance();
    chip::Groupcast::Group target;
    uint8_t key[kLengthBytes] = { 0 };
    size_t key_size   = 0;
    CHIP_ERROR err  = CHIP_NO_ERROR;

    // Collect arguments

    // Compressed fabric Id
    VerifyOrExit(nullptr != fabric, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(CHIP_NO_ERROR == fabric->GetCompressedFabricIdBytes(compressed_fabric_id), err = CHIP_ERROR_INTERNAL);

    // Group
    target.group_id = commandData.groupId;

    // Endpoints
    target.endpoint_count = 0;
    {
        auto iter             = commandData.endpoints.begin();
        while (iter.Next() && (target.endpoint_count < chip::Groupcast::kEndpointsMax))
        {
            target.endpoints[target.endpoint_count++] = iter.GetValue();
        }
    }

    // Parse Key
    VerifyOrExit(commandData.key.size() == 2 * sizeof(key), err = CHIP_ERROR_INVALID_LIST_LENGTH);
    key_size = chip::Encoding::HexToBytes((char *) commandData.key.data(), commandData.key.size(), key, sizeof(key));
    VerifyOrExit(key_size == sizeof(key), err = CHIP_ERROR_INVALID_LIST_LENGTH);

    // Store provider configuration
    err = provider.JoinGroup(fabric->GetFabricIndex(), compressed_fabric_id, target, ByteSpan(key), commandData.gracePeriod);
    VerifyOrExit(CHIP_NO_ERROR == err,);

    server.GetTransportManager().MulticastGroupJoinLeave(Transport::PeerAddress::Groupcast(), true);
exit:

    // Send response
    commandObj->AddStatus(commandPath, chip::app::StatusIB(err).mStatus);
    return true;
}

bool emberAfGroupcastClusterLeaveGroupCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Groupcast::Commands::LeaveGroup::DecodableType & commandData)
{
    return false;
}

bool emberAfGroupcastClusterLeaveAllGroupsCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Groupcast::Commands::LeaveAllGroups::DecodableType & commandData)
{
    return false;
}

bool emberAfGroupcastClusterUpdateGroupKeyCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Groupcast::Commands::UpdateGroupKey::DecodableType & commandData)
{
    return false;
}