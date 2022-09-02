/***************************************************************************//**
 * @brief ZigBee PRO Full Stack component configuration header.
 *\n*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
// <<< Use Configuration Wizard in Context Menu >>>

// <h>Zigbee PRO Stack Library configuration

// <o EMBER_MAX_END_DEVICE_CHILDREN> Child Table Size <0-64>
// <i> Default: 6
// <i> The maximum number of ZigBee PRO End Devices that can be supported by a single device.
#define EMBER_MAX_END_DEVICE_CHILDREN   32

// <o EMBER_PACKET_BUFFER_COUNT> Packet Buffer Count <20-254>
// <i> Default: 75
// <i> The maximum number of packet buffers supported by the system.  This is limited by the amount of available RAM.
#define EMBER_PACKET_BUFFER_COUNT   75

// <o EMBER_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE> End Device keep alive support mode
// <i> End Device keep alive support mode
// <EMBER_MAC_DATA_POLL_KEEP_ALIVE=> MAC Data Poll Keep Alive
// <EMBER_END_DEVICE_TIMEOUT_KEEP_ALIVE=> End Device Timeout Keep Alive
// <EMBER_KEEP_ALIVE_SUPPORT_ALL=> Keep Alive Support All
// <i> Default: EMBER_KEEP_ALIVE_SUPPORT_ALL
// <i> End Device keep alive support mode on the coordinator/router could be set here.
#define EMBER_END_DEVICE_KEEP_ALIVE_SUPPORT_MODE   EMBER_KEEP_ALIVE_SUPPORT_ALL

// <o EMBER_END_DEVICE_POLL_TIMEOUT> End Device Poll Timeout Value
// <i> End Device Poll Timeout Value
// <SECONDS_10=> Seconds-10
// <MINUTES_2=> Minutes-2
// <MINUTES_4=> Minutes-4
// <MINUTES_8=> Minutes-8
// <MINUTES_16=> Minutes-16
// <MINUTES_32=> Minutes-32
// <MINUTES_64=> Minutes-64
// <MINUTES_128=> Minutes-128
// <MINUTES_256=> Minutes-256
// <MINUTES_512=> Minutes-512
// <MINUTES_1024=> Minutes-1024
// <MINUTES_2048=> Minutes-2048
// <MINUTES_4096=> Minutes-4096
// <MINUTES_8192=> Minutes-8192
// <MINUTES_16384=> Minutes-16384
// <i> Default: MINUTES_256
// <i> The amount of time that must pass without hearing a MAC data poll from the device before the end device is removed from the child table.  For a router device this applies to its children.  For an end device, this is the amount of time before it automatically times itself out.
#define EMBER_END_DEVICE_POLL_TIMEOUT   MINUTES_256

// <o EMBER_LINK_POWER_DELTA_INTERVAL> Link Power Delta Request Interval <1-65535>
// <i> Default: 300
// <i> The amount of time in seconds that pass between link power delta requests.
#define EMBER_LINK_POWER_DELTA_INTERVAL   300

// <o EMBER_APS_UNICAST_MESSAGE_COUNT> APS Unicast Message Queue Size <1-255>
// <i> Default: 10
// <i> The maximum number of APS unicast messages that can be queued up by the stack.  A message is considered queued when emberSendUnicast() is called and is de-queued when the emberMessageSentHandler() is called.
#define EMBER_APS_UNICAST_MESSAGE_COUNT   10

// <o EMBER_BROADCAST_TABLE_SIZE> Broadcast Table Size <15-254>
// <i> Default: 15
// <i> The size of the broadcast table.
#define EMBER_BROADCAST_TABLE_SIZE   15

// <o EMBER_NEIGHBOR_TABLE_SIZE> Neighbor Table Size
// <i> Neighbor Table Size
// <16=> 16
// <26=> 26
// <i> Default: 16
// <i> The size of the neighbor table.
#define EMBER_NEIGHBOR_TABLE_SIZE   16

// <o EMBER_TRANSIENT_KEY_TIMEOUT_S> Transient key timeout (in seconds) <0-65535>
// <i> Default: 300
// <i> The amount of time a device will store a transient link key that can be used to join a network.
#define EMBER_TRANSIENT_KEY_TIMEOUT_S   300
// </h>

// <<< end of configuration section >>>
