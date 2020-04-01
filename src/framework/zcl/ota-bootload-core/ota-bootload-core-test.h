/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef OTA_BOOTLOAD_CORE_TEST_H
#define OTA_BOOTLOAD_CORE_TEST_H

extern const ChipEventData emAppEvents[];

extern const ChipZclClusterSpec_t chipZclClusterOtaBootloadServerSpec;

typedef struct {
  int8u payloadType;
  int8u queryJitter;
  int16u manufacturerId;
  int16u imageType;
  int32u newFileVersion;
} ChipZclClusterOtaBootloadClientCommandImageNotifyRequest_t;
typedef struct {
  enum8_t status;
} ChipZclClusterOtaBootloadClientCommandImageNotifyResponse_t;
typedef void (*ChipZclClusterOtaBootloadClientCommandImageNotifyResponseHandler)(
  ChipZclMessageStatus_t status,
  const ChipZclCommandContext_t *context,
  const ChipZclClusterOtaBootloadClientCommandImageNotifyResponse_t *response);
ChipStatus chipZclSendClusterOtaBootloadClientCommandImageNotifyRequest(
  const ChipZclDestination_t *destination,
  const ChipZclClusterOtaBootloadClientCommandImageNotifyRequest_t *request,
  const ChipZclClusterOtaBootloadClientCommandImageNotifyResponseHandler handler);
ChipStatus chipZclSendClusterOtaBootloadClientCommandImageNotifyResponse(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterOtaBootloadClientCommandImageNotifyResponse_t *response);

typedef struct {
  int8u fieldControl;
  int16u manufacturerId;
  int16u imageType;
  int32u currentFileVersion;
  int16u hardwareVersion;
} ChipZclClusterOtaBootloadServerCommandQueryNextImageRequest_t;
typedef struct {
  int8u status;
  int32u fileVersion;
  int32u imageSize;
  ChipZclStringType_t fileUri;
} ChipZclClusterOtaBootloadServerCommandQueryNextImageResponse_t;
typedef void (*ChipZclClusterOtaBootloadServerCommandQueryNextImageResponseHandler)(
  ChipZclMessageStatus_t status,
  const ChipZclCommandContext_t *context,
  const ChipZclClusterOtaBootloadServerCommandQueryNextImageResponse_t *response);
ChipStatus chipZclSendClusterOtaBootloadServerCommandQueryNextImageRequest(
  const ChipZclDestination_t *destination,
  const ChipZclClusterOtaBootloadServerCommandQueryNextImageRequest_t *request,
  const ChipZclClusterOtaBootloadServerCommandQueryNextImageResponseHandler handler);
ChipStatus chipZclSendClusterOtaBootloadServerCommandQueryNextImageResponse(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterOtaBootloadServerCommandQueryNextImageResponse_t *response);
void chipZclClusterOtaBootloadServerCommandQueryNextImageRequestHandler(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterOtaBootloadServerCommandQueryNextImageRequest_t *request);

typedef struct {
  int8u status;
  int16u manufacturerId;
  int16u imageType;
  int32u fileVersion;
} ChipZclClusterOtaBootloadServerCommandUpgradeEndRequest_t;
typedef struct {
  int16u manufacturerId;
  int16u imageType;
  int32u fileVersion;
  int32u currentTime;
  int32u upgradeTime;
} ChipZclClusterOtaBootloadServerCommandUpgradeEndResponse_t;
typedef void (*ChipZclClusterOtaBootloadServerCommandUpgradeEndResponseHandler)(
  ChipZclMessageStatus_t status,
  const ChipZclCommandContext_t *context,
  const ChipZclClusterOtaBootloadServerCommandUpgradeEndResponse_t *response);
ChipStatus chipZclSendClusterOtaBootloadServerCommandUpgradeEndRequest(
  const ChipZclDestination_t *destination,
  const ChipZclClusterOtaBootloadServerCommandUpgradeEndRequest_t *request,
  const ChipZclClusterOtaBootloadServerCommandUpgradeEndResponseHandler handler);
ChipStatus chipZclSendClusterOtaBootloadServerCommandUpgradeEndResponse(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterOtaBootloadServerCommandUpgradeEndResponse_t *response);
void chipZclClusterOtaBootloadServerCommandUpgradeEndRequestHandler(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterOtaBootloadServerCommandUpgradeEndRequest_t *request);

typedef struct {
  int8u* requestNodeAddress;
  int16u manufacturerId;
  int16u imageType;
  int32u fileVersion;
  int16u currentZigbeeStackVersion;
} ChipZclClusterOtaBootloadServerCommandQuerySpecificFileRequest_t;
typedef struct {
  int8u status;
  int32u fileVersion;
  int32u imageSize;
  ChipZclStringType_t fileUri;
} ChipZclClusterOtaBootloadServerCommandQuerySpecificFileResponse_t;
typedef void (*ChipZclClusterOtaBootloadServerCommandQuerySpecificFileResponseHandler)(
  ChipZclMessageStatus_t status,
  const ChipZclCommandContext_t *context,
  const ChipZclClusterOtaBootloadServerCommandQuerySpecificFileResponse_t *response);
ChipStatus chipZclSendClusterOtaBootloadServerCommandQuerySpecificFileRequest(
  const ChipZclDestination_t *destination,
  const ChipZclClusterOtaBootloadServerCommandQuerySpecificFileRequest_t *request,
  const ChipZclClusterOtaBootloadServerCommandQuerySpecificFileResponseHandler handler);
ChipStatus chipZclSendClusterOtaBootloadServerCommandQuerySpecificFileResponse(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterOtaBootloadServerCommandQuerySpecificFileResponse_t *response);
void chipZclClusterOtaBootloadServerCommandQuerySpecificFileRequestHandler(
  const ChipZclCommandContext_t *context,
  const ChipZclClusterOtaBootloadServerCommandQuerySpecificFileRequest_t *request);

#endif // OTA_BOOTLOAD_CORE_TEST_H
