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
 * @brief CLI for the WWAH Server Silabs plugin.
 *******************************************************************************
   ******************************************************************************/

#include "af.h"
#include "wwah-server-silabs.h"

static void powerNotificationCb(EmberPowerDownNotificationResult result);

// plugin wwah-server poweringOff <srcEndpoint:1> <dstEndpoint:1>
//                                <reason:1> <mfgId:2>
void emAfWwahServerSendPoweringOff(void)
{
  uint8_t srcEndpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t dstEndpoint = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t reason = (uint8_t)emberUnsignedCommandArgument(2);
  uint16_t mfgId = (uint16_t)emberUnsignedCommandArgument(3);

  emberAfSlWwahClusterPrintln("Sending Powering Off Notification");
  emberAfPluginWwahServerSendPoweringOffNotification(EMBER_ZIGBEE_COORDINATOR_ADDRESS,
                                                     srcEndpoint,
                                                     dstEndpoint,
                                                     reason,
                                                     mfgId,
                                                     NULL, 0,
                                                     powerNotificationCb);
}

static void powerNotificationCb(EmberPowerDownNotificationResult result)
{
  emberAfDebugPrintln("Power Notification CB: result=%d", result);
}

// plugin wwah-server poweringOn <srcEndpoint:1> <dstEndpoint:1>
//                               <reason:1> <mfgId:2>
void emAfWwahServerSendPoweringOn(void)
{
  uint8_t srcEndpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t dstEndpoint = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t reason = (uint8_t)emberUnsignedCommandArgument(2);
  uint16_t mfgId = (uint16_t)emberUnsignedCommandArgument(3);

  emberAfSlWwahClusterPrintln("Sending Powering On Notification");
  emberAfPluginWwahServerSendPoweringOnNotification(EMBER_ZIGBEE_COORDINATOR_ADDRESS,
                                                    srcEndpoint,
                                                    dstEndpoint,
                                                    reason,
                                                    mfgId,
                                                    NULL, 0);
}

// plugin wwah-server confMode <mode:1>
void emAfSlWwahServerConfigurationMode(void)
{
  uint8_t mode = (uint8_t)emberUnsignedCommandArgument(0);
  if (mode) {
    emberAfSlWwahClusterEnableConfigurationModeCallback();
  } else {
    emberAfSlWwahClusterDisableConfigurationModeCallback();
  }
}
