/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#include CHIP_AF_API_HAL
#include CHIP_AF_API_ZCL_CORE
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#else
  #define chipAfPluginZclCorePrint(...)
  #define chipAfPluginZclCorePrintln(...)
  #define chipAfPluginZclCoreFlush()
  #define chipAfPluginZclCoreDebugExec(x)
  #define chipAfPluginZclCorePrintBuffer(buffer, len, withSpace)
  #define chipAfPluginZclCorePrintString(buffer)
#endif

// Group name "broadcast", this is a ZCL string, not NULL-terminated
const uint8_t bcastGroupName[] = { 0x62, 0x72, 0x6f, 0x61, 0x64, 0x63, 0x61, 0x73, 0x74 };
const uint8_t bcastGroupNameLength = 0x09;
const ChipIpv6Address broadcastGroupAddress  = { { 0xff, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xc1, 0xff, 0xff } };
const ChipIpv6Address allCoapNodesLinkScope  = { { 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd } };
const ChipIpv6Address allCoapNodesRealmScope = { { 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd } };
const ChipIpv6Address allCoapNodesAdminScope = { { 0xff, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd } };
const ChipIpv6Address allCoapNodesSiteScope  = { { 0xff, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd } };
const ChipIpv6Address allNodes               = { { 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } };

void chZclGroupNetworkStatusHandler(ChipNetworkStatus newNetworkStatus,
                                    ChipNetworkStatus oldNetworkStatus,
                                    ChipJoinFailureReason reason)
{
  // If the device is no longer associated with a network, its groups are
  // removed, because groups are specific to a network.
  if (newNetworkStatus == CHIP_NO_NETWORK) {
    chipZclRemoveAllGroups();
  }
}

bool chZclHasGroup(ChipZclGroupId_t groupId)
{
  for (size_t i = 0; i < chZclEndpointCount; i++) {
    if (chipZclIsEndpointInGroup(chZclEndpointTable[i].endpointId, groupId)) {
      return true;
    }
  }
  return false;
}

size_t chZclGetGroupsCapacity(void)
{
  // Returns the remaining (unused) capacity in the Groups table.

  size_t capacity = CHIP_ZCL_GROUP_TABLE_SIZE;

  for (size_t i = 0; i < CHIP_ZCL_GROUP_TABLE_SIZE; i++) {
    ChipZclGroupEntry_t entry;
    halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
    if (entry.groupId != CHIP_ZCL_GROUP_NULL) {
      --capacity;
    }
  }
  return capacity;
}

bool chipZclIsEndpointInGroup(ChipZclEndpointId_t endpointId,
                               ChipZclGroupId_t groupId)
{
  if (groupId == CHIP_ZCL_GROUP_ALL_ENDPOINTS) {
    return true;
  }
  if ((CHIP_ZCL_GROUP_MIN <= groupId)
      && (groupId <= CHIP_ZCL_GROUP_MAX)) {
    for (size_t i = 0; i < CHIP_ZCL_GROUP_TABLE_SIZE; i++) {
      ChipZclGroupEntry_t entry;
      halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
      if (groupId == entry.groupId
          && endpointId == entry.endpointId) {
        return true;
      }
    }
  }

  return false;
}

bool chipZclIsEndpointAndAddrInGroup(ChipZclEndpointId_t endpointId,
                                      const ChipIpv6Address *dstAddr,
                                      ChipZclGroupId_t groupId)
{
  // Always allow unicast and well-known addresses
  if (dstAddr == NULL
      || !emIsMulticastAddress(dstAddr->bytes)
      || memcmp(allCoapNodesLinkScope.bytes, dstAddr->bytes, sizeof(dstAddr->bytes))  == 0
      || memcmp(allCoapNodesRealmScope.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0
      || memcmp(allCoapNodesAdminScope.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0
      || memcmp(allCoapNodesSiteScope.bytes, dstAddr->bytes, sizeof(dstAddr->bytes))  == 0
      || memcmp(allNodes.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0) {
    return chipZclIsEndpointInGroup(endpointId, groupId);
  }

  if (groupId == CHIP_ZCL_GROUP_ALL_ENDPOINTS && memcmp(broadcastGroupAddress.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0) {
    return true;
  }

  if ((CHIP_ZCL_GROUP_MIN <= groupId) && (groupId <= CHIP_ZCL_GROUP_MAX)) {
    for (size_t i = 0; i < CHIP_ZCL_GROUP_TABLE_SIZE; i++) {
      ChipZclGroupEntry_t entry;
      halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
      if (groupId == entry.groupId && endpointId == entry.endpointId) {
        ChipIpv6Address address = { { 0 } };
        chZclBuildGroupMcastAddress(&entry, address.bytes);
        if (memcmp(address.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0) {
          return true;
        }
      }
    }
  }
  return false;
}

bool chipZclIsAddressGroupMulticast(const ChipIpv6Address *dstAddr)
{
  // First check against well-known addresses as it's cheaper than accessing NVRAM
  if (dstAddr == NULL
      || memcmp(allCoapNodesLinkScope.bytes, dstAddr->bytes, sizeof(dstAddr->bytes))  == 0
      || memcmp(allCoapNodesRealmScope.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0
      || memcmp(allCoapNodesAdminScope.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0
      || memcmp(allCoapNodesSiteScope.bytes, dstAddr->bytes, sizeof(dstAddr->bytes))  == 0
      || memcmp(allNodes.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0) {
    return false;
  }

  if (memcmp(broadcastGroupAddress.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0) {
    return true;
  }

  for (size_t i = 0; i < CHIP_ZCL_GROUP_TABLE_SIZE; i++) {
    ChipZclGroupEntry_t entry;
    halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);

    ChipIpv6Address address = { { 0 } };
    chZclBuildGroupMcastAddress(&entry, address.bytes);
    if (memcmp(address.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0) {
      return true;
    }
  }

  return false;
}

bool chipZclGetGroupName(ChipZclEndpointId_t endpointId,
                          ChipZclGroupId_t groupId,
                          uint8_t *groupName,
                          uint8_t *groupNameLength)
{
  if (CHIP_ZCL_MAX_GROUP_NAME_LENGTH == 0) {
    return false;
  } else if (groupId == CHIP_ZCL_GROUP_ALL_ENDPOINTS) {
    MEMCOPY(groupName, bcastGroupName, bcastGroupNameLength);
    *groupNameLength = bcastGroupNameLength;
    return true;
  } else if (CHIP_ZCL_GROUP_MIN <= groupId && groupId <= CHIP_ZCL_GROUP_MAX) {
    for (size_t i = 0; i < CHIP_ZCL_GROUP_TABLE_SIZE; i++) {
      ChipZclGroupEntry_t entry;
      halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
      if (groupId == entry.groupId && endpointId == entry.endpointId) {
#if CHIP_ZCL_MAX_GROUP_NAME_LENGTH == 0
#else
        MEMCOPY(groupName, entry.groupName, entry.groupNameLength);
        *groupNameLength = entry.groupNameLength;
#endif
        return true;
      }
    }
  }
  return false;
}

ChipZclStatus_t chipZclAddEndpointToGroup(ChipZclEndpointId_t endpointId,
                                            ChipZclGroupId_t groupId,
                                            const uint8_t *groupName,
                                            uint8_t groupNameLength,
                                            uint8_t assignmentMode,
                                            const uint8_t *groupAddress,
                                            uint16_t groupUdpPort)
{
  if (groupId == CHIP_ZCL_GROUP_ALL_ENDPOINTS
      || groupId < CHIP_ZCL_GROUP_MIN
      || groupId > CHIP_ZCL_GROUP_MAX
      || groupNameLength > CHIP_ZCL_MAX_GROUP_NAME_LENGTH
      || (groupNameLength && groupName == NULL)
      || (assignmentMode != CHIP_ZCL_GROUP_ASSIGNMENT_MODE_NULL && groupAddress == NULL)) {
    return CHIP_ZCL_STATUS_FAILURE;
  } else if (chipZclIsEndpointInGroup(endpointId, groupId)) {
    return CHIP_ZCL_STATUS_DUPLICATE_EXISTS;
  } else {
    for (size_t i = 0; i < CHIP_ZCL_GROUP_TABLE_SIZE; i++) {
      ChipZclGroupEntry_t entry;
      halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
      if (entry.groupId == CHIP_ZCL_GROUP_NULL) {
        entry.groupId = groupId;
        entry.endpointId = endpointId;
#if CHIP_ZCL_MAX_GROUP_NAME_LENGTH == 0
#else
        if (groupNameLength && groupName != NULL) {
          entry.groupNameLength = groupNameLength;
          MEMCOPY(entry.groupName, groupName, groupNameLength);
        }
#endif
        entry.addrAssignmentMode = assignmentMode;
        if ( assignmentMode != CHIP_ZCL_GROUP_ASSIGNMENT_MODE_NULL) {
          // If mode is 0xfe the whole IPv6 address is passed, otherwise -- 8 bits of flags
          MEMCOPY(entry.groupMcastAddress.bytes, groupAddress, assignmentMode == CHIP_ZCL_GROUP_ASSIGNMENT_MODE_MANUAL ? 16 : 1);
        }

        entry.groupUdpPort = groupUdpPort;
        // Now listen on the IPv6 address. Treat port 0 (invalid for UDP) as "no value", use default COAP port.
        ChipIpv6Address address = { { 0 } };
        chZclBuildGroupMcastAddress(&entry, address.bytes);
        chipUdpListen(entry.groupUdpPort ? entry.groupUdpPort : CHIP_COAP_PORT, address.bytes);

        uint8_t dst[CHIP_IPV6_ADDRESS_STRING_SIZE];
        if (chipIpv6AddressToString(&address, dst, sizeof(dst))) {
          chipAfPluginZclCorePrintln("Added endpoint %d to group %d with address %s ", endpointId, groupId, dst);
        }

        halCommonSetIndexedToken(TOKEN_ZCL_CORE_GROUP_TABLE, i, &entry);
        return CHIP_ZCL_STATUS_SUCCESS;
      }
    }
    return CHIP_ZCL_STATUS_INSUFFICIENT_SPACE;
  }
}

enum {
  GROUP_FLAG      = 0x01,
  NULL_GROUP_FLAG = 0x02,
  ENDPOINT_FLAG   = 0x04,
};

static ChipZclStatus_t removeEndpoints(uint8_t mask,
                                        ChipZclEndpointId_t endpointId,
                                        ChipZclGroupId_t groupId)
{
  if ((mask & GROUP_FLAG)
      && (groupId < CHIP_ZCL_GROUP_MIN || CHIP_ZCL_GROUP_MAX < groupId)) {
    return CHIP_ZCL_STATUS_INVALID_FIELD;
  }

  ChipZclStatus_t status = CHIP_ZCL_STATUS_NOT_FOUND;
  for (size_t i = 0; i < CHIP_ZCL_GROUP_TABLE_SIZE; i++) {
    ChipZclGroupEntry_t entry;
    halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
    uint8_t flags = 0;
    if (groupId == entry.groupId) {
      flags |= GROUP_FLAG;
    }
    if (endpointId == entry.endpointId) {
      flags |= ENDPOINT_FLAG;
    }
    if (mask == NULL_GROUP_FLAG || (flags & mask) == mask) {
      if ( entry.addrAssignmentMode != CHIP_ZCL_GROUP_ASSIGNMENT_MODE_NULL) {
        ChipIpv6Address address = { { 0 } };
        chZclBuildGroupMcastAddress(&entry, address.bytes);
        chipRemoveUdpListeners(address.bytes);
      }
      entry.addrAssignmentMode = CHIP_ZCL_GROUP_ASSIGNMENT_MODE_NULL;
      entry.groupId = CHIP_ZCL_GROUP_NULL;
      entry.endpointId = CHIP_ZCL_ENDPOINT_NULL;
      halCommonSetIndexedToken(TOKEN_ZCL_CORE_GROUP_TABLE, i, &entry);
      if ((mask & (GROUP_FLAG | ENDPOINT_FLAG))
          == (GROUP_FLAG | ENDPOINT_FLAG)) {
        return CHIP_ZCL_STATUS_SUCCESS;
      } else {
        status = CHIP_ZCL_STATUS_SUCCESS;
      }
    }
  }
  return status;
}

ChipZclStatus_t chipZclRemoveEndpointFromGroup(ChipZclEndpointId_t endpointId,
                                                 ChipZclGroupId_t groupId)
{
  return removeEndpoints(GROUP_FLAG | ENDPOINT_FLAG, endpointId, groupId);
}

ChipZclStatus_t chipZclRemoveEndpointFromAllGroups(ChipZclEndpointId_t endpointId)
{
  return removeEndpoints(ENDPOINT_FLAG, endpointId, CHIP_ZCL_GROUP_NULL);
}

ChipZclStatus_t chipZclRemoveGroup(ChipZclGroupId_t groupId)
{
  return removeEndpoints(GROUP_FLAG, CHIP_ZCL_ENDPOINT_NULL, groupId);
}

ChipZclStatus_t chipZclRemoveAllGroups(void)
{
  return removeEndpoints(NULL_GROUP_FLAG,
                         CHIP_ZCL_ENDPOINT_NULL,
                         CHIP_ZCL_GROUP_NULL);
}

// Re-register to listen to all group multicast addresses, called upon init
void chZclReregisterAllMcastAddresses(void)
{
  for (size_t i = 0; i < CHIP_ZCL_GROUP_TABLE_SIZE; i++) {
    ChipZclGroupEntry_t entry;
    halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
    ChipIpv6Address address = { { 0 } };
    if (entry.groupId != CHIP_ZCL_GROUP_NULL && entry.addrAssignmentMode != CHIP_ZCL_GROUP_ASSIGNMENT_MODE_NULL) {
      chZclBuildGroupMcastAddress(&entry, address.bytes);
      // Treat port 0 (invalid for UDP) as "no value", use default COAP port.
      chipUdpListen(entry.groupUdpPort ? entry.groupUdpPort : CHIP_COAP_PORT, address.bytes);
    }
  }
}

// zcl/g:
//   GET: list groups.
//   OTHER: not allowed.
void chZclUriGroupHandler(ChZclContext_t *context)
{
  ChipZclGroupId_t groups[CHIP_ZCL_GROUP_TABLE_SIZE + 1];
  uint16_t count = 0;
  for (size_t i = 0; i < CHIP_ZCL_GROUP_TABLE_SIZE; i++) {
    ChipZclGroupEntry_t entry;
    halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
    if (entry.groupId != CHIP_ZCL_GROUP_NULL) {
      chZclInsertGroupIdIntoSortedList(entry.groupId,
                                       groups,
                                       &count,
                                       COUNTOF(groups));
    }
  }
  // We know Broadcast Group's value is 0xffff and it will be last in the array.
  groups[count] = CHIP_ZCL_GROUP_ALL_ENDPOINTS;
  count++;

  CborState state;
  uint8_t buffer[CH_ZCL_MAX_PAYLOAD_SIZE];
  emCborEncodeIndefiniteArrayStart(&state, buffer, sizeof(buffer));
  for (size_t i = 0; i < count; i++) {
    if (!emCborEncodeValue(&state,
                           CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER,
                           sizeof(ChipZclGroupId_t),
                           (const uint8_t *)&groups[i])) {
      chZclRespond500InternalServerError(context->info);
      return;
    }
  }
  emCborEncodeBreak(&state);
  chZclRespond205ContentCborState(context->info, &state);
}

// GET zcl/g/XXXX:
//   GET: list endpoints in group.
//   OTHER: not allowed.
void chZclUriGroupIdHandler(ChZclContext_t *context)
{
  CborState state;
  uint8_t buffer[CH_ZCL_MAX_PAYLOAD_SIZE];
  bool endpointFound = false;
  emCborEncodeIndefiniteArrayStart(&state, buffer, sizeof(buffer));
  for (size_t i = 0; i < chZclEndpointCount; i++) {
    if (chipZclIsEndpointInGroup(chZclEndpointTable[i].endpointId,
                                  context->groupId)) {
      endpointFound = true;
      if (!emCborEncodeValue(&state,
                             CHIP_ZCLIP_TYPE_UNSIGNED_INTEGER,
                             sizeof(ChipZclEndpointId_t),
                             &chZclEndpointTable[i].endpointId)) {
        chZclRespond500InternalServerError(context->info);
        return;
      }
    }
  }

  if (endpointFound) {
    emCborEncodeBreak(&state);
    chZclRespond205ContentCborState(context->info, &state);
  } else {
    chZclRespond404NotFound(context->info);
  }
}

// Insert a group ID into a list of group IDs, maintaining ascending order.
void chZclInsertGroupIdIntoSortedList(const ChipZclGroupId_t groupId,
                                      ChipZclGroupId_t * const pgroups,
                                      uint16_t * const pcount,
                                      const uint16_t maxEntries)
{
  if (pgroups == NULL || pcount == NULL) {
    return;
  }
  if (*pcount < maxEntries) {
    bool insert = true;
    uint16_t i;
    for (i = 0; i < *pcount; i++) {
      if (groupId == pgroups[i]) {
        // Duplicate, skip.
        insert = false;
        break;
      } else if (groupId < pgroups[i]) {
        // Shift ordered successors forward to create gap.
        MEMMOVE(pgroups + i + 1,
                pgroups + i,
                (*pcount - i) * sizeof(ChipZclGroupId_t));
        break;
      }
    }
    if (insert) {
      // Insert/append.
      pgroups[i] = groupId;
      (*pcount)++;
    }
  }
}

// Given group IPv6 address parameters as passed in "add group" construct the actual address
// If this entry has no address assignment return without doing anything
void chZclBuildGroupMcastAddress(const ChipZclGroupEntry_t *entry, uint8_t *addressBytes)
{
  if (entry->addrAssignmentMode == CHIP_ZCL_GROUP_ASSIGNMENT_MODE_MANUAL) {
    MEMCOPY(addressBytes, entry->groupMcastAddress.bytes, 16);
  } else {
    // Auto-create an IPv6 address for group.
    // Mapped IPv6 address: ff[flags]::02c1:[GroupID_multicast_IPv6]
    //   where GroupID_multicast_IPv6 = (Group_ID - 1)mod(2^base) + 1
    //   where base is passed in assignmentMode parameter and flags is in the address parameter

    uint8_t flags;
    uint8_t base;
    if (entry->addrAssignmentMode == CHIP_ZCL_GROUP_ASSIGNMENT_MODE_NULL) {
      // ZLCIP BDB Section 2.5.6: Default flags: R=0, P=0, T=1. 2.5.5: Default scope = 5. Section E.1.7.1: Default base = 0x0A
      flags = 0x15;
      base = 0x0A;
    } else {
      flags = entry->groupMcastAddress.bytes[0];
      base = entry->addrAssignmentMode;
    }

    addressBytes[0] = 0xff;
    addressBytes[1] = flags;
    addressBytes[2] = addressBytes[3] = addressBytes[4] = addressBytes[5] = addressBytes[6] = addressBytes[7] = addressBytes[8] = \
                                                                                                                  addressBytes[9] = addressBytes[10] = addressBytes[11] = 0;
    addressBytes[12] = 0x02;
    addressBytes[13] = 0xc1;
    uint16_t twobytes = (((entry->groupId - 1) % (0x01 << base)) + 1);
    addressBytes[14] = twobytes >> 8;
    addressBytes[15] = twobytes;
  }
}
