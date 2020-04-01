/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_ZCL_CORE

const EmZclEndpointEntry_t *emZclFindEndpoint(EmberZclEndpointId_t endpointId)
{
  EmberZclEndpointIndex_t index = emberZclEndpointIdToIndex(endpointId, NULL);
  return (index == EMBER_ZCL_ENDPOINT_INDEX_NULL
          ? NULL
          : &emZclEndpointTable[index]);
}

bool emZclEndpointHasCluster(EmberZclEndpointId_t endpointId,
                             const EmberZclClusterSpec_t *clusterSpec)
{
  const EmZclEndpointEntry_t *endpoint = emZclFindEndpoint(endpointId);
  if (endpoint != NULL) {
    for (size_t i = 0; endpoint->clusterSpecs[i] != NULL; i++) {
      int32_t compare
        = emberZclCompareClusterSpec(endpoint->clusterSpecs[i], clusterSpec);
      if (compare > 0) {
        break;
      } else if (compare == 0) {
        return true;
      }
    }
  }
  return false;
}

EmberZclStatus_t emZclMultiEndpointDispatch(const EmZclContext_t *context,
                                            EmZclMultiEndpointHandler handler,
                                            CborState *state,
                                            void *data)
{
  const EmberIpv6Address *dstAddr = ((data == NULL) || (((EmberZclCommandContext_t *)data)->info == NULL)
                                     ? NULL
                                     : &(((EmberZclCommandContext_t *)data)->info->localAddress));

  if (context->groupId == EMBER_ZCL_GROUP_NULL) {
    return (*handler)(context, state, data);
  } else if (!emCborEncodeIndefiniteMap(state)) {
    return EMBER_ZCL_STATUS_FAILURE;
  } else {
    EmberZclStatus_t status = EMBER_ZCL_STATUS_NOT_FOUND;
    uint8_t *savedFinger;
    for (size_t i = 0; i < emZclEndpointCount; i++) {
      if (emberZclIsEndpointAndAddrInGroup(emZclEndpointTable[i].endpointId,
                                           dstAddr,
                                           context->groupId)
          && emZclEndpointHasCluster(emZclEndpointTable[i].endpointId,
                                     &context->clusterSpec)) {
        ((EmZclContext_t *)context)->endpoint = &emZclEndpointTable[i];

        if (!emCborEncodeValue(state,
                               EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                               sizeof(emZclEndpointTable[i].endpointId),
                               (const uint8_t *)&emZclEndpointTable[i].endpointId)) {
          return EMBER_ZCL_STATUS_FAILURE;
        }

        savedFinger = state->finger;

        status = (*handler)(context, state, data);
        if (status == EMBER_ZCL_STATUS_FAILURE) {
          return EMBER_ZCL_STATUS_FAILURE;
        }

        // If command execution results in empty response the corresponding endpoint entry should not
        // be present in the map (BDB Spec, Section 3.19.2). Remove endpoint key added above if the CBOR
        // payload has not changed in handler() invocation
        if (state->finger == savedFinger) {
          // Undo the emCborEncodeValue() call from above
          emCborEraseUnsignedInt(state, sizeof(emZclEndpointTable[i].endpointId));
        }
      }
    }
    if (emCborEncodeBreak(state)) {
      return status;
    }
  }
  return EMBER_ZCL_STATUS_FAILURE;
}

EmberZclEndpointIndex_t emberZclEndpointIdToIndex(EmberZclEndpointId_t endpointId,
                                                  const EmberZclClusterSpec_t *clusterSpec)
{
  EmberZclEndpointIndex_t index = 0;
  for (size_t i = 0;
       (i < emZclEndpointCount
        && emZclEndpointTable[i].endpointId <= endpointId);
       i++) {
    if (clusterSpec == NULL
        || emZclEndpointHasCluster(emZclEndpointTable[i].endpointId,
                                   clusterSpec)) {
      if (endpointId == emZclEndpointTable[i].endpointId) {
        return index;
      } else {
        index++;
      }
    }
  }
  return EMBER_ZCL_ENDPOINT_INDEX_NULL;
}

EmberZclEndpointId_t emberZclEndpointIndexToId(EmberZclEndpointIndex_t index,
                                               const EmberZclClusterSpec_t *clusterSpec)
{
  for (size_t i = 0; i < emZclEndpointCount; i++) {
    if (clusterSpec == NULL
        || emZclEndpointHasCluster(emZclEndpointTable[i].endpointId,
                                   clusterSpec)) {
      if (index == 0) {
        return emZclEndpointTable[i].endpointId;
      } else {
        index--;
      }
    }
  }
  return EMBER_ZCL_ENDPOINT_NULL;
}

// zcl/e:
//   GET: list endpoints.
//   OTHER: not allowed.
void emZclUriEndpointHandler(EmZclContext_t *context)
{
  CborState state;
  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  emCborEncodeIndefiniteArrayStart(&state, buffer, sizeof(buffer));
  for (size_t i = 0; i < emZclEndpointCount; i++) {
    if (!emCborEncodeValue(&state,
                           EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                           sizeof(EmberZclEndpointId_t),
                           &emZclEndpointTable[i].endpointId)) {
      emZclRespond500InternalServerError(context->info);
      return;
    }
  }
  emCborEncodeBreak(&state);
  emZclRespond205ContentCborState(context->info, &state);
}

// zcl/e/XX:
//   GET: list clusters on endpoint.
//   OTHER: not allowed.
void emZclUriEndpointIdHandler(EmZclContext_t *context)
{
  CborState state;
  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  emCborEncodeIndefiniteArrayStart(&state, buffer, sizeof(buffer));
  for (size_t i = 0; context->endpoint->clusterSpecs[i] != NULL; i++) {
    uint8_t clusterId[EMBER_ZCL_URI_PATH_CLUSTER_ID_MAX_LENGTH];
    emZclClusterToString(context->endpoint->clusterSpecs[i], clusterId);
    if (!emCborEncodeValue(&state,
                           EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING,
                           sizeof(clusterId),
                           clusterId)) {
      emZclRespond500InternalServerError(context->info);
      return;
    }
  }
  emCborEncodeBreak(&state);
  emZclRespond205ContentCborState(context->info, &state);
}
