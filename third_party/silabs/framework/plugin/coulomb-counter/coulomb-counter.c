/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @brief Routines for the Coulomb Counter plugin.
 ******************************************************************************/
#include PLATFORM_HEADER

// #include "af.h"
#include "stack/include/ember.h"
#include "stack/include/error.h"
#include "hal/hal.h"
#include "coulomb-counter.h"
#include "hal/micro/token.h"
#include "coulomb-counter-tokens.h"

// ------------------------------------------------------------------------------------
//    PRIVATE DECLARATIONS
// ------------------------------------------------------------------------------------
// Variables
static uint32_t currentState; // <31:16> User Defined states <15:0> EM mode + Radio States
static uint32_t stateTimeMarker = 0; // Marker updated on state transitions
static uint32_t tokenTimeMarker = 0; // Marker updated when we write the token to Flash
static uint32_t energyUsage = 0;
static uint32_t currentEnergyConsumption = 0;
static uint16_t additiveModeConsumption[COULOMB_COUNTER_MAX_NB_USER_DEFINED_ADDITIVE_STATES] = { 0 };

// Prototypes
static void updateConsumption(void);
static uint32_t computeElapsed(uint32_t currentTime, uint32_t previousTime);
static void updateEnergyUsage(bool writeToken);

// Externs
#if EMBER_AF_PLUGIN_COULOMB_COUNTER_TRIGGER_THRESHOLD > 0
extern void emberAfPluginCoulombCounterThresholdReachedCallback(uint32_t recordedEnergyConsumption);
#endif
void emberAfPluginCoulombCounterInitCallback(uint8_t *nbAdditiveModes, uint16_t *additiveModeConsumption);

// ------------------------------------------------------------------------------------
//    PUBLIC API
// ------------------------------------------------------------------------------------

uint16_t emberAfPluginCommandCoulombCounterGetCurrentCoreMode(void)
{
  return (uint16_t)(currentState & EMBER_AF_PLUGIN_COULOMB_COUNTER_MASK);
}

uint16_t emberAfPluginCommandCoulombCounterGetAdditiveModesBitfield(void)
{
  return (uint16_t)((currentState & EMBER_AF_PLUGIN_COULOMB_COUNTER_USER_DEF_STATES)
                    >> COULOMB_COUNTER_MAX_NB_USER_DEFINED_ADDITIVE_STATES);
}

uint32_t emberAfPluginCommandCoulombCounterGetEnergyUsage(void)
{
  DECLARE_INTERRUPT_STATE;

  DISABLE_INTERRUPTS();
  updateEnergyUsage(false);
  RESTORE_INTERRUPTS();

  return energyUsage;
}

void emberAfPluginCommandCoulombCounterResetEnergyUsage(void)
{
  DECLARE_INTERRUPT_STATE;

  DISABLE_INTERRUPTS();
  // Reset our total energy value and the stored token's value
  energyUsage = 0;
  halCommonSetToken(TOKEN_COULOMB_COUNTER_PLUGIN_ENERGY_USAGE, &energyUsage);

  // Reset our state time markers
  stateTimeMarker = halCommonGetInt32uMillisecondTick();
  tokenTimeMarker = stateTimeMarker;
  RESTORE_INTERRUPTS();
}

// ------------------------------------------------------------------------------------
//    CALLBACK SECTION
// ------------------------------------------------------------------------------------
void emberAfPluginCoulombCounterInit(void)
{
  DECLARE_INTERRUPT_STATE;

  DISABLE_INTERRUPTS();

  uint8_t nbAdditiveModes = 0;

  emberAfPluginCoulombCounterInitCallback(&nbAdditiveModes, additiveModeConsumption);

  assert(nbAdditiveModes <= COULOMB_COUNTER_MAX_NB_USER_DEFINED_ADDITIVE_STATES);

  // Get persisted energy consumption value
  halCommonGetToken(&energyUsage, TOKEN_COULOMB_COUNTER_PLUGIN_ENERGY_USAGE);

  // Set time references
  stateTimeMarker = halCommonGetInt32uMillisecondTick();
  tokenTimeMarker = stateTimeMarker;

  // Set default core mode as awake while initializing
  currentState = EMBER_AF_PLUGIN_COULOMB_COUNTER_CORE_MODE_AWAKE;
  updateConsumption();
  RESTORE_INTERRUPTS();
}

void emberAfPluginCoulombCounterTick(void)
{
  DECLARE_INTERRUPT_STATE;
  uint32_t currentTime = halCommonGetInt32uMillisecondTick();

  // If we've gone longer than the configured save rate, write the token to
  // NVM for safe keeping.
  if (computeElapsed(currentTime, tokenTimeMarker)
      >= (EMBER_AF_PLUGIN_COULOMB_COUNTER_TOKEN_SAVE_INTERVAL * 1000)) {
    DISABLE_INTERRUPTS();
    updateEnergyUsage(true);
    RESTORE_INTERRUPTS();
    tokenTimeMarker = currentTime;
  }
}
void halSleepCallback(boolean enter, SleepModes sleepMode)
{
  // Currently only idle and sleep are supported so if sleep mode is not
  // SLEEPMODE_IDLE, themode will be treated as sleep

  if (enter == true) {
    if (sleepMode == SLEEPMODE_IDLE) {
      emberAfPluginCoulombCounterEventCallback(
        EMBER_AF_PLUGIN_COULOMB_COUNTER_EM_MODE_MASK,
        EMBER_AF_PLUGIN_COULOMB_COUNTER_CORE_MODE_IDLE);
    } else {
      emberAfPluginCoulombCounterEventCallback(
        EMBER_AF_PLUGIN_COULOMB_COUNTER_EM_MODE_MASK,
        EMBER_AF_PLUGIN_COULOMB_COUNTER_CORE_MODE_SLEEP);
    }
  } else {
    emberAfPluginCoulombCounterEventCallback(
      EMBER_AF_PLUGIN_COULOMB_COUNTER_EM_MODE_MASK,
      EMBER_AF_PLUGIN_COULOMB_COUNTER_CORE_MODE_AWAKE);
  }
}

void coexCoulombCounterEventCallback(uint16_t ccState)
{
  emberAfPluginCoulombCounterEventCallback(EMBER_AF_PLUGIN_COULOMB_COUNTER_RADIO_EVENT_MASK,
                                           ((ccState & EMBER_AF_PLUGIN_COULOMB_COUNTER_RADIO_MODE_TX) != 0U)
                                           ? EMBER_AF_PLUGIN_COULOMB_COUNTER_RADIO_MODE_TX
                                           : ccState);
}

void emberAfPluginCoulombCounterEventCallback(uint32_t mask, uint32_t options)
{
  uint32_t newState;
  DECLARE_INTERRUPT_STATE;

  DISABLE_INTERRUPTS();

  // Derive what the new state will be
  newState = currentState & ~mask;
  newState |= options;

  if (newState != currentState) {
    // Update the energy usage since we're now switching states
    updateEnergyUsage(false);

    // Recompute the energy consumption for this new state
    updateConsumption();

    // Update the actual state variable
    currentState = newState;
  }

  RESTORE_INTERRUPTS();
}
#ifdef EMBER_AF_PLUGIN_EEPROM
#include EMBER_AF_API_EEPROM
void emberAfPluginEepromStateChangeCallback(HalEepromState oldEepromState, HalEepromState newEepromState)
{
  uint32_t eepromStateShifted = ((uint32_t)newEepromState << EMBER_AF_PLUGIN_COULOMB_COUNTER_EEPROM_STATE_SHIFT) & EMBER_AF_PLUGIN_COULOMB_COUNTER_EEPROM_MODE_MASK;

  emberAfPluginCoulombCounterEventCallback(EMBER_AF_PLUGIN_COULOMB_COUNTER_EEPROM_MODE_MASK,
                                           eepromStateShifted);
}
#endif //EMBER_AF_PLUGIN_EEPROM

// ------------------------------------------------------------------------------------
//    PRIVATE SECTION
// ------------------------------------------------------------------------------------
static void updateConsumption(void)
{
  uint32_t userDefinedStates = currentState >> COULOMB_COUNTER_MAX_NB_USER_DEFINED_ADDITIVE_STATES;
  uint32_t i = 0;
  currentEnergyConsumption = 0;

  // Add in the energy consumption for normal EM modes
  if ((currentState & EMBER_AF_PLUGIN_COULOMB_COUNTER_CORE_MODE_AWAKE) != 0U) {
    currentEnergyConsumption += EMBER_AF_PLUGIN_COULOMB_COUNTER_ENERGY_CONSUMPTION_WHILE_AWAKE;
  } else if ((currentState & EMBER_AF_PLUGIN_COULOMB_COUNTER_CORE_MODE_IDLE) != 0U) {
    currentEnergyConsumption += EMBER_AF_PLUGIN_COULOMB_COUNTER_ENERGY_CONSUMPTION_WHILE_IDLE;
  } else if ((currentState & EMBER_AF_PLUGIN_COULOMB_COUNTER_CORE_MODE_SLEEP) != 0U) {
    currentEnergyConsumption += EMBER_AF_PLUGIN_COULOMB_COUNTER_ENERGY_CONSUMPTION_WHILE_ASLEEP;
  }

  // Add in the current for radio modes
  if ((currentState & EMBER_AF_PLUGIN_COULOMB_COUNTER_RADIO_MODE_TX) != 0U) {
    currentEnergyConsumption += EMBER_AF_PLUGIN_COULOMB_COUNTER_ENERGY_CONSUMPTION_WHILE_TX;
  } else if ((currentState & EMBER_AF_PLUGIN_COULOMB_COUNTER_RADIO_MODE_RX) != 0U) {
    currentEnergyConsumption += EMBER_AF_PLUGIN_COULOMB_COUNTER_ENERGY_CONSUMPTION_WHILE_RX;
  }

  // Sum up the current from any additive states
  while (userDefinedStates != 0U) {
    if (userDefinedStates & 1U) {
      currentEnergyConsumption += additiveModeConsumption[i];
    }
    userDefinedStates >>= 1U;
    i++;
  }
}

static void updateEnergyUsage(bool writeToken)
{
  uint32_t currentTime = halCommonGetInt32uMillisecondTick();
  uint32_t timeSpentInState = computeElapsed(currentTime, stateTimeMarker);

  // At this point, we have the full current draw, we need to convert it to current draw per hour and persist it
  // WARNING: Very small energy draws or very short durations might result in a value of 0 after rounding, adjust your units
  //          accordingly.
  energyUsage += currentEnergyConsumption * timeSpentInState;
  if (writeToken) {
    halCommonSetToken(TOKEN_COULOMB_COUNTER_PLUGIN_ENERGY_USAGE, &energyUsage);
  }

#if EMBER_AF_PLUGIN_COULOMB_COUNTER_TRIGGER_THRESHOLD > 0
  // If the threshold has been reached, trigger the callback
  if (energyUsage >= EMBER_AF_PLUGIN_COULOMB_COUNTER_TRIGGER_THRESHOLD) {
    emberAfPluginCoulombCounterThresholdReachedCallback(energyUsage);
  }
#endif

  // Update the time marker now that we've included this energy in the total
  stateTimeMarker = currentTime;
}

static uint32_t computeElapsed(uint32_t currentTime, uint32_t previousTime)
{
  uint32_t elapsed;

  if (currentTime >= previousTime) {
    // Simple subtraction
    elapsed = currentTime - previousTime;
  } else {
    // Handle counter rollover
    elapsed = previousTime - currentTime;
  }

  return elapsed;
}
