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
#include <clusters/NetworkCommissioning/Commands.h>
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
        return encoder.Encode(mLogic.Features().Raw());
    case ClusterRevision::Id:
        return encoder.Encode(NetworkCommissioning::kRevision);
    case MaxNetworks::Id:
        return encoder.Encode(mLogic.GetMaxNetworks());
    case ScanMaxTimeSeconds::Id:
        return encoder.Encode(mLogic.GetScanNetworkTimeoutSeconds());
    case ConnectMaxTimeSeconds::Id:
        return encoder.Encode(mLogic.GetConnectNetworkTimeoutSeconds());
    case InterfaceEnabled::Id:
        return encoder.Encode(mLogic.GetInterfaceEnabled());
    case LastNetworkingStatus::Id:
        return encoder.Encode(mLogic.GetLastNetworkingStatus());
    case LastNetworkID::Id:
        if (mLogic.GetLastNetworkID().empty())
        {
            return encoder.EncodeNull();
        }
        return encoder.Encode(mLogic.GetLastNetworkID());
    case LastConnectErrorValue::Id:
        return encoder.Encode(mLogic.GetLastConnectErrorValue());
    case Networks::Id:
        return mLogic.EncodeNetworks(encoder);
#if (CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP)
    case SupportedWiFiBands::Id:
        return mLogic.EncodeSupportedWiFiBands(encoder);
#endif // (CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP)
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    case SupportedThreadFeatures::Id:
        return encoder.Encode(mLogic.GetThreadCapabilities());
    case ThreadVersion::Id:
        VerifyOrReturnError(mLogic.Features().Has(NetworkCommissioning::Feature::kThreadNetworkInterface),
                            Protocols::InteractionModel::Status::UnsupportedAttribute);
        return encoder.Encode(mLogic.GetThreadVersion());
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
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
    using namespace NetworkCommissioning::Commands;

    VerifyOrReturnError(!mLogic.IsProcessingAsyncCommand(), Protocols::InteractionModel::Status::Busy);

    switch (request.path.mCommandId)
    {
#if (CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP || CHIP_DEVICE_CONFIG_ENABLE_THREAD)
    case ScanNetworks::Id: {
        ScanNetworks::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.HandleScanNetworks(*handler, request.path, request_data);
    }
    case RemoveNetwork::Id: {
        RemoveNetwork::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.HandleRemoveNetwork(*handler, request.path, request_data);
    }
    case ConnectNetwork::Id: {
        ConnectNetwork::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.HandleConnectNetwork(*handler, request.path, request_data);
    }
    case ReorderNetwork::Id: {
        ReorderNetwork::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.HandleReorderNetwork(*handler, request.path, request_data);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP || CHIP_DEVICE_CONFIG_ENABLE_THREAD
#if (CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP)
    case AddOrUpdateWiFiNetwork::Id: {
        AddOrUpdateWiFiNetwork::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.HandleAddOrUpdateWiFiNetwork(*handler, request.path, request_data);
    }
#endif // (CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP)
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    case AddOrUpdateThreadNetwork::Id: {
        AddOrUpdateThreadNetwork::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.HandleAddOrUpdateThreadNetwork(*handler, request.path, request_data);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    case QueryIdentity::Id: {
        QueryIdentity::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));
        return mLogic.HandleQueryIdentity(*handler, request.path, request_data);
    }
#endif
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR NetworkCommissioningCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                         ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    using namespace NetworkCommissioning::Commands;
    using NetworkCommissioning::Feature;

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
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
    else
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
#if (CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP)
        if (mLogic.Features().Has(Feature::kWiFiNetworkInterface))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            ScanNetworks::kMetadataEntry,
            AddOrUpdateWiFiNetwork::kMetadataEntry,
            RemoveNetwork::kMetadataEntry,
            ConnectNetwork::kMetadataEntry,
            ReorderNetwork::kMetadataEntry,
        }));
    }
    else
#endif // (CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP)
    {}

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
    using namespace NetworkCommissioning::Attributes;
    using NetworkCommissioning::Feature;

    // mandatory attributes
    ReturnErrorOnFailure(builder.AppendElements({
        MaxNetworks::kMetadataEntry,
        Networks::kMetadataEntry,
        InterfaceEnabled::kMetadataEntry,
        LastNetworkingStatus::kMetadataEntry,
        LastNetworkID::kMetadataEntry,
        LastConnectErrorValue::kMetadataEntry,
    }));

    // NOTE: thread and wifi are mutually exclusive features
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (mLogic.Features().Has(Feature::kThreadNetworkInterface))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            ScanMaxTimeSeconds::kMetadataEntry,
            ConnectMaxTimeSeconds::kMetadataEntry,
            SupportedThreadFeatures::kMetadataEntry,
            ThreadVersion::kMetadataEntry,
        }));
    }
    else
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
#if (CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP)
        if (mLogic.Features().Has(Feature::kWiFiNetworkInterface))
    {
        ReturnErrorOnFailure(builder.AppendElements({
            ScanMaxTimeSeconds::kMetadataEntry,
            ConnectMaxTimeSeconds::kMetadataEntry,
            SupportedWiFiBands::kMetadataEntry,
        }));
    }
    else
#endif // (CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP)
    {}

    return builder.AppendElements(DefaultServerCluster::GlobalAttributes());
}

} // namespace Clusters
} // namespace app
} // namespace chip
