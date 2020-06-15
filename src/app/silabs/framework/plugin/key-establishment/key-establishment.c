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
 * @brief Routines for the Key Establishment plugin, which handles
 *        establishment of link keys per the Smart Energy Key Establishment
 *        cluster.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/security/crypto-state.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include "hal/micro/random.h"

#include "app/framework/plugin/test-harness/test-harness.h"

#if !defined(EZSP_HOST)
  #include "stack/include/cbke-crypto-engine.h"
#endif
#include "key-establishment.h"
#include "key-establishment-storage.h"

//------------------------------------------------------------------------------
// Globals

//#define FORCE_283K1_BEHAVIOR

#define LAST_KEY_ESTABLISH_EVENT INITIATOR_RECEIVED_CONFIRM_KEY

// These are initialized by the init routine.
static KeyEstablishEvent lastEvent;

typedef struct {
  EmberEUI64 eui64;
  EmberPanId panId;
} InterPan;

typedef struct {
  EmberNodeId nodeId;
  uint8_t       endpoint;
} IntraPan;

typedef union {
  InterPan interPan;
  IntraPan intraPan;
} PanData;

typedef struct {
  PanData pan;
  bool isInitiator;
  bool isIntraPan;
  uint8_t sequenceNumber;
} KeyEstablishmentPartner;

static KeyEstablishmentPartner keyEstPartner;

static EmberNodeId lastNodeSuccessfulCbke = EMBER_NULL_NODE_ID;
static uint8_t lastConfirmKeyDataResponseApsSequence;
EmberEventControl emberAfPluginKeyEstablishmentApsDuplicateDetectionEventControl;
void emberAfPluginKeyEstablishmentApsDuplicateDetectionEventHandler(
  EmberEventControl *control);

#define KEY_ESTABLISHMENT_TIMEOUT_BASE_SECONDS 10

//This is mask for the Valid Key Usage byte.
#define KEY_USAGE_BYTE 36
#define KEY_USAGE_BYTE_VALID_VALUE 0x08
#define TYPE_BYTE 0
#define TYPE_BYTE_VALID_VALUE 0x00
#define CURVE_BYTE 9
#define CURVE_BYTE_VALID_VALUE 0x0D
#define HASH_BYTE 10
#define HASH_BYTE_VALID_VALUE 0x08
// This is the delay time the local device will report when it sends the
// Terminate message
#define BACK_OFF_TIME_REPORTED_TO_PARTNER    30      // in seconds

// This is the delay time the local device will use when it encounters
// a failure that doesn't receive an explicit delay from the partner.
#define INTERNAL_ERROR_BACK_OFF_TIME         10      // in seconds

#if !defined(EMBER_AF_CUSTOM_KE_EPHEMERAL_DATA_GENERATE_TIME_SECONDS)
  #define EMBER_AF_CUSTOM_KE_EPHEMERAL_DATA_GENERATE_TIME_SECONDS \
  DEFAULT_EPHEMERAL_DATA_GENERATE_TIME_SECONDS
#endif

#if !defined(EMBER_AF_CUSTOM_KE_GENERATE_SHARED_SECRET_TIME_SECONDS)
  #define EMBER_AF_CUSTOM_KE_GENERATE_SHARED_SECRET_TIME_SECONDS \
  DEFAULT_GENERATE_SHARED_SECRET_TIME_SECONDS
#endif

#define EPHEMERAL_DATA_GENERATE_TIME_SECONDS   EMBER_AF_CUSTOM_KE_EPHEMERAL_DATA_GENERATE_TIME_SECONDS
#define GENERATE_SHARED_SECRET_TIME_SECONDS    EMBER_AF_CUSTOM_KE_GENERATE_SHARED_SECRET_TIME_SECONDS

// For purposes of testing timeouts, we allow redefining the advertised
// ephemeral data generate time separately from the actual
// ephemeral data generate time.
#if !defined(EM_AF_ADVERTISED_EPHEMERAL_DATA_GEN_TIME_SECONDS)
  #define EM_AF_ADVERTISED_EPHEMERAL_DATA_GEN_TIME_SECONDS EPHEMERAL_DATA_GENERATE_TIME_SECONDS
#endif

// Offsets within the payload of the Initiate Key request/response messages
#define EPHEMERAL_DATA_TIME_OFFSET 2
#define CONFIRM_KEY_TIME_OFFSET    3

EmberAfCbkeKeyEstablishmentSuite emAfAvailableCbkeSuite = EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1;
EmberAfCbkeKeyEstablishmentSuite emAfCurrentCbkeSuite   = EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1;
EmberAfCbkeKeyEstablishmentSuite emUseTestHarnessSuite  = EMBER_AF_INVALID_KEY_ESTABLISHMENT_SUITE;

#if defined EMBER_TEST
KeyEstablishEvent timeoutState = NO_KEY_ESTABLISHMENT_EVENT;
#endif

#define CBKE_163K1_SUITE_OFFSET 0
#define CBKE_283K1_SUITE_OFFSET 4

static uint8_t cbkeSuiteOffset = CBKE_163K1_SUITE_OFFSET;
//The SMAC and Terminate sizes are duplicated to be able to easily select
//suite based sizes for the 163k and the 283k curve.
// This KeyEstablishMessage enum to message size.
const uint8_t emAfKeyEstablishMessageToDataSize[] = {
  EM_AF_KE_INITIATE_SIZE,
  EM_AF_KE_EPHEMERAL_SIZE,
  EM_AF_KE_SMAC_SIZE,
  EM_AF_KE_TERMINATE_SIZE,
  EM_AF_KE_INITIATE_SIZE_283K1,
  EM_AF_KE_EPHEMERAL_SIZE_283K1,
  EM_AF_KE_SMAC_SIZE,
  EM_AF_KE_TERMINATE_SIZE,
};

// This relates AppNotifyMesage enum to EmberAfAmiKeyEstablishmentStatus codes.
// A value of EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS means no terminate
// message is sent.
static const EmberAfAmiKeyEstablishmentStatus appNotifyMessageToKeyEstablishStatus[] = {
  // Success!
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // NO_APP_MESSAGE
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // RECEIVED_PARTNER_CERTIFICATE
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // GENERATING_EPHEMERAL_KEYS
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // GENERATING_SHARED_SECRET
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // KEY_GENERATION_DONE
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // GENERATE_SHARED_SECRET_DONE
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // LINK_KEY_ESTABLISHED

  // Failures
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_NO_RESOURCES,       // NO_LOCAL_RESOURCES
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // PARTNER_NO_RESOURCES
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // TIMEOUT_OCCURRED
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // INVALID_APP_COMMAND
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // MESSAGE_SEND_FAILURE
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS,            // PARTNER_SENT_TERMINATE
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_BAD_MESSAGE,        // INVALID_PARTNER_MESSAGE
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_BAD_MESSAGE,        // PARTNER_SENT_DEFAULT_RESPONSE_ERROR

  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_UNKNOWN_ISSUER,     // BAD_CERTIFICATE_ISSUER
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_BAD_KEY_CONFIRM,    // KEY_CONFIRM_FAILURE
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_UNSUPPORTED_SUITE,  // BAD_KEY_ESTABLISHMENT_SUITE
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_NO_RESOURCES,       // KEY_TABLE_FULL
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_NO_RESOURCES,       // NO_ESTABLISHMENT_ALLOWED
  EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_INVALID_KEY_USAGE,  // INVALID_CERTIFICATE_KEY_USAGE
};

#define sendEphemeralData(publicKey)                                   \
  (sendNextKeyEstablishMessage(ZCL_EPHEMERAL_DATA_RESPONSE_COMMAND_ID, \
                               (publicKey)))
#define sendConfirmKey(smac)                                            \
  (sendNextKeyEstablishMessage(ZCL_CONFIRM_KEY_DATA_REQUEST_COMMAND_ID, \
                               (smac)))

// This relates the KeyEstablishEvent enum to the timeouts for each event.
// We will setup the values when we receive the first message.
// Values in seconds.  The timeout values passed in the protocol are 8-bit
// but that means when we add our fudge factor it can overflow beyond
// 255.  So we make these 16-bit values to prevent problems.
static uint16_t eventTimeoutsSec[LAST_KEY_ESTABLISH_EVENT];

// This is the last endpoint that was initialized for key establishment
// it is the one we use for all our event scheduling
static uint8_t keyEstablishmentEndpoint = 0xFF;

// We record the sequence numbers of our partner device's messages so
// we can filter out dupes.  3 messages can be received during normal
// KE, plus 1 for a possible Terminate message.
#define NUM_SEQ_NUMBER 4
static uint8_t apsSequenceNumbersReceived = 0;
static uint8_t apsSequenceNumbers[NUM_SEQ_NUMBER];

//Offsets for the ECC 163k and 283k curve. We assume this to default to
//the ECC 163k curve.
#define CERT_SUBJECT_OFFSET 22
#define CERT_ISSUER_OFFSET  (CERT_SUBJECT_OFFSET + 8)
#define CERT_SUBJECT_OFFSET_283K1 28
#define CERT_ISSUER_OFFSET_283K1  11
#define CERT_ISSUER_SIZE    8

static uint8_t certSubjectOffset = CERT_SUBJECT_OFFSET;
static uint8_t certIssuerOffset  = CERT_ISSUER_OFFSET;

#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_KEY_ESTABLISHMENT_CLUSTER)
static const char * terminateStatus[] = TERMINATE_STATUS_STRINGS;
#endif

// Over the air message lengths for Initiate Key Establishment Request and Response
// certificate + keyEstablishmentSuite + ephemeralDataGenerateTime + confirmKeyGenerateTime
# define INITIATE_KEY_ESTABLISHMENT_LENGTH_163K1 EMBER_CERTIFICATE_SIZE + 2 + 1 + 1
# define INITIATE_KEY_ESTABLISHMENT_LENGTH_283K1 EMBER_CERTIFICATE_283K1_SIZE + 2 + 1 + 1

//------------------------------------------------------------------------------
// Forward declarations
static bool checkMalformed283k1Command(bool isCertificate);
static void checkSupportedCurve(void);
static bool checkKeyTable(uint8_t *bigEndianEui64);
static void keyEstablishStateMachine(KeyEstablishEvent newEvent,
                                     uint8_t *data1,
                                     uint8_t *data2);
static void clearKeyEstablishmentState(void);
static bool askApplicationWithDelay(EmberAfKeyEstablishmentNotifyMessage message,
                                    uint8_t delayInSec);
#define askApplication(message) askApplicationWithDelay((message), 0)
static void sendKeyEstablishMessage(KeyEstablishMessage message);
static void sendTerminateMessage(EmberAfAmiKeyEstablishmentStatus status,
                                 uint8_t backOffTime);
static EmberAfKeyEstablishmentNotifyMessage verifySmac(EmberSmacData *otherSmac);
static bool checkRequestedSuite(uint8_t *suiteLittleEndian);

static void setupEventTimeouts(uint8_t theirGenerateKeyTimeSeconds,
                               uint8_t theirConfirmKeyTimeSeconds);
static bool commandReceivedHandler(EmberAfClusterCommand *cmd);
static void messageSentHandler(EmberOutgoingMessageType type,
                               uint16_t indexOrDestination,
                               EmberApsFrame *apsFrame,
                               uint16_t msgLen,
                               uint8_t *message,
                               EmberStatus status);
static bool commandIsFromOurPartner(const EmberAfClusterCommand *cmd);
static bool setPartnerFromCommand(const EmberAfClusterCommand *cmd);
static void writeKeyEstablishmentClusterAttribute(uint8_t endpoint);
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_KEY_ESTABLISHMENT_CLUSTER)
static void debugPrintSmac(bool initiatorSmac, uint8_t *smac);
static void debugPrintOtherSmac(bool received, uint8_t *smac);
static void debugPrintCert(bool initiatorCert, uint8_t *cert);
void emPrintData283k1(uint8_t *buffer, uint8_t size, unsigned char *type);
static void debugPrintKey(bool initiatorKey, uint8_t *key);
#else
  #define debugPrintSmac(initiatorSmac, smac)
  #define debugPrintOtherSmac(received, smac)
  #define debugPrintCert(initiatorCert, cert)
  #define debugPrintKey(initiatorKey, key)
#endif

#if defined(EMBER_AF_HAS_SECURITY_PROFILE_SE_TEST)

  #if defined(EMBER_AF_PLUGIN_TEST_HARNESS)
// Test code only
    #define NEW_KEY_TABLE_ENTRY_ALLOWED (emKeyEstablishmentPolicyAllowNewKeyEntries)

  #else
    #define NEW_KEY_TABLE_ENTRY_ALLOWED true
  #endif

#elif defined(EMBER_AF_HAS_SECURITY_PROFILE_SE_FULL)
  #define NEW_KEY_TABLE_ENTRY_ALLOWED false

#else
  #error SE Security not configuration defined
#endif

//------------------------------------------------------------------------------
// Internal Functions
static void checkSupportedCurve(void)
{
  // This function should only get called for Intra-Pan Key Establishment. We currently
  // don't support the read attributes stage for Inter-Pan Key Establishment.
  uint16_t attributeIds;
  attributeIds = ZCL_KEY_ESTABLISHMENT_SUITE_SERVER_ATTRIBUTE_ID;
  emberAfFillCommandGlobalClientToServerReadAttributes(ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                                       &attributeIds,
                                                       sizeof(attributeIds));
  emberAfSetCommandEndpoints(keyEstablishmentEndpoint, keyEstPartner.pan.intraPan.endpoint);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, keyEstPartner.pan.intraPan.nodeId);
}

static bool checkValidByteValue283k1(uint8_t byte, uint8_t requiredValue, uint8_t mask)
{
  if (isCbkeKeyEstablishmentSuite283k1()) {
    if ((byte & mask) != requiredValue) {
      emberAfKeyEstablishmentClusterPrintln("Error: Invalid byte in cert: byteValue %u requiredValue %u", byte, requiredValue);
      cleanupAndStop(INVALID_CERTIFICATE_KEY_USAGE);
      return false;
    }
  }
  return true;
}

static bool checkKeyTable(uint8_t *bigEndianEui64)
{
  EmberEUI64 eui64;
  EmberKeyStruct keyStruct;

  emberReverseMemCopy(eui64, bigEndianEui64, EUI64_SIZE);

  emberAfKeyEstablishmentClusterPrint("Checking key table for IEEE ");
  emberAfKeyEstablishmentClusterDebugExec(emberAfPrintBigEndianEui64(eui64));
  emberAfKeyEstablishmentClusterPrintln("");

  if (keyEstPartner.isIntraPan
      && keyEstPartner.pan.intraPan.nodeId == EMBER_TRUST_CENTER_NODE_ID) {
    if (emberGetKey(EMBER_TRUST_CENTER_LINK_KEY, &keyStruct) == EMBER_SUCCESS
        && (keyStruct.bitmask & EMBER_KEY_HAS_PARTNER_EUI64)
        && MEMCOMPARE(eui64, keyStruct.partnerEUI64, EUI64_SIZE) == 0) {
      // The key to be updated is our existing TC Link Key, therefore
      // we have room.
      return true;
    }
    emberAfKeyEstablishmentClusterPrintln("Error: EUI64 of TC does not match its cert.");
    cleanupAndStop(INVALID_PARTNER_MESSAGE);
    return false;
  } else {
    // We are the trust center
    // or at least act as one in a direct CBKE between two non-TC devices

    // We either have an existing link key entry that we can update,
    // or there is an empty entry.
    if (emberFindKeyTableEntry(eui64, true) != 0xFF) {
      return true;
    } else if (NEW_KEY_TABLE_ENTRY_ALLOWED != true) {
      emberAfKeyEstablishmentClusterPrintln("Error: Unknown EUI64 trying to perform CBKE.");
      cleanupAndStop(KEY_TABLE_FULL);
      return false;
    } else if (0xFF != emberFindKeyTableEntry((uint8_t*)emberAfNullEui64,
                                              true)) {
      return true;
    } else {
      // MISRA requires ..else if.. to have terminating else.
    }

    emberAfKeyEstablishmentClusterPrintln("Error: Key table full.");
    cleanupAndStop(KEY_TABLE_FULL);
    return false;
  }
}

// data1 will used for the initiate key metadata, ephemeral public key, the
//   initiator locally calculated smac, or the partner's received smac.
// data2 will used for the partner certificate or for the responder locally
//   calculated smac.

static void keyEstablishStateMachine(KeyEstablishEvent newEvent,
                                     uint8_t *data1,
                                     uint8_t *data2)
{
  // The state machine is called by the public APIs, library callbacks, and
  // command handlers.  Most of the processing in this state machine only cares
  // whether the next event is a result of a received command.
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();

  if (cmd != NULL && cmd->type >= EMBER_INCOMING_MULTICAST) {
    emberAfKeyEstablishmentClusterPrintln("Ignoring Broadcast KE message");
    return;
  }

  emberAfKeyEstablishmentClusterPrintln("Key Establish State Machine event:%d",
                                        newEvent);

  if (cmd != NULL) {
    // First catch any duplicates of the ConfirmKeyDataResponse message that
    // could be sent as APS retries by the neighboring device
    if ((cmd->interPanHeader == NULL)
        && (lastNodeSuccessfulCbke == cmd->source)
        && (lastConfirmKeyDataResponseApsSequence == cmd->apsFrame->sequence)
        && (newEvent == INITIATOR_RECEIVED_CONFIRM_KEY)) {
      emberAfKeyEstablishmentClusterPrintln("Got duplicate APS message "
                                            "(seq:%d), dropping!",
                                            cmd->apsFrame->sequence);
      scriptTestCheckpoint("Key Establishment dropped duplicate ConfirmKeyData "
                           "frame");
      return;
    } else if (commandIsFromOurPartner(cmd)
               && (keyEstPartner.isInitiator || lastEvent != NO_KEY_ESTABLISHMENT_EVENT)) {
      // Filter out duplicate APS messages.

      // Edge Case: If the same partner initiates key establishment with us and
      // they happen to use one of the same APS sequence numbers as the last
      // time, this will fail since we assume it is a duplicate message.  The
      // hope is that the partner will retry and it should succeed.
      uint8_t i;
      for (i = 0; i < apsSequenceNumbersReceived; i++) {
        if (cmd->apsFrame->sequence == apsSequenceNumbers[i]) {
          emberAfKeyEstablishmentClusterPrintln("Got duplicate APS message "
                                                "(seq:%d), dropping!",
                                                cmd->apsFrame->sequence);
          return;
        }
      }
    } else if (!isCbkeKeyEstablishmentSuiteValid() || (lastEvent != NO_KEY_ESTABLISHMENT_EVENT)) {
      KeyEstablishmentPartner tmpPartner;
      emberAfKeyEstablishmentClusterPrintln("emAfAvailableCbkeSuite %u last event %u", emAfAvailableCbkeSuite, lastEvent);
      // If we have not successfully initialized or we are already in doing
      // key establishment with another partner, tell this new partner to go
      // away and maybe try again later.  The sendTerminateMessage function
      // assumes it is sending to the current partner, so we have to temporarily
      // switch to the new partner, send the terminate, and then switch back to
      // our real partner.
      MEMCOPY(&tmpPartner, &keyEstPartner, sizeof(KeyEstablishmentPartner));
      emberAfKeyEstablishmentClusterPrintln(isCbkeKeyEstablishmentSuiteValid()
                                            ? "Second Key estabishment not supported, terminating it."
                                            : "Key Est. FAILED INITIALIZATION, terminating");
      if (setPartnerFromCommand(cmd)) {
        keyEstPartner.sequenceNumber = cmd->seqNum;
        sendTerminateMessage(EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_NO_RESOURCES,
                             BACK_OFF_TIME_REPORTED_TO_PARTNER);
      }
      MEMMOVE(&keyEstPartner, &tmpPartner, sizeof(KeyEstablishmentPartner));
      return;
    } else {
      // MISRA requires ..else if.. to have terminating else.
    }

    // If we got here and we're not currently doing key establishment, it means
    // we are starting key establishment with a new partner.  Remember the new
    // partner and clear the previous set of sequence numbers.  We must handle
    // the case where the same partner is initiating key establishment with us
    // that did it last time.  We can't use the above else clause to do that.
    if (lastEvent == NO_KEY_ESTABLISHMENT_EVENT) {
      if (!setPartnerFromCommand(cmd)) {
        return;
      }
      apsSequenceNumbersReceived = 0;
    }

    // Remember the received APS sequence numbers so we can filter duplicates.
    if (keyEstPartner.isIntraPan) {
      apsSequenceNumbers[apsSequenceNumbersReceived] = cmd->apsFrame->sequence;
      apsSequenceNumbersReceived++;
    }

    // Remember the received ZCL sequence number for the response.
    if (keyEstPartner.isInitiator) {
      keyEstPartner.sequenceNumber = cmd->seqNum;
    }
  }

  // If we receive an unexpected message, we terminate and hope the partner
  // tries again.
  //CHECK_SUPPORTED_CURVES is only done by the initiator. Therefore, the server
  // will see a jump of 2 for BEGIN_KEY_ESTABLISHMENT
  if (newEvent != lastEvent + 1) {
    if (keyEstPartner.isInitiator && lastEvent == NO_KEY_ESTABLISHMENT_EVENT) {
      if (newEvent != BEGIN_KEY_ESTABLISHMENT) {
        emberAfKeyEstablishmentClusterPrintln("Got wrong message in the sequence. last event %u, new event %u", lastEvent, newEvent);
        scriptTestCheckpoint("Got wrong message in CBKE sequence");
        cleanupAndStop(INVALID_PARTNER_MESSAGE);
        return;
      }
    } else {
      emberAfKeyEstablishmentClusterPrintln("Got wrong message in the sequence. last event %u, new event %u", lastEvent, newEvent);
      cleanupAndStop(INVALID_PARTNER_MESSAGE);
      return;
    }
  }

#if defined EMBER_TEST
  // Testing only
  if (timeoutState != NO_KEY_ESTABLISHMENT_EVENT && timeoutState <= newEvent) {
    emberAfKeyEstablishmentClusterPrintln("Dropping message to induce timeout in partner.");
    cleanupAndStop(TIMEOUT_OCCURRED);
    return;
  }
#endif

  switch (newEvent) {
    case CHECK_SUPPORTED_CURVES:
    {
      checkSupportedCurve();
      scriptTestCheckpoint("CBKE: check supported curves");
      break;
    }
    // For initiator, send the cert wait for reply.
    // For responder, we received the cert.
    case BEGIN_KEY_ESTABLISHMENT:
    {
      EmberAfKeyEstablishmentNotifyMessage result = NO_APP_MESSAGE;
      emberAfKeyEstablishmentClusterPrintln("Current Suite %u", emAfCurrentCbkeSuite);
      if (keyEstPartner.isInitiator) {
        if (!checkRequestedSuite(data1)
            || !checkIssuer(data2 + certIssuerOffset)
            || !checkKeyTable(data2 + certSubjectOffset)
            || !checkValidByteValue283k1(*(data2 + KEY_USAGE_BYTE), KEY_USAGE_BYTE_VALID_VALUE, KEY_USAGE_BYTE_VALID_VALUE)
            || !checkValidByteValue283k1(*(data2 + TYPE_BYTE), TYPE_BYTE_VALID_VALUE, 0xFF)
            || !checkValidByteValue283k1(*(data2 + CURVE_BYTE), CURVE_BYTE_VALID_VALUE, 0xFF)
            || !checkValidByteValue283k1(*(data2 + HASH_BYTE), HASH_BYTE_VALID_VALUE, 0xFF)) {
          debugPrintCert(true, data2);
          // For all of the above functions, a terminate message is
          // sent with the appropriate error code.
          return;
        }

        //TO DO: Print 283k1 cert here.
        debugPrintCert(true, data2);

        if (!askApplication(RECEIVED_PARTNER_CERTIFICATE)
            || !storePublicPartnerData(true, // certificate?
                                       data2)) {
          result = NO_LOCAL_RESOURCES;
        } else {
          setupEventTimeouts(data1[EPHEMERAL_DATA_TIME_OFFSET],
                             data1[CONFIRM_KEY_TIME_OFFSET]);
        }
      }
      if (result == NO_APP_MESSAGE) {
        result = sendCertificate();
      }
      if (result != NO_APP_MESSAGE) {
        // We now accept a keyEstablishment attribute of 0 and have
        // to watch out for this additional error case.
        // In case our device doesn't have the suite that was incorrectly
        // agreed on, we don't want to send a Terminate Key Establishment
        // message in response to a read attributes response.
        if (!keyEstPartner.isInitiator) {
          result = BAD_KEY_ESTABLISHMENT_SUITE;
        }
        cleanupAndStop(result);
        return;
      } else {
        scriptTestCheckpoint("CBKE: sent CBKE certificate");
      }
      break;
    }

    // For initiator, we received responder cert it is time to generate keys.
    // For responder, we received ephemeral data it is time to generate keys.
    case GENERATE_KEYS:
      if (!keyEstPartner.isInitiator) {
        if (!checkRequestedSuite(data1)
            || !checkIssuer(data2 + certIssuerOffset)
            || !checkKeyTable(data2 + certSubjectOffset)
            || !checkValidByteValue283k1(*(data2 + KEY_USAGE_BYTE), KEY_USAGE_BYTE_VALID_VALUE, KEY_USAGE_BYTE_VALID_VALUE)
            || !checkValidByteValue283k1(*(data2 + TYPE_BYTE), TYPE_BYTE_VALID_VALUE, 0xFF)
            || !checkValidByteValue283k1(*(data2 + CURVE_BYTE), CURVE_BYTE_VALID_VALUE, 0xFF)
            || !checkValidByteValue283k1(*(data2 + HASH_BYTE), HASH_BYTE_VALID_VALUE, 0xFF)) {
          // For all of the above functions, a terminate message is
          // sent with the appropriate error code.
          debugPrintCert(true, data2);
          return;
        }
        setupEventTimeouts(data1[EPHEMERAL_DATA_TIME_OFFSET],
                           data1[CONFIRM_KEY_TIME_OFFSET]);

        debugPrintCert(false, data2);
      } else {
        debugPrintKey(true, data1);
      }

      if (!askApplication(GENERATING_EPHEMERAL_KEYS)
          || !storePublicPartnerData(!keyEstPartner.isInitiator, // certificate?
                                     (!keyEstPartner.isInitiator
                                      ? data2            // partner cert
                                      : data1))          // partner key
          || emGenerateCbkeKeysForCurve() != EMBER_OPERATION_IN_PROGRESS) {
        cleanupAndStop(NO_LOCAL_RESOURCES);
        return;
      }
      scriptTestCheckpoint("CBKE: beginning crypto operation");
      emAfSetCryptoOperationInProgress();
      break;

    // For both roles, we are done generating keys.  Send the message.
    case SEND_EPHEMERAL_DATA_MESSAGE:
      if (!askApplication(KEY_GENERATION_DONE)) {
        cleanupAndStop(NO_LOCAL_RESOURCES);
        return;
      }
      scriptTestCheckpoint("CBKE: sending ephemeral data");
      sendEphemeralData(data1);
      break;

    // For initiator, we received ephemeral data resp.  Generate shared secret.
    // For responder, we received confirm key request.  Generate shared secret.
    case GENERATE_SHARED_SECRET:
    {
      EmberCertificate283k1Data partnerCert;
      EmberPublicKey283k1Data partnerEphemeralPublicKey;

#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_KEY_ESTABLISHMENT_CLUSTER)
      if (!keyEstPartner.isInitiator) {
        debugPrintKey(false, data1);
      } else {
        debugPrintOtherSmac(true, data1);
      }
#endif
      // For the initiator this is slightly ineffecient because we store
      // the public key but then immediately retrieve it.  However it
      // saves on flash to treat responder and initiator the same.
      if (!askApplication(GENERATING_SHARED_SECRET)
          || (!keyEstPartner.isInitiator
              ? !storePublicPartnerData(false, data1)
              : !storeSmac((EmberSmacData *)data1))
          || !retrieveAndClearPublicPartnerData(&partnerCert,
                                                &partnerEphemeralPublicKey)
          || (EMBER_OPERATION_IN_PROGRESS
              != emCalculateSmacsForCurve(!keyEstPartner.isInitiator,
                                          &partnerCert,
                                          &partnerEphemeralPublicKey))) {
        cleanupAndStop(NO_LOCAL_RESOURCES);
        return;
      }
      scriptTestCheckpoint("CBKE: begin crypto operation from ephemeral data");
      emAfSetCryptoOperationInProgress();
      break;
    }

    // For both roles, we are done generating shared secret,
    //   send confirm key message.
    case SEND_CONFIRM_KEY_MESSAGE:
    {
      EmberSmacData *initiatorSmac = (EmberSmacData *)data1;
      EmberSmacData *responderSmac = (EmberSmacData *)data2;

      debugPrintSmac(true, emberSmacContents(initiatorSmac));
      debugPrintSmac(false, emberSmacContents(responderSmac));

      if (!askApplication(GENERATE_SHARED_SECRET_DONE)
          || (!keyEstPartner.isInitiator && !storeSmac(responderSmac))) {
        cleanupAndStop(NO_LOCAL_RESOURCES);
        return;
      }

      if (keyEstPartner.isInitiator) {
        EmberAfKeyEstablishmentNotifyMessage result = verifySmac(initiatorSmac);
        if (result != NO_APP_MESSAGE) {
          cleanupAndStop(result);
          return;
        }
      }

      sendConfirmKey(emberSmacContents(!keyEstPartner.isInitiator
                                       ? initiatorSmac
                                       : responderSmac));

      scriptTestCheckpoint("CBKE: sent confirm key data");

      if (keyEstPartner.isInitiator) {
        // TODO:  Wait for the APS Ack from the initiator and then store
        // the link key.

        // Success!
        cleanupAndStop(LINK_KEY_ESTABLISHED);
        return;
      }
      break;
    }

    case INITIATOR_RECEIVED_CONFIRM_KEY:
    {
      EmberAfKeyEstablishmentNotifyMessage message = verifySmac((EmberSmacData *)data1);
      debugPrintOtherSmac(true, data1);

      if (keyEstPartner.isIntraPan) {
        // Store this the partner node ID and the APS sequence associated with
        // the ConfirmKeyDataResponse, so that an APS retry of it doesn't cause
        // a TerminateKeyResponse be sent in response (once CBKE completes here,
        // an APS retry of the ConfirmKeyDataResponse treated as a brand new
        // attempt at CBKE)
        // We put a timer on this APS duplicate detection mechanism, in the case
        // that a valid packet, whose APS counter has wrapped, is received later
        lastNodeSuccessfulCbke = keyEstPartner.pan.intraPan.nodeId;
        if (cmd != NULL) {
          lastConfirmKeyDataResponseApsSequence = cmd->apsFrame->sequence;
        }
        emberEventControlSetDelayMS(
          emberAfPluginKeyEstablishmentApsDuplicateDetectionEventControl,
          KEY_ESTABLISHMENT_APS_DUPLICATE_DETECTION_TIMEOUT_SEC << 10);
      }

      // For both success and failure, stop key establishment.
      cleanupAndStop(message == NO_APP_MESSAGE
                     ? LINK_KEY_ESTABLISHED
                     : message);

      return;
    }

    default:
      // Unknown message, ignore.
      return;
  }

  {
    uint32_t timeMs = (uint32_t)(eventTimeoutsSec[newEvent]) * MILLISECOND_TICKS_PER_SECOND;
    emberAfScheduleServerTick(keyEstablishmentEndpoint,
                              ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                              timeMs);
  }
  lastEvent = newEvent;
  return;
}

static void clearKeyEstablishmentState(void)
{
  keyEstPartner.isInitiator = true;
  //emberAfKeyEstablishmentClusterPrintln("keyEstPartner.isInitiator clearKeyState %u",keyEstPartner.isInitiator);
  lastEvent = NO_KEY_ESTABLISHMENT_EVENT;
  clearAllTemporaryPublicData();
  emberClearTemporaryDataMaybeStoreLinkKeyForCurve(false);
  emberAfDeactivateServerTick(keyEstablishmentEndpoint,
                              ZCL_KEY_ESTABLISHMENT_CLUSTER_ID);

  // NOTE: When clearing the state, we intentionally retain information about
  // the partner (e.g., node id, APS sequence numbers, etc.).  That information
  // is used to help determine if the a received message is a duplicate of one
  // we already processed.  Because we may receive a retried message after we
  // have completed key establishment, we must remember the partner information.
}

static bool askApplicationWithDelay(EmberAfKeyEstablishmentNotifyMessage message,
                                    uint8_t delayInSec)
{
  const char * appNotifyText[] = APP_NOTIFY_MESSAGE_TEXT;

  if (message == NO_APP_MESSAGE) {
    return true;
  }

  emberAfKeyEstablishmentClusterFlush();
  emberAfKeyEstablishmentClusterPrint("%p: %p %p: %p (%d), %p ",
                                      (!keyEstPartner.isInitiator
                                       ? "Initiator"
                                       : "Responder"),
                                      "Key Establish",
                                      (message >= APP_NOTIFY_ERROR_CODE_START
                                       ? "Error"
                                       : (message == LINK_KEY_ESTABLISHED
                                          ? "Success"
                                          : "Notify")),
                                      appNotifyText[message],
                                      message,
                                      "keyEstPartner");
  if (keyEstPartner.isIntraPan) {
    emberAfKeyEstablishmentClusterPrintln("0x%2x", keyEstPartner.pan.intraPan.nodeId);
    return emberAfKeyEstablishmentCallback(message,
                                           !keyEstPartner.isInitiator,
                                           keyEstPartner.pan.intraPan.nodeId,
                                           delayInSec);
  } else {
    emberAfKeyEstablishmentClusterDebugExec(emberAfPrintBigEndianEui64(keyEstPartner.pan.interPan.eui64));
    emberAfKeyEstablishmentClusterPrintln("");
    return emberAfInterPanKeyEstablishmentCallback(message,
                                                   !keyEstPartner.isInitiator,
                                                   keyEstPartner.pan.interPan.panId,
                                                   keyEstPartner.pan.interPan.eui64,
                                                   delayInSec);
  }
}

static void sendKeyEstablishMessage(KeyEstablishMessage message)
{
  // It is assumed the "appResponseData" has already been filled
  // with data and the "appResponseLength" has been set appropriately
  // including the message overhead that will be filled in by this function.
  uint8_t *ptr = appResponseData;
  *ptr++ = (ZCL_CLUSTER_SPECIFIC_COMMAND
            | (!keyEstPartner.isInitiator
               ? ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
               : ZCL_FRAME_CONTROL_SERVER_TO_CLIENT));
  *ptr++ = keyEstPartner.sequenceNumber;
  *ptr   = message;

  if (!emAfKeyEstablishmentTestHarnessMessageSendCallback(message)) {
    return;
  }

  if (keyEstPartner.isIntraPan) {
    EmberApsFrame apsFrame;
    apsFrame.clusterId = ZCL_KEY_ESTABLISHMENT_CLUSTER_ID;
    apsFrame.sourceEndpoint = keyEstablishmentEndpoint;
    apsFrame.destinationEndpoint = keyEstPartner.pan.intraPan.endpoint;
    apsFrame.options = (EMBER_AF_DEFAULT_APS_OPTIONS | EMBER_APS_OPTION_RETRY);
    emberAfSendUnicast(EMBER_OUTGOING_DIRECT,
                       keyEstPartner.pan.intraPan.nodeId,
                       &apsFrame,
                       appResponseLength,
                       appResponseData);
  } else {
    emberAfSendInterPan(keyEstPartner.pan.interPan.panId,
                        keyEstPartner.pan.interPan.eui64,
                        EMBER_NULL_NODE_ID,
                        0, // multicast id - unused
                        ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                        SE_PROFILE_ID,
                        appResponseLength,
                        appResponseData);
  }
}

static void sendTerminateMessage(EmberAfAmiKeyEstablishmentStatus status,
                                 uint8_t backOffTime)
{
  uint8_t *ptr = appResponseData + EMBER_AF_ZCL_OVERHEAD;
  *ptr++ = status;
  *ptr++ = backOffTime;

  // Since we only support the single key establishment suite, we can
  // get away with casting this into an uint8_t and simply zeroing the high bits.
  *ptr++ = LOW_BYTE(emAfCurrentCbkeSuite);
  *ptr++ = HIGH_BYTE(emAfCurrentCbkeSuite);

  appResponseLength = (ptr - appResponseData);
  sendKeyEstablishMessage(ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID);
}

static EmberAfKeyEstablishmentNotifyMessage verifySmac(EmberSmacData *otherSmac)
{
  // For the initiator, the stored SMAC will be the responder's version
  //   locally calculated.
  // For the responder, the stored SMAC will be the initiator's version
  //   received via the Confirm Key request message.
  EmberSmacData *ptr;
  if (!getSmacPointer(&ptr)) {
    return NO_LOCAL_RESOURCES;
  }

  emberAfKeyEstablishmentClusterPrintln("My local SMAC:");
  debugPrintOtherSmac(false, emberSmacContents(ptr));

  return (MEMCOMPARE(emberSmacContents(ptr), otherSmac, EMBER_SMAC_SIZE) == 0
          ? NO_APP_MESSAGE        // success
          : KEY_CONFIRM_FAILURE);
}

static bool checkRequestedSuite(uint8_t *suiteLittleEndian)
{
  uint16_t requestedSuite;
  requestedSuite = suiteLittleEndian[0] + ((uint16_t)(suiteLittleEndian[1]) << 8);
  if (requestedSuite == EMBER_AF_INVALID_KEY_ESTABLISHMENT_SUITE) {
    emberAfKeyEstablishmentClusterPrintln("Bad Key Establishment Suite");
    cleanupAndStop(BAD_KEY_ESTABLISHMENT_SUITE);
    return false;
  }
  return true;
}

static void setupEventTimeouts(uint8_t theirGenerateKeyTimeSeconds,
                               uint8_t theirConfirmKeyTimeSeconds)
{
  uint8_t i;
  eventTimeoutsSec[0] = 0;  // NO_KEY_ESTABLISHMENT_EVENT

  for (i = 1; i < LAST_KEY_ESTABLISH_EVENT; i++) {
    eventTimeoutsSec[i] = KEY_ESTABLISHMENT_TIMEOUT_BASE_SECONDS;
  }
  eventTimeoutsSec[BEGIN_KEY_ESTABLISHMENT]     += (!keyEstPartner.isInitiator
                                                    ? 0
                                                    : theirGenerateKeyTimeSeconds);
  eventTimeoutsSec[GENERATE_KEYS]               += EPHEMERAL_DATA_GENERATE_TIME_SECONDS;
  eventTimeoutsSec[GENERATE_SHARED_SECRET]      += GENERATE_SHARED_SECRET_TIME_SECONDS;
  eventTimeoutsSec[SEND_EPHEMERAL_DATA_MESSAGE] += (!keyEstPartner.isInitiator
                                                    ? theirGenerateKeyTimeSeconds
                                                    : theirConfirmKeyTimeSeconds);
  // Only initiator needs this timeout while waiting for the event
  // INITIATER_RECEIVED_CONFIRM_KEY_MESSAGE.  Responder ends KE when
  // getting to this event.
  eventTimeoutsSec[SEND_CONFIRM_KEY_MESSAGE]    += theirConfirmKeyTimeSeconds;
}

static bool commandReceivedHandler(EmberAfClusterCommand *cmd)
{
//  EmberAfStatus status = keyEstablishmentClusterCommandParse(cmd);
  EmberAfStatus status = (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER
                          ? emberAfKeyEstablishmentClusterServerCommandParse(cmd)
                          : emberAfKeyEstablishmentClusterClientCommandParse(cmd));
  if (status == EMBER_ZCL_STATUS_MALFORMED_COMMAND) {
    emberAfKeyEstablishmentClusterPrintln(
      "Invalid length for KE command: %d (expected: %d, actual: %d)",
      cmd->commandId,
      emAfKeyEstablishMessageToDataSize[cmd->commandId + cbkeSuiteOffset],
      (cmd->bufLen - cmd->payloadStartIndex));

    cleanupAndStop(INVALID_PARTNER_MESSAGE);
  } else if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfSendDefaultResponse(cmd, status);
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }
  return true;
}

static void messageSentHandler(EmberOutgoingMessageType type,
                               uint16_t indexOrDestination,
                               EmberApsFrame *apsFrame,
                               uint16_t msgLen,
                               uint8_t *message,
                               EmberStatus status)
{
  if (!keyEstPartner.isIntraPan
      || lastEvent == NO_KEY_ESTABLISHMENT_EVENT
      || keyEstPartner.pan.intraPan.nodeId != indexOrDestination) {
    // Unknown APS Ack, or an Ack for key establishment not in progress.
    return;
  }

  if (status != EMBER_SUCCESS) {
    emberAfKeyEstablishmentClusterPrintln("Error: Failed to send key establish message to 0x%2x, status: 0x%x",
                                          indexOrDestination,
                                          status);
    cleanupAndStop(MESSAGE_SEND_FAILURE);
    return;
  }
}

// This function should only return false for inter-PAN commands that do not
// include the source long address or if there is no endpoint for the inter-PAN
// network.
static bool setPartnerFromCommand(const EmberAfClusterCommand *cmd)
{
  keyEstPartner.isInitiator = true;
  //emberAfKeyEstablishmentClusterPrintln("keyEstPartner.isInitiator setPartnerFromCommand %u",keyEstPartner.isInitiator);
  keyEstPartner.isIntraPan = (cmd->interPanHeader == NULL);
  if (keyEstPartner.isIntraPan) {
    keyEstPartner.pan.intraPan.nodeId = cmd->source;
    keyEstPartner.pan.intraPan.endpoint = cmd->apsFrame->sourceEndpoint;
    keyEstablishmentEndpoint = cmd->apsFrame->destinationEndpoint;
  } else {
    if (!(cmd->interPanHeader->options
          & EMBER_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS)) {
      return false;
    }
    keyEstPartner.pan.interPan.panId = cmd->interPanHeader->panId;
    MEMMOVE(keyEstPartner.pan.interPan.eui64,
            cmd->interPanHeader->longAddress,
            EUI64_SIZE);
    keyEstablishmentEndpoint = emberAfPrimaryEndpointForCurrentNetworkIndex();
    if (keyEstablishmentEndpoint == 0xFF) {
      return false;
    }
  }
  return true;
}

static bool commandIsFromOurPartner(const EmberAfClusterCommand *cmd)
{
  // For intra-PAN commands, we should check that the source and destination
  // endpoints and the sequence numbers of the request/response pairs match.
  // However, devices in the field are sometimes not careful about endpoints
  // or sequence numbers, so strictness here would cause interoperability
  // problems.  Neither endpoint nor sequence number mismatches are likely to
  // cause serious problems in practice because Key Establishment is intended
  // to be device-wide rather than per-endpoint and we only support a single
  // partner at a time, so checking our partner and correlating requests and
  // responses is not difficult.

  // We generally make sure that the direction bit is set correctly, but for a
  // terminate command we do not.  This is due to timeouts and the fact that
  // our 4.3 stack will always assume that if it is not in the middle of
  // key establishment, then it is the server.  In that case it will send the
  // Terminate message with the direction set 'server-to-client' even if
  // the request was also 'server-to-client'.
  return (((keyEstPartner.isInitiator
            == (cmd->direction == ZCL_DIRECTION_CLIENT_TO_SERVER))
           || (cmd->commandId == ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID))
          && keyEstPartner.isIntraPan == (cmd->interPanHeader == NULL)
          && (keyEstPartner.isIntraPan
              ? keyEstPartner.pan.intraPan.nodeId == cmd->source
              : ((cmd->interPanHeader->options
                  & EMBER_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS)
                 && (MEMCOMPARE(keyEstPartner.pan.interPan.eui64,
                                cmd->interPanHeader->longAddress,
                                EUI64_SIZE) == 0))));
}

static void writeKeyEstablishmentClusterAttribute(uint8_t endpoint)
{
  if (isCbkeKeyEstablishmentSuiteValid()) {
    uint16_t keSuiteId = emAfAvailableCbkeSuite;
    EmberAfStatus attrWriteStatus;

    // Make the 'mask' 16-bit so that we can bit shift the client mask (0x80)
    // and detect it is bigger than 0x80 so our for() loop terminates.
    uint16_t mask;
    for (mask = CLUSTER_MASK_SERVER; mask <= CLUSTER_MASK_CLIENT; mask <<= 1) {
      attrWriteStatus = emberAfWriteAttribute(endpoint,
                                              ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                              // Conveniently the server and client attribute
                                              // ID are the same.  So we pass just the server ID.
                                              ZCL_KEY_ESTABLISHMENT_SUITE_SERVER_ATTRIBUTE_ID,
                                              (uint8_t)mask,
                                              (uint8_t*)&keSuiteId,
                                              ZCL_ENUM16_ATTRIBUTE_TYPE);
      if (attrWriteStatus != EMBER_ZCL_STATUS_SUCCESS) {
        emberAfKeyEstablishmentClusterPrintln("%p%p%p%X",
                                              "Key Est. Suite attr write failed (",
                                              ((mask == CLUSTER_MASK_SERVER)
                                               ? "server"
                                               : "client"),
                                              "): 0x",
                                              attrWriteStatus);
      }
    }
  }
}
//------------------------------------------------------------------------------
// DEBUG

#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_KEY_ESTABLISHMENT_CLUSTER)
static void debugPrintSmac(bool initiatorSmac, uint8_t *smac)
{
  emberAfKeyEstablishmentClusterPrintln("%p SMAC",
                                        (initiatorSmac
                                         ? "Initiator"
                                         : "Responder"));
  // Conveniently Zigbee Keys are the same length as the SMAC
  emberAfPrintZigbeeKey(smac);
}

static void debugPrintOtherSmac(bool received, uint8_t *smac)
{
  emberAfKeyEstablishmentClusterPrintln("%p SMAC",
                                        (received ? "Received" : "Calculated"));
  // Conveniently Zigbee Keys are the same length as the SMAC
  emberAfPrintZigbeeKey(smac);
}

static void debugPrintCert(bool initiatorCert, uint8_t *cert)
{
  emberAfKeyEstablishmentClusterPrintln("%p Cert",
                                        (initiatorCert
                                         ? "Initiator"
                                         : "Responder"));
  if (isCbkeKeyEstablishmentSuite163k1()) {
    emberAfPrintCert(cert);
  } else if (isCbkeKeyEstablishmentSuite283k1()) {
    emberAfPrintCert283k1(cert);
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }
}

static void debugPrintKey(bool initiatorKey, uint8_t *key)
{
  emberAfKeyEstablishmentClusterPrintln("%p public key",
                                        (initiatorKey
                                         ? "Initiator"
                                         : "Responder"));
  if (isCbkeKeyEstablishmentSuite163k1()) {
    emberAfPrintPublicKey(key);
  } else if (isCbkeKeyEstablishmentSuite283k1()) {
    emberAfPrintKey283k1(true, key);
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }
}
#endif

static EmberStatus initiateKeyEstablishment(const EmberEUI64 eui64,
                                            uint16_t nodeIdOrPanId,
                                            uint8_t endpoint)
{
  uint8_t validLastEvent = NO_KEY_ESTABLISHMENT_EVENT;
  if (isCbkeKeyEstablishmentSuiteValid() && lastEvent == NO_KEY_ESTABLISHMENT_EVENT) {
    keyEstablishmentEndpoint = emberAfPrimaryEndpointForCurrentNetworkIndex();
    if (keyEstablishmentEndpoint == 0xFF) {
      return EMBER_INVALID_CALL;
    }
    keyEstPartner.isInitiator = false;
    //emberAfKeyEstablishmentClusterPrintln("keyEstPartner.isInitiator initiateKeyEstablishment %u",keyEstPartner.isInitiator);
    keyEstPartner.isIntraPan = (eui64 == NULL);
    if (keyEstPartner.isIntraPan) {
      keyEstPartner.pan.intraPan.nodeId = nodeIdOrPanId;
      keyEstPartner.pan.intraPan.endpoint = endpoint;
    } else {
      keyEstPartner.pan.interPan.panId = nodeIdOrPanId;
      MEMMOVE(keyEstPartner.pan.interPan.eui64, eui64, EUI64_SIZE);
    }
    keyEstPartner.sequenceNumber = emberAfNextSequence();
    apsSequenceNumbersReceived = 0;

    // For inter-PAN key establishment, we don't read the supported curves and
    // use the support that the user specified for the image
    if (!keyEstPartner.isIntraPan) {
      lastEvent = CHECK_SUPPORTED_CURVES;
      validLastEvent = CHECK_SUPPORTED_CURVES;

      EmberAfCbkeKeyEstablishmentSuite suite;
      if ((emAfAvailableCbkeSuite & EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1) != 0U) {
        suite = EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1;
      } else if ((emAfAvailableCbkeSuite & EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1) != 0U) {
        suite = EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1;
      } else {
        return EMBER_ERR_FATAL;
      }

      emberAfKeyEstablishmentClusterPrintln(
        "Skipping read attributes of supported ECC curves for inter-PAN"
        " (using %sk1)",
        (EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1 == suite) ? "283" : "163");

      emAfKeyEstablishmentSelectCurve(suite);
      keyEstablishStateMachine(BEGIN_KEY_ESTABLISHMENT, NULL, NULL);
      return ((lastEvent == validLastEvent)
              ? EMBER_ERR_FATAL
              : EMBER_SUCCESS);
    }

    // The Test Harness can skip the read attributes stage. This is useful
    // in running curve specific key establishment tests, irrespective of
    // which binaries are supported.
    if (emUseTestHarnessSuite == EMBER_AF_INVALID_KEY_ESTABLISHMENT_SUITE) {
      keyEstablishStateMachine(CHECK_SUPPORTED_CURVES, NULL, NULL);
    } else {
      lastEvent = CHECK_SUPPORTED_CURVES;
      validLastEvent = CHECK_SUPPORTED_CURVES;
      emAfKeyEstablishmentSelectCurve(emUseTestHarnessSuite);
      keyEstablishStateMachine(BEGIN_KEY_ESTABLISHMENT, NULL, NULL);
    }

    return ((lastEvent == validLastEvent)
            ? EMBER_ERR_FATAL
            : EMBER_SUCCESS);
  }
  return EMBER_INVALID_CALL;
}

static bool checkMalformed283k1Command(bool isCertificate)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();
  uint8_t validSize = isCertificate
                      ? INITIATE_KEY_ESTABLISHMENT_LENGTH_283K1
                      : EMBER_PUBLIC_KEY_283K1_SIZE;
  if (isCbkeKeyEstablishmentSuite283k1()) {
    emberAfKeyEstablishmentClusterPrintln("Checking size Certificate size %u, expected %u", cmd->bufLen - cmd->payloadStartIndex, validSize);
    if (cmd->bufLen < cmd->payloadStartIndex + validSize) {
      emberAfKeyEstablishmentClusterPrintln(
        "Invalid length for KE command: %d (expected: %d, actual: %d)",
        cmd->commandId,
        emAfKeyEstablishMessageToDataSize[cmd->commandId + cbkeSuiteOffset],
        (cmd->bufLen - cmd->payloadStartIndex));

      cleanupAndStop(INVALID_PARTNER_MESSAGE);
      return true;
    }
  }
  return false;
}

static EmberAfStatus validateNewCbkeSuite(uint8_t endpoint,
                                          uint16_t newCbkeSuite)
{
  if (newCbkeSuite <= emberAfIsFullSmartEnergySecurityPresent()) {
    //emAfAvailableCbkeSuite = newCbkeSuite;
    return EMBER_ZCL_STATUS_SUCCESS;
  } else {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }
}
//-----------------------------------------------------------------------------
//Helper functions. These get called from key-establishment-curve-support.c

void cleanupAndStopWithDelay(EmberAfKeyEstablishmentNotifyMessage message,
                             uint8_t delayInSec)
{
  EmberAfAmiKeyEstablishmentStatus status;
  bool linkKeyEstablished = (message == LINK_KEY_ESTABLISHED);
  EmberStatus storeLinkKeyStatus = emberClearTemporaryDataMaybeStoreLinkKeyForCurve(linkKeyEstablished);

  scriptTestCheckpoint("CBKE: link key %s",
                       linkKeyEstablished ? "established" : "failure");

  if (delayInSec == 0) {
    delayInSec = INTERNAL_ERROR_BACK_OFF_TIME;
  }

  emberAfKeyEstablishmentClusterFlush();
  emberAfKeyEstablishmentClusterPrintln("End Key Establishment Status: 0x%x, Store Link Key Status: 0x%x",
                                        message,
                                        storeLinkKeyStatus);

  if (linkKeyEstablished && storeLinkKeyStatus != EMBER_SUCCESS) {
    message = NO_LOCAL_RESOURCES;
  }

  status = appNotifyMessageToKeyEstablishStatus[message];

  // This is a notify because either we are terminating key establishment
  // prematurely, or it succeeded.
  askApplicationWithDelay(message, delayInSec);

  if (!keyEstPartner.isInitiator && linkKeyEstablished) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  } else if (status != EMBER_ZCL_AMI_KEY_ESTABLISHMENT_STATUS_SUCCESS
             && message != PARTNER_SENT_DEFAULT_RESPONSE_ERROR
             && message != BAD_KEY_ESTABLISHMENT_SUITE
             && (keyEstPartner.isInitiator || lastEvent != NO_KEY_ESTABLISHMENT_EVENT)) {
    // No point in sending a terminate when this is the first step and we are
    // the initiator.
    sendTerminateMessage(status, BACK_OFF_TIME_REPORTED_TO_PARTNER);
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }

  clearKeyEstablishmentState();
}

void sendNextKeyEstablishMessage(KeyEstablishMessage message,
                                 uint8_t *data)
{
  uint8_t *ptr = appResponseData + EMBER_AF_ZCL_OVERHEAD;
  uint8_t size = emAfKeyEstablishMessageToDataSize[message + cbkeSuiteOffset];
  bool certMessage = (message == ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID);
  if (certMessage) {
    *ptr++ = LOW_BYTE(emAfCurrentCbkeSuite);
    *ptr++ = HIGH_BYTE(emAfCurrentCbkeSuite);
    *ptr++ = EM_AF_ADVERTISED_EPHEMERAL_DATA_GEN_TIME_SECONDS;
    *ptr++ = GENERATE_SHARED_SECRET_TIME_SECONDS;
    size -= 4;  // reduce the size for the 4 bytes we already added
  }
  MEMMOVE(ptr, data, size);
  ptr += size;

  appResponseLength = (ptr - appResponseData);
  sendKeyEstablishMessage(message);
}

//-----------------------------------------------------------------------------
//Helper functions for the test-harness.

void emAfSkipCheckSupportedCurves(EmberAfCbkeKeyEstablishmentSuite suite)
{
  emUseTestHarnessSuite = suite;
}

void emAfSetAvailableCurves(EmberAfCbkeKeyEstablishmentSuite suite)
{
  emAfAvailableCbkeSuite = suite;
  writeKeyEstablishmentClusterAttribute(keyEstablishmentEndpoint);
}

void emAfKeyEstablishmentSelectCurve(EmberAfCbkeKeyEstablishmentSuite suite)
{
  emAfCurrentCbkeSuite   = suite;
  if (suite == EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1) {
    cbkeSuiteOffset   = CBKE_163K1_SUITE_OFFSET;
    certSubjectOffset = CERT_SUBJECT_OFFSET;
    certIssuerOffset  = CERT_ISSUER_OFFSET;
  } else if (suite == EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1) {
    cbkeSuiteOffset   = CBKE_283K1_SUITE_OFFSET;
    certSubjectOffset = CERT_SUBJECT_OFFSET_283K1;
    certIssuerOffset  = CERT_ISSUER_OFFSET_283K1;
  } else {
    // MISRA requires ..else if.. to have terminating else.
  }
}

//-----------------------------------------------------------------------------
EmberAfStatus emberAfKeyEstablishmentClusterClientPreAttributeChangedCallback(uint8_t endpoint,
                                                                              EmberAfAttributeId attributeId,
                                                                              EmberAfAttributeType attributeType,
                                                                              uint8_t size,
                                                                              uint8_t *value)
{
  return emberAfKeyEstablishmentClusterServerPreAttributeChangedCallback(endpoint,
                                                                         attributeId,
                                                                         attributeType,
                                                                         size,
                                                                         value);
}

EmberAfStatus emberAfKeyEstablishmentClusterServerPreAttributeChangedCallback(uint8_t endpoint,
                                                                              EmberAfAttributeId attributeId,
                                                                              EmberAfAttributeType attributeType,
                                                                              uint8_t size,
                                                                              uint8_t *value)
{
  switch (attributeId) {
    case ZCL_KEY_ESTABLISHMENT_SUITE_SERVER_ATTRIBUTE_ID:
    {
      uint16_t newCbkeSuite;
      MEMMOVE(&newCbkeSuite, value, size);
      return validateNewCbkeSuite(endpoint, newCbkeSuite);
    }
    default:
      return EMBER_ZCL_STATUS_SUCCESS;
  }
}

void emberAfKeyEstablishmentClusterClientAttributeChangedCallback(uint8_t endpoint,
                                                                  EmberAfAttributeId attributeId)
{
  uint16_t availableSuites;
  uint16_t serverSuites;

  EmberAfStatus status = emberAfReadClientAttribute(endpoint,
                                                    ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                                    attributeId,
                                                    (uint8_t *)&availableSuites,
                                                    sizeof(availableSuites));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Reading KE client attribute failed");
    return;
  }

  emAfAvailableCbkeSuite = availableSuites;

  status = emberAfReadServerAttribute(endpoint,
                                      ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                      attributeId,
                                      (uint8_t *)&serverSuites,
                                      sizeof(serverSuites));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Reading KE server attribute failed");
    return;
  }

  if (availableSuites != serverSuites) {
    status = emberAfWriteServerAttribute(endpoint,
                                         ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                         attributeId,
                                         (uint8_t *)&availableSuites,
                                         ZCL_ENUM16_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      emberAfCorePrintln("Updating KE server status failed");
    }
  }
}

void emberAfKeyEstablishmentClusterServerAttributeChangedCallback(uint8_t endpoint,
                                                                  EmberAfAttributeId attributeId)
{
  uint16_t clientSuites;
  uint16_t availableSuites;
  EmberAfStatus status = emberAfReadServerAttribute(endpoint,
                                                    ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                                    attributeId,
                                                    (uint8_t *)&availableSuites,
                                                    sizeof(availableSuites));
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Reading server attribute failed");
    return;
  }

  emAfAvailableCbkeSuite = availableSuites;

  status = emberAfReadClientAttribute(endpoint,
                                      ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                      attributeId,
                                      (uint8_t *)&clientSuites,
                                      sizeof(clientSuites));

  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfCorePrintln("Reading KE client attribute failed");
    return;
  }
  if (availableSuites != clientSuites) {
    status = emberAfWriteClientAttribute(endpoint,
                                         ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
                                         attributeId,
                                         (uint8_t *)&availableSuites,
                                         ZCL_ENUM16_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      emberAfCorePrintln("Updating server status failed");
    }

    //emberAfCorePrintln("Upading client suite to %u",availableSuites);
  }
}

void emberAfPluginKeyEstablishmentReadAttributesCallback(EmberAfCbkeKeyEstablishmentSuite suite)
{
  emberAfKeyEstablishmentClusterPrintln("Supported Suites %u", suite);
  emberAfKeyEstablishmentClusterPrintln("keyEstPartner.isInitiator emberAfPluginKeyEstablishmentReadAttributesCallback %u", keyEstPartner.isInitiator);
  if (!keyEstPartner.isInitiator) {
    //We identify the highest available suite to use.
    if ((suite & emAfAvailableCbkeSuite)
        >= EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1) {
      emberAfKeyEstablishmentClusterPrintln("Using cbke-283k1");
      emAfKeyEstablishmentSelectCurve(EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1);
    } else if ((suite & emAfAvailableCbkeSuite)
               <= EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1) {
      //We accept 0x00(EMBER_AF_INVALID_KEY_ESTABLISHMENT_SUITE) as an acceptable value for
      //163k1 KE because our stacks 4.6 and older don't initialize the KE attribute to 1.
      emberAfKeyEstablishmentClusterPrintln("Using cbke-163k1");
      emAfKeyEstablishmentSelectCurve(EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1);
    } else {
      emberAfKeyEstablishmentClusterPrintln("Error: No valid Cluster");
      cleanupAndStop(BAD_KEY_ESTABLISHMENT_SUITE);
      return;
    }
    keyEstablishStateMachine(BEGIN_KEY_ESTABLISHMENT, NULL, NULL);
  }
}

EmberStatus emberAfInitiateKeyEstablishmentCallback(EmberNodeId nodeId,
                                                    uint8_t endpoint)
{
  return initiateKeyEstablishment(NULL, // eui64 - unused
                                  nodeId,
                                  endpoint);
}

EmberStatus emberAfInitiateInterPanKeyEstablishmentCallback(EmberPanId panId,
                                                            const EmberEUI64 eui64)
{
  return initiateKeyEstablishment(eui64,
                                  panId,
                                  0); // endpoint - unused
}

bool emberAfPerformingKeyEstablishmentCallback(void)
{
  return (lastEvent != NO_KEY_ESTABLISHMENT_EVENT);
}

void emberAfKeyEstablishmentClusterServerInitCallback(uint8_t endpoint)
{
  clearKeyEstablishmentState();
  setupEventTimeouts(0, 0);
  keyEstablishmentEndpoint = endpoint;

//If the test harness is selected, it is expected to force the selection of the ECC 283k1 curve.
//By default, all variables use the ECC 163k1 curve.
  emAfAvailableCbkeSuite = emberAfIsFullSmartEnergySecurityPresent();

  if (emAfCurrentCbkeSuite == EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1) {
    cbkeSuiteOffset   = CBKE_283K1_SUITE_OFFSET;
    certSubjectOffset = CERT_SUBJECT_OFFSET_283K1;
    certIssuerOffset  = CERT_ISSUER_OFFSET_283K1;
  }

  // We use a "core" print in hopes that this message will be seen.
  // Key establishment will not work and mysteriously fail when started.
  emberAfCorePrintln("Key Est. Init %p 0x%x", (isCbkeKeyEstablishmentSuiteValid() ? "Success" : "FAILED!"), LOW_BYTE(emAfAvailableCbkeSuite));
  emberAfKeyEstablishmentClusterPrintln("Key Est. Init %p",
                                        (isCbkeKeyEstablishmentSuiteValid()
                                         ? "Success"
                                         : "FAILED!"));

  // (Case 14692) If we do have full SE security, indicate the proper KE suite
  // in the K.E. Suite attribute for the appropriate endpoint.
  // NOTE: We also take this opportunity to set the client-side KE Suite
  // attribute value to match.  This is really the KEClusterClientInitCallback's
  // job, but since this plugin is a really a package deal (for client+server)
  // and since the values need to match anyway, we do it here.
  // Implementing the KEClusterClientInitCallback just for this setting
  // seems like overkill anyway and would consume more code space.
  writeKeyEstablishmentClusterAttribute(endpoint);
}

void emberAfKeyEstablishmentClusterServerTickCallback(uint8_t endpoint)
{
  cleanupAndStop(TIMEOUT_OCCURRED);
}

bool emberAfKeyEstablishmentClusterServerCommandReceivedCallback(EmberAfClusterCommand *cmd)
{
  return commandReceivedHandler(cmd);
}

bool emberAfKeyEstablishmentClusterInitiateKeyEstablishmentRequestCallback(uint16_t keyEstablishmentSuite,
                                                                           uint8_t ephemeralDataGenerateTime,
                                                                           uint8_t confirmKeyGenerateTime,
                                                                           uint8_t *identity)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();
  emberAfKeyEstablishmentClusterPrintln("Suite %u\r\n", keyEstablishmentSuite);
  emAfKeyEstablishmentSelectCurve(keyEstablishmentSuite);
  if (checkMalformed283k1Command(true)) {
    return true;
  }
  keyEstablishStateMachine(BEGIN_KEY_ESTABLISHMENT,
                           cmd->buffer + cmd->payloadStartIndex,
                           identity);
  return true;
}

bool emberAfKeyEstablishmentClusterEphemeralDataRequestCallback(uint8_t *ephemeralData)
{
  if (checkMalformed283k1Command(false)) {
    return true;
  }
  keyEstablishStateMachine(GENERATE_KEYS, ephemeralData, NULL);
  return true;
}

bool emberAfKeyEstablishmentClusterConfirmKeyDataRequestCallback(uint8_t *secureMessageAuthenticationCode)
{
  keyEstablishStateMachine(GENERATE_SHARED_SECRET,
                           secureMessageAuthenticationCode,
                           NULL);
  return true;
}

bool emberAfKeyEstablishmentClusterTerminateKeyEstablishmentCallback(uint8_t statusCode,
                                                                     uint8_t waitTime,
                                                                     uint16_t keyEstablishmentSuite)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();
  if (cmd != NULL && cmd->type >= EMBER_INCOMING_MULTICAST) {
    emberAfKeyEstablishmentClusterPrintln("Ignoring Broadcast KE terminate");
    return true;
  }

  if (lastEvent != NO_KEY_ESTABLISHMENT_EVENT && cmd != NULL && commandIsFromOurPartner(cmd)) {
    if (keyEstPartner.isIntraPan) {
      apsSequenceNumbers[apsSequenceNumbersReceived] = cmd->apsFrame->sequence;
      apsSequenceNumbersReceived++;
    }
    emberAfKeyEstablishmentClusterPrintln("Terminate Received, Status(%d): %p",
                                          statusCode,
                                          terminateStatus[((statusCode > UNKNOWN_TERMINATE_STATUS)
                                                           ? UNKNOWN_TERMINATE_STATUS
                                                           : statusCode)]);
    cleanupAndStopWithDelay(PARTNER_SENT_TERMINATE, waitTime);
  }
  emberAfSendDefaultResponse(cmd, EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

void emberAfKeyEstablishmentClusterServerMessageSentCallback(EmberOutgoingMessageType type,
                                                             uint16_t indexOrDestination,
                                                             EmberApsFrame *apsFrame,
                                                             uint16_t msgLen,
                                                             uint8_t *message,
                                                             EmberStatus status)
{
  messageSentHandler(type, indexOrDestination, apsFrame, msgLen, message, status);
}

static void defaultResponseCallback(uint8_t endpoint,
                                    uint8_t commandId,
                                    EmberAfStatus status)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();

  if (cmd != NULL && cmd->type >= EMBER_INCOMING_MULTICAST) {
    emberAfKeyEstablishmentClusterPrintln("Ignoring Broadcast KE default resp");
    return;
  }

  if (status != EMBER_ZCL_STATUS_SUCCESS
      && lastEvent != NO_KEY_ESTABLISHMENT_EVENT
      && cmd != NULL
      && commandIsFromOurPartner(cmd)) {
    emberAfKeyEstablishmentClusterPrintln("Got Default Response with error code: %d",
                                          status);
    // While the actual status code may be more meaningful, we don't really care.
    // We interpret it as a transient failure.  Otherwise we would have got an
    // explicit Terminate Message.
    cleanupAndStop(PARTNER_SENT_DEFAULT_RESPONSE_ERROR);
  }
}

void emberAfKeyEstablishmentClusterClientDefaultResponseCallback(uint8_t endpoint,
                                                                 uint8_t commandId,
                                                                 EmberAfStatus status)
{
  defaultResponseCallback(endpoint, commandId, status);
}

void emberAfKeyEstablishmentClusterServerDefaultResponseCallback(uint8_t endpoint,
                                                                 uint8_t commandId,
                                                                 EmberAfStatus status)
{
  defaultResponseCallback(endpoint, commandId, status);
}

bool emberAfKeyEstablishmentClusterClientCommandReceivedCallback(EmberAfClusterCommand *cmd)
{
  return commandReceivedHandler(cmd);
}

bool emberAfKeyEstablishmentClusterInitiateKeyEstablishmentResponseCallback(uint16_t requestedKeyEstablishmentSuite,
                                                                            uint8_t ephemeralDataGenerateTime,
                                                                            uint8_t confirmKeyGenerateTime,
                                                                            uint8_t *identity)
{
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();
  if (checkMalformed283k1Command(true)) {
    return true;
  }
  keyEstablishStateMachine(GENERATE_KEYS,
                           cmd->buffer + cmd->payloadStartIndex,
                           identity);
  return true;
}

bool emberAfKeyEstablishmentClusterEphemeralDataResponseCallback(uint8_t *ephemeralData)
{
  if (checkMalformed283k1Command(false)) {
    return true;
  }
  keyEstablishStateMachine(GENERATE_SHARED_SECRET, ephemeralData, NULL);
  return true;
}

bool emberAfKeyEstablishmentClusterConfirmKeyDataResponseCallback(uint8_t *secureMessageAuthenticationCode)
{
  keyEstablishStateMachine(INITIATOR_RECEIVED_CONFIRM_KEY,
                           secureMessageAuthenticationCode,
                           NULL);
  return true;
}

void emberAfKeyEstablishmentClusterClientMessageSentCallback(EmberOutgoingMessageType type,
                                                             uint16_t indexOrDestination,
                                                             EmberApsFrame *apsFrame,
                                                             uint16_t msgLen,
                                                             uint8_t *message,
                                                             EmberStatus status)
{
  messageSentHandler(type, indexOrDestination, apsFrame, msgLen, message, status);
}

//------------------------------------------------------------------------------
// CBKE Library Callbacks

void emAfPluginKeyEstablishmentGenerateCbkeKeysHandler(EmberStatus status,
                                                       EmberPublicKeyData *ephemeralPublicKey)
{
  emberAfKeyEstablishmentClusterPrintln("GenerateCbkeKeysHandler() returned: 0x%x",
                                        status);
  emAfCryptoOperationComplete();

  if (status != EMBER_SUCCESS) {
    cleanupAndStop(NO_LOCAL_RESOURCES);
    return;
  }

  if (emAfKeyEstablishmentTestHarnessCbkeCallback(CBKE_OPERATION_GENERATE_KEYS,
                                                  ephemeralPublicKey->contents,
                                                  NULL)) {
    return;
  }

  emberAfPushEndpointNetworkIndex(keyEstablishmentEndpoint);
  keyEstablishStateMachine(SEND_EPHEMERAL_DATA_MESSAGE,
                           emberPublicKeyContents(ephemeralPublicKey),
                           NULL);
  emberAfPopNetworkIndex();
}

void emAfPluginKeyEstablishmentCalculateSmacsHandler(EmberStatus status,
                                                     EmberSmacData *initiatorSmac,
                                                     EmberSmacData *responderSmac)
{
  emberAfKeyEstablishmentClusterPrintln("CalculateSmacsHandler() returned: 0x%x",
                                        status);
  emAfCryptoOperationComplete();
  debugPrintSmac(true, emberSmacContents(initiatorSmac));
  debugPrintSmac(false, emberSmacContents(responderSmac));

  if (status != EMBER_SUCCESS) {
    cleanupAndStop(NO_LOCAL_RESOURCES);
    return;
  }

  if (emAfKeyEstablishmentTestHarnessCbkeCallback(CBKE_OPERATION_GENERATE_SECRET,
                                                  initiatorSmac->contents,
                                                  responderSmac->contents)) {
    return;
  }

  emberAfPushEndpointNetworkIndex(keyEstablishmentEndpoint);
  keyEstablishStateMachine(SEND_CONFIRM_KEY_MESSAGE,
                           emberKeyContents(initiatorSmac),
                           emberKeyContents(responderSmac));
  emberAfPopNetworkIndex();
}

void emAfPluginKeyEstablishmentGenerateCbkeKeysHandler283k1(EmberStatus status,
                                                            EmberPublicKey283k1Data *ephemeralPublicKey)
{
  emberAfKeyEstablishmentClusterPrintln("GenerateCbkeKeysHandler283k1() returned: 0x%x",
                                        status);
  emAfCryptoOperationComplete();

  if (status != EMBER_SUCCESS) {
    cleanupAndStop(NO_LOCAL_RESOURCES);
    return;
  }

  emberAfPushEndpointNetworkIndex(keyEstablishmentEndpoint);
  keyEstablishStateMachine(SEND_EPHEMERAL_DATA_MESSAGE,
                           emberPublicKey283k1Contents(ephemeralPublicKey),
                           NULL);
  emberAfPopNetworkIndex();
}

void emAfPluginKeyEstablishmentCalculateSmacsHandler283k1(EmberStatus status,
                                                          EmberSmacData* initiatorSmac,
                                                          EmberSmacData* responderSmac)
{
  emberAfKeyEstablishmentClusterPrintln("CalculateSmacsHandler() returned: 0x%x",
                                        status);
  emAfCryptoOperationComplete();

  if (status != EMBER_SUCCESS) {
    cleanupAndStop(NO_LOCAL_RESOURCES);
    return;
  }

  emberAfPushEndpointNetworkIndex(keyEstablishmentEndpoint);
  keyEstablishStateMachine(SEND_CONFIRM_KEY_MESSAGE,
                           emberKeyContents(initiatorSmac),
                           emberKeyContents(responderSmac));
  emberAfPopNetworkIndex();
}

//------------------------------------------------------------------------------
// Event Handlers

void emberAfPluginKeyEstablishmentApsDuplicateDetectionEventHandler(
  EmberEventControl *control)
{
  if (!control) {
    return;
  }

  emberEventControlSetInactive(*control);

  lastNodeSuccessfulCbke = EMBER_NULL_NODE_ID;
}
