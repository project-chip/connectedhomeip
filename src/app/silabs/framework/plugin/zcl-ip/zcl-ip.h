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
 * @brief This file declares an interface to send and receive zcl messages over an
 * IP interface.
 *******************************************************************************
   ******************************************************************************/

#define EM_AF_ZCL_IP_VERSION 0x01

// Overall Packet Format
#define EM_AF_ZCL_IP_HEADER_INDEX        0
#define EM_AF_ZCL_IP_PACKET_LENGTH_INDEX 2
#define EM_AF_ZCL_IP_COMMAND_ID_INDEX    4
// Variable Payload

#define EM_AF_ZCL_IP_COMMAND_ID_FIELD_LENGTH 2

#define EM_AF_ZCL_IP_MINIMUM_PACKET_SIZE (EM_AF_ZCL_IP_COMMAND_ID_INDEX + EM_AF_ZCL_IP_COMMAND_ID_FIELD_LENGTH)

// Command IDs
#define EM_AF_ZCL_IP_KEEPALIVE_PACKET_ID          0x0000
#define EM_AF_ZCL_IP_COMMAND_ZCL_PROXY_PACKET_ID  0x0001
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_ID 0x0002
#define EM_AF_ZCL_IP_ADD_DEVICE_ID                0x0003
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_ID  0x0004
#define EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_ID   0x0005

// Keep alive Command (0x0000)
#define EM_AF_ZCL_IP_UNIX_TIME_INDEX                6
#define EM_AF_ZCL_IP_KEEPALIVE_DELAY_SECONDS_INDEX 10
#define EM_AF_ZCL_IP_KEEPALIVE_DELAY_SECONDS_SIZE   4

#define EM_AF_ZCL_IP_KEEPALIVE_PACKET_SIZE (EM_AF_ZCL_IP_KEEPALIVE_DELAY_SECONDS_INDEX + EM_AF_ZCL_IP_KEEPALIVE_DELAY_SECONDS_SIZE)

// ZCL Proxy Packet Format Command (0x0001)
#define EM_AF_ZCL_IP_CLUSTER_ID_INDEX    6
#define EM_AF_ZCL_IP_PROFILE_ID_INDEX    8
#define EM_AF_ZCL_IP_SOURCE_EP_INDEX     10
#define EM_AF_ZCL_IP_DEST_EP_INDEX       11
#define EM_AF_ZCL_IP_EUI64_INDEX         12
#define EM_AF_ZCL_IP_APS_OPTIONS_INDEX   20
#define EM_AF_ZCL_IP_ZCL_LENGTH_INDEX    22
#define EM_AF_ZCL_IP_ZCL_PAYLOAD_INDEX   24

#define EM_AF_ZCL_IP_PROXY_PACKET_MINIMUM_SIZE (EM_AF_ZCL_IP_ZCL_PAYLOAD_INDEX)

// ZCL Configure Remote Clusters Command (0x0002)
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_COUNT_INDEX         6
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_START_INDEX  8

#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_COMMAND_OVERHEAD 2

// These offsets relative to prior record
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_CLUSTER_OFFSET   0
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_PROFILE_OFFSET   2
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_DEVICE_ID_OFFSET 4
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_ENDPOINT_OFFSET  6
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_TYPE_OFFSET      7

#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_TYPE_OFFSET_LENGTH 2

#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_LENGTH \
  (EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_TYPE_OFFSET \
   + EM_AF_ZCL_IP_CONFIGURE_REMOTE_CLUSTERS_RECORD_TYPE_OFFSET_LENGTH)

// Add Device Command (0x0003)
#define EM_AF_ZCL_IP_ADD_DEVICE_EUI64_INDEX     6
#define EM_AF_ZCL_IP_ADD_DEVICE_LINK_KEY_INDEX 14
#define EM_AF_ZCL_IP_ADD_DEVICE_COMMAND_LENGTH 30

// ZDO Configure Remote Binding Command(0x0004)
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_COMMAND_TARGET_EUI_OFFSET       6
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_COMMAND_SOURCE_ENDPOINT_OFFSET 14
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_COMMAND_DEST_ENDPOINT_OFFSET   15
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_COMMAND_CLUSTER_ID_OFFSET      16
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_COMMAND_DEST_EUI_OFFSET        18
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_COMMAND_SOURCE_EUI_OFFSET      26
#define EM_AF_ZCL_IP_CONFIGURE_REMOTE_BINDING_COMMAND_LENGTH                 34

// Device Discovery Update Command (0x0004)
#define EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_EUI64_INDEX          6
#define EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_STATUS_INDEX         14
#define EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_CAPABILITIES_INDEX   15
#define EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_ENDPOINT_COUNT_INDEX 16
#define EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_ENDPOINT_INFO_INDEX  17

// These are relative to the start of the endpoint information struct.
#define EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_ENDPOINT_OFFSET      0
#define EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_PROFILE_ID_OFFSET    1
#define EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_DEVICE_ID_OFFSET     3
#define EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_CLUSTER_COUNT_OFFSET 5
#define EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_CLUSTER_ID_OFFSET    6

#define EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_ENDPOINT_INFO_MIN_LENGTH \
  EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_CLUSTER_ID_OFFSET

// A device with 0 endpoints discovered (such as a sleepy device), has no endpoint and cluster info.
#define EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_MIN_LENGTH \
  EM_AF_ZCL_IP_DEVICE_DISCOVERY_UPDATE_COMMAND_ENDPOINT_INFO_INDEX

EmberStatus emberAfPluginZclIpGenerateOutgoingProxyPacket(EmberApsFrame* apsFrame,
                                                          EmberNodeId nodeId,
                                                          EmberEUI64 eui64,
                                                          uint8_t* zclMessageBuffer,
                                                          uint16_t zclMessageBufferLength);

EmberStatus emberAfPluginZclIpGenerateOutgoingKeepalive(uint32_t unixTime,
                                                        uint32_t nextKeepaliveDelaySeconds);

EmberStatus emberAfPluginZclIpRetrieveIncomingEncapsulatedZclProxyPacket(EmberAfClusterCommand* returnCmd,
                                                                         EmberEUI64 returnEui64);
EmberStatus emberAfPluginZclIpRetrieveIncomingKeepalive(uint32_t* unixTimeSinceEpoch,
                                                        uint32_t* nextKeepaliveDelaySeconds);
EmberStatus emberAfPluginZclIpRetrieveIncomingConfigureRemoteClusters(EmberAfRemoteClusterStruct* returnData,
                                                                      uint16_t maxCount,
                                                                      uint16_t* returnedCount);
EmberStatus emberAfPluginZclIpGenerateOutgoingConfigureRemoteCluster(uint16_t count,
                                                                     const EmberAfRemoteClusterStruct* records);

EmberStatus emberAfPluginZclIpGenerateOutgoingAddDevice(EmberEUI64 newDeviceEui64,
                                                        const EmberKeyData* newDeviceKeyData);

EmberStatus emberAfPluginZclIpGenerateOutgoingConfigureRemoteBinding(EmberEUI64 targetEUI64,
                                                                     uint8_t sourceEndpoint,
                                                                     uint8_t destEndpoint,
                                                                     uint16_t clusterId,
                                                                     EmberEUI64 destEUI64,
                                                                     EmberEUI64 sourceEUI64);

EmberStatus emberAfPluginZclIpGenerateOutgoingDeviceDiscoveryUpdate(const EmberAfDeviceInfo* device);

uint16_t emberAfPluginZclIpGetPacketLength(const uint8_t* zclIpPacket);

uint8_t* emberAfPluginZclIpGetReceivedPacketPointer(void);
uint16_t emberAfPluginZclIpGetReceivedPacketCommandId(void);
uint8_t* emberAfPluginZclIpGetReceivedProxyPacketEui64Pointer(void);

#define emAfGetLittleEndianInt16uFromBuffer(pointer) \
  HIGH_LOW_TO_INT((pointer)[1], (pointer)[0])

#define emAfGetLittleEndianInt32uFromBuffer(pointer) \
  (emAfGetLittleEndianInt16uFromBuffer(pointer)      \
   + (((uint32_t)emAfGetLittleEndianInt16uFromBuffer((pointer) + 2)) << 16))

EmberStatus emberAfPluginZclIpRetrieveIncomingAddDevice(EmberEUI64 newDeviceEui64,
                                                        EmberKeyData* newDeviceKeyData);

EmberStatus emberAfPluginZclIpRetrieveIncomingConfigureRemoteBinding(EmberAfRemoteBindingStruct *returnData);

EmberStatus emberAfPluginZclIpRetrieveIncomingDeviceDiscoveryUpdate(EmberAfDeviceInfo* device);
