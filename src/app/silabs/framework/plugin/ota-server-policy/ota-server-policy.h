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
 * @brief A sample policy file that implements the callbacks for the
 * Zigbee Over-the-air bootload cluster server.
 *******************************************************************************
   ******************************************************************************/

void emAfOtaServerPolicyPrint(void);

void emAfOtaServerSetQueryPolicy(uint8_t value);
void emAfOtaServerSetBlockRequestPolicy(uint8_t value);
void emAfOtaServerSetUpgradePolicy(uint8_t value);

bool emAfServerPageRequestTickCallback(uint16_t relativeOffset, uint8_t blockSize);
void emAfSetPageRequestMissedBlockModulus(uint16_t modulus);
void emAfOtaServerSetPageRequestPolicy(uint8_t value);
void emAfOtaServerPolicySetMinBlockRequestPeriod(uint16_t minBlockRequestPeriodMS);
uint8_t emberAfOtaServerImageBlockRequestCallback(EmberAfImageBlockRequestCallbackStruct* data);

// Callbacks

#define OTA_SERVER_NO_RATE_LIMITING_FOR_CLIENT          0
#define OTA_SERVER_CLIENT_USES_MILLISECONDS             1
#define OTA_SERVER_CLIENT_USES_SECONDS                  2
#define OTA_SERVER_DISCOVER_CLIENT_DELAY_UNITS          3
#ifdef EMBER_TEST
 #define OTA_SERVER_DO_NOT_OVERRIDE_CLIENT_DELAY_UNITS  0xFF
#endif

uint8_t emberAfPluginOtaServerPolicyGetClientDelayUnits(EmberNodeId clientNodeId,
                                                        EmberEUI64 clientEui64);
