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
 * @brief CLI for the Price Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "price-server.h"

//=============================================================================
// Functions

void emAfPriceServerCliClear(void);
void emAfPriceServerCliInit(void);
void emAfPriceServerCliTClear(void);
void emAfPriceServerCliWho(void);
void emAfPriceServerCliTWho(void);
void emAfPriceServerCliWhat(void);
void emAfPriceServerCliTWhat(void);
void emAfPriceServerCliWhen(void);
void emAfPriceServerCliTWhen(void);
void emAfPriceServerCliPrice(void);
void emAfPriceServerCliTariff(void);
void emAfPriceServerCliAlternate(void);
void emAfPriceServerCliAck(void);
void emAfPriceServerCliValid(void);
void emAfPriceServerCliTAdd(void);
void emAfPriceServerCliGet(void);
void emAfPriceServerCliTGet(void);
void emAfPriceServerCliPrint(void);
void emAfPriceServerCliTPrint(void);
void emAfPriceServerCliSprint(void);
void emAfPriceServerCliTSPrint(void);
void emAfPriceServerCliPublish(void);
void emAfPriceServerCliPmPrint(void);
void emAfPriceServerCliPmSetMetadata(void);
void emAfPriceServerCliPmSetProvider(void);
void emAfPriceServerCliPmSetEvent(void);
void emAfPriceServerCliPmSetTariff(void);
void emAfPriceServerCliPmSetTime(void);
void emAfPriceServerCliPmSetStatus(void);
void emAfPriceServerCliPmGet(void);
void emAfPriceServerCliPmAdd(void);
void emAfPriceServerCliPmPut(void);
void emAfPriceServerCliPmFillTier(void);
void emAfPriceServerCliPmFillBlock(void);
void emAfPriceServerCliBlockPeriodAdd(void);
void emAfPriceServerCliBlockPeriodPrint(void);
void emAfPriceServerCliBlockPeriodPub(void);
void emAfPriceServerCliConversionFactor(void);
void emAfPriceServerCliCppEventSet(void);
void emAfPriceServerCliCppEventPub(void);
void emAfPriceServerCliCppEventPrint(void);
void emAfPriceServerCliCreditPaymentPub(void);
void emAfPriceServerCliCreditPaymentSet(void);
void emAfPriceServerCliCreditPaymentPrint(void);
void emAfPriceServerCliCurrencyConversionPub(void);
void emAfPriceServerCliCurrencyConversionSet(void);

//=============================================================================
// Globals

static EmberAfScheduledPrice price;

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginPriceServerCommands[] = {
  emberCommandEntryAction("clear", emAfPriceServerCliClear, "u", ""),
  emberCommandEntryAction("who", emAfPriceServerCliWho, "wbw", ""),
  emberCommandEntryAction("what", emAfPriceServerCliWhat, "uvuuu", ""),
  emberCommandEntryAction("when", emAfPriceServerCliWhen, "wv", ""),
  emberCommandEntryAction("price", emAfPriceServerCliPrice, "wuwu", ""),
  emberCommandEntryAction("alternate", emAfPriceServerCliAlternate, "wuu", ""),
  emberCommandEntryAction("ack", emAfPriceServerCliAck, "u", ""),
  emberCommandEntryAction("valid", emAfPriceServerCliValid, "uu", ""),
  emberCommandEntryAction("get", emAfPriceServerCliGet, "uu", ""),
  emberCommandEntryAction("print", emAfPriceServerCliPrint, "u", ""),
  emberCommandEntryAction("sprint", emAfPriceServerCliSprint, "u", ""),
  emberCommandEntryAction("publish", emAfPriceServerCliPublish, "vuuu", ""),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

//=============================================================================

// plugin price-server init <endpoint:1>
void emAfPriceServerCliInit(void)
{
  emberAfPriceClusterServerInitCallback(emberUnsignedCommandArgument(0) );
}

// plugin price-server clear <endpoint:1>
void emAfPriceServerCliClear(void)
{
  emberAfPriceClearPriceTable(emberUnsignedCommandArgument(0));
}

// plugin price-server who <provId:4> <label:1-13> <eventId:4>
void emAfPriceServerCliWho(void)
{
  uint8_t length;
  price.providerId = emberUnsignedCommandArgument(0);
  length = emberCopyStringArgument(1,
                                   price.rateLabel + 1,
                                   ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH,
                                   false);
  price.rateLabel[0] = length;
  price.issuerEventID = emberUnsignedCommandArgument(2);
}

// plugin price-server what <unitOfMeas:1> <curr:2> <ptd:1> <PTRT:1> <bt:1>
void emAfPriceServerCliWhat(void)
{
  price.unitOfMeasure = (uint8_t)emberUnsignedCommandArgument(0);
  price.currency = (uint16_t)emberUnsignedCommandArgument(1);
  price.priceTrailingDigitAndTier = (uint8_t)emberUnsignedCommandArgument(2);
  price.numberOfPriceTiersAndTier = (uint8_t)emberUnsignedCommandArgument(3);
  price.numberOfBlockThresholds = (uint8_t)emberUnsignedCommandArgument(4);
}

// plugin price-server when <startTime:4> <duration:2>
void emAfPriceServerCliWhen(void)
{
  price.startTime = emberUnsignedCommandArgument(0);
  price.duration = (uint16_t)emberUnsignedCommandArgument(1);
}

// plugin price-server price <price:4> <ratio:1> <genPrice:4> <genRatio:1>
void emAfPriceServerCliPrice(void)
{
  price.price = emberUnsignedCommandArgument(0);
  price.priceRatio = (uint8_t)emberUnsignedCommandArgument(1);
  price.generationPrice = emberUnsignedCommandArgument(2);
  price.generationPriceRatio = (uint8_t)emberUnsignedCommandArgument(3);
}

// plugin price-server alternate <alternateCostDelivered:4> <alternateCostUnit:1> <alternateCostTrailingDigit:1>
void emAfPriceServerCliAlternate(void)
{
  price.alternateCostDelivered = emberUnsignedCommandArgument(0);
  price.alternateCostUnit = (uint8_t)emberUnsignedCommandArgument(1);
  price.alternateCostTrailingDigit = (uint8_t)emberUnsignedCommandArgument(2);
}

// plugin price-server ack <req:1>
void emAfPriceServerCliAck(void)
{
  price.priceControl &= ~ZCL_PRICE_CLUSTER_PRICE_ACKNOWLEDGEMENT_MASK;
  if ( ((uint8_t)emberUnsignedCommandArgument(0)) == 1) {
    price.priceControl |= EMBER_ZCL_PRICE_CONTROL_ACKNOWLEDGEMENT_REQUIRED;
  }
}

// plugin price-server <valid | invalid | tinvalid> <endpoint:1> <index:1>
void emAfPriceServerCliValid(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(1);
  if (emberCurrentCommand->name[0] == 't') {
    if (!emberAfPriceSetTariffTableEntry(endpoint,
                                         index,
                                         NULL,
                                         NULL)) {
      emberAfPriceClusterPrintln("tariff entry %d not present", index);
    }

    return;
  }

  if (!emberAfPriceSetPriceTableEntry(endpoint,
                                      index,
                                      (emberCurrentCommand->name[0] == 'v'
                                       ? &price
                                       : NULL))) {
    emberAfPriceClusterPrintln("price entry %d not present", index);
  }
}

// plugin price-server get <endpoint:1> <index:1>
void emAfPriceServerCliGet(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t index = (uint8_t)emberUnsignedCommandArgument(1);
  if (!emberAfPriceGetPriceTableEntry(endpoint, index, &price)) {
    emberAfPriceClusterPrintln("price entry %d not present", index);
  }
}

// plugin price-server print <endpoint:1>
void emAfPriceServerCliPrint(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfPricePrintPriceTable(endpoint);
}

// plugin price-server sprint
void emAfPriceServerCliSprint(void)
{
  emberAfPricePrint(&price);
}

// plugin price-server publish <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <priceIndex:1>
void emAfPriceServerCliPublish(void)
{
  emberAfPluginPriceServerPublishPriceMessage((EmberNodeId)emberUnsignedCommandArgument(0),
                                              (uint8_t)emberUnsignedCommandArgument(1),
                                              (uint8_t)emberUnsignedCommandArgument(2),
                                              (uint8_t)emberUnsignedCommandArgument(3));
}

// plugin price-server pub-tariff-info <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <tariffIndex:1>
void emAfPriceServerCliPubTariffInfo(void)
{
  emberAfPluginPriceServerPublishTariffMessage((EmberNodeId)emberUnsignedCommandArgument(0),
                                               (uint8_t)emberUnsignedCommandArgument(1),
                                               (uint8_t)emberUnsignedCommandArgument(2),
                                               (uint8_t)emberUnsignedCommandArgument(3));
}

// plugin price-server block-period add <endpoint:1> <providerId:4> <issuerEventId:4> <blockPeriodStartTime:4> <blockPeriodDuration:4> <blockPeriodControl:1> <blockPeriodDurationType:1> <tariffType:1> <tariffResolutionPeriod:1>
void emAfPriceServerCliBlockPeriodAdd(void)
{
  emberAfPluginPriceServerBlockPeriodAdd( (uint8_t) emberUnsignedCommandArgument(0),
                                          (uint32_t)emberUnsignedCommandArgument(1),
                                          (uint32_t)emberUnsignedCommandArgument(2),
                                          (uint32_t)emberUnsignedCommandArgument(3),
                                          (uint32_t)emberUnsignedCommandArgument(4),
                                          (uint8_t) emberUnsignedCommandArgument(5),
                                          (uint8_t) emberUnsignedCommandArgument(6),
                                          (uint32_t)emberUnsignedCommandArgument(7),
                                          (uint32_t)emberUnsignedCommandArgument(8),
                                          (uint8_t) emberUnsignedCommandArgument(9),
                                          (uint8_t)emberUnsignedCommandArgument(10) );
}

// plugin price-server block-period print <endpoint:1> <index:1>
void emAfPriceServerCliBlockPeriodPrint(void)
{
  emberAfPluginPriceServerBlockPeriodPrint( (uint8_t)emberUnsignedCommandArgument(0),
                                            (uint8_t)emberUnsignedCommandArgument(1) );
}

// plugin price-server block-period pub <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <index:1>
void emAfPriceServerCliBlockPeriodPub(void)
{
  emberAfPluginPriceServerBlockPeriodPub( (uint16_t)emberUnsignedCommandArgument(0),
                                          (uint8_t)emberUnsignedCommandArgument(1),
                                          (uint8_t)emberUnsignedCommandArgument(2),
                                          (uint8_t)emberUnsignedCommandArgument(3) );
}

// plugin price-server block-period clear <endpoint:1>
void emAfPriceServerCliBlockPeriodClear(void)
{
  emberAfPriceClearBlockPeriodTable((uint8_t)emberUnsignedCommandArgument(0));
}

// plugin price-server conversion-factor add <endpoint:1> <issuerEventId:4> <startTime:4> <conversionFactor:4> <conversionFactorTrailingDigit:1>
void emAfPriceServerCliConversionFactorAdd(void)
{
  emberAfPluginPriceServerConversionFactorAdd((uint8_t) emberUnsignedCommandArgument(0),
                                              (uint32_t)emberUnsignedCommandArgument(1),
                                              (uint32_t)emberUnsignedCommandArgument(2),
                                              (uint32_t)emberUnsignedCommandArgument(3),
                                              (uint8_t) emberUnsignedCommandArgument(4) );
}

// plugin price-server conversion-factor clear <endpoint:1>
void emAfPriceServerCliConversionFactorClear(void)
{
  emberAfPluginPriceServerConversionFactorClear( (uint8_t)emberUnsignedCommandArgument(0) );
}

// plugin price-server conversion-factor pub <tableIndex:1>  <dst:2> <src endpoint:1>  <dst endpoint:1>
void emAfPriceServerCliConversionFactorPub(void)
{
  uint8_t tableIndex = (uint8_t)emberUnsignedCommandArgument(0);
  EmberNodeId dstAddr = (EmberNodeId)emberUnsignedCommandArgument(1);
  uint8_t srcEndpoint =  (uint8_t)emberUnsignedCommandArgument(2);
  uint8_t dstEndpoint =  (uint8_t)emberUnsignedCommandArgument(3);
  emberAfPluginPriceServerConversionFactorPub(tableIndex, dstAddr, srcEndpoint, dstEndpoint);
}

// plugin price-server conversion-factor print <endpoint:1>
void emAfPriceServerCliConversionFactorPrint(void)
{
  emberAfPrintConversionTable( (uint8_t)emberUnsignedCommandArgument(0) );
}

// plugin price-server calorific-value add <endpoint:1> <issuerEventId:4> <startTime:4> <calorificValue:4> <calorificValueUnit:1> <calorificValueTrailingDigit:1>
void emAfPriceServerCliCalorificValueAdd(void)
{
  emberAfPluginPriceServerCalorificValueAdd((uint8_t) emberUnsignedCommandArgument(0),
                                            (uint32_t)emberUnsignedCommandArgument(1),
                                            (uint32_t)emberUnsignedCommandArgument(2),
                                            (uint32_t)emberUnsignedCommandArgument(3),
                                            (uint8_t) emberUnsignedCommandArgument(4),
                                            (uint8_t) emberUnsignedCommandArgument(5));
}

// plugin price-server calorific-factor clear <endpoint:1>
void emAfPriceServerCliCalorificValueClear(void)
{
  emberAfPluginPriceServerCalorificValueClear( (uint8_t)emberUnsignedCommandArgument(0) );
}

// plugin price-server calorific-factor print <endpoint:1>
void emAfPriceServerCliCalorificValuePrint(void)
{
  emberAfPrintCalorificValuesTable( (uint8_t)emberUnsignedCommandArgument(0) );
}

// plugin price-server co2-val add <endpoint:1> <issuerEventId:4> <startTime:4> <providerId:4> <tariffType:1> <co2Value:4> <co2ValueUnit:1> <co2ValueTrailingDigit:1>
void emAfPriceServerCliCo2ValueAdd(void)
{
  emberAfPluginPriceServerCo2ValueAdd((uint8_t) emberUnsignedCommandArgument(0),
                                      (uint32_t)emberUnsignedCommandArgument(1),
                                      (uint32_t)emberUnsignedCommandArgument(2),
                                      (uint32_t)emberUnsignedCommandArgument(3),
                                      (uint8_t) emberUnsignedCommandArgument(4),
                                      (uint32_t)emberUnsignedCommandArgument(5),
                                      (uint8_t) emberUnsignedCommandArgument(6),
                                      (uint8_t) emberUnsignedCommandArgument(7));
}

// plugin price-server co2-val clear <endpoint:1>
void emAfPriceServerCliCo2ValueClear(void)
{
  emberAfPluginPriceServerCo2ValueClear( (uint8_t)emberUnsignedCommandArgument(0) );
}

// plugin price-server co2-val print <endpoint:1>
void emAfPriceServerCliCo2ValuePrint(void)
{
  emberAfPrintCo2ValuesTable( (uint8_t)emberUnsignedCommandArgument(0) );
}

// plugin price-server tier-label set
void emAfPriceServerCliTierLabelSet(void)
{
  uint8_t len;
  uint8_t tierLabel[13];
  len = emberCopyStringArgument(7, tierLabel + 1, 12, false);

  tierLabel[0] = len;
  emberAfPluginPriceServerTierLabelSet((uint8_t) emberUnsignedCommandArgument(0),
                                       (uint8_t) emberUnsignedCommandArgument(1),
                                       (uint8_t) emberUnsignedCommandArgument(2),
                                       (uint32_t)emberUnsignedCommandArgument(3),
                                       (uint32_t)emberUnsignedCommandArgument(4),
                                       (uint32_t)emberUnsignedCommandArgument(5),
                                       (uint8_t) emberUnsignedCommandArgument(6),
                                       tierLabel);
}

// plugin price-server tier-label add-label
void emAfPriceServerCliTierLabelAddLabel(void)
{
  uint8_t len;
  uint8_t tierLabel[13];
  uint32_t tariffId;
  len = emberCopyStringArgument(3, tierLabel + 1, 12, false);
  tierLabel[0] = len;

  tariffId = (uint32_t)emberUnsignedCommandArgument(1);
  emberAfPluginPriceServerTierLabelAddLabel( (uint8_t)emberUnsignedCommandArgument(0),
                                             tariffId,
                                             (uint8_t)emberUnsignedCommandArgument(2),
                                             tierLabel);
}

// plugin price-server tier-label print <endpoint:1>
void emAfPriceServerCliTierLabelPrint(void)
{
  emberAfPrintTierLabelsTable( (uint8_t)emberUnsignedCommandArgument(0) );
}

// plugin price-server tier-label pub  <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <tableIndex:1>
void emAfPriceServerCliTierLabelPub(void)
{
  emberAfPluginPriceServerTierLabelPub((uint16_t)emberUnsignedCommandArgument(0),
                                       (uint8_t)emberUnsignedCommandArgument(1),
                                       (uint8_t)emberUnsignedCommandArgument(2),
                                       (uint8_t)emberUnsignedCommandArgument(3));
}

// plugin price-server co2-val pub <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <tableIndex:1>
void emAfPriceServerCliCo2ValuePub(void)
{
  emberAfPluginPriceServerCo2LabelPub((uint16_t)emberUnsignedCommandArgument(0),
                                      (uint8_t)emberUnsignedCommandArgument(1),
                                      (uint8_t)emberUnsignedCommandArgument(2),
                                      (uint8_t)emberUnsignedCommandArgument(3));
}

// plugin price-server billing-period add <endpoint:1> <startTime:4>
//                                        <issuerEventId:4> <providerId:4>
//                                        <providerId:4> <billingPeriodDuration:4>
//                                        <billingPeriodDuration:1> <tariffType:1>
void emAfPriceServerCliBillingPeriodAdd(void)
{
  emberAfPluginPriceServerBillingPeriodAdd((uint8_t) emberUnsignedCommandArgument(0),
                                           (uint32_t)emberUnsignedCommandArgument(1),
                                           (uint32_t)emberUnsignedCommandArgument(2),
                                           (uint32_t)emberUnsignedCommandArgument(3),
                                           (uint32_t)emberUnsignedCommandArgument(4),
                                           (uint8_t) emberUnsignedCommandArgument(5),
                                           (uint8_t) emberUnsignedCommandArgument(6));
}

// plugin price-server billing-period print <endpoint:1>
void emAfPriceServerCliBillingPeriodPrint(void)
{
  emberAfPrintBillingPeriodTable( (uint8_t) emberUnsignedCommandArgument(0) );
}

// plugin price-server billing-period pub
void emAfPriceServerCliBillingPeriodPub(void)
{
  emberAfPluginPriceServerBillingPeriodPub((uint16_t)emberUnsignedCommandArgument(0),
                                           (uint8_t)emberUnsignedCommandArgument(1),
                                           (uint8_t)emberUnsignedCommandArgument(2),
                                           (uint8_t)emberUnsignedCommandArgument(3));
}

void emAfPriceServerCliBillingPeriodRepeat(void)
{
  emAfPluginPriceServerBillingPeriodRepeat = (bool)emberUnsignedCommandArgument(0);
}

// plugin price-server consol-bill add  <endpoint:1> <startTime:4>
//                                      <issuerEventId:4> <providerId:4>
//                                      <billingPeriodDuration:4> <billingPeriodDurationType:1>
//                                      <tariffType:1> <consolidatedBill:4>
//                                      <currency:2> <billTrailingDigit:1>
void emAfPriceServerCliConsolidatedBillAdd(void)
{
  emberAfPluginPriceServerConsolidatedBillAdd((uint8_t) emberUnsignedCommandArgument(0),
                                              (uint32_t)emberUnsignedCommandArgument(1),
                                              (uint32_t)emberUnsignedCommandArgument(2),
                                              (uint32_t)emberUnsignedCommandArgument(3),
                                              (uint32_t)emberUnsignedCommandArgument(4),
                                              (uint8_t) emberUnsignedCommandArgument(5),
                                              (uint8_t) emberUnsignedCommandArgument(6),
                                              (uint32_t)emberUnsignedCommandArgument(7),
                                              (uint16_t)emberUnsignedCommandArgument(8),
                                              (uint8_t) emberUnsignedCommandArgument(9) );
}

// plugin price-server consol-bill print <endpoint:1> <index:1>
void emAfPriceServerCliConsolidatedBillPrint(void)
{
  emberAfPrintConsolidatedBillTableEntry( (uint8_t)emberUnsignedCommandArgument(0),
                                          (uint8_t)emberUnsignedCommandArgument(1) );
}

// plugin price-server consol-bill pub  <nodeId:2> <srcEp:1> <dstEp:1> <index:1>
void emAfPriceServerCliConsolidatedBillPub(void)
{
  emberAfPluginPriceServerConsolidatedBillPub( (uint16_t)emberUnsignedCommandArgument(0),
                                               (uint8_t)emberUnsignedCommandArgument(1),
                                               (uint8_t)emberUnsignedCommandArgument(2),
                                               (uint8_t)emberUnsignedCommandArgument(3) );
}

// plugin price-server cpp-event set <endpoint:1> <valid:1> <providerId:4> <issuerEventId:4> <startTime:4>
//                                  <durationInMins:2> <tariffType:1> <cppPriceTier:1> <cppAuth:1>
void emAfPriceServerCliCppEventSet(void)
{
  emberAfPluginPriceServerCppEventSet( (uint8_t) emberUnsignedCommandArgument(0),
                                       (uint8_t) emberUnsignedCommandArgument(1),
                                       (uint32_t)emberUnsignedCommandArgument(2),
                                       (uint32_t)emberUnsignedCommandArgument(3),
                                       (uint32_t)emberUnsignedCommandArgument(4),
                                       (uint16_t)emberUnsignedCommandArgument(5),
                                       (uint8_t) emberUnsignedCommandArgument(6),
                                       (uint8_t) emberUnsignedCommandArgument(7),
                                       (uint8_t) emberUnsignedCommandArgument(8));
}

// plugin price-server cpp-event pub <nodeId:2> <srcEndpoint:1> <dstEndpoint:1>
void emAfPriceServerCliCppEventPub(void)
{
  emberAfPluginPriceServerCppEventPub( (uint16_t)emberUnsignedCommandArgument(0),
                                       (uint8_t )emberUnsignedCommandArgument(1),
                                       (uint8_t )emberUnsignedCommandArgument(2) );
}

// plugin price-server cpp-event print <endpoint:1>
void emAfPriceServerCliCppEventPrint(void)
{
  emberAfPluginPriceServerCppEventPrint( (uint8_t)emberUnsignedCommandArgument(0) );
}

// plugin price-server credit-pmt pub <nodeId:2> <srcEndpoint:1> <dstEndpoint:1> <index:1>
void emAfPriceServerCliCreditPaymentPub(void)
{
  emberAfPluginPriceServerCreditPaymentPub( (uint16_t)emberUnsignedCommandArgument(0),
                                            (uint8_t )emberUnsignedCommandArgument(1),
                                            (uint8_t )emberUnsignedCommandArgument(2),
                                            (uint8_t )emberUnsignedCommandArgument(3) );
}

// plugin price-server credit-pmt set <endpoint:1> <index:1> <valid:1>
//                      <providerId:4> <issuerEventId:4> <creditPaymentDueDate:4>
//                      <creditPaymentOverdueAmount:4> <creditPaymentStatus:1> <creditPayment:4> <creditPaymentDate:4>
//                      <creditPaymentRef:STRING>
void emAfPriceServerCliCreditPaymentSet(void)
{
  uint8_t len;
  uint8_t creditPaymentRef[21];
  len = emberCopyStringArgument(10, creditPaymentRef + 1, 20, false);
  creditPaymentRef[0] = len;
  emberAfPluginPriceServerCreditPaymentSet( (uint8_t) emberUnsignedCommandArgument(0),
                                            (uint8_t) emberUnsignedCommandArgument(1),
                                            (uint8_t) emberUnsignedCommandArgument(2),
                                            (uint32_t)emberUnsignedCommandArgument(3),
                                            (uint32_t)emberUnsignedCommandArgument(4),
                                            (uint32_t)emberUnsignedCommandArgument(5),
                                            (uint32_t)emberUnsignedCommandArgument(6),
                                            (uint8_t) emberUnsignedCommandArgument(7),
                                            (uint32_t)emberUnsignedCommandArgument(8),
                                            (uint32_t)emberUnsignedCommandArgument(9),
                                            creditPaymentRef);
}

// plugin price-server currency-conv pub <nodeId:2> <srcEndpoint:1> <dstEndpoint:1>
void emAfPriceServerCliCurrencyConversionPub(void)
{
  emberAfPluginPriceServerCurrencyConversionPub( (uint16_t)emberUnsignedCommandArgument(0),
                                                 (uint8_t)emberUnsignedCommandArgument(1),
                                                 (uint8_t)emberUnsignedCommandArgument(2) );
}

// plugin price-server currency-conv set <endpoint:1> <valid:1> <providerId:4> <issuerEventId:4> <startTime:4>
//                                       <oldCurrency:2> <newCurrency:2> <conversionFactor:4>
//                                       <conversionFactorTrailingDigit:1> <currencyChangeControlFlags:4>
void emAfPriceServerCliCurrencyConversionSet(void)
{
  emberAfPluginPriceServerCurrencyConversionSet( (uint8_t) emberUnsignedCommandArgument(0),
                                                 (uint8_t) emberUnsignedCommandArgument(1),
                                                 (uint32_t)emberUnsignedCommandArgument(2),
                                                 (uint32_t)emberUnsignedCommandArgument(3),
                                                 (uint32_t)emberUnsignedCommandArgument(4),
                                                 (uint16_t)emberUnsignedCommandArgument(5),
                                                 (uint16_t)emberUnsignedCommandArgument(6),
                                                 (uint32_t)emberUnsignedCommandArgument(7),
                                                 (uint8_t) emberUnsignedCommandArgument(8),
                                                 (uint32_t)emberUnsignedCommandArgument(9) );
}

// plugin price-server tariff-cancel set <endpoint:1> <valid:1> <providerId:4> <issuerTariffId:4> <tariffType:1>
void emAfPriceServerCliTariffCancellationSet(void)
{
  emberAfPluginPriceServerTariffCancellationSet( (uint8_t) emberUnsignedCommandArgument(0),
                                                 (uint8_t) emberUnsignedCommandArgument(1),
                                                 (uint32_t)emberUnsignedCommandArgument(2),
                                                 (uint32_t)emberUnsignedCommandArgument(3),
                                                 (uint8_t) emberUnsignedCommandArgument(4) );
}

// plugin price-server tariff-cancel pub <nodeId:2> <srcEndpoint:1> <dstEndpoint:1>
void emAfPriceServerCliTariffCancellationPub(void)
{
  emberAfPluginPriceServerTariffCancellationPub( (uint16_t)emberUnsignedCommandArgument(0),
                                                 (uint8_t)emberUnsignedCommandArgument(1),
                                                 (uint8_t)emberUnsignedCommandArgument(2) );
}
