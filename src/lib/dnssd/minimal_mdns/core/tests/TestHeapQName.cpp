/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/dnssd/minimal_mdns/core/HeapQName.h>
#include <lib/dnssd/minimal_mdns/core/tests/QNameStrings.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace mdns::Minimal;

void Construction(nlTestSuite * inSuite, void * inContext)
{
    {

        const testing::TestQName<2> kShort({ "some", "test" });

        HeapQName heapQName(kShort.Serialized());

        NL_TEST_ASSERT(inSuite, heapQName.IsOk());
        NL_TEST_ASSERT(inSuite, heapQName.Content() == kShort.Full());
        NL_TEST_ASSERT(inSuite, kShort.Serialized() == heapQName.Content());
    }

    {

        const testing::TestQName<5> kLonger({ "these", "are", "more", "elements", "here" });

        HeapQName heapQName(kLonger.Serialized());

        NL_TEST_ASSERT(inSuite, heapQName.IsOk());
        NL_TEST_ASSERT(inSuite, heapQName.Content() == kLonger.Full());
        NL_TEST_ASSERT(inSuite, kLonger.Serialized() == heapQName.Content());
    }
}

void Copying(nlTestSuite * inSuite, void * inContext)
{
    const testing::TestQName<2> kShort({ "some", "test" });

    HeapQName name1(kShort.Serialized());
    HeapQName name2(name1);
    HeapQName name3;

    name3 = name2;

    NL_TEST_ASSERT(inSuite, name1.IsOk());
    NL_TEST_ASSERT(inSuite, name2.IsOk());
    NL_TEST_ASSERT(inSuite, name3.IsOk());
    NL_TEST_ASSERT(inSuite, name1.Content() == name2.Content());
    NL_TEST_ASSERT(inSuite, name1.Content() == name3.Content());
}

static const nlTest sTests[] = {               //
    NL_TEST_DEF("Construction", Construction), //
    NL_TEST_DEF("Copying", Copying),           //
    NL_TEST_SENTINEL()
};

int Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

int TestHeapQName(void)
{
    nlTestSuite theSuite = {
        "HeapQName",
        &sTests[0],
        &Setup,
        &Teardown,
    };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestHeapQName)
