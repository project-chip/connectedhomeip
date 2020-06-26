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
 * @brief CLI for the Polling plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/util/serial/command-interpreter2.h"
#include "end-device-support.h"

// *****************************************************************************
// Forward declarations

void emberAfPluginEndDeviceSupportStatusCommand(void);
void emberAfPluginEndDeviceSupportPollCompletedCallbackCommand(void);
void emberAfPluginEndDeviceSupportForceShortPollCommand(void);

// *****************************************************************************
// Globals

#ifndef EMBER_AF_GENERATE_CLI
EmberCommandEntry emberAfPluginEndDeviceSupportCommands[] = {
  emberCommandEntryAction("status", emberAfPluginEndDeviceSupportStatusCommand, "",
                          "Display the status of the End Device's polling"),

  emberCommandEntryAction("poll-completed-callback", emberAfPluginEndDeviceSupportPollCompletedCallbackCommand, "u",
                          "Sets whether the device's poll completed callback function is enabled"),

  emberCommandEntryAction("force-short-poll", emberAfPluginEndDeviceSupportForceShortPollCommand, "u",
                          "Sets whether the CLI forces the device to short poll"),

  emberCommandEntryTerminator()
};
#endif // EMBER_AF_GENERATE_CLI

const char * pollControlStrings[] = {
  "EMBER_AF_LONG_POLL",
  "EMBER_AF_SHORT_POLL",
};

// *****************************************************************************
// Functions

void emberAfPluginEndDeviceSupportStatusCommand(void)
{
  const char * names[] = {
    EM_AF_GENERATED_NETWORK_STRINGS
  };
  uint8_t i;

  emberAfCorePrintln("End Device Poll Information");
  emberAfCorePrintln("EMBER_END_DEVICE_TIMEOUT:       %d", EMBER_END_DEVICE_POLL_TIMEOUT);
  emberAfCorePrintln("Poll completed callback: %p",
                     (emAfEnablePollCompletedCallback
                      ? "yes"
                      : "no"));
  emberAfCoreFlush();

  for (i = 0; i < EMBER_SUPPORTED_NETWORKS; i++) {
    emberAfPushNetworkIndex(i);
    if (emAfProIsCurrentNetwork()
        && EMBER_END_DEVICE <= emAfCurrentZigbeeProNetwork->nodeType) {
      emberAfCorePrintln("nwk %d [%p]", i, names[i]);
      emberAfCorePrintln("  Current Poll Interval (qs):   %l",
                         emberAfGetCurrentPollIntervalQsCallback());
      emberAfCorePrintln("  Long Poll Interval (qs):      %l",
                         emberAfGetLongPollIntervalQsCallback());
      if (EMBER_SLEEPY_END_DEVICE <= emAfCurrentZigbeeProNetwork->nodeType) {
        emberAfCorePrintln("  Short Poll Interval (qs):     %l",
                           emberAfGetShortPollIntervalQsCallback());
        emberAfCoreFlush();
        emberAfCorePrintln("  Wake Timeout (qs):            %l",
                           emberAfGetWakeTimeoutQsCallback());
        emberAfCoreFlush();
        emberAfCorePrintln("  Wake Timeout Bitmask:         0x%4x",
                           emberAfGetWakeTimeoutBitmaskCallback());
        emberAfCoreFlush();
        emberAfCorePrintln("  Current App Tasks:            0x%4x",
                           emberAfGetCurrentAppTasksCallback());
        emberAfCorePrintln("  Current Poll Control          %p",
                           pollControlStrings[emberAfGetCurrentPollControlCallback()]);
        emberAfCoreFlush();
        emberAfCorePrintln("  Default Poll Control          %p",
                           pollControlStrings[emberAfGetDefaultPollControlCallback()]);
        emberAfCoreFlush();
      }
    }
    emberAfPopNetworkIndex();
  }
}

void emberAfPluginEndDeviceSupportForceShortPollCommand(void)
{
  bool shortPoll = (bool)emberUnsignedCommandArgument(0);
  if (shortPoll) {
    emberAfCorePrintln("Forcing device to short poll");
    emberAfAddToCurrentAppTasksCallback(EMBER_AF_FORCE_SHORT_POLL);
  } else {
    emberAfCorePrintln("Allowing device to long poll");
    emberAfRemoveFromCurrentAppTasksCallback(EMBER_AF_FORCE_SHORT_POLL);
  }
}

void emberAfPluginEndDeviceSupportPollCompletedCallbackCommand(void)
{
  emAfEnablePollCompletedCallback = (bool)emberUnsignedCommandArgument(0);
}
