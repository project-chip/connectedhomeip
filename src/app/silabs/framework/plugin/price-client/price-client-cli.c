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
 * @brief CLI for the Price Client plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/price-client/price-client.h"

void emAfPriceClientCliPrint(void);
void emAfPriceClientCliTableClear(void);
void emAfPriceClientCliConversionFactorPrintByEventId(void);
void emAfPriceClientCliCalorificValuePrintByEventId(void);
void emAfPriceClientCliSetCppEventAuth(void);
void emAfPriceClientCliCo2ValueTablePrintCurrent(void);
void emAfPriceClientCliBillingPeriodPrintCurrent(void);
void emAfPriceClientCliTierLabelTablePrintTariffId(void);
void emAfPriceClientCliConsolidatedBillTablePrint(void);
void emAfPriceClientCliCppEventPrint(void);
void emAfPriceClientCliCreditPaymentTablePrint(void);
void emAfPriceClientCliCreditPaymentPrintEntryByEventId(void);
void emAfPriceClientClieCurrencyConversionPrintCurrentCurrency(void);

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginPriceClientCommands[] = {
  emberCommandEntryAction("print", emAfPriceClientCliPrint, "u", "Print the price info"),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

// plugin price-client init <endpoint:1>
void emAfPriceClientCliInit(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfPriceClusterClientInitCallback(endpoint);
}

// plugin price-client print <endpoint:1>
void emAfPriceClientCliPrint(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  emAfPluginPriceClientPrintInfo(endpoint);
}

// plugin price-client printEvent <endpoint:1> <issuerEventId:4>
void emAfPriceClientCliPrintEvent(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint32_t issuerEventId = (uint32_t)emberUnsignedCommandArgument(1);
  emAfPluginPriceClientPrintByEventId(endpoint, issuerEventId);
}

// plugin price-client table-clear <endpoint:1>
void emAfPriceClientCliTableClear(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  emAfPriceClearPriceTable(endpoint);
}

// plugin price-client conv-factor printEvent <endpoint:1> <issuerEventId:4>
void emAfPriceClientCliConversionFactorPrintByEventId(void)
{
  uint8_t  i;
  uint32_t issuerEventId;
  uint8_t  endpoint;
  endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  issuerEventId = (uint32_t)emberUnsignedCommandArgument(1);
  i = emAfPriceGetConversionFactorIndexByEventId(endpoint, issuerEventId);
  emAfPricePrintConversionFactorEntryIndex(endpoint, i);
}

// plugin price-client calf-value printEvent <endpoint:1> <issuerEventId:4>
void emAfPriceClientCliCalorificValuePrintByEventId(void)
{
  uint8_t  i;
  uint32_t issuerEventId;
  uint8_t  endpoint;
  endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  issuerEventId = (uint32_t)emberUnsignedCommandArgument(1);
  i = emAfPriceGetCalorificValueIndexByEventId(endpoint, issuerEventId);
  emAfPricePrintCalorificValueEntryIndex(endpoint, i);
}

// plugin price-client co2-value print <endpoint:1>
void emAfPriceClientCliCo2ValueTablePrintCurrent(void)
{
  uint8_t i;
  uint8_t  endpoint;
  endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  i = emberAfPriceClusterGetActiveCo2ValueIndex(endpoint);
  if ( i < EMBER_AF_PLUGIN_PRICE_CLIENT_CO2_TABLE_SIZE ) {
    emAfPricePrintCo2ValueTablePrintIndex(endpoint, i);
  } else {
    emberAfPriceClusterPrintln("NO CURRENT CO2 VALUE");
  }
}

// plugin price-client bill-period printCurrent <endpoint:1>
void emAfPriceClientCliBillingPeriodPrintCurrent(void)
{
  uint8_t  i;
  uint8_t  endpoint;
  endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  i = emAfPriceGetActiveBillingPeriodIndex(endpoint);
  emAfPricePrintBillingPeriodTableEntryIndex(endpoint, i);
}

// plugin price-client block-period printEvent <endpoint:1> <issuerEventId:4>
void emAfPriceClientCliBlockPeriodPrintEntryByEventId(void)
{
  uint32_t issuerEventId;
  uint8_t  endpoint;
  uint8_t  i;

  endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  issuerEventId = (uint32_t)emberUnsignedCommandArgument(1);
  i = emAfPriceGetBlockPeriodTableIndexByEventId(endpoint, issuerEventId);
  emAfPricePrintBlockPeriodTableIndex(endpoint, i);
}

// plugin price-client tier-label printTariff <endpoint:1> <issuerTariffId:4>
void emAfPriceClientCliTierLabelTablePrintTariffId(void)
{
  uint32_t issuerTariffId;
  uint8_t  i;
  uint8_t  endpoint;
  endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  issuerTariffId = (uint32_t)emberUnsignedCommandArgument(1);
  i = emAfPriceGetActiveTierLabelTableIndexByTariffId(endpoint, issuerTariffId);
  emAfPricePrintTierLabelTableEntryIndex(endpoint, i);
}

extern uint8_t emberAfPriceClusterDefaultCppEventAuthorization;

// plugin price-client cpp-event setAuth <cppEventAuth:1>
void emAfPriceClientCliSetCppEventAuth(void)
{
  emberAfPriceClusterDefaultCppEventAuthorization = (uint8_t)emberUnsignedCommandArgument(0);
}

// plugin price-client consol-bill print <endpoint:1> <index:1>
void emAfPriceClientCliConsolidatedBillTablePrint(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t i = (uint8_t)emberUnsignedCommandArgument(1);
  emAfPricePrintConsolidatedBillTableIndex(endpoint, i);
}

// plugin price-client consol-bill printEvent <endpoint:1> <issuerEventId:4>
void emAfPriceClientCliConsolidatedBillPrintEntryByEventId(void)
{
  uint8_t  i;
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint32_t issuerEventId = (uint32_t)emberUnsignedCommandArgument(1);
  i = emAfPriceConsolidatedBillTableGetIndexWithEventId(endpoint, issuerEventId);
  if ( i >= EMBER_AF_PLUGIN_PRICE_CLIENT_CONSOLIDATED_BILL_TABLE_SIZE ) {
    emberAfPriceClusterPrintln("NOT FOUND, Event ID=%d", issuerEventId);
  } else {
    emAfPricePrintConsolidatedBillTableIndex(endpoint, i);
  }
}

// plugin price-client consol-bill printCurrent <endpoint:1
void emAfPriceClientCliConsolidatedBillPrintCurrentEntry(void)
{
  uint8_t i;
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  i = emAfPriceConsolidatedBillTableGetCurrentIndex(endpoint);
  if ( i >= EMBER_AF_PLUGIN_PRICE_CLIENT_CONSOLIDATED_BILL_TABLE_SIZE ) {
    emberAfPriceClusterPrintln("NO CURRENT BILL");
  } else {
    emAfPricePrintConsolidatedBillTableIndex(endpoint, i);
  }
}

// plugin price-client cpp-event print <endpoint:1>
void emAfPriceClientCliCppEventPrint(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfPricePrintCppEvent(endpoint);
}

// plugin price-client credit-pmt print <endpoint:1> <index:1>
void emAfPriceClientCliCreditPaymentTablePrint(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint8_t i = (uint8_t)emberUnsignedCommandArgument(1);
  emAfPricePrintCreditPaymentTableIndex(endpoint, i);
}

// plugin price-client credit-pmt printEvent <endpoint:1> <issuerEventId:4>
void emAfPriceClientCliCreditPaymentPrintEntryByEventId(void)
{
  uint8_t  i;
  uint32_t endpoint = (uint32_t)emberUnsignedCommandArgument(0);
  uint32_t issuerEventId = (uint32_t)emberUnsignedCommandArgument(1);
  i = emAfPriceCreditPaymentTableGetIndexWithEventId(endpoint, issuerEventId);
  if ( i >= EMBER_AF_PLUGIN_PRICE_CLIENT_CREDIT_PAYMENT_TABLE_SIZE ) {
    emberAfPriceClusterPrintln("NOT FOUND, Event ID=%d", issuerEventId);
  } else {
    emAfPricePrintCreditPaymentTableIndex(endpoint, i);
  }
}

// plugin price-client currency-convers printEvent <endpoint:1> <issuerEventId:4>
void emAfPriceClientCliCurrencyConversionPrintByEventId(void)
{
  uint8_t i;
  uint8_t  endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  uint32_t issuerEventId = (uint32_t)emberUnsignedCommandArgument(1);
  i = emberAfPriceClusterCurrencyConversionTableGetIndexByEventId(endpoint, issuerEventId);
  if ( i >= EMBER_AF_PLUGIN_PRICE_CLIENT_CURRENCY_CONVERSION_TABLE_SIZE ) {
    emberAfPriceClusterPrintln("NOT FOUND, Event ID=%d", issuerEventId);
  } else {
    emAfPricePrintCurrencyConversionTableIndex(endpoint, i);
  }
}

// plugin price-client currency-convers printCurrent <endpoint:1>
void emAfPriceClientClieCurrencyConversionPrintCurrentCurrency(void)
{
  uint8_t i;
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  i = emberAfPriceClusterGetActiveCurrencyIndex(endpoint);
  if ( i < EMBER_AF_PLUGIN_PRICE_CLIENT_CURRENCY_CONVERSION_TABLE_SIZE ) {
    emAfPricePrintCurrencyConversionTableIndex(endpoint, i);
  } else {
    emberAfPriceClusterPrintln("NO CURRENT CURRENCY");
  }
}
