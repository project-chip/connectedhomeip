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
 * @brief Splits long messages into smaller blocks for transmission and reassembles
 * received blocks.
 *******************************************************************************
   ******************************************************************************/

#ifndef ZIGBEE_APSC_MAX_TRANSMIT_RETRIES
#define ZIGBEE_APSC_MAX_TRANSMIT_RETRIES 3
#endif //ZIGBEE_APSC_MAX_TRANSMIT_RETRIES

#ifndef EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS
#define EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS 2
#endif //EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS

#ifndef EMBER_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS
#define EMBER_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS 2
#endif //EMBER_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS

#ifndef EMBER_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE
#define EMBER_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE 1500
#endif //EMBER_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE

#ifndef EMBER_AF_PLUGIN_FRAGMENTATION_RX_WINDOW_SIZE
#define EMBER_AF_PLUGIN_FRAGMENTATION_RX_WINDOW_SIZE 1
#endif //EMBER_AF_PLUGIN_FRAGMENTATION_RX_WINDOW_SIZE

// TODO: We should have the App Builder generating these events. For now, I
// manually added 10 events which means we will be able to set and accept up to
// 10 incoming distinct fragmented packets. In AppBuilder the max incoming
// packets number is capped to 10, therefore we will never run out of events.
#define EMBER_AF_FRAGMENTATION_EVENTS                                                 \
  { &(emAfFragmentationEvents[0]), (void (*)(void))emAfFragmentationAbortReception }, \
  { &(emAfFragmentationEvents[1]), (void (*)(void))emAfFragmentationAbortReception }, \
  { &(emAfFragmentationEvents[2]), (void (*)(void))emAfFragmentationAbortReception }, \
  { &(emAfFragmentationEvents[3]), (void (*)(void))emAfFragmentationAbortReception }, \
  { &(emAfFragmentationEvents[4]), (void (*)(void))emAfFragmentationAbortReception }, \
  { &(emAfFragmentationEvents[5]), (void (*)(void))emAfFragmentationAbortReception }, \
  { &(emAfFragmentationEvents[6]), (void (*)(void))emAfFragmentationAbortReception }, \
  { &(emAfFragmentationEvents[7]), (void (*)(void))emAfFragmentationAbortReception }, \
  { &(emAfFragmentationEvents[8]), (void (*)(void))emAfFragmentationAbortReception }, \
  { &(emAfFragmentationEvents[9]), (void (*)(void))emAfFragmentationAbortReception },

#define EMBER_AF_FRAGMENTATION_EVENT_STRINGS \
  "Frag 0",                                  \
  "Frag 1",                                  \
  "Frag 2",                                  \
  "Frag 3",                                  \
  "Frag 4",                                  \
  "Frag 5",                                  \
  "Frag 6",                                  \
  "Frag 7",                                  \
  "Frag 8",                                  \
  "Frag 9",

extern EmberEventControl emAfFragmentationEvents[10];

//------------------------------------------------------------------------------
// Sending

typedef struct {
  EmberOutgoingMessageType  messageType;
  uint16_t                    indexOrDestination;
  uint8_t                     sequence;
  EmberApsFrame             apsFrame;
  uint8_t*                    bufferPtr;
  uint16_t                    bufLen;
  uint8_t                     fragmentLen;
  uint8_t                     fragmentCount;
  uint8_t                     fragmentBase;
  uint8_t                     fragmentsInTransit;
}txFragmentedPacket;

EmberStatus emAfFragmentationSendUnicast(EmberOutgoingMessageType type,
                                         uint16_t indexOrDestination,
                                         EmberApsFrame *apsFrame,
                                         uint8_t *buffer,
                                         uint16_t bufLen,
                                         uint8_t *messageTag);

bool emAfFragmentationMessageSent(EmberApsFrame *apsFrame,
                                  EmberStatus status);

void emAfFragmentationMessageSentHandler(EmberOutgoingMessageType type,
                                         uint16_t indexOrDestination,
                                         EmberApsFrame *apsFrame,
                                         uint8_t *buffer,
                                         uint16_t bufLen,
                                         EmberStatus status,
                                         uint8_t messageTag);

//------------------------------------------------------------------------------
// Receiving.

typedef enum {
  EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_AVAILABLE         = 0,
  EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_ACKED             = 1,
  EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_IN_USE            = 2,
  EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_PAYLOAD_TOO_LARGE = 3
}rxPacketStatus;

typedef struct {
  rxPacketStatus status;
  uint8_t       ackedPacketAge;
  uint8_t       buffer[EMBER_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE];
  EmberNodeId fragmentSource;
  uint8_t       fragmentSequenceNumber;
  uint8_t       fragmentBase; // first fragment inside the rx window.
  uint16_t      windowFinger; //points to the first byte inside the rx window.
  uint8_t       fragmentsExpected; // total number of fragments expected.
  uint8_t       fragmentsReceived; // fragments received so far.
  uint8_t       fragmentMask; // bitmask of received fragments inside the rx window.
  uint8_t       lastfragmentLen; // Length of the last fragment.
  uint8_t       fragmentLen; // Length of the fragment inside the rx window.
                             // All the fragments inside the rx window should have
                             // the same length.
  EmberEventControl *fragmentEventControl;
}rxFragmentedPacket;

bool emAfFragmentationIncomingMessage(EmberIncomingMessageType type,
                                      EmberApsFrame *apsFrame,
                                      EmberNodeId sender,
                                      uint8_t **buffer,
                                      uint16_t *bufLen);

void emAfFragmentationAbortReception(EmberEventControl* control);

extern uint8_t  emberFragmentWindowSize;

extern uint16_t emberMacIndirectTimeout;
#if defined(EZSP_HOST)
extern uint16_t emberApsAckTimeoutMs;
#endif

void emAfPluginFragmentationPlatformInitCallback(void);

EmberStatus emAfPluginFragmentationSend(txFragmentedPacket* txPacket,
                                        uint8_t fragmentNumber,
                                        uint16_t fragmentLen,
                                        uint16_t offset);

void emAfPluginFragmentationHandleSourceRoute(txFragmentedPacket* txPacket,
                                              uint16_t indexOrDestination);

void emAfPluginFragmentationSendReply(EmberNodeId sender,
                                      EmberApsFrame* apsFrame,
                                      rxFragmentedPacket* rxPacket);

#if defined(EMBER_TEST)
extern uint8_t emAfPluginFragmentationArtificiallyDropBlockNumber;
#endif
