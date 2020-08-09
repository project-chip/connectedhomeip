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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/****************************************************************************
 * @file
 * @brief CLI for the IAS Zone Client plugin.
 *******************************************************************************
 ******************************************************************************/

#include "af.h"
#include "app/util/serial/command-interpreter2.h"
#include "ias-zone-client.h"

//-----------------------------------------------------------------------------
// Globals

void emAfPluginIasZoneClientPrintServersCommand(void);
void emAfPluginIasZoneClientClearAllServersCommand(void);

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginIasZoneClientCommands[] = {
    emberCommandEntryAction("print-servers", emAfPluginIasZoneClientPrintServersCommand, "", "Print the known IAS Zone Servers"),
    emberCommandEntryAction("clear-all", emAfPluginIasZoneClientClearAllServersCommand, "",
                            "Clear all known IAS Zone Servers from local device"),
    emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

//-----------------------------------------------------------------------------
// Functions

void emAfPluginIasZoneClientPrintServersCommand(void)
{
    uint8_t i;
    emberAfIasZoneClusterPrintln("Index IEEE                 EP   Type   Status State ID");
    emberAfIasZoneClusterPrintln("---------------------------------------------------");
    for (i = 0; i < EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES; i++)
    {
        if (i < 10)
        {
            emberAfIasZoneClusterPrint(" ");
        }
        emberAfIasZoneClusterPrint(
            "%d    (>)%X%X%X%X%X%X%X%X  ", i, emberAfIasZoneClientKnownServers[i].ieeeAddress[7],
            emberAfIasZoneClientKnownServers[i].ieeeAddress[6], emberAfIasZoneClientKnownServers[i].ieeeAddress[5],
            emberAfIasZoneClientKnownServers[i].ieeeAddress[4], emberAfIasZoneClientKnownServers[i].ieeeAddress[3],
            emberAfIasZoneClientKnownServers[i].ieeeAddress[2], emberAfIasZoneClientKnownServers[i].ieeeAddress[1],
            emberAfIasZoneClientKnownServers[i].ieeeAddress[0]);
        if (emberAfIasZoneClientKnownServers[i].endpoint < 10)
        {
            emberAfIasZoneClusterPrint(" ");
        }
        if (emberAfIasZoneClientKnownServers[i].endpoint < 100)
        {
            emberAfIasZoneClusterPrint(" ");
        }
        emberAfIasZoneClusterPrint("%d  ", emberAfIasZoneClientKnownServers[i].endpoint);
        emberAfIasZoneClusterPrintln("0x%2X 0x%2X 0x%X 0x%X", emberAfIasZoneClientKnownServers[i].zoneType,
                                     emberAfIasZoneClientKnownServers[i].zoneStatus, emberAfIasZoneClientKnownServers[i].zoneState,
                                     emberAfIasZoneClientKnownServers[i].zoneId);
    }
}

void emAfPluginIasZoneClientClearAllServersCommand(void)
{
    emAfClearServers();
}
