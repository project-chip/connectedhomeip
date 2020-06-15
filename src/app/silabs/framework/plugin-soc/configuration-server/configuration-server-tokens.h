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
 * @brief Tokens for the Configuration Server plugin.
 *******************************************************************************
   ******************************************************************************/

/**
 * Custom Application Tokens
 */

#define OTA_CONFIG_MAX_TOKEN_LENGTH 30

#define CREATOR_OTA_CONFIG_LOCK              (0x7000)
#define CREATOR_OTA_CONFIG_TX_POWER          (0x7001)
#define CREATOR_OTA_CONFIG_TX_POWER25        (0x7002)
#define CREATOR_OTA_CONFIG_TX_POWER26        (0x7003)
#define CREATOR_OTA_CONFIG_MODEL_NAME        (0x7004)
#define CREATOR_OTA_CONFIG_MANUFACTURER_NAME (0x7005)
#define CREATOR_OTA_CONFIG_HW_VERSION        (0x7006)

#define OTA_CONFIG_INVALID_TX_POWER    100
#define OTA_CONFIG_MODEL_NAME_SIZE         OTA_CONFIG_MAX_TOKEN_LENGTH
#define OTA_CONFIG_MANUFACTURER_NAME_SIZE  OTA_CONFIG_MAX_TOKEN_LENGTH
#define OTA_CONFIG_DEFAULT_HW_VERSION  0xFF
#define OTA_CONFIG_DISTRIBUTED_KEY_SIZE 16

#define CREATOR_BULB_PWM_FREQUENCY_HZ  (0x7007)
#define CREATOR_BULB_PWM_MIN_ON_US     (0x7008)
#define CREATOR_BULB_PWM_MAX_ON_US     (0x7009)

#define BULB_PWM_FREQ_DEFAULT          1000
#define BULB_PWM_TIME_UNSET            (0xffff)

#define CREATOR_BULB_UI_MIN_ON_TIME         (0x700a)
#define CREATOR_BULB_UI_TIMEOUT             (0x700b)
#define CREATOR_BULB_UI_POWER_UP_BEHAVIOR   (0x700c)
#define CREATOR_POWER_METER_CALIBRATION_CURRENT (0x700d)
#define CREATOR_OTA_CONFIG_DISTRIBUTED_KEY   (0x700e)
#define BULB_UI_MIN_ON_TIME_DEFAULT    100
#define BULB_UI_TIMEOUT_DEFAULT        10000
#define POWER_METER_CALIBRATION_CURRENT_DEFAULT 0xFFFF

#define NVM3KEY_OTA_CONFIG_LOCK                 (NVM3KEY_DOMAIN_USER | 0x7000)
#define NVM3KEY_OTA_CONFIG_TX_POWER             (NVM3KEY_DOMAIN_USER | 0x7001)
#define NVM3KEY_OTA_CONFIG_TX_POWER25           (NVM3KEY_DOMAIN_USER | 0x7002)
#define NVM3KEY_OTA_CONFIG_TX_POWER26           (NVM3KEY_DOMAIN_USER | 0x7003)
#define NVM3KEY_OTA_CONFIG_MODEL_NAME           (NVM3KEY_DOMAIN_USER | 0x7004)
#define NVM3KEY_OTA_CONFIG_MANUFACTURER_NAME    (NVM3KEY_DOMAIN_USER | 0x7005)
#define NVM3KEY_OTA_CONFIG_HW_VERSION           (NVM3KEY_DOMAIN_USER | 0x7006)
#define NVM3KEY_BULB_PWM_FREQUENCY_HZ           (NVM3KEY_DOMAIN_USER | 0x7007)
#define NVM3KEY_BULB_PWM_MIN_ON_US              (NVM3KEY_DOMAIN_USER | 0x7008)
#define NVM3KEY_BULB_PWM_MAX_ON_US              (NVM3KEY_DOMAIN_USER | 0x7009)
#define NVM3KEY_BULB_UI_MIN_ON_TIME             (NVM3KEY_DOMAIN_USER | 0x700a)
#define NVM3KEY_BULB_UI_TIMEOUT                 (NVM3KEY_DOMAIN_USER | 0x700b)
#define NVM3KEY_BULB_UI_POWER_UP_BEHAVIOR       (NVM3KEY_DOMAIN_USER | 0x700c)
#define NVM3KEY_POWER_METER_CALIBRATION_CURRENT (NVM3KEY_DOMAIN_USER | 0x700d)
#define NVM3KEY_OTA_CONFIG_DISTRIBUTED_KEY      (NVM3KEY_DOMAIN_USER | 0x700e)

#ifdef DEFINETYPES
// Include or define any typedef for tokens
typedef uint8_t ModelName[OTA_CONFIG_MODEL_NAME_SIZE];
typedef uint8_t ManufacturerName[OTA_CONFIG_MANUFACTURER_NAME_SIZE];
typedef uint8_t DistributedKey[OTA_CONFIG_DISTRIBUTED_KEY_SIZE];

enum {
  BULB_UI_POWER_UP_ON   = 0,
  BULB_UI_POWER_UP_OFF  = 1,
  BULB_UI_POWER_UP_LAST = 2
};

typedef uint8_t BulbUiPowerUpBehavior;

#endif //DEFINETYPES
#ifdef DEFINETOKENS
// Define the actual token storage information here

DEFINE_BASIC_TOKEN(OTA_CONFIG_LOCK, uint8_t, FALSE)
DEFINE_BASIC_TOKEN(OTA_CONFIG_TX_POWER, int8_t, OTA_CONFIG_INVALID_TX_POWER)
DEFINE_BASIC_TOKEN(OTA_CONFIG_TX_POWER25, int8_t, OTA_CONFIG_INVALID_TX_POWER)
DEFINE_BASIC_TOKEN(OTA_CONFIG_TX_POWER26, int8_t, OTA_CONFIG_INVALID_TX_POWER)
DEFINE_BASIC_TOKEN(OTA_CONFIG_MODEL_NAME, ModelName, "")
DEFINE_BASIC_TOKEN(OTA_CONFIG_MANUFACTURER_NAME, ManufacturerName, "")
DEFINE_BASIC_TOKEN(OTA_CONFIG_HW_VERSION, uint8_t, OTA_CONFIG_DEFAULT_HW_VERSION)
DEFINE_BASIC_TOKEN(OTA_CONFIG_DISTRIBUTED_KEY, DistributedKey, { 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF })

#if defined(EMBER_AF_PLUGIN_LED_RGB_PWM) || defined(EMBER_AF_PLUGIN_LED_TEMP_PWM) || defined(EMBER_AF_PLUGIN_LED_DIM_PWM)
DEFINE_BASIC_TOKEN(BULB_PWM_FREQUENCY_HZ, uint16_t, BULB_PWM_FREQ_DEFAULT)
DEFINE_BASIC_TOKEN(BULB_PWM_MIN_ON_US, uint16_t, BULB_PWM_TIME_UNSET)
DEFINE_BASIC_TOKEN(BULB_PWM_MAX_ON_US, uint16_t, BULB_PWM_TIME_UNSET)
#endif

#ifdef EMBER_AF_PLUGIN_BULB_UI
DEFINE_BASIC_TOKEN(BULB_UI_MIN_ON_TIME, uint16_t, BULB_UI_MIN_ON_TIME_DEFAULT)
DEFINE_BASIC_TOKEN(BULB_UI_TIMEOUT, uint16_t, BULB_UI_TIMEOUT_DEFAULT)
DEFINE_BASIC_TOKEN(BULB_UI_POWER_UP_BEHAVIOR, BulbUiPowerUpBehavior, BULB_UI_POWER_UP_LAST)
#endif

#if defined (EMBER_AF_PLUGIN_POWER_METER_CS5463)
DEFINE_BASIC_TOKEN(POWER_METER_CALIBRATION_CURRENT, uint16_t, POWER_METER_CALIBRATION_CURRENT_DEFAULT)
#endif //EMBER_AF_PLUGIN_POWER_METER_CS5463
#endif //DEFINETOKENS
