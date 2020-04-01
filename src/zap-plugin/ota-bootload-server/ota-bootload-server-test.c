/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE
#include "../ota-bootload-storage-core/ota-static-file-data.h"
#include "ota-bootload-server-test.h"

#include "stack/core/ember-stack.h"
#include "stack/core/scripted-stub.h"
#include "stack/core/parcel.h"

// -----------------------------------------------------------------------------
// Constants

// Conversion the same as in ota-bootload-client.c.
#define IMAGE_NOTIFY_PERIOD_MS \
  (EMBER_AF_PLUGIN_OTA_BOOTLOAD_SERVER_IMAGE_NOTIFY_PERIOD_MINUTES << 16)

#define IMAGE_DATA_SIZE 1024
static const uint8_t image[IMAGE_DATA_SIZE] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
  0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
  0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
  0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
  0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
  0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
  0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
  0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
  0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
  0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
  0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
  0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
  0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
  0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
  0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
  0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
  0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
  0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
  0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
  0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
  0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
  0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
  0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
  0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
  0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

#define BLOCK_SIZE_LOG 5 /* 32 */

// -----------------------------------------------------------------------------
// Globals

const EmberEventData emAppEvents[] = {
  { &emZclOtaBootloadServerImageNotifyEventControl, emZclOtaBootloadServerImageNotifyEventHandler, },
  { NULL, NULL, },
};

static const EmberZclOtaBootloadFileSpec_t theCurrentFileSpec = {
  .manufacturerCode = 0x1234,
  .type = 0xACAC,
  .version = 0xD00DF00D,
};

static const EmberZclOtaBootloadFileSpec_t theNextFileSpec = {
  .manufacturerCode = 0x1234,
  .type = 0xACAC,
  .version = 0xD00DF00E,
};

static const EmberZclOtaBootloadFileSpec_t emptyFileSpec = {
  .manufacturerCode = 0x0000,
  .type = 0x0000,
  .version = 0x00000000,
};

static const EmberIpv6Address multicastAddress = {
  { 0xFF, 0x33, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, }
};

static const EmberIpv6Address theRemoteAddress = {
  { 0xFD, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, }
};

const EmberIpv6Address allCoapNodesSiteScope = {
  { 0xFF, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd, }
};

// -----------------------------------------------------------------------------
// Scripted stubs

// From zcl-core.c
EmberStatus emberZclRespondWithPath(const EmberCoapRequestInfo *requestInfo,
                                    EmberCoapCode code,
                                    const uint8_t *path,
                                    const EmberCoapOption *options,
                                    uint8_t numberOfOptions,
                                    const uint8_t *payload,
                                    uint16_t payloadLength)
{
  return emberCoapRespondWithPath(requestInfo,
                                  code,
                                  path,
                                  options,
                                  numberOfOptions,
                                  payload,
                                  payloadLength);
}

// From ota-bootload-server.c.
static EmberNetworkStatus currentNetworkStatus = EMBER_NO_NETWORK;
#define addNetworkStatusCallbackAction(newStatus)                 \
  do {                                                            \
    addSimpleAction("NetworkStatusCallback %x %x",                \
                    emZclOtaBootloadServerNetworkStatusCallback,  \
                    3,                                            \
                    (newStatus),                                  \
                    (currentNetworkStatus),                       \
                    (0)); /* EmberJoinFailureReason - whatever */ \
    currentNetworkStatus = newStatus;                             \
  } while (0);

bool emberZclOtaBootloadServerGetImageNotifyInfoCallback(EmberIpv6Address *address,
                                                         EmberZclOtaBootloadFileSpec_t *fileSpec)
{
  *address = multicastAddress;
  *fileSpec = theCurrentFileSpec;
  return true;
}

static EmberZclClusterOtaBootloadClientCommandImageNotifyResponseHandler imageNotifyResponseHandler = NULL;
EmberStatus emberZclSendClusterOtaBootloadClientCommandImageNotifyRequest(
  const EmberZclDestination_t *destination,
  const EmberZclClusterOtaBootloadClientCommandImageNotifyRequest_t *request,
  const EmberZclClusterOtaBootloadClientCommandImageNotifyResponseHandler handler)
{
  functionCallCheck("ImageNotifyRequest", "");

  assert(!(destination->network.flags & EMBER_ZCL_USE_COAPS_FLAG));
  assert(MEMCOMPARE(destination->network.address.bytes,
                    multicastAddress.bytes,
                    sizeof(multicastAddress.bytes))
         == 0);
  assert(destination->network.flags & EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG);
  assert(destination->network.port == EMBER_COAP_PORT);
  assert(destination->application.data.endpointId == 1);
  assert(destination->application.type == EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT);

  assert(request->payloadType == 0x03);
  assert(request->manufacturerId == theCurrentFileSpec.manufacturerCode);
  assert(request->imageType == theCurrentFileSpec.type);
  assert(request->newFileVersion == theCurrentFileSpec.version);

  imageNotifyResponseHandler = handler;

  return EMBER_SUCCESS;
}
#define addImageNotifyRequestCheck() addSimpleCheck("ImageNotifyRequest", "")

EmberZclStatus_t emberZclOtaBootloadServerGetNextImageCallback(const EmberIpv6Address *source,
                                                               const EmberZclOtaBootloadFileSpec_t *deviceFileSpec,
                                                               EmberZclOtaBootloadFileSpec_t *nextFileSpec)
{
  long *contents
    = functionCallCheck("GetNextImageCallback",
                        "iii!",
                        deviceFileSpec->manufacturerCode,
                        deviceFileSpec->type,
                        deviceFileSpec->version);
  assert(memcmp(source->bytes,
                theRemoteAddress.bytes,
                sizeof(theRemoteAddress.bytes))
         == 0);

  EmberZclStatus_t status = (EmberZclStatus_t)contents[3];
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    *nextFileSpec = theNextFileSpec;
  } else {
    *nextFileSpec = emberZclOtaBootloadFileSpecNull;
  }
  return status;
}

#define addGetNextImageCallbackCheck(manufacturerCode, type, version, status) \
  addSimpleCheck("GetNextImageCallback",                                      \
                 "iii!",                                                      \
                 (manufacturerCode),                                          \
                 (type),                                                      \
                 (version),                                                   \
                 (status))

static void callQueryNextRequest(EmberZclManufacturerCode_t manufacturerCode,
                                 EmberZclOtaBootloadFileType_t type,
                                 EmberZclOtaBootloadFileVersion_t version,
                                 EmberZclOtaBootloadHardwareVersion_t hardwareVersion)
{
  EmberZclCommandContext_t context = {
    .remoteAddress = theRemoteAddress,
  };
  EmberZclClusterOtaBootloadServerCommandQueryNextImageRequest_t request = {
    .fieldControl = ((hardwareVersion
                      == EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL)
                     ? 0
                     : BIT(0)),
    .manufacturerId = manufacturerCode,
    .imageType = type,
    .currentFileVersion = version,
    .hardwareVersion = hardwareVersion,
  };
  emberZclClusterOtaBootloadServerCommandQueryNextImageRequestHandler(&context,
                                                                      &request);
}

#define addQueryNextRequestAction(manufacturerCode, \
                                  type,             \
                                  version,          \
                                  hardwareVersion)  \
  addSimpleAction("QueryNextRequest %x %x %x %x",   \
                  callQueryNextRequest,             \
                  4,                                \
                  (manufacturerCode),               \
                  (type),                           \
                  (version),                        \
                  (hardwareVersion))

EmberStatus emberZclSendClusterOtaBootloadServerCommandQueryNextImageResponse(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandQueryNextImageResponse_t *response)
{
  functionCallCheck("QueryNextImageResponse",
                    "i",
                    response->status);
  assert(memcmp(context->remoteAddress.bytes,
                theRemoteAddress.bytes,
                sizeof(theRemoteAddress.bytes))
         == 0);
  if (response->status == EMBER_ZCL_STATUS_SUCCESS) {
    assert(response->fileVersion == theNextFileSpec.version);
    assert(response->imageSize == IMAGE_DATA_SIZE);
  }

  return EMBER_SUCCESS;
}

#define addQueryNextImageResponseCheck(status) \
  addSimpleCheck("QueryNextImageResponse", "i", (status))

static void callUpgradeEndRequest(EmberZclStatus_t status)
{
  EmberZclCommandContext_t context = {
    .remoteAddress = theRemoteAddress,
  };
  EmberZclClusterOtaBootloadServerCommandUpgradeEndRequest_t request = {
    .status = status,
    .manufacturerId = theNextFileSpec.manufacturerCode,
    .imageType = theNextFileSpec.type,
    .fileVersion = theNextFileSpec.version,
  };
  emberZclClusterOtaBootloadServerCommandUpgradeEndRequestHandler(&context,
                                                                  &request);
}

EmberStatus emberZclSendClusterOtaBootloadServerCommandQuerySpecificFileResponse(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandQuerySpecificFileResponse_t *response)
{
  return EMBER_SUCCESS;
}

#define addUpgradeEndRequestAction(status) \
  addSimpleAction("UpgradeEndRequest %x",  \
                  callUpgradeEndRequest,   \
                  1,                       \
                  (status))

uint32_t emberZclOtaBootloadServerUpgradeEndRequestCallback(const EmberIpv6Address *source,
                                                            const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                            EmberZclStatus_t status)
{
  long *contents = functionCallCheck("UpgradeEndRequestCallback",
                                     "i!",
                                     status);
  uint32_t upgradeTime = (uint32_t)contents[1];

  assert(memcmp(source->bytes,
                theRemoteAddress.bytes,
                sizeof(theRemoteAddress.bytes))
         == 0);
  assert(fileSpec->manufacturerCode == theNextFileSpec.manufacturerCode);
  assert(fileSpec->type == theNextFileSpec.type);
  assert(fileSpec->version == theNextFileSpec.version);

  return upgradeTime;
}

#define addUpgradeEndRequestCallbackCheck(status, upgradeTime) \
  addSimpleCheck("UpgradeEndRequestCallback", "i!", (status), (upgradeTime))

EmberStatus emberZclSendDefaultResponse(const EmberZclCommandContext_t *context,
                                        EmberZclStatus_t status)
{
  functionCallCheck("DefaultResponse", "");
  assert(memcmp(context->remoteAddress.bytes,
                theRemoteAddress.bytes,
                sizeof(theRemoteAddress.bytes))
         == 0);

  return EMBER_SUCCESS;
}

#define addDefaultResponseCheck() addSimpleCheck("DefaultResponse", "")

EmberStatus emberZclSendClusterOtaBootloadServerCommandUpgradeEndResponse(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandUpgradeEndResponse_t *response)
{
  functionCallCheck("UpgradeEndResponse",
                    "i",
                    response->upgradeTime);

  assert(memcmp(context->remoteAddress.bytes,
                theRemoteAddress.bytes,
                sizeof(theRemoteAddress.bytes))
         == 0);
  assert(response->manufacturerId == theNextFileSpec.manufacturerCode);
  assert(response->imageType == theNextFileSpec.type);
  assert(response->fileVersion == theNextFileSpec.version);
  assert(response->currentTime == 0); // use offset time

  return EMBER_SUCCESS;
}

#define addUpgradeEndResponseCheck(upgradeTime) \
  addSimpleCheck("UpgradeEndResponse", "i", (upgradeTime))

static void callServerDownloadHandler(uint16_t manufacturerCode,
                                      uint16_t type,
                                      uint32_t version,
                                      uint8_t logSize,
                                      uint8_t number)
{
  // See emberReadBlockOption stub above.
  uint8_t cheatToWin[] = { 0x00, logSize, number };
  EmberCoapReadOptions options;
  MEMMOVE(&options, cheatToWin, sizeof(cheatToWin));
  uint8_t uriPath[EM_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_LENGTH + 1];
  sprintf(uriPath, EM_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_FORMAT, EM_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI, manufacturerCode, type, version);
  emZclOtaBootloadServerDownloadHandler(EMBER_COAP_CODE_GET,
                                        uriPath,
                                        &options,
                                        NULL,  // payload
                                        0,     // payloadLength
                                        NULL); // info (whatever...)
}

#define addDownloadHandlerAction(fileSpec, logSize, number) \
  addSimpleAction("ServerDownloadHandler %d %d",            \
                  callServerDownloadHandler,                \
                  5,                                        \
                  (fileSpec).manufacturerCode,              \
                  (fileSpec).type,                          \
                  (fileSpec).version,                       \
                  (logSize),                                \
                  (number))

EmberStatus emberCoapRespondWithPath(const EmberCoapRequestInfo *requestInfo,
                                     EmberCoapCode code,
                                     const uint8_t *path,
                                     const EmberCoapOption *options,
                                     uint8_t numberOfOptions,
                                     const uint8_t *payload,
                                     uint16_t payloadLength)
{
  long *contents = functionCallCheck("CoapRespond", "i!!", code);

  assert(path == NULL);
  if (code == EMBER_COAP_CODE_205_CONTENT) {
    assert(numberOfOptions == 2);

    assert(options[0].type == EMBER_COAP_OPTION_CONTENT_FORMAT);
    assert(options[0].intValue == EMBER_COAP_CONTENT_FORMAT_OCTET_STREAM);

    EmberCoapBlockOption blockOption;
    emberParseBlockOptionValue(options[1].intValue, &blockOption);
    assert(blockOption.logSize == (uint8_t)contents[1]);
    assert(blockOption.number == (uint32_t)contents[2]);
    assert(emberVerifyBlockOption(&blockOption,
                                  payloadLength,
                                  (uint8_t)contents[1]));

    assert(memcmp(payload,
                  image + ((1 << blockOption.logSize) * blockOption.number),
                  payloadLength)
           == 0);
  } else {
    assert(numberOfOptions == 0);
  }

  return EMBER_SUCCESS;
}

#define addCoapRespondCheck(code, logSize, number) \
  addSimpleCheck("CoapRespond", "i!!", (code), (logSize), (number))

#define addCoapRespondSuccessCheck(logSize, number) \
  addCoapRespondCheck(EMBER_COAP_CODE_205_CONTENT, (logSize), (number))

#define addCoapRespondFailureCheck(code) \
  addCoapRespondCheck((code), 0, 0)

// -----------------------------------------------------------------------------
// Tests

static void imageNotifyTest(void)
{
  // When the network goes down, we don't send ImageNotify.
  addNetworkStatusCallbackAction(EMBER_NO_NETWORK);
  addRunAction(IMAGE_NOTIFY_PERIOD_MS);
  addRunAction(IMAGE_NOTIFY_PERIOD_MS);

  // When the network comes up, we start sending ImageNotify.
  addNetworkStatusCallbackAction(EMBER_JOINED_NETWORK_ATTACHED);
  addImageNotifyRequestCheck();
  addRunAction(10);

  // After another period, we send another one.
  addRunAction(IMAGE_NOTIFY_PERIOD_MS - 10 - 1); // TODO: why do we need -1 here?
  addImageNotifyRequestCheck();

  runScript();
}

static void queryNextImageTest(void)
{
  // We get a QueryNextImage command with totally bogus file specification data.
  // We send back a NO_IMAGE_AVAILABLE status.
  addQueryNextRequestAction(0xFFFF,
                            0xFFFF,
                            0xFFFFFFFF,
                            EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL);
  addGetNextImageCallbackCheck(0xFFFF,
                               0xFFFF,
                               0xFFFFFFFF,
                               EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE);
  addQueryNextImageResponseCheck(EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE);

  // We get a QueryNextImage from a weird node that isn't authorized, so we tell
  // them no as well.
  addQueryNextRequestAction(theCurrentFileSpec.manufacturerCode,
                            theCurrentFileSpec.type,
                            theCurrentFileSpec.version,
                            EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL);
  addGetNextImageCallbackCheck(theCurrentFileSpec.manufacturerCode,
                               theCurrentFileSpec.type,
                               theCurrentFileSpec.version,
                               EMBER_ZCL_STATUS_NOT_AUTHORIZED);
  addQueryNextImageResponseCheck(EMBER_ZCL_STATUS_NOT_AUTHORIZED);

  // Finally, we get a QueryNextImage from a friendly node that has all the
  // right file specifications. We give them an updated file spec.
  addQueryNextRequestAction(theCurrentFileSpec.manufacturerCode,
                            theCurrentFileSpec.type,
                            theCurrentFileSpec.version,
                            EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL);
  addGetNextImageCallbackCheck(theCurrentFileSpec.manufacturerCode,
                               theCurrentFileSpec.type,
                               theCurrentFileSpec.version,
                               EMBER_ZCL_STATUS_SUCCESS);
  addQueryNextImageResponseCheck(EMBER_ZCL_STATUS_SUCCESS);

  // What if we get a node that wants an image that works with their hardware
  // version? We should still process the command normally if the application
  // says to do so.
  addQueryNextRequestAction(theCurrentFileSpec.manufacturerCode,
                            theCurrentFileSpec.type,
                            theCurrentFileSpec.version,
                            3); // hardwareVersion
  addGetNextImageCallbackCheck(theCurrentFileSpec.manufacturerCode,
                               theCurrentFileSpec.type,
                               theCurrentFileSpec.version,
                               EMBER_ZCL_STATUS_SUCCESS);
  addQueryNextImageResponseCheck(EMBER_ZCL_STATUS_SUCCESS);

  // If we get a hardware version that the application doesn't like, they should
  // be able to say no.
  addQueryNextRequestAction(theCurrentFileSpec.manufacturerCode,
                            theCurrentFileSpec.type,
                            theCurrentFileSpec.version,
                            3); // hardwareVersion
  addGetNextImageCallbackCheck(theCurrentFileSpec.manufacturerCode,
                               theCurrentFileSpec.type,
                               theCurrentFileSpec.version,
                               EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE);
  addQueryNextImageResponseCheck(EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE);

  runScript();
}

static void downloadTest(void)
{
  // Try to download a random non-existent image
  addDownloadHandlerAction(emptyFileSpec, BLOCK_SIZE_LOG, 0);
  addCoapRespondFailureCheck(EMBER_COAP_CODE_404_NOT_FOUND);

  // If a client sends a QueryNextImage, but the application says that it
  // doesn't want the client to download anything, then we once again throw up
  // our hands.
  addQueryNextRequestAction(emptyFileSpec.manufacturerCode,
                            emptyFileSpec.type,
                            emptyFileSpec.version,
                            EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL);
  addGetNextImageCallbackCheck(emptyFileSpec.manufacturerCode,
                               emptyFileSpec.type,
                               emptyFileSpec.version,
                               EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE);
  addQueryNextImageResponseCheck(EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE);

  addDownloadHandlerAction(emptyFileSpec, BLOCK_SIZE_LOG, 0);
  addCoapRespondFailureCheck(EMBER_COAP_CODE_404_NOT_FOUND);

  // After a client sends a query next image, it should be able to request
  // whatever block it wants.
  addQueryNextRequestAction(theCurrentFileSpec.manufacturerCode,
                            theCurrentFileSpec.type,
                            theCurrentFileSpec.version,
                            EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL);
  addGetNextImageCallbackCheck(theCurrentFileSpec.manufacturerCode,
                               theCurrentFileSpec.type,
                               theCurrentFileSpec.version,
                               EMBER_ZCL_STATUS_SUCCESS);
  addQueryNextImageResponseCheck(EMBER_ZCL_STATUS_SUCCESS);

  addDownloadHandlerAction(theNextFileSpec, BLOCK_SIZE_LOG, 2);
  addCoapRespondSuccessCheck(BLOCK_SIZE_LOG, 2);
  addDownloadHandlerAction(theNextFileSpec, BLOCK_SIZE_LOG, 1);
  addCoapRespondSuccessCheck(BLOCK_SIZE_LOG, 1);
  addDownloadHandlerAction(theNextFileSpec, BLOCK_SIZE_LOG, 3);
  addCoapRespondSuccessCheck(BLOCK_SIZE_LOG, 3);
  addDownloadHandlerAction(theNextFileSpec, BLOCK_SIZE_LOG, 0);
  addCoapRespondSuccessCheck(BLOCK_SIZE_LOG, 0);

  // If we request a block that extends past the end of the OTA file, we should
  // send back a payload that only contains the number of bytes left in the OTA
  // file.
  assert((IMAGE_DATA_SIZE / (1 << BLOCK_SIZE_LOG)) != 0);
  size_t lastBlockNumber = IMAGE_DATA_SIZE >> BLOCK_SIZE_LOG;
  addDownloadHandlerAction(theNextFileSpec, BLOCK_SIZE_LOG, lastBlockNumber);
  addCoapRespondSuccessCheck(BLOCK_SIZE_LOG, lastBlockNumber);

  // If we receive a request for a block that starts off the end of the file, we
  // blame it on the client.
  addDownloadHandlerAction(theNextFileSpec, BLOCK_SIZE_LOG, lastBlockNumber + 1);
  addCoapRespondFailureCheck(EMBER_COAP_CODE_402_BAD_OPTION);

  // If we receive a request for a block that doesn't fit in our
  // MAX_RESPONSE_BLOCK_SIZE constant, we return 5.00.
  addDownloadHandlerAction(theNextFileSpec, 10 /* 1024 */, 0);
  addCoapRespondFailureCheck(EMBER_COAP_CODE_500_INTERNAL_SERVER_ERROR);

  // Other block sizes should work just as well.
  addDownloadHandlerAction(theNextFileSpec, BLOCK_SIZE_LOG - 1, 2);
  addCoapRespondSuccessCheck(BLOCK_SIZE_LOG - 1, 2);
  addDownloadHandlerAction(theNextFileSpec, BLOCK_SIZE_LOG - 1, 1);
  addCoapRespondSuccessCheck(BLOCK_SIZE_LOG - 1, 1);
  addDownloadHandlerAction(theNextFileSpec, BLOCK_SIZE_LOG - 1, 3);
  addCoapRespondSuccessCheck(BLOCK_SIZE_LOG - 1, 3);
  addDownloadHandlerAction(theNextFileSpec, BLOCK_SIZE_LOG - 1, 0);
  addCoapRespondSuccessCheck(BLOCK_SIZE_LOG - 1, 0);

  addDownloadHandlerAction(theNextFileSpec, BLOCK_SIZE_LOG + 1, 0);
  addCoapRespondSuccessCheck(BLOCK_SIZE_LOG + 1, 0);
  addDownloadHandlerAction(theNextFileSpec, BLOCK_SIZE_LOG + 1, 1);
  addCoapRespondSuccessCheck(BLOCK_SIZE_LOG + 1, 1);

  runScript();
}

static void upgradeEndTest(void)
{
  // We should report whatever status we get to the application.
  // If it is a failure status, then we should be sending back a default
  // response...which seems weird, but that is what 11.13.6.4 says.
  addUpgradeEndRequestAction(EMBER_ZCL_STATUS_REQUIRE_MORE_IMAGE);
  addUpgradeEndRequestCallbackCheck(EMBER_ZCL_STATUS_REQUIRE_MORE_IMAGE,
                                    0); // upgradeTime
  addDefaultResponseCheck();

  addUpgradeEndRequestAction(EMBER_ZCL_STATUS_INVALID_IMAGE);
  addUpgradeEndRequestCallbackCheck(EMBER_ZCL_STATUS_INVALID_IMAGE,
                                    0); // upgradeTime
  addDefaultResponseCheck();

  addUpgradeEndRequestAction(EMBER_ZCL_STATUS_ABORT);
  addUpgradeEndRequestCallbackCheck(EMBER_ZCL_STATUS_ABORT,
                                    0); // upgradeTime
  addDefaultResponseCheck();

  // When we get a success status, the application tells us when the client
  // should perform the upgrade.
  addUpgradeEndRequestAction(EMBER_ZCL_STATUS_SUCCESS);
  addUpgradeEndRequestCallbackCheck(EMBER_ZCL_STATUS_SUCCESS,
                                    0); // upgradeTime
  addUpgradeEndResponseCheck(0); // upgradeTime

  addUpgradeEndRequestAction(EMBER_ZCL_STATUS_SUCCESS);
  addUpgradeEndRequestCallbackCheck(EMBER_ZCL_STATUS_SUCCESS,
                                    0xABCD); // upgradeTime
  addUpgradeEndResponseCheck(0xABCD); // upgradeTime

  runScript();
}

// -------------------------------------
// Main

int main(int argc, char *argv[])
{
  emInitializeBuffers();
  // From ota-bootload-storage-ram.c.
  extern void emZclOtaBootloadStorageInitCallback(void);
  emZclOtaBootloadStorageInitCallback();

  // Populate the OTA storage with a fake image for testing.
  assert(emberZclOtaBootloadStorageCreate(&theNextFileSpec)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);
  assert(emberZclOtaBootloadStorageWrite(&theNextFileSpec,
                                         0,
                                         image,
                                         IMAGE_DATA_SIZE)
         == EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS);

  Test tests[] = {
    { "image-notify", imageNotifyTest, },
    { "query-next-image", queryNextImageTest, },
    { "download", downloadTest, },
    { "upgrade-end", upgradeEndTest, },
    { NULL, NULL, },
  };
  (*parseTestArgument(argc, argv, tests))();

  fprintf(stderr, " done ]\n");

  return 0;
}
