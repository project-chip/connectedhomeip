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
 * @brief Matrix routines for the Price Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "price-server.h"
#include "price-server-tick.h"

#if defined(EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_MATRIX_SUPPORT)

//=============================================================================
// Functions
void emberAfPriceClearTariffTable(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    MEMSET(&priceServerInfo.scheduledTariffTable.commonInfos[ep][i], 0x00, sizeof(EmberAfPriceCommonInfo));
    MEMSET(&priceServerInfo.scheduledTariffTable.scheduledTariffs[ep][i], 0x00, sizeof(EmberAfScheduledTariff));
  }
}

void emberAfPriceClearPriceMatrixTable(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    MEMSET(&priceServerInfo.scheduledPriceMatrixTable.commonInfos[ep][i], 0x00, sizeof(EmberAfPriceCommonInfo));
    MEMSET(&priceServerInfo.scheduledPriceMatrixTable.scheduledPriceMatrix[ep][i], 0x00, sizeof(EmberAfScheduledPriceMatrix));
  }
}

void emberAfPriceClearBlockThresholdsTable(uint8_t endpoint)
{
  uint8_t i;
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    MEMSET(&priceServerInfo.scheduledBlockThresholdsTable.scheduledBlockThresholds[ep][i], 0x00, sizeof(EmberAfScheduledBlockThresholds));
    MEMSET(&priceServerInfo.scheduledBlockThresholdsTable.commonInfos[ep][i], 0x00, sizeof(EmberAfPriceCommonInfo));
  }
}

// Retrieves the tariff at the index. Returns false if the index is invalid.
bool emberAfPriceGetTariffTableEntry(uint8_t endpoint,
                                     uint8_t index,
                                     EmberAfPriceCommonInfo *info,
                                     EmberAfScheduledTariff *tariff)
{
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF || index == 0xFF) {
    return false;
  }

  if (index < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE) {
    emberAfDebugPrintln("emberAfPriceGetTariffTableEntry: retrieving tariff at ep(%d), index(%d)", ep, index);
    MEMCOPY(info, &priceServerInfo.scheduledTariffTable.commonInfos[ep][index], sizeof(EmberAfPriceCommonInfo));
    MEMCOPY(tariff, &priceServerInfo.scheduledTariffTable.scheduledTariffs[ep][index], sizeof(EmberAfScheduledTariff));
    return true;
  }

  return false;
}

// Retrieves the price matrix at the index. Returns false if the index is invalid.
bool emberAfPriceGetPriceMatrix(uint8_t endpoint,
                                uint8_t index,
                                EmberAfPriceCommonInfo *info,
                                EmberAfScheduledPriceMatrix *pm)
{
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF || index == 0xFF) {
    return false;
  }

  if (index < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE) {
    emberAfDebugPrintln("emberAfPriceGetTariffTableEntry: retrieving tariff at ep(%d), index(%d)", ep, index);
    MEMCOPY(info, &priceServerInfo.scheduledPriceMatrixTable.commonInfos[ep][index], sizeof(EmberAfPriceCommonInfo));
    MEMCOPY(pm, &priceServerInfo.scheduledPriceMatrixTable.scheduledPriceMatrix[ep][index], sizeof(EmberAfScheduledPriceMatrix));
    return true;
  }

  return false;
}

// Retrieves the block thresholds at the index. Returns false if the index is invalid.
bool emberAfPriceGetBlockThresholdsTableEntry(uint8_t endpoint,
                                              uint8_t index,
                                              EmberAfScheduledBlockThresholds *bt)
{
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF || index == 0xFF) {
    return false;
  }

  if (index < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE) {
    emberAfDebugPrintln("emberAfPriceGetBlockThresholdsTableEntry: retrieving tariff at index %d, %d", ep, index);
    MEMCOPY(bt,
            &priceServerInfo.scheduledBlockThresholdsTable.scheduledBlockThresholds[ep][index],
            sizeof(EmberAfScheduledBlockThresholds));
    return true;
  }

  return false;
}

// Retrieves the tariff with the corresponding issuer tariff ID. Returns false
// if the tariff is not found or is invalid.
bool emberAfPriceGetTariffByIssuerTariffId(uint8_t endpoint,
                                           uint32_t issuerTariffId,
                                           EmberAfPriceCommonInfo *info,
                                           EmberAfScheduledTariff *tariff)
{
  uint8_t i;
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return false;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    EmberAfPriceCommonInfo *curInfo = &priceServerInfo.scheduledTariffTable.commonInfos[ep][i];
    EmberAfScheduledTariff *lookup = &priceServerInfo.scheduledTariffTable.scheduledTariffs[ep][i];

    if (lookup->status != 0
        && lookup->issuerTariffId == issuerTariffId) {
      MEMCOPY(info, curInfo, sizeof(EmberAfPriceCommonInfo));
      MEMCOPY(tariff, lookup, sizeof(EmberAfScheduledTariff));
      return true;
    }
  }

  return false;
}

// Retrieves the price matrix with the corresponding issuer tariff ID. Returns false
// if the price matrix is not found or is invalid.
bool emberAfPriceGetPriceMatrixByIssuerTariffId(uint8_t endpoint,
                                                uint32_t issuerTariffId,
                                                EmberAfPriceCommonInfo *info,
                                                EmberAfScheduledPriceMatrix *pm)
{
  uint8_t i;
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return false;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    EmberAfPriceCommonInfo * lookup_info = &priceServerInfo.scheduledPriceMatrixTable.commonInfos[ep][i];
    EmberAfScheduledPriceMatrix *lookup_pm = &priceServerInfo.scheduledPriceMatrixTable.scheduledPriceMatrix[ep][i];
    if (lookup_info->valid
        && (lookup_pm->status != 0)
        && lookup_pm->issuerTariffId == issuerTariffId) {
      MEMCOPY(info, lookup_info, sizeof(EmberAfPriceCommonInfo));
      MEMCOPY(pm, lookup_pm, sizeof(EmberAfScheduledPriceMatrix));
      return true;
    }
  }

  return false;
}

// Retrieves the block thresholds with the corresponding issuer tariff ID. Returns false
// if the block thresholds is not found or is invalid.
bool emberAfPriceGetBlockThresholdsByIssuerTariffId(uint8_t endpoint,
                                                    uint32_t issuerTariffId,
                                                    EmberAfPriceCommonInfo *info,
                                                    EmberAfScheduledBlockThresholds *bt)
{
  uint8_t i;
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return false;
  }

  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    EmberAfPriceCommonInfo *lookup_info = &priceServerInfo.scheduledBlockThresholdsTable.commonInfos[ep][i];
    EmberAfScheduledBlockThresholds *lookup_bt = &priceServerInfo.scheduledBlockThresholdsTable.scheduledBlockThresholds[ep][i];
    if ((lookup_info->valid)
        && (lookup_bt->status != 0)
        && lookup_bt->issuerTariffId == issuerTariffId) {
      if (info != NULL) {
        MEMCOPY(info, lookup_info, sizeof(EmberAfPriceCommonInfo));
      }
      if (bt != NULL) {
        MEMCOPY(bt, lookup_bt, sizeof(EmberAfScheduledBlockThresholds));
      }
      return true;
    }
  }

  return false;
}

// Query the tariff table for tariffs matching the selection requirements dictated
// by the GetTariffInformation command. Returns the number of matching tariffs found.
/*
   static uint8_t findTariffs(uint8_t endpoint,
                         uint32_t earliestStartTime,
                         uint32_t minIssuerId,
                         uint8_t numTariffs,
                         EmberAfTariffType tariffType,
                         EmberAfPriceCommonInfo *dstInfos,
                         EmberAfScheduledTariff *dstTariffs)
   {
   uint8_t ep;
   uint8_t i, tariffsFound = 0;
   EmberAfScheduledTariff *curTariff;
   EmberAfPriceCommonInfo *curInfo;

   emberAfDebugPrintln("findTariffs: selection criteria");
   emberAfDebugPrintln("             earliestStartTime: %4x", earliestStartTime);
   emberAfDebugPrintln("             minIssuerId: %4x", minIssuerId);
   emberAfDebugPrintln("             numberOfTariffsRequested: %d", numTariffs);
   emberAfDebugPrintln("             tariffType: %x", tariffType);

   ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
   if (ep == 0xFF) {
    goto kickout;
   }

   while (numTariffs == 0 || tariffsFound < numTariffs) {
    uint32_t referenceStartTime = MAX_INT32U_VALUE;
    uint8_t indexToSend = 0xFF;

    // Find active or scheduled tariffs matching the filter fields in the
    // request that have not been sent out yet.  Of those, find the one that
    // starts the earliest.
    for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
      curTariff = &info.scheduledTariffTable.scheduledTariffs[ep][i];
      curInfo = &info.scheduledTariffTable.commonInfos[ep][i];

      if (!tariffIsPublished(curTariff)
          && (curInfo->startTime < referenceStartTime)
          && (minIssuerId == ZCL_PRICE_CLUSTER_WILDCARD_ISSUER_ID
 || curInfo->issuerEventId >= minIssuerId)
          && (curTariff->tariffTypeChargingScheme & TARIFF_TYPE_MASK) == (tariffType & TARIFF_TYPE_MASK)
          && (tariffIsCurrent(curTariff) || tariffIsFuture(tariff))) {
        referenceStartTime =*curTariff->startTime;
        indexToSend = i;
      }
    }

    // If no active or scheduled tariffs were found, it either means there are
    // no active or scheduled tariffs at the specified time or we've already
    // found all of them in previous iterations.  If we did find one, we send
    // it, mark it as sent, and move on.
    if (indexToSend == 0xFF) {
      break;
    } else {
      emberAfDebugPrintln("findTariffs: found matching tariff at index %d", indexToSend);
      MEMCOPY(&dstInfos[tariffsFound],
              &info.scheduledTariffTable.commonInfos[ep][indexToSend],
              sizeof(EmberAfPriceCommonInfo));
      MEMCOPY(&dstTariffs[tariffsFound],
              &info.scheduledTariffTable.scheduledTariffs[ep][indexToSend],
              sizeof(EmberAfScheduledTariff));
      tariffsFound++;
      emberAfDebugPrintln("findTariffs: %d tariffs found so far", tariffsFound);
      info.scheduledTariffTable.scheduledTariffs[ep][indexToSend].status |= PUBLISHED;
    }
   }

   // Roll through all the tariffs and clear the published bit. We only use it in
   // in this function to keep track of which tariffs have already been added to
   // the return tariffs array.
   if (tariffsFound > 0) {
    for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
      if (tariffIsPublished(&info.scheduledTariffTable.scheduledTariffs[ep][i])) {
        info.scheduledTariffTable.scheduledTariffs[ep][i].status &= ~PUBLISHED;
      }
    }
   }
   kickout:
   return tariffsFound;
   }
 */
bool emberAfPriceAddTariffTableEntry(uint8_t endpoint,
                                     EmberAfPriceCommonInfo *info,
                                     const EmberAfScheduledTariff *curTariff)
{
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  uint8_t tableIndex = ZCL_PRICE_INVALID_INDEX;

  if (ep == 0xFF) {
    emberAfPriceClusterPrintln("ERR: Unable to find endpoint (%d)!", endpoint);
    return false;
  }

  // init
  info->valid = true;
  info->actionsPending = true;

  // get table index for new entry
  tableIndex = emberAfPluginPriceCommonGetCommonMatchingOrUnusedIndex(priceServerInfo.scheduledTariffTable.commonInfos[ep],
                                                                      EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE,
                                                                      info->issuerEventId,
                                                                      info->startTime,
                                                                      false);

  if (tableIndex == ZCL_PRICE_INVALID_INDEX) {
    emberAfPriceClusterPrintln("ERR: Unable to add new tariff info entry(%d)!", endpoint);
    return false;
  }

  return emberAfPriceSetTariffTableEntry(endpoint,
                                         tableIndex,
                                         info,
                                         curTariff);
}

// Sets the tariff at the index. Returns false if the index is invalid.
bool emberAfPriceSetTariffTableEntry(uint8_t endpoint,
                                     uint8_t index,
                                     EmberAfPriceCommonInfo *info,
                                     const EmberAfScheduledTariff *newTariff)
{
  bool update = false;
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    emberAfPriceClusterPrintln("ERR: Unable to find endpoint (%d)!", endpoint);
    update = false;
    goto kickout;
  }

  if (index >= EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE) {
    emberAfPriceClusterPrintln("ERR: Index out of bound! (%d)!", index);
    update = false;
    goto kickout;
  }

  if ((newTariff == NULL)
      || (info == NULL)) {
    priceServerInfo.scheduledTariffTable.commonInfos[ep][index].valid = 0;
    priceServerInfo.scheduledTariffTable.scheduledTariffs[ep][index].status = 0;
    update = true;
    goto kickout;
  }

  // this command doesn't use a corresponding duration time, so we set the duration as never ending.
  info->durationSec = ZCL_PRICE_CLUSTER_END_TIME_NEVER;

  MEMCOPY(&priceServerInfo.scheduledTariffTable.commonInfos[ep][index], info, sizeof(EmberAfPriceCommonInfo));
  MEMCOPY(&priceServerInfo.scheduledTariffTable.scheduledTariffs[ep][index], newTariff, sizeof(EmberAfScheduledTariff));

  emberAfPriceClusterPrintln("Info: Updated TariffInfo(index=%d) with tariff infos below.", index);
  emberAfPricePrintTariff(&priceServerInfo.scheduledTariffTable.commonInfos[ep][index],
                          &priceServerInfo.scheduledTariffTable.scheduledTariffs[ep][index]);

  if (priceServerInfo.scheduledTariffTable.scheduledTariffs[ep][index].status == 0) {
    priceServerInfo.scheduledTariffTable.scheduledTariffs[ep][index].status |= FUTURE;
  }
  update = true;

  kickout:
  if (update) {
    emberAfPluginPriceCommonSort((EmberAfPriceCommonInfo *)&priceServerInfo.scheduledTariffTable.commonInfos[ep],
                                 (uint8_t *)&priceServerInfo.scheduledTariffTable.scheduledTariffs[ep],
                                 sizeof(EmberAfScheduledTariff),
                                 EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE);
    emberAfPriceClusterScheduleTickCallback(endpoint,
                                            EMBER_AF_PRICE_SERVER_CHANGE_TARIFF_INFORMATION_EVENT_MASK);
  }
  return update;
}

bool emberAfPriceAddPriceMatrix(uint8_t endpoint,
                                EmberAfPriceCommonInfo *info,
                                EmberAfScheduledPriceMatrix * pm)
{
  uint8_t ep;
  uint8_t tableIndex;
  ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  if (ep == 0xFF) {
    emberAfPriceClusterPrintln("ERR: Unable to find endpoint (%d)!", endpoint);
    return false;
  }

  if (info->startTime == ZCL_PRICE_CLUSTER_START_TIME_NOW) {
    info->startTime = emberAfGetCurrentTime();
  }

  tableIndex = emberAfPluginPriceCommonGetCommonMatchingOrUnusedIndex(priceServerInfo.scheduledPriceMatrixTable.commonInfos[ep],
                                                                      EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE,
                                                                      info->issuerEventId,
                                                                      info->startTime,
                                                                      false);
  if (tableIndex == ZCL_PRICE_INVALID_INDEX) {
    emberAfPriceClusterPrintln("ERR: Unable to add block threshold entry!");
    return false;
  }

  info->valid = true;
  info->durationSec = ZCL_PRICE_CLUSTER_DURATION_SEC_UNTIL_CHANGED;

  return emberAfPriceSetPriceMatrix(endpoint,
                                    tableIndex,
                                    info,
                                    pm);
}

bool emberAfPriceAddPriceMatrixRaw(uint8_t endpoint,
                                   uint32_t providerId,
                                   uint32_t issuerEventId,
                                   uint32_t startTime,
                                   uint32_t issuerTariffId,
                                   uint8_t commandIndex,
                                   uint8_t numberOfCommands,
                                   uint8_t subPayloadControl,
                                   uint8_t* payload)
{
  EmberAfScheduledPriceMatrix pm;
  EmberAfPriceCommonInfo pmInfo;
  uint16_t payloadIndex = 0;
  uint16_t payloadLength = fieldLength(payload);

  pmInfo.valid = true;
  pmInfo.startTime = startTime;
  pmInfo.issuerEventId = issuerEventId;
  pmInfo.durationSec = ZCL_PRICE_CLUSTER_DURATION_SEC_UNTIL_CHANGED;
  pm.issuerTariffId = issuerTariffId;
  pm.providerId = providerId;

  while (payloadIndex + ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUBPAYLOAD_BLOCK_SIZE <= payloadLength) {
    uint32_t price;
    if ((subPayloadControl & 0x01) == 0x01) {
      uint8_t tier = payload[payloadIndex];
      MEMCOPY(&price, &payload[payloadIndex + 1], 4);
      pm.matrix.tier[tier] = price;
      emberAfPriceClusterPrintln("Info: Updating PriceMatrix tier[%d] = 0x%4X",
                                 tier,
                                 price);
    } else if ((subPayloadControl & 0x01) == 0x00) {
      uint8_t blockNumber = payload[payloadIndex] & 0x0F;
      uint8_t tier = (payload[payloadIndex] & 0xF0) >> 4;
      MEMCOPY(&price, &payload[payloadIndex + 1], 4);
      pm.matrix.blockAndTier[tier][blockNumber] = price;
      emberAfPriceClusterPrintln("Info: Updating PriceMatrix blockAndTier[%d][%d] = 0x%4X",
                                 tier,
                                 blockNumber,
                                 price);
    }

    payloadIndex += 5;
  }

  return emberAfPriceAddPriceMatrix(endpoint, &pmInfo, &pm);
}

// Sets the price matrix at the index. Returns false if the index is invalid.
bool emberAfPriceSetPriceMatrix(uint8_t endpoint,
                                uint8_t index,
                                EmberAfPriceCommonInfo *info,
                                const EmberAfScheduledPriceMatrix *pm)
{
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == ZCL_PRICE_INVALID_ENDPOINT_INDEX ) {
    emberAfPriceClusterPrintln("ERR: Unable to find endpoint (%d)!", endpoint);
    return false;
  }

  if (index < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE) {
    EmberAfPriceCommonInfo * curInfo = &priceServerInfo.scheduledPriceMatrixTable.commonInfos[ep][index];
    EmberAfScheduledPriceMatrix * curPM = &priceServerInfo.scheduledPriceMatrixTable.scheduledPriceMatrix[ep][index];

    if (pm == NULL || info == NULL) {
      curInfo->valid = false;
      curPM->status = 0;
      return true;
    }

    info->actionsPending = true;

    MEMCOPY(curInfo, info, sizeof(EmberAfPriceCommonInfo));
    MEMCOPY(curPM, pm, sizeof(EmberAfScheduledPriceMatrix));

    if (curPM->status == 0) {
      curPM->status |= FUTURE;
    }
    emberAfPluginPriceCommonSort((EmberAfPriceCommonInfo *)&priceServerInfo.scheduledPriceMatrixTable.commonInfos[ep],
                                 (uint8_t *)&priceServerInfo.scheduledPriceMatrixTable.scheduledPriceMatrix[ep],
                                 sizeof(EmberAfScheduledPriceMatrix),
                                 EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE);
    emberAfPriceClusterScheduleTickCallback(endpoint,
                                            EMBER_AF_PRICE_SERVER_ACTIVATED_PRICE_MATRIX_EVENT_MASK);

    return true;
  }

  return false;
}

bool emberAfPriceAddBlockThresholdsTableEntry(uint8_t endpoint,
                                              uint32_t providerId,
                                              uint32_t issuerEventId,
                                              uint32_t startTime,
                                              uint32_t issuerTariffId,
                                              uint8_t commandIndex,
                                              uint8_t numberOfCommands,
                                              uint8_t subpayloadControl,
                                              uint8_t* payload)
{
  EmberAfPriceCommonInfo inf;
  EmberAfScheduledBlockThresholds bt;
  uint16_t payloadIndex = 0;
  bool status = true;
  uint8_t tableIndex;
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  uint16_t payloadLength = fieldLength(payload);

  if (ep == 0xFF) {
    return false;
  }

  if (startTime == ZCL_PRICE_CLUSTER_START_TIME_NOW) {
    startTime = emberAfGetCurrentTime();
  }

  tableIndex = emberAfPluginPriceCommonGetCommonMatchingOrUnusedIndex(priceServerInfo.scheduledBlockThresholdsTable.commonInfos[ep],
                                                                      EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE,
                                                                      issuerEventId,
                                                                      startTime,
                                                                      false);
  if (tableIndex == ZCL_PRICE_INVALID_INDEX) {
    emberAfPriceClusterPrintln("ERR: Unable to add block threshold entry!");
    return false;
  }

  inf.startTime = startTime;
  inf.issuerEventId = issuerEventId;
  bt.providerId = providerId;
  bt.issuerTariffId = issuerTariffId;

  if ((subpayloadControl & 0x01) == 0x01) {
    // tier/numberofblockthresholds
    // least sig nibble - number of thresholds
    // most  sig nibble - not used
    // number of thresholds will apply to all tier
    // we assume payload will contain 1 8bit bit/numberOfBlockThresholds
    // followed by block thresholds
    uint8_t i;
    uint8_t numberOfBt = (payload[payloadIndex] & 0x0F);

    payloadIndex++;
    if (payloadIndex + numberOfBt * ZCL_PRICE_CLUSTER_BLOCK_THRESHOLDS_PAYLOAD_SIZE
        > payloadLength) {
      emberAfPriceClusterPrintln("ERR: Invalid number of thresholds(%d) within BlockThresholds payload!",
                                 numberOfBt);
      status = false;
      goto kickout;
    }

    if (numberOfBt > ZCL_PRICE_CLUSTER_MAX_TOU_BLOCKS - 1) {
      emberAfPriceClusterPrintln("ERR: The number(%d) of block threshold is larger than the max (%d)!",
                                 numberOfBt,
                                 ZCL_PRICE_CLUSTER_MAX_TOU_BLOCKS - 1);
      status = false;
      goto kickout;
    }

    for (i = 0; i < numberOfBt; i++) {
      MEMCOPY(&bt.thresholds.block[i],
              &payload[payloadIndex + i * ZCL_PRICE_CLUSTER_BLOCK_THRESHOLDS_PAYLOAD_SIZE],
              ZCL_PRICE_CLUSTER_BLOCK_THRESHOLDS_PAYLOAD_SIZE);
      emberAfPriceClusterPrint("Info: Updating block[%d] = 0x",
                               i);
      emberAfPriceClusterPrintBuffer(&payload[payloadIndex + i * ZCL_PRICE_CLUSTER_BLOCK_THRESHOLDS_PAYLOAD_SIZE],
                                     ZCL_PRICE_CLUSTER_BLOCK_THRESHOLDS_PAYLOAD_SIZE,
                                     false);
      emberAfPriceClusterPrintln("");
    }
  } else if ((subpayloadControl & 0x01) == 0x00) {
    // tier/numberofblockthresholds
    // least sig nibble - number of thresholds
    // most  sig nibble - tier
    while (payloadIndex < payloadLength) {
      uint8_t tier = (payload[payloadIndex] & 0xF0) >> 4;
      uint8_t numberOfBt = (payload[payloadIndex] & 0x0F);
      uint8_t i;
      payloadIndex++;

      // do the block thresholds data exist in the payloads?
      if (payloadIndex
          + numberOfBt * ZCL_PRICE_CLUSTER_BLOCK_THRESHOLDS_PAYLOAD_SIZE
          > payloadLength) {
        emberAfPriceClusterPrintln("ERR: Invalid number of thresholds(%d) within BlockThresholds payload!",
                                   numberOfBt);
        status = false;
        goto kickout;
      }

      if (tier > ZCL_PRICE_CLUSTER_MAX_TOU_BLOCK_TIERS) {
        emberAfPriceClusterPrintln("ERR: Invalid tier id within BlockThresholds payload!");
        status = false;
        goto kickout;
      }

      if (numberOfBt > ZCL_PRICE_CLUSTER_MAX_TOU_BLOCKS - 1) {
        emberAfPriceClusterPrintln("ERR: The number(%d) of block threshold is larger than the max (%d)!",
                                   numberOfBt,
                                   ZCL_PRICE_CLUSTER_MAX_TOU_BLOCKS - 1);
        status = false;
        goto kickout;
      }

      for (i = 0; i < numberOfBt; i++) {
        MEMCOPY(&bt.thresholds.blockAndTier[tier][i],
                &payload[payloadIndex + i * ZCL_PRICE_CLUSTER_BLOCK_THRESHOLDS_PAYLOAD_SIZE],
                ZCL_PRICE_CLUSTER_BLOCK_THRESHOLDS_PAYLOAD_SIZE);
        emberAfPriceClusterPrint("Info: Updating blockAndTier[%d][%d] = 0x",
                                 tier,
                                 i);
        emberAfPriceClusterPrintBuffer(&payload[payloadIndex + i * ZCL_PRICE_CLUSTER_BLOCK_THRESHOLDS_PAYLOAD_SIZE],
                                       ZCL_PRICE_CLUSTER_BLOCK_THRESHOLDS_PAYLOAD_SIZE,
                                       false);
        emberAfPriceClusterPrintln("");
      }
      payloadIndex += numberOfBt * ZCL_PRICE_CLUSTER_BLOCK_THRESHOLDS_PAYLOAD_SIZE;
    }
  }

  kickout:
  if (status) {
    return emberAfPriceSetBlockThresholdsTableEntry(endpoint,
                                                    tableIndex,
                                                    &inf,
                                                    &bt);
  } else {
    return false;
  }
}

// Sets the block thresholds at the index. Returns false if the index is invalid.
bool emberAfPriceSetBlockThresholdsTableEntry(uint8_t endpoint,
                                              uint8_t index,
                                              const EmberAfPriceCommonInfo *info,
                                              const EmberAfScheduledBlockThresholds *bt)
{
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);
  EmberAfPriceCommonInfo * inf;
  EmberAfScheduledBlockThresholds * blockThresholds;
  if (ep == 0xFF) {
    return false;
  }

  if (index < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE) {
    inf = &priceServerInfo.scheduledBlockThresholdsTable.commonInfos[ep][index];
    blockThresholds = &priceServerInfo.scheduledBlockThresholdsTable.scheduledBlockThresholds[ep][index];

    if (bt == NULL || info == NULL) {
      inf->valid = false;
      blockThresholds->status = 0;
      return true;
    }

    MEMCOPY(inf,
            info,
            sizeof(EmberAfPriceCommonInfo));
    MEMCOPY(blockThresholds,
            bt,
            sizeof(EmberAfScheduledBlockThresholds));

    inf->valid = true;
    inf->actionsPending = true;
    inf->durationSec = ZCL_PRICE_CLUSTER_DURATION_SEC_UNTIL_CHANGED;
    if (inf->startTime == ZCL_PRICE_CLUSTER_START_TIME_NOW) {
      inf->startTime = emberAfGetCurrentTime();
    }

    if (blockThresholds->status == 0) {
      blockThresholds->status |= FUTURE;
    }

    emberAfPluginPriceCommonSort((EmberAfPriceCommonInfo *)&priceServerInfo.scheduledBlockThresholdsTable.commonInfos[ep],
                                 (uint8_t *)&priceServerInfo.scheduledBlockThresholdsTable.scheduledBlockThresholds[ep],
                                 sizeof(EmberAfScheduledBlockThresholds),
                                 EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE);
    emberAfPriceClusterScheduleTickCallback(endpoint,
                                            EMBER_AF_PRICE_SERVER_ACTIVATED_BLOCK_THRESHOLD_EVENT_MASK);

    return true;
  }
  return false;
}

void emberAfPricePrintTariff(const EmberAfPriceCommonInfo *info,
                             const EmberAfScheduledTariff *tariff)
{
  emberAfPriceClusterPrintln(" valid: %d", info->valid);
  emberAfPriceClusterPrint(" label: ");
  emberAfPriceClusterPrintString(tariff->tariffLabel);
  if (emberAfStringLength(tariff->tariffLabel) > 0) {
    emberAfPriceClusterPrint("(0x%X)", emberAfStringLength(tariff->tariffLabel));
  }
  emberAfPriceClusterPrint("\n uom/cur: 0x%X/0x%2X"
                           "\r\n pid/eid/etid: 0x%4X/0x%4X/0x%4X"
                           "\r\n st/tt: 0x%4X/0x%X",
                           tariff->unitOfMeasure,
                           tariff->currency,
                           tariff->providerId,
                           info->issuerEventId,
                           tariff->issuerTariffId,
                           info->startTime,
                           tariff->tariffTypeChargingScheme);
  emberAfPriceClusterFlush();
  emberAfPriceClusterPrint("\r\n ptu/btu: 0x%X/0x%X"
                           "\r\n ptd/sc/tbm: 0x%X/0x%4X/0x%X"
                           "\r\n btm/btd: 0x%4X/0x%4X",
                           tariff->numberOfPriceTiersInUse,
                           tariff->numberOfBlockThresholdsInUse,
                           tariff->priceTrailingDigit,
                           tariff->standingCharge,
                           tariff->tierBlockMode,
                           tariff->blockThresholdMultiplier,
                           tariff->blockThresholdDivisor);
  emberAfPriceClusterFlush();
  emberAfPriceClusterPrint("\n tariff is ");
  if (tariffIsCurrent(tariff)) {
    emberAfPriceClusterPrintln("current");
  } else if (tariffIsFuture(tariff)) {
    emberAfPriceClusterPrintln("future");
  } else {
    emberAfPriceClusterPrintln("invalid");
  }
  emberAfPriceClusterFlush();
}

void emberAfPricePrintPriceMatrix(uint8_t endpoint,
                                  const EmberAfPriceCommonInfo *info,
                                  const EmberAfScheduledPriceMatrix *pm)
{
  EmberAfScheduledTariff t;
  EmberAfPriceCommonInfo tariffInfo;
  bool found;
  uint8_t i, j, chargingScheme;

  found = emberAfPriceGetTariffByIssuerTariffId(endpoint,
                                                pm->issuerTariffId,
                                                &tariffInfo,
                                                &t);

  if (!found) {
    emberAfPriceClusterPrint("  No corresponding valid tariff found; price matrix cannot be printed.\n");
    emberAfPriceClusterPrint("  (NOTE: If printing from command line, be sure the tariff has been pushed to the tariff table.)\n");
    return;
  }

  chargingScheme = t.tariffTypeChargingScheme;

  emberAfPriceClusterPrint("  provider id: %4x\r\n", pm->providerId);
  emberAfPriceClusterPrint("  issuer event id: %4x\r\n", info->issuerEventId);
  emberAfPriceClusterPrint("  issuer tariff id: %4x\r\n", pm->issuerTariffId);
  emberAfPriceClusterPrint("  start time: %4x\r\n", info->startTime);

  emberAfPriceClusterFlush();

  emberAfPriceClusterPrint("  == matrix contents == \r\n");
  switch (chargingScheme >> 4) {
    case 0: // TOU only
      for (i = 0; i < t.numberOfPriceTiersInUse; i++) {
        emberAfPriceClusterPrint("  tier %d: %4x\r\n", i, pm->matrix.tier[i]);
      }
      break;
    case 1: // Block only
      for (i = 0; i < t.numberOfPriceTiersInUse; i++) {
        emberAfPriceClusterPrint("  tier %d (block 1): %4x\r\n", i, pm->matrix.blockAndTier[i][0]);
      }
      break;
    case 2: // TOU and Block
    case 3:
      for (i = 0; i < t.numberOfPriceTiersInUse; i++) {
        for (j = 0; j < t.numberOfBlockThresholdsInUse + 1; j++) {
          emberAfPriceClusterPrint("  tier %d block %d: %4x\r\n", i, j + 1, pm->matrix.blockAndTier[i][j]);
        }
      }
      break;
    default:
      emberAfPriceClusterPrint("  Invalid pricing scheme; no contents. \r\n");
      break;
  }

  emberAfPriceClusterPrint("  == end matrix contents == \r\n");
  emberAfPriceClusterFlush();
}

void emberAfPricePrintBlockThresholds(uint8_t endpoint,
                                      const EmberAfPriceCommonInfo *info,
                                      const EmberAfScheduledBlockThresholds *bt)
{
  EmberAfScheduledTariff t;
  EmberAfPriceCommonInfo newInfo;
  bool found;
  uint8_t i, j, tierBlockMode;

  found = emberAfPriceGetTariffByIssuerTariffId(endpoint,
                                                bt->issuerTariffId,
                                                &newInfo,
                                                &t);

  if (!found) {
    emberAfPriceClusterPrint("  No corresponding valid tariff found; block thresholds cannot be printed.\n");
    emberAfPriceClusterPrint("  (NOTE: If printing from command line, be sure the tariff has been pushed to the tariff table.)\n");
    return;
  }

  tierBlockMode = t.tierBlockMode;

  emberAfPriceClusterPrint("  provider id: %4x\r\n", bt->providerId);
  emberAfPriceClusterPrint("  issuer event id: %4x\r\n", info->issuerEventId);
  emberAfPriceClusterPrint("  issuer tariff id: %4x\r\n", bt->issuerTariffId);
  emberAfPriceClusterPrint("  start time: %4x\r\n", info->startTime);

  emberAfPriceClusterFlush();

  emberAfPriceClusterPrint("  == thresholds contents == \r\n");
  switch (tierBlockMode) {
    case 0: // ActiveBlock
    case 1: // ActiveBlockPriceTier
      for (j = 0; j < t.numberOfBlockThresholdsInUse; j++) {
        emberAfPriceClusterPrint("  block threshold %d: 0x%x%x%x%x%x%x\r\n", j,
                                 bt->thresholds.block[j][0],
                                 bt->thresholds.block[j][1],
                                 bt->thresholds.block[j][2],
                                 bt->thresholds.block[j][3],
                                 bt->thresholds.block[j][4],
                                 bt->thresholds.block[j][5]);
      }
      break;
    case 2: // ActiveBlockPriceTierThreshold
      for (i = 0; i < t.numberOfPriceTiersInUse; i++) {
        for (j = 0; j < t.numberOfBlockThresholdsInUse; j++) {
          emberAfPriceClusterPrint("  tier %d block threshold %d: 0x%x%x%x%x%x%x\r\n", i, j,
                                   bt->thresholds.blockAndTier[i][j][0],
                                   bt->thresholds.blockAndTier[i][j][1],
                                   bt->thresholds.blockAndTier[i][j][2],
                                   bt->thresholds.blockAndTier[i][j][3],
                                   bt->thresholds.blockAndTier[i][j][4],
                                   bt->thresholds.blockAndTier[i][j][5]);
        }
      }
      break;
    case 0xFF://Not used: in case of TOU tariff or Block tariff charging scheme only
      if ((t.tariffTypeChargingScheme & CHARGING_SCHEME_MASK) == 0x10) {//block tariff only
        for (j = 0; j < t.numberOfBlockThresholdsInUse; j++) {
          emberAfPriceClusterPrint("  block threshold %d: 0x%x%x%x%x%x%x\r\n", j,
                                   bt->thresholds.block[j][0],
                                   bt->thresholds.block[j][1],
                                   bt->thresholds.block[j][2],
                                   bt->thresholds.block[j][3],
                                   bt->thresholds.block[j][4],
                                   bt->thresholds.block[j][5]);
        }
      } else if ((t.tariffTypeChargingScheme & CHARGING_SCHEME_MASK) == 0x00) {//TOU tariff only
        for (i = 0; i < t.numberOfPriceTiersInUse; i++) {
          for (j = 0; j < t.numberOfBlockThresholdsInUse; j++) {
            emberAfPriceClusterPrint("  tier %d block threshold %d: 0x%x%x%x%x%x%x\r\n", i, j,
                                     bt->thresholds.blockAndTier[i][j][0],
                                     bt->thresholds.blockAndTier[i][j][1],
                                     bt->thresholds.blockAndTier[i][j][2],
                                     bt->thresholds.blockAndTier[i][j][3],
                                     bt->thresholds.blockAndTier[i][j][4],
                                     bt->thresholds.blockAndTier[i][j][5]);
          }
        }
      } else {
        emberAfPriceClusterPrint("invalid tariff charging scheme 0x%x for tierblockmode 0xFF", t.tariffTypeChargingScheme);
      }
      break;
    default:
      emberAfPriceClusterPrint("  Invalid tier block mode; no contents. \r\n");
      break;
  }

  emberAfPriceClusterPrint("  == end thresholds contents == \r\n");
  emberAfPriceClusterFlush();
}

void emberAfPricePrintTariffTable(uint8_t endpoint)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_PRICE_CLUSTER)
  uint8_t i;
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return;
  }

  emberAfPriceClusterFlush();
  emberAfPriceClusterPrintln("Tariff Table Contents: ");
  emberAfPriceClusterFlush();
  emberAfPriceClusterPrintln("  Note: ALL values given in HEX\r\n");
  emberAfPriceClusterFlush();
  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    emberAfPriceClusterPrintln("= TARIFF %d =",
                               i);
    emberAfPricePrintTariff(&priceServerInfo.scheduledTariffTable.commonInfos[ep][i],
                            &priceServerInfo.scheduledTariffTable.scheduledTariffs[ep][i]);
  }
#endif // defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_PRICE_CLUSTER)
}

void emberAfPricePrintPriceMatrixTable(uint8_t endpoint)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_PRICE_CLUSTER)
  uint8_t i;
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return;
  }

  emberAfPriceClusterFlush();
  emberAfPriceClusterPrintln("Price Matrix Table Contents: ");
  emberAfPriceClusterFlush();
  emberAfPriceClusterPrintln("  Note: ALL values given in HEX (except indices)\r\n");
  emberAfPriceClusterFlush();
  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    emberAfPriceClusterPrintln("=PRICE MATRIX %x =",
                               i);
    emberAfPricePrintPriceMatrix(endpoint,
                                 &priceServerInfo.scheduledPriceMatrixTable.commonInfos[ep][i],
                                 &priceServerInfo.scheduledPriceMatrixTable.scheduledPriceMatrix[ep][i]);
  }

#endif // defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_PRICE_CLUSTER)
}

void emberAfPricePrintBlockThresholdsTable(uint8_t endpoint)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_PRICE_CLUSTER)
  uint8_t i;
  uint8_t ep = emberAfFindClusterServerEndpointIndex(endpoint, ZCL_PRICE_CLUSTER_ID);

  if (ep == 0xFF) {
    return;
  }

  emberAfPriceClusterFlush();
  emberAfPriceClusterPrintln("Block Thresholds Table Contents: ");
  emberAfPriceClusterFlush();
  emberAfPriceClusterPrintln("  Note: ALL values given in HEX (except indices)\r\n");
  emberAfPriceClusterFlush();
  for (i = 0; i < EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE; i++) {
    emberAfPriceClusterPrintln("=BLOCK THRESHOLDS %x =",
                               i);
    emberAfPricePrintBlockThresholds(endpoint,
                                     &priceServerInfo.scheduledBlockThresholdsTable.commonInfos[ep][i],
                                     &priceServerInfo.scheduledBlockThresholdsTable.scheduledBlockThresholds[ep][i]);
  }

#endif // defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_PRICE_CLUSTER)
}

bool emberAfPriceClusterGetTariffInformationCallback(uint32_t earliestStartTime,
                                                     uint32_t minIssuerEventId,
                                                     uint8_t numberOfCommands,
                                                     uint8_t tariffType)
{
  uint8_t validCmds[EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];
  uint8_t ep = emberAfFindClusterServerEndpointIndex(emberAfCurrentEndpoint(), ZCL_PRICE_CLUSTER_ID);
  uint8_t entriesCount;
  uint8_t validEntriesCount;

  emberAfDebugPrintln("RX: GetTariffInformation, 0x%4X, 0x%4X, 0x%X, 0x%X",
                      earliestStartTime,
                      minIssuerEventId,
                      numberOfCommands,
                      tariffType);

  if (ep == 0xFF) {
    emberAfPriceClusterPrintln("ERR: Unable to find endpoint (%d)!", ep);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_VALUE);
    return true;
  }

  entriesCount = emberAfPluginPriceCommonFindValidEntries(validCmds,
                                                          EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE,
                                                          (EmberAfPriceCommonInfo *)priceServerInfo.scheduledTariffTable.commonInfos[ep],
                                                          earliestStartTime,
                                                          minIssuerEventId,
                                                          numberOfCommands);
  validEntriesCount = entriesCount;

  // eliminate commands with mismatching tariffType
  // upper nibble is reserved. we'll ignore them.
  {
    uint8_t i;
    for (i = 0; i < entriesCount; i++) {
      uint8_t index = validCmds[i];
      if ((priceServerInfo.scheduledTariffTable.scheduledTariffs[ep][index].tariffTypeChargingScheme & TARIFF_TYPE_MASK) != (tariffType & TARIFF_TYPE_MASK)) {
        validCmds[i] = ZCL_PRICE_INVALID_INDEX;
        validEntriesCount--;
      }
    }
  }

  emberAfDebugPrintln("Tariffs found: %d", validEntriesCount);
  sendValidCmdEntries(ZCL_PUBLISH_TARIFF_INFORMATION_COMMAND_ID,
                      ep,
                      validCmds,
                      entriesCount);

  return true;
}

bool emberAfPriceClusterGetPriceMatrixCallback(uint32_t issuerTariffId)
{
  EmberAfScheduledTariff tariff;
  EmberAfPriceCommonInfo tariffInfo;
  EmberAfScheduledPriceMatrix pm;
  EmberAfPriceCommonInfo pmInfo;
  bool found;
  uint8_t endpoint = emberAfCurrentEndpoint(), i, j, payloadControl;
  uint16_t size = 0;
  // Allocate for the largest possible size, unfortunately
  uint8_t subPayload[ZCL_PRICE_CLUSTER_MAX_TOU_BLOCKS
                     * ZCL_PRICE_CLUSTER_MAX_TOU_BLOCK_TIERS
                     * ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE];

  // A price matrix must have an associated tariff, otherwise it is meaningless
  found = emberAfPriceGetTariffByIssuerTariffId(endpoint,
                                                issuerTariffId,
                                                &tariffInfo,
                                                &tariff);

  if (!found) {
    emberAfDebugPrintln("GetPriceMatrix: no corresponding tariff for id 0x%4x found",
                        issuerTariffId);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return true;
  }

  // Grab the actual price matrix
  found = emberAfPriceGetPriceMatrixByIssuerTariffId(endpoint,
                                                     issuerTariffId,
                                                     &pmInfo,
                                                     &pm);

  if (!found) {
    emberAfDebugPrintln("GetPriceMatrix: no corresponding price matrix for id 0x%4x found",
                        issuerTariffId);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return true;
  }

  // The structure of the price matrix will vary depending on the type of the tariff
  switch (tariff.tariffTypeChargingScheme >> 4) {
    case 0: // TOU only
      payloadControl = 1;
      for (i = 0; i < tariff.numberOfPriceTiersInUse; i++) {
        subPayload[i * ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE] = i + 1;
        emberAfCopyInt32u(subPayload,
                          i * ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE + 1,
                          pm.matrix.tier[i]);
        size += ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE;
      }
      break;
    case 1: // Block only
      payloadControl = 0;
      for (i = 0; i < tariff.numberOfPriceTiersInUse; i++) {
        subPayload[i * ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE] = i << 4;
        emberAfCopyInt32u(subPayload,
                          i * ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE + 1,
                          pm.matrix.blockAndTier[i][0]);
        size += ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE;
      }
      break;
    case 2:
    case 3: // TOU / Block combined
      payloadControl = 0;
      for (i = 0; i < tariff.numberOfPriceTiersInUse; i++) {
        for (j = 0; j < tariff.numberOfBlockThresholdsInUse + 1; j++) {
          subPayload[size] = (i << 4) | j;
          emberAfCopyInt32u(subPayload,
                            size + 1,
                            pm.matrix.blockAndTier[i][j]);
          size += ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE;
        }
      }
      break;
    default:
      emberAfDebugPrintln("GetPriceMatrix: invalid tariff type / charging scheme");
      emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_VALUE);
      return true;
  }

  // Populate and send the PublishPriceMatrix command
  emberAfDebugPrintln("GetPriceMatrix: subpayload size 0x%2x", size);

#ifdef EMBER_AF_GBCS_COMPATIBLE
  // GBCS explicitly lists some commands that need to be sent with "disable
  // default response" flag set. This is one of them.
  // We make it conditional on GBCS so it does not affect standard SE apps.
  emberAfSetDisableDefaultResponse(EMBER_AF_DISABLE_DEFAULT_RESPONSE_ONE_SHOT);
#endif

  emberAfFillCommandPriceClusterPublishPriceMatrix(pm.providerId,
                                                   pmInfo.issuerEventId,
                                                   pmInfo.startTime,
                                                   pm.issuerTariffId,
                                                   0,
                                                   1,
                                                   payloadControl,
                                                   subPayload,
                                                   size);
  emberAfSendResponse();

  return true;
}

static void emberAfPutPriceBlockThresholdInResp(emAfPriceBlockThreshold *threshold)
{
  uint16_t length = sizeof(emAfPriceBlockThreshold);

#if BIGENDIAN_CPU
  int8_t loc  = length - 1;
  int8_t end  = -1;
  int8_t incr = -1;
#else
  int8_t loc  = 0;
  int8_t end  = length;
  int8_t incr = 1;
#endif

  while ( loc != end ) {
    emberAfPutInt8uInResp(((uint8_t *)threshold)[loc]);
    loc += incr;
  }
}

bool emberAfPriceClusterGetBlockThresholdsCallback(uint32_t issuerTariffId)
{
  EmberAfPriceCommonInfo tariffInfo;
  EmberAfScheduledTariff tariff;
  EmberAfPriceCommonInfo btInfo;
  EmberAfScheduledBlockThresholds bt;
  bool found;
  uint8_t endpoint = emberAfCurrentEndpoint(), i, j;
  uint16_t size = 0;

  // Block thresholds must have an associated tariff, otherwise it is meaningless
  found = emberAfPriceGetTariffByIssuerTariffId(endpoint,
                                                issuerTariffId,
                                                &tariffInfo,
                                                &tariff);

  if (!found) {
    emberAfDebugPrintln("GetBlockThresholds: no corresponding tariff for id 0x%4x found",
                        issuerTariffId);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return true;
  }

  // Grab the actual block thresholds
  found = emberAfPriceGetBlockThresholdsByIssuerTariffId(endpoint,
                                                         issuerTariffId,
                                                         &btInfo,
                                                         &bt);

  if (!found) {
    emberAfDebugPrintln("GetBlockThresholds: no corresponding block thresholds for id 0x%4x found",
                        issuerTariffId);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_NOT_FOUND);
    return true;
  }

  // Populate and send the PublishBlockThresholds command
  emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                             | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT),
                            ZCL_PRICE_CLUSTER_ID,
                            ZCL_PUBLISH_BLOCK_THRESHOLDS_COMMAND_ID,
                            "wwwwuu",
                            bt.providerId,
                            btInfo.issuerEventId,
                            btInfo.startTime,
                            bt.issuerTariffId,
                            0,
                            1);

  // The structure of the block thresholds subpayload will vary depending on the tier block mode
  switch (tariff.tierBlockMode) {
    case 0: // ActiveBlock
    case 1: // ActiveBlockPriceTier
      emberAfPutInt8uInResp(1); // payload control
      emberAfPutInt8uInResp(tariff.numberOfBlockThresholdsInUse);
      size += 1;
      for (j = 0; j < tariff.numberOfBlockThresholdsInUse; j++) {
        emberAfPutPriceBlockThresholdInResp(&bt.thresholds.block[j]);
        size += 6;
      }
      break;
    case 2: // ActiveBlockPriceTierThreshold
      emberAfPutInt8uInResp(0); // payload control
      for (i = 0; i < tariff.numberOfPriceTiersInUse; i++) {
        emberAfPutInt8uInResp((i << 4) | tariff.numberOfBlockThresholdsInUse);
        size += 1;
        for (j = 0; j < tariff.numberOfBlockThresholdsInUse; j++) {
          emberAfPutPriceBlockThresholdInResp(&bt.thresholds.blockAndTier[i][j]);
          size += 6;
        }
      }
      break;
    case 0xFF://Not used: in case of TOU tariff or Block tariff charging scheme only
      if ((tariff.tariffTypeChargingScheme & CHARGING_SCHEME_MASK) == 0x10) {//block tariff only
        emberAfPutInt8uInResp(1); // payload control
        emberAfPutInt8uInResp(tariff.numberOfBlockThresholdsInUse);
        size += 1;
        for (j = 0; j < tariff.numberOfBlockThresholdsInUse; j++) {
          emberAfPutPriceBlockThresholdInResp(&bt.thresholds.block[j]);
          size += 6;
        }
      } else if ((tariff.tariffTypeChargingScheme & CHARGING_SCHEME_MASK) == 0x00) {//TOU tariff only
        emberAfPutInt8uInResp(0); // payload control
        for (i = 0; i < tariff.numberOfPriceTiersInUse; i++) {
          emberAfPutInt8uInResp((i << 4) | tariff.numberOfBlockThresholdsInUse);
          size += 1;
          for (j = 0; j < tariff.numberOfBlockThresholdsInUse; j++) {
            emberAfPutPriceBlockThresholdInResp(&bt.thresholds.blockAndTier[i][j]);
            size += 6;
          }
        }
      } else {
        emberAfDebugPrintln("GetBlockThresholds: invalid tariff charging scheme 0x%x for tierblockmode 0xFF",
                            tariff.tariffTypeChargingScheme);
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_VALUE);
        return true;
      }
      break;
    default:
      emberAfDebugPrintln("GetBlockThresholds: invalid tier block mode");
      emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_VALUE);
      return true;
  }

  emberAfDebugPrintln("GetBlockThresholds: subpayload size 0x%2x", size);
  emberAfSendResponse();

  return true;
}
#else // !defined(EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_MATRIX_SUPPORT)

void emberAfPriceClearTariffTable(uint8_t endpoint)
{
}

void emberAfPriceClearPriceMatrixTable(uint8_t endpoint)
{
}

void emberAfPriceClearBlockThresholdsTable(uint8_t endpoint)
{
}

bool emberAfPriceSetTariffTableEntry(uint8_t endpoint,
                                     uint8_t index,
                                     EmberAfPriceCommonInfo *info,
                                     const EmberAfScheduledTariff *tariff)
{
  return false;
}

bool emberAfPriceClusterGetTariffInformationCallback(uint32_t earliestStartTime,
                                                     uint32_t minIssuerEventId,
                                                     uint8_t numberOfCommands,
                                                     uint8_t tariffType)
{
  return false;
}

bool emberAfPriceClusterGetPriceMatrixCallback(uint32_t issuerTariffId)
{
  return false;
}

bool emberAfPriceClusterGetBlockThresholdsCallback(uint32_t issuerTariffId)
{
  return false;
}

bool emberAfPriceGetTariffTableEntry(uint8_t endpoint,
                                     uint8_t index,
                                     EmberAfPriceCommonInfo *info,
                                     EmberAfScheduledTariff *tariff)
{
  return false;
}

bool emberAfPriceAddTariffTableEntry(uint8_t endpoint,
                                     EmberAfPriceCommonInfo *info,
                                     const EmberAfScheduledTariff *curTariff)
{
  return false;
}

bool emberAfPriceGetPriceMatrix(uint8_t endpoint,
                                uint8_t index,
                                EmberAfPriceCommonInfo *info,
                                EmberAfScheduledPriceMatrix *pm)
{
  return false;
}

bool emberAfPriceGetBlockThresholdsTableEntry(uint8_t endpoint,
                                              uint8_t index,
                                              EmberAfScheduledBlockThresholds *bt)
{
  return false;
}

bool emberAfPriceGetTariffByIssuerTariffId(uint8_t endpoint,
                                           uint32_t issuerTariffId,
                                           EmberAfPriceCommonInfo *info,
                                           EmberAfScheduledTariff *tariff)
{
  return false;
}

bool emberAfPriceGetPriceMatrixByIssuerTariffId(uint8_t endpoint,
                                                uint32_t issuerTariffId,
                                                EmberAfPriceCommonInfo *info,
                                                EmberAfScheduledPriceMatrix *pm)
{
  return false;
}

bool emberAfPriceGetBlockThresholdsByIssuerTariffId(uint8_t endpoint,
                                                    uint32_t issuerTariffId,
                                                    EmberAfPriceCommonInfo *info,
                                                    EmberAfScheduledBlockThresholds *bt)
{
  return false;
}

bool emberAfPriceSetPriceMatrix(uint8_t endpoint,
                                uint8_t index,
                                EmberAfPriceCommonInfo *info,
                                const EmberAfScheduledPriceMatrix *pm)
{
  return false;
}

bool emberAfPriceSetBlockThresholdsTableEntry(uint8_t endpoint,
                                              uint8_t index,
                                              const EmberAfPriceCommonInfo *info,
                                              const EmberAfScheduledBlockThresholds *bt)
{
  return false;
}

void emberAfPricePrintTariff(const EmberAfPriceCommonInfo *info,
                             const EmberAfScheduledTariff *tariff)
{
}
void emberAfPricePrintPriceMatrix(uint8_t endpoint,
                                  const EmberAfPriceCommonInfo *info,
                                  const EmberAfScheduledPriceMatrix *pm)
{
}
void emberAfPricePrintBlockThresholds(uint8_t endpoint,
                                      const EmberAfPriceCommonInfo *info,
                                      const EmberAfScheduledBlockThresholds *bt)
{
}
void emberAfPricePrintTariffTable(uint8_t endpoint)
{
}
void emberAfPricePrintPriceMatrixTable(uint8_t endpoint)
{
}
void emberAfPricePrintBlockThresholdsTable(uint8_t endpoint)
{
}
bool emberAfPriceAddBlockThresholdsTableEntry(uint8_t endpoint,
                                              uint32_t providerId,
                                              uint32_t issuerEventId,
                                              uint32_t startTime,
                                              uint32_t issuerTariffId,
                                              uint8_t commandIndex,
                                              uint8_t numberOfCommands,
                                              uint8_t subpayloadControl,
                                              uint8_t* payload)
{
  return false;
}

bool emberAfPriceAddPriceMatrixRaw(uint8_t endpoint,
                                   uint32_t providerId,
                                   uint32_t issuerEventId,
                                   uint32_t startTime,
                                   uint32_t issuerTariffId,
                                   uint8_t commandIndex,
                                   uint8_t numberOfCommands,
                                   uint8_t subPayloadControl,
                                   uint8_t* payload)
{
  return false;
}

#endif // EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_MATRIX_SUPPORT
