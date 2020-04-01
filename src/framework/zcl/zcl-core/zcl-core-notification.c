/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include "thread-bookkeeping.h"
#include CHIP_AF_API_ZCL_CORE

typedef struct {
  ChipZclReportingConfigurationId_t reportingConfigurationId;
  uint8_t uri[CHIP_ZCL_URI_MAX_LENGTH];
  uint32_t timestamp;
  uint8_t bindingId;
} Notification_t;
#define CHIP_ZCLIP_STRUCT Notification_t
static const ZclipStructSpec notificationSpec[] = {
  CHIP_ZCLIP_OBJECT(sizeof(CHIP_ZCLIP_STRUCT),
                     4,     // fieldCount
                     NULL), // names
  CHIP_ZCLIP_FIELD_NAMED_MANDATORY(CHIP_ZCLIP_TYPE_MAX_LENGTH_STRING, uri, "u"),
  CHIP_ZCLIP_FIELD_NAMED_MANDATORY(CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER, reportingConfigurationId, "r"),
  CHIP_ZCLIP_FIELD_NAMED_MANDATORY(CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER, bindingId, "b"),
  CHIP_ZCLIP_FIELD_NAMED(CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER, timestamp, "t"),
};
#undef CHIP_ZCLIP_STRUCT

static bool findAttributeMap(CborState *state);
static bool getNextAttributeKeyValue(CborState *state,
                                     const ChipZclClusterSpec_t *clusterSpec,
                                     const ChZclAttributeEntry_t **attribute,
                                     uint8_t *buffer,
                                     const ChipCoapRequestInfo *info);
static ChipStatus notify(const ChZclContext_t *context, CborState *state, void *data);

// zcl/e/XX/<cluster>/n:
// zcl/g/XXXX/<cluster>/n:
//   POST: report notification.
//   OTHER: not allowed.
void chZclUriClusterNotificationHandler(ChZclContext_t *context)
{
  Notification_t notification = {
    .reportingConfigurationId = CHIP_ZCL_REPORTING_CONFIGURATION_NULL,
    .uri = { 0 },
    .timestamp = 0,
    .bindingId = 0,
  };
  if (!emCborDecodeOneStruct(context->payload,
                             context->payloadLength,
                             notificationSpec,
                             &notification)) {
    chZclRespond400BadRequest(context->info);
    return;
  }

  ChipZclClusterSpec_t clusterSpec;
  chipZclReverseClusterSpec(&context->clusterSpec, &clusterSpec);
  uint8_t buffer[CHIP_ZCL_ATTRIBUTE_MAX_SIZE];
  ChipZclNotificationContext_t notificationContext = {
    .remoteAddress = context->info->remoteAddress,
    .sourceEndpointId = CHIP_ZCL_ENDPOINT_NULL, // filled in later
    .sourceReportingConfigurationId = notification.reportingConfigurationId,
    .sourceTimestamp = notification.timestamp,
    .groupId = context->groupId,
    .endpointId = context->endpoint->endpointId,
    .clusterSpec = &clusterSpec,
    .attributeId = CHIP_ZCL_ATTRIBUTE_NULL, // filled in later
    .buffer = buffer,
    .bufferLength = 0, // filled in later
  };

  // TODO: This verifies the URI up to the cluster.  It does not verify that
  // the URI path ends in /a.
  ChipZclBindingEntry_t source;
  if (chZclUriToBindingEntry(notification.uri, &source, true)
      && (source.destination.application.type
          == CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT)
      && chipZclAreClusterSpecsEqual(&clusterSpec, &source.clusterSpec)) {
    notificationContext.sourceEndpointId
      = source.destination.application.data.endpointId;
  } else {
    chZclRespond400BadRequest(context->info);
    return;
  }

  CborState state;
  emCborDecodeStart(&state, context->payload, context->payloadLength);
  if (findAttributeMap(&state)) {
    const ChZclAttributeEntry_t *attribute;
    notificationContext.buffer = buffer;
    while (getNextAttributeKeyValue(&state,
                                    notificationContext.clusterSpec,
                                    &attribute,
                                    buffer,
                                    context->info)) {
      notificationContext.attributeId = attribute->attributeId;
      notificationContext.bufferLength = attribute->size;
      chZclMultiEndpointDispatch(context,
                                 notify,
                                 &state,
                                 &notificationContext);
    }
  } else {
    chZclRespond400BadRequest(context->info);
  }
}

static bool findAttributeMap(CborState *state)
{
  if (emCborDecodeMap(state)) {
    while (true) {
      uint8_t type = emCborDecodePeek(state, NULL);
      if (type == CBOR_TEXT) {
        uint8_t key[2]; // "a" plus a NUL
        if (emCborDecodeValue(state,
                              CHIP_ZCLIP_TYPE_MAX_LENGTH_STRING,
                              sizeof(key),
                              key)
            && strcmp((const char *)key, "a") == 0) {
          if (emCborDecodeMap(state)) {
            return true;
          } else {
            break;
          }
        } else if (!emCborDecodeSkipValue(state)) {
          break;
        }
      } else if (type == CBOR_BREAK) {
        break;
      } else if (!emCborDecodeSkipValue(state)
                 || !emCborDecodeSkipValue(state)) {
        break;
      }
    }
  }
  return false;
}

// TODO: This could probably be used in the over-the-air write handler.
static bool getNextAttributeKeyValue(CborState *state,
                                     const ChipZclClusterSpec_t *clusterSpec,
                                     const ChZclAttributeEntry_t **attribute,
                                     uint8_t *buffer,
                                     const ChipCoapRequestInfo *info)
{
  while (true) {
    uint8_t type = emCborDecodePeek(state, NULL);
    if (type == CBOR_UNSIGNED) {
      ChipZclAttributeId_t attributeId;
      if (!emCborDecodeValue(state,
                             CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER,
                             sizeof(attributeId),
                             (uint8_t *)&attributeId)) {
        break;
      }

      *attribute = chZclFindAttribute(clusterSpec,
                                      attributeId,
                                      true); // include remote
      if (*attribute != NULL
          && emCborDecodeValue(state,
                               (*attribute)->type,
                               (*attribute)->size,
                               buffer)) {
        return true;
      } else if (!emCborDecodeSkipValue(state)) {
        break;
      }
    } else if (type == CBOR_BREAK) {
      chZclRespond204Changed(info);
      return false;
    } else if (!emCborDecodeSkipValue(state)
               || !emCborDecodeSkipValue(state)) {
      break;
    }
  }

  chZclRespond400BadRequest(info);
  return false;
}

static ChipStatus notify(const ChZclContext_t *context, CborState *state, void *data)
{
  ChipZclNotificationContext_t *notificationContext = data;
  notificationContext->endpointId = context->endpoint->endpointId;
  chZclNotification(notificationContext,
                    notificationContext->clusterSpec,
                    notificationContext->attributeId,
                    notificationContext->buffer,
                    notificationContext->bufferLength);
  return CHIP_ZCL_STATUS_SUCCESS;
}
