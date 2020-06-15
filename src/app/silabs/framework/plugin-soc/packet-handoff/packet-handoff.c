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
 * @brief Implementation of the packet-handoff mechanism.  Definitions for the
 * emberPacketHandoff stack hooks, intercepting packets from the stack,
 * copying the data into a flat buffer, then passing the flat packet
 * to other plugins for further processing.
 *******************************************************************************
   ******************************************************************************/

#include PLATFORM_HEADER
#include "stack/include/ember-types.h"
#include "stack/include/packet-buffer.h"

#ifdef PRO_COMPLIANCE
 #include "app/test/pro-compliance.h"
 #include "app/test/pro-compliance-printing.h"
#endif

#ifndef PACKET_HANDOFF_BUFFER_SIZE
  #define PACKET_HANDOFF_BUFFER_SIZE 256
#endif

#define EMBER_APPLICATION_HAS_PACKET_HANDOFF

// ----------------------------------------------------------------
// Intermediate Packet Handoff Mechanism
// Converts ::EmberMessageBuffer into a flat array then produces the callback
// to emberAfIncomingPacketFilterCallback and emberAfOutgoingPacketFilterCallback
// ----------------------------------------------------------------

// External Function Definitions
EmberMessageBuffer emPacketHeaderPayload(EmberMessageBuffer header);
void emSetPacketHeaderPayload(EmberMessageBuffer header, EmberMessageBuffer payload);

#if defined(EMBER_CALLBACK_INCOMING_PACKET_FILTER) \
  || defined(EMBER_CALLBACK_OUTGOING_PACKET_FILTER)
static uint8_t flatPacket[PACKET_HANDOFF_BUFFER_SIZE];
#endif

#ifdef EMBER_CALLBACK_INCOMING_PACKET_FILTER
EmberPacketAction emberAfIncomingPacketFilterCallback(EmberZigbeePacketType,
                                                      uint8_t*,
                                                      uint8_t*,
                                                      void *);
EmberPacketAction emberPacketHandoffIncoming(EmberZigbeePacketType packetType,
                                             EmberMessageBuffer packetBuffer,
                                             uint8_t index,
                                             void *data)
{
  switch (packetType) {
    #ifndef EMBER_AF_PLUGIN_PACKET_HANDOFF_ALLOW_ALL_PACKETS

      #ifndef EMBER_AF_PLUGIN_PACKET_HANDOFF_ALLOW_RAW_MAC
    case EMBER_ZIGBEE_PACKET_TYPE_RAW_MAC:
      return EMBER_ACCEPT_PACKET;
      #endif // RAW_MAC

      #ifndef EMBER_AF_PLUGIN_PACKET_HANDOFF_ALLOW_MAC_COMMAND
    case EMBER_ZIGBEE_PACKET_TYPE_MAC_COMMAND:
      return EMBER_ACCEPT_PACKET;
      #endif // MAC_COMMAND

      #ifndef EMBER_AF_PLUGIN_PACKET_HANDOFF_ALLOW_NETWORK_DATA
    case EMBER_ZIGBEE_PACKET_TYPE_NWK_DATA:
      return EMBER_ACCEPT_PACKET;
      #endif // NETWORK_DATA

      #ifndef EMBER_AF_PLUGIN_PACKET_HANDOFF_ALLOW_NETWORK_COMMAND
    case EMBER_ZIGBEE_PACKET_TYPE_NWK_COMMAND:
      return EMBER_ACCEPT_PACKET;
      #endif // NETWORK_COMMAND

      #ifndef EMBER_AF_PLUGIN_PACKET_HANDOFF_ALLOW_APS_DATA
    case EMBER_ZIGBEE_PACKET_TYPE_APS_DATA:
      return EMBER_ACCEPT_PACKET;
      #endif // APS_DATA

      #ifndef EMBER_AF_PLUGIN_PACKET_HANDOFF_ALLOW_APS_COMMAND
    case EMBER_ZIGBEE_PACKET_TYPE_APS_COMMAND:
      return EMBER_ACCEPT_PACKET;
      #endif // APS_COMMAND

      #ifndef EMBER_AF_PLUGIN_PACKET_HANDOFF_ALLOW_ZDO
    case EMBER_ZIGBEE_PACKET_TYPE_ZDO:
      return EMBER_ACCEPT_PACKET;
      #endif // ZDO

      #ifndef EMBER_AF_PLUGIN_PACKET_HANDOFF_ALLOW_ZCL
    case EMBER_ZIGBEE_PACKET_TYPE_ZCL:
      return EMBER_ACCEPT_PACKET;
      #endif // ZCL

      #ifndef EMBER_AF_PLUGIN_PACKET_HANDOFF_ALLOW_BEACON
    case EMBER_ZIGBEE_PACKET_TYPE_BEACON:
      return EMBER_ACCEPT_PACKET;
      #endif // BEACON

    #endif // !EMBER_AF_PLUGIN_ALLOW_ALL_PACKETS
    default:
    {
      uint8_t packetLength = emberMessageBufferLength(packetBuffer) - index;
      EmberPacketAction act;
      emberCopyFromLinkedBuffers(packetBuffer,
                                 index,
                                 flatPacket,
                                 packetLength);
      act = emberAfIncomingPacketFilterCallback(packetType, flatPacket, &packetLength, data);
      if (act == EMBER_MANGLE_PACKET) {
        EmberStatus status = emberSetLinkedBuffersLength(packetBuffer,
                                                         packetLength + index);
        // If changing the length of the buffer is unsuccessful (ran out of free block)
        // tell the stack to drop the packet because something has probably gone wrong
        if (status != EMBER_SUCCESS) {
          return EMBER_DROP_PACKET;
        } else {
          emberCopyToLinkedBuffers(flatPacket,
                                   packetBuffer,
                                   index,
                                   packetLength);
        }
      }
      return act;
    }
  }
}
#else
EmberPacketAction emberPacketHandoffIncoming(EmberZigbeePacketType packetType,
                                             EmberMessageBuffer packetBuffer,
                                             uint8_t index,
                                             void *data)
{
  return EMBER_ACCEPT_PACKET;
}
#endif // EMBER_CALLBACK_INCOMING_PACKET_FILTER

#ifdef EMBER_CALLBACK_OUTGOING_PACKET_FILTER
EmberPacketAction emberAfOutgoingPacketFilterCallback(EmberZigbeePacketType,
                                                      uint8_t*,
                                                      uint8_t*,
                                                      void *);
EmberPacketAction emberPacketHandoffOutgoing(EmberZigbeePacketType packetType,
                                             EmberMessageBuffer packetBuffer,
                                             uint8_t index,
                                             void *data)
{
  uint8_t packetLength = emberMessageBufferLength(packetBuffer) - index;
  EmberPacketAction act;
  emberCopyFromLinkedBuffers(packetBuffer,
                             index,
                             flatPacket,
                             packetLength);
  if (emPacketHeaderPayload(packetBuffer) != EMBER_NULL_MESSAGE_BUFFER) {
    EmberMessageBuffer payload = emPacketHeaderPayload(packetBuffer);
    uint8_t payloadLength = emberMessageBufferLength(payload);
    emberCopyFromLinkedBuffers(payload,
                               0,
                               flatPacket + packetLength,
                               payloadLength);
    packetLength += payloadLength;
  }
  act = emberAfOutgoingPacketFilterCallback(packetType, flatPacket, &packetLength, data);
  if (act == EMBER_MANGLE_PACKET) {
    EmberStatus status = emberSetLinkedBuffersLength(packetBuffer,
                                                     packetLength + index);
    // If changing the length of the buffer is unsuccessful (ran out of free block)
    // tell the stack to drop the packet because something has probably gone wrong
    if (status != EMBER_SUCCESS) {
      return EMBER_DROP_PACKET;
    } else {
      emberCopyToLinkedBuffers(flatPacket,
                               packetBuffer,
                               index,
                               packetLength);
      // The payload is now appended to the end of the packet
      emSetPacketHeaderPayload(packetBuffer, EMBER_NULL_MESSAGE_BUFFER);
    }
  }
  return act;
}
#else
EmberPacketAction emberPacketHandoffOutgoing(EmberZigbeePacketType packetType,
                                             EmberMessageBuffer packetBuffer,
                                             uint8_t index,
                                             void *data)
{
  return EMBER_ACCEPT_PACKET;
}
#endif // EMBER_CALLBACK_OUTGOING_PACKET_FILTER
