/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <lib/support/CHIPMem.h>
#include <lib/support/ThreadOperationalDataset.h>

// #include <lib/support/UnitTestContext.h>
// #include <lib/support/UnitTestRegistration.h>

#include <gtest/gtest.h>
// #include <nlunit-test.h>

namespace {

using namespace chip;

class TestThreadOperationalDataset : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    static Thread::OperationalDataset dataset;
};

Thread::OperationalDataset TestThreadOperationalDataset::dataset;

TEST_F(TestThreadOperationalDataset, TestInit)
{

    uint8_t longerThanOperationalDatasetSize[255]{};
    EXPECT_EQ(dataset.Init(ByteSpan(longerThanOperationalDatasetSize)), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(dataset.Init(ByteSpan()), CHIP_NO_ERROR);

    {
        uint8_t data[] = { 0x01, 0x02, 0x03 };

        EXPECT_EQ(dataset.Init(ByteSpan(data)), CHIP_ERROR_INVALID_ARGUMENT);
    }

    {
        uint8_t data[] = { 0x01 };

        EXPECT_EQ(dataset.Init(ByteSpan(data)), CHIP_ERROR_INVALID_ARGUMENT);
    }
}

TEST_F(TestThreadOperationalDataset, TestActiveTimestamp)
{
    static constexpr uint64_t kActiveTimestampValue = 1;

    uint64_t activeTimestamp = 0;

    EXPECT_EQ(dataset.SetActiveTimestamp(kActiveTimestampValue), CHIP_NO_ERROR);
    EXPECT_EQ(dataset.GetActiveTimestamp(activeTimestamp), CHIP_NO_ERROR);
    EXPECT_EQ(activeTimestamp, kActiveTimestampValue);
}

TEST_F(TestThreadOperationalDataset, TestChannel)
{
    static constexpr uint16_t kChannelValue = 15;

    uint16_t channel = 0;

    EXPECT_EQ(dataset.SetChannel(kChannelValue), CHIP_NO_ERROR);
    EXPECT_EQ(dataset.GetChannel(channel), CHIP_NO_ERROR);
    EXPECT_EQ(channel, kChannelValue);
}

TEST_F(TestThreadOperationalDataset, TestExtendedPanId)
{
    static constexpr uint8_t kExtendedPanId[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };

    uint8_t extendedPanId[Thread::kSizeExtendedPanId] = { 0 };

    EXPECT_EQ(dataset.SetExtendedPanId(kExtendedPanId), CHIP_NO_ERROR);
    EXPECT_EQ(dataset.GetExtendedPanId(extendedPanId), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(extendedPanId, kExtendedPanId, sizeof(kExtendedPanId)), 0);

    ByteSpan span;
    EXPECT_EQ(dataset.GetExtendedPanIdAsByteSpan(span), CHIP_NO_ERROR);
    EXPECT_EQ(span.size(), sizeof(kExtendedPanId));
    EXPECT_EQ(memcmp(extendedPanId, span.data(), sizeof(kExtendedPanId)), 0);
}

TEST_F(TestThreadOperationalDataset, TestMasterKey)
{
    static constexpr uint8_t kMasterKey[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                              0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

    uint8_t masterKey[Thread::kSizeMasterKey] = { 0 };

    EXPECT_EQ(dataset.SetMasterKey(kMasterKey), CHIP_NO_ERROR);
    EXPECT_EQ(dataset.GetMasterKey(masterKey), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(masterKey, kMasterKey, sizeof(kMasterKey)), 0);
}

TEST_F(TestThreadOperationalDataset, TestMeshLocalPrefix)
{
    static constexpr uint8_t kMeshLocalPrefix[] = { 0xfd, 0xde, 0xad, 0x00, 0xbe, 0xef, 0x00, 0x00 };

    uint8_t meshLocalPrefix[Thread::kSizeMeshLocalPrefix] = { 0 };

    EXPECT_EQ(dataset.SetMeshLocalPrefix(kMeshLocalPrefix), CHIP_NO_ERROR);
    EXPECT_EQ(dataset.GetMeshLocalPrefix(meshLocalPrefix), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(meshLocalPrefix, kMeshLocalPrefix, sizeof(kMeshLocalPrefix)), 0);
}

TEST_F(TestThreadOperationalDataset, TestNetworkName)
{
    static constexpr char kNetworkName[] = "ThreadNetwork";

    char networkName[Thread::kSizeNetworkName + 1] = { 0 };

    EXPECT_EQ(dataset.SetNetworkName(kNetworkName), CHIP_NO_ERROR);
    EXPECT_EQ(dataset.GetNetworkName(networkName), CHIP_NO_ERROR);
    EXPECT_STREQ(networkName, kNetworkName);

    EXPECT_EQ(dataset.SetNetworkName("0123456789abcdef"), CHIP_NO_ERROR);
    EXPECT_EQ(dataset.SetNetworkName("0123456789abcdefg"), CHIP_ERROR_INVALID_STRING_LENGTH);
    EXPECT_EQ(dataset.SetNetworkName(""), CHIP_ERROR_INVALID_STRING_LENGTH);
}

TEST_F(TestThreadOperationalDataset, TestPanId)
{
    static constexpr uint16_t kPanIdValue = 0x1234;

    uint16_t panid = 0;

    EXPECT_EQ(dataset.SetPanId(kPanIdValue), CHIP_NO_ERROR);
    EXPECT_EQ(dataset.GetPanId(panid), CHIP_NO_ERROR);
    EXPECT_EQ(panid, kPanIdValue);
}

TEST_F(TestThreadOperationalDataset, TestPSKc)
{
    static constexpr uint8_t kPSKc[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                         0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

    uint8_t pskc[Thread::kSizePSKc] = { 0 };

    EXPECT_EQ(dataset.SetPSKc(kPSKc), CHIP_NO_ERROR);
    EXPECT_EQ(dataset.GetPSKc(pskc), CHIP_NO_ERROR);
    EXPECT_FALSE(memcmp(pskc, kPSKc, sizeof(kPSKc)));
}

TEST_F(TestThreadOperationalDataset, TestUnsetMasterKey)
{

    uint8_t masterKey[Thread::kSizeMasterKey] = { 0 };

    EXPECT_EQ(dataset.GetMasterKey(masterKey), CHIP_NO_ERROR);
    dataset.UnsetMasterKey();
    EXPECT_EQ(dataset.GetMasterKey(masterKey), CHIP_ERROR_TLV_TAG_NOT_FOUND);
    EXPECT_EQ(dataset.SetMasterKey(masterKey), CHIP_NO_ERROR);
}

TEST_F(TestThreadOperationalDataset, TestUnsetPSKc)
{

    uint8_t pskc[Thread::kSizePSKc] = { 0 };

    EXPECT_EQ(dataset.GetPSKc(pskc), CHIP_NO_ERROR);
    dataset.UnsetPSKc();
    EXPECT_EQ(dataset.GetPSKc(pskc), CHIP_ERROR_TLV_TAG_NOT_FOUND);
    EXPECT_EQ(dataset.SetPSKc(pskc), CHIP_NO_ERROR);
}

TEST_F(TestThreadOperationalDataset, TestClear)
{

    {
        uint64_t activeTimestamp;
        EXPECT_EQ(dataset.GetActiveTimestamp(activeTimestamp), CHIP_NO_ERROR);
    }

    {
        uint16_t channel;
        EXPECT_EQ(dataset.GetChannel(channel), CHIP_NO_ERROR);
    }

    {
        uint8_t extendedPanId[Thread::kSizeExtendedPanId] = { 0 };
        EXPECT_EQ(dataset.GetExtendedPanId(extendedPanId), CHIP_NO_ERROR);
    }

    {
        uint8_t masterKey[Thread::kSizeMasterKey] = { 0 };
        EXPECT_EQ(dataset.GetMasterKey(masterKey), CHIP_NO_ERROR);
    }

    {
        uint8_t meshLocalPrefix[Thread::kSizeMeshLocalPrefix] = { 0 };
        EXPECT_EQ(dataset.GetMeshLocalPrefix(meshLocalPrefix), CHIP_NO_ERROR);
    }

    {
        char networkName[Thread::kSizeNetworkName + 1] = { 0 };
        EXPECT_EQ(dataset.GetNetworkName(networkName), CHIP_NO_ERROR);
    }

    {
        uint16_t panid;
        EXPECT_EQ(dataset.GetPanId(panid), CHIP_NO_ERROR);
    }

    {
        uint8_t pskc[Thread::kSizePSKc] = { 0 };
        EXPECT_EQ(dataset.GetPSKc(pskc), CHIP_NO_ERROR);
    }

    dataset.Clear();

    {
        uint64_t activeTimestamp;
        EXPECT_EQ(dataset.GetActiveTimestamp(activeTimestamp), CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }

    {
        uint16_t channel;
        EXPECT_EQ(dataset.GetChannel(channel), CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }

    {
        uint8_t extendedPanId[Thread::kSizeExtendedPanId] = { 0 };
        EXPECT_EQ(dataset.GetExtendedPanId(extendedPanId), CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }

    {
        uint8_t masterKey[Thread::kSizeMasterKey] = { 0 };
        EXPECT_EQ(dataset.GetMasterKey(masterKey), CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }

    {
        uint8_t meshLocalPrefix[Thread::kSizeMeshLocalPrefix] = { 0 };
        EXPECT_EQ(dataset.GetMeshLocalPrefix(meshLocalPrefix), CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }

    {
        char networkName[Thread::kSizeNetworkName + 1] = { 0 };
        EXPECT_EQ(dataset.GetNetworkName(networkName), CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }

    {
        uint16_t panid;
        EXPECT_EQ(dataset.GetPanId(panid), CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }

    {
        uint8_t pskc[Thread::kSizePSKc] = { 0 };
        EXPECT_EQ(dataset.GetPSKc(pskc), CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }
}

} // namespace
