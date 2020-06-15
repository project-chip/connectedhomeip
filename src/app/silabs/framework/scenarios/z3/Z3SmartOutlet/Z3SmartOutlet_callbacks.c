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
 * @brief
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/attribute-storage.h"
#include "app/framework/plugin/reporting/reporting.h"
#include EMBER_AF_API_LED_BLINK
#include "hal/micro/led.h"
#include EMBER_AF_API_POWER_METER_CS5463
#include "app/framework/plugin-soc/connection-manager/connection-manager.h"
#include "app/framework/plugin-soc/illuminance-measurement-server/illuminance-measurement-server.h"
#include "app/framework/plugin-soc/temperature-measurement-server/temperature-measurement-server.h"
#include "app/framework/plugin-soc/relative-humidity-measurement-server/relative-humidity-measurement-server.h"
#include "app/framework/plugin-soc/electrical-measurement-server/electrical-measurement-server.h"
#include "app/framework/plugin/find-and-bind-initiator/find-and-bind-initiator.h"

// Event control struct declarations
EmberEventControl buttonCountEventControl;
EmberEventControl powerExceptionEventControl;
EmberEventControl ledBlinkEventControl;

// defines for simulation tests
#ifdef EMBER_TEST
#define GPIO_PACLR_ADDR      1
#define GPIO_PASET_ADDR      1
#define GPIO_PAOUT_ADDR      1
#define GPIO_PACFGL_ADDR     1
#define GPIO_PBCFGL_ADDR     1
#define P_RELAY_ON_PORT      1
#define P_RELAY_OFF_PORT     1
#define P_RELAY_ON           1
#define P_RELAY_OFF          1
#define POWER_LED            1
#undef STATUS_LED
#define STATUS_LED           1
#endif

//-- GPIO access for nSEL signals
#define GPIO_PxCLR_BASE       (GPIO_PACLR_ADDR)
#define GPIO_PxSET_BASE       (GPIO_PASET_ADDR)
#define GPIO_PxOUT_BASE       (GPIO_PAOUT_ADDR)
#define GPIO_PxCFG_BASE       (GPIO_PACFGL_ADDR)
// Each port is offset from the previous port by the same amount
#define GPIO_Px_OFFSET        (GPIO_PBCFGL_ADDR - GPIO_PACFGL_ADDR)

#define LED_OVERHEAT_ON_TIME_MS         250
#define LED_OVERHEAT_OFF_TIME_MS        250
#define LED_OVERCURRENT_ON_TIME_MS      250
#define LED_OVERCURRENT_OFF_TIME_MS     750
#define LED_POWERON_ON_TIME_MS         1000
#define LED_POWEROFF_ON_TIME_MS         250
#define LED_POWERON_OFF_TIME_MS         250
#define LED_POWEROFF_OFF_TIME_MS        250

#define LED_LOST_ON_TIME_MS          250
#define LED_LOST_OFF_TIME_MS         750
#define LED_BLINK_ON_TIME_MS         200
#define LED_PA_REJOIN_ON_TIME_MS     250
#define LED_SEARCH_BLINK_OFF_TIME_MS 1800
#define LED_FOUND_BLINK_OFF_TIME_MS  250
#define LED_FOUND_BLINK_ON_TIME_MS   250
#define LED_PA_REJOIN_OFF1_TIME_MS   250
#define LED_PA_REJOIN_OFF2_TIME_MS   750
#define LED_IDENTIFY_ON_TIME_MS      250
#define LED_IDENTIFY_OFF1_TIME_MS    250
#define LED_IDENTIFY_OFF2_TIME_MS    1250
#define DEFAULT_NUM_SEARCH_BLINKS    250
#define DEFAULT_NUM_IDENTIFY_BLINKS  100

#define DEFAULT_NUM_OVERHEAT_BLINKS \
  ((2 * 60) * (1000 / (LED_OVERHEAT_ON_TIME_MS + LED_OVERHEAT_OFF_TIME_MS)))
#define DEFAULT_NUM_OVERCURRENT_BLINKS \
  ((2 * 60) * (1000 / (LED_OVERCURRENT_ON_TIME_MS + LED_OVERCURRENT_OFF_TIME_MS)))
#define DEFAULT_NUM_POWERON_BLINKS       1
#define DEFAULT_NUM_POWEROFF_BLINKS      1

#define DEFAULT_NUM_JOIN_BLINKS      6
#define DEFAULT_NUM_LEAVE_BLINKS     3
#define DEFAULT_NUM_ON_NETWORK_BLINKS \
  ((3) * (1000 / (LED_FOUND_BLINK_ON_TIME_MS + LED_FOUND_BLINK_OFF_TIME_MS)))

#define LED_LOST_BLINK_TOTAL_TIME_MS \
  DEFAULT_NUM_LEAVE_BLINKS * (LED_LOST_ON_TIME_MS + LED_LOST_OFF_TIME_MS)

#define POWER_LED_OVERHEAT           1 << 0
#define POWER_LED_OVERCURRENT        1 << 1
#define POWER_LED_ON                 1 << 2
#define POWER_LED_OFF                1 << 3

#define POWER_BUTTON_MAX_MS          2000
#define POWER_BUTTON_MIN_MS           500
#define NETWORK_LEAVE_MIN_MS         3000
#define BUTTON_COUNT_TIMEOUT_MS      2000
#define BUTTON_PRESS_TIME_MIN_MS      100

#define RELAY_SWTICH_STATE_KEEP_TIME_MS 20

#define DEFAULT_IDENTIFY_TIME_S        180
#define POWER_STATE_OFF                  0
#define POWER_STATE_ON                   1
#define POWER_STATE_TOGGLE               2

#define POWER_EXCEPTION_TIMEOUT_MS                 120000
#define POWER_EXCEPTION_BIT_CS5463_OVERHEAT             0
#define POWER_EXCEPTION_BIT_AMBIENT_OVERHEAT            1
#define POWER_EXCEPTION_OVER_HEAT_MASK               0x03

#define POWER_EXCEPTION_BIT_CS5463_OVERCURRENT          2
#define POWER_EXCEPTION_MASK                         0x07

#define POWER_EXCEPTION_BIT_POWERSHUTDOWN               3

#define EZ_MODE_ENDPOINT                 1
#define EZ_MODE_NUMBER_OF_CLUSTERS       5

#define GPIO_RELAY_ON           P_RELAY_ON
#define GPIO_RELAY_OFF         P_RELAY_OFF

static uint8_t powerExceptionMask;
static uint8_t buttonCountShortPressed;
static bool networkSearching = false;
static bool ledLeaveBlinking = false;

static uint16_t powerOnBlinkPattern[] = {
  LED_POWERON_ON_TIME_MS, LED_POWERON_OFF_TIME_MS
};
static uint16_t powerOffBlinkPattern[] = {
  LED_POWEROFF_ON_TIME_MS, LED_POWEROFF_OFF_TIME_MS
};
static uint16_t overHeatBlinkPattern[] = {
  LED_OVERHEAT_ON_TIME_MS, LED_OVERHEAT_OFF_TIME_MS
};
static uint16_t overCurrentBlinkPattern[] = {
  LED_OVERCURRENT_ON_TIME_MS, LED_OVERCURRENT_OFF_TIME_MS
};
static uint16_t onNetworkBlinkPattern[] = {
  LED_FOUND_BLINK_ON_TIME_MS, LED_FOUND_BLINK_OFF_TIME_MS
};
static uint16_t networkLostBlinkPattern[] =
{ LED_LOST_ON_TIME_MS, LED_LOST_OFF_TIME_MS };
static uint16_t networkSearchBlinkPattern[] =
{ LED_BLINK_ON_TIME_MS, LED_SEARCH_BLINK_OFF_TIME_MS };
static uint16_t networkIdentifyBlinkPattern[] =
{ LED_IDENTIFY_ON_TIME_MS, LED_IDENTIFY_OFF1_TIME_MS,
  LED_IDENTIFY_ON_TIME_MS, LED_IDENTIFY_OFF2_TIME_MS };

// Event function forward declarations
void buttonCountEventHandler(void);

#if ((!(defined (EMBER_TEST))) && (!(defined(CORTEXM3_EFR32))))
static void internalGpioClr(uint8_t gpio)
{
  *((volatile uint32_t *)(GPIO_PxCLR_BASE + (GPIO_Px_OFFSET * (gpio / 8)))) \
    = BIT(gpio & 7);
}

static void internalGpioSet(uint8_t gpio)
{
  *((volatile uint32_t *)(GPIO_PxSET_BASE + (GPIO_Px_OFFSET * (gpio / 8)))) \
    = BIT(gpio & 7);
}
#endif

static void relayOnOff(uint8_t status)
{
#ifndef EMBER_TEST
#if defined(CORTEXM3_EFR32)
  if (status) {
    if (!(READBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN))) {
      GPIO_PinOutClear(P_RELAY_ON_PORT, P_RELAY_ON_PIN);
      GPIO_PinOutSet(P_RELAY_OFF_PORT, P_RELAY_OFF_PIN);
      halCommonDelayMilliseconds(RELAY_SWTICH_STATE_KEEP_TIME_MS);
      GPIO_PinOutClear(P_RELAY_ON_PORT, P_RELAY_ON_PIN);
      GPIO_PinOutClear(P_RELAY_OFF_PORT, P_RELAY_OFF_PIN);
    }
  } else {
    GPIO_PinOutClear(P_RELAY_OFF_PORT, P_RELAY_OFF_PIN);
    GPIO_PinOutSet(P_RELAY_ON_PORT, P_RELAY_ON_PIN);
    halCommonDelayMilliseconds(RELAY_SWTICH_STATE_KEEP_TIME_MS);
    GPIO_PinOutClear(P_RELAY_ON_PORT, P_RELAY_ON_PIN);
    GPIO_PinOutClear(P_RELAY_OFF_PORT, P_RELAY_OFF_PIN);
  }
#else
  if (status) {
    if (!(READBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN))) {
      internalGpioClr(GPIO_RELAY_ON);
      internalGpioSet(GPIO_RELAY_OFF);
      halCommonDelayMilliseconds(RELAY_SWTICH_STATE_KEEP_TIME_MS);
      internalGpioClr(GPIO_RELAY_ON);
      internalGpioClr(GPIO_RELAY_OFF);
    }
  } else {
    internalGpioClr(GPIO_RELAY_OFF);
    internalGpioSet(GPIO_RELAY_ON);
    halCommonDelayMilliseconds(RELAY_SWTICH_STATE_KEEP_TIME_MS);
    internalGpioClr(GPIO_RELAY_ON);
    internalGpioClr(GPIO_RELAY_OFF);
  }
#endif // CORTEXM3_EFR32
#endif // EMBER_TEST
  return;
}

static void powerOnOff(uint8_t status, bool updateOnOffAttribute)
{
  static uint8_t powerState;
  uint8_t endpointIndex, endpoint;

  if (status == POWER_STATE_TOGGLE) {
    powerState ^= 1;
  } else {
    if (powerState == status) {
      return;
    } else {
      powerState = status;
    }
  }
  relayOnOff(powerState);
  if (!(READBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN))) {
    if (powerState) {
      halMultiLedBlinkPattern(DEFAULT_NUM_POWERON_BLINKS,
                              2,
                              powerOnBlinkPattern,
                              POWER_LED);
    } else {
      halMultiLedBlinkPattern(DEFAULT_NUM_POWEROFF_BLINKS,
                              2,
                              powerOffBlinkPattern,
                              POWER_LED);
    }
  }

  if (updateOnOffAttribute) {
    // Cycle through all endpoints, check to see if the endpoint has a
    // on off server, and if so update that endpoint
    for (endpointIndex = 0;
         endpointIndex < emberAfEndpointCount();
         endpointIndex++) {
      endpoint = emberAfEndpointFromIndex(endpointIndex);
      if (emberAfContainsServer(endpoint, ZCL_ON_OFF_CLUSTER_ID)) {
        // write current on/off value
        emberAfWriteAttribute(endpoint,
                              ZCL_ON_OFF_CLUSTER_ID,
                              ZCL_ON_OFF_ATTRIBUTE_ID,
                              CLUSTER_MASK_SERVER,
                              (uint8_t *)&powerState,
                              ZCL_BOOLEAN_ATTRIBUTE_TYPE);
      }
    }
  }
}

//------------------------------------------------------------------------------
// This function returns the current value in identify time attribute
static uint16_t getIdentifyTime(void)
{
  uint8_t endpoint, status;
  uint16_t identifyTimeS;

  endpoint = emberAfEndpointFromIndex(0);
  if (emberAfContainsServer(endpoint, ZCL_IDENTIFY_CLUSTER_ID)) {
    status = emberAfReadAttribute(endpoint,
                                  ZCL_IDENTIFY_CLUSTER_ID,
                                  ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                                  CLUSTER_MASK_SERVER,
                                  (uint8_t *)&identifyTimeS,
                                  sizeof(identifyTimeS),
                                  NULL); // data type
    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      emberAfAppPrintln("ERR: reading identify time %x", status);
      return 0;
    } else {
      return identifyTimeS;
    }
  } else {
    return 0;
  }
}

//------------------------------------------------------------------------------
// This function enable identify mode for endpoint 0 as multiple endpoint is
// not supported in current implementation
static void toggleIdentify(void)
{
  uint8_t endpoint;
  uint16_t identifyTimeS;

  identifyTimeS = getIdentifyTime();
  if (identifyTimeS == 0) {
    identifyTimeS = DEFAULT_IDENTIFY_TIME_S;
  }
  endpoint = emberAfEndpointFromIndex(0);
  emberAfWriteAttribute(endpoint,
                        ZCL_IDENTIFY_CLUSTER_ID,
                        ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        (uint8_t *) &identifyTimeS,
                        ZCL_INT16U_ATTRIBUTE_TYPE);
}

static void powerExceptionHelper(void)
{
  if (READBITS(powerExceptionMask, POWER_EXCEPTION_OVER_HEAT_MASK)) {
    halMultiLedBlinkPattern(DEFAULT_NUM_OVERHEAT_BLINKS,
                            2,
                            overHeatBlinkPattern,
                            POWER_LED);
  } else if (READBIT(powerExceptionMask,
                     POWER_EXCEPTION_BIT_CS5463_OVERCURRENT)) {
    halMultiLedBlinkPattern(DEFAULT_NUM_OVERCURRENT_BLINKS,
                            2,
                            overCurrentBlinkPattern,
                            POWER_LED);
  }
  if (READBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN)) {
    powerOnOff(POWER_STATE_OFF, true);
  }
  emberEventControlSetDelayMS(powerExceptionEventControl,
                              POWER_EXCEPTION_TIMEOUT_MS);
}

void powerExceptionEventHandler(void)
{
  emberEventControlSetInactive(powerExceptionEventControl);
  if (READBITS(powerExceptionMask, POWER_EXCEPTION_OVER_HEAT_MASK)) {
    halMultiLedBlinkPattern(DEFAULT_NUM_OVERHEAT_BLINKS,
                            2,
                            overHeatBlinkPattern,
                            POWER_LED);
    emberEventControlSetDelayMS(powerExceptionEventControl,
                                POWER_EXCEPTION_TIMEOUT_MS);
  } else if (READBIT(powerExceptionMask,
                     POWER_EXCEPTION_BIT_CS5463_OVERCURRENT)) {
    halMultiLedBlinkPattern(DEFAULT_NUM_OVERCURRENT_BLINKS,
                            2,
                            overCurrentBlinkPattern,
                            POWER_LED);
    emberEventControlSetDelayMS(powerExceptionEventControl,
                                POWER_EXCEPTION_TIMEOUT_MS);
  } else {
    CLEARBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN);
  }
}

void buttonCountEventHandler(void)
{
  uint8_t i;
  uint8_t endpoint;
  EmberStatus status;

  emberEventControlSetInactive(buttonCountEventControl);
  if (emberAfNetworkState() != EMBER_NO_NETWORK) {
    switch (buttonCountShortPressed) {
      case 1:
        //Do nothing;
        break;
      case 2:
        //Identify, if we are already in identify mode, we abort it.
        toggleIdentify();
        break;
      case 3:
        //Network status
        if (getIdentifyTime() > 0) {
          emberAfAppPrintln("no network status blinking in Identify mode");
        } else {
          emberAfAppPrintln("Blinking user requested network status");
          halMultiLedBlinkPattern(DEFAULT_NUM_ON_NETWORK_BLINKS,
                                  2,
                                  onNetworkBlinkPattern,
                                  STATUS_LED);
        }
        break;
      case 4:
        //Do nothing
        break;
      case 5:
        for (i = 0; i < emberAfEndpointCount(); i++) {
          // When using emberAfSendCommandUnicastToBindings, the destination
          // endpoint will be automatically filled out, and the command will only
          // send if the source endpoint matches the source endpoint in the
          // binding table.
          endpoint = emberAfEndpointFromIndex(i);
          emberAfSetCommandEndpoints(endpoint, 0);
          emberAfFillCommandIdentifyClusterIdentify(DEFAULT_IDENTIFY_TIME_S);
          status = emberAfSendCommandUnicastToBindings();

          if (status != EMBER_SUCCESS) {
            emberAfAppPrintln("Error 0x%X sending identify from endpoint %d",
                              status,
                              i);
          } else {
            emberAfAppPrintln("Sending identify from ep %d", i);
          }
        }
        break;
      case 6:
        endpoint = emberAfEndpointFromIndex(0);
        emberAfPluginFindAndBindInitiatorStart(endpoint);
        break;
      default:
        break;
    }
  } else {
    // If not a network, then regardless of button presses or length, we want to
    // make sure we are looking for a network.
    emberAfPluginConnectionManagerResetJoinAttempts();
    emberAfPluginConnectionManagerLeaveNetworkAndStartSearchForNewOne();
  }

  buttonCountShortPressed = 0;
}

//------------------------------------------------------------------------------
// Handle the possible conflict between two LED blinking.
void ledBlinkEventHandler(void)
{
  emberEventControlSetInactive(ledBlinkEventControl);
  if (ledLeaveBlinking) {
    ledLeaveBlinking = false;
    if (networkSearching) {
      halMultiLedBlinkPattern(DEFAULT_NUM_SEARCH_BLINKS,
                              2,
                              networkSearchBlinkPattern,
                              STATUS_LED);
    }
  }
}

// This callback will execute any time the reporting intervals are modified.
// In order to verify the smart outlet is polling the sensors
// frequently enough, it is necessary to call the respetive functions to change
// actual measurement rate any time the reporting intervals are changed.
EmberAfStatus emberAfPluginReportingConfiguredCallback(
  const EmberAfPluginReportingEntry *entry)
{
  emberAfAppPrintln("Reporting Callback cluster : 0x%4x",
                    entry->clusterId);
  if ((entry->clusterId == ZCL_TEMP_MEASUREMENT_CLUSTER_ID)
      && (entry->attributeId == ZCL_TEMP_MEASURED_VALUE_ATTRIBUTE_ID)
      && (entry->direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
      && (entry->endpoint != EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID)) {
    //Max interval is set in seconds, which is the same unit of time the
    //emberAfPluginTemperatureMeasurementServerSetMeasurementInterval
    //expects in this API.
    emberAfAppPrintln("Temperature reporting interval set: %d",
                      entry->data.reported.maxInterval);
    emberAfPluginTemperatureMeasurementServerSetMeasurementInterval(
      entry->data.reported.maxInterval);
  } else if ((entry->clusterId == ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID)
             && (entry->attributeId
                 == ZCL_RELATIVE_HUMIDITY_MEASURED_VALUE_ATTRIBUTE_ID)
             && (entry->direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
             && (entry->endpoint != EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID)) {
    //Max interval is set in seconds, which is the same unit of time the
    //emberAfPluginRelativeHumidityMeasurementServerSetMeasurementInterval
    //expects in this API.
    emberAfAppPrintln("Humidity reporting interval set: %d",
                      entry->data.reported.maxInterval);
    emberAfPluginRelativeHumidityMeasurementServerSetMeasurementInterval(
      entry->data.reported.maxInterval);
  } else if ((entry->clusterId == ZCL_ILLUM_MEASUREMENT_CLUSTER_ID)
             && (entry->attributeId == ZCL_ILLUM_MEASURED_VALUE_ATTRIBUTE_ID)
             && (entry->direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
             && (entry->endpoint != EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID)) {
    //Max interval is set in seconds, which is the same unit of time the
    //emberAfPluginIlluminanceMeasurementServerSetMeasurementInterval expects
    //in this API.
    emberAfAppPrintln("Illuminance reporting interval set: %d",
                      entry->data.reported.maxInterval);
    emberAfPluginIlluminanceMeasurementServerSetMeasurementInterval(
      entry->data.reported.maxInterval);
  } else if ((entry->clusterId == ZCL_ELECTRICAL_MEASUREMENT_CLUSTER_ID)
             && (entry->direction == EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
             && (entry->endpoint != EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID)) {
    //Max interval is set in seconds, which is the same unit of time the
    //emberAfPluginElectricalMeasurementServerSetMeasurementInterval expects
    //in this API.
    emberAfAppPrintln("Electrical Measurement reporting interval set: %d",
                      entry->data.reported.maxInterval);
    emberAfPluginElectricalMeasurementServerSetMeasurementInterval(
      entry->data.reported.maxInterval);
  }
  return EMBER_ZCL_STATUS_SUCCESS;
}

/** @brief On/off Cluster Server Post Init
 *
 * Following resolution of the On/Off state at startup for this endpoint, perform any
 * additional initialization needed; e.g., synchronize hardware state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginOnOffClusterServerPostInitCallback(uint8_t endpoint)
{
  // At startup, trigger a read of the attribute and possibly a toggle of the
  // LED to make sure they are always in sync.
  emberAfOnOffClusterServerAttributeChangedCallback(endpoint,
                                                    ZCL_ON_OFF_ATTRIBUTE_ID);
}

/** @brief Identify Start Feedback
 *
 * This function is called by the Identify plugin when identification begins.
 * It informs the Identify Feedback plugin that it should begin providing its
 * implemented feedback functionality (e.g. LED blinking, buzzer sounding,
 * etc.) until the Identify plugin tells it to stop. The identify time is
 * purely a matter of informational convenience; this plugin does not need to
 * know how long it will identify (the Identify plugin will perform the
 * necessary timekeeping.)
 *
 * @param endpoint The identifying endpoint Ver.: always
 * @param identifyTime The identify time Ver.: always
 */
void emberAfPluginIdentifyStartFeedbackCallback(uint8_t endpoint,
                                                uint16_t identifyTime)
{
  emberAfAppPrintln("Beginning identify blink pattern");
  halMultiLedBlinkPattern(DEFAULT_NUM_IDENTIFY_BLINKS,
                          4,
                          networkIdentifyBlinkPattern,
                          STATUS_LED);
}
/** @brief Identify Stop Feedback
 *
 * This function is called by the Identify plugin when identification is
 * finished. It tells the Identify Feedback plugin to stop providing its
 * implemented feedback functionality.
 *
 * @param endpoint The identifying endpoint Ver.: always
 */
void emberAfPluginIdentifyStopFeedbackCallback(uint8_t endpoint)
{
  emberAfAppPrintln("Identify has finished");
  halMultiLedBlinkLedOff(0, STATUS_LED);
}

/** @brief Connection Manager callback when finished forming or joining process
 *
 * This callback is fired when the Connection Manager plugin is finished with
 * the forming or joining process. The result of the operation will be returned
 * in the status parameter.
 *
 * @param status   Ver.: always
 */
void emberAfPluginConnectionManagerFinishedCallback(EmberStatus status)
{
  networkSearching = false;
  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    halMultiLedBlinkPattern(DEFAULT_NUM_JOIN_BLINKS,
                            2,
                            onNetworkBlinkPattern,
                            STATUS_LED);
  } else {
    halMultiLedBlinkLedOff(0, STATUS_LED);
  }
}

/** @brief Connection Manager callbacks when it starts searching a network.
 *
 * This function is called by the Connection Manager Plugin when it starts
 * to search a new network.  It is normally used to trigger a UI event to
 * notify the user that the device is currently searching for a network.
 */
void emberAfPluginConnectionManagerStartNetworkSearchCallback(void)
{
  if (emberAfNetworkState() == EMBER_NO_NETWORK) {
    networkSearching = true;
    if (!ledLeaveBlinking) {
      halMultiLedBlinkPattern(DEFAULT_NUM_SEARCH_BLINKS,
                              2,
                              networkSearchBlinkPattern,
                              STATUS_LED);
    }
  } else {
    emberAfAppPrintln("Error: we shouldn't reach here!");
  }
}

/** @brief Connection Manager callback when it leaves the network.
 *
 * This function is called by the Connection Manager Plugin when the device
 * leaves the network.  It is normally used to trigger a UI event to notify
 * the user of a network leave.
 */
void emberAfPluginConnectionManagerLeaveNetworkCallback(void)
{
  if (!ledLeaveBlinking) {
    if ((emberAfNetworkState() != EMBER_NO_NETWORK) || (!networkSearching)) {
      halMultiLedBlinkPattern(DEFAULT_NUM_LEAVE_BLINKS,
                              2,
                              networkLostBlinkPattern,
                              STATUS_LED);
      emberEventControlSetDelayMS(ledBlinkEventControl,
                                  LED_LOST_BLINK_TOTAL_TIME_MS);
      ledLeaveBlinking = true;
    }
  }
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be
 * notified of changes to the stack status and take appropriate action.  The
 * application should return true if the status has been handled and should
 * not be handled by the application framework.
 *
 * @param status   Ver.: always
 */
bool emberAfStackStatusCallback(EmberStatus status)
{
  uint8_t endpointCount, endpoint, currentValue;

  // If we go up or down, let the user know, although the down case shouldn't
  // happen.
  if (status == EMBER_NETWORK_UP) {
    endpointCount = emberAfEndpointCount();
    if (endpointCount == 1) {
      endpoint = emberAfEndpointFromIndex(0);
      if (emberAfContainsServer(endpoint, ZCL_ON_OFF_CLUSTER_ID)) {
        // read current on/off value
        status = emberAfReadAttribute(endpoint,
                                      ZCL_ON_OFF_CLUSTER_ID,
                                      ZCL_ON_OFF_ATTRIBUTE_ID,
                                      CLUSTER_MASK_SERVER,
                                      (uint8_t *)&currentValue,
                                      sizeof(currentValue),
                                      NULL); // data type
        if (status != EMBER_ZCL_STATUS_SUCCESS) {
          emberAfAppPrintln("ERR: reading on/off %x", status);
          return status;
        } else {
          powerOnOff(currentValue, false);
        }
      }
    } else if (endpointCount == 0) {
      emberAfAppPrintln("no endpoint found!");
    } else {
      emberAfAppPrintln("multiple endpoints not supported!");
    }
  } else if (status == EMBER_NETWORK_DOWN
             && emberAfNetworkState() == EMBER_NO_NETWORK) {
    powerOnOff(POWER_STATE_OFF, true);
  }
  return false;
}

/** @brief On/off Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
void emberAfOnOffClusterServerAttributeChangedCallback(uint8_t endpoint,
                                                       EmberAfAttributeId attributeId)
{
  uint8_t onOff;

  // When the on/off attribute changes, set the power relay appropriately.
  // If an error occurs, ignore it because there's really nothing we can do.
  if (attributeId == ZCL_ON_OFF_ATTRIBUTE_ID) {
    if (emberAfReadServerAttribute(endpoint,
                                   ZCL_ON_OFF_CLUSTER_ID,
                                   ZCL_ON_OFF_ATTRIBUTE_ID,
                                   (uint8_t *)&onOff,
                                   sizeof(onOff))
        == EMBER_ZCL_STATUS_SUCCESS) {
      if ((READBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN))
          && (onOff != POWER_STATE_OFF)) {
        onOff = POWER_STATE_OFF;
        emberAfWriteAttribute(endpoint,
                              ZCL_ON_OFF_CLUSTER_ID,
                              ZCL_ON_OFF_ATTRIBUTE_ID,
                              CLUSTER_MASK_SERVER,
                              (uint8_t *)&onOff,
                              ZCL_BOOLEAN_ATTRIBUTE_TYPE);
      } else {
        powerOnOff(onOff, false);
      }
    }
  }
}

/** @brief Main Init
 *
 * This function is called from the application's main function. It gives the
 * application a chance to do any initialization required at system startup.
 * Any code that you would normally put into the top of the application's
 * main() routine should be put into this function. This is called before the
 * clusters, plugins, and the network are initialized so some functionality is
 * not yet available.
        Note: No callback in the Application Framework is
 * associated with resource cleanup. If you are implementing your application
 * on a Unix host where resource cleanup is a consideration, we expect that you
 * will use the standard Posix system calls, including the use of atexit() and
 * handlers for signals such as SIGTERM, SIGINT, SIGCHLD, SIGPIPE and so on. If
 * you use the signal() function to register your signal handler, please mind
 * the returned value which may be an Application Framework function. If the
 * return value is non-null, please make sure that you call the returned
 * function from your handler to avoid negating the resource cleanup of the
 * Application Framework itself.
 *
 */
void emberAfMainInitCallback(void)
{
#if (!(defined (EMBER_TEST)) && (defined(CORTEXM3_EFR32)))
  //config gpio ports for replay control
  GPIO_PinModeSet(P_RELAY_ON_PORT, P_RELAY_ON_PIN, gpioModePushPull, 0);
  GPIO_PinModeSet(P_RELAY_OFF_PORT, P_RELAY_OFF_PIN, gpioModePushPull, 0);
#endif
  halMultiLedBlinkSetActivityLeds(POWER_LED);
  halMultiLedBlinkSetActivityLeds(STATUS_LED);
  relayOnOff(POWER_STATE_OFF);
  buttonCountShortPressed = 0;
  networkSearching = false;
  powerExceptionMask = 0;
  halMultiLedBlinkPattern(DEFAULT_NUM_POWEROFF_BLINKS,
                          2,
                          powerOffBlinkPattern,
                          POWER_LED);
}

/** @brief Button0 Pressed Long
 *
 * This function returns the number of times a button was short pressed.
 *
 * @param timePressedMs Amount of time button 0 was pressed.  Ver.: always
 * @param pressedAtReset Was the button pressed at startup.  Ver.: always
 */
void emberAfPluginButtonInterfaceButton0PressedLongCallback(
  uint16_t timePressedMs,
  bool pressedAtReset)
{
  if ((timePressedMs < POWER_BUTTON_MAX_MS)
      && (!(READBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN)))) {
    powerOnOff(POWER_STATE_TOGGLE, true);
    emberAfAppPrintln("Toggle Power!\r\n");
  } else if ( timePressedMs > NETWORK_LEAVE_MIN_MS ) { //leave network;
    emberAfAppPrintln("Leave and rescan!\r\n");
    emberAfPluginConnectionManagerResetJoinAttempts();
    emberAfPluginConnectionManagerLeaveNetworkAndStartSearchForNewOne();
  }
}

/** @brief Button0 Pressed Short
 *
 * This function returns the number of times a button was short pressed.
 *
 * @param timePressedMs Time (in ms) button 0 was pressed  Ver.: always
 */
void emberAfPluginButtonInterfaceButton0PressedShortCallback(
  uint16_t timePressedMs)
{
  if (timePressedMs > BUTTON_PRESS_TIME_MIN_MS) {
    buttonCountShortPressed++;
    emberEventControlSetDelayMS(buttonCountEventControl,
                                BUTTON_COUNT_TIMEOUT_MS);
  }
}

/** @brief Over Current Callback
 *
 * This function is called upon the status change of over current condition.
 *
 * @param status OVER_CURRENT_TO_NORMAL: changed from over current to normal;
 *               NORMAL_TO_OVER_CURRENT: over current occured.
 *
 */
void halPowerMeterOverCurrentStatusChangeCallback(
  uint8_t status)
{
  if (status == CS5463_NORMAL_TO_OVER_CURRENT) { // over current happened
    emberAfAppPrintln("Over Current!!\r\n");
    SETBIT(powerExceptionMask, POWER_EXCEPTION_BIT_CS5463_OVERCURRENT);
    SETBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN);
    powerExceptionHelper();
  } else { // over current gone
    emberAfAppPrintln("Over Current Lifted!!\r\n");
    powerExceptionHelper();
    CLEARBIT(powerExceptionMask, POWER_EXCEPTION_BIT_CS5463_OVERCURRENT);
  }
}

/** @brief Over Heat Callback
 *
 * This function is called upon the status change of over heat condition.
 *
 * @param status  OVER_HEAT_TO_NORMAL: changed from over heat to normal;
 *                NORMAL_TO_OVER_CURRENT: over heat occured.
 *
 */
void halPowerMeterOverHeatStatusChangeCallback(
  uint8_t status)
{
  if (status == CS5463_NORMAL_TO_OVER_HEAT) { // over heat happened
    emberAfAppPrintln("CS5463 Over Heat!!\r\n");
    SETBIT(powerExceptionMask, POWER_EXCEPTION_BIT_CS5463_OVERHEAT);
    SETBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN);
    powerExceptionHelper();
  } else {
    emberAfAppPrintln("CS5463 Over Heat Lifted!!\r\n");
    powerExceptionHelper();
    CLEARBIT(powerExceptionMask, POWER_EXCEPTION_BIT_CS5463_OVERHEAT);
  }
}

/** @brief Over temperature state changed
 *
 * This callback is generated when the temperature rises over the assert
 * thresholds (warning and critical) or drops below the deassert thresholds
 * (warning and critical)
 *
 * @param previousState, the state prior to the state change
 *
 * @param newState, the new state of the state change
 *
 *  EMBER_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_NORMAL : normal state
 *  EMBER_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_WARNING : warning state
 *  EMBER_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_CRITICAL : critical state
 *
 *  Ver.: always
 */
void emberAfPluginTemperatureMeasurementServerOverTemperatureCallback(
  uint8_t previousState,
  uint8_t newState)
{
  if (newState == EMBER_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_WARNING) {
    emberAfAppPrintln("WARNING: Ambient temperature in warning range!!\r\n");
    SETBIT(powerExceptionMask, POWER_EXCEPTION_BIT_AMBIENT_OVERHEAT);
    powerExceptionHelper();
  } else if (newState == EMBER_TEMPERATURE_MEASUREMENT_SERVER_OVER_TEMPERATURE_CRITICAL) {
    emberAfAppPrintln("WARNING: Ambient temperature in critical range!!\r\n");
    SETBIT(powerExceptionMask, POWER_EXCEPTION_BIT_AMBIENT_OVERHEAT);
    SETBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN);
    powerExceptionHelper();
  } else {
    emberAfAppPrintln("Ambient temperature changed from warning to normal!!\r\n");
    powerExceptionHelper();
    CLEARBIT(powerExceptionMask, POWER_EXCEPTION_BIT_AMBIENT_OVERHEAT);
  }
}
