/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_HAL
#include CHIP_AF_API_STACK
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_ZCL_CORE_WELL_KNOWN
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_CLIENT
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE
#include CHIP_AF_API_ZCL_CORE_DTLS_MANAGER

#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#else
  #define chipAfPluginOtaBootloadClientPrint(...)
  #define chipAfPluginOtaBootloadClientPrintln(...)
  #define chipAfPluginOtaBootloadClientFlush()
  #define chipAfPluginOtaBootloadClientDebugExec(x)
  #define chipAfPluginOtaBootloadClientPrintBuffer(buffer, len, withSpace)
  #define chipAfPluginOtaBootloadClientPrintString(buffer)
#endif

#ifdef CHIP_SCRIPTED_TEST
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

ChipEventControl chZclOtaBootloadClientEventControl;

static ChipZclOtaBootloadClientServerInfo_t serverInfo;
#define HAVE_SERVER_INFO()  (serverInfo.endpointId != CHIP_ZCL_ENDPOINT_NULL)
#define ERASE_SERVER_INFO() do { serverInfo.endpointId = CHIP_ZCL_ENDPOINT_NULL; curBlockSizeLog = DEFAULT_BLOCK_SIZE_LOG; } while (0)

static ChipZclOtaBootloadFileSpec_t fileSpecToDownload;
#define HAVE_FILE_SPEC_TO_DOWNLOAD()  (fileSpecToDownload.version != CHIP_ZCL_OTA_BOOTLOAD_FILE_VERSION_NULL)
#define ERASE_FILE_SPEC_TO_DOWNLOAD() do { fileSpecToDownload.version = CHIP_ZCL_OTA_BOOTLOAD_FILE_VERSION_NULL; } while (0)

static ChipZclStatus_t downloadStatus;
#define HAVE_DOWNLOAD_STATUS() (downloadStatus != CHIP_ZCL_STATUS_NULL)
#define ERASE_DOWNLOAD_STATUS() do { downloadStatus = CHIP_ZCL_STATUS_NULL; } while (0)

static bool ipAddressResolved;

static bool immediateUpgrade = false;

// This has to accomodate the leading '/' in the URI. The URI is custom to each
// vendor, thus it could be any length. For now we just base the length off of
// what our OTA server sends back.
#define FILE_URI_LENGTH 32
#if FILE_URI_LENGTH < (CH_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_LENGTH + 1)
  #error Length of the URI buffer is not large enough
#endif
static uint8_t fileUri[FILE_URI_LENGTH];

static uint8_t curBlockSizeLog = DEFAULT_BLOCK_SIZE_LOG;

// -----------------------------------------------------------------------------
// Private API

static void scheduleCreateDtlsSession(bool addJitter);
static ChipStatus schedulePostDiscoverServerState();
static void scheduleQueryNextImage(bool addJitter);
static void queryNextImageResponseHandler(ChipZclMessageStatus_t status,
                                          const ChipZclCommandContext_t *context,
                                          const ChipZclClusterOtaBootloadServerCommandQueryNextImageResponse_t *response);
static void processQueryNextImageResponse(const ChipCoapCode code, const ChipZclClusterOtaBootloadServerCommandQueryNextImageResponse_t *response);
static void blockResponseHandler(ChipCoapStatus status,
                                 ChipCoapCode code,
                                 ChipCoapReadOptions *options,
                                 uint8_t *payload,
                                 uint16_t payloadLength,
                                 ChipCoapResponseInfo *info);
static void scheduleUpgradeEnd(void);
static void upgradeEndResponseHandler(ChipZclMessageStatus_t status,
                                      const ChipZclCommandContext_t *context,
                                      const ChipZclClusterOtaBootloadServerCommandUpgradeEndResponse_t *response);
static void processUpgradeEndResponse(const ChipZclClusterOtaBootloadServerCommandUpgradeEndResponse_t *response);

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
  chipEventControlSetDelayMS(chZclOtaBootloadClientEventControl, delayMs);
}

static void serverInfoToZclDestination(ChipZclDestination_t *destination)
{
  // TODO: send this command to an IPV6 address until UID discovery works.
  assert(HAVE_SERVER_INFO());
  destination->network.address = serverInfo.address;
  destination->network.flags =
    (serverInfo.scheme == CHIP_ZCL_SCHEME_COAPS
     ? (CHIP_ZCL_USE_COAPS_FLAG | CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG | CHIP_ZCL_HAVE_UID_FLAG)
     : CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG);
  destination->network.uid = serverInfo.uid;
  destination->network.port = serverInfo.port;
  destination->application.data.endpointId = serverInfo.endpointId;
  destination->application.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT;
}

void chipZclClusterOtaBootloadClientCommandImageNotifyRequestHandler(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterOtaBootloadClientCommandImageNotifyRequest_t *request)
{
  chipAfPluginOtaBootloadClientPrintln("ImageNotifyRequest received");
  ChipZclOtaBootloadFileSpec_t fileSpec;
  ChipZclOtaBootloadHardwareVersion_t hardwareVersion
    = CHIP_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL;
  if ((request->payloadType != 0x00)
      && !chipZclOtaBootloadClientGetQueryNextImageParametersCallback(&fileSpec,
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
  chipAfPluginOtaBootloadClientPrintln("ImageNotifyRequest match. Jitter: %u <= %u", requestJitter, request->queryJitter);
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
           convertToMs(CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_DISCOVER_SERVER_PERIOD_MINUTES,
                       DELAY_TYPE_MINUTES,
                       addJitter));
  immediateUpgrade = false;
}

static void dnsLookupResponseHandler(ChipDnsLookupStatus status,
                                     const uint8_t *domainName,
                                     uint8_t domainNameLength,
                                     const ChipDnsResponse *response,
                                     void *applicationData,
                                     uint16_t applicationDataLength)
{
  chipAfPluginOtaBootloadClientPrintln("dnsLookupResponseHandler: %u", status);
  switch (status) {
    case CHIP_DNS_LOOKUP_SUCCESS:
      serverInfo.address = response->ipAddress;
      chipAfPluginOtaBootloadClientPrint("Found via DNS: ");
      chipAfPluginOtaBootloadClientDebugExec(chipAfPrintIpv6Address(&serverInfo.address));
      chipAfPluginOtaBootloadClientPrintln("->%u", serverInfo.port);
      if (schedulePostDiscoverServerState() == CHIP_SUCCESS) {
        return;
      }
      chipAfPluginOtaBootloadClientPrintln("No suitable OTA server at address");
      scheduleDiscoverServer(true, false); // Let's try again
      break;
    case CHIP_DNS_LOOKUP_NO_BORDER_ROUTER:
    case CHIP_DNS_LOOKUP_NO_BORDER_ROUTER_RESPONSE:
    case CHIP_DNS_LOOKUP_NO_DNS_RESPONSE:
      chipAfPluginOtaBootloadClientPrintln("No DNS response: %u", status);
      scheduleDiscoverServer(true, false); // Let's try again
      break;
    case CHIP_DNS_LOOKUP_BORDER_ROUTER_RESPONSE_ERROR:
    case CHIP_DNS_LOOKUP_NO_DNS_RESPONSE_ERROR:
      chipAfPluginOtaBootloadClientPrintln("DNS Error: %u", status);
      scheduleDiscoverServer(true, false); // Let's try again
      break;
    case CHIP_DNS_LOOKUP_NO_DNS_SERVER:
    case CHIP_DNS_LOOKUP_NO_ENTRY_FOR_NAME:
    case CHIP_DNS_LOOKUP_NO_BUFFERS:
      chipAfPluginOtaBootloadClientPrintln("Can't lookup host: %u", status);
      scheduleDiscoverServer(true, false); // Let's try again
      break;
    default:
      assert(false);
  }
}

static void discoverServerResponseHandler(ChipCoapStatus status,
                                          ChipCoapCode code,
                                          ChipCoapReadOptions *options,
                                          uint8_t *payload,
                                          uint16_t payloadLength,
                                          ChipCoapResponseInfo *info)
{
  if (HAVE_SERVER_INFO()) { // We have already found a server to download from
    return;
  }
  switch (status) {
    case CHIP_COAP_MESSAGE_TIMED_OUT:
    case CHIP_COAP_MESSAGE_RESET:
      // Since our discovery is multicast, we get the TIMED_OUT status when we
      // are done receiving responses (note that we could have received no
      // responses). If we found a server, we should have already moved on to the
      // next state. Otherwise, then we keep trying to discover a server.
      break;
    case CHIP_COAP_MESSAGE_ACKED:
      // This means that our request was ACK'd, but the actual response is coming.
      return;
    case CHIP_COAP_MESSAGE_RESPONSE:
    {
      if (payloadLength == 0) {
        break;
      }

      // TODO: shouldn't the UID be contained somewhere in this response?
      // TODO: how do we get the endpoint from the response?
      serverInfo.scheme = CHIP_ZCL_SCHEME_COAP;
      serverInfo.address = info->remoteAddress;
      serverInfo.port = info->remotePort;
      MEMSET(serverInfo.uid.bytes, 0x00, CHIP_ZCL_UID_SIZE);
      serverInfo.endpointId = 1;
      chipAfPluginOtaBootloadClientPrint("Discovered server: ");
      chipAfPluginOtaBootloadClientDebugExec(chipAfPrintIpv6Address(&serverInfo.address));
      chipAfPluginOtaBootloadClientPrintln("->%u", serverInfo.port);
      if (schedulePostDiscoverServerState() == CHIP_SUCCESS) {
        return;
      }
      chipAfPluginOtaBootloadClientPrintln("Discovered server was filtered out.");
      ERASE_SERVER_INFO();
      break;
    }
    default:
      assert(false);
  }

  scheduleDiscoverServer(false, false); // eraseServerInfo? addJitter?
  return;
}

ChipStatus schedulePostDiscoverServerState()
{
  chipAfPluginOtaBootloadClientPrintln("Scheduling post discoverServerState");
  if (chipZclOtaBootloadClientExpectSecureOta()) {
    scheduleCreateDtlsSession(false); // addJitter?
    return CHIP_SUCCESS;
  } else {
    chipAfPluginOtaBootloadClientPrintln("Don't need secure OTA");
  }
  if (chipZclOtaBootloadClientServerDiscoveredCallback(&serverInfo)) {
    scheduleQueryNextImage(false);
    return CHIP_SUCCESS;
  } else {
    chipAfPluginOtaBootloadClientPrintln("ServerInfo rejected by policy");
  }
  return CHIP_ERR_FATAL;
}

void chipGetGlobalAddressReturn(const ChipIpv6Address *address,
                                 uint32_t preferredLifetime,
                                 uint32_t validLifetime,
                                 uint8_t addressFlags)
{
  if (ipAddressResolved) {
    return;
  }
  if (chipIsIpv6UnspecifiedAddress(address)) {
    chipAfPluginOtaBootloadClientPrintln("DNS Lookup: Failed to get Global Address");
    scheduleDiscoverServer(true, false); // addJitter?
    return;
  }
  if (chipDnsLookup(serverInfo.name,
                     serverInfo.nameLength,
                     address->bytes,
                     dnsLookupResponseHandler,
                     NULL,
                     0) != CHIP_SUCCESS) {
    chipAfPluginOtaBootloadClientPrintln("DNS Lookup: Failed to lookup hostname");
    scheduleDiscoverServer(true, false); // addJitter?
  } else {
    ipAddressResolved = true;
  }
}

static void discoverServerStateHandler(void)
{
  if (HAVE_SERVER_INFO()) { // we have the server, go to next state.
    scheduleQueryNextImage(false); // addJitter?
  } else if ( chipZclOtaBootloadClientServerHasStaticAddressCallback(&serverInfo)) {
    chipAfPluginOtaBootloadClientPrint("Using static OTA server IP: ");
    chipAfPluginOtaBootloadClientDebugExec(chipAfPrintIpv6Address(&serverInfo.address));
    chipAfPluginOtaBootloadClientPrintln("->%u", serverInfo.port);
    if (schedulePostDiscoverServerState() == CHIP_SUCCESS) {
      return;
    }
  } else if ( chipZclOtaBootloadClientServerHasDnsNameCallback(&serverInfo) ) {
    chipAfPluginOtaBootloadClientPrintln("Using DNS lookup");
    ipAddressResolved = false;
    chipGetGlobalAddresses(NULL, 0); // force an update of the global address
  } else if (chipZclOtaBootloadClientServerHasDiscByClusterId(&chipZclClusterOtaBootloadServerSpec, discoverServerResponseHandler)) {
    chipAfPluginOtaBootloadClientPrintln("Using upgrade server discovery");
  } else {
    chipAfPluginOtaBootloadClientPrintln("Failed to discoverServer");
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
  chipAfPluginOtaBootloadClientPrintln("DTLS Session ID was returned: %u", sessionId);
  if (sessionId != CHIP_NULL_SESSION_ID) {
    serverInfo.scheme = CHIP_ZCL_SCHEME_COAPS;
    chipAfPluginOtaBootloadClientPrintln("DTLS session has been established: %u", sessionId);
  }
  if ((chipZclDtlsManagerGetUidBySessionId(sessionId, &serverInfo.uid) != CHIP_SUCCESS)
      || (!chipZclOtaBootloadClientServerDiscoveredCallback(&serverInfo))) {
    chipAfPluginOtaBootloadClientPrintln("UID or ServerInfo mismatch");
    ERASE_SERVER_INFO();
    scheduleDiscoverServer(true, false); // addJitter?
    return;
  }
  scheduleQueryNextImage(false);
}

static void createDtlsSessionStateHandler(void)
{
  if (!chipZclOtaBootloadIsWildcard(serverInfo.uid.bytes, sizeof(serverInfo.uid.bytes))) {
    chipAfPluginOtaBootloadClientPrint("Looking up DTLS session by UID...");
    uint8_t sessionId = chipZclDtlsManagerGetSessionIdByUid(&serverInfo.uid, serverInfo.port);
    if (sessionId != CHIP_NULL_SESSION_ID) {
      chipAfPluginOtaBootloadClientPrintln("success");
      dtlsSessionIdReturn(sessionId);
      return;
    } else {
      chipAfPluginOtaBootloadClientPrintln("fail");
    }
  }

  chipAfPluginOtaBootloadClientPrintln("Creating DTLS session to server");
  chipZclDtlsManagerGetConnection(&(serverInfo.address), serverInfo.port, CHIP_DTLS_MODE_CERT, dtlsSessionIdReturn);
}

// -------------------------------------
// Query Next Image

static void scheduleQueryNextImage(bool addJitter)
{
  setState(STATE_QUERY_NEXT_IMAGE,
           convertToMs(immediateUpgrade ? 0 : CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_QUERY_NEXT_IMAGE_PERIOD_MINUTES,
                       DELAY_TYPE_MINUTES,
                       addJitter));
}

static void queryNextImageStateHandler(void)
{
  ChipZclOtaBootloadFileSpec_t fileSpec;
  ChipZclOtaBootloadHardwareVersion_t hardwareVersion
    = CHIP_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL;
  if (!chipZclOtaBootloadClientGetQueryNextImageParametersCallback(&fileSpec,
                                                                    &hardwareVersion)) {
    scheduleQueryNextImage(false); // addJitter?
    return;
  }
  fileSpecToDownload.manufacturerCode = fileSpec.manufacturerCode;
  fileSpecToDownload.type = fileSpec.type;

  // TODO: should we create our own constants for this fieldControl value?
  ChipZclDestination_t destination;
  serverInfoToZclDestination(&destination);
  ChipZclClusterOtaBootloadServerCommandQueryNextImageRequest_t request = {
    .fieldControl = 0,
    .manufacturerId = fileSpec.manufacturerCode,
    .imageType = fileSpec.type,
    .currentFileVersion = fileSpec.version,
    .hardwareVersion = hardwareVersion,
  };
  ChipStatus status
    = chipZclSendClusterOtaBootloadServerCommandQueryNextImageRequest(&destination,
                                                                       &request,
                                                                       queryNextImageResponseHandler);
  if (status != CHIP_SUCCESS) {
    // If we failed to submit the message for transmission, it could be
    // something on our side, so let's just try again later.
    scheduleQueryNextImage(true); // addJitter?
  }
}

static void queryNextImageResponseHandler(ChipZclMessageStatus_t status,
                                          const ChipZclCommandContext_t *context,
                                          const ChipZclClusterOtaBootloadServerCommandQueryNextImageResponse_t *response)
{
  switch (status) {
    case CHIP_ZCL_MESSAGE_STATUS_DISCOVERY_TIMEOUT:
    case CHIP_ZCL_MESSAGE_STATUS_COAP_TIMEOUT:
    case CHIP_ZCL_MESSAGE_STATUS_COAP_RESET:
      // If we couldn't reach the server for some reason, we need to go back and
      // try to find it again. Maybe it dropped off the network?
      chipAfPluginOtaBootloadClientPrintln("Query: Got COAP Timeout/Reset");
      scheduleDiscoverServer(true, false); // eraseServerInfo? addJitter?
      break;
    case CHIP_ZCL_MESSAGE_STATUS_COAP_ACK:
      // This means that our request was ACK'd, but the actual response is coming.
      chipAfPluginOtaBootloadClientPrintln("Query: Got COAP ACK");
      break;
    case CHIP_ZCL_MESSAGE_STATUS_COAP_RESPONSE:
      if (context->payloadLength < 6) { // if there is an error status code or no payload
        chipAfPluginOtaBootloadClientPrintln("Query: Bad payloadLength");
        scheduleDiscoverServer(true, false); // eraseServerInfo? addJitter?
        break;
      }
      chipAfPluginOtaBootloadClientPrintln("Query: Got COAP Response");
      assert(context != NULL);
      processQueryNextImageResponse(context->code, response);
      break;
    default:
      chipAfPluginOtaBootloadClientPrintln("Query: Bad query: %u", status);
      scheduleDiscoverServer(true, false); // eraseServerInfo? addJitter?
  }
}

static void processQueryNextImageResponse(const ChipCoapCode code, const ChipZclClusterOtaBootloadServerCommandQueryNextImageResponse_t *response)
{
  if (code != CHIP_COAP_CODE_204_CHANGED) {
    chipAfPluginOtaBootloadClientPrintln("Query: CoAP response (%d) from server not understood", code);
    downloadStatus = CHIP_ZCL_STATUS_FAILURE;
    scheduleUpgradeEnd();
    return;
  }

  ChipZclOtaBootloadStorageInfo_t storageInfo;
  chipZclOtaBootloadStorageGetInfo(&storageInfo, NULL, 0);
  if (response->imageSize > storageInfo.maximumFileSize) {
    chipAfPluginOtaBootloadClientPrintln("Query: Not enough space (need %u, have %u)",
                                          response->imageSize,
                                          storageInfo.maximumFileSize);
    downloadStatus = CHIP_ZCL_STATUS_INSUFFICIENT_SPACE;
    scheduleUpgradeEnd();
    return;
  }

  fileSpecToDownload.version = response->fileVersion;
  if (response->fileUri.length > FILE_URI_LENGTH - 1) {
    downloadStatus = CHIP_ZCL_STATUS_INSUFFICIENT_SPACE;
    chipAfPluginOtaBootloadClientPrintln("Query: URI too long");
    scheduleUpgradeEnd();
    return;
  }
  MEMCOPY(fileUri, response->fileUri.ptr, response->fileUri.length);
  fileUri[response->fileUri.length] = 0; // null terminator

  switch (response->status) {
    case CHIP_ZCL_STATUS_SUCCESS:
      ERASE_DOWNLOAD_STATUS();
      setState(STATE_WAIT_FOR_DELETE, 0); // run now
      break;
    case CHIP_ZCL_STATUS_NOT_AUTHORIZED:
      chipAfPluginOtaBootloadClientPrintln("Query: Not authorized");
      scheduleDiscoverServer(true, false); // eraseServerInfo? addJitter?
      break;
    case CHIP_ZCL_STATUS_NO_IMAGE_AVAILABLE:
      chipAfPluginOtaBootloadClientPrintln("Query: No Image Available");
      scheduleQueryNextImage(false); // addJitter?
      break;
    default:
      chipAfPluginOtaBootloadClientPrintln("Bad query: %u", response->status);
      scheduleDiscoverServer(true, false); // eraseServerInfo? addJitter?
  }
}

static void waitForDeleteStateHandler(void)
{
  ChipZclOtaBootloadStorageFileInfo_t storageFileInfo;
  bool fileExists
    = (chipZclOtaBootloadStorageFind(&fileSpecToDownload, &storageFileInfo)
       == CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  if (chipZclOtaBootloadClientStartDownloadCallback(&fileSpecToDownload,
                                                     fileExists)) {
    ERASE_DOWNLOAD_STATUS();
    setState(STATE_DOWNLOAD, 0); // run now
  } else {
    setState(STATE_WAIT_FOR_DELETE, 100); // run now
  }
}

// -------------------------------------
// Download

static ChipStatus getCurrentDownloadFileOffset(size_t *offset)
{
  assert(HAVE_FILE_SPEC_TO_DOWNLOAD());
  ChipZclOtaBootloadStorageFileInfo_t storageFileInfo;
  ChipZclOtaBootloadStorageStatus_t storageStatus
    = chipZclOtaBootloadStorageFind(&fileSpecToDownload, &storageFileInfo);

  if (storageStatus == CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE) {
    // need to create the OTA file storage
    storageStatus = chipZclOtaBootloadStorageCreate(&fileSpecToDownload);
    if (storageStatus != CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
      chipAfPluginOtaBootloadClientPrintln("Could not create OTA Storage: %u", storageStatus);
      return CHIP_ERR_FATAL;
    }

    storageStatus = chipZclOtaBootloadStorageFind(&fileSpecToDownload, &storageFileInfo);
  }

  if (storageStatus != CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
    chipAfPluginOtaBootloadClientPrintln("Could not find OTA Storage: %u", storageStatus);
    return CHIP_ERR_FATAL;
  }

  *offset = storageFileInfo.size;
  return CHIP_SUCCESS;
}

static void handleBlockError(uint8_t *errors)
{
  (*errors)++;
  if (*errors >= CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_BLOCK_ERROR_THRESHOLD) {
    *errors = 0;
    downloadStatus = CHIP_ZCL_STATUS_ABORT;
    chipAfPluginOtaBootloadClientPrintln("Exceeded block error count maximum");
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
  if (getCurrentDownloadFileOffset(&offset) != CHIP_SUCCESS) {
    chipAfPluginOtaBootloadClientPrintln("Error resolving storage offset");
    scheduleQueryNextImage(true); // addJitter?
    return;
  }

  // TODO: what if the offset of the file is not at a multiple of block size?
  assert(offset % (1 << curBlockSizeLog) == 0);
  ChipCoapBlockOption block2Option = {
    .more = false,
    .logSize = curBlockSizeLog,
    .number = offset >> curBlockSizeLog
  };

  if (!HAVE_SERVER_INFO()) {
    chipAfPluginOtaBootloadClientPrintln("Lost server info while downloading");
    scheduleDiscoverServer(true, true); // eraseServerInfo? addJitter?
    return;
  }

  ChipZclCoapEndpoint_t destination = {
    .flags = CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG
             | (chipZclOtaBootloadClientExpectSecureOta() ? CHIP_ZCL_USE_COAPS_FLAG : 0x00)
             | (chipZclOtaBootloadIsWildcard(serverInfo.uid.bytes, sizeof(serverInfo.uid.bytes)) ? 0x00 : CHIP_ZCL_HAVE_UID_FLAG),
    .uid = serverInfo.uid,
    .address = serverInfo.address,
    .port = serverInfo.port
  };

  chipAfPluginOtaBootloadClientPrintln("requesting block: %d*2^%d", block2Option.number, block2Option.logSize);
  ChipStatus status = chipZclRequestBlock(&destination,
                                            fileUri,
                                            &block2Option,
                                            blockResponseHandler);

  if (status != CHIP_SUCCESS) {
    chipAfPluginOtaBootloadClientPrintln("zcl send error: %u", status);
    // If we fail to submit the message to the stack, just try again. Maybe
    // second time is the charm?
    setState(STATE_DOWNLOAD, 0); // run now
  }
}

static void blockResponseHandler(ChipCoapStatus status,
                                 ChipCoapCode code,
                                 ChipCoapReadOptions *options,
                                 uint8_t *payload,
                                 uint16_t payloadLength,
                                 ChipCoapResponseInfo *info)
{
  static uint8_t errors = 0;
  chipAfPluginOtaBootloadClientPrintln("block responded: %d", status);
  ChipCoapBlockOption block2Option;
  if (status == CHIP_COAP_MESSAGE_ACKED) {
    // If we got an ACK, that means the response is coming later, so just wait
    // for the next response.
    return;
  } else if (status != CHIP_COAP_MESSAGE_RESPONSE
             || code != CHIP_COAP_CODE_205_CONTENT
             || !chipReadBlockOption(options,
                                      CHIP_COAP_OPTION_BLOCK2,
                                      &block2Option)) {
    chipAfPluginOtaBootloadClientPrintln("block error #%u: (%u != %u) || (%u != %u)",
                                          errors,
                                          status,
                                          CHIP_COAP_MESSAGE_RESPONSE,
                                          code,
                                          CHIP_COAP_CODE_205_CONTENT);
    handleBlockError(&errors);
    return;
  }
  if ((block2Option.logSize != curBlockSizeLog) && (block2Option.number == 0)) {
    // A server may report back that they can only support a smaller block size,
    // so we must send subsequent requests with that size.
    curBlockSizeLog = block2Option.logSize;
  }
  if (!chipVerifyBlockOption(&block2Option, payloadLength, curBlockSizeLog)) {
    chipAfPluginOtaBootloadClientPrintln("block error #%u: invalidBlockOption(more: %u, number: 0x%x, log: %u != %u, payloadLength: %u)",
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
  ChipZclOtaBootloadStorageStatus_t storageStatus
    = chipZclOtaBootloadStorageWrite(&fileSpecToDownload,
                                      offset,
                                      payload,
                                      payloadLength);
  if (storageStatus != CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
    // If the storage is in a weird state, then we might keep hitting this
    // error. If that is the case, we can just let our errors go over the
    // threshold so we will give up on the download.
    chipAfPluginOtaBootloadClientPrintln("block error #%u (blockNo: 0x%x): Storage status fail at offset=%u, length=%u",
                                          errors,
                                          block2Option.number,
                                          offset,
                                          payloadLength);
    handleBlockError(&errors);
    return;
  } else if (!block2Option.more) {
    chipAfPluginOtaBootloadClientPrintln("download complete");
    setState(STATE_VERIFY_DOWNLOAD, 0); // run now
  } else {
    chipAfPluginOtaBootloadClientPrintln("got blockNo: 0x%x", block2Option.number);
    setState(STATE_DOWNLOAD, 0); // continue download
  }
}

// -------------------------------------
// Verify download

static void verifyDownloadStateHandler(void)
{
  assert(HAVE_FILE_SPEC_TO_DOWNLOAD());
  if (downloadStatus != CHIP_ZCL_STATUS_NULL) {
    chipAfPluginOtaBootloadClientPrintln("Verify failed: Invalid download status (%u != %u)", downloadStatus, CHIP_ZCL_STATUS_NULL);
    scheduleUpgradeEnd();
    return;
  }
  ChipZclOtaBootloadStorageStatus_t storageStatus;
  ChipZclOtaBootloadStorageFileInfo_t storageFileInfo;
  storageStatus = chipZclOtaBootloadStorageFind(&fileSpecToDownload, &storageFileInfo);
  if (storageStatus != CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
    chipAfPluginOtaBootloadClientPrintln("Verify failed: file does not exist");
    downloadStatus = CHIP_ZCL_STATUS_INVALID_IMAGE;
    scheduleUpgradeEnd();
    return;
  }

  uint8_t bytes[sizeof(ChipZclOtaBootloadFileHeaderInfo_t)];
  storageStatus = chipZclOtaBootloadStorageRead(&fileSpecToDownload,
                                                 0,
                                                 &bytes,
                                                 sizeof(bytes));
  if (storageStatus != CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
    chipAfPluginOtaBootloadClientPrintln("Verify failed: Error reading header (%u)", storageStatus);
    downloadStatus = CHIP_ZCL_STATUS_INVALID_IMAGE;
    scheduleUpgradeEnd();
    return;
  }

  ChipZclOtaBootloadFileHeaderInfo_t fileHeaderInfo;
  storageStatus = chipZclOtaBootloadFetchFileHeaderInfo(bytes, &fileHeaderInfo);
  if (storageStatus != CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_VALID) {
    chipAfPluginOtaBootloadClientPrintln("Verify failed: Error fetching header (%u)", storageStatus);
    downloadStatus = CHIP_ZCL_STATUS_INVALID_IMAGE;
  } else if (fileHeaderInfo.version != CHIP_ZCL_OTA_BOOTLOAD_FILE_VERSION) {
    chipAfPluginOtaBootloadClientPrintln("Verify failed: version mismatch (%u != %u)",
                                          fileHeaderInfo.version,
                                          CHIP_ZCL_OTA_BOOTLOAD_FILE_VERSION);
    downloadStatus = CHIP_ZCL_STATUS_INVALID_IMAGE;
  } else if (fileHeaderInfo.spec.manufacturerCode != fileSpecToDownload.manufacturerCode) {
    chipAfPluginOtaBootloadClientPrintln("Verify failed: manufacturerCode mismatch (%u != %u)",
                                          fileHeaderInfo.spec.manufacturerCode,
                                          fileSpecToDownload.manufacturerCode);
    downloadStatus = CHIP_ZCL_STATUS_INVALID_IMAGE;
  } else if (fileHeaderInfo.spec.type != fileSpecToDownload.type) {
    chipAfPluginOtaBootloadClientPrintln("Verify failed: imageType mismatch (%u != %u)",
                                          fileHeaderInfo.spec.type,
                                          fileSpecToDownload.type);
    downloadStatus = CHIP_ZCL_STATUS_INVALID_IMAGE;
  } else if (fileHeaderInfo.spec.version != fileSpecToDownload.version) {
    chipAfPluginOtaBootloadClientPrintln("Verify failed: imageVersion mismatch (%u != %u)",
                                          fileHeaderInfo.spec.version,
                                          fileSpecToDownload.version);
    downloadStatus = CHIP_ZCL_STATUS_INVALID_IMAGE;
  } else if (fileHeaderInfo.stackVersion != CHIP_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP) {
    chipAfPluginOtaBootloadClientPrintln("Verify failed: stackVersion mismatch (%u != %u)",
                                          fileHeaderInfo.stackVersion,
                                          CHIP_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP);
    downloadStatus = CHIP_ZCL_STATUS_INVALID_IMAGE;
  } else if (fileHeaderInfo.fileSize != storageFileInfo.size) {
    chipAfPluginOtaBootloadClientPrintln("Verify failed: fileSize mismatch (%u != %u)",
                                          fileHeaderInfo.fileSize,
                                          storageFileInfo.size);
    downloadStatus = CHIP_ZCL_STATUS_INVALID_IMAGE;
  } else if (fileHeaderInfo.securityCredentialVersion != CHIP_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP) {
    chipAfPluginOtaBootloadClientPrintln("Verify failed: securityCredentialVersion mismatch (%u != %u)",
                                          fileHeaderInfo.securityCredentialVersion,
                                          CHIP_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP);
    downloadStatus = CHIP_ZCL_STATUS_INVALID_IMAGE;
  } else {
    chipAfPluginOtaBootloadClientPrintln("download verified");
    downloadStatus = CHIP_ZCL_STATUS_SUCCESS;
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
  chipAfPluginOtaBootloadClientPrintln("Download Complete: %u", downloadStatus);
  ChipStatus status
    = chipZclOtaBootloadClientDownloadCompleteCallback(&fileSpecToDownload,
                                                        downloadStatus);
  // Only accept the application layer's status if we have not already encountered
  // some error
  if (downloadStatus == CHIP_ZCL_STATUS_SUCCESS) {
    downloadStatus = status;
  }
  ChipZclDestination_t destination;
  serverInfoToZclDestination(&destination);
  ChipZclClusterOtaBootloadServerCommandUpgradeEndRequest_t request = {
    .status = downloadStatus,
    .manufacturerId = fileSpecToDownload.manufacturerCode,
    .imageType = fileSpecToDownload.type,
    .fileVersion = fileSpecToDownload.version,
  };
  status
    = chipZclSendClusterOtaBootloadServerCommandUpgradeEndRequest(&destination,
                                                                   &request,
                                                                   upgradeEndResponseHandler);
  if (status != CHIP_SUCCESS) {
    // If we failed to submit the message for transmission, it could be
    // something on our side, so let's just try again later. We jitter the
    // response since giving the stack some time to breathe could solve our
    // problem.
    chipAfPluginOtaBootloadClientPrintln("Download command fail: %u", status);
    scheduleUpgradeEnd();
  } else if (downloadStatus != CHIP_ZCL_STATUS_SUCCESS) {
    // If our downloadStatus is not successful, then we are going to get a
    // default response from the server, so we should kick off the next image
    // query here.
    chipAfPluginOtaBootloadClientPrintln("Download fail: %u", downloadStatus);
    scheduleQueryNextImage(false); // addJitter?
  }
}

static void upgradeEndResponseHandler(ChipZclMessageStatus_t status,
                                      const ChipZclCommandContext_t *context,
                                      const ChipZclClusterOtaBootloadServerCommandUpgradeEndResponse_t *response)
{
  switch (status) {
    case CHIP_ZCL_MESSAGE_STATUS_DISCOVERY_TIMEOUT:
    case CHIP_ZCL_MESSAGE_STATUS_COAP_TIMEOUT:
    case CHIP_ZCL_MESSAGE_STATUS_COAP_RESET:
      // If we couldn't reach the server for some reason, then try again to send
      // the command.
      // TODO: we should probably add this to our error threshold!
      chipAfPluginOtaBootloadClientPrintln("Timeout/reset");
      scheduleUpgradeEnd();
      break;
    case CHIP_ZCL_MESSAGE_STATUS_COAP_ACK:
      chipAfPluginOtaBootloadClientPrintln("COAP ack");
      // This means that our request was ACK'd, but the actual response is coming.
      break;
    case CHIP_ZCL_MESSAGE_STATUS_COAP_RESPONSE:
      if ((context != NULL) && (context->code != CHIP_COAP_CODE_204_CHANGED)) {
        chipAfPluginOtaBootloadClientPrintln("COAP response: OTA server did not respond with 2.04 to upgradeEndRequest");
        scheduleDiscoverServer(true, true); // eraseServerInfo? addJitter?
        break;
      }
      chipAfPluginOtaBootloadClientPrintln("COAP response");
      processUpgradeEndResponse(response);
      break;
    default:
      assert(false);
  }
}

static void processUpgradeEndResponse(const ChipZclClusterOtaBootloadServerCommandUpgradeEndResponse_t *response)
{
  if (!HAVE_FILE_SPEC_TO_DOWNLOAD() || (response == NULL)) {
    chipAfPluginOtaBootloadClientPrintln("UpgradeEnd with no file spec to download");
    scheduleDiscoverServer(true, true); // eraseServerInfo? addJitter?
    return;
  }

  // This logic is brought you by 11.11.4 in the OTA spec. If currentTime is 0,
  // then upgradeTime is the offset, in seconds, of when we should perform the
  // upgrade. Else if upgradeTime is not 0 and not 0xFFFFFFFF, then we should
  // treat the two values like UTC times. Else if the upgrade time is 0xFFFFFFFF,
  // then that means to check back in later to see if we can upgrade.
  if (response->upgradeTime == 0xFFFFFFFF) {
    chipAfPluginOtaBootloadClientPrintln("Will retry upgradeEnd");
    scheduleUpgradeEnd();
    return;
  }

  if (fileSpecToDownload.manufacturerCode != response->manufacturerId
      || fileSpecToDownload.type != response->imageType
      || fileSpecToDownload.version != response->fileVersion) {
    // TODO: handle default response that may be sent back (see 11.13.6.4).
    // As a workaround for this issue, let's just reschedule the download.
    chipAfPluginOtaBootloadClientPrintln("Mismatched firmware image properties %u:%u, %u:%u, %u:%u", fileSpecToDownload.manufacturerCode,
                                          response->manufacturerId, fileSpecToDownload.type, response->imageType, fileSpecToDownload.version, response->fileVersion);
    scheduleQueryNextImage(false);
    return;
  }

  chipAfPluginOtaBootloadClientPrintln("Will bootload in %d seconds", response->upgradeTime - response->currentTime);
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
  chipZclOtaBootloadClientPreBootloadCallback(&fileSpecToDownload);

  chipAfPluginOtaBootloadClientPrintln("Loading new image");
  ChipStatus status = halAppBootloaderInstallNewImage();
  // This call should not return!
  (void)status;
}

// -----------------------------------------------------------------------------
// Public API downward

void chZclOtaBootloadClientInitCallback(void)
{
  ERASE_SERVER_INFO();
  ERASE_FILE_SPEC_TO_DOWNLOAD();
  ERASE_DOWNLOAD_STATUS();
  if (chipZclOtaBootloadClientSetVersionInfoCallback()) {
    chipAfPluginOtaBootloadClientPrintln("Failure initializing OTA attributes");
  }
  setState(STATE_NONE, 0); // run now
}

void chZclOtaBootloadClientNetworkStatusCallback(ChipNetworkStatus newNetworkStatus,
                                                 ChipNetworkStatus oldNetworkStatus,
                                                 ChipJoinFailureReason reason)
{
#ifdef CHIP_AF_PLUGIN_OTA_BOOTLOAD_CLIENT_AUTO_START
  if (newNetworkStatus == CHIP_JOINED_NETWORK_ATTACHED) {
    scheduleDiscoverServer(false, true); // eraseServerInfo? addJitter?
  }
#endif
}

#ifdef CHIP_AF_PLUGIN_POLLING
bool chZclOtaBootloadClientOkToLongPoll(uint32_t durationMs)
{
  return state != STATE_DOWNLOAD;
}
#endif

void chZclOtaBootloadClientEventHandler(void)
{
  chipEventControlSetInactive(chZclOtaBootloadClientEventControl);

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
