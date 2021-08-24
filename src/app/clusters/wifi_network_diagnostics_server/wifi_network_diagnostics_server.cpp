/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <app/CommandHandler.h>
#include <app/util/af.h>

using namespace chip;
using namespace chip::app::Clusters;

bool emberAfWiFiNetworkDiagnosticsClusterResetCountsCallback(EndpointId endpoint, app::CommandHandler * commandObj)
{
    EmberAfStatus status = WiFiNetworkDiagnostics::Attributes::SetBeaconLostCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset BeaconLostCount attribute"));

    status = WiFiNetworkDiagnostics::Attributes::SetBeaconRxCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset BeaconRxCount attribute"));

    status = WiFiNetworkDiagnostics::Attributes::SetPacketMulticastRxCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset PacketMulticastRxCount attribute"));

    status = WiFiNetworkDiagnostics::Attributes::SetPacketMulticastTxCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset PacketMulticastTxCount attribute"));

    status = WiFiNetworkDiagnostics::Attributes::SetPacketUnicastRxCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset PacketUnicastRxCount attribute"));

    status = WiFiNetworkDiagnostics::Attributes::SetPacketUnicastTxCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset PacketUnicastTxCount attribute"));

    status = WiFiNetworkDiagnostics::Attributes::SetOverrunCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset OverrunCount attribute"));

exit:
    emberAfSendImmediateDefaultResponse(status);

    return true;
}
