/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <mdns/minimal/QName.h>
#include <support/TestUtils.h>

#include <nlunit-test.h>

namespace {

using namespace mdns::Minimal;

void IteratorTest(nlTestSuite * inSuite, void * inContext)
{
    {
        static const uint8_t kOneItem[] = "\04test\00";
        SerializedQNameIterator it(BytesRange(kOneItem, kOneItem + sizeof(kOneItem)), kOneItem);

        NL_TEST_ASSERT(inSuite, it.Next());
        NL_TEST_ASSERT(inSuite, strcmp(it.Value(), "test") == 0);
        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, it.IsValid());
    }

    {
        static const uint8_t kManyItems[] = "\04this\02is\01a\04test\00";
        SerializedQNameIterator it(BytesRange(kManyItems, kManyItems + sizeof(kManyItems)), kManyItems);

        NL_TEST_ASSERT(inSuite, it.Next());
        NL_TEST_ASSERT(inSuite, strcmp(it.Value(), "this") == 0);

        NL_TEST_ASSERT(inSuite, it.Next());
        NL_TEST_ASSERT(inSuite, strcmp(it.Value(), "is") == 0);

        NL_TEST_ASSERT(inSuite, it.Next());
        NL_TEST_ASSERT(inSuite, strcmp(it.Value(), "a") == 0);

        NL_TEST_ASSERT(inSuite, it.Next());
        NL_TEST_ASSERT(inSuite, strcmp(it.Value(), "test") == 0);

        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, it.IsValid());
    }
    {
        static const uint8_t kPtrItems[] = "abc\02is\01a\04test\00\04this\xc0\03";
        SerializedQNameIterator it(BytesRange(kPtrItems, kPtrItems + sizeof(kPtrItems)), kPtrItems + 14);

        NL_TEST_ASSERT(inSuite, it.Next());
        NL_TEST_ASSERT(inSuite, strcmp(it.Value(), "this") == 0);

        NL_TEST_ASSERT(inSuite, it.Next());
        NL_TEST_ASSERT(inSuite, strcmp(it.Value(), "is") == 0);

        NL_TEST_ASSERT(inSuite, it.Next());
        NL_TEST_ASSERT(inSuite, strcmp(it.Value(), "a") == 0);

        NL_TEST_ASSERT(inSuite, it.Next());
        NL_TEST_ASSERT(inSuite, strcmp(it.Value(), "test") == 0);

        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, it.IsValid());
    }
}

void ErrorTest(nlTestSuite * inSuite, void * inContext)
{
    {
        // Truncated before the end
        static const uint8_t kData[] = "\04test";
        SerializedQNameIterator it(BytesRange(kData, kData + 5), kData);

        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, !it.IsValid());
    }

    {
        // Truncated before the end
        static const uint8_t kData[] = "\02";
        SerializedQNameIterator it(BytesRange(kData, kData + 1), kData);

        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, !it.IsValid());
    }

    {
        // Truncated before the end
        static const uint8_t kData[] = "\xc0";
        SerializedQNameIterator it(BytesRange(kData, kData + 1), kData);

        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, !it.IsValid());
    }

    {
        // Truncated before the end (but seemingly valid in case of error)
        static const uint8_t kData[] = "\00\xc0\x00";
        SerializedQNameIterator it(BytesRange(kData, kData + 2), kData + 1);

        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, !it.IsValid());
    }
    {
        // Infinite recursion
        static const uint8_t kData[] = "\03test\xc0\x00";
        SerializedQNameIterator it(BytesRange(kData, kData + 7), kData);

        NL_TEST_ASSERT(inSuite, it.Next());
        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, !it.IsValid());
    }
}

} // namespace

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("IteratorTest", IteratorTest),
    NL_TEST_DEF("ErrorTest", ErrorTest),

    NL_TEST_SENTINEL()
};
// clang-format on

int TestQName(void)
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "QName",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestQName)
