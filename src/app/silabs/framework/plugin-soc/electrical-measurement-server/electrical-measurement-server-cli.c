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
 * @brief CLI for the Electrical Measurement Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include EMBER_AF_API_POWER_METER

//HalPowerMeterCalibrationData calibrationData;

//******************************************************************************
// CLI function to read Active Power
//******************************************************************************
void emAfElectricalMeasurementServerReadActivePower(void)
{
  int32_t activePower;

  activePower = halGetActivePowerMilliW();
  emberAfAppPrintln("Active Power = %d mW", activePower);
}

//******************************************************************************
// CLI function to read Power Factor
//******************************************************************************
void emAfElectricalMeasurementServerReadPowerFactor(void)
{
  int8_t powerFactor;

  powerFactor = halGetPowerFactor();
  emberAfAppPrintln("Power Factor= %d", powerFactor);
}

//******************************************************************************
// CLI function to read RMS Voltage
//******************************************************************************
void emAfElectricalMeasurementServerReadVrms(void)
{
  uint32_t rmsVoltage;

  rmsVoltage = halGetVrmsMilliV();
  emberAfAppPrintln("RMS Voltage = %d mV", rmsVoltage);
}

//******************************************************************************
// CLI function to read RMS Current
//******************************************************************************
void emAfElectricalMeasurementServerReadIrms(void)
{
  uint32_t rmsCurrent;

  rmsCurrent = halGetCrmsMilliA();
  emberAfAppPrintln("RMS Current = %d mA", rmsCurrent);
}

//******************************************************************************
// CLI command to perform calibration of power meter sensor
//******************************************************************************
void emAfElectricalMeasurementServerCalibrate(void)
{
  int8_t temperatureDegree = emberSignedCommandArgument(0);

  emberAfAppPrintln("Temperature = %d DegreeC, Calibrating...",
                    temperatureDegree);
//  halPowerMeterCalibrateAll((int16_t)temperatureDegree * 100, &calibrationData);

//  emberAfAppPrintln("Done!");
//  emberAfAppPrintln("Calibrated DC Voltage offset = 0x%x",
//                    calibrationData.dcVoltageOffset);
//  emberAfAppPrintln("Calibrated DC Current offset = 0x%x",
//                    calibrationData.dcCurrentOffset);
//  emberAfAppPrintln("Calibrated AC Voltage offset = 0x%x",
//                    calibrationData.acVoltageOffset);
//  emberAfAppPrintln("Calibrated AC Current offset = 0x%x",
//                    calibrationData.acCurrentOffset);
//  emberAfAppPrintln("Calibrated temperature offset = 0x%x",
//                    calibrationData.temperatureOffset);
}
