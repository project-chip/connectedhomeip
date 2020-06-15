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
 * @brief Unit test for Key Establishment plugin code.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "callback.h"
#include "app/framework/test/script/afv2-scripted.h"
#include "config/ember-configuration.c"
#include "app/framework/test/test-framework.h"
#include "app/framework/plugin/key-establishment/key-establishment.h"

extern void emAfPluginKeyEstablishmentGenerateCbkeKeysHandler283k1(EmberStatus status,
                                                                   EmberPublicKey283k1Data *ephemeralPublicKey);
extern void emAfPluginKeyEstablishmentCalculateSmacsHandler283k1(EmberStatus status,
                                                                 EmberSmacData* initiatorSmac,
                                                                 EmberSmacData* responderSmac);

#define APS_BUFFER_LEN                128
#define CBKE_ENDPOINT                 1
#define CBKE_GENERATE_TIME            10
#define CBKE_283K1_CERT_SUITE_OFFSET  9

// Used
uint8_t buffer[APS_BUFFER_LEN];
EmberApsFrame apsFrame;
EmberAfClusterCommand command;
EmberAfClusterCommand *emAfCurrentCommand = NULL;

uint8_t responderCertificateStatic[] = {
  //Type of certificate
  0x00,
  //Serial Number
  0x26, 0x22, 0xA5, 0x05, 0xE8, 0x93, 0x8F, 0x27,
  //Curve
  0x0D,
  //Hash Identifier
  0x08,
  //Issuer
  0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
  //Valid From
  0x00, 0x52, 0x92, 0xA3, 0x5B,
  //Valid Until
  0xFF, 0xFF, 0xFF, 0xFF,
  //Subject Id
  0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
  //Key Usage
  0x88,
  //Compressed Public Key
  0x03, 0x03, 0xB4, 0xE9, 0xDC, 0x54, 0x3A, 0x64,
  0x33, 0x3C, 0x98, 0x23, 0x08, 0x02, 0x2B, 0x54,
  0xE6, 0x7E, 0x2F, 0x15, 0xF5, 0x32, 0x55, 0x1B,
  0x0A, 0x11, 0xE2, 0xE2, 0xC1, 0xC1, 0xD3, 0x09,
  0x7A, 0x43, 0x24, 0xE7, 0xED
};

// The contents of this don't matter
EmberPublicKey283k1Data ephemeralPublicKey = {
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
  0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
  0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
  0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
  0x21, 0x22, 0x23, 0x24, 0x25
};

EmberSmacData smacData = {
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
  0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10
};

extern EmberEventControl emberAfPluginKeyEstablishmentApsDuplicateDetectionEventControl;
extern void emberAfPluginKeyEstablishmentApsDuplicateDetectionEventHandler(void);

EmberEventData keyEstablishmentEvents[] = {
  { &emberAfPluginKeyEstablishmentApsDuplicateDetectionEventControl, emberAfPluginKeyEstablishmentApsDuplicateDetectionEventHandler },
  { NULL, NULL }
};

// Stubs
typedef uint8_t EmAfCryptoStatus;

EmberStatus halCommonIdleForMilliseconds(uint32_t *duration)
{
  return EMBER_SUCCESS;
}
void emCallCounterHandler(EmberCounterType type, uint8_t data)
{
}
void emBuildAndSendCounterInfo(EmberCounterType counter, EmberNodeId dst, uint8_t data)
{
}

uint8_t emberAfPrimaryEndpointForCurrentNetworkIndex(void)
{
  return 1;
}

EmberStatus emberAfSendUnicast(EmberOutgoingMessageType type,
                               uint16_t indexOrDestination,
                               EmberApsFrame *apsFrame,
                               uint16_t messageLength,
                               uint8_t* message)
{
  return EMBER_SUCCESS;
}

EmberStatus emberAfSendImmediateDefaultResponse(EmberAfStatus status)
{
  return EMBER_SUCCESS;
}

EmberAfCbkeKeyEstablishmentSuite emberAfIsFullSmartEnergySecurityPresent(void)
{
  return EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_163K1 | EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1;
}

void emAfSetCryptoStatus(EmAfCryptoStatus newStatus)
{
}

uint8_t emberFindKeyTableEntry(EmberEUI64 address, bool linkKey)
{
  return 0xFF;
}

EmberAfStatus emberAfReadClientAttribute(uint8_t endpoint,
                                         EmberAfClusterId cluster,
                                         EmberAfAttributeId attributeID,
                                         uint8_t* dataPtr,
                                         uint8_t readLength)
{
  return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfReadServerAttribute(uint8_t endpoint,
                                         EmberAfClusterId cluster,
                                         EmberAfAttributeId attributeID,
                                         uint8_t *dataPtr,
                                         uint8_t readLength)
{
  return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfWriteClientAttribute(uint8_t endpoint,
                                          EmberAfClusterId cluster,
                                          EmberAfAttributeId attributeID,
                                          uint8_t* dataPtr,
                                          EmberAfAttributeType dataType)
{
  return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfWriteServerAttribute(uint8_t endpoint,
                                          EmberAfClusterId cluster,
                                          EmberAfAttributeId attributeID,
                                          uint8_t *dataPtr,
                                          uint8_t dataType)
{
  return EMBER_ZCL_STATUS_SUCCESS;
}

bool emberAfKeyEstablishmentCallback(EmberAfKeyEstablishmentNotifyMessage status,
                                     bool amInitiator,
                                     EmberNodeId partnerShortId,
                                     uint8_t delayInSeconds)
{
  return true;
}

bool emberAfInterPanKeyEstablishmentCallback(EmberAfKeyEstablishmentNotifyMessage status,
                                             bool amInitiator,
                                             EmberPanId panId,
                                             const EmberEUI64 eui64,
                                             uint8_t delayInSeconds)
{
  return true;
}

EmberAfStatus emberAfKeyEstablishmentClusterClientCommandParse(EmberAfClusterCommand *cmd)
{
  return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfKeyEstablishmentClusterServerCommandParse(EmberAfClusterCommand *cmd)
{
  return EMBER_ZCL_STATUS_SUCCESS;
}

bool checkIssuer(uint8_t *issuer)
{
  return true;
}

EmberStatus emCalculateSmacsForCurve(bool amInitiator,
                                     EmberCertificate283k1Data* partnerCert,
                                     EmberPublicKey283k1Data* partnerEphemeralPublicKey)
{
  // This is actually the only return value we want in the plugin code, not
  // EMBER_SUCCESS
  return EMBER_OPERATION_IN_PROGRESS;
}

EmberStatus emGenerateCbkeKeysForCurve(void)
{
  // This is actually the only return value we want in the plugin code, not
  // EMBER_SUCCESS
  return EMBER_OPERATION_IN_PROGRESS;
}

EmberStatus emberClearTemporaryDataMaybeStoreLinkKeyForCurve(bool storeLinkKey)
{
  return EMBER_SUCCESS;
}

bool retrieveAndClearPublicPartnerData(EmberCertificate283k1Data* partnerCertificate,
                                       EmberPublicKey283k1Data* partnerEphemeralPublicKey)
{
  return true;
}

EmberAfKeyEstablishmentNotifyMessage sendCertificate(void)
{
  return NO_APP_MESSAGE;
}

bool storePublicPartnerData(bool isCertificate,
                            uint8_t* data)
{
  return true;
}

void clearAllTemporaryPublicData(void)
{
}

bool getSmacPointer(EmberSmacData** smacPtr)
{
  if (smacPtr) {
    *smacPtr = &smacData;
  }
  return true;
}

bool storeSmac(EmberSmacData* smac)
{
  return true;
}

void scriptTickCallback(void)
{
  emberRunEvents(keyEstablishmentEvents);
}

static uint32_t systemTick = 0;

uint32_t scriptTime(void)
{
  return systemTick;
}

void scriptTick(void)
{
  systemTick += 1;
  microSetSystemTime(systemTick);
  scriptTickCallback();
}

EmberStatus emberAfPushEndpointNetworkIndex(uint8_t endpoint)
{
  (void)endpoint;
  return EMBER_SUCCESS;
}

// Test functions

static void cbkeApsDuplicationTest(void)
{
  command.type = EMBER_INCOMING_UNICAST;
  command.source = 0x0000;
  command.apsFrame = &apsFrame;
  command.direction = ZCL_DIRECTION_SERVER_TO_CLIENT;
  command.buffer = buffer;
  command.interPanHeader = NULL;
  // The following becomes relevant when we simulate an
  // InitiateKeyEstablishmentResponse. For the other messages it doesn't matter.
  command.payloadStartIndex = CBKE_283K1_CERT_SUITE_OFFSET;
  command.buffer = responderCertificateStatic;
  command.bufLen = APS_BUFFER_LEN;

  emAfSetAvailableCurves(EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1);
  emAfKeyEstablishmentSelectCurve(EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1);

  // Perform a successful CBKE exchange

  // NULL cmd on CHECK_SUPPORTED_CURVES
  addSimpleAction("emberAfInitiateKeyEstablishmentCallback(%d,%d)",
                  emberAfInitiateKeyEstablishmentCallback,
                  2,
                  0x0000,
                  CBKE_ENDPOINT);
  addCheckpointTest("CBKE: check supported curves");
  runScript();

  // Valid cmd on BEGIN_KEY_ESTABLISHMENT
  emAfCurrentCommand = &command;
  addSimpleAction("emberAfPluginKeyEstablishmentReadAttributesCallback(%d)",
                  emberAfPluginKeyEstablishmentReadAttributesCallback,
                  1,
                  EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1);
  addCheckpointTest("CBKE: sent CBKE certificate");
  runScript();

  // Valid cmd on GENERATE_KEYS
  command.apsFrame->sequence++;
  addSimpleAction("emberAfKeyEstablishmentClusterInitiateKeyEstablishmentResponseCallback(%d,%d,%d,%p)",
                  emberAfKeyEstablishmentClusterInitiateKeyEstablishmentResponseCallback,
                  4,
                  EMBER_AF_CBKE_KEY_ESTABLISHMENT_SUITE_283K1,
                  CBKE_GENERATE_TIME,
                  CBKE_GENERATE_TIME,
                  responderCertificateStatic);
  addCheckpointTest("CBKE: beginning crypto operation");
  runScript();

  // NULL cmd on SEND_EPHEMERAL_DATA_MESSAGE
  emAfCurrentCommand = NULL;
  addSimpleAction("emAfPluginKeyEstablishmentGenerateCbkeKeysHandler283k1(%d,%p)",
                  emAfPluginKeyEstablishmentGenerateCbkeKeysHandler283k1,
                  2,
                  EMBER_SUCCESS,
                  &ephemeralPublicKey);
  addCheckpointTest("CBKE: sending ephemeral data");
  runScript();

  // Valid cmd on GENERATE_SHARED_SECRET
  command.apsFrame->sequence++;
  emAfCurrentCommand = &command;
  addSimpleAction("emberAfKeyEstablishmentClusterEphemeralDataResponseCallback(%p)",
                  emberAfKeyEstablishmentClusterEphemeralDataResponseCallback,
                  1,
                  buffer);
  addCheckpointTest("CBKE: begin crypto operation from ephemeral data");
  runScript();

  // NULL cmd on SEND_CONFIRM_KEY_MESSAGE
  emAfCurrentCommand = NULL;
  addSimpleAction("emAfPluginKeyEstablishmentCalculateSmacsHandler283k1(%d,%p,%p)",
                  emAfPluginKeyEstablishmentCalculateSmacsHandler283k1,
                  3,
                  EMBER_SUCCESS,
                  &smacData,
                  &smacData);
  addCheckpointTest("CBKE: sent confirm key data");
  runScript();

  // Valid cmd on INITIATOR_RECEIVED_CONFIRM_KEY
  command.apsFrame->sequence++;
  emAfCurrentCommand = &command;
  addSimpleAction("emberAfKeyEstablishmentClusterConfirmKeyDataResponseCallback(%p)",
                  emberAfKeyEstablishmentClusterConfirmKeyDataResponseCallback,
                  1,
                  (uint8_t*)&smacData);
  addCheckpointTest("CBKE: link key established");
  runScript();

  // Now send another INITIATOR_RECEIVED_CONFIRM_KEY with the same APS sequence
  addSimpleAction("emberAfKeyEstablishmentClusterConfirmKeyDataResponseCallback(%p)",
                  emberAfKeyEstablishmentClusterConfirmKeyDataResponseCallback,
                  1,
                  (uint8_t*)&smacData);
  addCheckpointTest("Key Establishment dropped duplicate ConfirmKeyData frame");
  runScript();

  // Now wait for half of the timeout for duplication detection
  addRunAction((KEY_ESTABLISHMENT_APS_DUPLICATE_DETECTION_TIMEOUT_SEC / 2
                * MILLISECOND_TICKS_PER_SECOND) + MILLISECOND_TICKS_PER_SECOND);
  runScript();

  // Now send the same INITIATOR_RECEIVED_CONFIRM_KEY again with same APS seq
  addSimpleAction("emberAfKeyEstablishmentClusterConfirmKeyDataResponseCallback(%p)",
                  emberAfKeyEstablishmentClusterConfirmKeyDataResponseCallback,
                  1,
                  (uint8_t*)&smacData);
  addCheckpointTest("Key Establishment dropped duplicate ConfirmKeyData frame");
  runScript();

  // Wait for the rest of the timeout (with some tolerance) to ensure the
  // duplication detection has timed out
  addRunAction((KEY_ESTABLISHMENT_APS_DUPLICATE_DETECTION_TIMEOUT_SEC / 2
                * MILLISECOND_TICKS_PER_SECOND) + MILLISECOND_TICKS_PER_SECOND);
  addRunAction(MILLISECOND_TICKS_PER_SECOND);
  runScript();

  // Send the same INITIATOR_RECEIVED_CONFIRM_KEY again with same APS sequence
  // This time, we should get a TerminateKeyEstablishment message
  addSimpleAction("emberAfKeyEstablishmentClusterConfirmKeyDataResponseCallback(%p)",
                  emberAfKeyEstablishmentClusterConfirmKeyDataResponseCallback,
                  1,
                  (uint8_t*)&smacData);
  addCheckpointTest("Got wrong message in CBKE sequence");
  addCheckpointTest("CBKE: link key failure");
  runScript();
}

int main(int argc, char* argv[])
{
  Test tests[] = {
    { "cbke-aps-duplication", cbkeApsDuplicationTest },
    { NULL, NULL },
  };

  // Set up an external buffer for filling ZCL commands (which test code doesn't
  // care about)
  emEmberTestSetExternalBuffer();

  Thunk test = parseTestArgument(argc, argv, tests);
  test();
  fprintf(stderr, " done]\n");

  return 0;
}
