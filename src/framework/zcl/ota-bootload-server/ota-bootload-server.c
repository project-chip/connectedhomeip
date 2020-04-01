/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE
#include CHIP_AF_API_BUFFER_MANAGEMENT

#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#else
  #define chipAfPluginOtaBootloadServerPrint(...)
  #define chipAfPluginOtaBootloadServerPrintln(...)
  #define chipAfPluginOtaBootloadServerFlush()
  #define chipAfPluginOtaBootloadServerDebugExec(x)
  #define chipAfPluginOtaBootloadServerPrintBuffer(buffer, len, withSpace)
  #define chipAfPluginOtaBootloadServerPrintString(buffer)
#endif

#ifdef CHIP_SCRIPTED_TEST
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

ChipEventControl chZclOtaBootloadServerImageNotifyEventControl;
bool forceNotify = false;

// -----------------------------------------------------------------------------
// Private API

static uint8_t getPayloadTypeForFileSpec(const ChipZclOtaBootloadFileSpec_t *fileSpec)
{
  // See section 11.13.3.2.1 ImageNotify Command Payload Type. This logic should
  // be communicated through the documentation for
  // chipZclOtaBootloadServerGetImageNotifyInfoCallback.
  uint8_t payloadType = 0x00;
  if (fileSpec->manufacturerCode != CHIP_ZCL_MANUFACTURER_CODE_NULL) {
    payloadType++;
    if (fileSpec->type != CHIP_ZCL_OTA_BOOTLOAD_FILE_TYPE_WILDCARD) {
      payloadType++;
      if (fileSpec->version != CHIP_ZCL_OTA_BOOTLOAD_FILE_VERSION_NULL) {
        payloadType++;
      }
    }
  }
  return payloadType;
}

// -----------------------------------------------------------------------------
// Public API downward

void chZclOtaBootloadServerNetworkStatusCallback(ChipNetworkStatus newNetworkStatus,
                                                 ChipNetworkStatus oldNetworkStatus,
                                                 ChipJoinFailureReason reason)
{
  if (newNetworkStatus == CHIP_JOINED_NETWORK_ATTACHED) {
    chipEventControlSetActive(chZclOtaBootloadServerImageNotifyEventControl);
  } else if (newNetworkStatus == CHIP_NO_NETWORK) {
    chipEventControlSetInactive(chZclOtaBootloadServerImageNotifyEventControl);
  }
}

void chZclOtaBootloadServerImageNotifyEventHandler(void)
{
  // Get ImageNotify stuff from application.
  extern const ChipIpv6Address allCoapNodesSiteScope;
  ChipIpv6Address address = allCoapNodesSiteScope;
  ChipZclOtaBootloadStorageInfo_t info;
  ChipZclOtaBootloadFileSpec_t fileSpecs[IMAGE_NOTIFY_FILE_COUNT];
  chipZclOtaBootloadStorageGetInfo(&info, fileSpecs, COUNTOF(fileSpecs));
  for (uint8_t i = 0; i < info.fileCount; i++) {
    if (chipZclOtaBootloadServerGetImageNotifyInfoCallback(&address,
                                                            &fileSpecs[i])) {
      ChipZclDestination_t destination = {
        .network = {
          .address = { { 0, } }, // filled in below
          .flags = CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG,
          .port = CHIP_COAP_PORT,
        },
        .application = {
          .data = {
            .endpointId = 1,
          },
          .type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT,
        },
      };
      destination.network.address = address;
      ChipZclClusterOtaBootloadClientCommandImageNotifyRequest_t request = {
        .payloadType = getPayloadTypeForFileSpec(&fileSpecs[i]),
        .queryJitter = DEFAULT_QUERY_JITTER,
        .manufacturerId = fileSpecs[i].manufacturerCode,
        .imageType = fileSpecs[i].type,
        .newFileVersion = fileSpecs[i].version,
      };
      if (forceNotify) {
        request.queryJitter = 100;
      }
      ChipStatus status
        = chipZclSendClusterOtaBootloadClientCommandImageNotifyRequest(&destination,
                                                                        &request,
                                                                        NULL);
      // If we fail to send the message, then just try again later.
      (void)status;
    }
    forceNotify = false;
  }

  chipEventControlSetDelayMinutes(chZclOtaBootloadServerImageNotifyEventControl,
                                   CHIP_AF_PLUGIN_OTA_BOOTLOAD_SERVER_IMAGE_NOTIFY_PERIOD_MINUTES);
}

// Handle: ota-bootload-server notify
void otaBootloadServerForceImageNotify()
{
  forceNotify = true;
  chipEventControlSetDelayMinutes(chZclOtaBootloadServerImageNotifyEventControl, 0);
}

// -------------------------------------
// Command handling

void chipZclClusterOtaBootloadServerCommandQuerySpecificFileRequestHandler(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterOtaBootloadServerCommandQuerySpecificFileRequest_t *request)
{
  // TODO: Correctly implement this using the chipZclClusterOtaBootloadServerCommandQueryNextImageRequestHandler as a template
  chipAfPluginOtaBootloadServerPrintln("QuerySpecificFileRequest");
  ChipZclClusterOtaBootloadServerCommandQuerySpecificFileResponse_t response = {
    .status = (request->manufacturerId == 0x1234) ? CHIP_ZCL_STATUS_SUCCESS : CHIP_ZCL_STATUS_NO_IMAGE_AVAILABLE,
    .fileVersion = (request->manufacturerId == 0x1234) ? request->fileVersion : 0xFFFFFFFF,
    .imageSize = 0, // conditionally filled in below
    .fileUri = { 0, NULL } // conditionally filled in below
  };

  ChipStatus status
    = chipZclSendClusterOtaBootloadServerCommandQuerySpecificFileResponse(context, &response);
  // TODO: what if we fail to send this message?
  assert(status == CHIP_SUCCESS);
}

void chipZclClusterOtaBootloadServerCommandQueryNextImageRequestHandler(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterOtaBootloadServerCommandQueryNextImageRequest_t *request)
{
  chipAfPluginOtaBootloadServerPrintln("QueryNextImageRequest");
  ChipZclOtaBootloadFileSpec_t currentFileSpec = {
    .manufacturerCode = request->manufacturerId,
    .type = request->imageType,
    .version = request->currentFileVersion,
  };
  ChipZclOtaBootloadFileSpec_t nextFileSpec = chipZclOtaBootloadFileSpecNull;
  ChipZclStatus_t zclStatus
    = chipZclOtaBootloadServerGetNextImageCallback(&context->remoteAddress,
                                                    &currentFileSpec,
                                                    &nextFileSpec);
  ChipZclClusterOtaBootloadServerCommandQueryNextImageResponse_t response = {
    .status = zclStatus,
    .fileVersion = nextFileSpec.version,
    .imageSize = 0, // conditionally filled in below
    .fileUri = { 0, NULL } // conditionally filled in below
  };
  if (response.status == CHIP_ZCL_STATUS_SUCCESS) {
    ChipZclOtaBootloadStorageFileInfo_t storageFileInfo;
    ChipZclOtaBootloadStorageStatus_t storageStatus
      = chipZclOtaBootloadStorageFind(&nextFileSpec, &storageFileInfo);
    // TODO: handle this case more gracefully.
    assert(storageStatus == CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
    response.imageSize = storageFileInfo.size;

    Buffer message = emAllocateBuffer(CH_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_LENGTH);
    assert(message != NULL_BUFFER);

    response.fileUri.ptr = emGetBufferPointer(message);
    // Add in the leading '/' and account for the null terminator in the length
    assert(snprintf((char *)response.fileUri.ptr,
                    CH_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_LENGTH + 2,
                    CH_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_FORMAT,
                    "/"CH_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI,
                    nextFileSpec.manufacturerCode,
                    nextFileSpec.type,
                    nextFileSpec.version) == CH_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_LENGTH + 1);
    response.fileUri.length = CH_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_LENGTH + 1;
  }

  ChipStatus status
    = chipZclSendClusterOtaBootloadServerCommandQueryNextImageResponse(context,
                                                                        &response);
  // TODO: what if we fail to send this message?
  assert(status == CHIP_SUCCESS);
}

void chipZclClusterOtaBootloadServerCommandUpgradeEndRequestHandler(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterOtaBootloadServerCommandUpgradeEndRequest_t *request)
{
  ChipZclOtaBootloadFileSpec_t fileSpec = {
    .manufacturerCode = request->manufacturerId,
    .type = request->imageType,
    .version = request->fileVersion,
  };
  uint32_t upgradeTime
    = chipZclOtaBootloadServerUpgradeEndRequestCallback(&context->remoteAddress,
                                                         &fileSpec,
                                                         request->status);
  ChipStatus status;
  if (request->status != CHIP_ZCL_STATUS_SUCCESS) {
    // See 17-07023 7.13.6.4 for this default response mandate.
    status = chipZclSendDefaultResponse(context, CHIP_ZCL_STATUS_SUCCESS);
  } else {
    // See 17-07023 7.11.4 for discussion regarding these currentTime and upgradeTime
    // parameters.
    ChipZclClusterOtaBootloadServerCommandUpgradeEndResponse_t response = {
      .manufacturerId = fileSpec.manufacturerCode,
      .imageType = fileSpec.type,
      .fileVersion = fileSpec.version,
      .currentTime = 0, // this means upgradeTime is an offset from now
      .upgradeTime = upgradeTime,
    };
    status
      = chipZclSendClusterOtaBootloadServerCommandUpgradeEndResponse(context,
                                                                      &response);
  }
  // TODO: what if we fail to send this message?
  assert(status == CHIP_SUCCESS);
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
static bool readOtaUri(const uint8_t *uri, ChipZclOtaBootloadFileSpec_t *fileSpec)
{
  if (MEMCOMPARE(CH_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI,
                 uri,
                 CH_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI_LENGTH)
      != 0) {
    return true;
  }
  uri += CH_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI_LENGTH + 1; // account for "<BASE_URI>/"
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

void chZclOtaBootloadServerDownloadHandler(ChipCoapCode code,
                                           uint8_t *uri,
                                           ChipCoapReadOptions *options,
                                           const uint8_t *payload,
                                           uint16_t payloadLength,
                                           const ChipCoapRequestInfo *info)
{
  assert(code == CHIP_COAP_CODE_GET);

  ChipStatus status;
  chipAfPluginOtaBootloadServerPrintln("Next block request");

  ChipCoapBlockOption blockOption;
  if (!chipReadBlockOption(options, CHIP_COAP_OPTION_BLOCK2, &blockOption)) {
    chipAfPluginOtaBootloadServerPrintln("Received bad request");
    status = chZclRespond400BadRequest(info);
    assert(status == CHIP_SUCCESS);
    return;
  }

  ChipZclOtaBootloadFileSpec_t nextImageFileSpec;
  if (readOtaUri(uri, &nextImageFileSpec)) {
    chipAfPluginOtaBootloadServerPrintln("Received invalid URI: %s", uri);
    status = chZclRespond400BadRequest(info);
    assert(status == CHIP_SUCCESS);
    return;
  }

  ChipZclOtaBootloadStorageFileInfo_t fileInfo;
  if (chipZclOtaBootloadStorageFind(&nextImageFileSpec, &fileInfo)
      != CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
    chipAfPluginOtaBootloadServerPrintln("Request for non-existent file: %04x-%04x-%08x",
                                          nextImageFileSpec.manufacturerCode,
                                          nextImageFileSpec.type,
                                          nextImageFileSpec.version);
    status = chZclRespond404NotFound(info);
    assert(status == CHIP_SUCCESS);
    return;
  }

  size_t blockOffset = chipBlockOptionOffset(&blockOption);
  if (blockOffset > fileInfo.size) {
    chipAfPluginOtaBootloadServerPrintln("Received block offset (%d) greater than file size (%d)",
                                          blockOffset,
                                          fileInfo.size);
    status = chZclRespond402BadOption(info);
    assert(status == CHIP_SUCCESS);
    return;
  }

  size_t blockSize = chipBlockOptionSize(&blockOption);
  size_t undownloadedFileSize = fileInfo.size - blockOffset;
  size_t dataSize = (undownloadedFileSize < blockSize ? undownloadedFileSize : blockSize);
  uint8_t data[MAX_RESPONSE_BLOCK_SIZE];
  if (dataSize > MAX_RESPONSE_BLOCK_SIZE
      || (chipZclOtaBootloadStorageRead(&nextImageFileSpec,
                                         blockOffset,
                                         data,
                                         dataSize)
          != CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS)) {
    chipAfPluginOtaBootloadServerPrintln("Unable to read from OTA storage");
    status = chZclRespond500InternalServerError(info);
    assert(status == CHIP_SUCCESS);
    return;
  }

  ChipCoapOption responseOptions[2];

  // Options must be added in the increasing order of Option Number
  chipInitCoapOption(&(responseOptions[0]),
                      CHIP_COAP_OPTION_CONTENT_FORMAT,
                      CHIP_COAP_CONTENT_FORMAT_OCTET_STREAM);
  chipInitCoapOption(&(responseOptions[1]),
                      CHIP_COAP_OPTION_BLOCK2,
                      chipBlockOptionValue(undownloadedFileSize > blockSize,
                                            blockOption.logSize,
                                            blockOption.number));
  chipAfPluginOtaBootloadServerPrintln("Accessing block: 0x%x\n", blockOption.number);

  status = chZclRespond205ContentCborWithOptions(info, responseOptions, COUNTOF(responseOptions), data, dataSize);
  if (status != CHIP_SUCCESS) {
    chipAfPluginOtaBootloadServerPrintln("Assertion error: %d", status);
    assert(0);
  }
}
