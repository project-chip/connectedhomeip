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

typedef struct {
  EmberZclBindingContext_t context;
  EmberZclBindingEntry_t entry;
  EmberZclBindingResponseHandler handler;
} Response;

typedef struct {
  EmberZclReportingConfigurationId_t reportingConfigurationId;
  uint8_t uri[EMBER_ZCL_URI_MAX_LENGTH];
} Binding_t;
#define EMBER_ZCLIP_STRUCT Binding_t
static const ZclipStructSpec bindingSpec[] = {
  EMBER_ZCLIP_OBJECT(sizeof(EMBER_ZCLIP_STRUCT),
                     2,     // fieldCount
                     NULL), // names
  EMBER_ZCLIP_FIELD_NAMED(EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, reportingConfigurationId, "r"),
  EMBER_ZCLIP_FIELD_NAMED(EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, uri, "u"),
};
#undef EMBER_ZCLIP_STRUCT

static bool decodeBindingOta(const EmZclContext_t *context,
                             EmberZclBindingEntry_t *entry);

static EmberStatus sendBindingCommand(const EmberZclDestination_t *destination,
                                      const EmberZclBindingEntry_t *entry,
                                      EmberZclBindingId_t bindingId,
                                      const EmberZclBindingResponseHandler handler,
                                      EmberCoapCode code);

static void responseHandler(EmberCoapStatus status,
                            EmberCoapCode code,
                            EmberCoapReadOptions *options,
                            uint8_t *payload,
                            uint16_t payloadLength,
                            EmberCoapResponseInfo *info);

// Use the port as an 'in-use' marker, because zero is not a legal port.
static bool isFree(EmberZclBindingId_t bindingId)
{
  EmberZclBindingEntry_t entry = { 0 };
  halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_BINDING_TABLE, bindingId);
  return (entry.destination.network.port == 0);
}

static bool compareNetworkDestination(const EmberZclBindingEntry_t *b1,
                                      const EmberZclBindingEntry_t *b2)
{
  if (b1->destination.network.type == b2->destination.network.type) {
    switch (b1->destination.network.type) {
      case EMBER_ZCL_NETWORK_DESTINATION_TYPE_ADDRESS:
        return (MEMCOMPARE(&b1->destination.network.data.address,
                           &b2->destination.network.data.address,
                           sizeof(EmberIpv6Address))
                == 0);
      case EMBER_ZCL_NETWORK_DESTINATION_TYPE_UID:
        return (MEMCOMPARE(&b1->destination.network.data.uid,
                           &b2->destination.network.data.uid,
                           sizeof(EmberZclUid_t))
                == 0);
      default:
        assert(false);
        return false;
    }
  }
  return false;
}

static bool compareApplicationDestination(const EmberZclBindingEntry_t *b1,
                                          const EmberZclBindingEntry_t *b2)
{
  // The compare would take in account the "type"
  if (b1->destination.application.type == b2->destination.application.type) {
    switch (b1->destination.application.type) {
      case EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT:
        return (b1->destination.application.data.endpointId
                == b2->destination.application.data.endpointId);
        break;
      case EMBER_ZCL_APPLICATION_DESTINATION_TYPE_GROUP:
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

static bool compare(const EmberZclBindingEntry_t *b1, EmberZclBindingId_t b2id)
{
  EmberZclBindingEntry_t b2;
  halCommonGetIndexedToken(&b2, TOKEN_ZCL_CORE_BINDING_TABLE, b2id);
  return (b1->endpointId == b2.endpointId
          && emberZclAreClusterSpecsEqual(&b1->clusterSpec, &b2.clusterSpec)
          && b1->destination.network.scheme == b2.destination.network.scheme
          && compareNetworkDestination(b1, &b2)
          && compareApplicationDestination(b1, &b2)
          && b1->destination.network.port == b2.destination.network.port
          && b1->reportingConfigurationId == b2.reportingConfigurationId);
}

static EmberZclBindingId_t find(const EmberZclBindingEntry_t *entry,
                                bool findFree)
{
  EmberZclBindingId_t index = EMBER_ZCL_BINDING_NULL;
  if (entry != NULL) {
    for (EmberZclBindingId_t i = 0; i < EMBER_ZCL_BINDING_TABLE_SIZE; i++) {
      if (compare(entry, i)) {
        return i;
      } else if (findFree && index == EMBER_ZCL_BINDING_NULL && isFree(i)) {
        index = i;
      }
    }
  }
  return index;
}

static bool validateScheme(const EmberZclBindingEntry_t *entry)
{
  switch (entry->destination.network.scheme) {
    case EMBER_ZCL_SCHEME_COAP:
    case EMBER_ZCL_SCHEME_COAPS:
      return true;
    default:
      return false;
  }
}

static bool validateNetworkDestination(const EmberZclBindingEntry_t *entry)
{
  switch (entry->destination.network.type) {
    case EMBER_ZCL_NETWORK_DESTINATION_TYPE_ADDRESS:
    case EMBER_ZCL_NETWORK_DESTINATION_TYPE_UID:
      return true;
    default:
      return false;
  }
}

static bool validateApplicationDestination(const EmberZclBindingEntry_t *entry)
{
  switch (entry->destination.application.type) {
    case EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT:
      if ((entry->destination.application.data.endpointId >= EMBER_ZCL_ENDPOINT_MIN)
          && (entry->destination.application.data.endpointId <= EMBER_ZCL_ENDPOINT_MAX)) {
        return true;
      }
      break;
    case EMBER_ZCL_APPLICATION_DESTINATION_TYPE_GROUP:
      if ((entry->destination.application.data.groupId >= EMBER_ZCL_GROUP_MIN)
          && (entry->destination.application.data.groupId <= EMBER_ZCL_GROUP_MAX)) {
        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

static bool validate(const EmberZclBindingEntry_t *entry)
{
  // emZclEndpointHasCluster verifies the endpoint exists and implements the
  // cluster.  Zero is not a valid UDP port.
  return (entry == NULL
          || (emZclEndpointHasCluster(entry->endpointId, &entry->clusterSpec)
              && validateScheme(entry)
              && validateNetworkDestination(entry)
              && validateApplicationDestination(entry)
              && entry->destination.network.port != 0
              && (entry->reportingConfigurationId == EMBER_ZCL_REPORTING_CONFIGURATION_NULL
                  || emZclHasReportingConfiguration(entry->endpointId,
                                                    &entry->clusterSpec,
                                                    entry->reportingConfigurationId))));
}

static bool set(EmberZclBindingId_t bindingId,
                const EmberZclBindingEntry_t *entry)
{
  if (bindingId < EMBER_ZCL_BINDING_TABLE_SIZE && validate(entry)) {
    EmberZclBindingEntry_t unused = { 0 };
    halCommonSetIndexedToken(TOKEN_ZCL_CORE_BINDING_TABLE,
                             bindingId,
                             (EmberZclBindingId_t *)(entry == NULL
                                                     ? &unused
                                                     : entry));
    return true;
  } else {
    return false;
  }
}

void emZclBindingNetworkStatusHandler(EmberNetworkStatus newNetworkStatus,
                                      EmberNetworkStatus oldNetworkStatus,
                                      EmberJoinFailureReason reason)
{
  // If the device is no longer associated with a network, its bindings are
  // removed, because they point to devices that are no longer accessible.
  if (newNetworkStatus == EMBER_NO_NETWORK) {
    emberZclRemoveAllBindings();
  }
}

bool emberZclHasBinding(EmberZclBindingId_t bindingId)
{
  return emberZclGetBinding(bindingId, NULL);
}

bool emberZclGetBinding(EmberZclBindingId_t bindingId,
                        EmberZclBindingEntry_t *entry)
{
  if (EMBER_ZCL_BINDING_TABLE_SIZE <= bindingId || isFree(bindingId)) {
    return false;
  } else {
    if (entry != NULL) {
      halCommonGetIndexedToken(entry, TOKEN_ZCL_CORE_BINDING_TABLE, bindingId);
    }
    return true;
  }
}

bool emberZclSetBinding(EmberZclBindingId_t bindingId,
                        const EmberZclBindingEntry_t *entry)
{
  EmberZclBindingId_t duplicateId = find(entry, false);   // find duplicate only

  return (duplicateId == bindingId
          || (duplicateId == EMBER_ZCL_BINDING_NULL
              && set(bindingId, entry)));
}

EmberZclBindingId_t emberZclAddBinding(const EmberZclBindingEntry_t *entry)
{
  EmberZclBindingId_t bindingId = find(entry, true); // find duplicate or free
  return (set(bindingId, entry) ? bindingId : EMBER_ZCL_BINDING_NULL);
}

bool emberZclRemoveBinding(EmberZclBindingId_t bindingId)
{
  return emberZclSetBinding(bindingId, NULL);
}

bool emberZclRemoveAllBindings(void)
{
  bool removed = false;
  for (EmberZclBindingId_t i = 0; i < EMBER_ZCL_BINDING_TABLE_SIZE; i++) {
    if (emberZclRemoveBinding(i)) {
      removed = true;
    }
  }
  return removed;
}

EmberStatus emberZclSendAddBinding(const EmberZclDestination_t *destination,
                                   const EmberZclBindingEntry_t *entry,
                                   const EmberZclBindingResponseHandler handler)
{
  return sendBindingCommand(destination,
                            entry,
                            EMBER_ZCL_BINDING_NULL,
                            handler,
                            EMBER_COAP_CODE_POST);
}

EmberStatus emberZclSendUpdateBinding(const EmberZclDestination_t *destination,
                                      const EmberZclBindingEntry_t *entry,
                                      EmberZclBindingId_t bindingId,
                                      const EmberZclBindingResponseHandler handler)
{
  return sendBindingCommand(destination,
                            entry,
                            bindingId,
                            handler,
                            EMBER_COAP_CODE_PUT);
}

EmberStatus emberZclSendRemoveBinding(const EmberZclDestination_t *destination,
                                      const EmberZclClusterSpec_t *clusterSpec,
                                      EmberZclBindingId_t bindingId,
                                      const EmberZclBindingResponseHandler handler)
{
  EmberZclBindingEntry_t entry = {
    .clusterSpec = *clusterSpec,
  };
  return sendBindingCommand(destination,
                            &entry,
                            bindingId,
                            handler,
                            EMBER_COAP_CODE_DELETE);
}

void emZclReadDestinationFromBinding(const EmberZclBindingEntry_t *binding,
                                     EmberZclDestination_t *destination)
{
  destination->application.type = binding->destination.application.type;
  destination->application.data = binding->destination.application.data;
  destination->network.flags =
    (binding->destination.network.scheme == EMBER_ZCL_SCHEME_COAPS
     ? EMBER_ZCL_USE_COAPS_FLAG
     : EMBER_ZCL_NO_FLAGS);
  switch (binding->destination.network.type) {
    case EMBER_ZCL_NETWORK_DESTINATION_TYPE_ADDRESS:
      destination->network.flags |= EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG;
      MEMCOPY(&destination->network.address,
              &binding->destination.network.data.address,
              sizeof(EmberIpv6Address));
      break;
    case EMBER_ZCL_NETWORK_DESTINATION_TYPE_UID:
      destination->network.flags |= EMBER_ZCL_HAVE_UID_FLAG;
      MEMCOPY(&destination->network.uid,
              &binding->destination.network.data.uid,
              sizeof(EmberZclUid_t));
      break;
  }
  destination->network.port = binding->destination.network.port;
}

bool emberZclGetDestinationFromBinding(const EmberZclClusterSpec_t *clusterSpec,
                                       EmberZclBindingId_t *bindingIdx,
                                       EmberZclDestination_t *destination)
{
  EmberZclBindingId_t i;
  EmberZclBindingEntry_t bindingEntry;

  for (i = *bindingIdx; i < EMBER_ZCL_BINDING_TABLE_SIZE; i++) {
    if ((emberZclGetBinding(i, &bindingEntry))
        && (bindingEntry.clusterSpec.id == clusterSpec->id)) {
      emZclReadDestinationFromBinding(&bindingEntry, destination);
      *bindingIdx = i + 1;  // Increment index if binding found.
      return true;
    }
  }

  return false;
}

size_t emZclGetBindingCount(void)
{
  size_t bindingCount = 0;
  for (EmberZclBindingId_t i = 0; i < EMBER_ZCL_BINDING_TABLE_SIZE; i++) {
    if (!isFree(i)) {
      bindingCount++;
    }
  }
  return bindingCount;
}

bool emZclHasBinding(const EmZclContext_t *context,
                     EmberZclBindingId_t bindingId)
{
  EmberZclBindingEntry_t entry = { 0 };
  return (emberZclGetBinding(bindingId, &entry)
          && context->endpoint->endpointId == entry.endpointId
          && emberZclAreClusterSpecsEqual(&context->clusterSpec,
                                          &entry.clusterSpec));
}

// GET .../b
static void getBindingIdsHandler(const EmZclContext_t *context)
{
  CborState state;
  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  emCborEncodeIndefiniteArrayStart(&state, buffer, sizeof(buffer));
  for (EmberZclBindingId_t i = 0; i < EMBER_ZCL_BINDING_TABLE_SIZE; i++) {
    if (emZclHasBinding(context, i)
        && !emCborEncodeValue(&state,
                              EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                              sizeof(i),
                              (const uint8_t *)&i)) {
      emZclRespond500InternalServerError(context->info);
      return;
    }
  }
  if (emCborEncodeBreak(&state)) {
    emZclRespond205ContentCborState(context->info, &state);
  } else {
    emZclRespond500InternalServerError(context->info);
  }
}

// POST .../b
static void addBindingHandler(const EmZclContext_t *context)
{
  EmberZclBindingEntry_t entry = {
    .endpointId = context->endpoint->endpointId,
    .clusterSpec = context->clusterSpec,
    .destination = { { 0 } },
    .reportingConfigurationId = EMBER_ZCL_REPORTING_CONFIGURATION_DEFAULT,
  };
  if (decodeBindingOta(context, &entry)) {
    EmberZclBindingId_t bindingId = emberZclAddBinding(&entry);
    if (bindingId == EMBER_ZCL_BINDING_NULL) {
      // Spec 16-07008-040: 3.9.4 - the device SHALL respond with 5.00 Internal Server Error.
      emZclRespond500InternalServerError(context->info);
    } else {
      EmberZclApplicationDestination_t destination = {
        .data.endpointId = context->endpoint->endpointId,
        .type = EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT,
      };
      uint8_t uriPath[EMBER_ZCL_URI_PATH_MAX_LENGTH];
      emZclBindingIdToUriPath(&destination,
                              &context->clusterSpec,
                              bindingId,
                              uriPath);
      emZclRespond201Created(context->info, uriPath);
    }
  } else {
    // Spec 16-07008-040: 3.9.4 - If the Destination URI is not set the device SHALL respond with 4.00 Bad Request
    emZclRespond400BadRequest(context->info);
  }
}

// GET .../b/B
static void getBindingHandler(const EmZclContext_t *context)
{
  EmberZclBindingEntry_t entry = { 0 };
  emberZclGetBinding(context->bindingId, &entry);

  // emZclDestinationToUri adds a null terminator, so we don't need to add one
  // here.
  Binding_t binding = {
    .reportingConfigurationId = entry.reportingConfigurationId,
    .uri = { 0 },
  };
  EmberZclDestination_t destination;
  emZclReadDestinationFromBinding(&entry, &destination);
  emZclDestinationToUri(&destination, binding.uri);

  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  uint16_t payloadLength = emCborEncodeOneStruct(buffer,
                                                 sizeof(buffer),
                                                 bindingSpec,
                                                 &binding);
  if (payloadLength != 0) {
    emZclRespond205ContentCbor(context->info, buffer, payloadLength);
  } else {
    emZclRespond500InternalServerError(context->info);
  }
}

// PUT .../b/B
static void updateBindingHandler(const EmZclContext_t *context)
{
  EmberZclBindingEntry_t entry = { 0 };
  if (emberZclGetBinding(context->bindingId, &entry)) {
    if (decodeBindingOta(context, &entry)
        && emberZclSetBinding(context->bindingId, &entry)) {
      emZclRespond204ChangedWithStatus(context->info, EMBER_ZCL_STATUS_SUCCESS);
    } else {
      // Spec 16-07008-071: 3.10.4: If a match exists respond with 4.00 and code DUPLICATE_EXISTS.
      emZclRespond400BadRequestWithStatus(context->info, EMBER_ZCL_STATUS_DUPLICATE_EXISTS);
    }
  } else {
    emZclRespond500InternalServerError(context->info);
  }
}

// DELETE .../b/B
static void removeBindingHandler(const EmZclContext_t *context)
{
  emberZclRemoveBinding(context->bindingId);
  emZclRespond202Deleted(context->info);
}

static bool decodeBindingOta(const EmZclContext_t *context,
                             EmberZclBindingEntry_t *entry)
{
  Binding_t binding = {
    .reportingConfigurationId = EMBER_ZCL_REPORTING_CONFIGURATION_DEFAULT,
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
  return (emZclUriToBindingEntry(binding.uri, entry, false)
          && validate(entry));
}

static EmberStatus sendBindingCommand(const EmberZclDestination_t *destination,
                                      const EmberZclBindingEntry_t *entry,
                                      EmberZclBindingId_t bindingId,
                                      const EmberZclBindingResponseHandler handler,
                                      EmberCoapCode code)
{
  assert(code == EMBER_COAP_CODE_POST        // add binding
         || code == EMBER_COAP_CODE_PUT      // update binding
         || code == EMBER_COAP_CODE_DELETE); // delete binding

  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  uint16_t payloadLength = 0;
  if (code != EMBER_COAP_CODE_DELETE) {
    Binding_t binding = {
      .reportingConfigurationId = entry->reportingConfigurationId,
      .uri = { 0 },
    };
    // emZclDestinationToUri adds a null terminator, so we don't need to add
    // one here.
    EmberZclDestination_t temp;
    emZclReadDestinationFromBinding(entry, &temp);
    emZclDestinationToUri(&temp, binding.uri);
    binding.reportingConfigurationId = entry->reportingConfigurationId;
    payloadLength = emCborEncodeOneStruct(buffer,
                                          sizeof(buffer),
                                          bindingSpec,
                                          &binding);
  }

  uint8_t uriPath[EMBER_ZCL_URI_PATH_MAX_LENGTH];
  if (bindingId == EMBER_ZCL_BINDING_NULL) {
    emZclBindingToUriPath(&destination->application,
                          &entry->clusterSpec,
                          uriPath);
  } else {
    emZclBindingIdToUriPath(&destination->application,
                            &entry->clusterSpec,
                            bindingId,
                            uriPath);
  }

  Response response = {
    .context = {
      .code = EMBER_COAP_CODE_EMPTY, // filled in after response
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
      .clusterSpec = &entry->clusterSpec,
      .bindingId = bindingId,
    },
    .entry = *entry,
    .handler = handler,
  };

  return emZclSend(&destination->network,
                   code,
                   uriPath,
                   buffer,
                   payloadLength,
                   (handler == NULL ? NULL : responseHandler),
                   &response,
                   sizeof(Response),
                   false);
}

static void responseHandler(EmberCoapStatus status,
                            EmberCoapCode code,
                            EmberCoapReadOptions *options,
                            uint8_t *payload,
                            uint16_t payloadLength,
                            EmberCoapResponseInfo *info)
{
  // We should only be here if the application specified a handler.
  assert(info->applicationDataLength == sizeof(Response));
  const Response *response = info->applicationData;
  assert(*response->handler != NULL);

  emZclCoapStatusHandler(status, info);
  ((Response *)response)->context.code = code;

  // TODO: What should happen for failures?
  // TODO: How do we know what the binding ID is in the received URI?
  // TODO: When creating a new binding, populate response->context.bindingId
  // with the new id.
  (*response->handler)((EmberZclMessageStatus_t) status,
                       &response->context,
                       &response->entry);
}

// zcl/e/XX/<cluster>/b:
//   GET: return list of binding ids.
//   POST: add binding.
//   OTHER: not allowed.
void emZclUriClusterBindingHandler(EmZclContext_t *context)
{
  if (context->groupId != EMBER_ZCL_GROUP_NULL) {
    emZclRespond404NotFound(context->info);
  } else if (context->code == EMBER_COAP_CODE_GET) {
    getBindingIdsHandler(context);
  } else if (context->code == EMBER_COAP_CODE_POST) {
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
void emZclUriClusterBindingIdHandler(EmZclContext_t *context)
{
  if (context->groupId != EMBER_ZCL_GROUP_NULL) {
    emZclRespond404NotFound(context->info);
  } else if (context->code == EMBER_COAP_CODE_GET) {
    getBindingHandler(context);
  } else if (context->code == EMBER_COAP_CODE_PUT) {
    updateBindingHandler(context);
  } else if (context->code == EMBER_COAP_CODE_DELETE) {
    removeBindingHandler(context);
  } else {
    assert(false);
  }
}
