/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 */

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
 */
/***************************************************************************//**
 * @file
 * @brief This code provides support for managing the address table.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "address-table.h"

#define FREE_EUI64 { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }

#ifdef EZSP_HOST
  #define SET_CACHE_ADDRESS(index, longId)   MEMCOPY(addressTable[(index)], (longId), EUI64_SIZE)
  #define CLEAR_CACHE_ADDRESS(index)         MEMSET(&addressTable[(index)], 0xFF, EUI64_SIZE)
  #define GET_ADDRESS(index, dstEntry)       MEMMOVE((dstEntry), addressTable[(index)], EUI64_SIZE);
#else
  #define SET_CACHE_ADDRESS(index, longId)
  #define CLEAR_CACHE_ADDRESS(index)
  #define GET_ADDRESS(index, dstEntry)       emberGetAddressTableRemoteEui64((index), (dstEntry))
#endif

static uint8_t addressTableReferenceCounts[EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE];
#ifdef EZSP_HOST
static EmberEUI64 addressTable[EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE];
static bool initPending = true;
#endif  // EZSP_HOST

#ifdef EZSP_HOST
void emberAfPluginAddressTableNcpInitCallback(bool memoryAllocation)
{
  uint8_t index;
  EmberEUI64 freeEui = FREE_EUI64;
  uint16_t addressTableSize;

  if (memoryAllocation) {
    return;
  }

  // If the host and the ncp disagree on the address table size, explode.
  ezspGetConfigurationValue(EZSP_CONFIG_ADDRESS_TABLE_SIZE, &addressTableSize);
  assert(EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE == addressTableSize);

  if (initPending) {
    // Initialize all the entries to all 0xFFs. All 0xFFs means that the entry
    // is unused.
    MEMSET(addressTable, 0xFF, EUI64_SIZE * EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE);
    initPending = false;
    return;
  }

  // When the NCP resets the address table at the NCP is empty (it is only
  // stored in RAM). We re-add all the non-empty entries at the NCP.
  for (index = 0; index < EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE; index++) {
    if (MEMCOMPARE(addressTable[index], freeEui, EUI64_SIZE) != 0) {
      if (emberSetAddressTableRemoteEui64(index, addressTable[index])
          != EMBER_SUCCESS) {
        assert(0);  // We expect the host and the NCP table to always match, so
      }
      // we should always be able to add an entry at the NCP here.
    }
  }
}
#endif

// Non Plugin Specific functions.
EmberStatus emberAfSendCommandUnicastToEui64(EmberEUI64 destination)
{
  return emberAfSendUnicastToEui64(destination,
                                   emAfCommandApsFrame,
                                   *emAfResponseLengthPtr,
                                   emAfZclBuffer);
}

uint8_t emberAfLookupAddressTableEntryByEui64(EmberEUI64 longId)
{
  uint8_t i;
  if (longId == NULL) {
    goto kickout;
  }
#ifdef EZSP_HOST
  for (i = 0; i < EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE; i++) {
    if (MEMCOMPARE(addressTable[i], longId, EUI64_SIZE) == 0) {
      return i;
    }
  }
#else
  for (i = 0; i < EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE; i++) {
    if (emberGetAddressTableRemoteNodeId(i)
        != EMBER_TABLE_ENTRY_UNUSED_NODE_ID) {
      EmberEUI64 eui64;
      emberGetAddressTableRemoteEui64(i, eui64);
      if (MEMCOMPARE(longId, eui64, EUI64_SIZE) == 0) {
        return i;
      }
    }
  }
#endif
  kickout:
  return EMBER_NULL_ADDRESS_TABLE_INDEX;
}

EmberStatus emberAfSendUnicastToEui64(EmberEUI64 destination,
                                      EmberApsFrame *apsFrame,
                                      uint16_t messageLength,
                                      uint8_t *message)
{
  uint8_t index = emberAfPluginAddressTableLookupByEui64(destination);

  if (index == EMBER_NULL_ADDRESS_TABLE_INDEX) {
    return EMBER_INVALID_CALL;
  }

  return emberAfSendUnicast(EMBER_OUTGOING_VIA_ADDRESS_TABLE,
                            index,
                            apsFrame,
                            messageLength,
                            message);
}

uint8_t emberAfAddAddressTableEntry(EmberEUI64 longId, EmberNodeId shortId)
{
  uint8_t i, index = EMBER_NULL_ADDRESS_TABLE_INDEX;
  // search for unused node entry / finding existing entry with matching longId
  for (i = 0; i < EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE; i++) {
    if (emberGetAddressTableRemoteNodeId(i) != EMBER_TABLE_ENTRY_UNUSED_NODE_ID) {
      EmberEUI64 eui64;
      emberGetAddressTableRemoteEui64(i, eui64);
      if (MEMCOMPARE(longId, eui64, EUI64_SIZE) == 0) {
        index = i;
        goto kickout;
      }
    } else if (index == EMBER_NULL_ADDRESS_TABLE_INDEX) {
      index = i;
      break;
    } else {
      // MISRA requires ..else if.. to have terminating else.
    }
  }

  if (index == EMBER_NULL_ADDRESS_TABLE_INDEX) {
    return index;
  }

  emberSetAddressTableRemoteEui64(index, longId);
  SET_CACHE_ADDRESS(index, longId);

  kickout:
  if (shortId != EMBER_UNKNOWN_NODE_ID) {
    emberSetAddressTableRemoteNodeId(index, shortId);
  }
  if (index < EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE) {
    addressTableReferenceCounts[index]++;
  }
  return index;
}

EmberStatus emberAfSetAddressTableEntry(uint8_t index,
                                        EmberEUI64 longId,
                                        EmberNodeId shortId)
{
  EmberStatus status = EMBER_ADDRESS_TABLE_INDEX_OUT_OF_RANGE;
  if (index < EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE) {
    if (addressTableReferenceCounts[index] == 0) {
      status = emberSetAddressTableRemoteEui64(index, longId);
      SET_CACHE_ADDRESS(index, longId);
      if (status == EMBER_SUCCESS && shortId != EMBER_UNKNOWN_NODE_ID) {
        emberSetAddressTableRemoteNodeId(index, shortId);
      }
      addressTableReferenceCounts[index] = 1;
    } else {
      status = EMBER_ADDRESS_TABLE_ENTRY_IS_ACTIVE;
    }
  }
  return status;
}

EmberStatus emberAfRemoveAddressTableEntry(uint8_t index)
{
  EmberStatus status = EMBER_ADDRESS_TABLE_INDEX_OUT_OF_RANGE;
  if (index < EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE) {
    if (0 < addressTableReferenceCounts[index]) {
      addressTableReferenceCounts[index]--;
    }
    if (addressTableReferenceCounts[index] == 0) {
      emberSetAddressTableRemoteNodeId(index,
                                       EMBER_TABLE_ENTRY_UNUSED_NODE_ID);
      CLEAR_CACHE_ADDRESS(index);
    }
    status = EMBER_SUCCESS;
  }
  return status;
}

EmberStatus emberAfGetCurrentSenderEui64(EmberEUI64 address)
{
  uint8_t index = emberAfGetAddressIndex();
  if (index == EMBER_NULL_ADDRESS_TABLE_INDEX) {
    return EMBER_INVALID_CALL;
  } else {
    return emberAfPluginAddressTableLookupByIndex(index, address);
  }
}

// Plugin functions.

EmberNodeId emberAfPluginAddressTableLookupNodeIdByIndex(uint8_t index)
{
  EmberEUI64 eui64;
  if (emberAfPluginAddressTableLookupByIndex(index, eui64) == EMBER_SUCCESS) {
    EmberNodeId nodeId = emberLookupNodeIdByEui64(eui64);
    return nodeId;
  }
  return EMBER_NULL_NODE_ID;
}

EmberStatus emberAfPluginAddressTableRemoveEntry(EmberEUI64 entry)
{
  uint8_t index = emberAfPluginAddressTableLookupByEui64(entry);
  return emberAfPluginAddressTableRemoveEntryByIndex(index);
}

EmberStatus emberAfPluginAddressTableLookupByIndex(uint8_t index,
                                                   EmberEUI64 entry)
{
  if (index < EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE) {
    GET_ADDRESS(index, entry);
    return EMBER_SUCCESS;
  } else {
    return EMBER_INVALID_CALL;
  }
}
