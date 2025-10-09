/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/network-commissioning/ThreadScanResponse.h>
#include <clusters/NetworkCommissioning/Commands.h>
#include <clusters/NetworkCommissioning/Ids.h>
#include <clusters/NetworkCommissioning/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLVReader.h>
#include <lib/core/TLVTypes.h>
#include <platform/NetworkCommissioning.h>

#include <vector>

namespace {

using namespace chip;
using namespace chip::TLV;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::NetworkCommissioning;
using namespace chip::app::Clusters::NetworkCommissioning::Structs;
using namespace chip::DeviceLayer::NetworkCommissioning;

template <size_t N>
class TestResponseIterator : public ThreadScanResponseIterator
{
public:
    template <typename... Args>
    TestResponseIterator(Args &&... args) : mItems{ { std::forward<Args>(args)... } }
    {}

    size_t Count() override { return N; }
    bool Next(ThreadScanResponse & item) override
    {
        if (mIndex < N)
        {
            item = mItems[mIndex];
            mIndex++;
            return true;
        }
        return false;
    }
    void Release() override {}

private:
    std::array<ThreadScanResponse, N> mItems;
    size_t mIndex = 0;
};

// Encoding tags are not relevant at the leven the encoder works
constexpr TLV::Tag kEncodeTag = TLV::ContextTag(1);

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestThreadResponseEncoding : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestThreadResponseEncoding, TestErrorEncode)
{
    ThreadScanResponseToTLV encoder(NetworkCommissioningStatusEnum::kUnknownError, "Some debug text"_span, nullptr);

    TLVWriter writer;
    uint8_t buffer[128];

    writer.Init(buffer, sizeof(buffer));

    TLVType container;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container), CHIP_NO_ERROR);
    ASSERT_EQ(encoder.EncodeTo(writer, kEncodeTag), CHIP_NO_ERROR);
    ASSERT_EQ(writer.EndContainer(container), CHIP_NO_ERROR);
}

TEST_F(TestThreadResponseEncoding, TestSuccessEncode)
{
    TestResponseIterator<3> fakeResponses{
        ThreadScanResponse{
            .panId           = 100,
            .extendedPanId   = 234,
            .networkName     = { 'f', 'o', 'o', 'b', 'a', 'r', 'b', 'a', 'z', 0 },
            .networkNameLen  = 9,
            .channel         = 12,
            .version         = 10,
            .extendedAddress = 2345,
            .rssi            = -33,
            .lqi             = 10,
        },
        ThreadScanResponse{
            .panId           = 321,
            .extendedPanId   = 112233,
            .networkName     = { 't', 'e', 's', 't', 0 },
            .networkNameLen  = 4,
            .channel         = 1,
            .version         = 100,
            .extendedAddress = 0x1234,
            .rssi            = -10,
            .lqi             = 20,
        },
        ThreadScanResponse{
            .panId           = 123,
            .extendedPanId   = 112233,
            .networkName     = { 'g', 'o', 'o', 'd', 0 },
            .networkNameLen  = 4,
            .channel         = 1,
            .version         = 100,
            .extendedAddress = 0x1234,
            .rssi            = -5,
            .lqi             = 5,
        },
    };
    ThreadScanResponseToTLV encoder(NetworkCommissioningStatusEnum::kSuccess, ""_span, &fakeResponses);

    uint8_t dataBuffer[256];

    {
        TLVWriter writer;
        writer.Init(dataBuffer, sizeof(dataBuffer));

        TLVType container;
        ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container), CHIP_NO_ERROR);
        ASSERT_EQ(encoder.EncodeTo(writer, kEncodeTag), CHIP_NO_ERROR);
        ASSERT_EQ(writer.EndContainer(container), CHIP_NO_ERROR);
    }

    {
        TLVReader reader;
        reader.Init(dataBuffer);

        // structure of TLV is:
        //   - AnonymousStructure (structure)
        //      - kEncodeTag (structure)
        //         - kNetworkingStatus
        //         - kDebugText
        //         - KThreadScanResults (array)
        TLVType containerType;

        ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
        ASSERT_EQ(reader.EnterContainer(containerType), CHIP_NO_ERROR);
        ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
        ASSERT_EQ(reader.GetTag(), kEncodeTag);

        NetworkCommissioning::Commands::ScanNetworksResponse::DecodableType response;
        ASSERT_EQ(response.Decode(reader), CHIP_NO_ERROR);

        ASSERT_EQ(reader.ExitContainer(containerType), CHIP_NO_ERROR);

        ASSERT_TRUE(response.threadScanResults.HasValue());
        std::vector<ThreadInterfaceScanResultStruct::DecodableType> items;

        auto value = response.threadScanResults.Value().begin();
        while (value.Next())
        {
            items.push_back(value.GetValue());
        }

        // assert expectations:
        //   - values exist
        //   - values sorted by RSSI in decreasing order (this is our implementation)
        //
        // NOTE: this does NOT match spec which says to use LQI
        //       Open issue: https://github.com/project-chip/connectedhomeip/issues/39309
        //
        // Actual spec requirement:
        //
        //   The order in which results are reported is implementation-specific.
        //   Results SHOULD be reported in decreasing LQI order, to maximize the
        //   likelihood that most likely to be reachable elements are included within
        //   the size limits of the response.
        ASSERT_EQ(items.size(), 3u);

        ASSERT_EQ(items[0].rssi, -5);
        ASSERT_EQ(items[0].lqi, 5);
        ASSERT_EQ(items[0].panId, 123);

        ASSERT_EQ(items[1].rssi, -10);
        ASSERT_EQ(items[1].lqi, 20);
        ASSERT_EQ(items[1].panId, 321);

        ASSERT_EQ(items[2].rssi, -33);
        ASSERT_EQ(items[2].lqi, 10);
        ASSERT_EQ(items[2].panId, 100);
    }
}

} // namespace
