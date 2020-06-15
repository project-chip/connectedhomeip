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
 * @brief Definitions for the LED Temp PWM plugin.
 *******************************************************************************
   ******************************************************************************/

#ifdef EMBER_SCRIPTED_TEST
  #include "led-temp-pwm-test.h"
#endif

#include "app/framework/include/af.h"
#include "app/framework/util/attribute-storage.h"
#include EMBER_AF_API_BULB_PWM_DRIVER

#include "app/framework/plugin-soc/led-temp-pwm/led-temp-pwm.h"

#ifdef EMBER_AF_PLUGIN_SCENES
  #include "app/framework/plugin/scenes/scenes.h"
#endif //EMBER_AF_PLUGIN_SCENES

#ifdef EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER
  #include "app/framework/plugin/zll-level-control-server/zll-level-control-server.h"
#endif //EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER

#ifdef EMBER_AF_PLUGIN_CONFIGURATION_SERVER
  #include "app/framework/plugin-soc/configuration-server/configuration-server.h"
#endif

#define MIN_ON_TIME_MICROSECONDS 0
#ifndef BULB_PWM_WHITE
  #define BULB_PWM_WHITE 1
#endif

#ifndef BULB_PWM_LOWTEMP
  #define BULB_PWM_LOWTEMP 2
#endif

#define OFF_TICKS 0
#define MAX_LEVEL 254

// ---------- Hardware values required for computing drive levels ----------
static uint32_t minTemperature = 0, maxTemperature = 0;
static uint16_t minPwmDrive, maxPwmDrive;
static bool onoffStartupDone;
static bool levelStartupDone;
static bool blinking;

#define MIN_MIREDS_DEFAULT 200
#define MAX_MIREDS_DEFAULT 454

#define MIRED_CONSTANT  1000000 // color temperature is specified in MIREDS
// which is 1000000 / temp (in k)

// handle transitions between on and off state.
enum {
  ON_OFF_STATE_OFF           = 0,
  ON_OFF_STATE_ON            = 1,
};

#define SUCCESS_CODE 0

static uint8_t onOffState;

static void doInit(void);
static uint16_t minDriveValue(void);
static uint16_t maxDriveValue(void);
static void computeLoHighFromColorTemp(uint8_t endpoint);

static uint8_t currentEndpoint(void)
{
  // Note:  LED bulbs only support one endpoint
  assert(emberAfEndpointCount() == 1);

  return(emberAfEndpointFromIndex(0));
}

static void driveTempHiLo(uint16_t white, uint16_t lowtemp)
{
  halBulbPwmDriverSetPwmLevel(white, BULB_PWM_WHITE);
  halBulbPwmDriverSetPwmLevel(lowtemp, BULB_PWM_LOWTEMP);
}

static uint8_t readOnOffAttribute(void)
{
  uint8_t endpoint = currentEndpoint();
  uint8_t onOff;

  assert(EMBER_ZCL_STATUS_SUCCESS
         == emberAfReadServerAttribute(endpoint,
                                       ZCL_ON_OFF_CLUSTER_ID,
                                       ZCL_ON_OFF_ATTRIBUTE_ID,
                                       (uint8_t *)&onOff,
                                       sizeof(onOff)));
  return onOff;
}

static uint8_t readLevelAttribute(void)
{
  uint8_t endpoint = currentEndpoint();
  uint8_t level;

  assert(EMBER_ZCL_STATUS_SUCCESS
         == emberAfReadServerAttribute(endpoint,
                                       ZCL_LEVEL_CONTROL_CLUSTER_ID,
                                       ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                                       (uint8_t *)&level,
                                       sizeof(level)));
  return level;
}

static void initOnOffState(void)
{
  uint8_t onOff;

  onOff = readOnOffAttribute();

  if (onOff == 0) {
    onOffState = ON_OFF_STATE_OFF;
  } else {
    onOffState = ON_OFF_STATE_ON;
  }
}

// When we have on off and level control, the on/off attribute changes before
// the current level finishes its transition.  When turning off, this can lead
// to a bad behavior of not having the fade.
// To solve this issue, we need to create a filtering function that will only
// allow transition between on and off when the current level is set to zero.
static void readFilteredOnOffAndLevel(uint8_t *onOff, uint8_t *level)
{
  *onOff = readOnOffAttribute();
  *level = readLevelAttribute();

  // see function comment for an explanation of what is going on here.
  if (onOffState == ON_OFF_STATE_OFF) {
    if (*onOff == ON_OFF_STATE_ON) {
      onOffState = ON_OFF_STATE_ON;
    }
  } else {
    if (*onOff == ON_OFF_STATE_OFF) {
      if (*level == 0) {
        onOffState = ON_OFF_STATE_OFF;
      } else {
        *onOff = ON_OFF_STATE_ON;
      }
    }
  }
}

// Defer initialization until after OnOff and CurrentLevel attribute values are established
// by their respective StartUp settings. The "actionable" function computeLoHighFromColorTemp()
// blocks until both minTemperature and maxTemperature variables are set to nonzero values here.
void doInit(void)
{
  // Only proceed when both onoff and level startup settings have completed.
  if (onoffStartupDone && levelStartupDone) {
    onoffStartupDone = false;
    levelStartupDone = false;
    uint8_t endpoint = currentEndpoint();

    uint16_t miredsValue;
    EmberAfStatus status;

    blinking = false;

    minPwmDrive = minDriveValue();
    maxPwmDrive = maxDriveValue();

    initOnOffState();

    status = emberAfReadServerAttribute(endpoint,
                                        ZCL_COLOR_CONTROL_CLUSTER_ID,
                                        ZCL_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MIN_ATTRIBUTE_ID,
                                        (uint8_t *)&miredsValue,
                                        sizeof(miredsValue));

    if (miredsValue == 0) {
      miredsValue = 1;
    }

    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      maxTemperature = (uint32_t) MIN_MIREDS_DEFAULT;
    } else {
      maxTemperature = MIRED_CONSTANT / (uint32_t) miredsValue;
    }

    status = emberAfReadServerAttribute(endpoint,
                                        ZCL_COLOR_CONTROL_CLUSTER_ID,
                                        ZCL_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MAX_ATTRIBUTE_ID,
                                        (uint8_t *)&miredsValue,
                                        sizeof(miredsValue));

    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      minTemperature = (uint32_t) MAX_MIREDS_DEFAULT;
    } else {
      minTemperature = MIRED_CONSTANT / (uint32_t) miredsValue;
    }

    computeLoHighFromColorTemp(endpoint);
  }
}

/** @brief On/off Cluster Server Post Init
 *
 * Following resolution of the On/Off state at startup for this endpoint, perform any
 * additional initialization needed; e.g., synchronize hardware state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginOnOffClusterServerPostInitCallback(uint8_t endpoint)
{
  onoffStartupDone = true;
  doInit();
}

/** @brief Level Control Cluster Server Post Init
 *
 * Following resolution of the Current Level at startup for this endpoint,
 * perform any additional initialization needed; e.g., synchronize hardware
 * state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginLevelControlClusterServerPostInitCallback(uint8_t endpoint)
{
  levelStartupDone = true;
  doInit();
}

void emberAfPluginLedTempPwmInitCallback(void)
{
  // Await initialization based on StartUpOnOff and StartUpCurrentLevel.
}

static void computeLoHighFromColorTemp(uint8_t endpoint)
{
  uint16_t currentMireds;
  uint32_t currentTemp;
  uint8_t onOff, currentLevel;

  uint32_t high32, low32;
  uint16_t highDrive, lowDrive;

  if (blinking) {
    // we are in a mode where we are blinking an output pattern.  Don't blink
    // anything
    return;
  }

  emberAfCorePrintln("max: %d, min %d", maxTemperature, minTemperature);

  // during framework init, this funciton sometimes is called before we set up
  // the values for max/min color temperautre.
  if (maxTemperature == 0 || minTemperature == 0) {
    return;
  }

  readFilteredOnOffAndLevel(&onOff, &currentLevel);
  emberAfCorePrintln("on/off: %d, level %d", onOff, currentLevel);

  if (onOff == 0 || currentLevel == 0) {
    driveTempHiLo(OFF_TICKS, OFF_TICKS);

    return;
  }

  emberAfReadServerAttribute(endpoint,
                             ZCL_COLOR_CONTROL_CLUSTER_ID,
                             ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID,
                             (uint8_t *)&currentMireds,
                             sizeof(currentMireds));

  currentTemp = MIRED_CONSTANT;
  currentTemp /= (uint32_t) currentMireds;

  //bounds checking of the attribute temp.
  if (currentTemp > maxTemperature) {
    currentTemp = maxTemperature;
  } else if (currentTemp < minTemperature) {
    currentTemp = minTemperature;
  }

  // Compute the low and high drive based on linear interpolation.
  // y=mx+b, where y = low temperature LED drive,
  // m = (PWM Range) / (color range)
  // x = color temp - minimum color temperature
  // b = minimum PWM drive.
  high32 = maxPwmDrive - minPwmDrive;
  high32 *= (currentTemp - minTemperature);
  high32 /= (maxTemperature - minTemperature);
  high32 += minPwmDrive;

  // y=mx+b, where y = high temperature LED drive
  // m = (PWM Range)/(color range)
  // x = maxColor - currentTemp
  // b = minPwmDrive.
  low32 = maxPwmDrive - minPwmDrive;
  low32 *= (maxTemperature - currentTemp);
  low32 /= (maxTemperature - minTemperature);
  low32 += minPwmDrive;

  // Re-factor PWM drive values based on the current level.
  high32 *= currentLevel;
  high32 /= MAX_LEVEL;
  if (high32 < minPwmDrive && high32 != 0) {
    high32 = minPwmDrive;
  }

  low32 *= currentLevel;
  low32 /= MAX_LEVEL;
  if (low32 < minPwmDrive && low32 != 0) {
    low32 = minPwmDrive;
  }

  // convert to uint16_t and drive the PWMs.
  highDrive = (uint16_t) high32;
  lowDrive = (uint16_t) low32;

  driveTempHiLo(highDrive, lowDrive);
}

/** @brief Server Attribute Changedyes.
 *
 * Level Control cluster, Server Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
void emberAfLevelControlClusterServerAttributeChangedCallback(uint8_t endpoint,
                                                              EmberAfAttributeId attributeId)
{
  computeLoHighFromColorTemp(endpoint);
}

void emberAfOnOffClusterServerAttributeChangedCallback(uint8_t endpoint,
                                                       EmberAfAttributeId attributeId)
{
  computeLoHighFromColorTemp(endpoint);
}

/** @brief Color Control Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
void emberAfColorControlClusterServerAttributeChangedCallback(uint8_t endpoint,
                                                              EmberAfAttributeId attributeId)
{
  computeLoHighFromColorTemp(endpoint);
}

// Part of the bulb driver is the minimum/maximum drive times as well as
// the frequency.
static uint16_t minDriveValue(void)
{
  uint16_t minDriveValue;

#ifdef EMBER_AF_PLUGIN_CONFIGURATION_SERVER
  minDriveValue = emberAfPluginConfigurationServerReadMinOnTimeUs();
  if (minDriveValue == BULB_PWM_TIME_UNSET) {
    minDriveValue = MIN_ON_TIME_MICROSECONDS;
  }
#else
  minDriveValue = MIN_ON_TIME_MICROSECONDS;
#endif

  minDriveValue *= halBulbPwmDriverTicksPerMicrosecond();

  return minDriveValue;
}

static uint16_t maxDriveValue(void)
{
  uint16_t maxDriveValue;

#ifdef EMBER_AF_PLUGIN_CONFIGURATION_SERVER
  maxDriveValue = emberAfPluginConfigurationServerReadMaxOnTimeUs();
  if (maxDriveValue == BULB_PWM_TIME_UNSET) {
    maxDriveValue = halBulbPwmDriverTicksPerPeriod();
  } else {
    maxDriveValue *= halBulbPwmDriverTicksPerMicrosecond();
  }
#else
  maxDriveValue = halBulbPwmDriverTicksPerPeriod();
#endif

  return maxDriveValue;
}

// **********************************************
// HAL callbacks.

/** @brief After the PWM driver has been initialized, we need to kick off
 * the first value.
 *
 * @appusage Should be implemented by an applicaiton layer configuration
 * plugin.
 *
 */
void halBulbPwmDriverInitCompleteCallback(void)
{
  computeLoHighFromColorTemp(currentEndpoint());
}

/** @brief During blinking behavior, we rely on the bulb implementation code
 * to tell us how to turn the bulb on.
 *
 * @appusage Should be implemented by an applicaiton layer configuration
 * plugin.
 *
 */
void halBulbPwmDriverBlinkOnCallback(void)
{
  halBulbPwmDriverSetPwmLevel(halBulbPwmDriverTicksPerPeriod(),
                              BULB_PWM_WHITE);
  halBulbPwmDriverSetPwmLevel(halBulbPwmDriverTicksPerPeriod(),
                              BULB_PWM_LOWTEMP);
}

/** @brief During blinking behavior, we rely on the bulb implementation code
 * to tell us how to turn the bulb off.
 *
 * @appusage Should be implemented by an applicaiton layer configuration
 * plugin.
 *
 */
void halBulbPwmDriverBlinkOffCallback(void)
{
  halBulbPwmDriverSetPwmLevel(OFF_TICKS, BULB_PWM_WHITE);
  halBulbPwmDriverSetPwmLevel(OFF_TICKS, BULB_PWM_LOWTEMP);
}

/** @brief When we start blinking behavior, we need to tell the bulb
 * implementation code to hold off on any changes requried from the
 * application.  I.e. don't change the dim level or color during a blink
 * event.
 *
 * @appusage Should be implemented by an applicaiton layer configuration
 * plugin.
 *
 */
void halBulbPwmDriverBlinkStartCallback(void)
{
  blinking = true;
}

/** @brief When we finish blinking behavior, we need the bulb implementation
 * code to resumem normal non-blinking behavoir.
 *
 * @appusage Should be implemented by an applicaiton layer configuration
 * plugin.
 *
 */
void halBulbPwmDriverBlinkStopCallback(void)
{
  blinking = false;
  computeLoHighFromColorTemp(currentEndpoint());
}

void emberAfPluginColorControlServerComputePwmFromTempCallback(uint8_t endpoint)
{
  emberAfCorePrintln("testA");
  computeLoHighFromColorTemp(endpoint);
}
