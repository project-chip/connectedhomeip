/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef OTA_BOOTLOAD_CORE_TEST_H
#define OTA_BOOTLOAD_CORE_TEST_H

extern const EmberEventData emAppEvents[];

extern const EmberZclClusterSpec_t emberZclClusterOtaBootloadServerSpec;

typedef struct {
  int8u payloadType;
  int8u queryJitter;
  int16u manufacturerId;
  int16u imageType;
  int32u newFileVersion;
} EmberZclClusterOtaBootloadClientCommandImageNotifyRequest_t;
typedef struct {
  enum8_t status;
} EmberZclClusterOtaBootloadClientCommandImageNotifyResponse_t;
typedef void (*EmberZclClusterOtaBootloadClientCommandImageNotifyResponseHandler)(
  EmberZclMessageStatus_t status,
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadClientCommandImageNotifyResponse_t *response);
EmberStatus emberZclSendClusterOtaBootloadClientCommandImageNotifyRequest(
  const EmberZclDestination_t *destination,
  const EmberZclClusterOtaBootloadClientCommandImageNotifyRequest_t *request,
  const EmberZclClusterOtaBootloadClientCommandImageNotifyResponseHandler handler);
EmberStatus emberZclSendClusterOtaBootloadClientCommandImageNotifyResponse(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadClientCommandImageNotifyResponse_t *response);

typedef struct {
  int8u fieldControl;
  int16u manufacturerId;
  int16u imageType;
  int32u currentFileVersion;
  int16u hardwareVersion;
} EmberZclClusterOtaBootloadServerCommandQueryNextImageRequest_t;
typedef struct {
  int8u status;
  int32u fileVersion;
  int32u imageSize;
  EmberZclStringType_t fileUri;
} EmberZclClusterOtaBootloadServerCommandQueryNextImageResponse_t;
typedef void (*EmberZclClusterOtaBootloadServerCommandQueryNextImageResponseHandler)(
  EmberZclMessageStatus_t status,
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandQueryNextImageResponse_t *response);
EmberStatus emberZclSendClusterOtaBootloadServerCommandQueryNextImageRequest(
  const EmberZclDestination_t *destination,
  const EmberZclClusterOtaBootloadServerCommandQueryNextImageRequest_t *request,
  const EmberZclClusterOtaBootloadServerCommandQueryNextImageResponseHandler handler);
EmberStatus emberZclSendClusterOtaBootloadServerCommandQueryNextImageResponse(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandQueryNextImageResponse_t *response);
void emberZclClusterOtaBootloadServerCommandQueryNextImageRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandQueryNextImageRequest_t *request);

typedef struct {
  int8u status;
  int16u manufacturerId;
  int16u imageType;
  int32u fileVersion;
} EmberZclClusterOtaBootloadServerCommandUpgradeEndRequest_t;
typedef struct {
  int16u manufacturerId;
  int16u imageType;
  int32u fileVersion;
  int32u currentTime;
  int32u upgradeTime;
} EmberZclClusterOtaBootloadServerCommandUpgradeEndResponse_t;
typedef void (*EmberZclClusterOtaBootloadServerCommandUpgradeEndResponseHandler)(
  EmberZclMessageStatus_t status,
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandUpgradeEndResponse_t *response);
EmberStatus emberZclSendClusterOtaBootloadServerCommandUpgradeEndRequest(
  const EmberZclDestination_t *destination,
  const EmberZclClusterOtaBootloadServerCommandUpgradeEndRequest_t *request,
  const EmberZclClusterOtaBootloadServerCommandUpgradeEndResponseHandler handler);
EmberStatus emberZclSendClusterOtaBootloadServerCommandUpgradeEndResponse(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandUpgradeEndResponse_t *response);
void emberZclClusterOtaBootloadServerCommandUpgradeEndRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandUpgradeEndRequest_t *request);

typedef struct {
  int8u* requestNodeAddress;
  int16u manufacturerId;
  int16u imageType;
  int32u fileVersion;
  int16u currentZigbeeStackVersion;
} EmberZclClusterOtaBootloadServerCommandQuerySpecificFileRequest_t;
typedef struct {
  int8u status;
  int32u fileVersion;
  int32u imageSize;
  EmberZclStringType_t fileUri;
} EmberZclClusterOtaBootloadServerCommandQuerySpecificFileResponse_t;
typedef void (*EmberZclClusterOtaBootloadServerCommandQuerySpecificFileResponseHandler)(
  EmberZclMessageStatus_t status,
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandQuerySpecificFileResponse_t *response);
EmberStatus emberZclSendClusterOtaBootloadServerCommandQuerySpecificFileRequest(
  const EmberZclDestination_t *destination,
  const EmberZclClusterOtaBootloadServerCommandQuerySpecificFileRequest_t *request,
  const EmberZclClusterOtaBootloadServerCommandQuerySpecificFileResponseHandler handler);
EmberStatus emberZclSendClusterOtaBootloadServerCommandQuerySpecificFileResponse(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandQuerySpecificFileResponse_t *response);
void emberZclClusterOtaBootloadServerCommandQuerySpecificFileRequestHandler(
  const EmberZclCommandContext_t *context,
  const EmberZclClusterOtaBootloadServerCommandQuerySpecificFileRequest_t *request);

#endif // OTA_BOOTLOAD_CORE_TEST_H
