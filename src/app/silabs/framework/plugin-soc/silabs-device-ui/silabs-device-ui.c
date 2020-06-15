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
 * @brief Definitions for the Silabs Device UI plugin.
 *******************************************************************************
   ******************************************************************************/

#ifdef EMBER_SCRIPTED_TEST
#include "app/framework/plugin-soc/connection-manager/connection-manager-test.h"
#endif

#define FINDING_BINDING_IS_TARGET \
  EMBER_AF_PLUGIN_SILABS_DEVICE_UI_FINDING_AND_BINDING_ROLE

#include "app/framework/include/af.h"
#include "app/framework/util/attribute-storage.h"
#include "app/framework/plugin-soc/connection-manager/connection-manager.h"
#include "app/framework/plugin/manufacturing-library-cli/manufacturing-library-cli-plugin.h"
#include "app/framework/plugin-soc/silabs-device-ui/silabs-device-ui.h"
#include EMBER_AF_API_LED_BLINK
#if defined(EMBER_AF_PLUGIN_SILABS_DEVICE_UI_USE_IAS_UI)
#include "app/framework/plugin/ias-zone-server/ias-zone-server.h"
#include EMBER_AF_API_BUTTON_INTERFACE
#endif
#if FINDING_BINDING_IS_TARGET
#include "app/framework/plugin/find-and-bind-target/find-and-bind-target.h"
#else
#include "app/framework/plugin/find-and-bind-initiator/find-and-bind-initiator.h"
#endif //FINDING_BINDING_IS_TARGET

#if defined(EMBER_AF_PLUGIN_SILABS_DEVICE_UI_USE_IAS_UI) && !defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER)
#error "IAS Zone Server UI selected on a device that is not an IAS ZONE SERVER! \
  Add the IAS Zone Server plugin to the project or deselect the IAS Zone        \
  Server UI EMBER_AF_PLUGIN_SILABS_DEVICE_UI_USE_IAS_UI from the Silabs Device UI plugin"
#endif

//------------------------------------------------------------------------------
// Plugin private macro definitions

#define DEBOUNCE_TIME_MS \
  EMBER_AF_PLUGIN_SILABS_DEVICE_UI_BUTTON_DEBOUNCE_TIME_MS
#define BUTTON_NETWORK_LEAVE_TIME_MS  1000
#define IDENTIFY_ENABLE_LENGTH_S      180

#define FINDING_BINDING_ENDPOINT \
  EMBER_AF_PLUGIN_SILABS_DEVICE_UI_FINDING_AND_BINDING_ENDPOINT

// These are default values to modify the UI LED's blink pattern for network
// join and network leave.
#define LED_LOST_ON_TIME_MS          250
#define LED_LOST_OFF_TIME_MS         750
#define LED_BLINK_ON_TIME_MS         200
#define LED_PA_REJOIN_ON_TIME_MS     250
#define LED_FOUND_BLINK_OFF_TIME_MS  250
#define LED_FOUND_BLINK_ON_TIME_MS   250
#define LED_PA_REJOIN_OFF1_TIME_MS   250
#define LED_PA_REJOIN_OFF2_TIME_MS   750
#define LED_IDENTIFY_ON_TIME_MS      250
#define LED_IDENTIFY_OFF1_TIME_MS    250
#define LED_IDENTIFY_OFF2_TIME_MS    1250
#define DEFAULT_NUM_IDENTIFY_BLINKS  100
#define DEFAULT_NUM_PA_REJOIN_BLINKS 100

#if defined(EMBER_AF_PLUGIN_SILABS_DEVICE_UI_USE_IAS_UI)
  #define LED_SEARCH_BLINK_OFF_TIME_MS 250
  #define DEFAULT_NUM_SEARCH_BLINKS    2
  #define IAS_UI_NO_TAMPER_START_TIME  2000
  #define IAS_UI_TAMPER_START_TIME     4000
#else
  #define LED_SEARCH_BLINK_OFF_TIME_MS 1800
  #define DEFAULT_NUM_SEARCH_BLINKS    250
#endif

#define LED_LOST_BLINK_TOTAL_TIME_MS                \
  EMBER_AF_PLUGIN_SILABS_DEVICE_UI_NUM_LEAVE_BLINKS \
  * (LED_LOST_ON_TIME_MS + LED_LOST_OFF_TIME_MS)

typedef enum {
  NOT_INITIALIZED          = 0x00,
  BUTTON_PRESS_ON_START    = 0x01,
  NO_BUTTON_PRESS_ON_START = 0x02,
  DEVICE_INITIALIZED       = 0x03
} DeviceInitState;

//------------------------------------------------------------------------------
// Plugin events
EmberEventControl emberAfPluginSilabsDeviceUiButtonPressCountEventControl;
EmberEventControl emberAfPluginSilabsDeviceUiLedBlinkEventControl;
EmberEventControl emberAfPluginSilabsDeviceUiInitEventControl;

//------------------------------------------------------------------------------
// plugin private global variables

// State variables for controlling LED blink behavior on network join/leave
static uint16_t networkLostBlinkPattern[] =
{ LED_LOST_ON_TIME_MS, LED_LOST_OFF_TIME_MS };
static uint16_t networkSearchBlinkPattern[] =
{ LED_BLINK_ON_TIME_MS, LED_SEARCH_BLINK_OFF_TIME_MS };
static uint16_t networkFoundBlinkPattern[] =
{ LED_FOUND_BLINK_ON_TIME_MS, LED_FOUND_BLINK_OFF_TIME_MS };
static uint16_t networkIdentifyBlinkPattern[] =
{ LED_IDENTIFY_ON_TIME_MS, LED_IDENTIFY_OFF1_TIME_MS,
  LED_IDENTIFY_ON_TIME_MS, LED_IDENTIFY_OFF2_TIME_MS };
static uint16_t networkProactiveRejoinBlinkPattern[] =
{ LED_PA_REJOIN_ON_TIME_MS, LED_PA_REJOIN_OFF1_TIME_MS,
  LED_PA_REJOIN_ON_TIME_MS, LED_PA_REJOIN_OFF1_TIME_MS,
  LED_PA_REJOIN_ON_TIME_MS, LED_PA_REJOIN_OFF2_TIME_MS };

#if defined(EMBER_AF_PLUGIN_SILABS_DEVICE_UI_USE_IAS_UI)
static uint16_t iasZoneNoTamperStartPattern[] =
{ IAS_UI_NO_TAMPER_START_TIME, 10000 };
static uint16_t iasZoneTamperStartPattern[] =
{ IAS_UI_TAMPER_START_TIME, 10000 };
static DeviceInitState initState = NOT_INITIALIZED;
#endif

static uint8_t consecutiveButtonPressCount;
static bool networkSearching = false;
static bool ledLeaveBlinking = false;
//------------------------------------------------------------------------------
// plugin private function prototypes
static void enableIdentify(void);

//------------------------------------------------------------------------------
// Plugin consumed callback implementations

//******************************************************************************
// Init callback, executes sometime early in device boot chain.  This function
// will handle debug and UI LED control on startup, and schedule the reboot
// event to occur after the system finishes initializing.
//******************************************************************************
void emberAfPluginSilabsDeviceUiInitCallback(void)
{
  consecutiveButtonPressCount = 0;

  emberEventControlSetActive(emberAfPluginSilabsDeviceUiInitEventControl);
}

void emberAfPluginButtonInterfaceButton0PressedShortCallback(
  uint16_t timePressedMs)
{
#if defined(EMBER_AF_PLUGIN_SILABS_DEVICE_UI_USE_IAS_UI)
  if (initState == BUTTON_PRESS_ON_START) {
    halLedBlinkLedOff(0);
    emberAfPluginConnectionManagerFactoryReset();
    emberAfPluginConnectionManagerLeaveNetworkAndStartSearchForNewOne();
    emberAfPluginSilabsDeviceUiLedNetworkSearchingBlink();
    initState = DEVICE_INITIALIZED;
  }
#else
  // If the button was not held for longer than the debounce time, ignore the
  // press.
  if (timePressedMs < DEBOUNCE_TIME_MS) {
    return;
  }

  if (timePressedMs >= BUTTON_NETWORK_LEAVE_TIME_MS) {
    emberAfPluginConnectionManagerLeaveNetworkAndStartSearchForNewOne();
  } else {
    consecutiveButtonPressCount++;
    emberEventControlSetDelayMS(
      emberAfPluginSilabsDeviceUiButtonPressCountEventControl,
      EMBER_AF_PLUGIN_SILABS_DEVICE_UI_CONSECUTIVE_PRESS_TIMEOUT_MS);
  }
#endif
}

// The following code implements a fix for issue EMAPPFWKV2-1294 where the
// identify code fails to call the stop identifiy feedback callback after
// receiving a reset to factory defaults command during an identify operation.
void emberAfPluginBasicResetToFactoryDefaultsCallback(uint8_t endpoint)
{
  emberAfIdentifyClusterServerAttributeChangedCallback(endpoint,
                                                       ZCL_IDENTIFY_TIME_ATTRIBUTE_ID);
}

// This is called when identify is started, either from an attribute write or a
// start identify command.  This should cause the device to blink the identify
// pattern
void emberAfPluginIdentifyStartFeedbackCallback(uint8_t endpoint,
                                                uint16_t identifyTime)
{
  emberAfAppPrintln("Beginning identify blink pattern");
  emberAfPluginSilabsDeviceUiLedIdentifyBlink();
}

// This is called when identify is stopped, either from an attribute write or a
// stop identify command.  This should cause the device to stop blinking its
// identify pattern.
void emberAfPluginIdentifyStopFeedbackCallback(uint8_t endpoint)
{
  emberAfAppPrintln("Identify has finished");
  halLedBlinkLedOff(0);
}

//------------------------------------------------------------------------------
// Plugin event handlers

//------------------------------------------------------------------------------
// Handle the possible conflict between two LED blinking.
void emberAfPluginSilabsDeviceUiLedBlinkEventHandler(void)
{
  emberEventControlSetInactive(emberAfPluginSilabsDeviceUiLedBlinkEventControl);
  if (ledLeaveBlinking) {
    ledLeaveBlinking = false;
    if (networkSearching) {
      emberAfPluginSilabsDeviceUiLedNetworkSearchingBlink();
    }
  }
}

//------------------------------------------------------------------------------
// Handle the buttonPressEvent going active.  This signals that the user has
// finished a series of short button presses on the UI button, so some sort of
// network activity should take place.
void emberAfPluginSilabsDeviceUiButtonPressCountEventHandler(void)
{
  uint8_t i;
  uint8_t endpoint;
  EmberStatus status;

  emberEventControlSetInactive(
    emberAfPluginSilabsDeviceUiButtonPressCountEventControl);

  if (emberAfNetworkState() != EMBER_NO_NETWORK) {
    // If on a network:
    // 2 presses activates identify
    // 3 presses blinks network status
    // 4 presses initiates a proactive rejoin
    // 5 presses sends an identify to anyone the device is controlling
    // 6 presses starts finding and binding
    switch (consecutiveButtonPressCount) {
      case 2:
        enableIdentify();
        break;
      case 3:
        emberAfAppPrintln("Blinking user requested network status");
        emberAfPluginSilabsDeviceUiLedNetworkFoundBlink();
        break;
      case 4:
        emberAfPluginSilabsDeviceUiLedProactiveRejoinBlink();
        emberAfStartMoveCallback();
        break;
      case 5:
        for (i = 0; i < emberAfEndpointCount(); i++) {
          // When using emberAfSendCommandUnicastToBindings, the destination
          // endpoint will be automatically filled out, and the command will only
          // send if the source endpoint matches the source endpoint in the
          // binding table.
          endpoint = emberAfEndpointFromIndex(i);
          emberAfSetCommandEndpoints(endpoint, 0);
          emberAfFillCommandIdentifyClusterIdentify(IDENTIFY_ENABLE_LENGTH_S);
          status = emberAfSendCommandUnicastToBindings();

          if (status != EMBER_SUCCESS) {
            emberAfCorePrintln("Error 0x%X sending identify from endpoint %d",
                               status,
                               i);
          } else {
            emberAfCorePrintln("Sending identify from ep %d", i);
          }
        }
        break;
      case 6:
#if FINDING_BINDING_IS_TARGET
        emberAfPluginFindAndBindTargetStart(FINDING_BINDING_ENDPOINT);
#else
        emberAfPluginFindAndBindInitiatorStart(FINDING_BINDING_ENDPOINT);
#endif //FINDING_BINDING_IS_TARGET
        break;
      default:
        break;
    }
  } else {
    // If not a network, then regardless of button presses or length, we want to
    // make sure we are looking for a network.
    emberAfPluginConnectionManagerResetJoinAttempts();
    if (!emberStackIsPerformingRejoin()) {
      emberAfPluginConnectionManagerLeaveNetworkAndStartSearchForNewOne();
    }
  }

  consecutiveButtonPressCount = 0;
}

//******************************************************************************
// Init event.  To be called sometime after all system init functions have
// executed.  This event is used to check the network state on startup and
// blink the appropriate pattern.
//******************************************************************************
void emberAfPluginSilabsDeviceUiInitEventHandler(void)
{
  emberEventControlSetInactive(emberAfPluginSilabsDeviceUiInitEventControl);

#if defined(EMBER_AF_PLUGIN_SILABS_DEVICE_UI_USE_IAS_UI)
  if (initState == NOT_INITIALIZED) {
    if (!emberAfIasZoneClusterAmIEnrolled(
          EMBER_AF_PLUGIN_SECURITY_SENSOR_IAS_ZONE_ENDPOINT)) {
      // On init, If the device is not enrolled, illuminate LED for a short
      // time, then leave network and start searching for a new one.s
      emberAfAppPrintln("Device not enrolled, searching for a new network");
      initState = NO_BUTTON_PRESS_ON_START;
      halLedBlinkPattern(1,
                         COUNTOF(iasZoneNoTamperStartPattern),
                         iasZoneNoTamperStartPattern);
      emberEventControlSetDelayMS(emberAfPluginSilabsDeviceUiInitEventControl,
                                  IAS_UI_NO_TAMPER_START_TIME);
      emberAfPluginConnectionManagerFactoryReset();
      return;
    }

    // If the device is enrolled, and the tamper switch is pressed, illuminate
    // for a short time and either look for a network to join or keep working,
    // based on network state
    if (halPluginButtonInterfaceButtonPoll(BUTTON0) == BUTTON_RELEASED) {
      halLedBlinkPattern(1,
                         COUNTOF(iasZoneNoTamperStartPattern),
                         iasZoneNoTamperStartPattern);
      if (emberAfNetworkState() == EMBER_NO_NETWORK) {
        emberAfAppPrintln("Device is enroll, but no network found");
        initState = NO_BUTTON_PRESS_ON_START;
        emberEventControlSetDelayMS(
          emberAfPluginSilabsDeviceUiInitEventControl,
          IAS_UI_NO_TAMPER_START_TIME);
      }
      return;
    }

    // If the device is enrolled and no tamper pressed, do a long illumination.
    // If the button is released during that timeout, RTFD and start searching.
    initState = BUTTON_PRESS_ON_START;
    halLedBlinkPattern(1,
                       COUNTOF(iasZoneTamperStartPattern),
                       iasZoneTamperStartPattern);
    emberEventControlSetDelayMS(
      emberAfPluginSilabsDeviceUiInitEventControl,
      IAS_UI_TAMPER_START_TIME);
  } else if (initState == NO_BUTTON_PRESS_ON_START) {
    emberAfAppPrintln("UI done showing initial state, start network search");
    emberAfPluginConnectionManagerLeaveNetworkAndStartSearchForNewOne();
    emberAfPluginSilabsDeviceUiLedNetworkSearchingBlink();
    initState = DEVICE_INITIALIZED;
  } else if (initState == BUTTON_PRESS_ON_START) {
    emberAfAppPrintln("UI done showing initial state");
    initState = DEVICE_INITIALIZED;
  }
#endif
}

//------------------------------------------------------------------------------
// Plugin public API function implementations

void emberAfPluginSilabsDeviceUiLedNetworkFoundBlink(void)
{
  halLedBlinkPattern(EMBER_AF_PLUGIN_SILABS_DEVICE_UI_NUM_JOIN_BLINKS,
                     COUNTOF(networkFoundBlinkPattern),
                     networkFoundBlinkPattern);
}

void emberAfPluginSilabsDeviceUiLedNetworkLostBlink(void)
{
  halLedBlinkPattern(EMBER_AF_PLUGIN_SILABS_DEVICE_UI_NUM_LEAVE_BLINKS,
                     COUNTOF(networkLostBlinkPattern),
                     networkLostBlinkPattern);
}

void emberAfPluginSilabsDeviceUiLedNetworkSearchingBlink(void)
{
  halLedBlinkPattern(DEFAULT_NUM_SEARCH_BLINKS,
                     COUNTOF(networkSearchBlinkPattern),
                     networkSearchBlinkPattern);
}

void emberAfPluginSilabsDeviceUiLedIdentifyBlink(void)
{
  halLedBlinkPattern(DEFAULT_NUM_IDENTIFY_BLINKS,
                     COUNTOF(networkIdentifyBlinkPattern),
                     networkIdentifyBlinkPattern);
}

void emberAfPluginSilabsDeviceUiLedProactiveRejoinBlink(void)
{
  halLedBlinkPattern(DEFAULT_NUM_PA_REJOIN_BLINKS,
                     COUNTOF(networkProactiveRejoinBlinkPattern),
                     networkProactiveRejoinBlinkPattern);
}

void emberAfPluginConnectionManagerStartNetworkSearchCallback(void)
{
  if (emberAfNetworkState() == EMBER_NO_NETWORK) {
    networkSearching = true;
    if (!ledLeaveBlinking) {
      emberAfPluginSilabsDeviceUiLedNetworkSearchingBlink();
    }
  } else {
    emberAfAppPrintln("UI Error: we shouldn't reach here!");
  }
}

void emberAfPluginConnectionManagerLeaveNetworkCallback(void)
{
#if !defined(EMBER_AF_PLUGIN_SILABS_DEVICE_UI_USE_IAS_UI)
  if (!ledLeaveBlinking) {
    if ((emberAfNetworkState() != EMBER_NO_NETWORK) || (!networkSearching)) {
      emberAfPluginSilabsDeviceUiLedNetworkLostBlink();
      emberEventControlSetDelayMS(emberAfPluginSilabsDeviceUiLedBlinkEventControl,
                                  LED_LOST_BLINK_TOTAL_TIME_MS);
      ledLeaveBlinking = true;
    }
  }
#endif
}

void emberAfPluginConnectionManagerFinishedCallback(EmberStatus status)
{
  networkSearching = false;
  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    emberAfPluginSilabsDeviceUiLedNetworkFoundBlink();
  } else {
    halLedBlinkLedOff(0);
  }
}

//------------------------------------------------------------------------------
// Plugin private function implementations

// This function will cycle through all of the endpoints in the system and
// enable identify mode for each of them.
static void enableIdentify(void)
{
  uint8_t endpoint;
  uint8_t i;
  uint16_t identifyTimeS = IDENTIFY_ENABLE_LENGTH_S;

  for (i = 0; i < emberAfEndpointCount(); i++) {
    endpoint = emberAfEndpointFromIndex(i);
    if (emberAfContainsServer(endpoint, ZCL_IDENTIFY_CLUSTER_ID)) {
      emberAfWriteServerAttribute(endpoint,
                                  ZCL_IDENTIFY_CLUSTER_ID,
                                  ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                                  (uint8_t *) &identifyTimeS,
                                  ZCL_INT16U_ATTRIBUTE_TYPE);
    }
  }
}
