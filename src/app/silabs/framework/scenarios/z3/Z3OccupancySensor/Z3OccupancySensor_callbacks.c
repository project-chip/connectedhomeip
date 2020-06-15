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
#include "app/framework/plugin/reporting/reporting.h"
#include "app/framework/plugin-soc/illuminance-measurement-server/illuminance-measurement-server.h"
#include "app/framework/plugin-soc/relative-humidity-measurement-server/relative-humidity-measurement-server.h"
#include "app/framework/plugin-soc/temperature-measurement-server/temperature-measurement-server.h"
#include "app/framework/plugin-soc/connection-manager/connection-manager.h"
#include "app/framework/plugin-soc/silabs-device-ui/silabs-device-ui.h"
#include EMBER_AF_API_LED_BLINK

#define BLINK_OCCUPANCY_NOTIFICATION

#if defined(CORTEXM3_EFR32)
uint16_t occupancyDetectedBlinkPattern[] = { 5, 100 };
#endif

// This callback is generated when the occupancy measurement server receives
// a new occupancy status.  It will cause the occupancy detected pattern to
// blink when occupancy is detected.
void emberAfPluginOccupancySensorServerOccupancyStateChangedCallback(
  HalOccupancyState occupancyState)
{
  uint16_t identifyTime;

  if (occupancyState == HAL_OCCUPANCY_STATE_UNOCCUPIED) {
    return;
  }

  // we should not blink LED when the sensor is not on a network.
  if (emberAfNetworkState() == EMBER_NO_NETWORK) {
    return;
  }

  // On an occupancy, the LED should never blink if the sensor is in the middle
  // of an identify
  emberAfReadServerAttribute(1,
                             ZCL_IDENTIFY_CLUSTER_ID,
                             ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                             (uint8_t *) (&identifyTime),
                             sizeof(uint16_t));
  if (identifyTime != 0) {
    return;
  }

#if defined(CORTEXM3_EFR32)
  // If the installation jumper is in place, the LED should always blink on
  // occupancy detected (pin is low when jumper is in installation mode).
  // If the installation jumper is not in place, the LED should only blink if
  // the application has defined the BLINK_OCCUPANCY_NOTIFICATION macro.
  if (!GPIO_PinInGet(BSP_PYD1698_INSTALLATION_JP_PORT,
                     BSP_PYD1698_INSTALLATION_JP_PIN)) {
    halLedBlinkPattern(3, 2, occupancyDetectedBlinkPattern);
  } else {
#if defined(BLINK_OCCUPANCY_NOTIFICATION)
    halLedBlinkPattern(3, 2, occupancyDetectedBlinkPattern);
#endif //defined(BLINK_OCCUPANCY_NOTIFICATION)
  }
#endif //defined(CORTEXM3_EFR32)
}

// This callback will execute any time the reporting intervals are modified.
// In order to verify the occupancy sensor is polling the environment sensors
// frequently enough for the report intervals to be effective, it is necessary
// to call the SetMeasurementInterval function for each sensor any time the
// reporting intervals are changed.
EmberAfStatus emberAfPluginReportingConfiguredCallback(
  const EmberAfPluginReportingEntry *entry)
{
  if (entry->direction != EMBER_ZCL_REPORTING_DIRECTION_REPORTED) {
    return EMBER_ZCL_STATUS_SUCCESS;
  }

  if ((entry->clusterId == ZCL_TEMP_MEASUREMENT_CLUSTER_ID)
      && (entry->attributeId == ZCL_TEMP_MEASURED_VALUE_ATTRIBUTE_ID)) {
    // If setMeasurementInterval is called with a value of 0, the hardware will
    // revert to polling the hardware at the maximum rate, specified by the HAL
    // plugin.
    if (entry->endpoint == EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID) {
      emberAfCorePrintln("Temperature reporting disabled");
      emberAfPluginTemperatureMeasurementServerSetMeasurementInterval(0);
    } else {
      //Max interval is set in seconds, which is the same unit of time the
      //emberAfPluginTemperatureMeasurementServerSetMeasurementInterval expects
      // in this API.
      emberAfCorePrintln("Temperature reporting interval set: %d seconds",
                         entry->data.reported.maxInterval);
      emberAfPluginTemperatureMeasurementServerSetMeasurementInterval(
        entry->data.reported.maxInterval);
    }
  } else if ((entry->clusterId == ZCL_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_ID)
             && (entry->attributeId
                 == ZCL_RELATIVE_HUMIDITY_MEASURED_VALUE_ATTRIBUTE_ID)) {
    // If setMeasurementInterval is called with a value of 0, the hardware will
    // revert to polling the hardware at the maximum rate, specified by the HAL
    // plugin.
    if (entry->endpoint == EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID) {
      emberAfCorePrintln("Relative Humidity reporting disabled");
      emberAfPluginRelativeHumidityMeasurementServerSetMeasurementInterval(0);
    } else {
      //Max interval is set in seconds, which is the same unit of time the
      //emberAfPluginRelativeHumidityMeasurementServerSetMeasurementInterval
      //expects in this API.
      emberAfCorePrintln("Humidity reporting interval set: %d seconds",
                         entry->data.reported.maxInterval);
      emberAfPluginRelativeHumidityMeasurementServerSetMeasurementInterval(
        entry->data.reported.maxInterval);
    }
  } else if ((entry->clusterId == ZCL_ILLUM_MEASUREMENT_CLUSTER_ID)
             && (entry->attributeId == ZCL_ILLUM_MEASURED_VALUE_ATTRIBUTE_ID)) {
    // If setMeasurementInterval is called with a value of 0, the hardware will
    // revert to polling the hardware at the maximum rate, specified by the HAL
    // plugin.
    if (entry->endpoint == EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID) {
      emberAfCorePrintln("Illuminance reporting disabled");
      emberAfPluginIlluminanceMeasurementServerSetMeasurementInterval(0);
    } else {
      //Max interval is set in seconds, which is the same unit of time the
      //emberAfPluginIlluminanceMeasurementServerSetMeasurementInterval expects
      //in this API.
      emberAfCorePrintln("Illuminance reporting interval set: %d seconds",
                         entry->data.reported.maxInterval);
      emberAfPluginIlluminanceMeasurementServerSetMeasurementInterval(
        entry->data.reported.maxInterval);
    }
  }

  return EMBER_ZCL_STATUS_SUCCESS;
}

bool emberAfPluginIdleSleepOkToSleepCallback(uint32_t durationMs)
{
  // EFR32 does not yet support the virtual UART, and the physical UART will
  // drop the first character it receives when it is asleep.  As such, if the
  // installation jumper is in place, the device should never go to sleep,
  // which gives us a way to get a functional CLI for field diagnostics.
#if defined(CORTEXM3_EFR32)
  // pin will be low when device is in installation mode, hi when it is not
  if (GPIO_PinInGet(BSP_PYD1698_INSTALLATION_JP_PORT,
                    BSP_PYD1698_INSTALLATION_JP_PIN)) {
    return true;
  }
#endif
  return false;
}

void emberAfMainInitCallback(void)
{
#if defined(CORTEXM3_EFR32)
  // Set up the LED configuration to have the LED turned off on boot
  halInternalInitLed();

  // Currently, the BSP init functions will only set the LED to be a push/pull
  // output.  As such, the pin needs to be manually configured if it is
  // intended to be used in any mode other than push/pull
  GPIO_PinModeSet(BSP_LED0_PORT, BSP_LED0_PIN, gpioModeWiredAndPullUp, 1);

  // Currently, the button init functions do not allow for setting the pin mode
  // to internal pull up/down.  As such, the pin needs to be manually
  // configured if it is to be used as intended with the reference design hw.
  GPIO_PinModeSet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN, gpioModeInputPull, 1);
#endif

  halLedBlinkLedOff(0);
}
