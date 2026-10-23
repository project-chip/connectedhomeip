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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/ThreadOperationalDataset.h>

namespace {

using namespace chip;

// Don't warn/error on self assignments in these tests, since
// we're explicitly testing that self-assignments behave correctly.
#ifdef __clang__
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif

class TestThreadOperationalDataset : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    Thread::OperationalDataset dataset;
};

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

    dataset.UnsetMasterKey();
    EXPECT_EQ(dataset.GetMasterKey(masterKey), CHIP_ERROR_TLV_TAG_NOT_FOUND);
    EXPECT_EQ(dataset.SetMasterKey(masterKey), CHIP_NO_ERROR);
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
    EXPECT_EQ(dataset.SetNetworkName(nullptr), CHIP_ERROR_INVALID_ARGUMENT);
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

    dataset.UnsetPSKc();
    EXPECT_EQ(dataset.GetPSKc(pskc), CHIP_ERROR_TLV_TAG_NOT_FOUND);
    EXPECT_EQ(dataset.SetPSKc(pskc), CHIP_NO_ERROR);
}

TEST_F(TestThreadOperationalDataset, TestClear)
{
    {
        EXPECT_EQ(dataset.SetActiveTimestamp(123), CHIP_NO_ERROR);
        EXPECT_EQ(dataset.SetChannel(5), CHIP_NO_ERROR);
        uint8_t extendedPanId[Thread::kSizeExtendedPanId] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        EXPECT_EQ(dataset.SetExtendedPanId(extendedPanId), CHIP_NO_ERROR);
        uint8_t masterKey[Thread::kSizeMasterKey] = { 0 };
        EXPECT_EQ(dataset.SetMasterKey(masterKey), CHIP_NO_ERROR);
        uint8_t meshLocalPrefix[Thread::kSizeMeshLocalPrefix] = { 0 };
        EXPECT_EQ(dataset.SetMeshLocalPrefix(meshLocalPrefix), CHIP_NO_ERROR);
        EXPECT_EQ(dataset.SetNetworkName("w00tw00t"), CHIP_NO_ERROR);
        EXPECT_EQ(dataset.SetPanId(0x4242), CHIP_NO_ERROR);
        uint8_t pskc[Thread::kSizePSKc] = { 0 };
        EXPECT_EQ(dataset.SetPSKc(pskc), CHIP_NO_ERROR);
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

static constexpr uint8_t kValidExampleDataset[] = {
    0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // Active Timestamp 1
    0x00, 0x03, 0x00, 0x00, 0x0f,                               // Channel 15
    0x35, 0x04, 0x07, 0xff, 0xf8, 0x00,                         // Channel Mask 0x07fff800
    0x02, 0x08, 0x39, 0x75, 0x8e, 0xc8, 0x14, 0x4b, 0x07, 0xfb, // Ext PAN ID 39758ec8144b07fb
    0x07, 0x08, 0xfd, 0xf1, 0xf1, 0xad, 0xd0, 0x79, 0x7d, 0xc0, // Mesh Local Prefix fdf1:f1ad:d079:7dc0::/64
    0x05, 0x10, 0xf3, 0x66, 0xce, 0xc7, 0xa4, 0x46, 0xba, 0xb9, 0x78, 0xd9, 0x0d, 0x27, 0xab, 0xe3, 0x8f, 0x23, // Network Key
    0x03, 0x0f, 0x4f, 0x70, 0x65, 0x6e, 0x54, 0x68, 0x72, 0x65, 0x61, 0x64, 0x2d, 0x35, 0x39, 0x33, 0x38,       // "OpenThread-5938"
    0x01, 0x02, 0x59, 0x38,                                                                                     // PAN ID : 0x5938
    0x04, 0x10, 0x3c, 0xa6, 0x7c, 0x96, 0x9e, 0xfb, 0x0d, 0x0c, 0x74, 0xa4, 0xd8, 0xee, 0x92, 0x3b, 0x57, 0x6c, // PKSc
    0x0c, 0x04, 0x02, 0xa0, 0xf7, 0xf8,                                                                         // Security Policy
};

template <typename T>
static void TestExampleDatasetTemplate()
{
    T dataset;
    EXPECT_EQ(dataset.Init(ByteSpan(kValidExampleDataset)), CHIP_NO_ERROR);

    uint64_t activeTimestamp;
    EXPECT_EQ(dataset.GetActiveTimestamp(activeTimestamp), CHIP_NO_ERROR);
    EXPECT_EQ(activeTimestamp, 1u);

    uint16_t channel;
    EXPECT_EQ(dataset.GetChannel(channel), CHIP_NO_ERROR);
    EXPECT_EQ(channel, 15u);

    uint8_t extPanId[Thread::kSizeExtendedPanId];
    EXPECT_EQ(dataset.GetExtendedPanId(extPanId), CHIP_NO_ERROR);
    const uint8_t expectedExtPanId[] = { 0x39, 0x75, 0x8e, 0xc8, 0x14, 0x4b, 0x07, 0xfb };
    EXPECT_TRUE(ByteSpan(extPanId).data_equal(ByteSpan(expectedExtPanId)));

    uint8_t meshLocalPrefix[Thread::kSizeMeshLocalPrefix];
    EXPECT_EQ(dataset.GetMeshLocalPrefix(meshLocalPrefix), CHIP_NO_ERROR);
    const uint8_t expectedMeshLocalPrefix[] = { 0xfd, 0xf1, 0xf1, 0xad, 0xd0, 0x79, 0x7d, 0xc0 };
    EXPECT_TRUE(ByteSpan(meshLocalPrefix).data_equal(ByteSpan(expectedMeshLocalPrefix)));

    uint8_t masterKey[Thread::kSizeMasterKey];
    EXPECT_EQ(dataset.GetMasterKey(masterKey), CHIP_NO_ERROR);
    const uint8_t expectedMasterKey[] = { 0xf3, 0x66, 0xce, 0xc7, 0xa4, 0x46, 0xba, 0xb9,
                                          0x78, 0xd9, 0x0d, 0x27, 0xab, 0xe3, 0x8f, 0x23 };
    EXPECT_TRUE(ByteSpan(masterKey).data_equal(ByteSpan(expectedMasterKey)));

    char networkName[Thread::kSizeNetworkName + 1];
    EXPECT_EQ(dataset.GetNetworkName(networkName), CHIP_NO_ERROR);
    EXPECT_STREQ(networkName, "OpenThread-5938");

    uint16_t panId;
    EXPECT_EQ(dataset.GetPanId(panId), CHIP_NO_ERROR);
    EXPECT_EQ(panId, 0x5938u);

    uint8_t pksc[Thread::kSizePSKc];
    EXPECT_EQ(dataset.GetPSKc(pksc), CHIP_NO_ERROR);
    const uint8_t expectedPksc[] = {
        0x3c, 0xa6, 0x7c, 0x96, 0x9e, 0xfb, 0x0d, 0x0c, 0x74, 0xa4, 0xd8, 0xee, 0x92, 0x3b, 0x57, 0x6c
    };
    EXPECT_TRUE(ByteSpan(pksc).data_equal(ByteSpan(expectedPksc)));

    ByteSpan channelMask;
    EXPECT_EQ(dataset.GetChannelMask(channelMask), CHIP_NO_ERROR);
    const uint8_t expectedChannelMask[] = { 0x07, 0xff, 0xf8, 0x00 };
    EXPECT_TRUE(channelMask.data_equal(ByteSpan(expectedChannelMask)));

    uint32_t securityPolicy;
    EXPECT_EQ(dataset.GetSecurityPolicy(securityPolicy), CHIP_NO_ERROR);
    EXPECT_EQ(securityPolicy, 0x02a0f7f8u);
}

TEST_F(TestThreadOperationalDataset, TestExampleDataset)
{
    TestExampleDatasetTemplate<Thread::OperationalDataset>();
}

TEST_F(TestThreadOperationalDataset, TestExampleDatasetView)
{
    TestExampleDatasetTemplate<Thread::OperationalDatasetView>();
}

template <typename T>
static void TestInvalidExampleDatasetTemplate()
{
    T dataset;
    const uint8_t invalid[] = {
        0x0e, 0x01, 0x01,                                                                                     // Active Timestamp
        0x00, 0x01, 0x0f,                                                                                     // Channel
        0x35, 0x00,                                                                                           // Channel Mask
        0x02, 0x09, 0x39, 0x75, 0x8e, 0xc8, 0x14, 0x4b, 0x07, 0xfb, 0xff,                                     // Ext PAN ID
        0x07, 0x01, 0x01,                                                                                     // Mesh Local Prefix
        0x05, 0x0f, 0xf3, 0x66, 0xce, 0xc7, 0xa4, 0x46, 0xba, 0xb9, 0x78, 0xd9, 0x0d, 0x27, 0xab, 0xe3, 0x8f, // Network Key
        0x03, 0x11, 0x4f, 0x70, 0x65, 0x6e, 0x54, 0x68, 0x72, 0x65, 0x61, 0x64, 0x2d, 0x35, 0x39, 0x33, 0x38, 0xff, 0xff, // Name
        0x01, 0x01, 0xff,                                                                                                 // PAN ID
        0x04, 0x01, 0x3c,                                                                                                 // PKSc
        0x0c, 0x01, 0x01, // Security Policy
    };

    // The overall TLV structure is valid, but all TLVs have invalid sizes
    EXPECT_EQ(dataset.Init(ByteSpan(invalid)), CHIP_NO_ERROR);

    uint64_t activeTimestamp;
    EXPECT_EQ(dataset.GetActiveTimestamp(activeTimestamp), CHIP_ERROR_INVALID_TLV_ELEMENT);

    uint16_t channel;
    EXPECT_EQ(dataset.GetChannel(channel), CHIP_ERROR_INVALID_TLV_ELEMENT);

    ByteSpan extPanId;
    EXPECT_EQ(dataset.GetExtendedPanIdAsByteSpan(extPanId), CHIP_ERROR_INVALID_TLV_ELEMENT);

    uint8_t meshLocalPrefix[Thread::kSizeMeshLocalPrefix];
    EXPECT_EQ(dataset.GetMeshLocalPrefix(meshLocalPrefix), CHIP_ERROR_INVALID_TLV_ELEMENT);

    uint8_t masterKey[Thread::kSizeMasterKey];
    EXPECT_EQ(dataset.GetMasterKey(masterKey), CHIP_ERROR_INVALID_TLV_ELEMENT);

    char networkName[Thread::kSizeNetworkName + 1];
    EXPECT_EQ(dataset.GetNetworkName(networkName), CHIP_ERROR_INVALID_TLV_ELEMENT);

    uint16_t panId;
    EXPECT_EQ(dataset.GetPanId(panId), CHIP_ERROR_INVALID_TLV_ELEMENT);

    uint8_t pksc[Thread::kSizePSKc];
    EXPECT_EQ(dataset.GetPSKc(pksc), CHIP_ERROR_INVALID_TLV_ELEMENT);

    ByteSpan channelMask;
    EXPECT_EQ(dataset.GetChannelMask(channelMask), CHIP_ERROR_INVALID_TLV_ELEMENT);

    uint32_t securityPolicy;
    EXPECT_EQ(dataset.GetSecurityPolicy(securityPolicy), CHIP_ERROR_INVALID_TLV_ELEMENT);
}

TEST_F(TestThreadOperationalDataset, TestInvalidExampleDataset)
{
    TestInvalidExampleDatasetTemplate<Thread::OperationalDataset>();
}

TEST_F(TestThreadOperationalDataset, TestInvalidExampleDatasetView)
{
    TestInvalidExampleDatasetTemplate<Thread::OperationalDatasetView>();
}

TEST_F(TestThreadOperationalDataset, TestMaxDatasetSize)
{
    uint8_t data[Thread::kSizeOperationalDataset - 2] = { 0 };        // minus 2 for Type and Length
    EXPECT_EQ(dataset.SetChannelMask(ByteSpan(data)), CHIP_NO_ERROR); // channel mask takes an arbitrary size value
    EXPECT_EQ(dataset.AsByteSpan().size(), Thread::kSizeOperationalDataset);
    EXPECT_EQ(dataset.SetPanId(0x1234), CHIP_ERROR_NO_MEMORY);

    EXPECT_EQ(dataset.SetChannelMask(ByteSpan(data, sizeof(data) - 3)), CHIP_NO_ERROR);
    EXPECT_EQ(dataset.SetPanId(0x1234), CHIP_ERROR_NO_MEMORY); // still too small by one byte

    EXPECT_EQ(dataset.SetChannelMask(ByteSpan(data, sizeof(data) - 4)), CHIP_NO_ERROR);
    EXPECT_EQ(dataset.SetPanId(0x1234), CHIP_NO_ERROR);
    EXPECT_EQ(dataset.AsByteSpan().size(), Thread::kSizeOperationalDataset);
}

TEST_F(TestThreadOperationalDataset, TestGrowAndShrinkValue)
{
    uint16_t panId;
    char networkName[Thread::kSizeNetworkName + 1];
    EXPECT_EQ(dataset.AsByteSpan().size(), 0u);
    EXPECT_EQ(dataset.SetNetworkName("A"), CHIP_NO_ERROR); // 2 + 1 bytes
    EXPECT_EQ(dataset.AsByteSpan().size(), 3u);
    EXPECT_EQ(dataset.SetPanId(0x4321), CHIP_NO_ERROR); // 2 + 2 bytes
    EXPECT_EQ(dataset.AsByteSpan().size(), 7u);
    EXPECT_EQ(dataset.SetNetworkName("BB"), CHIP_NO_ERROR); // 1 byte longer
    EXPECT_EQ(dataset.AsByteSpan().size(), 8u);
    EXPECT_EQ(dataset.GetPanId(panId), CHIP_NO_ERROR);
    EXPECT_EQ(panId, 0x4321u);
    EXPECT_EQ(dataset.GetNetworkName(networkName), CHIP_NO_ERROR);
    EXPECT_STREQ(networkName, "BB");
    EXPECT_EQ(dataset.SetNetworkName("C"), CHIP_NO_ERROR); // 1 byte shorter
    EXPECT_EQ(dataset.AsByteSpan().size(), 7u);
    EXPECT_EQ(dataset.GetPanId(panId), CHIP_NO_ERROR);
    EXPECT_EQ(panId, 0x4321u);
    EXPECT_EQ(dataset.GetNetworkName(networkName), CHIP_NO_ERROR);
    EXPECT_STREQ(networkName, "C");
}

TEST_F(TestThreadOperationalDataset, TestInitWithViewInitAndModify)
{
    uint64_t activeTimestamp;
    uint8_t example[] = { 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }; // Active Timestamp 1

    EXPECT_EQ(dataset.Thread::OperationalDatasetView::Init(ByteSpan(example)), CHIP_NO_ERROR); // super class Init
    EXPECT_EQ(dataset.GetActiveTimestamp(activeTimestamp), CHIP_NO_ERROR);
    EXPECT_EQ(activeTimestamp, 1u);
    EXPECT_EQ(dataset.SetActiveTimestamp(42), CHIP_NO_ERROR);
    EXPECT_EQ(dataset.GetActiveTimestamp(activeTimestamp), CHIP_NO_ERROR);
    EXPECT_EQ(activeTimestamp, 42u);

    // Ensure the original data was not modified
    Thread::OperationalDatasetView view;
    EXPECT_EQ(view.Init(ByteSpan(example)), CHIP_NO_ERROR);
    EXPECT_EQ(view.GetActiveTimestamp(activeTimestamp), CHIP_NO_ERROR);
    EXPECT_EQ(activeTimestamp, 1u);
}

TEST_F(TestThreadOperationalDataset, TestCopyAndAssignView)
{
    ByteSpan data(kValidExampleDataset);
    Thread::OperationalDatasetView view1;
    EXPECT_EQ(view1.Init(data), CHIP_NO_ERROR);
    EXPECT_EQ(view1.AsByteSpan().data(), data.data());
    EXPECT_EQ(view1.AsByteSpan().size(), data.size());

    view1 = view1; // self-assignment
    EXPECT_EQ(view1.AsByteSpan().data(), data.data());
    EXPECT_EQ(view1.AsByteSpan().size(), data.size());

    Thread::OperationalDatasetView view2(view1);
    EXPECT_EQ(view2.AsByteSpan().data(), data.data());
    EXPECT_EQ(view2.AsByteSpan().size(), data.size());

    Thread::OperationalDatasetView view3;
    EXPECT_TRUE(view3.IsEmpty());
    view3 = view1;
    EXPECT_EQ(view3.AsByteSpan().data(), data.data());
    EXPECT_EQ(view3.AsByteSpan().size(), data.size());
}

TEST_F(TestThreadOperationalDataset, TestCopyAndAssignDataset)
{
    ByteSpan data(kValidExampleDataset);
    EXPECT_EQ(dataset.Init(data), CHIP_NO_ERROR);
    const uint8_t * buffer = dataset.AsByteSpan().data();
    EXPECT_NE(buffer, data.data());
    EXPECT_TRUE(dataset.AsByteSpan().data_equal(data));

    dataset = dataset; // self-assignment
    EXPECT_EQ(dataset.AsByteSpan().data(), buffer);
    EXPECT_TRUE(dataset.AsByteSpan().data_equal(data));

    Thread::OperationalDataset dataset2(dataset);
    EXPECT_NE(dataset2.AsByteSpan().data(), dataset.AsByteSpan().data());
    EXPECT_NE(dataset2.AsByteSpan().data(), data.data());
    EXPECT_TRUE(dataset2.AsByteSpan().data_equal(data));

    Thread::OperationalDataset dataset3;
    EXPECT_TRUE(dataset3.IsEmpty());
    dataset3 = static_cast<const Thread::OperationalDatasetView &>(dataset);
    EXPECT_NE(dataset3.AsByteSpan().data(), dataset.AsByteSpan().data());
    EXPECT_NE(dataset3.AsByteSpan().data(), data.data());
    EXPECT_TRUE(dataset3.AsByteSpan().data_equal(data));
}
} // namespace
