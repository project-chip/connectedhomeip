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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/minimal_mdns/core/HeapQName.h>

namespace {

using namespace mdns::Minimal;

TEST(TestHeapQName, Construction)
{
    {

        const testing::TestQName<2> kShort({ "some", "test" });

        HeapQName heapQName(kShort.Serialized());

        EXPECT_TRUE(heapQName.IsOk());
        EXPECT_EQ(heapQName.Content(), kShort.Full());
        EXPECT_EQ(kShort.Serialized(), heapQName.Content());
    }

    {

        const testing::TestQName<5> kLonger({ "these", "are", "more", "elements", "here" });

        HeapQName heapQName(kLonger.Serialized());

        EXPECT_TRUE(heapQName.IsOk());
        EXPECT_EQ(heapQName.Content(), kLonger.Full());
        EXPECT_EQ(kLonger.Serialized(), heapQName.Content());
    }
}

TEST(TestHeapQName, Copying)
{
    const testing::TestQName<2> kShort({ "some", "test" });

    HeapQName name1(kShort.Serialized());
    HeapQName name2(name1);
    HeapQName name3;

    name3 = name2;

    EXPECT_TRUE(name1.IsOk());
    EXPECT_TRUE(name2.IsOk());
    EXPECT_TRUE(name3.IsOk());
    EXPECT_EQ(name1.Content(), name2.Content());
    EXPECT_EQ(name1.Content(), name3.Content());
}

} // namespace
