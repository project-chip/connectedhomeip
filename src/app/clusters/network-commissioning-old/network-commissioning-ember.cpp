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

#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/enums.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>

using namespace chip;
using namespace chip::app::Clusters::NetworkCommissioning;

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

void MatterNetworkCommissioningPluginServerInitCallback() {}
