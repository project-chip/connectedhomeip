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
 * @brief CLI for the WWAH Connectivity Manager plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "wwah-connectivity-manager.h"

void emAfPluginWwahConnectivityManagerInfoCommand(void)
{
  wwahConnectivityManagerPrintInfo();
}

void emAfPluginWwahConnectivityManagerEnableCommand(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfPluginWwahConnectivityManagerEnableRejoinAlgorithm(endpoint);
}

void emAfPluginWwahConnectivityManagerDisableCommand(void)
{
  uint8_t endpoint = (uint8_t)emberUnsignedCommandArgument(0);
  emberAfPluginWwahConnectivityManagerDisableRejoinAlgorithm(endpoint);
}

void emAfPluginWwahConnectivityManagerShowCommand(void)
{
  wwahConnectivityManagerShow();
}
// e.g. plugin wwah-connectivity-manager parent-recovery 1 0
// badParentRejoinPeriod == 0 means use the default value of 24 hours (24*60 mins)

void emAfPluginWwahConnectivityManagerBadParentRecoveryCommand(void)
{
  uint8_t enabled = (uint8_t)emberUnsignedCommandArgument(0);
  uint16_t badParentRejoinPeriod = (uint8_t)emberUnsignedCommandArgument(1);
  if (enabled) {
    emberAfPluginWwahConnectivityManagerEnableBadParentRecovery(badParentRejoinPeriod);
  } else {
    emberAfPluginWwahConnectivityManagerDisableBadParentRecovery();
  }
}
// e.g. plugin wwah-connectivity-manager minRssi 50
void emAfPluginWwahConnectivityManagerSetMinRssiCommand()
{
  int8_t rssi = (int8_t)emberSignedCommandArgument(0);
  EmberBeaconClassificationParams param;
  emberGetBeaconClassificationParams(&param);
  param.minRssiForReceivingPkts = rssi;
  emberSetBeaconClassificationParams(&param);
}
