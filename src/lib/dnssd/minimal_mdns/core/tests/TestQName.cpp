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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/minimal_mdns/core/QName.h>

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

TEST(TestQName, IteratorTest)
{
    {
        static const uint8_t kOneItem[] = "\04test\00";
        SerializedQNameIterator it      = AsSerializedQName(kOneItem);

        EXPECT_TRUE(it.Next());
        EXPECT_STREQ(it.Value(), "test");
        EXPECT_FALSE(it.Next());
        EXPECT_TRUE(it.IsValid());
    }

    {
        static const uint8_t kManyItems[] = "\04this\02is\01a\04test\00";
        SerializedQNameIterator it        = AsSerializedQName(kManyItems);

        EXPECT_TRUE(it.Next());
        EXPECT_STREQ(it.Value(), "this");

        EXPECT_TRUE(it.Next());
        EXPECT_STREQ(it.Value(), "is");

        EXPECT_TRUE(it.Next());
        EXPECT_STREQ(it.Value(), "a");

        EXPECT_TRUE(it.Next());
        EXPECT_STREQ(it.Value(), "test");

        EXPECT_FALSE(it.Next());
        EXPECT_TRUE(it.IsValid());
    }
    {
        static const uint8_t kPtrItems[] = "abc\02is\01a\04test\00\04this\xc0\03";
        SerializedQNameIterator it(BytesRange(kPtrItems, kPtrItems + sizeof(kPtrItems)), kPtrItems + 14);

        EXPECT_TRUE(it.Next());
        EXPECT_STREQ(it.Value(), "this");

        EXPECT_TRUE(it.Next());
        EXPECT_STREQ(it.Value(), "is");

        EXPECT_TRUE(it.Next());
        EXPECT_STREQ(it.Value(), "a");

        EXPECT_TRUE(it.Next());
        EXPECT_STREQ(it.Value(), "test");

        EXPECT_FALSE(it.Next());
        EXPECT_TRUE(it.IsValid());
    }
}

TEST(TestQName, ErrorTest)
{
    {
        // Truncated before the end
        static const uint8_t kData[] = "\04test";
        SerializedQNameIterator it   = AsSerializedQName(kData);

        EXPECT_FALSE(it.Next());
        EXPECT_FALSE(it.IsValid());
    }

    {
        // Truncated before the end
        static const uint8_t kData[] = "\02";
        SerializedQNameIterator it   = AsSerializedQName(kData);

        EXPECT_FALSE(it.Next());
        EXPECT_FALSE(it.IsValid());
    }

    {
        // Truncated before the end
        static const uint8_t kData[] = "\xc0";
        SerializedQNameIterator it   = AsSerializedQName(kData);

        EXPECT_FALSE(it.Next());
        EXPECT_FALSE(it.IsValid());
    }
}

TEST(TestQName, InvalidReferencing)
{
    {
        // Truncated before the end (but seemingly valid in case of error)
        // does NOT use AsSerializedQName (because out of range)
        static const uint8_t kData[] = "\00\xc0\x00";
        SerializedQNameIterator it(BytesRange(kData, kData + 2), kData + 1);

        EXPECT_FALSE(it.Next());
        EXPECT_FALSE(it.IsValid());
    }

    {
        // Infinite recursion
        static const uint8_t kData[] = "\04test\xc0\x00";
        SerializedQNameIterator it   = AsSerializedQName(kData);

        EXPECT_TRUE(it.Next());
        EXPECT_FALSE(it.Next());
        EXPECT_FALSE(it.IsValid());
    }

    {
        // Infinite recursion by referencing own element (inside the stream)
        static const uint8_t kData[] = "\04test\xc0\x05";
        SerializedQNameIterator it   = AsSerializedQName(kData);

        EXPECT_TRUE(it.Next());
        EXPECT_FALSE(it.Next());
        EXPECT_FALSE(it.IsValid());
    }

    {
        // Infinite recursion by referencing own element at the start
        static const uint8_t kData[] = "\xc0\x00";
        SerializedQNameIterator it   = AsSerializedQName(kData);

        EXPECT_FALSE(it.Next());
        EXPECT_FALSE(it.IsValid());
    }

    {
        // Reference that goes forwad instead of backward
        static const uint8_t kData[] = "\04test\xc0\x07";
        SerializedQNameIterator it   = AsSerializedQName(kData);

        EXPECT_TRUE(it.Next());
        EXPECT_FALSE(it.Next());
        EXPECT_FALSE(it.IsValid());
    }
}

TEST(TestQName, Comparison)
{
    static const uint8_t kManyItems[] = "\04this\02is\01a\04test\00";

    {
        const QNamePart kTestName[] = { "this" };
        EXPECT_NE(AsSerializedQName(kManyItems), FullQName(kTestName));
    }

    {
        const QNamePart kTestName[] = { "this", "is" };
        EXPECT_NE(AsSerializedQName(kManyItems), FullQName(kTestName));
    }

    {
        const QNamePart kTestName[] = { "is", "a", "test" };
        EXPECT_NE(AsSerializedQName(kManyItems), FullQName(kTestName));
    }

    {
        const QNamePart kTestName[] = { "this", "is", "a", "test" };
        EXPECT_EQ(AsSerializedQName(kManyItems), FullQName(kTestName));
    }

    {
        const QNamePart kTestName[] = { "this", "is", "a", "test", "suffix" };
        EXPECT_NE(AsSerializedQName(kManyItems), FullQName(kTestName));
    }

    {
        const QNamePart kTestName[] = { "prefix", "this", "is", "a", "test" };
        EXPECT_NE(AsSerializedQName(kManyItems), FullQName(kTestName));
    }
}

TEST(TestQName, CaseInsensitiveSerializedCompare)
{
    static const uint8_t kManyItems[] = "\04thIs\02iS\01a\04tEst\00";

    {
        const QNamePart kTestName[] = { "this", "is", "a", "test" };
        EXPECT_EQ(SerializedQNameIterator(BytesRange(kManyItems, kManyItems + sizeof(kManyItems)), kManyItems),
                  FullQName(kTestName));
    }

    {
        const QNamePart kTestName[] = { "THIS", "IS", "A", "test" };
        EXPECT_EQ(SerializedQNameIterator(BytesRange(kManyItems, kManyItems + sizeof(kManyItems)), kManyItems),
                  FullQName(kTestName));
    }

    {
        const QNamePart kTestName[] = { "THIS", "IS", "A", "TEST" };
        EXPECT_EQ(SerializedQNameIterator(BytesRange(kManyItems, kManyItems + sizeof(kManyItems)), kManyItems),
                  FullQName(kTestName));
    }
}

TEST(TestQName, CaseInsensitiveFullQNameCompare)
{
    {
        const QNamePart kName1[] = { "this", "is", "a", "test" };
        const QNamePart kName2[] = { "this", "IS", "a", "TEST" };
        EXPECT_EQ(FullQName(kName1), FullQName(kName2));
    }

    {
        const QNamePart kName1[] = { "THIS", "IS", "a", "tesT" };
        const QNamePart kName2[] = { "this", "IS", "A", "TEst" };
        EXPECT_EQ(FullQName(kName1), FullQName(kName2));
    }

    {
        const QNamePart kName1[] = { "THIS", "IS", "a", "test" };
        const QNamePart kName2[] = { "this", "IS", "A", "NEST" };
        EXPECT_NE(FullQName(kName1), FullQName(kName2));
    }

    {
        const QNamePart kName1[] = { "THIS", "IS", "a" };
        const QNamePart kName2[] = { "this", "IS", "A", "NEST" };
        EXPECT_NE(FullQName(kName1), FullQName(kName2));
    }

    {
        const QNamePart kName1[] = { "THIS", "IS", "a" };
        const QNamePart kName2[] = { "this", "IS" };
        EXPECT_NE(FullQName(kName1), FullQName(kName2));
    }

    {
        const QNamePart kName[] = { "this" };
        EXPECT_NE(FullQName(), FullQName(kName));
        EXPECT_NE(FullQName(kName), FullQName());
    }
}

TEST(TestQName, SerializedCompare)
{
    static const uint8_t kThisIsATest1[]    = "\04this\02is\01a\04test\00";
    static const uint8_t kThisIsATest2[]    = "\04ThIs\02is\01A\04tESt\00";
    static const uint8_t kThisIsDifferent[] = "\04this\02is\09different\00";
    static const uint8_t kThisIs[]          = "\04this\02is";

    EXPECT_EQ(AsSerializedQName(kThisIsATest1), AsSerializedQName(kThisIsATest1));
    EXPECT_EQ(AsSerializedQName(kThisIsATest2), AsSerializedQName(kThisIsATest2));
    EXPECT_EQ(AsSerializedQName(kThisIsATest1), AsSerializedQName(kThisIsATest2));
    EXPECT_NE(AsSerializedQName(kThisIsATest1), AsSerializedQName(kThisIsDifferent));
    EXPECT_NE(AsSerializedQName(kThisIsDifferent), AsSerializedQName(kThisIsATest1));
    EXPECT_NE(AsSerializedQName(kThisIsDifferent), AsSerializedQName(kThisIs));
    EXPECT_NE(AsSerializedQName(kThisIs), AsSerializedQName(kThisIsDifferent));

    // These items have back references and are "this.is.a.test"
    static const uint8_t kPtrItems[] = "\03abc\02is\01a\04test\00\04this\xc0\04";
    SerializedQNameIterator thisIsATestPtr(BytesRange(kPtrItems, kPtrItems + sizeof(kPtrItems)), kPtrItems + 15);

    EXPECT_EQ(thisIsATestPtr, AsSerializedQName(kThisIsATest1));
    EXPECT_EQ(thisIsATestPtr, AsSerializedQName(kThisIsATest2));
    EXPECT_EQ(AsSerializedQName(kThisIsATest1), thisIsATestPtr);
    EXPECT_EQ(AsSerializedQName(kThisIsATest2), thisIsATestPtr);
    EXPECT_NE(thisIsATestPtr, AsSerializedQName(kThisIs));
    EXPECT_NE(AsSerializedQName(kThisIs), thisIsATestPtr);
}

} // namespace
