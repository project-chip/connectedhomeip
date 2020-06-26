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
 * @brief Unit tests for the OTA Client plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "callback.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"
#include "app/framework/plugin/ota-client-policy/ota-client-policy.h"
#include "app/framework/plugin/ota-client/ota-client-signature-verify.h"
#include "app/framework/plugin/ota-client/ota-client-page-request-test.h"

#include "app/framework/test/test-framework.h"

#include "app/framework/plugin/ota-common/ota.h"

//------------------------------------------------------------------------------
// Globals

#define MANUFACTURER_ID 0x79ab
#define IMAGE_TYPE_ID   0xd073
#define CURRENT_VERSION 0x10003721L

// +1 to force upgrade
#define NEW_VERSION (CURRENT_VERSION + 1)

static EmberAfOtaImageId imageId = {
  MANUFACTURER_ID,
  IMAGE_TYPE_ID,
  CURRENT_VERSION,
  { 0, 0, 0, 0, 0, 0, 0, 0 },
};
static bool expectZdoDiscovery = false;

#define OTA_CLIENT_ENDPOINT 230
#define OTA_SERVER_ID 0x4701
#define OTA_SERVER_ENDPOINT 0x89

static uint8_t otaServerEui[] = { 0xd, 0xe, 0xa, 0xd, 0xb, 0xe, 0xe, 0xf };

const static uint8_t queryMessage[] = {
  (ZCL_CLUSTER_SPECIFIC_COMMAND
   | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),
  TEST_ZCL_SEQUENCE,
  ZCL_QUERY_NEXT_IMAGE_REQUEST_COMMAND_ID,
  0,  // field control
  BYTE_0(MANUFACTURER_ID),
  BYTE_1(MANUFACTURER_ID),
  BYTE_0(IMAGE_TYPE_ID),
  BYTE_1(IMAGE_TYPE_ID),
  BYTE_0(CURRENT_VERSION),
  BYTE_1(CURRENT_VERSION),
  BYTE_2(CURRENT_VERSION),
  BYTE_3(CURRENT_VERSION),
};

static uint32_t currentOffset = 0;

static uint8_t blockRequestMessage[] = {
  (ZCL_CLUSTER_SPECIFIC_COMMAND
   | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),
  TEST_ZCL_SEQUENCE,
  ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID,
  0,  // field control
  BYTE_0(MANUFACTURER_ID),
  BYTE_1(MANUFACTURER_ID),
  BYTE_0(IMAGE_TYPE_ID),
  BYTE_1(IMAGE_TYPE_ID),
  BYTE_0(NEW_VERSION),
  BYTE_1(NEW_VERSION),
  BYTE_2(NEW_VERSION),
  BYTE_3(NEW_VERSION),

  // Offset bytes
  0,
  0,
  0,
  0,

  // Data size
  63,
};

#define IMAGE_SIZE 200

static EmberApsFrame apsFrame = {
  0,         // profile  (don't care)
  ZCL_OTA_BOOTLOAD_CLUSTER_ID,
  OTA_SERVER_ENDPOINT,
  OTA_CLIENT_ENDPOINT,
  EMBER_APS_OPTION_NONE,
  0,                       // group ID
  0,                       // APS sequence
};
static uint8_t otaMessage[100];
static EmberAfClusterCommand otaClientMessage = {
  &apsFrame,
  0,              // incoming message type
  OTA_SERVER_ID,
  otaMessage,
  0,            // message length
  true,         // cluster specific?
  false,        // mfg specific?
  0,            // mfg code
  0x34,         // sequence number
  0,            // command ID
  0,            // payload start index
  ZCL_FRAME_CONTROL_SERVER_TO_CLIENT
};

#define COMMAND_INDEX 2

const EmberAfOtaImageId emberAfInvalidImageId = INVALID_OTA_IMAGE_ID;

static uint8_t upgradeStatusAttribute = OTA_UPGRADE_STATUS_NORMAL;

//------------------------------------------------------------------------------
// Forward declarations

static void discoveryTest(void);

// in ota-client.c
void emAfOtaClientServiceDiscoveryCallback(const EmberAfServiceDiscoveryResult *result);

//------------------------------------------------------------------------------
// Stubs

EmberAfDefinedEndpoint emAfEndpoints[] = {
  { 1, SE_PROFILE_ID, 1, 1, NULL }
};

uint8_t emberAfIndexFromEndpoint(uint8_t endpoint)
{
  return 0;
}
EmberAfOtaStorageStatus emberAfOtaStorageInitCallback(void)
{
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

void emberAfOtaClientVersionInfoCallback(EmberAfOtaImageId* currentImageInfo,
                                         uint16_t* hardwareVersion)
{
  MEMMOVE(currentImageInfo, &imageId, sizeof(EmberAfOtaImageId));
  if (hardwareVersion) {
    *hardwareVersion = EMBER_AF_INVALID_HARDWARE_VERSION;
  }
}

EmberAfOtaImageId emberAfOtaStorageSearchCallback(uint16_t manufacturerId,
                                                  uint16_t manufacturerDeviceId,
                                                  const uint16_t* hardwareVersion)
{
  return emberAfInvalidImageId;
}

EmberAfOtaStorageStatus emberAfOtaStorageGetFullHeaderCallback(const EmberAfOtaImageId* id,
                                                               EmberAfOtaHeader* returnData)
{
  return EMBER_AF_OTA_STORAGE_ERROR;
}

EmberAfOtaStorageStatus emberAfOtaStorageDeleteImageCallback(const EmberAfOtaImageId* id)
{
  return EMBER_AF_OTA_STORAGE_ERROR;
}

EmberAfOtaStorageStatus emberAfOtaStorageReadImageDataCallback(const EmberAfOtaImageId* id,
                                                               uint32_t offset,
                                                               uint32_t length,
                                                               uint8_t* returnData,
                                                               uint32_t* returnedLength)
{
  return EMBER_AF_OTA_STORAGE_ERROR;
}

EmberStatus emberAfFindDevicesByProfileAndCluster(EmberNodeId target,
                                                  EmberAfProfileId profileId,
                                                  EmberAfClusterId clusterId,
                                                  bool serverCluster,
                                                  EmberAfServiceDiscoveryCallback *callback)
{
  if (!expectZdoDiscovery) {
    note("Error: Unexpected ZDO discovery.\n");
    assert(0);
  }
  debug("Got ZDO discovery request to 0x%02X, profile 0x%04X, cluster 0x%04X\n",
        target,
        clusterId);
  expectZdoDiscovery = false;
  return EMBER_SUCCESS;
}

EmberAfOtaStorageStatus emberAfOtaStorageCheckTempDataCallback(uint32_t* currentOffset,
                                                               uint32_t* totalImageSize,
                                                               EmberAfOtaImageId* newFileInfo)
{
  return EMBER_AF_OTA_STORAGE_ERROR;
}

bool emberAfOtaClientDownloadCompleteCallback(EmberAfOtaDownloadResult success,
                                              const EmberAfOtaImageId* id)
{
  return true;
}

EmberAfImageVerifyStatus emberAfOtaClientCustomVerifyCallback(bool newVerification,
                                                              const EmberAfOtaImageId* id)
{
  return EMBER_AF_IMAGE_GOOD;
}

EmberAfOtaStorageStatus emberAfOtaStorageClearTempDataCallback(void)
{
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

EmberAfOtaStorageStatus emberAfOtaStorageWriteTempDataCallback(uint32_t offset,
                                                               uint32_t length,
                                                               const uint8_t* data)
{
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

void emberAfOtaClientBootloadCallback(const EmberAfOtaImageId* id)
{
}

void emberAfPluginOtaClientPreBootloadCallback(uint8_t srcEndpoint,
                                               uint8_t serverEndpoint,
                                               EmberNodeId serverNodeId)
{
}

EmberAfImageVerifyStatus emAfOtaImageSignatureVerify(uint16_t maxHashCalculations,
                                                     const EmberAfOtaImageId* id,
                                                     bool newVerification)
{
  return EMBER_AF_IMAGE_GOOD;
}

EmberAfOtaStorageStatus emberAfOtaStorageFinishDownloadCallback(uint32_t offset)
{
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

uint32_t emberAfOtaStorageDriverMaxDownloadSizeCallback(void)
{
  return 0xFFFFFFFFUL;
}

EmberAfOtaStorageStatus emberAfOtaStorageDriverPrepareToResumeDownloadCallback(void)
{
  return EMBER_AF_OTA_STORAGE_SUCCESS;
}

//------------------------------------------------------------------------------

static EmberAfStatus attributeFunction(AttributeCallbackData* data)
{
  if (data->attributeId != ZCL_FILE_OFFSET_ATTRIBUTE_ID) {
    // We don't care about other attributes.
    return EMBER_ZCL_STATUS_SUCCESS;
  }
  if (data->read) {
    MEMMOVE(data->dataPtr, (uint8_t*)(&currentOffset), sizeof(uint32_t));
    debug("Read of offset value %d\n", currentOffset);
  } else {
    MEMMOVE((uint8_t*)(&currentOffset), data->dataPtr, sizeof(uint32_t));
    debug("Write of offset value %d\n", currentOffset);
  }
  return EMBER_ZCL_STATUS_SUCCESS;
}

static uint8_t putInt16uInBuffer(uint8_t* buffer, uint16_t value)
{
  buffer[0] = BYTE_0(value);
  buffer[1] = BYTE_1(value);
  return 2;
}

static uint8_t putInt32uInBuffer(uint8_t* buffer, uint32_t value)
{
  putInt16uInBuffer(buffer, (uint16_t)value);
  putInt16uInBuffer(&(buffer[2]), (uint16_t)(value >> 16));

  return 4;
}

static EmberAfStatus serverEuiAttributeCallback(AttributeCallbackData* data)
{
  //fprintf(stderr, "[[cluster: %d attribute: %d dataType: %d\r\n]]",
  //        data->cluster, data->attributeId, *(data->dataType) );
  assert(!(data->read));
  assert(data->cluster == ZCL_OTA_BOOTLOAD_CLUSTER_ID);
  if (data->attributeId == ZCL_UPGRADE_SERVER_ID_ATTRIBUTE_ID) {
    assert(*(data->dataType) == ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE);
    assert(0 == MEMCOMPARE(data->dataPtr, otaServerEui, EUI64_SIZE));
  } else if (data->attributeId == ZCL_IMAGE_UPGRADE_STATUS_ATTRIBUTE_ID) {
    assert(*(data->dataType) == ZCL_ENUM8_ATTRIBUTE_TYPE);
    assert(upgradeStatusAttribute == *(data->dataPtr));
  } else {
    assert("Invalid attribute ID" == 0);
  }
  return EMBER_ZCL_STATUS_SUCCESS;
}

static void discoveryTest(void)
{
  EmberAfServiceDiscoveryResult result;
  EmberAfEndpointList list;

  result.status = EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE;
  result.matchAddress = EMBER_NULL_NODE_ID;
  result.responseData = NULL;

  emberAfOtaBootloadClusterClientInitCallback(OTA_CLIENT_ENDPOINT);
  expectZdoDiscovery = true;
  emberAfOtaClientStartCallback();
  assert(expectZdoDiscovery == false);

  debug("*** Test 1: First, no match found.  Client should wait and try again later.\n");
  emAfOtaClientServiceDiscoveryCallback(&result);
  note(".");

  expectZdoDiscovery = true;
  emberAfOtaBootloadClusterClientTickCallback(OTA_CLIENT_ENDPOINT);
  assert(expectZdoDiscovery == false);
  note(".");

  debug("*** Test 2: Arbitrarily the OTA server returns multiple endpoints.  The client should pick the first.\n");
  result.status = EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED;
  result.matchAddress = OTA_SERVER_ID;
  list.count = 3;
  uint8_t endpoints[] = { 67, 5, 180 };
  list.list = endpoints;
  result.responseData = &list;

  expectIeeeRequest = false;
  emAfOtaClientServiceDiscoveryCallback(&result);
  assert(!expectIeeeRequest);
  note(".");

  result.status = EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE;
  result.matchAddress = EMBER_NULL_NODE_ID;
  result.responseData = NULL;

  expectIeeeRequest = true;
  emAfOtaClientServiceDiscoveryCallback(&result);
  assert(!expectIeeeRequest);

  // Check to make sure we are still trying to look up an EUI
  expectIeeeRequest = true;
  emAfOtaClientServiceDiscoveryCallback(&result);
  assert(!expectIeeeRequest);
  note(".");

  result.status = EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE;
  result.matchAddress = OTA_SERVER_ID;
  result.responseData = otaServerEui;

  attributeCallbackFunction = serverEuiAttributeCallback;
  emAfOtaClientServiceDiscoveryCallback(&result);
  assert(0 == (appResponseData[0] & ZCL_DISABLE_DEFAULT_RESPONSE_MASK) );
  verifyMessage("Query Next Image", sizeof(queryMessage), queryMessage);
  note(".");

  attributeCallbackFunction = attributeFunction;

  // At this point the client should be ready to just to its next state
  // after EUI lookup.  This state will differ depending on whether
  // an existing image is present in storage or not.
}

static void queryTest(void)
{
  // To get the client in the right state for a query, we must
  // first make them do discovery.
  discoveryTest();

  emberAfOtaBootloadClusterClientTickCallback(OTA_CLIENT_ENDPOINT);
  assert(0 == (appResponseData[0] & ZCL_DISABLE_DEFAULT_RESPONSE_MASK) );
  verifyMessage("Query Request", sizeof(queryMessage), queryMessage);

  // No upgrade image yet.
  otaClientMessage.buffer[COMMAND_INDEX]
    = ZCL_QUERY_NEXT_IMAGE_RESPONSE_COMMAND_ID;
  otaClientMessage.buffer[COMMAND_INDEX + 1] // status code
    = EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE;
  otaClientMessage.bufLen = COMMAND_INDEX + 2;
  emberAfOtaClientIncomingMessageRawCallback(&otaClientMessage);

  // The client should check later and send the same message
  emberAfOtaBootloadClusterClientTickCallback(OTA_CLIENT_ENDPOINT);
  assert(0 == (appResponseData[0] & ZCL_DISABLE_DEFAULT_RESPONSE_MASK) );
  verifyMessage("Query Request", sizeof(queryMessage), queryMessage);

  // New upgrade is available!
  otaClientMessage.buffer[COMMAND_INDEX]
    = ZCL_QUERY_NEXT_IMAGE_RESPONSE_COMMAND_ID;
  otaClientMessage.buffer[COMMAND_INDEX + 1] // status code
    = EMBER_ZCL_STATUS_SUCCESS;
  int index = COMMAND_INDEX + 2;
  index += putInt16uInBuffer(&(otaClientMessage.buffer[index]),
                             imageId.manufacturerId);
  index += putInt16uInBuffer(&(otaClientMessage.buffer[index]),
                             imageId.imageTypeId);
  index += putInt32uInBuffer(&(otaClientMessage.buffer[index]),
                             NEW_VERSION);
  index += putInt32uInBuffer(&(otaClientMessage.buffer[index]),
                             IMAGE_SIZE);
  otaClientMessage.bufLen = index + 1;
  emberAfOtaClientIncomingMessageRawCallback(&otaClientMessage);
}

static void downloadTest(void)
{
  // To get to the right state, we need to query for a new upgrade.
  queryTest();
  assert(0 == (appResponseData[0] & ZCL_DISABLE_DEFAULT_RESPONSE_MASK) );
  verifyMessage("Image Block request",
                sizeof(blockRequestMessage),
                blockRequestMessage);
}

int main(int argc, char* argv[])
{
  const TestCase allTests[] = {
    { "ota-client-discovery", discoveryTest },
    { "ota-client-query", queryTest },
    { "ota-client-download", downloadTest, },
    { "ota-client-page-request", pageRequestTest, },
    { NULL },
  };

  emEmberTestSetExternalBuffer();

  attributeCallbackFunction = attributeFunction;

  return parseCommandLineAndExecuteTest(argc,
                                        argv,
                                        "af-ota-client-test",
                                        allTests);
}
