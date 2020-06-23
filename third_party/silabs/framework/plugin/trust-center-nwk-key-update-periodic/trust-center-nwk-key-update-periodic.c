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
 * @brief Functionality for periodically updating the NWK key.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/util/util.h"

#include "app/framework/plugin/trust-center-nwk-key-update-broadcast/trust-center-nwk-key-update-broadcast.h"

// *****************************************************************************
// Globals

EmberEventControl emberAfPluginTrustCenterNwkKeyUpdatePeriodicMyEventControl;

#define myEvent emberAfPluginTrustCenterNwkKeyUpdatePeriodicMyEventControl

#define KEY_UPDATE_PERIOD EMBER_AF_PLUGIN_TRUST_CENTER_NWK_KEY_UPDATE_PERIODIC_KEY_UPDATE_PERIOD
#define KEY_UPDATE_UNITS EMBER_AF_PLUGIN_TRUST_CENTER_NWK_KEY_UPDATE_PERIODIC_KEY_UPDATE_UNITS

#define DAYS    0
#define MINUTES 1
#define HOURS   2

// All numbers in minutes
#if KEY_UPDATE_UNITS == DAYS
  #define KEY_UPDATE_MULTIPLIER (24 * 60)
  #define KEY_UPDATE_UNITS_STRING "days"
#elif KEY_UPDATE_UNITS == HOURS
  #define KEY_UPDATE_MULTIPLIER (60)
  #define KEY_UPDATE_UNITS_STRING "hours"
#elif KEY_UPDATE_UNITS == MINUTES
  #define KEY_UPDATE_MULTIPLIER (1)
  #define KEY_UPDATE_UNITS_STRING "minutes"
#else
  #error "Error: Unknown value for EMBER_AF_PLUGIN_TRUST_CENTER_NWK_KEY_UPDATE_PERIODIC_KEY_UPDATE_UNITS"
#endif

#define KEY_UPDATE_PERIOD_MINUTES (KEY_UPDATE_PERIOD * KEY_UPDATE_MULTIPLIER)

// The delay value for emberEventControlSetDelayMinutes() is limited to an
// uint16_t so we want to guard against the compiler truncating the value and
// just spitting out a warning.
#if KEY_UPDATE_PERIOD_MINUTES > 65535
  #error "Error: Value is too large: EMBER_AF_PLUGIN_TRUST_CENTER_NWK_KEY_UPDATE_PERIODIC_KEY_UPDATE_UNITS"
#endif

// *****************************************************************************

static void scheduleNextUpdate(void)
{
  emberEventControlSetDelayMinutes(myEvent, KEY_UPDATE_PERIOD_MINUTES);
  emberAfAppPrintln("Next NWK key update in %d %p",
                    KEY_UPDATE_PERIOD,
                    KEY_UPDATE_UNITS_STRING);
}

void emberAfPluginTrustCenterNwkKeyUpdatePeriodicMyEventHandler(void)
{
  // A very simple approach is to always kick off the NWK key update
  // assuming it will take much longer to complete than the period
  // at which we initiate.  If something went wrong and the NWK key update
  // didn't complete, a subsequent key update attempt may help recover this and
  // finish the previous one.  This also handles the reboot case where
  // the TC may have rebooted while in the middle of the key update.

  EmberStatus status = emberAfTrustCenterStartNetworkKeyUpdate();
  emberAfAppPrintln("Starting NWK Key update, status: 0x%X", status);
  scheduleNextUpdate();
}

void emberAfPluginTrustCenterNwkKeyUpdatePeriodicInitCallback(void)
{
  // Even though we might not be on a network at this point,
  // we want to schedule the process for sending key updates from
  // now on.

  // However for the test-harness, we manually manipulate the process
  // so we don't want it to automatically kick off.
#if !defined(EMBER_AF_PLUGIN_TEST_HARNESS)
  scheduleNextUpdate();
#endif
}
