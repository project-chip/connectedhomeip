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
 * @brief Unit test for fragmentation library.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/test/script/afv2-scripted.h"
#include <stdlib.h>

#include "af-types.h"
#include "fragmentation.h"

#include "stack/framework/zigbee-packet-header.h"
#include "app/util/ezsp/ezsp-enum.h"

#define EMBER_AF_PLUGIN_FRAGMENTATION_MAX_INCOMING_PACKETS 2
#define EMBER_AF_PLUGIN_FRAGMENTATION_MAX_OUTGOING_PACKETS 2

//------------------------------------------------------------------------------
// Global Variables.

static EmberNodeId remoteNodeId = 0xB1B2;
static EmberApsFrame apsFrame = {
  0xABCD,         // profile ID
  0x5040,         // cluster ID
  0x51,           // source endpoint
  0x52,           // destination endpoint
  EMBER_APS_OPTION_NONE,
  0x5340,         // group ID
  0x54            // sequence
};

static uint8_t fragmentData[10];
static bool setSourceRoute = false;
static uint8_t maxApsPayloadLength = 70;

EmberEventData fragmentationEvents[] = {
  EMBER_AF_FRAGMENTATION_EVENTS
  { NULL, NULL }
};

static uint32_t currentAppTasks = 0;

//------------------------------------------------------------------------------
// Forward Declarations.

Parcel *parcelFragment(Parcel *wholeMessage,
                       uint8_t startIndex,
                       uint8_t length);
Parcel *makeIncrementingParcel(uint8_t length);
void callFragmentInit(void);
void callFragmentTick(void);
EmberStatus callSendFragmentedMessage(EmberOutgoingMessageType type,
                                      uint16_t indexOrDestination,
                                      EmberApsFrame *apsFrame,
                                      Parcel *payload,
                                      uint8_t maxFragmentSize,
                                      bool sourceRoute);
bool callIsOutgoingFragment(EmberApsFrame *apsFrame,
                            EmberMessageBuffer *messageLoc,
                            EmberStatus status);
bool callIsIncomingFragment(EmberApsFrame *apsFrame,
                            EmberNodeId sender,
                            Parcel *payloadBefore,
                            Parcel *payloadAfter);
void callSetWindowSize(uint8_t windowSize);
long callIsFragInProgress(void);

void emberAfPluginFragmentationInitCallback(void);
void emberAfPluginFragmentationNcpInitCallback(void);
void emberFragmentTick(void);

//------------------------------------------------------------------------------
// Macros For Test Scripts.
#define initWindowSize(windowSize)     \
  do {                                 \
    addSimpleAction("FragmentInit",    \
                    callFragmentInit,  \
                    0);                \
    addSimpleAction("SetWindowSize",   \
                    callSetWindowSize, \
                    1,                 \
                    windowSize);       \
  } while (0)

#define tick()                        \
  do {                                \
    addSimpleAction("FragmentTick",   \
                    callFragmentTick, \
                    0);               \
  } while (0)

#define sendFragmented(wholeMessage, fragmentSize, sourceRoute) \
  do {                                                          \
    addFunctionCheck("SendFragmentedMessage",                   \
                     EMBER_SUCCESS,                             \
                     callSendFragmentedMessage,                 \
                     6,                                         \
                     EMBER_OUTGOING_DIRECT,                     \
                     remoteNodeId,                              \
                     &apsFrame,                                 \
                     wholeMessage,                              \
                     fragmentSize,                              \
                     sourceRoute);                              \
  } while (0)

#define checkSend(messageFragment, totalTxBlocks, blockNumber, data,           \
                  sourceRoute)                                                 \
  do {                                                                         \
    EmberApsFrame *newFrame = (EmberApsFrame *) malloc(sizeof(EmberApsFrame)); \
    memcpy(newFrame, &apsFrame, sizeof(EmberApsFrame));                        \
    newFrame->options = EMBER_APS_OPTION_RETRY | EMBER_APS_OPTION_FRAGMENT;    \
    newFrame->groupId = HIGH_LOW_TO_INT(totalTxBlocks, blockNumber);           \
    addSimpleCheck("emberSendUnicast",                                         \
                   "iipp!",                                                    \
                   EMBER_OUTGOING_DIRECT,                                      \
                   remoteNodeId,                                               \
                   apsStructParcel(newFrame),                                  \
                   messageFragment,                                            \
                   data);                                                      \
  } while (0)

#define outgoingFragment(data, status)                                         \
  do {                                                                         \
    EmberApsFrame *newFrame = (EmberApsFrame *) malloc(sizeof(EmberApsFrame)); \
    memcpy(newFrame, &apsFrame, sizeof(EmberApsFrame));                        \
    newFrame->options = EMBER_APS_OPTION_RETRY | EMBER_APS_OPTION_FRAGMENT;    \
    addFunctionCheck("IsOutgoingFragment",                                     \
                     true,                                                     \
                     callIsOutgoingFragment,                                   \
                     3,                                                        \
                     newFrame,                                                 \
                     data,                                                     \
                     status);                                                  \
  } while (0)

#define fragmentedMessageSent(status)                     \
  do {                                                    \
    addSimpleCheck("emAfFragmentationMessageSentHandler", \
                   "i",                                   \
                   status);                               \
  } while (0)

#define incomingFragment(payloadBefore, totalTxBlocks,                         \
                         blockNumber, payloadAfter, newMessage)                \
  do {                                                                         \
    EmberApsFrame *newFrame = (EmberApsFrame *) malloc(sizeof(EmberApsFrame)); \
    memcpy(newFrame, &apsFrame, sizeof(EmberApsFrame));                        \
    newFrame->options = EMBER_APS_OPTION_RETRY | EMBER_APS_OPTION_FRAGMENT;    \
    newFrame->groupId = HIGH_LOW_TO_INT(totalTxBlocks, blockNumber);           \
    addFunctionCheck("IsIncomingFragment",                                     \
                     payloadAfter == NULL,                                     \
                     callIsIncomingFragment,                                   \
                     4,                                                        \
                     newFrame,                                                 \
                     remoteNodeId,                                             \
                     payloadBefore,                                            \
                     payloadAfter);                                            \
  } while (0)

#define checkSendReply(blockMask, rxWindowBase)                                \
  do {                                                                         \
    EmberApsFrame *newFrame = (EmberApsFrame *) malloc(sizeof(EmberApsFrame)); \
    memcpy(newFrame, &apsFrame, sizeof(EmberApsFrame));                        \
    newFrame->options = EMBER_APS_OPTION_RETRY | EMBER_APS_OPTION_FRAGMENT;    \
    newFrame->groupId = HIGH_LOW_TO_INT(blockMask, rxWindowBase);              \
    addSimpleCheck("emberSetReplyFragmentData",                                \
                   "i",                                                        \
                   newFrame->groupId);                                         \
    addSimpleCheck("emberSendReply",                                           \
                   "ip",                                                       \
                   newFrame->clusterId,                                        \
                   makeMessage(""));                                           \
  } while (0)

#define expectFragInProgress(inProgress)   \
  do {                                     \
    addFunctionCheck("FragInProgress",     \
                     inProgress,           \
                     callIsFragInProgress, \
                     0);                   \
  } while (0)

//------------------------------------------------------------------------------
// Test scripts.

// Send a message in two fragments.
static void simpleSend(void)
{
  Parcel *wholeMessage = makeIncrementingParcel(40);
  setFreeBufferBenchmark();
  initWindowSize(3);
  expectFragInProgress(false);
  sendFragmented(wholeMessage, 20, false);
  checkSend(parcelFragment(wholeMessage, 0, 20),
            2, 0, fragmentData, false);
  checkSend(parcelFragment(wholeMessage, 20, 20),
            2, 1, fragmentData + 1, false);
  expectFragInProgress(true);
  addRunAction(10);
  outgoingFragment(fragmentData, EMBER_SUCCESS);
  expectFragInProgress(true);
  outgoingFragment(fragmentData + 1, EMBER_SUCCESS);
  fragmentedMessageSent(EMBER_SUCCESS);
  expectFragInProgress(false);
  addRunAction(100);
  runScript();
  assert(testFreeBuffers(0));
}

// Send a message that fills multiple windows.
static void sendWindow(void)
{
  Parcel *wholeMessage = makeIncrementingParcel(30);
  setFreeBufferBenchmark();
  initWindowSize(1);
  sendFragmented(wholeMessage, 10, false);
  checkSend(parcelFragment(wholeMessage, 0, 10),
            3, 0, fragmentData, false);
  addRunAction(10);
  outgoingFragment(fragmentData, EMBER_SUCCESS);
  // Need to use fragmentData + 1 because callIsOutgoingFragment calls
  // emberFragmentMessageSent (which uses fragmentData + 1) before it
  // releases fragmentData.
  checkSend(parcelFragment(wholeMessage, 10, 10),
            3, 1, fragmentData + 1, false);
  addRunAction(10);
  outgoingFragment(fragmentData + 1, EMBER_SUCCESS);
  checkSend(parcelFragment(wholeMessage, 20, 10),
            3, 2, fragmentData, false);
  addRunAction(10);
  outgoingFragment(fragmentData, EMBER_SUCCESS);
  fragmentedMessageSent(EMBER_SUCCESS);
  addRunAction(100);
  runScript();
  assert(testFreeBuffers(0));
}

// Check that the mask in the ACK is 0xFF with window 3, fragments 2.
static void ackMask1(void)
{
  Parcel *wholeMessage = makeIncrementingParcel(40);
  setFreeBufferBenchmark();
  initWindowSize(3);
  expectFragInProgress(false);
  incomingFragment(parcelFragment(wholeMessage, 0, 20),
                   2, 0, NULL, true);
  expectFragInProgress(true);
  addRunAction(10);
  incomingFragment(parcelFragment(wholeMessage, 20, 20),
                   2, 1, wholeMessage, false);
  checkSendReply(0xFF, 0);
  expectFragInProgress(false);
  addRunAction(100);
  runScript();
  assert(testFreeBuffers(0));
}

// Check that the mask in the ACK is 0xFF with window 4, fragments 5.
static void ackMask2(void)
{
  Parcel *wholeMessage = makeIncrementingParcel(45);
  setFreeBufferBenchmark();
  initWindowSize(4);
  expectFragInProgress(false);
  incomingFragment(parcelFragment(wholeMessage, 0, 10),
                   5, 0, NULL, true);
  expectFragInProgress(true);
  addRunAction(10);
  incomingFragment(parcelFragment(wholeMessage, 10, 10),
                   5, 1, NULL, false);
  expectFragInProgress(true);
  addRunAction(10);
  incomingFragment(parcelFragment(wholeMessage, 20, 10),
                   5, 2, NULL, false);
  expectFragInProgress(true);
  addRunAction(10);
  incomingFragment(parcelFragment(wholeMessage, 30, 10),
                   5, 3, NULL, false);
  checkSendReply(0xFF, 0);
  expectFragInProgress(true);
  addRunAction(10);
  incomingFragment(parcelFragment(wholeMessage, 40, 5),
                   5, 4, wholeMessage, false);
  checkSendReply(0xFF, 4);
  expectFragInProgress(false);
  addRunAction(100);
  runScript();
  assert(testFreeBuffers(0));
}

// Check that ACKs are sent for retries of the last fragment in the window.
static void ackRetries(void)
{
  Parcel *wholeMessage = makeIncrementingParcel(100);
  setFreeBufferBenchmark();
  initWindowSize(1);
  expectFragInProgress(false);

  incomingFragment(parcelFragment(wholeMessage, 0, 40),
                   3, 0, NULL, true);
  checkSendReply(0xFF, 0);
  expectFragInProgress(true);
  addRunAction(10);
  incomingFragment(parcelFragment(wholeMessage, 0, 40),
                   3, 0, NULL, false);
  checkSendReply(0xFF, 0);
  expectFragInProgress(true);
  addRunAction(10);

  incomingFragment(parcelFragment(wholeMessage, 40, 40),
                   3, 1, NULL, false);
  checkSendReply(0xFF, 1);
  expectFragInProgress(true);
  addRunAction(10);
  incomingFragment(parcelFragment(wholeMessage, 40, 40),
                   3, 1, NULL, false);
  checkSendReply(0xFF, 1);
  expectFragInProgress(true);
  addRunAction(10);

  incomingFragment(parcelFragment(wholeMessage, 80, 20),
                   3, 2, wholeMessage, false);
  checkSendReply(0xFF, 2);
  expectFragInProgress(false);
  addRunAction(10);
  incomingFragment(parcelFragment(wholeMessage, 80, 20),
                   3, 2, NULL, false);

  // Send the ACK for an already acked packet
  checkSendReply(0xFF, 2);
  expectFragInProgress(false);
  addRunAction(10);

  addRunAction(100);
  runScript();
  assert(testFreeBuffers(0));
}

// Check that the ACK is sent when the window is full, even if an earlier
// fragment was missed.
static void ackMissingFragment(void)
{
  Parcel *wholeMessage = makeIncrementingParcel(40);
  setFreeBufferBenchmark();
  initWindowSize(3);
  expectFragInProgress(false);

  incomingFragment(parcelFragment(wholeMessage, 0, 10),
                   4, 0, NULL, true);
  expectFragInProgress(true);
  addRunAction(10);
  // Miss second fragment.
  //incomingFragment(parcelFragment(wholeMessage, 10, 10),
  //                 4, 1, NULL, false);
  addRunAction(10);
  incomingFragment(parcelFragment(wholeMessage, 20, 10),
                   4, 2, NULL, false);
  checkSendReply(0xFF - BIT(1), 0);
  expectFragInProgress(true);
  addRunAction(10);
  incomingFragment(parcelFragment(wholeMessage, 20, 10),
                   4, 2, NULL, false);
  checkSendReply(0xFF - BIT(1), 0);
  expectFragInProgress(true);
  addRunAction(10);

  incomingFragment(parcelFragment(wholeMessage, 0, 10),
                   4, 0, NULL, false);
  expectFragInProgress(true);
  addRunAction(10);
  incomingFragment(parcelFragment(wholeMessage, 10, 10),
                   4, 1, NULL, false);
  checkSendReply(0xFF, 0);
  expectFragInProgress(true);
  addRunAction(10);
  incomingFragment(parcelFragment(wholeMessage, 20, 10),
                   4, 2, NULL, false);
  checkSendReply(0xFF, 0);
  expectFragInProgress(true);
  addRunAction(10);

  incomingFragment(parcelFragment(wholeMessage, 30, 10),
                   4, 3, wholeMessage, false);
  checkSendReply(0xFF, 3);
  expectFragInProgress(false);

  addRunAction(100);
  runScript();
  assert(testFreeBuffers(0));
}

// Check that reception is aborted if a fragment takes too long to arrive.
static void timeout(void)
{
  Parcel *wholeMessage = makeIncrementingParcel(20);
  setFreeBufferBenchmark();
  initWindowSize(3);
  expectFragInProgress(false);

  // Successful reception and reassmebly of two frags.
  incomingFragment(parcelFragment(wholeMessage, 0, 10),
                   2, 0, NULL, true);
  expectFragInProgress(true);
  addRunAction(emberApsAckTimeoutMs * 2);
  tick();
  expectFragInProgress(true);
  incomingFragment(parcelFragment(wholeMessage, 10, 10),
                   2, 1, wholeMessage, false);
  checkSendReply(0xFF, 0);
  expectFragInProgress(false);

  // This frag is ignored as a duplicate (APS sequence number matches
  // prior reassembled packet, which currently is in ACKED state).
  incomingFragment(parcelFragment(wholeMessage, 0, 10),
                   2, 0, NULL, true);
  expectFragInProgress(false);

  // Let time elapse and abort reception.
  addRunAction(emberApsAckTimeoutMs * 4);
  tick();
  expectFragInProgress(false);

  // Reception has been aborted. This fragment is treated as a new message.
  incomingFragment(parcelFragment(wholeMessage, 10, 10),
                   2, 1, NULL, true);
  expectFragInProgress(true);

  // Wait for reception to be aborted again so buffer is freed.
  addRunAction(emberApsAckTimeoutMs * 4);
  tick();
  expectFragInProgress(false);

  addRunAction(100);
  runScript();
  assert(testFreeBuffers(0));
}

// Check that the correct source route is used.
static void sourceRoute(void)
{
  Parcel *wholeMessage = makeIncrementingParcel(40);
  setFreeBufferBenchmark();
  initWindowSize(3);
  expectFragInProgress(false);
  sendFragmented(wholeMessage, 20, true);
  checkSend(parcelFragment(wholeMessage, 0, 20),
            2, 0, fragmentData, true);
  checkSend(parcelFragment(wholeMessage, 20, 20),
            2, 1, fragmentData + 1, true);
  expectFragInProgress(true);
  addRunAction(10);
  outgoingFragment(fragmentData, EMBER_SUCCESS);
  expectFragInProgress(true);
  outgoingFragment(fragmentData + 1, EMBER_SUCCESS);
  fragmentedMessageSent(EMBER_SUCCESS);
  expectFragInProgress(false);
  addRunAction(100);
  runScript();
  assert(testFreeBuffers(0));
}

// Expect FRAG_IN_PROGRESS is true throughout overlapped Tx-then-Rx.
static void txRxOverlap(void)
{
  Parcel *txMessage = makeIncrementingParcel(40);
  Parcel *rxMessage = makeIncrementingParcel(40);
  setFreeBufferBenchmark();
  initWindowSize(3);
  expectFragInProgress(false);

  // Rx fragment 1 - Rx fragmentation begins
  incomingFragment(parcelFragment(rxMessage, 0, 20),
                   2, 0, NULL, true);
  expectFragInProgress(true);
  addRunAction(10);

  // Tx fragments 1 and 2 - Tx fragmentation begins
  sendFragmented(txMessage, 20, false);
  checkSend(parcelFragment(txMessage, 0, 20),
            2, 0, fragmentData, false);
  checkSend(parcelFragment(txMessage, 20, 20),
            2, 1, fragmentData + 1, false);
  expectFragInProgress(true);
  addRunAction(10);

  // Rx fragment 2 - Rx fragmentation completes
  incomingFragment(parcelFragment(rxMessage, 20, 20),
                   2, 1, rxMessage, false);
  checkSendReply(0xFF, 0);
  expectFragInProgress(true);
  addRunAction(10);

  // Tx acks - Tx fragmentation completes
  outgoingFragment(fragmentData, EMBER_SUCCESS);
  expectFragInProgress(true);
  outgoingFragment(fragmentData + 1, EMBER_SUCCESS);
  fragmentedMessageSent(EMBER_SUCCESS);
  expectFragInProgress(false);
  addRunAction(100);

  runScript();
  assert(testFreeBuffers(0));
}

// Expect FRAG_IN_PROGRESS is true throughout overlapped Rx-then-Tx.
static void rxTxOverlap(void)
{
  Parcel *txMessage = makeIncrementingParcel(40);
  Parcel *rxMessage = makeIncrementingParcel(40);
  setFreeBufferBenchmark();
  initWindowSize(3);
  expectFragInProgress(false);

  // Tx fragments 1 and 2 - Tx fragmentation begins
  sendFragmented(txMessage, 20, false);
  checkSend(parcelFragment(txMessage, 0, 20),
            2, 0, fragmentData, false);
  checkSend(parcelFragment(txMessage, 20, 20),
            2, 1, fragmentData + 1, false);
  expectFragInProgress(true);
  addRunAction(10);

  // Rx fragment 1 - Rx fragmentation begins
  incomingFragment(parcelFragment(rxMessage, 0, 20),
                   2, 0, NULL, true);
  expectFragInProgress(true);
  addRunAction(10);

  // Tx acks - Tx fragmentation completes
  outgoingFragment(fragmentData, EMBER_SUCCESS);
  expectFragInProgress(true);
  outgoingFragment(fragmentData + 1, EMBER_SUCCESS);
  fragmentedMessageSent(EMBER_SUCCESS);
  expectFragInProgress(true);
  addRunAction(10);

  // Rx fragment 2 - Rx fragmentation completes
  incomingFragment(parcelFragment(rxMessage, 20, 20),
                   2, 1, rxMessage, false);
  checkSendReply(0xFF, 0);
  expectFragInProgress(false);

  addRunAction(100);

  runScript();
  assert(testFreeBuffers(0));
}

//------------------------------------------------------------------------------
// Utilities.

Parcel *apsStructParcel(EmberApsFrame *apsStruct)
{
  return makeMessage("<2<2<211<21",
                     apsStruct->options,
                     apsStruct->profileId,
                     apsStruct->clusterId,
                     apsStruct->sourceEndpoint,
                     apsStruct->destinationEndpoint,
                     apsStruct->groupId,
                     apsStruct->sequence);
}

Parcel *parcelFragment(Parcel *wholeMessage,
                       uint8_t startIndex,
                       uint8_t length)
{
  return makeMessage("s", wholeMessage->contents + startIndex, length);
}

Parcel *makeIncrementingParcel(uint8_t length)
{
  Parcel *result = makeParcel(length);
  uint8_t i;
  for (i = 0; i < length; i++) {
    result->contents[i] = i;
  }
  return result;
}

//------------------------------------------------------------------------------
// Callback Actions.

void callFragmentInit(void)
{
  emberAfPluginFragmentationInitCallback();
  currentAppTasks = 0;
}

void callFragmentTick(void)
{
  emberFragmentTick();
}

EmberStatus callSendFragmentedMessage(EmberOutgoingMessageType type,
                                      uint16_t indexOrDestination,
                                      EmberApsFrame *apsFrame,
                                      Parcel *payload,
                                      uint8_t maxFragmentSize,
                                      bool sourceRoute)
{
  setSourceRoute = sourceRoute;
  EmberStatus status;
  uint8_t oldMaxApsPayloadLength = maxApsPayloadLength;
  uint8_t messageTag;

  maxApsPayloadLength = maxFragmentSize;

  status = emAfFragmentationSendUnicast(type,
                                        indexOrDestination,
                                        apsFrame,
                                        payload->contents,
                                        payload->length,
                                        &messageTag);

  maxApsPayloadLength = oldMaxApsPayloadLength;

  return status;
}

bool callIsOutgoingFragment(EmberApsFrame *apsFrame,
                            EmberMessageBuffer *messageLoc,
                            EmberStatus status)
{
  bool result = emAfFragmentationMessageSent(apsFrame, status);

  return result;
}

bool callIsIncomingFragment(EmberApsFrame *apsFrame,
                            EmberNodeId sender,
                            Parcel *payloadBefore,
                            Parcel *payloadAfter)
{
  bool result;
  uint16_t payloadLength = payloadBefore->length;
  uint8_t* payloadBuffer = (uint8_t*)malloc(sizeof(uint8_t) * payloadBefore->length);

  uint16_t i;
  for (i = 0; i < payloadLength; i++) {
    payloadBuffer[i] = payloadBefore->contents[i];
  }

  result = emAfFragmentationIncomingMessage(EMBER_INCOMING_UNICAST,
                                            apsFrame,
                                            sender,
                                            &payloadBuffer,
                                            &payloadLength);

  Parcel *actualPayloadAfter = makeParcel(payloadLength);
  for (i = 0; i < payloadLength; i++) {
    actualPayloadAfter->contents[i] = payloadBuffer[i];
  }

  if (payloadAfter != NULL
      && !sameParcel(actualPayloadAfter, payloadAfter)) {
    fprintf(stderr, "\n[wrong payload\n have [");
    printParcelBytes(actualPayloadAfter);
    fprintf(stderr, "]\n want [");
    printParcelBytes(payloadAfter);
    fprintf(stderr, "]]\n");
    assert(false);
  }

  return result;
}

void callSetWindowSize(uint8_t windowSize)
{
  emberFragmentWindowSize = windowSize;
}

long callIsFragInProgress(void)
{
  return ((READBITS(currentAppTasks,
                    EMBER_AF_FRAGMENTATION_IN_PROGRESS)
           == EMBER_AF_FRAGMENTATION_IN_PROGRESS)
          ? (long)true
          : (long)false);
}

//------------------------------------------------------------------------------
// Stack Stubs.

EmberStatus emberSendUnicast(EmberOutgoingMessageType type,
                             uint16_t indexOrDestination,
                             EmberApsFrame *apsFrame,
                             EmberMessageBuffer message)
{
  functionCallCheck("emberSendUnicast",
                    "iipp",
                    type,
                    indexOrDestination,
                    apsStructParcel(apsFrame),
                    bufferToParcel(message));

  return EMBER_SUCCESS;
}

void emberSetReplyFragmentData(uint16_t fragmentData)
{
  functionCallCheck("emberSetReplyFragmentData", "i", fragmentData);
}

EmberStatus emberSendReply(uint16_t clusterId, EmberMessageBuffer reply)
{
  functionCallCheck("emberSendReply", "ip", clusterId, bufferToParcel(reply));
  return EMBER_SUCCESS;
}

EmberNodeId emberGetSender(void)
{
  long *values = functionCallCheck("emberGetSender", "");
  EmberNodeId sender = values[0];
  return sender;
}

//------------------------------------------------------------------------------
// Application Stubs.

void emberAfAddToCurrentAppTasksCallback(EmberAfApplicationTask tasks)
{
  currentAppTasks |= tasks;
}

void emberAfRemoveFromCurrentAppTasksCallback(EmberAfApplicationTask tasks)
{
  currentAppTasks &= (~tasks);
}

void emAfFragmentationMessageSentHandler(EmberOutgoingMessageType type,
                                         uint16_t indexOrDestination,
                                         EmberApsFrame *apsFrame,
                                         uint8_t *buffer,
                                         uint16_t bufLen,
                                         EmberStatus status,
                                         uint8_t messageTag)
{
  functionCallCheck("emAfFragmentationMessageSentHandler", "i", status);
}

void ezspFragmentMessageSentHandler(EmberStatus status)
{
  functionCallCheck("ezspFragmentMessageSentHandler", "i", status);
}

void emberFragmentTick(void)
{
  emberRunEvents(fragmentationEvents);
}

bool emberAfProcessMessageIntoZclCmd(EmberApsFrame* apsFrame,
                                     EmberIncomingMessageType type,
                                     uint8_t* message,
                                     uint16_t messageLength,
                                     EmberNodeId source,
                                     InterPanHeader* interPanHeader,
                                     EmberAfClusterCommand* returnCmd)
{
  return true;
}

EmberStatus emberAfSendDefaultResponse(const EmberAfClusterCommand *cmd,
                                       EmberAfStatus status)
{
  return EMBER_SUCCESS;
}

//------------------------------------------------------------------------------

uint32_t emberUnsignedCommandArgument(uint8_t argNum)
{
  return 0;
}

uint8_t emberAfMaximumApsPayloadLength(EmberOutgoingMessageType type,
                                       uint16_t indexOrDestination,
                                       EmberApsFrame *apsFrame)
{
  return maxApsPayloadLength;
}

EmAfZigbeeProNetwork zigbeeProNetwork = {
  EMBER_ROUTER,
  EMBER_AF_SECURITY_PROFILE_SE_FULL,
};
const EmAfZigbeeProNetwork *emAfCurrentZigbeeProNetwork = &zigbeeProNetwork;

EmberStatus emberLookupEui64ByNodeId(EmberNodeId nodeId, EmberEUI64 eui64Return)
{
  return EMBER_SUCCESS;
}

bool emberGetExtendedTimeout(EmberEUI64 eui64)
{
  return false;
}

//------------------------------------------------------------------------------

static Test tests[] =
{
  { "simple send", simpleSend },
  { "send window", sendWindow },
  { "ACK mask 1", ackMask1 },
  { "ACK mask 2", ackMask2 },
  { "ACK retries", ackRetries },
  { "ACK missing fragments", ackMissingFragment },
  { "timeout", timeout },
  { "source route", sourceRoute },
  { "tx/rx overlap", txRxOverlap },
  { "rx/tx overlap", rxTxOverlap },
  { NULL, NULL }                 // terminator
};

int main(int argc, char **argv)
{
  uint8_t i = 0;
  if (argc == 2 && strcmp(argv[1], "--debug") == 0) {
    scriptDebug = true;
  }
  while (tests[i].name != NULL) {
    fprintf(stderr, "[Testing %s ", tests[i].name);
    tests[i].test();
    fprintf(stderr, " done]\n");
    i++;
  }
  return 0;
}

// -----------------------------------------------------------------------------

void scriptTickCallback(void)
{
}
