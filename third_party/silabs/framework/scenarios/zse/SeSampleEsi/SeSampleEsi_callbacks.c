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
 * @brief
 *******************************************************************************
   ******************************************************************************/

// Copyright 2013 Silicon Laboratories, Inc.
//
//
// This callback file is created for your convenience. You may add application code
// to this file. If you regenerate this file over a previous version, the previous
// version will be overwritten and any code you have added will be lost.

#include "app/framework/include/af.h"
#include "app/framework/plugin/price-server/price-server.h"

//------------------------------------------------------------------------------
// Globals

static const uint8_t normalPriceLabel[] = "Normal Price";
static const uint8_t salePriceLabel[]   = "SALE! Buy now to save!";

static const uint32_t normalPrice = 40;  // $0.40
static const uint32_t salePrice = 3;    // $0.13

static EmberAfScheduledPrice myPrice = {
  "",         // price label (filled in later)
  0xABCD,     // provider Id     (made-up)
  0x1234,     // issuer event ID (made-up)
  0,          // Start time (0 = now)
  0,          // Price according to Current and trailing digit (set later)
  0xFFFFFFFFU,// Generation price (all F's = unused)
  0xFFFFFFFFU,// Alternate cost delivered (e.g. cost in CO2 emissions)
              //   (all F's = unused)
  840,        // Currency: US Dollar See ISO 4217: http://en.wikipedia.org/wiki/ISO_4217
  0xFFFF,     // Duration (0xFFFF = until changed)
  0x00,       // unit of measure (kwh in pure binary format, see Table D.22 in 07-5356-17
  0x20,       // Price Trailing Digit and Price Tier:
              //   High nibble = digits to the right of decimal point
              //   Low nibble = Price Tier (0 = no tier)
  0,          // Number of price tiers and tier
              //   High nibble = maximum number of tiers (0 = no tiers)
              //   Low nibble = Price Tier (0 = no tier)
  0xFF,       // Price ratio to the "normal" price.  0xFF = unused
  0xFF,       // Generation price ratio (0xFF = unused)
  0xFF,       // Alternate cost unit
  0xFF,       // Alternate cost trailing digit
  0,          // Number of block threshholds
  0,          // Price control (i.e. Price Ack or Repeating Block)
};

#define NORMAL_PRICE false
#define SALE_PRICE   true

#define PRICE_CHANGE_DELAY_MINUTES 1

EmberEventControl priceEvent;

//------------------------------------------------------------------------------
// Forward Declarations

static void setupPrice(bool onSale);

//------------------------------------------------------------------------------
// Functions

void priceEventChange(void)
{
  setupPrice(normalPrice == myPrice.price);
  emberEventControlSetDelayMinutes(priceEvent, PRICE_CHANGE_DELAY_MINUTES);
}

static void setupPrice(bool onSale)
{
  const uint8_t* label = (onSale ? salePriceLabel : normalPriceLabel);
  uint8_t labelSize = (onSale ? sizeof(salePriceLabel) : sizeof(normalPriceLabel));
  MEMSET(myPrice.rateLabel, 0, ZCL_PRICE_CLUSTER_MAXIMUM_RATE_LABEL_LENGTH + 1);
  MEMMOVE(myPrice.rateLabel, label, labelSize);
  myPrice.price = (onSale ? salePrice : normalPrice);

  // One of the few times '%s' is appropriate.  We are using a RAM string
  // %p is used for all CONST strings (which is the norm)
  emberAfCorePrintln("%s", myPrice.rateLabel);

  // This print assumes 2 digit prices less than 1 dollar
  emberAfCorePrintln("Price: $0.%d/kwh\n", myPrice.price);
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be
 * notified of changes to the stack status and take appropriate action.  The
 * return code from this callback is ignored by the framework.  The framework
 * will always process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
bool emberAfStackStatusCallback(EmberStatus status)
{
  if (status == EMBER_NETWORK_UP) {
    priceEventChange();
  }

  return false;
}

/** @brief Broadcast Sent
 *
 * This function is called when a new MTORR broadcast has been successfully
 * sent by the concentrator plugin.
 *
 */
void emberAfPluginConcentratorBroadcastSentCallback(void)
{
}

/** @brief Finished
 *
 * This callback is fired when the network-find plugin is finished with the
 * forming or joining process.  The result of the operation will be returned
 * in the status parameter.
 *
 * @param status   Ver.: always
 */
void emberAfPluginNetworkFindFinishedCallback(EmberStatus status)
{
}

/** @brief Get Radio Power For Channel
 *
 * This callback is called by the framework when it is setting the radio power
 * during the discovery process. The framework will set the radio power
 * depending on what is returned by this callback.
 *
 * @param channel   Ver.: always
 */
int8_t emberAfPluginNetworkFindGetRadioPowerForChannelCallback(uint8_t channel)
{
  return EMBER_AF_PLUGIN_NETWORK_FIND_RADIO_TX_POWER;
}

/** @brief Join
 *
 * This callback is called by the plugin when a joinable network has been
 * found.  If the application returns true, the plugin will attempt to join
 * the network.  Otherwise, the plugin will ignore the network and continue
 * searching.  Applications can use this callback to implement a network
 * blacklist.
 *
 * @param networkFound   Ver.: always
 * @param lqi   Ver.: always
 * @param rssi   Ver.: always
 */
bool emberAfPluginNetworkFindJoinCallback(EmberZigbeeNetwork * networkFound,
                                          uint8_t lqi,
                                          int8_t rssi)
{
  return true;
}

/** @brief Button Event
 *
 * This allows another module to get notification when a button is pressed and
 * the button joining plugin does not handle it.  For example, if the device
 * is already joined to the network and button 0 is pressed.  Button 0
 * normally forms or joins a network.  This callback is NOT called in ISR
 * context so there are no restrictions on what code can execute.
 *
 * @param buttonNumber The button number that was pressed.  Ver.: always
 */
void emberAfPluginButtonJoiningButtonEventCallback(uint8_t buttonNumber,
                                                   uint32_t buttonPressDurationMs)
{
  if (buttonNumber == 1) {
    priceEventChange();
  }
}

/** @brief Main Tick
 *
 * Whenever main application tick is called, this callback will be called at
 * the end of the main tick execution.
 *
 */
void emberAfMainTickCallback(void)
{
  static bool executedAlready = false;
  EmberNodeType nodeType;
  EmberNetworkParameters parameters;

  if (executedAlready) {
    return;
  }

  if (EMBER_SUCCESS != emberAfGetNetworkParameters(&nodeType,
                                                   &parameters)) {
    // Form a network for the first time on one of the preferred
    // Smart Energy Channels.  Scan for an unused short pan ID.
    // Long PAN ID is randomly generated.
    emberAfFindUnusedPanIdAndFormCallback();

    // Subsequent reboots will cause the device to re-initalize
    // the previous network parameters and start running again.
  }

  executedAlready = true;
}
