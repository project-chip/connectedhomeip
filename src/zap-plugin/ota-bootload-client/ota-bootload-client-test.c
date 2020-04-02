/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_HAL
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_CORE_WELL_KNOWN
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CLIENT
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_STATIC_FILE
#include "ota-bootload-client-test.h"

#include "stack/core/ember-stack.h"
#include "stack/core/scripted-stub.h"
#include "stack/core/parcel.h"

#include <stdlib.h>

// -----------------------------------------------------------------------------
// Constants

// Conversion the same as in ota-bootload-client.c.
#define DISCOVER_SERVER_PERIOD_MS \
  (EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_DISCOVER_SERVER_PERIOD_MINUTES << 16)

#define SERVER_ADDRESS { 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, \
                         0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, }
#define SERVER_PORT      12345
#define SERVER_UID     { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, }
#define SERVER_ENDPOINT 1

// Conversion the same as in ota-bootload-client.c.
#define QUERY_NEXT_IMAGE_PERIOD_MS \
  (EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_QUERY_NEXT_IMAGE_PERIOD_MINUTES << 16)

// -----------------------------------------------------------------------------
// Globals

const EmberEventData emAppEvents[] = {
  { &emZclOtaBootloadClientEventControl, emZclOtaBootloadClientEventHandler, },
  { NULL, NULL, },
};

static EmberZclOtaBootloadClientServerInfo_t serverInfo = {
  .scheme     = EMBER_ZCL_SCHEME_COAP,
  .address    = { SERVER_ADDRESS },
  .port       = SERVER_PORT,
  .uid        = { SERVER_UID },
  .endpointId = SERVER_ENDPOINT,
};

static const EmberZclOtaBootloadFileSpec_t myFileSpec = { STATIC_IMAGE_DATA_MANUFACTURER_ID, STATIC_IMAGE_DATA_IMAGE_TYPE, STATIC_IMAGE_DATA_FIRMWARE_VERSION };

#define BLOCK_SIZE_LOG 8
#define BLOCK_SIZE (1 << BLOCK_SIZE_LOG)
static uint8_t imageData[] = STATIC_IMAGE_DATA;

#if STATIC_IMAGE_DATA_SIZE % BLOCK_SIZE == 0
  #error STATIC_IMAGE_DATA_SIZE must not be multiple of BLOCK_SIZE for improved testing.
#endif

#define CURRENT_TIME_S 0xABCD1234
#define UPGRADE_DELAY_S 0xACAC
#define UPGRADE_TIME_S (CURRENT_TIME_S + UPGRADE_DELAY_S)

// Conversion the same as in ota-bootload-client.c.
#define UPGRADE_DELAY_MS (UPGRADE_DELAY_S << 10)

// From ota-bootload-client.c.
#define DOWNLOAD_BLOCK_ERROR_BACKOFF_MS 100
#define UPGRADE_END_RESPONSE_BACKOFF_MS 100

// -----------------------------------------------------------------------------
// Scripted definitions

// From zcl-core-message.c
static void defaultCoapResponseHandler(EmberCoapStatus status,
                                       EmberCoapCode code,
                                       EmberCoapReadOptions *options,
                                       uint8_t *payload,
                                       uint16_t payloadLength,
                                       EmberCoapResponseInfo *info)
{
}
EmberStatus emZclSendWithOptions(const EmberZclCoapEndpoint_t *destination,
                                 EmberCoapCode code,
                                 const uint8_t *uriPath,
                                 const EmberCoapOption options[],
                                 uint16_t optionsLength,
                                 const uint8_t *payload,
                                 uint16_t payloadLength,
                                 EmberCoapResponseHandler handler,
                                 void *applicationData,
                                 uint16_t applicationDataLength,
                                 bool skipRetryEvent)
{
  assert(destination->flags & EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG);

  EmberCoapSendInfo info = {
    .nonConfirmed = false,

    .localAddress = { { 0 } }, // use default
    .localPort = 0,        // use default
    .remotePort = destination->port,

    .options = options,
    .numberOfOptions = optionsLength,

    .responseTimeoutMs = 0, // use default

    .responseAppData = applicationData,
    .responseAppDataLength = applicationDataLength,

    .transmitHandler = NULL // unused
  };

  return emberCoapSend(&destination->address,
                       code,
                       uriPath,
                       payload,
                       payloadLength,
                       (handler == NULL
                        ? defaultCoapResponseHandler
                        : handler),
                       &info);
}

// From elsewhere

bool emberIsIpv6UnspecifiedAddress(const EmberIpv6Address *address)
{
  return false;
}

EmberIpv6Address unspecAddr = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
void emberGetGlobalAddresses(const uint8_t *prefix,
                             uint8_t prefixLengthInBits)
{
  emberGetGlobalAddressReturn(&unspecAddr, 0, 0, 0);
}

static EmberCoapResponseHandler discByClusterIdResponseHandler = NULL;
bool emberZclDiscByClusterId(const EmberZclClusterSpec_t *clusterSpec,
                             EmberCoapResponseHandler responseHandler)
{
  discByClusterIdResponseHandler = responseHandler;
  return true;
}

static void callDiscByClusterIdResponse(EmberCoapStatus status,
                                        uint8_t *payload,
                                        uint16_t payloadLength)
{
  assert(discByClusterIdResponseHandler != NULL);

  EmberCoapResponseInfo info = {
    .remoteAddress = { SERVER_ADDRESS },
    .remotePort    = SERVER_PORT,
  };
  discByClusterIdResponseHandler(status,
                                 EMBER_COAP_CODE_EMPTY,
                                 NULL, /* options - whatever */
                                 payload,
                                 payloadLength,
                                 &info);

  if (status == EMBER_COAP_MESSAGE_TIMED_OUT) {
    discByClusterIdResponseHandler = NULL;
  }
}
#define addDiscByClusterIdResponseAction(status, payload, payloadLength) \
  addSimpleAction("DiscByClusterIdResponse %d %d",                       \
                  callDiscByClusterIdResponse,                           \
                  3,                                                     \
                  (status),                                              \
                  (payload),                                             \
                  (payloadLength))

static EmberNetworkStatus currentNetworkStatus = EMBER_NO_NETWORK;
#define addNetworkStatusCallbackAction(newStatus)                 \
  do {                                                            \
    addSimpleAction("NetworkStatusCallback %d %d",                \
                    emZclOtaBootloadClientNetworkStatusCallback,  \
                    3,                                            \
                    (newStatus),                                  \
                    (currentNetworkStatus),                       \
                    (0)); /* EmberJoinFailureReason - whatever */ \
    currentNetworkStatus = newStatus;                             \
  } while (0);

#define makeServerInfoParcel(serverInfo)                        \
  makeMessage("1p2p1",                                          \
              (serverInfo)->scheme,                             \
              makeMessage("m",                                  \
                          (serverInfo)->address.bytes,          \
                          sizeof((serverInfo)->address.bytes)), \
              (serverInfo)->port,                               \
              makeMessage("m",                                  \
                          (serverInfo)->uid.bytes,              \
                          EMBER_ZCL_UID_SIZE),                  \
              (serverInfo)->endpointId)

EmberStatus emberDnsLookup(const uint8_t *domainName,
                           uint8_t domainNameLength,
                           const uint8_t *prefix64,
                           EmberDnsResponseHandler responseHandler,
                           uint8_t *appData,
                           uint16_t appDataLength)
{
  return EMBER_ERR_FATAL;
}

EmberStatus emberZclDtlsManagerGetConnection(const EmberIpv6Address *remoteAddress,
                                             uint16_t remotePort,
                                             EmberDtlsMode mode,
                                             void (*returnHandle)(uint8_t))
{
  return EMBER_ERR_FATAL;
}

EmberStatus emberZclDtlsManagerGetUidBySessionId(const uint8_t sessionId,
                                                 EmberZclUid_t *remoteUid)
{
  return EMBER_ERR_FATAL;
}

uint8_t emberZclDtlsManagerGetSessionIdByUid(const EmberZclUid_t *remoteUid)
{
  return EMBER_NULL_SESSION_ID;
}

bool emberZclOtaBootloadClientExpectSecureOta(void)
{
  return false;
}

bool emberZclOtaBootloadClientSetVersionInfoCallback()
{
  return false;
}

bool emberZclOtaBootloadClientServerHasStaticAddressCallback(EmberZclOtaBootloadClientServerInfo_t *serverInfo)
{
  return false;
}

bool emberZclOtaBootloadClientServerHasDnsNameCallback(EmberZclOtaBootloadClientServerInfo_t *serverInfo)
{
  return false;
}

bool emberZclOtaBootloadClientServerHasDiscByClusterId(const EmberZclClusterSpec_t *clusterSpec, EmberCoapResponseHandler responseHandler)
{
  return emberZclDiscByClusterId(clusterSpec, responseHandler);
}

bool emberZclOtaBootloadClientServerDiscoveredCallback(const EmberZclOtaBootloadClientServerInfo_t *serverInfo)
{
  long *contents
    = functionCallCheck("emberZclOtaBootloadClientServerDiscoveredCallback",
                        "p!",
                        makeServerInfoParcel(serverInfo));
  return (bool)contents[1];
}
#define addServerDiscoveredCallbackCheck(serverInfo, accept)          \
  addSimpleCheck("emberZclOtaBootloadClientServerDiscoveredCallback", \
                 "p!",                                                \
                 makeServerInfoParcel((serverInfo)),                  \
                 (accept))

#define makeFileSpecParcel(fileSpec) \
  makeMessage("m", fileSpec, sizeof(*(fileSpec)))

bool emberZclOtaBootloadClientGetQueryNextImageParametersCallback(EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                                  EmberZclOtaBootloadHardwareVersion_t *hardwareVersion)
{
  long *contents
    = functionCallCheck("emberZclOtaBootloadClientGetQueryNextImageParametersCallback",
                        "!");
  *fileSpec = myFileSpec;
  return (bool)contents[0];
}
#define addGetQueryNextImageParametersCallbackCheck(accept)                      \
  addSimpleCheck("emberZclOtaBootloadClientGetQueryNextImageParametersCallback", \
                 "!",                                                            \
                 (accept))

static EmberZclClusterOtaBootloadServerCommandQueryNextImageResponseHandler queryNextImageResponseHandler = NULL;
EmberStatus emberZclSendClusterOtaBootloadServerCommandQueryNextImageRequest(
  const EmberZclDestination_t *destination,
  const EmberZclClusterOtaBootloadServerCommandQueryNextImageRequest_t *request,
  const EmberZclClusterOtaBootloadServerCommandQueryNextImageResponseHandler handler)
{
  assert(((destination->network.flags & EMBER_ZCL_USE_COAPS_FLAG) != 0)
         == (serverInfo.scheme == EMBER_ZCL_SCHEME_COAPS));
  assert(MEMCOMPARE(destination->network.address.bytes,
                    serverInfo.address.bytes,
                    sizeof(destination->network.address.bytes))
         == 0);
  assert(destination->network.flags & EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG);
  assert(destination->network.port == serverInfo.port);
  assert(destination->application.data.endpointId == serverInfo.endpointId);
  assert(destination->application.type == EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT);

  assert(request->manufacturerId == myFileSpec.manufacturerCode);
  assert(request->imageType == myFileSpec.type);
  assert(request->currentFileVersion == myFileSpec.version);
  assert(request->hardwareVersion == EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL);

  queryNextImageResponseHandler = handler;

  return EMBER_SUCCESS;
}

static void callQueryNextImageResponse(EmberZclMessageStatus_t status,
                                       EmberZclStatus_t imageStatus)
{
  assert(queryNextImageResponseHandler != NULL);

  EmberZclClusterOtaBootloadServerCommandQueryNextImageResponse_t response = {
    .status = imageStatus,
    .fileVersion = myFileSpec.version,
    .imageSize = STATIC_IMAGE_DATA_SIZE,
  };
  uint8_t fileUri[EM_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_LENGTH + 1];
  sprintf(fileUri, EM_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_FORMAT, EM_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI, myFileSpec.manufacturerCode, myFileSpec.type, myFileSpec.version);
  response.fileUri.ptr = fileUri;
  response.fileUri.length = EM_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_LENGTH;
  // CBOR {0: 0, 1: 4660, 2: 22136, 3: 0, 4: 1}
  uint8_t payload[] = { 0xa5, 0x00, 0x00, 0x01, 0x19, 0x12, 0x34, 0x02, 0x19, 0x56, 0x78, 0x03, 0x00, 0x04, 0x01 };
  EmberZclCommandContext_t context = {
    .code = EMBER_COAP_CODE_204_CHANGED,
    .payload = payload,
    .payloadLength = COUNTOF(payload)
  };
  queryNextImageResponseHandler(status, &context, &response);

  if (status == EMBER_ZCL_MESSAGE_STATUS_COAP_RESPONSE) {
    queryNextImageResponseHandler = NULL;
  }
}
#define addQueryNextImageResponseAction(status, imageStatus) \
  addSimpleAction("QueryNextImageResponse %d %d",            \
                  callQueryNextImageResponse,                \
                  2,                                         \
                  (status),                                  \
                  (imageStatus))

bool emberZclOtaBootloadClientStartDownloadCallback(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                    bool existingFile)
{
  long *contents
    = functionCallCheck("emberZclOtaBootloadClientStartDownloadCallback", "!");
  return (bool)contents[0];
}
#define addStartDownloadCallbackCheck(accept)                      \
  addSimpleCheck("emberZclOtaBootloadClientStartDownloadCallback", \
                 "!",                                              \
                 (accept))

static EmberCoapResponseHandler coapSendResponseHandler = NULL;
static void *coapSendApplicationData = NULL;
static uint16_t coapSendApplicationDataLength = 0;
EmberStatus emberCoapSend(const EmberIpv6Address *destination,
                          EmberCoapCode code,
                          const uint8_t *path,
                          const uint8_t *payload,
                          uint16_t payloadLength,
                          EmberCoapResponseHandler responseHandler,
                          const EmberCoapSendInfo *info)
{
  assert(MEMCOMPARE(destination->bytes, serverInfo.address.bytes, 16) == 0);
  assert(code == EMBER_COAP_CODE_GET);
  assert(strncmp((const char *)path, EM_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI, EM_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI_LENGTH) == 0);

  EmberCoapBlockOption blockOption;
  assert(info->numberOfOptions >= 1);
  emberParseBlockOptionValue(info->options[0].intValue, &blockOption);

  long *contents = functionCallCheck("emberCoapSend", "i!", blockOption.number);

  coapSendResponseHandler = responseHandler;
  if (info->responseAppDataLength > 0) {
    coapSendApplicationData = malloc(info->responseAppDataLength);
    MEMCOPY(coapSendApplicationData,
            info->responseAppData,
            info->responseAppDataLength);
    coapSendApplicationDataLength = info->responseAppDataLength;
  }

  return (EmberStatus)contents[1];
}
#define addCoapSendCheck(blockNumber, returnStatus) \
  addSimpleCheck("emberCoapSend", "i!", (blockNumber), (returnStatus))

EmberStatus emberZclRequestBlock(const EmberZclCoapEndpoint_t *destination,
                                 const uint8_t *uriPath,
                                 EmberCoapBlockOption *block2Option,
                                 EmberCoapResponseHandler responseHandler)
{
  long *contents
    = functionCallCheck("emberZclRequestBlock",
                        "ii!",
                        block2Option->logSize,
                        block2Option->number);

  assert(destination->flags & EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG);
  assert(MEMCOMPARE(destination->address.bytes, serverInfo.address.bytes, 16) == 0);
  assert(strncmp((const char *)uriPath, EM_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI, EM_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI_LENGTH) == 0);

  coapSendResponseHandler = responseHandler;

  return (EmberStatus)contents[2];
}
#define addZclRequestBlockCheck(logSize, number, returnStatus) \
  addSimpleCheck("emberZclRequestBlock",                       \
                 "ii!",                                        \
                 (logSize),                                    \
                 (number),                                     \
                 (returnStatus))

static void callCoapSendResponseHandler(EmberCoapStatus status,
                                        EmberCoapCode code,
                                        bool moreBlocks,
                                        uint8_t blockLogSize,
                                        uint8_t blockNumber,
                                        uint8_t *payload,
                                        uint16_t payloadLength,
                                        bool writeSuccess)
{
  assert(coapSendResponseHandler != NULL);
  assert(coapSendApplicationData != NULL || coapSendApplicationDataLength == 0);

  // Cheat with the opague EmberCoapReadOptions data so that we can have a
  // simple implementation for emberReadBlockOption.
  uint8_t blockData[] = { moreBlocks, blockLogSize, blockNumber };
  EmberCoapReadOptions *options = (EmberCoapReadOptions *)blockData;
  EmberCoapResponseInfo info = {
    .localAddress = serverInfo.address,
    .remoteAddress = { { 0 } }, // whatever
    .localPort = serverInfo.port,
    .remotePort = 0, // whatever
    .applicationData = coapSendApplicationData,
    .applicationDataLength = coapSendApplicationDataLength,
  };

  // From ota-bootload-storage-ram.c.
  extern bool emZclOtaBootloadStorageRamWriteSuccess;
  bool previousWriteSuccess = emZclOtaBootloadStorageRamWriteSuccess;
  emZclOtaBootloadStorageRamWriteSuccess = writeSuccess;

  coapSendResponseHandler(status,
                          code,
                          options,
                          payload,
                          payloadLength,
                          &info);

  emZclOtaBootloadStorageRamWriteSuccess = previousWriteSuccess;
}
#define addCoapSendResponseAction(status,               \
                                  code,                 \
                                  moreBlocks,           \
                                  blockLogSize,         \
                                  blockNumber,          \
                                  payload,              \
                                  payloadLength,        \
                                  writeSuccess)         \
  addSimpleAction("CoapSendResponse %d %d %d %d %d %d", \
                  callCoapSendResponseHandler,          \
                  8,                                    \
                  (status),                             \
                  (code),                               \
                  (moreBlocks),                         \
                  (blockLogSize),                       \
                  (blockNumber),                        \
                  (payload),                            \
                  (payloadLength),                      \
                  (writeSuccess))

EmberZclStatus_t emberZclOtaBootloadClientDownloadCompleteCallback(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                                   EmberZclStatus_t status)
{
  long *contents
    = functionCallCheck("emberZclOtaBootloadClientDownloadCompleteCallback",
                        "i!",
                        status);

  assert(fileSpec->manufacturerCode == myFileSpec.manufacturerCode);
  assert(fileSpec->type == myFileSpec.type);
  assert(fileSpec->version == myFileSpec.version);

  return (EmberZclStatus_t)contents[1];
}
#define addDownloadCompleteCallbackCheck(status, returnedStatus)      \
  addSimpleCheck("emberZclOtaBootloadClientDownloadCompleteCallback", \
                 "i!",                                                \
                 (status),                                            \
                 (returnedStatus))

static EmberZclClusterOtaBootloadServerCommandUpgradeEndResponseHandler upgradeEndResponseHandler = NULL;
EmberStatus emberZclSendClusterOtaBootloadServerCommandUpgradeEndRequest(
  const EmberZclDestination_t *destination,
  const EmberZclClusterOtaBootloadServerCommandUpgradeEndRequest_t *request,
  const EmberZclClusterOtaBootloadServerCommandUpgradeEndResponseHandler handler)
{
  assert(((destination->network.flags & EMBER_ZCL_USE_COAPS_FLAG) != 0)
         == (serverInfo.scheme == EMBER_ZCL_SCHEME_COAPS));
  assert(MEMCOMPARE(destination->network.address.bytes,
                    serverInfo.address.bytes,
                    sizeof(destination->network.address.bytes))
         == 0);
  assert(destination->network.flags & EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG);
  assert(destination->network.port == serverInfo.port);
  assert(destination->application.data.endpointId == serverInfo.endpointId);
  assert(destination->application.type == EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT);

  assert(request->manufacturerId == myFileSpec.manufacturerCode);
  assert(request->imageType == myFileSpec.type);
  assert(request->fileVersion == myFileSpec.version);

  functionCallCheck("emberZclSendClusterOtaBootloadServerCommandUpgradeEndRequest", "i!", request->status);

  upgradeEndResponseHandler = handler;

  return EMBER_SUCCESS;
}
#define addUpgradeEndRequestCheck(status, returnedStatus)                        \
  addSimpleCheck("emberZclSendClusterOtaBootloadServerCommandUpgradeEndRequest", \
                 "i!",                                                           \
                 (status),                                                       \
                 (returnedStatus))

static void callUpgradeEndResponse(EmberZclMessageStatus_t status,
                                   uint32_t currentTime,
                                   uint32_t upgradeTime)
{
  assert(upgradeEndResponseHandler != NULL);

  EmberZclClusterOtaBootloadServerCommandUpgradeEndResponse_t response = {
    .manufacturerId = myFileSpec.manufacturerCode,
    .imageType = myFileSpec.type,
    .fileVersion = myFileSpec.version,
    .currentTime = currentTime,
    .upgradeTime = upgradeTime,
  };
  upgradeEndResponseHandler(status, NULL, &response);

  if (status == EMBER_ZCL_MESSAGE_STATUS_COAP_RESPONSE) {
    upgradeEndResponseHandler = NULL;
  }
}
#define addUpgradeEndResponseAction(status, currentTime, upgradeTime) \
  addSimpleAction("UpgradeEndResponse %d %d",                         \
                  callUpgradeEndResponse,                             \
                  3,                                                  \
                  (status),                                           \
                  (currentTime),                                      \
                  (upgradeTime))

void emberZclOtaBootloadClientPreBootloadCallback(const EmberZclOtaBootloadFileSpec_t *fileSpec)
{
  functionCallCheck("emberZclOtaBootloadClientPreBootloadCallback", "");
  assert(fileSpec->manufacturerCode == myFileSpec.manufacturerCode);
  assert(fileSpec->type == myFileSpec.type);
  assert(fileSpec->version == myFileSpec.version);
}
#define addPreBootloadCallbackCheck() \
  addSimpleCheck("emberZclOtaBootloadClientPreBootloadCallback", "")

EmberStatus halAppBootloaderInstallNewImage(void)
{
  functionCallCheck("halAppBootloaderInstallNewImage", "");
  return EMBER_SUCCESS;
}
#define addBootloadCheck() \
  addSimpleCheck("halAppBootloaderInstallNewImage", "")

// -----------------------------------------------------------------------------
// Tests

// -------------------------------------
// Server discovery test

static void discoverServerTest(void)
{
  // If we are not on a network, we don't do anything.
  addRunAction(1);
  runScript();

  // If we lose our network, we don't do anything.
  addNetworkStatusCallbackAction(EMBER_NO_NETWORK);
  addRunAction(100);
  runScript();

  // On network up, we kick off discovery (per discoverServerPeriodMinutes
  // plugin option).
  addNetworkStatusCallbackAction(EMBER_JOINED_NETWORK_ATTACHED);
  addRunAction(DISCOVER_SERVER_PERIOD_MS);
  runScript();

  // If we don't get any responses from our discovery multicast, we should just
  // schedule the discovery for later.
  uint8_t payload[64];
  addRunAction(10);
  addDiscByClusterIdResponseAction(EMBER_COAP_MESSAGE_TIMED_OUT,
                                   payload,
                                   COUNTOF(payload));
  addRunAction(DISCOVER_SERVER_PERIOD_MS);
  runScript();

  // If we get an ACK, we just keep waiting.
  addRunAction(10);
  addDiscByClusterIdResponseAction(EMBER_COAP_MESSAGE_ACKED,
                                   payload,
                                   COUNTOF(payload));
  runScript();

  // When the application says that they don't want to use a server, we keep
  // trying to discover one.
  addRunAction(10);
  addDiscByClusterIdResponseAction(EMBER_COAP_MESSAGE_RESPONSE,
                                   payload,
                                   COUNTOF(payload));
  addServerDiscoveredCallbackCheck(&serverInfo, false);
  addRunAction(10);
  addDiscByClusterIdResponseAction(EMBER_COAP_MESSAGE_TIMED_OUT,
                                   payload,
                                   COUNTOF(payload));
  addRunAction(DISCOVER_SERVER_PERIOD_MS);
  runScript();

  // When the application says that they do want to use a server, we move on to
  // the next state after the TIMEOUT status has come in - this status indicates
  // that all the CoAP multicast responses should have been in by now.
  addRunAction(10);
  addDiscByClusterIdResponseAction(EMBER_COAP_MESSAGE_RESPONSE,
                                   payload,
                                   COUNTOF(payload));
  addServerDiscoveredCallbackCheck(&serverInfo, true);
  addRunAction(10);
  addDiscByClusterIdResponseAction(EMBER_COAP_MESSAGE_TIMED_OUT,
                                   payload,
                                   COUNTOF(payload));
  runScript();
}

// -------------------------------------
// Query next image test

#define DISCOVER_SERVER_SUCCESSFULLY()                           \
  addRunAction(DISCOVER_SERVER_PERIOD_MS);                       \
  addDiscByClusterIdResponseAction(EMBER_COAP_MESSAGE_RESPONSE,  \
                                   payload,                      \
                                   COUNTOF(payload));            \
  addServerDiscoveredCallbackCheck(&serverInfo, true);           \
  addDiscByClusterIdResponseAction(EMBER_COAP_MESSAGE_TIMED_OUT, \
                                   payload,                      \
                                   COUNTOF(payload));            \
  runScript();

#define QUERY_NEXT_IMAGE_SUCCESSFULLY()                                                   \
  addRunAction(QUERY_NEXT_IMAGE_PERIOD_MS - 1);       /* TODO: why do we need -1 here? */ \
  addGetQueryNextImageParametersCallbackCheck(true);                                      \
  runScript();

static void queryNextImageTest(void)
{
  uint8_t payload[64];
  // We are happily set up with our OTA Server.
  addNetworkStatusCallbackAction(EMBER_JOINED_NETWORK_ATTACHED);
  DISCOVER_SERVER_SUCCESSFULLY();

  // If the application says not to send the QueryNextImage, we reschedule
  // it for later.
  addRunAction(QUERY_NEXT_IMAGE_PERIOD_MS - 1); // TODO: why do we need -1 here?
  addGetQueryNextImageParametersCallbackCheck(false);
  runScript();

  // After the application tells us that it wants to query a new image, we
  // receive a response back from the OTA Server. Any negative status means we
  // go and find our server again.
  QUERY_NEXT_IMAGE_SUCCESSFULLY();
  addRunAction(10);
  addQueryNextImageResponseAction(EMBER_ZCL_MESSAGE_STATUS_COAP_TIMEOUT,
                                  EMBER_ZCL_STATUS_SUCCESS);
  DISCOVER_SERVER_SUCCESSFULLY();

  QUERY_NEXT_IMAGE_SUCCESSFULLY();
  addRunAction(10);
  addQueryNextImageResponseAction(EMBER_ZCL_MESSAGE_STATUS_DISCOVERY_TIMEOUT,
                                  EMBER_ZCL_STATUS_SUCCESS);
  DISCOVER_SERVER_SUCCESSFULLY();

  // Now we finally get a successful response, but the server doesn't have
  // any images for us. No problem, we just reschedule another query.
  QUERY_NEXT_IMAGE_SUCCESSFULLY();
  addRunAction(10);
  addQueryNextImageResponseAction(EMBER_ZCL_MESSAGE_STATUS_COAP_RESPONSE,
                                  EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE);
  runScript();

  // If we, for some reasons, get a NOT_AUTHORIZED status back from the server,
  // then go discover another one. This will give the application an opportunity
  // to pick a different server.
  QUERY_NEXT_IMAGE_SUCCESSFULLY();
  addRunAction(10);
  addQueryNextImageResponseAction(EMBER_ZCL_MESSAGE_STATUS_COAP_RESPONSE,
                                  EMBER_ZCL_STATUS_NOT_AUTHORIZED);
  DISCOVER_SERVER_SUCCESSFULLY();

  // Hooray! The server tells us that we finally have an image to download.
  QUERY_NEXT_IMAGE_SUCCESSFULLY();
  addRunAction(10);
  addQueryNextImageResponseAction(EMBER_ZCL_MESSAGE_STATUS_COAP_RESPONSE,
                                  EMBER_ZCL_STATUS_SUCCESS);
  addStartDownloadCallbackCheck(true);
  runScript();
}

// -------------------------------------
// Download test

#define START_DOWNLOAD_SUCCESSFULLY()                                     \
  addQueryNextImageResponseAction(EMBER_ZCL_MESSAGE_STATUS_COAP_RESPONSE, \
                                  EMBER_ZCL_STATUS_SUCCESS);              \
  addStartDownloadCallbackCheck(true);                                    \
  runScript();

#define RECEIVE_BLOCK_SUCCESSFULLY(block)                       \
  addCoapSendResponseAction(EMBER_COAP_MESSAGE_RESPONSE,        \
                            EMBER_COAP_CODE_205_CONTENT,        \
                            true,           /* moreBlocks   */  \
                            BLOCK_SIZE_LOG, /* blockLogSize */  \
                            (block),        /* blockNumber  */  \
                            imageData + ((block) * BLOCK_SIZE), \
                            BLOCK_SIZE,                         \
                            true);          /* writeSuccess */  \
  addZclRequestBlockCheck(BLOCK_SIZE_LOG,                       \
                          (block + 1),                          \
                          EMBER_SUCCESS);                       \
  runScript();

static void downloadTest(void)
{
  uint8_t payload[64];
  // We get set up with a server and an image to download.
  addNetworkStatusCallbackAction(EMBER_JOINED_NETWORK_ATTACHED);
  DISCOVER_SERVER_SUCCESSFULLY();
  QUERY_NEXT_IMAGE_SUCCESSFULLY();
  START_DOWNLOAD_SUCCESSFULLY();

  // We send the first block and fail - we just rechedule the event to try again
  // in a bit. We try again a second time and pass.
  addZclRequestBlockCheck(BLOCK_SIZE_LOG, 0, EMBER_ERR_FATAL);
  addZclRequestBlockCheck(BLOCK_SIZE_LOG, 0, EMBER_SUCCESS);
  runScript();

  // Bring on the blocks. This tests failure cases.
  // These lines assume that there are at least 5 blocks and the error threshold
  // is 7 so that we can test all of these error cases before the client state
  // machine gives up downloading the file.
  assert((STATIC_IMAGE_DATA_SIZE / BLOCK_SIZE) > 4);
  assert(EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_BLOCK_ERROR_THRESHOLD == 7);
  // Block 0 arrives successfully.
  RECEIVE_BLOCK_SUCCESSFULLY(0);
  // Block 1 times out (error 1).
  addCoapSendResponseAction(EMBER_COAP_MESSAGE_TIMED_OUT,
                            EMBER_COAP_CODE_EMPTY, // this shouldn't matter
                            false,        // this shouldn't matter
                            0,            // this shouldn't matter
                            0,            // this shouldn't matter
                            0,            // this shouldn't matter
                            0,            // this shouldn't matter
                            false);       // this shouldn't matter
  addRunAction(DOWNLOAD_BLOCK_ERROR_BACKOFF_MS - 1);
  addZclRequestBlockCheck(BLOCK_SIZE_LOG, 1, EMBER_SUCCESS);
  // Block 1 has some response code that we don't recognize (error 2, 3).
  addCoapSendResponseAction(EMBER_COAP_MESSAGE_RESPONSE,
                            EMBER_COAP_CODE_500_INTERNAL_SERVER_ERROR,
                            false,         // this shouldn't matter
                            0,             // this shouldn't matter
                            0,             // this shouldn't matter
                            0,             // this shouldn't matter
                            0,             // this shouldn't matter
                            false);        // this shouldn't matter
  addRunAction(DOWNLOAD_BLOCK_ERROR_BACKOFF_MS - 1);
  addZclRequestBlockCheck(BLOCK_SIZE_LOG, 1, EMBER_SUCCESS);
  addCoapSendResponseAction(EMBER_COAP_MESSAGE_RESPONSE,
                            EMBER_COAP_CODE_503_SERVICE_UNAVAILABLE,
                            false,          // this shouldn't matter
                            0,              // this shouldn't matter
                            0,              // this shouldn't matter
                            0,              // this shouldn't matter
                            0,              // this shouldn't matter
                            false);         // this shouldn't matter
  addRunAction(DOWNLOAD_BLOCK_ERROR_BACKOFF_MS - 1);
  addZclRequestBlockCheck(BLOCK_SIZE_LOG, 1, EMBER_SUCCESS);
  // Block 1 arrives successfully.
  RECEIVE_BLOCK_SUCCESSFULLY(1);
  // Block 2 is sent with wrong block option parameters (error 4, 5).
  addCoapSendResponseAction(EMBER_COAP_MESSAGE_RESPONSE,
                            EMBER_COAP_CODE_205_CONTENT,
                            true,           // moreBlocks
                            1,              // blockLogSize
                            2,              // blockNumber
                            imageData + (2 * BLOCK_SIZE),
                            BLOCK_SIZE,
                            false);         // this shouldn't matter
  addRunAction(DOWNLOAD_BLOCK_ERROR_BACKOFF_MS - 1);
  addZclRequestBlockCheck(BLOCK_SIZE_LOG, 2, EMBER_SUCCESS);
  addCoapSendResponseAction(EMBER_COAP_MESSAGE_RESPONSE,
                            EMBER_COAP_CODE_205_CONTENT,
                            true,           // moreBlocks
                            BLOCK_SIZE_LOG, // blockLogSize
                            2,              // blockNumber
                            imageData + (2 * BLOCK_SIZE),
                            BLOCK_SIZE - 2,
                            false);         // this shouldn't matter
  // We receive block 2, but fail to write it two times (error 6, 7).
  addCoapSendResponseAction(EMBER_COAP_MESSAGE_RESPONSE,
                            EMBER_COAP_CODE_205_CONTENT,
                            true,           // moreBlocks
                            BLOCK_SIZE_LOG, // blockLogSize
                            2,              // blockNumber
                            imageData + (2 * BLOCK_SIZE),
                            BLOCK_SIZE,
                            false);         // writeSuccess
  addRunAction(DOWNLOAD_BLOCK_ERROR_BACKOFF_MS - 1);
  addZclRequestBlockCheck(BLOCK_SIZE_LOG, 2, EMBER_SUCCESS);
  addCoapSendResponseAction(EMBER_COAP_MESSAGE_RESPONSE,
                            EMBER_COAP_CODE_205_CONTENT,
                            true,           // moreBlocks
                            BLOCK_SIZE_LOG, // blockLogSize
                            2,              // blockNumber
                            imageData + (2 * BLOCK_SIZE),
                            BLOCK_SIZE,
                            false);         // writeSuccess
  // We give up and pass the bad news on to the application.
  addDownloadCompleteCallbackCheck(EMBER_ZCL_STATUS_ABORT,
                                   EMBER_ZCL_STATUS_ABORT);
  addUpgradeEndRequestCheck(EMBER_ZCL_STATUS_ABORT, EMBER_ZCL_STATUS_ABORT);
  runScript();

  // We start back up the upgrade process with querying the next image.
  QUERY_NEXT_IMAGE_SUCCESSFULLY();
  START_DOWNLOAD_SUCCESSFULLY();

  // When we start the download again, we try to pick up where we left off,
  // i.e., on block 3.
  for (size_t i = 2; i < STATIC_IMAGE_DATA_SIZE / BLOCK_SIZE; i++) {
    // Getting an ACK means that we just keep on moving.
    if (i % 3 == 0) {
      addCoapSendResponseAction(EMBER_COAP_MESSAGE_ACKED,
                                EMBER_COAP_CODE_EMPTY, // this shouldn't matter
                                false,                 // this shouldn't matter
                                0,                     // this shouldn't matter
                                0,                     // this shouldn't matter
                                0,                     // this shouldn't matter
                                0,                     // this shouldn't matter
                                false);                // this shouldn't matter
    }
    RECEIVE_BLOCK_SUCCESSFULLY(i);
  }

  // We should have one last block to request, which is lastBlockSize large.
  size_t lastBlockSize = STATIC_IMAGE_DATA_SIZE % BLOCK_SIZE;
  addCoapSendResponseAction(EMBER_COAP_MESSAGE_RESPONSE,
                            EMBER_COAP_CODE_205_CONTENT,
                            false,                   // moreBlocks
                            BLOCK_SIZE_LOG,          // blockLogSize
                            STATIC_IMAGE_DATA_SIZE / BLOCK_SIZE, // blockNumber
                            imageData + ((((size_t)STATIC_IMAGE_DATA_SIZE / BLOCK_SIZE)) * BLOCK_SIZE), \
                            lastBlockSize,
                            true);                   // writeSuccess
  runScript();

  addRunAction(UPGRADE_END_RESPONSE_BACKOFF_MS);
  // A successful download!
  addDownloadCompleteCallbackCheck(EMBER_ZCL_STATUS_SUCCESS, EMBER_ZCL_STATUS_SUCCESS);
  addUpgradeEndRequestCheck(EMBER_ZCL_STATUS_SUCCESS, EMBER_ZCL_STATUS_SUCCESS);
  runScript();
}

// -------------------------------------
// Verify download test

#define FINISH_DOWNLOAD_SUCCESSFULLY()                                                                  \
  addZclRequestBlockCheck(BLOCK_SIZE_LOG, 0, EMBER_SUCCESS);                                            \
  for (size_t i = 0; i < STATIC_IMAGE_DATA_SIZE / BLOCK_SIZE; i++) {                                    \
    RECEIVE_BLOCK_SUCCESSFULLY(i);                                                                      \
  }                                                                                                     \
  size_t lastBlockSize = STATIC_IMAGE_DATA_SIZE % BLOCK_SIZE;                                           \
  addCoapSendResponseAction(EMBER_COAP_MESSAGE_RESPONSE,                                                \
                            EMBER_COAP_CODE_205_CONTENT,                                                \
                            false,                   /* moreBlocks   */                                 \
                            BLOCK_SIZE_LOG,          /* blockLogSize */                                 \
                            STATIC_IMAGE_DATA_SIZE / BLOCK_SIZE, /* blockNumber  */                     \
                            imageData + ((((size_t)STATIC_IMAGE_DATA_SIZE / BLOCK_SIZE)) * BLOCK_SIZE), \
                            lastBlockSize,                                                              \
                            true);                   /* writeSuccess */                                 \
  runScript();

static void verifyDownloadTest(void)
{
  uint8_t payload[64];
  // We have currently downloaded an image successfully.
  addNetworkStatusCallbackAction(EMBER_JOINED_NETWORK_ATTACHED);
  DISCOVER_SERVER_SUCCESSFULLY();
  QUERY_NEXT_IMAGE_SUCCESSFULLY();
  START_DOWNLOAD_SUCCESSFULLY();
  FINISH_DOWNLOAD_SUCCESSFULLY();

  addRunAction(UPGRADE_END_RESPONSE_BACKOFF_MS);
  // A successful verification! But if the application says no, we go back and
  // download the image again.
  addDownloadCompleteCallbackCheck(EMBER_ZCL_STATUS_SUCCESS, EMBER_ZCL_STATUS_SUCCESS);
  addUpgradeEndRequestCheck(EMBER_ZCL_STATUS_SUCCESS, EMBER_ZCL_STATUS_SUCCESS);
  runScript();
}

// -------------------------------------
// Upgrade end

static void upgradeEndTest(void)
{
  uint8_t payload[64];
  // We downloaded and verified an image successfully.
  addNetworkStatusCallbackAction(EMBER_JOINED_NETWORK_ATTACHED);
  DISCOVER_SERVER_SUCCESSFULLY();
  QUERY_NEXT_IMAGE_SUCCESSFULLY();
  START_DOWNLOAD_SUCCESSFULLY();
  FINISH_DOWNLOAD_SUCCESSFULLY();

  // We send an upgrade end request after a backoff. We abide by the normal
  // rules when we get ACK's and TIMEOUT's - just try again or wait for a
  // response.
  addRunAction(UPGRADE_END_RESPONSE_BACKOFF_MS);
  addDownloadCompleteCallbackCheck(EMBER_ZCL_STATUS_SUCCESS, EMBER_ZCL_STATUS_SUCCESS);
  addUpgradeEndRequestCheck(EMBER_ZCL_STATUS_SUCCESS, EMBER_ZCL_STATUS_SUCCESS);
  addUpgradeEndResponseAction(EMBER_ZCL_MESSAGE_STATUS_DISCOVERY_TIMEOUT,
                              CURRENT_TIME_S,
                              UPGRADE_TIME_S);
  addRunAction(UPGRADE_END_RESPONSE_BACKOFF_MS - 1); // TODO: why do we need -1 here?
  addDownloadCompleteCallbackCheck(EMBER_ZCL_STATUS_SUCCESS, EMBER_ZCL_STATUS_SUCCESS);
  addUpgradeEndRequestCheck(EMBER_ZCL_STATUS_SUCCESS, EMBER_ZCL_STATUS_SUCCESS);
  addUpgradeEndResponseAction(EMBER_ZCL_MESSAGE_STATUS_COAP_TIMEOUT,
                              CURRENT_TIME_S,
                              UPGRADE_TIME_S);
  addRunAction(UPGRADE_END_RESPONSE_BACKOFF_MS - 1); // TODO: why do we need -1 here?
  addDownloadCompleteCallbackCheck(EMBER_ZCL_STATUS_SUCCESS, EMBER_ZCL_STATUS_SUCCESS);
  addUpgradeEndRequestCheck(EMBER_ZCL_STATUS_SUCCESS, EMBER_ZCL_STATUS_SUCCESS);
  addUpgradeEndResponseAction(EMBER_ZCL_MESSAGE_STATUS_COAP_ACK,
                              CURRENT_TIME_S,
                              UPGRADE_TIME_S);
  runScript();

  // We finally get a good response back that says to upgrade now. The bootload
  // callback should be called for whenever the server said to upgrade.
  addUpgradeEndResponseAction(EMBER_ZCL_MESSAGE_STATUS_COAP_RESPONSE,
                              CURRENT_TIME_S,
                              UPGRADE_TIME_S);
  addRunAction(UPGRADE_DELAY_MS - 1); // TODO: why do we need -1 here?
  addPreBootloadCallbackCheck();
  addBootloadCheck();
  runScript();
}

// -------------------------------------
// Main

int main(int argc, char *argv[])
{
  emZclOtaBootloadClientInitCallback();

  // From ota-bootload-storage-ram.c.
  extern void emZclOtaBootloadStorageInitCallback(void);
  emZclOtaBootloadStorageInitCallback();

  Test tests[] = {
    { "discover-server", discoverServerTest, },
    { "query-next-image", queryNextImageTest, },
    { "download", downloadTest, },
    { "verify-download", verifyDownloadTest, },
    { "upgrade-end", upgradeEndTest, },
    { NULL, NULL, },
  };
  (*parseTestArgument(argc, argv, tests))();

  fprintf(stderr, " done ]\n");

  return 0;
}
