/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE
#include EMBER_AF_API_BUFFER_MANAGEMENT

#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#else
  #define emberAfPluginOtaBootloadServerPrint(...)
  #define emberAfPluginOtaBootloadServerPrintln(...)
  #define emberAfPluginOtaBootloadServerFlush()
  #define emberAfPluginOtaBootloadServerDebugExec(x)
  #define emberAfPluginOtaBootloadServerPrintBuffer(buffer, len, withSpace)
  #define emberAfPluginOtaBootloadServerPrintString(buffer)
#endif

#ifdef EMBER_SCRIPTED_TEST
  #include "ota-bootload-server-test.h"
#else
  #include "thread-callbacks.h"
#endif

// TODO: do we need to support QuerySpecificFile command?
// TODO: add a callback for each block received so app can resize block?

// -----------------------------------------------------------------------------
// Constants

// See 11.11.1 (QueryJitter Parameter) from the OTA spec.
// "The server chooses the parameter value between 1 and 100 (inclusively) and
// includes it in the Image Notify Command. On receipt of the command, the
// client will examine other information (the manufacturer code and image type)
// to determine if they match its own values. If they do not, it SHALL discard
// the command and no further processing SHALL continue. If they do match then
// it will determine whether or not it SHOULD query the upgrade server. It does
// this by randomly choosing a number between 1 and 100 and comparing it to the
// value of the QueryJitter parameter received. If it is less than or equal to
// the QueryJitter value from the server, it SHALL continue with the query
// process. If not, then it SHALL discard the command and no further processing
// SHALL continue."
// I picked a value right in the middle of the range, because I am lazy.
#define DEFAULT_QUERY_JITTER 50

// See 11.13.4.2.1 (Query Next Image Request Command Field Control) from the OTA
// spec.
#define QUERY_NEXT_IMAGE_REQUEST_FIELD_CONTROL_HARDWARE_VERSION_PRESENT BIT(0)

// This is super duper arbitrary. It should be updated as needed. However, we
// should think about the MTU of the transport layer below us.
// Increased to 256 which results in a 2min OTA of a 250K file.
#define MAX_RESPONSE_BLOCK_SIZE 256

// Set this to the number of files that are expected to be stored in the OTA server.
#define IMAGE_NOTIFY_FILE_COUNT 10

// -----------------------------------------------------------------------------
// Globals

EmberEventControl emZclOtaBootloadServerImageNotifyEventControl;
bool forceNotify = false;

// -----------------------------------------------------------------------------
// Private API

static uint8_t getPayloadTypeForFileSpec(const EmberZclOtaBootloadFileSpec_t *fileSpec)
{
  // See section 11.13.3.2.1 ImageNotify Command Payload Type. This logic should
  // be communicated through the documentation for
  // emberZclOtaBootloadServerGetImageNotifyInfoCallback.
  uint8_t payloadType = 0x00;
  if (fileSpec->manufacturerCode != EMBER_ZCL_MANUFACTURER_CODE_NULL) {
    payloadType++;
    if (fileSpec->type != EMBER_ZCL_OTA_BOOTLOAD_FILE_TYPE_WILDCARD) {
      payloadType++;
      if (fileSpec->version != EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION_NULL) {
        payloadType++;
      }
    }
  }
  return payloadType;
}

// -----------------------------------------------------------------------------
// Public API downward

void emZclOtaBootloadServerNetworkStatusCallback(EmberNetworkStatus newNetworkStatus,
                                                 EmberNetworkStatus oldNetworkStatus,
                                                 EmberJoinFailureReason reason)
{
  if (newNetworkStatus == EMBER_JOINED_NETWORK_ATTACHED) {
    emberEventControlSetActive(emZclOtaBootloadServerImageNotifyEventControl);
  } else if (newNetworkStatus == EMBER_NO_NETWORK) {
    emberEventControlSetInactive(emZclOtaBootloadServerImageNotifyEventControl);
  }
}

void emZclOtaBootloadServerImageNotifyEventHandler(void)
{
  // Get ImageNotify stuff from application.
  extern const EmberIpv6Address allCoapNodesSiteScope;
  EmberIpv6Address address = allCoapNodesSiteScope;
  EmberZclOtaBootloadStorageInfo_t info;
  EmberZclOtaBootloadFileSpec_t fileSpecs[IMAGE_NOTIFY_FILE_COUNT];
  emberZclOtaBootloadStorageGetInfo(&info, fileSpecs, COUNTOF(fileSpecs));
  for (uint8_t i = 0; i < info.fileCount; i++) {
    if (emberZclOtaBootloadServerGetImageNotifyInfoCallback(&address,
                                                            &fileSpecs[i])) {
      EmberZclDestination_t destination = {
        .network = {
          .address = { { 0, } }, // filled in below
          .flags = EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG,
          .port = EMBER_COAP_PORT,
        },
        .application = {
          .data = {
            .endpointId = 1,
          },
          .type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT,
        },
      };
      destination.network.address = address;
      EmberZclClusterOtaBootloadClientCommandImageNotifyRequest_t request = {
        .payloadType = getPayloadTypeForFileSpec(&fileSpecs[i]),
        .queryJitter = DEFAULT_QUERY_JITTER,
        .manufacturerId = fileSpecs[i].manufacturerCode,
        .imageType = fileSpecs[i].type,
        .newFileVersion = fileSpecs[i].version,
      };
      if (forceNotify) {
        request.queryJitter = 100;
      }
      EmberStatus status
        = emberZclSendClusterOtaBootloadClientCommandImageNotifyRequest(&destination,
                                                                        &request,
                                                                        NULL);
      // If we fail to send the message, then just try again later.
      (void)status;
    }
    forceNotify = false;
  }

  emberEventControlSetDelayMinutes(emZclOtaBootloadServerImageNotifyEventControl,
                                   EMBER_AF_PLUGIN_OTA_BOOTLOAD_SERVER_IMAGE_NOTIFY_PERIOD_MINUTES);
}

// Handle: ota-bootload-server notify
void otaBootloadServerForceImageNotify()
{
  forceNotify = true;
  emberEventControlSetDelayMinutes(emZclOtaBootloadServerImageNotifyEventControl, 0);
}

// -------------------------------------
// Command handling

void emberZclClusterOtaBootloadServerCommandQuerySpecificFileRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandQuerySpecificFileRequest_t *request)
{
  // TODO: Correctly implement this using the emberZclClusterOtaBootloadServerCommandQueryNextImageRequestHandler as a template
  emberAfPluginOtaBootloadServerPrintln("QuerySpecificFileRequest");
  EmberZclClusterOtaBootloadServerCommandQuerySpecificFileResponse_t response = {
    .status = (request->manufacturerId == 0x1234) ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE,
    .fileVersion = (request->manufacturerId == 0x1234) ? request->fileVersion : 0xFFFFFFFF,
    .imageSize = 0, // conditionally filled in below
    .fileUri = { 0, NULL } // conditionally filled in below
  };

  EmberStatus status
    = emberZclSendClusterOtaBootloadServerCommandQuerySpecificFileResponse(context, &response);
  // TODO: what if we fail to send this message?
  assert(status == EMBER_SUCCESS);
}

void emberZclClusterOtaBootloadServerCommandQueryNextImageRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandQueryNextImageRequest_t *request)
{
  emberAfPluginOtaBootloadServerPrintln("QueryNextImageRequest");
  EmberZclOtaBootloadFileSpec_t currentFileSpec = {
    .manufacturerCode = request->manufacturerId,
    .type = request->imageType,
    .version = request->currentFileVersion,
  };
  EmberZclOtaBootloadFileSpec_t nextFileSpec = emberZclOtaBootloadFileSpecNull;
  EmberZclStatus_t zclStatus
    = emberZclOtaBootloadServerGetNextImageCallback(&context->remoteAddress,
                                                    &currentFileSpec,
                                                    &nextFileSpec);
  EmberZclClusterOtaBootloadServerCommandQueryNextImageResponse_t response = {
    .status = zclStatus,
    .fileVersion = nextFileSpec.version,
    .imageSize = 0, // conditionally filled in below
    .fileUri = { 0, NULL } // conditionally filled in below
  };
  if (response.status == EMBER_ZCL_STATUS_SUCCESS) {
    EmberZclOtaBootloadStorageFileInfo_t storageFileInfo;
    EmberZclOtaBootloadStorageStatus_t storageStatus
      = emberZclOtaBootloadStorageFind(&nextFileSpec, &storageFileInfo);
    // TODO: handle this case more gracefully.
    assert(storageStatus == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
    response.imageSize = storageFileInfo.size;

    Buffer message = emAllocateBuffer(EM_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_LENGTH);
    assert(message != NULL_BUFFER);

    response.fileUri.ptr = emGetBufferPointer(message);
    // Add in the leading '/' and account for the null terminator in the length
    assert(snprintf((char *)response.fileUri.ptr,
                    EM_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_LENGTH + 2,
                    EM_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_FORMAT,
                    "/"EM_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI,
                    nextFileSpec.manufacturerCode,
                    nextFileSpec.type,
                    nextFileSpec.version) == EM_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_LENGTH + 1);
    response.fileUri.length = EM_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_LENGTH + 1;
  }

  EmberStatus status
    = emberZclSendClusterOtaBootloadServerCommandQueryNextImageResponse(context,
                                                                        &response);
  // TODO: what if we fail to send this message?
  assert(status == EMBER_SUCCESS);
}

void emberZclClusterOtaBootloadServerCommandUpgradeEndRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandUpgradeEndRequest_t *request)
{
  EmberZclOtaBootloadFileSpec_t fileSpec = {
    .manufacturerCode = request->manufacturerId,
    .type = request->imageType,
    .version = request->fileVersion,
  };
  uint32_t upgradeTime
    = emberZclOtaBootloadServerUpgradeEndRequestCallback(&context->remoteAddress,
                                                         &fileSpec,
                                                         request->status);
  EmberStatus status;
  if (request->status != EMBER_ZCL_STATUS_SUCCESS) {
    // See 17-07023 7.13.6.4 for this default response mandate.
    status = emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  } else {
    // See 17-07023 7.11.4 for discussion regarding these currentTime and upgradeTime
    // parameters.
    EmberZclClusterOtaBootloadServerCommandUpgradeEndResponse_t response = {
      .manufacturerId = fileSpec.manufacturerCode,
      .imageType = fileSpec.type,
      .fileVersion = fileSpec.version,
      .currentTime = 0, // this means upgradeTime is an offset from now
      .upgradeTime = upgradeTime,
    };
    status
      = emberZclSendClusterOtaBootloadServerCommandUpgradeEndResponse(context,
                                                                      &response);
  }
  // TODO: what if we fail to send this message?
  assert(status == EMBER_SUCCESS);
}

// -------------------------------------
// Block transfer handling

static uint8_t asciiToValue(uint8_t digit)
{
  if (('0' <= digit) && (digit <= '9')) {
    return digit - '0';
  }
  if (('a' <= digit) && (digit <= 'f')) {
    return digit - 'a' + 10;
  }
  if (('A' <= digit) && (digit <= 'F')) {
    return digit - 'A' + 10;
  }
  return 0xFF;
}

static bool readHexUInt16(const uint8_t *str, uint16_t *value)
{
  uint8_t count = 4;
  uint8_t nextDigit;
  *value = 0;
  while (count--) {
    nextDigit = asciiToValue(*(str++));
    if (nextDigit == 0xFF) {
      return true;
    }
    *value = (*value << 4) + nextDigit;
  }
  return false;
}

static bool readHexUInt32(const uint8_t *str, uint32_t *value)
{
  uint8_t count = 8;
  uint8_t nextDigit;
  *value = 0;
  while (count--) {
    nextDigit = asciiToValue(*(str++));
    if (nextDigit == 0xFF) {
      return true;
    }
    *value = (*value << 4) + nextDigit;
  }
  return false;
}

// This function expects a uri to match the format string "%s/%04X-%04X-%08X".
static bool readOtaUri(const uint8_t *uri, EmberZclOtaBootloadFileSpec_t *fileSpec)
{
  if (MEMCOMPARE(EM_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI,
                 uri,
                 EM_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI_LENGTH)
      != 0) {
    return true;
  }
  uri += EM_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI_LENGTH + 1; // account for "<BASE_URI>/"
  if (readHexUInt16(uri, &(fileSpec->manufacturerCode))) {
    return true;
  }
  uri += 5; // account for "xxxx-"
  if (readHexUInt16(uri, &(fileSpec->type))) {
    return true;
  }
  uri += 5; // account for "xxxx-"
  if (readHexUInt32(uri, &(fileSpec->version))) {
    return true;
  }
  uri += 8; // account for "xxxxxxxx"

  return false;
}

void emZclOtaBootloadServerDownloadHandler(EmberCoapCode code,
                                           uint8_t *uri,
                                           EmberCoapReadOptions *options,
                                           const uint8_t *payload,
                                           uint16_t payloadLength,
                                           const EmberCoapRequestInfo *info)
{
  assert(code == EMBER_COAP_CODE_GET);

  EmberStatus status;
  emberAfPluginOtaBootloadServerPrintln("Next block request");

  EmberCoapBlockOption blockOption;
  if (!emberReadBlockOption(options, EMBER_COAP_OPTION_BLOCK2, &blockOption)) {
    emberAfPluginOtaBootloadServerPrintln("Received bad request");
    status = emZclRespond400BadRequest(info);
    assert(status == EMBER_SUCCESS);
    return;
  }

  EmberZclOtaBootloadFileSpec_t nextImageFileSpec;
  if (readOtaUri(uri, &nextImageFileSpec)) {
    emberAfPluginOtaBootloadServerPrintln("Received invalid URI: %s", uri);
    status = emZclRespond400BadRequest(info);
    assert(status == EMBER_SUCCESS);
    return;
  }

  EmberZclOtaBootloadStorageFileInfo_t fileInfo;
  if (emberZclOtaBootloadStorageFind(&nextImageFileSpec, &fileInfo)
      != EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
    emberAfPluginOtaBootloadServerPrintln("Request for non-existent file: %04x-%04x-%08x",
                                          nextImageFileSpec.manufacturerCode,
                                          nextImageFileSpec.type,
                                          nextImageFileSpec.version);
    status = emZclRespond404NotFound(info);
    assert(status == EMBER_SUCCESS);
    return;
  }

  size_t blockOffset = emberBlockOptionOffset(&blockOption);
  if (blockOffset > fileInfo.size) {
    emberAfPluginOtaBootloadServerPrintln("Received block offset (%d) greater than file size (%d)",
                                          blockOffset,
                                          fileInfo.size);
    status = emZclRespond402BadOption(info);
    assert(status == EMBER_SUCCESS);
    return;
  }

  size_t blockSize = emberBlockOptionSize(&blockOption);
  size_t undownloadedFileSize = fileInfo.size - blockOffset;
  size_t dataSize = (undownloadedFileSize < blockSize ? undownloadedFileSize : blockSize);
  uint8_t data[MAX_RESPONSE_BLOCK_SIZE];
  if (dataSize > MAX_RESPONSE_BLOCK_SIZE
      || (emberZclOtaBootloadStorageRead(&nextImageFileSpec,
                                         blockOffset,
                                         data,
                                         dataSize)
          != EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS)) {
    emberAfPluginOtaBootloadServerPrintln("Unable to read from OTA storage");
    status = emZclRespond500InternalServerError(info);
    assert(status == EMBER_SUCCESS);
    return;
  }

  EmberCoapOption responseOptions[2];

  // Options must be added in the increasing order of Option Number
  emberInitCoapOption(&(responseOptions[0]),
                      EMBER_COAP_OPTION_CONTENT_FORMAT,
                      EMBER_COAP_CONTENT_FORMAT_OCTET_STREAM);
  emberInitCoapOption(&(responseOptions[1]),
                      EMBER_COAP_OPTION_BLOCK2,
                      emberBlockOptionValue(undownloadedFileSize > blockSize,
                                            blockOption.logSize,
                                            blockOption.number));
  emberAfPluginOtaBootloadServerPrintln("Accessing block: 0x%x\n", blockOption.number);

  status = emZclRespond205ContentCborWithOptions(info, responseOptions, COUNTOF(responseOptions), data, dataSize);
  if (status != EMBER_SUCCESS) {
    emberAfPluginOtaBootloadServerPrintln("Assertion error: %d", status);
    assert(0);
  }
}
