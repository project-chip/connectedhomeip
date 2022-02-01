/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "network-commissioning.h"

#include <cstring>

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::NetworkCommissioning;

// TODO: this is a TEMPORARY solution for enabling tests. The all-zero version
// is not spec compliang and will generally not even commission on thread/wifi
// because of a missing feature map
//
// Instead of this, the app::Clusters::NetworkCommissioning::Instance class
// from the "network-commissioning" cluster (note no "-old" suffix) should
// be used.
#ifdef __APPLE__
#define FAKE_NETWORK_COMMISSIONING_ATTRIBUTES
#else
#undef FAKE_NETWORK_COMMISSIONING_ATTRIBUTES
#endif

namespace {

#ifdef FAKE_NETWORK_COMMISSIONING_ATTRIBUTES
class NetworkCommissioningAttributeAccess : public AttributeAccessInterface
{
public:
    NetworkCommissioningAttributeAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        switch (aPath.mAttributeId)
        {
        case Attributes::MaxNetworks::Id:
            return aEncoder.Encode(static_cast<uint8_t>(0));
        case Attributes::Networks::Id:
            return aEncoder.EncodeEmptyList();
        case Attributes::ScanMaxTimeSeconds::Id:
            return aEncoder.Encode(static_cast<uint8_t>(0));
        case Attributes::ConnectMaxTimeSeconds::Id:
            return aEncoder.Encode(static_cast<uint8_t>(0));
        case Attributes::InterfaceEnabled::Id:
            return aEncoder.Encode(static_cast<bool>(false));
        case Attributes::LastNetworkingStatus::Id:
            return aEncoder.Encode(NetworkCommissioningStatus::kSuccess);
        case Attributes::LastNetworkID::Id:
            return aEncoder.Encode(ByteSpan());
        case Attributes::LastConnectErrorValue::Id:
            return aEncoder.Encode(Attributes::LastConnectErrorValue::TypeInfo::Type(static_cast<int32_t>(0)));
        case Attributes::FeatureMap::Id:
            return aEncoder.Encode(static_cast<uint32_t>(0));
        default:
            return CHIP_NO_ERROR;
        }
    }
};
#endif // FAKE_NETWORK_COMMISSIONING_ATTRIBUTES
} // namespace

bool emberAfNetworkCommissioningClusterAddOrUpdateThreadNetworkCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::AddOrUpdateThreadNetwork::DecodableType & commandData)
{
    auto & operationalDataset = commandData.operationalDataset;
    auto & breadcrumb         = commandData.breadcrumb;

    app::Clusters::NetworkCommissioning::OnAddOrUpdateThreadNetworkCommandCallbackInternal(
        commandObj, commandPath, operationalDataset, breadcrumb, 0 /* ignored timeout ms */);
    return true;
}

bool emberAfNetworkCommissioningClusterAddOrUpdateWiFiNetworkCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::AddOrUpdateWiFiNetwork::DecodableType & commandData)
{
    auto & ssid        = commandData.ssid;
    auto & credentials = commandData.credentials;
    auto & breadcrumb  = commandData.breadcrumb;

    app::Clusters::NetworkCommissioning::OnAddOrUpdateWiFiNetworkCommandCallbackInternal(commandObj, commandPath, ssid, credentials,
                                                                                         breadcrumb, 0 /* ignored timeout ms */);
    return true;
}

bool emberAfNetworkCommissioningClusterConnectNetworkCallback(app::CommandHandler * commandObj,
                                                              const app::ConcreteCommandPath & commandPath,
                                                              const Commands::ConnectNetwork::DecodableType & commandData)
{
    auto & networkID  = commandData.networkID;
    auto & breadcrumb = commandData.breadcrumb;

    app::Clusters::NetworkCommissioning::OnConnectNetworkCommandCallbackInternal(commandObj, commandPath, networkID, breadcrumb,
                                                                                 0 /* ignored timeout ms */);
    return true;
}

// TODO: The following commands needed to be implemented.
// These commands are not implemented thus not handled yet, return false so ember will return a error.

bool emberAfNetworkCommissioningClusterRemoveNetworkCallback(app::CommandHandler * commandObj,
                                                             const app::ConcreteCommandPath & commandPath,
                                                             const Commands::RemoveNetwork::DecodableType & commandData)
{
    return false;
}

bool emberAfNetworkCommissioningClusterScanNetworksCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::ScanNetworks::DecodableType & commandData)
{
    return false;
}

bool emberAfNetworkCommissioningClusterReorderNetworkCallback(app::CommandHandler * commandObj,
                                                              const app::ConcreteCommandPath & commandPath,
                                                              const Commands::ReorderNetwork::DecodableType & commandData)
{
    return false;
}

void MatterNetworkCommissioningPluginServerInitCallback()
{

#ifdef FAKE_NETWORK_COMMISSIONING_ATTRIBUTES
    static NetworkCommissioningAttributeAccess gAttrAccess;
    registerAttributeAccessOverride(&gAttrAccess);
#endif // FAKE_NETWORK_COMMISSIONING_ATTRIBUTES
}
