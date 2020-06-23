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
 * @brief Routines for the Comms Hub Function plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include "app/framework/plugin/gbcs-device-log/gbcs-device-log.h"
#include "app/framework/plugin/meter-mirror/meter-mirror.h"
#include "app/framework/plugin/sleepy-message-queue/sleepy-message-queue.h"
#include "app/framework/plugin/events-server/events-server.h"
#include "comms-hub-function.h"
#include "comms-hub-tunnel-endpoints.h"
#include "tunnel-manager.h"

// default sleep message timeout is 24 hours
#define DEFAULT_SLEEPY_MSG_TIMEOUT_SEC (60 * 60 * 24)
static uint32_t defaultMessageTimeout = DEFAULT_SLEEPY_MSG_TIMEOUT_SEC;

// Tunnel Manager Header values per GNBCS spec
#define TUNNEL_MANAGER_HEADER_GET          0x01
#define TUNNEL_MANAGER_HEADER_GET_RESPONSE 0x02
#define TUNNEL_MANAGER_HEADER_PUT          0x03

static uint8_t currentDeviceLogEntry = 0;
static bool discoveryInProgress = false;

static const uint8_t PLUGIN_NAME[] = "CommsHubFunction";

static EmberNodeId tunnelTargetNodeId = EMBER_NULL_NODE_ID;
static uint8_t tunnelTargetAttempts = 0;

// Unfortunately both the attempt to do node ID discovery and Match descriptor
// are each considered separate "attempts" and thus reduce the number of total attempts,
// even in the case that both succeed without any issues.
// However differentiating between node ID discovery attempts (to reset this back to zero)
// and keep track of Match Descriptor events separates is complicated and ultimately less important.
// So we set the attempts high enough such that in the succesful case we don't hit the max attempts,
// one or two failures won't hit the limit.  But repeated failures will (for example, a node offline
// and unresponsive).
#define MAX_TUNNEL_TARGET_ATTEMPTS 5

EmberEventControl emberAfPluginCommsHubFunctionTunnelCheckEventControl;

#define PLUGIN_DEBUG
#if defined(PLUGIN_DEBUG)
#define pluginDebugPrint(...)   emberAfCorePrint(__VA_ARGS__)
#define pluginDebugPrintln(...) emberAfCorePrintln(__VA_ARGS__)
#define pluginDebugExec(x)      (x)
#define pluginDebugPrintBuffer(buffer, len, withSpace) emberAfPrintBuffer(EMBER_AF_PRINT_CORE, (buffer), (len), (withSpace))
#else
#define pluginDebugPrint(...)
#define pluginDebugPrintln(...)
#define pluginDebugExec(x)
#define pluginDebugPrintBuffer(x, y, z)
#endif

// As long as this plugin requires the sleepy-message-queue plugin, this #define
// will exist.
static uint16_t sleepyMessageUseCaseCodes[EMBER_AF_PLUGIN_SLEEPY_MESSAGE_QUEUE_SLEEPY_QUEUE_SIZE];

//------------------------------------------------------------------------------
// Forward Declarations

static EmberAfPluginCommsHubFunctionStatus setTunnelMessagePending(EmberEUI64 deviceId);
static EmberAfPluginCommsHubFunctionStatus clearTunnelMessagePending(EmberEUI64 deviceId);
static void tunnelDiscoveryCallback(const EmberAfServiceDiscoveryResult *result);
static void initiateDiscovery(EmberNodeId nodeId, EmberEUI64 deviceEui64);
static void checkForAnyDeviceThatNeedsTunnelCreated(void);
static bool checkForSpecificDeviceThatNeedsTunnelCreated(EmberNodeId nodeId,
                                                         EmberEUI64 deviceEui64);

//------------------------------------------------------------------------------
// API functions

void emberAfPluginCommsHubFunctionInitCallback(void)
{
  discoveryInProgress = false;
  emAfPluginCommsHubFunctionTunnelInit(EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_COMMSHUB_TUNNEL_CLIENT_ENDPOINT);
  emberAfPluginTunnelingEndpointInit();
}

EmberAfPluginCommsHubFunctionStatus emberAfPluginCommsHubFunctionSend(EmberEUI64 destinationDeviceId,
                                                                      uint16_t length,
                                                                      uint8_t *payload,
                                                                      uint16_t messageCode)
{
  EmberAfGBCSDeviceLogInfo deviceInfo;
  EmberAfSleepyMessage sleepyMessage;
  EmberAfSleepyMessageId sleepyMessageId;
  EmberAfPluginCommsHubFunctionStatus status;
  EmberAfEvent event;
  uint8_t * eventData = event.eventData;
  bool isSleepyDevice = false;

  eventData[0] = 0x00;

  //  Check to make sure the destination device is in the device log.
  if (!emberAfPluginGbcsDeviceLogGet(destinationDeviceId, &deviceInfo)) {
    char * msg = "CHF: Given destination device ID has not been added to the GBCS device log";

    emberAfPluginCommsHubFunctionPrintln(msg);
    status = EMBER_AF_CHF_STATUS_NO_ACCESS;
    goto kickout;
  }

  // If it's a sleepy device then we must queue the message for later delivery
  // using the GET, GET_RESPONSE, PUT protocol as defined in the GBCS spec.
  if (emberAfPluginGbcsDeviceLogIsSleepyType(deviceInfo.deviceType)) {
    EmberAfPluginCommsHubFunctionStatus chfStatus;
    isSleepyDevice = true;

    // set mirror notification flags and queue the data for the sleepy device
    chfStatus = setTunnelMessagePending(destinationDeviceId);
    if (chfStatus != EMBER_AF_CHF_STATUS_SUCCESS) {
      char * msg = "CHF: Unable to set mirror notification flags and queue the data for the sleepy device ";

      emberAfPluginCommsHubFunctionPrintln(msg);
      status = chfStatus;
      goto kickout;
    }

    MEMCOPY(sleepyMessage.dstEui64, destinationDeviceId, EUI64_SIZE);
    sleepyMessage.length = length;
    sleepyMessage.payload = payload;
    sleepyMessageId = emberAfPluginSleepyMessageQueueStoreMessage(&sleepyMessage, defaultMessageTimeout);
    if (sleepyMessageId == EMBER_AF_PLUGIN_SLEEPY_MESSAGE_INVALID_ID) {
      char * msg = "CHF: Unable to add message to sleepy message queue ";

      emberAfPluginCommsHubFunctionPrintln(msg);
      status = EMBER_AF_CHF_STATUS_TOO_MANY_PEND_MESSAGES;
      goto kickout;
    }

    sleepyMessageUseCaseCodes[sleepyMessageId] = messageCode;
    status = EMBER_AF_CHF_STATUS_SUCCESS;
  } else {
    isSleepyDevice = false;

    // Not a sleepy device so send the message now.
    if (emAfPluginCommsHubFunctionTunnelSendData(destinationDeviceId, 0, NULL, length, payload)) {
      status = EMBER_AF_CHF_STATUS_SUCCESS;
    } else {
      char * msg = "CHF: Unable to send the message through tunnel to destination";
      status = EMBER_AF_CHF_STATUS_TUNNEL_FAILURE;

      emberAfPluginCommsHubFunctionPrintln(msg);
    }

    emberAfPluginCommsHubFunctionSendCallback(status,
                                              destinationDeviceId,
                                              length,
                                              payload);
  }

  kickout:
  if ((status == EMBER_AF_CHF_STATUS_SUCCESS)
      && (isSleepyDevice)) {
    // msg is queued for later delivery.
    // can't really determine if msg is sent/actioned or not yet.
  } else {
    if (status == EMBER_AF_CHF_STATUS_SUCCESS) {
      event.eventId = GBCS_EVENT_ID_IMM_HAN_CMD_RXED_ACTED;
    } else {
      event.eventId = GBCS_EVENT_ID_IMM_HAN_CMD_RXED_NOT_ACTED;
    }
    // EMAPPFWKV2-1315 - "For any Event Log entries relating to Event Codes
    // 0x0054 and 0x0055, the Device shall record the Commands received on the
    // Network Interface by including the Message Code in the Event Log"
    eventData[0] = 0x02;
    eventData[1] = HIGH_BYTE(messageCode);
    eventData[2] = LOW_BYTE(messageCode);

    event.eventTime = emberAfGetCurrentTime();
    emberAfEventsServerAddEvent(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_REMOTE_COMMSHUB_ENDPOINT,
                                EMBER_ZCL_EVENT_LOG_ID_GENERAL_EVENT_LOG,
                                &event);
  }
  return status;
}

void emAfPluginCommsHubFunctionSetDefaultTimeout(uint32_t timeout)
{
  defaultMessageTimeout = timeout;
}

//------------------------------------------------------------------------------
// Callback Functions

/** @brief Trust Center Join
 *
 * This callback is called from within the application framework's
 * implementation of emberTrustCenterJoinHandler or ezspTrustCenterJoinHandler.
 * This callback provides the same arguments passed to the
 * TrustCenterJoinHandleI. For more information about the TrustCenterJoinHandler
 * please see documentation included in stack/include/trust-center.h.
 *
 * @param newNodeId   Ver.: always
 * @param newNodeEui64   Ver.: always
 * @param parentOfNewNode   Ver.: always
 * @param status   Ver.: always
 * @param decision   Ver.: always
 */
void emberAfTrustCenterJoinCallback(EmberNodeId newNodeId,
                                    EmberEUI64 newNodeEui64,
                                    EmberNodeId parentOfNewNode,
                                    EmberDeviceUpdate status,
                                    EmberJoinDecision decision)
{
  pluginDebugPrint("%p: TrustCenterJoin 0x%2x ", PLUGIN_NAME, newNodeId);
  pluginDebugExec(emberAfPrintBigEndianEui64(newNodeEui64));
  pluginDebugPrintln(" 0x%2x 0x%x 0x%x", parentOfNewNode, status, decision);

  // If a device is leaving or rejoining the trust center we may have knowledge of a
  // tunnel previous established with that device.  If so remove all knowledge
  // of that tunnel because it is no longer valid.
  emAfPluginCommsHubFunctionTunnelCleanup(newNodeEui64);

  if (EMBER_DEVICE_LEFT != status) {
    // If the device did a first time join, then it will not have
    // done key establishment yet and so attempting to initiate a tunnel
    // to the device is premature.

    // Our event handler will periodically kick off and see if the device
    // is done with key establishment and thus will initiate a tunnel to
    // it.

    // If the device did a rejoin then we could try to initiate a tunnel
    // to the device immediately, but our event will periodically kick off to do that.
    // More importantly we can't call checkForSpecificDeviceThatNeedsTunnelCreated()
    // now because our currentDeviceLogEntry variable won't necessarily correspond to the
    // device that is doing a rejoin here.  Better let the event fire later.

    // Cleanest thing to do is assume the tunnel needs to be re-created.
    // Although the local device may have a tunnel up and running fine,
    // the remote device (e.g. ESME) may have forgotten its tunnel.
  }
}

/** @brief Key Establishment
 *
 * A callback by the key-establishment code to indicate an event has occurred.
 * For error codes this is purely a notification.  For non-error status codes
 * (besides LINK_KEY_ESTABLISHED), it is the application's chance to allow or
 * disallow the operation.  If the application returns true then the key
 * establishment is allowed to proceed.  If it returns false, then key
 * establishment is aborted.  LINK_KEY_ESTABLISHED is a notification of success.
 *
 * @param status   Ver.: always
 * @param amInitiator   Ver.: always
 * @param partnerShortId   Ver.: always
 * @param delayInSeconds   Ver.: always
 */
bool emberAfKeyEstablishmentCallback(EmberAfKeyEstablishmentNotifyMessage status,
                                     bool amInitiator,
                                     EmberNodeId partnerShortId,
                                     uint8_t delayInSeconds)
{
  EmberEUI64 partnerEui;
  EmberAfGBCSDeviceLogInfo deviceInfo;

  pluginDebugPrintln("%p: KeyEstablishmentCallback 0x%x, 0x%x, 0x%2x, 0x%x",
                     PLUGIN_NAME,
                     status,
                     amInitiator,
                     partnerShortId,
                     delayInSeconds);

  /*
   * As defined in section 10.2.2.1 of the GBCS version 0.8:
   *
   * "When a Communications Hub has successfully established a shared secret key
   * using CBKE with a Device of type ESME, HCALCS or PPMID, the CHF shall send a
   * RequestTunnel command to the Device to request a tunnel association with the
   * Device.
   */

  if (status == LINK_KEY_ESTABLISHED) {
    if (EMBER_SUCCESS != emberLookupEui64ByNodeId(partnerShortId, partnerEui)) {
      // We're only giving the tunnel creation a best effort at link key establishment
      // stage.  If it fails for any reason we'll just wait until the host app
      // attempts to send data to try the tunnel creation again.
      return true;
    }

    if (false == emberAfPluginGbcsDeviceLogGet(partnerEui, &deviceInfo)) {
      // This is not one the devices in the device log so no need to bring up a
      // tunnel to this device.
      return true;
    }

    // If this is not a sleepy device then bring up the tunnel.
    if (!emberAfPluginGbcsDeviceLogIsSleepyType(deviceInfo.deviceType)) {
      // We'll look for all devices that support the Tunneling cluster. Once a device
      // responds we'll add their information to the endpoint table.
      emberAfFindDevicesByProfileAndCluster(partnerShortId,
                                            SE_PROFILE_ID,
                                            ZCL_TUNNELING_CLUSTER_ID,
                                            EMBER_AF_SERVER_CLUSTER_DISCOVERY,
                                            tunnelDiscoveryCallback);
    }
  }

  // Always allow key establishment to continue.
  return true;
}

/** @brief tunnelDiscoveryCallback
 *
 * This function is called when device endpoint discovery completes.
 * It adds the device and all endpoints that support the Tunneling cluster to the list.
 *
 * @param result Contains the list of discovered endpoints.
 */
static void tunnelDiscoveryCallback(const EmberAfServiceDiscoveryResult *result)
{
  pluginDebugPrintln("%p: Discovery callback, cluster 0x%2X, status:0x%X",
                     PLUGIN_NAME,
                     result->zdoRequestClusterId,
                     result->status);
  if (result->status != EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED) {
    discoveryInProgress = false;
  }

  if (result->zdoRequestClusterId == MATCH_DESCRIPTORS_REQUEST) {
    const EmberAfEndpointList* epList;
    EmberEUI64 eui64;
    uint8_t status;

    if (result->status == EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE) {
      epList = (const EmberAfEndpointList*)result->responseData;
      status = emberLookupEui64ByNodeId(result->matchAddress, eui64);
      if ( (status == EMBER_SUCCESS) && (epList->count >= 1) ) {
        emberAfPluginAddTunnelingEndpoint(result->matchAddress, (uint8_t *)epList->list, epList->count);
        emAfPluginCommsHubFunctionTunnelCreate(eui64, epList->list[0]);
      } else {
        // Failed to store endpoint.  Try with default endpoint.
        emberAfPluginCommsHubFunctionPrintln("Error: Failure to find address or endpoint, status=0x%x, nodeId=0x%2x, epCount=%d",
                                             status,
                                             result->matchAddress,
                                             epList->count);
      }
    }
  } else if (result->zdoRequestClusterId == NETWORK_ADDRESS_REQUEST) {
    if (result->status == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE
        || result->status == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE) {
      pluginDebugPrintln("%p: Broadcast node ID discovery complete.", PLUGIN_NAME);
    }

    if (result->status == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE) {
      EmberEUI64 deviceEui64;
      EmberAfGBCSDeviceLogInfo deviceInfo;
      tunnelTargetNodeId = result->matchAddress;
      pluginDebugPrintln("%p: Recorded node ID for 0x%2X", PLUGIN_NAME, tunnelTargetNodeId);
      if (tunnelTargetNodeId != EMBER_NULL_NODE_ID
          && emberAfPluginGbcsDeviceLogRetrieveByIndex(currentDeviceLogEntry,
                                                       deviceEui64,
                                                       &deviceInfo)) {
        emberEventControlSetActive(emberAfPluginCommsHubFunctionTunnelCheckEventControl);
      }
    }
  }
}

/*
 * @brief Logging timed out message to CHF Event Log.
 */
void emAfPluginCommsHubFunctionLogTimedOutMessageEvent(EmberAfSleepyMessage * sleepyMessage)
{
#if defined(ZCL_USING_EVENTS_CLUSTER_SERVER)
  EmberAfEvent event;

  event.eventData[0] = 0x00;
  event.eventTime = emberAfGetCurrentTime();
  event.eventId = GBCS_EVENT_ID_GSME_CMD_NOT_RETRVD;

  pluginDebugPrintln("CHF: Adding timed out message to CHF Event Log");
  emberAfEventsServerAddEvent(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_REMOTE_COMMSHUB_ENDPOINT,
                              EMBER_ZCL_EVENT_LOG_ID_GENERAL_EVENT_LOG,
                              &event);
#endif
}

/** @brief Message Timed Out
 *
 * This function is called by the sleepy message queue when a message times out.
 *  The plugin will invalidate the entry in the queue after giving the
 * application a chance to perform any actions on the timed-out message.
 *
 * @param sleepyMsgId   Ver.: always
 */
void emberAfPluginSleepyMessageQueueMessageTimedOutCallback(EmberAfSleepyMessageId sleepyMessageId)
{
  EmberAfSleepyMessage sleepyMessage;

  if (emberAfPluginSleepyMessageQueueGetPendingMessage(sleepyMessageId, &sleepyMessage)) {
    emberAfPluginSleepyMessageQueueRemoveMessage(sleepyMessageId);
    emAfPluginCommsHubFunctionLogTimedOutMessageEvent(&sleepyMessage);
    emberAfPluginCommsHubFunctionSendCallback(EMBER_AF_CHF_STATUS_SEND_TIMEOUT,
                                              sleepyMessage.dstEui64,
                                              sleepyMessage.length,
                                              sleepyMessage.payload);
  }
}

/** @brief Device Removed
 *
 * This callback is called by the plugin when a device is removed from the
 * device log.
 *
 * @param deviceId Identifier of the device removed  Ver.: always
 */
void emberAfPluginGbcsDeviceLogDeviceRemovedCallback(EmberEUI64 deviceId)
{
  EmberAfSleepyMessage sleepyMessage;
  EmberAfSleepyMessageId sleepyMessageId = emberAfPluginSleepyMessageQueueGetPendingMessageId(deviceId);

  pluginDebugPrint("CHF: DeviceLogDeviceRemoved ");
  pluginDebugExec(emberAfPrintBigEndianEui64(deviceId));
  pluginDebugPrintln("");

  // Remove all pending messages to the device, reset the functional notification flags
  // attribute, and teardown the tunnel associated with the device.
  while (sleepyMessageId != EMBER_AF_PLUGIN_SLEEPY_MESSAGE_INVALID_ID) {
    emberAfPluginSleepyMessageQueueGetPendingMessage(sleepyMessageId, &sleepyMessage);
    emberAfPluginSleepyMessageQueueRemoveMessage(sleepyMessageId);
    emberAfPluginCommsHubFunctionSendCallback(EMBER_AF_CHF_STATUS_NO_ACCESS,
                                              sleepyMessage.dstEui64,
                                              sleepyMessage.length,
                                              sleepyMessage.payload);
    sleepyMessageId = emberAfPluginSleepyMessageQueueGetPendingMessageId(deviceId);
  }
  clearTunnelMessagePending(deviceId);
  emAfPluginCommsHubFunctionTunnelDestroy(deviceId);
}

/**
 * @brief Tunnel Accept
 *
 * This callback is called by the tunnel manager when a tunnel is requested. The
 * given device identifier should be checked against the Device Log to verify
 * whether tunnels from the device should be accepted or not.
 *
 * @param deviceId Identifier of the device from which a tunnel is requested
 * @return true is the tunnel should be allowed, false otherwise
 */
bool emAfPluginCommsHubFunctionTunnelAcceptCallback(EmberEUI64 deviceId)
{
  EmberAfGBCSDeviceLogInfo deviceInfo;

  pluginDebugPrint("CHF: TunnelAccept ");
  pluginDebugExec(emberAfPrintBigEndianEui64(deviceId));
  pluginDebugPrintln("");

  return emberAfPluginGbcsDeviceLogGet(deviceId, &deviceInfo);
}

/** @brief Tunnel Data Received
 *
 * This callback is called by the tunnel manager when data is received over a tunnel.
 *
 * @param senderDeviceId Identifier of the device from which the data was received
 * @param length The length of the data received
 * @param payload The data received
 */
void emAfPluginCommsHubFunctionTunnelDataReceivedCallback(EmberEUI64 senderDeviceId,
                                                          uint16_t length,
                                                          uint8_t *payload)
{
  EmberAfGBCSDeviceLogInfo deviceInfo;
  uint8_t tunnelHeader[2];
  uint8_t pendingMessages;
  uint16_t dataLen;
  uint8_t *data;
  EmberAfSleepyMessageId sleepyMessageId = EMBER_AF_PLUGIN_SLEEPY_MESSAGE_INVALID_ID;
  EmberAfSleepyMessage sleepyMessage;
  uint16_t messageCode;

  pluginDebugPrint("CHF: TunnelDataReceived ");
  pluginDebugExec(emberAfPrintBigEndianEui64(senderDeviceId));
  pluginDebugPrint(" [");
  pluginDebugPrintBuffer(payload, length, false);
  pluginDebugPrintln("]");

  //  Check to make sure the destination device is in the device log.
  if (!emberAfPluginGbcsDeviceLogGet(senderDeviceId, &deviceInfo)) {
    emAfPluginCommsHubFunctionTunnelDestroy(senderDeviceId);
    emberAfPluginCommsHubFunctionPrintln("Given destination device ID has not been configured in the GBCS device log");
    return;
  }

  if (0 == length) {
    return;
  }

  // If it's a sleepy device then check for the GET and PUT message headers
  if (emberAfPluginGbcsDeviceLogIsSleepyType(deviceInfo.deviceType)) {
    if (*payload == TUNNEL_MANAGER_HEADER_GET) {
      bool result;
      // GET-RESPONSE (the concatenation 0x02 || number of Remote Party Messages remaining):
      // this is used by the CHF to send a Remote Party Message to the GSME. It also indicates
      // how many Remote Party Messages have yet to be retrieved;
      pendingMessages = emberAfPluginSleepyMessageQueueGetNumMessages(senderDeviceId);
      if (pendingMessages > 0) {
        sleepyMessageId = emberAfPluginSleepyMessageQueueGetPendingMessageId(senderDeviceId);
        emberAfPluginSleepyMessageQueueGetPendingMessage(sleepyMessageId, &sleepyMessage);
        emberAfPluginSleepyMessageQueueRemoveMessage(sleepyMessageId);
        pendingMessages--;
        dataLen = sleepyMessage.length;
        data = sleepyMessage.payload;
        messageCode = sleepyMessageUseCaseCodes[sleepyMessageId];
      } else {
        dataLen = 0;
        data = NULL;
      }
      tunnelHeader[0] = TUNNEL_MANAGER_HEADER_GET_RESPONSE;
      tunnelHeader[1] = pendingMessages;
      result = emAfPluginCommsHubFunctionTunnelSendData(senderDeviceId, 2, tunnelHeader, dataLen, data);

      // If we sent or attempted to send a message from the sleepy queue then
      // we need to let the calling application know the status of that message.
      if (sleepyMessageId != EMBER_AF_PLUGIN_SLEEPY_MESSAGE_INVALID_ID) {
        // log status of sent message to sleepy device.
        // EMAPPFWKV2-1315 - "For any Event Log entries relating to Event Codes
        // 0x0054 and 0x0055, the Device shall record the Commands received on the
        // Network Interface by including the Message Code in the Event Log"
        EmberAfEvent event;
        event.eventData[0] = 0x02;
        event.eventData[1] = HIGH_BYTE(messageCode);
        event.eventData[2] = LOW_BYTE(messageCode);
        event.eventId = (result) ? GBCS_EVENT_ID_IMM_HAN_CMD_RXED_ACTED : GBCS_EVENT_ID_IMM_HAN_CMD_RXED_NOT_ACTED;
        event.eventTime = emberAfGetCurrentTime();
        emberAfEventsServerAddEvent(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION_REMOTE_COMMSHUB_ENDPOINT,
                                    EMBER_ZCL_EVENT_LOG_ID_GENERAL_EVENT_LOG,
                                    &event);
        emberAfPluginCommsHubFunctionSendCallback((result) ? EMBER_AF_CHF_STATUS_SUCCESS : EMBER_AF_CHF_STATUS_TUNNEL_FAILURE,
                                                  senderDeviceId,
                                                  dataLen,
                                                  data);
      }

      // If there are no more pending messages then clean the FNF attribute in the mirror.
      if (pendingMessages == 0) {
        clearTunnelMessagePending(senderDeviceId);
      }
    } else if (*payload == TUNNEL_MANAGER_HEADER_PUT) {
      emberAfPluginCommsHubFunctionReceivedCallback(senderDeviceId, length - 1, payload + 1);
    } else {
      // Not sure what this is so let's just pass it up to the application to deal with
      emberAfPluginCommsHubFunctionReceivedCallback(senderDeviceId, length, payload);
    }
  }
  // Data from a non-sleepy device just pass it on to the app.
  else {
    emberAfPluginCommsHubFunctionReceivedCallback(senderDeviceId, length, payload);
  }
}

/** @brief Upfate Functional Notification Flags routines
 */
EmberAfPluginCommsHubFunctionStatus emAfUpdateFunctionalNotificationFlagsByEndpoint(uint8_t endpoint,
                                                                                    uint32_t resetMask,
                                                                                    uint32_t setMask)
{
  EmberAfStatus status;
  uint32_t notificationFlags;

  status = emberAfReadClientAttribute(endpoint,
                                      ZCL_SIMPLE_METERING_CLUSTER_ID,
                                      ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
                                      (uint8_t *)&notificationFlags,
                                      4);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfPluginCommsHubFunctionPrintln("Unable to read the functional notification flags attribute: 0x%x", status);
    return EMBER_AF_CHF_STATUS_FNF_ATTR_FAILURE;
  }

  notificationFlags &= resetMask;
  notificationFlags |= setMask;

  status = emberAfWriteClientAttribute(endpoint,
                                       ZCL_SIMPLE_METERING_CLUSTER_ID,
                                       ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID,
                                       (uint8_t *)&notificationFlags,
                                       ZCL_BITMAP32_ATTRIBUTE_TYPE);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfPluginCommsHubFunctionPrintln("Unable to write the functional notification flags attribute: 0x%x", status);
    return EMBER_AF_CHF_STATUS_FNF_ATTR_FAILURE;
  }

  return EMBER_AF_CHF_STATUS_SUCCESS;
}

EmberAfPluginCommsHubFunctionStatus emAfUpdateFunctionalNotificationFlagsByEui64(EmberEUI64 deviceId,
                                                                                 uint32_t resetMask,
                                                                                 uint32_t setMask)
{
  uint8_t mirrorEndpoint;

  if (!emberAfPluginMeterMirrorGetEndpointByEui64(deviceId, &mirrorEndpoint)) {
    emberAfPluginCommsHubFunctionPrintln("Mirror endpoint for given EUI64 has not been configured");
    return EMBER_AF_CHF_STATUS_NO_MIRROR;
  }

  return emAfUpdateFunctionalNotificationFlagsByEndpoint(mirrorEndpoint,
                                                         resetMask,
                                                         setMask);
}

//------------------------------------------------------------------------------
// Internal Functions

static EmberAfPluginCommsHubFunctionStatus setTunnelMessagePending(EmberEUI64 deviceId)
{
  return emAfUpdateFunctionalNotificationFlagsByEui64(deviceId,
                                                      0xFFFFFFFF,
                                                      EMBER_AF_METERING_FNF_TUNNEL_MESSAGE_PENDING);
}

static EmberAfPluginCommsHubFunctionStatus clearTunnelMessagePending(EmberEUI64 deviceId)
{
  return emAfUpdateFunctionalNotificationFlagsByEui64(deviceId,
                                                      ~EMBER_AF_METERING_FNF_TUNNEL_MESSAGE_PENDING,
                                                      0);
}

void emberAfPluginCommsHubFunctionTunnelCheckEventHandler(void)
{
  emberAfEventControlSetDelay(&emberAfPluginCommsHubFunctionTunnelCheckEventControl,
                              (EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_CHECK_PERIOD_SECONDS
                               * MILLISECOND_TICKS_PER_SECOND));
  if (discoveryInProgress) {
    return;
  }
  checkForAnyDeviceThatNeedsTunnelCreated();
}

static void initiateDiscovery(EmberNodeId nodeId, EmberEUI64 deviceEui64)
{
  EmberStatus status;

  // In case we need to discover multiple devices AND
  // our periodic event kicks off again while waiting for responses,
  // we can't perform another discovery.
  // This is because we only support a single discovery at one
  // time.  We can't perform multiple discoveries.  It is a limitation of
  // the service discovery code.
  discoveryInProgress = true;

  if (nodeId == EMBER_NULL_NODE_ID) {
    pluginDebugPrintln("%p: Initiating node ID discovery.", PLUGIN_NAME);
    status = emberAfFindNodeId(deviceEui64, tunnelDiscoveryCallback);
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln("%p: Failed to initiate node ID discovery for tunnel.",
                         PLUGIN_NAME);
    }
  } else {
    pluginDebugPrintln("%p: Initiating endpoint discovery.", PLUGIN_NAME);
    status = emberAfFindDevicesByProfileAndCluster(nodeId,
                                                   SE_PROFILE_ID,
                                                   ZCL_TUNNELING_CLUSTER_ID,
                                                   EMBER_AF_SERVER_CLUSTER_DISCOVERY,
                                                   tunnelDiscoveryCallback);
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln("%p: Failed to initiate tunnel service discovery to 0x%2X",
                         PLUGIN_NAME,
                         nodeId);
    }
  }
}

static bool deviceTypeRequiresTunnelInitiated(EmberAfGBCSDeviceLogInfo deviceInfo)
{
  // Per the GBCS spec section 10.2.2.1,
  // only these devices require tunnel initiated to them.
  return (deviceInfo.deviceType == EMBER_AF_GBCS_ESME_DEVICE_TYPE
          || deviceInfo.deviceType == EMBER_AF_GBCS_HCALCS_DEVICE_TYPE
          || deviceInfo.deviceType == EMBER_AF_GBCS_PPMID_DEVICE_TYPE);
}

static bool checkForSpecificDeviceThatNeedsTunnelCreated(EmberNodeId nodeId,
                                                         EmberEUI64 deviceEui64)
{
  EmberAfGBCSDeviceLogInfo deviceInfo;
  pluginDebugPrint("%p: Checking whether device needs tunnel created ", PLUGIN_NAME);
  pluginDebugExec(emberAfPrintBigEndianEui64(deviceEui64));
  if (emberAfPluginGbcsDeviceLogGet(deviceEui64, &deviceInfo)
      && deviceTypeRequiresTunnelInitiated(deviceInfo)
      && !emAfPluginCommsHubFunctionTunnelExists(deviceEui64)) {
    EmberKeyStruct keyStruct;
    uint8_t index = emberFindKeyTableEntry(deviceEui64, true);
    if (((index != 0xFF)
         && (EMBER_SUCCESS == emberGetKeyTableEntry(index, &keyStruct))
         && (keyStruct.bitmask & EMBER_KEY_IS_AUTHORIZED))
#ifdef EMBER_TEST
        || (emberAfIsFullSmartEnergySecurityPresent() == EMBER_AF_INVALID_KEY_ESTABLISHMENT_SUITE)
#endif //EMBER_TEST
        ) {
      pluginDebugPrintln(": YES.");
      initiateDiscovery(nodeId, deviceEui64);
      tunnelTargetAttempts++;
      pluginDebugPrintln("Initiate discovery attempt: %d", tunnelTargetAttempts);
      return true;
    }
  }
  pluginDebugPrintln(": No");
  return false;
}

static void checkForAnyDeviceThatNeedsTunnelCreated(void)
{
  EmberEUI64 deviceEui64;
  EmberAfGBCSDeviceLogInfo deviceInfo;

  uint8_t max = emberAfPluginGbcsDeviceLogMaxSize();

  for (; currentDeviceLogEntry < max; currentDeviceLogEntry++) {
    if (emberAfPluginGbcsDeviceLogRetrieveByIndex(currentDeviceLogEntry, deviceEui64, &deviceInfo)) {
      if (tunnelTargetAttempts < MAX_TUNNEL_TARGET_ATTEMPTS
          && checkForSpecificDeviceThatNeedsTunnelCreated(tunnelTargetNodeId,
                                                          deviceEui64)) {
        emberAfCorePrint("%p: Device needs tunnel created", PLUGIN_NAME);
        emberAfPrintBigEndianEui64(deviceEui64);
        emberAfCorePrintln("");
        return;
      }
    }
    tunnelTargetAttempts = 0;
    tunnelTargetNodeId = EMBER_NULL_NODE_ID;
  }
  currentDeviceLogEntry = 0;
}

void emberAfPluginCommsHubFunctionStackStatusCallback(EmberStatus status)
{
  if (status != EMBER_NETWORK_UP) {
    // Disable
    emberAfEventControlSetDelay(&emberAfPluginCommsHubFunctionTunnelCheckEventControl, 0);
    return;
  }

  tunnelTargetNodeId = EMBER_NULL_NODE_ID;
  currentDeviceLogEntry = 0;
  tunnelTargetAttempts = 0;
  discoveryInProgress = false;

  emberAfCorePrintln("%p: Setting up event for monitoring tunnels.", PLUGIN_NAME);
  emberAfEventControlSetDelay(&emberAfPluginCommsHubFunctionTunnelCheckEventControl,
                              (EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_CHECK_PERIOD_SECONDS
                               * MILLISECOND_TICKS_PER_SECOND));
}
