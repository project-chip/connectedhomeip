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
 * @brief Drives all the unit tests for the app framework.
 *******************************************************************************
   ******************************************************************************/

#include PLATFORM_HEADER

#define ENDPOINT_COUNT 10

#include "common.h"

#include "app/framework/security/crypto-state.h"
#include "app/framework/include/af.h"
#include "app/framework/util/af-event.h"

#include "app/framework/test/test-framework.h"

//#include "print.h"

#include <stdlib.h>

#define N_CLUSTERS 10
#define N_ATTRIBUTES 10
#define ATTR_SIZE 64
#define EMBER_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK 4

#define PROGRESS_STEP() fprintf(stderr, ".")

// Stubs. These are provided because this doesn't compile against whole AF
EmberStatus halCommonIdleForMilliseconds(uint32_t *duration)
{
  return EMBER_SUCCESS;
}
EmberTaskControl emTasks[2];
const uint8_t emTaskCount = 2;
const EmberAfOtaImageId emberAfInvalidImageId;

// Needed for price cluster
uint8_t emberAfPriceClusterDefaultCppEventAuthorization = 0;

void emberAfClusterInitCallback(uint8_t endpoint,
                                EmberAfClusterId clusterId)
{
}

EmberAfStatus emberAfExternalAttributeWriteCallback(uint8_t endpoint,
                                                    EmberAfClusterId clusterId,
                                                    EmberAfAttributeMetadata * attributeMetadata,
                                                    uint16_t manufacturerCode,
                                                    uint8_t * buffer)
{
  return EMBER_ZCL_STATUS_FAILURE;
}

EmberAfStatus emberAfExternalAttributeReadCallback(uint8_t endpoint,
                                                   EmberAfClusterId clusterId,
                                                   EmberAfAttributeMetadata * attributeMetadata,
                                                   uint16_t manufacturerCode,
                                                   uint8_t * buffer,
                                                   uint16_t maxReadLength)
{
  return EMBER_ZCL_STATUS_FAILURE;
}

void emAfResetAttributes(uint8_t endpointId)
{
}

uint8_t emberAfStringLength(const uint8_t *buffer)
{
  return (buffer[0] == 0xFF ? 0 : buffer[0]);
}
uint16_t emberAfLongStringLength(const uint8_t *buffer)
{
  uint16_t length = buffer[0] + (buffer[1] << 8);
  return (length == 0xFFFF ? 0 : length);
}

void emberAfCopyString(uint8_t *dest, uint8_t *src, uint8_t size)
{
  uint8_t length = emberAfStringLength(src);
  if (size < length) {
    length = size;
  }
  MEMMOVE(dest + 1, src + 1, length);
  dest[0] = length;
}

void emberAfCopyLongString(uint8_t *dest, uint8_t *src, uint16_t size)
{
  uint16_t length = emberAfLongStringLength(src);
  if (size < length) {
    length = size;
  }
  MEMMOVE(dest + 2, src + 2, length);
  dest[0] = LOW_BYTE(length);
  dest[1] = HIGH_BYTE(length);
}

EmberStatus emberAfSendResponse(void)
{
  return 0;
}

EmberStatus emberAfSendResponseWithCallback(EmberAfMessageSentFunction callback)
{
  return 0;
}

EmberStatus emberAfSendMulticast(EmberMulticastId multicastId,
                                 EmberApsFrame *apsFrame,
                                 uint16_t messageLength,
                                 uint8_t* message)
{
  return 0;
}
EmberStatus emberAfSendMulticastWithCallback(EmberMulticastId multicastId,
                                             EmberApsFrame *apsFrame,
                                             uint16_t messageLength,
                                             uint8_t* message,
                                             EmberAfMessageSentFunction callback)
{
  return 0;
}
EmberStatus emberAfSendMulticastWithAliasWithCallback(EmberMulticastId multicastId,
                                                      EmberApsFrame *apsFrame,
                                                      uint16_t messageLength,
                                                      uint8_t* message,
                                                      EmberNodeId alias,
                                                      uint8_t sequence,
                                                      EmberAfMessageSentFunction callback)
{
  return 0;
}
EmberStatus emberAfSendBroadcast(EmberNodeId destination,
                                 EmberApsFrame *apsFrame,
                                 uint16_t messageLength,
                                 uint8_t* message)
{
  return 0;
}

EmberStatus emberAfSendBroadcastWithCallback(EmberNodeId destination,
                                             EmberApsFrame *apsFrame,
                                             uint16_t messageLength,
                                             uint8_t* message,
                                             EmberAfMessageSentFunction callback)
{
  return 0;
}

EmberStatus emberAfSendBroadcastWithAliasWithCallback(EmberNodeId destination,
                                                      EmberApsFrame *apsFrame,
                                                      uint16_t messageLength,
                                                      uint8_t* message,
                                                      EmberNodeId alias,
                                                      uint8_t sequence,
                                                      EmberAfMessageSentFunction callback)
{
  return 0;
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
  return 0;
}

EmberStatus emberAfSendUnicast(EmberOutgoingMessageType type,
                               uint16_t indexOrDestination,
                               EmberApsFrame *apsFrame,
                               uint16_t messageLength,
                               uint8_t* message)
{
  return 0;
}
EmberStatus emberAfSendUnicastWithCallback(EmberOutgoingMessageType type,
                                           uint16_t indexOrDestination,
                                           EmberApsFrame *apsFrame,
                                           uint16_t messageLength,
                                           uint8_t* message,
                                           EmberAfMessageSentFunction callback)
{
  return 0;
}
EmberStatus emberAfSendUnicastToBindings(EmberApsFrame *apsFrame,
                                         uint16_t messageLength,
                                         uint8_t* message)
{
  return 0;
}
EmberStatus emberAfSendUnicastToBindingsWithCallback(EmberApsFrame *apsFrame,
                                                     uint16_t messageLength,
                                                     uint8_t* message,
                                                     EmberAfMessageSentFunction callback)
{
  return 0;
}
EmberStatus emberAfSendMulticastToBindings(EmberApsFrame *apsFrame,
                                           uint16_t messageLength,
                                           uint8_t* message)
{
  return 0;
}
bool emberAfDetermineIfLinkSecurityIsRequired(uint8_t commandId,
                                              bool incoming,
                                              bool broadcast,
                                              EmberAfProfileId profileId,
                                              EmberAfClusterId clusterId,
                                              EmberNodeId remoteNodeId)
{
  return false;
}
bool emberAfIsCurrentSecurityProfileSmartEnergy(void)
{
  return false;
}
uint8_t emberAfMaximumApsPayloadLength(EmberOutgoingMessageType type,
                                       uint16_t indexOrDestination,
                                       EmberApsFrame *apsFrame)
{
  return EMBER_AF_MAXIMUM_APS_PAYLOAD_LENGTH;
}
EmberStatus emAfInitializeNetworkIndexStack(void)
{
  return EMBER_SUCCESS;
}
EmberStatus emberAfPushNetworkIndex(uint8_t networkIndex)
{
  return EMBER_SUCCESS;
}
EmberStatus emberAfPushCallbackNetworkIndex(void)
{
  return EMBER_SUCCESS;
}
EmberStatus emberAfPushEndpointNetworkIndex(uint8_t endpoint)
{
  return EMBER_SUCCESS;
}
EmberStatus emberAfPopNetworkIndex(void)
{
  return EMBER_SUCCESS;
}
void emAfAssertNetworkIndexStackIsEmpty(void)
{
}

uint8_t emberGetCurrentNetwork(void)
{
  return 0;
}

void emAfTestAttributeChanged(uint8_t ep,
                              EmberAfAttributeId attributeId)
{
}

void emAfApplyRetryOverride(EmberApsOption *options)
{
}

void emAfApplyDisableDefaultResponse(uint8_t *frame_control)
{
}

// Globals
const uint8_t lgDefaults[] = { 0xab, 0xab, 0xab, 0xab,
                               0xbb, 0xbb, 0xbb, 0xbb,
                               0xaa, 0xaa, 0xaa, 0xaa,
                               0xcc, 0xcc, 0xcc, 0xcc };

const EmberAfAttributeMinMaxValue minMaxValues[] = {
  {
    { (uint8_t*)&(lgDefaults[4]) },
    { (uint8_t*)&(lgDefaults[8]) },
    { (uint8_t*)&(lgDefaults[12]) },
  },
  {
    {
      .defaultValue = 3
    },
    {
      .defaultValue = 1
    },
    {
      .defaultValue = 6
    }
  }
};

const EmberAfAttributeMetadata amTest[] = {
  { 101, 1, 1, ATTRIBUTE_MASK_EXTERNAL_STORAGE, { .defaultValue = 1 } },
  { 102, 1, 2, 0, { .defaultValue = 1 } },
  { 103, 1, 3, 0, { .defaultValue = 1 } },
  { 104, 1, 4, 0, { .defaultValue = 1 } },
  { 105, 1, 5, 0, { .defaultValue = 1 } },
  { 106, 0x23, 0x04, 0, { (uint8_t*)&(lgDefaults[0])   } },
  { 107, 0x23, 0x04, ATTRIBUTE_MASK_MIN_MAX, { (uint8_t*)&(minMaxValues[0]) } },
  { 108, 1, 2, ATTRIBUTE_MASK_MIN_MAX, { (uint8_t*)&(minMaxValues[1]) } },
  { 105, 1, 2, ATTRIBUTE_MASK_MANUFACTURER_SPECIFIC, { .defaultValue = 0xaa } },
  { 105, 1, 2, ATTRIBUTE_MASK_MANUFACTURER_SPECIFIC, { .defaultValue = 0xfd } },
};

uint16_t clusterTestInitCount = 0;
void clusterTestInit(uint8_t ep)
{
  clusterTestInitCount++;
}

uint16_t clusterTestDefaultResponseCount = 0;
void clusterTestDefaultResponse(uint8_t ep, uint8_t cid, EmberAfStatus st)
{
  clusterTestDefaultResponseCount++;
}

uint16_t clusterTestMessageSentCount = 0;
void clusterTestMessageSent(EmberOutgoingMessageType type,
                            uint16_t indexOrDestination,
                            EmberApsFrame *apsFrame,
                            uint16_t msgLen,
                            uint8_t *message,
                            EmberStatus status)
{
  clusterTestMessageSentCount++;
}

EmberAfGenericClusterFunction functions[] =
{ (EmberAfGenericClusterFunction)clusterTestInit,
  (EmberAfGenericClusterFunction)clusterTestDefaultResponse,
  (EmberAfGenericClusterFunction)clusterTestMessageSent };

EmberAfCluster clusterTest = {
  1,
  (EmberAfAttributeMetadata*)amTest,
  10,
  ((5 * 5) + 10),
  CLUSTER_MASK_SERVER | CLUSTER_MASK_CLIENT | CLUSTER_MASK_INIT_FUNCTION | CLUSTER_MASK_DEFAULT_RESPONSE_FUNCTION | CLUSTER_MASK_MESSAGE_SENT_FUNCTION,
  functions
};

const EmberAfEndpointType epTest = {
  (EmberAfCluster*)&clusterTest, 1, (5 * 5) + 10
};

uint8_t tickCount = 0;

// We don't bring in util.c, so we declare this here
uint8_t emberAfSequenceNumber = 0xFF;
uint8_t emberAfNextSequence(void)
{
  return ((++emberAfSequenceNumber) & EMBER_AF_ZCL_SEQUENCE_MASK);
}

void genericTick(void)
{
  fprintf(stderr, ".");
  tickCount++;
}

uint8_t initCount = 0;
void genericInit(void)
{
  fprintf(stderr, ".");
  initCount++;
}

const EmberAfGenericClusterFunction attribute_storage_test_set_up_array1[] = { genericInit };

void attribute_storage_test_set_up(void)
{
  // Set up test data
  uint16_t i;

  emberAfEndpointConfigure();

  EmberAfAttributeMetadata *am = malloc(sizeof(EmberAfAttributeMetadata) * N_CLUSTERS * N_ATTRIBUTES);
  for ( i = 0; i < N_CLUSTERS * N_ATTRIBUTES; i++ ) {
    am[i].attributeId = (EmberAfAttributeId)(i % N_CLUSTERS);
    am[i].attributeType = (uint8_t)(i & 0xFF);
    am[i].size = ATTR_SIZE;
    am[i].mask = 0;
    am[i].defaultValue.defaultValue = (uint16_t)i;
  }

  EmberAfCluster *cluster = malloc(sizeof(EmberAfCluster) * N_CLUSTERS);
  for ( i = 0; i < N_CLUSTERS; i++ ) {
    cluster[i].clusterId = (EmberAfClusterId)i;
    cluster[i].attributes = &(am[i * N_ATTRIBUTES]);
    cluster[i].attributeCount = N_ATTRIBUTES;
    cluster[i].clusterSize = ATTR_SIZE * N_ATTRIBUTES;
    if ( i < 2 ) {
      cluster[i].mask = CLUSTER_MASK_CLIENT | CLUSTER_MASK_SERVER;
    } else if ( i < 6 ) {
      cluster[i].mask = CLUSTER_MASK_CLIENT;
    } else {
      cluster[i].mask = CLUSTER_MASK_SERVER;
    }
    cluster[i].mask |= CLUSTER_MASK_INIT_FUNCTION;
    cluster[i].functions = attribute_storage_test_set_up_array1;
  }

  EmberAfEndpointType *ep0, *ep1;
  ep0 = malloc(sizeof(EmberAfEndpointType));
  ep1 = malloc(sizeof(EmberAfEndpointType));

  ep0->cluster = &(cluster[0]);
  ep1->cluster = &(cluster[N_CLUSTERS / 2]);
  ep0->clusterCount = ep1->clusterCount = (N_CLUSTERS / 2);
  ep0->endpointSize = ep1->endpointSize = (N_CLUSTERS / 2 * ATTR_SIZE * N_ATTRIBUTES);

  for ( i = 0; i < ENDPOINT_COUNT; i++ ) { // 50, 51, .....
    emAfEndpoints[i].endpoint = 50 + i;
    if ( i % 2 == 0 ) {
      emAfEndpoints[i].endpointType = ep0;
    } else {
      emAfEndpoints[i].endpointType = ep1;
    }
  }
}

void attribute_size_test(void)
{
  int n = emberAfGetDataSize(ZCL_INT8U_ATTRIBUTE_TYPE);
  if ( n != 1 ) {
    assert(0);
  }
  PROGRESS_STEP();
  n = emberAfGetDataSize(ZCL_SECURITY_KEY_ATTRIBUTE_TYPE);
  if ( n != 16 ) {
    assert(0);
  }
  PROGRESS_STEP();
}

void cluster_index_test(void)
{
  int i;
  int index;
  EmberAfCluster *cluster;
  for ( i = 0; i < 4; i++ ) {
    PROGRESS_STEP();
    assert(emberAfContainsCluster(50, i));

    cluster = emberAfFindCluster(50, i, 0);
    assert(cluster != NULL);

    if ( i < 2 ) {
      assert(emberAfClusterIsClient(cluster));
      assert(emberAfClusterIsServer(cluster));
    } else {
      assert(emberAfClusterIsClient(cluster));
      assert(!emberAfClusterIsServer(cluster));
    }

    assert(!emberAfContainsCluster(50, i + 5));

    cluster = emberAfFindCluster(50, i + 5, 0);
    assert(cluster == NULL);

    PROGRESS_STEP();
    assert(!emberAfContainsCluster(51, i));
    assert(emberAfContainsCluster(51, i + 5));

    PROGRESS_STEP();
    index = emberAfClusterIndex(50, i, CLUSTER_MASK_CLIENT);
    assert(index == 0);
    index = emberAfClusterIndex(51, i, CLUSTER_MASK_SERVER);
    assert(index == 0xFF);
    index = emberAfClusterIndex(52, i, CLUSTER_MASK_CLIENT);
    assert(index == 1);
    index = emberAfClusterIndex(53, i, CLUSTER_MASK_SERVER);
    assert(index == 0xFF);
  }
}

void init_test(void)
{
  emAfCallInits();
  assert(initCount == (N_CLUSTERS * ENDPOINT_COUNT / 2));
}

void attribute_storage_test(void)
{
  int i, j, k, m;
  EmberAfCluster *cid;

  for ( i = 0; i < ENDPOINT_COUNT; i++ ) {
    // Check afNthCluster()
    if ( i % 2 == 0 ) {
      cid = emberAfGetNthCluster(50 + i, 0, 1);
      assert(cid->clusterId == 0);
      cid = emberAfGetNthCluster(50 + i, 1, 1);
      assert(cid->clusterId == 1);
      cid = emberAfGetNthCluster(50 + i, 2, 1);
      assert(cid == NULL);
      cid = emberAfGetNthCluster(50 + i, 0, 0);
      assert(cid->clusterId == 0);
      cid = emberAfGetNthCluster(50 + i, 1, 0);
      assert(cid->clusterId == 1);
      cid = emberAfGetNthCluster(50 + i, 2, 0);
      assert(cid->clusterId == 2);
    } else {
      cid = emberAfGetNthCluster(50 + i, 0, 1);
      assert(cid->clusterId == 6);
      cid = emberAfGetNthCluster(50 + i, 1, 1);
      assert(cid->clusterId == 7);
      cid = emberAfGetNthCluster(50 + i, 2, 1);
      assert(cid->clusterId == 8);
      cid = emberAfGetNthCluster(50 + i, 0, 0);
      assert(cid->clusterId == 5);
      cid = emberAfGetNthCluster(50 + i, 1, 0);
      assert(cid == NULL);
      cid = emberAfGetNthCluster(50 + i, 2, 0);
      assert(cid == NULL);
    }

    // Check emberAfClusterCount()
    if ( i % 2 == 0 ) {
      assert(emberAfClusterCount(50 + i, 1) == 2);
      assert(emberAfClusterCount(50 + i, 0) == 5);
    } else {
      assert(emberAfClusterCount(50 + i, 1) == 4);
      assert(emberAfClusterCount(50 + i, 0) == 1);
    }
    for ( j = 0; j < N_CLUSTERS / 2; j++ ) {
      PROGRESS_STEP();
      EmberAfClusterId cid;
      if ( i % 2 == 0 ) {
        cid = j;
      } else {
        cid = j + N_CLUSTERS / 2;
      }
      for ( k = 0; k < N_ATTRIBUTES; k++ ) {
        bool x = emberAfContainsAttribute(50 + i,                    // endpoint
                                          cid,                        // clusterId
                                          k,                          // attributeId
                                          (cid >= 2 && cid < 6        // mask
                                           ? CLUSTER_MASK_CLIENT
                                           : CLUSTER_MASK_SERVER),
                                          EMBER_AF_NULL_MANUFACTURER_CODE);
        if (!x) {
          fprintf(stderr, "\ni=%d, j=%d, k=%d, cid=%d\n", i, j, k, cid);
        }
        assert(x);
      }
    }

    //check emberAfGetClustersFromEndpoint
    uint16_t clusterList[20];
    uint8_t serverClusterCount = emberAfGetClustersFromEndpoint(50 + i,
                                                                clusterList,
                                                                10, true);
    uint8_t clientClusterCount = emberAfGetClustersFromEndpoint(50 + i,
                                                                (clusterList + serverClusterCount),
                                                                10, false);
    if (i % 2 == 0) {
      assert(serverClusterCount == 2 && clientClusterCount == 5);
      for (m = 0; m < serverClusterCount; m++) {
        assert(clusterList[m] == m);
      }
      for (m = 0; m < clientClusterCount; m++) {
        assert(clusterList[serverClusterCount + m] == m);
      }
    } else {
      assert(serverClusterCount == 4 && clientClusterCount == 1);
      for (m = 0; m < serverClusterCount; m++) {
        assert(clusterList[m] == (m + 6));
      }
      assert(clusterList[serverClusterCount] == 5);
    }
  }
}

void attribute_storage_test_static(void)
{
  EmberAfAttributeMetadata *am;
  uint8_t i;
  uint8_t *ptr;

  emAfEndpoints[0].endpointType = (EmberAfEndpointType*)&epTest;
  emAfEndpoints[0].endpoint = 50;

  // EXTERNAL STORAGE attribute
  am = emberAfLocateAttributeMetadata(50, clusterTest.clusterId, 101, CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE);
  PROGRESS_STEP();
  if ( am->attributeId != 101 ) {
    assert(0);
  }
  if ( am->size != 1 ) {
    assert(0);
  }

  am = emberAfLocateAttributeMetadata(50, clusterTest.clusterId, 102, CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE);
  PROGRESS_STEP();
  if ( am->attributeId != 102 ) {
    assert(0);
  }
  if ( am->size != 2 ) {
    assert(0);
  }

  am = emberAfLocateAttributeMetadata(50, clusterTest.clusterId, 103, CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE);
  PROGRESS_STEP();
  if ( am->attributeId != 103 ) {
    assert(0);
  }
  if ( am->size != 3 ) {
    assert(0);
  }

  am = emberAfLocateAttributeMetadata(50, clusterTest.clusterId, 104, CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE);
  PROGRESS_STEP();
  if ( am->attributeId != 104 ) {
    assert(0);
  }
  if ( am->size != 4 ) {
    assert(0);
  }

  am = emberAfLocateAttributeMetadata(50, clusterTest.clusterId, 105, CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE);
  PROGRESS_STEP();
  if ( am->attributeId != 105 ) {
    assert(0);
  }
  if ( am->size != 5 ) {
    assert(0);
  }

  am = emberAfLocateAttributeMetadata(50, clusterTest.clusterId, 106, CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE);
  PROGRESS_STEP();
  if ( am->attributeId != 106 ) {
    assert(0);
  }
  if ( am->size != 4 ) {
    assert(0);
  }
  PROGRESS_STEP();
  ptr = am->defaultValue.ptrToDefaultValue;
  for (i = 0; i < am->size; i++) {
    if (ptr[i] != 0xab) {
      assert(0);
    }
  }

  am = emberAfLocateAttributeMetadata(50, clusterTest.clusterId, 107, CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE);
  PROGRESS_STEP();
  if (am->attributeId != 107) {
    assert(0);
  }
  if (am->size != 4) {
    assert(0);
  }
  PROGRESS_STEP();
  ptr = am->defaultValue.ptrToMinMaxValue->defaultValue.ptrToDefaultValue;
  for (i = 0; i < am->size; i++) {
    if (ptr[i] != 0xbb) {
      assert(0);
    }
  }
  PROGRESS_STEP();
  ptr = am->defaultValue.ptrToMinMaxValue->minValue.ptrToDefaultValue;
  for (i = 0; i < am->size; i++) {
    if (ptr[i] != 0xaa) {
      assert(0);
    }
  }
  PROGRESS_STEP();
  ptr = am->defaultValue.ptrToMinMaxValue->maxValue.ptrToDefaultValue;
  for (i = 0; i < am->size; i++) {
    if (ptr[i] != 0xcc) {
      assert(0);
    }
  }

  am = emberAfLocateAttributeMetadata(50, clusterTest.clusterId, 108, CLUSTER_MASK_SERVER, EMBER_AF_NULL_MANUFACTURER_CODE);
  PROGRESS_STEP();
  if ( am->attributeId != 108 ) {
    assert(0);
  }
  if ( am->size != 2 ) {
    assert(0);
  }
  if (am->defaultValue.ptrToMinMaxValue->defaultValue.defaultValue != 3) {
    assert(0);
  }
  if (am->defaultValue.ptrToMinMaxValue->minValue.defaultValue != 1) {
    assert(0);
  }
  if (am->defaultValue.ptrToMinMaxValue->maxValue.defaultValue != 6) {
    assert(0);
  }
  PROGRESS_STEP();

  PROGRESS_STEP();
  EmberAfInitFunction initF;
  EmberAfClusterAttributeChangedCallback attrF;
  EmberAfDefaultResponseFunction respF;
  EmberAfMessageSentFunction sentF;

  initF = (EmberAfInitFunction)emberAfFindClusterFunction(&clusterTest, CLUSTER_MASK_INIT_FUNCTION);
  attrF = (EmberAfClusterAttributeChangedCallback)emberAfFindClusterFunction(&clusterTest, CLUSTER_MASK_ATTRIBUTE_CHANGED_FUNCTION);
  respF = (EmberAfDefaultResponseFunction)emberAfFindClusterFunction(&clusterTest, CLUSTER_MASK_DEFAULT_RESPONSE_FUNCTION);
  sentF = (EmberAfMessageSentFunction)emberAfFindClusterFunction(&clusterTest, CLUSTER_MASK_MESSAGE_SENT_FUNCTION);

  assert(initF != NULL);
  assert(attrF == NULL);
  assert(respF != NULL);
  assert(sentF != NULL);

  clusterTestInitCount = 0;
  (initF)(1);
  assert(clusterTestInitCount == 1);

  clusterTestDefaultResponseCount = 0;
  (respF)(1, 2, 3);
  assert(clusterTestDefaultResponseCount == 1);

  clusterTestMessageSentCount = 0;
  (sentF)(0, 0, NULL, 0, NULL, 0);
  assert(clusterTestMessageSentCount == 1);
}

// This function tests the client api
void client_api_test(void)
{
  uint8_t testBuffer[1000];
  uint16_t ret;
  char string[] = "01234567890";
  uint16_t len;
  EmberApsFrame apsFrame;

  PROGRESS_STEP();

  memset(testBuffer, 0, 1000);
  PROGRESS_STEP();

  // Test the simplistic case with no varargs
  emberAfSetExternalBuffer(testBuffer, 1000, &len, &apsFrame);

  emberAfSequenceNumber = 13;
  ret = emberAfFillExternalBuffer(1, 2, 3, "");
  assert(testBuffer[0] == 1);
  assert(testBuffer[1] == 14);
  assert(testBuffer[2] == 3);
  assert(ret == 3);
  assert(len == 3);
  assert(apsFrame.clusterId == 2);
  PROGRESS_STEP();

  // Test basic integers
  ret = emberAfFillExternalBuffer(1, 0x42, 3, "uvw", 0xAB, 0xCDEF, 0x12345678);
  assert(testBuffer[3] == 0xAB);
  assert(testBuffer[4] == 0xEF);
  assert(testBuffer[5] == 0xCD);
  assert(testBuffer[6] == 0x78);
  assert(testBuffer[7] == 0x56);
  assert(testBuffer[8] == 0x34);
  assert(testBuffer[9] == 0x12);
  assert(ret == 10);
  assert(len == 10);
  assert(apsFrame.clusterId == 0x42);
  PROGRESS_STEP();

  // Test more complex case
  ret = emberAfFillExternalBuffer(1, 2, 3, "6789", string, string, string, string);
  assert(ret == (3 + 6 + 7 + 8 + 9));
  assert(testBuffer[3] == '0');
  assert(testBuffer[3 + 6] == '0');
  assert(testBuffer[3 + 6 + 7] == '0');
  assert(testBuffer[3 + 6 + 7 + 8] == '0');
  assert(apsFrame.clusterId == 2);
  PROGRESS_STEP();

  // Test method which gets buffer passed in
  ret = emberAfFillBuffer(testBuffer, 33, 1, 3, "6789", string, string, string, string);
  assert(ret == (3 + 6 + 7 + 8 + 9));
  assert(testBuffer[3] == '0');
  assert(testBuffer[3 + 6] == '0');
  assert(testBuffer[3 + 6 + 7] == '0');
  assert(testBuffer[3 + 6 + 7 + 8] == '0');
  PROGRESS_STEP();

  // Test proper error reporting if buffer is too small
  ret = emberAfFillBuffer(testBuffer, 32, 1, 3, "6789", string, string, string, string);
  assert(ret == 0);
  PROGRESS_STEP();

  // Test String
  string[0] = 10;
  ret = emberAfFillExternalBuffer(1, 2, 3, "s", string);
  assert(ret == 14);
  assert(testBuffer[3] == 10);
  assert(testBuffer[4] == '1');
  PROGRESS_STEP();

  ret = emberAfFillCommandBasicClusterResetToFactoryDefaults();
  PROGRESS_STEP();

  string[0] = '0';
  ret = emberAfFillBuffer(testBuffer, 100, 1, 3, "S", string, 10);
  assert(ret == 14);
  assert(testBuffer[3] == 10);
  assert(testBuffer[13] == '9');
  PROGRESS_STEP();

  ret = emberAfFillBuffer(testBuffer, 100, 1, 3, "b", string, 10);
  assert(ret == 13);
  assert(testBuffer[3] == '0');
  assert(testBuffer[12] == '9');
  PROGRESS_STEP();
}

// Stubs for printing
int printTestCounter = 0;
const char * firstString;

EmberStatus emberSerialWaitSend(uint8_t port)
{
  printTestCounter++;
  return 0;
}
EmberStatus emberSerialPrintf(uint8_t port, const char * formatString, ...)
{
  if ( printTestCounter == 0 ) {
    firstString = formatString;
  }
  printTestCounter++;
  return 0;
}
EmberStatus emberSerialPrintfLine(uint8_t port, const char * formatString, ...)
{
  if ( printTestCounter == 0 ) {
    firstString = formatString;
  }
  printTestCounter++;
  return 0;
}
EmberStatus emberSerialPrintfVarArg(uint8_t port, const char * formatString, va_list ap)
{
  printTestCounter++;
  return 0;
}
EmberStatus emberSerialPrintCarriageReturn(uint8_t port)
{
  printTestCounter++;
  return 0;
}
uint32_t emberUnsignedCommandArgument(uint8_t x)
{
  printTestCounter++;
  return 0;
}

void printing_test(void)
{
  PROGRESS_STEP();
  emberAfPrintAllOn();

  assert(emberAfPrintEnabled(EMBER_AF_PRINT_BASIC_CLUSTER));
  PROGRESS_STEP();

  emberAfPrintOff(EMBER_AF_PRINT_BASIC_CLUSTER);
  assert(!emberAfPrintEnabled(EMBER_AF_PRINT_BASIC_CLUSTER));
  PROGRESS_STEP();

  assert(emberAfPrintEnabled(EMBER_AF_PRINT_CUSTOM3));
  PROGRESS_STEP();

  emberAfPrintOff(EMBER_AF_PRINT_CUSTOM3);
  assert(!emberAfPrintEnabled(EMBER_AF_PRINT_CUSTOM3));
  PROGRESS_STEP();

  emberAfPrintAllOff();
  assert(!emberAfPrintEnabled(EMBER_AF_PRINT_DEBUG));
  PROGRESS_STEP();

  emberAfPrintOn(EMBER_AF_PRINT_DEBUG);
  assert(emberAfPrintEnabled(EMBER_AF_PRINT_DEBUG));
  PROGRESS_STEP();

  emberAfPrintAllOn();
  printTestCounter = 0;
  emberAfBasicClusterPrintln("Test");
  assert(printTestCounter == 4);
  PROGRESS_STEP();

  emberAfPrintOff(EMBER_AF_PRINT_BASIC_CLUSTER);
  emberAfBasicClusterPrintln("Test");
  assert(printTestCounter == 4);
  PROGRESS_STEP();

  emberAfPrintOff(EMBER_AF_PRINT_CUSTOM2);
  emberAfCustom2Println("More test");
  assert(printTestCounter == 4);
  PROGRESS_STEP();

  emberAfPrintOn(EMBER_AF_PRINT_CUSTOM2);
  emberAfCustom2Println("More test");
  assert(printTestCounter == 8);
  PROGRESS_STEP();

  emberAfCustom2Flush();
  assert(printTestCounter == 9);
  PROGRESS_STEP();

  emberAfPrintOff(EMBER_AF_PRINT_CUSTOM2);
  emberAfCustom2Flush();
  assert(printTestCounter == 9);
  PROGRESS_STEP();

  // Makes sure function doesn't bomb with wrong argument...
  assert(!emberAfPrintEnabled(0xFACE));
  PROGRESS_STEP();

  // ... or that it doesn't work by accident!
  emberAfPrintOff(0xFACE);
  emberAfPrintOn(0xFACE);
  assert(!emberAfPrintEnabled(0xFACE));
  PROGRESS_STEP();

  // Check that the right name is printed

  emberAfPrintAllOn();

  printTestCounter = 0;
  emberAfBasicClusterPrintln("B");
  assert(strcmp(firstString, "Basic") == 0);

  printTestCounter = 0;
  emberAfPowerConfigClusterPrintln("P");
  assert(strcmp(firstString, "Power Configuration") == 0);

  printTestCounter = 0;
  emberAfColorControlClusterPrintln("C");
  assert(strcmp(firstString, "Color Control") == 0);

  printTestCounter = 0;
  emberAfZdoPrintln("E");
  assert(strcmp(firstString, "End device binding") == 0);

  printTestCounter = 0;
  emberAfAttributesPrintln("A");
  assert(strcmp(firstString, "Attributes") == 0);

  printTestCounter = 0;
  emberAfCustom2Println("2");
  assert(strcmp(firstString, "Custom messages (2)") == 0);

  printTestCounter = 0;
  emberAfCustom3Println("3");
  assert(strcmp(firstString, "Custom messages (3)") == 0);
}

uint16_t tickFreqTestTickCount = 0;
void tickFreqTestTick(uint8_t ept)
{
  fprintf(stderr, ".");
  tickFreqTestTickCount++;
}

uint8_t tickFreqTestInitCount = 0;
void tickFreqTestInit(void)
{
  fprintf(stderr, ".");
  tickFreqTestInitCount++;
}

const EmberAfGenericClusterFunction tick_freq_test_set_up_array1[] = { tickFreqTestInit };
void tick_freq_test_set_up(void)
{
  uint16_t i;
  emberAfEndpointConfigure();
  EmberAfAttributeMetadata *am = malloc(sizeof(EmberAfAttributeMetadata) * N_CLUSTERS * N_ATTRIBUTES);
  for ( i = 0; i < N_CLUSTERS * N_ATTRIBUTES; i++ ) {
    am[i].attributeId = (EmberAfAttributeId)(i % N_CLUSTERS);
    am[i].attributeType = (uint8_t)(i & 0xFF);
    am[i].size = ATTR_SIZE;
    am[i].mask = 0;
    am[i].defaultValue.defaultValue = (uint16_t)i;
  }
  EmberAfCluster *cluster = malloc(sizeof(EmberAfCluster) * N_CLUSTERS);
  for ( i = 0; i < N_CLUSTERS; i++ ) {
    cluster[i].clusterId = (EmberAfClusterId)i;
    cluster[i].attributes = &(am[i * N_ATTRIBUTES]);
    cluster[i].attributeCount = N_ATTRIBUTES;
    cluster[i].clusterSize = ATTR_SIZE * N_ATTRIBUTES;
    if ( i < 3 ) {
      cluster[i].mask = CLUSTER_MASK_CLIENT
                        | CLUSTER_MASK_SERVER
                        | CLUSTER_TICK_FREQ_SECOND;
    } else if ( i < 6 ) {
      cluster[i].mask = CLUSTER_MASK_CLIENT
                        | CLUSTER_TICK_FREQ_HALF_SECOND;
    } else {
      cluster[i].mask = CLUSTER_MASK_SERVER
                        | CLUSTER_TICK_FREQ_QUARTER_SECOND;
    }
    cluster[i].mask |= CLUSTER_MASK_INIT_FUNCTION;
    cluster[i].functions =  tick_freq_test_set_up_array1;
  }
  EmberAfEndpointType *ep0;
  ep0 = malloc(sizeof(EmberAfEndpointType));
  ep0->cluster = &(cluster[0]);
  ep0->clusterCount = (N_CLUSTERS);
  ep0->endpointSize = (N_CLUSTERS * ATTR_SIZE * N_ATTRIBUTES);
  for ( i = 0; i < ENDPOINT_COUNT; i++ ) {
    emAfEndpoints[i].endpoint = 50 + i;
    emAfEndpoints[i].endpointType = ep0;
  }
}

void testEventControlServerCallback(uint8_t endpoint)
{
  PROGRESS_STEP();
  emberAfDeactivateClusterTick(0x01, 0xabba, false);
}
void testEventControlClientCallback(uint8_t endpoint)
{
  PROGRESS_STEP();
  emberAfDeactivateClusterTick(0x02, 0xabba, true);
}

//stubs for event testing
EmberEventControl emAfServiceDiscoveryEventControls[EMBER_SUPPORTED_SERVICE_DISCOVERY_STATES_PER_NETWORK][EMBER_SUPPORTED_NETWORKS];
void emAfServiceDiscoveryTimeoutHandler(EmberEventControl *control)
{
}
void event_test(void)
{
  uint32_t time = halCommonGetInt16uMillisecondTick();

  // Must configure dummy endpoints otherwise we cannot
  // schedule events associated to those endpoints.
  emAfEndpoints[0].endpoint   = 0x01;
  emAfEndpoints[0].bitmask    = EMBER_AF_ENDPOINT_ENABLED;

  emAfEndpoints[1].endpoint   = 0x02;
  emAfEndpoints[1].bitmask    = EMBER_AF_ENDPOINT_ENABLED;

  emberEndpointCount = 2;

  assert(EMBER_SUCCESS
         == emberAfScheduleClusterTick(0x01,
                                       0xabba,
                                       false,
                                       10,
                                       EMBER_AF_OK_TO_HIBERNATE));
  assert(EMBER_SUCCESS
         == emberAfScheduleClusterTick(0x02,
                                       0xabba,
                                       true,
                                       10,
                                       EMBER_AF_OK_TO_HIBERNATE));
  assert(10 == emberAfMsToNextEvent(MAX_INT32U_VALUE));

  while (true) {
    uint16_t t = elapsedTimeInt16u(time, halCommonGetInt16uMillisecondTick());
    if ( t > MILLISECOND_TICKS_PER_SECOND
         + MILLISECOND_TICKS_PER_SECOND / 10) {
      break;
    }
    emberAfRunEvents();
  }
  assert(MAX_INT32U_VALUE == emberAfMsToNextEvent(MAX_INT32U_VALUE));
  assert(EMBER_SUCCESS
         == emberAfScheduleClusterTick(0x01,
                                       0xabba,
                                       false,
                                       100,
                                       EMBER_AF_STAY_AWAKE));
  assert(EMBER_SUCCESS
         == emberAfScheduleClusterTick(0x02,
                                       0xabba,
                                       true,
                                       100,
                                       EMBER_AF_OK_TO_HIBERNATE));
  assert(100 == emberAfMsToNextEvent(MAX_INT32U_VALUE));
  assert(EMBER_SUCCESS
         == emberAfScheduleClusterTick(0x01,
                                       0xabba,
                                       false,
                                       50,
                                       EMBER_AF_OK_TO_NAP));
  emberAfCorePrint("\nemberAFMstoNextEvent %d\n", emberAfMsToNextEvent(MAX_INT32U_VALUE));
  assert(50 == emberAfMsToNextEvent(MAX_INT32U_VALUE));
  assert(EMBER_SUCCESS
         == emberAfScheduleClusterTick(0x02,
                                       0xabba,
                                       true,
                                       100,
                                       EMBER_AF_OK_TO_HIBERNATE));
  assert(50 == emberAfMsToNextEvent(MAX_INT32U_VALUE));
  assert(EMBER_SUCCESS
         == emberAfScheduleClusterTick(0x01,
                                       0xabba,
                                       false,
                                       30,
                                       EMBER_AF_OK_TO_HIBERNATE));
  assert(EMBER_SUCCESS
         == emberAfScheduleClusterTick(0x02,
                                       0xabba,
                                       true,
                                       100,
                                       EMBER_AF_OK_TO_HIBERNATE));
  assert(30 == emberAfMsToNextEvent(MAX_INT32U_VALUE));

  //Test scheduling an event for longer than a QS

  assert(EMBER_SUCCESS
         == emberAfScheduleClusterTick(0x01,
                                       0xabba,
                                       false,
                                       MILLISECOND_TICKS_PER_SECOND,
                                       EMBER_AF_OK_TO_HIBERNATE));
  // @ a QS
  assert(EMBER_SUCCESS
         == emberAfScheduleClusterTick(0x02,
                                       0xabba,
                                       true,
                                       MILLISECOND_TICKS_PER_QUARTERSECOND,
                                       EMBER_AF_OK_TO_HIBERNATE));
  assert(1 == emberAfQSToNextEvent(4));

  // way under a QS
  assert(EMBER_SUCCESS
         == emberAfScheduleClusterTick(0x02,
                                       0xabba,
                                       true,
                                       100,
                                       EMBER_AF_OK_TO_HIBERNATE));
  assert(0 == emberAfQSToNextEvent(4));

  //Just under a QS
  assert(EMBER_SUCCESS
         == emberAfScheduleClusterTick(0x02,
                                       0xabba,
                                       true,
                                       MILLISECOND_TICKS_PER_QUARTERSECOND - 1,
                                       EMBER_AF_OK_TO_HIBERNATE));
  assert(0 == emberAfQSToNextEvent(4));

  //Just over a QS
  assert(EMBER_SUCCESS
         == emberAfScheduleClusterTick(0x02,
                                       0xabba,
                                       true,
                                       MILLISECOND_TICKS_PER_QUARTERSECOND + 1,
                                       EMBER_AF_OK_TO_HIBERNATE));
  assert(1 == emberAfQSToNextEvent(4));

  //Return the max
  assert(EMBER_SUCCESS
         == emberAfScheduleClusterTick(0x01,
                                       0xabba,
                                       false,
                                       MILLISECOND_TICKS_PER_SECOND * 4,
                                       EMBER_AF_OK_TO_HIBERNATE));
  assert(EMBER_SUCCESS
         == emberAfScheduleClusterTick(0x02,
                                       0xabba,
                                       true,
                                       MILLISECOND_TICKS_PER_SECOND * 4,
                                       EMBER_AF_OK_TO_HIBERNATE));
  assert(4 == emberAfQSToNextEvent(4));
}

// Test some stuff used to sleep
#define MAX_MS MAX_TIMER_UNITS_HOST
#define MAX_QS (MAX_TIMER_UNITS_HOST << 8)
#define MAX_MIN (MAX_TIMER_UNITS_HOST << 16)
void sleep_test(void)
{
  uint32_t sleepMS;
  uint8_t unit;
  uint16_t duration;

  sleepMS = MAX_MS;
  emAfGetTimerDurationAndUnitFromMS(sleepMS, &duration, &unit);
  assert(duration == MAX_MS);
  assert(unit == EMBER_EVENT_MS_TIME);

  sleepMS = MAX_MS + 1;
  emAfGetTimerDurationAndUnitFromMS(sleepMS, &duration, &unit);
  assert(duration == (sleepMS >> 8));
  assert(unit == EMBER_EVENT_QS_TIME);

  sleepMS = MAX_QS;
  emAfGetTimerDurationAndUnitFromMS(sleepMS, &duration, &unit);
  assert(duration == (sleepMS >> 8));
  assert(unit == EMBER_EVENT_QS_TIME);

  sleepMS = MAX_QS + (1 << 8);
  emAfGetTimerDurationAndUnitFromMS(sleepMS, &duration, &unit);
  assert(duration == (sleepMS >> 16));
  assert(unit == EMBER_EVENT_MINUTE_TIME);

  sleepMS = MAX_QS + (1 << 16);
  emAfGetTimerDurationAndUnitFromMS(sleepMS, &duration, &unit);
  assert(duration == (sleepMS >> 16));
  assert(unit == EMBER_EVENT_MINUTE_TIME);

  sleepMS = MAX_MIN + ((uint32_t)(1 << 16)) - 1;
  emAfGetTimerDurationAndUnitFromMS(sleepMS, &duration, &unit);
  assert(duration == (sleepMS >> 16));
  assert(unit == EMBER_EVENT_MINUTE_TIME);

  sleepMS = MAX_MIN + ((uint32_t)(1 << 16));
  emAfGetTimerDurationAndUnitFromMS(sleepMS, &duration, &unit);
  assert(duration == MAX_TIMER_UNITS_HOST);
  assert(unit == EMBER_EVENT_MINUTE_TIME);

  sleepMS = MAX_MIN + MILLISECOND_TICKS_PER_MINUTE;
  emAfGetTimerDurationAndUnitFromMS(sleepMS, &duration, &unit);
  assert(duration == (sleepMS >> 16));
  assert(unit == EMBER_EVENT_MINUTE_TIME);

  sleepMS = MAX_INT32U_VALUE;
  emAfGetTimerDurationAndUnitFromMS(sleepMS, &duration, &unit);
  assert(duration == MAX_TIMER_UNITS_HOST);
  assert(unit == EMBER_EVENT_MINUTE_TIME);
}

void attributeStorageTest(void)
{
  attribute_storage_test_set_up();
  attribute_storage_test();
  attribute_storage_test_static();
}

void clusterIndexTest(void)
{
  attribute_storage_test_set_up();
  cluster_index_test();
}

void initTest(void)
{
  attribute_storage_test_set_up();
  init_test();
}

void eventTest(void)
{
  emAfInitEvents();
  event_test();
  sleep_test();
}

void endiannessTest(void)
{
  uint32_t x = 0x12345678;
  uint8_t *ptr = (uint8_t*)&x;

  PROGRESS_STEP();
#if (BIGENDIAN_CPU)
  assert(*ptr == 0x12);
  PROGRESS_STEP();
  assert(*(ptr + 1) == 0x34);
  PROGRESS_STEP();
  assert(*(ptr + 2) == 0x56);
  PROGRESS_STEP();
  assert(*(ptr + 3) == 0x78);
#else
  assert(*ptr == 0x78);
  PROGRESS_STEP();
  assert(*(ptr + 1) == 0x56);
  PROGRESS_STEP();
  assert(*(ptr + 2) == 0x34);
  PROGRESS_STEP();
  assert(*(ptr + 3) == 0x12);
#endif
  PROGRESS_STEP();
}

int main(int argc, char **argv)
{
  const TestCase allTests[] = {
    { "attribute-storage", attributeStorageTest },
    { "attribute-size", attribute_size_test },
    { "cluster-index", clusterIndexTest },
    { "init", initTest },
    { "client-api", client_api_test },
    { "printing", printing_test },
    { "event", eventTest },
    { "endianness", endiannessTest },
    { NULL }
  };

  return parseCommandLineAndExecuteTest(argc,
                                        argv,
                                        "application framework",
                                        allTests);
}

EmAfCryptoStatus emAfGetCryptoStatus(void)
{
  return EM_AF_NO_CRYPTO_OPERATION;
}

bool emberRtosIdleHandler(uint32_t *idleTimeMs)
{
  return false;
}
