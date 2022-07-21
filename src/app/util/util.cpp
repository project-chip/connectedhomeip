/**
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 * @brief This file contains all of the common ZCL
 *command and attribute handling code for Ember's ZCL
 *implementation
 *******************************************************************************
 ******************************************************************************/

#include "app/util/common.h"
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/print-cluster.h>
#include <app/util/af-event.h>
#include <app/util/af.h>
#include <app/util/ember-compatibility-functions.h>
#include <zap-generated/PluginApplicationCallbacks.h>

#ifdef EMBER_AF_PLUGIN_GROUPS_SERVER
#include <app/clusters/groups-server/groups-server.h>
#endif // EMBER_AF_PLUGIN_GROUPS_SERVER

using namespace chip;

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------
// Globals

// Storage and functions for turning on and off devices
bool afDeviceEnabled[MAX_ENDPOINT_COUNT];

#ifdef EMBER_AF_ENABLE_STATISTICS
// a variable containing the number of messages send from the utilities
// since emberAfInit was called.
uint32_t afNumPktsSent;
#endif

const EmberAfClusterName zclClusterNames[] = {
    CLUSTER_IDS_TO_NAMES              // defined in print-cluster.h
    { ZCL_NULL_CLUSTER_ID, nullptr }, // terminator
};

// A pointer to the current command being processed
// This struct is allocated inside ember-compatibility-functions.cpp.
// The pointer below is set to NULL when not processing a command.
EmberAfClusterCommand * emAfCurrentCommand;

// A pointer to the global exchange manager
chip::Messaging::ExchangeManager * emAfExchangeMgr = nullptr;

// DEPRECATED.
uint8_t emberAfIncomingZclSequenceNumber = 0xFF;

// Sequence used for outgoing messages if they are
// not responses.
uint8_t emberAfSequenceNumber = 0xFF;

static uint8_t /*enum EmberAfRetryOverride*/ emberAfApsRetryOverride                      = EMBER_AF_RETRY_OVERRIDE_NONE;
static uint8_t /*enum EmberAfDisableDefaultResponse*/ emAfDisableDefaultResponse          = EMBER_AF_DISABLE_DEFAULT_RESPONSE_NONE;
static uint8_t /*enum EmberAfDisableDefaultResponse*/ emAfSavedDisableDefaultResponseVale = EMBER_AF_DISABLE_DEFAULT_RESPONSE_NONE;

// Holds the response type
uint8_t emberAfResponseType = ZCL_UTIL_RESP_NORMAL;

#ifdef EMBER_AF_GENERATED_PLUGIN_TICK_FUNCTION_DECLARATIONS
EMBER_AF_GENERATED_PLUGIN_TICK_FUNCTION_DECLARATIONS
#endif

//------------------------------------------------------------------------------

// Device enabled/disabled functions
bool emberAfIsDeviceEnabled(EndpointId endpoint)
{
    uint16_t index;
#ifdef ZCL_USING_BASIC_CLUSTER_DEVICE_ENABLED_ATTRIBUTE
    bool deviceEnabled;
    if (emberAfReadServerAttribute(endpoint, ZCL_BASIC_CLUSTER_ID, ZCL_DEVICE_ENABLED_ATTRIBUTE_ID, (uint8_t *) &deviceEnabled,
                                   sizeof(deviceEnabled)) == EMBER_ZCL_STATUS_SUCCESS)
    {
        return deviceEnabled;
    }
#endif
    index = emberAfIndexFromEndpoint(endpoint);
    if (index != 0xFFFF && index < sizeof(afDeviceEnabled))
    {
        return afDeviceEnabled[index];
    }
    return false;
}

void emberAfSetDeviceEnabled(EndpointId endpoint, bool enabled)
{
    uint16_t index = emberAfIndexFromEndpoint(endpoint);
    if (index != 0xFFFF && index < sizeof(afDeviceEnabled))
    {
        afDeviceEnabled[index] = enabled;
    }
#ifdef ZCL_USING_BASIC_CLUSTER_DEVICE_ENABLED_ATTRIBUTE
    emberAfWriteServerAttribute(endpoint, ZCL_BASIC_CLUSTER_ID, ZCL_DEVICE_ENABLED_ATTRIBUTE_ID, (uint8_t *) &enabled,
                                ZCL_BOOLEAN_ATTRIBUTE_TYPE);
#endif
}

// Is the device identifying?
bool emberAfIsDeviceIdentifying(EndpointId endpoint)
{
#ifdef ZCL_USING_IDENTIFY_CLUSTER_SERVER
    uint16_t identifyTime;
    EmberAfStatus status = emberAfReadServerAttribute(endpoint, ZCL_IDENTIFY_CLUSTER_ID, ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                                                      (uint8_t *) &identifyTime, sizeof(identifyTime));
    return (status == EMBER_ZCL_STATUS_SUCCESS && 0 < identifyTime);
#else
    return false;
#endif
}

// Calculates difference. See EmberAfDifferenceType for the maximum data size
// that this function will support.
EmberAfDifferenceType emberAfGetDifference(uint8_t * pData, EmberAfDifferenceType value, uint8_t dataSize)
{
    EmberAfDifferenceType value2 = 0, diff;
    uint8_t i;

    // only support data types up to 8 bytes
    if (dataSize > sizeof(EmberAfDifferenceType))
    {
        return 0;
    }

    // get the value
    for (i = 0; i < dataSize; i++)
    {
        value2 = value2 << 8;
#if (BIGENDIAN_CPU)
        value2 += pData[i];
#else  // BIGENDIAN
        value2 += pData[dataSize - i - 1];
#endif // BIGENDIAN
    }

    if (value > value2)
    {
        diff = value - value2;
    }
    else
    {
        diff = value2 - value;
    }

    return diff;
}

// ****************************************
// Initialize Clusters
// ****************************************
void emberAfInit(chip::Messaging::ExchangeManager * exchangeMgr)
{
    uint8_t i;
#ifdef EMBER_AF_ENABLE_STATISTICS
    afNumPktsSent = 0;
#endif

    emAfExchangeMgr = exchangeMgr;

    for (i = 0; i < EMBER_SUPPORTED_NETWORKS; i++)
    {
        // FIXME: Do we need to support more than one network?
        // emberAfPushNetworkIndex(i);
        emberAfInitializeAttributes(EMBER_BROADCAST_ENDPOINT);
        // emberAfPopNetworkIndex();
    }

    memset(afDeviceEnabled, true, emberAfEndpointCount());

    MATTER_PLUGINS_INIT

    emAfCallInits();
}

void emberAfTick(void)
{
    // Call the AFV2-specific per-endpoint callbacks
    // Anything that defines callbacks as void *TickCallback(void) is called in
    // emAfInit(), which is a generated file
#ifdef EMBER_AF_GENERATED_PLUGIN_TICK_FUNCTION_CALLS
    EMBER_AF_GENERATED_PLUGIN_TICK_FUNCTION_CALLS
#endif
}

// Cluster init functions that don't have a cluster implementation to define
// them in.
void MatterBooleanStatePluginServerInitCallback() {}
void MatterBridgedDeviceBasicPluginServerInitCallback() {}
void MatterElectricalMeasurementPluginServerInitCallback() {}
void MatterRelativeHumidityMeasurementPluginServerInitCallback() {}
void MatterIlluminanceMeasurementPluginServerInitCallback() {}
void MatterBinaryInputBasicPluginServerInitCallback() {}
void MatterPressureMeasurementPluginServerInitCallback() {}
void MatterTemperatureMeasurementPluginServerInitCallback() {}
void MatterFlowMeasurementPluginServerInitCallback() {}
void MatterOnOffSwitchConfigurationPluginServerInitCallback() {}
void MatterThermostatUserInterfaceConfigurationPluginServerInitCallback() {}
void MatterBridgedDeviceBasicInformationPluginServerInitCallback() {}
void MatterPowerConfigurationPluginServerInitCallback() {}
void MatterPowerProfilePluginServerInitCallback() {}
void MatterPulseWidthModulationPluginServerInitCallback() {}
void MatterAlarmsPluginServerInitCallback() {}
void MatterTimePluginServerInitCallback() {}
void MatterAclPluginServerInitCallback() {}
void MatterPollControlPluginServerInitCallback() {}
void MatterUnitLocalizationPluginServerInitCallback() {}
void MatterTimeSynchronizationPluginServerInitCallback() {}
void MatterProxyValidPluginServerInitCallback() {}
void MatterProxyDiscoveryPluginServerInitCallback() {}
void MatterProxyConfigurationPluginServerInitCallback() {}
void MatterFanControlPluginServerInitCallback() {}

// ****************************************
// This function is called by the application when the stack goes down,
// such as after a leave network. This allows zcl utils to clear state
// that should not be kept when changing networks
// ****************************************
void emberAfStackDown(void)
{
    emberAfRegistrationAbortCallback();
}

// ****************************************
// Print out information about each cluster
// ****************************************

uint16_t emberAfFindClusterNameIndex(ClusterId cluster)
{
    static_assert(sizeof(ClusterId) == 4, "May need to adjust our index type or somehow define it in terms of cluster id type");
    uint16_t index = 0;
    while (zclClusterNames[index].id != ZCL_NULL_CLUSTER_ID)
    {
        if (zclClusterNames[index].id == cluster)
        {
            return index;
        }
        index++;
    }
    return 0xFFFF;
}

// This function parses into the cluster name table, and tries to find
// the index in the table that has the right cluster id.
void emberAfDecodeAndPrintCluster(ClusterId cluster)
{
    uint16_t index = emberAfFindClusterNameIndex(cluster);
    if (index == 0xFFFF)
    {
        static_assert(sizeof(ClusterId) == 4, "Adjust the print formatting");
        emberAfPrint(emberAfPrintActiveArea, "(Unknown clus. [" ChipLogFormatMEI "])", ChipLogValueMEI(cluster));
    }
    else
    {
        emberAfPrint(emberAfPrintActiveArea, "(%p)", zclClusterNames[index].name);
    }
}

// This function makes the assumption that
// emberAfCurrentCommand will either be NULL
// when invalid, or will have a valid mfgCode
// when called.
// If it is invalid, we just return the
// EMBER_AF_NULL_MANUFACTURER_CODE, which we tend to use
// for references to the standard library.
uint16_t emberAfGetMfgCodeFromCurrentCommand(void)
{
    if (emberAfCurrentCommand() != nullptr)
    {
        return emberAfCurrentCommand()->mfgCode;
    }

    return EMBER_AF_NULL_MANUFACTURER_CODE;
}

uint8_t emberAfNextSequence(void)
{
    return ((++emberAfSequenceNumber) & EMBER_AF_ZCL_SEQUENCE_MASK);
}

uint8_t emberAfGetLastSequenceNumber(void)
{
    return (emberAfSequenceNumber & EMBER_AF_ZCL_SEQUENCE_MASK);
}

// the caller to the library can set a flag to say do not respond to the
// next ZCL message passed in to the library. Passing true means disable
// the reply for the next ZCL message. Setting to false re-enables the
// reply (in the case where the app disables it and then doesnt send a
// message that gets parsed).
void emberAfSetNoReplyForNextMessage(bool set)
{
    if (set)
    {
        emberAfResponseType |= ZCL_UTIL_RESP_NONE;
    }
    else
    {
        emberAfResponseType = static_cast<uint8_t>(emberAfResponseType & ~ZCL_UTIL_RESP_NONE);
    }
}

void emberAfSetRetryOverride(EmberAfRetryOverride value)
{
    emberAfApsRetryOverride = value;
}

EmberAfRetryOverride emberAfGetRetryOverride(void)
{
    return (EmberAfRetryOverride) emberAfApsRetryOverride;
}

void emAfApplyRetryOverride(EmberApsOption * options)
{
    if (options == nullptr)
    {
        return;
    }
    if (emberAfApsRetryOverride == EMBER_AF_RETRY_OVERRIDE_SET)
    {
        *options |= EMBER_APS_OPTION_RETRY;
    }
    else if (emberAfApsRetryOverride == EMBER_AF_RETRY_OVERRIDE_UNSET)
    {
        *options = static_cast<EmberApsOption>(*options & ~EMBER_APS_OPTION_RETRY);
    }
    else
    {
        // MISRA requires ..else if.. to have terminating else.
    }
}

void emberAfSetDisableDefaultResponse(EmberAfDisableDefaultResponse value)
{
    emAfDisableDefaultResponse = value;
    if (value != EMBER_AF_DISABLE_DEFAULT_RESPONSE_ONE_SHOT)
    {
        emAfSavedDisableDefaultResponseVale = value;
    }
}

EmberAfDisableDefaultResponse emberAfGetDisableDefaultResponse(void)
{
    return (EmberAfDisableDefaultResponse) emAfDisableDefaultResponse;
}

void emAfApplyDisableDefaultResponse(uint8_t * frame_control)
{
    if (frame_control == nullptr)
    {
        return;
    }
    if (emAfDisableDefaultResponse == EMBER_AF_DISABLE_DEFAULT_RESPONSE_ONE_SHOT)
    {
        emAfDisableDefaultResponse = emAfSavedDisableDefaultResponseVale;
        *frame_control |= ZCL_DISABLE_DEFAULT_RESPONSE_MASK;
    }
    else if (emAfDisableDefaultResponse == EMBER_AF_DISABLE_DEFAULT_RESPONSE_PERMANENT)
    {
        *frame_control |= ZCL_DISABLE_DEFAULT_RESPONSE_MASK;
    }
    else
    {
        // MISRA requires ..else if.. to have terminating else.
    }
}

EmberStatus emberAfSendImmediateDefaultResponse(EmberAfStatus status)
{
    return emberAfSendDefaultResponse(emberAfCurrentCommand(), status);
}

EmberStatus emberAfSendDefaultResponse(const EmberAfClusterCommand * cmd, EmberAfStatus status)
{
    // Default Response commands are only sent in response to unicast commands.
    if (cmd->type != EMBER_INCOMING_UNICAST && cmd->type != EMBER_INCOMING_UNICAST_REPLY)
    {
        return EMBER_SUCCESS;
    }

    if (!chip::app::Compatibility::IMEmberAfSendDefaultResponseWithCallback(status))
    {
        // Caller is not responding to anything!
        return EMBER_ERR_FATAL;
    }

    return EMBER_SUCCESS;
}

void emberAfCopyInt16u(uint8_t * data, uint16_t index, uint16_t x)
{
    data[index]     = (uint8_t)(((x)) & 0xFF);
    data[index + 1] = (uint8_t)(((x) >> 8) & 0xFF);
}

void emberAfCopyInt24u(uint8_t * data, uint16_t index, uint32_t x)
{
    data[index]     = (uint8_t)(((x)) & 0xFF);
    data[index + 1] = (uint8_t)(((x) >> 8) & 0xFF);
    data[index + 2] = (uint8_t)(((x) >> 16) & 0xFF);
}

void emberAfCopyInt32u(uint8_t * data, uint16_t index, uint32_t x)
{
    data[index]     = (uint8_t)(((x)) & 0xFF);
    data[index + 1] = (uint8_t)(((x) >> 8) & 0xFF);
    data[index + 2] = (uint8_t)(((x) >> 16) & 0xFF);
    data[index + 3] = (uint8_t)(((x) >> 24) & 0xFF);
}

void emberAfCopyString(uint8_t * dest, const uint8_t * src, size_t size)
{
    if (src == nullptr)
    {
        dest[0] = 0; // Zero out the length of string
    }
    else if (src[0] == 0xFF)
    {
        dest[0] = src[0];
    }
    else
    {
        uint8_t length = emberAfStringLength(src);
        if (size < length)
        {
            // Since we have checked that size < length, size must be able to fit into the type of length.
            length = static_cast<decltype(length)>(size);
        }
        memmove(dest + 1, src + 1, length);
        dest[0] = length;
    }
}

void emberAfCopyLongString(uint8_t * dest, const uint8_t * src, size_t size)
{
    if (src == nullptr)
    {
        dest[0] = dest[1] = 0; // Zero out the length of string
    }
    else if ((src[0] == 0xFF) && (src[1] == 0xFF))
    {
        dest[0] = 0xFF;
        dest[1] = 0xFF;
    }
    else
    {
        uint16_t length = emberAfLongStringLength(src);
        if (size < length)
        {
            // Since we have checked that size < length, size must be able to fit into the type of length.
            length = static_cast<decltype(length)>(size);
        }
        memmove(dest + 2, src + 2, length);
        dest[0] = EMBER_LOW_BYTE(length);
        dest[1] = EMBER_HIGH_BYTE(length);
    }
}

#if (BIGENDIAN_CPU)
#define EM_BIG_ENDIAN true
#else
#define EM_BIG_ENDIAN false
#endif

// You can pass in val1 as NULL, which will assume that it is
// pointing to an array of all zeroes. This is used so that
// default value of NULL is treated as all zeroes.
int8_t emberAfCompareValues(const uint8_t * val1, const uint8_t * val2, uint16_t len, bool signedNumber)
{
    if (len == 0)
    {
        // no length means nothing to compare.  Shouldn't even happen, since len is sizeof(some-integer-type).
        return 0;
    }

    if (signedNumber)
    { // signed number comparison
        if (len <= 4)
        { // only number with 32-bits or less is supported
            int32_t accum1 = 0x0;
            int32_t accum2 = 0x0;
            int32_t all1s  = -1;

            for (uint16_t i = 0; i < len; i++)
            {
                uint8_t j = (val1 == nullptr ? 0 : (EM_BIG_ENDIAN ? val1[i] : val1[(len - 1) - i]));
                accum1 |= j << (8 * (len - 1 - i));

                uint8_t k = (EM_BIG_ENDIAN ? val2[i] : val2[(len - 1) - i]);
                accum2 |= k << (8 * (len - 1 - i));
            }

            // sign extending, no need for 32-bits numbers
            if (len < 4)
            {
                if ((accum1 & (1 << (8 * len - 1))) != 0)
                { // check sign
                    accum1 |= all1s - ((1 << (len * 8)) - 1);
                }
                if ((accum2 & (1 << (8 * len - 1))) != 0)
                { // check sign
                    accum2 |= all1s - ((1 << (len * 8)) - 1);
                }
            }

            if (accum1 > accum2)
            {
                return 1;
            }
            if (accum1 < accum2)
            {
                return -1;
            }

            return 0;
        }

        // not supported
        return 0;
    }

    // regular unsigned number comparison
    for (uint16_t i = 0; i < len; i++)
    {
        uint8_t j = (val1 == nullptr ? 0 : (EM_BIG_ENDIAN ? val1[i] : val1[(len - 1) - i]));
        uint8_t k = (EM_BIG_ENDIAN ? val2[i] : val2[(len - 1) - i]);

        if (j > k)
        {
            return 1;
        }
        if (k > j)
        {
            return -1;
        }
    }
    return 0;
}

#if 0
// Moving to time-util.c
int8_t emberAfCompareDates(EmberAfDate* date1, EmberAfDate* date2)
{
  uint32_t val1 = emberAfEncodeDate(date1);
  uint32_t val2 = emberAfEncodeDate(date2);
  return (val1 == val2) ? 0 : ((val1 < val2) ? -1 : 1);
}
#endif

// Zigbee spec says types between signed 8 bit and signed 64 bit
bool emberAfIsTypeSigned(EmberAfAttributeType dataType)
{
    return (dataType >= ZCL_INT8S_ATTRIBUTE_TYPE && dataType <= ZCL_INT64S_ATTRIBUTE_TYPE);
}

EmberStatus emberAfEndpointEventControlSetInactive(EmberEventControl * controls, EndpointId endpoint)
{
    uint16_t index = emberAfIndexFromEndpoint(endpoint);
    if (index == 0xFFFF)
    {
        return EMBER_INVALID_ENDPOINT;
    }
    emberEventControlSetInactive(&controls[index]);
    return EMBER_SUCCESS;
}

bool emberAfEndpointEventControlGetActive(EmberEventControl * controls, EndpointId endpoint)
{
    uint16_t index = emberAfIndexFromEndpoint(endpoint);
    return (index != 0xFFFF && emberEventControlGetActive(&controls[index]));
}

EmberStatus emberAfEndpointEventControlSetActive(EmberEventControl * controls, EndpointId endpoint)
{
    uint16_t index = emberAfIndexFromEndpoint(endpoint);
    if (index == 0xFFFF)
    {
        return EMBER_INVALID_ENDPOINT;
    }
    emberEventControlSetActive(&controls[index]);
    return EMBER_SUCCESS;
}

uint8_t emberAfAppendCharacters(uint8_t * zclString, uint8_t zclStringMaxLen, const uint8_t * appendingChars,
                                uint8_t appendingCharsLen)
{
    uint8_t freeChars;
    uint8_t curLen;
    uint8_t charsToWrite;

    if ((zclString == nullptr) || (zclStringMaxLen == 0) || (appendingChars == nullptr) || (appendingCharsLen == 0))
    {
        return 0;
    }

    curLen = emberAfStringLength(zclString);

    if ((zclString[0] == 0xFF) || (curLen >= zclStringMaxLen))
    {
        return 0;
    }

    freeChars    = static_cast<uint8_t>(zclStringMaxLen - curLen);
    charsToWrite = (freeChars > appendingCharsLen) ? appendingCharsLen : freeChars;

    memcpy(&zclString[1 + curLen], // 1 is to account for zcl's length byte
           appendingChars, charsToWrite);
    // Cast is safe, because the sum can't be bigger than zclStringMaxLen.
    zclString[0] = static_cast<uint8_t>(curLen + charsToWrite);
    return charsToWrite;
}

/*
   On each page, first channel maps to channel number zero and so on.
   Example:
   page    Band      Rage of 90 channels    Per page channel mapping
   28     863 MHz        0-26                    0-26
   29     863 MHz        27-34,62                0-8 (Here 7th channel maps to 34 and 8th to 62)
   30     863 MHz        35 - 61                 0-26
   31     915            0-26                    0-26

 */
EmberStatus emAfValidateChannelPages(uint8_t page, uint8_t channel)
{
    switch (page)
    {
    case 0:
        if (!((channel <= EMBER_MAX_802_15_4_CHANNEL_NUMBER) &&
              ((EMBER_MIN_802_15_4_CHANNEL_NUMBER == 0) || (channel >= EMBER_MIN_802_15_4_CHANNEL_NUMBER))))
        {
            return EMBER_PHY_INVALID_CHANNEL;
        }
        break;
    case 28:
    case 30:
    case 31:
        if (channel > EMBER_MAX_SUBGHZ_CHANNEL_NUMBER_ON_PAGES_28_30_31)
        {
            return EMBER_PHY_INVALID_CHANNEL;
        }
        break;
    case 29:
        if (channel > EMBER_MAX_SUBGHZ_CHANNEL_NUMBER_ON_PAGE_29)
        {
            return EMBER_PHY_INVALID_CHANNEL;
        }
        break;
    default:
        return EMBER_PHY_INVALID_CHANNEL;
        break;
    }
    return EMBER_SUCCESS;
}

void slabAssert(const char * file, int line)
{
    (void) file; // Unused parameter
    (void) line; // Unused parameter
    // Wait forever until the watchdog fires
    while (true)
    {
    }
}

#define ENCODED_8BIT_CHANPG_PAGE_MASK 0xE0         // top 3 bits
#define ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_0 0x00  // 0b000xxxxx
#define ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_28 0x80 // 0b100xxxxx
#define ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_29 0xA0 // 0b101xxxxx
#define ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_30 0xC0 // 0b110xxxxx
#define ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_31 0xE0 // 0b111xxxxx

#define ENCODED_8BIT_CHANPG_CHANNEL_MASK 0x1F // bottom 5 bits

uint8_t emberAfGetPageFrom8bitEncodedChanPg(uint8_t chanPg)
{
    switch (chanPg & ENCODED_8BIT_CHANPG_PAGE_MASK)
    {
    case ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_0:
        return 0;
    case ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_28:
        return 28;
    case ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_29:
        return 29;
    case ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_30:
        return 30;
    case ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_31:
        return 31;
    default:
        return 0xFF;
    }
}

uint8_t emberAfGetChannelFrom8bitEncodedChanPg(uint8_t chanPg)
{
    return chanPg & ENCODED_8BIT_CHANPG_CHANNEL_MASK;
}

uint8_t emberAfMake8bitEncodedChanPg(uint8_t page, uint8_t channel)
{
    if (emAfValidateChannelPages(page, channel) != EMBER_SUCCESS)
    {
        return 0xFF;
    }

    switch (page)
    {
    case 28:
        return channel | ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_28;
    case 29:
        return channel | ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_29;
    case 30:
        return channel | ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_30;
    case 31:
        return channel | ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_31;
    default:
        // Strictly speaking, we only need case 0 here, but MISRA in its infinite
        // wisdom requires a default case. Since we have validated the arguments
        // already, and 0 is the only remaining case, we simply treat the default
        // as case 0 to make MISRA happy.
        return channel | ENCODED_8BIT_CHANPG_PAGE_MASK_PAGE_0;
    }
}

bool emberAfContainsAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId)
{
    return (emberAfGetServerAttributeIndexByAttributeId(endpoint, clusterId, attributeId) != UINT16_MAX);
}

bool emberAfIsKnownVolatileAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId)
{
    const EmberAfAttributeMetadata * metadata = emberAfLocateAttributeMetadata(endpoint, clusterId, attributeId);

    if (metadata == nullptr)
    {
        return false;
    }

    return !metadata->IsAutomaticallyPersisted() && !metadata->IsExternal();
}

chip::Messaging::ExchangeManager * chip::ExchangeManager()
{
    return emAfExchangeMgr;
}
