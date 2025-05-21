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

// #include "multicast-server.h"
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

#include <credentials/MulticastDataProvider.h>
#include <lib/support/BytesToHex.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Multicast;
using Protocols::InteractionModel::Status;

namespace {

class MulticastAttributeAccess : public AttributeAccessInterface
{
public:
    // Register for the Multicast cluster on all endpoints.
    MulticastAttributeAccess() : AttributeAccessInterface(Optional<EndpointId>(0), Clusters::Multicast::Id) {}

    // TODO: Once there is MCSP support, this may need to change.
    static constexpr bool IsMCSPSupported() { return false; }
    static constexpr uint16_t kImplementedClusterRevision = 2;

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        VerifyOrDie(aPath.mClusterId == Clusters::Multicast::Id);

        switch (aPath.mAttributeId)
        {
        case Attributes::Targets::Id:
            return ReadMulticastTargets(aPath.mEndpointId, aEncoder);
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
    static constexpr uint16_t kClusterRevision = 1;

    CHIP_ERROR ReadMulticastTargets(EndpointId endpoint, AttributeValueEncoder & aEncoder)
    {
        chip::Multicast::DataProvider & multicast = chip::Multicast::DataProvider::Instance();

        CHIP_ERROR err = aEncoder.EncodeList([&multicast](const auto & encoder) -> CHIP_ERROR {
            CHIP_ERROR status = CHIP_NO_ERROR;
            for (auto & fabric : Server::GetInstance().GetFabricTable())
            {
                auto fabric_index = fabric.GetFabricIndex();
                auto iter         = multicast.IterateTargets(fabric_index);
                VerifyOrReturnError(nullptr != iter, CHIP_ERROR_NO_MEMORY);
                chip::Multicast::Target entry;
                while (iter->Next(entry))
                {
                    Clusters::Multicast::Structs::MulticastTargetStruct::Type target;
                    target.fabricIndex = fabric_index;
                    target.groupId     = entry.group_id;
                    target.endpoints   = DataModel::List<const chip::EndpointId>(entry.endpoints, entry.endpoint_count);
                    status             = encoder.Encode(target);
                    ChipLogProgress(Crypto, "~~~ Multicast::ReadMulticastTargets: g:#%04x, ec:%u #%02x", (unsigned) target.groupId,
                                    (unsigned) target.endpoints.size(), (unsigned) status.AsInteger());
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
};

MulticastAttributeAccess gAttribute;

} // namespace

void MatterMulticastPluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&gAttribute);
}

void emberAfMulticastClusterServerInitCallback(chip::EndpointId endpoint) {}

bool emberAfMulticastClusterSetTargetCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Multicast::Commands::SetTarget::DecodableType & commandData)
{
    Server & server                           = Server::GetInstance();
    const FabricInfo * fabric                 = server.GetFabricTable().FindFabricWithIndex(commandObj->GetAccessingFabricIndex());
    chip::Multicast::DataProvider & multicast = chip::Multicast::DataProvider::Instance();
    CHIP_ERROR err                            = CHIP_NO_ERROR;

    // Collect arguments
    chip::Multicast::Target target;
    uint8_t key[chip::Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0 };
    size_t key_size                                                   = 0;

    // Target
    target.group_id = commandData.groupId;

    // Endpoints
    target.endpoint_count = 0;
    auto iter             = commandData.endpoints.begin();
    while (iter.Next() && (target.endpoint_count < chip::Multicast::kEndpointsMax))
    {
        target.endpoints[target.endpoint_count++] = iter.GetValue();
    }

    // Parse Key
    VerifyOrExit(commandData.key.size() == 2 * sizeof(key), err = CHIP_ERROR_INVALID_LIST_LENGTH);
    key_size = chip::Encoding::HexToBytes((char *) commandData.key.data(), commandData.key.size(), key, sizeof(key));
    VerifyOrExit(key_size == sizeof(key), err = CHIP_ERROR_INVALID_LIST_LENGTH);

    // Store multicast configuration
    err = multicast.SetTarget(fabric, target, ByteSpan(key), commandData.gracePeriod);
    ChipLogProgress(Crypto, "~~~ Multicast::Set: #%02x", (unsigned) err.AsInteger());

exit:
    // Send response
    commandObj->AddStatus(commandPath, chip::app::StatusIB(err).mStatus);
    return true;
}

bool emberAfMulticastClusterRemoveTargetCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Multicast::Commands::RemoveTarget::DecodableType & commandData)
{
    return false;
}
