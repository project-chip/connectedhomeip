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
 * @brief APIs and defines for the Price Server plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_PRICE_SERVER_H
#define SILABS_PRICE_SERVER_H

#include "app/framework/plugin/price-common/price-common.h"

#ifndef EMBER_AF_PLUGIN_PRICE_SERVER_PRICE_TABLE_SIZE
  #define EMBER_AF_PLUGIN_PRICE_SERVER_PRICE_TABLE_SIZE (5)
#endif

#ifndef EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE
  #define EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE (2)
#endif

#ifndef EMBER_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE
  #define EMBER_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE (2)
#endif

#ifndef EMBER_AF_PLUGIN_PRICE_SERVER_MAX_TIERS_PER_TARIFF
  #define EMBER_AF_PLUGIN_PRICE_SERVER_MAX_TIERS_PER_TARIFF (2)
#endif

#ifndef EMBER_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE
  #define EMBER_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE (2)
#endif

#ifndef EMBER_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE
  #define EMBER_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE (2)
#endif

#ifndef EMBER_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE
  #define EMBER_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE (5)
#endif

#ifndef EMBER_AF_PLUGIN_PRICE_SERVER_CREDIT_PAYMENT_TABLE_SIZE
  #define EMBER_AF_PLUGIN_PRICE_SERVER_CREDIT_PAYMENT_TABLE_SIZE (5)
#endif

#ifndef EMBER_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE
  #define EMBER_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE (2)
#endif

#ifndef EMBER_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE
  #define EMBER_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE (2)
#endif

#ifndef EMBER_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE
  #define EMBER_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE (2)
#endif

#define ZCL_PRICE_CLUSTER_PRICE_ACKNOWLEDGEMENT_MASK 0x01
#define ZCL_PRICE_CLUSTER_RESERVED_MASK              0xFE
#define ZCL_PRICE_CLUSTER_BLOCK_THRESHOLDS_PAYLOAD_SIZE (6)
#define ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUBPAYLOAD_BLOCK_SIZE     (5)

#define fieldLength(field) \
  (emberAfCurrentCommand()->bufLen - (field - emberAfCurrentCommand()->buffer));

#define ZCL_PRICE_CLUSTER_MAX_TOU_BLOCKS 15
#define ZCL_PRICE_CLUSTER_MAX_TOU_BLOCK_TIERS 15
#define ZCL_PRICE_CLUSTER_MAX_TOU_TIERS 48
#define ZCL_PRICE_CLUSTER_PRICE_MATRIX_SUB_PAYLOAD_ENTRY_SIZE 5

// To help keep track of the status of the tariffs in the table
// (also, corresponding price matrices).
#define CURRENT       BIT(1)
#define FUTURE        BIT(2)
#define PUBLISHED     BIT(3)

#define TARIFF_TYPE_MASK (0x0F)
#define CHARGING_SCHEME_MASK (0xF0)

#define tariffIsCurrent(tariff) ((tariff)->status & CURRENT)
#define tariffIsFuture(tariff)  ((tariff)->status & FUTURE)
#define tariffIsPublished(tariff)  ((tariff)->status & PUBLISHED)
#define priceMatrixIsCurrent(pm) ((pm)->status & CURRENT)
#define priceMatrixIsFuture(pm) ((pm)->status & FUTURE)
#define priceMatrixIsPublished(pm) ((pm)->status & PUBLISHED)
#define blockThresholdsIsCurrent(bt) ((bt)->status & CURRENT)
#define blockThresholdsIsFuture(bt) ((bt) > status & FUTURE)
#define blockThresholdsIsPublished(bt) ((bt)->status & PUBLISHED)

/**
 * @brief The price and metadata used by the Price server plugin.
 *
 * The application can get and set the prices used by the plugin by calling
 * ::emberAfPriceGetPriceTableEntry and
 * ::emberAfPriceSetPriceTableEntry.
 */

/**
 * @brief conversion factor infos by the Price server plugin.
 *
 */

typedef struct {
  uint32_t providerId;
  uint32_t rawBlockPeriodStartTime;
  uint32_t blockPeriodDuration;
  // The "thresholdMultiplier" and "threadholdDivisor" are included in this stucture
  // since these should be specified with the block period.
  // These values are stored as the "Threshold Multiplier" and "Threshold Divisor"
  // attributes in the Block Period (Delivered) attribute set (D.4.2.2.3).
  uint32_t thresholdMultiplier;
  uint32_t thresholdDivisor;
  uint8_t  blockPeriodControl;
  uint8_t  blockPeriodDurationType;
  uint8_t  tariffType;
  uint8_t  tariffResolutionPeriod;
} EmberAfPriceBlockPeriod;

typedef struct {
  uint32_t providerId;
  uint32_t rawBillingPeriodStartTime;
  uint32_t billingPeriodDuration;
  uint8_t billingPeriodDurationType;
  uint8_t tariffType;
} EmberAfPriceBillingPeriod;

typedef struct {
  uint32_t providerId;
  uint32_t durationInMinutes;
  uint8_t  tariffType;
  uint8_t  cppPriceTier;
  uint8_t  cppAuth;
} EmberAfPriceCppEvent;

typedef struct {
  uint32_t providerId;
  uint32_t rawStartTimeUtc;   // start time as received from caller, prior to any adjustments
  uint32_t billingPeriodDuration;
  uint32_t consolidatedBill;
  uint16_t currency;
  uint8_t  billingPeriodDurationType;
  uint8_t  tariffType;
  uint8_t  billTrailingDigit;
} EmberAfPriceConsolidatedBills;

#define CREDIT_PAYMENT_REF_STRING_LEN 20
typedef struct {
  uint32_t providerId;
  uint32_t creditPaymentDueDate;
  uint32_t creditPaymentAmountOverdue;
  uint32_t creditPayment;
  uint32_t creditPaymentDate;
  uint8_t  creditPaymentStatus;
  uint8_t  creditPaymentRef[CREDIT_PAYMENT_REF_STRING_LEN + 1];
} EmberAfPriceCreditPayment;

typedef struct {
  uint32_t conversionFactor;
  uint8_t conversionFactorTrailingDigit;
} EmberAfPriceConversionFactor;

typedef struct {
  uint32_t calorificValue;
  uint8_t calorificValueUnit;
  uint8_t calorificValueTrailingDigit;
} EmberAfPriceCalorificValue;

typedef struct {
  uint32_t providerId;
  uint32_t issuerTariffId;
  uint8_t  tariffType;
  bool valid;
} EmberAfPriceCancelTariff;

typedef struct {
  uint32_t providerId;
  uint32_t co2Value;
  uint8_t tariffType;
  uint8_t co2ValueUnit;
  uint8_t co2ValueTrailingDigit;
} EmberAfPriceCo2Value;

typedef struct {
  uint32_t providerId;
  uint16_t oldCurrency;
  uint16_t newCurrency;
  uint32_t conversionFactor;
  uint8_t  conversionFactorTrailingDigit;
  uint32_t currencyChangeControlFlags;
} EmberAfPriceCurrencyConversion;

#define TIER_LABEL_SIZE  12
typedef struct {
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t issuerTariffId;
  uint8_t  valid;
  uint8_t  numberOfTiers;
  uint8_t  tierIds[EMBER_AF_PLUGIN_PRICE_SERVER_MAX_TIERS_PER_TARIFF];
  uint8_t  tierLabels[EMBER_AF_PLUGIN_PRICE_SERVER_MAX_TIERS_PER_TARIFF][TIER_LABEL_SIZE + 1];
} EmberAfPriceTierLabelValue;

typedef struct {
  EmberAfPriceTierLabelValue entry[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE];
//  uint8_t valid[EMBER_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE];
//  uint32_t providerId[EMBER_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE];
//  uint32_t issuerEventId[EMBER_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE];
//  uint32_t issuerTariffId[EMBER_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE];
//  uint8_t tierId[EMBER_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE];
//  uint8_t tierLabel[EMBER_AF_PLUGIN_PRICE_SERVER_TIER_LABELS_TABLE_SIZE][13];
} EmberAfPriceTierLabelTable;

typedef struct {
  EmberAfPriceCommonInfo commonInfos[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE];
  EmberAfPriceBlockPeriod blockPeriods[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_BLOCK_PERIOD_TABLE_SIZE];
} EmberAfPriceBlockPeriodTable;

typedef struct {
  EmberAfPriceCommonInfo commonInfos[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE];
  EmberAfPriceBillingPeriod billingPeriods[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_BILLING_PERIOD_TABLE_SIZE];
} EmberAfPriceBillingPeriodTable;

typedef struct {
  EmberAfPriceCommonInfo commonInfos[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT];
  EmberAfPriceCppEvent cppEvent[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT];
} EmberAfPriceCppTable;

typedef struct {
  EmberAfPriceCommonInfo commonInfos[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE];
  EmberAfPriceConsolidatedBills consolidatedBills[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_CONSOLIDATED_BILL_TABLE_SIZE];
} EmberAfPriceConsolidatedBillsTable;

typedef struct {
  EmberAfPriceCommonInfo commonInfos[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_CREDIT_PAYMENT_TABLE_SIZE];
  EmberAfPriceCreditPayment creditPayment[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_CREDIT_PAYMENT_TABLE_SIZE];
} EmberAfPriceCreditPaymentTable;

typedef struct {
  EmberAfPriceCommonInfo commonInfos[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE];
  EmberAfPriceConversionFactor priceConversionFactors[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_CONVERSION_FACTOR_TABLE_SIZE];
} EmberAfPriceConversionFactorTable;

typedef struct {
  EmberAfPriceCommonInfo commonInfos[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE];
  EmberAfPriceCalorificValue calorificValues[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_CALORIFIC_VALUE_TABLE_SIZE];
} EmberAfPriceCalorificValueTable;

typedef struct {
  EmberAfPriceCommonInfo commonInfos[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE];
  EmberAfPriceCo2Value co2Values[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_CO2_VALUE_TABLE_SIZE];
} EmberAfPriceCO2Table;

typedef struct {
  EmberAfPriceCommonInfo commonInfos[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT];
  EmberAfPriceCurrencyConversion currencyConversion[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT];
} EmberAfPriceCurrencyConversionTable;

typedef struct {
  EmberAfPriceCancelTariff cancelTariff[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT];
} EmberAfPriceCancelTariffTable;

typedef struct {
  uint8_t   rateLabel[ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH + 1];
  uint32_t  providerId;
  uint32_t  issuerEventID;
  uint32_t  startTime;
  uint32_t  price;
  uint32_t  generationPrice;
  uint32_t  alternateCostDelivered;
  uint16_t  currency;
  uint16_t  duration; // in minutes
  uint8_t   unitOfMeasure;
  uint8_t   priceTrailingDigitAndTier;
  uint8_t   numberOfPriceTiersAndTier; // added later in errata
  uint8_t   priceRatio;
  uint8_t   generationPriceRatio;
  uint8_t   alternateCostUnit;
  uint8_t   alternateCostTrailingDigit;
  uint8_t   numberOfBlockThresholds;
  uint8_t   priceControl;
} EmberAfScheduledPrice;

typedef uint8_t emAfPriceBlockThreshold[ZCL_PRICE_CLUSTER_BLOCK_THRESHOLDS_PAYLOAD_SIZE];
typedef struct {
  union {
    emAfPriceBlockThreshold blockAndTier[ZCL_PRICE_CLUSTER_MAX_TOU_BLOCK_TIERS][ZCL_PRICE_CLUSTER_MAX_TOU_BLOCKS - 1];
    emAfPriceBlockThreshold block[ZCL_PRICE_CLUSTER_MAX_TOU_BLOCKS - 1];
  } thresholds;
  uint32_t providerId;
  uint32_t issuerTariffId;
  uint8_t status;
} EmberAfScheduledBlockThresholds;

typedef struct {
  EmberAfPriceCommonInfo commonInfos[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];
  EmberAfScheduledBlockThresholds scheduledBlockThresholds[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];
} EmberAfScheduledBlockThresholdsTable;

typedef struct {
  uint32_t providerId;
  uint32_t issuerTariffId;
  uint8_t status;
  uint8_t tariffTypeChargingScheme;

  // below fields have corresponding zcl attributes.
  uint8_t  tariffLabel[ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH + 1];
  uint8_t  numberOfPriceTiersInUse;
  uint8_t  numberOfBlockThresholdsInUse;
  uint8_t  tierBlockMode;
  uint8_t  unitOfMeasure;
  uint16_t currency;
  uint8_t  priceTrailingDigit;
  uint32_t standingCharge;
  uint32_t blockThresholdMultiplier;
  uint32_t blockThresholdDivisor;
} EmberAfScheduledTariff;

typedef struct {
  EmberAfPriceCommonInfo commonInfos[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];
  EmberAfScheduledTariff scheduledTariffs[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];
} EmberAfScheduledTariffTable;

typedef struct {
  union {
    uint32_t blockAndTier[ZCL_PRICE_CLUSTER_MAX_TOU_BLOCK_TIERS][ZCL_PRICE_CLUSTER_MAX_TOU_BLOCKS];
    uint32_t tier[ZCL_PRICE_CLUSTER_MAX_TOU_TIERS];
  } matrix;
  uint32_t providerId;
  uint32_t issuerTariffId;
  uint8_t status;
} EmberAfScheduledPriceMatrix;

typedef struct {
  EmberAfPriceCommonInfo commonInfos[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];
  EmberAfScheduledPriceMatrix scheduledPriceMatrix[EMBER_AF_PRICE_CLUSTER_SERVER_ENDPOINT_COUNT][EMBER_AF_PLUGIN_PRICE_SERVER_TARIFF_TABLE_SIZE];
} EmberAfScheduledPriceMatrixTable;

typedef struct {
  EmberAfPriceBlockPeriodTable blockPeriodTable;
  EmberAfPriceConversionFactorTable conversionFactorTable;
  EmberAfPriceCalorificValueTable calorificValueTable;
  EmberAfPriceCO2Table co2ValueTable;
  EmberAfPriceTierLabelTable tierLabelTable;
  EmberAfPriceBillingPeriodTable billingPeriodTable;
  EmberAfPriceConsolidatedBillsTable consolidatedBillsTable;
  EmberAfPriceCppTable cppTable;
  EmberAfPriceCreditPaymentTable creditPaymentTable;
  EmberAfPriceCurrencyConversionTable currencyConversionTable;
  EmberAfPriceCancelTariffTable cancelTariffTable;
  EmberAfScheduledTariffTable scheduledTariffTable;
  EmberAfScheduledBlockThresholdsTable scheduledBlockThresholdsTable;
  EmberAfScheduledPriceMatrixTable  scheduledPriceMatrixTable;
} EmberAfPriceServerInfo;

extern EmberAfPriceServerInfo priceServerInfo;

extern bool emAfPluginPriceServerBillingPeriodRepeat;

/**
 * @brief Sends the next Get Scheduled Prices command.
 *
 * @param endpoint The endpoint in question.
 **/
void emberAfPriceServerSendGetScheduledPrices(uint8_t endpoint);

/**
 * @brief Returns the number of seconds until the next Get Scheduled Prices command should be sent.
 *
 * @return The number of seconds until the next Get Scheduled Prices command should be sent.
 **/
uint32_t emberAfPriceServerSecondsUntilGetScheduledPricesEvent(void);

/**
 * @brief Clears all prices in the price table.
 *
 * @param endpoint The endpoint in question.
 **/
void emberAfPriceClearPriceTable(uint8_t endpoint);

/**
 * @brief Clears all tariffs in the tariff table.
 *
 * @param endpoint The endpoint in question.
 */
void emberAfPriceClearTariffTable(uint8_t endpoint);

/**
 * @brief Clears all price matrices in the price matrix table.
 *
 * @param endpoint The endpoint in question.
 */
void emberAfPriceClearPriceMatrixTable(uint8_t endpoint);

/**
 * @brief Clears all block thresholds in the block thresholds table.
 *
 * @param endpoint The endpoint in question.
 */
void emberAfPriceClearBlockThresholdsTable(uint8_t endpoint);

/**
 * @brief Gets a price used by the Price server plugin.
 *
 * This function can be used to get a price and metadata that the plugin will
 * send to clients. For "start now" prices that are current or scheduled, the
 * duration is adjusted to reflect how many minutes remain for the price.
 * Otherwise, the start time and duration of "start now" prices reflect the
 * actual start and the original duration.
 *
 * @param endpoint The relevant endpoint.
 * @param index The index in the price table.
 * @param price The ::EmberAfScheduledPrice structure describing the price.
 * @return True if the price was found or false is the index is invalid.
 */
bool emberAfPriceGetPriceTableEntry(uint8_t endpoint,
                                    uint8_t index,
                                    EmberAfScheduledPrice *price);

/**
 * @brief Sets values in the Block Period table.
 *
 * @param endpoint The relevant endpoint.
 * @param providerId A unique identifier for the commodity provider.
 * @param issuerEventId The event ID of the block period data.
 * @param blockPeriodStartTime Time at which the block period data is valid.
 * @param blockPeriodDuration The block period duration. Units are specified by the blockPeriodDurationType.
 * @param blockPeriodControl Identifies additional control options for the block period command.
 * @param blockPeriodDurationType A bitmap that indicates the units used in the block period.
 * @param tariffType A bitmap identifying the type of tariff published in this command.
 * @param tariffResolutionPeriod The resolution period for the block tariff.
 *
 **/
void emberAfPluginPriceServerBlockPeriodAdd(uint8_t endpoint, uint32_t providerId, uint32_t issuerEventId,
                                            uint32_t blockPeriodStartTime, uint32_t blockPeriodDuration,
                                            uint8_t  blockPeriodControl, uint8_t blockPeriodDurationType,
                                            uint32_t thresholdMultiplier, uint32_t thresholdDivisor,
                                            uint8_t  tariffType, uint8_t tariffResolutionPeriod);

/**
 * @brief Sends a Publish Block Period command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 * @param index The index of the table whose data will be used in the command.
 *
 **/
void emberAfPluginPriceServerBlockPeriodPub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t index);

/**
 * @brief Prints the data in the specified index of the Block Period table.
 *
 * @param endpoint The relevant endpoint.
 * @param index The index of the table whose index will be printed.
 *
 **/
void emberAfPluginPriceServerBlockPeriodPrint(uint8_t endpoint, uint8_t index);

/**
 * @brief Returns the number of seconds until the next block period event occurs.
 *
 * @param endpoint The relevant endpoint.
 * @return Returns the number of seconds until the next block period event.
 *
 **/
uint32_t emberAfPriceServerSecondsUntilBlockPeriodEvent(uint8_t endpoint);

/**
 * @brief Updates block period attributes to match the current block period.
 *
 * @param endpoint The relevant endpoint.
 * @param repeat   Indicates whether or not to try to repeat the current block period
 * in the next block period. This is also controlled by a bit in the
 * blockPeriodControl, saying whether or not the block period should repeat
 * on expiry.
 **/
void emberAfPriceServerRefreshBlockPeriod(uint8_t endpoint, bool repeat);

/**
 * @brief Gets a tariff used by the price server plugin.
 *
 * This function can be used to get a tariff and associated metadata that
 * the plugin will send to clients.
 *
 * @param endpoint The relevant endpoint.
 * @param index    The index in the tariff table.
 * @param info   The ::EmberAfPriceCommonInfo structure describing the tariff.
 * @param tariff   The ::EmberAfScheduledTariff structure describing the tariff.
 * @return         True if the tariff was found.
 */
bool emberAfPriceGetTariffTableEntry(uint8_t endpoint,
                                     uint8_t index,
                                     EmberAfPriceCommonInfo *info,
                                     EmberAfScheduledTariff *tariff);

/**
 * @brief Retrieves a price matrix entry by index.
 *
 * This function can be used to get a price matrix and associated metadata that
 * the plugin will send to clients.
 *
 * @param endpoint The relevant endpoint.
 * @param index    The index in the price matrix table.
 * @param pm       The ::EmberAfScheduledPriceMatrix structure describing the price matrix.
 * @return         True if the price matrix was found.
 */
bool emberAfPriceGetPriceMatrix(uint8_t endpoint,
                                uint8_t index,
                                EmberAfPriceCommonInfo *info,
                                EmberAfScheduledPriceMatrix *pm);

/**
 * @brief Gets the block thresholds used by the Price server plugin.
 *
 * This function can be used to get the block thresholds and associated metadata that
 * the plugin will send to clients.
 *
 * @param endpoint The relevant endpoint.
 * @param index    The index in the block thresholds table.
 * @param bt       The ::EmberAfScheduledBlockThresholds structure describing the block thresholds.
 * @return         True if the block thresholds was found.
 */
bool emberAfPriceGetBlockThresholdsTableEntry(uint8_t endpoint,
                                              uint8_t index,
                                              EmberAfScheduledBlockThresholds *bt);

/**
 * @brief Gets a tariff by issuer tariff ID and endpoint.
 *
 * @param endpoint        The relevant endpoint.
 * @param issuerTariffId  The issuer tariff ID.
 * @param info            The ::EmberAfPriceCommonInfo structure describing the tariff.
 * @param tariff          The ::EmberAfScheduledTariff structure describing the tariff.
 * @return                True if the tariff was found.
 */
bool emberAfPriceGetTariffByIssuerTariffId(uint8_t endpoint,
                                           uint32_t issuerTariffId,
                                           EmberAfPriceCommonInfo *info,
                                           EmberAfScheduledTariff *tariff);

/**
 * @brief Gets a price matrix by issuer tariff ID and endpoint.
 *
 * @param endpoint The relevant endpoint.
 * @param issuerTariffId  The issuer tariff ID.
 * @param pm       The ::EmberAfScheduledPriceMatrix structure describing the price matrix.
 * @return         True if the price matrix was found.
 */
bool emberAfPriceGetPriceMatrixByIssuerTariffId(uint8_t endpoint,
                                                uint32_t issuerTariffId,
                                                EmberAfPriceCommonInfo *info,
                                                EmberAfScheduledPriceMatrix *pm);

/**
 * @brief Gets the block thresholds by issuer tariff ID and endpoint.
 *
 * @param endpoint The relevant endpoint.
 * @param issuerTariffId  The issuer tariff ID.
 * @param bt       The ::EmberAfScheduledBlockThresholds structure describing the block thresholds.
 * @return         True if the block thresholds were found.
 */
bool emberAfPriceGetBlockThresholdsByIssuerTariffId(uint8_t endpoint,
                                                    uint32_t issuerTariffId,
                                                    EmberAfPriceCommonInfo *info,
                                                    EmberAfScheduledBlockThresholds *bt);
/**
 * @brief Sets a price used by the Price server plugin.
 *
 * This function can be used to set a price and metadata that the plugin will
 * send to clients. Setting the start time to zero instructs clients to start
 * the price now. For "start now" prices, the plugin will automatically adjust
 * the duration reported to clients based on the original start time of the
 * price.
 *
 * @param endpoint The relevant endpoint.
 * @param index The index in the price table.
 * @param price The ::EmberAfScheduledPrice structure describing the price. If
 * NULL, the price is removed from the server.
 * @return True if the price was set or removed or false is the index is
 * invalid.
 */
bool emberAfPriceSetPriceTableEntry(uint8_t endpoint,
                                    uint8_t index,
                                    const EmberAfScheduledPrice *price);

/**
 * @brief Sets a tariff used by the Price server plugin.
 *
 * This function can be used to set a tariff and metadata that the plugin
 * will send to clients.
 *
 * @param endpoint The relevant endpoint.
 * @param index    The index in the tariff table.
 * @param tariff   The ::EmberAfScheduledTariff structure describing the tariff.
 *                 If NULL, the tariff is removed from the server.
 * @return         True if the tariff was set or removed, or false if the
 *                 index is invalid.
 */
bool emberAfPriceSetTariffTableEntry(uint8_t endpoint,
                                     uint8_t index,
                                     EmberAfPriceCommonInfo *info,
                                     const EmberAfScheduledTariff *tariff);

/**
 * @brief Sets a price matrix entry by index.
 *
 * This function can be used to set a price matrix and metadata that the plugin
 * will send to clients.
 *
 * @param endpoint The relevant endpoint.
 * @param index    The index in the price matrix table.
 * @param pm       The ::EmberAfScheduledPriceMatrix structure describing the
 *                 price matrix. If NULL, the price matrix is removed from the
 *                 server.
 * @return         True if the price matrix was set or removed, or false if the
 *                 index is invalid.
 */
bool emberAfPriceSetPriceMatrix(uint8_t endpoint,
                                uint8_t index,
                                EmberAfPriceCommonInfo *info,
                                const EmberAfScheduledPriceMatrix *pm);

/**
 * @brief Sets the block thresholds used by the price server plugin.
 *
 * This function can be used to set the block thresholds and metadata that the plugin
 * will send to clients.
 *
 * @param endpoint The relevant endpoint.
 * @param index    The index in the block thresholds table.
 * @param bt       The ::EmberAfScheduledBlockThresholds structure describing the
 *                 block thresholds. If NULL, the block thresholds entry is removed
 *                 from the table.
 * @return         True if the block thresholds was set or removed, or false if the
 *                 index is invalid.
 */
bool emberAfPriceSetBlockThresholdsTableEntry(uint8_t endpoint,
                                              uint8_t index,
                                              const EmberAfPriceCommonInfo *info,
                                              const EmberAfScheduledBlockThresholds *bt);

/**
 * @brief Gets the current price used by the price server plugin.
 *
 * This function can be used to get the current price and metadata that the
 * plugin will send to clients. For "start now" prices, the duration is
 * adjusted to reflect how many minutes remain for the price. Otherwise, the
 * start time and duration reflect the actual start and the original duration.
 *
 * @param endpoint The relevant endpoint.
 * @param price The ::EmberAfScheduledPrice structure describing the price.
 * @return True if the current price was found or false is there is no current
 * price.
 */
bool emberAfGetCurrentPrice(uint8_t endpoint, EmberAfScheduledPrice *price);

/**
 * @brief Finds the first free index in the price table.
 *
 * This function looks through the price table and determines whether
 * the entry is in-use or scheduled to be in use. If not, it's
 * considered "free" for the purposes of the user adding a new price
 * entry to the server's table and the index is returned.
 *
 * @param endpoint The relevant endpoint.
 * @return The index of the first free (unused/unscheduled) entry in
 * the requested endpoint's price table, or ZCL_PRICE_INVALID_INDEX
 * if no available entry could be found.
 */
uint8_t emberAfPriceFindFreePriceIndex(uint8_t endpoint);

void emberAfPricePrint(const EmberAfScheduledPrice *price);
void emberAfPricePrintPriceTable(uint8_t endpoint);
void emberAfPricePrintTariff(const EmberAfPriceCommonInfo *info,
                             const EmberAfScheduledTariff *tariff);
void emberAfPricePrintTariffTable(uint8_t endpoint);
void emberAfPricePrintPriceMatrix(uint8_t endpoint,
                                  const EmberAfPriceCommonInfo *info,
                                  const EmberAfScheduledPriceMatrix *pm);
void emberAfPricePrintPriceMatrixTable(uint8_t endpoint);
void emberAfPricePrintBlockThresholds(uint8_t endpoint,
                                      const EmberAfPriceCommonInfo *info,
                                      const EmberAfScheduledBlockThresholds *bt);
void emberAfPricePrintBlockThresholdsTable(uint8_t endpoint);
void emberAfPluginPriceServerPublishPriceMessage(EmberNodeId nodeId,
                                                 uint8_t srcEndpoint,
                                                 uint8_t dstEndpoint,
                                                 uint8_t priceIndex);

/**
 * @brief Sets parameters in the conversion factors table.
 *
 * @param endpoint The endpoint in question.
 * @param issuerEventId The event ID of the conversion factor data.
 * @param startTime The time when the conversion factor data is valid.
 * @param conversionFactor Accounts for changes in the volume of gas
 * based on temperature and pressure.
 * @param conversionFactorTrailingDigit Determines where the decimal
 * point is located in the conversion factor.
 **/
EmberAfStatus emberAfPluginPriceServerConversionFactorAdd(uint8_t endpoint,
                                                          uint32_t issuerEventId,
                                                          uint32_t startTime,
                                                          uint32_t conversionFactor,
                                                          uint8_t conversionFactorTrailingDigit);
/**
 * @brief Clears the conversion factors table and invalidates all entries.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void emberAfPluginPriceServerConversionFactorClear(uint8_t endpoint);

/**
 * @brief Sends a Publish Conversion Factor command using the data
 * at the specified table index.
 *
 * @param tableIndex The index of the conversion factor table whose data
 * should be used in the publish conversion factor command.
 * @param dstAddr The destination address to which the command should be sent.
 * @param srcEp The source endpoint used in the transmission.
 * @param dstEp The destination endpoint used in the transmission.
 *
 **/
void emberAfPluginPriceServerConversionFactorPub(uint8_t tableIndex,
                                                 EmberNodeId dstAddr,
                                                 uint8_t srcEndpoint,
                                                 uint8_t dstEndpoint);

/**
 * @brief Returns the number of seconds until the next conversion factor
 * event will become active.
 *
 * @param endpoint The endpoint in question
 * @return The number of seconds until the next conversion factor
 * event becomes active.
 *
 **/
uint32_t emberAfPriceServerSecondsUntilConversionFactorEvent(uint8_t endpoint);

/**
 * @brief Refreshes the conversion factor information if necessary.
 * If the second conversion factor event is active, the first
 * is inactivated and the array is re-sorted.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void emberAfPriceServerRefreshConversionFactor(uint8_t endpoint);

/**
 * @brief Sets values in the Calorific Value table.
 *
 * @param endpoint The endpoint in question
 * @issuerEventId The event ID of the calorific value data.
 * @startTime The time at which the calorific value data is valid.
 * @calorificValue The amount of heat generated when a given mass
 * of fuel is burned.
 * @calorificValueTrailingDigit Determines where the decimal point
 * is located in the calorific value.
 *
 **/
EmberAfStatus emberAfPluginPriceServerCalorificValueAdd(uint8_t endpoint,
                                                        uint32_t issuerEventId,
                                                        uint32_t startTime,
                                                        uint32_t calorificValue,
                                                        uint8_t calorificValueUnit,
                                                        uint8_t calorificValueTrailingDigit);

/**
 * @brief Returns the number of seconds until the next calorific value event will become active.
 *
 * @param endpoint The endpoint in question
 * @return The number of seconds until the next calorific value event becomes active.
 *
 **/
uint32_t emberAfPriceServerSecondsUntilCalorificValueEvent(uint8_t endpoint);

/**
 * @brief Refreshes the caloric value information if necessary.
 * If the second calorific value event is active, the first is inactivated
 * and the array is re-sorted.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void emberAfPriceServerRefreshCalorificValue(uint8_t endpoint);

/**
 * @brief Clears the calorific value table and invalidates all entries.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void emberAfPluginPriceServerCalorificValueClear(uint8_t endpoint);

/**
 * @brief Sends a Publish Tariff Information command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 * @param tariffIndex The index of the tariff table whose data will
 * be used in the Publish Tariff Information command.
 *
 **/
void emberAfPluginPriceServerPublishTariffMessage(EmberNodeId nodeId,
                                                  uint8_t srcEndpoint,
                                                  uint8_t dstEndpoint,
                                                  uint8_t tariffIndex);

/**
 * @brief Prints the data in the conversion factor table.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void emberAfPrintConversionTable(uint8_t endpoint);

/**
 * @brief Prints the data in the calorific values table.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void emberAfPrintCalorificValuesTable(uint8_t endpoint);

/**
 * @brief Returns the number of seconds until the next CO2
 * value event becomes active.
 *
 * @param endpoint The endpoint in question.
 * @return The number of seconds until the next CO2 value event becomes active.
 *
 **/
uint32_t emberAfPriceServerSecondsUntilCO2ValueEvent(uint8_t endpoint);

/**
 * @brief Refreshes the CO2 value information if necessary.
 * If the second CO2 value event is active, the first is inactivated
 * and the array is re-sorted.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void emberAfPriceServerRefreshCO2Value(uint8_t endpoint);

/**
 * @brief Sets values in the CO2 Value table.
 *
 * @param endpoint The endpoint in question.
 * @param issuerEventId The event ID of the CO2 value table data.
 * @param startTime The time at which the CO2 value data is valid.
 * @param providerId A unique identifier for the commodity provider.
 * @param tariffType Bitmap identifying the type of tariff published
 * in this command.
 * @param co2Value Used to calculate the amount of carbon dioxide
 * produced from energy use.
 * @param co2ValueUnit An enumeration which defines the unit of the
 * co2Value attribute.
 * @param co2ValueTrailingDigit Determines where the decimal point
 * is located in the co2Value.
 *
 **/
void emberAfPluginPriceServerCo2ValueAdd(uint8_t endpoint,
                                         uint32_t issuerEventId,
                                         uint32_t startTime,
                                         uint32_t providerId,
                                         uint8_t tariffType,
                                         uint32_t co2Value,
                                         uint8_t co2ValueUnit,
                                         uint8_t co2ValueTrailingDigit);

/**
 * @brief Clears the CO2 value table and invalidates all entries.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void emberAfPluginPriceServerCo2ValueClear(uint8_t endpoint);

/**
 * @brief Prints the data in the CO2 values table.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void emberAfPrintCo2ValuesTable(uint8_t endpoint);

/**
 * @brief Sends a Publish CO2 Value command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 * @param index The index of the CO2 values table whose data will be used in the command.
 *
 **/
void emberAfPluginPriceServerCo2LabelPub(uint16_t nodeId,
                                         uint8_t srcEndpoint,
                                         uint8_t dstEndpoint,
                                         uint8_t index);

/**
 * @brief Sets values in the Tier Label table.
 *
 * @param endpoint The endpoint in question.
 * @param index The index of the billing period table whose data will be modified.
 * @param valid Indicates if the data at this index is valid or not.
 * @param providerId A unique identifier for the commodity provider.
 * @param issuerEventId The event ID of the tier labels table data.
 * @param issuerTariffId A unique identifier that identifies which tariff the labels apply to.
 * @param tierId The tier number that associated tier label applies to.
 * @param tierLabel A character string descriptor for this tier.
 *
 **/
void emberAfPluginPriceServerTierLabelSet(uint8_t  endpoint,
                                          uint8_t  index,
                                          uint8_t  valid,
                                          uint32_t providerId,
                                          uint32_t issuerEventId,
                                          uint32_t issuerTariffId,
                                          uint8_t tierId,
                                          uint8_t* tierLabel);
void emberAfPrintPrintTierLabelsTable(void);

/**
 * @brief Adds a tier label to the specified tier label table.
 *
 * @param endpoint The endpoint in question.
 * @param issuerTariffId A unique identifier that identifies which tariff the labels apply to.
 * @param tierId The tier number that associated tier label applies to.
 * @param tierLabel Character string descriptor for this tier.
 *
 **/
void emberAfPluginPriceServerTierLabelAddLabel(uint8_t endpoint,
                                               uint32_t issuerTariffId,
                                               uint8_t tierId,
                                               uint8_t *tierLabel);

/**
 * @brief Prints the tier labels table.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void emberAfPrintTierLabelsTable(uint8_t endpoint);

/**
 * @brief Sends a Publish Tier Labels command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 * @param index The index of the tier labels table whose data will
 * be used in the Publish Tier Labels command.
 *
 **/
void emberAfPluginPriceServerTierLabelPub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t index);

/**
 * @brief Returns the number of seconds until the next billing period
 * event will become active.
 *
 * @param endpoint The endpoint in question.
 * @return The number of seconds until the next billing period event becomes active.
 *
 **/
uint32_t emberAfPriceServerSecondsUntilBillingPeriodEvent(uint8_t endpoint);

/**
 * @brief Refreshes the billing period information if necessary.
 * If the second billing period event is active, the first is inactivated
 * and the array is re-sorted.
 * If the force argument is set to true, the billing period will be forced to refresh.
 *
 * @param endpoint The endpoint in question.
 * @param force    Whether or not the billing period should be forced to refresh.
 *
 **/
void emberAfPriceServerRefreshBillingPeriod(uint8_t endpoint, bool force);

/**
 * @brief Sets values in the billing period table.
 *
 * @param endpoint The endpoint in question.
 * @param startTime The time at which the billing period data is valid.
 * @param issuerEventId The event ID of the billing period data.
 * @param providerId A unique identifier for the commodity provider.
 * @param billingPeriodDuration The billing period duration.
 * Units are specified by the billingPeriodDurationType.
 * @param billingPeriodDurationType A bitmap that indicates
 * the units used in the billing period.
 * @param tariffType Bitmap identifying the type of tariff published in this command.
 *
 **/
EmberStatus emberAfPluginPriceServerBillingPeriodAdd(uint8_t endpoint,
                                                     uint32_t startTime,
                                                     uint32_t issuerEventId,
                                                     uint32_t providerId,
                                                     uint32_t billingPeriodDuration,
                                                     uint8_t billingPeriodDurationType,
                                                     uint8_t tariffType);
/**
 * @brief Sends a Publish Billing Period command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 * @param index The index of the table whose data will be used in the command.
 *
 **/
void emberAfPluginPriceServerBillingPeriodPub(uint16_t nodeId, uint8_t srcEndpoint,
                                              uint8_t dstEndpoint, uint8_t index);

/**
 * @brief Prints the data in the billing period table for the specified endpoint.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void emberAfPrintBillingPeriodTable(uint8_t endpoint);

/**
 * @brief Prints the data in the consolidated bills table at the specified index.
 *
 * @param endpoint The endpoint in question.
 * @param index The index of the consolidated bills table whose data should be printed.
 *
 **/
void emberAfPrintConsolidatedBillTableEntry(uint8_t endpoint, uint8_t index);

/**
 * @brief Sets values in the consolidated bills table.
 *
 * @param endpoint The endpoint in question.
 * @param startTime The time at which the consolidated bills data is valid.
 * @param issuerEventId The event ID of the consolidated bills data.
 * @param providerId A unique identifier for the commodity provider.
 * @param billingPeriodDuration The billing period duration. Units are specified by
 * the billingPeriodDurationType.
 * @param billingPeriodDurationType A bitmap that indicates the units used in the billing period.
 * @param tariffType Bitmap identifying the type of tariff published in this command.
 * @param consolidatedBill The consolidated bill value for the specified billing period.
 * @param currency The currency used in the consolidatedBill field.
 * @param billTrailingDigit Determines where the decimal point is located
 * in the consolidatedBill field.
 *
 **/
void emberAfPluginPriceServerConsolidatedBillAdd(uint8_t endpoint, uint32_t startTime,
                                                 uint32_t issuerEventId, uint32_t providerId,
                                                 uint32_t billingPeriodDuration, uint8_t billingPeriodDurationType,
                                                 uint8_t tariffType, uint32_t consolidatedBill,
                                                 uint16_t currency, uint8_t billTrailingDigit);

/**
 * @brief Sends a Publish Consolidated Bill command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 * @param index The index of the table whose data will be used in the command.
 *
 **/
void emberAfPluginPriceServerConsolidatedBillPub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t index);

/**
 * @brief Sets values of the CPP Event.
 *
 * @param endpoint The endpoint in question.
 * @param valid Indicates if the CPP Event data is valid or not.
 * @param providerId A unique identifier for the commodity provider.
 * @param issuerEventId The event ID of the CPP Event.
 * @param startTime The time at which the CPP Event data is valid.
 * @param durationInMinutes Defines the duration of the CPP Event.
 * @param tariffType Bitmap identifying the type of tariff published in this command.
 * @param cppPriceTier Indicates which CPP price tier should be used for the event.
 * @param cppAuth The status of the CPP event.
 *
 **/
void emberAfPluginPriceServerCppEventSet(uint8_t endpoint, uint8_t valid, uint32_t providerId, uint32_t issuerEventId, uint32_t startTime,
                                         uint16_t durationInMinutes, uint8_t tariffType, uint8_t cppPriceTier, uint8_t cppAuth);

/**
 * @brief Sends a Publish CPP Event command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 *
 **/
void emberAfPluginPriceServerCppEventPub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint);

/**
 * @brief Prints data in the CPP Event.
 *
 * @param endpoint The endpoint in question.
 *
 **/
void emberAfPluginPriceServerCppEventPrint(uint8_t endpoint);

/**
 * @brief Sends a Publish Credit Payment command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 * @param index The index of the table whose data will be used in the command.
 *
 **/
void emberAfPluginPriceServerCreditPaymentPub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint, uint8_t index);

/**
 * @brief Sets values in the credit payment table.
 *
 * @param endpoint The endpoint in question.
 * @param index The index of the credit payment table whose data will be modified.
 * @param valid Indicates if the data at this index is valid or not.
 * @param providerId A unique identifier for the commodity provider.
 * @param issuerEventId The event ID of the credit payment data.
 * @param creditPaymentDueDate The time the next credit payment is due.
 * @param creditPaymentOverdueAmount The current amount that is overdue from the customer.
 * @param creditPaymentStatus Indicates the current credit payment status.
 * @param creditPayment The amount of the last credit payment.
 * @param creditPaymentDate The time at which the last credit payment was made.
 * @param creditPaymentRef A string used to denote the last credit payment
 * reference used by the energy supplier.
 *
 **/
void emberAfPluginPriceServerCreditPaymentSet(uint8_t endpoint, uint8_t index, uint8_t valid,
                                              uint32_t providerId, uint32_t issuerEventId,
                                              uint32_t creditPaymentDueDate, uint32_t creditPaymentOverdueAmount,
                                              uint8_t creditPaymentStatus, uint32_t creditPayment,
                                              uint32_t creditPaymentDate, uint8_t *creditPaymentRef);

//void emberAfPluginPriceServerCreditPaymentPrint( void );

/**
 * @brief Sends a Publish Currency Conversion command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 *
 **/
void emberAfPluginPriceServerCurrencyConversionPub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint);

/**
 * @brief Sets values for the Currency Conversion command.
 *
 * @param endpoint The endpoint in question.
 * @param valid Indicates if the currency conversion data is valid or not.
 * @param providerId A unique identifier for the commodity provider.
 * @param issuerEventId The event ID of the currency conversion data.
 * @param startTime The time at which the currency conversion data is valid.
 * @param oldCurrency Information about the old unit of currency.
 * @param newCurrency Information about the new unit of currency.
 * @param conversionFactor Accounts for changes in the volume of gas based on temperature and pressure.
 * @param conversionFactorTrailingDigit Determines where the decimal point is located in the conversion factor.
 * @param currencyChangeControlFlags Denotes functions that are required to be carried out by the client.
 *
 **/
void emberAfPluginPriceServerCurrencyConversionSet(uint8_t endpoint, uint8_t valid,
                                                   uint32_t providerId, uint32_t issuerEventId,
                                                   uint32_t startTime, uint16_t oldCurrency, uint16_t newCurrency,
                                                   uint32_t conversionFactor, uint8_t conversionFactorTrailingDigit,
                                                   uint32_t currencyChangeControlFlags);

/**
 * @brief Sets values in the tariff cancellation command.
 *
 * @param endpoint The endpoint in question.
 * @param valid Indicates if the tariff cancellation command is valid or not.
 * @param providerId A unique identifier for the commodity provider.
 * @param issuerTariffId Unique identifier that identifies which tariff should be canceled.
 * @param tariffType A bitmap identifying the type of tariff to be canceled.
 *
 **/
void emberAfPluginPriceServerTariffCancellationSet(uint8_t endpoint, uint8_t valid, uint32_t providerId,
                                                   uint32_t issuerTariffId, uint8_t tariffType);

/**
 * @brief Sends a Cancel Tariff command.
 *
 * @param nodeId The destination address to which the command should be sent.
 * @param srcEndpoint The source endpoint used in the transmission.
 * @param dstEndpoint The destination endpoint used in the transmission.
 *
 **/
void emberAfPluginPriceServerTariffCancellationPub(uint16_t nodeId, uint8_t srcEndpoint, uint8_t dstEndpoint);

uint32_t emberAfPriceServerSecondsUntilTariffInfoEvent(uint8_t endpoint);

void emberAfPriceServerRefreshTariffInformation(uint8_t endpoint);

bool emberAfPriceAddTariffTableEntry(uint8_t endpoint,
                                     EmberAfPriceCommonInfo *info,
                                     const EmberAfScheduledTariff *curTariff);

bool emberAfPriceAddPriceMatrixRaw(uint8_t endpoint,
                                   uint32_t providerId,
                                   uint32_t issuerEventId,
                                   uint32_t startTime,
                                   uint32_t issuerTariffId,
                                   uint8_t commandIndex,
                                   uint8_t numberOfCommands,
                                   uint8_t subPayloadControl,
                                   uint8_t* payload);

bool emberAfPriceAddPriceMatrix(uint8_t endpoint,
                                EmberAfPriceCommonInfo *info,
                                EmberAfScheduledPriceMatrix * pm);

bool emberAfPriceAddBlockThresholdsTableEntry(uint8_t endpoint,
                                              uint32_t providerId,
                                              uint32_t issuerEventId,
                                              uint32_t startTime,
                                              uint32_t issuerTariffId,
                                              uint8_t commandIndex,
                                              uint8_t numberOfCommands,
                                              uint8_t subpayloadControl,
                                              uint8_t* payload);

void emberAfPriceClearBlockPeriodTable(uint8_t endpoint);
void sendValidCmdEntries(uint8_t cmdId,
                         uint8_t endpoint,
                         uint8_t* validEntries,
                         uint8_t validEntryCount);
void emberAfPluginPriceServerPriceUpdateBindings(void);

uint32_t emberAfPriceServerSecondsUntilActivePriceMatrixEvent(uint8_t endpoint);
void emberAfPriceServerRefreshPriceMatrixInformation(uint8_t endpoint);
uint32_t emberAfPriceServerSecondsUntilActiveBlockThresholdsEvent(uint8_t endpoint);
void emberAfPriceServerRefreshBlockThresholdsInformation(uint8_t endpoint);

#endif  // #ifndef _PRICE_SERVER_H_
