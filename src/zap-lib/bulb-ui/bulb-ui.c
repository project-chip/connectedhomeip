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

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_BULB_PWM_DRIVER
#include EMBER_AF_API_CONNECTION_MANAGER
#include EMBER_AF_API_HAL
#include EMBER_AF_API_ZCL_CORE
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

#define BULB_UI_SHORT_REBOOT_TIMEOUT_MS  (1 * MILLISECOND_TICKS_PER_SECOND)

// When flash attribute storage is supported, level will be retained from state
// before poweron.  For now, if the device is set to power on with the bulb on,
// this value will be used for the default level.
#define DEFAULT_LEVEL                             0x20

// Number of times to blink and duration of blink to signal various network
// states
#define EZ_MODE_BLINK_NUMBER                      2
#define EZ_MODE_BLINK_TIME_MS                     200
#define EZ_MODE_BLINK_REPEAT_TIME_MS              (4 * MILLISECOND_TICKS_PER_SECOND)
#define NEW_BIND_BLINK_NUMBER                     3
#define NEW_BIND_BLINK_TIME_MS                    500
#define STACK_LEFT_BLINK_NUMBER                   5
#define STACK_LEFT_BLINK_TIME_MS                  100

#define DEFAULT_IDENTIFY_TIME_S      180

enum {
  REBOOT_EVENT_STATE_INITIAL_CHECK       = 0,
  REBOOT_EVENT_STATE_SHORT_REBOOT_TIME   = 1,
};

static void incrementShortRebootMonitor(void);
static void resetBindingsAndAttributes(void);
static void enableIdentify(void);
static void joinNetwork(void);
static void leaveNetwork(void);
static size_t getNumberOfUsedBinds(void);

EmberEventControl emBulbUiEzModeControl;
EmberEventControl emBulbUiEzBlinkControl;
EmberEventControl emBulbUiRebootControl;

static uint8_t rebootEventState;
static bool initiateEzModeOnAttach;
static size_t numBindingsStartEzMode = 0;
static EmberZclEndpointId_t endpoint = EMBER_AF_PLUGIN_BULB_UI_LIGHT_ENDPOINT;

void emBulbUiInitHandler(void)
{
  EmberZclEndpointIndex_t endpointIndex;
  rebootEventState = REBOOT_EVENT_STATE_INITIAL_CHECK;
  initiateEzModeOnAttach = false;

  endpointIndex = emberZclEndpointIdToIndex(EMBER_AF_PLUGIN_BULB_UI_LIGHT_ENDPOINT,
                                            &emberZclClusterOnOffServerSpec);
  if (endpointIndex == EMBER_ZCL_ENDPOINT_INDEX_NULL) {
    emberAfCorePrintln("####ERROR!! Invalid endpoint selected for bulb UI Light Endpoint option!  Bulb UI will not function properly!!\n");
    endpoint = emZclEndpointTable[0].endpointId;
  }

  emberEventControlSetActive(emBulbUiRebootControl);
}

// On boot, increment the counter token and perform any bulb initialization.
// Then, wait the short reboot timeout.  If the device power cycles before that
// amount of time passes, the counter will be incremented again, and so on
// until the user has power cycled enough times to cause the desired activity
// to take place.  If the reboot event does activate a second time, it means
// that the user is done short power cycling, so action should occur based on
// the following table:
//
// 0-2 reboots: No action
// 3-4 reboots: Begin EZ Mode commissioning
// 5-6 reboots: Enter identify mode
// 7-9 reboots: Reset the existing binding table
// 10+ reboots: Perform a network reset
void emBulbUiRebootHandler(void)
{
  uint32_t shortRebootMonitor;

  halCommonGetToken(&shortRebootMonitor, TOKEN_SHORT_REBOOT_MONITOR);

  switch (rebootEventState) {
    case REBOOT_EVENT_STATE_INITIAL_CHECK:
      emberAfDebugPrintln("Bulb UI event Reset Reason:  %x %2x %p %p",
                          halGetResetInfo(),
                          halGetExtendedResetInfo(),
                          halGetResetString(),
                          halGetExtendedResetString());

      incrementShortRebootMonitor();
      rebootEventState = REBOOT_EVENT_STATE_SHORT_REBOOT_TIME;
      emberEventControlSetDelayMS(emBulbUiRebootControl,
                                  BULB_UI_SHORT_REBOOT_TIMEOUT_MS);

      break;

    case REBOOT_EVENT_STATE_SHORT_REBOOT_TIME:
      emberEventControlSetInactive(emBulbUiRebootControl);
      emberAfCorePrintln("Short reboot timer ended, %d reboots detected",
                         shortRebootMonitor);

      if (shortRebootMonitor >= 10) {
        leaveNetwork();
        // Try again to join a network, it could be that we are detaching from
        // an old network and joining a new one.
        joinNetwork();
      } else if (shortRebootMonitor >= 7) {
        resetBindingsAndAttributes();
      } else if (shortRebootMonitor >= 5) {
        enableIdentify();
      } else if (shortRebootMonitor >= 3) {
        // If the user wants to enter EZ Mode and the device is already attached
        // to the network, do so immediately.  Otherwise, set the initEzMode flag
        // so that the device will enter ez mode on the next attach.
        if (emberNetworkStatus() == EMBER_JOINED_NETWORK_ATTACHED) {
          initiateEzModeOnAttach = false;
          emberEventControlSetActive(emBulbUiEzModeControl);
        } else {
          initiateEzModeOnAttach = true;
        }
      } else if (shortRebootMonitor == 1) {
        // If the status is no network, then also try to join one now since we
        // are just now booting up
        if (emberNetworkStatus() == EMBER_NO_NETWORK) {
          joinNetwork();
        }
      }

      // Once the short reboot sequence is complete, reset the short reboot count
      // to zero and return to normal operation.
      shortRebootMonitor = 0;
      halCommonSetToken(TOKEN_SHORT_REBOOT_MONITOR, &shortRebootMonitor);
      break;

    default:
      assert(0);
  }
}

void emBulbUiNetworkStatusHandler(EmberNetworkStatus newNetworkStatus,
                                  EmberNetworkStatus oldNetworkStatus,
                                  EmberJoinFailureReason reason)
{
  if ((newNetworkStatus == EMBER_JOINED_NETWORK_ATTACHED)
      && (initiateEzModeOnAttach)) {
    // Ez Mode should only start on the first attach, not every time the bulb
    // attaches / reattaches for the life of the bulb
    initiateEzModeOnAttach = false;
    emberEventControlSetDelayMS(emBulbUiEzModeControl,
                                EZ_MODE_BLINK_REPEAT_TIME_MS);
  } else if ((newNetworkStatus == EMBER_NO_NETWORK)
             && (oldNetworkStatus == EMBER_JOINED_NETWORK_ATTACHED)) {
    emberAfCorePrintln("Lost network");
    halBulbPwmDriverLedBlink(STACK_LEFT_BLINK_NUMBER,
                             STACK_LEFT_BLINK_TIME_MS);
  }
}

// Before starting EZ Mode, the system will determine how many binds are
// present in the binding table (as there is no callback generated when a bind
// is created).  A call to emberZclStartEzMode will then initiate EZ mode
// operation, and the bulb will start to blink the EZ Mode search pattern.  The
// ezModeBlink event will then be used to poll the number of binds created
// every few seconds to see if any new entries have been generated.
void emBulbUiEzModeHandler(void)
{
  EmberStatus status;

  emberEventControlSetInactive(emBulbUiEzModeControl);

  status = emberZclStartEzMode();

  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("starting ez mode\n");
    numBindingsStartEzMode = getNumberOfUsedBinds();
    halBulbPwmDriverLedBlink(EZ_MODE_BLINK_NUMBER,
                             EZ_MODE_BLINK_TIME_MS);
    emberEventControlSetDelayMS(emBulbUiEzBlinkControl,
                                EZ_MODE_BLINK_REPEAT_TIME_MS);
  } else {
    emberAfCorePrintln("Unable to start EZ mode: %d", status);
  }
}

void emBulbUiEzBlinkHandler(void)
{
  size_t numBindings;
  bool blinkForNewBind = false;

  numBindings = getNumberOfUsedBinds();

  if (numBindings != numBindingsStartEzMode) {
    emberAfCorePrintln("%d new bindings created",
                       numBindings - numBindingsStartEzMode);
    blinkForNewBind = true;

    numBindingsStartEzMode = numBindings;
    emberEventControlSetDelayMS(emBulbUiEzBlinkControl,
                                EZ_MODE_BLINK_REPEAT_TIME_MS);
  }

  if (emberZclEzModeIsActive()) {
    if (blinkForNewBind) {
      halBulbPwmDriverLedBlink(NEW_BIND_BLINK_NUMBER,
                               NEW_BIND_BLINK_TIME_MS);
    } else {
      halBulbPwmDriverLedBlink(EZ_MODE_BLINK_NUMBER,
                               EZ_MODE_BLINK_TIME_MS);
    }
    emberEventControlSetDelayMS(emBulbUiEzBlinkControl,
                                EZ_MODE_BLINK_REPEAT_TIME_MS);
  } else {
    emberEventControlSetInactive(emBulbUiEzBlinkControl);
  }
}

static size_t getNumberOfUsedBinds(void)
{
  EmberZclBindingId_t numberOfBinds = 0;
  EmberZclBindingId_t currentBindIndex;
  EmberZclBindingEntry_t currentBind;

  for (currentBindIndex = 0;
       currentBindIndex < EMBER_ZCL_BINDING_TABLE_SIZE;
       currentBindIndex++) {
    // Check to see if the binding table entry is active
    if (emberZclGetBinding(currentBindIndex, &currentBind)) {
      numberOfBinds++;
    }
  }
  return numberOfBinds;
}

static void resetBindingsAndAttributes(void)
{
  emberAfCorePrintln("Clearing binding table and resetting all attributes\n");
  emberZclRemoveAllBindings();
  emberZclResetAttributes(endpoint);
}

static void incrementShortRebootMonitor(void)
{
  halCommonIncrementCounterToken(TOKEN_SHORT_REBOOT_MONITOR);
}

static void enableIdentify(void)
{
  uint16_t identifyTimeS = DEFAULT_IDENTIFY_TIME_S;
  EmberZclStatus_t status;
  EmberZclEndpointId_t endpoint;
  EmberZclEndpointIndex_t i;

  emberAfCorePrintln("bulb-ui: identify mode enabled for %d seconds", identifyTimeS);

  for (i = 0; i < emZclEndpointCount; i++) {
    endpoint = emberZclEndpointIndexToId(i,
                                         &emberZclClusterIdentifyServerSpec);
    if (endpoint != EMBER_ZCL_ENDPOINT_NULL) {
      status = emberZclWriteAttribute(endpoint,
                                      &emberZclClusterIdentifyServerSpec,
                                      EMBER_ZCL_CLUSTER_IDENTIFY_SERVER_ATTRIBUTE_IDENTIFY_TIME,
                                      &identifyTimeS,
                                      sizeof(identifyTimeS));
      if (status != EMBER_ZCL_STATUS_SUCCESS) {
        emberAfCorePrintln("End node UI unable to identify on endpoint %d!",
                           i);
      } else {
        emberAfCorePrintln("Identifying for %d seconds on endpoint %d",
                           identifyTimeS,
                           i);
      }
    }
  }

  return;
}

static void leaveNetwork(void)
{
  emberAfCorePrintln("Attempting to reset the network connection");
  emberConnectionManagerLeaveNetwork();
}

static void joinNetwork(void)
{
  emberAfCorePrintln("Attempting to join a network");
  emberConnectionManagerStartConnect();
}

void emberZclIdentifyServerStartIdentifyingCallback(uint16_t identifyTimeS)
{
  // This callback is called whenever the endpoint should identify itself.  The
  // identification procedure is application specific, and could be implemented
  // by blinking an LED, playing a sound, or displaying a message.

  emberAfCorePrintln("Identifying...");
  halBulbPwmDriverLedBlink(HAL_BULB_PWM_DRIVER_BLINK_FOREVER, 1000);
}

void emberZclIdentifyServerStopIdentifyingCallback(void)
{
  // This callback is called whenever the endpoint should stop identifying
  // itself.

  emberAfCorePrintln("Identify complete");
  halBulbPwmDriverLedOn(0);
}
