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
 * @brief APIs and defines for the Device Table plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_DEVICE_TABLE_H
#define SILABS_DEVICE_TABLE_H

typedef uint8_t EmberAfPluginDeviceTableDeviceState;

enum {
  EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_NULL         = 0x00,
  EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_JOINED       = 0x10,
  EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_UNRESPONSIVE = 0x11,
  EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_LEAVE_SENT   = 0x20,
  EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_LEFT         = 0x30,
  EMBER_AF_PLUGIN_DEVICE_TABLE_STATE_UNKNOWN      = 0xff
};

#define EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE 20

typedef struct {
  uint8_t     endpoint;
  uint16_t    deviceId;
  uint32_t    lastMsgTimestamp;
  EmberNodeId nodeId;
  EmberEUI64  eui64;
  EmberAfPluginDeviceTableDeviceState state;
  uint16_t clusterIds[EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE];
  uint8_t clusterOutStartPosition;
} EmberAfPluginDeviceTableEntry;

#define EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE 250
#define EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID 0xffff
#define EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX 0xffff

/** @brief Sends a message using the device table.
 *
 * Sends the current framework ZCL command to the device specified
 * by an index.
 *
 * @param index  The index of the device to which to send the CLI command.
 */
void emberAfDeviceTableCliIndexSend(uint16_t index);

/** @brief Sends a message using the device table.
 *
 * Sends the current framework ZCL command using a device specified
 * by the EUI64 and endpoint.
 *
 * @param eui64  EUI64 to which to send the CLI command.
 * @param index  Endpoint of the device to which to send the CLI command.
 */
void emberAfDeviceTableSend(EmberEUI64 eui64, uint8_t endpoint);

/** @brief Sends a CLI command based on index and endpoint.
 *
 * Sends the current framework ZCL command using a device indexed in
 * the device table.
 *
 * @param index  The index of the device to which to send the command.
 * @param endpoint The endpoint of the device to which to send the command.
 */
void emberAfDeviceTableCliIndexSendWithEndpoint(uint16_t index,
                                                uint8_t endpoint);

/** @brief Sends an internally-generated command.
 *
 * Sends an internally-generated command, such as the one generated
 * by the emberAfFillCommandOnOffClusterOn(); command.  Note:  this is
 * different from the above send command, which works with the CLI-generated
 * outgoing commands.
 *
 * @param  The index of the device to which to send the command.
 */
void emberAfDeviceTableCommandIndexSend(uint16_t index);

/** @brief Sends an internally generated command.
 *
 * Sends an internally-generated command, such as the one generated
 * by the emberAfFillCommandOnOffClusterOn(); command.  Note:  this is
 * different from the above send command, which works with the CLI-generated
 * outgoing commands.
 *
 * @param eui64  EUI64 of the device to which to send the command.
 * @param endpoint  The endpoint of the device to which to send the command.
 */
void emberAfDeviceTableCommandSendWithEndpoint(EmberEUI64 eui64,
                                               uint8_t endpoint);

/** @brief Sends an internally-generated command.
 *
 * Sends an internally-generated command, such as the one generated
 * by the emberAfFillCommandOnOffClusterOn(); command.  Note:  this is
 * different from the above send command, which works with the CLI-generated
 * outgoing commands.
 *
 * @param index  The index of the device to which to send the command.
 * @param endpoint  The endpoint of the device to which to send the command.
 */
void emberAfDeviceTableCommandIndexSendWithEndpoint(uint16_t index,
                                                    uint8_t endpoint);

/** @brief Computes the EUI64 from the node ID in the device table.
 *
 * This function looks up the device-table entry using the node ID and
 * return the EUI64.
 *
 * @param emberNodeId  The node ID of the device.
 * @param eui64  The location of the EUI64 variable.
 *
 * @return bool  True if an EUI can be found, false if not.
 */
bool emberAfDeviceTableGetEui64FromNodeId(EmberNodeId emberNodeId,
                                          EmberEUI64 eui64);

/** @brief Computes the node ID from the device table index.
 *
 * Looks up the device table entry based on the index and returns
 * the node ID.
 *
 * @param index  The index of the device.
 *
 * @return  The node ID of the device.
 */
uint16_t emberAfDeviceTableGetNodeIdFromIndex(uint16_t index);

/** @brief Finds the endpoint index from node ID and endpoint number.
 *
 * Computes the index into the endpoint table based on the node ID and the
 * endpoint number.
 *
 * @param emberNodeId  The node ID of the device.
 * @param endpoint  The desired endpoint.
 *
 * @return  The index of the device.
 */
uint16_t emberAfDeviceTableGetEndpointFromNodeIdAndEndpoint(EmberNodeId emberNodeId,
                                                            uint8_t endpoint);

/** @brief Finds the index based on the node ID.
 *
 * Computes the device-table index based on the node ID.
 *
 * @param emberNodeId  The node ID of the current device.
 *
 * @return  The index in the device-table.
 */
uint16_t emberAfDeviceTableGetIndexFromNodeId(EmberNodeId emberNodeId);

/** @brief Finds the index based on the EUI64.
 *
 * Computes the first device table index from the EUI64.
 *
 * @param eui64  EUI64 of the desired node ID.
 *
 * @return  The first index that matches the EUI64.
 */
uint16_t emberAfDeviceTableGetFirstIndexFromEui64(EmberEUI64 eui64);

/** @brief Finds the index based on the EUI64 and endpoint.
 *
 * Computes the node ID based on the device table entry that matches the EUI64.
 *
 * @param eui64  EUI64 of the desired index.
 * @param endpoint  The endpoint for the desired index.
 *
 * @return  Index that matches the EUI64
 */
uint16_t emberAfDeviceTableGetIndexFromEui64AndEndpoint(EmberEUI64 eui64,
                                                        uint8_t endpoint);

/** @brief Finds the node ID based on the EUI64.
 *
 * Computes the node ID based on the device table entry that matches the EUI64.
 *
 * @param eui64  EUI64 of the desired node ID.
 *
 * @return  The node ID that matches the EUI64.
 */
uint16_t emberAfDeviceTableGetNodeIdFromEui64(EmberEUI64 eui64);

/** @brief Changes the device status based on the last message sent.
 *
 * If the last sent message was a success, change the device state to "joined".
 * However, if the last message sent was not a success, change the device state
 * to unresponsive.
 *
 * @param nodeId  The node ID in the last outgoing message.
 * @param status  EmberStatus of the last outgoing message.
 * @param profileId  The profile ID of the last outgoing message.
 * @param clusterId  The cluster ID of the last outgoing message.
 */
void emberAfPluginDeviceTableMessageSentStatus(EmberNodeId nodeId,
                                               EmberStatus status,
                                               uint16_t profileId,
                                               uint16_t clusterId);

/** @brief Sends a leave message to a device based on the index.
 *
 * Sends a leave message to the device indicated by the index.
 *
 * @param index  The index of the device to whom to send the leave message.
 */
void emberAfPluginDeviceTableSendLeave(uint16_t index);

/** @brief Informs the device table that a message was received.
 *
 * The device table keeps track of whether a device has become unresponsive.
 * Hearing from a known device will cause the device table to transition the
 * device state from "unresponsive" to "joined". Hearing from an unknown
 * device will cause the device table to discover the device, potentially
 * fixing routing issues (i.e., if the node ID has changed).
 *
 * @param nodeId  The node ID from which the last incoming message came.
 */
void emberAfPluginDeviceTableMessageReceived(EmberNodeId nodeId);

/** @brief Kicks off device table route repair.
 *
 * A message was sent to the device and failed. Therefore, the route
 * repair will be attempted and the node marked as unresponsive. Currently, route
 * repair simply sends a broadcast message looking for the node ID based on
 * the EUI64. It may be replaced with something more sophisticated in the
 * future.
 *
 * @param nodeId  The node ID of the newly unresponsive node.
 */
void emberAfPluginDeviceTableInitiateRouteRepair(EmberNodeId nodeId);

/** @brief Returns a pointer to the device table structure.
 *
 * Returns a pointer to the device table structure.
 *
 * @return  A pointer to the device table structure.
 */
EmberAfPluginDeviceTableEntry* emberAfDeviceTablePointer(void);

/** @brief Returns a pointer to the device table entry.
 *
 * Returns a pointer to the device table entry based on the device table index.
 *
 * @param index  The index of the device table entry in which we are interested.
 *
 * @return  A pointer to the device table entry.
 */
EmberAfPluginDeviceTableEntry* emberAfDeviceTableFindDeviceTableEntry(uint16_t index);

/** @brief Informs the device table that a new device joined.
 *
 * When a device joins, rejoins, or sends an end-device announce,
 * call this function in the device table. If the device is new, it will
 * generate a new device table entry and kick off the discovery process. If
 * the device is not new, the device-table will make sure the node ID matches
 * the EUI64 and make an update if necessary.  It will also check to
 * verify the discovery process has completed and attempt to complete it.
 *
 * @param newNodeId  The node ID of the newly joined device.
 *
 * @param newNodeEui64  The EUI64 of the newly joined device.
 */
void emberAfDeviceTableNewDeviceJoinHandler(EmberNodeId newNodeId,
                                            EmberEUI64 newNodeEui64);

/** @brief Calculates time since the device heard the last message.
 *
 * Computes the amount of time, in seconds, since receiving a
 * message from the device.
 *
 * @param index  The index of the device.
 *
 * @return  The time since the last message (in seconds).
 */
uint32_t emberAfDeviceTableTimeSinceLastMessage(uint16_t index);

/** @brief Returns true if the two EUI64 parameters match.
 *
 * Determines whether two EUI64 values are the same. The function
 * will print a feedback message to indicate if both EUI64 values are NULL.
 * The function will also print out a feedback message to indicate if the EUI64
 * values match but in reverse order.
 *
 * @param eui64a  The first EUI64 to compare.
 * @param eui64b  The second EUI64 to compare.
 */
bool emberAfDeviceTableMatchEui64(EmberEUI64 a, EmberEUI64 b);

/** @brief Clears the device table.
 *
 * Clears the device table and the backup file of the device
 * table is also cleared if it is a host.
 *
 */
void emberAfDeviceTableClear(void);

// ZigBee device IDs
#define DEVICE_ID_ON_OFF_SWITCH 0x0000
#define DEVICE_ID_LEVEL_CONTROL_SWITCH 0x0001
#define DEVICE_ID_ON_OFF_OUTPUT 0x0002
#define DEVICE_ID_LEVEL_CONTROL_OUTPUT 0x0003
#define DEVICE_ID_SCENE_SELECTOR 0x0004
#define DEVICE_ID_CONFIG_TOOL 0x0005
#define DEVICE_ID_REMOTE_CONTROL 0x0006
#define DEVICE_ID_COMBINED_INTERFACE 0x0007
#define DEVICE_ID_RANGE_EXTENDER 0x0008
#define DEVICE_ID_MAINS_POWER_OUTLET 0x0009
#define DEVICE_ID_DOOR_LOCK 0x000a
#define DEVICE_ID_DOOR_LOCK_CONTROLLER 0x000b
#define DEVICE_ID_SIMPLE_SENSOR 0x000c
#define DEVICE_ID_CONSUMPTION_AWARENESS_DEVICE 0x000d
#define DEVICE_ID_HOME_GATEWAY 0x0050
#define DEVICE_ID_SMART_PLUG 0x0051
#define DEVICE_ID_WHITE_GOODS 0x0052
#define DEVICE_ID_METER_INTERFACE 0x0053

#define DEVICE_ID_ON_OFF_LIGHT 0x0100
#define DEVICE_ID_DIMMABLE_LIGHT 0x0101
#define DEVICE_ID_COLOR_DIMMABLE_LIGHT 0x0102
#define DEVICE_ID_ON_OFF_LIGHT_SWITCH 0x0103
#define DEVICE_ID_DIMMER_SWITCH 0x0104
#define DEVICE_ID_COLOR_DIMMER_SWITCH 0x0105
#define DEVICE_ID_LIGHT_SENSOR 0x0106
#define DEVICE_ID_OCCUPANCY_SENSOR 0x0107

#define DEVICE_ID_SHADE 0x0200
#define DEVICE_ID_SHADE_CONTROLLER 0x0201
#define DEVICE_ID_WINDOW_COVERING_DEVICE 0x0202
#define DEVICE_ID_WINDOW_COVERING_CONTROLLER 0x0203

#define DEVICE_ID_HEATING_COOLING_UNIT 0x0300
#define DEVICE_ID_THERMOSTAT 0x0301
#define DEVICE_ID_TEMPERATURE_SENSOR 0x0302
#define DEVICE_ID_PUMP 0x0303
#define DEVICE_ID_PUMP_CONTROLLER 0x0304
#define DEVICE_ID_PRESSURE_SENSOR 0x0305
#define DEVICE_ID_FLOW_SENSOR 0x0306
#define DEVICE_ID_MINI_SPLIT_AC 0x0307

#define DEVICE_ID_IAS_CIE 0x0400
#define DEVICE_ID_IAS_ANCILLARY_CONTROL 0x0401
#define DEVICE_ID_IAS_ZONE 0x0402
#define DEVICE_ID_IAS_WARNING 0x0403

#endif //__DEVICE_TABLE_H
