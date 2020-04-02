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
#include EMBER_AF_API_STACK
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_CORE_WELL_KNOWN
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CLIENT
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE
#include EMBER_AF_API_ZCL_CORE_DTLS_MANAGER

#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#else
  #define emberAfPluginOtaBootloadClientPrint(...)
  #define emberAfPluginOtaBootloadClientPrintln(...)
  #define emberAfPluginOtaBootloadClientFlush()
  #define emberAfPluginOtaBootloadClientDebugExec(x)
  #define emberAfPluginOtaBootloadClientPrintBuffer(buffer, len, withSpace)
  #define emberAfPluginOtaBootloadClientPrintString(buffer)
#endif

#ifdef EMBER_SCRIPTED_TEST
  #include "ota-bootload-client-test.h"
#else
  #include "thread-callbacks.h"
#endif

// TODO: if the network goes down, set the state to NONE and erase global data.
// TODO: handle ImageNotify command.
// TODO: should we use the event-queue system to pass global data around?
// TODO: run the ota-bootload-client-test.c with eeprom storage implementation.
// TODO: add callback for each block rx? So app can resize/backoff next block?

// -----------------------------------------------------------------------------
// Constants

#define MAX_JITTER_MS (MILLISECOND_TICKS_PER_SECOND * 10)

// Larger blocksizes produce shorter OTA times
#define DEFAULT_BLOCK_SIZE_LOG 8 /* 256 */

#define DOWNLOAD_BLOCK_ERROR_BACKOFF_MS 100
#define UPGRADE_END_RESPONSE_BACKOFF_MS 100

// -----------------------------------------------------------------------------
// Types

enum {
  DELAY_TYPE_MILLISECONDS = 0x00,
  DELAY_TYPE_SECONDS      = 0x01,
  DELAY_TYPE_MINUTES      = 0x02,
};
typedef uint8_t DelayType;

enum {
  STATE_NONE,
  STATE_DISCOVER_SERVER,
  STATE_CREATE_DTLS_SESSION,
  STATE_QUERY_NEXT_IMAGE,
  STATE_WAIT_FOR_DELETE,
  STATE_DOWNLOAD,
  STATE_VERIFY_DOWNLOAD,
  STATE_UPGRADE_END,
  STATE_BOOTLOAD,
};
typedef uint8_t State_t;
static State_t state;

// -----------------------------------------------------------------------------
// Globals

EmberEventControl emZclOtaBootloadClientEventControl;

static EmberZclOtaBootloadClientServerInfo_t serverInfo;
#define HAVE_SERVER_INFO()  (serverInfo.endpointId != EMBER_ZCL_ENDPOINT_NULL)
#define ERASE_SERVER_INFO() do { serverInfo.endpointId = EMBER_ZCL_ENDPOINT_NULL; curBlockSizeLog = DEFAULT_BLOCK_SIZE_LOG; } while (0)

static EmberZclOtaBootloadFileSpec_t fileSpecToDownload;
#define HAVE_FILE_SPEC_TO_DOWNLOAD()  (fileSpecToDownload.version != EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION_NULL)
#define ERASE_FILE_SPEC_TO_DOWNLOAD() do { fileSpecToDownload.version = EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION_NULL; } while (0)

static EmberZclStatus_t downloadStatus;
#define HAVE_DOWNLOAD_STATUS() (downloadStatus != EMBER_ZCL_STATUS_NULL)
#define ERASE_DOWNLOAD_STATUS() do { downloadStatus = EMBER_ZCL_STATUS_NULL; } while (0)

static bool ipAddressResolved;

static bool immediateUpgrade = false;

// This has to accomodate the leading '/' in the URI. The URI is custom to each
// vendor, thus it could be any length. For now we just base the length off of
// what our OTA server sends back.
#define FILE_URI_LENGTH 32
#if FILE_URI_LENGTH < (EM_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_LENGTH + 1)
  #error Length of the URI buffer is not large enough
#endif
static uint8_t fileUri[FILE_URI_LENGTH];

static uint8_t curBlockSizeLog = DEFAULT_BLOCK_SIZE_LOG;

// -----------------------------------------------------------------------------
// Private API

static void scheduleCreateDtlsSession(bool addJitter);
static EmberStatus schedulePostDiscoverServerState();
static void scheduleQueryNextImage(bool addJitter);
static void queryNextImageResponseHandler(EmberZclMessageStatus_t status,
                                          const EmberZclCommandContext_t *context,
                                          const EmberZclClusterOtaBootloadServerCommandQueryNextImageResponse_t *response);
static void processQueryNextImageResponse(const EmberCoapCode code, const EmberZclClusterOtaBootloadServerCommandQueryNextImageResponse_t *response);
static void blockResponseHandler(EmberCoapStatus status,
                                 EmberCoapCode code,
                                 EmberCoapReadOptions *options,
                                 uint8_t *payload,
                                 uint16_t payloadLength,
                                 EmberCoapResponseInfo *info);
static void scheduleUpgradeEnd(void);
static void upgradeEndResponseHandler(EmberZclMessageStatus_t status,
                                      const EmberZclCommandContext_t *context,
                                      const EmberZclClusterOtaBootloadServerCommandUpgradeEndResponse_t *response);
static void processUpgradeEndResponse(const EmberZclClusterOtaBootloadServerCommandUpgradeEndResponse_t *response);

static uint32_t convertToMs(uint32_t delay,
                            DelayType delayType,
                            bool addJitter)
{
  const uint8_t delayShifts[] = {
    0,  // DELAY_TYPE_MILLISECONDS
    10, // DELAY_TYPE_SECONDS
    16, // DELAY_TYPE_MINUTES
  };
  assert(delayType < COUNTOF(delayShifts));
  delay <<= delayShifts[delayType];

  if (addJitter) {
    delay += (halCommonGetRandom() % MAX_JITTER_MS);
  }

  return delay;
}

static void setState(State_t newState, uint32_t delayMs)
{
  state = newState;
  emberEventControlSetDelayMS(emZclOtaBootloadClientEventControl, delayMs);
}

static void serverInfoToZclDestination(EmberZclDestination_t *destination)
{
  // TODO: send this command to an IPV6 address until UID discovery works.
  assert(HAVE_SERVER_INFO());
  destination->network.address = serverInfo.address;
  destination->network.flags =
    (serverInfo.scheme == EMBER_ZCL_SCHEME_COAPS
     ? (EMBER_ZCL_USE_COAPS_FLAG | EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG | EMBER_ZCL_HAVE_UID_FLAG)
     : EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG);
  destination->network.uid = serverInfo.uid;
  destination->network.port = serverInfo.port;
  destination->application.data.endpointId = serverInfo.endpointId;
  destination->application.type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;
}

void emberZclClusterOtaBootloadClientCommandImageNotifyRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadClientCommandImageNotifyRequest_t *request)
{
  emberAfPluginOtaBootloadClientPrintln("ImageNotifyRequest received");
  EmberZclOtaBootloadFileSpec_t fileSpec;
  EmberZclOtaBootloadHardwareVersion_t hardwareVersion
    = EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL;
  if ((request->payloadType != 0x00)
      && !emberZclOtaBootloadClientGetQueryNextImageParametersCallback(&fileSpec,
                                                                       &hardwareVersion)) {
    return;
  }

  switch (request->payloadType) {
    case 0x03: // check version + image type + manufID
      if (request->newFileVersion == fileSpec.version) {
        return;
      }
    // intentional fallthrough
    case 0x02: // check image type + manufID
      if (request->imageType != fileSpec.type) {
        return;
      }
    // intentional fallthrough
    case 0x01: // check manufacturer ID
      if (request->manufacturerId != fileSpec.manufacturerCode) {
        return;
      }
    // intentional fallthrough
    case 0x00: // check Jitter
      break;
  }
  uint8_t requestJitter = (halCommonGetRandom() % 100) + 1;
  emberAfPluginOtaBootloadClientPrintln("ImageNotifyRequest match. Jitter: %u <= %u", requestJitter, request->queryJitter);
  if (requestJitter <= request->queryJitter) {
    ERASE_SERVER_INFO();
    setState(STATE_DISCOVER_SERVER, requestJitter);
    immediateUpgrade = true;
  }
}

static void noneStateHandler(void)
{
}

// -------------------------------------
// OTA server discovery

static void scheduleDiscoverServer(bool eraseServerInfo, bool addJitter)
{
  if (eraseServerInfo) {
    ERASE_SERVER_INFO();
  }
  setState(STATE_DISCOVER_SERVER,
           convertToMs(EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_DISCOVER_SERVER_PERIOD_MINUTES,
                       DELAY_TYPE_MINUTES,
                       addJitter));
  immediateUpgrade = false;
}

static void dnsLookupResponseHandler(EmberDnsLookupStatus status,
                                     const uint8_t *domainName,
                                     uint8_t domainNameLength,
                                     const EmberDnsResponse *response,
                                     void *applicationData,
                                     uint16_t applicationDataLength)
{
  emberAfPluginOtaBootloadClientPrintln("dnsLookupResponseHandler: %u", status);
  switch (status) {
    case EMBER_DNS_LOOKUP_SUCCESS:
      serverInfo.address = response->ipAddress;
      emberAfPluginOtaBootloadClientPrint("Found via DNS: ");
      emberAfPluginOtaBootloadClientDebugExec(emberAfPrintIpv6Address(&serverInfo.address));
      emberAfPluginOtaBootloadClientPrintln("->%u", serverInfo.port);
      if (schedulePostDiscoverServerState() == EMBER_SUCCESS) {
        return;
      }
      emberAfPluginOtaBootloadClientPrintln("No suitable OTA server at address");
      scheduleDiscoverServer(true, false); // Let's try again
      break;
    case EMBER_DNS_LOOKUP_NO_BORDER_ROUTER:
    case EMBER_DNS_LOOKUP_NO_BORDER_ROUTER_RESPONSE:
    case EMBER_DNS_LOOKUP_NO_DNS_RESPONSE:
      emberAfPluginOtaBootloadClientPrintln("No DNS response: %u", status);
      scheduleDiscoverServer(true, false); // Let's try again
      break;
    case EMBER_DNS_LOOKUP_BORDER_ROUTER_RESPONSE_ERROR:
    case EMBER_DNS_LOOKUP_NO_DNS_RESPONSE_ERROR:
      emberAfPluginOtaBootloadClientPrintln("DNS Error: %u", status);
      scheduleDiscoverServer(true, false); // Let's try again
      break;
    case EMBER_DNS_LOOKUP_NO_DNS_SERVER:
    case EMBER_DNS_LOOKUP_NO_ENTRY_FOR_NAME:
    case EMBER_DNS_LOOKUP_NO_BUFFERS:
      emberAfPluginOtaBootloadClientPrintln("Can't lookup host: %u", status);
      scheduleDiscoverServer(true, false); // Let's try again
      break;
    default:
      assert(false);
  }
}

static void discoverServerResponseHandler(EmberCoapStatus status,
                                          EmberCoapCode code,
                                          EmberCoapReadOptions *options,
                                          uint8_t *payload,
                                          uint16_t payloadLength,
                                          EmberCoapResponseInfo *info)
{
  if (HAVE_SERVER_INFO()) { // We have already found a server to download from
    return;
  }
  switch (status) {
    case EMBER_COAP_MESSAGE_TIMED_OUT:
    case EMBER_COAP_MESSAGE_RESET:
      // Since our discovery is multicast, we get the TIMED_OUT status when we
      // are done receiving responses (note that we could have received no
      // responses). If we found a server, we should have already moved on to the
      // next state. Otherwise, then we keep trying to discover a server.
      break;
    case EMBER_COAP_MESSAGE_ACKED:
      // This means that our request was ACK'd, but the actual response is coming.
      return;
    case EMBER_COAP_MESSAGE_RESPONSE:
    {
      if (payloadLength == 0) {
        break;
      }

      // TODO: shouldn't the UID be contained somewhere in this response?
      // TODO: how do we get the endpoint from the response?
      serverInfo.scheme = EMBER_ZCL_SCHEME_COAP;
      serverInfo.address = info->remoteAddress;
      serverInfo.port = info->remotePort;
      MEMSET(serverInfo.uid.bytes, 0x00, EMBER_ZCL_UID_SIZE);
      serverInfo.endpointId = 1;
      emberAfPluginOtaBootloadClientPrint("Discovered server: ");
      emberAfPluginOtaBootloadClientDebugExec(emberAfPrintIpv6Address(&serverInfo.address));
      emberAfPluginOtaBootloadClientPrintln("->%u", serverInfo.port);
      if (schedulePostDiscoverServerState() == EMBER_SUCCESS) {
        return;
      }
      emberAfPluginOtaBootloadClientPrintln("Discovered server was filtered out.");
      ERASE_SERVER_INFO();
      break;
    }
    default:
      assert(false);
  }

  scheduleDiscoverServer(false, false); // eraseServerInfo? addJitter?
  return;
}

EmberStatus schedulePostDiscoverServerState()
{
  emberAfPluginOtaBootloadClientPrintln("Scheduling post discoverServerState");
  if (emberZclOtaBootloadClientExpectSecureOta()) {
    scheduleCreateDtlsSession(false); // addJitter?
    return EMBER_SUCCESS;
  } else {
    emberAfPluginOtaBootloadClientPrintln("Don't need secure OTA");
  }
  if (emberZclOtaBootloadClientServerDiscoveredCallback(&serverInfo)) {
    scheduleQueryNextImage(false);
    return EMBER_SUCCESS;
  } else {
    emberAfPluginOtaBootloadClientPrintln("ServerInfo rejected by policy");
  }
  return EMBER_ERR_FATAL;
}

void emberGetGlobalAddressReturn(const EmberIpv6Address *address,
                                 uint32_t preferredLifetime,
                                 uint32_t validLifetime,
                                 uint8_t addressFlags)
{
  if (ipAddressResolved) {
    return;
  }
  if (emberIsIpv6UnspecifiedAddress(address)) {
    emberAfPluginOtaBootloadClientPrintln("DNS Lookup: Failed to get Global Address");
    scheduleDiscoverServer(true, false); // addJitter?
    return;
  }
  if (emberDnsLookup(serverInfo.name,
                     serverInfo.nameLength,
                     address->bytes,
                     dnsLookupResponseHandler,
                     NULL,
                     0) != EMBER_SUCCESS) {
    emberAfPluginOtaBootloadClientPrintln("DNS Lookup: Failed to lookup hostname");
    scheduleDiscoverServer(true, false); // addJitter?
  } else {
    ipAddressResolved = true;
  }
}

static void discoverServerStateHandler(void)
{
  if (HAVE_SERVER_INFO()) { // we have the server, go to next state.
    scheduleQueryNextImage(false); // addJitter?
  } else if ( emberZclOtaBootloadClientServerHasStaticAddressCallback(&serverInfo)) {
    emberAfPluginOtaBootloadClientPrint("Using static OTA server IP: ");
    emberAfPluginOtaBootloadClientDebugExec(emberAfPrintIpv6Address(&serverInfo.address));
    emberAfPluginOtaBootloadClientPrintln("->%u", serverInfo.port);
    if (schedulePostDiscoverServerState() == EMBER_SUCCESS) {
      return;
    }
  } else if ( emberZclOtaBootloadClientServerHasDnsNameCallback(&serverInfo) ) {
    emberAfPluginOtaBootloadClientPrintln("Using DNS lookup");
    ipAddressResolved = false;
    emberGetGlobalAddresses(NULL, 0); // force an update of the global address
  } else if (emberZclOtaBootloadClientServerHasDiscByClusterId(&emberZclClusterOtaBootloadServerSpec, discoverServerResponseHandler)) {
    emberAfPluginOtaBootloadClientPrintln("Using upgrade server discovery");
  } else {
    emberAfPluginOtaBootloadClientPrintln("Failed to discoverServer");
    scheduleDiscoverServer(true, false); // addJitter?
  }
}

void otaBootloadClientForceUpdate()
{
  ERASE_SERVER_INFO();
  setState(STATE_DISCOVER_SERVER, 0);
  immediateUpgrade = true;
}
// -------------------------------------
// Create DTLS Session

static void scheduleCreateDtlsSession(bool addJitter)
{
  setState(STATE_CREATE_DTLS_SESSION,
           convertToMs(0, DELAY_TYPE_MINUTES, addJitter));
}

void dtlsSessionIdReturn(uint8_t sessionId)
{
  emberAfPluginOtaBootloadClientPrintln("DTLS Session ID was returned: %u", sessionId);
  if (sessionId != EMBER_NULL_SESSION_ID) {
    serverInfo.scheme = EMBER_ZCL_SCHEME_COAPS;
    emberAfPluginOtaBootloadClientPrintln("DTLS session has been established: %u", sessionId);
  }
  if ((emberZclDtlsManagerGetUidBySessionId(sessionId, &serverInfo.uid) != EMBER_SUCCESS)
      || (!emberZclOtaBootloadClientServerDiscoveredCallback(&serverInfo))) {
    emberAfPluginOtaBootloadClientPrintln("UID or ServerInfo mismatch");
    ERASE_SERVER_INFO();
    scheduleDiscoverServer(true, false); // addJitter?
    return;
  }
  scheduleQueryNextImage(false);
}

static void createDtlsSessionStateHandler(void)
{
  if (!emberZclOtaBootloadIsWildcard(serverInfo.uid.bytes, sizeof(serverInfo.uid.bytes))) {
    emberAfPluginOtaBootloadClientPrint("Looking up DTLS session by UID...");
    uint8_t sessionId = emberZclDtlsManagerGetSessionIdByUid(&serverInfo.uid, serverInfo.port);
    if (sessionId != EMBER_NULL_SESSION_ID) {
      emberAfPluginOtaBootloadClientPrintln("success");
      dtlsSessionIdReturn(sessionId);
      return;
    } else {
      emberAfPluginOtaBootloadClientPrintln("fail");
    }
  }

  emberAfPluginOtaBootloadClientPrintln("Creating DTLS session to server");
  emberZclDtlsManagerGetConnection(&(serverInfo.address), serverInfo.port, EMBER_DTLS_MODE_CERT, dtlsSessionIdReturn);
}

// -------------------------------------
// Query Next Image

static void scheduleQueryNextImage(bool addJitter)
{
  setState(STATE_QUERY_NEXT_IMAGE,
           convertToMs(immediateUpgrade ? 0 : EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_QUERY_NEXT_IMAGE_PERIOD_MINUTES,
                       DELAY_TYPE_MINUTES,
                       addJitter));
}

static void queryNextImageStateHandler(void)
{
  EmberZclOtaBootloadFileSpec_t fileSpec;
  EmberZclOtaBootloadHardwareVersion_t hardwareVersion
    = EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL;
  if (!emberZclOtaBootloadClientGetQueryNextImageParametersCallback(&fileSpec,
                                                                    &hardwareVersion)) {
    scheduleQueryNextImage(false); // addJitter?
    return;
  }
  fileSpecToDownload.manufacturerCode = fileSpec.manufacturerCode;
  fileSpecToDownload.type = fileSpec.type;

  // TODO: should we create our own constants for this fieldControl value?
  EmberZclDestination_t destination;
  serverInfoToZclDestination(&destination);
  EmberZclClusterOtaBootloadServerCommandQueryNextImageRequest_t request = {
    .fieldControl = 0,
    .manufacturerId = fileSpec.manufacturerCode,
    .imageType = fileSpec.type,
    .currentFileVersion = fileSpec.version,
    .hardwareVersion = hardwareVersion,
  };
  EmberStatus status
    = emberZclSendClusterOtaBootloadServerCommandQueryNextImageRequest(&destination,
                                                                       &request,
                                                                       queryNextImageResponseHandler);
  if (status != EMBER_SUCCESS) {
    // If we failed to submit the message for transmission, it could be
    // something on our side, so let's just try again later.
    scheduleQueryNextImage(true); // addJitter?
  }
}

static void queryNextImageResponseHandler(EmberZclMessageStatus_t status,
                                          const EmberZclCommandContext_t *context,
                                          const EmberZclClusterOtaBootloadServerCommandQueryNextImageResponse_t *response)
{
  switch (status) {
    case EMBER_ZCL_MESSAGE_STATUS_DISCOVERY_TIMEOUT:
    case EMBER_ZCL_MESSAGE_STATUS_COAP_TIMEOUT:
    case EMBER_ZCL_MESSAGE_STATUS_COAP_RESET:
      // If we couldn't reach the server for some reason, we need to go back and
      // try to find it again. Maybe it dropped off the network?
      emberAfPluginOtaBootloadClientPrintln("Query: Got COAP Timeout/Reset");
      scheduleDiscoverServer(true, false); // eraseServerInfo? addJitter?
      break;
    case EMBER_ZCL_MESSAGE_STATUS_COAP_ACK:
      // This means that our request was ACK'd, but the actual response is coming.
      emberAfPluginOtaBootloadClientPrintln("Query: Got COAP ACK");
      break;
    case EMBER_ZCL_MESSAGE_STATUS_COAP_RESPONSE:
      if (context->payloadLength < 6) { // if there is an error status code or no payload
        emberAfPluginOtaBootloadClientPrintln("Query: Bad payloadLength");
        scheduleDiscoverServer(true, false); // eraseServerInfo? addJitter?
        break;
      }
      emberAfPluginOtaBootloadClientPrintln("Query: Got COAP Response");
      assert(context != NULL);
      processQueryNextImageResponse(context->code, response);
      break;
    default:
      emberAfPluginOtaBootloadClientPrintln("Query: Bad query: %u", status);
      scheduleDiscoverServer(true, false); // eraseServerInfo? addJitter?
  }
}

static void processQueryNextImageResponse(const EmberCoapCode code, const EmberZclClusterOtaBootloadServerCommandQueryNextImageResponse_t *response)
{
  if (code != EMBER_COAP_CODE_204_CHANGED) {
    emberAfPluginOtaBootloadClientPrintln("Query: CoAP response (%d) from server not understood", code);
    downloadStatus = EMBER_ZCL_STATUS_FAILURE;
    scheduleUpgradeEnd();
    return;
  }

  EmberZclOtaBootloadStorageInfo_t storageInfo;
  emberZclOtaBootloadStorageGetInfo(&storageInfo, NULL, 0);
  if (response->imageSize > storageInfo.maximumFileSize) {
    emberAfPluginOtaBootloadClientPrintln("Query: Not enough space (need %u, have %u)",
                                          response->imageSize,
                                          storageInfo.maximumFileSize);
    downloadStatus = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
    scheduleUpgradeEnd();
    return;
  }

  fileSpecToDownload.version = response->fileVersion;
  if (response->fileUri.length > FILE_URI_LENGTH - 1) {
    downloadStatus = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
    emberAfPluginOtaBootloadClientPrintln("Query: URI too long");
    scheduleUpgradeEnd();
    return;
  }
  MEMCOPY(fileUri, response->fileUri.ptr, response->fileUri.length);
  fileUri[response->fileUri.length] = 0; // null terminator

  switch (response->status) {
    case EMBER_ZCL_STATUS_SUCCESS:
      ERASE_DOWNLOAD_STATUS();
      setState(STATE_WAIT_FOR_DELETE, 0); // run now
      break;
    case EMBER_ZCL_STATUS_NOT_AUTHORIZED:
      emberAfPluginOtaBootloadClientPrintln("Query: Not authorized");
      scheduleDiscoverServer(true, false); // eraseServerInfo? addJitter?
      break;
    case EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE:
      emberAfPluginOtaBootloadClientPrintln("Query: No Image Available");
      scheduleQueryNextImage(false); // addJitter?
      break;
    default:
      emberAfPluginOtaBootloadClientPrintln("Bad query: %u", response->status);
      scheduleDiscoverServer(true, false); // eraseServerInfo? addJitter?
  }
}

static void waitForDeleteStateHandler(void)
{
  EmberZclOtaBootloadStorageFileInfo_t storageFileInfo;
  bool fileExists
    = (emberZclOtaBootloadStorageFind(&fileSpecToDownload, &storageFileInfo)
       == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  if (emberZclOtaBootloadClientStartDownloadCallback(&fileSpecToDownload,
                                                     fileExists)) {
    ERASE_DOWNLOAD_STATUS();
    setState(STATE_DOWNLOAD, 0); // run now
  } else {
    setState(STATE_WAIT_FOR_DELETE, 100); // run now
  }
}

// -------------------------------------
// Download

static EmberStatus getCurrentDownloadFileOffset(size_t *offset)
{
  assert(HAVE_FILE_SPEC_TO_DOWNLOAD());
  EmberZclOtaBootloadStorageFileInfo_t storageFileInfo;
  EmberZclOtaBootloadStorageStatus_t storageStatus
    = emberZclOtaBootloadStorageFind(&fileSpecToDownload, &storageFileInfo);

  if (storageStatus == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE) {
    // need to create the OTA file storage
    storageStatus = emberZclOtaBootloadStorageCreate(&fileSpecToDownload);
    if (storageStatus != EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
      emberAfPluginOtaBootloadClientPrintln("Could not create OTA Storage: %u", storageStatus);
      return EMBER_ERR_FATAL;
    }

    storageStatus = emberZclOtaBootloadStorageFind(&fileSpecToDownload, &storageFileInfo);
  }

  if (storageStatus != EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
    emberAfPluginOtaBootloadClientPrintln("Could not find OTA Storage: %u", storageStatus);
    return EMBER_ERR_FATAL;
  }

  *offset = storageFileInfo.size;
  return EMBER_SUCCESS;
}

static void handleBlockError(uint8_t *errors)
{
  (*errors)++;
  if (*errors >= EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_BLOCK_ERROR_THRESHOLD) {
    *errors = 0;
    downloadStatus = EMBER_ZCL_STATUS_ABORT;
    emberAfPluginOtaBootloadClientPrintln("Exceeded block error count maximum");
    setState(STATE_UPGRADE_END, 0); // Fail right away
  } else {
    setState(STATE_DOWNLOAD,
             convertToMs(DOWNLOAD_BLOCK_ERROR_BACKOFF_MS,
                         DELAY_TYPE_MILLISECONDS,
                         true)); // addJitter?
  }
}

static void downloadStateHandler(void)
{
  size_t offset = 0;
  if (getCurrentDownloadFileOffset(&offset) != EMBER_SUCCESS) {
    emberAfPluginOtaBootloadClientPrintln("Error resolving storage offset");
    scheduleQueryNextImage(true); // addJitter?
    return;
  }

  // TODO: what if the offset of the file is not at a multiple of block size?
  assert(offset % (1 << curBlockSizeLog) == 0);
  EmberCoapBlockOption block2Option = {
    .more = false,
    .logSize = curBlockSizeLog,
    .number = offset >> curBlockSizeLog
  };

  if (!HAVE_SERVER_INFO()) {
    emberAfPluginOtaBootloadClientPrintln("Lost server info while downloading");
    scheduleDiscoverServer(true, true); // eraseServerInfo? addJitter?
    return;
  }

  EmberZclCoapEndpoint_t destination = {
    .flags = EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG
             | (emberZclOtaBootloadClientExpectSecureOta() ? EMBER_ZCL_USE_COAPS_FLAG : 0x00)
             | (emberZclOtaBootloadIsWildcard(serverInfo.uid.bytes, sizeof(serverInfo.uid.bytes)) ? 0x00 : EMBER_ZCL_HAVE_UID_FLAG),
    .uid = serverInfo.uid,
    .address = serverInfo.address,
    .port = serverInfo.port
  };

  emberAfPluginOtaBootloadClientPrintln("requesting block: %d*2^%d", block2Option.number, block2Option.logSize);
  EmberStatus status = emberZclRequestBlock(&destination,
                                            fileUri,
                                            &block2Option,
                                            blockResponseHandler);

  if (status != EMBER_SUCCESS) {
    emberAfPluginOtaBootloadClientPrintln("zcl send error: %u", status);
    // If we fail to submit the message to the stack, just try again. Maybe
    // second time is the charm?
    setState(STATE_DOWNLOAD, 0); // run now
  }
}

static void blockResponseHandler(EmberCoapStatus status,
                                 EmberCoapCode code,
                                 EmberCoapReadOptions *options,
                                 uint8_t *payload,
                                 uint16_t payloadLength,
                                 EmberCoapResponseInfo *info)
{
  static uint8_t errors = 0;
  emberAfPluginOtaBootloadClientPrintln("block responded: %d", status);
  EmberCoapBlockOption block2Option;
  if (status == EMBER_COAP_MESSAGE_ACKED) {
    // If we got an ACK, that means the response is coming later, so just wait
    // for the next response.
    return;
  } else if (status != EMBER_COAP_MESSAGE_RESPONSE
             || code != EMBER_COAP_CODE_205_CONTENT
             || !emberReadBlockOption(options,
                                      EMBER_COAP_OPTION_BLOCK2,
                                      &block2Option)) {
    emberAfPluginOtaBootloadClientPrintln("block error #%u: (%u != %u) || (%u != %u)",
                                          errors,
                                          status,
                                          EMBER_COAP_MESSAGE_RESPONSE,
                                          code,
                                          EMBER_COAP_CODE_205_CONTENT);
    handleBlockError(&errors);
    return;
  }
  if ((block2Option.logSize != curBlockSizeLog) && (block2Option.number == 0)) {
    // A server may report back that they can only support a smaller block size,
    // so we must send subsequent requests with that size.
    curBlockSizeLog = block2Option.logSize;
  }
  if (!emberVerifyBlockOption(&block2Option, payloadLength, curBlockSizeLog)) {
    emberAfPluginOtaBootloadClientPrintln("block error #%u: invalidBlockOption(more: %u, number: 0x%x, log: %u != %u, payloadLength: %u)",
                                          errors,
                                          block2Option.more,
                                          block2Option.number,
                                          block2Option.logSize,
                                          curBlockSizeLog,
                                          payloadLength);
    handleBlockError(&errors);
    return;
  }

  assert(HAVE_FILE_SPEC_TO_DOWNLOAD());
  size_t offset = block2Option.number << block2Option.logSize;
  EmberZclOtaBootloadStorageStatus_t storageStatus
    = emberZclOtaBootloadStorageWrite(&fileSpecToDownload,
                                      offset,
                                      payload,
                                      payloadLength);
  if (storageStatus != EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
    // If the storage is in a weird state, then we might keep hitting this
    // error. If that is the case, we can just let our errors go over the
    // threshold so we will give up on the download.
    emberAfPluginOtaBootloadClientPrintln("block error #%u (blockNo: 0x%x): Storage status fail at offset=%u, length=%u",
                                          errors,
                                          block2Option.number,
                                          offset,
                                          payloadLength);
    handleBlockError(&errors);
    return;
  } else if (!block2Option.more) {
    emberAfPluginOtaBootloadClientPrintln("download complete");
    setState(STATE_VERIFY_DOWNLOAD, 0); // run now
  } else {
    emberAfPluginOtaBootloadClientPrintln("got blockNo: 0x%x", block2Option.number);
    setState(STATE_DOWNLOAD, 0); // continue download
  }
}

// -------------------------------------
// Verify download

static void verifyDownloadStateHandler(void)
{
  assert(HAVE_FILE_SPEC_TO_DOWNLOAD());
  if (downloadStatus != EMBER_ZCL_STATUS_NULL) {
    emberAfPluginOtaBootloadClientPrintln("Verify failed: Invalid download status (%u != %u)", downloadStatus, EMBER_ZCL_STATUS_NULL);
    scheduleUpgradeEnd();
    return;
  }
  EmberZclOtaBootloadStorageStatus_t storageStatus;
  EmberZclOtaBootloadStorageFileInfo_t storageFileInfo;
  storageStatus = emberZclOtaBootloadStorageFind(&fileSpecToDownload, &storageFileInfo);
  if (storageStatus != EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
    emberAfPluginOtaBootloadClientPrintln("Verify failed: file does not exist");
    downloadStatus = EMBER_ZCL_STATUS_INVALID_IMAGE;
    scheduleUpgradeEnd();
    return;
  }

  uint8_t bytes[sizeof(EmberZclOtaBootloadFileHeaderInfo_t)];
  storageStatus = emberZclOtaBootloadStorageRead(&fileSpecToDownload,
                                                 0,
                                                 &bytes,
                                                 sizeof(bytes));
  if (storageStatus != EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
    emberAfPluginOtaBootloadClientPrintln("Verify failed: Error reading header (%u)", storageStatus);
    downloadStatus = EMBER_ZCL_STATUS_INVALID_IMAGE;
    scheduleUpgradeEnd();
    return;
  }

  EmberZclOtaBootloadFileHeaderInfo_t fileHeaderInfo;
  storageStatus = emberZclOtaBootloadFetchFileHeaderInfo(bytes, &fileHeaderInfo);
  if (storageStatus != EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_VALID) {
    emberAfPluginOtaBootloadClientPrintln("Verify failed: Error fetching header (%u)", storageStatus);
    downloadStatus = EMBER_ZCL_STATUS_INVALID_IMAGE;
  } else if (fileHeaderInfo.version != EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION) {
    emberAfPluginOtaBootloadClientPrintln("Verify failed: version mismatch (%u != %u)",
                                          fileHeaderInfo.version,
                                          EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION);
    downloadStatus = EMBER_ZCL_STATUS_INVALID_IMAGE;
  } else if (fileHeaderInfo.spec.manufacturerCode != fileSpecToDownload.manufacturerCode) {
    emberAfPluginOtaBootloadClientPrintln("Verify failed: manufacturerCode mismatch (%u != %u)",
                                          fileHeaderInfo.spec.manufacturerCode,
                                          fileSpecToDownload.manufacturerCode);
    downloadStatus = EMBER_ZCL_STATUS_INVALID_IMAGE;
  } else if (fileHeaderInfo.spec.type != fileSpecToDownload.type) {
    emberAfPluginOtaBootloadClientPrintln("Verify failed: imageType mismatch (%u != %u)",
                                          fileHeaderInfo.spec.type,
                                          fileSpecToDownload.type);
    downloadStatus = EMBER_ZCL_STATUS_INVALID_IMAGE;
  } else if (fileHeaderInfo.spec.version != fileSpecToDownload.version) {
    emberAfPluginOtaBootloadClientPrintln("Verify failed: imageVersion mismatch (%u != %u)",
                                          fileHeaderInfo.spec.version,
                                          fileSpecToDownload.version);
    downloadStatus = EMBER_ZCL_STATUS_INVALID_IMAGE;
  } else if (fileHeaderInfo.stackVersion != EMBER_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP) {
    emberAfPluginOtaBootloadClientPrintln("Verify failed: stackVersion mismatch (%u != %u)",
                                          fileHeaderInfo.stackVersion,
                                          EMBER_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP);
    downloadStatus = EMBER_ZCL_STATUS_INVALID_IMAGE;
  } else if (fileHeaderInfo.fileSize != storageFileInfo.size) {
    emberAfPluginOtaBootloadClientPrintln("Verify failed: fileSize mismatch (%u != %u)",
                                          fileHeaderInfo.fileSize,
                                          storageFileInfo.size);
    downloadStatus = EMBER_ZCL_STATUS_INVALID_IMAGE;
  } else if (fileHeaderInfo.securityCredentialVersion != EMBER_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP) {
    emberAfPluginOtaBootloadClientPrintln("Verify failed: securityCredentialVersion mismatch (%u != %u)",
                                          fileHeaderInfo.securityCredentialVersion,
                                          EMBER_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP);
    downloadStatus = EMBER_ZCL_STATUS_INVALID_IMAGE;
  } else {
    emberAfPluginOtaBootloadClientPrintln("download verified");
    downloadStatus = EMBER_ZCL_STATUS_SUCCESS;
  }

  scheduleUpgradeEnd();
}

// -------------------------------------
// Upgrade end request

static void scheduleUpgradeEnd(void)
{
  // Add some backoff into this guy, since there may be many nodes that have
  // downloaded an image (in the multicast scenario).
  setState(STATE_UPGRADE_END,
           convertToMs(UPGRADE_END_RESPONSE_BACKOFF_MS,
                       DELAY_TYPE_MILLISECONDS,
                       true)); // addJitter?
}

static void upgradeEndStateHandler(void)
{
  emberAfPluginOtaBootloadClientPrintln("Download Complete: %u", downloadStatus);
  EmberStatus status
    = emberZclOtaBootloadClientDownloadCompleteCallback(&fileSpecToDownload,
                                                        downloadStatus);
  // Only accept the application layer's status if we have not already encountered
  // some error
  if (downloadStatus == EMBER_ZCL_STATUS_SUCCESS) {
    downloadStatus = status;
  }
  EmberZclDestination_t destination;
  serverInfoToZclDestination(&destination);
  EmberZclClusterOtaBootloadServerCommandUpgradeEndRequest_t request = {
    .status = downloadStatus,
    .manufacturerId = fileSpecToDownload.manufacturerCode,
    .imageType = fileSpecToDownload.type,
    .fileVersion = fileSpecToDownload.version,
  };
  status
    = emberZclSendClusterOtaBootloadServerCommandUpgradeEndRequest(&destination,
                                                                   &request,
                                                                   upgradeEndResponseHandler);
  if (status != EMBER_SUCCESS) {
    // If we failed to submit the message for transmission, it could be
    // something on our side, so let's just try again later. We jitter the
    // response since giving the stack some time to breathe could solve our
    // problem.
    emberAfPluginOtaBootloadClientPrintln("Download command fail: %u", status);
    scheduleUpgradeEnd();
  } else if (downloadStatus != EMBER_ZCL_STATUS_SUCCESS) {
    // If our downloadStatus is not successful, then we are going to get a
    // default response from the server, so we should kick off the next image
    // query here.
    emberAfPluginOtaBootloadClientPrintln("Download fail: %u", downloadStatus);
    scheduleQueryNextImage(false); // addJitter?
  }
}

static void upgradeEndResponseHandler(EmberZclMessageStatus_t status,
                                      const EmberZclCommandContext_t *context,
                                      const EmberZclClusterOtaBootloadServerCommandUpgradeEndResponse_t *response)
{
  switch (status) {
    case EMBER_ZCL_MESSAGE_STATUS_DISCOVERY_TIMEOUT:
    case EMBER_ZCL_MESSAGE_STATUS_COAP_TIMEOUT:
    case EMBER_ZCL_MESSAGE_STATUS_COAP_RESET:
      // If we couldn't reach the server for some reason, then try again to send
      // the command.
      // TODO: we should probably add this to our error threshold!
      emberAfPluginOtaBootloadClientPrintln("Timeout/reset");
      scheduleUpgradeEnd();
      break;
    case EMBER_ZCL_MESSAGE_STATUS_COAP_ACK:
      emberAfPluginOtaBootloadClientPrintln("COAP ack");
      // This means that our request was ACK'd, but the actual response is coming.
      break;
    case EMBER_ZCL_MESSAGE_STATUS_COAP_RESPONSE:
      if ((context != NULL) && (context->code != EMBER_COAP_CODE_204_CHANGED)) {
        emberAfPluginOtaBootloadClientPrintln("COAP response: OTA server did not respond with 2.04 to upgradeEndRequest");
        scheduleDiscoverServer(true, true); // eraseServerInfo? addJitter?
        break;
      }
      emberAfPluginOtaBootloadClientPrintln("COAP response");
      processUpgradeEndResponse(response);
      break;
    default:
      assert(false);
  }
}

static void processUpgradeEndResponse(const EmberZclClusterOtaBootloadServerCommandUpgradeEndResponse_t *response)
{
  if (!HAVE_FILE_SPEC_TO_DOWNLOAD() || (response == NULL)) {
    emberAfPluginOtaBootloadClientPrintln("UpgradeEnd with no file spec to download");
    scheduleDiscoverServer(true, true); // eraseServerInfo? addJitter?
    return;
  }

  // This logic is brought you by 11.11.4 in the OTA spec. If currentTime is 0,
  // then upgradeTime is the offset, in seconds, of when we should perform the
  // upgrade. Else if upgradeTime is not 0 and not 0xFFFFFFFF, then we should
  // treat the two values like UTC times. Else if the upgrade time is 0xFFFFFFFF,
  // then that means to check back in later to see if we can upgrade.
  if (response->upgradeTime == 0xFFFFFFFF) {
    emberAfPluginOtaBootloadClientPrintln("Will retry upgradeEnd");
    scheduleUpgradeEnd();
    return;
  }

  if (fileSpecToDownload.manufacturerCode != response->manufacturerId
      || fileSpecToDownload.type != response->imageType
      || fileSpecToDownload.version != response->fileVersion) {
    // TODO: handle default response that may be sent back (see 11.13.6.4).
    // As a workaround for this issue, let's just reschedule the download.
    emberAfPluginOtaBootloadClientPrintln("Mismatched firmware image properties %u:%u, %u:%u, %u:%u", fileSpecToDownload.manufacturerCode,
                                          response->manufacturerId, fileSpecToDownload.type, response->imageType, fileSpecToDownload.version, response->fileVersion);
    scheduleQueryNextImage(false);
    return;
  }

  emberAfPluginOtaBootloadClientPrintln("Will bootload in %d seconds", response->upgradeTime - response->currentTime);
  setState(STATE_BOOTLOAD,
           convertToMs(response->upgradeTime - response->currentTime,
                       DELAY_TYPE_SECONDS,
                       false)); // addJitter?
}

// -------------------------------------
// Bootload

static void bootloadStateHandler(void)
{
  assert(HAVE_FILE_SPEC_TO_DOWNLOAD());
  emberZclOtaBootloadClientPreBootloadCallback(&fileSpecToDownload);

  emberAfPluginOtaBootloadClientPrintln("Loading new image");
  EmberStatus status = halAppBootloaderInstallNewImage();
  // This call should not return!
  (void)status;
}

// -----------------------------------------------------------------------------
// Public API downward

void emZclOtaBootloadClientInitCallback(void)
{
  ERASE_SERVER_INFO();
  ERASE_FILE_SPEC_TO_DOWNLOAD();
  ERASE_DOWNLOAD_STATUS();
  if (emberZclOtaBootloadClientSetVersionInfoCallback()) {
    emberAfPluginOtaBootloadClientPrintln("Failure initializing OTA attributes");
  }
  setState(STATE_NONE, 0); // run now
}

void emZclOtaBootloadClientNetworkStatusCallback(EmberNetworkStatus newNetworkStatus,
                                                 EmberNetworkStatus oldNetworkStatus,
                                                 EmberJoinFailureReason reason)
{
#ifdef EMBER_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_AUTO_START
  if (newNetworkStatus == EMBER_JOINED_NETWORK_ATTACHED) {
    scheduleDiscoverServer(false, true); // eraseServerInfo? addJitter?
  }
#endif
}

#ifdef EMBER_AF_PLUGIN_POLLING
bool emZclOtaBootloadClientOkToLongPoll(uint32_t durationMs)
{
  return state != STATE_DOWNLOAD;
}
#endif

void emZclOtaBootloadClientEventHandler(void)
{
  emberEventControlSetInactive(emZclOtaBootloadClientEventControl);

  typedef void (*StateHandler)(void);
  const StateHandler stateHandlers[] = {
    noneStateHandler,              // STATE_NONE
    discoverServerStateHandler,    // STATE_DISCOVER_SERVER
    createDtlsSessionStateHandler, // STATE_CREATE_DTLS_SESSION
    queryNextImageStateHandler,    // STATE_QUERY_NEXT_IMAGE
    waitForDeleteStateHandler,     // STATE_WAIT_FOR_DELETE
    downloadStateHandler,          // STATE_DOWNLOAD
    verifyDownloadStateHandler,    // STATE_VERIFY_DOWNLOAD
    upgradeEndStateHandler,        // STATE_UPGRADE_END
    bootloadStateHandler,          // STATE_BOOTLOAD
  };
  assert(state < COUNTOF(stateHandlers));
  (*(stateHandlers[state]))();
}
