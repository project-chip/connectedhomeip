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
 * @brief Test code for the Polling plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "callback.h"

#include "app/framework/plugin/end-device-support/end-device-support.h"

#include "app/framework/test/test-framework.h"
#include "app/framework/test/test-framework-security.h"

//-----------------------------------------------------------------------------

// Verify the secure vs. unsecure rejoin works correctly for both Smart Energy
// and HA key.

EmberEventControl emberAfPluginEndDeviceSupportMoveNetworkEventControls[EM_AF_MAX_SUPPORTED_NETWORKS_FOR_UNIT_TESTS];

static bool secureRejoinExpected = false;

const static EmberKeyData zigbeeAlliance09Key = ZIGBEE_PROFILE_INTEROPERABILITY_LINK_KEY;

const static EmberKeyData distributedKey = {
  0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
  0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF
};

const static EmberKeyData superSecretKey = {
  0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
  0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00
};

static EmberCurrentSecurityState testSecurityState;

//-----------------------------------------------------------------------------
// Stubs

EmberStatus emberFindAndRejoinNetworkWithReason(bool haveCurrentNetworkKey,
                                                uint32_t channelMask,
                                                EmberRejoinReason reason)
{
  expectMessage(secureRejoinExpected == haveCurrentNetworkKey,
                "secureRejoinExpected: %d == haveCurrentNetworkKey %d",
                secureRejoinExpected,
                haveCurrentNetworkKey);
  return EMBER_SUCCESS;
}

/** @brief Lost Parent Connectivity
 *
 * This function is called by the End Device Support plugin when the end device
 * cannot connect to its parent. This callback is called under the following
 * instances:
 * - When the network state of the device is EMBER_JOINED_NETWORK_NO_PARENT.
 * - If there are 3 or more data poll transaction failures for an end device.
 *
 * If this callback returns false, the End Device Support plugin code will
 * proceed with issuing a Trust Center rejoin. Otherwise, if this function
 * returns true, the End Device Support plugin will not issue a Trust Center
 * rejoin.
 */
bool emberAfPluginEndDeviceSupportLostParentConnectivityCallback(void)
{
  return false;
}

bool emberAfPluginEndDeviceSupportPreNetworkMoveCallback(void)
{
  return false;
}

bool emberZllOperationInProgress(void)
{
  return false;
}

EmberStatus emberGetCurrentSecurityState(EmberCurrentSecurityState *state)
{
  MEMCOPY(state, &testSecurityState, sizeof(EmberCurrentSecurityState));
  return EMBER_SUCCESS;
}

void setSecurityState(EmberCurrentSecurityState* state)
{
  MEMCOPY(&testSecurityState, state, sizeof(EmberCurrentSecurityState));
}

void setSecurityForTest(boolean centralizedNetwork)
{
  EmberCurrentSecurityState state;
  MEMSET(&state, 0, sizeof(state));
  state.bitmask = (EMBER_TRUST_CENTER_GLOBAL_LINK_KEY
                   | EMBER_HAVE_PRECONFIGURED_KEY
                   | EMBER_HAVE_NETWORK_KEY
                   | EMBER_NO_FRAME_COUNTER_RESET
                   | EMBER_REQUIRE_ENCRYPTED_KEY);

  if (!centralizedNetwork) {
    // Use distributed trust center mode.
    state.bitmask |= EMBER_DISTRIBUTED_TRUST_CENTER_MODE;
  }
  setSecurityState(&state);
}

//-----------------------------------------------------------------------------
// Tests

static void secureAndUnsecureRejoinTestCentralized(void)
{
  // Test with centralized network
  setSecurityForTest(true);

  // First Attempt should be Secure Rejoin regardless of key value.
  debug("Attempt 0 with well-known keys, should use Secure Rejoin.\n");
  MEMMOVE(emberKeyContents(&(testFrameworkTrustCenterLinkKey.key)),
          emberKeyContents(&(zigbeeAlliance09Key)),
          EMBER_ENCRYPTION_KEY_SIZE);
  secureRejoinExpected = true;
  emberAfPluginEndDeviceSupportMoveNetworkEventHandler();

  debug("Attempt 1 with well-known keys should be converted to Secure Rejoin.");
  secureRejoinExpected = true;
  emberAfPluginEndDeviceSupportMoveNetworkEventHandler();

  debug("Attempt 2 with NOT well-known key should use TC rejoin.\n");
  MEMMOVE(emberKeyContents(&(testFrameworkTrustCenterLinkKey.key)),
          emberKeyContents(&(superSecretKey)),
          EMBER_ENCRYPTION_KEY_SIZE);
  secureRejoinExpected = false;
  emberAfPluginEndDeviceSupportMoveNetworkEventHandler();
}

static void secureAndUnsecureRejoinTestDistributed(void)
{
  // Test with distributed network
  setSecurityForTest(false);

  debug("Attempt 0 with well-known keys, should use Secure Rejoin.\n");
  MEMMOVE(emberKeyContents(&(testFrameworkTrustCenterLinkKey.key)),
          emberKeyContents(&(distributedKey)),
          EMBER_ENCRYPTION_KEY_SIZE);
  secureRejoinExpected = true;
  emberAfPluginEndDeviceSupportMoveNetworkEventHandler();

  debug("Attempt 1 with well-known keys should be converted to Secure Rejoin.");
  secureRejoinExpected = true;
  emberAfPluginEndDeviceSupportMoveNetworkEventHandler();

  debug("Attempt 2 with NOT well-known key should also be converted to Secure Rejoin.\n");
  MEMMOVE(emberKeyContents(&(testFrameworkTrustCenterLinkKey.key)),
          emberKeyContents(&(superSecretKey)),
          EMBER_ENCRYPTION_KEY_SIZE);
  secureRejoinExpected = true;
  emberAfPluginEndDeviceSupportMoveNetworkEventHandler();
}

int main(int argc, char* argv[])
{
  const TestCase allTests[] = {
    { "secure-and-unsecure-rejoin-centralized", secureAndUnsecureRejoinTestCentralized },
    { "secure-and-unsecure-rejoin-distributed", secureAndUnsecureRejoinTestDistributed },
    { NULL },
  };

  return parseCommandLineAndExecuteTest(argc,
                                        argv,
                                        "end-device-move-test",
                                        allTests);
}
