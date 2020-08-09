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
 * @brief Unit tests for ias zone server plugin
 *******************************************************************************
 ******************************************************************************/

#include "app/framework/include/af.h"
#include "ias-zone-server.h"

#include "app/framework/plugin/ias-zone-server/ias-zone-server-test.h"
#include "app/framework/test/test-framework.h"

#define NUM_ENDPOINTS 3
#define UNDEFINED_ZONE_ID 0xFF

// These are non-public in ias-zone-server.c, so make a copy here.
#define IAS_ZONE_SERVER_PAYLOAD_COMMAND_IDX 0x02
#define ZCL_FRAME_CONTROL_IDX 0x00
#define RETRY_TEST_QUEUE_ENTRIES (3)
#define RETRY_TEST_MAKE_IT_UNLIMITED (10)

// This is non-public in ias-zone-server.c, so make a copy here.
typedef struct
{
    uint8_t endpoint;
    uint16_t status;
    uint32_t eventTimeMs;
} IasZoneStatusQueueEntry;

// This is non-public in ias-zone-server.c, so make a copy here.
typedef struct
{
    uint8_t entriesInQueue;
    uint8_t startIdx;
    uint8_t lastIdx;
    IasZoneStatusQueueEntry buffer[EMBER_AF_PLUGIN_IAS_ZONE_SERVER_QUEUE_DEPTH];
} IasZoneStatusQueue;

void emberAfPluginIasZoneServerStackStatusCallback(EmberStatus status);

// State variables used to stub out framework functionality
uint8_t currentEndpoint = 0;
EmberBindingTableEntry dummyBind;
EmberApsFrame frame;
EmberAfClusterCommand command;
EmberAfClusterCommand * emAfCurrentCommand = &command;
EmberAfDefinedEndpoint emAfEndpoints[NUM_ENDPOINTS];

// State variables used for later expect statements
bool attributeWritten;
bool bindSet                   = false;
uint8_t CurrentCieAddress[]    = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t zoneIdClearedCount     = 0;
uint8_t cieAddressClearedCount = 0;
uint8_t zoneTypeClearedCount   = 0;
uint8_t lastEndpointWritten    = 0;

extern IasZoneStatusQueue messageQueue;
extern EmberEventControl emberAfPluginIasZoneServerManageQueueEventControl;

// -----------------------------------------------------------------------------
// stubbed out functions: In general, we don't care about these functions; they
// just need to compile and always return success when called at runtime
// -----------------------------------------------------------------------------

bool emberAfStartMoveCallback(void)
{
    return true;
}

bool emberAfContainsServer(uint8_t endpoint, EmberAfClusterId clusterId)
{
    return true;
}

EmberStatus emberLeaveNetwork(void)
{
    return EMBER_SUCCESS;
}

EmberAfAttributeMetadata * emberAfLocateAttributeMetadata(uint8_t endpoint, EmberAfClusterId clusterId,
                                                          EmberAfAttributeId attributeId, uint8_t mask, uint16_t manufacturerCode)
{
    return NULL;
}

void halInternalGetTokenData(void * data, uint16_t token, uint8_t index, uint8_t len)
{
    return;
}

void halInternalSetTokenData(uint16_t token, uint8_t index, void * data, uint8_t len) {}

uint32_t halCommonGetInt32uMillisecondTick(void)
{
    return 0;
}

// -----------------------------------------------------------------------------
// test implemented functions: These functions need to be more representative of
// how the system will work
// -----------------------------------------------------------------------------

static const uint8_t attributeSizes[] = {
#include "attribute-size.h"
};

uint8_t emberAfGetDataSize(uint8_t dataType)
{
    uint8_t i;
    for (i = 0; (i + 1) < sizeof(attributeSizes); i += 2)
    {
        if (attributeSizes[i] == dataType)
        {
            return attributeSizes[i + 1];
        }
    }

    return 0;
}

EmberStatus emberGetBinding(uint8_t idx, EmberBindingTableEntry * entry)
{
    entry->type = EMBER_UNUSED_BINDING;
    bindSet     = true;

    return EMBER_SUCCESS;
}

EmberAfStatus emberAfReadServerAttribute(uint8_t endpoint, EmberAfClusterId cluster, EmberAfAttributeId attributeID,
                                         uint8_t * dataPtr, uint8_t readLength)
{
    uint8_t i;

    if (attributeID != ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID)
    {
        return -1;
    }

    for (i = 0; i < 8; i++)
    {
        dataPtr[i] = CurrentCieAddress[i];
    }

    return EMBER_SUCCESS;
}

uint8_t emberAfEndpointCount()
{
    return NUM_ENDPOINTS;
}

uint8_t emberAfEndpointFromIndex(uint8_t idx)
{
    currentEndpoint = idx + 1;
    return currentEndpoint;
}

EmberAfStatus emberAfWriteServerAttribute(uint8_t endpoint, EmberAfClusterId cluster, EmberAfAttributeId attributeID,
                                          uint8_t * dataPtr, uint8_t emberSetBindingataType)
{
    uint8_t zoneId;
    uint16_t zoneType;
    uint8_t i;
    bool zeroAddress = true;

    lastEndpointWritten = endpoint;

    if (attributeID == ZCL_ZONE_ID_ATTRIBUTE_ID)
    {
        zoneId = *dataPtr;
        if (zoneId == UNDEFINED_ZONE_ID)
        {
            zoneIdClearedCount++;
            return EMBER_SUCCESS;
        }
    }
    else if (attributeID == ZCL_ZONE_TYPE_ATTRIBUTE_ID)
    {
        zoneType = *(uint16_t *) dataPtr;
        if (zoneType == EMBER_AF_PLUGIN_IAS_ZONE_SERVER_ZONE_TYPE)
        {
            zoneTypeClearedCount++;
        }
    }
    else if (attributeID == ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID)
    {
        for (i = 0; i < 8; i++)
        {
            if (dataPtr[i] != 0)
            {
                zeroAddress = false;
            }
        }
        if (zeroAddress)
        {
            cieAddressClearedCount++;
        }
    }
    attributeWritten = true;
    return 0;
}

EmberStatus emberSetBinding(uint8_t index, EmberBindingTableEntry * value)
{
    bindSet = true;
    return 0;
}

// -----------------------------------------------------------------------------
// Test cases: These are the test cases called by the test framework when unit
// tests are run.
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Test case: enrollTest
// -----------------------------------------------------------------------------
// Verify that binding table entries are created when a new CIE address is
// written to the device
static void enrollTest(void)
{
    uint8_t endpoint                   = 1;
    EmberAfAttributeId attributeIdCie  = ZCL_IAS_CIE_ADDRESS_ATTRIBUTE_ID;
    EmberAfAttributeType attributeType = 0;
    uint8_t size                       = EUI64_SIZE;
    uint8_t newCieAddress[]            = { 1, 2, 3, 4, 5, 6, 7, 8 };
    uint8_t i;

    // bind is created

    // Verify that no extra action happens when we send an attribute other than
    // the CIE_ADDRESS_ATTRIBUTE
    emAfCurrentCommand->apsFrame                 = &frame;
    emAfCurrentCommand->apsFrame->sourceEndpoint = 1;

    for (i = 0; i < 8; i++)
    {
        CurrentCieAddress[i] = 0;
    }
    bindSet = false;

    emberAfIasZoneClusterServerPreAttributeChangedCallback(endpoint, attributeIdCie, attributeType, size, newCieAddress);

    expectComparisonDecimal(true, bindSet, "expected binding entry state", "observed binding entry state");
}

// -----------------------------------------------------------------------------
// Test case: leaveTest
// -----------------------------------------------------------------------------
// Verify that on network leave, attributes are reset to default values
static void leaveTest(void)
{
    currentEndpoint        = 0;
    zoneIdClearedCount     = 0;
    cieAddressClearedCount = 0;
    zoneTypeClearedCount   = 0;

    // EMAPPFWKV2-1530: if we are told that are network is down, but we are
    // rejoining, then we should not unenroll.
    testFrameworkNetworkState = EMBER_JOINED_NETWORK_NO_PARENT;
    emberAfPluginIasZoneServerStackStatusCallback(EMBER_NETWORK_DOWN);
    expectComparisonDecimal(0, zoneTypeClearedCount, "expected number of zone Type cleared",
                            "observed number of zone Type cleared");
    expectComparisonDecimal(0, cieAddressClearedCount, "expected number of CieAddr cleared", "observed number of CieAddr cleared");
    expectComparisonDecimal(0, zoneIdClearedCount, "expected number of zone ID cleared", "observed number of zone ID cleared");

    // If we are told that are network is really down, then we do unenroll.
    testFrameworkNetworkState = EMBER_NO_NETWORK;
    emberAfPluginIasZoneServerStackStatusCallback(EMBER_NETWORK_DOWN);
    expectComparisonDecimal(NUM_ENDPOINTS, zoneTypeClearedCount, "expected number of zone Type cleared",
                            "observed number of zone Type cleared");
    expectComparisonDecimal(NUM_ENDPOINTS, cieAddressClearedCount, "expected number of CieAddr cleared",
                            "observed number of CieAddr cleared");
    expectComparisonDecimal(NUM_ENDPOINTS, zoneIdClearedCount, "expected number of zone ID cleared",
                            "observed number of zone ID cleared");
}

// -----------------------------------------------------------------------------
// Test case: updateTest
// -----------------------------------------------------------------------------
// Verify that updates are sent to the correct endpoint when updateStatus
// function is called
static void updateTest(void)
{
    uint8_t testEndpoint = 0;

    for (testEndpoint = 0; testEndpoint < NUM_ENDPOINTS; testEndpoint++)
    {
        emberAfPluginIasZoneServerUpdateZoneStatus(testEndpoint, 0, 0);
        expectComparisonDecimal(testEndpoint, lastEndpointWritten, "expected attribute write to endpoint",
                                "observed attribute write to endpoint");
    }
}

// -----------------------------------------------------------------------------
// Test case: status queue retry tests
// -----------------------------------------------------------------------------

#define testConfigStatusQueueRetryParamsBadArgument()                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        EmberStatus status = emberAfIasZoneServerConfigStatusQueueRetryParams(&retryConfig);                                       \
        expectComparisonDecimal(EMBER_BAD_ARGUMENT, status, "expected return status", "observed return status");                   \
    } while (0)

// Verify that status queue configuration parameter check is operating per the current
// requirements. Both parameter limits and the input parameters are tested.
void statusQueueParameterTest(uint8_t firstBackoffTimeSec, uint8_t backoffSeqCommonRatio, uint32_t maxBackoffTimeSec,
                              bool unlimitedRetries, uint8_t maxRetryAttempts)
{
    // Filling up retryConfig with the input parameters of this function
    // and changing each parameters to an invalid value one by one so that
    // parameter check is tested.
    IasZoneStatusQueueRetryConfig retryConfig = { firstBackoffTimeSec, backoffSeqCommonRatio, maxBackoffTimeSec, unlimitedRetries,
                                                  maxRetryAttempts };

    retryConfig.firstBackoffTimeSec = 0;
    testConfigStatusQueueRetryParamsBadArgument();
    retryConfig.firstBackoffTimeSec = firstBackoffTimeSec;

    retryConfig.backoffSeqCommonRatio = 0;
    testConfigStatusQueueRetryParamsBadArgument();
    retryConfig.backoffSeqCommonRatio = backoffSeqCommonRatio;

    retryConfig.maxRetryAttempts = 0;
    testConfigStatusQueueRetryParamsBadArgument();
    retryConfig.maxRetryAttempts = maxRetryAttempts;

    retryConfig.maxBackoffTimeSec = 0;
    testConfigStatusQueueRetryParamsBadArgument();
    retryConfig.maxBackoffTimeSec = IAS_ZONE_STATUS_QUEUE_RETRY_ABS_MAX_BACKOFF_TIME_SEC + 1;
    testConfigStatusQueueRetryParamsBadArgument();
    retryConfig.maxBackoffTimeSec = maxBackoffTimeSec;
}

// Helper function to test status queue retry functionality with different configuration values.
uint32_t retryTestRunner(uint8_t firstBackoffTimeSec, uint8_t backoffSeqCommonRatio, uint32_t maxBackoffTimeSec,
                         bool unlimitedRetries, uint16_t maxTestRetryAttempts)
{
    uint32_t retryCount;
    uint32_t expectedBackoffTimeSec = 0;
    uint8_t message[3];
    uint8_t maxRetryAttempts;
    uint8_t maxBackoffTestCount = 3;

    // Unlimited retry is configured by setting the max retry attempts to 0xFFFF.
    if (unlimitedRetries)
    {
        maxRetryAttempts = 0xFF;
    }
    else
    {
        maxRetryAttempts = maxTestRetryAttempts - 1;
    }

    // Setup specific message parameters that are checked in
    // emberAfIasZoneClusterServerMessageSentCallback().
    // Note: these are independent of this actual test.
    message[ZCL_FRAME_CONTROL_IDX]               = ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT;
    message[IAS_ZONE_SERVER_PAYLOAD_COMMAND_IDX] = ZCL_ZONE_STATUS_CHANGE_NOTIFICATION_COMMAND_ID;

    IasZoneStatusQueueRetryConfig retryConfig = { firstBackoffTimeSec, backoffSeqCommonRatio, maxBackoffTimeSec, unlimitedRetries,
                                                  maxRetryAttempts };

    // Setting up retry parameters
    emberAfIasZoneServerConfigStatusQueueRetryParams(&retryConfig);

    // Calling message sent callback of the plugin with status parameter set to failure,
    // so that retry functionality is tested.
    expectedBackoffTimeSec = firstBackoffTimeSec;
    for (retryCount = 0; retryCount < maxTestRetryAttempts; retryCount++)
    {
        emberAfIasZoneClusterServerMessageSentCallback(0, 0, NULL, IAS_ZONE_SERVER_PAYLOAD_COMMAND_IDX, message,
                                                       EMBER_DELIVERY_FAILED);

        // No need to test max backoff if already reached and tested for a few times.
        // This makes the test run shorter and prints less garbage (ie. unnecessary dots)
        // on the screen.
        if (maxBackoffTestCount)
        {
            // Testing that the backoff time is increased according to current backoff configuration.
            expectComparisonDecimal(expectedBackoffTimeSec * MILLISECOND_TICKS_PER_SECOND,
                                    emberAfPluginIasZoneServerManageQueueEventControl.timeToExecute, "expected back-off time in ms",
                                    "observed back-off time in ms");
        }

        // Max retry attempts is reached, break here.
        if (emberAfPluginIasZoneServerManageQueueEventControl.status == EMBER_EVENT_ZERO_DELAY)
        {
            break;
        }

        // Increase expected backoff time until it reaches max backoff time.
        // Make sure that the current backoff time is still tested for a few
        // times after max backoff time is reached.
        if (expectedBackoffTimeSec * backoffSeqCommonRatio <= maxBackoffTimeSec)
        {
            expectedBackoffTimeSec = expectedBackoffTimeSec * backoffSeqCommonRatio;
        }
        else if (maxBackoffTestCount)
        {
            maxBackoffTestCount--;
        }
    }

    return retryCount;
}

// Verify that the status queue retry funcionality works as expected with
// different backoff parameters. Also test unlimited and limited retries.
static void retryTest(void)
{
    uint8_t firstBackoffTimeSec, backoffSeqCommonRatio;
    uint32_t retryCount, maxBackoffTimeSec, maxTestRetryAttempts;

    // "Loading" the status queue with some elements.
    messageQueue.entriesInQueue = RETRY_TEST_QUEUE_ENTRIES;
    messageQueue.startIdx       = 0;
    messageQueue.lastIdx        = RETRY_TEST_QUEUE_ENTRIES - 1;

    // --------------------------------------------------------------------
    // Test 1: test with minimal settings (small max retry attempts).
    // --------------------------------------------------------------------
    firstBackoffTimeSec   = 2;
    backoffSeqCommonRatio = 3;
    maxBackoffTimeSec     = 500;
    maxTestRetryAttempts  = 11;

    statusQueueParameterTest(firstBackoffTimeSec, backoffSeqCommonRatio, maxBackoffTimeSec, false, maxTestRetryAttempts);

    retryCount = retryTestRunner(firstBackoffTimeSec, backoffSeqCommonRatio, maxBackoffTimeSec, false, maxTestRetryAttempts);

    // Testing if the number of retry attempts is what is expected.
    expectComparisonDecimal(retryCount, maxTestRetryAttempts - 1, "number of retry attempts at end of test",
                            "expected max retry attempts");

    // Testing if queue element is removed after reaching max retry attempts.
    expectComparisonDecimal(messageQueue.entriesInQueue, RETRY_TEST_QUEUE_ENTRIES - 1, "current number of element in message queue",
                            "expected number of element in message queue");

    // -----------------------------------------------------------
    // Test 2: test with default (legacy) settings of this plugin.
    // -----------------------------------------------------------
    firstBackoffTimeSec   = 3;
    backoffSeqCommonRatio = 2;
    maxBackoffTimeSec     = 12;
    // 0xFF is unlimited attempts per the requirements. Making the retry attempts
    // in this test way higher than this limit so unlimited retires can be tested.
    maxTestRetryAttempts = 3 * 0xFF;

    statusQueueParameterTest(firstBackoffTimeSec, backoffSeqCommonRatio, maxBackoffTimeSec, true, maxTestRetryAttempts);

    retryCount = retryTestRunner(firstBackoffTimeSec, backoffSeqCommonRatio, maxBackoffTimeSec, true, maxTestRetryAttempts);

    // Testing if the number of retry attempts equals to what is expected.
    expectComparisonDecimal(retryCount, maxTestRetryAttempts, "number of retry attempts at end of test",
                            "expected retry attempts (ie. unlimited)");

    // Testing if queue element was not cleared (ie. same as after the previous test),
    // since this is an unlimited retry test.
    expectComparisonDecimal(messageQueue.entriesInQueue, RETRY_TEST_QUEUE_ENTRIES - 1, "current number of element in message queue",
                            "expected number of element in message queue");
}

int main(int argc, char * argv[])
{
    const TestCase tests[] = {
        { "attribute-response", enrollTest },
        { "leave-test", leaveTest },
        { "update-test", updateTest },
        { "retry-test", retryTest },
        { NULL },
    };

    // This test only runs correctly with network 0.
    // Ideally we should look into this.
    emberSetCurrentNetwork(0);

    return parseCommandLineAndExecuteTest(argc, argv, "IAS Zone Server", tests);
}
