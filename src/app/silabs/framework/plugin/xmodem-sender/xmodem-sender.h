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
 * @brief Definitions for the XMODEM Sender plugin.
 *******************************************************************************
   ******************************************************************************/

#define XMODEM_SOH   (0x01)
#define XMODEM_EOT   (0x04)
#define XMODEM_ACK   (0x06)
#define XMODEM_NAK   (0x15)
#define XMODEM_CANCEL (0x18)
#define XMODEM_BLOCKOK (0x19)
#define XMODEM_FILEDONE (0x17)

typedef EmberStatus (EmberAfXmodemSenderTransmitFunction)(uint8_t* data, uint8_t length);

typedef EmberStatus (EmberAfXmodemSenderGetNextBlockFunction)(uint32_t address,
                                                              uint8_t length,
                                                              uint8_t* returnData,
                                                              uint8_t* returnLength,
                                                              bool* done);
typedef void (EmberAfXmodemSenderFinishedFunction)(bool success);

void emberAfPluginXmodemSenderIncomingBlock(uint8_t* data,
                                            uint8_t  length);

// The maxSizeOfBlock does not include the Xmodem overhead (5-bytes)
EmberStatus emberAfPluginXmodemSenderStart(EmberAfXmodemSenderTransmitFunction* sendRoutine,
                                           EmberAfXmodemSenderGetNextBlockFunction* getNextBlockRoutine,
                                           EmberAfXmodemSenderFinishedFunction* finishedRoutine,
                                           uint8_t maxSizeOfBlock,
                                           bool waitForReady);

void emberAfPluginXmodemSenderAbort(void);
