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
 * @brief This file provides an interface to send and receive zcl messages over an
 * IP interface.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "zcl-ip.h"

//=============================================================================
// Globals

static uint8_t  outgoingZclIpPacket[EMBER_AF_PLUGIN_ZCL_IP_MAX_PAYLOAD_SIZE];
static uint8_t  incomingZclIpPacket[EMBER_AF_PLUGIN_ZCL_IP_MAX_PAYLOAD_SIZE];
static uint16_t outgoingZclIpPacketLength = 0;

static const char pluginName[] = "ZCL-IP";
#define PLUGIN_NAME pluginName

//#define DEBUG_ON
#if defined(DEBUG_ON)
  #define debugPrintBlock(blocks, pointer, printCarriageReturns) \
  emberAfPrint8ByteBlocks(blocks, pointer, printCarriageReturns)
  #define debugPrintln(...) emberAfCorePrintln(__VA_ARGS__)
#else
  #define debugPrintln(...)
  #define debugPrintBlock(blocks, pointer, printCarriageReturns)
#endif

//=============================================================================

void emberAfPluginZclIpInitCallback(void)
{
  outgoingZclIpPacketLength = 0;
}

static void copyInt32uLittleEndianToBuffer(uint32_t value, uint8_t* buffer)
{
  uint8_t index = 0;
  buffer[index++] = LOW_BYTE(value);
  buffer[index++] = HIGH_BYTE(value);
  value = (uint16_t)(value >> 16);

  buffer[index++] = LOW_BYTE(value);
  buffer[index++] = HIGH_BYTE(value);
}

static uint16_t createOutgoingZclIpPacket(uint16_t commandId, uint16_t packetLength)
{
  uint16_t index = 0;
  MEMSET(outgoingZclIpPacket, 0, EMBER_AF_PLUGIN_ZCL_IP_MAX_PAYLOAD_SIZE);

  outgoingZclIpPacket[index++] = LOW_BYTE(EM_AF_ZCL_IP_VERSION);
  outgoingZclIpPacket[index++] = HIGH_BYTE(EM_AF_ZCL_IP_VERSION);

  outgoingZclIpPacket[index++] = LOW_BYTE(packetLength);
  outgoingZclIpPacket[index++] = HIGH_BYTE(packetLength);

  outgoingZclIpPacket[index++] = LOW_BYTE(commandId);
  outgoingZclIpPacket[index++] = HIGH_BYTE(commandId);
  return index;
}

EmberStatus emberAfPluginZclIpGenerateOutgoingKeepalive(uint32_t unixTime,
                                                        uint32_t nextKeepaliveDelaySeconds)
{
  uint16_t index = createOutgoingZclIpPacket(EM_AF_ZCL_IP_KEEPALIVE_PACKET_ID,
                                             EM_AF_ZCL_IP_KEEPALIVE_PACKET_SIZE);

  copyInt32uLittleEndianToBuffer(unixTime, outgoingZclIpPacket + index);
  index += 4;
  copyInt32uLittleEndianToBuffer(nextKeepaliveDelaySeconds, outgoingZclIpPacket + index);
  index += 4;

// Commented out because Keepalives occur very often and generate a lot of printing.
//  debugPrintln("%p Generated keepalive ", PLUGIN_NAME);
//  debugPrintBlock(4, outgoingZclIpPacket, true);

  return emberAfPluginZclIpSendPacketCallback(outgoingZclIpPacket, index);
}

// Node ID may be undefined (EMBER_UNKNOWN_NODE_ID), in which case the EUI64
// is used.
EmberStatus emberAfPluginZclIpGenerateOutgoingProxyPacket(EmberApsFrame* apsFrame,
                                                          EmberNodeId nodeId,
                                                          EmberEUI64 eui64,
                                                          uint8_t* zclMessageBuffer,
                                                          uint16_t zclMessageBufferLength)
{
  uint16_t index = 0;
  uint16_t packetLength = EM_AF_ZCL_IP_PROXY_PACKET_MINIMUM_SIZE + zclMessageBufferLength;

  if (packetLength > EMBER_AF_PLUGIN_ZCL_IP_MAX_PAYLOAD_SIZE) {
    emberAfCorePrintln("Error: %p can't encapsulate.  Packet size %d > %d",
                       PLUGIN_NAME,
                       packetLength,
                       EMBER_AF_PLUGIN_ZCL_IP_MAX_PAYLOAD_SIZE);
    return EMBER_NO_BUFFERS;
  }

  index = createOutgoingZclIpPacket(EM_AF_ZCL_IP_COMMAND_ZCL_PROXY_PACKET_ID, packetLength);

  outgoingZclIpPacket[index++] = LOW_BYTE(apsFrame->clusterId);
  outgoingZclIpPacket[index++] = HIGH_BYTE(apsFrame->clusterId);

  outgoingZclIpPacket[index++] = LOW_BYTE(apsFrame->profileId);
  outgoingZclIpPacket[index++] = HIGH_BYTE(apsFrame->profileId);

  outgoingZclIpPacket[index++] = apsFrame->sourceEndpoint;
  outgoingZclIpPacket[index++] = apsFrame->destinationEndpoint;

  if (nodeId == EMBER_UNKNOWN_NODE_ID || nodeId == EMBER_NULL_NODE_ID) {
    MEMMOVE(&(outgoingZclIpPacket[index]), eui64, EUI64_SIZE);
    if (emberAfMemoryByteCompare(eui64, EUI64_SIZE, 0xFF)) {
      emberAfCorePrintln("Error: %p Node ID and EUI64 are both null, cannot determine address.", PLUGIN_NAME);
      return EMBER_ERR_FATAL;
    }
  } else if (EMBER_SUCCESS != emberLookupEui64ByNodeId(nodeId,
                                                       &outgoingZclIpPacket[index])) {
    emberAfCorePrintln("Error: %p No Source EUI found for 0x%2X, dropping packet", PLUGIN_NAME, nodeId);
    return EMBER_ERR_FATAL;
  }
  index += EUI64_SIZE;

  outgoingZclIpPacket[index++] = LOW_BYTE(apsFrame->options);
  outgoingZclIpPacket[index++] = HIGH_BYTE(apsFrame->options);

  outgoingZclIpPacket[index++] = LOW_BYTE(zclMessageBufferLength);
  outgoingZclIpPacket[index++] = HIGH_BYTE(zclMessageBufferLength);

  MEMMOVE(&outgoingZclIpPacket[index], zclMessageBuffer, zclMessageBufferLength);
  index += zclMessageBufferLength;

  debugPrintln("Encapsulated ZCL-IP: Packet Length %d, Index: %d, ZCL Length %d", packetLength, index, zclMessageBufferLength);
  //debugPrintBlock(4, outgoingZclIpPacket, true);
  emberAfCorePrintln("packetLength %u, index %u", packetLength, index);
  return emberAfPluginZclIpSendPacketCallback(outgoingZclIpPacket, packetLength);
}

EmberStatus emberAfPluginZclIpGenerateOutgoingConfigureRemoteCluster(uint16_t count,
                                                                     const EmberAfRemoteClusterStruct* records)
{
  uint16_t packetLength = (EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_START_INDEX
                           + (EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_LENGTH
                              * count));
  if (packetLength > EMBER_AF_PLUGIN_ZCL_IP_MAX_PAYLOAD_SIZE) {
    emberAfCorePrintln("Error: %p could generate Configure Remote Cluster command, payload too long.");
    return EMBER_ERR_FATAL;
  }

  uint16_t index = createOutgoingZclIpPacket(EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_ID, packetLength);
  outgoingZclIpPacket[index++] = LOW_BYTE(count);
  outgoingZclIpPacket[index++] = HIGH_BYTE(count);

  uint16_t i;
  for (i = 0; i < count; i++) {
    outgoingZclIpPacket[index++] = LOW_BYTE(records[i].clusterId);
    outgoingZclIpPacket[index++] = HIGH_BYTE(records[i].clusterId);

    outgoingZclIpPacket[index++] = LOW_BYTE(records[i].profileId);
    outgoingZclIpPacket[index++] = HIGH_BYTE(records[i].profileId);

    outgoingZclIpPacket[index++] = LOW_BYTE(records[i].deviceId);
    outgoingZclIpPacket[index++] = HIGH_BYTE(records[i].deviceId);

    outgoingZclIpPacket[index++] = records[i].endpoint;

    outgoingZclIpPacket[index++] = LOW_BYTE(records[i].type);
    outgoingZclIpPacket[index++] = HIGH_BYTE(records[i].type);
  }

  debugPrintln("Created ZCL-IP Configure Remote Clusters Packet");
  debugPrintBlock(4, outgoingZclIpPacket, true);
  emberAfCorePrintln("Packet Length %u, index %u", packetLength, index);
  return emberAfPluginZclIpSendPacketCallback(outgoingZclIpPacket, packetLength);
}

EmberStatus emberAfPluginZclIpGenerateOutgoingConfigureRemoteBinding(EmberEUI64 targetEUI,
                                                                     uint8_t sourceEndpoint,
                                                                     uint8_t destEndpoint,
                                                                     uint16_t clusterId,
                                                                     EmberEUI64 destEUI,
                                                                     EmberEUI64 sourceEUI)
{
  uint16_t index = createOutgoingZclIpPacket(EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_ID,
                                             EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_COMMAND_LENGTH);
  // outgoingZclIpPacket[index++] = LOW_BYTE(targetId);
  // outgoingZclIpPacket[index++] = HIGH_BYTE(targetId);

  MEMMOVE(outgoingZclIpPacket + index, targetEUI, EUI64_SIZE);
  index += EUI64_SIZE;

  outgoingZclIpPacket[index++] = sourceEndpoint;

  outgoingZclIpPacket[index++] = destEndpoint;

  outgoingZclIpPacket[index++] = LOW_BYTE(clusterId);
  outgoingZclIpPacket[index++] = HIGH_BYTE(clusterId);

  MEMMOVE(outgoingZclIpPacket + index, destEUI, EUI64_SIZE);
  index += EUI64_SIZE;

  MEMMOVE(outgoingZclIpPacket + index, sourceEUI, EUI64_SIZE);
  index += EUI64_SIZE;

  debugPrintln("Created ZCL-IP Configure Remote Binding packet,index %u", index);
  return emberAfPluginZclIpSendPacketCallback(outgoingZclIpPacket, index);
}

EmberStatus emberAfPluginZclIpGenerateOutgoingAddDevice(EmberEUI64 newDeviceEui64,
                                                        const EmberKeyData* newDeviceKeyData)
{
  uint16_t index = createOutgoingZclIpPacket(EM_AF_ZCL_IP_ADD_DEVICE_ID,
                                             EM_AF_ZCL_IP_ADD_DEVICE_COMMAND_LENGTH);

  MEMMOVE(outgoingZclIpPacket + index, newDeviceEui64, EUI64_SIZE);
  index += EUI64_SIZE;
  MEMMOVE(outgoingZclIpPacket + index, emberKeyContents(newDeviceKeyData), EMBER_ENCRYPTION_KEY_SIZE);
  index += EMBER_ENCRYPTION_KEY_SIZE;

  debugPrintln("Created ZCL-IP Add Device Packet");
  debugPrintBlock(4, outgoingZclIpPacket, true);

  return emberAfPluginZclIpSendPacketCallback(outgoingZclIpPacket, index);
}

static uint16_t calculateDeviceDiscoveryUpdateCommandLength(const EmberAfDeviceInfo* device)
{
  uint16_t length = EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_MIN_LENGTH;
  int i;
  for (i = 0; i < device->endpointCount; i++) {
    length += EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_ENDPOINT_INFO_MIN_LENGTH;
    // Each cluster takes 2 bytes for the cluster ID, and 1-byte for the server bool value.
    length += (device->endpoints[i].clusterCount * (2 + 1));
  }
  return length;
}

EmberStatus emberAfPluginZclIpGenerateOutgoingDeviceDiscoveryUpdate(const EmberAfDeviceInfo* device)
{
  uint16_t packetLength = calculateDeviceDiscoveryUpdateCommandLength(device);
  if (packetLength > EMBER_AF_PLUGIN_ZCL_IP_MAX_PAYLOAD_SIZE) {
    emberAfCorePrintln("Error: Device Discovery Update packet too long (%d > %d)",
                       packetLength,
                       EMBER_AF_PLUGIN_ZCL_IP_MAX_PAYLOAD_SIZE);
    return EMBER_NO_BUFFERS;
  }

  uint16_t index = createOutgoingZclIpPacket(EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_ID,
                                             packetLength);

  MEMMOVE(outgoingZclIpPacket + index, device->eui64, EUI64_SIZE);
  index += EUI64_SIZE;
  outgoingZclIpPacket[index++] = device->status;
  outgoingZclIpPacket[index++] = device->capabilities;
  outgoingZclIpPacket[index++] = device->endpointCount;

  int i;
  for (i = 0; i < device->endpointCount; i++) {
    outgoingZclIpPacket[index++] = device->endpoints[i].endpoint;

    outgoingZclIpPacket[index++] = LOW_BYTE(device->endpoints[i].profileId);
    outgoingZclIpPacket[index++] = HIGH_BYTE(device->endpoints[i].profileId);

    outgoingZclIpPacket[index++] = LOW_BYTE(device->endpoints[i].deviceId);
    outgoingZclIpPacket[index++] = HIGH_BYTE(device->endpoints[i].deviceId);

    outgoingZclIpPacket[index++] = device->endpoints[i].clusterCount;
    debugPrintln("%p cluster count offset is %d", PLUGIN_NAME, index - 1);

    int j;
    for (j = 0; j < device->endpoints[i].clusterCount; j++) {
      outgoingZclIpPacket[index++] = LOW_BYTE(device->endpoints[i].clusters[j].clusterId);
      outgoingZclIpPacket[index++] = HIGH_BYTE(device->endpoints[i].clusters[j].clusterId);
      outgoingZclIpPacket[index++] = LOW_BYTE(device->endpoints[i].clusters[j].server);
    }
  }

  EMBER_TEST_ASSERT(index == packetLength);

  debugPrintln("Created ZCL-IP Device Discovery Update Packet (%d bytes)", index);
  debugPrintBlock(4, outgoingZclIpPacket, true);
  return emberAfPluginZclIpSendPacketCallback(outgoingZclIpPacket, index);
}

//=============================================================================
// Incoming Messages

uint16_t emberAfPluginZclIpRetrieveEncapsulatedPacketLength(uint8_t* zclIpPacket)
{
  return emAfGetLittleEndianInt16uFromBuffer(zclIpPacket + EM_AF_ZCL_IP_PACKET_LENGTH_INDEX);
}

static bool checkIncomingPacketOverhead(void)
{
  uint16_t version = emAfGetLittleEndianInt16uFromBuffer(incomingZclIpPacket + EM_AF_ZCL_IP_HEADER_INDEX);
  if (version != EM_AF_ZCL_IP_VERSION) {
    emberAfCorePrintln("Error: %p invalid Version 0x%X", PLUGIN_NAME, version);
    return false;
  }
  return true;
}

static bool checkCommandIdAndLength(uint16_t expectedCommandId,
                                    uint16_t expectedLength,
                                    bool exactLength,
                                    const char* expectedCommandIdString)
{
  if (!checkIncomingPacketOverhead()) {
    return false;
  }
  uint16_t commandId = emAfGetLittleEndianInt16uFromBuffer(incomingZclIpPacket + EM_AF_ZCL_IP_COMMAND_ID_INDEX);
  if (commandId != expectedCommandId) {
    emberAfCorePrintln("Error: %p expected %p (id: 0x%2X) but got id: 0x%02X",
                       PLUGIN_NAME,
                       expectedCommandIdString,
                       expectedCommandId,
                       commandId);
    return false;
  }
  uint16_t receivedLength = emberAfPluginZclIpGetPacketLength(emberAfPluginZclIpGetReceivedPacketPointer());
  if (exactLength) {
    if (receivedLength != expectedLength) {
      emberAfCorePrintln("Error: %p command %p (id 0x%2X) expected exact length of %d but got %d",
                         PLUGIN_NAME,
                         expectedCommandIdString,
                         commandId,
                         expectedLength,
                         receivedLength);
      return false;
    }
  } else {
    if (receivedLength < expectedLength) {
      emberAfCorePrintln("Error: %p command %p (id 0x%) expected min length of %d but got %d",
                         PLUGIN_NAME,
                         expectedCommandIdString,
                         expectedLength,
                         receivedLength);
      return false;
    }
  }

  return true;
}

EmberStatus emberAfPluginZclIpRetrieveIncomingKeepalive(uint32_t* unixTimeSinceEpoch,
                                                        uint32_t* nextKeepaliveDelaySeconds)
{
  if (!checkCommandIdAndLength(EM_AF_ZCL_IP_KEEPALIVE_PACKET_ID,
                               EM_AF_ZCL_IP_KEEPALIVE_PACKET_SIZE,
                               true, // exact length?
                               "Keepalive packet")) {
    return EMBER_ERR_FATAL;
  }

// Commented out because Keepalives occur very often and generate a lot of printing.
//  debugPrintln("%p retrieve incoming keepalive.", PLUGIN_NAME);
//  debugPrintBlock(4, incomingZclIpPacket, true);

  *unixTimeSinceEpoch = emAfGetLittleEndianInt32uFromBuffer(incomingZclIpPacket + EM_AF_ZCL_IP_UNIX_TIME_INDEX);
  *nextKeepaliveDelaySeconds = emAfGetLittleEndianInt32uFromBuffer(incomingZclIpPacket + EM_AF_ZCL_IP_KEEPALIVE_DELAY_SECONDS_INDEX);
  return EMBER_SUCCESS;
}

EmberStatus emberAfPluginZclIpRetrieveIncomingEncapsulatedZclProxyPacket(EmberAfClusterCommand* returnCmd,
                                                                         EmberEUI64 returnEui64)
{
  if (!checkCommandIdAndLength(EM_AF_ZCL_IP_COMMAND_ZCL_PROXY_PACKET_ID,
                               EM_AF_ZCL_IP_PROXY_PACKET_MINIMUM_SIZE,
                               false,  // exact length?
                               "ZCL Proxy Packet")) {
    return EMBER_ERR_FATAL;
  }

  debugPrintln("%p received ZCL Proxy   ", PLUGIN_NAME);
  debugPrintBlock(4, incomingZclIpPacket, true);

  returnCmd->apsFrame->clusterId = emAfGetLittleEndianInt16uFromBuffer(incomingZclIpPacket + EM_AF_ZCL_IP_CLUSTER_ID_INDEX);
  returnCmd->apsFrame->profileId = emAfGetLittleEndianInt16uFromBuffer(incomingZclIpPacket + EM_AF_ZCL_IP_PROFILE_ID_INDEX);
  returnCmd->apsFrame->sourceEndpoint = incomingZclIpPacket[EM_AF_ZCL_IP_SOURCE_EP_INDEX];
  returnCmd->apsFrame->destinationEndpoint = incomingZclIpPacket[EM_AF_ZCL_IP_DEST_EP_INDEX];
  MEMMOVE(returnEui64, incomingZclIpPacket + EM_AF_ZCL_IP_EUI64_INDEX, EUI64_SIZE);
  returnCmd->apsFrame->options = emAfGetLittleEndianInt16uFromBuffer(incomingZclIpPacket + EM_AF_ZCL_IP_APS_OPTIONS_INDEX);
  returnCmd->bufLen = emAfGetLittleEndianInt16uFromBuffer(incomingZclIpPacket + EM_AF_ZCL_IP_ZCL_LENGTH_INDEX);
  if (returnCmd->bufLen > EMBER_AF_PLUGIN_ZCL_IP_MAX_PAYLOAD_SIZE) {
    emberAfCorePrintln("Error: %p encapsulated packet length %d exceeds max ZCL message length %d",
                       PLUGIN_NAME,
                       returnCmd->bufLen,
                       EMBER_AF_PLUGIN_ZCL_IP_MAX_PAYLOAD_SIZE);
    return EMBER_NO_BUFFERS;
  }
  returnCmd->buffer = incomingZclIpPacket + EM_AF_ZCL_IP_ZCL_PAYLOAD_INDEX;
  return EMBER_SUCCESS;
}

EmberStatus emberAfPluginZclIpRetrieveIncomingConfigureRemoteClusters(EmberAfRemoteClusterStruct* returnData,
                                                                      uint16_t maxCount,
                                                                      uint16_t* returnedCount)
{
  if (!checkCommandIdAndLength(EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_ID,
                               EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_COMMAND_OVERHEAD,
                               false, // exact length?
                               "Configure Remote Clusters")) {
    return EMBER_ERR_FATAL;
  }
  uint16_t count = emAfGetLittleEndianInt16uFromBuffer(incomingZclIpPacket + EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_COUNT_INDEX);

  //debugPrintln("%p received Configure Remote cluster packet, count: %d", count);
  //debugPrintBlock(4, incomingZclIpPacket, true);

  if (count > maxCount) {
    emberAfCorePrintln("Error: %p the remote cluster count of %d exceeded local max of %d",
                       PLUGIN_NAME,
                       count,
                       maxCount);
    return EMBER_ERR_FATAL;
  }
  *returnedCount = count;

  uint16_t packetLength = emberAfPluginZclIpGetPacketLength(incomingZclIpPacket);
  uint16_t expectedLength = (EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_START_INDEX
                             + (EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_LENGTH * count));
  if (packetLength != expectedLength) {
    emberAfCorePrintln("Error: %p expected packet length of %d (%d records * %d length) but got %d",
                       PLUGIN_NAME,
                       expectedLength,
                       count,
                       EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_LENGTH,
                       packetLength);
    return EMBER_ERR_FATAL;
  }

  MEMSET(returnData, 0xFF, sizeof(EmberAfRemoteClusterStruct) * maxCount);
  uint16_t i;
  uint8_t* pointer = incomingZclIpPacket + EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_START_INDEX;
  for (i = 0; i < count; i++, returnData++) {
    returnData->clusterId = emAfGetLittleEndianInt16uFromBuffer(pointer);
    pointer += 2;

    returnData->profileId = emAfGetLittleEndianInt16uFromBuffer(pointer);
    pointer += 2;

    returnData->deviceId = emAfGetLittleEndianInt16uFromBuffer(pointer);
    pointer += 2;

    returnData->endpoint = emAfGetLittleEndianInt16uFromBuffer(pointer);
    pointer++;

    returnData->type = emAfGetLittleEndianInt16uFromBuffer(pointer);
    pointer += 2;
  }
  return EMBER_SUCCESS;
}

EmberStatus emberAfPluginZclIpRetrieveIncomingAddDevice(EmberEUI64 newDeviceEui64,
                                                        EmberKeyData* newDeviceKeyData)
{
  if (!checkCommandIdAndLength(EM_AF_ZCL_IP_ADD_DEVICE_ID,
                               EM_AF_ZCL_IP_ADD_DEVICE_COMMAND_LENGTH,
                               true, // exact length?
                               "Add device")) {
    return EMBER_ERR_FATAL;
  }

  MEMMOVE(newDeviceEui64,
          incomingZclIpPacket + EM_AF_ZCL_IP_ADD_DEVICE_EUI64_INDEX,
          EUI64_SIZE);
  MEMMOVE(emberKeyContents(newDeviceKeyData),
          incomingZclIpPacket + EM_AF_ZCL_IP_ADD_DEVICE_LINK_KEY_INDEX,
          EMBER_ENCRYPTION_KEY_SIZE);
  return EMBER_SUCCESS;
}

EmberStatus emberAfPluginZclIpRetrieveIncomingConfigureRemoteBinding(EmberAfRemoteBindingStruct *returnData)
{
  if (!checkCommandIdAndLength(EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_ID,
                               EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_COMMAND_LENGTH,
                               true, //exact length?
                               "Configure Remote Binding")) {
    return EMBER_ERR_FATAL;
  }
  MEMSET(returnData, 0xFF, sizeof(EmberAfRemoteBindingStruct));

  MEMMOVE(returnData->targetEUI64,
          incomingZclIpPacket + EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_COMMAND_TARGET_EUI_OFFSET,
          EUI64_SIZE);
  // returnData->targetId = emAfGetLittleEndianInt16uFromBuffer(pointer);
  // pointer += 2;
  uint8_t* pointer = incomingZclIpPacket + EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_COMMAND_SOURCE_ENDPOINT_OFFSET;

  returnData->sourceEndpoint = *pointer;
  pointer++;

  returnData->destEndpoint = *pointer;
  pointer++;

  returnData->clusterId = emAfGetLittleEndianInt16uFromBuffer(pointer);
  pointer += 2;

  MEMMOVE(returnData->destEUI64,
          incomingZclIpPacket + EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_COMMAND_DEST_EUI_OFFSET,
          EUI64_SIZE);

  MEMMOVE(returnData->sourceEUI64,
          incomingZclIpPacket + EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_COMMAND_SOURCE_EUI_OFFSET,
          EUI64_SIZE);

  return EMBER_SUCCESS;
}

uint16_t emberAfPluginZclIpGetPacketLength(const uint8_t* zclIpPacket)
{
  return emAfGetLittleEndianInt16uFromBuffer(zclIpPacket + EM_AF_ZCL_IP_PACKET_LENGTH_INDEX);
}

uint16_t emberAfPluginZclIpGetReceivedPacketCommandId(void)
{
  return emAfGetLittleEndianInt16uFromBuffer(incomingZclIpPacket + EM_AF_ZCL_IP_COMMAND_ID_INDEX);
}

uint8_t* emberAfPluginZclIpGetReceivedPacketPointer(void)
{
  return incomingZclIpPacket;
}

uint8_t* emberAfPluginZclIpGetReceivedProxyPacketEui64Pointer(void)
{
  return incomingZclIpPacket + EM_AF_ZCL_IP_EUI64_INDEX;
}

static bool calculateIncomingDeviceDiscoveryPacketLength(uint16_t* packetLength)
{
  *packetLength = EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_MIN_LENGTH;
  uint8_t i;

  debugPrintln("%p %d endpoints in Device Discovery Update message.", PLUGIN_NAME, incomingZclIpPacket[EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_ENDPOINT_COUNT_INDEX]);

  // We add +2 to the allowed packet length to check overflow beyond the max payload size.
  // A 2-byte cluster with 1-byte server fla will extend beyond the limit.
  for (i = 0;
       i < incomingZclIpPacket[EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_ENDPOINT_COUNT_INDEX]
       && *packetLength < EMBER_AF_PLUGIN_ZCL_IP_MAX_PAYLOAD_SIZE + 3;
       i++) {
    uint8_t clusterCount = incomingZclIpPacket[*packetLength + EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_CLUSTER_COUNT_OFFSET];
    debugPrintln("%p cluster count is %d (offset %d)",
                 PLUGIN_NAME,
                 clusterCount,
                 *packetLength + EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_CLUSTER_COUNT_OFFSET);

    // The '2 + 1' is for the overhead of each cluster.  A cluster has a 2-byte ID and a server bool.
    *packetLength += (EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_ENDPOINT_INFO_MIN_LENGTH
                      + (clusterCount * (2 + 1)));
  }
  if (*packetLength > EMBER_AF_PLUGIN_ZCL_IP_MAX_PAYLOAD_SIZE) {
    return false;
  }
  return true;
}

EmberStatus emberAfPluginZclIpRetrieveIncomingDeviceDiscoveryUpdate(EmberAfDeviceInfo* device)
{
  uint16_t packetLength;
  if (!calculateIncomingDeviceDiscoveryPacketLength(&packetLength)) {
    return EMBER_ERR_FATAL;
  }
  if (!checkCommandIdAndLength(EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_ID,
                               packetLength,
                               true,  // exact length?
                               "Device Discovery Update")) {
    return EMBER_ERR_FATAL;
  }
  MEMSET(device, 0, sizeof(EmberAfDeviceInfo));
  MEMMOVE(device->eui64,
          incomingZclIpPacket + EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_EUI64_INDEX,
          EUI64_SIZE);

  device->status = incomingZclIpPacket[EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_STATUS_INDEX];
  device->capabilities = incomingZclIpPacket[EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_CAPABILITIES_INDEX];
  device->endpointCount = incomingZclIpPacket[EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_ENDPOINT_COUNT_INDEX];
  uint16_t index = EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_ENDPOINT_INFO_INDEX;

  uint16_t i;
  for (i = 0;
       i < device->endpointCount
       && i < EMBER_AF_MAX_ENDPOINTS_PER_DEVICE
       && index < EMBER_AF_PLUGIN_ZCL_IP_MAX_PAYLOAD_SIZE;
       i++) {
    device->endpoints[i].endpoint = incomingZclIpPacket[index++];
    device->endpoints[i].profileId = emAfGetLittleEndianInt16uFromBuffer(incomingZclIpPacket + index);
    index += 2;
    device->endpoints[i].deviceId = emAfGetLittleEndianInt16uFromBuffer(incomingZclIpPacket + index);
    index += 2;
    device->endpoints[i].clusterCount = incomingZclIpPacket[index++];
    uint8_t j;
    for (j = 0;
         j < device->endpoints[i].clusterCount
         && j < EMBER_AF_MAX_CLUSTERS_PER_ENDPOINT
         && index < EMBER_AF_PLUGIN_ZCL_IP_MAX_PAYLOAD_SIZE;
         j++) {
      device->endpoints[i].clusters[j].clusterId = emAfGetLittleEndianInt16uFromBuffer(incomingZclIpPacket + index);
      index += 2;
      device->endpoints[i].clusters[j].server = incomingZclIpPacket[index++];
    }
  }
  return EMBER_SUCCESS;
}
