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

#include <lib/dnssd/minimal_mdns/core/QName.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace mdns::Minimal;

/// Convenience method to have a  serialized QName:
///
/// static const uint8_t kData[] = "datahere\00";
///  AsSerializedQName(kData);
///
/// NOTE: this MUST be using the string "" format to add an extra NULL
/// terminator that this method discards.
template <size_t N>
static SerializedQNameIterator AsSerializedQName(const uint8_t (&v)[N])
{
    // NOTE: the -1 is because we format these items as STRINGS and that
    // appends an extra NULL terminator
    return SerializedQNameIterator(BytesRange(v, v + N - 1), v);
}

void IteratorTest(nlTestSuite * inSuite, void * inContext)
{
    {
        static const uint8_t kOneItem[] = "\04test\00";
        SerializedQNameIterator it      = AsSerializedQName(kOneItem);

        NL_TEST_ASSERT(inSuite, it.Next());
        NL_TEST_ASSERT(inSuite, strcmp(it.Value(), "test") == 0);
        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, it.IsValid());
    }

    {
        static const uint8_t kManyItems[] = "\04this\02is\01a\04test\00";
        SerializedQNameIterator it        = AsSerializedQName(kManyItems);

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
        SerializedQNameIterator it   = AsSerializedQName(kData);

        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, !it.IsValid());
    }

    {
        // Truncated before the end
        static const uint8_t kData[] = "\02";
        SerializedQNameIterator it   = AsSerializedQName(kData);

        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, !it.IsValid());
    }

    {
        // Truncated before the end
        static const uint8_t kData[] = "\xc0";
        SerializedQNameIterator it   = AsSerializedQName(kData);

        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, !it.IsValid());
    }
}

void InvalidReferencing(nlTestSuite * inSuite, void * inContext)
{
    {
        // Truncated before the end (but seemingly valid in case of error)
        // does NOT use AsSerializedQName (because out of range)
        static const uint8_t kData[] = "\00\xc0\x00";
        SerializedQNameIterator it(BytesRange(kData, kData + 2), kData + 1);

        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, !it.IsValid());
    }

    {
        // Infinite recursion
        static const uint8_t kData[] = "\04test\xc0\x00";
        SerializedQNameIterator it   = AsSerializedQName(kData);

        NL_TEST_ASSERT(inSuite, it.Next());
        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, !it.IsValid());
    }

    {
        // Infinite recursion by referencing own element (inside the stream)
        static const uint8_t kData[] = "\04test\xc0\x05";
        SerializedQNameIterator it   = AsSerializedQName(kData);

        NL_TEST_ASSERT(inSuite, it.Next());
        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, !it.IsValid());
    }

    {
        // Infinite recursion by referencing own element at the start
        static const uint8_t kData[] = "\xc0\x00";
        SerializedQNameIterator it   = AsSerializedQName(kData);

        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, !it.IsValid());
    }

    {
        // Reference that goes forwad instead of backward
        static const uint8_t kData[] = "\04test\xc0\x07";
        SerializedQNameIterator it   = AsSerializedQName(kData);

        NL_TEST_ASSERT(inSuite, it.Next());
        NL_TEST_ASSERT(inSuite, !it.Next());
        NL_TEST_ASSERT(inSuite, !it.IsValid());
    }
}

void Comparison(nlTestSuite * inSuite, void * inContext)
{
    static const uint8_t kManyItems[] = "\04this\02is\01a\04test\00";

    {
        const QNamePart kTestName[] = { "this" };
        NL_TEST_ASSERT(inSuite, AsSerializedQName(kManyItems) != FullQName(kTestName));
    }

    {
        const QNamePart kTestName[] = { "this", "is" };
        NL_TEST_ASSERT(inSuite, AsSerializedQName(kManyItems) != FullQName(kTestName));
    }

    {
        const QNamePart kTestName[] = { "is", "a", "test" };
        NL_TEST_ASSERT(inSuite, AsSerializedQName(kManyItems) != FullQName(kTestName));
    }

    {
        const QNamePart kTestName[] = { "this", "is", "a", "test" };
        NL_TEST_ASSERT(inSuite, AsSerializedQName(kManyItems) == FullQName(kTestName));
    }

    {
        const QNamePart kTestName[] = { "this", "is", "a", "test", "suffix" };
        NL_TEST_ASSERT(inSuite, AsSerializedQName(kManyItems) != FullQName(kTestName));
    }

    {
        const QNamePart kTestName[] = { "prefix", "this", "is", "a", "test" };
        NL_TEST_ASSERT(inSuite, AsSerializedQName(kManyItems) != FullQName(kTestName));
    }
}

void CaseInsensitiveSerializedCompare(nlTestSuite * inSuite, void * inContext)
{
    static const uint8_t kManyItems[] = "\04thIs\02iS\01a\04tEst\00";

    {
        const QNamePart kTestName[] = { "this", "is", "a", "test" };
        NL_TEST_ASSERT(inSuite,
                       SerializedQNameIterator(BytesRange(kManyItems, kManyItems + sizeof(kManyItems)), kManyItems) ==
                           FullQName(kTestName));
    }

    {
        const QNamePart kTestName[] = { "THIS", "IS", "A", "test" };
        NL_TEST_ASSERT(inSuite,
                       SerializedQNameIterator(BytesRange(kManyItems, kManyItems + sizeof(kManyItems)), kManyItems) ==
                           FullQName(kTestName));
    }

    {
        const QNamePart kTestName[] = { "THIS", "IS", "A", "TEST" };
        NL_TEST_ASSERT(inSuite,
                       SerializedQNameIterator(BytesRange(kManyItems, kManyItems + sizeof(kManyItems)), kManyItems) ==
                           FullQName(kTestName));
    }
}

void CaseInsensitiveFullQNameCompare(nlTestSuite * inSuite, void * inContext)
{
    {
        const QNamePart kName1[] = { "this", "is", "a", "test" };
        const QNamePart kName2[] = { "this", "IS", "a", "TEST" };
        NL_TEST_ASSERT(inSuite, FullQName(kName1) == FullQName(kName2));
    }

    {
        const QNamePart kName1[] = { "THIS", "IS", "a", "tesT" };
        const QNamePart kName2[] = { "this", "IS", "A", "TEst" };
        NL_TEST_ASSERT(inSuite, FullQName(kName1) == FullQName(kName2));
    }

    {
        const QNamePart kName1[] = { "THIS", "IS", "a", "test" };
        const QNamePart kName2[] = { "this", "IS", "A", "NEST" };
        NL_TEST_ASSERT(inSuite, FullQName(kName1) != FullQName(kName2));
    }

    {
        const QNamePart kName1[] = { "THIS", "IS", "a" };
        const QNamePart kName2[] = { "this", "IS", "A", "NEST" };
        NL_TEST_ASSERT(inSuite, FullQName(kName1) != FullQName(kName2));
    }

    {
        const QNamePart kName1[] = { "THIS", "IS", "a" };
        const QNamePart kName2[] = { "this", "IS" };
        NL_TEST_ASSERT(inSuite, FullQName(kName1) != FullQName(kName2));
    }

    {
        const QNamePart kName[] = { "this" };
        NL_TEST_ASSERT(inSuite, FullQName() != FullQName(kName));
        NL_TEST_ASSERT(inSuite, FullQName(kName) != FullQName());
    }
}

void SerializedCompare(nlTestSuite * inSuite, void * inContext)
{
    static const uint8_t kThisIsATest1[]    = "\04this\02is\01a\04test\00";
    static const uint8_t kThisIsATest2[]    = "\04ThIs\02is\01A\04tESt\00";
    static const uint8_t kThisIsDifferent[] = "\04this\02is\09different\00";
    static const uint8_t kThisIs[]          = "\04this\02is";

    NL_TEST_ASSERT(inSuite, AsSerializedQName(kThisIsATest1) == AsSerializedQName(kThisIsATest1));
    NL_TEST_ASSERT(inSuite, AsSerializedQName(kThisIsATest2) == AsSerializedQName(kThisIsATest2));
    NL_TEST_ASSERT(inSuite, AsSerializedQName(kThisIsATest1) == AsSerializedQName(kThisIsATest2));
    NL_TEST_ASSERT(inSuite, AsSerializedQName(kThisIsATest1) != AsSerializedQName(kThisIsDifferent));
    NL_TEST_ASSERT(inSuite, AsSerializedQName(kThisIsDifferent) != AsSerializedQName(kThisIsATest1));
    NL_TEST_ASSERT(inSuite, AsSerializedQName(kThisIsDifferent) != AsSerializedQName(kThisIs));
    NL_TEST_ASSERT(inSuite, AsSerializedQName(kThisIs) != AsSerializedQName(kThisIsDifferent));

    // These items have back references and are "this.is.a.test"
    static const uint8_t kPtrItems[] = "\03abc\02is\01a\04test\00\04this\xc0\04";
    SerializedQNameIterator thisIsATestPtr(BytesRange(kPtrItems, kPtrItems + sizeof(kPtrItems)), kPtrItems + 15);

    NL_TEST_ASSERT(inSuite, thisIsATestPtr == AsSerializedQName(kThisIsATest1));
    NL_TEST_ASSERT(inSuite, thisIsATestPtr == AsSerializedQName(kThisIsATest2));
    NL_TEST_ASSERT(inSuite, AsSerializedQName(kThisIsATest1) == thisIsATestPtr);
    NL_TEST_ASSERT(inSuite, AsSerializedQName(kThisIsATest2) == thisIsATestPtr);
    NL_TEST_ASSERT(inSuite, thisIsATestPtr != AsSerializedQName(kThisIs));
    NL_TEST_ASSERT(inSuite, AsSerializedQName(kThisIs) != thisIsATestPtr);
}

} // namespace

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("IteratorTest", IteratorTest),
    NL_TEST_DEF("ErrorTest", ErrorTest),
    NL_TEST_DEF("Comparison", Comparison),
    NL_TEST_DEF("CaseInsensitiveSerializedCompare", CaseInsensitiveSerializedCompare),
    NL_TEST_DEF("CaseInsensitiveFullQNameCompare", CaseInsensitiveFullQNameCompare),
    NL_TEST_DEF("SerializedCompare", SerializedCompare),
    NL_TEST_DEF("InvalidReferencing", InvalidReferencing),

    NL_TEST_SENTINEL()
};
// clang-format on

int TestQName()
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
