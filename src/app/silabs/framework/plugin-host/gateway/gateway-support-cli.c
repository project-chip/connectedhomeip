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
/***************************************************************************//**
 * @file
 * @brief Gateway specific CLI behavior for a host application.
 *   In this case we assume our application is running on
 *   a PC with Unix library support, connected to an NCP via serial uart.
 *******************************************************************************
   ******************************************************************************/

// common include file
#include "app/framework/util/common.h"

#include <time.h>

//------------------------------------------------------------------------------
// Forward Declarations

void emberAfPluginGatewaySupportTimeSyncLocal(void);

//------------------------------------------------------------------------------
// Globals

// The difference in seconds between the ZigBee Epoch: January 1st, 2000
// and the Unix Epoch: January 1st 1970.
#define UNIX_ZIGBEE_EPOCH_DELTA (uint32_t)946684800UL

#if !defined(EMBER_AF_GENERATE_CLI)

EmberCommandEntry emberAfPluginGatewayCommands[] = {
  emberCommandEntryAction("time-sync-local",
                          emberAfPluginGatewaySupportTimeSyncLocal,
                          "",
                          "This command retrieves the local unix time and syncs the Time Server attribute to it."),
  emberCommandEntryTerminator(),
};

#endif

//------------------------------------------------------------------------------
// Functions

void emberAfPluginGatewaySupportTimeSyncLocal(void)
{
  time_t unixTime = time(NULL);
  unixTime -= UNIX_ZIGBEE_EPOCH_DELTA;
  emberAfSetTime(unixTime);
  emberAfPrintTime(emberAfGetCurrentTime());
}
