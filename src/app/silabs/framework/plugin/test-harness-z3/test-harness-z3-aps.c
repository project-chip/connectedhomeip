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
 * @brief ZigBee 3.0 aps test harness functionality
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#include "stack/include/packet-buffer.h"

#include "test-harness-z3-core.h"

// -----------------------------------------------------------------------------
// Constants

#define APS_REQUEST_KEY_COMMAND (0x08)

#define APS_REQUEST_KEY_TYPE_NETWORK     (0x01)
#define APS_REQUEST_KEY_TYPE_APPLICATION (0x02)
#define APS_REQUEST_KEY_TYPE_TCLK        (0x04)
#define APS_REQUEST_REMOVE_DEVICE        (0x07)

#define APS_COMMAND_OPTION_NWK_ENCRYPTION (0x01)
#define APS_COMMAND_OPTION_APS_ENCRYPTION (0x02)

// -----------------------------------------------------------------------------
// Reset key

// This allows us to send a message out with a wrong key, and then reset to
// use the correct key soon afterward.

// If the APS encrypted transmission fails, then it will be decrypted and re-added
// to the APS queue. Therefore, the next time we encrypt the packet for
// transmission, the key will have been reset to the original. This is an issue.

#define RANDOM_KEY                                    \
  { {                                                 \
      0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, \
      0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, \
    } }

#define ZTT_INSTALL_CODE_KEY                          \
  { {                                                 \
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, \
      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, \
    } }

static EmberKeyData zigbeeAlliance09 = ZIGBEE_PROFILE_INTEROPERABILITY_LINK_KEY;
static EmberKeyData installCodeKey = ZTT_INSTALL_CODE_KEY;
static EmberKeyData randomKey = RANDOM_KEY;

#define KEY_TYPE_DEFAULT_TCLK (0)
#define KEY_TYPE_RANDOM       (1)
#define KEY_TYPE_INSTALL_CODE (2)
#define KEY_TYPE_MAX          ((sizeof(keys) / sizeof(keys[0])) - 1)

static EmberKeyData *keys[] = {
  &zigbeeAlliance09,
  &randomKey,
  &installCodeKey,
};

EmberEventControl emberAfPluginTestHarnessZ3ResetKeyEventControl;

static uint8_t mostRecentKeyIndex = 0xFF;

static uint32_t configApsRemoveDeviceConfigOptions = 0;

uint32_t emberGetTestHarnessConfigurationOptions(uint8_t commandType,
                                                 uint8_t commandId)
{
  switch (commandType) {
    case EMBER_ZIGBEE_COMMAND_TYPE_APS:
      if (commandId == APS_REQUEST_REMOVE_DEVICE) {
        return configApsRemoveDeviceConfigOptions;
      }
      break;
    default:
      break;
  }
  return 0;
}

void emberAfPluginTestHarnessZ3ResetKeyEventHandler(void)
{
  EmberStatus status;

  emberEventControlSetInactive(emberAfPluginTestHarnessZ3ResetKeyEventControl);

  // Remove the most recently added key.
  if (mostRecentKeyIndex != 0xFF) {
    status = emberEraseKeyTableEntry(mostRecentKeyIndex);
    emberAfCorePrintln("%p: %p: 0x%X",
                       TEST_HARNESS_Z3_PRINT_NAME,
                       "Reset key handler",
                       status);
    mostRecentKeyIndex = 0xFF;
  }
}

static EmberStatus setNextKey(EmberEUI64 partner, bool linkKey, uint8_t keyType)
{
  EmberStatus status;

  status = emberAddOrUpdateKeyTableEntry(partner,
                                         linkKey,
                                         (keyType > KEY_TYPE_MAX
                                          ? keys[KEY_TYPE_MAX]
                                          : keys[keyType]));

  emberAfCorePrintln("setNextKey: status = %X", status);

  mostRecentKeyIndex = emberFindKeyTableEntry(partner, true);
  emberEventControlSetActive(emberAfPluginTestHarnessZ3ResetKeyEventControl);

  return status;
}

// -----------------------------------------------------------------------------
// APS CLI Commands

#ifdef EZSP_HOST
  #define emberSendRemoveDevice(...) EMBER_INVALID_CALL
  #define emSendApsCommand(...)      EMBER_INVALID_CALL
#else

// Internal stack API.
extern bool emSendApsCommand(EmberNodeId destination,
                             EmberEUI64 longDestination,
                             EmberMessageBuffer payload,
                             uint8_t options);

#endif /* EZSP_HOST */

// plugin test-harness z3 aps aps-remove-device <parentLong:8> <dstLong:8> <options:4>
void emAfPluginTestHarnessZ3ApsApsRemoveDevice(void)
{
  EmberEUI64 parentLong, targetLong;
  EmberNodeId parentShort;
  EmberStatus status = EMBER_SUCCESS;
  uint32_t options = emAfPluginTestHarnessZ3GetSignificantBit(2);

  emberCopyBigEndianEui64Argument(0, parentLong);
  emberCopyBigEndianEui64Argument(1, targetLong);

  switch (options) {
    case BIT32(0):
      status = setNextKey(targetLong, true, KEY_TYPE_DEFAULT_TCLK);
      break;

    case BIT32(1):
      // This is our own hook so we can test the next key functionality.
      status = setNextKey(targetLong, true, KEY_TYPE_RANDOM);
      break;

    default:
      ; // meh
  }

  if (status == EMBER_SUCCESS) {
    parentShort = emberLookupNodeIdByEui64(parentLong);
    if (parentShort == EMBER_NULL_NODE_ID) {
      status = EMBER_BAD_ARGUMENT;
    } else {
      status = emberSendRemoveDevice(parentShort, parentLong, targetLong);
    }
  }

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Remove device",
                     status);
}

void emAfPluginTestHarnessZ3ApsApsRemoveDeviceConfig(void)
{
  configApsRemoveDeviceConfigOptions = emAfPluginTestHarnessZ3GetSignificantBit(0);
  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Remove device configuration",
                     0);
}

// plugin test-harness z3 aps aps-request-key <dstShort:2> <keyType:1>
// <parentLong:8> <options:4>
void emAfPluginTestHarnessZ3ApsApsRequestKeyCommand(void)
{
  EmberStatus status = EMBER_INVALID_CALL;
#ifndef EZSP_HOST
  EmberNodeId destShort = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t keyType       = (uint8_t)emberUnsignedCommandArgument(1);
  uint32_t options      = emAfPluginTestHarnessZ3GetSignificantBit(3);
  EmberEUI64 partnerLong, trustCenterEui64;
  uint8_t frame[10];
  uint8_t *finger = &frame[0];
  EmberMessageBuffer commandBuffer;
  uint8_t apsCommandOptions;

  emberCopyBigEndianEui64Argument(2, partnerLong);

  status = emberLookupEui64ByNodeId(EMBER_TRUST_CENTER_NODE_ID, trustCenterEui64);
  if (status != EMBER_SUCCESS) {
    goto done;
  }

  // Set the default command options as APS and NWK encryption.
  apsCommandOptions = (APS_COMMAND_OPTION_NWK_ENCRYPTION
                       | APS_COMMAND_OPTION_APS_ENCRYPTION);

  // What key should we be using?
  switch (options) {
    case 0:
    case BIT32(0):
      // Handled elsewhere.
      break;
    case BIT32(1):
      // KeyLoadKey
      // TODO: key id in security control.
      status = setNextKey(trustCenterEui64, false, KEY_TYPE_DEFAULT_TCLK);
      break;
    case BIT32(2):
      // TODO: KeyTransport key id in security control.
      status = setNextKey(trustCenterEui64, false, KEY_TYPE_DEFAULT_TCLK);
      break;
    case BIT32(3):
      // TODO: DataKey key id in security control.
      status = setNextKey(trustCenterEui64, false, KEY_TYPE_DEFAULT_TCLK);
      break;
    case BIT32(4):
      // TODO: DataKey key id in security control.
      status = setNextKey(trustCenterEui64, false, KEY_TYPE_RANDOM);
      break;
    case BIT32(5):
      // APS encrypted with install code derived key.
      status = setNextKey(trustCenterEui64, false, KEY_TYPE_INSTALL_CODE);
      break;
    case BIT32(6):
      // No APS or NWK encryption.
      apsCommandOptions &= ~(APS_COMMAND_OPTION_NWK_ENCRYPTION
                             | APS_COMMAND_OPTION_APS_ENCRYPTION);
      break;
    case BIT32(7):
      // KeyType is 0x01 (network key).
      keyType = APS_REQUEST_KEY_TYPE_NETWORK;
      break;
    case BIT32(8):
    case BIT32(9):
      // KeyType is 0x03 (application link key).
      // FIXME: email sent to bozena regarding this key type disagreement.
      keyType = APS_REQUEST_KEY_TYPE_APPLICATION;
      break;
    case BIT32(10):
      // TODO: do not issue verify key.
      break;
    case BIT32(11):
      // KeyType is 0x04.
      // TODO: KeyTransportKey key id in security control.
      keyType = APS_REQUEST_KEY_TYPE_TCLK;
      status = setNextKey(trustCenterEui64, false, KEY_TYPE_DEFAULT_TCLK);
      break;
    case BIT32(12):
      // TODO: incorrect APS VerifyKey hash.
      break;
    default:
      status = EMBER_BAD_ARGUMENT;
  }

  if (status != EMBER_SUCCESS) {
    goto done;
  }

  *finger++ = APS_REQUEST_KEY_COMMAND;
  *finger++ = keyType;
  if (keyType == APS_REQUEST_KEY_TYPE_APPLICATION && !(options & BIT32(0))) {
    MEMMOVE(finger, partnerLong, EUI64_SIZE);
    finger += EUI64_SIZE;
  }

  commandBuffer = emberFillLinkedBuffers(frame, finger - &frame[0]);
  if (commandBuffer == EMBER_NULL_MESSAGE_BUFFER) {
    status = EMBER_NO_BUFFERS;
    goto done;
  }

  status = (emSendApsCommand(destShort,
                             trustCenterEui64,
                             commandBuffer,
                             apsCommandOptions)
            ? EMBER_SUCCESS
            : EMBER_ERR_FATAL);
  emberReleaseMessageBuffer(commandBuffer);

  done:
#endif /* EZSP_HOST */

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Request key",
                     status);
}
