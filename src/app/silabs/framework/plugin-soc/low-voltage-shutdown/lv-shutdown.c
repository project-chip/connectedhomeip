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
 * @brief Definitions for the LV Shutdown plugin, which monitors VDD levels.
 *******************************************************************************
   ******************************************************************************/

#include "af.h"
#include "lv-shutdown.h"

// Rate to use for halMeasureVdd; some ADC_SAMPLE_CLOCKS_xxx from adc.h
#define VDD_MEASURE_RATE ADC_SAMPLE_CLOCKS_64

// Event control logic for voltage-checking
EmberEventControl emberAfPluginLowVoltageShutdownCheckForShutdownEventControl;
#define myEvent emberAfPluginLowVoltageShutdownCheckForShutdownEventControl

// Forward declaration of local routines
static void lvShutdown(void);

// ****** CLI Command helpers (public APIs) ******
void emberAfPluginLowVoltageShutdownEnable(bool enable)
{
  if (enable) {
    emberEventControlSetActive(myEvent);
  } else {
    emberEventControlSetInactive(myEvent);
  }
}

bool emberAfPluginLowVoltageShutdownEnabled(void)
{
  return emberEventControlGetActive(myEvent);
}

void emberAfPluginLowVoltageShutdownForceShutdown(void)
{
  lvShutdown();
}

// ****** Other public functions for this plugin ******

// Can also call this function directly (rather than letting it be scheduled)
// if application needs to do a check (and maybe shutdown) right now.  Note that
// doing so will reset the schedule interval for the next check.
void emberAfPluginLowVoltageShutdownCheckForShutdownEventHandler(void)
{
  // See if we're below VDD threshold and shut down the chip until reset if so
  if (emberAfPluginLowVoltageShutdownGetVoltage()
      < EMBER_AF_PLUGIN_LOW_VOLTAGE_SHUTDOWN_VDD_LOW_THRESHOLD) {
    lvShutdown();
    // Don't deactiveate or reschedule event here; if we come out of shutdown, we want the check
    // to re-run ASAP in case the low voltage condition has not cleared.
  } else {
    emberEventControlSetDelayMinutes(myEvent, EMBER_AF_PLUGIN_LOW_VOLTAGE_SHUTDOWN_SHUTDOWN_CHECK_DURATION_MINUTES);
  }
}

// Init callback for this plugin; invoked during emberAfInit()
void emberAfPluginLowVoltageShutdownInitCallback(void)
{
  // Run the event handler directly upon init to do an initial check of VDD
  // (without waiting for the scheduler to check events) and schedule recurrence
  // for the check event.  If we're already below threshold on init, shut down
  // immediately.
  emberAfPluginLowVoltageShutdownCheckForShutdownEventHandler();
}

// Ask the HAL for the current VDD voltage (e.g. battery voltage)
uint16_t emberAfPluginLowVoltageShutdownGetVoltage(void)
{
  return halMeasureVdd(VDD_MEASURE_RATE);
}

// ****** Local (static) routines ******

// Shut down for good after the user says it's OK and runs any last-minute logic
static void lvShutdown(void)
{
  uint16_t shutdownVoltage = emberAfPluginLowVoltageShutdownGetVoltage();
  if (emberAfPluginLowVoltageShutdownOkToShutdownCallback(shutdownVoltage)) {
    emberAfPluginLowVoltageShutdownPreShutdownCallback(shutdownVoltage);
    halPowerDown();
    halInternalSleep(SLEEPMODE_NOTIMER);
  } else {
    // User won't let us shut down yet, so check back after a brief interval
    emberEventControlSetDelayMS(myEvent, EMBER_AF_PLUGIN_LOW_VOLTAGE_SHUTDOWN_SHUTDOWN_POSTPONE_DURATION_MS);
  }
}
