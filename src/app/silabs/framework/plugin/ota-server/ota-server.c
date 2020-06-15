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
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "callback.h"
#include "app/framework/plugin/ota-common/ota.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"
#include "ota-server.h"
#include "app/framework/util/util.h"
#include "app/framework/util/common.h"
#include "app/framework/plugin/ota-server/ota-server-dynamic-block-period.h"
#include "app/framework/plugin/ota-server-policy/ota-server-policy.h"

#ifdef EMBER_AF_PLUGIN_SUB_GHZ_SERVER
  #include "app/framework/plugin/sub-ghz-server/sub-ghz-server.h"
#endif

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

extern uint8_t appDefaultResponseStatus;

#define LAST_MESSAGE_ID ZCL_QUERY_SPECIFIC_FILE_RESPONSE_COMMAND_ID

uint8_t otaServerEndpoint = 0;  // invalid endpoint

#define QUERY_NEXT_IMAGE_HW_VER_PRESENT_MASK  0x01

// this mask is the same for both Image block request and image page request
#define IMAGE_REQUEST_IEEE_PRESENT_MASK       0x01

// This determines the maximum amount of size the server can handle in one request.
// Normally the protocol does not use Zigbee's fragmentation and thus puts as
// much data as possible in a single message.  We have to size the response
// due to static data structures that need to know the limit.
// 63 bytes is the maximum amount that can be used without APS encryption.
// The server will automatically shrink the size of the response further based on
// the other send options (e.g. APS encryption or source routing).
#define MAX_POSSIBLE_SERVER_BLOCK_SIZE 63

#if defined(EMBER_AF_PLUGIN_OTA_SERVER_MIN_BLOCK_REQUEST_SUPPORT)
  #define MIN_BLOCK_REQUEST_SERVER_SUPPORT EMBER_AF_IMAGE_BLOCK_REQUEST_MIN_BLOCK_REQUEST_SUPPORTED_BY_SERVER
#else
  #define MIN_BLOCK_REQUEST_SERVER_SUPPORT 0
#endif

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

static bool commandParse(EmberAfClusterCommand* command);

#define prepareClusterResponse(commandId, status) \
  emAfOtaServerPrepareResponse(false,             \
                               (commandId),       \
                               (status),          \
                               0)       // defaultResponsePayloadCommandId

#define prepareDefaultResponse(status, commandId)               \
  emAfOtaServerPrepareResponse(true,                            \
                               ZCL_DEFAULT_RESPONSE_COMMAND_ID, \
                               (status),                        \
                               (commandId))

static void addEmberAfOtaImageIdIntoResponse(const EmberAfOtaImageId* id);

// -------------------------------------------------------
// OTA Server Init and Tick functions
// -------------------------------------------------------

void emberAfOtaBootloadClusterServerInitCallback(uint8_t endpoint)
{
  emberAfOtaStorageInitCallback();
  otaServerEndpoint = endpoint;
  emAfOtaServerDynamicBlockPeriodInit();
}

void emberAfOtaBootloadClusterServerTickCallback(uint8_t endpoint)
{
  emAfOtaPageRequestTick(endpoint);
}

// This tick is endpointless and declared differently in plugin.properties
void emberAfOtaServerTick(void)
{
  emAfOtaServerDynamicBlockPeriodTick();
}

// -------------------------------------------------------
// OTA Server Handler functions
// -------------------------------------------------------
bool emberAfOtaServerIncomingMessageRawCallback(EmberAfClusterCommand* command)
{
  EmberStatus status;
  if (!commandParse(command)) {
    emberAfOtaBootloadClusterPrintln("ClusterError: failed parsing cmd 0x%x",
                                     command->commandId);
    emberAfOtaBootloadClusterFlush();
    status = emberAfSendDefaultResponse(command, EMBER_ZCL_STATUS_INVALID_FIELD);
  } else {
    // Avoid sending duplicate image block responses if it is an ota page request
    // The page request responses are also handled by sendBlockRequest() in
    // ota-server-page-request.c, we don't sendResponse() again here.
    if (command->commandId == ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID
        && emberAfOtaPageRequestServerPolicyCallback() == EMBER_ZCL_STATUS_SUCCESS) {
      return true;
    }
    status = emberAfSendResponse();
  }

  if (EMBER_SUCCESS != status) {
    emberAfOtaBootloadClusterPrintln("OTA: failed sending response to cmd 0x%x:"
                                     " error 0x%x",
                                     command->commandId,
                                     status);
  }

  // Always return true to indicate we processed the message.
  return true;
}

static uint8_t queryNextImageRequestHandler(const EmberAfOtaImageId* currentImageId,
                                            uint16_t* hardwareVersion)
{
  uint8_t status;
  EmberAfOtaImageId upgradeImageId;

  status = emberAfOtaServerQueryCallback(currentImageId,
                                         hardwareVersion,
                                         &upgradeImageId);

  prepareClusterResponse(ZCL_QUERY_NEXT_IMAGE_RESPONSE_COMMAND_ID,
                         status);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  }

  addEmberAfOtaImageIdIntoResponse(&upgradeImageId);
  emberAfPutInt32uInResp(emberAfOtaStorageGetTotalImageSizeCallback(&upgradeImageId));
  return status;
}

bool emberAfOtaServerSendImageNotifyCallback(EmberNodeId dest,
                                             uint8_t endpoint,
                                             uint8_t payloadType,
                                             uint8_t queryJitter,
                                             const EmberAfOtaImageId* id)
{
  EmberStatus status;

  // Clear the frame since we are initiating the conversation
  MEMSET(&emberAfResponseApsFrame, 0, sizeof(EmberApsFrame));

  emberAfResponseApsFrame.options = 0;
  emberAfResponseDestination = dest;
  emberAfResponseApsFrame.destinationEndpoint = endpoint;
  prepareClusterResponse(ZCL_IMAGE_NOTIFY_COMMAND_ID,
                         payloadType);

  emberAfPutInt8uInResp(queryJitter);

  if (payloadType >= 1) {
    emberAfPutInt16uInResp(id->manufacturerId);
  }
  if (payloadType >= 2) {
    emberAfPutInt16uInResp(id->imageTypeId);
  }
  if (payloadType >= 3) {
    emberAfPutInt32uInResp(id->firmwareVersion);
  }

  if (dest >= EMBER_BROADCAST_ADDRESS) {
    status = emberAfSendBroadcast(dest,
                                  &emberAfResponseApsFrame,
                                  appResponseLength,
                                  appResponseData);
  } else {
    status = emberAfSendUnicast(EMBER_OUTGOING_DIRECT,
                                dest,
                                &emberAfResponseApsFrame,
                                appResponseLength,
                                appResponseData);
  }
  return (status == EMBER_SUCCESS);
}

static void printBlockRequestInfo(const EmberAfOtaImageId* id,
                                  uint8_t maxDataSize,
                                  uint32_t offset)
{
  // To reduce the redundant data printed by the server, it will only print
  // a request for a different image than the last one.  To change this
  // behavior update the bool below.
  const bool printAllRequests = false;

  static EmberAfOtaImageId lastImageId = INVALID_OTA_IMAGE_ID;

  if (!printAllRequests
      && (0 == MEMCOMPARE(id, &lastImageId, sizeof(EmberAfOtaImageId)))) {
    return;
  }
  MEMMOVE(&lastImageId, id, sizeof(EmberAfOtaImageId));

  emberAfOtaBootloadClusterPrintln("NEW ImageBlockReq mfgId:%2x imageTypeId:%2x, file:%4x, maxDataSize:%d, offset:0x%4x",
                                   id->manufacturerId,
                                   id->imageTypeId,
                                   id->firmwareVersion,
                                   maxDataSize,
                                   offset);
  emberAfPluginOtaServerUpdateStartedCallback(id->manufacturerId,
                                              id->imageTypeId,
                                              id->firmwareVersion,
                                              maxDataSize,
                                              offset);
}

// This function is made non-static for the Page request code
// It returns 0 on error, or the number of bytes sent on success.
uint8_t emAfOtaImageBlockRequestHandler(EmberAfImageBlockRequestCallbackStruct* callbackData)
{
  uint8_t data[MAX_POSSIBLE_SERVER_BLOCK_SIZE];
  uint32_t actualLength;
  uint8_t status = EMBER_ZCL_STATUS_SUCCESS;
  uint8_t serverBlockSize = emberAfOtaServerBlockSizeCallback(callbackData->source);
#ifdef EMBER_AF_PLUGIN_SUB_GHZ_SERVER
  EmberDutyCycleState dcState;
#endif

  if (serverBlockSize > MAX_POSSIBLE_SERVER_BLOCK_SIZE) {
    serverBlockSize = MAX_POSSIBLE_SERVER_BLOCK_SIZE;
  }

  // Delay checks
  // We delay the OTA client if the following conditions happen, listed by order
  // 1. The duty cycle state has moved to Critical state (for SE1.4 subghz)
  // 2. We support a dynamically-treated Minimum Block Period, which means we
  //    can only support so many clients downloading at once. When we're at that
  //    max, we tell new clients to delay and come back later
  // 3. The OTA client indicates that it supports the Minimum Block Period, and
  //    its value differs from the value the server holds. We tell it to wait
  //    for data with the updated block period value
  // 4. Test code desires a delay

#ifdef EMBER_AF_PLUGIN_SUB_GHZ_SERVER
  // SE1.4 says that when in Critical Duty Cycle state... "If active, the OTA server
  // shall respond to any Image Block Request command with an Image Block Response
  // command with a status of WAIT_FOR_DATA."
  if (emberGetDutyCycleState(&dcState) == EMBER_SUCCESS
      && dcState >= EMBER_DUTY_CYCLE_LBT_LIMITED_THRESHOLD_REACHED) {
    callbackData->waitTimeSecondsResponse = emberAfPluginSubGhzServerSuspendZclMessagesStatus(callbackData->source);
    status = EMBER_ZCL_STATUS_WAIT_FOR_DATA;
  } else
#endif
  status = emberAfOtaServerImageBlockRequestCallback(callbackData);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    prepareClusterResponse(ZCL_IMAGE_BLOCK_RESPONSE_COMMAND_ID,
                           status);
    if (status == EMBER_ZCL_STATUS_WAIT_FOR_DATA) {
      // Current time (0 = use relative time, not UTC)
      emberAfPutInt32uInResp(0);
      emberAfPutInt32uInResp(callbackData->waitTimeSecondsResponse);

#if defined(EMBER_AF_PLUGIN_OTA_SERVER_MIN_BLOCK_REQUEST_SUPPORT)
      // The min block request period is in milliseconds as defined in af-types.h
      // This attribute, BockRequestDelay, used to be in milliseconds
      // (09-5264-23, section 6.7.10)
      // It now (15-0324-02) is called MinimumBlockPeriod and is defined to be
      // in seconds (section 11.10.10)
      emberAfPutInt16uInResp(callbackData->minBlockRequestPeriod);
#endif

      // If this is a Page Request and we've told the client to delay, we can
      // abort this session. It has to come back with another Image Page Request
      // in the future anyway (NOTE: an Image Page Request gets refunneled into
      // this code as a simulated Image Block Request for processing purposes)
      if (callbackData->bitmask
          & EMBER_AF_IMAGE_BLOCK_REQUEST_SIMULATED_FROM_PAGE_REQUEST) {
        (void)emAfOtaPageRequestErrorHandler(); // only aborts current session
      }
    }
    return 0;
  }

  MEMSET(data, 0, MAX_POSSIBLE_SERVER_BLOCK_SIZE);
  printBlockRequestInfo(callbackData->id,
                        callbackData->maxDataSize,
                        callbackData->offset);

  callbackData->maxDataSize = (callbackData->maxDataSize < serverBlockSize
                               ? callbackData->maxDataSize
                               : serverBlockSize);
  if (EMBER_AF_OTA_STORAGE_SUCCESS
      != emberAfOtaStorageReadImageDataCallback(callbackData->id,
                                                callbackData->offset,
                                                callbackData->maxDataSize,
                                                data,
                                                &actualLength)
      || actualLength == 0) {
    status = EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE;
    emberAfPluginOtaServerUpdateCompleteCallback(callbackData->id->manufacturerId,
                                                 callbackData->id->imageTypeId,
                                                 callbackData->id->firmwareVersion,
                                                 callbackData->source,
                                                 status);
  }

  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    if (!emAfOtaPageRequestErrorHandler()) {
      // If the page request code didn't handle the error (because this code
      // wasn't called due to a page request) then we send a normal
      // response.  We don't generate an error message because in that case
      // we were sending an unsolicited image block response.
      prepareDefaultResponse(status, ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID);
    }
    return 0;
  }

  prepareClusterResponse(ZCL_IMAGE_BLOCK_RESPONSE_COMMAND_ID, status);
  addEmberAfOtaImageIdIntoResponse(callbackData->id);
  emberAfPutInt32uInResp(callbackData->offset);
  emberAfPutInt8uInResp((uint8_t)actualLength);
  emberAfPutBlockInResp(data, actualLength);
  emberAfSetCommandEndpoints(otaServerEndpoint, callbackData->clientEndpoint);

  emberAfPluginOtaServerBlockSentCallback((uint8_t)actualLength,
                                          callbackData->id->manufacturerId,
                                          callbackData->id->imageTypeId,
                                          callbackData->id->firmwareVersion);
  // We can't send more than 128 bytes in a packet so we can safely cast this
  // to a 1-byte number.
  return (uint8_t)actualLength;
}

static void constructUpgradeEndResponse(const EmberAfOtaImageId* imageId,
                                        uint32_t upgradeTime)
{
  prepareClusterResponse(ZCL_UPGRADE_END_RESPONSE_COMMAND_ID,
                         0);      // status code (will ignore)

  appResponseLength--;  // The above function wrote an extra byte which we
                        // don't want because there is no status code for this
                        // message

  addEmberAfOtaImageIdIntoResponse(imageId);

  // We always use relative time.  There is no benefit in using
  // UTC time since the client has to support both.
  emberAfPutInt32uInResp(0);                   // current time
  emberAfPutInt32uInResp(upgradeTime);
}

static void upgradeEndRequestHandler(EmberNodeId source,
                                     uint8_t status,
                                     const EmberAfOtaImageId* imageId)
{
  uint32_t upgradeTime;
  bool goAhead;
  EmberAfStatus defaultRespStatus = EMBER_ZCL_STATUS_SUCCESS;
  emberAfOtaBootloadClusterPrintln("RX UpgradeEndReq status:%x",
                                   status);

  // This callback is considered only informative when the status
  // is a failure.
  goAhead = emberAfOtaServerUpgradeEndRequestCallback(source,
                                                      status,
                                                      &upgradeTime,
                                                      imageId);

  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    if (goAhead) {
      constructUpgradeEndResponse(imageId, upgradeTime);
      emberAfPluginOtaServerUpdateCompleteCallback(imageId->manufacturerId,
                                                   imageId->imageTypeId,
                                                   imageId->firmwareVersion,
                                                   source,
                                                   status);
      return;
    } else {
      defaultRespStatus = EMBER_ZCL_STATUS_ABORT;
    }
  }

  prepareDefaultResponse(defaultRespStatus,
                         ZCL_UPGRADE_END_REQUEST_COMMAND_ID);
}

static void querySpecificFileRequestHandler(uint8_t* requestNodeAddress,
                                            const EmberAfOtaImageId* imageId,
                                            uint16_t currentStackVersion)
{
  // Not supported yet.
  prepareDefaultResponse(EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND,
                         ZCL_QUERY_SPECIFIC_FILE_REQUEST_COMMAND_ID);
}

void emAfOtaServerPrepareResponse(bool useDefaultResponse,
                                  uint8_t commandId,
                                  uint8_t status,
                                  uint8_t defaultResponsePayloadCommandId)
{
  emberAfResponseApsFrame.sourceEndpoint = otaServerEndpoint;
  appResponseLength = 0;
  emberAfResponseApsFrame.clusterId = ZCL_OTA_BOOTLOAD_CLUSTER_ID;

  emberAfResponseApsFrame.options =
    ((emAfOtaServerHandlingPageRequest()
      && commandId == ZCL_IMAGE_BLOCK_RESPONSE_COMMAND_ID)
     ? EMBER_APS_OPTION_NONE
     : EMBER_APS_OPTION_RETRY);

  uint8_t disable_default_response_mask = ZCL_DISABLE_DEFAULT_RESPONSE_MASK;

  if ( ZCL_IMAGE_NOTIFY_COMMAND_ID == commandId ) {
    if ( emberAfResponseDestination < EMBER_BROADCAST_ADDRESS ) {
      //Optional - Not set if sent as unicast, set if sent as broadcast
      disable_default_response_mask = EMBER_AF_DEFAULT_RESPONSE_POLICY_RESPONSES;
    }
  }

  // Assume emberAfResponseApsFrame.destinationEndpoint has already
  // been set based on the framework.  In most cases it is as simple
  // as swapping source and dest endpoints.

  emberAfPutInt8uInResp((useDefaultResponse
                         ? ZCL_GLOBAL_COMMAND
                         : (ZCL_CLUSTER_SPECIFIC_COMMAND
                            | disable_default_response_mask))
                        | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT);
  emberAfPutInt8uInResp(commandId == ZCL_IMAGE_NOTIFY_COMMAND_ID
                        ? emberAfNextSequence()
                        : emberAfIncomingZclSequenceNumber);

  emberAfPutInt8uInResp(commandId);
  if (useDefaultResponse) {
    emberAfPutInt8uInResp(defaultResponsePayloadCommandId);
  }
  emberAfPutInt8uInResp(status);

  // Assume the emberAfAppResponseDestination is already set.
}

static void addEmberAfOtaImageIdIntoResponse(const EmberAfOtaImageId* id)
{
  emberAfPutInt16uInResp(id->manufacturerId);
  emberAfPutInt16uInResp(id->imageTypeId);
  emberAfPutInt32uInResp(id->firmwareVersion);
}

static bool commandParse(EmberAfClusterCommand* command)
{
  uint8_t commandId = command->commandId;
  uint8_t* buffer = command->buffer;
  uint8_t length = command->bufLen;
  uint8_t index = EMBER_AF_ZCL_OVERHEAD;
  EmberAfOtaImageId imageId = INVALID_OTA_IMAGE_ID;

  if (commandId > LAST_MESSAGE_ID
      || (length
          < (EMBER_AF_ZCL_OVERHEAD + emAfOtaMinMessageLengths[commandId]))) {
    return false;
  }
  switch (commandId) {
    case ZCL_QUERY_NEXT_IMAGE_REQUEST_COMMAND_ID:
    case ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID:
    case ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID: {
      uint8_t fieldControl;
      uint32_t offset;
      uint8_t maxDataSize;
      uint16_t hardwareVersion;
      bool hardwareVersionPresent = false;

      fieldControl = emberAfGetInt8u(buffer, index, length);
      index++;
      index += emAfOtaParseImageIdFromMessage(&imageId,
                                              &(buffer[index]),
                                              length);

      if (commandId == ZCL_QUERY_NEXT_IMAGE_REQUEST_COMMAND_ID) {
        if (fieldControl & QUERY_NEXT_IMAGE_HW_VER_PRESENT_MASK) {
          hardwareVersionPresent = true;
          hardwareVersion = emberAfGetInt16u(buffer, index, length);
          index += 2;
        }

        queryNextImageRequestHandler(&imageId,
                                     (hardwareVersionPresent
                                      ? &hardwareVersion
                                      : NULL));
        return true;
      } // else // ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID
        //   or ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID

      offset = emberAfGetInt32u(buffer, index, length);
      index += 4;
      maxDataSize = emberAfGetInt8u(buffer, index, length);
      index++;

      if (commandId == ZCL_IMAGE_BLOCK_REQUEST_COMMAND_ID) {
        EmberAfImageBlockRequestCallbackStruct callbackStruct;
        MEMSET(&callbackStruct, 0, sizeof(EmberAfImageBlockRequestCallbackStruct));

        if (fieldControl & OTA_FIELD_CONTROL_NODE_EUI64_PRESENT_BIT) {
          MEMCOPY(callbackStruct.sourceEui, &(buffer[index]), EUI64_SIZE);
          index += EUI64_SIZE;
        }

        if (fieldControl & OTA_FIELD_CONTROL_MIN_BLOCK_REQUEST_PRESENT_BIT) {
          callbackStruct.minBlockRequestPeriod = emberAfGetInt16u(buffer, index, length);
          callbackStruct.bitmask |= EMBER_AF_IMAGE_BLOCK_REQUEST_MIN_BLOCK_REQUEST_SUPPORTED_BY_CLIENT;
          index += 2;
        }

        callbackStruct.clientEndpoint = command->apsFrame->sourceEndpoint;
        callbackStruct.source = command->source;
        callbackStruct.id = &imageId;
        callbackStruct.offset = offset;
        callbackStruct.maxDataSize = maxDataSize;
        callbackStruct.bitmask |= MIN_BLOCK_REQUEST_SERVER_SUPPORT;

        emAfOtaImageBlockRequestHandler(&callbackStruct);
      } else { // ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID
        uint16_t pageSize;
        uint16_t responseSpacing;
        uint8_t status;

        pageSize = emberAfGetInt16u(buffer, index, length);
        index += 2;

        responseSpacing = emberAfGetInt16u(buffer, index, length);
        index += 2;

        status = emAfOtaPageRequestHandler(command->apsFrame->sourceEndpoint,
                                           otaServerEndpoint,
                                           &imageId,
                                           offset,
                                           maxDataSize,
                                           pageSize,
                                           responseSpacing);

        if (status != EMBER_ZCL_STATUS_SUCCESS) {
          prepareDefaultResponse(status,
                                 ZCL_IMAGE_PAGE_REQUEST_COMMAND_ID);
        }
      }
      return true;
    }
    case ZCL_UPGRADE_END_REQUEST_COMMAND_ID: {
      uint8_t status = emberAfGetInt8u(buffer, index, length);
      index++;
      if (status == EMBER_ZCL_STATUS_SUCCESS) {
        index += emAfOtaParseImageIdFromMessage(&imageId,
                                                &(buffer[index]),
                                                length);
      }
      upgradeEndRequestHandler(command->source, status, &imageId);
      return true;
    }
    case ZCL_QUERY_SPECIFIC_FILE_REQUEST_COMMAND_ID: {
      uint8_t* requestNodeAddress = &(buffer[index]);
      uint16_t currentStackVersion;
      index += 8;  // add 8 to jump over the requestNodeAddress
      index += emAfOtaParseImageIdFromMessage(&imageId,
                                              &(buffer[index]),
                                              length);
      currentStackVersion = emberAfGetInt16u(buffer, index, length);
      index += 2;
      querySpecificFileRequestHandler(requestNodeAddress,
                                      &imageId,
                                      currentStackVersion);
      return true;
    }
  }
  return false;
}

void emberAfOtaServerSendUpgradeCommandCallback(EmberNodeId dest,
                                                uint8_t endpoint,
                                                const EmberAfOtaImageId* id)
{
  EmberStatus status;
  emberAfResponseDestination = dest;
  emberAfResponseApsFrame.destinationEndpoint = endpoint;
  constructUpgradeEndResponse(id,
                              0);  // upgrade time (0 = now)
  status = emberAfSendResponse();
  if (EMBER_SUCCESS != status) {
    emberAfOtaBootloadClusterPrintln("OTA: failed sending upgrade response: "
                                     "error 0x%x", status);
  }
}
