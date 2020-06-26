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
 * @brief Test code for the Trust Center Network Key Update Unicast plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "callback.h"
#include "trust-center-nwk-key-update-unicast.h"

#include "app/framework/test/test-framework.h"
#include "app/framework/test/test-framework-security.h"

#include "app/framework/util/af-main.h"

//------------------------------------------------------------------------------
// Globals

#define ROUTE_DISCOVERY_MIN_QS (10 * 4)

#define UNDEFINED_STATUS EMBER_ERR_FATAL
static EmberStatus expectedKeyUpdateCompleteResult = UNDEFINED_STATUS;
static bool expectDiscovery = false;
static bool discoveryUnderway = false;
static bool noDiscoveryCallback = false;
static bool mtorrExpected = false;
static bool mtorrQueued = false;
static bool firstKeyHasBeenSent = false;

static bool keyUpdateExpected = false;
static bool keyUpdateFailure = false;
static uint8_t currentKeyTableIndex = 0;

//------------------------------------------------------------------------------
// Stubs

//------------------------------------------------------------------------------
// Functions

EmberStatus emberAfFindNodeId(EmberEUI64 longAddress,
                              EmberAfServiceDiscoveryCallback *callback)
{
  expectMessage(expectDiscovery,
                "emberAfFindNodeId() called but no discovery expected.\n");
  expectMessage(0 == MEMCOMPARE(longAddress,
                                testFrameworkKeyTable[currentKeyTableIndex].partnerEUI64,
                                EUI64_SIZE),
                "EUI64 mismatch for discovery.\n");
  if (discoveryUnderway) {
    discoveryUnderway = false;
    return EMBER_INVALID_CALL;
  }

  if (noDiscoveryCallback) {
    noDiscoveryCallback = false;
    return EMBER_SUCCESS;
  }

  EmberAfServiceDiscoveryResult result = {
    EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
    NETWORK_ADDRESS_REQUEST,
    testFrameworkKeyTableNodeId[currentKeyTableIndex],
    (void*)&(testFrameworkKeyTableNodeId[currentKeyTableIndex])
  };

  zdoDiscoveryCallback(&result);

  expectDiscovery = false;
  return EMBER_SUCCESS;
}

void emberAfNetworkKeyUpdateCompleteCallback(EmberStatus status)
{
  expectMessage(expectedKeyUpdateCompleteResult == status,
                "Expected Key update result of 0x%X != actual of 0x%X\n",
                expectedKeyUpdateCompleteResult,
                status);
  expectedKeyUpdateCompleteResult = UNDEFINED_STATUS;
  note(".");
}

EmberStatus emSendAlternateNetworkKeyToAddress(EmberNodeId targetShort,
                                               EmberEUI64  targetLong,
                                               EmberKeyData* newKey)
{
  expectMessage(keyUpdateExpected,
                "No key update message expected for 0x%02X.\n",
                targetShort);

  // First time we send a key, the key should be zeroes indicating that the stack
  // should randomly generate one.  The second time the stack should send
  // a real value, the one generated previously.
  expectMessage((0 == MEMCOMPARE(newKey,
                                 (firstKeyHasBeenSent
                                  ? emberKeyContents(&(testFrameworkNextNwkKey.key))
                                  : emberKeyContents(&testFrameworkZeroKey)),
                                 sizeof(EmberKeyData))),
                "Error: Next NWK key does not match correct value.\n");
  firstKeyHasBeenSent = true;

  // When passing a key of zero, the stack automatically generates a new key
  // for the next network key and updates the sequence number.
  // Here is where we simulate that.
  MEMMOVE(testFrameworkNextNwkKey.key.contents,
          testFrameworkRandomlyGeneratedKey.contents,
          sizeof(EmberKeyData));
  testFrameworkNextNwkKey.sequenceNumber = testFrameworkCurrentNwkKey.sequenceNumber + 1;

  note(".");
  if (keyUpdateFailure) {
    keyUpdateFailure = false;
    return EMBER_NO_BUFFERS;
  }

  keyUpdateExpected = false;
  return EMBER_SUCCESS;
}

uint32_t emberAfPluginConcentratorQueueDiscovery(void)
{
  expectMessage(mtorrExpected,
                "Unexpected MTORR\n");
  mtorrQueued = true;
  return ROUTE_DISCOVERY_MIN_QS;
}

// The plugin is expected to traverse the key table and for each qualifying node
// it should discover the node ID and then send a TransportKey command.
// The test appliaction defines a number of nodes, some of which do not qualify
// because they are either sleepy or not authorized. Assuming we have at least
// two that do, we simulate three sorts of error conditions on the first one:
// 1. The node ID discovery fails to start;
// 2. The discovery starts but fails (discovery callback is not called);
// 3. The discovery succeeds but sending TransportKey fails.
// We test that the plugin recovers from all three errors.
// All other nodes will slip through without a hitch.
static void basicTest(void)
{
  testFrameworkNetworkState = EMBER_JOINED_NETWORK;

  // Trying to update the network key when we are not the TC should fail.
  testFrameworkNodeId = 0xABCD;
  expect(EMBER_INVALID_CALL == emberAfTrustCenterStartNetworkKeyUpdate());

  testFrameworkNodeId = EMBER_TRUST_CENTER_NODE_ID;
  mtorrExpected = true;
  expect(EMBER_SUCCESS == emberAfTrustCenterStartNetworkKeyUpdate());
  expect(mtorrQueued);
  mtorrQueued = false;
  mtorrExpected = false;

  // A second call will fail.
  expect(EMBER_INVALID_CALL == emberAfTrustCenterStartNetworkKeyUpdate());

  uint8_t i;

  for (currentKeyTableIndex = i = 0;
       currentKeyTableIndex < emberAfGetKeyTableSize();
       currentKeyTableIndex++) {
    if (((testFrameworkKeyTable[currentKeyTableIndex].bitmask
          & EMBER_KEY_IS_AUTHORIZED) != EMBER_KEY_IS_AUTHORIZED)
        || (testFrameworkKeyTable[currentKeyTableIndex].bitmask
            & EMBER_KEY_PARTNER_IS_SLEEPY)) {
      debug("Test Framework not expecting key update for index %d", currentKeyTableIndex);
      continue;
    }

    expectDiscovery = true;
    keyUpdateExpected = true;

    // See the function head comment on the three modes of failure
    if (i == 0) {
      discoveryUnderway = true;
      noDiscoveryCallback = true;
      keyUpdateFailure = true;
    }

    // This will trigger the node ID discovery
    emberAfPluginTrustCenterNwkKeyUpdateUnicastMyEventHandler();
    expect(!discoveryUnderway);         // this should have been cleared...

    if (i == 0) {
      expect(noDiscoveryCallback);      // ...but not this yet...
      expect(keyUpdateFailure);         // ...or this

      // Unicast code will retry after a delay.
      emberAfPluginTrustCenterNwkKeyUpdateUnicastMyEventHandler();
      expect(!noDiscoveryCallback);     // this should have been cleared now
      expect(keyUpdateFailure);         // ...but not this
    }

    // We expect a "discovery complete" callback only when the discovery fails,
    // in which case it should trigger a recovery. Any superfluous "discovery
    // complete" callback should be accepted but ignored.
    EmberAfServiceDiscoveryResult result = {
      EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE,
      NETWORK_ADDRESS_REQUEST,
      testFrameworkKeyTableNodeId[currentKeyTableIndex],
      (void*)&(testFrameworkKeyTableNodeId[currentKeyTableIndex])
    };

    zdoDiscoveryCallback(&result);
    expect(!keyUpdateFailure);          // this should have been cleared now

    if (i == 0) {
      // Trigger the recovery scheduled by the "discovery complete" callback
      emberAfPluginTrustCenterNwkKeyUpdateUnicastMyEventHandler();
    }

    expect(!keyUpdateExpected);
    note(".");
    i++;
  }

  // Execute one last time so that TC can realize there are no
  // more devices to send to.
  emberAfPluginTrustCenterNwkKeyUpdateUnicastMyEventHandler();

  expectedKeyUpdateCompleteResult = EMBER_SUCCESS;
  emberKeySwitchExpected = true;
  emberAfPluginTrustCenterNwkKeyUpdateUnicastMyEventHandler();

  assert(emberKeySwitchReceived);
  emberKeySwitchReceived = false;
  assert(expectedKeyUpdateCompleteResult == UNDEFINED_STATUS);
}

int main(int argc, char* argv[])
{
  const TestCase allTests[] = {
    { "basic-test", basicTest },
    { NULL },
  };
  return parseCommandLineAndExecuteTest(argc,
                                        argv,
                                        argv[0],
                                        allTests);
}
