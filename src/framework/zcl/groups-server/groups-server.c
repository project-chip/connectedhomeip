/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_HAL

#define isMulticastAddress(ipAddress) ((ipAddress)[0] == 0xFF)

bool chZclGroupsServerPreAttributeChangeHandler(ChipZclEndpointId_t endpointId,
                                                const ChipZclClusterSpec_t *clusterSpec,
                                                ChipZclAttributeId_t attributeId,
                                                const void *buffer,
                                                size_t bufferLength)
{
  return (!chipZclAreClusterSpecsEqual(&chipZclClusterGroupsServerSpec,
                                        clusterSpec)
          || *(const uint8_t *)buffer == 0);
}

static ChipZclStatus_t sanityCheckAddressParameters(int8u addrAssignmentMode, uint8_t *addrPtr, uint32_t addrLength)
{
  // For manual assignment mode the address parameter must be present and must carry a full IPv6 address
  if (addrAssignmentMode == CHIP_ZCL_GROUP_ASSIGNMENT_MODE_MANUAL  && addrLength != 16) {
    return CHIP_ZCL_STATUS_FAILURE;
  }

  // For auto assignment mode the address parameter must be either absent or be a single byte
  if ((CHIP_ZCL_GROUP_ASSIGNMENT_MODE_AUTO_MIN <= addrAssignmentMode)
      && (addrAssignmentMode <= CHIP_ZCL_GROUP_ASSIGNMENT_MODE_AUTO_MAX)
      && (addrLength != 1)
      && (addrLength != 0)) {
    return CHIP_ZCL_STATUS_FAILURE;
  }

  // Check against manually-assigned address not being multicast, check against invalid mode.
  if ((addrAssignmentMode == CHIP_ZCL_GROUP_ASSIGNMENT_MODE_MANUAL  && addrLength != 0 && addrPtr != NULL && !emIsMulticastAddress(addrPtr))
      || (addrAssignmentMode > CHIP_ZCL_GROUP_ASSIGNMENT_MODE_AUTO_MAX && addrAssignmentMode != CHIP_ZCL_GROUP_ASSIGNMENT_MODE_MANUAL)) {
    return CHIP_ZCL_STATUS_FAILURE;
  }

  return CHIP_ZCL_STATUS_SUCCESS;
}

void chipZclClusterGroupsServerCommandAddGroupRequestHandler(const ChipZclCommandContext_t *context,
                                                              const ChipZclClusterGroupsServerCommandAddGroupRequest_t *request)
{
  chipAfCorePrintln("RX: AddGroup");
  uint8_t assignmentMode = CHIP_ZCL_GROUP_ASSIGNMENT_MODE_NULL;
  ChipZclClusterGroupsServerCommandAddGroupResponse_t response = { 0 };
  ChZclGroupNameSupportMask_t nameSupport;

  // Check that address parameters are valid
  if (sanityCheckAddressParameters(request->addrAssignmentMode,
                                   request->groupMcastAddress.ptr,
                                   request->groupMcastAddress.length) != CHIP_ZCL_STATUS_SUCCESS) {
    response.status  = CHIP_ZCL_STATUS_FAILURE;
    goto send_response;
  }

  // CBOR decoder sets missing parameters to all zeros, if address length is zero  we conclude
  // that both parameters are missing and use default values, we'll track this by setting mode to 0xff.
  // Revisit this logic when proper handling of optional parameters is implemented.
  if (request->groupMcastAddress.length == 0 || request->groupMcastAddress.ptr == NULL) {
    assignmentMode = CHIP_ZCL_GROUP_ASSIGNMENT_MODE_NULL;
  } else {
    assignmentMode = request->addrAssignmentMode;
  }

  if ((chipZclReadAttribute(context->endpointId,
                             &chipZclClusterGroupsServerSpec,
                             CHIP_ZCL_CLUSTER_GROUPS_SERVER_ATTRIBUTE_GROUP_NAME_SUPPORT,
                             &nameSupport,
                             sizeof(nameSupport))
       == CHIP_ZCL_STATUS_SUCCESS)
      && nameSupport & CH_ZCL_GROUP_NAME_SUPPORT_ENABLED) {
    response.status = chipZclAddEndpointToGroup(context->endpointId,
                                                 request->groupId,
                                                 request->groupName.ptr,
                                                 request->groupName.length,
                                                 assignmentMode,
                                                 request->groupMcastAddress.ptr,
                                                 request->groupUdpPort);
  } else {
    // ignore the groupName if support is not enabled
    response.status = chipZclAddEndpointToGroup(context->endpointId,
                                                 request->groupId,
                                                 NULL,
                                                 0,
                                                 assignmentMode,
                                                 request->groupMcastAddress.ptr,
                                                 request->groupUdpPort);
  }

  send_response:

  if (isMulticastAddress(context->info->localAddress.bytes)) {
    ((ChipZclCommandContext_t *)context)->result = CHIP_NO_BUFFERS; // command response will be suppressed
  } else {
    response.groupId = request->groupId;
    chipZclSendClusterGroupsServerCommandAddGroupResponse(context, &response);
  }
}

void chipZclClusterGroupsServerCommandViewGroupRequestHandler(const ChipZclCommandContext_t *context,
                                                               const ChipZclClusterGroupsServerCommandViewGroupRequest_t *request)
{
  chipAfCorePrintln("RX: ViewGroup");

  ChipZclClusterGroupsServerCommandViewGroupResponse_t response = { 0 };
  uint8_t emptyString[] = { 0 };
  // Group name "broadcast", this is a ZCL string, not NULL-terminated
  uint8_t broadcastGroupName[] = { 0x62, 0x72, 0x6f, 0x61, 0x64, 0x63, 0x61, 0x73, 0x74 };
  ChZclGroupNameSupportMask_t nameSupport = 0;
  size_t                      i = 0;
  ChipZclGroupEntry_t        entry;

  response.groupId = request->groupId;

  // Start with invalid/empty values
  response.groupName.ptr = emptyString;
  response.groupName.length = 0;
  response.addrAssignmentMode = 0xff;
  response.groupMcastAddress.ptr = NULL;
  response.groupMcastAddress.length = 0;
  response.groupUdpPort = 0xffff;

  if ((request->groupId < CHIP_ZCL_GROUP_MIN)
      || isMulticastAddress(context->info->localAddress.bytes)) {
    response.status = CHIP_ZCL_STATUS_NOT_FOUND;
    goto send_response;
  }

  chipZclReadAttribute(context->endpointId,
                        &chipZclClusterGroupsServerSpec,
                        CHIP_ZCL_CLUSTER_GROUPS_SERVER_ATTRIBUTE_GROUP_NAME_SUPPORT,
                        &nameSupport,
                        sizeof(nameSupport));

  for (i = 0; i < CHIP_ZCL_GROUP_TABLE_SIZE; i++) {
    halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
    if ((request->groupId == CHIP_ZCL_GROUP_ALL_ENDPOINTS)
        || (request->groupId == entry.groupId && context->endpointId == entry.endpointId)) {
      break;
    }
  }

  // Group not found
  if (i == CHIP_ZCL_GROUP_TABLE_SIZE) {
    response.status = CHIP_ZCL_STATUS_NOT_FOUND;
    goto send_response;
  }

  // Broadcast group uses hardcoded values
  if (request->groupId == CHIP_ZCL_GROUP_ALL_ENDPOINTS) {
    if (nameSupport & CH_ZCL_GROUP_NAME_SUPPORT_ENABLED) {
      response.groupName.ptr    = broadcastGroupName;
      response.groupName.length = sizeof(broadcastGroupName);
    }

    // ZCLIP Base Device Spec v0.66 does not specify what address and port to send here, assume
    // that we should omit these parameters. Values for addrAssignmentMode, groupMcastAddress and
    // groupUdpPort  are out of range for these ZCLIP types, this will cause CBOR encoder to
    // omit these entries from the payload map.
    // Revisit when proper handling of optional parameters is implemented.

    response.status = CHIP_ZCL_STATUS_SUCCESS;
  } else {
    if (nameSupport & CH_ZCL_GROUP_NAME_SUPPORT_ENABLED) {
#if CHIP_ZCL_MAX_GROUP_NAME_LENGTH == 0
#else
      response.groupName.ptr = entry.groupName;
      response.groupName.length = entry.groupNameLength;
#endif
    }

    if (entry.addrAssignmentMode != CHIP_ZCL_GROUP_ASSIGNMENT_MODE_NULL) {
      response.addrAssignmentMode = entry.addrAssignmentMode;
      response.groupMcastAddress.ptr = entry.groupMcastAddress.bytes;
      response.groupMcastAddress.length = (entry.addrAssignmentMode == CHIP_ZCL_GROUP_ASSIGNMENT_MODE_MANUAL ? 16 : 1);
      response.groupUdpPort = entry.groupUdpPort ? entry.groupUdpPort : CHIP_COAP_PORT; // Return what we actually use
    } else {
      // No address was assigned to this group. Return values for addrAssignmentMode, groupMcastAddress and
      // groupUdpPort that are out of range for these ZCLIP types, this will cause CBOR encoder to omit these
      // entries from the payload map.
      // Revisit when proper handling of optional parameters is implemented.
    }

    response.status = CHIP_ZCL_STATUS_SUCCESS;
  }

  send_response:
  if (isMulticastAddress(context->info->localAddress.bytes)) {
    ((ChipZclCommandContext_t *)context)->result = CHIP_NO_BUFFERS;     // command response will be suppressed
  } else {
    chipZclSendClusterGroupsServerCommandViewGroupResponse(context, &response);
  }
}

void chipZclClusterGroupsServerCommandGetGroupMchipshipRequestHandler(const ChipZclCommandContext_t *context,
                                                                        const ChipZclClusterGroupsServerCommandGetGroupMchipshipRequest_t *request)
{
  chipAfCorePrintln("RX: GetGroupMchipship");

  // Set buffer size for groupList buffers (request & response).
  #define GROUP_LIST_BUFFER_SIZE 40

  uint16_t requestGroupListBuffer[GROUP_LIST_BUFFER_SIZE];

  // Decode the request groupList (cbor array) into a buffer.
  uint16_t requestGroupListCount =
    emCborDecodeFieldArrayIntoBuffer((CborArray *)&request->groupList,
                                     (uint8_t *)requestGroupListBuffer,
                                     sizeof(requestGroupListBuffer),
                                     sizeof(requestGroupListBuffer[0]));

  ChipZclClusterGroupsServerCommandGetGroupMchipshipResponse_t response = { 0 };

  response.capacity = chZclGetGroupsCapacity();

  // Construct response groupList in a buffer. Sized +1 for broadcast group.
  ChipZclGroupId_t responseGroupListBuffer[CHIP_ZCL_GROUP_TABLE_SIZE + 1];
  uint16_t responseGroupListCount = 0;

  if (requestGroupListCount != 0) {
    // Match on the supplied GroupList.
    for (size_t i = 0; i < requestGroupListCount; i++) {
      ChipZclGroupId_t groupId = requestGroupListBuffer[i];
      if (chipZclIsEndpointInGroup(context->endpointId, groupId)) {
        chZclInsertGroupIdIntoSortedList(groupId,
                                         responseGroupListBuffer,
                                         &responseGroupListCount,
                                         COUNTOF(responseGroupListBuffer));
      }
    }
  } else {
    // Supplied GroupList is empty. Match on any group of which the
    // endpoint is a mchip, including Broadcast Group.
    for (size_t i = 0; i < CHIP_ZCL_GROUP_TABLE_SIZE; i++) {
      ChipZclGroupEntry_t entry;
      halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
      ChipZclGroupId_t groupId = entry.groupId;
      if (groupId != CHIP_ZCL_GROUP_NULL
          && context->endpointId == entry.endpointId) {
        chZclInsertGroupIdIntoSortedList(groupId,
                                         responseGroupListBuffer,
                                         &responseGroupListCount,
                                         COUNTOF(responseGroupListBuffer));
      }
    }
    // Broadcast Group is 0xffff, last in the array.
    responseGroupListBuffer[responseGroupListCount] = CHIP_ZCL_GROUP_ALL_ENDPOINTS;
    responseGroupListCount++;
  }

  // Set the response.
  response.groupList.ptr = (uint8_t *)responseGroupListBuffer;
  response.groupList.numElementsToEncode = responseGroupListCount;
  response.groupList.fieldData = request->groupList.fieldData; // (response fieldData is same as for request).

  if ((responseGroupListCount == 0)
      && (isMulticastAddress(context->info->localAddress.bytes))) {
    ((ChipZclCommandContext_t *)context)->result = CHIP_NO_BUFFERS; // command response will be suppressed
  } else {
    chipZclSendClusterGroupsServerCommandGetGroupMchipshipResponse(context,
                                                                     &response);
  }
}

void chipZclClusterGroupsServerCommandRemoveGroupRequestHandler(const ChipZclCommandContext_t *context,
                                                                 const ChipZclClusterGroupsServerCommandRemoveGroupRequest_t *request)
{
  chipAfCorePrintln("RX: RemoveGroup");

  ChipZclClusterGroupsServerCommandRemoveGroupResponse_t response = { 0 };
  response.status = chipZclRemoveEndpointFromGroup(context->endpointId,
                                                    request->groupId);
  response.groupId = request->groupId;

  if (isMulticastAddress(context->info->localAddress.bytes)) {
    ((ChipZclCommandContext_t *)context)->result = CHIP_NO_BUFFERS; // command response will be suppressed
  } else {
    chipZclSendClusterGroupsServerCommandRemoveGroupResponse(context, &response);
  }
}

void chipZclClusterGroupsServerCommandRemoveAllGroupsRequestHandler(const ChipZclCommandContext_t *context,
                                                                     const ChipZclClusterGroupsServerCommandRemoveAllGroupsRequest_t *request)
{
  chipAfCorePrintln("RX: RemoveAllGroups");

  chipZclRemoveEndpointFromAllGroups(context->endpointId);

  if (isMulticastAddress(context->info->localAddress.bytes)) {
    ((ChipZclCommandContext_t *)context)->result = CHIP_NO_BUFFERS; // command response will be suppressed
  } else {
    chZclRespond204Changed(context->info);   // Only 2.04 or 5.00 is allowed, 5.00 is not suitable here.
  }
}

void chipZclClusterGroupsServerCommandAddGroupIfIdentifyingRequestHandler(const ChipZclCommandContext_t *context,
                                                                           const ChipZclClusterGroupsServerCommandAddGroupIfIdentifyingRequest_t *request)
{
  chipAfCorePrintln("RX: AddGroupIfIdentifying");
  uint8_t assignmentMode = CHIP_ZCL_GROUP_ASSIGNMENT_MODE_NULL;
  ChipZclClusterGroupsServerCommandAddGroupResponse_t response = { 0 };
  uint16_t identifyTimeS;
  ChZclGroupNameSupportMask_t nameSupport;

  // Check that address parameters are valid
  if (sanityCheckAddressParameters(request->addrAssignmentMode,
                                   request->groupMcastAddress.ptr,
                                   request->groupMcastAddress.length) != CHIP_ZCL_STATUS_SUCCESS) {
    response.status  = CHIP_ZCL_STATUS_FAILURE;
    goto send_response;
  }

  // Determine if we should skip IPv6 address assignment to this group. We'll track this by setting mode to 0xff.
  // CBOR decoder sets missing parameters to all zeros, if address length is zero we conclude that all address-related
  // parameters are missing.
  // Revisit this logic when proper handling of optional parameters is implemented.
  if (request->groupMcastAddress.length == 0 || request->groupMcastAddress.ptr == NULL) {
    assignmentMode = CHIP_ZCL_GROUP_ASSIGNMENT_MODE_NULL;
  } else {
    assignmentMode = request->addrAssignmentMode;
  }

  ChipZclStatus_t status
    = chipZclReadAttribute(context->endpointId,
                            &chipZclClusterIdentifyServerSpec,
                            CHIP_ZCL_CLUSTER_IDENTIFY_SERVER_ATTRIBUTE_IDENTIFY_TIME,
                            &identifyTimeS,
                            sizeof(identifyTimeS));

  if ((status == CHIP_ZCL_STATUS_SUCCESS)
      && (identifyTimeS != 0)) {
    if ((chipZclReadAttribute(context->endpointId,
                               &chipZclClusterGroupsServerSpec,
                               CHIP_ZCL_CLUSTER_GROUPS_SERVER_ATTRIBUTE_GROUP_NAME_SUPPORT,
                               &nameSupport,
                               sizeof(nameSupport))
         == CHIP_ZCL_STATUS_SUCCESS)
        && nameSupport & CH_ZCL_GROUP_NAME_SUPPORT_ENABLED) {
      response.status = chipZclAddEndpointToGroup(context->endpointId,
                                                   request->groupId,
                                                   request->groupName.ptr,
                                                   request->groupName.length,
                                                   assignmentMode,
                                                   request->groupMcastAddress.ptr,
                                                   request->groupUdpPort);
    } else {
      // ignore the groupName if support is not enabled
      response.status = chipZclAddEndpointToGroup(context->endpointId,
                                                   request->groupId,
                                                   NULL,
                                                   0,
                                                   assignmentMode,
                                                   request->groupMcastAddress.ptr,
                                                   request->groupUdpPort);
    }
  } else {
    // Device is not identifying so send back failure without attempting to add
    // endpoint to group.
    response.status = CHIP_ZCL_STATUS_FAILURE;
  }

  send_response:

  if (isMulticastAddress(context->info->localAddress.bytes)) {
    ((ChipZclCommandContext_t *)context)->result = CHIP_NO_BUFFERS; // command response will be suppressed
  } else {
    response.groupId = request->groupId;
    chipZclSendClusterGroupsServerCommandAddGroupResponse(context, &response);
  }
}
