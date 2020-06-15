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
 * @brief Splits long messages into smaller fragments for transmission and
 * reassembles received fragments into full messages.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/util.h"
#include "fragmentation.h"

//------------------------------------------------------------------------------
// Globals

EmberEventControl emAfFragmentationEvents[10];

#if defined(EMBER_AF_PLUGIN_FRAGMENTATION_FREE_OUTGOING_MESSAGE_PRIOR_TO_FINAL_ACK)
#define FREE_OUTGOING_MESSAGE_PRIOR_TO_FINAL_ACK true
#else
#define FREE_OUTGOING_MESSAGE_PRIOR_TO_FINAL_ACK false
#endif

static const bool freeOutgoingMessagePriorToFinalAck = FREE_OUTGOING_MESSAGE_PRIOR_TO_FINAL_ACK;

#define UNUSED_TX_PACKET_ENTRY 0xFF

//------------------------------------------------------------------------------
// Forward Declarations

static EmberStatus sendNextFragments(txFragmentedPacket* txPacket);
static void abortTransmission(txFragmentedPacket *txPacket, EmberStatus status);
static txFragmentedPacket* getFreeTxPacketEntry(void);
static txFragmentedPacket* txPacketLookUp(EmberApsFrame *apsFrame);
static void updateCurrentAppTasksForFragmentationState(void);

// We separate the outgoing buffer from the txPacket entry to allow us to keep around
// data about previous fragmented messages that have sent their last packet
// but not yet been acknowledged.  This saves space by not replicating the entire
// buffer required to store the outgoing message.  However, in that case we do not
// pass the complete message to the message sent handler.
static txFragmentedPacket txPackets[EMBER_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS];
static uint8_t txMessageBuffers[EMBER_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS]
[EMBER_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE];

static txFragmentedPacket txPacketAwaitingFinalAck = {
  UNUSED_TX_PACKET_ENTRY,
};

#if defined(EMBER_TEST)
  #define NO_BLOCK_TO_DROP 0xFF
uint8_t emAfPluginFragmentationArtificiallyDropBlockNumber = NO_BLOCK_TO_DROP;
  #define artificiallyDropBlock(block) (block == emAfPluginFragmentationArtificiallyDropBlockNumber)
  #define clearArtificiallyDropBlock() emAfPluginFragmentationArtificiallyDropBlockNumber = NO_BLOCK_TO_DROP;
  #define artificiallyDropBlockPrintln(format, arg) emberAfCorePrintln((format), (arg))

#else
  #define artificiallyDropBlock(block) false
  #define clearArtificiallyDropBlock()
  #define artificiallyDropBlockPrintln(format, arg)

#endif

#define messageTag(txPacket) ((txPacket)->sequence)

//------------------------------------------------------------------------------
// Functions

EmberStatus emAfFragmentationSendUnicast(EmberOutgoingMessageType type,
                                         uint16_t indexOrDestination,
                                         EmberApsFrame *apsFrame,
                                         uint8_t *buffer,
                                         uint16_t bufLen,
                                         uint8_t *messageTag)
{
  EmberStatus status;
  uint16_t fragments;
  txFragmentedPacket* txPacket;

  if (emberFragmentWindowSize == 0) {
    return EMBER_INVALID_CALL;
  }

  if (bufLen > EMBER_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE) {
    return EMBER_MESSAGE_TOO_LONG;
  }

  txPacket = getFreeTxPacketEntry();
  if (txPacket == NULL) {
    return EMBER_MAX_MESSAGE_LIMIT_REACHED;
  }

  txPacket->messageType = type;
  txPacket->indexOrDestination = indexOrDestination;
  MEMMOVE(&txPacket->apsFrame, apsFrame, sizeof(EmberApsFrame));
  txPacket->apsFrame.options |=
    (EMBER_APS_OPTION_FRAGMENT | EMBER_APS_OPTION_RETRY);

  emAfPluginFragmentationHandleSourceRoute(txPacket,
                                           indexOrDestination);

  MEMMOVE(txPacket->bufferPtr, buffer, bufLen);
  txPacket->bufLen = bufLen;
  txPacket->fragmentLen = emberAfMaximumApsPayloadLength(type,
                                                         indexOrDestination,
                                                         &txPacket->apsFrame);
  fragments = ((bufLen + txPacket->fragmentLen - 1) / txPacket->fragmentLen);
  if (fragments > MAX_INT8U_VALUE) {
    return EMBER_MESSAGE_TOO_LONG;
  }

  txPacket->fragmentCount = (uint8_t)fragments;
  txPacket->fragmentBase = 0;
  txPacket->fragmentsInTransit = 0;

  status = sendNextFragments(txPacket);

  if (status == EMBER_SUCCESS) {
    // Set the APS sequence number in the passed apsFrame.
    apsFrame->sequence = txPacket->sequence;
    emberAfAddToCurrentAppTasks(EMBER_AF_FRAGMENTATION_IN_PROGRESS);
  } else {
    txPacket->messageType = UNUSED_TX_PACKET_ENTRY;
  }

  *messageTag = messageTag(txPacket);

  return status;
}

bool emAfFragmentationMessageSent(EmberApsFrame *apsFrame,
                                  EmberStatus status)
{
  if (apsFrame->options & EMBER_APS_OPTION_FRAGMENT) {
    // If the outgoing APS frame is fragmented, we should always have a
    // a corresponding record in the txFragmentedPacket array.
    txFragmentedPacket *txPacket = txPacketLookUp(apsFrame);
    if (txPacket == NULL) {
      return true;
    }

    if (status == EMBER_SUCCESS) {
      txPacket->fragmentsInTransit--;
      if (txPacket->fragmentsInTransit == 0) {
        txPacket->fragmentBase += emberFragmentWindowSize;
        abortTransmission(txPacket, sendNextFragments(txPacket));
      }
    } else {
      abortTransmission(txPacket, status);
    }
    updateCurrentAppTasksForFragmentationState();
    return true;
  } else {
    return false;
  }
}

static EmberStatus sendNextFragments(txFragmentedPacket* txPacket)
{
  uint8_t i;
  uint16_t offset;

  emberAfCorePrintln("Sending fragment %d of %d",
                     txPacket->fragmentBase,
                     txPacket->fragmentCount);

  offset = txPacket->fragmentBase * txPacket->fragmentLen;

  // Send fragments until the window is full.
  for (i = txPacket->fragmentBase;
       i < txPacket->fragmentBase + emberFragmentWindowSize
       && i < txPacket->fragmentCount;
       i++) {
    EmberStatus status;

    // For a message requiring n fragments, the length of each of the first
    // n - 1 fragments is the maximum fragment size.  The length of the last
    // fragment is whatever is leftover.
    uint8_t fragmentLen = (offset + txPacket->fragmentLen < txPacket->bufLen
                           ? txPacket->fragmentLen
                           : txPacket->bufLen - offset);

    txPacket->apsFrame.groupId = HIGH_LOW_TO_INT(txPacket->fragmentCount, i);

    status = emAfPluginFragmentationSend(txPacket,
                                         i,
                                         fragmentLen,
                                         offset);
    if (status != EMBER_SUCCESS) {
      return status;
    }

    txPacket->fragmentsInTransit++;
    offset += fragmentLen;
  } // close inner for

  if (txPacket->fragmentsInTransit == 0) {
    emAfFragmentationMessageSentHandler(txPacket->messageType,
                                        txPacket->indexOrDestination,
                                        &txPacket->apsFrame,
                                        txPacket->bufferPtr,
                                        txPacket->bufLen,
                                        EMBER_SUCCESS,
                                        messageTag(txPacket));
    txPacket->messageType = UNUSED_TX_PACKET_ENTRY;
  } else if (freeOutgoingMessagePriorToFinalAck
             && txPacket->bufferPtr != NULL
             && offset >= txPacket->bufLen
             && emberFragmentWindowSize == 1
             && txPacketAwaitingFinalAck.messageType == UNUSED_TX_PACKET_ENTRY) {
    // Awaiting final fragment
    MEMMOVE(&txPacketAwaitingFinalAck, txPacket, sizeof(txFragmentedPacket));
    txPacketAwaitingFinalAck.bufferPtr = NULL;
    txPacketAwaitingFinalAck.bufLen = 0;
    txPacket->messageType = UNUSED_TX_PACKET_ENTRY;
  }

  return EMBER_SUCCESS;
}

static void abortTransmission(txFragmentedPacket *txPacket,
                              EmberStatus status)
{
  if (status != EMBER_SUCCESS && txPacket->messageType != UNUSED_TX_PACKET_ENTRY) {
    emAfFragmentationMessageSentHandler(txPacket->messageType,
                                        txPacket->indexOrDestination,
                                        &txPacket->apsFrame,
                                        txPacket->bufferPtr,
                                        txPacket->bufLen,
                                        status,
                                        messageTag(txPacket));
    txPacket->messageType = UNUSED_TX_PACKET_ENTRY;
  }
}

static txFragmentedPacket* getFreeTxPacketEntry(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    txFragmentedPacket *txPacket = &(txPackets[i]);
    if (txPacket->messageType == UNUSED_TX_PACKET_ENTRY) {
      txPacket->bufferPtr = txMessageBuffers[i];
      return txPacket;
    }
  }
  return NULL;
}

static txFragmentedPacket* txPacketLookUp(EmberApsFrame *apsFrame)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    txFragmentedPacket *txPacket = &(txPackets[i]);
    if (txPacket->messageType == UNUSED_TX_PACKET_ENTRY) {
      continue;
    }

    // Each node has a single source APS counter.
    if (apsFrame->sequence == txPacket->apsFrame.sequence) {
      return txPacket;
    }
  }

  if (txPacketAwaitingFinalAck.messageType != UNUSED_TX_PACKET_ENTRY
      && apsFrame->sequence == txPacketAwaitingFinalAck.apsFrame.sequence) {
    return &txPacketAwaitingFinalAck;
  }
  return NULL;
}

//------------------------------------------------------------------------------
// Receiving.

#define lowBitMask(n) ((1u << (n)) - 1)
static void setFragmentMask(rxFragmentedPacket *rxPacket);
static bool storeRxFragment(rxFragmentedPacket *rxPacket,
                            uint8_t fragment,
                            uint8_t *buffer,
                            uint16_t bufLen);
static void moveRxWindow(rxFragmentedPacket *rxPacket);
static rxFragmentedPacket* getFreeRxPacketEntry(void);
static rxFragmentedPacket* rxPacketLookUp(EmberApsFrame *apsFrame,
                                          EmberNodeId sender);

static rxFragmentedPacket rxPackets[EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS];

static void ageAllAckedRxPackets(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    if (rxPackets[i].status == EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_ACKED) {
      rxPackets[i].ackedPacketAge++;
    }
  }
}

static uint16_t retryTimeoutMs(EmberNodeId nodeId)
{
  EmberEUI64 eui64;
  uint16_t retryTimeoutMs = emberApsAckTimeoutMs;
  if (EMBER_SLEEPY_END_DEVICE <= emAfCurrentZigbeeProNetwork->nodeType) {
    retryTimeoutMs += emberMacIndirectTimeout;
  }
  if (emberLookupEui64ByNodeId(nodeId, eui64) == EMBER_SUCCESS
      && emberGetExtendedTimeout(eui64)) {
    retryTimeoutMs += emberMacIndirectTimeout;
  }
  return retryTimeoutMs;
}

bool emAfFragmentationIncomingMessage(EmberIncomingMessageType type,
                                      EmberApsFrame *apsFrame,
                                      EmberNodeId sender,
                                      uint8_t **buffer,
                                      uint16_t *bufLen)
{
  static bool rxWindowMoved = false;
  bool newFragment;
  uint8_t fragment;
  uint8_t mask;
  rxFragmentedPacket *rxPacket;
  EmberAfClusterCommand cmd;

  if (!(apsFrame->options & EMBER_APS_OPTION_FRAGMENT)) {
    return false;
  }

  assert(*bufLen <= MAX_INT8U_VALUE);

  rxPacket = rxPacketLookUp(apsFrame, sender);
  fragment = LOW_BYTE(apsFrame->groupId);

  if (artificiallyDropBlock(fragment)) {
    artificiallyDropBlockPrintln("Artificially dropping block %d", fragment);
    clearArtificiallyDropBlock();
    return true;
  }

  // First fragment for this packet, we need to set up a new entry.
  if (rxPacket == NULL) {
    rxPacket = getFreeRxPacketEntry();
    if (rxPacket == NULL || fragment >= emberFragmentWindowSize) {
      return true;
    }

    rxPacket->status = EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_IN_USE;
    rxPacket->fragmentSource = sender;
    rxPacket->fragmentSequenceNumber = apsFrame->sequence;
    rxPacket->fragmentBase = 0;
    rxPacket->windowFinger = 0;
    rxPacket->fragmentsReceived = 0;
    rxPacket->fragmentsExpected = 0xFF;
    rxPacket->fragmentLen = (uint8_t)(*bufLen);
    setFragmentMask(rxPacket);

    emberEventControlSetDelayMS(*(rxPacket->fragmentEventControl),
                                (retryTimeoutMs(sender)
                                 * ZIGBEE_APSC_MAX_TRANSMIT_RETRIES));
    emberAfAddToCurrentAppTasks(EMBER_AF_FRAGMENTATION_IN_PROGRESS);
  }

  // All fragments inside the rx window have been received and the incoming
  // fragment is outside the receiving window: let's move the rx window.
  if (rxPacket->fragmentMask == 0xFF
      && rxPacket->fragmentBase + emberFragmentWindowSize <= fragment) {
    moveRxWindow(rxPacket);
    setFragmentMask(rxPacket);
    rxWindowMoved = true;

    emberEventControlSetDelayMS(*(rxPacket->fragmentEventControl),
                                (retryTimeoutMs(sender)
                                 * ZIGBEE_APSC_MAX_TRANSMIT_RETRIES));
  }

  // Fragment outside the rx window.
  if (fragment < rxPacket->fragmentBase
      || fragment >= rxPacket->fragmentBase + emberFragmentWindowSize) {
    return true;
  } else { // Fragment inside the rx window.
    if (rxWindowMoved) {
      // We assume that the fragment length for the new rx window is the length
      // of the first fragment received inside the window. However, if the first
      // fragment received is the last fragment of the packet, we do not
      // consider it for setting the fragment length.
      if (fragment < rxPacket->fragmentsExpected - 1) {
        rxPacket->fragmentLen = (uint8_t)(*bufLen);
        rxWindowMoved = false;
      }
    } else {
      // We enforce that all the subsequent fragments (except for the last
      // fragment) inside the rx window have the same length as the first one.
      if (fragment < rxPacket->fragmentsExpected - 1
          && rxPacket->fragmentLen != (uint8_t)(*bufLen)) {
        goto kickout;
      }
    }
  }

  mask = 1 << (fragment % emberFragmentWindowSize);
  newFragment = !(mask & rxPacket->fragmentMask);

  // First fragment, setting the total number of expected fragments.
  if (fragment == 0) {
    rxPacket->fragmentsExpected = HIGH_BYTE(apsFrame->groupId);
    if (rxPacket->fragmentsExpected < emberFragmentWindowSize) {
      setFragmentMask(rxPacket);
    }
  }

  emberAfCorePrintln("Receiving fragment %d of %d", fragment, rxPacket->fragmentsExpected);

  // If it's a new fragment, try to buffer it
  // If the payload is too long, we wait to hear all fragments (and ack each
  // one) and then respond with an INSUFFICIENT_SPACE
  rxPacket->fragmentMask |= mask;
  if (newFragment) {
    rxPacket->fragmentsReceived++;
    if (!storeRxFragment(rxPacket, fragment, *buffer, *bufLen)
        && (EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_PAYLOAD_TOO_LARGE != rxPacket->status)) {
      goto kickout;
    }
  }

  if (fragment == rxPacket->fragmentsExpected - 1
      || (rxPacket->fragmentMask
          | lowBitMask(fragment % emberFragmentWindowSize)) == 0xFF) {
    emAfPluginFragmentationSendReply(sender,
                                     apsFrame,
                                     rxPacket);
  }

  // Received all the expected fragments.
  if (rxPacket->fragmentsReceived == rxPacket->fragmentsExpected) {
    uint8_t fragmentsInLastWindow =
      rxPacket->fragmentsExpected % emberFragmentWindowSize;
    if (fragmentsInLastWindow == 0) {
      fragmentsInLastWindow = emberFragmentWindowSize;
    }

    // Pass the reassembled packet only once to the application.
    if (rxPacket->status == EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_IN_USE) {
      //Age all acked packets first
      ageAllAckedRxPackets();
      // Mark the packet entry as acked.
      rxPacket->status = EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_ACKED;
      // Set the age of the new acked packet as the youngest one.
      rxPacket->ackedPacketAge = 0;
      // This library sends replies for all fragments, so, before passing on the
      // reassembled message, clear the retry bit to prevent the application
      // from sending a duplicate reply.
      apsFrame->options &= ~EMBER_APS_OPTION_RETRY;

      // The total size is the window finger + (n-1) full fragments + the last
      // fragment.
      *bufLen = rxPacket->windowFinger + rxPacket->lastfragmentLen
                + (fragmentsInLastWindow - 1) * rxPacket->fragmentLen;
      *buffer = rxPacket->buffer;

      updateCurrentAppTasksForFragmentationState();
      return false;
    } else if (rxPacket->status
               == EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_PAYLOAD_TOO_LARGE) {
      // Send a default response with INSUFFICIENT_SPACE
      *bufLen = rxPacket->windowFinger + rxPacket->lastfragmentLen
                + (fragmentsInLastWindow - 1) * rxPacket->fragmentLen;
      *buffer = rxPacket->buffer;
      emberAfProcessMessageIntoZclCmd(apsFrame,
                                      type,
                                      *buffer,
                                      *bufLen,
                                      sender,
                                      NULL, // inter-PAN fragments don't reach here
                                      &cmd);
      emberAfSendDefaultResponse(&cmd, EMBER_ZCL_STATUS_INSUFFICIENT_SPACE);

      // Finally, free the buffer
      emAfFragmentationAbortReception(rxPacket->fragmentEventControl);
    }
  }
  return true;

  kickout:
  emAfFragmentationAbortReception(rxPacket->fragmentEventControl);
  return true;
}

void emAfFragmentationAbortReception(EmberEventControl *control)
{
  uint8_t i;
  emberEventControlSetInactive(*control);

  for (i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    rxFragmentedPacket *rxPacket = &(rxPackets[i]);
    if (rxPacket->fragmentEventControl == control) {
      rxPacket->status = EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_AVAILABLE;
    }
  }
  updateCurrentAppTasksForFragmentationState();
}

static void setFragmentMask(rxFragmentedPacket *rxPacket)
{
  // Unused bits must be 1.
  uint8_t highestZeroBit = emberFragmentWindowSize;
  // If we are in the final window, there may be additional unused bits.
  if (rxPacket->fragmentsExpected
      < rxPacket->fragmentBase + emberFragmentWindowSize) {
    highestZeroBit = (rxPacket->fragmentsExpected % emberFragmentWindowSize);
  }
  rxPacket->fragmentMask = ~lowBitMask(highestZeroBit);
}

static bool storeRxFragment(rxFragmentedPacket *rxPacket,
                            uint8_t fragment,
                            uint8_t *buffer,
                            uint16_t bufLen)
{
  uint16_t index = rxPacket->windowFinger;

  if (index + bufLen > EMBER_AF_PLUGIN_FRAGMENTATION_BUFFER_SIZE) {
    rxPacket->status = EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_PAYLOAD_TOO_LARGE;
    return false;
  }

  index += (fragment - rxPacket->fragmentBase) * rxPacket->fragmentLen;
  MEMMOVE(rxPacket->buffer + index, buffer, bufLen);

  // If this is the last fragment of the packet, store its length.
  if (fragment == rxPacket->fragmentsExpected - 1) {
    rxPacket->lastfragmentLen = (uint8_t)bufLen;
  }

  return true;
}

static void moveRxWindow(rxFragmentedPacket *rxPacket)
{
  rxPacket->fragmentBase += emberFragmentWindowSize;
  rxPacket->windowFinger += emberFragmentWindowSize * rxPacket->fragmentLen;
}

static rxFragmentedPacket* getFreeRxPacketEntry(void)
{
  uint8_t i;
  rxFragmentedPacket* ackedPacket = NULL;

  // Available entries first.
  for (i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    rxFragmentedPacket *rxPacket = &(rxPackets[i]);
    if (rxPacket->status == EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_AVAILABLE) {
      return rxPacket;
    }
  }

  // Acked packets: Look for the oldest one.
  for (i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    rxFragmentedPacket *rxPacket = &(rxPackets[i]);
    if (rxPacket->status == EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_ACKED) {
      if (ackedPacket == NULL
          || ackedPacket->ackedPacketAge < rxPacket->ackedPacketAge) {
        ackedPacket = rxPacket;
      }
    }
  }

  return ackedPacket;
}

static rxFragmentedPacket* rxPacketLookUp(EmberApsFrame *apsFrame,
                                          EmberNodeId sender)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    rxFragmentedPacket *rxPacket = &(rxPackets[i]);
    if (rxPacket->status == EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_AVAILABLE) {
      continue;
    }
    // Each packet is univocally identified by the pair (node id, seq. number).
    if (apsFrame->sequence == rxPacket->fragmentSequenceNumber
        && sender == rxPacket->fragmentSource) {
      return rxPacket;
    }
  }
  return NULL;
}

//------------------------------------------------------------------------------
// Initialization
void emberAfPluginFragmentationInitCallback(void)
{
  uint8_t i;
  emAfPluginFragmentationPlatformInitCallback();

  for (i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    rxPackets[i].status = EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_AVAILABLE;
    rxPackets[i].fragmentEventControl = &(emAfFragmentationEvents[i]);
  }

  for (i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    txPackets[i].messageType = 0xFF;
  }
}

//------------------------------------------------------------------------------
// Utility

// Clear app task frag bit only if neither tx nor rx frag is in progress.
static void updateCurrentAppTasksForFragmentationState(void)
{
  uint8_t i;
  bool fragmenting;

  fragmenting = false;

  if (txPacketAwaitingFinalAck.messageType != UNUSED_TX_PACKET_ENTRY) {
    fragmenting = true;
  }

  if (!fragmenting) {
    for (i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
      if (txPackets[i].messageType != UNUSED_TX_PACKET_ENTRY) {
        fragmenting = true;
        break;
      }
    }
  }

  if (!fragmenting) {
    for (i = 0; i < EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
      if (rxPackets[i].status == EMBER_AF_PLUGIN_FRAGMENTATION_RX_PACKET_IN_USE) {
        fragmenting = true;
        break;
      }
    }
  }

  if (!fragmenting) {
    emberAfRemoveFromCurrentAppTasks(EMBER_AF_FRAGMENTATION_IN_PROGRESS);
  }
}
