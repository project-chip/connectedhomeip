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
 * @brief Routines for the Sub-Ghz Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/util.h"

#include "stack/include/ember-duty-cycle.h"

#ifdef EMBER_AF_PLUGIN_OTA_SERVER
  #include "app/framework/plugin/ota-server/ota-server.h"
#endif

#include "sub-ghz-server.h"

#define PLUGIN_NAME_STR         "Sub-GHz server"

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

// An array of data to keep track of all the clients
typedef struct {
  // Client's node ID.
  // NOTE: The table is preinitialized on boot to all bytes 0, including node IDs.
  // That means all not yet assigned entries will have nodeId == 0. Fortunately,
  // the SE1.4 spec is on our side and assumes that the Sub-GHz server resides
  // on the comms hub.
  // NOTE: Once a table entry becomes used, it will never become "unused".
  EmberNodeId nodeId;

  // Time until the current suspend period expires. 0 = not suspended.
  // Suspend period is 1-255 min, or up to 1530 sec. We need to track it in seconds,
  // because different nodes may be sent the suspend command at different times,
  // and thus the timer has to have a finer granularity than one minute.
  uint16_t suspendTime;
} ClientTable;
static ClientTable clientTable[EMBER_MAX_CHILDREN_FOR_PER_DEVICE_DUTY_CYCLE_MONITOR] = { 0 };

// As per SE 1.4, a Sub-GHz server, on the transition to the Limited Duty Cycle
// state, uses "an implementation defined method" to choose a client and suspends
// it.
// If that client tries to communicate whilst the server is still in the Limited
// state, the server suspends it again.
// This is the node Id of the choosen client.
static EmberNodeId chosenNodeId = EMBER_NULL_NODE_ID;

// Seconds to minutes calculation
#define SECOND_TO_MINUTES(sec)  (((sec) + 59) / 60)

// The Sub-GHz cluster does not have an endpoint but our send function still
// needs one and fails if it is not provided. So we use endpoint 1 as default.
#define DEFAULT_ENDPOINT        1

//------------------------------------------------------------------------------
// Internal Functions

/** @brief Find a client with the given node ID
 *
 * Returns a pointer to the entry in the client table matching the given node ID
 * or NULL to indicate an entry was not found.
 */
static ClientTable* findClient(EmberNodeId nodeId)
{
  int i;

  // Find an entry in the table with the same node ID
  for (i = 0; i < sizeof clientTable / sizeof clientTable[0]; ++i) {
    if (clientTable[i].nodeId == nodeId) {
      return &clientTable[i];
    }
  }

  // A matching entry not found.
  return NULL;
}

/** @brief Find a client with the given node ID with succeed guaranteed
 *
 * Return a pointer to the client table that can be, in the given order:
 * 1) an entry matching the given node ID;
 * 2) the first unused entry, if any;
 * 3) an entry vacated from the table to make space for the new entry.
 */
static ClientTable* findOrMakeClient(EmberNodeId nodeId)
{
  ClientTable* client = findClient(nodeId);
  if (client == NULL) {
    // We need to find an entry to kick out, choosing the one with the shortest
    // remaining suspend time.
    uint16_t shortestSuspendTime = 0xFFFF;
    int i;
    for (i = 0; i < sizeof clientTable / sizeof clientTable[0]; ++i) {
      // Finding an unused entry has a priority...
      if (clientTable[i].nodeId == 0) {
        client = &clientTable[i];
        break;
      }
      // ...followed by one with the shortest remaining time to overwrite
      if (clientTable[i].suspendTime <= shortestSuspendTime) {
        client = &clientTable[i];
        shortestSuspendTime = clientTable[i].suspendTime;
      }
    }

    // Replacing a previously used client's entry, start from scratch
    client->nodeId = nodeId;
  }

  return client;
}

/** @brief Update client's remaining suspend time
 */
static void updateClientSuspendTime(ClientTable* client,
                                    uint8_t period)
{
  client->suspendTime = 60 * period;
  if (period != 0) {
    // Note: If the timer is already running, e.g. because another client is
    // already suspended, and there is let's say half a second to the next tick,
    // then this will reset that half a second to a full csecond. Fortunately,
    // a) our tick period is one second, so we will never get too far off, and
    // b) suspening new clients is (hopefully) not taking place very frequently
    emberAfScheduleServerTick(emberAfPrimaryEndpointForCurrentNetworkIndex(),
                              ZCL_SUB_GHZ_CLUSTER_ID,
                              MILLISECOND_TICKS_PER_SECOND);
  }
}

/** @brief Incoming ZDO message handler
 *
 * Calls the user's emberAfPluginSubGhzUnsolicitedEnhancedUpdateNotifyCallback
 * if the incoming message is Mgmt_NWK_Unsolicited_Enhanced_Update_notify.
 * That's it.
 */
void emAfSubGhzServerZDOMessageReceivedCallback(EmberNodeId sender,
                                                const uint8_t* payloadBuffer,
                                                uint16_t payloadLength)
{
  if (payloadBuffer != NULL
      && payloadLength >= NWK_UNSOLICITED_ENHANCED_UPDATE_NOTIFY_CMD_PAYLOAD_LENGTH) {
    uint32_t channelMask;
    uint8_t channelPage;
    uint8_t channel;
    uint16_t macTxUcastTotal;
    uint16_t macTxUcastFailures;
    uint16_t macTxUcastRetries;
    uint8_t period;

    emberAfSubGhzClusterPrintln("%p: Mgmt_NWK_Unsolicited_Enhanced_Update_notify received",
                                PLUGIN_NAME_STR);

    channelMask = INT8U_TO_INT32U(payloadBuffer[5], payloadBuffer[4], payloadBuffer[3], payloadBuffer[2]);
    channelPage = channelMask >> EMBER_MAX_CHANNELS_PER_PAGE;

    // Convert channel bitmask into a bit position (0x0000001 = 0, 0x04000000 = 26)
    for (channel = 0; channel < EMBER_MAX_CHANNELS_PER_PAGE && (channelMask & 0x01) == 0; ++channel) {
      channelMask >>= 1;
    }

    if (emAfValidateChannelPages(channelPage, channel) != EMBER_SUCCESS) {
      emberAfSubGhzClusterPrintln("Invalid channel mask: 0x%4x (page %d)",
                                  channelMask,
                                  channelPage);
      return;
    }

    macTxUcastTotal = HIGH_LOW_TO_INT(payloadBuffer[7], payloadBuffer[6]);
    macTxUcastFailures = HIGH_LOW_TO_INT(payloadBuffer[9], payloadBuffer[8]);
    macTxUcastRetries = HIGH_LOW_TO_INT(payloadBuffer[11], payloadBuffer[10]);
    period = payloadBuffer[12];

    emberAfSubGhzClusterPrintln("Current page      : %d", channelPage);
    emberAfSubGhzClusterPrintln("Current channel   : %d", channel);
    emberAfSubGhzClusterPrintln("MAC Tx Ucast Total: %d", macTxUcastTotal);
    emberAfSubGhzClusterPrintln("MAC Tx Ucast Fail : %d", macTxUcastFailures);
    emberAfSubGhzClusterPrintln("MAC Tx Ucast Retry: %d", macTxUcastRetries);
    emberAfSubGhzClusterPrintln("Sample period     : %d", period);

    // Leave it to the application to handle it
    emberAfPluginSubGhzUnsolicitedEnhancedUpdateNotifyCallback(channelPage,
                                                               channel,
                                                               macTxUcastTotal,
                                                               macTxUcastFailures,
                                                               macTxUcastRetries,
                                                               period);
  }
}

/** @brief Send the 'SuspendZclMessages command.
 */
static EmberStatus sendSuspendZclMessagesCommand(EmberNodeId nodeId,
                                                 uint8_t period)
{
  emberAfFillCommandSubGhzClusterSuspendZclMessages(period);
  emberAfSetCommandEndpoints(emberAfPrimaryEndpointForCurrentNetworkIndex(),
                             DEFAULT_ENDPOINT);
  return emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, nodeId);
}

//------------------------------------------------------------------------------
// Global APIs provided by the plugin

/** @brief Send the 'Suspend ZCL Messages' command.
 *
 * See prototype in sub-ghz-server.h for a detailed description.
 */
EmberStatus emberAfPluginSubGhzServerSendSuspendZclMessagesCommand(EmberNodeId nodeId,
                                                                   uint8_t endpoint,
                                                                   uint8_t period)
{
  if (!emberIsZigbeeBroadcastAddress(nodeId)) {
    // Update the client's suspend time
    ClientTable* client = findOrMakeClient(nodeId);
    updateClientSuspendTime(client, period);
    return sendSuspendZclMessagesCommand(nodeId, period);
  } else {
    // Broadcast is not allowed
    return EMBER_BAD_ARGUMENT;
  }
}

/** @brief Get the current suspend status for a given client.
 *
 * @return  the number of seconds remaining; 0 = not suspended
 */
uint16_t emberAfPluginSubGhzServerSuspendZclMessagesStatus(EmberNodeId nodeId)
{
  const ClientTable* client = findClient(nodeId);
  return client ? client->suspendTime : 0;
}

//------------------------------------------------------------------------------
// Non-public global functions

/** @brief An incoming message handler, called from emAfIncomingMessageHandler
 *
 * Based on the Duty Cycle state, client's ID (has the client been suspended?)
 * and the message type, determines the right action, which could be one of:
 * - let the message through to allow the framework to process it further;
 * - respond with 'Suspend ZCL Messages' and suppress this message
 * - suppress the message silently
 *
 * @return true if the message has been processed by the Sub-GHz plugin and
 *         should be suppressed, false to allow the message through
 *
 * Note:
 * We have to juggle some contradictory requirements:
 * - if the message is an OTA block request and we are in the "Critical" duty
 *   cycle mode, respond with "Wait for data"
 * - otherwise, if the message came from a suspended client, respond with
 *   "Suspend ZCL Messages"...
 * - ...UNLESS the message is a Default Response to a previous suspend command,
 *   to prevent a message loop.
 * This means we have to at least partially parse the incoming message BEFORE
 * we decide whether to let it through or not for parsing.
 */
boolean emAfSubGhzServerIncomingMessage(EmberIncomingMessageType type,
                                        EmberApsFrame *apsFrame,
                                        EmberNodeId sender,
                                        uint16_t messageLength,
                                        uint8_t *messageContents)
{
  EmberDutyCycleState dcState;
  ClientTable* client = findOrMakeClient(sender);

  if (emberGetDutyCycleState(&dcState) != EMBER_SUCCESS) {
    dcState = EMBER_DUTY_CYCLE_TRACKING_OFF;    // fallback to a sensible default
  } else if (client->suspendTime == 0
             && (dcState >= EMBER_DUTY_CYCLE_LBT_CRITICAL_THRESHOLD_REACHED
                 || (dcState >= EMBER_DUTY_CYCLE_LBT_LIMITED_THRESHOLD_REACHED
                     && sender == chosenNodeId))) {
    // Client's expired suspension needs to be renewed
    updateClientSuspendTime(client, EMBER_AF_PLUGIN_SUB_GHZ_SERVER_SUSPEND_PERIOD);
  }

  if (client->suspendTime > 0) {
    // First, sort out special cases like the OTA block requests and default
    // responses to previous suspend commands
    if (apsFrame->profileId == SE_PROFILE_ID
        && (apsFrame->clusterId == ZCL_OTA_BOOTLOAD_CLUSTER_ID
            || apsFrame->clusterId == ZCL_SUB_GHZ_CLUSTER_ID)) {
      EmberAfClusterCommand cmd;
      if (emberAfProcessMessageIntoZclCmd(apsFrame,
                                          type,
                                          messageContents,
                                          messageLength,
                                          sender,
                                          NULL, // interPanHeader, not needed here
                                          &cmd)) {
# ifdef EMBER_AF_PLUGIN_OTA_SERVER
        if (dcState >= EMBER_DUTY_CYCLE_LBT_CRITICAL_THRESHOLD_REACHED
            && apsFrame->clusterId == ZCL_OTA_BOOTLOAD_CLUSTER_ID
            && cmd.direction == ZCL_DIRECTION_CLIENT_TO_SERVER
            && cmd.clusterSpecific
            && cmd.commandId == ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID) {
          emberAfCorePrintln("%p: client %2x %p within its suspend period.",
                             PLUGIN_NAME_STR,
                             sender,
                             "requesting an OTA block");
          // Leave the responding with "wait for data" to the OTA plugin itself
          return false;
        } else
# endif // EMBER_AF_PLUGIN_OTA_SERVER
        if (apsFrame->clusterId == ZCL_SUB_GHZ_CLUSTER_ID
            && cmd.direction == ZCL_DIRECTION_CLIENT_TO_SERVER
            && !cmd.clusterSpecific
            && cmd.commandId == ZCL_DEFAULT_RESPONSE_COMMAND_ID) {
          // Silently drop a default response to a Sub-GHz server command
          // to prevent an infinite "suspend -> default response" message loop
          return true;
        }
      }
    }

    // Special cases sorted, back to the general case: send "Suspend ZCL Messages"
    if (sender != emberAfGetNodeId()) { // do not accidentally suspend ourselves
      emberAfCorePrintln("%p: client %2x %p within its suspend period.",
                         PLUGIN_NAME_STR,
                         sender,
                         "transmitting");
      sendSuspendZclMessagesCommand(sender, SECOND_TO_MINUTES(client->suspendTime));
    }
    return true;
  }

  // Message not handled by this plugin
  return false;
}

//------------------------------------------------------------------------------
// Sub-GHz Library Callbacks

/** @brief Duty cycle state change callback, code shared between SoC and NCP.
 */
static void jointDutyCycleHandler(uint8_t channelPage,
                                  uint8_t channel,
                                  EmberDutyCycleState newState,
                                  uint8_t totalDevices,
                                  EmberPerDeviceDutyCycle *arrayOfDutyCycles)
{
  static uint8_t /*EmberDutyCycleState*/ oldState = EMBER_DUTY_CYCLE_TRACKING_OFF;

  emberAfSubGhzClusterPrintln("%p: Duty Cycle handler called: page=%d, chan=%d, state=%d (previous=%d)",
                              PLUGIN_NAME_STR,
                              channelPage,
                              channel,
                              newState,
                              oldState);

  // Consult the user if they want to override the default decision
  if (emberAfPluginSubGhzDutyCycleCallback(channelPage,
                                           channel,
                                           oldState,
                                           newState)) {
    // The callback returning true means the user app has handled the callback
    // which means there is nothing for us to do.
  } else if (oldState < EMBER_DUTY_CYCLE_LBT_CRITICAL_THRESHOLD_REACHED
             && newState >= EMBER_DUTY_CYCLE_LBT_CRITICAL_THRESHOLD_REACHED) {
    // If the transition is from "Limited" to "Critical", suspend everyone.
    uint8_t i;
    for (i = 0; i < emberAfGetChildTableSize(); ++i) {
      EmberNodeId const nodeId = emberGetAddressTableRemoteNodeId(i);
      if (nodeId != EMBER_UNKNOWN_NODE_ID
          && nodeId != EMBER_DISCOVERY_ACTIVE_NODE_ID
          && nodeId != EMBER_TABLE_ENTRY_UNUSED_NODE_ID) {
        emberAfPluginSubGhzServerSendSuspendZclMessagesCommand(nodeId,
                                                               0, // use the saved EP (default to 1 if unknown)
                                                               EMBER_AF_PLUGIN_SUB_GHZ_SERVER_SUSPEND_PERIOD);
      }
    }
  } else if (oldState < EMBER_DUTY_CYCLE_LBT_LIMITED_THRESHOLD_REACHED
             && newState >= EMBER_DUTY_CYCLE_LBT_LIMITED_THRESHOLD_REACHED) {
    // If the transition is from "Normal" to "Limited", find a single client to suspend.
    // We choose the most talkative client that has not been suspended yet.
    // Failing that, we choose the one with the shortest remaining suspend time.
    uint8_t i;
    EmberDutyCycleHectoPct highestsDutyCycle = 0;

    chosenNodeId = EMBER_NULL_NODE_ID;

    // Iterate through the duty cycle results from the stack to find the candidate.
    // It must be one that has not been suspended yet.
    // Start from 1 to skip the cumulative duty cycle for the local device.
    for (i = 1; i < totalDevices; ++i) {
      const EmberNodeId currentNode = arrayOfDutyCycles[i].nodeId;
      const EmberDutyCycleHectoPct currentDutyCycle = arrayOfDutyCycles[i].dutyCycleConsumed;

      emberAfSubGhzClusterPrintln("%p: Checking client %2x, %p = %d",
                                  PLUGIN_NAME_STR,
                                  currentNode,
                                  "duty cycle",
                                  currentDutyCycle);

      if (currentNode != 0xFFFF                 // skip unused entries
          && currentNode != emberAfGetNodeId()  // skip ourselves
          && currentDutyCycle > highestsDutyCycle) {
        // Cross-check the node with our list. It can be used if unknown to us yet
        // (i.e. it is not suspended) or if the remaining suspend period is 0.
        const ClientTable* client = findClient(currentNode);
        if (!client
            || client->suspendTime == 0) {
          highestsDutyCycle = currentDutyCycle;
          chosenNodeId = currentNode;
          emberAfSubGhzClusterPrintln("%p: Found new candidate to suspend",
                                      PLUGIN_NAME_STR);
        }
      }
    }

    // If we have not found the candidate in the pool of duty cycle results
    // from the stack, we need to pick one with the least suspend time remaining.
    if (chosenNodeId == EMBER_NULL_NODE_ID) {
      uint16_t shortestSuspendTime = 0xFFFF;
      for (i = 0; i < sizeof clientTable / sizeof clientTable[0]; ++i) {
        emberAfSubGhzClusterPrintln("%p: Checking client %2x, %p = %d",
                                    PLUGIN_NAME_STR,
                                    clientTable[i].nodeId,
                                    "time left",
                                    clientTable[i].suspendTime);
        if (clientTable[i].nodeId != 0
            && clientTable[i].suspendTime < shortestSuspendTime) {
          shortestSuspendTime = clientTable[i].suspendTime;
          chosenNodeId = clientTable[i].nodeId;
          emberAfSubGhzClusterPrintln("%p: Found new candidate to suspend",
                                      PLUGIN_NAME_STR);
        }
      }
    }

    // There is still a chance that we have not found a suitable client, however
    // the only possible reason at this point is that our clients table is still
    // empty (unlikely given we are inside the "duty cycle limit reached" callback).
    if (chosenNodeId != EMBER_NULL_NODE_ID) {
      emberAfSubGhzClusterPrintln("%p: Suspending client %2x",
                                  PLUGIN_NAME_STR,
                                  chosenNodeId);
      emberAfPluginSubGhzServerSendSuspendZclMessagesCommand(chosenNodeId,
                                                             0, // use the saved EP (default to 1 if unknown)
                                                             EMBER_AF_PLUGIN_SUB_GHZ_SERVER_SUSPEND_PERIOD);
    }
  }

  oldState = newState;
}

/** @brief A duty cycle state change callback
 *
 * Please note that the Duty Cycle callbacks differ between SoC and NCP:
 * - on SoC, the information about the per-node duty cycles needs to be obtained
 *   by calling emberGetCurrentDutyCycle() inside the handler
 * - on NCP, to improve performance by avoiding the need for another xNCP call,
 *   the callback has extra two parameters containing the duty cycle information
 */
#ifndef EZSP_HOST
void emberDutyCycleHandler(uint8_t channelPage,
                           uint8_t channel,
                           EmberDutyCycleState newState)
#else
void ezspDutyCycleHandler(uint8_t channelPage,
                          uint8_t channel,
                          EmberDutyCycleState newState,
                          // EMZIGBEE-1173: the following two arguments do not contain valid values in 2017/Q3 Beta
                          // As a workaround, call emberGetCurrentDutyCycle() inside the handler like we do on SoC
                          uint8_t bogo_totalDevices,
                          EmberPerDeviceDutyCycle *bogo_arrayOfDutyCycles)
#endif
{
// Uncomment once EMZIGBEE-1173 is fixed
// #ifndef EZSP_HOST
  EmberPerDeviceDutyCycle arrayOfDutyCycles[EMBER_MAX_CHILDREN_FOR_PER_DEVICE_DUTY_CYCLE_MONITOR + 1];
  uint8_t totalDevices;

  // emAfPluginSubGhzServerDutyCycleHandler above uses arrayOfDutyCycles only
  // on the transition from the "normal" to "limited" state. If the callback is
  // about a different transition, we do not need to waste time querying the
  // per-node cycles and can use any value for totalDevices.
  if (newState >= EMBER_DUTY_CYCLE_LBT_LIMITED_THRESHOLD_REACHED
      && newState < EMBER_DUTY_CYCLE_LBT_CRITICAL_THRESHOLD_REACHED
      && emberGetCurrentDutyCycle(sizeof arrayOfDutyCycles / sizeof arrayOfDutyCycles[0],
                                  arrayOfDutyCycles) == EMBER_SUCCESS) {
    totalDevices = sizeof arrayOfDutyCycles / sizeof arrayOfDutyCycles[0];
  } else {
    totalDevices = 0;
  }
// Uncomment once EMZIGBEE-1173 is fixed
// #endif

  jointDutyCycleHandler(channelPage,
                        channel,
                        newState,
                        totalDevices,
                        arrayOfDutyCycles);
}

/** @brief Sub-GHz Server Get Suspend Zcl Messages Status
 *
 * Called when the server receives the command from the client.
 */
bool emberAfSubGhzClusterGetSuspendZclMessagesStatusCallback(void)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();
  if (cmd) {    // sanity, since we dereference cmd
    const uint16_t suspendTime = emberAfPluginSubGhzServerSuspendZclMessagesStatus(cmd->source);
    sendSuspendZclMessagesCommand(cmd->source, SECOND_TO_MINUTES(suspendTime));
  }
  return true;
}

/** @brief Sub-GHz Cluster Server Tick
 *
 * Server Tick.
 * Used for keeping the Suspend ZCL Messages counters up to date.
 */
void emberAfSubGhzClusterServerTickCallback(uint8_t endpoint)
{
  boolean timerStillNeeded = false;
  uint8_t i;

  // Go through the table of suspended nodes and decrement the timers
  for (i = 0; i < sizeof clientTable / sizeof clientTable[0]; ++i) {
    if (clientTable[i].suspendTime > 0) {
      if (--clientTable[i].suspendTime > 0) {
        timerStillNeeded = true;
      }
    }
  }

  // Schedule the next tick if required
  if (timerStillNeeded) {
    emberAfScheduleServerTick(endpoint, ZCL_SUB_GHZ_CLUSTER_ID, MILLISECOND_TICKS_PER_SECOND);
  }
}
