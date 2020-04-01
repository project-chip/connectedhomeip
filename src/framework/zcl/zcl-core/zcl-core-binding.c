/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#include CHIP_AF_API_HAL
#include CHIP_AF_API_ZCL_CORE

typedef struct {
  ChipZclBindingContext_t context;
  ChipZclBindingEntry_t entry;
  ChipZclBindingResponseHandler handler;
} Response;

typedef struct {
  ChipZclReportingConfigurationId_t reportingConfigurationId;
  uint8_t uri[CHIP_ZCL_URI_MAX_LENGTH];
} Binding_t;
#define CHIP_ZCLIP_STRUCT Binding_t
static const ZclipStructSpec bindingSpec[] = {
  CHIP_ZCLIP_OBJECT(sizeof(CHIP_ZCLIP_STRUCT),
                     2,     // fieldCount
                     NULL), // names
  CHIP_ZCLIP_FIELD_NAMED(CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER, reportingConfigurationId, "r"),
  CHIP_ZCLIP_FIELD_NAMED(CHIP_ZCLIP_TYPE_MAX_LENGTH_STRING, uri, "u"),
};
#undef CHIP_ZCLIP_STRUCT

static bool decodeBindingOta(const ChZclContext_t *context,
                             ChipZclBindingEntry_t *entry);

static ChipStatus sendBindingCommand(const ChipZclDestination_t *destination,
                                      const ChipZclBindingEntry_t *entry,
                                      ChipZclBindingId_t bindingId,
                                      const ChipZclBindingResponseHandler handler,
                                      ChipCoapCode code);

static void responseHandler(ChipCoapStatus status,
                            ChipCoapCode code,
                            ChipCoapReadOptions *options,
                            uint8_t *payload,
                            uint16_t payloadLength,
                            ChipCoapResponseInfo *info);

// Use the port as an 'in-use' marker, because zero is not a legal port.
static bool isFree(ChipZclBindingId_t bindingId)
{
  ChipZclBindingEntry_t entry = { 0 };
  halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_BINDING_TABLE, bindingId);
  return (entry.destination.network.port == 0);
}

static bool compareNetworkDestination(const ChipZclBindingEntry_t *b1,
                                      const ChipZclBindingEntry_t *b2)
{
  if (b1->destination.network.type == b2->destination.network.type) {
    switch (b1->destination.network.type) {
      case CHIP_ZCL_NETWORK_DESTINATION_TYPE_ADDRESS:
        return (MEMCOMPARE(&b1->destination.network.data.address,
                           &b2->destination.network.data.address,
                           sizeof(ChipIpv6Address))
                == 0);
      case CHIP_ZCL_NETWORK_DESTINATION_TYPE_UID:
        return (MEMCOMPARE(&b1->destination.network.data.uid,
                           &b2->destination.network.data.uid,
                           sizeof(ChipZclUid_t))
                == 0);
      default:
        assert(false);
        return false;
    }
  }
  return false;
}

static bool compareApplicationDestination(const ChipZclBindingEntry_t *b1,
                                          const ChipZclBindingEntry_t *b2)
{
  // The compare would take in account the "type"
  if (b1->destination.application.type == b2->destination.application.type) {
    switch (b1->destination.application.type) {
      case CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT:
        return (b1->destination.application.data.endpointId
                == b2->destination.application.data.endpointId);
        break;
      case CHIP_ZCL_APPLICATION_DESTINATION_TYPE_GROUP:
        return (b1->destination.application.data.groupId
                == b2->destination.application.data.groupId);
        break;
      default:
        assert(false);
        return false;
    }
  }
  return false;
}

static bool compare(const ChipZclBindingEntry_t *b1, ChipZclBindingId_t b2id)
{
  ChipZclBindingEntry_t b2;
  halCommonGetIndexedToken(&b2, TOKEN_ZCL_CORE_BINDING_TABLE, b2id);
  return (b1->endpointId == b2.endpointId
          && chipZclAreClusterSpecsEqual(&b1->clusterSpec, &b2.clusterSpec)
          && b1->destination.network.scheme == b2.destination.network.scheme
          && compareNetworkDestination(b1, &b2)
          && compareApplicationDestination(b1, &b2)
          && b1->destination.network.port == b2.destination.network.port
          && b1->reportingConfigurationId == b2.reportingConfigurationId);
}

static ChipZclBindingId_t find(const ChipZclBindingEntry_t *entry,
                                bool findFree)
{
  ChipZclBindingId_t index = CHIP_ZCL_BINDING_NULL;
  if (entry != NULL) {
    for (ChipZclBindingId_t i = 0; i < CHIP_ZCL_BINDING_TABLE_SIZE; i++) {
      if (compare(entry, i)) {
        return i;
      } else if (findFree && index == CHIP_ZCL_BINDING_NULL && isFree(i)) {
        index = i;
      }
    }
  }
  return index;
}

static bool validateScheme(const ChipZclBindingEntry_t *entry)
{
  switch (entry->destination.network.scheme) {
    case CHIP_ZCL_SCHEME_COAP:
    case CHIP_ZCL_SCHEME_COAPS:
      return true;
    default:
      return false;
  }
}

static bool validateNetworkDestination(const ChipZclBindingEntry_t *entry)
{
  switch (entry->destination.network.type) {
    case CHIP_ZCL_NETWORK_DESTINATION_TYPE_ADDRESS:
    case CHIP_ZCL_NETWORK_DESTINATION_TYPE_UID:
      return true;
    default:
      return false;
  }
}

static bool validateApplicationDestination(const ChipZclBindingEntry_t *entry)
{
  switch (entry->destination.application.type) {
    case CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT:
      if ((entry->destination.application.data.endpointId >= CHIP_ZCL_ENDPOINT_MIN)
          && (entry->destination.application.data.endpointId <= CHIP_ZCL_ENDPOINT_MAX)) {
        return true;
      }
      break;
    case CHIP_ZCL_APPLICATION_DESTINATION_TYPE_GROUP:
      if ((entry->destination.application.data.groupId >= CHIP_ZCL_GROUP_MIN)
          && (entry->destination.application.data.groupId <= CHIP_ZCL_GROUP_MAX)) {
        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

static bool validate(const ChipZclBindingEntry_t *entry)
{
  // chZclEndpointHasCluster verifies the endpoint exists and implements the
  // cluster.  Zero is not a valid UDP port.
  return (entry == NULL
          || (chZclEndpointHasCluster(entry->endpointId, &entry->clusterSpec)
              && validateScheme(entry)
              && validateNetworkDestination(entry)
              && validateApplicationDestination(entry)
              && entry->destination.network.port != 0
              && (entry->reportingConfigurationId == CHIP_ZCL_REPORTING_CONFIGURATION_NULL
                  || chZclHasReportingConfiguration(entry->endpointId,
                                                    &entry->clusterSpec,
                                                    entry->reportingConfigurationId))));
}

static bool set(ChipZclBindingId_t bindingId,
                const ChipZclBindingEntry_t *entry)
{
  if (bindingId < CHIP_ZCL_BINDING_TABLE_SIZE && validate(entry)) {
    ChipZclBindingEntry_t unused = { 0 };
    halCommonSetIndexedToken(TOKEN_ZCL_CORE_BINDING_TABLE,
                             bindingId,
                             (ChipZclBindingId_t *)(entry == NULL
                                                     ? &unused
                                                     : entry));
    return true;
  } else {
    return false;
  }
}

void chZclBindingNetworkStatusHandler(ChipNetworkStatus newNetworkStatus,
                                      ChipNetworkStatus oldNetworkStatus,
                                      ChipJoinFailureReason reason)
{
  // If the device is no longer associated with a network, its bindings are
  // removed, because they point to devices that are no longer accessible.
  if (newNetworkStatus == CHIP_NO_NETWORK) {
    chipZclRemoveAllBindings();
  }
}

bool chipZclHasBinding(ChipZclBindingId_t bindingId)
{
  return chipZclGetBinding(bindingId, NULL);
}

bool chipZclGetBinding(ChipZclBindingId_t bindingId,
                        ChipZclBindingEntry_t *entry)
{
  if (CHIP_ZCL_BINDING_TABLE_SIZE <= bindingId || isFree(bindingId)) {
    return false;
  } else {
    if (entry != NULL) {
      halCommonGetIndexedToken(entry, TOKEN_ZCL_CORE_BINDING_TABLE, bindingId);
    }
    return true;
  }
}

bool chipZclSetBinding(ChipZclBindingId_t bindingId,
                        const ChipZclBindingEntry_t *entry)
{
  ChipZclBindingId_t duplicateId = find(entry, false);   // find duplicate only

  return (duplicateId == bindingId
          || (duplicateId == CHIP_ZCL_BINDING_NULL
              && set(bindingId, entry)));
}

ChipZclBindingId_t chipZclAddBinding(const ChipZclBindingEntry_t *entry)
{
  ChipZclBindingId_t bindingId = find(entry, true); // find duplicate or free
  return (set(bindingId, entry) ? bindingId : CHIP_ZCL_BINDING_NULL);
}

bool chipZclRemoveBinding(ChipZclBindingId_t bindingId)
{
  return chipZclSetBinding(bindingId, NULL);
}

bool chipZclRemoveAllBindings(void)
{
  bool removed = false;
  for (ChipZclBindingId_t i = 0; i < CHIP_ZCL_BINDING_TABLE_SIZE; i++) {
    if (chipZclRemoveBinding(i)) {
      removed = true;
    }
  }
  return removed;
}

ChipStatus chipZclSendAddBinding(const ChipZclDestination_t *destination,
                                   const ChipZclBindingEntry_t *entry,
                                   const ChipZclBindingResponseHandler handler)
{
  return sendBindingCommand(destination,
                            entry,
                            CHIP_ZCL_BINDING_NULL,
                            handler,
                            CHIP_COAP_CODE_POST);
}

ChipStatus chipZclSendUpdateBinding(const ChipZclDestination_t *destination,
                                      const ChipZclBindingEntry_t *entry,
                                      ChipZclBindingId_t bindingId,
                                      const ChipZclBindingResponseHandler handler)
{
  return sendBindingCommand(destination,
                            entry,
                            bindingId,
                            handler,
                            CHIP_COAP_CODE_PUT);
}

ChipStatus chipZclSendRemoveBinding(const ChipZclDestination_t *destination,
                                      const ChipZclClusterSpec_t *clusterSpec,
                                      ChipZclBindingId_t bindingId,
                                      const ChipZclBindingResponseHandler handler)
{
  ChipZclBindingEntry_t entry = {
    .clusterSpec = *clusterSpec,
  };
  return sendBindingCommand(destination,
                            &entry,
                            bindingId,
                            handler,
                            CHIP_COAP_CODE_DELETE);
}

void chZclReadDestinationFromBinding(const ChipZclBindingEntry_t *binding,
                                     ChipZclDestination_t *destination)
{
  destination->application.type = binding->destination.application.type;
  destination->application.data = binding->destination.application.data;
  destination->network.flags =
    (binding->destination.network.scheme == CHIP_ZCL_SCHEME_COAPS
     ? CHIP_ZCL_USE_COAPS_FLAG
     : CHIP_ZCL_NO_FLAGS);
  switch (binding->destination.network.type) {
    case CHIP_ZCL_NETWORK_DESTINATION_TYPE_ADDRESS:
      destination->network.flags |= CHIP_ZCL_HAVE_IPV6_ADDRESS_FLAG;
      MEMCOPY(&destination->network.address,
              &binding->destination.network.data.address,
              sizeof(ChipIpv6Address));
      break;
    case CHIP_ZCL_NETWORK_DESTINATION_TYPE_UID:
      destination->network.flags |= CHIP_ZCL_HAVE_UID_FLAG;
      MEMCOPY(&destination->network.uid,
              &binding->destination.network.data.uid,
              sizeof(ChipZclUid_t));
      break;
  }
  destination->network.port = binding->destination.network.port;
}

bool chipZclGetDestinationFromBinding(const ChipZclClusterSpec_t *clusterSpec,
                                       ChipZclBindingId_t *bindingIdx,
                                       ChipZclDestination_t *destination)
{
  ChipZclBindingId_t i;
  ChipZclBindingEntry_t bindingEntry;

  for (i = *bindingIdx; i < CHIP_ZCL_BINDING_TABLE_SIZE; i++) {
    if ((chipZclGetBinding(i, &bindingEntry))
        && (bindingEntry.clusterSpec.id == clusterSpec->id)) {
      chZclReadDestinationFromBinding(&bindingEntry, destination);
      *bindingIdx = i + 1;  // Increment index if binding found.
      return true;
    }
  }

  return false;
}

size_t chZclGetBindingCount(void)
{
  size_t bindingCount = 0;
  for (ChipZclBindingId_t i = 0; i < CHIP_ZCL_BINDING_TABLE_SIZE; i++) {
    if (!isFree(i)) {
      bindingCount++;
    }
  }
  return bindingCount;
}

bool chZclHasBinding(const ChZclContext_t *context,
                     ChipZclBindingId_t bindingId)
{
  ChipZclBindingEntry_t entry = { 0 };
  return (chipZclGetBinding(bindingId, &entry)
          && context->endpoint->endpointId == entry.endpointId
          && chipZclAreClusterSpecsEqual(&context->clusterSpec,
                                          &entry.clusterSpec));
}

// GET .../b
static void getBindingIdsHandler(const ChZclContext_t *context)
{
  CborState state;
  uint8_t buffer[CH_ZCL_MAX_PAYLOAD_SIZE];
  emCborEncodeIndefiniteArrayStart(&state, buffer, sizeof(buffer));
  for (ChipZclBindingId_t i = 0; i < CHIP_ZCL_BINDING_TABLE_SIZE; i++) {
    if (chZclHasBinding(context, i)
        && !emCborEncodeValue(&state,
                              CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER,
                              sizeof(i),
                              (const uint8_t *)&i)) {
      chZclRespond500InternalServerError(context->info);
      return;
    }
  }
  if (emCborEncodeBreak(&state)) {
    chZclRespond205ContentCborState(context->info, &state);
  } else {
    chZclRespond500InternalServerError(context->info);
  }
}

// POST .../b
static void addBindingHandler(const ChZclContext_t *context)
{
  ChipZclBindingEntry_t entry = {
    .endpointId = context->endpoint->endpointId,
    .clusterSpec = context->clusterSpec,
    .destination = { { 0 } },
    .reportingConfigurationId = CHIP_ZCL_REPORTING_CONFIGURATION_DEFAULT,
  };
  if (decodeBindingOta(context, &entry)) {
    ChipZclBindingId_t bindingId = chipZclAddBinding(&entry);
    if (bindingId == CHIP_ZCL_BINDING_NULL) {
      // Spec 16-07008-040: 3.9.4 - the device SHALL respond with 5.00 Internal Server Error.
      chZclRespond500InternalServerError(context->info);
    } else {
      ChipZclApplicationDestination_t destination = {
        .data.endpointId = context->endpoint->endpointId,
        .type = CHIP_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT,
      };
      uint8_t uriPath[CHIP_ZCL_URI_PATH_MAX_LENGTH];
      chZclBindingIdToUriPath(&destination,
                              &context->clusterSpec,
                              bindingId,
                              uriPath);
      chZclRespond201Created(context->info, uriPath);
    }
  } else {
    // Spec 16-07008-040: 3.9.4 - If the Destination URI is not set the device SHALL respond with 4.00 Bad Request
    chZclRespond400BadRequest(context->info);
  }
}

// GET .../b/B
static void getBindingHandler(const ChZclContext_t *context)
{
  ChipZclBindingEntry_t entry = { 0 };
  chipZclGetBinding(context->bindingId, &entry);

  // chZclDestinationToUri adds a null terminator, so we don't need to add one
  // here.
  Binding_t binding = {
    .reportingConfigurationId = entry.reportingConfigurationId,
    .uri = { 0 },
  };
  ChipZclDestination_t destination;
  chZclReadDestinationFromBinding(&entry, &destination);
  chZclDestinationToUri(&destination, binding.uri);

  uint8_t buffer[CH_ZCL_MAX_PAYLOAD_SIZE];
  uint16_t payloadLength = emCborEncodeOneStruct(buffer,
                                                 sizeof(buffer),
                                                 bindingSpec,
                                                 &binding);
  if (payloadLength != 0) {
    chZclRespond205ContentCbor(context->info, buffer, payloadLength);
  } else {
    chZclRespond500InternalServerError(context->info);
  }
}

// PUT .../b/B
static void updateBindingHandler(const ChZclContext_t *context)
{
  ChipZclBindingEntry_t entry = { 0 };
  if (chipZclGetBinding(context->bindingId, &entry)) {
    if (decodeBindingOta(context, &entry)
        && chipZclSetBinding(context->bindingId, &entry)) {
      chZclRespond204ChangedWithStatus(context->info, CHIP_ZCL_STATUS_SUCCESS);
    } else {
      // Spec 16-07008-071: 3.10.4: If a match exists respond with 4.00 and code DUPLICATE_EXISTS.
      chZclRespond400BadRequestWithStatus(context->info, CHIP_ZCL_STATUS_DUPLICATE_EXISTS);
    }
  } else {
    chZclRespond500InternalServerError(context->info);
  }
}

// DELETE .../b/B
static void removeBindingHandler(const ChZclContext_t *context)
{
  chipZclRemoveBinding(context->bindingId);
  chZclRespond202Deleted(context->info);
}

static bool decodeBindingOta(const ChZclContext_t *context,
                             ChipZclBindingEntry_t *entry)
{
  Binding_t binding = {
    .reportingConfigurationId = CHIP_ZCL_REPORTING_CONFIGURATION_DEFAULT,
    .uri = { 0 },
  };

  if (!emCborDecodeOneStruct(context->payload,
                             context->payloadLength,
                             bindingSpec,
                             &binding)) {
    return false;
  }

  entry->reportingConfigurationId = binding.reportingConfigurationId;

  // The binding is validated here, even though adding or setting it later will
  // result in another validation.  This is so that we can reject invalid
  // bindings with a 4.00 instead of a 5.00, which is what would happen if we
  // waited for set() to do the validation.
  return (chZclUriToBindingEntry(binding.uri, entry, false)
          && validate(entry));
}

static ChipStatus sendBindingCommand(const ChipZclDestination_t *destination,
                                      const ChipZclBindingEntry_t *entry,
                                      ChipZclBindingId_t bindingId,
                                      const ChipZclBindingResponseHandler handler,
                                      ChipCoapCode code)
{
  assert(code == CHIP_COAP_CODE_POST        // add binding
         || code == CHIP_COAP_CODE_PUT      // update binding
         || code == CHIP_COAP_CODE_DELETE); // delete binding

  uint8_t buffer[CH_ZCL_MAX_PAYLOAD_SIZE];
  uint16_t payloadLength = 0;
  if (code != CHIP_COAP_CODE_DELETE) {
    Binding_t binding = {
      .reportingConfigurationId = entry->reportingConfigurationId,
      .uri = { 0 },
    };
    // chZclDestinationToUri adds a null terminator, so we don't need to add
    // one here.
    ChipZclDestination_t temp;
    chZclReadDestinationFromBinding(entry, &temp);
    chZclDestinationToUri(&temp, binding.uri);
    binding.reportingConfigurationId = entry->reportingConfigurationId;
    payloadLength = emCborEncodeOneStruct(buffer,
                                          sizeof(buffer),
                                          bindingSpec,
                                          &binding);
  }

  uint8_t uriPath[CHIP_ZCL_URI_PATH_MAX_LENGTH];
  if (bindingId == CHIP_ZCL_BINDING_NULL) {
    chZclBindingToUriPath(&destination->application,
                          &entry->clusterSpec,
                          uriPath);
  } else {
    chZclBindingIdToUriPath(&destination->application,
                            &entry->clusterSpec,
                            bindingId,
                            uriPath);
  }

  Response response = {
    .context = {
      .code = CHIP_COAP_CODE_EMPTY, // filled in after response
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
      .clusterSpec = &entry->clusterSpec,
      .bindingId = bindingId,
    },
    .entry = *entry,
    .handler = handler,
  };

  return chZclSend(&destination->network,
                   code,
                   uriPath,
                   buffer,
                   payloadLength,
                   (handler == NULL ? NULL : responseHandler),
                   &response,
                   sizeof(Response),
                   false);
}

static void responseHandler(ChipCoapStatus status,
                            ChipCoapCode code,
                            ChipCoapReadOptions *options,
                            uint8_t *payload,
                            uint16_t payloadLength,
                            ChipCoapResponseInfo *info)
{
  // We should only be here if the application specified a handler.
  assert(info->applicationDataLength == sizeof(Response));
  const Response *response = info->applicationData;
  assert(*response->handler != NULL);

  chZclCoapStatusHandler(status, info);
  ((Response *)response)->context.code = code;

  // TODO: What should happen for failures?
  // TODO: How do we know what the binding ID is in the received URI?
  // TODO: When creating a new binding, populate response->context.bindingId
  // with the new id.
  (*response->handler)((ChipZclMessageStatus_t) status,
                       &response->context,
                       &response->entry);
}

// zcl/e/XX/<cluster>/b:
//   GET: return list of binding ids.
//   POST: add binding.
//   OTHER: not allowed.
void chZclUriClusterBindingHandler(ChZclContext_t *context)
{
  if (context->groupId != CHIP_ZCL_GROUP_NULL) {
    chZclRespond404NotFound(context->info);
  } else if (context->code == CHIP_COAP_CODE_GET) {
    getBindingIdsHandler(context);
  } else if (context->code == CHIP_COAP_CODE_POST) {
    addBindingHandler(context);
  } else {
    assert(false);
  }
}

// zcl/e/XX/<cluster>/b/XX:
//   GET: return binding.
//   PUT: replace binding.
//   DELETE: remove binding.
//   OTHER: not allowed.
void chZclUriClusterBindingIdHandler(ChZclContext_t *context)
{
  if (context->groupId != CHIP_ZCL_GROUP_NULL) {
    chZclRespond404NotFound(context->info);
  } else if (context->code == CHIP_COAP_CODE_GET) {
    getBindingHandler(context);
  } else if (context->code == CHIP_COAP_CODE_PUT) {
    updateBindingHandler(context);
  } else if (context->code == CHIP_COAP_CODE_DELETE) {
    removeBindingHandler(context);
  } else {
    assert(false);
  }
}
