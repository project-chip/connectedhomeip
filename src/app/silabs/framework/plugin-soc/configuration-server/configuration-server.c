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
 * @brief Definitions for the Configuration Server plugin.
 *******************************************************************************
   ******************************************************************************/

// this file contains all the common includes for clusters in the util
#include "app/framework/include/af.h"
#include "app/framework/util/attribute-storage.h"
#if defined (EMBER_AF_PLUGIN_POWER_METER_CS5463)
#include EMBER_AF_API_POWER_METER
#endif

#define FALSE_UINT8_T 0
#define TRUE_UINT8_T  1
#define TOKEN_DATA_OFFSET 1
#define TOKEN_DATA_LENGTH 0
#define STRING_TERMINATOR 0

void emAfPluginConfigurationServerReadTokenDataFromCreator(uint16_t creator,
                                                           uint8_t *data);

bool emAfPluginConfigurationServerLocked(void)
{
  uint8_t lockTokenWrite;

  halCommonGetToken((uint8_t *) (&lockTokenWrite), TOKEN_OTA_CONFIG_LOCK);

  if (lockTokenWrite == FALSE_UINT8_T) {
    return false;
  } else {
    return true;
  }
}

void emberAfOtaConfigurationClusterServerInitCallback(uint8_t endpoint)
{
  uint8_t returnData[OTA_CONFIG_MAX_TOKEN_LENGTH + 1];
#if defined (EMBER_AF_PLUGIN_POWER_METER_CS5463)
  uint16_t currentMultiplier;
#endif
  // configure the model name here.
  MEMSET(returnData, 0, OTA_CONFIG_MAX_TOKEN_LENGTH + 1);

  halCommonGetToken((returnData + TOKEN_DATA_OFFSET),
                    TOKEN_OTA_CONFIG_MODEL_NAME);

  returnData[0] = strlen(returnData + TOKEN_DATA_OFFSET);

  if (returnData[1] != 0xff && returnData[0] != 0) {
    emberAfWriteServerAttribute(endpoint,
                                ZCL_BASIC_CLUSTER_ID,
                                ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID,
                                returnData,
                                ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
  }

  halCommonGetToken((returnData + TOKEN_DATA_OFFSET),
                    TOKEN_OTA_CONFIG_MANUFACTURER_NAME);

  returnData[0] = strlen(returnData + TOKEN_DATA_OFFSET);

  if (returnData[1] != 0xff && returnData[0] != 0) {
    emberAfWriteServerAttribute(endpoint,
                                ZCL_BASIC_CLUSTER_ID,
                                ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID,
                                returnData,
                                ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
  }

  halCommonGetToken(returnData, TOKEN_OTA_CONFIG_HW_VERSION);

  if (returnData[0] != 0xff) {
    emberAfWriteServerAttribute(endpoint,
                                ZCL_BASIC_CLUSTER_ID,
                                ZCL_HW_VERSION_ATTRIBUTE_ID,
                                returnData,
                                ZCL_INT8U_ATTRIBUTE_TYPE);
  }

#if defined (EMBER_AF_PLUGIN_POWER_METER_CS5463)
  halCommonGetToken(returnData, TOKEN_POWER_METER_CALIBRATION_CURRENT);
  currentMultiplier  = HIGH_LOW_TO_INT(returnData[1], returnData[0]);
  emberAfCorePrintln("Configuration Server: Current setting from Token: 0x%2x",
                     currentMultiplier);
  halSetCurrentGain(currentMultiplier);
#endif
}

bool emberAfOtaConfigurationClusterLockTokensCallback(void)
{
  uint8_t lockTokenWrite = (uint8_t) TRUE_UINT8_T;

  halCommonSetToken(TOKEN_OTA_CONFIG_LOCK, (uint8_t *) (&lockTokenWrite));

  return true;
}

/** @brief Configuration Cluster Cluster Read Tokens
 *
 * This function will read the data specified by the 16-bit token value and
 * generate the read response command.
 *
 * @param token   Ver.: always
 */
bool emberAfOtaConfigurationClusterReadTokensCallback(uint16_t token)
{
  uint8_t returnData[OTA_CONFIG_MAX_TOKEN_LENGTH + 1];
  emAfPluginConfigurationServerReadTokenDataFromCreator(token, returnData);

  // First, set up the outgoing command
  emberAfFillCommandOtaConfigurationClusterReturnToken(token, returnData);

  // now send to the current incoming device
  emberAfSendResponse();

  return true;
}

bool emberAfOtaConfigurationClusterSetTokenCallback(uint16_t token,
                                                    uint8_t* data)
{
#if defined (EMBER_AF_PLUGIN_POWER_METER_CS5463)
  uint16_t currentGainRefernceMa;
#endif
  if (emAfPluginConfigurationServerLocked()) {
    emberAfCorePrintln("Locked");
    return true;
  } else {
    emberAfCorePrintln("Unlocked");
  }

  switch (token) {
    case CREATOR_OTA_CONFIG_TX_POWER:
      halCommonSetToken(TOKEN_OTA_CONFIG_TX_POWER, (data + TOKEN_DATA_OFFSET));
      break;
    case CREATOR_OTA_CONFIG_TX_POWER25:
      halCommonSetToken(TOKEN_OTA_CONFIG_TX_POWER25, (data + TOKEN_DATA_OFFSET));
      break;
    case CREATOR_OTA_CONFIG_TX_POWER26:
      halCommonSetToken(TOKEN_OTA_CONFIG_TX_POWER26, (data + TOKEN_DATA_OFFSET));
      break;
    case CREATOR_OTA_CONFIG_MODEL_NAME:
      if (data[TOKEN_DATA_LENGTH] > OTA_CONFIG_MAX_TOKEN_LENGTH) {
        data[TOKEN_DATA_LENGTH] = OTA_CONFIG_MAX_TOKEN_LENGTH;
      }
      data[data[TOKEN_DATA_LENGTH] + 1] = STRING_TERMINATOR;
      halCommonSetToken(TOKEN_OTA_CONFIG_MODEL_NAME, (data + TOKEN_DATA_OFFSET));
      break;
    case CREATOR_OTA_CONFIG_MANUFACTURER_NAME:
      if (data[TOKEN_DATA_LENGTH] > OTA_CONFIG_MAX_TOKEN_LENGTH) {
        data[TOKEN_DATA_LENGTH] = OTA_CONFIG_MAX_TOKEN_LENGTH;
      }
      data[data[TOKEN_DATA_LENGTH] + 1] = STRING_TERMINATOR;
      halCommonSetToken(TOKEN_OTA_CONFIG_MANUFACTURER_NAME, (data + TOKEN_DATA_OFFSET));
      break;
    case CREATOR_OTA_CONFIG_HW_VERSION:
      halCommonSetToken(TOKEN_OTA_CONFIG_HW_VERSION, (data + TOKEN_DATA_OFFSET));
      break;
    case CREATOR_OTA_CONFIG_DISTRIBUTED_KEY:
      halCommonSetToken(TOKEN_OTA_CONFIG_DISTRIBUTED_KEY, (data + TOKEN_DATA_OFFSET));
      break;

#if defined(EMBER_AF_PLUGIN_LED_RGB_PWM) || defined(EMBER_AF_PLUGIN_LED_TEMP_PWM) || defined(EMBER_AF_PLUGIN_LED_DIM_PWM)
    case CREATOR_BULB_PWM_FREQUENCY_HZ:
      halCommonSetToken(TOKEN_BULB_PWM_FREQUENCY_HZ, (data + TOKEN_DATA_OFFSET));
      break;
    case CREATOR_BULB_PWM_MIN_ON_US:
      halCommonSetToken(TOKEN_BULB_PWM_MIN_ON_US, (data + TOKEN_DATA_OFFSET));
      break;
    case CREATOR_BULB_PWM_MAX_ON_US:
      halCommonSetToken(TOKEN_BULB_PWM_MAX_ON_US, (data + TOKEN_DATA_OFFSET));
      break;
#endif
#ifdef EMBER_AF_PLUGIN_BULB_UI
    case CREATOR_BULB_UI_MIN_ON_TIME:
      halCommonSetToken(TOKEN_BULB_UI_MIN_ON_TIME, (data + TOKEN_DATA_OFFSET));
      break;
    case CREATOR_BULB_UI_TIMEOUT:
      halCommonSetToken(TOKEN_BULB_UI_TIMEOUT, (data + TOKEN_DATA_OFFSET));
      break;
    case CREATOR_BULB_UI_POWER_UP_BEHAVIOR:
      halCommonSetToken(TOKEN_BULB_UI_POWER_UP_BEHAVIOR, (data + TOKEN_DATA_OFFSET));
      break;
#endif
#if defined (EMBER_AF_PLUGIN_POWER_METER_CS5463)
    case CREATOR_POWER_METER_CALIBRATION_CURRENT:
      currentGainRefernceMa = HIGH_LOW_TO_INT(*((data + TOKEN_DATA_OFFSET) + 1),
                                              *(data + TOKEN_DATA_OFFSET));
      emberAfCorePrintln("Configuration Server:  Reference Current: 0x%2x",
                         currentGainRefernceMa);
      if (halPowerMeterCalibrateCurrentGain(currentGainRefernceMa)) {
        emberAfCorePrintln("Configuration Server:  Calibration started!");
      } else {
        emberAfCorePrintln("Configuration Server:  Calibration failure!");
      }
      break;
#endif
    default:
      emberAfCorePrintln("Configuration Server:  Unsupported Token %2x",
                         token);
      break;
  }

  return true;
}

bool emberAfOtaConfigurationClusterUnlockTokensCallback(uint8_t* data)
{
  uint8_t lockTokenWrite = (uint8_t) FALSE_UINT8_T;
  uint8_t *eui64;
  uint8_t hashData[EUI64_SIZE];
  uint8_t i;

  eui64 = emberGetEui64();

  if (data[0] != EUI64_SIZE) {
    return false;
  }

  // simple hash of the EUI64;
  for (i = 0; i < EUI64_SIZE; i++) {
    hashData[EUI64_SIZE - i - 1] = data[i + 1] + 0x80;
  }

  if (MEMCOMPARE(eui64, hashData, 8) == 0) {
    halCommonSetToken(TOKEN_OTA_CONFIG_LOCK, (uint8_t *) (&lockTokenWrite));
  }

  return true;
}

void emAfPluginConfigurationServerReadTokenDataFromCreator(uint16_t creator,
                                                           uint8_t *data)
{
  uint8_t returnData[OTA_CONFIG_MAX_TOKEN_LENGTH + 1];
  switch (creator) {
    case CREATOR_OTA_CONFIG_TX_POWER:
      halCommonGetToken((uint8_t *) (data + TOKEN_DATA_OFFSET),
                        TOKEN_OTA_CONFIG_TX_POWER);
      break;
    case CREATOR_OTA_CONFIG_TX_POWER25:
      halCommonGetToken((uint8_t *) (data + TOKEN_DATA_OFFSET),
                        TOKEN_OTA_CONFIG_TX_POWER25);
      break;
    case CREATOR_OTA_CONFIG_TX_POWER26:
      halCommonGetToken((uint8_t *) (data + TOKEN_DATA_OFFSET),
                        TOKEN_OTA_CONFIG_TX_POWER26);
      break;
    case CREATOR_OTA_CONFIG_MODEL_NAME:
      returnData[data[0] + 1] = 0;
      halCommonGetToken((uint8_t *) (data + TOKEN_DATA_OFFSET),
                        TOKEN_OTA_CONFIG_MODEL_NAME);
      break;
    case CREATOR_OTA_CONFIG_MANUFACTURER_NAME:
      returnData[data[0] + 1] = 0;
      halCommonGetToken((uint8_t *) (data + TOKEN_DATA_OFFSET),
                        TOKEN_OTA_CONFIG_MANUFACTURER_NAME);
      break;
    case CREATOR_OTA_CONFIG_HW_VERSION:
      halCommonGetToken((uint8_t *) (data + TOKEN_DATA_OFFSET),
                        TOKEN_OTA_CONFIG_HW_VERSION);
      break;
    case CREATOR_OTA_CONFIG_DISTRIBUTED_KEY:
      halCommonGetToken((uint8_t *) (data + TOKEN_DATA_OFFSET),
                        TOKEN_OTA_CONFIG_DISTRIBUTED_KEY);
      break;

#if defined(EMBER_AF_PLUGIN_LED_RGB_PWM) || defined(EMBER_AF_PLUGIN_LED_TEMP_PWM) || defined(EMBER_AF_PLUGIN_LED_DIM_PWM)
    case CREATOR_BULB_PWM_FREQUENCY_HZ:
      halCommonGetToken((uint8_t *) (data + TOKEN_DATA_OFFSET),
                        TOKEN_BULB_PWM_FREQUENCY_HZ);
      break;
    case CREATOR_BULB_PWM_MIN_ON_US:
      halCommonGetToken((uint8_t *) (data + TOKEN_DATA_OFFSET),
                        TOKEN_BULB_PWM_MIN_ON_US);
      break;
    case CREATOR_BULB_PWM_MAX_ON_US:
      halCommonGetToken((uint8_t *) (data + TOKEN_DATA_OFFSET),
                        TOKEN_BULB_PWM_MAX_ON_US);
      break;
#endif
#ifdef EMBER_AF_PLUGIN_BULB_UI
    case CREATOR_BULB_UI_MIN_ON_TIME:
      halCommonGetToken((uint8_t *) (data + TOKEN_DATA_OFFSET),
                        TOKEN_BULB_UI_MIN_ON_TIME);
      break;
    case CREATOR_BULB_UI_TIMEOUT:
      halCommonGetToken((uint8_t *) (data + TOKEN_DATA_OFFSET),
                        TOKEN_BULB_UI_TIMEOUT);
      break;
    case CREATOR_BULB_UI_POWER_UP_BEHAVIOR:
      halCommonGetToken((uint8_t *) (data + TOKEN_DATA_OFFSET),
                        TOKEN_BULB_UI_POWER_UP_BEHAVIOR);
      break;
#endif
#if defined (EMBER_AF_PLUGIN_POWER_METER_CS5463)
    case CREATOR_POWER_METER_CALIBRATION_CURRENT:
      halCommonGetToken((uint8_t *) (data + TOKEN_DATA_OFFSET),
                        TOKEN_POWER_METER_CALIBRATION_CURRENT);
      break;
#endif
    default:
      break;
  }
}

// callbacks for reading configuration values.
/** @brief Get Power For Radio Channel
 *
 * This callback is fired when the Network Steering plugin needs to set the
 * power level. The application has the ability to change the max power level
 * used for this particular channel.
 *
 * @param channel The channel that the plugin is inquiring about the power
 * level. Ver.: always
 */
int8_t emberAfPluginNetworkSteeringGetPowerForRadioChannelCallback(uint8_t channel)
{
  int8_t power, defaultPower;

  switch (channel) {
    case 26:
      halCommonGetToken((uint8_t *) &power, TOKEN_OTA_CONFIG_TX_POWER26);
      defaultPower = EMBER_AF_PLUGIN_CONFIGURATION_SERVER_TX_POWER26;
      break;
    case 25:
      halCommonGetToken((uint8_t *) &power, TOKEN_OTA_CONFIG_TX_POWER25);
      defaultPower = EMBER_AF_PLUGIN_CONFIGURATION_SERVER_TX_POWER25;
      break;
    default:
      halCommonGetToken((uint8_t *) &power, TOKEN_OTA_CONFIG_TX_POWER);
      defaultPower = EMBER_AF_PLUGIN_CONFIGURATION_SERVER_TX_POWER;
      break;
  }

  emberAfCorePrintln("Config Tx Power:  %d %d %d",
                     channel,
                     power,
                     defaultPower);

  if (power == OTA_CONFIG_INVALID_TX_POWER) {
    return defaultPower;
  } else {
    return power;
  }
}

/** @brief Get Distributed Key
 *
 * This callback is fired when the Network Steering plugin needs to set the distributed
 * key. The application set the distributed key from Zigbee Alliance thru this callback
 * or the network steering will use the default test key.
 *
 * @param pointer to the distributed key struct
 * @return true if the key is loaded successfully, otherwise false.
 * level. Ver.: always
 */
bool emberAfPluginNetworkSteeringGetDistributedKeyCallback(EmberKeyData * key)
{
  halCommonGetToken((key->contents), TOKEN_OTA_CONFIG_DISTRIBUTED_KEY);

  return true;
}

#if defined(EMBER_AF_PLUGIN_LED_RGB_PWM) || defined(EMBER_AF_PLUGIN_LED_TEMP_PWM) || defined(EMBER_AF_PLUGIN_LED_DIM_PWM)
uint16_t emberAfPluginConfigurationServerReadMinOnTimeUs(void)
{
  uint16_t time;

  halCommonGetToken((uint8_t *) &time, TOKEN_BULB_PWM_MIN_ON_US);

  return time;
}

uint16_t emberAfPluginConfigurationServerReadMaxOnTimeUs(void)
{
  uint16_t time;

  halCommonGetToken((uint8_t *) &time, TOKEN_BULB_PWM_MAX_ON_US);

  return time;
}

uint16_t halBulbPwmDriverFrequencyCallback(void)
{
  uint16_t frequency;

  halCommonGetToken((uint8_t *) &frequency, TOKEN_BULB_PWM_FREQUENCY_HZ);

  // Note:  default value of token is the default value of the PWM frequency.
  return frequency;
}
#endif //(EMBER_AF_PLUGIN_LED_RGB_PWM) || defined(EMBER_AF_PLUGIN_LED_TEMP_PWM) || defined(EMBER_AF_PLUGIN_LED_DIM_PWM)

#if defined (EMBER_AF_PLUGIN_POWER_METER_CS5463)
void halPowerMeterCalibrationFinishedCallback(uint16_t gainSetting)
{
  halCommonSetToken(TOKEN_POWER_METER_CALIBRATION_CURRENT,
                    (uint8_t *)&gainSetting);
  emberAfCorePrintln("Configuration Server: Calibration gain setting : 0x%2x",
                     gainSetting);
}
#endif
