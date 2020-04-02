/***************************************************************************//**
 *
 *    <COPYRIGHT>
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_HAL
#include EMBER_AF_API_ZCL_CORE
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#else
  #define emberAfPluginZclCorePrint(...)
  #define emberAfPluginZclCorePrintln(...)
  #define emberAfPluginZclCoreFlush()
  #define emberAfPluginZclCoreDebugExec(x)
  #define emberAfPluginZclCorePrintBuffer(buffer, len, withSpace)
  #define emberAfPluginZclCorePrintString(buffer)
#endif

// Group name "broadcast", this is a ZCL string, not NULL-terminated
const uint8_t bcastGroupName[] = { 0x62, 0x72, 0x6f, 0x61, 0x64, 0x63, 0x61, 0x73, 0x74 };
const uint8_t bcastGroupNameLength = 0x09;
const EmberIpv6Address broadcastGroupAddress  = { { 0xff, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xc1, 0xff, 0xff } };
const EmberIpv6Address allCoapNodesLinkScope  = { { 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd } };
const EmberIpv6Address allCoapNodesRealmScope = { { 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd } };
const EmberIpv6Address allCoapNodesAdminScope = { { 0xff, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd } };
const EmberIpv6Address allCoapNodesSiteScope  = { { 0xff, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd } };
const EmberIpv6Address allNodes               = { { 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } };

void emZclGroupNetworkStatusHandler(EmberNetworkStatus newNetworkStatus,
                                    EmberNetworkStatus oldNetworkStatus,
                                    EmberJoinFailureReason reason)
{
  // If the device is no longer associated with a network, its groups are
  // removed, because groups are specific to a network.
  if (newNetworkStatus == EMBER_NO_NETWORK) {
    emberZclRemoveAllGroups();
  }
}

bool emZclHasGroup(EmberZclGroupId_t groupId)
{
  for (size_t i = 0; i < emZclEndpointCount; i++) {
    if (emberZclIsEndpointInGroup(emZclEndpointTable[i].endpointId, groupId)) {
      return true;
    }
  }
  return false;
}

size_t emZclGetGroupsCapacity(void)
{
  // Returns the remaining (unused) capacity in the Groups table.

  size_t capacity = EMBER_ZCL_GROUP_TABLE_SIZE;

  for (size_t i = 0; i < EMBER_ZCL_GROUP_TABLE_SIZE; i++) {
    EmberZclGroupEntry_t entry;
    halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
    if (entry.groupId != EMBER_ZCL_GROUP_NULL) {
      --capacity;
    }
  }
  return capacity;
}

bool emberZclIsEndpointInGroup(EmberZclEndpointId_t endpointId,
                               EmberZclGroupId_t groupId)
{
  if (groupId == EMBER_ZCL_GROUP_ALL_ENDPOINTS) {
    return true;
  }
  if ((EMBER_ZCL_GROUP_MIN <= groupId)
      && (groupId <= EMBER_ZCL_GROUP_MAX)) {
    for (size_t i = 0; i < EMBER_ZCL_GROUP_TABLE_SIZE; i++) {
      EmberZclGroupEntry_t entry;
      halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
      if (groupId == entry.groupId
          && endpointId == entry.endpointId) {
        return true;
      }
    }
  }

  return false;
}

bool emberZclIsEndpointAndAddrInGroup(EmberZclEndpointId_t endpointId,
                                      const EmberIpv6Address *dstAddr,
                                      EmberZclGroupId_t groupId)
{
  // Always allow unicast and well-known addresses
  if (dstAddr == NULL
      || !emIsMulticastAddress(dstAddr->bytes)
      || memcmp(allCoapNodesLinkScope.bytes, dstAddr->bytes, sizeof(dstAddr->bytes))  == 0
      || memcmp(allCoapNodesRealmScope.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0
      || memcmp(allCoapNodesAdminScope.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0
      || memcmp(allCoapNodesSiteScope.bytes, dstAddr->bytes, sizeof(dstAddr->bytes))  == 0
      || memcmp(allNodes.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0) {
    return emberZclIsEndpointInGroup(endpointId, groupId);
  }

  if (groupId == EMBER_ZCL_GROUP_ALL_ENDPOINTS && memcmp(broadcastGroupAddress.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0) {
    return true;
  }

  if ((EMBER_ZCL_GROUP_MIN <= groupId) && (groupId <= EMBER_ZCL_GROUP_MAX)) {
    for (size_t i = 0; i < EMBER_ZCL_GROUP_TABLE_SIZE; i++) {
      EmberZclGroupEntry_t entry;
      halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
      if (groupId == entry.groupId && endpointId == entry.endpointId) {
        EmberIpv6Address address = { { 0 } };
        emZclBuildGroupMcastAddress(&entry, address.bytes);
        if (memcmp(address.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0) {
          return true;
        }
      }
    }
  }
  return false;
}

bool emberZclIsAddressGroupMulticast(const EmberIpv6Address *dstAddr)
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

  for (size_t i = 0; i < EMBER_ZCL_GROUP_TABLE_SIZE; i++) {
    EmberZclGroupEntry_t entry;
    halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);

    EmberIpv6Address address = { { 0 } };
    emZclBuildGroupMcastAddress(&entry, address.bytes);
    if (memcmp(address.bytes, dstAddr->bytes, sizeof(dstAddr->bytes)) == 0) {
      return true;
    }
  }

  return false;
}

bool emberZclGetGroupName(EmberZclEndpointId_t endpointId,
                          EmberZclGroupId_t groupId,
                          uint8_t *groupName,
                          uint8_t *groupNameLength)
{
  if (EMBER_ZCL_MAX_GROUP_NAME_LENGTH == 0) {
    return false;
  } else if (groupId == EMBER_ZCL_GROUP_ALL_ENDPOINTS) {
    MEMCOPY(groupName, bcastGroupName, bcastGroupNameLength);
    *groupNameLength = bcastGroupNameLength;
    return true;
  } else if (EMBER_ZCL_GROUP_MIN <= groupId && groupId <= EMBER_ZCL_GROUP_MAX) {
    for (size_t i = 0; i < EMBER_ZCL_GROUP_TABLE_SIZE; i++) {
      EmberZclGroupEntry_t entry;
      halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
      if (groupId == entry.groupId && endpointId == entry.endpointId) {
#if EMBER_ZCL_MAX_GROUP_NAME_LENGTH == 0
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

EmberZclStatus_t emberZclAddEndpointToGroup(EmberZclEndpointId_t endpointId,
                                            EmberZclGroupId_t groupId,
                                            const uint8_t *groupName,
                                            uint8_t groupNameLength,
                                            uint8_t assignmentMode,
                                            const uint8_t *groupAddress,
                                            uint16_t groupUdpPort)
{
  if (groupId == EMBER_ZCL_GROUP_ALL_ENDPOINTS
      || groupId < EMBER_ZCL_GROUP_MIN
      || groupId > EMBER_ZCL_GROUP_MAX
      || groupNameLength > EMBER_ZCL_MAX_GROUP_NAME_LENGTH
      || (groupNameLength && groupName == NULL)
      || (assignmentMode != EMBER_ZCL_GROUP_ASSIGNMENT_MODE_NULL && groupAddress == NULL)) {
    return EMBER_ZCL_STATUS_FAILURE;
  } else if (emberZclIsEndpointInGroup(endpointId, groupId)) {
    return EMBER_ZCL_STATUS_DUPLICATE_EXISTS;
  } else {
    for (size_t i = 0; i < EMBER_ZCL_GROUP_TABLE_SIZE; i++) {
      EmberZclGroupEntry_t entry;
      halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
      if (entry.groupId == EMBER_ZCL_GROUP_NULL) {
        entry.groupId = groupId;
        entry.endpointId = endpointId;
#if EMBER_ZCL_MAX_GROUP_NAME_LENGTH == 0
#else
        if (groupNameLength && groupName != NULL) {
          entry.groupNameLength = groupNameLength;
          MEMCOPY(entry.groupName, groupName, groupNameLength);
        }
#endif
        entry.addrAssignmentMode = assignmentMode;
        if ( assignmentMode != EMBER_ZCL_GROUP_ASSIGNMENT_MODE_NULL) {
          // If mode is 0xfe the whole IPv6 address is passed, otherwise -- 8 bits of flags
          MEMCOPY(entry.groupMcastAddress.bytes, groupAddress, assignmentMode == EMBER_ZCL_GROUP_ASSIGNMENT_MODE_MANUAL ? 16 : 1);
        }

        entry.groupUdpPort = groupUdpPort;
        // Now listen on the IPv6 address. Treat port 0 (invalid for UDP) as "no value", use default COAP port.
        EmberIpv6Address address = { { 0 } };
        emZclBuildGroupMcastAddress(&entry, address.bytes);
        emberUdpListen(entry.groupUdpPort ? entry.groupUdpPort : EMBER_COAP_PORT, address.bytes);

        uint8_t dst[EMBER_IPV6_ADDRESS_STRING_SIZE];
        if (emberIpv6AddressToString(&address, dst, sizeof(dst))) {
          emberAfPluginZclCorePrintln("Added endpoint %d to group %d with address %s ", endpointId, groupId, dst);
        }

        halCommonSetIndexedToken(TOKEN_ZCL_CORE_GROUP_TABLE, i, &entry);
        return EMBER_ZCL_STATUS_SUCCESS;
      }
    }
    return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  }
}

enum {
  GROUP_FLAG      = 0x01,
  NULL_GROUP_FLAG = 0x02,
  ENDPOINT_FLAG   = 0x04,
};

static EmberZclStatus_t removeEndpoints(uint8_t mask,
                                        EmberZclEndpointId_t endpointId,
                                        EmberZclGroupId_t groupId)
{
  if ((mask & GROUP_FLAG)
      && (groupId < EMBER_ZCL_GROUP_MIN || EMBER_ZCL_GROUP_MAX < groupId)) {
    return EMBER_ZCL_STATUS_INVALID_FIELD;
  }

  EmberZclStatus_t status = EMBER_ZCL_STATUS_NOT_FOUND;
  for (size_t i = 0; i < EMBER_ZCL_GROUP_TABLE_SIZE; i++) {
    EmberZclGroupEntry_t entry;
    halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
    uint8_t flags = 0;
    if (groupId == entry.groupId) {
      flags |= GROUP_FLAG;
    }
    if (endpointId == entry.endpointId) {
      flags |= ENDPOINT_FLAG;
    }
    if (mask == NULL_GROUP_FLAG || (flags & mask) == mask) {
      if ( entry.addrAssignmentMode != EMBER_ZCL_GROUP_ASSIGNMENT_MODE_NULL) {
        EmberIpv6Address address = { { 0 } };
        emZclBuildGroupMcastAddress(&entry, address.bytes);
        emberRemoveUdpListeners(address.bytes);
      }
      entry.addrAssignmentMode = EMBER_ZCL_GROUP_ASSIGNMENT_MODE_NULL;
      entry.groupId = EMBER_ZCL_GROUP_NULL;
      entry.endpointId = EMBER_ZCL_ENDPOINT_NULL;
      halCommonSetIndexedToken(TOKEN_ZCL_CORE_GROUP_TABLE, i, &entry);
      if ((mask & (GROUP_FLAG | ENDPOINT_FLAG))
          == (GROUP_FLAG | ENDPOINT_FLAG)) {
        return EMBER_ZCL_STATUS_SUCCESS;
      } else {
        status = EMBER_ZCL_STATUS_SUCCESS;
      }
    }
  }
  return status;
}

EmberZclStatus_t emberZclRemoveEndpointFromGroup(EmberZclEndpointId_t endpointId,
                                                 EmberZclGroupId_t groupId)
{
  return removeEndpoints(GROUP_FLAG | ENDPOINT_FLAG, endpointId, groupId);
}

EmberZclStatus_t emberZclRemoveEndpointFromAllGroups(EmberZclEndpointId_t endpointId)
{
  return removeEndpoints(ENDPOINT_FLAG, endpointId, EMBER_ZCL_GROUP_NULL);
}

EmberZclStatus_t emberZclRemoveGroup(EmberZclGroupId_t groupId)
{
  return removeEndpoints(GROUP_FLAG, EMBER_ZCL_ENDPOINT_NULL, groupId);
}

EmberZclStatus_t emberZclRemoveAllGroups(void)
{
  return removeEndpoints(NULL_GROUP_FLAG,
                         EMBER_ZCL_ENDPOINT_NULL,
                         EMBER_ZCL_GROUP_NULL);
}

// Re-register to listen to all group multicast addresses, called upon init
void emZclReregisterAllMcastAddresses(void)
{
  for (size_t i = 0; i < EMBER_ZCL_GROUP_TABLE_SIZE; i++) {
    EmberZclGroupEntry_t entry;
    halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
    EmberIpv6Address address = { { 0 } };
    if (entry.groupId != EMBER_ZCL_GROUP_NULL && entry.addrAssignmentMode != EMBER_ZCL_GROUP_ASSIGNMENT_MODE_NULL) {
      emZclBuildGroupMcastAddress(&entry, address.bytes);
      // Treat port 0 (invalid for UDP) as "no value", use default COAP port.
      emberUdpListen(entry.groupUdpPort ? entry.groupUdpPort : EMBER_COAP_PORT, address.bytes);
    }
  }
}

// zcl/g:
//   GET: list groups.
//   OTHER: not allowed.
void emZclUriGroupHandler(EmZclContext_t *context)
{
  EmberZclGroupId_t groups[EMBER_ZCL_GROUP_TABLE_SIZE + 1];
  uint16_t count = 0;
  for (size_t i = 0; i < EMBER_ZCL_GROUP_TABLE_SIZE; i++) {
    EmberZclGroupEntry_t entry;
    halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_GROUP_TABLE, i);
    if (entry.groupId != EMBER_ZCL_GROUP_NULL) {
      emZclInsertGroupIdIntoSortedList(entry.groupId,
                                       groups,
                                       &count,
                                       COUNTOF(groups));
    }
  }
  // We know Broadcast Group's value is 0xffff and it will be last in the array.
  groups[count] = EMBER_ZCL_GROUP_ALL_ENDPOINTS;
  count++;

  CborState state;
  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  emCborEncodeIndefiniteArrayStart(&state, buffer, sizeof(buffer));
  for (size_t i = 0; i < count; i++) {
    if (!emCborEncodeValue(&state,
                           EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                           sizeof(EmberZclGroupId_t),
                           (const uint8_t *)&groups[i])) {
      emZclRespond500InternalServerError(context->info);
      return;
    }
  }
  emCborEncodeBreak(&state);
  emZclRespond205ContentCborState(context->info, &state);
}

// GET zcl/g/XXXX:
//   GET: list endpoints in group.
//   OTHER: not allowed.
void emZclUriGroupIdHandler(EmZclContext_t *context)
{
  CborState state;
  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  bool endpointFound = false;
  emCborEncodeIndefiniteArrayStart(&state, buffer, sizeof(buffer));
  for (size_t i = 0; i < emZclEndpointCount; i++) {
    if (emberZclIsEndpointInGroup(emZclEndpointTable[i].endpointId,
                                  context->groupId)) {
      endpointFound = true;
      if (!emCborEncodeValue(&state,
                             EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                             sizeof(EmberZclEndpointId_t),
                             &emZclEndpointTable[i].endpointId)) {
        emZclRespond500InternalServerError(context->info);
        return;
      }
    }
  }

  if (endpointFound) {
    emCborEncodeBreak(&state);
    emZclRespond205ContentCborState(context->info, &state);
  } else {
    emZclRespond404NotFound(context->info);
  }
}

// Insert a group ID into a list of group IDs, maintaining ascending order.
void emZclInsertGroupIdIntoSortedList(const EmberZclGroupId_t groupId,
                                      EmberZclGroupId_t * const pgroups,
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
                (*pcount - i) * sizeof(EmberZclGroupId_t));
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
void emZclBuildGroupMcastAddress(const EmberZclGroupEntry_t *entry, uint8_t *addressBytes)
{
  if (entry->addrAssignmentMode == EMBER_ZCL_GROUP_ASSIGNMENT_MODE_MANUAL) {
    MEMCOPY(addressBytes, entry->groupMcastAddress.bytes, 16);
  } else {
    // Auto-create an IPv6 address for group.
    // Mapped IPv6 address: ff[flags]::02c1:[GroupID_multicast_IPv6]
    //   where GroupID_multicast_IPv6 = (Group_ID - 1)mod(2^base) + 1
    //   where base is passed in assignmentMode parameter and flags is in the address parameter

    uint8_t flags;
    uint8_t base;
    if (entry->addrAssignmentMode == EMBER_ZCL_GROUP_ASSIGNMENT_MODE_NULL) {
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
