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
 * @brief This code tests the service discovery.
 *******************************************************************************
   ******************************************************************************/

#define EMBER_AF_HAS_SECURITY_PROFILE_SE_TEST

#include "app/framework/util/common.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/config.h"

#include "app/framework/util/service-discovery.h"
#include "app/framework/include/af.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "plugin/serial/ember-printf-convert.h"

//------------------------------------------------------------------------------
// Globals

static EmberAfClusterId discoveryCluster;
static EmberNodeId discoveryTarget;

static bool emAfSendMatchDescriptorFailure = false;

static bool debugOn = false;

static bool expectedCallback = false;
static uint8_t expectedResponses;
static uint8_t receivedResponses;
static EmberAfServiceDiscoveryResult expectedResult;

#define MAX_ENDPOINTS 100
static uint8_t expectedEndpoints[MAX_ENDPOINTS];
static EmberAfEndpointList expectedEndpointList = {
  0,   // count
  expectedEndpoints,
};

static EmberEUI64  expectedEui64;

#define CLUSTER_ID 0xAABB
#define PROFILE_ID 0xCCDD
#define LOCAL_NODE_ID 0x1234
#define SENDER_ID  0x5678
#define TOTAL_BROADCAST_RESPONSES 5

enum {
  INCOMING_GOOD_MESSAGE,
  INCOMING_SHORT_MESSAGE,
  INCOMING_BAD_PROFILE,
  INCOMING_BAD_CLUSTER,
  INCOMING_BAD_STATUS,
  INCOMING_WITHOUT_DISCOVERY,
};
typedef uint8_t IncomingMessageType;

static const char* incomingMessageTypeStrings[] = {
  "good",
  "short",
  "bad profile",
  "bad cluster",
  "bad status",
  "bad sender",
};

// Relates to the enums for "EmberAfServiceDiscoveryStatus"
static const char* discoveryStatusStrings[] = {
  "Broadcast service discovery complete",
  "Broadcast service discovery response received",
  "Unicast service discovery timeout",
  "Unicast service discovery complete with response",
};

// 5 bytes overhead
//   ZCL sequence number (1 byte)
//   status   (1 byte)
//   address  (2 bytes)
//   length   (1 byte)
#define MATCH_DESCRIPTOR_OVERHEAD 5

#define REMOTE_NODE_ID 0xBEEF
#define REMOTE_NODE_ID2 0xFEED

// The EUI64 is in hex and has 3 bytes for (>) and 1 byte for '\0'.
#define EUI64_STRING_SIZE ((EUI64_SIZE * 2) + 3 + 1)

typedef struct {
  EmberNodeId shortAddr;
  EmberEUI64 longAddr;
} AddressPair;

static uint8_t zigDevRequestSequence = 0;
// Application generated ZDO messages use sequence numbers 0-127, and the stack
// uses sequence numbers 128-255.  This simplifies life by eliminating the need
// for coordination between the two entities, and allows both to send ZDO
// messages with non-conflicting sequence numbers.
#define APPLICATION_ZDO_SEQUENCE_MASK (0x7F)

//------------------------------------------------------------------------------
// Forward declarations

static void note(const char* format, ...);
uint8_t emberNextZigDevRequestSequence(void);
uint8_t emberGetLastAppZigDevRequestSequence(void);
void setDiscoveryStateLimit(uint8_t limit);

//------------------------------------------------------------------------------
// stubs

void testEventControlServerCallback(uint8_t endpoint)
{
};
void testEventControlClientCallback(uint8_t endpoint)
{
};

EmberStatus emberIeeeAddressRequest(EmberNodeId target,
                                    bool reportKids,
                                    uint8_t childStartIndex,
                                    EmberApsOption options)
{
  if (expectedResult.zdoRequestClusterId != IEEE_ADDRESS_REQUEST) {
    note("Got unexpected IEEE address request.\n");
    assert(0);
  }
  return EMBER_SUCCESS;
}

EmberStatus emberNetworkAddressRequest(EmberEUI64 target,
                                       bool reportKids,
                                       uint8_t childStartIndex)
{
  if (expectedResult.zdoRequestClusterId != NETWORK_ADDRESS_REQUEST) {
    note("Got unexpected NETWORK address request.  ");
    assert(0);
  }
  return EMBER_SUCCESS;
}

EmberStatus emberSimpleDescriptorRequest(EmberNodeId target,
                                         uint8_t targetEndpoint,
                                         EmberApsOption options)
{
  if (expectedResult.zdoRequestClusterId != SIMPLE_DESCRIPTOR_REQUEST) {
    note("Got unexpected SIMPLE DESCRIPTOR request.  ");
    assert(0);
  }
  return EMBER_SUCCESS;
}

EmberStatus emberSendZigDevRequestTarget(EmberNodeId target,
                                         uint16_t clusterId,
                                         EmberApsOption options)
{
  if (expectedResult.zdoRequestClusterId != clusterId) {
    note("Got unexpected request 0x%02X", clusterId);
    assert(0);
  }

  return EMBER_SUCCESS;
}

void emberAfAddToCurrentAppTasks(uint32_t mask)
{
}

void emberAfRemoveFromCurrentAppTasks(uint32_t mask)
{
}

void emberAfNetworkEventControlSetInactive(EmberEventControl *controls)
{
}

EmberStatus emberAfNetworkEventControlSetDelayQS(EmberEventControl *controls,
                                                 uint32_t delayQs)
{
  return EMBER_SUCCESS;
}

uint8_t emberGetCurrentNetwork(void)
{
  return 0;
}

EmberStatus emberAfPushNetworkIndex(uint8_t networkIndex)
{
  return EMBER_SUCCESS;
}

EmberStatus emberAfPopNetworkIndex(void)
{
  return EMBER_SUCCESS;
}

void emberReverseMemCopy(uint8_t* dest, const uint8_t* src, uint8_t length)
{
  uint8_t i;
  uint8_t j = (length - 1);

  for ( i = 0; i < length; i++) {
    dest[i] = src[j];
    j--;
  }
}

//------------------------------------------------------------------------------

// returnData is expected to be EUI64_STRING_SIZE in length
static void formatEui64String(const EmberEUI64 eui64, char* returnData)
{
  uint8_t i, index;
  returnData[0] = '(';
  returnData[1] = '>';
  returnData[2] = ')';
  index = 3;
  for (i = 0; i < EUI64_SIZE; i++) {
    sprintf(returnData + index, "%02X", eui64[i]);
    index += 2;
  }
  returnData[index] = '\0';
}

static void noteEndpointList(const EmberAfEndpointList* endpointList)
{
  uint8_t i;
  for (i = 0; i < endpointList->count; i++) {
    fprintf(stderr, "0x%02X ", endpointList->list[i]);
  }
}

static void note(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
}

static void vDebug(const char* format, va_list ap)
{
  if (debugOn) {
    char* newFormat = transformEmberPrintfToStandardPrintf(format,
                                                           CONVERT_SLASH_R);
    vfprintf(stderr, newFormat, ap);
    free(newFormat);
  }
}

static void debug(const char* format, ...)
{
  va_list ap;
  va_start(ap, format);
  vDebug(format, ap);
  va_end(ap);
}

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

EmberNodeId emberAfGetNodeId(void)
{
  return LOCAL_NODE_ID;
}

static void printRequestType(bool debugPrint,
                             uint16_t cluster)
{
  if ((debugPrint && debugOn)
      || !debugPrint) {
    note("%s discovery",
         (cluster == MATCH_DESCRIPTORS_REQUEST
          ? "MATCH"
          : (cluster == NETWORK_ADDRESS_REQUEST
             ? "NETWORK address"
             : "IEEE address")));
  }
}

EmberStatus emAfSendMatchDescriptor(EmberNodeId target,
                                    uint16_t profileId,
                                    uint16_t clusterId,
                                    bool serverCluster)
{
  if (emAfSendMatchDescriptorFailure) {
    return EMBER_ERR_FATAL;
  }

  discoveryTarget = target;
  discoveryCluster = clusterId;

  return EMBER_SUCCESS;
}

static void testCallback(const EmberAfServiceDiscoveryResult* result)
{
  debug("\n  Got callback of type ");
  printRequestType(true, result->zdoRequestClusterId);
  debug(" result: %s\n", discoveryStatusStrings[result->status]);

  if (!expectedCallback) {
    note("Callback fired but none expected\n");
    assert(0);
  }

  if (expectedResult.zdoRequestClusterId != result->zdoRequestClusterId) {
    note("Request cluster ID does not match expected result.  Wanted ");
    printRequestType(false, expectedResult.zdoRequestClusterId);
    note(", but got ");
    printRequestType(false, expectedResult.zdoRequestClusterId);
    note("\n");
    assert(0);
  }

  if (expectedResult.status != result->status) {
    note("\nError: Expected result of: %s\n"
         "       but got           : %s\n",
         discoveryStatusStrings[expectedResult.status],
         discoveryStatusStrings[result->status]);
    assert(0);
  }

  if (result->status == EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE
      || result->status == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED) {
    if (result->zdoRequestClusterId == MATCH_DESCRIPTORS_REQUEST) {
      const EmberAfEndpointList* endpointList = (const EmberAfEndpointList*)(result->responseData);
      receivedResponses++;

      if (endpointList->count != expectedEndpointList.count
          || (0 != memcmp(expectedEndpointList.list, endpointList->list, endpointList->count))) {
        note("\nError: Expected %d endpoints but got %d.\n",
             expectedEndpointList.count,
             endpointList->count);
        note("       Got: ");
        noteEndpointList(endpointList);
        note("\n  Expected: ");
        noteEndpointList(&expectedEndpointList);
        note("\n");
        assert(0);
      }
    } else if (result->zdoRequestClusterId == NETWORK_ADDRESS_REQUEST
               || result->zdoRequestClusterId == IEEE_ADDRESS_REQUEST) {
      if (result->matchAddress != expectedResult.matchAddress
          || (result->responseData == NULL && expectedResponses != 0)
          || (0 != memcmp(result->responseData, expectedEui64, EUI64_SIZE))) {
        char expectedEui64String[EUI64_STRING_SIZE];
        char actualEui64String[EUI64_STRING_SIZE];
        formatEui64String(expectedEui64, expectedEui64String);
        formatEui64String(result->responseData, actualEui64String);
        note("\nError: Expected node ID 0x%2X, EUI64 %s\n"
             "       But got node ID  0x%2X, EUI64 %s\n",
             expectedResult.matchAddress,
             expectedEui64String,
             result->matchAddress,
             actualEui64String);
        assert(0);
      }
    } else {
      note("Unexpected cluster ID: 0x%2X\n", result->zdoRequestClusterId);
      assert(0);
    }
  }
}

static uint8_t createZdoMessageFromEndpointList(uint8_t* message,
                                                EmberNodeId sender,
                                                const EmberAfEndpointList* list)
{
  uint8_t i;
  uint8_t index = 0;
  message[index] = (uint8_t)(sender);
  index++;
  message[index] = (uint8_t)(sender >> 8);
  index++;
  message[index] = list->count;
  index++;
  for (i = 0; i < list->count; i++) {
    message[index + i] = list->list[i];
  }
  return index + i;
}

static uint8_t createZdoAddressMessage(uint8_t* message,
                                       EmberNodeId sender,
                                       const uint8_t* eui64LittleEndian)
{
  uint8_t index;
  MEMMOVE(message, eui64LittleEndian, EUI64_SIZE);
  index = EUI64_SIZE;
  message[index] = (uint8_t)(sender);
  index++;
  message[index] = (uint8_t)(sender >> 8);
  index++;
  return index;
}

static void incomingDiscoveryMessage(IncomingMessageType type,
                                     bool callbackExpected,
                                     uint16_t zdoClusterId,
                                     EmberAfServiceDiscoveryStatus status,
                                     const void* responseData,
                                     uint8_t sequenceNumber)
{
  EmberApsFrame apsFrame;
  bool shouldBeProcessed = (callbackExpected
                            || !(type == INCOMING_BAD_CLUSTER
                                 || type == INCOMING_BAD_PROFILE
                                 || type == INCOMING_WITHOUT_DISCOVERY));
  EmberNodeId sender = SENDER_ID;

  // ZDO message is: sequence number + status code
  // The memset will set those both accordingly.
  uint8_t zdoMessageLength = 2;
  uint8_t zdoMessage[MATCH_DESCRIPTOR_OVERHEAD + MAX_ENDPOINTS];
  memset(zdoMessage, 0, MATCH_DESCRIPTOR_OVERHEAD + MAX_ENDPOINTS);
  memset(zdoMessage, sequenceNumber, 1);

  debug("\n Simulating incoming ZDO match descriptor response that should%s be processed: %s message\n",
        (shouldBeProcessed ? "" : " NOT"),
        incomingMessageTypeStrings[type]);

  memset(&apsFrame, 0, sizeof(EmberApsFrame));

  apsFrame.profileId = EMBER_ZDO_PROFILE_ID;
  apsFrame.clusterId = (CLUSTER_ID_RESPONSE_MINIMUM | zdoClusterId);
  // Src and dest endpoints already set to zero.
  // Don't care about groupId, sequence, or options.

  if (type == INCOMING_BAD_PROFILE) {
    apsFrame.profileId = 0x1234;
  } else if (type == INCOMING_BAD_CLUSTER) {
    apsFrame.clusterId = 0x1234;
  } else if (type == INCOMING_BAD_STATUS) {
    zdoMessage[1] = 1;
  } else if (type == INCOMING_SHORT_MESSAGE) {
    zdoMessageLength = 3;
  } else if (type == INCOMING_WITHOUT_DISCOVERY) {
    // Do nothing
  }

  expectedResult.status = status;
  expectedResult.matchAddress = sender;
  expectedCallback = callbackExpected;
  expectedResult.zdoRequestClusterId = zdoClusterId;

  if (status == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED
      || status == EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE) {
    expectedResult.responseData = responseData;
  } else {
    expectedResult.responseData = NULL;
  }

  if (type != INCOMING_SHORT_MESSAGE) {
    if (zdoClusterId == MATCH_DESCRIPTORS_REQUEST) {
      const EmberAfEndpointList* responseList = (const EmberAfEndpointList*)responseData;
      memcpy(expectedEndpoints, responseList->list, responseList->count);
      expectedEndpointList.count = responseList->count;
      zdoMessageLength
        += createZdoMessageFromEndpointList(&(zdoMessage[zdoMessageLength]),
                                            sender,
                                            (const EmberAfEndpointList*)responseData);
    } else {
      const uint8_t* responseEui64 = (const uint8_t*)responseData;
      memcpy(expectedEui64, responseEui64, EUI64_SIZE);
      zdoMessageLength
        += createZdoAddressMessage(&(zdoMessage[zdoMessageLength]),
                                   sender,
                                   (const uint8_t*)responseData);
    }
  }

  bool processed = emAfServiceDiscoveryIncoming(sender,
                                                &apsFrame,
                                                zdoMessage,
                                                zdoMessageLength);
  if ((processed && !shouldBeProcessed)
      || (!processed && shouldBeProcessed)) {
    note("Error: Incoming discovery message of type '%s' was%s processed but should%s have been.\n",
         incomingMessageTypeStrings[type],
         (processed ? "" : " NOT"),
         (shouldBeProcessed ? "" : " NOT"));
    assert(0);
  }

  note(".");
}

static void clearExpectations(void)
{
  memset(&expectedResult, 0, sizeof(EmberAfServiceDiscoveryResult));
  expectedCallback = false;
  expectedEndpointList.count = 0;
  memset(expectedEndpoints, 0, MAX_ENDPOINTS);
}

static void matchDescriptorTest(EmberNodeId target,
                                const EmberAfEndpointList* list)
{
  EmberStatus status;
  EmberAfServiceDiscoveryStatus discoveryStatus;
  bool expectedTimeout = (list->count == 0);

  debug("\n  Testing target: 0x%04X, timeout: %s, matching endpoints: %d \n",
        target,
        (expectedTimeout ? "yes" : "no"),
        list->count);

  receivedResponses = 0;
  bool unicastDiscovery = (target < EMBER_BROADCAST_ADDRESS);
  expectedResponses = (expectedTimeout
                       ? 0
                       : (unicastDiscovery
                          ? 1
                          : TOTAL_BROADCAST_RESPONSES));

  discoveryStatus = (expectedTimeout
                     ? (unicastDiscovery
                        ? EMBER_AF_UNICAST_SERVICE_DISCOVERY_TIMEOUT
                        : EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE)
                     : (unicastDiscovery
                        ? EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE
                        : EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED));
  status = emberAfFindDevicesByProfileAndCluster(target,
                                                 PROFILE_ID,
                                                 CLUSTER_ID,
                                                 EMBER_AF_SERVER_CLUSTER_DISCOVERY,
                                                 testCallback);
  emberNextZigDevRequestSequence();
  assert(EMBER_SUCCESS == status);
  expectedResult.zdoRequestClusterId = MATCH_DESCRIPTORS_REQUEST;

  note(".");

  // A second call to service discovery should fail.
  assert(EMBER_SUCCESS != emberAfFindDevicesByProfileAndCluster(target,
                                                                PROFILE_ID,
                                                                CLUSTER_ID,
                                                                EMBER_AF_SERVER_CLUSTER_DISCOVERY,
                                                                testCallback));
  emberNextZigDevRequestSequence();
  note(".");

  uint8_t i;
  for (i = 0; i < expectedResponses; i++) {
    incomingDiscoveryMessage(INCOMING_GOOD_MESSAGE,
                             true,                   // expect callback?
                             MATCH_DESCRIPTORS_REQUEST,
                             discoveryStatus,
                             list,
                             //Subtracting by two since two service discovery requests
                             //were made above with calls to emberAfFindDevicesByProfileAndCluster
                             //which led to an increment of the SD request sequence and dealing with
                             // that request
                             emberGetLastAppZigDevRequestSequence() - 2);
  }
  if (unicastDiscovery) {
    if (expectedTimeout) {
      expectedCallback = true;
      expectedResult.status = discoveryStatus;
    } else {
      expectedCallback = false;
    }
  } else {
    if (expectedTimeout) {
      expectedCallback = true;
    }
    // Regardless of whether we hit a timeout (no responses),
    // a broadcast discovery should execute a callback.
    expectedResult.status = EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE;
  }
  //Subtracting by two since two service discovery requests were made above with calls
  //to emberAfFindDevicesByProfileAndCluster which led to an increment of the SD request
  //sequence and dealing with that request.
  emAfServiceDiscoveryComplete(0, emberGetLastAppZigDevRequestSequence() - 2);

  if (expectedResponses != receivedResponses) {
    note("\nError: Expected responses of %d != received responses of %d\n",
         expectedResponses,
         receivedResponses);
    assert(0);
  }
  clearExpectations();

  // A late discovery (or a discovery we don't care about) should be ignored
  incomingDiscoveryMessage(INCOMING_WITHOUT_DISCOVERY,
                           false,                        // expect callback?
                           MATCH_DESCRIPTORS_REQUEST,
                           discoveryStatus,
                           list,
                           0);
}

static void parallelMatchDescriptorTest(EmberNodeId target,
                                        const EmberAfEndpointList* list)
{
  EmberStatus status;
  EmberAfServiceDiscoveryStatus discoveryStatus;
  bool expectedTimeout = (list->count == 0);

  debug("\n  Testing target: 0x%04X, timeout: %s, matching endpoints: %d \n",
        target,
        (expectedTimeout ? "yes" : "no"),
        list->count);

  receivedResponses = 0;
  bool unicastDiscovery = (target < EMBER_BROADCAST_ADDRESS);
  expectedResponses = (expectedTimeout
                       ? 0
                       : (unicastDiscovery
                          ? 2 * 1
                          : 2 * TOTAL_BROADCAST_RESPONSES));

  discoveryStatus = (expectedTimeout
                     ? (unicastDiscovery
                        ? EMBER_AF_UNICAST_SERVICE_DISCOVERY_TIMEOUT
                        : EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE)
                     : (unicastDiscovery
                        ? EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE
                        : EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED));
  status = emberAfFindDevicesByProfileAndCluster(target,
                                                 PROFILE_ID,
                                                 CLUSTER_ID,
                                                 EMBER_AF_SERVER_CLUSTER_DISCOVERY,
                                                 testCallback);
  emberNextZigDevRequestSequence();
  assert(EMBER_SUCCESS == status);
  expectedResult.zdoRequestClusterId = MATCH_DESCRIPTORS_REQUEST;

  note(".");

  // A second call to service discovery should pass and if unicast then attempting request on another node.
  assert(EMBER_SUCCESS == emberAfFindDevicesByProfileAndCluster(unicastDiscovery ? REMOTE_NODE_ID2 : target,
                                                                PROFILE_ID,
                                                                CLUSTER_ID,
                                                                EMBER_AF_SERVER_CLUSTER_DISCOVERY,
                                                                testCallback));
  emberNextZigDevRequestSequence();
  note(".");

  uint8_t i;
  for (i = 0; i < expectedResponses; i++) {
    incomingDiscoveryMessage(INCOMING_GOOD_MESSAGE,
                             true,                   // expect callback?
                             MATCH_DESCRIPTORS_REQUEST,
                             discoveryStatus,
                             list,
                             //Subtracting by two since two service discovery requests
                             //were made above with calls to emberAfFindDevicesByProfileAndCluster
                             //which led to an increment of the SD request sequence and dealing with
                             // that request
                             unicastDiscovery
                             ? emberGetLastAppZigDevRequestSequence() - 2 + i
                             : TOTAL_BROADCAST_RESPONSES);
  }

  if (unicastDiscovery) {
    if (expectedTimeout) {
      expectedCallback = true;
      expectedResult.status = discoveryStatus;
    } else {
      expectedCallback = false;
    }
  } else {
    if (expectedTimeout) {
      expectedCallback = true;
    }
    // Regardless of whether we hit a timeout (no responses),
    // a broadcast discovery should execute a callback.
    expectedResult.status = EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE;
  }
  //Subtracting by two since two service discovery requests were made above with calls
  //to emberAfFindDevicesByProfileAndCluster which led to an increment of the SD request
  //sequence and dealing with that request.
  emAfServiceDiscoveryComplete(0, emberGetLastAppZigDevRequestSequence() - 2);
  emAfServiceDiscoveryComplete(0, emberGetLastAppZigDevRequestSequence() - 1);

  if (expectedResponses != receivedResponses) {
    note("\nError: Expected responses of %d != received responses of %d\n",
         expectedResponses,
         receivedResponses);
    assert(0);
  }
  clearExpectations();

  // A late discovery (or a discovery we don't care about) should be ignored
  incomingDiscoveryMessage(INCOMING_WITHOUT_DISCOVERY,
                           false,                        // expect callback?
                           MATCH_DESCRIPTORS_REQUEST,
                           discoveryStatus,
                           list,
                           0);
}

// Simple set of tests that the service discovery code correctly
// checks for invalid incoming messages.
static void runIncomingDiscoveryTests(void)
{
  uint8_t endpointList[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  EmberAfEndpointList testList = {
    8,   // list length
    endpointList,
  };

  clearExpectations();

  // We must initiate a discovery otherwise our code will drop all
  // incoming ZDO response on the floor since it isn't expecting one.
  assert(EMBER_SUCCESS
         == emberAfFindDevicesByProfileAndCluster(0x0000,  // target,
                                                  PROFILE_ID,
                                                  CLUSTER_ID,
                                                  EMBER_AF_SERVER_CLUSTER_DISCOVERY,
                                                  testCallback));
  emberNextZigDevRequestSequence();
  debug("*** Testing invalid inputs to ZDO processing code ***\n");

  incomingDiscoveryMessage(INCOMING_BAD_PROFILE,
                           false,                // callback expected?
                           MATCH_DESCRIPTORS_REQUEST,
                           EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                           &testList,
                           //Subtracting by 1 since 1 service discovery request is made
                           //above with a call to emberAfFindDevicesByProfileAndCluster
                           //which led to an increment of the SD request sequence and dealing
                           //with that request
                           emberGetLastAppZigDevRequestSequence() - 1);
  incomingDiscoveryMessage(INCOMING_BAD_CLUSTER,
                           false,                // callback expected?
                           MATCH_DESCRIPTORS_REQUEST,
                           EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                           &testList,
                           //Subtracting by 1 since 1 service discovery request is made
                           //above with a call to emberAfFindDevicesByProfileAndCluster
                           //which led to an increment of the SD request sequence and dealing
                           //with that request
                           emberGetLastAppZigDevRequestSequence() - 1);
  incomingDiscoveryMessage(INCOMING_BAD_STATUS,
                           false,                // callback expected?
                           MATCH_DESCRIPTORS_REQUEST,
                           EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                           &testList,
                           //Subtracting by 1 since 1 service discovery request is made
                           //above with a call to emberAfFindDevicesByProfileAndCluster
                           //which led to an increment of the SD request sequence and dealing
                           //with that request
                           emberGetLastAppZigDevRequestSequence() - 1);
  incomingDiscoveryMessage(INCOMING_SHORT_MESSAGE,
                           false,                // callback expected?
                           MATCH_DESCRIPTORS_REQUEST,
                           EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                           &testList,
                           //Subtracting by 1 since 1 service discovery request is made
                           //above with a call to emberAfFindDevicesByProfileAndCluster
                           //which led to an increment of the SD request sequence and dealing
                           //with that request
                           emberGetLastAppZigDevRequestSequence() - 1);
  expectedCallback = true;
  expectedResult.status = EMBER_AF_UNICAST_SERVICE_DISCOVERY_TIMEOUT;
  //Subtracting by 1 since 1 service discovery request is made above with a call to
  //emberAfFindDevicesByProfileAndCluster which led to an increment of the SD request sequence
  //and dealing with that request.
  emAfServiceDiscoveryComplete(0, emberGetLastAppZigDevRequestSequence() - 1);
  clearExpectations();
}

static void addressRequest(bool ieeeRequest,
                           const AddressPair* addressPairPtr)
{
  // Copy locally to prevent an issue with 'const' pointers.
  AddressPair addressPair;
  memcpy(&addressPair, addressPairPtr, sizeof(AddressPair));

  if (ieeeRequest) {
    expectedResult.zdoRequestClusterId = IEEE_ADDRESS_REQUEST;
  } else {
    expectedResult.zdoRequestClusterId = NETWORK_ADDRESS_REQUEST;
  }

  assert(EMBER_SUCCESS
         == (ieeeRequest
             ? emberAfFindIeeeAddress(addressPair.shortAddr,
                                      testCallback)
             : emberAfFindNodeId(addressPair.longAddr,
                                 testCallback)));
  emberNextZigDevRequestSequence();
  // A second call to perform discovery should fail.
  assert(EMBER_INVALID_CALL == emberAfFindIeeeAddress(addressPair.shortAddr,
                                                      testCallback));
  emberNextZigDevRequestSequence();
  if (ieeeRequest) {
    incomingDiscoveryMessage(INCOMING_GOOD_MESSAGE,
                             true,                   // callback expected?
                             IEEE_ADDRESS_REQUEST,
                             EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                             addressPair.longAddr,
                             // Subtracting by 2 since 2 service discovery requests were made
                             // above with calls to emberAfFindIeeeAddress/emberAfFindNodeId
                             // which led to an increment of the SD request sequence and dealing
                             // with that request.
                             emberGetLastStackZigDevRequestSequence() - 2);
    expectedCallback = false;
  } else {
    incomingDiscoveryMessage(INCOMING_GOOD_MESSAGE,
                             true,                  // callback expected?
                             NETWORK_ADDRESS_REQUEST,
                             EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                             // even though this a NWK address response,
                             // the long address is stored in the 'responseData' section.
                             addressPair.longAddr,
                             // Subtracting by 2 since 2 service discovery requests were made
                             // above with calls to emberAfFindIeeeAddress/emberAfFindNodeId
                             // which led to an increment of the SD request sequence and dealing
                             // with that request.
                             emberGetLastAppZigDevRequestSequence() - 2);

    // With address discovery, even though it is broadcast we only expect a single
    // response.
    expectedCallback = false;
  }
  // Subtracting by 2 since 2 service discovery requests were made
  // above with calls to emberAfFindIeeeAddress/emberAfFindNodeId
  // which led to an increment of the SD request sequence and dealing
  // with that request.
  emAfServiceDiscoveryComplete(0, emberGetLastStackZigDevRequestSequence() - 2);
  clearExpectations();
}

static void addressParallelRequest(bool ieeeRequest,
                                   const AddressPair* addressPairPtr)
{
  // Copy locally to prevent an issue with 'const' pointers.
  AddressPair addressPair;
  memcpy(&addressPair, addressPairPtr, sizeof(AddressPair));

  if (ieeeRequest) {
    expectedResult.zdoRequestClusterId = IEEE_ADDRESS_REQUEST;
  } else {
    expectedResult.zdoRequestClusterId = NETWORK_ADDRESS_REQUEST;
  }
  assert(EMBER_SUCCESS
         == (ieeeRequest
             ? emberAfFindIeeeAddress(addressPair.shortAddr,
                                      testCallback)
             : emberAfFindNodeId(addressPair.longAddr,
                                 testCallback)));
  emberNextZigDevRequestSequence();

  // A second call to perform discovery should pass when multiple service discovery states
  // are allowed.
  assert(EMBER_SUCCESS
         == (ieeeRequest
             ? emberAfFindIeeeAddress(addressPair.shortAddr,
                                      testCallback)
             : emberAfFindNodeId(addressPair.longAddr,
                                 testCallback)));
  emberNextZigDevRequestSequence();

  // A third call to perform discovery should pass when multiple service discovery states
  // are allowed.
  assert(EMBER_SUCCESS
         == (ieeeRequest
             ? emberAfFindIeeeAddress(addressPair.shortAddr,
                                      testCallback)
             : emberAfFindNodeId(addressPair.longAddr,
                                 testCallback)));
  emberNextZigDevRequestSequence();

  // A fourth call to perform discovery should pass when multiple service discovery states
  // are allowed.
  assert(EMBER_SUCCESS
         == (ieeeRequest
             ? emberAfFindIeeeAddress(addressPair.shortAddr,
                                      testCallback)
             : emberAfFindNodeId(addressPair.longAddr,
                                 testCallback)));
  emberNextZigDevRequestSequence();

  if (ieeeRequest) {
    incomingDiscoveryMessage(INCOMING_GOOD_MESSAGE,
                             true,                   // callback expected?
                             IEEE_ADDRESS_REQUEST,
                             EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                             addressPair.longAddr,
                             // Subtracting by 4 since 4 service discovery requests were made
                             // above with calls to emberAfFindIeeeAddress/emberAfFindNodeId
                             // which led to an increment of the SD request sequence and dealing
                             // with that request.
                             emberGetLastStackZigDevRequestSequence() - 4);
    expectedCallback = false;
  } else {
    incomingDiscoveryMessage(INCOMING_GOOD_MESSAGE,
                             true,                  // callback expected?
                             NETWORK_ADDRESS_REQUEST,
                             EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                             // even though this a NWK address response,
                             // the long address is stored in the 'responseData' section.
                             addressPair.longAddr,
                             // Subtracting by 4 since 4 service discovery requests were made
                             // above with calls to emberAfFindIeeeAddress/emberAfFindNodeId
                             // which led to an increment of the SD request sequence and dealing
                             // with that request.
                             emberGetLastAppZigDevRequestSequence() - 4);

    // With address discovery, even though it is broadcast we only expect a single
    // response.
    expectedCallback = false;
  }

  if (ieeeRequest) {
    incomingDiscoveryMessage(INCOMING_GOOD_MESSAGE,
                             true,                   // callback expected?
                             IEEE_ADDRESS_REQUEST,
                             EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                             addressPair.longAddr,
                             // Subtracting by 3 since 4 service discovery requests were made
                             // above with calls to emberAfFindIeeeAddress/emberAfFindNodeId
                             // which led to an increment of the SD request sequence and dealing
                             // with that request.
                             emberGetLastStackZigDevRequestSequence() - 3);
    expectedCallback = false;
  } else {
    incomingDiscoveryMessage(INCOMING_GOOD_MESSAGE,
                             true,                  // callback expected?
                             NETWORK_ADDRESS_REQUEST,
                             EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                             // even though this a NWK address response,
                             // the long address is stored in the 'responseData' section.
                             addressPair.longAddr,
                             // Subtracting by 3 since 4 service discovery requests were made
                             // above with calls to emberAfFindIeeeAddress/emberAfFindNodeId
                             // which led to an increment of the SD request sequence and dealing
                             // with that request.
                             emberGetLastAppZigDevRequestSequence() - 3);

    // With address discovery, even though it is broadcast we only expect a single
    // response.
    expectedCallback = false;
  }

  if (ieeeRequest) {
    incomingDiscoveryMessage(INCOMING_GOOD_MESSAGE,
                             true,                   // callback expected?
                             IEEE_ADDRESS_REQUEST,
                             EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                             addressPair.longAddr,
                             // Subtracting by 2 since 4 service discovery requests were made
                             // above with calls to emberAfFindIeeeAddress/emberAfFindNodeId
                             // which led to an increment of the SD request sequence and dealing
                             // with that request.
                             emberGetLastStackZigDevRequestSequence() - 2);
    expectedCallback = false;
  } else {
    incomingDiscoveryMessage(INCOMING_GOOD_MESSAGE,
                             true,                  // callback expected?
                             NETWORK_ADDRESS_REQUEST,
                             EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                             // even though this a NWK address response,
                             // the long address is stored in the 'responseData' section.
                             addressPair.longAddr,
                             // Subtracting by 2 since 4 service discovery requests were made
                             // above with calls to emberAfFindIeeeAddress/emberAfFindNodeId
                             // which led to an increment of the SD request sequence and dealing
                             // with that request.
                             emberGetLastAppZigDevRequestSequence() - 2);

    // With address discovery, even though it is broadcast we only expect a single
    // response.
    expectedCallback = false;
  }

  if (ieeeRequest) {
    incomingDiscoveryMessage(INCOMING_GOOD_MESSAGE,
                             true,                   // callback expected?
                             IEEE_ADDRESS_REQUEST,
                             EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                             addressPair.longAddr,
                             // Subtracting by 1 since 4 service discovery requests were made
                             // above with calls to emberAfFindIeeeAddress/emberAfFindNodeId
                             // which led to an increment of the SD request sequence and dealing
                             // with that request.
                             emberGetLastStackZigDevRequestSequence() - 1);
    expectedCallback = false;
  } else {
    incomingDiscoveryMessage(INCOMING_GOOD_MESSAGE,
                             true,                  // callback expected?
                             NETWORK_ADDRESS_REQUEST,
                             EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
                             // even though this a NWK address response,
                             // the long address is stored in the 'responseData' section.
                             addressPair.longAddr,
                             // Subtracting by 1 since 4 service discovery requests were made
                             // above with calls to emberAfFindIeeeAddress/emberAfFindNodeId
                             // which led to an increment of the SD request sequence and dealing
                             // with that request.
                             emberGetLastAppZigDevRequestSequence() - 1);

    // With address discovery, even though it is broadcast we only expect a single
    // response.
    expectedCallback = false;
  }

  // Subtracting by 4/3/2/1 since 4 service discovery requests were made
  // above with calls to emberAfFindIeeeAddress/emberAfFindNodeId
  // which led to an increment of the SD request sequence and dealing
  // with that request.
  emAfServiceDiscoveryComplete(0, emberGetLastStackZigDevRequestSequence() - 4);
  emAfServiceDiscoveryComplete(0, emberGetLastStackZigDevRequestSequence() - 3);
  emAfServiceDiscoveryComplete(0, emberGetLastStackZigDevRequestSequence() - 2);
  emAfServiceDiscoveryComplete(0, emberGetLastStackZigDevRequestSequence() - 1);
  clearExpectations();
}

static void runAddressDiscoveryTests(void)
{
  AddressPair addresses[] = {
    { 0x0C0D, { 0xd, 0xe, 0xa, 0xd, 0xb, 0xe, 0xe, 0xf, } },
    { 0x0A0B, { 0xf, 0xe, 0xe, 0xd, 0x0, 0x0, 0x0, 0x0, } },
    { 0x1020, { 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x1, 0x2, } },
    { 0x7239, { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, } },

    // Terminator.  Technically 0x0000 is valid but we won't test it.
    { 0x0000, {} },
  };
  int ieeeRequest;
  for (ieeeRequest = 0; ieeeRequest < 2; ieeeRequest++) {
    debug("\n*** %s Address requests with a response. ***\n",
          (ieeeRequest
           ? "IEEE"
           : "NETWORK"));
    int i = 0;
    while (addresses[i].shortAddr != 0x0000) {
      addressRequest(ieeeRequest,
                     &(addresses[i]));
      i++;
    }
  }
}

static void runParallelAddressDiscoveryTests(void)
{
  setDiscoveryStateLimit(EMBER_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK);
  AddressPair addresses[] = {
    { 0x0C0D, { 0xd, 0xe, 0xa, 0xd, 0xb, 0xe, 0xe, 0xf, } },
    { 0x0A0B, { 0xf, 0xe, 0xe, 0xd, 0x0, 0x0, 0x0, 0x0, } },
    { 0x1020, { 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x1, 0x2, } },
    { 0x7239, { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, } },

    // Terminator.  Technically 0x0000 is valid but we won't test it.
    { 0x0000, {} },
  };
  int ieeeRequest;
  for (ieeeRequest = 0; ieeeRequest < 2; ieeeRequest++) {
    debug("\n*** %s Address requests with a response. ***\n",
          (ieeeRequest
           ? "IEEE"
           : "NETWORK"));
    int i = 0;
    while (addresses[i].shortAddr != 0x0000) {
      addressParallelRequest(ieeeRequest,
                             &(addresses[i]));
      i++;
    }
  }
}

static void runMatchDiscoveryTests(void)
{
  uint8_t endpointList[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  EmberAfEndpointList list = {
    8,
    endpointList,
  };
  EmberAfEndpointList emptyList = {
    0,
    NULL,
  };

  debug("\n*** Unicast match discovery with a response. ***\n");
  matchDescriptorTest(REMOTE_NODE_ID,
                      &list);

  debug("\n*** Unicast match discovery WITHOUT a response.***\n");
  matchDescriptorTest(REMOTE_NODE_ID,
                      &emptyList);

  debug("\n*** Broadcast match discovery with multiple responses.***\n");
  matchDescriptorTest(EMBER_BROADCAST_ADDRESS,
                      &list);

  debug("\n*** Broadcast match discovery WITHOUT any responses.***\n");
  matchDescriptorTest(EMBER_BROADCAST_ADDRESS,
                      &emptyList);
}

static void runParallelMatchDiscoveryTests(void)
{
  setDiscoveryStateLimit(EMBER_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK);
  uint8_t endpointList[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  EmberAfEndpointList list = {
    8,
    endpointList,
  };
  EmberAfEndpointList emptyList = {
    0,
    NULL,
  };

  debug("\n*** Unicast match discovery with a response. ***\n");
  parallelMatchDescriptorTest(REMOTE_NODE_ID,
                              &list);

  debug("\n*** Unicast match discovery WITHOUT a response.***\n");
  parallelMatchDescriptorTest(REMOTE_NODE_ID,
                              &emptyList);

  debug("\n*** Broadcast match discovery with multiple responses.***\n");
  parallelMatchDescriptorTest(EMBER_BROADCAST_ADDRESS,
                              &list);

  debug("\n*** Broadcast match discovery WITHOUT any responses.***\n");
  parallelMatchDescriptorTest(EMBER_BROADCAST_ADDRESS,
                              &emptyList);
}

typedef void (FunctionPtr)(void);

typedef struct {
  const char* name;
  FunctionPtr* functionPtr;
} TestInfo;

static const TestInfo tests[] = {
  { "incoming-zdo", runIncomingDiscoveryTests, },
  { "match-descriptor", runMatchDiscoveryTests, },
  { "address-discovery", runAddressDiscoveryTests, },
  { "parallel-match-discoveries", runParallelMatchDiscoveryTests },
  { "parallel-address-requests", runParallelAddressDiscoveryTests },
  { NULL, NULL, },
};

static void printUsage(void)
{
  int i = 0;
  note("Usage: af-service-discovery-test <test> [ --debug ]\n");
  note("       Available Tests:\n");
  while (tests[i].name != NULL) {
    note("         %s\n", tests[i].name);
    i++;
  }
}

int main(int argc, char* argv[])
{
  setDiscoveryStateLimit(1);
  bool usageError = false;
  const TestInfo* testToRun = NULL;
  if (argc == 3) {
    if (0 == strcasecmp(argv[2], "--debug")) {
      debugOn = true;
    } else {
      usageError = true;
    }
  }
  if (argc < 2 || argc > 3) {
    usageError = true;
  }

  if (usageError) {
    printUsage();
    return 1;
  }

  int i = 0;
  while (tests[i].name != NULL && !testToRun) {
    if (0 == strcasecmp(argv[1], tests[i].name)) {
      testToRun = &(tests[i]);
    }
    i++;
  }

  if (!testToRun) {
    note("Error: Unknown test '%s'\n", argv[1]);
    printUsage();
    return 1;
  }

  note("[Testing Service Discovery (afv2) - %s ", testToRun->name);

  (testToRun->functionPtr)();
  note(" done]\n");

  return 0;
}

uint8_t emberGetLastAppZigDevRequestSequence(void)
{
  return (zigDevRequestSequence & APPLICATION_ZDO_SEQUENCE_MASK);
}

uint8_t emberNextZigDevRequestSequence(void)
{
  return ((++zigDevRequestSequence) & APPLICATION_ZDO_SEQUENCE_MASK);
}

uint8_t emberGetLastStackZigDevRequestSequence(void)
{
  return (zigDevRequestSequence & APPLICATION_ZDO_SEQUENCE_MASK);
}
