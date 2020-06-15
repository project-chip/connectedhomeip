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
 * @brief Routines for the Bulb UI plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/attribute-storage.h"
#include "app/framework/plugin/manufacturing-library-cli/manufacturing-library-cli-plugin.h"
#include EMBER_AF_API_BULB_PWM_DRIVER

#ifdef EMBER_AF_PLUGIN_CONFIGURATION_SERVER
  #include "app/framework/plugin-soc/configuration-server/configuration-server.h"
#endif

#include "app/framework/plugin/find-and-bind-target/find-and-bind-target.h"

// ****************
// stack status typedefs.
#define REBOOT_MONITOR_RE_WRITE_THRESHOLD 30
#define EMBER_AF_BULB_UI_END_OF_RECORD 0xff
typedef struct {
  EmberNetworkStatus networkStatus;
  const uint8_t *name;
} EmberNetworkStates;

EmberNetworkStates emberNetworkStates[] = {
  { EMBER_NO_NETWORK, "EMBER_NO_NETWORK" },
  { EMBER_JOINING_NETWORK, "EMBER_JOINING_NETWORK" },
  { EMBER_JOINED_NETWORK, "EMBER_JOINED_NETWORK" },
  { EMBER_JOINED_NETWORK_NO_PARENT, "EMBER_JOINED_NETWORK_NO_PARENT" },
  { EMBER_LEAVING_NETWORK, "EMBER_LEAVING_NETWORK" },
  { EMBER_AF_BULB_UI_END_OF_RECORD, "EOR" }
};

typedef struct {
  EmberStatus stackStatus;
  const uint8_t *name;
} EmberStackStatus;

EmberStackStatus emberStackStatus[] = {
  { EMBER_NETWORK_UP, "EMBER_NETWORK_UP" },
  { EMBER_NETWORK_DOWN, "EMBER_NETWORK_DOWN" },
  { EMBER_JOIN_FAILED, "EMBER_JOIN_FAILED" },
  { EMBER_MOVE_FAILED, "EMBER_MOVE_FAILED" },
  { EMBER_CANNOT_JOIN_AS_ROUTER, "EMBER_CANNOT_JOIN_AS_ROUTER" },
  { EMBER_NODE_ID_CHANGED, "EMBER_NODE_ID_CHANGED" },
  { EMBER_PAN_ID_CHANGED, "EMBER_PAN_ID_CHANGED" },
  { EMBER_CHANNEL_CHANGED, "EMBER_CHANNEL_CHANGED" },
  { EMBER_NO_BEACONS, "EMBER_NO_BEACONS" },
  { EMBER_RECEIVED_KEY_IN_THE_CLEAR, "EMBER_RECEIVED_KEY_IN_THE_CLEAR" },
  { EMBER_NO_NETWORK_KEY_RECEIVED, "EMBER_NO_NETWORK_KEY_RECEIVED" },
  { EMBER_NO_LINK_KEY_RECEIVED, "EMBER_NO_LINK_KEY_RECEIVED" },
  { EMBER_PRECONFIGURED_KEY_REQUIRED, "EMBER_PRECONFIGURED_KEY_REQUIRED" },
  { EMBER_AF_BULB_UI_END_OF_RECORD, "EOR" }
};

// end of stack status typedefs.
// ****************

// forward declarations
static void emAfBulbUiLeftNetwork(void);
static void resetBindingsAndAttributes(void);
void emberAfGroupsClusterClearGroupTableCallback(uint8_t endpoint);
void emberAfScenesClusterClearSceneTableCallback(uint8_t endpoint);
uint8_t halGetResetInfo(void);
void emberAfPluginBulbUiFinishedCallback(EmberStatus status);
EmberStatus emberAfStartSearchForJoinableNetworkAllChannels(void);

// Framework declaratoins
EmberEventControl emberAfPluginBulbUiBulbUiRebootEventControl;
EmberEventControl emberAfPluginBulbUiRejoinEventControl;

// **********
// LED blinking behavior definitions
#define STACK_UP_BLINK_NUMBER    10
#define STACK_UP_BLINK_TIME_MS   100
#define STACK_LEFT_BLINK_NUMBER  3
#define STACK_LEFT_BLINK_TIME_MS 500
#define STACK_REJOINED_BLINK_NUMBER 0
#define STACK_REJOINED_BLINK_TIME_MS 0
// definitions
#define rejoinEventControl   emberAfPluginBulbUiRejoinEventControl
#define REJOIN_TIME_MINUTES 30
#define BOOT_MONITOR_RESET_TIME_SECONDS 2  // number of seconds before the
// reboot monitor is reset.
#define BOOT_DELAY_MS 100
#define JOIN_DELAY_NO_CALLBACK_SECONDS 40
#define JOIN_DELAY_SECONDS 5
#define MFGLIB_DELAY_SECONDS   10

#define IDENTIFY_TIME_SECONDS (3 * 60)  // three minutes

#define REBOOT_COUNT_LEAVE 10
#define REBOOT_COUNT_FORCE_REJOIN 5
#define REBOOT_COUNT_FINDING_BINDING 4

#define TOTAL_JOIN_ATTEMPTS 20
#define FIRST_JOIN_ATTEMPT 1

// When we are joined, there is a race condition between the stack reporting its
// status and the initial check done by the lighting firmware.  This can result
// in certain builds (i.e. routers) from falsely reporting a newly joined state.
// The fix is to initialize this tracker variable to JOINED so when the stack
// status handler reports its initial joined state, we won't blink.  This
// eliminates the race condition (which only occurs when we are joined).
static EmberNetworkStatus currentNetworkState = EMBER_JOINED_NETWORK;
static uint8_t networkJoinAttempts = 0;
static uint8_t rebootEventState;
static bool leaveNetwork = false;
static void startSearchForJoinableNetwork(void);
static void scheduleSearchForJoinableNetwork(void);
static void setPowerUpOnOffState(void);

enum {
  REBOOT_EVENT_STATE_INITIAL_CHECK       = 0,
  REBOOT_EVENT_STATE_SHORT_REBOOT_TIME   = 1,
  REBOOT_EVENT_STATE_LONG_REBOOT_TIME    = 2,
  REBOOT_EVENT_STATE_WAIT_MFGLIB_SECONDS = 3
};

// bulb ui only supports a single endpoint:
static uint8_t bulbEndpoint(void)
{
  return emberAfEndpointFromIndex(0);
}

// Custom event stubs. Custom events will be run along with all other events in
// the application framework. They should be managed using the Ember Event API
// documented in stack/include/events.h

static void advanceRebootMonitor(void)
{
  tokTypeStackBootCounter shortRebootMonitor, longRebootMonitor;

  halCommonGetToken(&shortRebootMonitor, TOKEN_SHORT_REBOOT_MONITOR);
  halCommonGetToken(&longRebootMonitor, TOKEN_REBOOT_MONITOR);

  emberAfCorePrintln("Bulb UI tracked %d reboots",
                     shortRebootMonitor - longRebootMonitor);

  // "30" is an arbitrary number beyond which it is faster to just set the
  // counter token rather than increment it.
  if ( ((uint16_t) (shortRebootMonitor - longRebootMonitor))
       > REBOOT_MONITOR_RE_WRITE_THRESHOLD) {
    halCommonSetToken(TOKEN_REBOOT_MONITOR, &shortRebootMonitor);
  } else {
    while ( ((uint16_t) (shortRebootMonitor - longRebootMonitor)) > 0) {
      halCommonIncrementCounterToken(TOKEN_REBOOT_MONITOR);
      longRebootMonitor++;
    }
  }
}

static void advanceShortRebootMonitor(void)
{
  halCommonIncrementCounterToken(TOKEN_SHORT_REBOOT_MONITOR);
}

static uint16_t shortRebootWaitTime(void)
{
#ifdef EMBER_AF_PLUGIN_CONFIGURATION_SERVER
  uint16_t waitTime;

  halCommonGetToken((uint8_t *) &waitTime,
                    TOKEN_BULB_UI_MIN_ON_TIME);

  if (waitTime != BULB_PWM_TIME_UNSET) {
    return waitTime;
  }
#endif

  return BOOT_DELAY_MS;
}

static uint16_t longRebootWaitTime(void)
{
  uint16_t waitTime;
#ifdef EMBER_AF_PLUGIN_CONFIGURATION_SERVER

  halCommonGetToken((uint8_t *) &waitTime,
                    TOKEN_BULB_UI_TIMEOUT);

  if (waitTime != BULB_PWM_TIME_UNSET) {
    return waitTime;
  }
#endif

  waitTime = BOOT_MONITOR_RESET_TIME_SECONDS;
  waitTime *= MILLISECOND_TICKS_PER_SECOND;

  return waitTime;
}

// Event function stub
void emberAfPluginBulbUiBulbUiRebootEventHandler(void)
{
  tokTypeStackBootCounter shortRebootMonitor, longRebootMonitor;

  halCommonGetToken(&shortRebootMonitor, TOKEN_SHORT_REBOOT_MONITOR);
  halCommonGetToken(&longRebootMonitor, TOKEN_REBOOT_MONITOR);

  emberAfCorePrintln("Reboot State: %d %d", rebootEventState, emberNetworkState());

  switch (rebootEventState) {
    case REBOOT_EVENT_STATE_INITIAL_CHECK:
      currentNetworkState = emberNetworkState();

      // When we initially boot up, the network state will say we have no parent.
      // This is not something we wish to indicate to a user, as this is expected
      // behavior even if we have a parent.
      if (currentNetworkState > EMBER_JOINED_NETWORK) {
        currentNetworkState = EMBER_JOINED_NETWORK;
      }
      emberAfCorePrintln("INITIAL_CHECK, %d", halCommonGetInt32uMillisecondTick());
      setPowerUpOnOffState();
#ifndef EMBER_TEST
      emberAfCorePrintln("Bulb UI event Reset Reason:  %x %2x %p %p",
                         halGetResetInfo(),
                         halGetExtendedResetInfo(),
                         halGetResetString(),
                         halGetExtendedResetString());
#endif

      advanceShortRebootMonitor();

      rebootEventState = REBOOT_EVENT_STATE_SHORT_REBOOT_TIME;
      emberEventControlSetDelayMS(emberAfPluginBulbUiBulbUiRebootEventControl,
                                  shortRebootWaitTime());

      break;

    case REBOOT_EVENT_STATE_SHORT_REBOOT_TIME:
      emberAfCorePrintln("SHORT_REBOOT, %d", halCommonGetInt32uMillisecondTick());

      if (emberNetworkState() == EMBER_NO_NETWORK) {
        advanceRebootMonitor();
        // don't kick off join if the mfglib is running.
        if (emberAfMfglibEnabled()) {
          emberEventControlSetDelayMS(emberAfPluginBulbUiBulbUiRebootEventControl,
                                      MFGLIB_DELAY_SECONDS * MILLISECOND_TICKS_PER_SECOND);
          rebootEventState = REBOOT_EVENT_STATE_WAIT_MFGLIB_SECONDS;
          return;
        }

        startSearchForJoinableNetwork();
        emberEventControlSetInactive(emberAfPluginBulbUiBulbUiRebootEventControl);
        resetBindingsAndAttributes();
      } else {
        emberAfCorePrintln("counters:  %2x %2x",
                           shortRebootMonitor,
                           longRebootMonitor);

        if (((uint16_t) (shortRebootMonitor - longRebootMonitor))
            >= REBOOT_COUNT_LEAVE) {
          leaveNetwork = true;
        }
        rebootEventState = REBOOT_EVENT_STATE_LONG_REBOOT_TIME;
        // set up the reboot event to run after BOOT_MONITOR_RESET_TIME_SECONDS
        emberEventControlSetDelayMS(emberAfPluginBulbUiBulbUiRebootEventControl,
                                    longRebootWaitTime());
      }

      break;

    case REBOOT_EVENT_STATE_LONG_REBOOT_TIME:
      emberAfCorePrintln("LONG_REBOOT, %d", halCommonGetInt32uMillisecondTick());
      emberAfCorePrintln("Bulb UI wait N seconds, %x",
                         ((int16u) (shortRebootMonitor - longRebootMonitor)));
      if (leaveNetwork) {
        leaveNetwork = false;
        emberLeaveNetwork();
        // depending on the number of reboots, we will want to take one of several actions
      } else if (((uint16_t) (shortRebootMonitor - longRebootMonitor))
                 >= REBOOT_COUNT_FORCE_REJOIN) {
        // Need to force a rejoin here
        emberAfStartMoveCallback();
        emberAfCorePrintln("Bulb UI Rejoin");
      } else if (((uint16_t) (shortRebootMonitor - longRebootMonitor))
                 == REBOOT_COUNT_FINDING_BINDING) {
        // start finding and binding
        emberAfCorePrintln("Bulb UI start finding and binding");
        emberAfPluginFindAndBindTargetStart(bulbEndpoint());
      }
      advanceRebootMonitor();
      emberEventControlSetInactive(emberAfPluginBulbUiBulbUiRebootEventControl);
      break;
    case REBOOT_EVENT_STATE_WAIT_MFGLIB_SECONDS:
      // we get here when the manufacturing lib token is enabled, and
      // the bulb has been on for 10 seconds.
      emberEventControlSetInactive(emberAfPluginBulbUiBulbUiRebootEventControl);
      startSearchForJoinableNetwork();
      break;

    default:
      assert(0);
  }
}

void emberAfPluginBulbUiInitCallback(void)
{
  if (emberAfMfglibRunning()) {
    return;
  }

  rebootEventState = REBOOT_EVENT_STATE_INITIAL_CHECK;
  emberEventControlSetActive(emberAfPluginBulbUiBulbUiRebootEventControl);

  // print out reset reason.
  // this is test code for the eventual changing of bulb level and on/off
  // settings based on a power on reset.
#ifndef EMBER_TEST
  emberAfCorePrintln("Bulb UI Reset Reason:  %x %2x %p %p",
                     halGetResetInfo(),
                     halGetExtendedResetInfo(),
                     halGetResetString(),
                     halGetExtendedResetString());
#endif
}

// *****************join code

static void scheduleSearchForJoinableNetwork(void)
{
  emberEventControlSetDelayMS(rejoinEventControl,
                              (JOIN_DELAY_SECONDS
                               * MILLISECOND_TICKS_PER_SECOND));
}

static void startSearchForJoinableNetwork(void)
{
  if (emberAfMfglibRunning()) {
    return;
  }

  if (networkJoinAttempts < TOTAL_JOIN_ATTEMPTS) {
    networkJoinAttempts++;
    emberAfPluginNetworkSteeringStart();
    // call the event in JOIN_DELAY_SECONDS in case we don't get the callback.
    emberEventControlSetDelayMS(rejoinEventControl,
                                (JOIN_DELAY_NO_CALLBACK_SECONDS
                                 * MILLISECOND_TICKS_PER_SECOND));
  } else {
    emberAfPluginBulbUiFinishedCallback(EMBER_NOT_JOINED);
  }
}

static void resetJoinAttempts(void)
{
  networkJoinAttempts = 0;
}

// stack status routines
static void printNetworkState(void)
{
  uint8_t i = 0;
  EmberNetworkStatus currentNetworkStatus = emberNetworkState();

  while (emberNetworkStates[i].networkStatus != EMBER_AF_BULB_UI_END_OF_RECORD) {
    if (emberNetworkStates[i].networkStatus == currentNetworkStatus) {
      emberAfCorePrint("%p ",
                       emberNetworkStates[i].name);
      return;
    }
    i++;
  }
}

static void printNetworkStatus(EmberStatus status)
{
  uint8_t i = 0;

  while (emberStackStatus[i].stackStatus != EMBER_AF_BULB_UI_END_OF_RECORD) {
    if (emberStackStatus[i].stackStatus == status) {
      emberAfCorePrint("%p",
                       emberStackStatus[i].name);
      return;
    }
    i++;
  }
  emberAfCorePrint("unknown %x", status);
}

void emberAfPluginBulbUiStackStatusCallback(EmberStatus status)
{
  // The network state may have changed.  We need to keep the previous state
  // so we can determine which type of state transition has occurred.
  // Different state transitions result in different UI behavior.
  uint8_t previousNetworkState = currentNetworkState;

  emberAfCorePrint("Stack Status Handler:  ");

  printNetworkState();
  printNetworkStatus(status);

  switch (status) {
    case EMBER_NETWORK_UP:
      if (leaveNetwork) {
        leaveNetwork = false;
        emberLeaveNetwork();
      } else {
        resetJoinAttempts();
        if (previousNetworkState == EMBER_NO_NETWORK) {
          halBulbPwmDriverLedBlink(STACK_UP_BLINK_NUMBER,
                                   STACK_UP_BLINK_TIME_MS);
        } else if (previousNetworkState == EMBER_JOINED_NETWORK_NO_PARENT) {
          halBulbPwmDriverLedBlink(STACK_REJOINED_BLINK_NUMBER,
                                   STACK_REJOINED_BLINK_TIME_MS);
        }

        currentNetworkState = emberNetworkState();
        halBulbPwmDriverStatusOn(0);
      }
      break;
    case EMBER_NETWORK_DOWN:
      halBulbPwmDriverStatusBlink(5, 100);

      currentNetworkState = emberNetworkState();
      break;
    case EMBER_JOIN_FAILED:
      halBulbPwmDriverStatusBlink(5, 100);
      break;
    case EMBER_MOVE_FAILED:
      break;
    case EMBER_CANNOT_JOIN_AS_ROUTER:
      break;
    case EMBER_NODE_ID_CHANGED:
      break;
    case EMBER_PAN_ID_CHANGED:
      break;
    case EMBER_CHANNEL_CHANGED:
      break;
    case EMBER_NO_BEACONS:
      break;
    case EMBER_RECEIVED_KEY_IN_THE_CLEAR:
      break;
    case EMBER_NO_NETWORK_KEY_RECEIVED:
      break;
    case EMBER_NO_LINK_KEY_RECEIVED:
      break;
    case EMBER_PRECONFIGURED_KEY_REQUIRED:
      break;
    default:
      break;
  }
  emberAfCorePrintln("");

  if (status == EMBER_NETWORK_DOWN
      && emberNetworkState() == EMBER_NO_NETWORK) {
    emberAfCorePrintln("BulbUi: search for joinable network");

    emAfBulbUiLeftNetwork();

    scheduleSearchForJoinableNetwork();
  }

  if (status == EMBER_NETWORK_DOWN
      && emberNetworkState() == EMBER_JOINED_NETWORK_NO_PARENT) {
    emberAfCorePrintln("BulbUi: kick off rejoin event");
    emberEventControlSetDelayMinutes(rejoinEventControl, 0);
    halBulbPwmDriverStatusBlink(0xff, 200);
  }
}

// rejoin code
void emberAfPluginBulbUiRejoinEventHandler(void)
{
  emberEventControlSetInactive(rejoinEventControl);

  emberAfCorePrint("Rejoin event function ");
  printNetworkState();

  switch (emberNetworkState()) {
    case EMBER_NO_NETWORK:
      startSearchForJoinableNetwork();
      break;
    case EMBER_JOINED_NETWORK_NO_PARENT:
      // in case the bulb is a sleep end device
      // perform the secure rejoin every 30 minutes until we find a network.
      emberAfCorePrintln("Perform and schedule rejoin");
      emberEventControlSetDelayMinutes(rejoinEventControl, REJOIN_TIME_MINUTES);
      emberFindAndRejoinNetworkWithReason(FALSE, // unsecure rejoin
                                          EMBER_ALL_802_15_4_CHANNELS_MASK,
                                          EMBER_AF_REJOIN_DUE_TO_END_DEVICE_MOVE);
      break;
    case EMBER_JOINING_NETWORK:
      break;
    default:
      emberAfCorePrintln("No More Rejoin!");
      break;
  }
}

/** @brief Finished
 *
 * This callback is fired when the network-find plugin is finished with the
 * forming or joining process.  The result of the operation will be returned
 * in the status parameter.
 *
 * @param status   Ver.: always
 */
void emberAfPluginNetworkFindFinishedCallback(EmberStatus status)
{
  emberAfCorePrintln("BULB_UI:  Network Find status %x", status);
  emberAfPluginBulbUiFinishedCallback(status);

  if (status == EMBER_SUCCESS) {
    emberEventControlSetInactive(rejoinEventControl);
  } else {
    // delay before attempting a new rejoin.
    emberEventControlSetDelayMS(rejoinEventControl,
                                JOIN_DELAY_SECONDS * MILLISECOND_TICKS_PER_SECOND);
  }
}

static void resetBindingsAndAttributes(void)
{
  uint8_t endpoint;
  uint8_t i;

  // need to clear the binding table here
  emberClearBindingTable();
  emberAfClearReportTableCallback();

  for (i = 0; i < emberAfEndpointCount(); i++) {
    endpoint = emberAfEndpointFromIndex(i);

    // first restore defaults.
    emberAfResetAttributes(endpoint);

    // now, clear the scene table for the current endpoint.
    emberAfScenesClusterClearSceneTableCallback(endpoint);
  }
}

// Function to clean up non-network state after we leave a network.
// Also, will kick off the appropriate blinking behavior.
static void emAfBulbUiLeftNetwork(void)
{
  resetBindingsAndAttributes();

  // now, blink the LED 3 times
  halBulbPwmDriverLedBlink(STACK_LEFT_BLINK_NUMBER,
                           STACK_LEFT_BLINK_TIME_MS);
}

// API to kick off a join attempt.
void emberAfPluginBulbUiInitiateNetworkSearch(void)
{
  resetJoinAttempts();

  scheduleSearchForJoinableNetwork();
}

static void setPowerUpOnOffState(void)
{
#ifdef EMBER_AF_PLUGIN_CONFIGURATION_SERVER
  uint8_t defaultBehavior;
  uint8_t onOffState, currentOnOffState, level, currentLevel;
  uint8_t endpoint = bulbEndpoint();

  halCommonGetToken((uint8_t *) &defaultBehavior,
                    TOKEN_BULB_UI_POWER_UP_BEHAVIOR);

  emberAfReadServerAttribute(endpoint,
                             ZCL_ON_OFF_CLUSTER_ID,
                             ZCL_ON_OFF_ATTRIBUTE_ID,
                             (uint8_t *)&currentOnOffState,
                             sizeof(currentOnOffState));

  emberAfReadServerAttribute(endpoint,
                             ZCL_LEVEL_CONTROL_CLUSTER_ID,
                             ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                             (uint8_t *)&currentLevel,
                             sizeof(currentLevel));

  switch (defaultBehavior) {
    case BULB_UI_POWER_UP_ON:
      onOffState = 1;
      level = 0xFE;
      break;
    case BULB_UI_POWER_UP_OFF:
      onOffState = 0;
      level = 0xFE;
      break;
    case BULB_UI_POWER_UP_LAST:
    default:
      // nothing to do.  Just exit.
      return;
      break;
  }

  if (currentLevel != level) {
    // time to write the new level
    emberAfWriteServerAttribute(endpoint,
                                ZCL_LEVEL_CONTROL_CLUSTER_ID,
                                ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
                                (uint8_t *)&level,
                                ZCL_INT8U_ATTRIBUTE_TYPE);
  }

  if (currentOnOffState != onOffState) {
    emberAfWriteServerAttribute(endpoint,
                                ZCL_ON_OFF_CLUSTER_ID,
                                ZCL_ON_OFF_ATTRIBUTE_ID,
                                (uint8_t *)&onOffState,
                                ZCL_BOOLEAN_ATTRIBUTE_TYPE);
  }

#endif
}
