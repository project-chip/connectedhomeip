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
 * @brief Test code for the Trust Center Network Key Update Broadcast plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "callback.h"
#include "trust-center-nwk-key-update-broadcast.h"

#include "app/framework/test/test-framework.h"
#include "app/framework/test/test-framework-security.h"

//------------------------------------------------------------------------------
// Globals

static EmberStatus expectedKeyUpdateCompleteResult = EMBER_ERR_FATAL;
static bool broadcastFails = false;

//------------------------------------------------------------------------------
// Stubs

//------------------------------------------------------------------------------
// Functions

void emberAfNetworkKeyUpdateCompleteCallback(EmberStatus status)
{
  expect(expectedKeyUpdateCompleteResult == status);
  note(".");
}

EmberStatus emberBroadcastNextNetworkKey(EmberKeyData* newKey)
{
  // We expect that the Application passes a key of zeroes, indicating
  // to the stack that it should generate a random key.  Since there is
  // no stack code being used in this test, the key is all zeroes.
  expectMessage((0 == MEMCOMPARE(newKey,
                                 &testFrameworkZeroKey,
                                 EMBER_ENCRYPTION_KEY_SIZE)),
                "Error: New key does not match expected key of 0's.\n");
  return (broadcastFails
          ? EMBER_NO_BUFFERS
          : EMBER_SUCCESS);
}

static void basicTest(void)
{
  testFrameworkNetworkState = EMBER_JOINED_NETWORK;

  // Trying to switch the network key when we are not the TC should fail.
  testFrameworkNodeId = 0xABCD;
  expect(EMBER_INVALID_CALL == emberAfTrustCenterStartNetworkKeyUpdate());

  testFrameworkNodeId = EMBER_TRUST_CENTER_NODE_ID;
  expect(EMBER_SUCCESS == emberAfTrustCenterStartNetworkKeyUpdate());

  // A second call will fail.
  expect(EMBER_INVALID_CALL == emberAfTrustCenterStartNetworkKeyUpdate());

  // Simulate the broadcast expiring and the switch being sent out.
  expectedKeyUpdateCompleteResult = EMBER_SUCCESS;
  emberKeySwitchExpected = true;
  emberAfPluginTrustCenterNwkKeyUpdateBroadcastMyEventHandler();
  assert(emberKeySwitchReceived);
  note(".");
}

static void failureCasesTest(void)
{
  testFrameworkNetworkState = EMBER_JOINED_NETWORK;

  testFrameworkNodeId = EMBER_TRUST_CENTER_NODE_ID;
  broadcastFails = true;
  expect(EMBER_NO_BUFFERS == emberAfTrustCenterStartNetworkKeyUpdate());
  note(".");
}

int main(int argc, char* argv[])
{
  const TestCase allTests[] = {
    { "basic-test", basicTest },
    { "failure-cases", failureCasesTest },
    { NULL },
  };
  return parseCommandLineAndExecuteTest(argc,
                                        argv,
                                        argv[0],
                                        allTests);
}
