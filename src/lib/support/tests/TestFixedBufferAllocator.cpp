/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/support/FixedBufferAllocator.h>
#include <lib/support/UnitTestRegistration.h>

#include <cstring>
#include <nlunit-test.h>

using namespace chip;

namespace {

void TestClone(nlTestSuite * inSuite, void * inContext)
{
    uint8_t buffer[128];
    FixedBufferAllocator alloc(buffer);

    const char * kTestString     = "Test string";
    const char * allocatedString = alloc.Clone(kTestString);

    NL_TEST_ASSERT(inSuite, allocatedString != nullptr);
    NL_TEST_ASSERT(inSuite, allocatedString != kTestString);

    // NOLINTNEXTLINE(clang-analyzer-unix.cstring.NullArg): null check for allocated string already done
    NL_TEST_ASSERT(inSuite, strcmp(allocatedString, kTestString) == 0);

    const uint8_t kTestData[]     = { 0xDE, 0xAD, 0xBE, 0xEF };
    const uint8_t * allocatedData = alloc.Clone(kTestData, sizeof(kTestData));

    NL_TEST_ASSERT(inSuite, allocatedData != nullptr);
    NL_TEST_ASSERT(inSuite, allocatedData != kTestData);

    // NOLINTNEXTLINE(clang-analyzer-unix.cstring.NullArg): null check for allocated data already done
    NL_TEST_ASSERT(inSuite, memcmp(allocatedData, kTestData, sizeof(kTestData)) == 0);
}

void TestOutOfMemory(nlTestSuite * inSuite, void * inContext)
{
    uint8_t buffer[16];
    FixedBufferAllocator alloc(buffer);

    const char * kTestData = "0123456789abcdef";

    // Allocating 16 bytes still works...
    NL_TEST_ASSERT(inSuite, alloc.Clone(kTestData, 16) != nullptr);
    NL_TEST_ASSERT(inSuite, !alloc.AnyAllocFailed());

    // ...but cannot allocate even one more byte...
    NL_TEST_ASSERT(inSuite, alloc.Clone(kTestData, 1) == nullptr);
    NL_TEST_ASSERT(inSuite, alloc.AnyAllocFailed());
}

const nlTest sTests[] = { NL_TEST_DEF("Test successful clone", TestClone), NL_TEST_DEF("Test out of memory", TestOutOfMemory),
                          NL_TEST_SENTINEL() };

} // namespace

int TestFixedBufferAllocator()
{
    nlTestSuite theSuite = { "CHIP FixedBufferAllocator tests", &sTests[0], nullptr, nullptr };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestFixedBufferAllocator)
