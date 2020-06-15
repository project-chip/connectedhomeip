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
 * @brief Matrix CLI for the Price Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/price-server/price-server.h"

#if defined(EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_MATRIX_SUPPORT)

//=============================================================================
// Globals

static EmberAfPriceCommonInfo          scheduledTariffInfo;
static EmberAfScheduledTariff          scheduledTariff;
static EmberAfPriceCommonInfo          btInfo;
static EmberAfScheduledBlockThresholds bt;
static EmberAfPriceCommonInfo          pmInfo;
static EmberAfScheduledPriceMatrix     pm;

//=============================================================================
// Functions

// plugin price-server tclear <endpoint:1>
void emAfPriceServerCliTClear(void)
{
  emberAfPriceClearTariffTable(emberUnsignedCommandArgument(0));
}

// plugin price-server pmclear <endpoint:1>
void emAfPriceServerCliPmClear(void)
{
  emberAfPriceClearPriceMatrixTable(emberUnsignedCommandArgument(0));
}

// plugin price-server btclear <endpoint:1>
void emAfPriceServerCliBtClear(void)
{
  emberAfPriceClearBlockThresholdsTable(emberUnsignedCommandArgument(0));
}

// plugin price-server twho <provId:4> <label:1-13> <eventId:4> <tariffId:4>
void emAfPriceServerCliTWho(void)
{
  uint8_t length;
  scheduledTariff.providerId = emberUnsignedCommandArgument(0);
  length = emberCopyStringArgument(1,
                                   scheduledTariff.tariffLabel + 1,
                                   ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH,
                                   false);
  scheduledTariff.tariffLabel[0] = length;
  scheduledTariffInfo.issuerEventId = (uint32_t)emberUnsignedCommandArgument(2);
  scheduledTariff.issuerTariffId = (uint32_t)emberUnsignedCommandArgument(3);
}

// plugin price-server twhat <type:1> <unitOfMeas:1> <curr:2> <ptd:1> <prt:1> <btu:1> <blockMode:1>
void emAfPriceServerCliTWhat(void)
{
  scheduledTariff.tariffTypeChargingScheme = (uint8_t)emberUnsignedCommandArgument(0);
  scheduledTariff.unitOfMeasure = (uint8_t)emberUnsignedCommandArgument(1);
  scheduledTariff.currency = (uint16_t)emberUnsignedCommandArgument(2);
  scheduledTariff.priceTrailingDigit = (uint8_t)emberUnsignedCommandArgument(3);
  scheduledTariff.numberOfPriceTiersInUse = (uint8_t)emberUnsignedCommandArgument(4);
  scheduledTariff.numberOfBlockThresholdsInUse = (uint8_t)emberUnsignedCommandArgument(5);
  scheduledTariff.tierBlockMode = (uint8_t)emberUnsignedCommandArgument(6);
}

// plugin price-server twhen <startTime:4>
void emAfPriceServerCliTWhen(void)
{
  scheduledTariffInfo.startTime = emberUnsignedCommandArgument(0);
  scheduledTariffInfo.durationSec = ZCL_PRICE_CLUSTER_DURATION_SEC_UNTIL_CHANGED;
}

// plugin price-server tariff <standingCharge:4> <btm:4> <btd:4>
void emAfPriceServerCliTariff(void)
{
  scheduledTariff.standingCharge = emberUnsignedCommandArgument(0);
  scheduledTariff.blockThresholdMultiplier = emberUnsignedCommandArgument(1);
  scheduledTariff.blockThresholdDivisor = emberUnsignedCommandArgument(2);
}

// plugin price-server tadd <endpoint:1> <status:1>
void emAfPriceServerCliTAdd(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t status = (uint8_t)emberUnsignedCommandArgument(1);

  if (status == 0) {
    scheduledTariff.status |= CURRENT;
  } else {
    scheduledTariff.status |= FUTURE;
  }

  if (!emberAfPriceAddTariffTableEntry(endpoint,
                                       &scheduledTariffInfo,
                                       &scheduledTariff)) {
    emberAfPriceClusterPrintln("ERR:Failed to set table entry!");
  }
}

// plugin price-server tget <endpoint:1> <index:1>
void emAfPriceServerCliTGet(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(1);
  if (!emberAfPriceGetTariffTableEntry(endpoint,
                                       index,
                                       &scheduledTariffInfo,
                                       &scheduledTariff)) {
    emberAfPriceClusterPrintln("tariff entry %d not present", index);
  }
}

// plugin price-server tset <endpoint:1> <index:1> <status:1>
void emAfPriceServerCliTSet(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t status = (uint8_t)emberUnsignedCommandArgument(2);

  scheduledTariffInfo.valid = true;
  scheduledTariff.status = status;

  if (!emberAfPriceSetTariffTableEntry(endpoint,
                                       index,
                                       &scheduledTariffInfo,
                                       &scheduledTariff)) {
    emberAfPriceClusterPrintln("tariff entry %d not present", index);
  }
}

// plugin price-server tprint <endpoint:1>
void emAfPriceServerCliTPrint(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfPricePrintTariffTable(endpoint);
}

// plugin price-server tsprint
void emAfPriceServerCliTSPrint(void)
{
  emberAfPricePrintTariff(&scheduledTariffInfo, &scheduledTariff);
}

// plugin price-server pmprint <endpoint:1>
void emAfPriceServerCliPmPrint(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfPricePrintPriceMatrixTable(endpoint);
}

// plugin price-server pm set-metadata <endpoint:1> <providerId:4> <issuerEventId:4>
//                                     <issuerTariffId:4> <startTime:4> <status:1>
void emAfPriceServerCliPmSetMetadata(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint32_t tariffId = (uint32_t)emberUnsignedCommandArgument(3);
  EmberAfScheduledTariff t;
  EmberAfPriceCommonInfo i;

  if (!(emberAfPriceGetTariffByIssuerTariffId(endpoint, tariffId, &i, &t)
        || scheduledTariff.issuerTariffId == tariffId)) {
    emberAfPriceClusterPrint("Invalid issuer tariff ID; no corresponding tariff found.\n");
    return;
  }

  pm.providerId = (uint32_t)emberUnsignedCommandArgument(1);
  pmInfo.issuerEventId = (uint32_t)emberUnsignedCommandArgument(2);
  pm.issuerTariffId = tariffId;
  pmInfo.startTime = (uint32_t)emberUnsignedCommandArgument(4);
  pm.status = (uint8_t)emberUnsignedCommandArgument(5);
}

// plugin price-server pm set-provider <providerId:4>
void emAfPriceServerCliPmSetProvider(void)
{
  pm.providerId = (uint32_t)emberUnsignedCommandArgument(0);
}

// plugin price-server pm set-event <issuerEventId:4>
void emAfPriceServerCliPmSetEvent(void)
{
  pmInfo.issuerEventId = (uint32_t)emberUnsignedCommandArgument(0);
}

// plugin price-server pm set-tariff <issuerTariffId:4>
void emAfPriceServerCliPmSetTariff(void)
{
  pm.issuerTariffId = (uint32_t)emberUnsignedCommandArgument(0);
}

// plugin price-server pm set-time <startTime:4>
void emAfPriceServerCliPmSetTime(void)
{
  pmInfo.startTime = (uint32_t)emberUnsignedCommandArgument(0);
}

// plugin price-server pm set-status <status:1>
void emAfPriceServerCliPmSetStatus(void)
{
  pm.status = (uint8_t)emberUnsignedCommandArgument(0);
}

// plugin price-server pm get <endpoint:1> <index:1>
void emAfPriceServerCliPmGet(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(1);
  emberAfPriceGetPriceMatrix(endpoint, index, &pmInfo, &pm);
}

// plugin price-server pm add <endpoint:1>
void emAfPriceServerCliPmAdd(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  EmberAfScheduledTariff t;
  EmberAfPriceCommonInfo i;

  if (!(emberAfPriceGetTariffByIssuerTariffId(endpoint, pm.issuerTariffId, &i, &t)
        || scheduledTariff.issuerTariffId == pm.issuerTariffId)) {
    emberAfPriceClusterPrint("Invalid issuer tariff ID; no corresponding tariff found.\n");
    return;
  }

  emberAfPriceAddPriceMatrix(endpoint, &pmInfo, &pm);
}

// plugin price-server pm put <endpoint:1> <tier:1> <block:1> <price:4>
void emAfPriceServerCliPmPut(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t tier = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t block = (uint8_t)emberUnsignedCommandArgument(2);
  uint32_t price = (uint32_t)emberUnsignedCommandArgument(3);
  uint8_t chargingScheme;
  EmberAfScheduledTariff t;
  EmberAfPriceCommonInfo i;

  if (pm.issuerTariffId == scheduledTariff.issuerTariffId) {
    t = scheduledTariff;
    chargingScheme = scheduledTariff.tariffTypeChargingScheme;
  } else {
    bool found = emberAfPriceGetTariffByIssuerTariffId(endpoint, pm.issuerTariffId, &i, &t);
    if (!found) {
      emberAfPriceClusterPrint("Invalid issuer tariff ID in price matrix; no corresponding tariff found.\n");
      return;
    } else {
      chargingScheme = t.tariffTypeChargingScheme;
    }
  }

  if (tier >= t.numberOfPriceTiersInUse
      || block > t.numberOfBlockThresholdsInUse) {
    emberAfPriceClusterPrint("Invalid index into price matrix. Value not set.\n");
    return;
  }

  switch (chargingScheme >> 4) {
    case 0: // TOU only
      pm.matrix.tier[tier] = price;
      break;
    case 1: // Block only
      pm.matrix.blockAndTier[tier][0] = price;
      break;
    case 2:
    case 3: // TOU and Block
      pm.matrix.blockAndTier[tier][block] = price;
      break;
    default:
      emberAfDebugPrintln("Invalid tariff type / charging scheme.");
      break;
  }
}

// plugin price-server pm fill-tier <endpoint:1> <tier:1> <price:4>
void emAfPriceServerCliPmFillTier(void)
{
  uint8_t chargingScheme, i;
  EmberAfScheduledTariff t;
  EmberAfPriceCommonInfo inf;
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t tier = (uint8_t)emberUnsignedCommandArgument(1);
  uint32_t price = (uint32_t)emberUnsignedCommandArgument(2);

  if (pm.issuerTariffId == scheduledTariff.issuerTariffId) {
    t = scheduledTariff;
    chargingScheme = scheduledTariff.tariffTypeChargingScheme;
  } else {
    bool found = emberAfPriceGetTariffByIssuerTariffId(endpoint, pm.issuerTariffId, &inf, &t);
    if (!found) {
      emberAfPriceClusterPrint("Invalid issuer tariff ID in price matrix; no corresponding tariff found.\n");
      return;
    } else {
      chargingScheme = t.tariffTypeChargingScheme;
    }
  }

  if (tier >= t.numberOfPriceTiersInUse) {
    emberAfPriceClusterPrint("Tier exceeds number of price tiers in use for this tariff. Values not set.\n");
    return;
  }

  switch (chargingScheme >> 4) {
    case 0: // TOU only
      pm.matrix.tier[tier] = price;
      break;
    case 1: // Block only
      pm.matrix.blockAndTier[tier][0] = price;
      break;
    case 2:
    case 3: // TOU and Block
      for (i = 0; i < t.numberOfBlockThresholdsInUse + 1; i++) {
        pm.matrix.blockAndTier[tier][i] = price;
      }
      break;
    default:
      emberAfDebugPrintln("Invalid tariff type / charging scheme.");
      break;
  }
}

// plugin price-server pm fill-tier <endpoint:1> <block:1> <price:4>
void emAfPriceServerCliPmFillBlock(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t block = (uint8_t)emberUnsignedCommandArgument(1);
  uint32_t price = (uint32_t)emberUnsignedCommandArgument(2);
  uint8_t chargingScheme, i;
  EmberAfScheduledTariff t;
  EmberAfPriceCommonInfo inf;

  if (pm.issuerTariffId == scheduledTariff.issuerTariffId) {
    t = scheduledTariff;
    chargingScheme = scheduledTariff.tariffTypeChargingScheme;
  } else {
    bool found = emberAfPriceGetTariffByIssuerTariffId(endpoint, pm.issuerTariffId, &inf, &t);
    if (!found) {
      emberAfPriceClusterPrint("Invalid issuer tariff ID in price matrix; no corresponding tariff found.\n");
      return;
    } else {
      chargingScheme = t.tariffTypeChargingScheme;
    }
  }

  if ( block > t.numberOfBlockThresholdsInUse) {
    emberAfPriceClusterPrint("Block exceeds number of block thresholds in use for this tariff. Values not set.\n");
    return;
  }

  switch (chargingScheme >> 4) {
    case 0: // TOU only
      for (i = 0; i < t.numberOfPriceTiersInUse; i++) {
        pm.matrix.tier[i] = price;
      }
      break;
    case 1: // Block only
      for (i = 0; i < t.numberOfPriceTiersInUse; i++) {
        pm.matrix.blockAndTier[i][0] = price;
      }
      break;
    case 2:
    case 3: // TOU and Block
      for (i = 0; i < t.numberOfPriceTiersInUse; i++) {
        pm.matrix.blockAndTier[i][block] = price;
      }
      break;
    default:
      emberAfDebugPrintln("Invalid tariff type / charging scheme.");
      break;
  }
}

// plugin price-server btprint <endpoint:1>
void emAfPriceServerCliBtPrint(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfPricePrintBlockThresholdsTable(endpoint);
}

// plugin price-server bt set-metadata <endpoint:1> <providerId:4> <issuerEventId:4>
//                                     <issuerTariffId:4> <startTime:4> <status:1>
void emAfPriceServerCliBtSetMetadata(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint32_t tariffId = (uint32_t)emberUnsignedCommandArgument(3);
  EmberAfScheduledTariff t;
  EmberAfPriceCommonInfo i;

  if (!(emberAfPriceGetTariffByIssuerTariffId(endpoint, tariffId, &i, &t)
        || scheduledTariff.issuerTariffId == tariffId)) {
    emberAfPriceClusterPrint("Invalid issuer tariff ID; no corresponding tariff found.\n");
    return;
  }

  btInfo.issuerEventId = (uint32_t)emberUnsignedCommandArgument(2);
  btInfo.startTime = (uint32_t)emberUnsignedCommandArgument(4);
  bt.issuerTariffId = tariffId;
  bt.providerId = (uint32_t)emberUnsignedCommandArgument(1);
  bt.issuerTariffId = tariffId;
  bt.status = (uint8_t)emberUnsignedCommandArgument(5);
}

// plugin price-server bt set-provider <providerId:4>
void emAfPriceServerCliBtSetProvider(void)
{
  bt.providerId = (uint32_t)emberUnsignedCommandArgument(0);
}

// plugin price-server bt set-event <issuerEventId:4>
void emAfPriceServerCliBtSetEvent(void)
{
  btInfo.issuerEventId = (uint32_t)emberUnsignedCommandArgument(0);
}

// plugin price-server bt set-tariff <issuerTariffId:4>
void emAfPriceServerCliBtSetTariff(void)
{
  bt.issuerTariffId = (uint32_t)emberUnsignedCommandArgument(0);
}

// plugin price-server bt set-time <startTime:4>
void emAfPriceServerCliBtSetTime(void)
{
  btInfo.startTime = (uint32_t)emberUnsignedCommandArgument(0);
}

// plugin price-server bt set-status <status:1>
void emAfPriceServerCliBtSetStatus(void)
{
  bt.status = (uint8_t)emberUnsignedCommandArgument(0);
}

// plugin price-server bt get <endpoint:1> <index:1>
void emAfPriceServerCliBtGet(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(1);

  emberAfPriceGetBlockThresholdsTableEntry(endpoint, index, &bt);
}

// plugin price-server bt set <endpoint:1> <index:1>
void emAfPriceServerCliBtSet(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(1);
  EmberAfScheduledTariff t;
  EmberAfPriceCommonInfo i;

  if (!emberAfPriceGetTariffByIssuerTariffId(endpoint, bt.issuerTariffId, &i, &t)) {
    emberAfPriceClusterPrint("Invalid issuer tariff ID; no corresponding tariff found.\n");
    return;
  }

  emberAfPriceSetBlockThresholdsTableEntry(endpoint, index, &btInfo, &bt);
  MEMSET(&btInfo, 0x00, sizeof(EmberAfPriceCommonInfo));
  MEMSET(&bt, 0x00, sizeof(EmberAfScheduledBlockThresholds));
}

// plugin price-server bt put <endpoint:1> <tier:1> <block:1> <threshold:6>
void emAfPriceServerCliBtPut(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t tier = (uint8_t)emberUnsignedCommandArgument(1);
  uint8_t block = (uint8_t)emberUnsignedCommandArgument(2);
  emAfPriceBlockThreshold threshold;
  uint8_t tierBlockMode;
  EmberAfScheduledTariff t;
  EmberAfPriceCommonInfo i;
  uint8_t *dst = NULL;
  emberCopyStringArgument(3, (uint8_t *)&threshold, sizeof(emAfPriceBlockThreshold), true);

  if (bt.issuerTariffId == scheduledTariff.issuerTariffId) {
    t = scheduledTariff;
    tierBlockMode = scheduledTariff.tierBlockMode;
  } else {
    bool found = emberAfPriceGetTariffByIssuerTariffId(endpoint, bt.issuerTariffId, &i, &t);
    if (!found) {
      emberAfPriceClusterPrintln("Invalid issuer tariff ID in block thresholds; no corresponding tariff found.");
      return;
    } else {
      tierBlockMode = t.tierBlockMode;
    }
  }

  if (tier >= t.numberOfPriceTiersInUse
      || block > t.numberOfBlockThresholdsInUse) {
    emberAfPriceClusterPrintln("Invalid index into block thresholds. Value not set.");
    return;
  }

  switch (tierBlockMode) {
    case 0: // ActiveBlock
    case 1: // ActiveBlockPriceTier
      dst = (uint8_t *)&bt.thresholds.block[block];
      break;
    case 2: // ActiveBlockPriceTierThreshold
      dst = (uint8_t *)&bt.thresholds.blockAndTier[tier][block];
      break;
    default:
      emberAfDebugPrintln("Invalid tier block mode.");
      break;
  }

  if (dst != NULL) {
#if BIGENDIAN_CPU
    MEMCOPY(dst, &threshold, sizeof(emAfPriceBlockThreshold));
#else
    uint8_t i;
    for (i = 0; i < sizeof(emAfPriceBlockThreshold); i++) {
      dst[i] = threshold[sizeof(emAfPriceBlockThreshold) - 1 - i];
    }
#endif
  }
}

#else // !defined(EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_MATRIX_SUPPORT)

#endif // defined(EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_MATRIX_SUPPORT)
