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
 * @brief Implements code to maintain a network connection.  It will implement rejoin
 * algorithms.
 *******************************************************************************
   ******************************************************************************/

#ifdef EMBER_SCRIPTED_TEST
#include "app/framework/plugin-soc/connection-manager/connection-manager-test.h"
#endif

#include "app/framework/include/af.h"
#include "app/framework/util/attribute-storage.h"
#include "app/framework/plugin/manufacturing-library-cli/manufacturing-library-cli-plugin.h"
#include "app/framework/plugin/network-steering/network-steering.h"
#include "connection-manager.h"

//------------------------------------------------------------------------------
// Plugin private macro definitions
#define REJOIN_TIME_MINUTES   EMBER_AF_PLUGIN_CONNECTION_MANAGER_REJOIN_TIME_M
#define REJOIN_FAILED_RETRY_TIME_SECONDS \
  EMBER_AF_PLUGIN_CONNECTION_MANAGER_RETRY_TIME_S
#define REJOIN_FAILED_RETRY_TIME_QS \
  REJOIN_FAILED_RETRY_TIME_SECONDS * 4
#define REJOIN_ATTEMPTS       EMBER_AF_PLUGIN_CONNECTION_MANAGER_REJOIN_ATTEMPTS

#define SECONDS_BETWEEN_JOIN_ATTEMPTS 40
#define QS_BETWEEN_JOIN_ATTEMPTS      (SECONDS_BETWEEN_JOIN_ATTEMPTS * 4)

#define HA_SLEEPY_DEVICE_POLL_LENGTH_S    60
#define HA_SLEEPY_DEVICE_POLL_LENGTH_MS \
  (HA_SLEEPY_DEVICE_POLL_LENGTH_S * MILLISECOND_TICKS_PER_SECOND)

#ifdef EMBER_AF_HAS_SLEEPY_NETWORK
#define FORCE_SHORT_POLL() \
  emberAfAddToCurrentAppTasks(EMBER_AF_FORCE_SHORT_POLL)
#define UNFORCE_SHORT_POLL() \
  emberAfRemoveFromCurrentAppTasks(EMBER_AF_FORCE_SHORT_POLL)
#else
#define FORCE_SHORT_POLL()
#define UNFORCE_SHORT_POLL()
#endif

typedef struct {
  EmberStatus status;
  const char * const message;
} NetworkStateMessage;

typedef struct {
  EmberNetworkStatus status;
  const char * const message;
} EmberStatusMessage;

//------------------------------------------------------------------------------
// Plugin events
EmberEventControl emberAfPluginConnectionManagerRebootEventControl;
EmberEventControl emberAfPluginConnectionManagerRejoinEventControl;
EmberEventControl emberAfPluginConnectionManagerPollEventControl;

//------------------------------------------------------------------------------
// plugin private global variables

static NetworkStateMessage networkStateMessages[] =
{
  { EMBER_NETWORK_UP, "EMBER_NETWORK_UP" },
  { EMBER_NETWORK_DOWN, "EMBER_NETWORK_DOWN" },
  { EMBER_CHANNEL_CHANGED, "EMBER_CHANNEL_CHANGED" },
  { EMBER_JOIN_FAILED, "EMBER_JOIN_FAILED" },
  { EMBER_NO_NETWORK_KEY_RECEIVED, "EMBER_NO_NETWORK_KEY_RECEIVED" },
};

static EmberStatusMessage emberStatusMessages[] =
{
  { EMBER_NO_NETWORK, "EMBER_NO_NETWORK" },
  { EMBER_JOINING_NETWORK, "EMBER_JOINING_NETWORK" },
  { EMBER_JOINED_NETWORK, "EMBER_JOINED_NETWORK" },
  { EMBER_JOINED_NETWORK_NO_PARENT, "EMBER_JOINED_NETWORK_NO_PARENT" },
  { EMBER_LEAVING_NETWORK, "EMBER_LEAVING_NETWORK" },
};

// Track number of times plugin has attempted to join a network
static uint8_t networkJoinAttempts = 0;

//------------------------------------------------------------------------------
// plugin private function prototypes

static void printNetworkState(EmberNetworkStatus emStatus);
static void printStackStatus(EmberStatus status);
static void clearNetworkTables(void);

// forward declaration of plugin callbacks
void emberAfPluginConnectionManagerFinishedCallback(EmberStatus status);
void emberAfPluginConnectionManagerStartNetworkSearchCallback(void);
void emberAfPluginConnectionManagerLeaveNetworkCallback(void);

//------------------------------------------------------------------------------
// Plugin consumed callback implementations

//******************************************************************************
// Init callback, executes sometime early in device boot chain.  This function
// will handle debug and UI LED control on startup, and schedule the reboot
// event to occur after the system finishes initializing.
//******************************************************************************
void emberAfPluginConnectionManagerInitCallback(void)
{
  emberEventControlSetActive(emberAfPluginConnectionManagerRebootEventControl);
}

/** @brief Complete
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to EMBER_SUCCESS to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt. Ver.: always
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID. Ver.: always
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network. Ver.: always
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete. Ver.: always
 */
void emberAfPluginNetworkSteeringCompleteCallback(EmberStatus status,
                                                  uint8_t totalBeacons,
                                                  uint8_t joinAttempts,
                                                  uint8_t finalState)
{
  emberAfAppPrintln("Network Steering Completed: %p (0x%X)",
                    (status == EMBER_SUCCESS ? "Join Success" : "FAILED"),
                    status);
  emberAfAppPrintln("Finishing state: 0x%X", finalState);
  emberAfAppPrintln("Beacons heard: %d\nJoin Attempts: %d", totalBeacons, joinAttempts);
  emberAfAppPrintln("Connection Manager:  Network Find status %x",
                    status);

  if (status == EMBER_SUCCESS) {
    emberEventControlSetInactive(emberAfPluginConnectionManagerRejoinEventControl);
  } else {
    // delay the rejoin for the retry time in seconds set by plugin option
    emberEventControlSetDelayQS(emberAfPluginConnectionManagerRejoinEventControl,
                                (REJOIN_FAILED_RETRY_TIME_QS));
  }
}

//******************************************************************************
// This callback will execute whenever the network stack has a significant
// change in state (network joined, network join failed, etc).
//******************************************************************************
void emberAfPluginConnectionManagerStackStatusCallback(EmberStatus status)
{
  emberAfAppPrint("Stack Status Handler:  ");

  printNetworkState(emberAfNetworkState());
  printStackStatus(status);

  if (status == EMBER_NETWORK_UP) {
    emberAfPluginConnectionManagerFinishedCallback(status);
    emberAfPluginConnectionManagerResetJoinAttempts();

    // After a successful join, a sleepy end device must actively respond to
    // communication from the network in order to not be marked as an
    // unresponsive device.  This is facilitated by manually performing a poll
    // once per second for a minute, which is done with the PollEvent.
    FORCE_SHORT_POLL();
    emberEventControlSetDelayMS(emberAfPluginConnectionManagerPollEventControl,
                                HA_SLEEPY_DEVICE_POLL_LENGTH_MS);
  } else if (status == EMBER_NETWORK_DOWN
             && emberAfNetworkState() == EMBER_NO_NETWORK) {
    // If the network goes down, the device needs to clear its binding table
    // (as a new one will be generated by the network when it comes back up),
    // and clear any groups or scenes the network has set.
    emberAfAppPrintln("Connection Manager: search for joinable network");
    clearNetworkTables();
    networkJoinAttempts = 0;
    emberAfPluginConnectionManagerLeaveNetworkCallback();
    emberEventControlSetDelayQS(emberAfPluginConnectionManagerRejoinEventControl,
                                REJOIN_FAILED_RETRY_TIME_QS);
  } else if (status == EMBER_NETWORK_DOWN
             && emberAfNetworkState() == EMBER_JOINED_NETWORK_NO_PARENT) {
    emberAfAppPrintln("Connection Manager: kick off rejoin event.");
    emberEventControlSetDelayMinutes(
      emberAfPluginConnectionManagerRejoinEventControl,
      REJOIN_TIME_MINUTES);
  }
}

//------------------------------------------------------------------------------
// Plugin event handlers

//******************************************************************************
// Reboot event.  To be called sometime after all system init functions have
// executed.  This function will check the network state, and initiate a search
// for new networks to join if the device is not currently on a network.
//******************************************************************************
void emberAfPluginConnectionManagerRebootEventHandler(void)
{
  uint8_t shortPollForced;

  halCommonGetToken(&shortPollForced, TOKEN_FORCE_SHORT_POLL);

  if (shortPollForced) {
    emberAfAppPrint("Short poll forced to permanently enabled.");
    FORCE_SHORT_POLL();
  } else {
    UNFORCE_SHORT_POLL();
  }

  emberEventControlSetInactive(emberAfPluginConnectionManagerRebootEventControl);

  if (emberAfNetworkState() == EMBER_NO_NETWORK) {
    emberAfPluginConnectionManagerLeaveNetworkCallback();
    emberAfPluginConnectionManagerStartSearchForJoinableNetwork();
  }
}

//******************************************************************************
// Fast Poll event handler.  This will cause the (sleepy) device to poll the
// network for pending data every second for a minute after joining the
// newtork.  This is necessary to service messages from the gatweay in a
// reasonable amount of time (one sec delay as opposed to 30 second delay)
//******************************************************************************
void emberAfPluginConnectionManagerPollEventHandler(void)
{
  uint8_t shortPollForced;
  emberEventControlSetInactive(emberAfPluginConnectionManagerPollEventControl);

  halCommonGetToken(&shortPollForced, TOKEN_FORCE_SHORT_POLL);

  // We should only remove ourselves from the short poll task if one minute
  // has expired since network join AND the token to force short poll is set to
  // disable.
  if (shortPollForced) {
    emberAfAppPrint("Short poll forced to permanently enabled.");
  } else {
    UNFORCE_SHORT_POLL();
  }
}

//******************************************************************************
// Rejoin Event.  This event is used to attempt a network rejoin and to verify
// that the parent node has not died.
//******************************************************************************
void emberAfPluginConnectionManagerRejoinEventHandler(void)
{
  emberEventControlSetInactive(emberAfPluginConnectionManagerRejoinEventControl);

  emberAfAppPrint("Rejoin event function ");
  printNetworkState(emberAfNetworkState());

  switch (emberAfNetworkState()) {
    case EMBER_NO_NETWORK:
      emberAfPluginConnectionManagerStartSearchForJoinableNetwork();
      break;
    case EMBER_JOINED_NETWORK_NO_PARENT:
      // since the sensor is a sleepy end device, perform the secure rejoing
      // every 30 minutes until we find a network.
      emberAfAppPrintln("Perform and schedule rejoin");
      emberEventControlSetDelayMinutes(
        emberAfPluginConnectionManagerRejoinEventControl,
        REJOIN_TIME_MINUTES);
      emberAfStartMoveCallback();
      break;
    case EMBER_JOINING_NETWORK:
      break;
    default:
      emberAfAppPrintln("No More Rejoin!");
      break;
  }
}

//------------------------------------------------------------------------------
// Plugin public API function implementations

// *****************************************************************************
// If this is the first search, search only on preferred channels
// Otherwise, search on all channels
// Make sure another attempt occurs in 40 seconds until 20 failures are seen
// If more than 20 network join attempts fail, inform user via callback
// *****************************************************************************
void emberAfPluginConnectionManagerStartSearchForJoinableNetwork(void)
{
  if (emberAfMfglibRunning() || emberAfMfglibEnabled()) {
    return;
  }
  if (networkJoinAttempts < REJOIN_ATTEMPTS) {
    networkJoinAttempts++;
    emberAfPluginNetworkSteeringStart();
    emberAfPluginConnectionManagerStartNetworkSearchCallback();
    // call the event in 40 seconds in case we don't get the stack status
    // callback (which will happen if there's no network to join)
    emberEventControlSetDelayQS(emberAfPluginConnectionManagerRejoinEventControl,
                                QS_BETWEEN_JOIN_ATTEMPTS);
  } else {
    emberAfAppPrintln("Failed to find network to join within %d attempts",
                      networkJoinAttempts);
    emberAfPluginConnectionManagerFinishedCallback(EMBER_NOT_JOINED);
  }
}

// *****************************************************************************
// Reset network join attempts to zero
// *****************************************************************************
void emberAfPluginConnectionManagerResetJoinAttempts(void)
{
  networkJoinAttempts = 0;
}

// *****************************************************************************
// Print current state of the network, leave the network or start searching
// for a new network if it's not on one.
// *****************************************************************************
void emberAfPluginConnectionManagerLeaveNetworkAndStartSearchForNewOne(void)
{
  printNetworkState(emberAfNetworkState());
  networkJoinAttempts = 0;
  if ((emberAfNetworkState() == EMBER_JOINED_NETWORK)
      || (emberAfNetworkState() == EMBER_JOINED_NETWORK_NO_PARENT)) {
    emberAfPluginConnectionManagerLeaveNetworkCallback();
    emberLeaveNetwork();
  } else {
    emberAfPluginConnectionManagerStartSearchForJoinableNetwork();
  }
}

static void clearNetworkTables(void)
{
  uint8_t endpointIndex;
  uint8_t endpoint;

  emberClearBindingTable();
  emberAfClearReportTableCallback();
  for (endpointIndex = 0; endpointIndex < emberAfEndpointCount();
       endpointIndex++) {
    endpoint = emberAfEndpointFromIndex(endpointIndex);
    emberAfResetAttributes(endpoint);
    emberAfGroupsClusterClearGroupTableCallback(endpoint);
    emberAfScenesClusterClearSceneTableCallback(endpoint);
  }
}

// *****************************************************************************
// Reset all the endpoints, clear all binding, scene, and group tables, leave
// the network, and start searching for a new one.
// *****************************************************************************
void emberAfPluginConnectionManagerFactoryReset(void)
{
  clearNetworkTables();
}

//------------------------------------------------------------------------------
// Plugin private function implementations

static void printStackStatus(EmberStatus emStatus)
{
  uint8_t i;

  for (i = 0; i < COUNTOF(emberStatusMessages); i++) {
    if (emStatus == emberStatusMessages[i].status) {
      emberAfAppPrint("%s ", emberStatusMessages[i].message);
      return;
    }
  }
}

static void printNetworkState(EmberNetworkStatus emStatus)
{
  uint8_t i;

  for (i = 0; i < COUNTOF(networkStateMessages); i++) {
    if (emStatus == networkStateMessages[i].status) {
      emberAfAppPrint("%s ", networkStateMessages[i].message);
      return;
    }
  }
}
