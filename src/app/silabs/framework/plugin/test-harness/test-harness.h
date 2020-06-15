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
 * @brief Definitions for the Test Harness plugin.
 *******************************************************************************
   ******************************************************************************/

#define CBKE_OPERATION_GENERATE_KEYS   0
#define CBKE_OPERATION_GENERATE_SECRET 1

#if defined(EMBER_AF_PLUGIN_TEST_HARNESS)
extern EmberEventControl emAfKeyEstablishmentTestHarnessEventControl;

extern uint16_t emAfKeyEstablishmentTestHarnessGenerateKeyTime;
extern uint16_t emAfKeyEstablishmentTestHarnessConfirmKeyTime;

extern uint16_t emAfKeyEstablishmentTestHarnessAdvertisedGenerateKeyTime;

extern bool emAfTestHarnessAllowRegistration;

// Allows test harness to change the message or suppress it.
// Returns true if the message should be sent, false if not.
bool emAfKeyEstablishmentTestHarnessMessageSendCallback(uint8_t message);

bool emAfKeyEstablishmentTestHarnessCbkeCallback(uint8_t cbkeOperation,
                                                 uint8_t* data1,
                                                 uint8_t* data2);
void emAfKeyEstablishementTestHarnessEventHandler(void);

  #define EMBER_AF_TEST_HARNESS_EVENT_STRINGS "Test harness",

  #define EMBER_KEY_ESTABLISHMENT_TEST_HARNESS_EVENT \
  { &emAfKeyEstablishmentTestHarnessEventControl, emAfKeyEstablishementTestHarnessEventHandler },
  #define EMBER_AF_CUSTOM_KE_EPHEMERAL_DATA_GENERATE_TIME_SECONDS \
  emAfKeyEstablishmentTestHarnessGenerateKeyTime
  #define EMBER_AF_CUSTOM_KE_GENERATE_SHARED_SECRET_TIME_SECONDS \
  emAfKeyEstablishmentTestHarnessConfirmKeyTime

  #define EM_AF_ADVERTISED_EPHEMERAL_DATA_GEN_TIME_SECONDS \
  emAfKeyEstablishmentTestHarnessAdvertisedGenerateKeyTime

extern bool emKeyEstablishmentPolicyAllowNewKeyEntries;
extern bool emAfTestHarnessSupportForNewPriceFields;

  #define sendSE11PublishPriceCommand emAfTestHarnessSupportForNewPriceFields

#else
  #define sendSE11PublishPriceCommand true

  #define EMBER_AF_TEST_HARNESS_EVENT_STRINGS

  #define emAfKeyEstablishmentTestHarnessMessageSendCallback(x)      (true)
  #define emAfKeyEstablishmentTestHarnessCbkeCallback(x, y, z) (false)

  #define EMBER_KEY_ESTABLISHMENT_TEST_HARNESS_EVENT

  #define emAfTestHarnessAllowRegistration (1)
#endif

void emAfTestHarnessResetApsFrameCounter(void);
void emAfTestHarnessAdvanceApsFrameCounter(void);

void emberAfPluginTestHarnessWriteAttributesResponseCallback(EmberAfClusterId clusterId,
                                                             uint8_t * buffer,
                                                             uint16_t bufLen);
void emberAfPluginTestHarnessReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                                            uint8_t * buffer,
                                                            uint16_t bufLen);

void emAfTestHarnessStartImageStampCalculation(void);
