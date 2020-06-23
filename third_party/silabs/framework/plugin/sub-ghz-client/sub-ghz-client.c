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
/***************************************************************************//**
 * @file
 * @brief Routines for the Sub-Ghz Client plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/util.h"
#include "app/framework/util/af-main.h"                 // emAfSend()
#include "zigbee-framework/zigbee-device-common.h"      // emberNextZigDevRequestSequence()

#include "sub-ghz-client.h"

//------------------------------------------------------------------------------
// Internal declarations

// The Mgmt_NWK_Unsolicited_Enhanced_Update_notify command payload:
// Offset Length Desription
//    0     1    Sequence No
//    1     1    Status - always 0; pointless but consistent with other "notify" commands
//    2     4    Channel Mask - 32 bits, top 5 for page No, the rest a bitmask for channels
//    6     2    MAC Ucast Total
//    8     2    MAC Ucast Failures
//   10     2    MAC Ucast Retries
//   12     1    Period - in minutes
// The overall length of the command payload is 13.
#define NWK_UNSOLICITED_ENHANCED_UPDATE_NOTIFY_CMD_PAYLOAD_LENGTH       13

// A bitmask representing the current suspend status
#define SUSPEND_STATUS_SUSPENDED        0x01
#define SUSPEND_STATUS_SUSPEND_IGNORED  0x02
static uint8_t suspendStatus = 0;

//------------------------------------------------------------------------------
// Global APIs provided by the plugin

/** Send the 'Get Suspend ZCL Messages Status' command.
 *
 * See prototype in sub-ghz-client.h for a detailed description.
 */
EmberStatus emberAfPluginSubGhzClientSendGetSuspendZclMessagesStatusCommand(EmberNodeId nodeId,
                                                                            uint8_t endpoint)
{
  const uint8_t sourceEndpoint = emberAfPrimaryEndpointForCurrentNetworkIndex();
  emberAfFillCommandSubGhzClusterGetSuspendZclMessagesStatus();
  emberAfSetCommandEndpoints(sourceEndpoint,
                             endpoint);
  return emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
}

/** @brief Sends a request to change the channel to the server by means of the
 * Mgmt_NWK_Unsolicited_Enhanced_Update_notify command.
 *
 * See prototype in sub-ghz-client.h for a detailed description.
 */
EmberStatus emberAfPluginSubGhzClientSendUnsolicitedEnhancedUpdateNotify(EmberNodeId destination,
                                                                         uint8_t channelPage,
                                                                         uint8_t channel,
                                                                         uint16_t macTxUcastTotal,
                                                                         uint16_t macTxUcastFailures,
                                                                         uint16_t macTxUcastRetries,
                                                                         uint8_t period)
{
  EmberApsFrame apsFrame = {
    EMBER_ZDO_PROFILE_ID,
    NWK_UNSOLICITED_ENHANCED_UPDATE_NOTIFY,
    EMBER_ZDO_ENDPOINT,
    EMBER_ZDO_ENDPOINT,
    0,  // no APS ACK, otherwise use EMBER_APS_OPTION_RETRY
    0,  // group id
    0,  // sequence
  };
  uint8_t messageTag = 0xFF;    // INVALID_MESSAGE_TAG
  uint32_t channelMask;
  uint8_t payload[NWK_UNSOLICITED_ENHANCED_UPDATE_NOTIFY_CMD_PAYLOAD_LENGTH];
  uint8_t i = 0;

  // Sanity check. Allow only page 0, 28, 29, 30 or 31 and only channel 0-26.
  if (emAfValidateChannelPages(channelPage, channel) != EMBER_SUCCESS) {
    return EMBER_BAD_ARGUMENT;
  }

  // Broadcast not allowed. The spec says we unicast this msg to the network manager.
  // We still leave the destination open for testing purposes.
  if (emberIsZigbeeBroadcastAddress(destination)) {
    return EMBER_BAD_ARGUMENT;
  }

  channelMask = (channelPage << EMBER_MAX_CHANNELS_PER_PAGE) | BIT32(channel);

  payload[i++] = emberNextZigDevRequestSequence();
  payload[i++] = 0;     // status code, always success
  payload[i++] = BYTE_0(channelMask);
  payload[i++] = BYTE_1(channelMask);
  payload[i++] = BYTE_2(channelMask);
  payload[i++] = BYTE_3(channelMask);
  payload[i++] = LOW_BYTE(macTxUcastTotal);
  payload[i++] = HIGH_BYTE(macTxUcastTotal);
  payload[i++] = LOW_BYTE(macTxUcastFailures);
  payload[i++] = HIGH_BYTE(macTxUcastFailures);
  payload[i++] = LOW_BYTE(macTxUcastRetries);
  payload[i++] = HIGH_BYTE(macTxUcastRetries);
  payload[i++] = period;

  assert(i == NWK_UNSOLICITED_ENHANCED_UPDATE_NOTIFY_CMD_PAYLOAD_LENGTH);

  return emAfSend(EMBER_OUTGOING_DIRECT,
                  destination,
                  &apsFrame,
                  sizeof payload,
                  payload,
                  &messageTag,
                  0,    // alias
                  0);   // sequence
}

/** Return the current suspend status.
 *
 * @return  true if called within a suspend period, false otherwise
 */
boolean emberAfPluginSubGhzClientIsSendingZclMessagesSuspended()
{
  return ((suspendStatus & (SUSPEND_STATUS_SUSPENDED | SUSPEND_STATUS_SUSPEND_IGNORED)) == SUSPEND_STATUS_SUSPENDED);
}

/** @brief Ignore the SuspendZclMessages command.
 *
 * When the client receives the SuspendZclMessages command, it suspends sending
 * any ZCL messages for a given time. This function allows the client to ignore
 * the command and continue sending messages even when suspended.
 *
 * @param ignore  true to ignore, false to switch back to normal behaviour
 */
void emberAfPluginSubGhzClientIgnoreSuspendZclMessagesCommand(boolean ignore)
{
  if (ignore) {
    suspendStatus |= SUSPEND_STATUS_SUSPEND_IGNORED;
  } else {
    suspendStatus &= ~SUSPEND_STATUS_SUSPEND_IGNORED;
  }
}

//------------------------------------------------------------------------------
// Sub-GHz Library Callbacks

/** @brief Cluster Suspend Zcl Messages callback
 *
 * Called when the client receives the command from the server.
 */
bool emberAfSubGhzClusterSuspendZclMessagesCallback(uint8_t period)
{
  // Check we are on the sub-GHz interface
  EmberNodeType nodeType;
  EmberNetworkParameters parameters;
  if (emberAfGetNetworkParameters(&nodeType, &parameters) != EMBER_SUCCESS) {
    emberAfDebugPrintln("Error: Could not determine node type");
    return false;
  } else if (parameters.radioChannel < 128) {
    emberAfDebugPrintln("Ignoring SuspendZclMessages received on a 2.4GHz channel");
    return false;
  }

  if (!emberAfPluginSubGhzSuspendZclMessagesCallback(period)) {
    // Suspend ZCL messages for 'period' minutes.
    suspendStatus |= SUSPEND_STATUS_SUSPENDED;

    emberAfDebugPrintln("%p suspended for %d minutes",
                        "Sub-GHz client: sending of ZCL messages",
                        period);

    // Set a timer to clear the suspend status.
    const uint8_t sourceEndpoint = emberAfPrimaryEndpointForCurrentNetworkIndex();
    emberAfScheduleClientTick(sourceEndpoint, ZCL_SUB_GHZ_CLUSTER_ID, period * MILLISECOND_TICKS_PER_MINUTE);
  }

  if (period == 0) {
    // Send Default Response
    EmberAfClusterCommand * const cmd = emberAfCurrentCommand();
    if (cmd) {  // sanity, in case emberAfCurrentCommand() returns NULL
      emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_SUCCESS);
    }
  }
  return true;
}

/** @brief Sub-GHz Cluster Client Tick
 *
 * Client Tick.
 * Used for timing out the client's suspend state.
 * The timer is started in emberAfSubGhzClusterSuspendZclMessagesCallback above,
 * set to trigger after a given number of minutes. So triggering the timer
 * simply means our suspend period is over and we can start sending messages again.
 */
void emberAfSubGhzClusterClientTickCallback(uint8_t endpoint)
{
  // Cancel the suspend status. That's it.
  suspendStatus &= ~SUSPEND_STATUS_SUSPENDED;

  emberAfDebugPrintln("%p resumed",
                      "Sub-GHz client: sending of ZCL messages");
}
