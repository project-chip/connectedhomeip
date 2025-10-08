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

/**
 *    @file
 *      This file implements a unit test suite for the CommodityTariffContainers helper classes.
 *
 */

#include "pw_unit_test/framework.h"
#include <app/clusters/commodity-tariff-server/CommodityTariffContainers.h>

#include <array>
#include <cstdint>
#include <mutex>
#include <thread>

#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::CommodityTariffContainers;

// =================================
//      Unit tests
// =================================

namespace chip {
namespace app {

class TestCommodityTariffContainers : public ::testing::Test
{
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}

protected:
    void SetUp() override
    {
        // Initialize CHIP stack if needed
        Platform::MemoryInit();
    }

    void TearDown() override { Platform::MemoryShutdown(); }
};

TEST_F(TestCommodityTariffContainers, CreateEmptyInitAndDestroy)
{
    {
        CTC_UnorderedSet<int> sample(10);
        EXPECT_EQ(sample.capacity(), 10u);
        EXPECT_EQ(sample.size(), 0u);
        EXPECT_TRUE(sample.empty());
    }
}

TEST_F(TestCommodityTariffContainers, CreateEmptyNotInitAndDestroy)
{
    {
        CTC_UnorderedSet<uint32_t> sample;
        EXPECT_EQ(sample.capacity(), 0u);
        EXPECT_EQ(sample.size(), 0u);
        EXPECT_TRUE(sample.empty());

        EXPECT_TRUE(sample.insert(0xAA550000));
        EXPECT_EQ(sample.size(), 1u);

        EXPECT_TRUE(sample.insert(0xAA550001));
        EXPECT_TRUE(sample.insert(0xAA550002));
        EXPECT_EQ(sample.size(), 3u); 
    }
}

TEST_F(TestCommodityTariffContainers, CreateAndPutItems)
{
    {
        CTC_UnorderedSet<uint32_t> sample(10);
        EXPECT_EQ(sample.capacity(), 10u);
        EXPECT_EQ(sample.size(), 0u);
        EXPECT_TRUE(sample.empty());

        EXPECT_TRUE(sample.insert(0xAA550000));
        EXPECT_TRUE(sample.insert(0x55AA0001));

        EXPECT_EQ(sample.size(), 2u);
        EXPECT_FALSE(sample.empty());

        for (size_t i = sample.size(); i <= sample.capacity() + 1; i++)
        {
            bool ret = sample.insert((uint32_t) ((i % 2 ? 0x55AA : 0xAA55) << 16) | (uint32_t) i);

            if (i < sample.capacity())
            {
                EXPECT_TRUE(ret);
            }
            else
            {
                EXPECT_FALSE(ret);
            }
        }

        for (size_t i = 0; i < sample.capacity(); i++)
        {
            uint32_t entry = sample[i];
            if (i % 2)
            {
                EXPECT_EQ((entry >> 16), 0x55AAu);
            }
            else
            {
                EXPECT_EQ((entry >> 16), 0xAA55u);
            }

            EXPECT_EQ((entry & 0x0000ffff), (uint32_t) i);
        }
    }
}

TEST_F(TestCommodityTariffContainers, UnorderedSet_BasicInsertAndContains)
{
    CTC_UnorderedSet<uint32_t> sample(12);

    for (size_t i = 0; i <= sample.capacity() - 3; i++)
    {
        bool ret = sample.insert((uint32_t) ((i % 2 ? 0x55AA : 0xAA55) << 16) | (uint32_t) i);

        if (i < sample.capacity())
        {
            EXPECT_TRUE(ret);
        }
        else
        {
            EXPECT_FALSE(ret);
        }
    }

    EXPECT_TRUE(sample.contains(0x55AA0001));
    EXPECT_TRUE(sample.contains(0xAA550002));
    EXPECT_TRUE(sample.contains(0x55AA0007));
    EXPECT_FALSE(sample.contains(0x55AA0008));

    EXPECT_FALSE(sample.insert(0xAA550000));
    EXPECT_FALSE(sample.insert(0x55AA0005));

    EXPECT_TRUE(sample.insert(0xAA550010));
    EXPECT_TRUE(sample.insert(0x55AA0011));
}

TEST_F(TestCommodityTariffContainers, UnorderedSet_DuplicatePrevention)
{
    CTC_UnorderedSet<int> set(10);

    EXPECT_TRUE(set.insert(100));
    EXPECT_TRUE(set.insert(200));
    EXPECT_TRUE(set.insert(300));

    EXPECT_FALSE(set.insert(100));
    EXPECT_FALSE(set.insert(200));
    EXPECT_FALSE(set.insert(300));
}

TEST_F(TestCommodityTariffContainers, UnorderedSet_RemoveEntry)
{
    CTC_UnorderedSet<int> set(10);

    EXPECT_TRUE(set.insert(100));
    EXPECT_TRUE(set.insert(200));
    EXPECT_TRUE(set.insert(300));

    EXPECT_EQ(set[0], 100);
    EXPECT_EQ(set[1], 200);
    EXPECT_EQ(set[2], 300);

    EXPECT_EQ(set.size(), 3u);

    set.remove(200);

    EXPECT_EQ(set.size(), 2u);

    EXPECT_EQ(set[0], 100);
    EXPECT_EQ(set[1], 300);

    EXPECT_TRUE(set.insert(200));

    EXPECT_EQ(set[2], 200);

    EXPECT_EQ(set.size(), 3u);
}

TEST_F(TestCommodityTariffContainers, UnorderedSet_Iteration)
{
    CTC_UnorderedSet<int> set(5);
    std::array<int, 3> expected = { 10, 20, 30 };

    for (int value : expected)
    {
        EXPECT_TRUE(set.insert(value));
    }

    EXPECT_EQ(set.size(), 3u);

    // Test range-based for loop
    std::vector<int> actual;
    for (const auto & item : set)
    {
        actual.push_back(item);
    }

    // Since it's unordered, we only check that all expected items are present
    for (int expected_value : expected)
    {
        EXPECT_NE(std::find(actual.begin(), actual.end(), expected_value), actual.end());
    }
}

TEST_F(TestCommodityTariffContainers, UnorderedSet_CapacityLimits)
{
    CTC_UnorderedSet<int> set(3);

    EXPECT_TRUE(set.insert(1));
    EXPECT_TRUE(set.insert(2));
    EXPECT_TRUE(set.insert(3));

    // Should fail to insert beyond capacity
    EXPECT_FALSE(set.insert(4));
    EXPECT_EQ(set.size(), 3u);
}

TEST_F(TestCommodityTariffContainers, UnorderedMap_BasicOperations)
{
    CTC_UnorderedMap<uint8_t, uint32_t> map(10);

    EXPECT_TRUE(map.insert(1, 0x11));
    EXPECT_TRUE(map.insert(2, 0x12));
    EXPECT_TRUE(map.insert(3, 0x13));

    EXPECT_EQ(map.size(), 3u);
    EXPECT_TRUE(map.contains(1));
    EXPECT_TRUE(map.contains(2));
    EXPECT_TRUE(map.contains(3));
    EXPECT_FALSE(map.contains(4));

    EXPECT_EQ(map[1], 0x11u);
    EXPECT_EQ(map[2], 0x12u);
    EXPECT_EQ(map[3], 0x13u);

    map.remove(2);

    EXPECT_FALSE(map.contains(2));
    EXPECT_EQ(map[1], 0x11u);
    EXPECT_EQ(map[3], 0x13u);
    EXPECT_EQ(map.size(), 2u);

    EXPECT_EQ(map[2], 0u);
    EXPECT_TRUE(map.contains(2));
    EXPECT_EQ(map.size(), 3u);

    map[4] = 0x14;
    EXPECT_EQ(map[4], 0x14u);

    EXPECT_EQ(map.size(), 4u);

    EXPECT_FALSE(map.insert(1, 100)); // Duplicate key
}

} // namespace app
} // namespace chip
