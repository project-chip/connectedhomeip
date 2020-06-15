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
 * @brief Definitions for the LED RGB PWM plugin.
 *******************************************************************************
   ******************************************************************************/

#ifdef EMBER_SCRIPTED_TEST
  #include "led-rgb-pwm-test.h"
#endif

#include "app/framework/include/af.h"
#include "app/framework/util/attribute-storage.h"
#include "temp-to-rgb.h"
#include EMBER_AF_API_BULB_PWM_DRIVER

#ifdef EMBER_AF_PLUGIN_SCENES
  #include "app/framework/plugin/scenes/scenes.h"
#endif //EMBER_AF_PLUGIN_SCENES

#ifdef EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER
  #include "app/framework/plugin/zll-level-control-server/zll-level-control-server.h"
#endif //EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER

#ifdef EMBER_AF_PLUGIN_CONFIGURATION_SERVER
  #include "app/framework/plugin-soc/configuration-server/configuration-server.h"
#endif

#ifndef EMBER_AF_PLUGIN_LEVEL_CONTROL_MINIMUM_LEVEL
  #define EMBER_AF_PLUGIN_LEVEL_CONTROL_MINIMUM_LEVEL 0
#endif
#ifndef EMBER_AF_PLUGIN_LEVEL_CONTROL_MAXIMUM_LEVEL
  #define EMBER_AF_PLUGIN_LEVEL_CONTROL_MAXIMUM_LEVEL 255
#endif

#define MIN_ON_TIME_MICROSECONDS 15
#ifndef BULB_PWM_WHITE
  #define BULB_PWM_WHITE 1
  #define BULB_PWM_RED   2
  #define BULB_PWM_GREEN 3
  #define BULB_PWM_BLUE  4
#endif

#define OFF_TICKS         0

static uint8_t minLevel;
static uint8_t maxLevel;

static uint16_t minPwmDrive, maxPwmDrive;

static bool onoffStartupDone;
static bool levelStartupDone;
static bool blinking;

// Use precomputed values for RED, GREEN and BLUE based on algorithm by
// Robertson.
uint8_t tempRedValue[]   = { RED_VALUES };
uint8_t tempGreenValue[] = { GREEN_VALUES };
uint8_t tempBlueValue[]  = { BLUE_VALUES };

#define PWM_POLARITY      EMBER_AF_PLUGIN_PWM_CONTROL_PWM_POLARITY
#define ON_OFF_OUTPUT     EMBER_AF_PLUGIN_PWM_CONTROL_ON_OFF_OUTPUT

// handle transitions between on and off state.
enum {
  ON_OFF_STATE_OFF           = 0,
  ON_OFF_STATE_ON            = 1,
};

#define SUCCESS_CODE 0

static uint8_t onOffState;

static void doInit(void);
static void updateDriveLevel(uint8_t endpoint);
static uint16_t minDriveValue(void);
static uint16_t maxDriveValue(void);

static int8u currentEndpoint(void)
{
  // Note:  LED bulbs only support one endpoint
  assert(emberAfEndpointCount() == 1);

  return(emberAfEndpointFromIndex(0));
}

static uint8_t readOnOffAttribute(void)
{
  uint8_t onOff;

  assert(EMBER_ZCL_STATUS_SUCCESS
         == emberAfReadServerAttribute(currentEndpoint(),
                                       ZCL_ON_OFF_CLUSTER_ID,
                                       ZCL_ON_OFF_ATTRIBUTE_ID,
                                       (uint8_t *)&onOff,
                                       sizeof(onOff)));
  return onOff;
}

static uint8_t readLevelAttribute(void)
{
  uint8_t level;

  assert(EMBER_ZCL_STATUS_SUCCESS
         == emberAfReadServerAttribute(currentEndpoint(),
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

void emberAfPluginLedRgbPwmInitCallback(void)
{
  // Await initialization based on StartUpOnOff and StartUpCurrentLevel.
}

static void driveWRGB(uint16_t white, uint16_t red, uint16_t green, uint16_t blue)
{
  halBulbPwmDriverSetPwmLevel(white, BULB_PWM_WHITE);
  halBulbPwmDriverSetPwmLevel(red, BULB_PWM_RED);
  halBulbPwmDriverSetPwmLevel(green, BULB_PWM_GREEN);
  halBulbPwmDriverSetPwmLevel(blue, BULB_PWM_BLUE);
}

// apply RGB transform to XY input
// Note:  X and Y are well-known quantities in the color space.  It comes
// from the CIE xyY color model.  Here, currentX and currentY are the ZCL
// attributes where x = currentX / 65536 and y = currentY / 65536.
// http://en.wikipedia.org/wiki/CIE_1931_color_space for more details
void emberAfPluginColorControlServerComputePwmFromXyCallback(uint8_t endpoint)
{
  uint16_t currentX, currentY;
  uint8_t onOff, currentLevel;

  uint32_t scratch;
  uint32_t X32, Y32, Z32;
  int32_t R32, G32, B32;
  uint16_t rDrive, gDrive, bDrive;

  // read the attributes from the attribute table.
  emberAfReadServerAttribute(endpoint,
                             ZCL_COLOR_CONTROL_CLUSTER_ID,
                             ZCL_COLOR_CONTROL_CURRENT_X_ATTRIBUTE_ID,
                             (uint8_t *)&currentX,
                             sizeof(currentX));
  emberAfReadServerAttribute(endpoint,
                             ZCL_COLOR_CONTROL_CLUSTER_ID,
                             ZCL_COLOR_CONTROL_CURRENT_Y_ATTRIBUTE_ID,
                             (uint8_t *)&currentY,
                             sizeof(currentY));

  readFilteredOnOffAndLevel(&onOff, &currentLevel);

  if (onOff == 0 || currentLevel == 0) {
    driveWRGB(0, 0, 0, 0);
    return;
  }

  // compute x, y, z
  X32 = currentX;
  Y32 = currentY;

  scratch = X32 + Y32;

  if (scratch > 65536l) {
    emberAfAppPrintln("X and Y are too big");
    return;
  }

  Z32 = 65536l - (X32 + Y32);

  // now we can compute the RGB values in 65,536,000
  // these are well-known constants but are documented at:
  // http://docs-hoffmann.de/ciexyz29082000.pdf which came from
  // "Digital Color Management, Giorgianni+Madden
  R32 = (X32 * 2365) - (Y32 * 897) - (Z32 * 468);
  G32 = ((Y32 * 1426) + (Z32 * 89)) - (X32 * 515);
  B32 = (X32 * 5) + (Z32 * 1009) - (Y32 * 14);

  // Note:  it is possible the above algorithm will create a negative drive
  // value.  We need to check for that and set it to zero.
  if (R32 < 0) {
    R32 = 0;
  }

  if (G32 < 0) {
    G32 = 0;
  }

  if (B32 < 0) {
    B32 = 0;
  }

  R32 = R32 / 65536;
  R32 = R32 * maxPwmDrive;
  R32 = R32 / 1000;

  G32 = G32 / 65536;
  G32 = G32 * maxPwmDrive;
  G32 = G32 / 1000;

  B32 = B32 / 65536;
  B32 = B32 * maxPwmDrive;
  B32 = B32 / 1000;

  // limits checking.  Also, handle level.
  R32 *= currentLevel;
  R32 /= 256;
  rDrive = (uint16_t) R32;
  if (rDrive > maxPwmDrive) {
    rDrive = maxPwmDrive;
  }

  G32 *= currentLevel;
  G32 /= 256;
  gDrive = (uint16_t) G32;
  if (gDrive > maxPwmDrive) {
    gDrive = maxPwmDrive;
  }

  B32 *= currentLevel;
  B32 /= 256;
  bDrive = (uint16_t) B32;
  if (bDrive > maxPwmDrive) {
    bDrive = maxPwmDrive;
  }

  driveWRGB(0, rDrive, gDrive, bDrive);
}

void emberAfPluginColorControlServerComputePwmFromTempCallback(uint8_t endpoint)
{
  uint16_t currentTemp;
  uint8_t onOff, currentLevel;

  uint32_t R32, G32, B32, W32;
  uint16_t rDrive, gDrive, bDrive, wDrive;

  emberAfReadServerAttribute(endpoint,
                             ZCL_COLOR_CONTROL_CLUSTER_ID,
                             ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID,
                             (uint8_t *)&currentTemp,
                             sizeof(currentTemp));

  readFilteredOnOffAndLevel(&onOff, &currentLevel);

  if (onOff == 0 || currentLevel == 0) {
    driveWRGB(0, 0, 0, 0);
    return;
  }

  // temperature is 1,000,000/currentTemperature.
  // I computed R,G,B from a simple algorithm from Robertson and created
  // tables.
  currentTemp = currentTemp / 10;
  if (currentTemp > 99) {
    currentTemp = 99;
  }

  // red, green, blue values are in 0..255.  Just add white at the max.
  W32 = 0;
  R32 = tempRedValue[currentTemp];
  G32 = tempGreenValue[currentTemp];
  B32 = tempBlueValue[currentTemp];

  // handle conversion to ticks and level control.
  W32 = W32 * maxPwmDrive;
  R32 = R32 * maxPwmDrive;
  G32 = G32 * maxPwmDrive;
  B32 = B32 * maxPwmDrive;

  // now we have white, red, green, and blue in 0 to 255 * TICS_PER_PERIOD.
  // level is 0.255.  To convert to drive, we multiply by level and divide by
  // 255 * 255.
  R32 = R32 * currentLevel;
  R32 = R32 / (255 * 255);
  G32 = G32 * currentLevel;
  G32 = G32 / (255 * 255);
  B32 = B32 * currentLevel;
  B32 = B32 / (255 * 255);
  W32 = W32 * currentLevel;
  W32 = W32 / (255 * 255);

  // convert to uint16_t and drive the PWMs.
  rDrive = (uint16_t) R32;
  gDrive = (uint16_t) G32;
  bDrive = (uint16_t) B32;
  wDrive = (uint16_t) W32;

  driveWRGB(wDrive, rDrive, gDrive, bDrive);
}

void emberAfPluginColorControlServerComputePwmFromHsvCallback(uint8_t endpoint)
{
  uint8_t hue, saturation;
  uint8_t onOff, currentLevel;

  uint32_t min32, hue32, delta32, sat32, level32;

  uint32_t R32, G32, B32;
  uint16_t rDrive, gDrive, bDrive;

  emberAfReadServerAttribute(endpoint,
                             ZCL_COLOR_CONTROL_CLUSTER_ID,
                             ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID,
                             (uint8_t *)&hue,
                             sizeof(hue));

  emberAfReadServerAttribute(endpoint,
                             ZCL_COLOR_CONTROL_CLUSTER_ID,
                             ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                             (uint8_t *)&saturation,
                             sizeof(saturation));

  readFilteredOnOffAndLevel(&onOff, &currentLevel);

  if (onOff == 0 || currentLevel == 0) {
    driveWRGB(0, 0, 0, 0);

    return;
  }

  // algorithm taken from wikipedia
  // http://en.wikipedia.org/wiki/CIE_1931_color_space for more details

  // note:  hue and saturation are 0..254.  level (i.e. value for HSV) is
  // 0..255.  But most of these computations assume 0..1 for saturation and
  // value, and 0..360 for hue.  This will be a little tricky to compute RGB
  // using fixed point math and not lose any bits of significance.

  // first switch to 32 bit mode.
  level32 = (uint32_t) currentLevel;
  sat32 = (uint32_t) saturation;

  min32 = level32 * (254 - sat32);
  min32 = min32 / 254;
  delta32 = level32 - min32;
  hue32 = ((uint32_t) hue);  // need to map it to 0..6.  really is 0..254.

  // formula is X = C * { 1 - ( H mod2 - 1) }.  Becuase the nubmers don't line
  // up, we need to do this with if's.
  // The ranges are 0, 42, 84, 127, 169, 211, 254.
  if (hue < 43) {
    R32 = level32; // 0..254
    // convert 0..42 to 0..delta32
    G32 = hue32 * delta32;
    G32 = G32 / 42;
    G32 = G32 + min32;
    B32 = min32;
  } else if (hue < 85) {
    hue32 -= 42;
    // convert 0..42 to delta32..0
    R32 = (42 - hue32) * delta32;
    R32 = R32 / 42;
    R32 = R32 + min32;
    G32 = level32;
    B32 = min32;
  } else if (hue < 128) {
    hue32 -= 84;
    R32 = min32;
    G32 = level32;
    // convert 0..43 to 0..delta32
    B32 = hue32 * delta32;
    B32 = B32 / 43;
    B32 = B32 + min32;
  } else if (hue < 170) {
    hue32 -= 127;
    R32 = min32;
    // convert 0..42 to delta32..0
    G32 = (42 - hue32) * delta32;
    G32 = G32 / 42;
    G32 = G32 + min32;
    B32 = level32;
  } else if (hue < 212) {
    hue32 -= 169;
    // convert 0..42 to 0..delta32
    R32 = hue32 * delta32;
    R32 = R32 / 42;
    R32 = R32 + min32;
    G32 = min32;
    B32 = level32;
  } else { //hue is 212..254
    hue32 -= 211;
    R32 = level32;
    G32 = min32;
    // convert 0..43 to delta32..0
    B32 = (43 - hue32) * delta32;
    B32 = B32 / 42;
    B32 = B32 + min32;
  }

  R32 = R32 * maxPwmDrive;
  G32 = G32 * maxPwmDrive;
  B32 = B32 * maxPwmDrive;
  R32 = R32 / 254;
  G32 = G32 / 254;
  B32 = B32 / 254;

  rDrive = (uint16_t) R32;
  gDrive = (uint16_t) G32;
  bDrive = (uint16_t) B32;

  driveWRGB(0, rDrive, gDrive, bDrive);
}

/** @brief Server Attribute Changed.
 *
 * Level Control cluster, Server Attribute Changed.
 *
 * @param endpoint Endpoint that is being initialized. Ver.: always
 * @param attributeId Attribute that changed. Ver.: always
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
  updateDriveLevel(endpoint);
}

static void updateDriveLevel(uint8_t endpoint)
{
  if (maxLevel == 0) {
    // Await initialization based on StartUpOnOff and StartUpCurrentLevel.
    return;
  }

  uint8_t colorMode;

  if (blinking) {
    // we are in a mode where we are blinking an output pattern.  Don't blink
    // anything
    return;
  }

  emberAfReadServerAttribute(endpoint,
                             ZCL_COLOR_CONTROL_CLUSTER_ID,
                             ZCL_COLOR_CONTROL_COLOR_MODE_ATTRIBUTE_ID,
                             (uint8_t *)&colorMode,
                             sizeof(colorMode));

  switch (colorMode) {
    case EMBER_ZCL_COLOR_MODE_CURRENT_HUE_AND_CURRENT_SATURATION:
      emberAfPluginColorControlServerComputePwmFromHsvCallback(endpoint);
      break;
    case EMBER_ZCL_COLOR_MODE_CURRENT_X_AND_CURRENT_Y:
      emberAfPluginColorControlServerComputePwmFromXyCallback(endpoint);
      break;
    case EMBER_ZCL_COLOR_MODE_COLOR_TEMPERATURE:
      emberAfPluginColorControlServerComputePwmFromTempCallback(endpoint);
      break;
  }
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

/** @brief Called after PWM driver initialization, updating drive level
 *
 * @appusage Should be implemented by an application layer configuration plugin.
 *
 */
void halBulbPwmDriverInitCompleteCallback(void)
{
  updateDriveLevel(currentEndpoint());
}

/** @brief Called during blinking behavior telling the bulb implementation to turn the bulb on.
 *
 * @appusage Should be implemented by an application layer configuration
 * plugin.
 *
 */
void halBulbPwmDriverBlinkOnCallback(void)
{
  halBulbPwmDriverSetPwmLevel(halBulbPwmDriverTicksPerPeriod(), BULB_PWM_WHITE);
  halBulbPwmDriverSetPwmLevel(halBulbPwmDriverTicksPerPeriod(), BULB_PWM_RED);
  halBulbPwmDriverSetPwmLevel(halBulbPwmDriverTicksPerPeriod(), BULB_PWM_GREEN);
  halBulbPwmDriverSetPwmLevel(halBulbPwmDriverTicksPerPeriod(), BULB_PWM_BLUE);
}

/** @brief Called during blinking behavior telling the bulb implementation to turn the bulb off.
 *
 * @appusage Should be implemented by an application layer configuration
 * plugin.
 *
 */
void halBulbPwmDriverBlinkOffCallback(void)
{
  halBulbPwmDriverSetPwmLevel(OFF_TICKS, BULB_PWM_WHITE);
  halBulbPwmDriverSetPwmLevel(OFF_TICKS, BULB_PWM_RED);
  halBulbPwmDriverSetPwmLevel(OFF_TICKS, BULB_PWM_GREEN);
  halBulbPwmDriverSetPwmLevel(OFF_TICKS, BULB_PWM_BLUE);
}

/** @brief Called when blinking behavior is started, instructing the bulb
 * implementation code to hold off on any changes required from the
 * application (i.e., do not change the dim level or color during a blink
 * event).
 *
 * @appusage Should be implemented by an application layer configuration
 * plugin.
 *
 */
void halBulbPwmDriverBlinkStartCallback(void)
{
  blinking = TRUE;
}

/** @brief Called when blinking behavior is finished, resuming normal
 * non-blinking behavior.
 *
 * @appusage Should be implemented by an application layer configuration
 * plugin.
 *
 */
void halBulbPwmDriverBlinkStopCallback(void)
{
  blinking = false;
  updateDriveLevel(currentEndpoint());
}
