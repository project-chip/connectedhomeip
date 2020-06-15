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
 * @brief Common test code and stubs for unit tests of the Afv2 code.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "callback.h"

#include "app/framework/test/test-framework.h"
#include "plugin/serial/ember-printf-convert.h"

#include "app/framework/util/af-main.h"

#include <stdlib.h>

//------------------------------------------------------------------------------
// Globals

#define MAX_DATA_VERIFY_LENGTH 100

bool giveEuiLookupResponse = false;
EmberEUI64 euiLookupResponse;

EmberEUI64 localEui64;

static bool expectDefaultResponse = false;
static uint8_t defaultResponseStatus = 0;
bool expectIeeeRequest = false;

AttributeCallbackFunction* attributeCallbackFunction = NULL;

const EmberEUI64 emberAfNullEui64 = { 0, 0, 0, 0, 0, 0, 0, 0 };

uint16_t emberAfPrintActiveArea = 0;

uint8_t emberAfResponseType = ZCL_UTIL_RESP_NORMAL;

typedef struct {
  EmberNodeId destination;
  EmberApsFrame apsFrame;
  uint16_t length;
  uint8_t* message;
  EmberAfMessageSentFunction callback;
  bool sentMessage;
} MessageToSend;

MessageToSend messageToSend = {
  EMBER_NULL_NODE_ID,
  {
    0, // profile ID
    0, // cluster ID,
    0, // source EP
    0, // dest EP
    0, // options
    0, // group ID
    0, // sequence
  },
  0,
  NULL,
  NULL,
  false,
};

//#define EMBER_AF_RESPONSE_BUFFER_LEN 100
uint8_t  appResponseData[EMBER_AF_RESPONSE_BUFFER_LEN];
uint16_t appResponseLength = 0;

EmberApsFrame emberAfApsResponseFrame;
EmberNodeId emberAfResponseDestination = EMBER_NULL_NODE_ID;

//------------------------------------------------------------------------------

void emEmberTestSetExternalBuffer(void)
{
  emberAfSetExternalBuffer(appResponseData,
                           EMBER_AF_RESPONSE_BUFFER_LEN,
                           &appResponseLength,
                           &emberAfApsResponseFrame);
}

void testAfSetNetworkState(EmberNetworkStatus networkState)
{
  testFrameworkNetworkState = networkState;
}

EmberAfStatus emberAfWriteAttribute(uint8_t endpoint,
                                    EmberAfClusterId cluster,
                                    EmberAfAttributeId attributeID,
                                    uint8_t mask,
                                    uint8_t* dataPtr,
                                    EmberAfAttributeType dataType)
{
  if (attributeCallbackFunction == NULL) {
    return EMBER_ZCL_STATUS_SUCCESS;
  }
  AttributeCallbackData data = {
    false,   // read?
    endpoint,
    cluster,
    attributeID,
    dataPtr,
    0,             // max length
    &dataType,
  };
  return attributeCallbackFunction(&data);
}

EmberAfStatus emberAfReadAttribute(uint8_t endpoint,
                                   EmberAfClusterId cluster,
                                   EmberAfAttributeId attributeID,
                                   uint8_t mask,
                                   uint8_t *dataPtr,
                                   uint8_t readLength,
                                   EmberAfAttributeType *dataType)
{
  if (attributeCallbackFunction == NULL) {
    return EMBER_ZCL_STATUS_SUCCESS;
  }
  AttributeCallbackData data = {
    true,   // read?
    endpoint,
    cluster,
    attributeID,
    dataPtr,
    readLength,
    dataType,
  };
  return attributeCallbackFunction(&data);
}

EmberStatus emberAfScheduleTickExtended(uint8_t endpoint,
                                        EmberAfClusterId clusterId,
                                        bool isClient,
                                        uint32_t delayMs,
                                        EmberAfEventPollControl pollControl,
                                        EmberAfEventSleepControl sleepControl)
{
  debug("Tick scheduled, ep: %d, cluster: 0x%04X, client: %s, time: %d ms\n",
        endpoint,
        clusterId,
        (isClient ? "yes" : "no"),
        delayMs);
  return EMBER_SUCCESS;
}

EmberStatus emberAfScheduleClusterTick(uint8_t endpoint,
                                       EmberAfClusterId clusterId,
                                       bool isClient,
                                       uint32_t delayMs,
                                       EmberAfEventSleepControl sleepControl)
{
  return emberAfScheduleTickExtended(endpoint,
                                     clusterId,
                                     isClient,
                                     delayMs,
                                     (sleepControl == EMBER_AF_OK_TO_HIBERNATE
                                      ? EMBER_AF_LONG_POLL
                                      : EMBER_AF_SHORT_POLL),
                                     (sleepControl == EMBER_AF_STAY_AWAKE
                                      ? EMBER_AF_STAY_AWAKE
                                      : EMBER_AF_OK_TO_SLEEP));
}

EmberStatus emberAfScheduleClientTickExtended(uint8_t endpoint,
                                              EmberAfClusterId clusterId,
                                              uint32_t delayMs,
                                              EmberAfEventPollControl pollControl,
                                              EmberAfEventSleepControl sleepControl)
{
  return emberAfScheduleTickExtended(endpoint,
                                     clusterId,
                                     EMBER_AF_CLIENT_CLUSTER_TICK,
                                     delayMs,
                                     pollControl,
                                     sleepControl);
}

EmberStatus emberAfScheduleClientTick(uint8_t endpoint,
                                      EmberAfClusterId clusterId,
                                      uint32_t delayMs)
{
  return emberAfScheduleClientTickExtended(endpoint,
                                           clusterId,
                                           delayMs,
                                           EMBER_AF_LONG_POLL,
                                           EMBER_AF_OK_TO_SLEEP);
}

EmberStatus emberAfScheduleServerTickExtended(uint8_t endpoint,
                                              EmberAfClusterId clusterId,
                                              uint32_t delayMs,
                                              EmberAfEventPollControl pollControl,
                                              EmberAfEventSleepControl sleepControl)
{
  return emberAfScheduleTickExtended(endpoint,
                                     clusterId,
                                     EMBER_AF_SERVER_CLUSTER_TICK,
                                     delayMs,
                                     pollControl,
                                     sleepControl);
}

EmberStatus emberAfScheduleServerTick(uint8_t endpoint,
                                      EmberAfClusterId clusterId,
                                      uint32_t delayMs)
{
  return emberAfScheduleServerTickExtended(endpoint,
                                           clusterId,
                                           delayMs,
                                           EMBER_AF_LONG_POLL,
                                           EMBER_AF_OK_TO_SLEEP);
}

EmberStatus emberAfDeactivateClusterTick(uint8_t endpoint,
                                         uint16_t clusterId,
                                         bool isClient)
{
  debug("Tick deactivated, ep: %d, cluster 0x%04X, client: %s\n",
        endpoint,
        clusterId,
        (isClient ? "yes" : "no"));
  return EMBER_SUCCESS;
}

EmberStatus emberAfDeactivateClientTick(uint8_t endpoint,
                                        EmberAfClusterId clusterId)
{
  return emberAfDeactivateClusterTick(endpoint, clusterId, true);
}

EmberStatus emberAfDeactivateServerTick(uint8_t endpoint,
                                        EmberAfClusterId clusterId)
{
  return emberAfDeactivateClusterTick(endpoint, clusterId, false);
}

EmberStatus emberLookupEui64ByNodeId(EmberNodeId nodeId,
                                     EmberEUI64 eui64)
{
  if (giveEuiLookupResponse) {
    MEMMOVE(eui64, euiLookupResponse, EUI64_SIZE);
    giveEuiLookupResponse = false;
    return EMBER_SUCCESS;
  }
  return EMBER_ERR_FATAL;
}

EmberStatus emberAfSendDefaultResponse(const EmberAfClusterCommand *cmd,
                                       EmberAfStatus status)
{
  if (!expectDefaultResponse) {
    note("Error: Got unexpected default response (0x%02X)\n", status);
    assert(0);
  }
  if (defaultResponseStatus != status) {
    note("Error: Expected default response status (0x%02X) does not equal received status (0x%02X)\n",
         defaultResponseStatus,
         status);
    assert(0);
  }
  return EMBER_SUCCESS;
}

void defaultResponseExpected(EmberAfStatus status)
{
  expectDefaultResponse = true;
  defaultResponseStatus = status;
}

// NOTE:  There are two APIs to do IEEE address lookup:
//   (1) via the Stack directly
//   (2) via the App. Framework (which normally uses the stack)

EmberStatus emberIeeeAddressRequest(EmberNodeId target,
                                    bool reportKids,
                                    uint8_t childStartIndex,
                                    EmberApsOption options)
{
  if (!expectIeeeRequest) {
    note("Error: Unexpected IEEE request.\n");
    assert(0);
  }
  expectIeeeRequest = false;
  debug("Got IEEE request for node ID 0x%02X\n", target);
  return EMBER_SUCCESS;
}

EmberStatus emberAfFindIeeeAddress(EmberNodeId shortAddress,
                                   EmberAfServiceDiscoveryCallback *callback)
{
  assert(expectIeeeRequest);
  expectIeeeRequest = false;
  return EMBER_SUCCESS;
}

uint8_t emberAfNextSequence(void)
{
  return TEST_ZCL_SEQUENCE;
}

void sendZclMessage(bool request)
{
  // This stub is similar to the way the real util.c function works.
  uint8_t sequenceNumber = (request
                            ? emberAfNextSequence()
                            : 0); // last sequence number.  Should default to 0
                                  // for the calls to this.

  if ( appResponseData[0] & ZCL_MANUFACTURER_SPECIFIC_MASK ) {
    appResponseData[3] = sequenceNumber;
  } else {
    appResponseData[1] = sequenceNumber;
  }

  messageToSend.sentMessage = true;
  MEMCOPY(&(messageToSend.apsFrame), &emberAfApsResponseFrame, sizeof(EmberApsFrame));
  messageToSend.length = appResponseLength;
  messageToSend.message = malloc(appResponseLength);
  assert(messageToSend.message != NULL);
  MEMCOPY(messageToSend.message, appResponseData, appResponseLength);
  messageToSend.callback = NULL;
  messageToSend.destination = emberAfResponseDestination;
}

EmberStatus emberAfSendResponse(void)
{
  sendZclMessage(false); // request?
  return EMBER_SUCCESS;
}

void emberAfFormatMfgString(uint8_t* mfgString)
{
  uint8_t string[] = "test-framework";
  MEMSET(mfgString, 0, MFG_STRING_MAX_LENGTH + 1);
  MEMMOVE(mfgString,
          string,
          strlen((char*)string));
}

void emAfApplyRetryOverride(EmberApsOption *options)
{
}

void emAfApplyDisableDefaultResponse(uint8_t *frame_control)
{
}
//------------------------------------------------------------------------------

void emberAfPrint(uint16_t functionality, const char * formatString, ...)
{
  va_list ap;
  va_start(ap, formatString);
  vDebug(formatString, ap);
  va_end(ap);
}

void emberAfPrintln(uint16_t functionality, const char * formatString, ...)
{
  va_list ap;
  va_start(ap, formatString);
  vDebug(formatString, ap);
  va_end(ap);
  debug("\n");
}

bool emberAfPrintEnabled(uint16_t area)
{
  // Always print all areas when running unit tests.
  return true;
}

static void printMessage(const char* prefix,
                         bool debug,
                         uint16_t length,
                         const uint8_t* message)
{
  if ((debug && isTestFrameworkDebugOn()) || !debug) {
    bool truncated = false;
    int i;
    char string[100]; // arbitrarily pick 100 as a max limit,
                      // bigger than most any strings we will print
    sprintf(string, "%s (%d bytes): ", prefix, length);
    note(string);
    if (length > MAX_DATA_VERIFY_LENGTH) {
      length = MAX_DATA_VERIFY_LENGTH;
      truncated = true;
    }
    for (i = 0; i < length; i++) {
      note("0x%02X ", message[i]);
      if ((i >= 8) && ((i % 8) == 0)) {
        note("\n%*s", strlen(string), "");
      }
    }
    note("\n");
    if (truncated) {
      note("MAX_DATA_VERIFY_LENGTH (%d) exceeded! Truncated output.\n\n", MAX_DATA_VERIFY_LENGTH);
    }
  }
}

void verifyDataWithFileInfo(const char* name,
                            uint16_t expectedLength,
                            uint8_t* expectedData,
                            uint16_t actualLength,
                            uint8_t* actualData,
                            const char* file,
                            int lineNumber)
{
  bool checkFailed = false;
  debug("%s\n", name);
  if (expectedLength != actualLength) {
    note("\nLength mismatch.  Expected %d bytes, got %d bytes\n",
         expectedLength,
         actualLength);
    checkFailed = true;
  } else if (0 != MEMCOMPARE(expectedData, actualData, actualLength)) {
    note("\nActual data does not equal expected data.\n");
    checkFailed = true;
  }

  if (expectedLength > MAX_DATA_VERIFY_LENGTH
      || actualLength > MAX_DATA_VERIFY_LENGTH) {
    note("\nExpected or actual length exceeds MAX_DATA_VERIFY_LENGTH (%d)\n", MAX_DATA_VERIFY_LENGTH);
  }

  if (checkFailed) {
    note("Check Failed at %s:%d\n", file, lineNumber);
    printMessage("Expected Message", false, expectedLength, expectedData);
    printMessage("Actual Data     ", false, actualLength, actualData);
    assert(0);
  }

  note(".");
}

void verifyMessage(const char* name, uint16_t length, const uint8_t* message)
{
  if (!messageToSend.sentMessage) {
    note("Error: %s message expected but none queued up.\n", name);
    printMessage("Expected Message", false, length, message);
    assert(0);
  }
  if (length != messageToSend.length
      || 0 != MEMCOMPARE(message, messageToSend.message, length)) {
    note("Error: Expected and received %s messages do not match.\n", name);
    printMessage("Expected Message", false, length, message);
    printMessage("Actual Message  ", false, appResponseLength, appResponseData);
    assert(0);
  }
  printMessage(name, true, length, message);
  note(".");

  debug("[Verified TX of %s] Dest: 0x%2X, Profile: 0x%2X, Cluster: 0x%2X, Length: %d\n",
        name,
        messageToSend.destination,
        messageToSend.apsFrame.profileId,
        messageToSend.apsFrame.clusterId,
        messageToSend.length);
  printMessage("Message: ", true, messageToSend.length, messageToSend.message);

  if (messageToSend.callback) {
    // Currently this test code doesn't support multicasts, but it could
    (messageToSend.callback)(((emberAfResponseDestination < EMBER_BROADCAST_ADDRESS)
                              ? EMBER_OUTGOING_DIRECT
                              : EMBER_OUTGOING_BROADCAST),
                             messageToSend.destination,
                             &(messageToSend.apsFrame),
                             messageToSend.length,
                             messageToSend.message,
                             EMBER_SUCCESS);
  }

  messageToSend.sentMessage = false;
  free(messageToSend.message);
  messageToSend.message = NULL;
  messageToSend.length = 0;
  messageToSend.callback = NULL;
  messageToSend.destination = EMBER_NULL_NODE_ID;
}

static void testSend(EmberNodeId destination,
                     EmberApsFrame *apsFrame,
                     uint16_t messageLength,
                     uint8_t *message,
                     EmberAfMessageSentFunction callback)
{
  if (messageToSend.message != NULL) {
    note("Error: Previous message of %d bytes was not verified.\n", messageToSend.length);
    printMessage("Message: ", false, messageToSend.length, messageToSend.message);
    assert(0);
  }

  messageToSend.destination = destination;
  MEMCOPY(&(messageToSend.apsFrame), apsFrame, sizeof(EmberApsFrame));
  messageToSend.length = messageLength;
  messageToSend.message = malloc(messageLength);
  assert(messageToSend.message);
  MEMCOPY(messageToSend.message, message, messageLength);
  messageToSend.sentMessage = true;
  messageToSend.callback = callback;

  debug("[TX] Dest: 0x%2X, Profile: 0x%2X, Cluster: 0x%2X, EP: %d->%d, Length: %d\n",
        messageToSend.destination,
        messageToSend.apsFrame.profileId,
        messageToSend.apsFrame.clusterId,
        messageToSend.apsFrame.sourceEndpoint,
        messageToSend.apsFrame.destinationEndpoint,
        messageToSend.length);
  printMessage("Message: ", true, messageToSend.length, messageToSend.message);
}

EmberStatus emberAfSendBroadcastWithAliasWithCallback(EmberNodeId destination,
                                                      EmberApsFrame *apsFrame,
                                                      uint16_t messageLength,
                                                      uint8_t *message,
                                                      EmberNodeId alias,
                                                      uint8_t sequence,
                                                      EmberAfMessageSentFunction callback)
{
  assert(0);
  return EMBER_SUCCESS;
}

EmberStatus emberAfSendMulticastWithAliasWithCallback(EmberMulticastId multicastId,
                                                      EmberApsFrame *apsFrame,
                                                      uint16_t messageLength,
                                                      uint8_t *message,
                                                      EmberNodeId alias,
                                                      uint8_t sequence,
                                                      EmberAfMessageSentFunction callback)
{
  assert(0);
  return EMBER_SUCCESS;
}

EmberStatus emberAfSendBroadcastWithCallback(EmberNodeId destination,
                                             EmberApsFrame *apsFrame,
                                             uint16_t messageLength,
                                             uint8_t *message,
                                             EmberAfMessageSentFunction callback)
{
  // This could be implemented to test broadcasts.
  assert(0);
  return EMBER_SUCCESS;
}

EmberStatus emberAfSendUnicastToBindingsWithCallback(EmberApsFrame *apsFrame,
                                                     uint16_t messageLength,
                                                     uint8_t* message,
                                                     EmberAfMessageSentFunction callback)
{
  assert(0);
  return EMBER_SUCCESS;
}

EmberStatus emberAfSendMulticastWithCallback(EmberMulticastId multicastId,
                                             EmberApsFrame *apsFrame,
                                             uint16_t messageLength,
                                             uint8_t* message,
                                             EmberAfMessageSentFunction callback)
{
  // This could be implemented to test multicasts.
  assert(0);
  return EMBER_SUCCESS;
}

EmberStatus emberAfSendUnicastWithCallback(EmberOutgoingMessageType type,
                                           uint16_t indexOrDestination,
                                           EmberApsFrame *apsFrame,
                                           uint16_t messageLength,
                                           uint8_t* message,
                                           EmberAfMessageSentFunction callback)
{
  testSend(indexOrDestination,
           apsFrame,
           messageLength,
           message,
           callback);
  return EMBER_SUCCESS;
}

EmberStatus emberAfSendMulticastToBindings(EmberApsFrame *apsFrame,
                                           uint16_t messageLength,
                                           uint8_t* message)
{
  return EMBER_SUCCESS;
}

EmberStatus emberAfSendInterPan(EmberPanId panId,
                                const EmberEUI64 destinationLongId,
                                EmberNodeId destinationShortId,
                                EmberMulticastId multicastId,
                                EmberAfClusterId clusterId,
                                EmberAfProfileId profileId,
                                uint16_t messageLength,
                                uint8_t* messageBytes)
{
  // This could be implemented to test interpan.
  assert(0);
  return EMBER_SUCCESS;
}
