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
 * @brief Routines for the Tunneling Client plugin.
 *******************************************************************************
   ******************************************************************************/

#include "../../include/af.h"
#include "../../util/common.h"
#include "tunneling-client.h"
#include "app/framework/plugin/address-table/address-table.h"

#define UNUSED_ENDPOINT_ID 0xFF

// If addressIndex is EMBER_NULL_ADDRESS_TABLE_INDEX and serverEndpoint is
// UNUSED_ENDPOINT_ID, then the entry is unused and available for use by a new
// tunnel.  Occasionally, serverEndpoint will be UNUSED_ENDPOINT_ID, but
// addressIndex will contain a valid index.  This happens after a tunnel is
// removed but before the address table entry has been cleaned up.  There is a
// delay between closure and cleanup to allow the stack to continue using the
// address table entry to send messages to the server.
typedef struct {
  /* addressIndex refers to the index of addr table within the Addr Table Plugin */
  uint8_t   addressIndex;
  uint8_t   clientEndpoint;
  uint8_t   serverEndpoint;
  uint16_t  tunnelId;
  uint8_t   protocolId;
  uint16_t  manufacturerCode;
  bool flowControlSupport;
} EmAfTunnelingClientTunnel;

static EmAfTunnelingClientTunnel tunnels[EMBER_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT];

static uint8_t pendingIndex = EMBER_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX;

static bool findIeeeAddress(EmberNodeId server);
static void ieeeAddressCallback(const EmberAfServiceDiscoveryResult *result);
static bool createAddressTableEntry(EmAfTunnelingClientTunnel *tunnel,
                                    EmberEUI64 eui64);
bool haveLinkKey(const EmAfTunnelingClientTunnel *tunnel);
static bool partnerLinkKeyExchange(const EmAfTunnelingClientTunnel *tunnel);
static void partnerLinkKeyExchangeCallback(bool success);
static bool sendRequestTunnel(const EmAfTunnelingClientTunnel *tunnel);
static void cleanUp(EmberAfPluginTunnelingClientStatus status);
static EmberAfStatus clientFindTunnel(uint16_t tunnelId,
                                      uint8_t addressIndex,
                                      uint8_t clientEndpoint,
                                      uint8_t serverEndpoint,
                                      EmAfTunnelingClientTunnel **tunnel,
                                      uint8_t *tunnelIndex);

void emberAfTunnelingClusterClientInitCallback(uint8_t endpoint)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT; i++) {
    tunnels[i].addressIndex = EMBER_NULL_ADDRESS_TABLE_INDEX;
    tunnels[i].serverEndpoint = UNUSED_ENDPOINT_ID;
  }
}

void emberAfTunnelingClusterClientTickCallback(uint8_t endpoint)
{
  uint8_t i;

  if (pendingIndex != EMBER_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX
      && tunnels[pendingIndex].clientEndpoint == endpoint) {
    cleanUp(EMBER_AF_PLUGIN_TUNNELING_CLIENT_REQUEST_TUNNEL_TIMEOUT);
    return;
  }

  // Search the table for unused entries that still have an address table
  // index.  These tunnels have been closed, but the address table entry was
  // not immediately removed so the stack could continue using it.  By this
  // point, we've given the stack a fair shot to use it, so now remove the
  // address table entry.
  for (i = 0; i < EMBER_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT; i++) {
    if (tunnels[i].addressIndex != EMBER_NULL_ADDRESS_TABLE_INDEX
        && tunnels[i].clientEndpoint == endpoint
        && tunnels[i].serverEndpoint == UNUSED_ENDPOINT_ID) {
      emberAfPluginAddressTableRemoveEntryByIndex(tunnels[i].addressIndex);
      tunnels[i].addressIndex = EMBER_NULL_ADDRESS_TABLE_INDEX;
    }
  }
}

void emberAfPluginTunnelingClientStackStatusCallback(EmberStatus status)
{
  uint8_t i;

  if (status == EMBER_NETWORK_DOWN
      && !emberStackIsPerformingRejoin()) {
    emberAfSimpleMeteringClusterPrintln("Re-initializing tunnels due to stack down.");
    for (i = 0; i < EMBER_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT; i++) {
      emberAfPluginTunnelingClientCleanup(i);
    }
  }
}

EmberAfPluginTunnelingClientStatus emberAfPluginTunnelingClientRequestTunnel(EmberNodeId server,
                                                                             uint8_t clientEndpoint,
                                                                             uint8_t serverEndpoint,
                                                                             uint8_t protocolId,
                                                                             uint16_t manufacturerCode,
                                                                             bool flowControlSupport)
{
  uint8_t i;

  // Only one outgoing request is allowed at a time.
  if (pendingIndex != EMBER_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX) {
    return EMBER_AF_PLUGIN_TUNNELING_CLIENT_BUSY;
  }

  // TODO: Implement support for flow control.
  if (flowControlSupport) {
    return EMBER_AF_PLUGIN_TUNNELING_CLIENT_FLOW_CONTROL_NOT_SUPPORTED;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT; i++) {
    if (tunnels[i].addressIndex == EMBER_NULL_ADDRESS_TABLE_INDEX
        && tunnels[i].serverEndpoint == UNUSED_ENDPOINT_ID) {
      EmberEUI64 eui64;
      EmAfTunnelingClientTunnel tunnel;
      tunnel.clientEndpoint = clientEndpoint;
      tunnel.serverEndpoint = serverEndpoint;
      // The node id of the server is tucked away in the tunnel id field so we
      // can easily print it during setup.  Once the tunnel is opened, we'll
      // overwrite it.
      tunnel.tunnelId = server;
      tunnel.protocolId = protocolId;
      tunnel.manufacturerCode = manufacturerCode;
      tunnel.flowControlSupport = flowControlSupport;

      // If we know the long address of the server and have a link key to it,
      // we can send the request right now.  Otherwise, we need discover the
      // long address or exchange keys before proceeding.  If an operation
      // fails along the way, we give up.
      if (emberLookupEui64ByNodeId(server, eui64) == EMBER_SUCCESS) {
        if (!createAddressTableEntry(&tunnel, eui64)) {
          return EMBER_AF_PLUGIN_TUNNELING_CLIENT_ADDRESS_TABLE_FULL;
        }
        if (haveLinkKey(&tunnel)) {
          if (!sendRequestTunnel(&tunnel)) {
            emberAfPluginAddressTableRemoveEntryByIndex(tunnel.addressIndex);
            return EMBER_AF_PLUGIN_TUNNELING_CLIENT_REQUEST_TUNNEL_FAILED;
          }
        } else if (!partnerLinkKeyExchange(&tunnel)) {
          emberAfPluginAddressTableRemoveEntryByIndex(tunnel.addressIndex);
          return EMBER_AF_PLUGIN_TUNNELING_CLIENT_LINK_KEY_EXCHANGE_REQUEST_FAILED;
        }
      } else if (!findIeeeAddress(server)) {
        return EMBER_AF_PLUGIN_TUNNELING_CLIENT_IEEE_ADDRESS_REQUEST_FAILED;
      }

      // If we made it here, we either managed to send out the request or we're
      // waiting for a long address lookup or link key exchange to finish.
      // Either way, we can save the tunnel to the table and return.
      pendingIndex = i;
      MEMMOVE(&tunnels[i], &tunnel, sizeof(EmAfTunnelingClientTunnel));
      return EMBER_AF_PLUGIN_TUNNELING_CLIENT_SUCCESS;
    }
  }

  return EMBER_AF_PLUGIN_TUNNELING_CLIENT_NO_MORE_TUNNEL_IDS;
}

static bool findIeeeAddress(EmberNodeId server)
{
  EmberStatus status = emberAfFindIeeeAddress(server, ieeeAddressCallback);
  if (status != EMBER_SUCCESS) {
    emberAfTunnelingClusterPrintln("ERR: Could not request EUI64"
                                   " for node 0x%2x: 0x%x",
                                   server,
                                   status);
  }
  return (status == EMBER_SUCCESS);
}

static void ieeeAddressCallback(const EmberAfServiceDiscoveryResult *result)
{
  if (!emberAfHaveDiscoveryResponseStatus(result->status)) {
    emberAfTunnelingClusterPrintln("ERR: Could not find EUI64"
                                   " for node 0x%2x",
                                   tunnels[pendingIndex].tunnelId);
    cleanUp(EMBER_AF_PLUGIN_TUNNELING_CLIENT_IEEE_ADDRESS_NOT_FOUND);
  } else if (!createAddressTableEntry(&tunnels[pendingIndex],
                                      (uint8_t *)result->responseData)) {
    cleanUp(EMBER_AF_PLUGIN_TUNNELING_CLIENT_ADDRESS_TABLE_FULL);
  } else if (haveLinkKey(&tunnels[pendingIndex])) {
    if (!sendRequestTunnel(&tunnels[pendingIndex])) {
      cleanUp(EMBER_AF_PLUGIN_TUNNELING_CLIENT_REQUEST_TUNNEL_FAILED);
    }
  } else if (!partnerLinkKeyExchange(&tunnels[pendingIndex])) {
    cleanUp(EMBER_AF_PLUGIN_TUNNELING_CLIENT_LINK_KEY_EXCHANGE_REQUEST_FAILED);
  }
}

static bool createAddressTableEntry(EmAfTunnelingClientTunnel *tunnel,
                                    EmberEUI64 eui64)
{
  tunnel->addressIndex = emberAfPluginAddressTableAddEntry(eui64);
  if (tunnel->addressIndex == EMBER_NULL_ADDRESS_TABLE_INDEX) {
    emberAfTunnelingClusterPrintln("ERR: Could not create address"
                                   " table entry for node 0x%2x",
                                   tunnel->tunnelId);
  }
  return (tunnel->addressIndex != EMBER_NULL_ADDRESS_TABLE_INDEX);
}

static void transferDataMessageSentCallback(EmberOutgoingMessageType type,
                                            uint16_t indexOrDestination,
                                            EmberApsFrame *apsFrame,
                                            uint16_t msgLen,
                                            uint8_t *message,
                                            EmberStatus status)
{
  if (status != EMBER_SUCCESS) {
    emberAfPluginTunnelingClientTransferDataFailureCallback(indexOrDestination,
                                                            apsFrame,
                                                            msgLen,
                                                            message,
                                                            status);
  }
}

bool haveLinkKey(const EmAfTunnelingClientTunnel *tunnel)
{
  // We assume we always have a link with the server if one of us is the trust
  // center.  We may already have a link key with other nodes, but we don't
  // know if the other side still has the key, so it's safer to assume we don't
  // have a link key in that case and instead always request a new one.
  return (emberAfGetNodeId() == EMBER_TRUST_CENTER_NODE_ID
          || (emberAfPluginAddressTableLookupNodeIdByIndex(tunnel->addressIndex)
              == EMBER_TRUST_CENTER_NODE_ID));
}

static bool partnerLinkKeyExchange(const EmAfTunnelingClientTunnel *tunnel)
{
  EmberStatus status = emberAfInitiatePartnerLinkKeyExchangeCallback(tunnel->tunnelId,
                                                                     tunnel->serverEndpoint,
                                                                     partnerLinkKeyExchangeCallback);
  if (status != EMBER_SUCCESS) {
    emberAfTunnelingClusterPrintln("ERR: Could not request link key exchange"
                                   " with node 0x%2x: 0x%x",
                                   tunnel->tunnelId,
                                   status);
  }
  return (status == EMBER_SUCCESS);
}

static void partnerLinkKeyExchangeCallback(bool success)
{
  if (!success) {
    emberAfTunnelingClusterPrintln("ERR: Link key exchange"
                                   " with node 0x%2x failed",
                                   tunnels[pendingIndex].tunnelId);
    cleanUp(EMBER_AF_PLUGIN_TUNNELING_CLIENT_LINK_KEY_EXCHANGE_FAILED);
  } else if (!sendRequestTunnel(&tunnels[pendingIndex])) {
    cleanUp(EMBER_AF_PLUGIN_TUNNELING_CLIENT_REQUEST_TUNNEL_FAILED);
  }
}

static bool sendRequestTunnel(const EmAfTunnelingClientTunnel *tunnel)
{
  EmberStatus status;
  emberAfFillCommandTunnelingClusterRequestTunnel(tunnel->protocolId,
                                                  tunnel->manufacturerCode,
                                                  tunnel->flowControlSupport,
                                                  EMBER_AF_PLUGIN_TUNNELING_CLIENT_MAXIMUM_INCOMING_TRANSFER_SIZE);
  emberAfSetCommandEndpoints(tunnel->clientEndpoint, tunnel->serverEndpoint);
  emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_VIA_ADDRESS_TABLE,
                                     tunnel->addressIndex);
  if (status == EMBER_SUCCESS) {
    emberAfScheduleClientTickExtended(tunnel->clientEndpoint,
                                      ZCL_TUNNELING_CLUSTER_ID,
                                      (EMBER_AF_PLUGIN_TUNNELING_CLIENT_TIMEOUT_SECONDS
                                       * MILLISECOND_TICKS_PER_SECOND),
                                      EMBER_AF_SHORT_POLL,
                                      EMBER_AF_OK_TO_SLEEP);
  } else {
    emberAfTunnelingClusterPrintln("ERR: Could not send RequestTunnel: 0x%x",
                                   status);
  }
  return (status == EMBER_SUCCESS);
}

static void cleanUp(EmberAfPluginTunnelingClientStatus status)
{
  if (tunnels[pendingIndex].addressIndex != EMBER_NULL_ADDRESS_TABLE_INDEX) {
    emberAfPluginAddressTableRemoveEntryByIndex(tunnels[pendingIndex].addressIndex);
    tunnels[pendingIndex].addressIndex = EMBER_NULL_ADDRESS_TABLE_INDEX;
  }
  tunnels[pendingIndex].serverEndpoint = UNUSED_ENDPOINT_ID;
  pendingIndex = EMBER_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX;
  emberAfPluginTunnelingClientTunnelOpenedCallback(EMBER_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX,
                                                   status,
                                                   0xFFFF);    // transfer size
}

bool emberAfTunnelingClusterRequestTunnelResponseCallback(uint16_t tunnelId,
                                                          uint8_t tunnelStatus,
                                                          uint16_t maximumIncomingTransferSize)
{
  EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;

  emberAfTunnelingClusterPrintln("RX: RequestTunnelResponse 0x%2x, 0x%x, 0x%2x",
                                 tunnelId,
                                 tunnelStatus,
                                 maximumIncomingTransferSize);

  if (pendingIndex != EMBER_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX
      && (tunnels[pendingIndex].addressIndex
          == emberAfGetAddressIndex())
      && (tunnels[pendingIndex].clientEndpoint
          == emberAfCurrentCommand()->apsFrame->destinationEndpoint)
      && (tunnels[pendingIndex].serverEndpoint
          == emberAfCurrentCommand()->apsFrame->sourceEndpoint)) {
    // If the server accepted the request and created the tunnel, we need to
    // save the assigned tunnel id for future communication.  However, we do
    // not inform the application of the real tunnel id and instead give them
    // an index into our table.  This makes it possible for the application to
    // uniquely identify a tunnel even if multiple servers give us duplicate
    // tunnel ids.
    uint8_t tunnelIndex;
    if (tunnelStatus == EMBER_ZCL_TUNNELING_TUNNEL_STATUS_SUCCESS) {
      tunnels[pendingIndex].tunnelId = tunnelId;
      tunnelIndex = pendingIndex;
    } else {
      tunnels[pendingIndex].serverEndpoint = UNUSED_ENDPOINT_ID;
      tunnelIndex = EMBER_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX;
    }

    // Reschedule the tick to clean up any unused entries that may have been
    // closed while we were waiting for this request to finish.
    emberAfScheduleClientTick(emberAfCurrentCommand()->apsFrame->destinationEndpoint,
                              ZCL_TUNNELING_CLUSTER_ID,
                              MILLISECOND_TICKS_PER_SECOND);

    pendingIndex = EMBER_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX;
    status = EMBER_ZCL_STATUS_SUCCESS;
    emberAfPluginTunnelingClientTunnelOpenedCallback(tunnelIndex,
                                                     (EmberAfPluginTunnelingClientStatus)tunnelStatus,
                                                     maximumIncomingTransferSize);
  }

  emberAfSendImmediateDefaultResponse(status);
  return true;
}

bool emberAfTunnelingClusterTransferDataServerToClientCallback(uint16_t tunnelId,
                                                               uint8_t* data)
{
  EmAfTunnelingClientTunnel *tunnel;
  EmberAfStatus status;
  EmberAfTunnelingTransferDataStatus tunnelError = EMBER_ZCL_TUNNELING_TRANSFER_DATA_STATUS_DATA_OVERFLOW;
  uint16_t dataLen = (emberAfCurrentCommand()->bufLen
                      - (emberAfCurrentCommand()->payloadStartIndex
                         + sizeof(tunnelId)));
  uint8_t tunnelIndex;

  emberAfTunnelingClusterPrint("RX: TransferData 0x%2x, [", tunnelId);
  emberAfTunnelingClusterPrintBuffer(data, dataLen, false);
  emberAfTunnelingClusterPrintln("]");

  status = clientFindTunnel(tunnelId,
                            emberAfGetAddressIndex(),
                            emberAfCurrentCommand()->apsFrame->destinationEndpoint,
                            emberAfCurrentCommand()->apsFrame->sourceEndpoint,
                            &tunnel,
                            &tunnelIndex);
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    if (dataLen <= EMBER_AF_PLUGIN_TUNNELING_CLIENT_MAXIMUM_INCOMING_TRANSFER_SIZE) {
      emberAfPluginTunnelingClientDataReceivedCallback(tunnelIndex,
                                                       data,
                                                       dataLen);
      emberAfSendImmediateDefaultResponse(status);
      return true;
    }
    // else
    //   tunnelError code already set (overflow)
  } else {
    tunnelError = (status == EMBER_ZCL_STATUS_NOT_AUTHORIZED
                   ? EMBER_ZCL_TUNNELING_TRANSFER_DATA_STATUS_WRONG_DEVICE
                   : EMBER_ZCL_TUNNELING_TRANSFER_DATA_STATUS_NO_SUCH_TUNNEL);
  }

  // Error
  emberAfFillCommandTunnelingClusterTransferDataErrorClientToServer(tunnelId,
                                                                    tunnelError);
  emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
  emberAfSendResponse();

  return true;
}

bool emberAfTunnelingClusterTransferDataErrorServerToClientCallback(uint16_t tunnelId,
                                                                    uint8_t transferDataStatus)
{
  EmAfTunnelingClientTunnel *tunnel;
  EmberAfStatus status;
  uint8_t tunnelIndex;

  emberAfTunnelingClusterPrintln("RX: TransferDataError 0x%2x, 0x%x",
                                 tunnelId,
                                 transferDataStatus);

  status = clientFindTunnel(tunnelId,
                            emberAfGetAddressIndex(),
                            emberAfCurrentCommand()->apsFrame->destinationEndpoint,
                            emberAfCurrentCommand()->apsFrame->sourceEndpoint,
                            &tunnel,
                            &tunnelIndex);
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    emberAfPluginTunnelingClientDataErrorCallback(tunnelIndex,
                                                  (EmberAfTunnelingTransferDataStatus)transferDataStatus);
  }

  emberAfSendImmediateDefaultResponse(status);
  return true;
}

EmberAfStatus emberAfPluginTunnelingClientTransferData(uint8_t tunnelIndex,
                                                       uint8_t *data,
                                                       uint16_t dataLen)
{
  if (tunnelIndex < EMBER_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT
      && tunnels[tunnelIndex].serverEndpoint != UNUSED_ENDPOINT_ID) {
    EmberStatus status;
    emberAfFillCommandTunnelingClusterTransferDataClientToServer(tunnels[tunnelIndex].tunnelId,
                                                                 data,
                                                                 dataLen);
    emberAfSetCommandEndpoints(tunnels[tunnelIndex].clientEndpoint,
                               tunnels[tunnelIndex].serverEndpoint);
    emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
    status = emberAfSendCommandUnicastWithCallback(EMBER_OUTGOING_VIA_ADDRESS_TABLE,
                                                   tunnels[tunnelIndex].addressIndex,
                                                   transferDataMessageSentCallback);
    return (status == EMBER_SUCCESS
            ? EMBER_ZCL_STATUS_SUCCESS
            : EMBER_ZCL_STATUS_FAILURE);
  }
  return EMBER_ZCL_STATUS_NOT_FOUND;
}

EmberAfStatus emberAfPluginTunnelingClientCloseTunnel(uint8_t tunnelIndex)
{
  if (tunnelIndex < EMBER_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT
      && tunnels[tunnelIndex].serverEndpoint != UNUSED_ENDPOINT_ID) {
    EmberStatus status;
    emberAfFillCommandTunnelingClusterCloseTunnel(tunnels[tunnelIndex].tunnelId);
    emberAfSetCommandEndpoints(tunnels[tunnelIndex].clientEndpoint,
                               tunnels[tunnelIndex].serverEndpoint);
    emberAfGetCommandApsFrame()->options |= EMBER_APS_OPTION_SOURCE_EUI64;
    status = emberAfSendCommandUnicast(EMBER_OUTGOING_VIA_ADDRESS_TABLE,
                                       tunnels[tunnelIndex].addressIndex);
    if (status == EMBER_SUCCESS) {
      // Mark the entry as unused and, unless we have a pending tunnel request
      // on the same endpoint, schedule a tick to clean up the address table
      // entry.  The delay before cleaning up the address table is to give the
      // stack some time to continue using it for sending the response to the
      // server.  However, if we have a pending request, the tick is already
      // being used as a timeout for the request and we don't want to interfere
      // with that.
      tunnels[tunnelIndex].serverEndpoint = UNUSED_ENDPOINT_ID;
      if (pendingIndex == EMBER_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX
          || (tunnels[pendingIndex].clientEndpoint
              != tunnels[tunnelIndex].clientEndpoint)) {
        emberAfScheduleClientTick(tunnels[tunnelIndex].clientEndpoint,
                                  ZCL_TUNNELING_CLUSTER_ID,
                                  MILLISECOND_TICKS_PER_SECOND);
      }
      return EMBER_ZCL_STATUS_SUCCESS;
    } else {
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }
  return EMBER_ZCL_STATUS_NOT_FOUND;
}

bool emberAfTunnelingClusterTunnelClosureNotificationCallback(uint16_t tunnelId)
{
  EmAfTunnelingClientTunnel *tunnel;
  EmberAfStatus status;
  uint8_t tunnelIndex;

  emberAfTunnelingClusterPrintln("RX: TunnelClosureNotification 0x%2x",
                                 tunnelId);

  status = clientFindTunnel(tunnelId,
                            emberAfGetAddressIndex(),
                            emberAfCurrentCommand()->apsFrame->destinationEndpoint,
                            emberAfCurrentCommand()->apsFrame->sourceEndpoint,
                            &tunnel,
                            &tunnelIndex);
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    // Mark the entry as unused and, unless we have a pending tunnel request on
    // the same endpoint, schedule a tick to clean up the address table entry.
    // The delay before cleaning up the address table is to give the stack some
    // time to continue using it for sending the response to the server.
    // However, if we have a pending request, the tick is already being used as
    // a timeout for the request and we don't want to interfere with that.
    tunnel->serverEndpoint = UNUSED_ENDPOINT_ID;
    if (pendingIndex == EMBER_AF_PLUGIN_TUNNELING_CLIENT_NULL_INDEX
        || tunnels[pendingIndex].clientEndpoint != tunnel->clientEndpoint) {
      emberAfScheduleClientTick(tunnel->clientEndpoint,
                                ZCL_TUNNELING_CLUSTER_ID,
                                MILLISECOND_TICKS_PER_SECOND);
    }
    emberAfPluginTunnelingClientTunnelClosedCallback(tunnelIndex);
  }

  emberAfSendImmediateDefaultResponse(status);
  return true;
}

void emberAfPluginTunnelingClientCleanup(uint8_t tunnelIndex)
{
  if (tunnels[tunnelIndex].serverEndpoint != UNUSED_ENDPOINT_ID) {
    tunnels[tunnelIndex].serverEndpoint = UNUSED_ENDPOINT_ID;
    emberAfPluginTunnelingClientTunnelClosedCallback(tunnelIndex);
  }
  if (tunnels[tunnelIndex].addressIndex != EMBER_NULL_ADDRESS_TABLE_INDEX) {
    emberAfPluginAddressTableRemoveEntryByIndex(tunnels[tunnelIndex].addressIndex);
    tunnels[tunnelIndex].addressIndex = EMBER_NULL_ADDRESS_TABLE_INDEX;
  }
}

static EmberAfStatus clientFindTunnel(uint16_t tunnelId,
                                      uint8_t addressIndex,
                                      uint8_t clientEndpoint,
                                      uint8_t serverEndpoint,
                                      EmAfTunnelingClientTunnel **tunnel,
                                      uint8_t *tunnelIndex)
{
  uint8_t i;
  bool foundNonMatchingTunnel = false;
  for (i = 0; i < EMBER_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT; i++) {
    if (tunnels[i].clientEndpoint == clientEndpoint && tunnels[i].tunnelId == tunnelId) {
      if (tunnels[i].addressIndex == addressIndex
          && tunnels[i].serverEndpoint == serverEndpoint) {
        *tunnel = &tunnels[i];
        *tunnelIndex = i;
        return EMBER_ZCL_STATUS_SUCCESS;
      } else if (tunnels[i].addressIndex != EMBER_NULL_ADDRESS_TABLE_INDEX
                 && tunnels[i].serverEndpoint != UNUSED_ENDPOINT_ID) {
        foundNonMatchingTunnel = true;
      }
    }
  }
  // In order to pass the SE test if only a nonmatching tunnel is found
  // we must return error of wrong device. EMAPPFWKV2-1300
  if (foundNonMatchingTunnel) {
    return EMBER_ZCL_STATUS_NOT_AUTHORIZED;
  }
  return EMBER_ZCL_STATUS_NOT_FOUND;
}

void emAfPluginTunnelingClientCliPrint(void)
{
  uint8_t i;
  uint8_t count = 0;
  emberAfTunnelingClusterPrintln("");
  emberAfTunnelingClusterPrintln("#   server              cep  sep  tid    pid  mfg");
  emberAfTunnelingClusterFlush();
  for (i = 0; i < EMBER_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT; i++) {
    emberAfTunnelingClusterPrint("%x: ", i);
    if (tunnels[i].serverEndpoint != UNUSED_ENDPOINT_ID) {
      EmberEUI64 eui64 = { 0 };
      EmberStatus validEntry;

      validEntry = emberAfPluginAddressTableLookupByIndex(tunnels[i].addressIndex,
                                                          eui64);

      if (validEntry == EMBER_INVALID_CALL) {
        continue;
      }
      count++;
      emberAfTunnelingClusterDebugExec(emberAfPrintBigEndianEui64(eui64));
      emberAfTunnelingClusterPrint(" 0x%x 0x%x 0x%2x",
                                   tunnels[i].clientEndpoint,
                                   tunnels[i].serverEndpoint,
                                   tunnels[i].tunnelId);
      emberAfTunnelingClusterFlush();
      emberAfTunnelingClusterPrint(" 0x%x 0x%2x",
                                   tunnels[i].protocolId,
                                   tunnels[i].manufacturerCode);
      emberAfTunnelingClusterFlush();
    }
    emberAfTunnelingClusterPrintln("");
  }
  emberAfTunnelingClusterPrintln("%d of %d tunnels in use.",
                                 count,
                                 EMBER_AF_PLUGIN_TUNNELING_CLIENT_TUNNEL_LIMIT);
}
