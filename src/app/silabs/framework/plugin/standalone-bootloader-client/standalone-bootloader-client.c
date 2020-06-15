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
 * @brief This file defines the client behavior for the Ember proprietary bootloader
 * protocol.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/plugin/standalone-bootloader-common/bootloader-protocol.h"
#include "standalone-bootloader-client.h"

//------------------------------------------------------------------------------
// Globals

// Most of the work is done by the actual bootloader code.  Client just
// has to authenticate the challenge and launch the bootloader.
typedef enum {
  CLIENT_BOOTLOAD_STATE_NONE           = 0,
  CLIENT_BOOTLOAD_STATE_CHALLENGE_SENT = 1,
  CLIENT_BOOTLOAD_STATE_COUNTDOWN      = 2,
} ClientBootloadState;

static const char * clientStateStrings[] = {
  "None",
  "Challenge Sent",
  "Countdown to bootload launch",
};
#define NULL_EUI64 { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }

static ClientBootloadState clientBootloadState = CLIENT_BOOTLOAD_STATE_NONE;
static EmberEUI64 bootloadServerEui64 = NULL_EUI64;
static uint8_t challenge[BOOTLOAD_AUTH_CHALLENGE_SIZE];

EmberEventControl emberAfPluginStandaloneBootloaderClientMyEventEventControl;

#define CHALLENGE_TIMEOUT_SECONDS 2

#if !defined(EMBER_TEST)
  #define getBootloadKey(returnData) emAfStandaloneBootloaderClientGetKey(returnData)
#endif

//------------------------------------------------------------------------------
// External Declarations

void emAesEncrypt(uint8_t *block, const uint8_t *key);

//------------------------------------------------------------------------------
// Functions

#if defined(EMBER_TEST)
// No MFG tokens in simulation.  Here is a hack to make it work.
void getBootloadKey(uint8_t* returnData)
{
  // Same key as the default for the Standalone Bootloader Server plugin
  uint8_t testKey[] = {
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
  };
  MEMMOVE(returnData, testKey, EMBER_ENCRYPTION_KEY_SIZE);
}
#endif

void emAfStandaloneBootloaderClientPrintStatus(void)
{
  uint8_t platformId;
  uint8_t microId;
  uint8_t phyId;
  uint16_t bootloaderVersion;
  uint16_t mfgId;
  uint8_t hardwareTag[EMBER_AF_STANDALONE_BOOTLOADER_HARDWARE_TAG_LENGTH];
  uint8_t key[EMBER_ENCRYPTION_KEY_SIZE];

  emAfStandaloneBootloaderClientGetInfo(&bootloaderVersion,
                                        &platformId,
                                        &microId,
                                        &phyId);
  emAfStandaloneBootloaderClientGetMfgInfo(&mfgId,
                                           hardwareTag);
  getBootloadKey(key);

  bootloadPrintln("Client status: %p", clientStateStrings[clientBootloadState]);
  bootloadPrintln("Platform: 0x%X", platformId);
  bootloadPrintln("Micro:    0x%X", microId);
  bootloadPrintln("Phy:      0x%X", phyId);
  bootloadPrintln("BTL Version: 0x%2X", bootloaderVersion);
  bootloadPrintln("MFG ID:   0x%2X", mfgId);
  bootloadPrint("Board Name: ");
  emAfStandaloneBootloaderCommonPrintHardwareTag(hardwareTag);

  bootloadPrint("MFG Key in Token: ");
  emberAfPrintZigbeeKey(key);
}

static EmberStatus sendChallenge(EmberEUI64 targetEui)
{
  uint8_t outgoingBlock[MAX_BOOTLOAD_MESSAGE_SIZE];
  uint8_t index = emberAfPluginStandaloneBootloaderCommonMakeHeader(outgoingBlock,
                                                                    XMODEM_AUTH_CHALLENGE);
  uint8_t platformId;
  uint8_t microId;
  uint8_t phyId;
  uint32_t macTimer;
  EmberStatus status;
  uint16_t bootloaderVersion;

  outgoingBlock[index++] = CHALLENGE_REQUEST_VERSION;
  emAfStandaloneBootloaderClientGetInfo(&bootloaderVersion,
                                        &platformId,
                                        &microId,
                                        &phyId);
  outgoingBlock[index++] = HIGH_BYTE(bootloaderVersion);
  outgoingBlock[index++] = LOW_BYTE(bootloaderVersion);
  outgoingBlock[index++] = platformId;
  outgoingBlock[index++] = microId;
  outgoingBlock[index++] = phyId;

  emberAfGetEui64(&(outgoingBlock[index]));
  index += EUI64_SIZE;
  macTimer = emAfStandaloneBootloaderClientGetRandomNumber();

  // The em250's mac timer is 20 bits long.  Disregard the first byte (zero),
  // even for other devices that may have a 32-bit symbol timer.
  outgoingBlock[index++] = LOW_BYTE(macTimer);
  outgoingBlock[index++] = HIGH_BYTE(macTimer);
  outgoingBlock[index++] = (uint8_t)(macTimer >> 16);

  // NOTE:  The protocol has a bug in it.  17-bytes are sent over-the-air
  // but only 16-bytes are used in the authentication.  That means only
  // 2-bytes of pseudo random data are used as the challenge (the MAC Timer).

  // Remember the challenge for later so we can authenticate it.
  MEMMOVE(challenge,
          &(outgoingBlock[BOOTLOAD_MESSAGE_OVERHEAD]),
          BOOTLOAD_AUTH_CHALLENGE_SIZE);
  MEMMOVE(bootloadServerEui64, targetEui, EUI64_SIZE);

  status = emberAfPluginStandaloneBootloaderCommonSendMessage(false,  // is broadcast?
                                                              targetEui,
                                                              index,
                                                              outgoingBlock);

  if (status == EMBER_SUCCESS) {
    clientBootloadState = CLIENT_BOOTLOAD_STATE_CHALLENGE_SENT;
    emberEventControlSetDelayQS(emberAfPluginStandaloneBootloaderClientMyEventEventControl,
                                CHALLENGE_TIMEOUT_SECONDS << 2);
  }
  return status;
}

static void resetClientState(void)
{
  bootloadPrintln("Clearing client bootload state");
  MEMSET(bootloadServerEui64, 0, EUI64_SIZE);
  clientBootloadState = CLIENT_BOOTLOAD_STATE_NONE;
}

static void launchBootloader(void)
{
  EmberStatus status;
  bootloadPrintln("Launching standalone bootloader now...");
  emberSerialWaitSend(APP_SERIAL);

  status = emAfStandaloneBootloaderClientLaunch();

  // If we got here, something went wrong.
  bootloadPrintln("ERROR: Bootloader Launch failed: 0x%X", status);
}

void emberAfPluginStandaloneBootloaderClientMyEventEventHandler(void)
{
  emberEventControlSetInactive(emberAfPluginStandaloneBootloaderClientMyEventEventControl);

  if (clientBootloadState == CLIENT_BOOTLOAD_STATE_CHALLENGE_SENT) {
    bootloadPrintln("Timeout waiting for challenge response");
  } else if (clientBootloadState == CLIENT_BOOTLOAD_STATE_COUNTDOWN) {
    launchBootloader();
  }
  resetClientState();
}

static EmberStatus sendQueryResponse(EmberEUI64 longId)
{
  uint8_t outgoingBlock[MAX_BOOTLOAD_MESSAGE_SIZE];
  uint8_t index = emberAfPluginStandaloneBootloaderCommonMakeHeader(outgoingBlock,
                                                                    XMODEM_QRESP);
  uint16_t mfgId;
  uint8_t platformId;
  uint8_t microId;
  uint8_t phyId;
  uint16_t bootloaderVersion;

  emAfStandaloneBootloaderClientGetMfgInfo(&mfgId,
                                           &(outgoingBlock[QRESP_OFFSET_HARDWARE_TAG]));
  emAfStandaloneBootloaderClientGetInfo(&bootloaderVersion,
                                        &platformId,
                                        &microId,
                                        &phyId);

  // Bootloader active?  1-byte
  outgoingBlock[index++] = 0;

  outgoingBlock[index++] = HIGH_BYTE(mfgId);
  outgoingBlock[index++] = LOW_BYTE(mfgId);

  // Hardware Tag has already been copied into place.
  index += EMBER_AF_STANDALONE_BOOTLOADER_HARDWARE_TAG_LENGTH;

  // Bootloader Capabilities : 1-byte
  // In the future, we should read the capabilities mask from fixed
  // location in bootloader.  However, currently we do not have any
  // bootload capabilities implemented.  This filed is included for
  // future use.
  outgoingBlock[index++] = 0;

  outgoingBlock[index++] = platformId;
  outgoingBlock[index++] = microId;
  outgoingBlock[index++] = phyId;
  outgoingBlock[index++] = HIGH_BYTE(bootloaderVersion);
  outgoingBlock[index++] = LOW_BYTE(bootloaderVersion);

  return emberAfPluginStandaloneBootloaderCommonSendMessage(false, // is broadcast?
                                                            longId,
                                                            index,
                                                            outgoingBlock);
}

static bool authenticateChallengeResponse(uint8_t* challengeResponse)
{
  uint8_t i;
  bool validKey = false;
  uint8_t key[EMBER_ENCRYPTION_KEY_SIZE];
  getBootloadKey(key);

  for (i = 0; i < EMBER_ENCRYPTION_KEY_SIZE && !validKey; i++) {
    if (key[i] != 0xFF) {
      validKey = true;
    }
  }
  if (!validKey) {
    bootloadPrintln("Error: Bootload key not set!  Cannot authenticate response!  Giving up!");
    return false;
  }

  emAfStandaloneBootloaderClientEncrypt(challenge, key);
  return (0 == MEMCOMPARE(challengeResponse, challenge, EMBER_ENCRYPTION_KEY_SIZE));
}

static void delayedLaunchBootloader(void)
{
  // If the time is 0, then this should fire on the next emberTick().
  emberEventControlSetDelayQS(emberAfPluginStandaloneBootloaderClientMyEventEventControl,
                              EMBER_AF_PLUGIN_STANDALONE_BOOTLOADER_CLIENT_COUNTDOWN_TIME_SECONDS << 2);
  clientBootloadState = CLIENT_BOOTLOAD_STATE_COUNTDOWN;
  bootloadPrintln("Delaying %d seconds before launching bootloader",
                  EMBER_AF_PLUGIN_STANDALONE_BOOTLOADER_CLIENT_COUNTDOWN_TIME_SECONDS);
}

static void decodeAndPrintClientMessageType(uint8_t command)
{
  uint8_t id = 0xFF;
  const char * commandStrings[] = {
    "Query",
    "Launch Request",
    "Auth Response",
  };
  switch (command) {
    case (XMODEM_QUERY):
      id = 0;
      break;
    case (XMODEM_LAUNCH_REQUEST):
      id = 1;
      break;
    case (XMODEM_AUTH_RESPONSE):
      id = 2;
    default:
      break;
  }
  bootloadPrintln("Received Standalone Bootloader message (%d): %p",
                  command,
                  (id == 0xFF
                   ? "??"
                   : commandStrings[id]));
}

static bool validateSenderEui64(EmberEUI64 sender)
{
  // Anyone may send us messages if we are not otherwise in the middle
  // of a bootload.
  if (clientBootloadState == CLIENT_BOOTLOAD_STATE_NONE) {
    return true;
  }

  return (0 == MEMCOMPARE(sender, bootloadServerEui64, EUI64_SIZE));
}

bool emberAfPluginStandaloneBootloaderCommonIncomingMessageCallback(EmberEUI64 longId,
                                                                    uint8_t length,
                                                                    uint8_t* message)
{
  if (!emberAfPluginStandaloneBootloaderCommonCheckIncomingMessage(length, message)) {
    return false;
  }

  decodeAndPrintClientMessageType(message[OFFSET_MESSAGE_TYPE]);

  if (!validateSenderEui64(longId)) {
    bootloadPrintln("Error: Sender EUI64 doesn't match current server EUI64");
    return true;
  }

  if (!emAfPluginStandaloneBootloaderClientCheckBootloader()) {
    // Wrong bootloader type.  But claim we processed the message since
    // we do not want anything else to respond.
    return true;
  }

  if (!emberAfPluginStandaloneBootloaderClientAllowIncomingMessageCallback(longId,
                                                                           message[OFFSET_MESSAGE_TYPE])) {
    bootloadPrintln("Standalone bootload client plugin told to ignore incoming message");
    return true;
  }

  switch (message[OFFSET_MESSAGE_TYPE]) {
    case (XMODEM_QUERY): {
      sendQueryResponse(longId);
      break;
    }
    case (XMODEM_LAUNCH_REQUEST): {
      if (clientBootloadState == CLIENT_BOOTLOAD_STATE_NONE) {
        sendChallenge(longId);
      }
      break;
    }
    case (XMODEM_AUTH_RESPONSE): {
      if (clientBootloadState == CLIENT_BOOTLOAD_STATE_CHALLENGE_SENT) {
        if (length < XMODEM_AUTH_RESPONSE_LENGTH) {
          bootloadPrintln("Error: Challenge-Response too short (%d < %d)", length, XMODEM_AUTH_RESPONSE_LENGTH);
          return true;
        }

        emberEventControlSetInactive(emberAfPluginStandaloneBootloaderClientMyEventEventControl);

        if (authenticateChallengeResponse(&(message[BOOTLOAD_MESSAGE_OVERHEAD]))) {
          if (emberAfPluginStandaloneBootloaderClientAllowBootloadLaunchCallback(longId)) {
            delayedLaunchBootloader();
            break;
          }
        } else {
          bootloadPrintln("Error: Invalid response to challenge");
        }
        resetClientState();
      }
      break;
    }
    default:
      // Assume error has already been printed.
      break;
  }

  return true;
}
