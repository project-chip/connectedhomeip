/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_ZCL_CORE

const ChZclEndpointEntry_t *chZclFindEndpoint(ChipZclEndpointId_t endpointId)
{
  ChipZclEndpointIndex_t index = chipZclEndpointIdToIndex(endpointId, NULL);
  return (index == CHIP_ZCL_ENDPOINT_INDEX_NULL
          ? NULL
          : &chZclEndpointTable[index]);
}

bool chZclEndpointHasCluster(ChipZclEndpointId_t endpointId,
                             const ChipZclClusterSpec_t *clusterSpec)
{
  const ChZclEndpointEntry_t *endpoint = chZclFindEndpoint(endpointId);
  if (endpoint != NULL) {
    for (size_t i = 0; endpoint->clusterSpecs[i] != NULL; i++) {
      int32_t compare
        = chipZclCompareClusterSpec(endpoint->clusterSpecs[i], clusterSpec);
      if (compare > 0) {
        break;
      } else if (compare == 0) {
        return true;
      }
    }
  }
  return false;
}

ChipZclStatus_t chZclMultiEndpointDispatch(const ChZclContext_t *context,
                                            ChZclMultiEndpointHandler handler,
                                            CborState *state,
                                            void *data)
{
  const ChipIpv6Address *dstAddr = ((data == NULL) || (((ChipZclCommandContext_t *)data)->info == NULL)
                                     ? NULL
                                     : &(((ChipZclCommandContext_t *)data)->info->localAddress));

  if (context->groupId == CHIP_ZCL_GROUP_NULL) {
    return (*handler)(context, state, data);
  } else if (!emCborEncodeIndefiniteMap(state)) {
    return CHIP_ZCL_STATUS_FAILURE;
  } else {
    ChipZclStatus_t status = CHIP_ZCL_STATUS_NOT_FOUND;
    uint8_t *savedFinger;
    for (size_t i = 0; i < chZclEndpointCount; i++) {
      if (chipZclIsEndpointAndAddrInGroup(chZclEndpointTable[i].endpointId,
                                           dstAddr,
                                           context->groupId)
          && chZclEndpointHasCluster(chZclEndpointTable[i].endpointId,
                                     &context->clusterSpec)) {
        ((ChZclContext_t *)context)->endpoint = &chZclEndpointTable[i];

        if (!emCborEncodeValue(state,
                               CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER,
                               sizeof(chZclEndpointTable[i].endpointId),
                               (const uint8_t *)&chZclEndpointTable[i].endpointId)) {
          return CHIP_ZCL_STATUS_FAILURE;
        }

        savedFinger = state->finger;

        status = (*handler)(context, state, data);
        if (status == CHIP_ZCL_STATUS_FAILURE) {
          return CHIP_ZCL_STATUS_FAILURE;
        }

        // If command execution results in empty response the corresponding endpoint entry should not
        // be present in the map (BDB Spec, Section 3.19.2). Remove endpoint key added above if the CBOR
        // payload has not changed in handler() invocation
        if (state->finger == savedFinger) {
          // Undo the emCborEncodeValue() call from above
          emCborEraseUnsignedInt(state, sizeof(chZclEndpointTable[i].endpointId));
        }
      }
    }
    if (emCborEncodeBreak(state)) {
      return status;
    }
  }
  return CHIP_ZCL_STATUS_FAILURE;
}

ChipZclEndpointIndex_t chipZclEndpointIdToIndex(ChipZclEndpointId_t endpointId,
                                                  const ChipZclClusterSpec_t *clusterSpec)
{
  ChipZclEndpointIndex_t index = 0;
  for (size_t i = 0;
       (i < chZclEndpointCount
        && chZclEndpointTable[i].endpointId <= endpointId);
       i++) {
    if (clusterSpec == NULL
        || chZclEndpointHasCluster(chZclEndpointTable[i].endpointId,
                                   clusterSpec)) {
      if (endpointId == chZclEndpointTable[i].endpointId) {
        return index;
      } else {
        index++;
      }
    }
  }
  return CHIP_ZCL_ENDPOINT_INDEX_NULL;
}

ChipZclEndpointId_t chipZclEndpointIndexToId(ChipZclEndpointIndex_t index,
                                               const ChipZclClusterSpec_t *clusterSpec)
{
  for (size_t i = 0; i < chZclEndpointCount; i++) {
    if (clusterSpec == NULL
        || chZclEndpointHasCluster(chZclEndpointTable[i].endpointId,
                                   clusterSpec)) {
      if (index == 0) {
        return chZclEndpointTable[i].endpointId;
      } else {
        index--;
      }
    }
  }
  return CHIP_ZCL_ENDPOINT_NULL;
}

// zcl/e:
//   GET: list endpoints.
//   OTHER: not allowed.
void chZclUriEndpointHandler(ChZclContext_t *context)
{
  CborState state;
  uint8_t buffer[CH_ZCL_MAX_PAYLOAD_SIZE];
  emCborEncodeIndefiniteArrayStart(&state, buffer, sizeof(buffer));
  for (size_t i = 0; i < chZclEndpointCount; i++) {
    if (!emCborEncodeValue(&state,
                           CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER,
                           sizeof(ChipZclEndpointId_t),
                           &chZclEndpointTable[i].endpointId)) {
      chZclRespond500InternalServerError(context->info);
      return;
    }
  }
  emCborEncodeBreak(&state);
  chZclRespond205ContentCborState(context->info, &state);
}

// zcl/e/XX:
//   GET: list clusters on endpoint.
//   OTHER: not allowed.
void chZclUriEndpointIdHandler(ChZclContext_t *context)
{
  CborState state;
  uint8_t buffer[CH_ZCL_MAX_PAYLOAD_SIZE];
  emCborEncodeIndefiniteArrayStart(&state, buffer, sizeof(buffer));
  for (size_t i = 0; context->endpoint->clusterSpecs[i] != NULL; i++) {
    uint8_t clusterId[CHIP_ZCL_URI_PATH_CLUSTER_ID_MAX_LENGTH];
    chZclClusterToString(context->endpoint->clusterSpecs[i], clusterId);
    if (!emCborEncodeValue(&state,
                           CHIP_ZCLIP_TYPE_MAX_LENGTH_STRING,
                           sizeof(clusterId),
                           clusterId)) {
      chZclRespond500InternalServerError(context->info);
      return;
    }
  }
  emCborEncodeBreak(&state);
  chZclRespond205ContentCborState(context->info, &state);
}
