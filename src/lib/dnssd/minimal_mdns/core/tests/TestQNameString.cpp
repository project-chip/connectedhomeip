/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <lib/dnssd/minimal_mdns/core/HeapQName.h>
#include <lib/dnssd/minimal_mdns/core/QNameString.h>
#include <lib/dnssd/minimal_mdns/core/tests/QNameStrings.h>

namespace {

using namespace chip;
using namespace mdns::Minimal;

class TestQNameString : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestQNameString, Construction)
{
    {
        const testing::TestQName<2> kShort({ "some", "test" });
        QNameString heapQName(kShort.Serialized());
        EXPECT_STREQ(heapQName.c_str(), "some.test");
        EXPECT_TRUE(heapQName.EndsWith("test"));
        EXPECT_TRUE(heapQName.EndsWith(".test"));
        EXPECT_FALSE(heapQName.EndsWith("some"));

        mdns::Minimal::SerializedQNameIterator SInvalid;
        QNameString heapQNameI(SInvalid);
        EXPECT_STREQ(heapQNameI.c_str(), "(!INVALID!)");
    }
}

TEST_F(TestQNameString, EndsWith)
{
    const testing::TestQName<3> kLong({ "abc", "test", "abc" });
    QNameString qName(kLong.Serialized());
    EXPECT_STREQ(qName.c_str(), "abc.test.abc");

    // Verify that EndsWith matches only the suffix, not the first occurrence of "abc"
    EXPECT_TRUE(qName.EndsWith("abc"));
    EXPECT_TRUE(qName.EndsWith(".abc"));
    EXPECT_TRUE(qName.EndsWith("test.abc"));
    EXPECT_FALSE(qName.EndsWith("test"));
    EXPECT_FALSE(qName.EndsWith("abc.test"));
}

TEST_F(TestQNameString, LongQName)
{
    const testing::TestQName<10> kLong({ "label1234567890", "label1234567890", "label1234567890", "label1234567890",
                                         "label1234567890", "label1234567890", "label1234567890", "label1234567890",
                                         "label1234567890", "label1234567890" });
    QNameString qName(kLong.Serialized());

    // QNameString buffer is 128 bytes. 10 * 15 + 9 = 159 bytes.
    EXPECT_FALSE(qName.Fit());
    // EndsWith should return false if Fit() is false, even if the suffix matches what's in the buffer
    EXPECT_FALSE(qName.EndsWith("label1234567890"));
}
} // namespace
