/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#include CHIP_AF_API_ZCL_CORE

typedef struct {
  ChipZclCommandContext_t context;
  const ZclipStructSpec *responseSpec;
  ChZclResponseHandler handler;
} Response;

static void responseHandler(ChipCoapStatus status,
                            ChipCoapCode code,
                            ChipCoapReadOptions *options,
                            uint8_t *payload,
                            uint16_t payloadLength,
                            ChipCoapResponseInfo *info);
static void handle(ChipZclMessageStatus_t status, const Response *response);
static ChipZclStatus_t getCommandIds(const ChZclContext_t *context,
                                      CborState *state,
                                      void *data);
static ChipZclStatus_t executeCommand(const ChZclContext_t *context,
                                       CborState *state,
                                       void *data);

const ChZclCommandEntry_t *chZclFindCommand(const ChipZclClusterSpec_t *clusterSpec,
                                            ChipZclCommandId_t commandId)
{
  for (size_t i = 0; i < chZclCommandCount; i++) {
    int32_t compare
      = chipZclCompareClusterSpec(chZclCommandTable[i].clusterSpec,
                                   clusterSpec);
    if (compare > 0) {
      break;
    } else if (compare == 0 && chZclCommandTable[i].commandId == commandId) {
      return &chZclCommandTable[i];
    }
  }
  return NULL;
}

ChipStatus chZclSendCommandRequest(const ChipZclDestination_t *destination,
                                    const ChipZclClusterSpec_t *clusterSpec,
                                    ChipZclCommandId_t commandId,
                                    const void *request,
                                    const ZclipStructSpec *requestSpec,
                                    const ZclipStructSpec *responseSpec,
                                    const ChZclResponseHandler handler)
{
  // We can only send a payload if we have the spec to encode it.  It is okay
  // to not send a payload, even if the command has fields.
  assert(request == NULL || requestSpec != NULL);

  uint8_t uriPath[CHIP_ZCL_URI_PATH_MAX_LENGTH];
  chZclCommandIdToUriPath(&destination->application,
                          clusterSpec,
                          commandId,
                          uriPath);

  Response response = {
    .context = {
      .remoteAddress = { { 0 } },        // unused
      .code = CHIP_COAP_CODE_EMPTY, // filled in when the response arrives
      .payload = NULL,               // filled in when the response arrives
      .payloadLength = 0,            // filled in when the response arrives
      .groupId
        = ((destination->application.type
            == CHIP_ZCL_APPLICATION_DESTINATION_TYPE_GROUP)
           ? destination->application.data.groupId
           : CHIP_ZCL_GROUP_NULL),
      .endpointId
        = ((destination->application.type
            == CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT)
           ? destination->application.data.endpointId
           : CHIP_ZCL_ENDPOINT_NULL),
      .clusterSpec = clusterSpec,
      .commandId = commandId,
      .state = NULL,    // filled in when the response arrives
      .buffer = NULL,   // unused
      .result = CHIP_ERR_FATAL, // unused
    },
    .responseSpec = responseSpec,
    .handler = handler,
  };

  uint8_t buffer[CH_ZCL_MAX_PAYLOAD_SIZE];
  uint16_t payloadLength = (request == NULL
                            ? 0
                            : emCborEncodeOneStruct(buffer,
                                                    sizeof(buffer),
                                                    requestSpec,
                                                    request));
  return chZclSend(&destination->network,
                   CHIP_COAP_CODE_POST,
                   uriPath,
                   buffer,
                   payloadLength,
                   (handler == NULL ? NULL : responseHandler),
                   &response,
                   sizeof(Response),
                   false);
}

ChipStatus chZclSendCommandResponse(const ChipZclCommandContext_t *context,
                                     const void *response,
                                     const ZclipStructSpec *responseSpec)
{
  if ((response == NULL) || (responseSpec == NULL)) {
    return CHIP_SUCCESS;
  }

  // Encode the cbor response payload.
  // TODO: How should we handle failures?  What if one endpoint fails but
  // another succeeds?  What if we can add the endpoint id, but not the
  // response itself?
  ChipStatus status;
  status = (emCborEncodeStruct(context->state, responseSpec, response))
           ? CHIP_SUCCESS
           : CHIP_ERR_FATAL;

  ((ChipZclCommandContext_t *)context)->result = status;

  return (status);
}

ChipStatus chipZclSendDefaultResponse(const ChipZclCommandContext_t *context,
                                        ChipZclStatus_t status)
{
  if (emIsMulticastAddress(context->info->localAddress.bytes)) {
    ((ChipZclCommandContext_t *)context)->result = CHIP_NO_BUFFERS; // command response will be suppressed
    return CHIP_ZCL_STATUS_ACTION_DENIED;
  }

  ChipStatus returnStatus = (chZclEncodeDefaultResponse(context->state, status)
                              ? CHIP_SUCCESS
                              : CHIP_ERR_FATAL);
  ((ChipZclCommandContext_t *)context)->result = returnStatus;
  return returnStatus;
}

static void responseHandler(ChipCoapStatus coapStatus,
                            ChipCoapCode code,
                            ChipCoapReadOptions *options,
                            uint8_t *payload,
                            uint16_t payloadLength,
                            ChipCoapResponseInfo *info)
{
  // We should only be here if the application specified a handler.
  assert(info->applicationDataLength == sizeof(Response));
  Response *response = info->applicationData;
  assert(*response->handler != NULL);
  ChipZclMessageStatus_t status = (ChipZclMessageStatus_t) coapStatus;
  chZclCoapStatusHandler(coapStatus, info);
  response->context.code = code;
  response->context.payload = payload;
  response->context.payloadLength = payloadLength;

  // TODO: What should happen for failures?
  // TODO: What should happen if the overall payload is missing or malformed?
  // Note that this is a separate issue from how missing or malformed responses
  // from the individual endpoints should be handled.
  if (status == CHIP_ZCL_MESSAGE_STATUS_COAP_RESPONSE
      && chipCoapIsSuccessResponse(code)) {
    CborState state;
    ((Response *)response)->context.state = &state;
    emCborDecodeStart(&state, payload, payloadLength);
    if (response->context.groupId == CHIP_ZCL_GROUP_NULL) {
      handle(status, response);
      return;
    } else if (emCborDecodeMap(&state) && !emCborPeekSequenceEnd(&state)) {   // Require non-empty map
      while (emCborDecodeValue(&state,
                               CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER,
                               sizeof(response->context.endpointId),
                               (uint8_t *)&response->context.endpointId)) {
        handle(status, response);
      }
      return;
    }
  }
  (*response->handler)(status, &response->context, NULL);
}

static void handle(ChipZclMessageStatus_t status, const Response *response)
{
  // TODO: If we expect a response payload but it is missing, or it is present
  // but malformed, would should we do?
  uint8_t buffer[CH_ZCL_MAX_PAYLOAD_SIZE];
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
void chZclUriClusterCommandHandler(ChZclContext_t *context)
{
  CborState state;
  uint8_t buffer[CH_ZCL_MAX_PAYLOAD_SIZE];
  ChipZclStatus_t status;
  emCborEncodeStart(&state, buffer, sizeof(buffer));

  status = chZclMultiEndpointDispatch(context, getCommandIds, &state, NULL);
  if (status == CHIP_ZCL_STATUS_SUCCESS) {
    chZclRespond205ContentCborState(context->info, &state);
  } else {
    chZclRespond500InternalServerError(context->info);
  }
}

static ChipZclStatus_t getCommandIds(const ChZclContext_t *context,
                                      CborState *state,
                                      void *data)
{
  emCborEncodeIndefiniteArray(state);
  for (size_t i = 0; i < chZclCommandCount; i++) {
    int32_t compare
      = chipZclCompareClusterSpec(chZclCommandTable[i].clusterSpec,
                                   &context->clusterSpec);
    if (compare > 0) {
      break;
    } else if (compare == 0
               && !emCborEncodeValue(state,
                                     CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER,
                                     sizeof(chZclCommandTable[i].commandId),
                                     (const uint8_t *)&chZclCommandTable[i].commandId)) {
      return CHIP_ZCL_STATUS_FAILURE;
    }
  }
  if (emCborEncodeBreak(state)) {
    return CHIP_ZCL_STATUS_SUCCESS;
  }

  return CHIP_ZCL_STATUS_FAILURE;
}

// zcl/[eg]/XX/<cluster>/c/XX:
//   POST: execute command.
//   OTHER: not allowed.
void chZclUriClusterCommandIdHandler(ChZclContext_t *context)
{
  // If a parameter is missing at least pass zeros
  uint8_t inBuffer[CH_ZCL_MAX_PAYLOAD_SIZE] = { 0 };

  if (context->command->spec != NULL
      && !emCborDecodeOneStruct(context->payload,
                                context->payloadLength,
                                context->command->spec,
                                inBuffer)) {
    // BDB Section 2.8.6.1.2, item 1. Also see zcl-test-group-1 test 1.55
    chZclRespond400BadRequestWithStatus(context->info, CHIP_ZCL_STATUS_MALFORMED_COMMAND);
    return;
  }

  CborState state;
  uint8_t outBuffer[CH_ZCL_MAX_PAYLOAD_SIZE];
  ChipZclStatus_t status;
  emCborEncodeStart(&state, outBuffer, sizeof(outBuffer));

  ChipZclCommandContext_t commandContext = {
    .remoteAddress = context->info->remoteAddress,
    .code = context->code,
    .info = context->info,
    .payload = context->payload,
    .payloadLength = context->payloadLength,
    .groupId = context->groupId,
    .endpointId = CHIP_ZCL_ENDPOINT_NULL, // filled in when the command is executed
    .clusterSpec = context->command->clusterSpec,
    .commandId = context->command->commandId,
    .state = &state,
    .buffer = inBuffer,
    .result = CHIP_SUCCESS,
  };

  status = chZclMultiEndpointDispatch(context,
                                      executeCommand,
                                      &state,
                                      &commandContext);
  switch (status) {
    case CHIP_ZCL_STATUS_SUCCESS:
      chZclRespond204ChangedCborState(context->info, &state);
      break;
    case CHIP_ZCL_STATUS_ACTION_DENIED:
      break; // command response is suppressed.
    default:
      chZclRespond500InternalServerError(context->info);
      break;
  }
}

static ChipZclStatus_t executeCommand(const ChZclContext_t *context,
                                       CborState *state,
                                       void *data)
{
  ChipZclCommandContext_t *commandContext = data;
  commandContext->endpointId = context->endpoint->endpointId;
  (*context->command->handler)(commandContext, commandContext->buffer);

  // Translate result to zclStatus.
  ChipZclStatus_t status;
  switch (commandContext->result) {
    case CHIP_SUCCESS:
      status = CHIP_ZCL_STATUS_SUCCESS;
      break;
    case CHIP_NO_BUFFERS:
      status = CHIP_ZCL_STATUS_ACTION_DENIED; // Will suppress command response.
      break;
    default:
      status = CHIP_ZCL_STATUS_FAILURE;
      break;
  }
  return status;
}
