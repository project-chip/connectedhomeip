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
 * @brief Definitions for the LED Dim PWM plugin, which derives PWM values based
 *        on several clusters' attribute values.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/attribute-storage.h"

#ifdef EMBER_SCRIPTED_TEST
  #include "led-dim-pwm-test.h"
#endif

#ifdef EMBER_AF_PLUGIN_SCENES
  #include "app/framework/plugin/scenes/scenes.h"
#endif //EMBER_AF_PLUGIN_SCENES

#ifdef EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER
  #include "app/framework/plugin/zll-level-control-server/zll-level-control-server.h"
#endif //EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER

#ifdef EMBER_AF_PLUGIN_CONFIGURATION_SERVER
  #include "app/framework/plugin-soc/configuration-server/configuration-server.h"
#endif

#include EMBER_AF_API_BULB_PWM_DRIVER

#include "led-dim-pwm-transform.h"

#ifndef EMBER_AF_PLUGIN_LEVEL_CONTROL_MINIMUM_LEVEL
  #define EMBER_AF_PLUGIN_LEVEL_CONTROL_MINIMUM_LEVEL 0
#endif
#ifndef EMBER_AF_PLUGIN_LEVEL_CONTROL_MAXIMUM_LEVEL
  #define EMBER_AF_PLUGIN_LEVEL_CONTROL_MAXIMUM_LEVEL 255
#endif

#define MIN_ON_TIME_MICROSECONDS 15
#define OFF_TICKS         0
#ifndef BULB_PWM_WHITE
  #define BULB_PWM_WHITE 1
  #define BULB_PWM_RED   2
  #define BULB_PWM_GREEN 3
  #define BULB_PWM_BLUE  4
#endif

// support for internal storage calibration table
#define TABLE_SIGNATURE_0 0x5a
#define TABLE_SIGNATURE_1 0xa5
extern uint8_t internalStorage[];

static uint8_t minLevel;
static uint8_t maxLevel;
static uint16_t minPwmDrive, maxPwmDrive;
static bool onoffStartupDone;
static bool levelStartupDone;
static bool blinking;

// Use precalculated values for the PWM based on a 1 kHz frequency to achieve
// the proper perceived LED output.
uint16_t pwmValues[] = { PWM_VALUES };

// handle transitions between on and off state.
enum {
  ON_OFF_STATE_OFF           = 0,
  ON_OFF_STATE_ON            = 1,
};

#define SUCCESS_CODE 0

static uint8_t onOffState;

static void doInit(void);
static void updateDriveLevel(int8u endpoint);
static uint16_t minDriveValue(void);
static uint16_t maxDriveValue(void);
void halBulbPwmDriverInitialize(void);

static uint8_t currentEndpoint(void)
{
  // Note:  LED bulbs only support one endpoint
  assert(emberAfEndpointCount() == 1);

  return(emberAfEndpointFromIndex(0));
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
// by their respective StartUp settings. The "actionable" function updateDriveLevel() blocks
// until maxLevel variable is set to a nonzero value here.
void doInit(void)
{
  // Only proceed when both onoff and level startup settings have completed.
  if (onoffStartupDone && levelStartupDone) {
    onoffStartupDone = false;
    levelStartupDone = false;

    blinking = false;

    minPwmDrive = minDriveValue();
    maxPwmDrive = maxDriveValue();

    initOnOffState();

    // Set the min and max levels
#ifdef EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER
    minLevel = EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER_MINIMUM_LEVEL;
    maxLevel = EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER_MAXIMUM_LEVEL;
#else
    minLevel = EMBER_AF_PLUGIN_LEVEL_CONTROL_MINIMUM_LEVEL;
    maxLevel = EMBER_AF_PLUGIN_LEVEL_CONTROL_MAXIMUM_LEVEL;
#endif

    updateDriveLevel(currentEndpoint());

#ifdef INTERNAL_STORAGE_SIZE_KB
    emberAfCorePrintln("Internal Storage Address %4x %x %x",
                       (uint32_t) internalStorage,
                       internalStorage[0], internalStorage[1]);
#else
    emberAfCorePrintln("Internal Storage Not Active");
#endif
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

void emberAfPluginLedDimPwmInitCallback(void)
{
  // Await initialization based on StartUpOnOff and StartUpCurrentLevel.
}

static void pwmSetValue(uint16_t value)
{
  halBulbPwmDriverSetPwmLevel(value, BULB_PWM_WHITE);
}

// update drive level based on linear power delivered to the light
static uint16_t updateDriveLevelLumens(uint8_t endpoint)
{
  uint32_t driveScratchpad;
  uint8_t currentLevel, mappedLevel;
  uint8_t onOff;
  uint16_t newDrive;

  readFilteredOnOffAndLevel(&onOff, &currentLevel);

  // First handle the degenerate case.
  if (currentLevel == 0 || onOff == 0) {
    return OFF_TICKS;
  }

  // Next, map the drive level into the size of the table
  // We have a 255 entry table that goes from 0 to 6000.
  // use 32 bit math to avoid losing information.
  driveScratchpad = currentLevel - minLevel;
  driveScratchpad *= PWM_VALUES_LENGTH;
  driveScratchpad /= (maxLevel - minLevel);
  mappedLevel = (uint8_t) driveScratchpad;

  driveScratchpad = (uint32_t) pwmValues[mappedLevel];

  // newDrive now is mapped to 0..6000.  We need to remap it
  // to WHITE_MIMIMUM_ON_VALUE..WHITE_MAXIMUM_ON_VALUE
  // use 32 bit math to avoid losing information.
  driveScratchpad = driveScratchpad * (maxPwmDrive - minPwmDrive);
  driveScratchpad = driveScratchpad / 6000;
  driveScratchpad += minPwmDrive;
  newDrive = (uint16_t) driveScratchpad;

  return newDrive;
}

static uint16_t updateDriveLevelCalibrationTable(uint8_t endpoint)
{
  // Note:  we are keying in the definition of INTERNAL_STORAGE_SIZE_KB which
  // needs to be defined in hte .isc file.  Note:  if you have a local storage
  // bootloader, you will need to define this to be 1/2 of the total flash
  // size of the part.  If you do not use the local storage bootloader, you
  // need to make sure INTERNAL_STORAGE_SIZE_KB is at least 8, as there is
  // an artificial requirement on this size in internal-storage.h
#ifdef INTERNAL_STORAGE_SIZE_KB
  uint8_t currentLevel, onOff;
  uint16_t index;
  uint16_t pwmValue;

  if (internalStorage[0] != TABLE_SIGNATURE_0
      || internalStorage[1] != TABLE_SIGNATURE_1) {
    emberAfCorePrintln("%x %x %x %x",
                       internalStorage[0], internalStorage[1],
                       TABLE_SIGNATURE_0, TABLE_SIGNATURE_1);
    return updateDriveLevelLumens(endpoint);
  } else {
    readFilteredOnOffAndLevel(&onOff, &currentLevel);

    if (onOff == 0) {
      return OFF_TICKS;
    }

    index = currentLevel;
    index += 1;
    index *= 2;

    pwmValue = HIGH_LOW_TO_INT(internalStorage[index], internalStorage[index + 1]);

    return pwmValue;
  }
#else
  return updateDriveLevelLumens(endpoint);
#endif
}

static void updateDriveLevel(uint8_t endpoint)
{
  if (maxLevel == 0) {
    // Await initialization based on StartUpOnOff and StartUpCurrentLevel.
    return;
  }

  if (blinking) {
    // we are in a mode where we are blinking an output pattern.  Don't blink
    // anything
    return;
  }

  // updateDriveLevel is called before maxLevel has been initialzied.  So if
  // maxLevel is zero, we need to set the PWM to 0 for now and exit.  This
  // will be called again after maxLevel has been initialzied.
  if (maxLevel == 0) {
    pwmSetValue(OFF_TICKS);
    return;
  }

  pwmSetValue(updateDriveLevelCalibrationTable(endpoint));
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
  updateDriveLevel(endpoint);
}

void emberAfOnOffClusterServerAttributeChangedCallback(uint8_t endpoint,
                                                       EmberAfAttributeId attributeId)
{
  updateDriveLevel(endpoint);
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
  updateDriveLevel(currentEndpoint());
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
  updateDriveLevel(currentEndpoint());
}
