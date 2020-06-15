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
 * @brief CLI for the Illuminance Measurement Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "illuminance-measurement-server.h"
#include EMBER_AF_API_ILLUMINANCE

#define ILLUMINANCE_MEASUREMENT_SERVER_CALIBRATION_LUX_MAX   2000
#define ILLUMINANCE_MEASUREMENT_SERVER_CALIBRATION_LUX_MIN   50

//******************************************************************************
// CLI function to calibrate the illuminance sensor with
// its overlaid materials.
//******************************************************************************
void emAfIlluminanceMeasurementServerCalibrateCommand(void)
{
  uint8_t multiplier;
  uint32_t extLux = (uint32_t)emberUnsignedCommandArgument(0);

  if ((extLux < ILLUMINANCE_MEASUREMENT_SERVER_CALIBRATION_LUX_MIN)
      || (extLux > ILLUMINANCE_MEASUREMENT_SERVER_CALIBRATION_LUX_MAX)) {
    emberAfAppPrintln("Error in calibration, value must be between %d and %d",
                      ILLUMINANCE_MEASUREMENT_SERVER_CALIBRATION_LUX_MIN,
                      ILLUMINANCE_MEASUREMENT_SERVER_CALIBRATION_LUX_MAX);
  } else {
    multiplier = halIlluminanceCalibrate(extLux);
    halCommonSetToken(TOKEN_SI1141_MULTIPLIER, &multiplier);
    emberAfAppPrintln("Multiplier Set: %d", multiplier);
  }
}
