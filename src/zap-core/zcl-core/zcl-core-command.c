/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_ZCL_CORE

typedef struct {
  EmberZclCommandContext_t context;
  const ZclipStructSpec *responseSpec;
  EmZclResponseHandler handler;
} Response;

static void responseHandler(EmberCoapStatus status,
                            EmberCoapCode code,
                            EmberCoapReadOptions *options,
                            uint8_t *payload,
                            uint16_t payloadLength,
                            EmberCoapResponseInfo *info);
static void handle(EmberZclMessageStatus_t status, const Response *response);
static EmberZclStatus_t getCommandIds(const EmZclContext_t *context,
                                      CborState *state,
                                      void *data);
static EmberZclStatus_t executeCommand(const EmZclContext_t *context,
                                       CborState *state,
                                       void *data);

const EmZclCommandEntry_t *emZclFindCommand(const EmberZclClusterSpec_t *clusterSpec,
                                            EmberZclCommandId_t commandId)
{
  for (size_t i = 0; i < emZclCommandCount; i++) {
    int32_t compare
      = emberZclCompareClusterSpec(emZclCommandTable[i].clusterSpec,
                                   clusterSpec);
    if (compare > 0) {
      break;
    } else if (compare == 0 && emZclCommandTable[i].commandId == commandId) {
      return &emZclCommandTable[i];
    }
  }
  return NULL;
}

EmberStatus emZclSendCommandRequest(const EmberZclDestination_t *destination,
                                    const EmberZclClusterSpec_t *clusterSpec,
                                    EmberZclCommandId_t commandId,
                                    const void *request,
                                    const ZclipStructSpec *requestSpec,
                                    const ZclipStructSpec *responseSpec,
                                    const EmZclResponseHandler handler)
{
  // We can only send a payload if we have the spec to encode it.  It is okay
  // to not send a payload, even if the command has fields.
  assert(request == NULL || requestSpec != NULL);

  uint8_t uriPath[EMBER_ZCL_URI_PATH_MAX_LENGTH];
  emZclCommandIdToUriPath(&destination->application,
                          clusterSpec,
                          commandId,
                          uriPath);

  Response response = {
    .context = {
      .remoteAddress = { { 0 } },        // unused
      .code = EMBER_COAP_CODE_EMPTY, // filled in when the response arrives
      .payload = NULL,               // filled in when the response arrives
      .payloadLength = 0,            // filled in when the response arrives
      .groupId
        = ((destination->application.type
            == EMBER_ZCL_APPLICATION_DESTINATION_TYPE_GROUP)
           ? destination->application.data.groupId
           : EMBER_ZCL_GROUP_NULL),
      .endpointId
        = ((destination->application.type
            == EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT)
           ? destination->application.data.endpointId
           : EMBER_ZCL_ENDPOINT_NULL),
      .clusterSpec = clusterSpec,
      .commandId = commandId,
      .state = NULL,    // filled in when the response arrives
      .buffer = NULL,   // unused
      .result = EMBER_ERR_FATAL, // unused
    },
    .responseSpec = responseSpec,
    .handler = handler,
  };

  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  uint16_t payloadLength = (request == NULL
                            ? 0
                            : emCborEncodeOneStruct(buffer,
                                                    sizeof(buffer),
                                                    requestSpec,
                                                    request));
  return emZclSend(&destination->network,
                   EMBER_COAP_CODE_POST,
                   uriPath,
                   buffer,
                   payloadLength,
                   (handler == NULL ? NULL : responseHandler),
                   &response,
                   sizeof(Response),
                   false);
}

EmberStatus emZclSendCommandResponse(const EmberZclCommandContext_t *context,
                                     const void *response,
                                     const ZclipStructSpec *responseSpec)
{
  if ((response == NULL) || (responseSpec == NULL)) {
    return EMBER_SUCCESS;
  }

  // Encode the cbor response payload.
  // TODO: How should we handle failures?  What if one endpoint fails but
  // another succeeds?  What if we can add the endpoint id, but not the
  // response itself?
  EmberStatus status;
  status = (emCborEncodeStruct(context->state, responseSpec, response))
           ? EMBER_SUCCESS
           : EMBER_ERR_FATAL;

  ((EmberZclCommandContext_t *)context)->result = status;

  return (status);
}

EmberStatus emberZclSendDefaultResponse(const EmberZclCommandContext_t *context,
                                        EmberZclStatus_t status)
{
  if (emIsMulticastAddress(context->info->localAddress.bytes)) {
    ((EmberZclCommandContext_t *)context)->result = EMBER_NO_BUFFERS; // command response will be suppressed
    return EMBER_ZCL_STATUS_ACTION_DENIED;
  }

  EmberStatus returnStatus = (emZclEncodeDefaultResponse(context->state, status)
                              ? EMBER_SUCCESS
                              : EMBER_ERR_FATAL);
  ((EmberZclCommandContext_t *)context)->result = returnStatus;
  return returnStatus;
}

static void responseHandler(EmberCoapStatus coapStatus,
                            EmberCoapCode code,
                            EmberCoapReadOptions *options,
                            uint8_t *payload,
                            uint16_t payloadLength,
                            EmberCoapResponseInfo *info)
{
  // We should only be here if the application specified a handler.
  assert(info->applicationDataLength == sizeof(Response));
  Response *response = info->applicationData;
  assert(*response->handler != NULL);
  EmberZclMessageStatus_t status = (EmberZclMessageStatus_t) coapStatus;
  emZclCoapStatusHandler(coapStatus, info);
  response->context.code = code;
  response->context.payload = payload;
  response->context.payloadLength = payloadLength;

  // TODO: What should happen for failures?
  // TODO: What should happen if the overall payload is missing or malformed?
  // Note that this is a separate issue from how missing or malformed responses
  // from the individual endpoints should be handled.
  if (status == EMBER_ZCL_MESSAGE_STATUS_COAP_RESPONSE
      && emberCoapIsSuccessResponse(code)) {
    CborState state;
    ((Response *)response)->context.state = &state;
    emCborDecodeStart(&state, payload, payloadLength);
    if (response->context.groupId == EMBER_ZCL_GROUP_NULL) {
      handle(status, response);
      return;
    } else if (emCborDecodeMap(&state) && !emCborPeekSequenceEnd(&state)) {   // Require non-empty map
      while (emCborDecodeValue(&state,
                               EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                               sizeof(response->context.endpointId),
                               (uint8_t *)&response->context.endpointId)) {
        handle(status, response);
      }
      return;
    }
  }
  (*response->handler)(status, &response->context, NULL);
}

static void handle(EmberZclMessageStatus_t status, const Response *response)
{
  // TODO: If we expect a response payload but it is missing, or it is present
  // but malformed, would should we do?
  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  (*response->handler)(status,
                       &response->context,
                       ((response->responseSpec != NULL
                         && emCborDecodeStruct(response->context.state,
                                               response->responseSpec,
                                               buffer))
                        ? buffer
                        : NULL));
}

// zcl/[eg]/XX/<cluster>/c:
//   GET: list commands in cluster.
//   OTHER: not allowed.
void emZclUriClusterCommandHandler(EmZclContext_t *context)
{
  CborState state;
  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  EmberZclStatus_t status;
  emCborEncodeStart(&state, buffer, sizeof(buffer));

  status = emZclMultiEndpointDispatch(context, getCommandIds, &state, NULL);
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    emZclRespond205ContentCborState(context->info, &state);
  } else {
    emZclRespond500InternalServerError(context->info);
  }
}

static EmberZclStatus_t getCommandIds(const EmZclContext_t *context,
                                      CborState *state,
                                      void *data)
{
  emCborEncodeIndefiniteArray(state);
  for (size_t i = 0; i < emZclCommandCount; i++) {
    int32_t compare
      = emberZclCompareClusterSpec(emZclCommandTable[i].clusterSpec,
                                   &context->clusterSpec);
    if (compare > 0) {
      break;
    } else if (compare == 0
               && !emCborEncodeValue(state,
                                     EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                                     sizeof(emZclCommandTable[i].commandId),
                                     (const uint8_t *)&emZclCommandTable[i].commandId)) {
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }
  if (emCborEncodeBreak(state)) {
    return EMBER_ZCL_STATUS_SUCCESS;
  }

  return EMBER_ZCL_STATUS_FAILURE;
}

// zcl/[eg]/XX/<cluster>/c/XX:
//   POST: execute command.
//   OTHER: not allowed.
void emZclUriClusterCommandIdHandler(EmZclContext_t *context)
{
  // If a parameter is missing at least pass zeros
  uint8_t inBuffer[EM_ZCL_MAX_PAYLOAD_SIZE] = { 0 };

  if (context->command->spec != NULL
      && !emCborDecodeOneStruct(context->payload,
                                context->payloadLength,
                                context->command->spec,
                                inBuffer)) {
    // BDB Section 2.8.6.1.2, item 1. Also see zcl-test-group-1 test 1.55
    emZclRespond400BadRequestWithStatus(context->info, EMBER_ZCL_STATUS_MALFORMED_COMMAND);
    return;
  }

  CborState state;
  uint8_t outBuffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  EmberZclStatus_t status;
  emCborEncodeStart(&state, outBuffer, sizeof(outBuffer));

  EmberZclCommandContext_t commandContext = {
    .remoteAddress = context->info->remoteAddress,
    .code = context->code,
    .info = context->info,
    .payload = context->payload,
    .payloadLength = context->payloadLength,
    .groupId = context->groupId,
    .endpointId = EMBER_ZCL_ENDPOINT_NULL, // filled in when the command is executed
    .clusterSpec = context->command->clusterSpec,
    .commandId = context->command->commandId,
    .state = &state,
    .buffer = inBuffer,
    .result = EMBER_SUCCESS,
  };

  status = emZclMultiEndpointDispatch(context,
                                      executeCommand,
                                      &state,
                                      &commandContext);
  switch (status) {
    case EMBER_ZCL_STATUS_SUCCESS:
      emZclRespond204ChangedCborState(context->info, &state);
      break;
    case EMBER_ZCL_STATUS_ACTION_DENIED:
      break; // command response is suppressed.
    default:
      emZclRespond500InternalServerError(context->info);
      break;
  }
}

static EmberZclStatus_t executeCommand(const EmZclContext_t *context,
                                       CborState *state,
                                       void *data)
{
  EmberZclCommandContext_t *commandContext = data;
  commandContext->endpointId = context->endpoint->endpointId;
  (*context->command->handler)(commandContext, commandContext->buffer);

  // Translate result to zclStatus.
  EmberZclStatus_t status;
  switch (commandContext->result) {
    case EMBER_SUCCESS:
      status = EMBER_ZCL_STATUS_SUCCESS;
      break;
    case EMBER_NO_BUFFERS:
      status = EMBER_ZCL_STATUS_ACTION_DENIED; // Will suppress command response.
      break;
    default:
      status = EMBER_ZCL_STATUS_FAILURE;
      break;
  }
  return status;
}
