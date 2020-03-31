/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_HAL
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_CORE_WELL_KNOWN
//#include "stack/core/log.h"

// TODO: Use an appropriate timeout.
#define EZ_MODE_TIMEOUT_MS (60 * MILLISECOND_TICKS_PER_SECOND)

EmberEventControl emZclEzModeEventControl;

static void ezMode1RequestHandler(const EmberZclCommandContext_t *context,
                                  const void *request);
static void ezMode0RequestHandler(const EmberZclCommandContext_t *context,
                                  const void *request);
static void ezMode1ResponseHandler(EmberCoapStatus status,
                                   EmberCoapCode code,
                                   EmberCoapReadOptions *options,
                                   uint8_t *payload,
                                   uint16_t payloadLength,
                                   EmberCoapResponseInfo *info);
static size_t append(EmberZclEndpointId_t endpointId,
                     const EmberZclClusterSpec_t *clusterSpec,
                     bool first,
                     uint8_t *result);
static size_t parse(const EmberIpv6Address *remoteAddress,
                    const uint8_t *payload,
                    uint16_t payloadLength,
                    EmberCoapReadOptions *options,
                    bool bind,
                    uint8_t *response);

const uint8_t managementPayload[] = { 0x81, 0x61, 0x63 }; // ["c"]
void emZclManagementHandler(EmZclContext_t *context)
{
  emZclRespond205ContentCbor(context->info, managementPayload, sizeof(managementPayload));
}

const uint8_t managementCommandPayload[] = { 0x82, 0x00, 0x01 }; // [0, 1]
void emZclManagementCommandHandler(EmZclContext_t *context)
{
  emZclRespond205ContentCbor(context->info, managementCommandPayload,
                             sizeof(managementCommandPayload));
}

void emZclManagementCommandIdHandler(EmZclContext_t *context)
{
  EmberZclCommandContext_t commandContext = {
    .remoteAddress = context->info->remoteAddress,
    .code = context->code,
    .info = context->info,
    .payload = context->payload,
    .payloadLength = context->payloadLength,
    .groupId = EMBER_ZCL_GROUP_NULL,
    .endpointId = EMBER_ZCL_ENDPOINT_NULL,
    .clusterSpec = NULL, // unused
    .commandId = context->command->commandId,
    .state = NULL,    // unused
    .buffer = NULL,   // unused
    .result = EMBER_ERR_FATAL, // unused
  };
  (*context->command->handler)(&commandContext, NULL);
}

// Command entries.
static const EmZclCommandEntry_t managementCommandTable[] = {
  { NULL, 0, NULL, (EmZclRequestHandler)ezMode0RequestHandler },
  { NULL, 1, NULL, (EmZclRequestHandler)ezMode1RequestHandler },
};
static const size_t managementCommandCount = COUNTOF(managementCommandTable);

const EmZclCommandEntry_t *emZclManagementFindCommand(EmberZclCommandId_t commandId)
{
  for (size_t i = 0; i < managementCommandCount; i++) {
    if (managementCommandTable[i].commandId == commandId) {
      return &managementCommandTable[i];
    }
  }
  return NULL;
}

EmberStatus emberZclStartEzMode(void)
{
  // TODO: Use an appropriate multicast address.
  EmberIpv6Address multicastAddr = { { 0 } };
  if (!emZclGetMulticastAddress(&multicastAddr)) {
    return EMBER_ERR_FATAL;
  }

//  emLogLine(TEMP, "start EZ");

  uint8_t payload[1024] = { 0 };
  uint8_t *outgoing = payload;
  for (size_t i = 0; i < emZclEndpointCount; i++) {
    const EmberZclClusterSpec_t **clusterSpecs
      = emZclEndpointTable[i].clusterSpecs;
    while (*clusterSpecs != NULL) {
      outgoing += append(emZclEndpointTable[i].endpointId,
                         *clusterSpecs,
                         (outgoing == payload),
                         outgoing);
      clusterSpecs++;
    }
  }

  EmberCoapSendInfo info = { 0 }; // use defaults
  EmberStatus status = emberCoapPost(&multicastAddr,
                                     (const uint8_t *)"zcl/m/c/0",
                                     payload,
                                     (outgoing - payload),
                                     NULL, // handler
                                     &info);
  if (status == EMBER_SUCCESS) {
    emberEventControlSetDelayMS(emZclEzModeEventControl, EZ_MODE_TIMEOUT_MS);
  }
  return status;
}

void emberZclStopEzMode(void)
{
  emberEventControlSetInactive(emZclEzModeEventControl);
}

bool emberZclEzModeIsActive(void)
{
  return emberEventControlGetActive(emZclEzModeEventControl);
}

void emZclEzModeEventHandler(void)
{
  emberZclStopEzMode();
}

// zcl/m/c/0 - multicast advertisement request
static void ezMode0RequestHandler(const EmberZclCommandContext_t *context,
                                  const void *request)
{
  if (emberEventControlGetActive(emZclEzModeEventControl)) {
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
      EmberCoapSendInfo info = { 0 }; // use defaults
      emberCoapPost(&context->remoteAddress,
                    (const uint8_t *)"zcl/m/c/1",
                    payload,
                    payloadLength,
                    ezMode1ResponseHandler,
                    &info);
    }
  }
}

// zcl/m/c/1 - unicast advertisement request
static void ezMode1RequestHandler(const EmberZclCommandContext_t *context,
                                  const void *request)
{
  if (emberEventControlGetActive(emZclEzModeEventControl)) {
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
    emZclRespond205ContentLinkFormat(context->info, payload, payloadLength, EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT);
  } else {
    emZclRespond404NotFound(context->info);
  }
}

// zcl/m/c/1 - unicast advertisement response
static void ezMode1ResponseHandler(EmberCoapStatus status,
                                   EmberCoapCode code,
                                   EmberCoapReadOptions *options,
                                   uint8_t *payload,
                                   uint16_t payloadLength,
                                   EmberCoapResponseInfo *info)
{
  if (status == EMBER_COAP_MESSAGE_RESPONSE) {
//    emLogBytesLine(TEMP, " EZ response 1", (uint8_t *) &info->remoteAddress, 16);
    parse(&info->remoteAddress,
          payload,
          payloadLength,
          options,
          true,  // add bindings
          NULL); // no response
  }
}

static size_t append(EmberZclEndpointId_t endpointId,
                     const EmberZclClusterSpec_t *clusterSpec,
                     bool first,
                     uint8_t *result)
{
  char *finger = (char *)result;
  finger += emZclUriAppendUriPath(finger, NULL, endpointId, clusterSpec);
  emZclUriBreak(finger);
  return (uint8_t *)finger - result;
}

static size_t parse(const EmberIpv6Address *remoteAddress,
                    const uint8_t *payload,
                    uint16_t payloadLength,
                    EmberCoapReadOptions *options,
                    bool bind,
                    uint8_t *response)
{
  EmberZclBindingEntry_t entry = {
    .destination.network.scheme = EMBER_ZCL_SCHEME_COAP,
    .destination.network.type = EMBER_ZCL_NETWORK_DESTINATION_TYPE_ADDRESS,
    .destination.network.data.address = *remoteAddress,
    .destination.network.port = EMBER_COAP_PORT,
    .destination.application.data.endpointId = EMBER_ZCL_ENDPOINT_NULL,
    .destination.application.type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT,
    .reportingConfigurationId = EMBER_ZCL_REPORTING_CONFIGURATION_DEFAULT,
  };

  // Get content format from response coap options.
  EmberCoapContentFormatType cf = EMBER_COAP_CONTENT_FORMAT_NONE;
  if (options != NULL) {
    uint32_t valueLoc;
    cf = (emberReadIntegerOption(options,
                                 EMBER_COAP_OPTION_CONTENT_FORMAT,
                                 &valueLoc))
         ? (EmberCoapContentFormatType)valueLoc
         : EMBER_COAP_CONTENT_FORMAT_NONE;
  }

  uint8_t *incoming = (uint8_t *)payload;
  uint8_t *outgoing = response;
  EmberZclClusterSpec_t spec;
  EmZclUriContext_t uri;
  uri.clusterSpec = &spec;

  // Decode all "/zcl/e/" tags in the response.
  while (emZclParseUri(payload,
                       payloadLength,
                       &incoming,   //ptr gets modified by call.
                       cf,
                       &uri)) {
    entry.destination.application.data.endpointId = uri.endpointId;

    // match the opposite role.
    uri.clusterSpec->role = (uri.clusterSpec->role == EMBER_ZCL_ROLE_CLIENT
                             ? EMBER_ZCL_ROLE_SERVER
                             : EMBER_ZCL_ROLE_CLIENT);
    MEMCOPY(&entry.clusterSpec, uri.clusterSpec, sizeof(EmberZclClusterSpec_t));

    for (size_t i = 0; i < emZclEndpointCount; i++) {
      if (emZclEndpointHasCluster(emZclEndpointTable[i].endpointId,
                                  &entry.clusterSpec)) {
        if (bind) {
          entry.endpointId = emZclEndpointTable[i].endpointId;
          emberZclAddBinding(&entry);
        }

        if (outgoing != NULL) {
          outgoing += append(emZclEndpointTable[i].endpointId,
                             uri.clusterSpec,
                             (outgoing == response),
                             outgoing);
        }
      }
    } // for
  } // while

  return outgoing - response;
}
