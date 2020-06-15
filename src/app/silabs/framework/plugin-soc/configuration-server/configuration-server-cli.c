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
 * @brief CLI for the Configuration Server plugin.
 *******************************************************************************
   ******************************************************************************/

// this file contains all the common includes for clusters in the util
#include "app/framework/include/af.h"
#include "app/framework/util/attribute-storage.h"

#define MAX_TOKEN_LENGTH OTA_CONFIG_MAX_TOKEN_LENGTH

bool emAfPluginConfigurationServerLocked(void);

void emAfConfigurationServerLockCommand(void)
{
  uint8_t lockTokenWrite = (uint8_t) emberUnsignedCommandArgument(0);

  halCommonSetToken(TOKEN_OTA_CONFIG_LOCK, (int8u *) (&lockTokenWrite));
}

void emAfConfigurationServerSetCommand(void)
{
  uint8_t tokenData[MAX_TOKEN_LENGTH + 1];
  uint16_t creator = (uint16_t)emberUnsignedCommandArgument(0);

  emberCopyStringArgument(1,
                          tokenData + 1,
                          MAX_TOKEN_LENGTH,
                          FALSE);

  tokenData[0] = 0xff;

  // Don't want to write stack tokens.  Also, don't want to write if this
  // has been locked down.
  if (emAfPluginConfigurationServerLocked() || creator >= 0x8000) {
    return;
  }

  emberAfOtaConfigurationClusterSetTokenCallback(creator, tokenData);
}

// Note:  One of our internal tests flags unused variables.  It would be easier
// to perform all of these reads in the same function below.  But it would make
// the code more complicated to optionally include the unsignedData16 variable
// and not fail this test.  The easiest way to avoid this issue is to create
// conditionally stubbable functions.
static void readColorConfigData(void)
{
#if defined(EMBER_AF_PLUGIN_LED_RGB_PWM) || defined(EMBER_AF_PLUGIN_LED_TEMP_PWM) || defined(EMBER_AF_PLUGIN_LED_DIM_PWM)
  uint16_t unsignedData16;

  halCommonGetToken((uint8_t *) &unsignedData16, TOKEN_BULB_PWM_FREQUENCY_HZ);
  emberAfCorePrintln("PWM FREQUENCY %2x %d",
                     CREATOR_BULB_PWM_FREQUENCY_HZ,
                     unsignedData16);
  halCommonGetToken((uint8_t *) &unsignedData16, TOKEN_BULB_PWM_MIN_ON_US);
  emberAfCorePrintln("PWM MIN_ON %2x %d",
                     CREATOR_BULB_PWM_MIN_ON_US,
                     unsignedData16);
  halCommonGetToken((uint8_t *) &unsignedData16, TOKEN_BULB_PWM_MAX_ON_US);
  emberAfCorePrintln("PWM MAX_ON %2x %d",
                     CREATOR_BULB_PWM_MAX_ON_US,
                     unsignedData16);
#endif
}

static void readBulbUiData(void)
{
#ifdef EMBER_AF_PLUGIN_BULB_UI
  uint16_t unsignedData16;
  uint8_t  unsignedData8;

  halCommonGetToken((uint8_t *) &unsignedData16, TOKEN_BULB_UI_MIN_ON_TIME);
  emberAfCorePrintln("UI MIN ON %2x %d",
                     CREATOR_BULB_UI_MIN_ON_TIME,
                     unsignedData16);
  halCommonGetToken((uint8_t *) &unsignedData16, TOKEN_BULB_UI_TIMEOUT);
  emberAfCorePrintln("UI MAX ON %2x %d",
                     CREATOR_BULB_UI_TIMEOUT,
                     unsignedData16);
  halCommonGetToken( (uint8_t *) &unsignedData8,
                     TOKEN_BULB_UI_POWER_UP_BEHAVIOR);
  emberAfCorePrintln("UI POWER UP BEHAVIOR %2x %d",
                     CREATOR_BULB_UI_POWER_UP_BEHAVIOR,
                     unsignedData8);
#endif
}

static void readPowerMeterData(void)
{
#if defined (EMBER_AF_PLUGIN_POWER_METER_CS5463)
  uint16_t unsignedData16;

  halCommonGetToken((uint8_t *) &unsignedData16,
                    TOKEN_POWER_METER_CALIBRATION_CURRENT);
  emberAfCorePrintln("CURRENT GAIN SETTING %2x 0x%4x",
                     CREATOR_POWER_METER_CALIBRATION_CURRENT,
                     unsignedData16);
#endif
}

void emAfConfigurationServerReadCommand(void)
{
  uint8_t  unsignedData8;
  int8_t   signedData8;
  uint8_t tokenData[MAX_TOKEN_LENGTH + 1];

  halCommonGetToken((uint8_t *) &unsignedData8, TOKEN_OTA_CONFIG_LOCK);
  emberAfCorePrintln("LOCK %2x %d",
                     CREATOR_OTA_CONFIG_LOCK,
                     unsignedData8);
  halCommonGetToken((uint8_t *) &signedData8, TOKEN_OTA_CONFIG_TX_POWER);
  if (signedData8 == OTA_CONFIG_INVALID_TX_POWER) {
    emberAfCorePrintln("TX_POWER %2x %d (%d)",
                       CREATOR_OTA_CONFIG_TX_POWER,
                       signedData8,
                       EMBER_AF_PLUGIN_CONFIGURATION_SERVER_TX_POWER);
  } else {
    emberAfCorePrintln("TX_POWER %2x %d",
                       CREATOR_OTA_CONFIG_TX_POWER,
                       signedData8);
  }
  halCommonGetToken((uint8_t *) &signedData8, TOKEN_OTA_CONFIG_TX_POWER25);
  if (signedData8 == OTA_CONFIG_INVALID_TX_POWER) {
    emberAfCorePrintln("TX_POWER_25 %2x %d (%d)",
                       CREATOR_OTA_CONFIG_TX_POWER25,
                       signedData8,
                       EMBER_AF_PLUGIN_CONFIGURATION_SERVER_TX_POWER25);
  } else {
    emberAfCorePrintln("TX_POWER_25 %2x %d",
                       CREATOR_OTA_CONFIG_TX_POWER25,
                       signedData8);
  }
  halCommonGetToken((uint8_t *) &signedData8, TOKEN_OTA_CONFIG_TX_POWER26);
  if (signedData8 == OTA_CONFIG_INVALID_TX_POWER) {
    emberAfCorePrintln("TX_POWER_26 %2x %d (%d)",
                       CREATOR_OTA_CONFIG_TX_POWER26,
                       signedData8,
                       EMBER_AF_PLUGIN_CONFIGURATION_SERVER_TX_POWER26);
  } else {
    emberAfCorePrintln("TX_POWER_26 %2x %d",
                       CREATOR_OTA_CONFIG_TX_POWER26,
                       signedData8);
  }
  halCommonGetToken((uint8_t *) tokenData, TOKEN_OTA_CONFIG_MODEL_NAME);
  emberAfCorePrintln("MODEL_NAME %2x %s",
                     CREATOR_OTA_CONFIG_MODEL_NAME,
                     tokenData);
  halCommonGetToken((uint8_t *) tokenData, TOKEN_OTA_CONFIG_MANUFACTURER_NAME);
  emberAfCorePrintln("MANUFACTURER_NAME %2x %s",
                     CREATOR_OTA_CONFIG_MANUFACTURER_NAME,
                     tokenData);
  halCommonGetToken((uint8_t *) &unsignedData8, TOKEN_OTA_CONFIG_HW_VERSION);
  emberAfCorePrintln("HW_VERSION %2x %d",
                     CREATOR_OTA_CONFIG_HW_VERSION,
                     unsignedData8);

  readColorConfigData();
  readBulbUiData();
  readPowerMeterData();
}
