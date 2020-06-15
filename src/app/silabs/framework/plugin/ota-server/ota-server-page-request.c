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
 * @brief Zigbee Over-the-air bootload cluster for upgrading firmware and
 * downloading specific file.
 *
 * This handles the optional feature of a device requesting a
 * a full page (of EEPROM) and getting multiple image block responses.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "callback.h"
#include "app/framework/plugin/ota-common/ota.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"
#include "ota-server.h"
#include "app/framework/util/util.h"
#include "app/framework/util/common.h"
#include "app/framework/plugin/ota-server-policy/ota-server-policy.h"

#if defined(EMBER_AF_PLUGIN_OTA_SERVER_PAGE_REQUEST_SUPPORT)

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

#define MAXIMUM_PAGE_SIZE 1024

static EmberNodeId requesterNodeId = EMBER_NULL_NODE_ID;
static uint32_t requesterBaseOffset;
static uint16_t requesterPageSize;
static uint16_t totalBytesSent;
static uint8_t requesterMaxDataSize;
static EmberAfOtaImageId requesterImageId;
static bool handlingPageRequest = false;
static uint16_t requesterResponseSpacing;
static uint8_t requesterEndpoint = 0xFF;

#define SHORTEST_SEND_RATE 10L  // ms.

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

static void sendBlockRequest(void);
static void abortPageRequest(void);

#if defined(EM_AF_TEST_HARNESS_CODE)
  #define pageRequestTickCallback(x, y) \
  emAfServerPageRequestTickCallback(x, y)
#else
  #define pageRequestTickCallback(x, y) true
#endif

// -----------------------------------------------------------------------------

uint8_t emAfOtaPageRequestHandler(uint8_t clientEndpoint,
                                  uint8_t serverEndpoint,
                                  const EmberAfOtaImageId* id,
                                  uint32_t offset,
                                  uint8_t maxDataSize,
                                  uint16_t pageSize,
                                  uint16_t responseSpacing)
{
  uint32_t totalSize;
  uint8_t status;
  emberAfOtaBootloadClusterPrintln("RX ImagePageReq mfgId:%2x imageType:%2x, file:%4x, offset:%4x dataSize:%d pageSize%2x spacing:%d",
                                   id->manufacturerId,
                                   id->imageTypeId,
                                   id->firmwareVersion,
                                   offset,
                                   maxDataSize,
                                   pageSize,
                                   responseSpacing);

  // Only allow 1 page request at a time.
  if (requesterNodeId != EMBER_NULL_NODE_ID) {
    otaPrintln("2nd page request not supported");
    return EMBER_ZCL_STATUS_FAILURE;
  }

  status = emberAfOtaPageRequestServerPolicyCallback();
  if (status) {
    return status;
  }

  MEMMOVE(&requesterImageId, id, sizeof(EmberAfOtaImageId));
  totalSize = emberAfOtaStorageGetTotalImageSizeCallback(id);

  if (totalSize == 0) {
    return EMBER_ZCL_STATUS_NOT_FOUND;
  } else if (offset > totalSize || (maxDataSize > pageSize)) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }

  requesterEndpoint = clientEndpoint;
  requesterNodeId = emberAfResponseDestination;
  requesterBaseOffset = offset;
  requesterPageSize = pageSize;
  requesterMaxDataSize = maxDataSize;
  requesterResponseSpacing = (responseSpacing < SHORTEST_SEND_RATE
                              ? SHORTEST_SEND_RATE
                              : responseSpacing);
  totalBytesSent = 0;

  emAfOtaPageRequestTick(serverEndpoint);

  return EMBER_ZCL_STATUS_SUCCESS;
}

void emAfOtaPageRequestTick(uint8_t endpoint)
{
  if (requesterNodeId == EMBER_NULL_NODE_ID) {
    return;
  }

  sendBlockRequest();
  emberAfScheduleServerTickExtended(endpoint,
                                    ZCL_OTA_BOOTLOAD_CLUSTER_ID,
                                    requesterResponseSpacing,
                                    EMBER_AF_SHORT_POLL,
                                    EMBER_AF_OK_TO_SLEEP);
}

bool emAfOtaPageRequestErrorHandler(void)
{
  if (handlingPageRequest) {
    abortPageRequest();
    return true;
  }
  return false;
}

static void abortPageRequest(void)
{
  requesterNodeId = EMBER_NULL_NODE_ID;
  requesterEndpoint = 0xFF;
}

static void sendBlockRequest(void)
{
  uint8_t bytesSentThisTime = 0;
  uint32_t totalSize = emberAfOtaStorageGetTotalImageSizeCallback(&requesterImageId);
  uint8_t maxDataToSend;
  uint32_t bytesLeft;

  if (totalSize == 0) {
    // The image no longer exists.
    abortPageRequest();
    return;
  }

  bytesLeft = totalSize - (requesterBaseOffset + totalBytesSent);

  // 3 possibilities for how much data to send
  //   - Up to requesterMaxSize
  //   - As many bytes are left in the file
  //   - As many bytes are left to fill up client's page size
  if ((requesterPageSize - totalBytesSent) > requesterMaxDataSize) {
    maxDataToSend = (bytesLeft > requesterMaxDataSize
                     ? requesterMaxDataSize
                     : (uint8_t)bytesLeft);
  } else {
    maxDataToSend = requesterPageSize - totalBytesSent;
  }

  emberAfResponseDestination = requesterNodeId;
  handlingPageRequest = true;

  // To enable sending as fast as possible without the receiver
  // having to waste battery power by responding, we clear the
  // retry flag.
  emberAfResponseApsFrame.options &= ~EMBER_APS_OPTION_RETRY;

  if (pageRequestTickCallback(totalBytesSent,
                              requesterMaxDataSize)) {
    // Simulate a block request to the server that we will generate
    // a response to.
    EmberAfImageBlockRequestCallbackStruct callbackStruct;
    MEMSET(&callbackStruct, 0, sizeof(EmberAfImageBlockRequestCallbackStruct));
    callbackStruct.source = requesterNodeId;
    callbackStruct.id = &requesterImageId;
    callbackStruct.offset = requesterBaseOffset + totalBytesSent;
    callbackStruct.maxDataSize = maxDataToSend;
    callbackStruct.clientEndpoint = requesterEndpoint;
    callbackStruct.bitmask =
      EMBER_AF_IMAGE_BLOCK_REQUEST_SIMULATED_FROM_PAGE_REQUEST;

    bytesSentThisTime = emAfOtaImageBlockRequestHandler(&callbackStruct);
    emberAfSendResponse();
  } else {
    bytesSentThisTime += maxDataToSend;
  }
  handlingPageRequest = false;
  if (bytesSentThisTime == 0) {
    emberAfOtaBootloadClusterPrintln("Failed to send image block for page request");
    // We don't need to call abortPageRequest();
    // here because the server will call into our otaPageRequestErrorHandler()
    // if that occurs.
  } else {
    totalBytesSent += bytesSentThisTime;

    if (totalBytesSent >= totalSize
        || totalBytesSent >= requesterPageSize) {
      emberAfOtaBootloadClusterPrintln("Done sending blocks for page request.");
      abortPageRequest();
    }
  }
}

bool emAfOtaServerHandlingPageRequest(void)
{
  return handlingPageRequest;
}

//------------------------------------------------------------------------------
#else // No page request support

uint8_t emAfOtaPageRequestHandler(uint8_t clientEndpoint,
                                  uint8_t serverEndpoint,
                                  const EmberAfOtaImageId* id,
                                  uint32_t offset,
                                  uint8_t maxDataSize,
                                  uint16_t pageSize,
                                  uint16_t responseSpacing)
{
  return EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND;
}

bool emAfOtaPageRequestErrorHandler(void)
{
  return false;
}

void emAfOtaPageRequestTick(uint8_t endpoint)
{
}

bool emAfOtaServerHandlingPageRequest(void)
{
  return false;
}

#endif //  defined (EMBER_AF_PLUGIN_OTA_SERVER_PAGE_REQUEST_SUPPORT)
