/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
 *    All rights reserved.
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
#include "NetworkCommissioningCluster.h"

#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/NetworkCommissioning/AttributeIds.h>
#include <clusters/NetworkCommissioning/CommandIds.h>
#include <clusters/NetworkCommissioning/Enums.h>
#include <clusters/NetworkCommissioning/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/StatusCode.h>

#include <optional>

namespace chip {
namespace app {
namespace Clusters {

DataModel::ActionReturnStatus NetworkCommissioningCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                         AttributeValueEncoder & encoder)
{
    using namespace NetworkCommissioning::Attributes;

    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(mLogic.Features());
    case ClusterRevision::Id:
        return encoder.Encode(NetworkCommissioning::kRevision);
        // FIXME: implement the rest
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }

#if 0
    switch (aPath.mAttributeId)
    {
    case Attributes::MaxNetworks::Id:
        return aEncoder.Encode(mpBaseDriver->GetMaxNetworks());

    case Attributes::Networks::Id:
        return aEncoder.EncodeList([this](const auto & encoder) {
            CHIP_ERROR err = CHIP_NO_ERROR;
            Structs::NetworkInfoStruct::Type networkForEncode;
            EnumerateAndRelease(mpBaseDriver->GetNetworks(), [&](const Network & network) {
                networkForEncode.networkID = ByteSpan(network.networkID, network.networkIDLen);
                networkForEncode.connected = network.connected;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
                // These fields are both optional and nullable in NetworkInfoStruct.
                // If PDC is supported, the fields are always present but may be null.
                if (mFeatureFlags.Has(Feature::kPerDeviceCredentials))
                {
                    networkForEncode.networkIdentifier = MakeOptional(Nullable<ByteSpan>(network.networkIdentifier));
                    networkForEncode.clientIdentifier  = MakeOptional(Nullable<ByteSpan>(network.clientIdentifier));
                }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

                err = encoder.Encode(networkForEncode);
                return (err == CHIP_NO_ERROR) ? Loop::Continue : Loop::Break;
            });
            return err;
        });

    case Attributes::ScanMaxTimeSeconds::Id:
        if (mpWirelessDriver != nullptr)
        {
            return aEncoder.Encode(mpWirelessDriver->GetScanNetworkTimeoutSeconds());
        }
        return CHIP_NO_ERROR;

    case Attributes::ConnectMaxTimeSeconds::Id:
        if (mpWirelessDriver != nullptr)
        {
            return aEncoder.Encode(mpWirelessDriver->GetConnectNetworkTimeoutSeconds());
        }
        return CHIP_NO_ERROR;

    case Attributes::InterfaceEnabled::Id:
        return aEncoder.Encode(mpBaseDriver->GetEnabled());

    case Attributes::LastNetworkingStatus::Id:
        return aEncoder.Encode(mLastNetworkingStatusValue);

    case Attributes::LastNetworkID::Id:
        if (mLastNetworkIDLen == 0)
        {
            return aEncoder.EncodeNull();
        }
        else
        {
            return aEncoder.Encode(ByteSpan(mLastNetworkID, mLastNetworkIDLen));
        }

    case Attributes::LastConnectErrorValue::Id:
        return aEncoder.Encode(mLastConnectErrorValue);

    case Attributes::SupportedWiFiBands::Id: {
#if (CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP)
        // TODO https://github.com/project-chip/connectedhomeip/issues/31431
        // This is a case of shared zap config where mandatory wifi attributes are enabled for a thread platform (e.g
        // all-cluster-app). Real world product must only enable the attributes tied to the network technology supported by their
        // product. Temporarily return an list of 1 element of value 0 when wifi is not supported or WiFiNetworkInterface is not
        // enabled until a solution is implemented with the attribute list.
        // Final implementation will return UnsupportedAttribute if we get here without the needed WiFi support .
        // VerifyOrReturnError(mFeatureFlags.Has(Feature::kWiFiNetworkInterface), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
        if (mFeatureFlags.Has(Feature::kWiFiNetworkInterface))
        {
            return aEncoder.EncodeList([this](const auto & encoder) {
                uint32_t bands = mpDriver.Get<WiFiDriver *>()->GetSupportedWiFiBandsMask();

                // Extract every band from the bitmap of supported bands, starting positionally on the right.
                for (uint32_t band_bit_pos = 0; band_bit_pos < std::numeric_limits<uint32_t>::digits; ++band_bit_pos)
                {
                    uint32_t band_mask = static_cast<uint32_t>(1UL << band_bit_pos);
                    if ((bands & band_mask) != 0)
                    {
                        ReturnErrorOnFailure(encoder.Encode(static_cast<WiFiBandEnum>(band_bit_pos)));
                    }
                }
                return CHIP_NO_ERROR;
            });
        }
#endif
        return aEncoder.EncodeList([](const auto & encoder) {
            WiFiBandEnum bands = WiFiBandEnum::k2g4;
            ReturnErrorOnFailure(encoder.Encode(bands));
            return CHIP_NO_ERROR;
        });
    }
    break;
    case Attributes::SupportedThreadFeatures::Id: {
        // TODO https://github.com/project-chip/connectedhomeip/issues/31431
        BitMask<ThreadCapabilities> ThreadCapabilities = 0;
#if (CHIP_DEVICE_CONFIG_ENABLE_THREAD)
        // This is a case of shared zap config where mandatory thread attributes are enabled for a wifi platform (e.g
        // all-cluster-app). Real world product must only enable the attributes tied to the network technology supported by their
        // product. Temporarily encode a value of 0 reflecting no thread capabilities hen CHIP_DEVICE_CONFIG_ENABLE_THREAD or
        // ThreadNetworkInterface are not enabled until a solution is implemented with the attribute list.
        // Final implementation will return UnsupportedAttribute if we get here without the needed thread support
        // VerifyOrReturnError(mFeatureFlags.Has(Feature::kThreadNetworkInterface), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
        if (mFeatureFlags.Has(Feature::kThreadNetworkInterface))
        {
            ThreadCapabilities = mpDriver.Get<ThreadDriver *>()->GetSupportedThreadFeatures();
        }
#endif
        return aEncoder.Encode(ThreadCapabilities);
    }
    break;
    case Attributes::ThreadVersion::Id: {
        // TODO https://github.com/project-chip/connectedhomeip/issues/31431ß
        uint16_t threadVersion = 0;
#if (CHIP_DEVICE_CONFIG_ENABLE_THREAD)
        // This is a case of shared zap config where mandatory thread attributes are enabled for a wifi platform (e.g
        // all-cluster-app) Real world product must only enable the attributes tied to the network technology supported by their
        // product. Temporarily encode a value of 0 reflecting no thread version when CHIP_DEVICE_CONFIG_ENABLE_THREAD or
        // ThreadNetworkInterface are not enabled until a solution is implemented with the attribute list.
        // Final implementation will return UnsupportedAttribute if we get here without the needed thread support
        // VerifyOrReturnError(mFeatureFlags.Has(Feature::kThreadNetworkInterface), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
        if (mFeatureFlags.Has(Feature::kThreadNetworkInterface))
        {
            threadVersion = mpDriver.Get<ThreadDriver *>()->GetThreadVersion();
        }
#endif
        return aEncoder.Encode(threadVersion);
    }
    break;
    default:
        return CHIP_NO_ERROR;
    }
#endif
}

DataModel::ActionReturnStatus NetworkCommissioningCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                          AttributeValueDecoder & decoder)
{
    using namespace NetworkCommissioning::Attributes;

    if (request.path.mAttributeId == InterfaceEnabled::Id)
    {
        bool value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return mLogic.SetInterfaceEnabled(value);
    }

    return Protocols::InteractionModel::Status::InvalidAction;
}

std::optional<DataModel::ActionReturnStatus> NetworkCommissioningCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                        TLV::TLVReader & input_arguments,
                                                                                        CommandHandler * handler)
{

    // FIXME: implement
    return Protocols::InteractionModel::Status::UnsupportedCommand;
#if 0
    if (mAsyncCommandHandle.Get() != nullptr)
    {
        // We have a command processing in the backend, reject all incoming commands.
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::Busy);
        ctxt.SetCommandHandled();
        return;
    }

    // Since mPath is used for building the response command, and we have checked that we are not pending the response of another
    // command above. So it is safe to set the mPath here and not clear it when return.
    mPath = ctxt.mRequestPath;

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::ScanNetworks::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface) || mFeatureFlags.Has(Feature::kThreadNetworkInterface));
        HandleCommand<Commands::ScanNetworks::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleScanNetworks(ctx, req); });
        return;

    case Commands::AddOrUpdateWiFiNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface));
        HandleCommand<Commands::AddOrUpdateWiFiNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleAddOrUpdateWiFiNetwork(ctx, req); });
        return;

    case Commands::AddOrUpdateThreadNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kThreadNetworkInterface));
        HandleCommand<Commands::AddOrUpdateThreadNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleAddOrUpdateThreadNetwork(ctx, req); });
        return;

    case Commands::RemoveNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface) || mFeatureFlags.Has(Feature::kThreadNetworkInterface));
        HandleCommand<Commands::RemoveNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleRemoveNetwork(ctx, req); });
        return;

    case Commands::ConnectNetwork::Id: {
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface) || mFeatureFlags.Has(Feature::kThreadNetworkInterface));

        HandleCommand<Commands::ConnectNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleConnectNetwork(ctx, req); });
        return;
    }

    case Commands::ReorderNetwork::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kWiFiNetworkInterface) || mFeatureFlags.Has(Feature::kThreadNetworkInterface));
        HandleCommand<Commands::ReorderNetwork::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleReorderNetwork(ctx, req); });
        return;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    case Commands::QueryIdentity::Id:
        VerifyOrReturn(mFeatureFlags.Has(Feature::kPerDeviceCredentials));
        HandleCommand<Commands::QueryIdentity::DecodableType>(
            ctxt, [this](HandlerContext & ctx, const auto & req) { HandleQueryIdentity(ctx, req); });
        return;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    }
#endif
}

CHIP_ERROR NetworkCommissioningCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                         ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    using namespace NetworkCommissioning::Commands;
    using NetworkCommissioning::Feature;

    if (mLogic.Features().Has(Feature::kThreadNetworkInterface))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            ScanNetworks::kMetadataEntry,
            AddOrUpdateThreadNetwork::kMetadataEntry,
            RemoveNetwork::kMetadataEntry,
            ConnectNetwork::kMetadataEntry,
            ReorderNetwork::kMetadataEntry,
        }));
    }
    else if (mLogic.Features().Has(Feature::kWiFiNetworkInterface))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            ScanNetworks::kMetadataEntry,
            AddOrUpdateWiFiNetwork::kMetadataEntry,
            RemoveNetwork::kMetadataEntry,
            ConnectNetwork::kMetadataEntry,
            ReorderNetwork::kMetadataEntry,
        }));
    }

    if (mLogic.Features().Has(Feature::kPerDeviceCredentials))
    {
        ReturnErrorOnFailure(builder.AppendElements({ QueryIdentity::kMetadataEntry }));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR NetworkCommissioningCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                                          ReadOnlyBufferBuilder<CommandId> & builder)
{
    using namespace NetworkCommissioning::Commands;
    using NetworkCommissioning::Feature;

    ReturnErrorOnFailure(builder.EnsureAppendCapacity(4));

    if (mLogic.Features().HasAny(Feature::kWiFiNetworkInterface, Feature::kThreadNetworkInterface))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            ScanNetworksResponse::Id,
            NetworkConfigResponse::Id,
            ConnectNetworkResponse::Id,
        }));
    }

    if (mLogic.Features().Has(Feature::kPerDeviceCredentials))
    {
        ReturnErrorOnFailure(builder.Append(QueryIdentityResponse::Id));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR NetworkCommissioningCluster::Attributes(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    // FIXME: implement: this is where existing code IS LACKING!!!!
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Clusters
} // namespace app
} // namespace chip
