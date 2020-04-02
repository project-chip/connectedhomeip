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
#include EMBER_AF_API_CONNECTION_MANAGER
#include EMBER_AF_API_HAL
#include EMBER_AF_API_LED_BLINK
#include EMBER_AF_API_STACK
#include EMBER_AF_API_ZCL_CORE
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

#define DEBOUNCE_TIME_MS \
  EMBER_AF_PLUGIN_END_NODE_UI_BUTTON_DEBOUNCE_TIME_MS
#define BUTTON_NETWORK_LEAVE_TIME_MS  (1 * MILLISECOND_TICKS_PER_SECOND)

#define DEFAULT_IDENTIFY_TIME_S      180

// These are default values to modify the UI LED's blink pattern for network
// join and network leave.
#define LED_LOST_ON_TIME_MS          250
#define LED_LOST_OFF_TIME_MS         750
#define LED_BLINK_ON_TIME_MS         200
#define LED_FOUND_BLINK_OFF_TIME_MS  250
#define LED_FOUND_BLINK_ON_TIME_MS   250
#define LED_IDENTIFY_ON_TIME_MS      250
#define LED_IDENTIFY_OFF1_TIME_MS    250
#define LED_IDENTIFY_OFF2_TIME_MS    1250
#define DEFAULT_NUM_IDENTIFY_BLINKS  DEFAULT_IDENTIFY_TIME_S * 1000 / (2                           \
                                                                       * LED_IDENTIFY_ON_TIME_MS   \
                                                                       + LED_IDENTIFY_OFF1_TIME_MS \
                                                                       + LED_IDENTIFY_OFF2_TIME_MS)
#define EZ_BIND_FOUND_ON_TIME_MS     750
#define EZ_BIND_FOUND_OFF_TIME_MS    250
#define NUM_BINDS_CHANGED_NUM_BLINKS 4

#define LED_SEARCH_BLINK_OFF_TIME_MS 1800

// Each join attempt is about 14500ms long, blink pattern is 3000ms long.
#if defined EMBER_AF_PLUGIN_CONNECTION_MANAGER_JIB_NUM_REJOIN_ATTEMPTS
  #define DEFAULT_NUM_SEARCH_BLINKS  EMBER_AF_PLUGIN_CONNECTION_MANAGER_JIB_NUM_REJOIN_ATTEMPTS * 5
#elif defined EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_NUM_REJOIN_ATTEMPTS
  #define DEFAULT_NUM_SEARCH_BLINKS  EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_NUM_REJOIN_ATTEMPTS * 5
#else
  #define DEFAULT_NUM_SEARCH_BLINKS  100
#endif

#define DEFAULT_EZ_MODE_BLINKS       3
#define DEFAULT_NUM_FOUND_BLINKS     6
#define DEFAULT_NUM_LEAVE_BLINKS     3

#define EZ_MODE_BLINK_REPEAT_TIME_MS  (4 * MILLISECOND_TICKS_PER_SECOND)

static void resetBindingsAndAttributes(void);
static void enableIdentify(void);
static void leaveNetwork(void);
static size_t getNumberOfUsedBinds(void);

EmberEventControl emEndNodeUiEzModeControl;
EmberEventControl emEndNodeUiEzBlinkControl;
EmberEventControl emEndNodeUiButtonPressCountEventControl;

// State variables for controlling LED blink behavior on network join/leave
static uint16_t networkLostBlinkPattern[] =
{ LED_LOST_ON_TIME_MS, LED_LOST_OFF_TIME_MS };
static uint16_t ezModeSearchingBlinkPattern[] =
{ LED_BLINK_ON_TIME_MS, LED_SEARCH_BLINK_OFF_TIME_MS, };
static uint16_t networkSearchBlinkPattern[] =
{ 250, 250, 250, 2250 };
static uint16_t networkFoundBlinkPattern[] =
{ LED_FOUND_BLINK_ON_TIME_MS, LED_FOUND_BLINK_OFF_TIME_MS };
static uint16_t numberOfBindsChangedBlinkPattern[] =
{ EZ_BIND_FOUND_ON_TIME_MS, EZ_BIND_FOUND_OFF_TIME_MS };
static uint16_t networkIdentifyBlinkPattern[] =
{ LED_IDENTIFY_ON_TIME_MS, LED_IDENTIFY_OFF1_TIME_MS,
  LED_IDENTIFY_ON_TIME_MS, LED_IDENTIFY_OFF2_TIME_MS };

static uint8_t consecutiveButtonPressCount = 0;
static size_t numBindingsStartEzMode = 0;
static uint8_t networkLed = EMBER_AF_PLUGIN_END_NODE_UI_NETWORK_ACTIVITY_LED;

void emberAfPluginButtonInterfaceButton0PressedShortCallback(uint16_t timePressedMs)
{
  // If the button was not held for longer than the debounce time, ignore the
  // press.
  if (timePressedMs < DEBOUNCE_TIME_MS) {
    return;
  }

  if (timePressedMs >= BUTTON_NETWORK_LEAVE_TIME_MS) {
    leaveNetwork();
  } else {
    consecutiveButtonPressCount++;
    emberEventControlSetDelayMS(emEndNodeUiButtonPressCountEventControl,
                                EMBER_AF_PLUGIN_END_NODE_UI_CONSECUTIVE_PRESS_TIMEOUT_MS);
  }
}

void emberEndNodeUiSetUiLed(uint8_t led)
{
  networkLed = led;
}

void emEndNodeUiButtonPressCountEventHandler(void)
{
  emberEventControlSetInactive(emEndNodeUiButtonPressCountEventControl);

  if (emberNetworkStatus() == EMBER_JOINED_NETWORK_ATTACHED) {
    // If on a network:
    // 1 press   starts EZ Mode commissioning
    // 2 presses activates identify
    // 3 presses blinks network status
    // 4 presses resets the bindings and attributes
    // 5 presses sends an identify to anyone the device is controlling (not yet
    //   supported)
    switch (consecutiveButtonPressCount) {
      case 1:
        emberEventControlSetActive(emEndNodeUiEzModeControl);
        break;
      case 2:
        enableIdentify();
        break;
      case 3:
        halMultiLedBlinkPattern(DEFAULT_NUM_FOUND_BLINKS,
                                COUNTOF(networkFoundBlinkPattern),
                                networkFoundBlinkPattern,
                                networkLed);
        break;
      case 4:
        resetBindingsAndAttributes();
        break;
      default:
        break;
    }
  } else {
    emberAfCorePrintln("attempting to connect");
    emberConnectionManagerStartConnect();
    halMultiLedBlinkPattern(DEFAULT_NUM_SEARCH_BLINKS,
                            COUNTOF(networkSearchBlinkPattern),
                            networkSearchBlinkPattern,
                            networkLed);
  }

  consecutiveButtonPressCount = 0;
}

void emberConnectionManagerConnectCompleteCallback(EmberConnectionManagerConnectionStatus status)
{
  if (status == EMBER_CONNECTION_MANAGER_STATUS_TIMED_OUT) {
    halMultiLedBlinkLedOff(0, networkLed);
  } else if (status == EMBER_CONNECTION_MANAGER_STATUS_CONNECTED) {
    halMultiLedBlinkPattern(DEFAULT_NUM_FOUND_BLINKS,
                            COUNTOF(networkFoundBlinkPattern),
                            networkFoundBlinkPattern,
                            networkLed);
  }
}

void emEndNodeUiNetworkStatusHandler(EmberNetworkStatus newNetworkStatus,
                                     EmberNetworkStatus oldNetworkStatus,
                                     EmberJoinFailureReason reason)
{
  static bool blinkingSearchPattern = false;
  static bool blinkingOrphanPattern = false;

  switch (newNetworkStatus) {
    case EMBER_NO_NETWORK:
    case EMBER_JOINED_NETWORK_ATTACHING:
    case EMBER_JOINING_NETWORK:
    case EMBER_JOINED_NETWORK_NO_PARENT:
      // If the device has not yet started blinking its state appropriate
      // notification, start blinking it.  Note that when the device starts up,
      // it will enter a NO_PARENT state before the connection manager is able to
      // detect the orphan state, so the orphan blink pattern should be able to
      // supersede the network serarch blink pattern
      if (emberConnectionmanagerIsOrphaned()) {
        if (!blinkingOrphanPattern) {
          halMultiLedBlinkPattern(DEFAULT_NUM_LEAVE_BLINKS,
                                  COUNTOF(networkLostBlinkPattern),
                                  networkLostBlinkPattern,
                                  networkLed);
          blinkingOrphanPattern = true;
        }
      } else if (!blinkingSearchPattern && !blinkingOrphanPattern) {
        halMultiLedBlinkPattern(DEFAULT_NUM_SEARCH_BLINKS,
                                COUNTOF(networkSearchBlinkPattern),
                                networkSearchBlinkPattern,
                                networkLed);
        blinkingSearchPattern = true;
      }
      break;
    case EMBER_JOINED_NETWORK_ATTACHED:
      blinkingSearchPattern = false;
      blinkingOrphanPattern = false;
      break;
    default:
      break;
  }
}

// Before starting EZ Mode, the system will determine how many binds are
// present in the binding table (as there is no callback generated when a bind
// is created).  A call to emberZclStartEzMode will then initiate EZ mode
// operation, and the device will start to blink the EZ Mode search pattern. The
// ezModeBlink event will then be used to poll the number of binds created
// every few seconds to see if any new entries have been generated.
void emEndNodeUiEzModeHandler(void)
{
  EmberStatus status;

  emberEventControlSetInactive(emEndNodeUiEzModeControl);

  status = emberZclStartEzMode();

  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("starting ez mode");
    numBindingsStartEzMode = getNumberOfUsedBinds();

    halMultiLedBlinkPattern(DEFAULT_EZ_MODE_BLINKS,
                            COUNTOF(ezModeSearchingBlinkPattern),
                            ezModeSearchingBlinkPattern,
                            networkLed);

    emberEventControlSetDelayMS(emEndNodeUiEzBlinkControl,
                                EZ_MODE_BLINK_REPEAT_TIME_MS);
  } else {
    emberAfCorePrintln("Unable to start EZ mode: %d", status);
  }
}

void emEndNodeUiEzBlinkHandler(void)
{
  size_t numBindings;
  bool blinkForNewBind = false;

  numBindings = getNumberOfUsedBinds();

  if (numBindings != numBindingsStartEzMode) {
    emberAfCorePrintln("%d new bindings created",
                       numBindings - numBindingsStartEzMode);
    blinkForNewBind = true;

    numBindingsStartEzMode = numBindings;
    emberEventControlSetDelayMS(emEndNodeUiEzBlinkControl,
                                EZ_MODE_BLINK_REPEAT_TIME_MS);
  }

  // If the network went down mid ez-mode, the LED will be blinking some type of
  // search pattern, so stop blinking the EZ pattern
  if (emberNetworkStatus() != EMBER_JOINED_NETWORK_ATTACHED) {
    emberEventControlSetInactive(emEndNodeUiEzBlinkControl);
  } else if (emberZclEzModeIsActive()) {
    if (blinkForNewBind) {
      halMultiLedBlinkPattern(NUM_BINDS_CHANGED_NUM_BLINKS,
                              COUNTOF(numberOfBindsChangedBlinkPattern),
                              numberOfBindsChangedBlinkPattern,
                              networkLed);
    } else {
      halMultiLedBlinkPattern(DEFAULT_EZ_MODE_BLINKS,
                              COUNTOF(ezModeSearchingBlinkPattern),
                              ezModeSearchingBlinkPattern,
                              networkLed);
    }
    emberEventControlSetDelayMS(emEndNodeUiEzBlinkControl,
                                EZ_MODE_BLINK_REPEAT_TIME_MS);
  } else {
    halLedBlinkLedOff(0);
    emberEventControlSetInactive(emEndNodeUiEzBlinkControl);
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
  size_t i;
  EmberZclEndpointId_t currentEndpoint;

  emberAfCorePrintln("Clearing binding table and resetting all attributes\n");
  emberZclRemoveAllBindings();

  for (i = 0; i < emZclEndpointCount; i++) {
    currentEndpoint = emberZclEndpointIndexToId(i, NULL);
    emberZclResetAttributes(currentEndpoint);
  }

  halMultiLedBlinkPattern(NUM_BINDS_CHANGED_NUM_BLINKS,
                          COUNTOF(numberOfBindsChangedBlinkPattern),
                          numberOfBindsChangedBlinkPattern,
                          networkLed);
}

static void enableIdentify(void)
{
  uint16_t identifyTimeS = DEFAULT_IDENTIFY_TIME_S;
  EmberZclStatus_t status;
  EmberZclEndpointId_t endpoint;
  EmberZclEndpointIndex_t i;

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
  // No need to blink here, as the blink will be performed in the network
  // status handler
  emberAfCorePrintln("Resetting the network connection");
  halMultiLedBlinkPattern(DEFAULT_NUM_LEAVE_BLINKS,
                          COUNTOF(networkLostBlinkPattern),
                          networkLostBlinkPattern,
                          networkLed);
  emberConnectionManagerLeaveNetwork();
}

void emberZclIdentifyServerStartIdentifyingCallback(uint16_t identifyTimeS)
{
  emberAfCorePrintln("Identifying...");
  halMultiLedBlinkPattern(DEFAULT_NUM_IDENTIFY_BLINKS,
                          COUNTOF(networkIdentifyBlinkPattern),
                          networkIdentifyBlinkPattern,
                          networkLed);
}

void emberZclIdentifyServerStopIdentifyingCallback(void)
{
  emberAfCorePrintln("Identify complete");
  halLedBlinkLedOff(0);
}
