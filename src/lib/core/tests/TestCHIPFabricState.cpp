/*
 *
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *    @file
 *     This file implements a unit test suite for <tt>chip::ChipFabricState</tt>.
 *
 */

#include <stdint.h>
#include <string.h>

#include <nlunit-test.h>

#include <core/CHIPCore.h>

#include "ToolCommon.h"

static const uint64_t kTestNodeId = 0x18B43000002DCF71ULL;
static const uint64_t kTestFabricId = 0xFEEDBEEFULL;
static const uint16_t kDefaultSubnet = 0x01;
static ChipFabricState sFabricState;

/**
 * Test generating a ULA using the default subnet.
 */
static void CheckSelectNodeAddress(nlTestSuite *inSuite, void *inContext)
{
    // Bank of psuedorandom test node IDs
    const uint64_t testIDs[] =
    {
        0x3e383920daadee75,
        0xaf17346309a3dc4b,
        0x000000000000ae14,
        0x3085051285957d2e,
        0xee8bffe99277378a,
        kAnyNodeId
    };

    const size_t numTestIds = sizeof(testIDs) / sizeof(testIDs[0]);

    for (unsigned int i = 0; i < numTestIds; i++)
    {
        IPAddress receivedULA = sFabricState.SelectNodeAddress(testIDs[i]);

        IPAddress calculatedULA;
        if (testIDs[i] == kAnyNodeId)
        {
            calculatedULA = IPAddress::MakeIPv6WellKnownMulticast(kIPv6MulticastScope_Link, kIPV6MulticastGroup_AllNodes);
        }
        else
        {
            calculatedULA = IPAddress::MakeULA(kTestFabricId, kDefaultSubnet, ChipNodeIdToIPv6InterfaceId(testIDs[i]));
        }
        NL_TEST_ASSERT(inSuite, receivedULA == calculatedULA);
    }
}

/**
 * Test generating a ULA using arbitrary subnets.
 */
static void CheckSelectNodeAddressWithSubnet(nlTestSuite *inSuite, void *inContext)
{
    // Bank of psuedorandom test node IDs
    const uint64_t testIDs[] =
    {
        0x3e383920daadee75,
        0xaf17346309a3dc4b,
        0x000000000000ae14,
        0x3085051285957d2e,
        0xee8bffe99277378a
    };

    const size_t numTestIds = sizeof(testIDs) / sizeof(testIDs[0]);
    const int subnetTestUpperLimit = 5;

    for (uint16_t j = 0; j < subnetTestUpperLimit; j++)
    {
        for (unsigned int i = 0; i < numTestIds; i++)
        {
            IPAddress receivedULA = sFabricState.SelectNodeAddress(testIDs[i], j);
            IPAddress calculatedULA;
            calculatedULA = IPAddress::MakeULA(kTestFabricId, j, ChipNodeIdToIPv6InterfaceId(testIDs[i]));
            NL_TEST_ASSERT(inSuite, receivedULA == calculatedULA);
        }
    }
}

/**
 *  Set up the test suite.
 */
static int TestSetup(void *inContext)
{
    (void)sFabricState.Init();
    sFabricState.LocalNodeId = kTestNodeId;
    sFabricState.FabricId = kTestFabricId;
    sFabricState.DefaultSubnet = kDefaultSubnet;

    return (SUCCESS);
}

/**
 *  Tear down the test suite.
 */
static int TestTeardown(void *inContext)
{
    return (SUCCESS);
}

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] =
{
    // TODO: ChipFabricState has much more functionality than tested here. A
    // more thorough collection of tests should be written.
    NL_TEST_DEF("ChipFabricState::SelectNodeAddress", CheckSelectNodeAddress),
    NL_TEST_DEF("ChipFabricState::SelectNodeAddress", CheckSelectNodeAddressWithSubnet),
    NL_TEST_SENTINEL()
};

int main(void)
{
    nlTestSuite theSuite =
    {
        "CHIP-fabric-state",
        &sTests[0],
        TestSetup,
        TestTeardown

    };

    // Generate machine-readable, comma-separated value (CSV) output.
    nl_test_set_output_style(OUTPUT_CSV);

    // Run test suit againt one context.
    nlTestRunner(&theSuite, NULL);

    return nlTestRunnerStats(&theSuite);
}
