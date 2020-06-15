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
 * @brief Definitions for the WWAH Connectivity Manager plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_WWAH_CONNECTION_MANAGER_H
#define SILABS_WWAH_CONNECTION_MANAGER_H

void emberAfPluginWwahConnectivityManagerInitCallback(uint8_t endpoint);
bool emberAfPluginWwahConnectivityManagerIsRejoinAlgorithmEnabled(void);
void emberAfPluginWwahConnectivityManagerEnableRejoinAlgorithm(uint8_t endpoint);
void emberAfPluginWwahConnectivityManagerDisableRejoinAlgorithm(uint8_t endpoint);
bool emberAfPluginWwahConnectivityManagerIsBadParentRecoveryEnabled(void);
void emberAfPluginWwahConnectivityManagerEnableBadParentRecovery(uint16_t badParentRejoinPeriod);
void emberAfPluginWwahConnectivityManagerDisableBadParentRecovery();
EmberStatus emberAfPluginWwahConnectivityManagerEnablePeriodicRouterCheckIns(uint16_t checkInInterval);
EmberStatus emberAfPluginWwahConnectivityManagerDisablePeriodicRouterCheckIns(void);

void emberAfPluginWwahConnectivityManagerSetRejoinParameters(uint16_t fastRejoinTimeoutSeconds,
                                                             uint16_t durationBetweenRejoinsSeconds,
                                                             uint16_t fastRejoinFirstBackoffSeconds,
                                                             uint16_t maxBackoffTimeSeconds,
                                                             uint16_t maxBackoffIterations);

// Internal APIs
void wwahConnectivityManagerPrintInfo(void);
void wwahConnectivityManagerShow(void);
void emAfPluginWwahConnectivityManagerEnableBadParentRecovery(uint8_t endpoint, uint16_t badParentRejoinPeriod);
void emAfPluginWwahConnectivityManagerDisableBadParentRecovery(uint8_t endpoint);

#endif // SILABS_WWAH_CONNECTION_MANAGER_H
