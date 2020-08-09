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
/****************************************************************************
 * @file
 * @brief This is the source for the command line
 *interface used for the ias zone server plugin.
 *******************************************************************************
 ******************************************************************************/

#include "af.h"
#include "ias-zone-server.h"

//-----------------------------------------------------------------------------
// Globals

#define RESERVED_END 0x7FFF
#define MANUFACTURER_SPECIFIC_START 0x8000
#define MANUFACTURER_SPECIFIC_END 0xFFFE

void emberAfPluginIasZoneServerInfoCommand(void);
void emberAfPluginIasZoneServerChangeStatusCommand(void);
void emberAfPluginIasZoneServerSetEnrollmentMethodCommand(void);

static const char * infoArguments[] = {
    "endpoint",
    NULL,
};

static const char * changeStatusArguments[] = {
    "new-status",
    "time-since-occurred-seconds",
    "endpoint",
    NULL,
};

static const char * setEnrollmentModeArguments[] = {
    "endpoint",
    "enrollmentMode",
    NULL,
};

EmberCommandEntry emberAfPluginIasZoneServerCommands[] = {
    emberCommandEntryActionWithDetails("info", emberAfPluginIasZoneServerInfoCommand, "", "Print IAS Zone information",
                                       infoArguments),
    emberCommandEntryActionWithDetails("change-status", emberAfPluginIasZoneServerChangeStatusCommand, "vu",
                                       "Change the current Zone Status", changeStatusArguments),
    emberCommandEntryActionWithDetails("set-enrollment-mode", emberAfPluginIasZoneServerSetEnrollmentMethodCommand, "uu",
                                       "Change the current enrollment method", setEnrollmentModeArguments),
    emberCommandEntryTerminator(),
};

typedef struct
{
    uint16_t zoneType;
    const char * zoneTypeString;
} ZoneTypeToStringMap;

// These functions and variables are only used to generate strings used with
// emberAfIasZoneClusterPrintln calls, so if EMBER_AF_PRINT_IAS_ZONE_CLUSTER is
// not defined and they are compiled in, these declarations generate "function
// declared but never referenced" warnings
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_IAS_ZONE_CLUSTER)
static ZoneTypeToStringMap zoneTypeToStringMap[] = {
    { 0x0000, "Standard CIE" },
    { 0x000d, "Motion Sensor" },
    { 0x0015, "Contact Switch" },
    { 0x0028, "Fire Sensor" },
    { 0x002a, "Water Sensor" },
    { 0x002b, "Gas Sensor" },
    { 0x002c, "Peersonal Emergency Device" },
    { 0x002d, "Vibration / Movement Sensor" },
    { 0x010f, "Remote Control" },
    { 0x0115, "Key Fob" },
    { 0x021d, "Keypad" },
    { 0x0225, "Standard Warning Device" },
    { 0xFFFF, NULL } // terminator
};

static const char manufacturerSpecificString[] = "Manufacturer Specific";
static const char invalidZoneTypeString[]      = "Invalid";
static const char reservedString[]             = "Reserved";

static const char notEnrolledString[]      = "NOT Enrolled";
static const char enrolledString[]         = "Enrolled";
static const char unknownZoneStateString[] = "Unknown";

//-----------------------------------------------------------------------------
// Functions

static const char * getZoneTypeString(uint16_t type)
{
    uint16_t i = 0;
    while (zoneTypeToStringMap[i].zoneTypeString != NULL)
    {
        if (zoneTypeToStringMap[i].zoneType == type)
        {
            return zoneTypeToStringMap[i].zoneTypeString;
        }
        i++;
    }

    if (type <= RESERVED_END)
    {
        return reservedString;
    }

    if (type >= MANUFACTURER_SPECIFIC_START && type <= MANUFACTURER_SPECIFIC_END)
    {
        return manufacturerSpecificString;
    }

    return invalidZoneTypeString;
}

static const char * getZoneStateString(uint8_t zoneState)
{
    switch (zoneState)
    {
    case EMBER_ZCL_IAS_ZONE_STATE_ENROLLED:
        return enrolledString;
    case EMBER_ZCL_IAS_ZONE_STATE_NOT_ENROLLED:
        return notEnrolledString;
    }
    return unknownZoneStateString;
}
#endif // defined(EMBER_AF_PRINT_ENABLE)
//&& defined(EMBER_AF_PRINT_IAS_ZONE_CLUSTER)

static void getAttributes(uint8_t * returnCieAddress, uint16_t * returnZoneStatus, uint16_t * returnZoneType,
                          uint8_t * returnZoneState, uint8_t endpoint)
{
    EMBER_TEST_ASSERT(endpoint != EM_AF_UNKNOWN_ENDPOINT);

    emberAfReadServerAttribute(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID, returnCieAddress, EUI64_SIZE);

    emberAfReadServerAttribute(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_STATUS_ATTRIBUTE_ID, (uint8_t *) returnZoneStatus,
                               2); // uint16_t size

    emberAfReadServerAttribute(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_TYPE_ATTRIBUTE_ID, (uint8_t *) returnZoneType,
                               2); // uint16_t size

    emberAfReadServerAttribute(endpoint, ZCL_IAS_ZONE_CLUSTER_ID, ZCL_ZONE_STATE_ATTRIBUTE_ID, (uint8_t *) returnZoneState,
                               1); // uint8_t size
}

void emberAfPluginIasZoneServerInfoCommand(void)
{
    uint8_t cieAddress[EUI64_SIZE];
    uint16_t zoneStatus;
    uint16_t zoneType;
    uint8_t zoneState;
    uint8_t endpoint = (uint8_t) emberUnsignedCommandArgument(0);

    getAttributes(cieAddress, &zoneStatus, &zoneType, &zoneState, endpoint);
    emberAfIasZoneClusterPrint("CIE Address: ");
    emberAfPrintBigEndianEui64(cieAddress);
    emberAfIasZoneClusterPrintln("");
    emberAfIasZoneClusterPrintln("Zone Type:   0x%2X (%p)", zoneType, getZoneTypeString(zoneType));
    emberAfIasZoneClusterPrintln("Zone State:  0x%X   (%p)", zoneState, getZoneStateString(zoneState));
    emberAfIasZoneClusterPrintln("Zone Status: 0x%2X", zoneStatus);
    emberAfIasZoneClusterPrintln("Zone ID:     0x%2X", emberAfPluginIasZoneServerGetZoneId(endpoint));
}

void emberAfPluginIasZoneServerChangeStatusCommand(void)
{
    uint16_t newStatus               = (uint16_t) emberUnsignedCommandArgument(0);
    uint8_t timeSinceOccurredSeconds = (uint8_t) emberUnsignedCommandArgument(1);
    uint8_t endpoint                 = (uint8_t) emberUnsignedCommandArgument(2);
    emberAfPluginIasZoneServerUpdateZoneStatus(endpoint, newStatus, timeSinceOccurredSeconds << 2);
}

void emberAfPluginIasZoneServerSetEnrollmentMethodCommand(void)
{
    uint8_t endpoint       = (uint8_t) emberUnsignedCommandArgument(0);
    uint8_t enrollmentMode = (uint8_t) emberUnsignedCommandArgument(1);
    emberAfPluginIasZoneClusterSetEnrollmentMethod(endpoint, enrollmentMode);
}

void emberAfPluginIasZoneServerChangeBackoffConfiguration(void)
{
#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
    IasZoneStatusQueueRetryConfig retryConfig = { (uint8_t) emberUnsignedCommandArgument(0),
                                                  (uint8_t) emberUnsignedCommandArgument(1),
                                                  (uint32_t) emberUnsignedCommandArgument(2),
                                                  (bool) emberUnsignedCommandArgument(3),
                                                  (uint16_t) emberUnsignedCommandArgument(4) };

    if (EMBER_BAD_ARGUMENT == emberAfIasZoneServerConfigStatusQueueRetryParams(&retryConfig))
    {
        emberAfIasZoneClusterPrintln("Parameter error! Valid parameters: %s, %s, %s %d, %s.", "0 < firstBackoff", "0 < commonRatio",
                                     "firstBackoff < maxBackoff <", IAS_ZONE_STATUS_QUEUE_RETRY_ABS_MAX_BACKOFF_TIME_SEC,
                                     "0 < maxRetryAttempts");
    }
#else
    emberAfIasZoneClusterPrintln("Command error! IAS Zone Server status queue is not enabled");
#endif
}

void emberAfPluginIasZoneServerCliPrintQueue(void)
{
#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
    emberAfPluginIasZoneServerPrintQueue();
#else
    emberAfIasZoneClusterPrintln("Command error! IAS Zone Server status queue is not enabled");
#endif
}

void emberAfPluginIasZoneServerCliPrintQueueConfig(void)
{
#if defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ENABLE_QUEUE)
    emberAfPluginIasZoneServerPrintQueueConfig();
#else
    emberAfIasZoneClusterPrintln("Command error! IAS Zone Server status queue is not enabled");
#endif
}
