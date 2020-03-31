/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include CONFIGURATION_HEADER
#include PLATFORM_HEADER
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_BULB_PWM_DRIVER

#include "led-dim-pwm-transform.h"

#ifndef EMBER_AF_PLUGIN_LEVEL_CONTROL_SERVER_MINIMUM_LEVEL
  #define EMBER_AF_PLUGIN_LEVEL_CONTROL_SERVER_MINIMUM_LEVEL 0
#endif
#ifndef EMBER_AF_PLUGIN_LEVEL_CONTROL_SERVER_MAXIMUM_LEVEL
  #define EMBER_AF_PLUGIN_LEVEL_CONTROL_SERVER_MAXIMUM_LEVEL 255
#endif

#define MIN_ON_TIME_MICROSECONDS 15
#define PWM_TICKS_WHEN_OFF       0
// These should be defined by the board configuration header for the reference
// design. They are PWM indeces that are used by the BULB_PWM driver to figure
// out which PWM should be used to control which color of bulb.
#ifndef BULB_PWM_WHITE
  #define BULB_PWM_WHITE 1
#endif

static uint16_t minPwmDrive, maxPwmDrive;
static bool blinking;

// Use precalculated values for the PWM based on a 1 kHz frequency to achieve
// the proper perceived LED output.
uint16_t pwmValues[] = { PWM_VALUES };
#define PWM_VALUES_LENGTH (COUNTOF(pwmValues))

static void updateDriveLevel(void);
static uint16_t updateDriveLevelLumens(void);
static uint16_t minDriveValue(void);
static uint16_t maxDriveValue(void);

static EmberZclEndpointId_t currentEndpoint(void)
{
  // Note:  LED bulbs only support one endpoint
  return emberZclEndpointIndexToId(0, &emberZclClusterOnOffServerSpec);
}

void emLedDimPwmInitHandler(void)
{
  blinking = false;

  minPwmDrive = minDriveValue();
  maxPwmDrive = maxDriveValue();

  updateDriveLevel();
}

static void pwmSetValue(uint16_t value)
{
  emberAfCorePrintln("setting a new PWM value: %d", value);
  halBulbPwmDriverSetPwmLevel(value, BULB_PWM_WHITE);
}

// update drive level based on linear power delivered to the light
static uint16_t updateDriveLevelLumens(void)
{
  uint32_t driveScratchpad;
  uint8_t currentLevel, mappedLevel;
  uint16_t newDrive;
  EmberZclStatus_t status;
  EmberZclEndpointId_t endpoint = currentEndpoint();

  status = emberZclReadAttribute(endpoint,
                                 &emberZclClusterLevelControlServerSpec,
                                 EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL,
                                 &currentLevel,
                                 sizeof(currentLevel));

  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Error reading level wehen updating driver lumens: 0x%x\n",
                       status);
    return 0;
  }

  // First handle the degenerate case.
  if (currentLevel == 0) {
    return 0;
  }

  // Next, map the drive level into the size of the table
  // We have a 255 entry table that goes from 0 to 6000.
  // use 32 bit math to avoid losing information.
  driveScratchpad = currentLevel - EMBER_AF_PLUGIN_LEVEL_CONTROL_SERVER_MINIMUM_LEVEL;
  driveScratchpad *= (PWM_VALUES_LENGTH - 1);
  driveScratchpad /= (EMBER_AF_PLUGIN_LEVEL_CONTROL_SERVER_MAXIMUM_LEVEL - EMBER_AF_PLUGIN_LEVEL_CONTROL_SERVER_MINIMUM_LEVEL);
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

static void updateDriveLevel(void)
{
  bool isOn;
  EmberZclStatus_t status;
  EmberZclEndpointId_t endpoint = currentEndpoint();

  if (blinking) {
    // we are in a mode where we are blinking an output pattern.  Don't blink
    // anything
    return;
  }

  status = emberZclReadAttribute(endpoint,
                                 &emberZclClusterOnOffServerSpec,
                                 EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                                 &isOn,
                                 sizeof(isOn));

  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    pwmSetValue(0);
    emberAfCorePrintln("Error reading onOff when updating driver level: 0x%x\n",
                       status);
    return;
  }

  if (isOn) {
    pwmSetValue(updateDriveLevelLumens());
  } else {
    pwmSetValue(0);
  }
}

void emLedDimPwmPostAttributeChangeHandler(EmberZclEndpointId_t endpointId,
                                           const EmberZclClusterSpec_t *clusterSpec,
                                           EmberZclAttributeId_t attributeId,
                                           const void *buffer,
                                           size_t bufferLength)
{
  if (emberZclAreClusterSpecsEqual(&emberZclClusterOnOffServerSpec, clusterSpec)
      || emberZclAreClusterSpecsEqual(&emberZclClusterLevelControlServerSpec, clusterSpec)) {
    emberAfCorePrintln("updatingDriveLevel");
    updateDriveLevel();
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
  updateDriveLevel();
}

void halBulbPwmDriverBlinkOnCallback(void)
{
  halBulbPwmDriverSetPwmLevel(halBulbPwmDriverTicksPerPeriod(),
                              BULB_PWM_WHITE);
}

void halBulbPwmDriverBlinkOffCallback(void)
{
  halBulbPwmDriverSetPwmLevel(PWM_TICKS_WHEN_OFF, BULB_PWM_WHITE);
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
