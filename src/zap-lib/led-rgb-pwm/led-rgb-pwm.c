/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#include CONFIGURATION_HEADER
#include PLATFORM_HEADER
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_BULB_PWM_DRIVER

#include "temp-to-rgb.h"

#define MIN_ON_TIME_MICROSECONDS 15
// These should be defined by the board configuration header for the reference
// design. They are PWM indeces that are used by the BULB_PWM driver to figure
// out which PWM should be used to control which color of bulb.
#ifndef BULB_PWM_WHITE
  #define BULB_PWM_WHITE 1
  #define BULB_PWM_RED   2
  #define BULB_PWM_GREEN 3
  #define BULB_PWM_BLUE  4
#endif

#define CIE1931_SCALE_FACTOR  65536L

#define PWM_TICKS_WHEN_OFF    0

#define EMBER_ZCL_COLOR_MODE_CURRENT_HUE_AND_CURRENT_SATURATION 0x00
#define EMBER_ZCL_COLOR_MODE_CURRENT_X_AND_CURRENT_Y            0x01
#define EMBER_ZCL_COLOR_MODE_COLOR_TEMPERATURE                  0x02

static uint16_t maxPwmDrive;

static bool blinking;

// Use precomputed values for RED, GREEN and BLUE based on algorithm by
// Robertson.
uint8_t tempRedValue[]   = { RED_VALUES };
uint8_t tempGreenValue[] = { GREEN_VALUES };
uint8_t tempBlueValue[]  = { BLUE_VALUES };

static void updateDriveLevel(void);
static uint16_t maxDriveValue(void);

static EmberZclEndpointId_t currentEndpoint(void)
{
  // Note:  LED bulbs only support one endpoint
  return emberZclEndpointIndexToId(0, &emberZclClusterOnOffServerSpec);
}

void emLedRgbPwmInitHandler(void)
{
  blinking = false;

  maxPwmDrive = maxDriveValue();

  updateDriveLevel();
}

static void driveWrgb(uint16_t white, uint16_t red, uint16_t green, uint16_t blue)
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
void emberAfPluginColorControlServerComputePwmFromXyCallback(EmberZclEndpointId_t endpointId)
{
  uint16_t currentX, currentY;
  bool onOff;
  uint8_t currentLevel;

  uint32_t scratch;
  uint32_t x32, y32, z32;
  int32_t r32, g32, b32;
  uint16_t rDrive, gDrive, bDrive;

  EmberZclStatus_t status;
  EmberZclEndpointId_t expectedEndpoint = currentEndpoint();

  if (expectedEndpoint != endpointId) {
    return;
  }

  status = emberZclReadAttribute(endpointId,
                                 &emberZclClusterColorControlServerSpec,
                                 EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_X,
                                 &currentX,
                                 sizeof(currentX));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Error reading current x attribute from ep %d\n",
                       endpointId);
    return;
  }

  status = emberZclReadAttribute(endpointId,
                                 &emberZclClusterColorControlServerSpec,
                                 EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_Y,
                                 &currentY,
                                 sizeof(currentY));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Error reading current y attribute from ep %d\n",
                       endpointId);
    return;
  }

  status = emberZclReadAttribute(endpointId,
                                 &emberZclClusterLevelControlServerSpec,
                                 EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL,
                                 &currentLevel,
                                 sizeof(currentLevel));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Error reading current level attribute from ep %d\n",
                       endpointId);
    return;
  }

  status = emberZclReadAttribute(endpointId,
                                 &emberZclClusterOnOffServerSpec,
                                 EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                                 &onOff,
                                 sizeof(onOff));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Error reading onOff attribute from ep %d\n",
                       endpointId);
    return;
  }

  if (!onOff || currentLevel == 0) {
    driveWrgb(0, 0, 0, 0);
    return;
  }

  // compute x, y, z
  x32 = currentX;
  y32 = currentY;

  scratch = x32 + y32;

  if (scratch > CIE1931_SCALE_FACTOR) {
    emberAfCorePrintln("X and Y are too big");
    return;
  }

  z32 = CIE1931_SCALE_FACTOR - (x32 + y32);

  // now we can compute the RGB values in 65,536,000
  // these are well-known constants but are documented at:
  // http://docs-hoffmann.de/ciexyz29082000.pdf which came from
  // "Digital Color Management, Giorgianni+Madden
  r32 = (x32 * 2365) - (y32 * 897) - (z32 * 468);
  g32 = ((y32 * 1426) + (z32 * 89)) - (x32 * 515);
  b32 = (x32 * 5) + (z32 * 1009) - (y32 * 14);

  r32 = r32 / 65536;
  r32 = r32 * maxPwmDrive;
  r32 = r32 / 1000;
  rDrive = (uint16_t) r32;

  g32 = g32 / 65536;
  g32 = g32 * maxPwmDrive;
  g32 = g32 / 1000;
  gDrive = (uint16_t) g32;

  b32 = b32 / 65536;
  b32 = b32 * maxPwmDrive;
  b32 = b32 / 1000;
  bDrive = (uint16_t) b32;

  // limits checking.  Also, handle level.
  if (rDrive > 32768) {
    rDrive = 0;
  } else {
    r32 *= currentLevel;
    r32 /= 256;
    rDrive = (uint16_t) r32;
  }
  if (gDrive > 32768) {
    gDrive = 0;
  } else {
    g32 *= currentLevel;
    g32 /= 256;
    gDrive = (uint16_t) g32;
  }
  if (bDrive > 32768) {
    bDrive = 0;
  } else {
    b32 *= currentLevel;
    b32 /= 256;
    bDrive = (uint16_t) b32;
  }

  if (rDrive > maxPwmDrive) {
    rDrive = maxPwmDrive;
  }
  if (gDrive > maxPwmDrive) {
    gDrive = maxPwmDrive;
  }
  if (bDrive > maxPwmDrive) {
    bDrive = maxPwmDrive;
  }

  driveWrgb(0, rDrive, gDrive, bDrive);
}

void emberAfPluginColorControlServerComputePwmFromTempCallback(EmberZclEndpointId_t endpointId)
{
  uint16_t currentTemp;
  bool onOff;
  uint8_t currentLevel;

  uint32_t r32, g32, b32, W32;
  uint16_t rDrive, gDrive, bDrive, wDrive;

  EmberZclStatus_t status;
  EmberZclEndpointId_t expectedEndpoint = currentEndpoint();

  if (expectedEndpoint != endpointId) {
    return;
  }

  status = emberZclReadAttribute(endpointId,
                                 &emberZclClusterColorControlServerSpec,
                                 EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_TEMPERATURE,
                                 &currentTemp,
                                 sizeof(currentTemp));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Error reading color temperature attribute from ep %d\n",
                       endpointId);
    return;
  }

  status = emberZclReadAttribute(endpointId,
                                 &emberZclClusterLevelControlServerSpec,
                                 EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL,
                                 &currentLevel,
                                 sizeof(currentLevel));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Error reading current level attribute from ep %d\n",
                       endpointId);
    return;
  }

  status = emberZclReadAttribute(endpointId,
                                 &emberZclClusterOnOffServerSpec,
                                 EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                                 &onOff,
                                 sizeof(onOff));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Error reading onOff attribute from ep %d\n",
                       endpointId);
    return;
  }

  if (!onOff || currentLevel == 0) {
    driveWrgb(0, 0, 0, 0);

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
  r32 = tempRedValue[currentTemp];
  g32 = tempGreenValue[currentTemp];
  b32 = tempBlueValue[currentTemp];

  // handle conversion to ticks and level control.
  W32 = W32 * maxPwmDrive;
  r32 = r32 * maxPwmDrive;
  g32 = g32 * maxPwmDrive;
  b32 = b32 * maxPwmDrive;

  // now we have white, red, green, and blue in 0 to 255 * TICS_PER_PERIOD.
  // level is 0.255.  To convert to drive, we multiply by level and divide by
  // 255 * 255.
  r32 = r32 * currentLevel;
  r32 = r32 / (255 * 255);
  g32 = g32 * currentLevel;
  g32 = g32 / (255 * 255);
  b32 = b32 * currentLevel;
  b32 = b32 / (255 * 255);
  W32 = W32 * currentLevel;
  W32 = W32 / (255 * 255);

  // convert to uint16_t and drive the PWMs.
  rDrive = (uint16_t) r32;
  gDrive = (uint16_t) g32;
  bDrive = (uint16_t) b32;
  wDrive = (uint16_t) W32;

  driveWrgb(wDrive, rDrive, gDrive, bDrive);
}

void emberAfPluginColorControlServerComputePwmFromHsvCallback(EmberZclEndpointId_t endpointId)
{
  uint8_t hue, saturation;
  bool onOff;
  uint8_t currentLevel;

  uint32_t min32, hue32, delta32, sat32, level32;

  uint32_t r32, g32, b32;
  uint16_t rDrive, gDrive, bDrive;

  EmberZclStatus_t status;
  EmberZclEndpointId_t expectedEndpoint = currentEndpoint();

  if (expectedEndpoint != endpointId) {
    return;
  }

  status = emberZclReadAttribute(endpointId,
                                 &emberZclClusterColorControlServerSpec,
                                 EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_HUE,
                                 &hue,
                                 sizeof(hue));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Error reading current hue attribute from ep %d\n",
                       endpointId);
    return;
  }

  status = emberZclReadAttribute(endpointId,
                                 &emberZclClusterColorControlServerSpec,
                                 EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_CURRENT_SATURATION,
                                 &saturation,
                                 sizeof(saturation));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Error reading current color attribute from ep %d\n",
                       endpointId);
    return;
  }

  status = emberZclReadAttribute(endpointId,
                                 &emberZclClusterLevelControlServerSpec,
                                 EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL,
                                 &currentLevel,
                                 sizeof(currentLevel));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Error reading level attribute from ep %d\n",
                       endpointId);
    return;
  }

  status = emberZclReadAttribute(endpointId,
                                 &emberZclClusterOnOffServerSpec,
                                 EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                                 &onOff,
                                 sizeof(onOff));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Error reading onOff attribute from ep %d\n",
                       endpointId);
    return;
  }

  if (!onOff || currentLevel == 0) {
    driveWrgb(0, 0, 0, 0);

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
    r32 = level32; // 0..254
    // convert 0..42 to 0..delta32
    g32 = hue32 * delta32;
    g32 = g32 / 42;
    g32 = g32 + min32;
    b32 = min32;
  } else if (hue < 85) {
    hue32 -= 42;
    // convert 0..42 to delta32..0
    r32 = (42 - hue32) * delta32;
    r32 = r32 / 42;
    r32 = r32 + min32;
    g32 = level32;
    b32 = min32;
  } else if (hue < 128) {
    hue32 -= 84;
    r32 = min32;
    g32 = level32;
    // convert 0..43 to 0..delta32
    b32 = hue32 * delta32;
    b32 = b32 / 43;
    b32 = b32 + min32;
  } else if (hue < 170) {
    hue32 -= 127;
    r32 = min32;
    // convert 0..42 to delta32..0
    g32 = (42 - hue32) * delta32;
    g32 = g32 / 42;
    g32 = g32 + min32;
    b32 = level32;
  } else if (hue < 212) {
    hue32 -= 169;
    // convert 0..42 to 0..delta32
    r32 = hue32 * delta32;
    r32 = r32 / 42;
    r32 = r32 + min32;
    g32 = min32;
    b32 = level32;
  } else { //hue is 212..254
    hue32 -= 211;
    r32 = level32;
    g32 = min32;
    // convert 0..43 to delta32..0
    b32 = (43 - hue32) * delta32;
    b32 = b32 / 42;
    b32 = b32 + min32;
  }

  r32 = r32 * maxPwmDrive;
  g32 = g32 * maxPwmDrive;
  b32 = b32 * maxPwmDrive;
  r32 = r32 / 254;
  g32 = g32 / 254;
  b32 = b32 / 254;

  rDrive = (uint16_t) r32;
  gDrive = (uint16_t) g32;
  bDrive = (uint16_t) b32;

  driveWrgb(0, rDrive, gDrive, bDrive);
}

void emLedRgbPwmPostAttributeChangeHandler(EmberZclEndpointId_t endpointId,
                                           const EmberZclClusterSpec_t *clusterSpec,
                                           EmberZclAttributeId_t attributeId,
                                           const void *buffer,
                                           size_t bufferLength)
{
  if (emberZclAreClusterSpecsEqual(&emberZclClusterOnOffServerSpec, clusterSpec)
      || emberZclAreClusterSpecsEqual(&emberZclClusterLevelControlServerSpec, clusterSpec)
      || emberZclAreClusterSpecsEqual(&emberZclClusterColorControlServerSpec, clusterSpec)) {
    updateDriveLevel();
  }
}

static void updateDriveLevel(void)
{
  EmberZclStatus_t status;
  uint8_t colorMode;
  EmberZclEndpointId_t endpointId = currentEndpoint();

  if (blinking) {
    // we are in a mode where we are blinking an output pattern.  Don't blink
    // anything
    emberAfCorePrintln("blinking");
    return;
  }

  status = emberZclReadAttribute(endpointId,
                                 &emberZclClusterColorControlServerSpec,
                                 EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_MODE,
                                 &colorMode,
                                 sizeof(colorMode));

  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("ERROR! Application must support color mode attribute for led-rgb-pwm plugin to function!\n");
    return;
  }

  switch (colorMode) {
    case EMBER_ZCL_COLOR_MODE_CURRENT_HUE_AND_CURRENT_SATURATION:
      emberAfPluginColorControlServerComputePwmFromHsvCallback(endpointId);
      break;
    case EMBER_ZCL_COLOR_MODE_CURRENT_X_AND_CURRENT_Y:
      emberAfPluginColorControlServerComputePwmFromXyCallback(endpointId);
      break;
    case EMBER_ZCL_COLOR_MODE_COLOR_TEMPERATURE:
      emberAfPluginColorControlServerComputePwmFromTempCallback(endpointId);
      break;
    default:
      emberAfCorePrintln("read unexpected color mode: 0x%x\n", colorMode);
      break;
  }
}

static uint16_t maxDriveValue(void)
{
  uint16_t maxDriveValue;

  maxDriveValue = halBulbPwmDriverTicksPerPeriod();

  return maxDriveValue;
}

// -----------------------------------------------------------------------------
// HAL callbacks.

void halBulbPwmDriverInitCompleteCallback(void)
{
  updateDriveLevel();
}

void halBulbPwmDriverBlinkOnCallback(void)
{
  uint16_t onValue = halBulbPwmDriverTicksPerPeriod();
  driveWrgb(onValue, onValue, onValue, onValue);
}

void halBulbPwmDriverBlinkOffCallback(void)
{
  driveWrgb(PWM_TICKS_WHEN_OFF,
            PWM_TICKS_WHEN_OFF,
            PWM_TICKS_WHEN_OFF,
            PWM_TICKS_WHEN_OFF);
}

void halBulbPwmDriverBlinkStartCallback(void)
{
  blinking = true;
}

void halBulbPwmDriverBlinkStopCallback(void)
{
  blinking = false;
  updateDriveLevel();
}
