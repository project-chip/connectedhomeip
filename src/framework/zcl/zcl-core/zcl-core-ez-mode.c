/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#include CHIP_AF_API_HAL
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_ZCL_CORE_WELL_KNOWN
//#include "stack/core/log.h"

// TODO: Use an appropriate timeout.
#define EZ_MODE_TIMEOUT_MS (60 * MILLISECOND_TICKS_PER_SECOND)

ChipEventControl chZclEzModeEventControl;

static void ezMode1RequestHandler(const ChipZclCommandContext_t *context,
                                  const void *request);
static void ezMode0RequestHandler(const ChipZclCommandContext_t *context,
                                  const void *request);
static void ezMode1ResponseHandler(ChipCoapStatus status,
                                   ChipCoapCode code,
                                   ChipCoapReadOptions *options,
                                   uint8_t *payload,
                                   uint16_t payloadLength,
                                   ChipCoapResponseInfo *info);
static size_t append(ChipZclEndpointId_t endpointId,
                     const ChipZclClusterSpec_t *clusterSpec,
                     bool first,
                     uint8_t *result);
static size_t parse(const ChipIpv6Address *remoteAddress,
                    const uint8_t *payload,
                    uint16_t payloadLength,
                    ChipCoapReadOptions *options,
                    bool bind,
                    uint8_t *response);

const uint8_t managementPayload[] = { 0x81, 0x61, 0x63 }; // ["c"]
void chZclManagementHandler(ChZclContext_t *context)
{
  chZclRespond205ContentCbor(context->info, managementPayload, sizeof(managementPayload));
}

const uint8_t managementCommandPayload[] = { 0x82, 0x00, 0x01 }; // [0, 1]
void chZclManagementCommandHandler(ChZclContext_t *context)
{
  chZclRespond205ContentCbor(context->info, managementCommandPayload,
                             sizeof(managementCommandPayload));
}

void chZclManagementCommandIdHandler(ChZclContext_t *context)
{
  ChipZclCommandContext_t commandContext = {
    .remoteAddress = context->info->remoteAddress,
    .code = context->code,
    .info = context->info,
    .payload = context->payload,
    .payloadLength = context->payloadLength,
    .groupId = CHIP_ZCL_GROUP_NULL,
    .endpointId = CHIP_ZCL_ENDPOINT_NULL,
    .clusterSpec = NULL, // unused
    .commandId = context->command->commandId,
    .state = NULL,    // unused
    .buffer = NULL,   // unused
    .result = CHIP_ERR_FATAL, // unused
  };
  (*context->command->handler)(&commandContext, NULL);
}

// Command entries.
static const ChZclCommandEntry_t managementCommandTable[] = {
  { NULL, 0, NULL, (ChZclRequestHandler)ezMode0RequestHandler },
  { NULL, 1, NULL, (ChZclRequestHandler)ezMode1RequestHandler },
};
static const size_t managementCommandCount = COUNTOF(managementCommandTable);

const ChZclCommandEntry_t *chZclManagementFindCommand(ChipZclCommandId_t commandId)
{
  for (size_t i = 0; i < managementCommandCount; i++) {
    if (managementCommandTable[i].commandId == commandId) {
      return &managementCommandTable[i];
    }
  }
  return NULL;
}

ChipStatus chipZclStartEzMode(void)
{
  // TODO: Use an appropriate multicast address.
  ChipIpv6Address multicastAddr = { { 0 } };
  if (!chZclGetMulticastAddress(&multicastAddr)) {
    return CHIP_ERR_FATAL;
  }

//  emLogLine(TEMP, "start EZ");

  uint8_t payload[1024] = { 0 };
  uint8_t *outgoing = payload;
  for (size_t i = 0; i < chZclEndpointCount; i++) {
    const ChipZclClusterSpec_t **clusterSpecs
      = chZclEndpointTable[i].clusterSpecs;
    while (*clusterSpecs != NULL) {
      outgoing += append(chZclEndpointTable[i].endpointId,
                         *clusterSpecs,
                         (outgoing == payload),
                         outgoing);
      clusterSpecs++;
    }
  }

  ChipCoapSendInfo info = { 0 }; // use defaults
  ChipStatus status = chipCoapPost(&multicastAddr,
                                     (const uint8_t *)"zcl/m/c/0",
                                     payload,
                                     (outgoing - payload),
                                     NULL, // handler
                                     &info);
  if (status == CHIP_SUCCESS) {
    chipEventControlSetDelayMS(chZclEzModeEventControl, EZ_MODE_TIMEOUT_MS);
  }
  return status;
}

void chipZclStopEzMode(void)
{
  chipEventControlSetInactive(chZclEzModeEventControl);
}

bool chipZclEzModeIsActive(void)
{
  return chipEventControlGetActive(chZclEzModeEventControl);
}

void chZclEzModeEventHandler(void)
{
  chipZclStopEzMode();
}

// zcl/m/c/0 - multicast advertisement request
static void ezMode0RequestHandler(const ChipZclCommandContext_t *context,
                                  const void *request)
{
  if (chipEventControlGetActive(chZclEzModeEventControl)) {
//    emLogBytesLine(TEMP, " EZ response 0", (uint8_t *) &context->remoteAddress, 16);
    uint8_t payload[1024] = { 0 };
    size_t payloadLength = parse(&context->remoteAddress,
                                 context->payload,
                                 context->payloadLength,
                                 NULL,  // coap read options.
                                 false, // no bindings
                                 payload);

    // We expect this advertisement to be multicast, so we only respond if we
    // have any corresponding clusters, to cut down on unnecessary traffic.
    if (payloadLength != 0) {
      ChipCoapSendInfo info = { 0 }; // use defaults
      chipCoapPost(&context->remoteAddress,
                    (const uint8_t *)"zcl/m/c/1",
                    payload,
                    payloadLength,
                    ezMode1ResponseHandler,
                    &info);
    }
  }
}

// zcl/m/c/1 - unicast advertisement request
static void ezMode1RequestHandler(const ChipZclCommandContext_t *context,
                                  const void *request)
{
  if (chipEventControlGetActive(chZclEzModeEventControl)) {
    uint8_t payload[1024] = { 0 };
    size_t payloadLength = parse(&context->remoteAddress,
                                 context->payload,
                                 context->payloadLength,
                                 NULL, // coap read options.
                                 true, // add bindings
                                 payload);

    // We expect this request to be unicast, so we always respond, even if we
    // have nothing to say.
    // TODO: 16-07010-000 say to send a 2.04, but that doesn't make sense.
    chZclRespond205ContentLinkFormat(context->info, payload, payloadLength, CHIP_COAP_CONTENT_FORMAT_LINK_FORMAT);
  } else {
    chZclRespond404NotFound(context->info);
  }
}

// zcl/m/c/1 - unicast advertisement response
static void ezMode1ResponseHandler(ChipCoapStatus status,
                                   ChipCoapCode code,
                                   ChipCoapReadOptions *options,
                                   uint8_t *payload,
                                   uint16_t payloadLength,
                                   ChipCoapResponseInfo *info)
{
  if (status == CHIP_COAP_MESSAGE_RESPONSE) {
//    emLogBytesLine(TEMP, " EZ response 1", (uint8_t *) &info->remoteAddress, 16);
    parse(&info->remoteAddress,
          payload,
          payloadLength,
          options,
          true,  // add bindings
          NULL); // no response
  }
}

static size_t append(ChipZclEndpointId_t endpointId,
                     const ChipZclClusterSpec_t *clusterSpec,
                     bool first,
                     uint8_t *result)
{
  char *finger = (char *)result;
  finger += chZclUriAppendUriPath(finger, NULL, endpointId, clusterSpec);
  chZclUriBreak(finger);
  return (uint8_t *)finger - result;
}

static size_t parse(const ChipIpv6Address *remoteAddress,
                    const uint8_t *payload,
                    uint16_t payloadLength,
                    ChipCoapReadOptions *options,
                    bool bind,
                    uint8_t *response)
{
  ChipZclBindingEntry_t entry = {
    .destination.network.scheme = CHIP_ZCL_SCHEME_COAP,
    .destination.network.type = CHIP_ZCL_NETWORK_DESTINATION_TYPE_ADDRESS,
    .destination.network.data.address = *remoteAddress,
    .destination.network.port = CHIP_COAP_PORT,
    .destination.application.data.endpointId = CHIP_ZCL_ENDPOINT_NULL,
    .destination.application.type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT,
    .reportingConfigurationId = CHIP_ZCL_REPORTING_CONFIGURATION_DEFAULT,
  };

  // Get content format from response coap options.
  ChipCoapContentFormatType cf = CHIP_COAP_CONTENT_FORMAT_NONE;
  if (options != NULL) {
    uint32_t valueLoc;
    cf = (chipReadIntegerOption(options,
                                 CHIP_COAP_OPTION_CONTENT_FORMAT,
                                 &valueLoc))
         ? (ChipCoapContentFormatType)valueLoc
         : CHIP_COAP_CONTENT_FORMAT_NONE;
  }

  uint8_t *incoming = (uint8_t *)payload;
  uint8_t *outgoing = response;
  ChipZclClusterSpec_t spec;
  ChZclUriContext_t uri;
  uri.clusterSpec = &spec;

  // Decode all "/zcl/e/" tags in the response.
  while (chZclParseUri(payload,
                       payloadLength,
                       &incoming,   //ptr gets modified by call.
                       cf,
                       &uri)) {
    entry.destination.application.data.endpointId = uri.endpointId;

    // match the opposite role.
    uri.clusterSpec->role = (uri.clusterSpec->role == CHIP_ZCL_ROLE_CLIENT
                             ? CHIP_ZCL_ROLE_SERVER
                             : CHIP_ZCL_ROLE_CLIENT);
    MEMCOPY(&entry.clusterSpec, uri.clusterSpec, sizeof(ChipZclClusterSpec_t));

    for (size_t i = 0; i < chZclEndpointCount; i++) {
      if (chZclEndpointHasCluster(chZclEndpointTable[i].endpointId,
                                  &entry.clusterSpec)) {
        if (bind) {
          entry.endpointId = chZclEndpointTable[i].endpointId;
          chipZclAddBinding(&entry);
        }

        if (outgoing != NULL) {
          outgoing += append(chZclEndpointTable[i].endpointId,
                             uri.clusterSpec,
                             (outgoing == response),
                             outgoing);
        }
      }
    } // for
  } // while

  return outgoing - response;
}
