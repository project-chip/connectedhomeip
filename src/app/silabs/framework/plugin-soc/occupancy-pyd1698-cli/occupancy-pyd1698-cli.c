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
 * @brief Definitions for the Occupancy PYD1698 CLI plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include EMBER_AF_API_OCCUPANCY
#include EMBER_AF_API_OCCUPANCY_PYD1698

typedef enum {
  CONFIG_PARAMETER_THRESHOLD      = 0x00,
  CONFIG_PARAMETER_BLIND_TIME     = 0x01,
  CONFIG_PARAMETER_PULSE_COUNTER  = 0x02,
  CONFIG_PARAMETER_WINDOW_TIME    = 0x03,
  CONFIG_PARAMETER_OPERATION_MODE = 0x04,
  CONFIG_PARAMETER_FILTER_SOURCE  = 0x05,
} pyd1698Parameter;

static void printPydMsg(HalPydInMsg_t *msg);
static void emberAfPluginOccupancyPyd1698CliSetCommand(
  pyd1698Parameter parameter,
  uint8_t newValue);

static void printPydMsg(HalPydInMsg_t *msg)
{
  emberAfAppPrintln("config:");
  emberAfAppPrintln("  Reserved: 0x%x", msg->config->reserved);
  emberAfAppPrintln("  Filter Source: 0x%x", msg->config->filterSource);
  emberAfAppPrintln("  Operation Mode: 0x%x", msg->config->operationMode);
  emberAfAppPrintln("  Window Time: 0x%x", msg->config->windowTime);
  emberAfAppPrintln("  Pulse Counter: 0x%x", msg->config->pulseCounter);
  emberAfAppPrintln("  Blind Time: 0x%x", msg->config->blindTime);
  emberAfAppPrintln("  Sensitivity: 0x%x", msg->config->sensitivity);
  emberAfAppPrintln("ADC Voltage: %d", msg->AdcVoltage);
}

void emberAfPluginOccupancyPyd1698CliReadCommand(void)
{
  HalPydCfg_t pydCfg;
  HalPydInMsg_t pydData;

  pydData.config = &pydCfg;
  halOccupancyPyd1698Read(&pydData);
  printPydMsg(&pydData);
}

void emberAfPluginOccupancyPyd1698CliGenerateOccupancyCallback(void)
{
  uint8_t newState;

  newState = (uint8_t)emberUnsignedCommandArgument(0);
  halOccupancyStateChangedCallback((HalOccupancyState)newState);
}

void emberAfPluginOccupancyPyd1698CliSetThresholdCommand(void)
{
  uint8_t newValue;

  newValue = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfPluginOccupancyPyd1698CliSetCommand(CONFIG_PARAMETER_THRESHOLD,
                                             newValue);
}

void emberAfPluginOccupancyPyd1698CliSetBlindTimeCommand(void)
{
  uint8_t newValue;

  newValue = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfPluginOccupancyPyd1698CliSetCommand(CONFIG_PARAMETER_BLIND_TIME,
                                             newValue);
}

void emberAfPluginOccupancyPyd1698CliSetPulseCounterCommand(void)
{
  uint8_t newValue;

  newValue = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfPluginOccupancyPyd1698CliSetCommand(CONFIG_PARAMETER_PULSE_COUNTER,
                                             newValue);
}

void emberAfPluginOccupancyPyd1698CliSetWindowTimeCommand(void)
{
  uint8_t newValue;

  newValue = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfPluginOccupancyPyd1698CliSetCommand(CONFIG_PARAMETER_WINDOW_TIME,
                                             newValue);
}

void emberAfPluginOccupancyPyd1698CliSetFilterSourceCommand(void)
{
  uint8_t newValue;

  newValue = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfPluginOccupancyPyd1698CliSetCommand(CONFIG_PARAMETER_FILTER_SOURCE,
                                             newValue);
}

void emberAfPluginOccupancyPyd1698CliSetOperationModeCommand(void)
{
  uint8_t newValue;

  newValue = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfPluginOccupancyPyd1698CliSetCommand(CONFIG_PARAMETER_OPERATION_MODE,
                                             newValue);
}

static void emberAfPluginOccupancyPyd1698CliSetCommand(
  pyd1698Parameter parameter,
  uint8_t newValue)
{
  HalPydInMsg_t pydData;
  HalPydCfg_t currentPydCfg;

  halOccupancyPyd1698GetCurrentConfiguration(&currentPydCfg);

  pydData.config = &currentPydCfg;
  halOccupancyPyd1698Read(&pydData);

  switch (parameter) {
    case CONFIG_PARAMETER_THRESHOLD:
      currentPydCfg.sensitivity = newValue;
      break;
    case CONFIG_PARAMETER_BLIND_TIME:
      currentPydCfg.blindTime = newValue;
      break;
    case CONFIG_PARAMETER_PULSE_COUNTER:
      currentPydCfg.pulseCounter = newValue;
      break;
    case CONFIG_PARAMETER_WINDOW_TIME:
      currentPydCfg.windowTime = newValue;
      break;
    case CONFIG_PARAMETER_OPERATION_MODE:
      currentPydCfg.operationMode = newValue;
      break;
    case CONFIG_PARAMETER_FILTER_SOURCE:
      currentPydCfg.filterSource = newValue;
      break;
  }

  emberAfAppPrintln("Writing the following:");
  emberAfAppPrintln("  threshold: 0x%x", currentPydCfg.sensitivity);
  emberAfAppPrintln("  blindTime: 0x%x", currentPydCfg.blindTime);
  emberAfAppPrintln("  pulseCounter: 0x%x", currentPydCfg.pulseCounter);
  emberAfAppPrintln("  windowTime: 0x%x", currentPydCfg.windowTime);
  emberAfAppPrintln("  operationMode: 0x%x", currentPydCfg.operationMode);
  emberAfAppPrintln("  filterSource: 0x%x\n\n", currentPydCfg.filterSource);

  emberAfAppPrintln("Before writing new value:");
  printPydMsg(&pydData);
  halOccupancyPyd1698WriteConfiguration(&currentPydCfg);
  emberAfAppPrintln("After writing new value:");
  halOccupancyPyd1698Read(&pydData);
  printPydMsg(&pydData);
}
