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
/***************************************************************************/
/**
 * @file
 * @brief
 *******************************************************************************
 ******************************************************************************/

// *******************************************************************
// * util.h
// *
// *
// * Copyright 2007-2017 by Silicon Laboratories. All rights reserved.      *80*
// *******************************************************************

#pragma once

#include <inttypes.h>
#include <messaging/ExchangeMgr.h>

// User asserts can override SLAB_ASSERT and should be defined as follows:
// void userAssert (int file, int line);                   // declaration
// #define USER_ASSERT(file, line) userAssert(file, line)  // definition

#if defined(NO_ASSERT)
#define SLAB_ASSERT(expr)
#else
#if defined(USER_ASSERT)
#define SLAB_ASSERT(expr) ((expr) ? ((void) 0) : USER_ASSERT(__FILE__, __LINE__))
#else
#define SLAB_ASSERT(expr) ((expr) ? ((void) 0) : slabAssert(__FILE__, __LINE__))
#endif // USER_ASSERT
#endif // NO_ASSERT

// This controls the type of response. Normally The library sends an automatic
// response (if appropriate) on the same PAN. The reply can be disabled by
// calling emberAfSetNoReplyForNextMessage.
#define ZCL_UTIL_RESP_NORMAL 0
#define ZCL_UTIL_RESP_NONE 1
#define ZCL_UTIL_RESP_INTERPAN 2

// Cluster name structure
typedef struct
{
    chip::ClusterId id;
    const char * name;
} EmberAfClusterName;

extern const EmberAfClusterName zclClusterNames[];

#define ZCL_NULL_CLUSTER_ID 0xFFFF

#include <app/util/af.h>

// Override APS retry: 0 - don't touch, 1 - always set, 2 - always unset
typedef enum
{
    EMBER_AF_RETRY_OVERRIDE_NONE  = 0,
    EMBER_AF_RETRY_OVERRIDE_SET   = 1,
    EMBER_AF_RETRY_OVERRIDE_UNSET = 2
} EmberAfRetryOverride;

// The default APS retry flag value is controlled by EMBER_AF_DEFAULT_APS_OPTIONS
// and is usually 1. In high traffic, low bandwidth networks (e.g. sub-GHz),
// the app may want to override this. Two methods are available for apps that
// may wants to do this dynamically per message:
// 1. Call emberAfSetRetryOverride each time before filling the message buffer;
// 2. Call emberAfSetRetryOverride once with a value covering most messages and
//    then toggling the flag as necessary in emberAfPreMessageSendCallback.
void emberAfSetRetryOverride(EmberAfRetryOverride value);

// Return the current override status
EmberAfRetryOverride emberAfGetRetryOverride(void);

// This function applies the curent override value to the APS options.
// It is called internally by the framework in the final stages of filling the
// message buffer.
void emAfApplyRetryOverride(EmberApsOption * options);

// Override Disable Default Response flag in the ZCL Frame Control
typedef enum
{
    EMBER_AF_DISABLE_DEFAULT_RESPONSE_NONE      = 0, // no override
    EMBER_AF_DISABLE_DEFAULT_RESPONSE_ONE_SHOT  = 1, // next message only
    EMBER_AF_DISABLE_DEFAULT_RESPONSE_PERMANENT = 2  // until cancelled
} EmberAfDisableDefaultResponse;

// The default value for the Disable Default Response flag in ZCL Frame Control
// is 0 (i.e. default responses are generated). The app can disable default
// reponses either for the next message only or for all messages until another
// call to this function.
void emberAfSetDisableDefaultResponse(EmberAfDisableDefaultResponse value);

// Return the current status
EmberAfDisableDefaultResponse emberAfGetDisableDefaultResponse(void);

// This function applies the curent override value to the ZCL Frame Control.
// It is called internally by the framework in the final stages of filling the
// message buffer.
void emAfApplyDisableDefaultResponse(uint8_t * frame_control);

// Returns a mfg code from current command.
// This should only be used within the command parsing context.
// Wraps emberAfCurrentCommand(), and assumes that the current comamnd
// is either the current valid command or NULL
// In the case of NULL, then it returns EMBER_AF_NULL_MANUFACTURER_CODE
uint16_t emberAfGetMfgCodeFromCurrentCommand(void);

// EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH is defined in config.h
#define EMBER_AF_RESPONSE_BUFFER_LEN EMBER_AF_MAXIMUM_SEND_PAYLOAD_LENGTH

void emberAfInit(chip::Messaging::ExchangeManager * exchangeContext);
void emberAfTick(void);
uint16_t emberAfFindClusterNameIndex(chip::ClusterId cluster);
void emberAfStackDown(void);

void emberAfDecodeAndPrintCluster(chip::ClusterId cluster);

/**
 * Retrieves the difference between the two passed values.
 * This function assumes that the two values have the same endianness.
 * On platforms that support 64-bit primitive types, this function will work
 * on data sizes up to 8 bytes. Otherwise, it will only work on data sizes of
 * up to 4 bytes.
 */
EmberAfDifferenceType emberAfGetDifference(uint8_t * pData, EmberAfDifferenceType value, uint8_t dataSize);

/**
 * Retrieves an uint64_t from the given Zigbee payload. The integer retrieved
 * may be cast into an integer of the appropriate size depending on the
 * number of bytes requested from the message. In Zigbee, all integers are
 * passed over the air in LSB form. LSB to MSB conversion is
 * done within this function automatically before the integer is returned.
 *
 * Obviously (due to return value) this function can only handle
 * the retrieval of integers between 1 and 8 bytes in length.
 *
 */
uint64_t emberAfGetInt(const uint8_t * message, uint16_t currentIndex, uint16_t msgLen, uint8_t bytes);

void emberAfClearResponseData(void);
uint8_t * emberAfPutInt8uInResp(uint8_t value);
uint16_t * emberAfPutInt16uInResp(uint16_t value);
uint32_t * emberAfPutInt32uInResp(uint32_t value);
uint32_t * emberAfPutInt24uInResp(uint32_t value);
uint8_t * emberAfPutBlockInResp(const uint8_t * data, uint16_t length);
uint8_t * emberAfPutStringInResp(const uint8_t * buffer);
uint8_t * emberAfPutDateInResp(EmberAfDate * value);
void emberAfPutInt16sInResp(int16_t value);
void emberAfPutStatusInResp(EmberAfStatus value);

bool emberAfIsThisMyEui64(EmberEUI64 eui64);

#ifdef EZSP_HOST
// the EM260 host application is expected to provide these functions if using
// a cluster that needs it.
EmberNodeId emberGetSender(void);
EmberStatus emberGetSenderEui64(EmberEUI64 senderEui64);
#endif // EZSP_HOST

// DEPRECATED.
extern uint8_t emberAfIncomingZclSequenceNumber;

// the caller to the library can set a flag to say do not respond to the
// next ZCL message passed in to the library. Passing true means disable
// the reply for the next ZCL message. Setting to false re-enables the
// reply (in the case where the app disables it and then doesnt send a
// message that gets parsed).
void emberAfSetNoReplyForNextMessage(bool set);

#define isThisDataTypeSentLittleEndianOTA(dataType) (!(emberAfIsThisDataTypeAStringType(dataType)))

extern uint8_t emberAfResponseType;

uint16_t emberAfStrnlen(const uint8_t * string, uint16_t maxLength);

/* @brief Append characters to a ZCL string.
 *
 * Appending characters to an existing ZCL string. If it is an invalid string
 * (length byte equals 0xFF), no characters will be appended.
 *
 * @param zclString - pointer to the zcl string
 * @param zclStringMaxLen - length of zcl string (does not include zcl length byte)
 * @param src - pointer to plain text characters
 * @param srcLen - length of plain text characters
 *
 * @return number of characters appended
 *
 */
uint8_t emberAfAppendCharacters(uint8_t * zclString, uint8_t zclStringMaxLen, const uint8_t * appendingChars,
                                uint8_t appendingCharsLen);

EmberStatus emAfValidateChannelPages(uint8_t page, uint8_t channel);

/* @brief A Silicon Labs assert function
 *
 * This function is provided to call an assert function in the application code.
 * It starts an infinite loop that provokes the watchdog to fire.
 *
 * @param file - the source file that calls this assert
 * @param line - the line that calls this assert
 *
 * @return void
 *
 */
void slabAssert(const char * file, int line);

/* @brief Get the page number from an 8-bit encoded channel-page
 *
 * The top three bits denote the page number, like this:
 *   000x xxxx = page 0
 *   100x xxxx = page 28
 *   101x xxxx = page 29
 *   110x xxxx = page 30
 *   111x xxxx = page 31
 *
 * @param chanPg - 8-bit encoded channel and page
 *
 * @return page number (0, 28-31, 0xFF if invalid)
 */
uint8_t emberAfGetPageFrom8bitEncodedChanPg(uint8_t chanPg);

/* @brief Get the channel number from an 8-bit encoded channel-page
 *
 * The bottom 5 bits denote the channel within the page.
 *
 * Provided for symmetry with the above emberAfGetPageFrom8bitEncodedChanPg().
 * It simply masks the bottom 5 bits.
 *
 * @param chanPg - 8-bit encoded channel and page
 *
 * @return channel number (0-8, 0-26, 11-26, depending on the page)
 */
uint8_t emberAfGetChannelFrom8bitEncodedChanPg(uint8_t chanPg);

/* @brief Make an 8-bit encoded channel-page from channel and page arguments
 *
 * @param page
 * @param channel
 *
 * @return 8-bit encoded channel-page, 0xFF if invalid
 */
uint8_t emberAfMake8bitEncodedChanPg(uint8_t page, uint8_t channel);

bool emberAfContainsAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId);

/* @brief returns true if the attribute is known to be volatile (i.e. RAM
 * storage).
 */
bool emberAfIsKnownVolatileAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId);

namespace chip {
chip::Messaging::ExchangeManager * ExchangeManager();
} // namespace chip
