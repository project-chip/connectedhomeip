/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @brief Definitions for the Standalone Bootloader Common plugin.
 *******************************************************************************
   ******************************************************************************/

#define BOOTLOAD_OTA_SIZE             64    // bytes

// Maximum bootload data packet size which is SOH (data) message.  The message
// contains BOOTLOAD_OTA_SIZE bytes data and 6 bytes header.
#define MAX_BOOTLOAD_MESSAGE_SIZE     (BOOTLOAD_OTA_SIZE + 6)

// Current bootloader protocol version
#define BOOTLOAD_PROTOCOL_VERSION 1

// Version field defined in the challenge-request Xmodem Auth message
#define CHALLENGE_REQUEST_VERSION 0x01

// version (1-byte)
// command (1-byte)
#define BOOTLOAD_MESSAGE_OVERHEAD 2

//offset into bootload header and bootloader payload
#define OFFSET_VERSION            0
#define OFFSET_MESSAGE_TYPE       1
#define OFFSET_DEVICE_TYPE        2
#define OFFSET_BLOCK_NUMBER       2
#define OFFSET_ERROR_TYPE         2
#define OFFSET_BLOCK_NUMBER_CHECK 3
#define OFFSET_ERROR_BLOCK        3
#define OFFSET_IMAGE_CONTENT      4

// Query Response Format
// (Overhead)
// Bootload active: 1-byte
// MFG ID: 2-bytes
// Board ID: 2-bytes
// Bootloader Capabiliteis: 1-byte
// Platform: 1-byte
// Micro: 1-byte
// PHY: 1-byte
// Bootloader Version: 2-bytes
#define QUERY_RESPONSE_LENGTH (BOOTLOAD_MESSAGE_OVERHEAD + 11)
#define QRESP_OFFSET_BL_ACTIVE             2 // 1 byte long
#define QRESP_OFFSET_MFG_ID                3 // 2 bytes long (little endian)
#define QRESP_OFFSET_HARDWARE_TAG          5 // 16 bytes long
#define QRESP_OFFSET_BL_CAPS              21 // 1 byte long
#define QRESP_OFFSET_PLATFORM             22 // 1 byte long
#define QRESP_OFFSET_MICRO                23 // 1 byte long
#define QRESP_OFFSET_PHY                  24 // 1 byte long
#define QRESP_OFFSET_BL_VERSION           25 // 2 bytes long (big endian)

// offsets into bootloader payload for launch request message.
#define OFFSET_MFG_ID             2 // 2 bytes long (little endian)
#define OFFSET_HARDWARE_TAG       4 // 16 bytes long (little endian)

// offsets into bootloader payload for authentication challenge message.
#define OFFSET_AUTH_CHALLENGE     2 // 16 bytes long

// offsets into bootloader payload for authentication response message.
#define OFFSET_AUTH_RESPONSE      2 // 16 bytes long

// Ember Over the air bootload message types extended from XModem message types.
// When transmitting bootload messages over the air, we also follow basic
// XModem protocol.
#define XMODEM_QUERY  0x51  // ASCII 'Q'
#define XMODEM_QRESP  0x52  // ASCII 'R'
#define XMODEM_CC     0x03  // Cancel (from sender, user)
#define XMODEM_LAUNCH_REQUEST 0x4c // ASCII 'L' (Launch Bootloader)
#define XMODEM_AUTH_CHALLENGE 0x63 // ASCII 'c' (Authentication Challenge)
#define XMODEM_AUTH_RESPONSE  0x72 // ASCII 'r' (Auth. Response to Challenge)

// Same size as a 128-bit AES key
#define BOOTLOAD_AUTH_COMMON_SIZE    16
#define BOOTLOAD_AUTH_CHALLENGE_SIZE BOOTLOAD_AUTH_COMMON_SIZE
#define BOOTLOAD_AUTH_RESPONSE_SIZE  BOOTLOAD_AUTH_COMMON_SIZE

// Query Format
// (Overhead)
#define QUERY_REQUEST_LENGTH (BOOTLOAD_MESSAGE_OVERHEAD)

// Launch Request
// (Overhead)
// MFG ID: 2-bytes (little endian)
// Hardware Tag: 16-bytes (a.k.a. Board name MFG token)
#define LAUNCH_REQUEST_LENGTH (BOOTLOAD_MESSAGE_OVERHEAD + 18)

// Challenge Format
// (Overhead)
// Request Version: 1-byte
// Bootloader Version: 2-bytes (big endian)
// Platform ID: 1-byte
// Micro ID: 1-byte
// Phy ID: 1-byte
// Local EUI64: 8-bytes
// MAC Timer: 3-bytes   (big endian)
// Random Data: 0-bytes
#define XMODEM_AUTH_CHALLENGE_LENGTH (BOOTLOAD_MESSAGE_OVERHEAD + 15)
#define XMODEM_AUTH_CHALLENGE_REQUEST_INDEX (BOOTLOAD_MESSAGE_OVERHEAD)

// Response Format
// (Overhead)
// Response: 16-bytes
#define XMODEM_AUTH_RESPONSE_LENGTH (BOOTLOAD_MESSAGE_OVERHEAD + 16)

EmberStatus emAfSendBootloadMessage(bool isBroadcast,
                                    EmberEUI64 destEui64,
                                    uint8_t length,
                                    uint8_t* message);

#define bootloadPrintln(...) emberAfCorePrintln(__VA_ARGS__)
#define bootloadPrint(...)   emberAfCorePrint(__VA_ARGS__)

uint8_t emberAfPluginStandaloneBootloaderCommonMakeHeader(uint8_t *message, uint8_t type);
EmberStatus emberAfPluginStandaloneBootloaderCommonSendMessage(bool isBroadcast,
                                                               EmberEUI64 targetEui,
                                                               uint8_t length,
                                                               uint8_t* message);
bool emberAfPluginStandaloneBootloaderCommonCheckIncomingMessage(uint8_t length,
                                                                 uint8_t* message);

void emAfStandaloneBootloaderCommonPrintHardwareTag(uint8_t* text);

void emAfStandaloneBootloaderClientEncrypt(uint8_t* block, uint8_t* key);
