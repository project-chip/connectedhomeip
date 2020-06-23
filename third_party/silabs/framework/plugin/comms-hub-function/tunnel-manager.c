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
 * @brief  The Tunnel Manager is responsible for establishing and maintaining tunnels
 * to all devices.  There are four APIs exposed by the tunnel manager. The
 * init function is called at startup and initializes internal data structures.
 * The create function is called after the CBKE with the device, the sendData
 * function is called whenever data is to be sent to the device, and the
 * destroy function is called whenever the tunnel to the device should
 * be torn down. There are also 1 callback that the tunnel manager will call.
 * It is emAfPluginCommsHubFunctionTunnelDataReceivedCallback which is
 * called when data is received from a tunnel.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/plugin/tunneling-client/tunneling-client.h"
#include "app/framework/plugin/tunneling-server/tunneling-server.h"
#include "comms-hub-function.h"
#include "tunnel-manager.h"

typedef enum {
  CLIENT_TUNNEL,
  SERVER_TUNNEL,
} EmAfCommsHubFunctionTunnelType;

typedef enum {
  UNUSED_TUNNEL,
  REQUEST_PENDING_TUNNEL,
  RESPONSE_PENDING_TUNNEL,
  ACTIVE_TUNNEL,
  CLOSED_TUNNEL
} EmAfCommsHubFunctionTunnelState;

typedef struct {
  EmberEUI64 remoteDeviceId;
  uint8_t remoteEndpoint;
  EmberNodeId remoteNodeId;
  EmAfCommsHubFunctionTunnelType type;
  EmAfCommsHubFunctionTunnelState state;
  uint8_t tunnelId;
  uint32_t timeoutMSec;
} EmAfCommsHubFunctionTunnel;

static EmAfCommsHubFunctionTunnel tunnels[EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT];

/*
 * All tunnels to all devices others than GSME devices will be initiated from
 * the "Remote Comms Endpoint". GSME devices will request tunnels to this
 * endpoint.
 */
#define UNDEFINED_ENDPOINT 0xFF
static uint8_t localTunnelEndpoint = UNDEFINED_ENDPOINT;

/*
 * The Tunneling-client plugin only allows one tunnel to be created at a time.
 * If you attempt to create a second tunnel before the first finishes it returns
 * EMBER_AF_PLUGIN_TUNNELING_CLIENT_BUSY so we only need to keep track of one
 * pending tunnel. Any other tunnel creation attempts which get the BUSY error
 * and will be attempted again when the emberAfPluginCommsHubFunctionTunnelEventControl
 * event fires (see emberAfPluginCommsHubFunctionTunnelEventHandler).
 */
static uint8_t responsePendingIndex = EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;

/*
 * Temporary storage for messages to sleepy devices where the data includes a
 * header
 */
static uint8_t message[1500];

/*
 * Per section 10.2.2 of the GBCS version 0.8
 *
 * "Devices shall set the value of the ManufacturerCode field in any
 * RequestTunnel command to 0xFFFF ('not used').
 *
 * The ProtocolID of all Remote Party Messages shall be 6 ('GB-HGRP'). Devices
 * shall set the value of the ProtocolID field in any RequestTunnel command to 6.
 *
 * Devices shall set the value of the FlowControlSupport field in any
 * RequestTunnel command to 'False'.
 */
#define GBCS_TUNNELING_MANUFACTURER_CODE      0xFFFF
#define GBCS_TUNNELING_PROTOCOL_ID            0x06
#define GBCS_TUNNELING_FLOW_CONTROL_SUPPORT   false

EmberEventControl emberAfPluginCommsHubFunctionTunnelEventControl;

//------------------------------------------------------------------------------
// Forward Declarations
static uint8_t findUnusedTunnel(void);
static bool requestTunnel(uint8_t tunnelIndex);
static bool handleRequestTunnelFailure(uint8_t tunnelIndex,
                                       EmberAfPluginTunnelingClientStatus status);
static uint8_t findTunnelByDeviceId(EmberEUI64 remoteDeviceId);
static uint8_t findTunnelByTunnelId(uint8_t tunnelId, EmAfCommsHubFunctionTunnelType type);

//------------------------------------------------------------------------------
// API Functions

// This should be called from the Comms Hub Function Plugin Init callback.
void emAfPluginCommsHubFunctionTunnelInit(uint8_t localEndpoint)
{
  uint8_t i;

  emberAfDebugPrintln("CHF: TunnelInit");

  localTunnelEndpoint = localEndpoint;
  for (i = 0; i < EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT; i++) {
    tunnels[i].state = UNUSED_TUNNEL;
  }

  // Per GBCS v0.8 section 10.2.2, Devices supporting the Tunneling Cluster
  // as a Server shall have a MaximumIncomingTransferSize set to 1500 octets,
  // in line with the ZSE default.  All Devices supporting the Tunneling
  // Cluster shall use this value in any RequestTunnelResponse command and
  // any RequestTunnel command.
  //
  // If the tunneling client's configured maximumIncomingTransferSize
  // is less than 1500 we'll log a warning.
  //
  // See equivalent check in emberAfPluginTunnelingClientTunnelOpenedCallback()
  if (EMBER_AF_PLUGIN_TUNNELING_CLIENT_MAXIMUM_INCOMING_TRANSFER_SIZE < 1500) {
    emberAfPluginCommsHubFunctionPrintln("WARN: Tunneling Client MaximumIncomingTransferSize is %d but should be 1500",
                                         EMBER_AF_PLUGIN_TUNNELING_CLIENT_MAXIMUM_INCOMING_TRANSFER_SIZE);
  }
}

bool emAfPluginCommsHubFunctionTunnelExists(EmberEUI64 deviceEui64)
{
  return (EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX != findTunnelByDeviceId(deviceEui64));
}

// This should be called after CBKE.
bool emAfPluginCommsHubFunctionTunnelCreate(EmberEUI64 remoteDeviceId,
                                            uint8_t remoteEndpoint)
{
  uint8_t tunnelIndex;
  emberAfDebugPrint("CHF: TunnelCreate ");
  emberAfDebugDebugExec(emberAfPrintBigEndianEui64(remoteDeviceId));
  emberAfDebugPrintln(" 0x%x", remoteEndpoint);

  // We only support one tunnel to a given remote device/endpoint so if we
  // already have a tunnel lets work with it.
  tunnelIndex = findTunnelByDeviceId(remoteDeviceId);
  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    if (tunnels[tunnelIndex].state == CLOSED_TUNNEL) {
      return requestTunnel(tunnelIndex);
    }
    return true;
  }

  // Find a slot in the tunnels table for the new tunnel
  tunnelIndex = findUnusedTunnel();
  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    MEMCOPY(tunnels[tunnelIndex].remoteDeviceId, remoteDeviceId, EUI64_SIZE);
    tunnels[tunnelIndex].remoteNodeId = emberLookupNodeIdByEui64(remoteDeviceId);
    tunnels[tunnelIndex].remoteEndpoint = remoteEndpoint;
    tunnels[tunnelIndex].type = CLIENT_TUNNEL;
    tunnels[tunnelIndex].state = CLOSED_TUNNEL;
    tunnels[tunnelIndex].tunnelId = 0xFF;
    tunnels[tunnelIndex].timeoutMSec = 0;
    return requestTunnel(tunnelIndex);
  }

  // This is a misconfiguration or a bug in the code calling this API. Either
  // the tunnel client plugin limit is set too low for the number of tunnels
  // required or the code that is calling this function is in error.  Either way,
  // we'll print the error and return false indicating that the tunnel was
  // not created.
  emberAfPluginCommsHubFunctionPrintln("%p%p%p",
                                       "Error: ",
                                       "Tunnel Create failed: ",
                                       "Too many tunnels");
  return false;
}

bool emAfPluginCommsHubFunctionTunnelSendData(EmberEUI64 remoteDeviceId,
                                              uint16_t headerLen,
                                              uint8_t *header,
                                              uint16_t dataLen,
                                              uint8_t *data)
{
  EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
  bool success;
  uint8_t tunnelIndex;

  emberAfDebugPrint("CHF: TunnelSendData ");
  emberAfDebugDebugExec(emberAfPrintBigEndianEui64(remoteDeviceId));
  emberAfDebugPrint(" [");
  emberAfDebugPrintBuffer(header, headerLen, false);
  emberAfDebugPrintBuffer(data, dataLen, false);
  emberAfDebugPrintln("]");

  tunnelIndex = findTunnelByDeviceId(remoteDeviceId);
  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    if (tunnels[tunnelIndex].state == ACTIVE_TUNNEL) {
      // Add the header to the output buffers to that we don't copy data twice
      if (headerLen > 0) {
        MEMCOPY(message, header, headerLen);
        MEMCOPY(message + headerLen, data, dataLen);
        data = message;
        dataLen += headerLen;
      }
      status = (tunnels[tunnelIndex].type == CLIENT_TUNNEL)
               ? emberAfPluginTunnelingClientTransferData(tunnels[tunnelIndex].tunnelId, data, dataLen)
               : emberAfPluginTunnelingServerTransferData(tunnels[tunnelIndex].tunnelId, data, dataLen);
    } else if (tunnels[tunnelIndex].state == CLOSED_TUNNEL) {
      // we'll return failure to this message but we'll start the process
      // of bring up the tunnel so that if the message is resent the tunnel
      // should be up.
      emberAfPluginCommsHubFunctionPrintln("Tunnel Closed: New tunnel is requested for the device.");
      requestTunnel(tunnelIndex);
    }
  }

  success = (status == EMBER_ZCL_STATUS_SUCCESS);
  if (!success) {
    emberAfPluginCommsHubFunctionPrintln("%p%p%p0x%x",
                                         "Error: ",
                                         "Tunnel SendData failed: ",
                                         "Tunneling Status: ",
                                         status);
  }
  return success;
}

bool emAfPluginCommsHubFunctionTunnelDestroy(EmberEUI64 remoteDeviceId)
{
  EmberAfStatus status = EMBER_ZCL_STATUS_NOT_FOUND;
  uint8_t tunnelIndex;

  emberAfDebugPrint("CHF: TunnelDestroy ");
  emberAfDebugDebugExec(emberAfPrintBigEndianEui64(remoteDeviceId));
  emberAfDebugPrintln("");

  tunnelIndex = findTunnelByDeviceId(remoteDeviceId);
  if (tunnelIndex < EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT) {
    status = (tunnels[tunnelIndex].type == CLIENT_TUNNEL)
             ? emberAfPluginTunnelingClientCloseTunnel(tunnels[tunnelIndex].tunnelId)
             : EMBER_ZCL_STATUS_SUCCESS;
    if (status == EMBER_ZCL_STATUS_SUCCESS) {
      tunnels[tunnelIndex].state = UNUSED_TUNNEL;
      if (tunnelIndex == responsePendingIndex) {
        responsePendingIndex = EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;
        // let the tunnel event handler retry any pending create requests
        emberEventControlSetActive(emberAfPluginCommsHubFunctionTunnelEventControl);
      }
    }
  }
  return (status == EMBER_ZCL_STATUS_SUCCESS);
}

void emAfPluginCommsHubFunctionTunnelCleanup(EmberEUI64 remoteDeviceId)
{
  uint8_t tunnelIndex;

  tunnelIndex = findTunnelByDeviceId(remoteDeviceId);
  if (tunnelIndex < EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT) {
    emberAfDebugPrint("CHF: TunnelCleanup ");
    emberAfDebugDebugExec(emberAfPrintBigEndianEui64(remoteDeviceId));
    emberAfDebugPrintln("");

    if (tunnels[tunnelIndex].type == CLIENT_TUNNEL) {
      emberAfPluginTunnelingClientCleanup(tunnels[tunnelIndex].tunnelId);
    } else {
      emberAfPluginTunnelingServerCleanup(tunnels[tunnelIndex].tunnelId);
    }
    tunnels[tunnelIndex].state = UNUSED_TUNNEL;
  }
}

void emAfPluginCommsHubFunctionTunnelClose(EmberEUI64 remoteDeviceId)
{
  uint8_t tunnelId;
  tunnelId = findTunnelByDeviceId(remoteDeviceId);
  emberAfDebugPrintln("CHF: TunnelClosed:0x%x", tunnelId);

  if (tunnelId != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    tunnels[tunnelId].state = CLOSED_TUNNEL;
  }
}

void emAfPluginCommsHubFunctionPrint(void)
{
  uint8_t i;
  emberAfPluginCommsHubFunctionPrintln("");
  emberAfPluginCommsHubFunctionPrintln("#   remNodeId remEp  remDevId             type     tId   timeout      state");
  for (i = 0; i < EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT; i++) {
    emberAfPluginCommsHubFunctionPrint("%x: ", i);
    if (tunnels[i].state != UNUSED_TUNNEL) {
      emberAfPluginCommsHubFunctionPrint("0x%2x    0x%x   ",
                                         tunnels[i].remoteNodeId,
                                         tunnels[i].remoteEndpoint);
      emberAfPluginCommsHubFunctionDebugExec(emberAfPrintBigEndianEui64(tunnels[i].remoteDeviceId));
      emberAfPluginCommsHubFunctionPrint("  %p  ", (tunnels[i].type == CLIENT_TUNNEL ? "client" : "server"));
      emberAfPluginCommsHubFunctionPrint(" 0x%x  0x%4x   ",
                                         tunnels[i].tunnelId,
                                         tunnels[i].timeoutMSec);
      switch (tunnels[i].state) {
        case REQUEST_PENDING_TUNNEL:
          emberAfPluginCommsHubFunctionPrint("request-pending");
          break;
        case RESPONSE_PENDING_TUNNEL:
          emberAfPluginCommsHubFunctionPrint("response-pending");
          break;
        case ACTIVE_TUNNEL:
          emberAfPluginCommsHubFunctionPrint("active");
          break;
        case CLOSED_TUNNEL:
          emberAfPluginCommsHubFunctionPrint("closed");
          break;
        default:
          emberAfPluginCommsHubFunctionPrint("unknown");
      }
    }
    emberAfPluginCommsHubFunctionPrintln("");
  }
}

//------------------------------------------------------------------------------
// Callbacks

// Tunnel event handler used to retry previously attempted tunnel creations
void emberAfPluginCommsHubFunctionTunnelEventHandler(void)
{
  uint8_t tunnelIndex;
  uint32_t timeNowMs;
  uint32_t nearestEventTimeoutDelayMs = UINT32_MAX;
  uint32_t currentTunnelTimeoutMs = 0;

  emberEventControlSetInactive(emberAfPluginCommsHubFunctionTunnelEventControl);
  timeNowMs = halCommonGetInt32uMillisecondTick();

  emberAfPluginCommsHubFunctionPrintln("CHF: emberAfPluginCommsHubFunctionTunnelEventHandler");

  // If we're no longer waiting for a tunnel to come up then find which tunnel
  // (or tunnels) are ready for another attempt at tunnel creation
  if (responsePendingIndex == EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    for (tunnelIndex = 0; tunnelIndex < EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT; tunnelIndex++) {
      if (tunnels[tunnelIndex].type == CLIENT_TUNNEL
          && tunnels[tunnelIndex].state == REQUEST_PENDING_TUNNEL) {
        // JIRA EMAPPFWKV2-1392: Event handler was not registered and was not working properly
        // if timeout time has passed, then request a tunnel, else retry after least time remaining
        if (timeGTorEqualInt32u(timeNowMs, tunnels[tunnelIndex].timeoutMSec)) {
          emberAfPluginCommsHubFunctionPrintln("Retrying tunnel creation to node ID 0x%2x",
                                               tunnels[tunnelIndex].remoteNodeId);
          if (requestTunnel(tunnelIndex)) {
            emberEventControlSetDelayMS(emberAfPluginCommsHubFunctionTunnelEventControl,
                                        MILLISECOND_TICKS_PER_SECOND);
            return;
          }
        } else {
          currentTunnelTimeoutMs = elapsedTimeInt32u(timeNowMs, tunnels[tunnelIndex].timeoutMSec);
          if (currentTunnelTimeoutMs < nearestEventTimeoutDelayMs) {
            nearestEventTimeoutDelayMs = currentTunnelTimeoutMs;
          }
        }
      }
    }
  }

  if (nearestEventTimeoutDelayMs != MAX_INT32U_VALUE) {
    emberEventControlSetDelayMS(emberAfPluginCommsHubFunctionTunnelEventControl, nearestEventTimeoutDelayMs);
  }
}

/** @brief Tunnel Opened
 *
 * This function is called by the Tunneling client plugin whenever a tunnel is
 * opened.
 *
 * @param tunnelId The ID of the tunnel that has been opened.  Ver.:
 * always
 * @param tunnelStatus The status of the request.  Ver.: always
 * @param maximumIncomingTransferSize The maximum incoming transfer size of
 * the server.  Ver.: always
 */
void emberAfPluginTunnelingClientTunnelOpenedCallback(uint8_t tunnelId,
                                                      EmberAfPluginTunnelingClientStatus tunnelStatus,
                                                      uint16_t maximumIncomingTransferSize)
{
  emberAfDebugPrintln("CHF: ClientTunnelOpened:0x%x,0x%x,0x%2x", tunnelId, tunnelStatus, maximumIncomingTransferSize);

  if (responsePendingIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    uint8_t tunnelIndex = responsePendingIndex;
    responsePendingIndex = EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;
    emberEventControlSetActive(emberAfPluginCommsHubFunctionTunnelEventControl);
    if (tunnelStatus == EMBER_AF_PLUGIN_TUNNELING_CLIENT_SUCCESS) {
      tunnels[tunnelIndex].tunnelId = tunnelId;
      tunnels[tunnelIndex].state = ACTIVE_TUNNEL;

      // Per GBCS v0.8 section 10.2.2, Devices supporting the Tunneling Cluster
      // as a Server shall have a MaximumIncomingTransferSize set to 1500 octets,
      // in line with the ZSE default.  All Devices supporting the Tunneling
      // Cluster shall use this value in any RequestTunnelResponse command and
      // any RequestTunnel command.
      //
      // So rather than bring down the tunnel in the case when the maximumIncomingTransferSize
      // is less than 1500 we'll just log a warning message.
      if (maximumIncomingTransferSize < 1500) {
        emberAfPluginCommsHubFunctionPrintln("Warning: tunnel opened but MaximumIncomingTransferSize of server is %d but should be 1500",
                                             maximumIncomingTransferSize);
      }

      // providing as a stub for the user to re-send data if the tunnel was re-established while sending data
      // and other cases
      emberAfPluginCommsHubFunctionTunnelOpenedCallback(tunnels[tunnelIndex].remoteDeviceId);
      return;
    }

    // see if we can recover from the open failure
    if (handleRequestTunnelFailure(tunnelIndex, tunnelStatus)) {
      //Jira EMAPPFWKV2-1421: tunnel event handler will take care of retries and request again to setup tunnel, don't need to save
      //tunnelIndex in responsePendingIndex here, as response to tunnel request is already received.
      responsePendingIndex = EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;
      return;
    }
  } else {
    // The tunnel has opened but we no longer require it so close it
    emberAfPluginTunnelingClientCloseTunnel(tunnelId);
  }
}

/** @brief Data Received
 *
 * This function is called by the Tunneling client plugin whenever data is
 * received from a server through a tunnel.
 *
 * @param tunnelId The id of the tunnel through which the data was
 * received.  Ver.: always
 * @param data Buffer containing the raw octets of the data.  Ver.: always
 * @param dataLen The length in octets of the data.  Ver.: always
 */
void emberAfPluginTunnelingClientDataReceivedCallback(uint8_t tunnelId,
                                                      uint8_t *data,
                                                      uint16_t dataLen)
{
  uint8_t tunnelIndex;

  emberAfDebugPrint("CHF: ClientDataReceived:%x,[", tunnelId);
  emberAfDebugPrintBuffer(data, dataLen, false);
  emberAfDebugPrintln("]");

  tunnelIndex = findTunnelByTunnelId(tunnelId, CLIENT_TUNNEL);
  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    emAfPluginCommsHubFunctionTunnelDataReceivedCallback(tunnels[tunnelIndex].remoteDeviceId,
                                                         dataLen,
                                                         data);
  }
}

/** @brief Tunnel Closed
 *
 * This function is called by the Tunneling client plugin whenever a server
 * sends a notification that it preemptively closed an inactive tunnel.
 * Servers are not required to notify clients of tunnel closures, so
 * applications cannot rely on this callback being called for all tunnels.
 *
 * @param tunnelId The ID of the tunnel that has been closed.  Ver.:
 * always
 */
void emberAfPluginTunnelingClientTunnelClosedCallback(uint8_t tunnelId)
{
  uint8_t tunnelIndex;
  emberAfDebugPrintln("CHF: ClientTunnelClosed:0x%x", tunnelId);

  tunnelIndex = findTunnelByTunnelId(tunnelId, CLIENT_TUNNEL);
  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    tunnels[tunnelIndex].state = CLOSED_TUNNEL;
    if (tunnelIndex == responsePendingIndex) {
      responsePendingIndex = EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;
      emberEventControlSetActive(emberAfPluginCommsHubFunctionTunnelEventControl);
    }
  }
}

/** @brief Is Protocol Supported
 *
 * This function is called by the Tunneling server plugin whenever a Request
 * Tunnel command is received.  The application should return true if the
 * protocol is supported and false otherwise.
 *
 * @param protocolId The identifier of the metering communication protocol for
 * which the tunnel is requested.  Ver.: always
 * @param manufacturerCode The manufacturer code for manufacturer-defined
 * protocols or 0xFFFF in unused.  Ver.: always
 */
bool emberAfPluginTunnelingServerIsProtocolSupportedCallback(uint8_t protocolId,
                                                             uint16_t manufacturerCode)
{
  EmberEUI64 remoteDeviceId;

  emberAfDebugPrintln("CHF: ServerIsProtocolSupported:0x%x 0x%2x", protocolId, manufacturerCode);

  // Since the tunneling cluster server code does not pass the EUI64 or the
  // node ID of the remote end of the tunnel so we need to look them up.
  // Luckily this callback is called in the context of the RequestTunnel
  // command processing and we look into the command for this info.
  emberLookupEui64ByNodeId(emberAfCurrentCommand()->source, remoteDeviceId);

  return (GBCS_TUNNELING_PROTOCOL_ID == protocolId
          && GBCS_TUNNELING_MANUFACTURER_CODE == manufacturerCode
          && emAfPluginCommsHubFunctionTunnelAcceptCallback(remoteDeviceId));
}

/** @brief Tunnel Opened
 *
 * This function is called by the Tunneling server plugin whenever a tunnel is
 * opened.  Clients may open tunnels by sending a Request Tunnel command.
 *
 * @param tunnelId The identifier of the tunnel that has been opened.  Ver.:
 * always
 * @param protocolId The identifier of the metering communication protocol for
 * the tunnel.  Ver.: always
 * @param manufacturerCode The manufacturer code for manufacturer-defined
 * protocols or 0xFFFF in unused.  Ver.: always
 * @param flowControlSupport true is flow control support is requested or false
 * if it is not.  Ver.: always
 * @param maximumIncomingTransferSize The maximum incoming transfer size of the
 * client.  Ver.: always
 */
void emberAfPluginTunnelingServerTunnelOpenedCallback(uint16_t tunnelId,
                                                      uint8_t protocolId,
                                                      uint16_t manufacturerCode,
                                                      bool flowControlSupport,
                                                      uint16_t maximumIncomingTransferSize)
{
  EmberEUI64 remoteDeviceId;
  EmberNodeId remoteNodeId;
  uint8_t tunnelIndex;

  emberAfDebugPrintln("CHF: ServerTunnelOpened:0x%x,0x%2x", tunnelId, maximumIncomingTransferSize);

  // Since the tunneling cluster server code does not pass the EUI64 or the
  // node ID of the remote end of the tunnel so we need to look them up.
  // Luckily this callback is called in the context of the RequestTunnel
  // command processing and we look into the command for this info.
  remoteNodeId = emberAfCurrentCommand()->source;
  emberLookupEui64ByNodeId(emberAfCurrentCommand()->source, remoteDeviceId);

  // We only support one tunnel to a given remote device/endpoint so if we
  // already have a tunnel lets work with it.
  tunnelIndex = findTunnelByDeviceId(remoteDeviceId);
  if (tunnelIndex == EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    // Find a slot in the tunnels table for the new tunnel
    tunnelIndex = findUnusedTunnel();
  }

  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    MEMCOPY(tunnels[tunnelIndex].remoteDeviceId, remoteDeviceId, EUI64_SIZE);
    tunnels[tunnelIndex].remoteNodeId = remoteNodeId;
    tunnels[tunnelIndex].remoteEndpoint = emberAfCurrentCommand()->apsFrame->sourceEndpoint;
    tunnels[tunnelIndex].type = SERVER_TUNNEL;
    tunnels[tunnelIndex].state = ACTIVE_TUNNEL;
    tunnels[tunnelIndex].tunnelId = tunnelId;
    tunnels[tunnelIndex].timeoutMSec = 0;

    // Per GBCS v0.8 section 10.2.2, Devices supporting the Tunneling Cluster
    // as a Server shall have a MaximumIncomingTransferSize set to 1500 octets,
    // in line with the ZSE default.  All Devices supporting the Tunneling
    // Cluster shall use this value in any RequestTunnelResponse command and
    // any RequestTunnel command.
    //
    // So rather than bring down the tunnel in the case when the maximumIncomingTransferSize
    // is less than 1500 we'll just log a warning message.
    if (maximumIncomingTransferSize < 1500) {
      emberAfPluginCommsHubFunctionPrintln("Warning: tunnel opened but MaximumIncomingTransferSize of client is %d but should be 1500",
                                           maximumIncomingTransferSize);
    }
    return;
  }

  // This is a misconfiguration or a bug in the code calling this API. Either
  // the tunnel client plugin limit is set too low for the number of tunnels
  // required or the code that is calling this function is in error.  Either way,
  // we'll print the error and return false indicating that the tunnel was
  // not created.
  emberAfPluginCommsHubFunctionPrintln("%p%p%p",
                                       "Error: ",
                                       "Tunnel Opened failed: ",
                                       "Too many tunnels");
}

/** @brief Data Received
 *
 * This function is called by the Tunneling server plugin whenever data is
 * received from a client through a tunnel.
 *
 * @param tunnelId The identifier of the tunnel through which the data was
 * received.  Ver.: always
 * @param data Buffer containing the raw octets of the data.  Ver.: always
 * @param dataLen The length in octets of the data.  Ver.: always
 */
void emberAfPluginTunnelingServerDataReceivedCallback(uint16_t tunnelId,
                                                      uint8_t * data,
                                                      uint16_t dataLen)
{
  uint8_t tunnelIndex;
  emberAfDebugPrint("CHF: ServerDataReceived:%x,[", tunnelId);
  emberAfDebugPrintBuffer(data, dataLen, false);
  emberAfDebugPrintln("]");

  tunnelIndex = findTunnelByTunnelId(tunnelId, SERVER_TUNNEL);
  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    emAfPluginCommsHubFunctionTunnelDataReceivedCallback(tunnels[tunnelIndex].remoteDeviceId,
                                                         dataLen,
                                                         data);
  }
}

/** @brief Tunnel Closed
 *
 * This function is called by the Tunneling server plugin whenever a tunnel is
 * closed.  Clients may close tunnels by sending a Close Tunnel command.  The
 * server can preemptively close inactive tunnels after a timeout.
 *
 * @param tunnelId The identifier of the tunnel that has been closed.  Ver.:
 * always
 * @param clientInitiated true if the client initiated the closing of the tunnel
 * or false if the server closed the tunnel due to inactivity.  Ver.: always
 */
void emberAfPluginTunnelingServerTunnelClosedCallback(uint16_t tunnelId,
                                                      bool clientInitiated)
{
  uint8_t tunnelIndex;
  emberAfDebugPrintln("CHF: ServerTunnelClosed:0x%x", tunnelId);

  tunnelIndex = findTunnelByTunnelId(tunnelId, SERVER_TUNNEL);
  if (tunnelIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    tunnels[tunnelIndex].state = CLOSED_TUNNEL;
  }
}

/** @brief Data Error
 *
 * This function is called by the Tunneling server plugin whenever a data error
 * occurs on a tunnel.  Errors occur if a device attempts to send data on tunnel
 * that is no longer active or if the tunneling does not belong to the device.
 *
 * @param tunnelIndex The identifier of the tunnel on which this data error
 * occurred.  Ver.: always
 * @param transferDataStatus The error that occurred.  Ver.: always
 */
void emberAfPluginTunnelingServerDataErrorCallback(uint16_t tunnelIndex,
                                                   EmberAfTunnelingTransferDataStatus transferDataStatus)
{
  emberAfPluginCommsHubFunctionPrintln("CHF: ServerDataError: 0x%x, 0x%x", tunnelIndex, transferDataStatus);
}

/** @brief Data Error
 *
 * This function is called by the Tunneling client plugin whenever a data error
 * occurs on a tunnel.  Errors occur if a device attempts to send data on tunnel
 * that is no longer active or if the tunneling does not belong to the device.
 *
 * @param tunnelIndex The index of the tunnel on which this data error occurred.
 *  Ver.: always
 * @param transferDataStatus The error that occurred.  Ver.: always
 */
void emberAfPluginTunnelingClientDataErrorCallback(uint8_t tunnelIndex,
                                                   EmberAfTunnelingTransferDataStatus transferDataStatus)
{
  uint8_t tunnelId;
  tunnelId = findTunnelByTunnelId(tunnelIndex, CLIENT_TUNNEL);
  emberAfPluginCommsHubFunctionPrintln("CHF: ClientDataError: 0x%x, 0x%x", tunnelIndex, transferDataStatus);
  if (transferDataStatus == EMBER_ZCL_TUNNELING_TRANSFER_DATA_STATUS_NO_SUCH_TUNNEL) {
    emberAfPluginCommsHubFunctionPrintln("No Tunnel Found: New tunnel is requested for the device.");
    emAfPluginCommsHubFunctionTunnelCleanup(tunnels[tunnelId].remoteDeviceId);
    requestTunnel(tunnelId);
  }
}

//------------------------------------------------------------------------------
// Internal Functions

static uint8_t findUnusedTunnel(void)
{
  uint8_t tunnelIndex;
  for (tunnelIndex = 0; tunnelIndex < EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT; tunnelIndex++) {
    if (tunnels[tunnelIndex].state == UNUSED_TUNNEL) {
      return tunnelIndex;
    }
  }
  return EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;
}

static bool requestTunnel(uint8_t tunnelIndex)
{
  EmberAfPluginTunnelingClientStatus status;

  // The tunneling cluster client code can only process one tunneling reuqest
  // at a time so if there's already on outstanding then mark this one pending
  // and let the tunnel event handler take care of the retry.
  if (responsePendingIndex != EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX) {
    tunnels[tunnelIndex].state = REQUEST_PENDING_TUNNEL;
    tunnels[tunnelIndex].timeoutMSec = halCommonGetInt32uMillisecondTick();
    emberEventControlSetActive(emberAfPluginCommsHubFunctionTunnelEventControl);
    return true;
  }

  status = emberAfPluginTunnelingClientRequestTunnel(tunnels[tunnelIndex].remoteNodeId,
                                                     localTunnelEndpoint,
                                                     tunnels[tunnelIndex].remoteEndpoint,
                                                     GBCS_TUNNELING_PROTOCOL_ID,
                                                     GBCS_TUNNELING_MANUFACTURER_CODE,
                                                     GBCS_TUNNELING_FLOW_CONTROL_SUPPORT);
  if (status != EMBER_AF_PLUGIN_TUNNELING_CLIENT_SUCCESS
      && !handleRequestTunnelFailure(tunnelIndex, status)) {
    return false;
  }

  responsePendingIndex = tunnelIndex;
  tunnels[tunnelIndex].state = RESPONSE_PENDING_TUNNEL;
  return true;
}

// See if we can recover from tunnel creation issues.
static bool handleRequestTunnelFailure(uint8_t tunnelIndex, EmberAfPluginTunnelingClientStatus status)
{
  uint8_t i;

  emberAfDebugPrintln("CHF: handleRequestTunnelFailure 0x%x, 0x%x",
                      tunnelIndex, status);

  if (status == EMBER_AF_PLUGIN_TUNNELING_CLIENT_BUSY) {
    // Per GBCS send another request 3 minutes from now
    tunnels[tunnelIndex].state = REQUEST_PENDING_TUNNEL;
    tunnels[tunnelIndex].timeoutMSec = halCommonGetInt32uMillisecondTick()
                                       + (MILLISECOND_TICKS_PER_SECOND * 180);
    emberEventControlSetActive(emberAfPluginCommsHubFunctionTunnelEventControl);
    emberAfPluginCommsHubFunctionPrintln("CHF: Busy status received from node ID 0x%2x", tunnels[tunnelIndex].remoteNodeId);
    return true;
  } else if (status == EMBER_AF_PLUGIN_TUNNELING_CLIENT_NO_MORE_TUNNEL_IDS) {
    // Per GBCS close any other tunnels we may have with the device
    // and once all responses are received try the RequestTunnel again
    bool retryRequest = false;
    for (i = 0; i < EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT; i++) {
      if (i != tunnelIndex && tunnels[i].remoteNodeId == tunnels[tunnelIndex].remoteNodeId) {
        retryRequest = true;
        emAfPluginCommsHubFunctionTunnelDestroy(tunnels[i].remoteDeviceId);
      }
    }
    if (retryRequest) {
      // We'll retry the request in the tunnel event handler so as to give the
      // tunnel(s) a chance to clean up.
      tunnels[tunnelIndex].state = REQUEST_PENDING_TUNNEL;
      tunnels[tunnelIndex].timeoutMSec = halCommonGetInt32uMillisecondTick()
                                         + (MILLISECOND_TICKS_PER_SECOND * 5);
      emberEventControlSetActive(emberAfPluginCommsHubFunctionTunnelEventControl);
      return true;
    }
    // no tunnels were closed so nothing more we can do
    emberAfPluginCommsHubFunctionPrintln("%p%p%p",
                                         "Error: ",
                                         "Tunnel Create failed: ",
                                         "No more tunnel ids");
    tunnels[tunnelIndex].state = CLOSED_TUNNEL;
    return false;
  }

  // All other errors are either due to mis-configuration or errors that we
  // cannot recover from so print the error and return false.
  emberAfPluginCommsHubFunctionPrintln("%p%p%p0x%x",
                                       "Error: ",
                                       "Tunnel Create failed: ",
                                       "Tunneling Client Status: ",
                                       status);
  tunnels[tunnelIndex].state = CLOSED_TUNNEL;
  return false;
}

// Find an active tunnel to the given device.
static uint8_t findTunnelByDeviceId(EmberEUI64 remoteDeviceId)
{
  uint8_t tunnelIndex;

  // emberAfDebugPrint("CHF: findTunnelByDeviceId ");
  // emberAfDebugDebugExec(emberAfPrintBigEndianEui64(remoteDeviceId));
  // emberAfDebugPrintln("");

  for (tunnelIndex = 0; tunnelIndex < EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT; tunnelIndex++) {
    // emberAfDebugPrint("CHF: findTunnelByDeviceId compare to 0x%x ",
    //                   tunnels[tunnelIndex].state);
    // emberAfDebugDebugExec(emberAfPrintBigEndianEui64(tunnels[tunnelIndex].remoteDeviceId));
    // emberAfDebugPrintln("");
    if (tunnels[tunnelIndex].state != UNUSED_TUNNEL
        && (MEMCOMPARE(tunnels[tunnelIndex].remoteDeviceId, remoteDeviceId, EUI64_SIZE) == 0)) {
      return tunnelIndex;
    }
  }
  return EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;
}

// Find an active tunnel from the given client tunneling plugin identifer,
static uint8_t findTunnelByTunnelId(uint8_t tunnelId, EmAfCommsHubFunctionTunnelType type)
{
  uint8_t tunnelIndex;

  // emberAfDebugPrintln("CHF: findTunnelByTunnelId 0x%x 0x%x",
  //                     tunnelId, type);

  for (tunnelIndex = 0; tunnelIndex < EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_TUNNEL_LIMIT; tunnelIndex++) {
    // emberAfDebugPrintln("CHF: findTunnelByTunnelId compare to 0x%x 0x%x 0x%x",
    //                     tunnels[tunnelIndex].state, tunnels[tunnelIndex].tunnelId, tunnels[tunnelIndex].type);
    if (tunnels[tunnelIndex].state != UNUSED_TUNNEL
        && tunnels[tunnelIndex].tunnelId == tunnelId
        && tunnels[tunnelIndex].type == type) {
      return tunnelIndex;
    }
  }
  return EM_AF_PLUGIN_COMMS_HUB_FUNCTION_NULL_TUNNEL_INDEX;
}
