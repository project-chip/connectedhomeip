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
 * @brief APIs and defines for the Key Establishment plugin, which handles
 *        establishment of link keys per the Smart Energy Key Establishment
 *        cluster.
 *******************************************************************************
   ******************************************************************************/

// Init - bytes: suite (2), key gen time (1), derive secret time (1), cert (48)
#define EM_AF_KE_INITIATE_SIZE        (2 + 1 + 1 + EMBER_CERTIFICATE_SIZE)
#define EM_AF_KE_INITIATE_SIZE_283K1  (2 + 1 + 1 + EMBER_CERTIFICATE_283K1_SIZE)
#define EM_AF_KE_EPHEMERAL_SIZE       EMBER_PUBLIC_KEY_SIZE
#define EM_AF_KE_EPHEMERAL_SIZE_283K1 EMBER_PUBLIC_KEY_283K1_SIZE
#define EM_AF_KE_SMAC_SIZE            EMBER_SMAC_SIZE

// Terminate - bytes: status (1), wait time (1), suite (2)
#define EM_AF_KE_TERMINATE_SIZE (1 + 1 + 2)

#define APS_ACK_TIMEOUT_SECONDS 1

#define KEY_ESTABLISHMENT_APS_DUPLICATE_DETECTION_TIMEOUT_SEC   5

// These values reported to the remote device as to how long the local
// device takes to execute these operations.
#define DEFAULT_EPHEMERAL_DATA_GENERATE_TIME_SECONDS   (10 + APS_ACK_TIMEOUT_SECONDS)
#define DEFAULT_GENERATE_SHARED_SECRET_TIME_SECONDS    (15 + APS_ACK_TIMEOUT_SECONDS)

extern const uint8_t emAfKeyEstablishMessageToDataSize[];

#ifdef EZSP_HOST
  #define emAfPluginKeyEstablishmentGenerateCbkeKeysHandler          ezspGenerateCbkeKeysHandler
  #define emAfPluginKeyEstablishmentCalculateSmacsHandler            ezspCalculateSmacsHandler
  #define emAfPluginKeyEstablishmentGenerateCbkeKeysHandler283k1    ezspGenerateCbkeKeysHandler283k1
  #define emAfPluginKeyEstablishmentCalculateSmacsHandler283k1       ezspCalculateSmacsHandler283k1
#else
  #define emAfPluginKeyEstablishmentGenerateCbkeKeysHandler          emberGenerateCbkeKeysHandler
  #define emAfPluginKeyEstablishmentCalculateSmacsHandler            emberCalculateSmacsHandler
  #define emAfPluginKeyEstablishmentGenerateCbkeKeysHandler283k1    emberGenerateCbkeKeysHandler283k1
  #define emAfPluginKeyEstablishmentCalculateSmacsHandler283k1       emberCalculateSmacsHandler283k1
#endif

#define TERMINATE_STATUS_STRINGS { \
    "Success",                     \
    "Unknown Issuer",              \
    "Bad Key Confirm",             \
    "Bad Message",                 \
    "No resources",                \
    "Unsupported suite",           \
    "Invalid certificate",         \
    "???",                         \
}
#define UNKNOWN_TERMINATE_STATUS 7

typedef enum {
  NO_KEY_ESTABLISHMENT_EVENT     = 0,
  //Initiator only event
  CHECK_SUPPORTED_CURVES         = 1,
  BEGIN_KEY_ESTABLISHMENT        = 2,
  GENERATE_KEYS                  = 3,
  SEND_EPHEMERAL_DATA_MESSAGE    = 4,
  GENERATE_SHARED_SECRET         = 5,
  SEND_CONFIRM_KEY_MESSAGE       = 6,

  // Initiator only event
  INITIATOR_RECEIVED_CONFIRM_KEY = 7,
} KeyEstablishEvent;

typedef uint8_t KeyEstablishMessage;

extern EmberAfCbkeKeyEstablishmentSuite emAfAvailableCbkeSuite;
extern EmberAfCbkeKeyEstablishmentSuite emAfCurrentCbkeSuite;

# define isCbkeKeyEstablishmentSuiteValid() \
  (emAfCurrentCbkeSuite <= EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1)

# define isCbkeKeyEstablishmentSuite163k1() \
  (emAfCurrentCbkeSuite                     \
   == EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1)

# define isCbkeKeyEstablishmentSuite283k1() \
  (emAfCurrentCbkeSuite                     \
   == EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1)

#define cleanupAndStop(message) cleanupAndStopWithDelay((message), 0)

bool checkIssuer(uint8_t *issuer);
void cleanupAndStopWithDelay(EmberAfKeyEstablishmentNotifyMessage message,
                             uint8_t delayInSec);
EmberAfKeyEstablishmentNotifyMessage sendCertificate(void);
void sendNextKeyEstablishMessage(KeyEstablishMessage message,
                                 uint8_t *data);
EmberStatus emGenerateCbkeKeysForCurve(void);
void emAfKeyEstablishmentSelectCurve(EmberAfCbkeKeyEstablishmentSuite suite);
EmberStatus emCalculateSmacsForCurve(bool amInitiator,
                                     EmberCertificate283k1Data* partnerCert,
                                     EmberPublicKey283k1Data* partnerEphemeralPublicKey);
EmberStatus emberClearTemporaryDataMaybeStoreLinkKeyForCurve(bool storeLinkKey);
void emberAfPluginKeyEstablishmentReadAttributesCallback(EmberAfCbkeKeyEstablishmentSuite suite);
void emAfSkipCheckSupportedCurves(EmberAfCbkeKeyEstablishmentSuite suite);
void emAfSetAvailableCurves(EmberAfCbkeKeyEstablishmentSuite suite);

// Test code
#if defined(EMBER_TEST) && defined(EMBER_SCRIPTED_TEST)
void scriptTestCheckpoint(char* formatString, ...);
#else
#define scriptTestCheckpoint(string, ...)
#endif /*defined(EMBER_TEST) && defined(EMBER_SCRIPTED_TEST)*/
