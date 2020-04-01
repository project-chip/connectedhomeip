/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include CONFIGURATION_HEADER
#include PLATFORM_HEADER
#include CHIP_AF_API_STACK
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif
#include CHIP_AF_API_BULB_PWM_DRIVER
#include CHIP_AF_API_ZCL_CORE

#define MIN_ON_TIME_MICROSECONDS 0

// These should be defined by the board configuration header for the reference
// design. They are PWM indeces that are used by the BULB_PWM driver to figure
// out which PWM should be used to control which color of bulb.
#ifndef BULB_PWM_WHITE
  #define BULB_PWM_WHITE 1
#endif
#ifndef BULB_PWM_LOWTEMP
  #define BULB_PWM_LOWTEMP 2
#endif

#define PWM_TICKS_WHEN_OFF 0
#define MAX_LEVEL 254

// ---------- Hardware values required for computing drive levels ----------
static uint16_t minColor, maxColor;
static uint16_t minPwmDrive, maxPwmDrive;
static bool blinking;

#define MIN_COLOR_DEFAULT 155
#define MAX_COLOR_DEFAULT 360

static uint16_t minDriveValue(void);
static uint16_t maxDriveValue(void);
static void computeRgbFromColorTemp(void);

static ChipZclEndpointId_t currentEndpoint(void)
{
  // Note:  LED bulbs only support one endpoint
  return chipZclEndpointIndexToId(0, &chipZclClusterOnOffServerSpec);
}

static void driveTempHiLo(uint16_t white, uint16_t lowtemp)
{
  halBulbPwmDriverSetPwmLevel(white, BULB_PWM_WHITE);
  halBulbPwmDriverSetPwmLevel(lowtemp, BULB_PWM_LOWTEMP);
}

void emLedTempPwmInitHandler(void)
{
  ChipZclEndpointId_t endpointId = currentEndpoint();
  ChipZclStatus_t status;

  blinking = false;

  minPwmDrive = minDriveValue();
  maxPwmDrive = maxDriveValue();

  status = chipZclReadAttribute(endpointId,
                                 &chipZclClusterColorControlServerSpec,
                                 CHIP_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MIN,
                                 &minColor,
                                 sizeof(minColor));

  if (status != CHIP_ZCL_STATUS_SUCCESS) {
    chipAfCorePrintln("Color Temp:  no color temp physical min attribute.");
    minColor = MIN_COLOR_DEFAULT;
  }

  status = chipZclReadAttribute(endpointId,
                                 &chipZclClusterColorControlServerSpec,
                                 CHIP_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MAX,
                                 &maxColor,
                                 sizeof(maxColor));

  if (status != CHIP_ZCL_STATUS_SUCCESS) {
    chipAfCorePrintln("Color Temp:  no color temp physical max attribute.");
    maxColor = MAX_COLOR_DEFAULT;
  }

  computeRgbFromColorTemp();
}

static void computeRgbFromColorTemp(void)
{
  uint16_t currentTemp;
  bool onOff;
  uint8_t currentLevel;
  ChipZclEndpointId_t endpointId = currentEndpoint();
  ChipZclStatus_t status;

  uint32_t high32, low32;
  uint16_t highDrive, lowDrive;

  if (blinking) {
    // we are in a mode where we are blinking an output pattern.  Don't blink
    // anything
    return;
  }

  // during framework init, this funciton sometimes is called before we set up
  // the values for max/min color temperautre.
  if (maxColor == 0 || minColor == 0) {
    return;
  }

  status = chipZclReadAttribute(endpointId,
                                 &chipZclClusterColorControlServerSpec,
                                 CHIP_ZCL_CLUSTER_COLOR_CONTROL_SERVER_ATTRIBUTE_COLOR_CONTROL_COLOR_TEMPERATURE,
                                 &currentTemp,
                                 sizeof(currentTemp));
  if (status != CHIP_ZCL_STATUS_SUCCESS) {
    chipAfCorePrintln("error reading color temperature attribute from ep %d\n",
                       endpointId);
    return;
  }

  status = chipZclReadAttribute(endpointId,
                                 &chipZclClusterLevelControlServerSpec,
                                 CHIP_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL,
                                 &currentLevel,
                                 sizeof(currentLevel));
  if (status != CHIP_ZCL_STATUS_SUCCESS) {
    chipAfCorePrintln("error reading current level attribute from ep %d\n",
                       endpointId);
    return;
  }

  status = chipZclReadAttribute(endpointId,
                                 &chipZclClusterOnOffServerSpec,
                                 CHIP_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                                 &onOff,
                                 sizeof(onOff));
  if (status != CHIP_ZCL_STATUS_SUCCESS) {
    chipAfCorePrintln("error reading onOff attribute from ep %d\n",
                       endpointId);
    return;
  }

  if ( !onOff || currentLevel == 0) {
    driveTempHiLo(PWM_TICKS_WHEN_OFF, PWM_TICKS_WHEN_OFF);

    return;
  }

  //bounds checking of the attribute temp.
  if (currentTemp > maxColor) {
    currentTemp = maxColor;
  } else if (currentTemp < minColor) {
    currentTemp = minColor;
  }

  // Compute the low and high drive based on linear interpolation.
  // y=mx+b, where y = low temperature LED drive,
  // m = (PWM Range) / (color range)
  // x = color temp - minimum color temperature
  // b = minimum PWM drive.
  low32 = maxPwmDrive - minPwmDrive;
  low32 *= (currentTemp - minColor);
  low32 /= (maxColor - minColor);
  low32 += minPwmDrive;

  // y=mx+b, where y = high temperature LED drive
  // m = (PWM Range)/(color range)
  // x = maxColor - currentTemp
  // b = minPwmDrive.
  high32 = maxPwmDrive - minPwmDrive;
  high32 *= (maxColor - currentTemp);
  high32 /= (maxColor - minColor);
  high32 += minPwmDrive;

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

void emLedTempPwmPostAttributeChangeHandler(ChipZclEndpointId_t endpointId,
                                            const ChipZclClusterSpec_t *clusterSpec,
                                            ChipZclAttributeId_t attributeId,
                                            const void *buffer,
                                            size_t bufferLength)
{
  if (chipZclAreClusterSpecsEqual(&chipZclClusterOnOffServerSpec, clusterSpec)
      || chipZclAreClusterSpecsEqual(&chipZclClusterLevelControlServerSpec, clusterSpec)
      || chipZclAreClusterSpecsEqual(&chipZclClusterColorControlServerSpec, clusterSpec)) {
    computeRgbFromColorTemp();
  }
}

// Part of the bulb driver is the minimum/maximum drive times as well as
// the frequency.
static uint16_t minDriveValue(void)
{
  uint16_t minDriveValue;

  minDriveValue = MIN_ON_TIME_MICROSECONDS;

  minDriveValue *= halBulbPwmDriverTicksPerMicrosecond();

  return minDriveValue;
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
  computeRgbFromColorTemp();
}

void halBulbPwmDriverBlinkOnCallback(void)
{
  uint16_t onAmount = halBulbPwmDriverTicksPerPeriod();
  driveTempHiLo(onAmount, onAmount);
}

void halBulbPwmDriverBlinkOffCallback(void)
{
  driveTempHiLo(PWM_TICKS_WHEN_OFF, PWM_TICKS_WHEN_OFF);
}

void halBulbPwmDriverBlinkStartCallback(void)
{
  blinking = true;
}

void halBulbPwmDriverBlinkStopCallback(void)
{
  blinking = false;
  computeRgbFromColorTemp();
}

void chipAfPluginColorControlServerComputePwmFromTempCallback(ChipZclEndpointId_t endpointId)
{
  computeRgbFromColorTemp();
}
