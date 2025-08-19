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

#include <app/clusters/network-commissioning/WifiScanResponse.h>
#include <clusters/NetworkCommissioning/Commands.h>
#include <clusters/NetworkCommissioning/Ids.h>
#include <clusters/NetworkCommissioning/Structs.h>
#include <lib/core/TLV.h>
#include <lib/support/BitFlags.h>
#include <platform/NetworkCommissioning.h>

#include <array>
#include <vector>

namespace {

using namespace chip;
using namespace chip::TLV;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::NetworkCommissioning;
using namespace chip::app::Clusters::NetworkCommissioning::Structs;
using namespace chip::DeviceLayer::NetworkCommissioning;

template <size_t N>
class TestResponseIterator : public WiFiScanResponseIterator
{
public:
    template <typename... Args>
    TestResponseIterator(Args &&... args) : mItems{ { std::forward<Args>(args)... } }
    {}

    size_t Count() override { return N; }
    bool Next(WiFiScanResponse & item) override
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
    std::array<WiFiScanResponse, N> mItems;
    size_t mIndex = 0;
};

// Encoding tags are not relevant at the leven the encoder works
constexpr TLV::Tag kEncodeTag = TLV::ContextTag(1);

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestWifiResponseEncoding : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestWifiResponseEncoding, TestErrorEncode)
{
    WifiScanResponseToTLV encoder(NetworkCommissioningStatusEnum::kUnknownError, "Some debug text"_span, nullptr);

    TLVWriter writer;
    uint8_t buffer[128];

    writer.Init(buffer, sizeof(buffer));

    TLVType container;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container), CHIP_NO_ERROR);
    ASSERT_EQ(encoder.EncodeTo(writer, kEncodeTag), CHIP_NO_ERROR);
    ASSERT_EQ(writer.EndContainer(container), CHIP_NO_ERROR);
}

TEST_F(TestWifiResponseEncoding, TestSuccessEncode)
{
    TestResponseIterator<3> fakeResponses{ WiFiScanResponse{
                                               .security = BitFlags<WiFiSecurityBitmap>(WiFiSecurityBitmap::kWpa2Personal),
                                               .ssid     = { 'f', 'o', 'o', 0 },
                                               .ssidLen  = 3,
                                               .bssid    = { '1', '2', '3', '4', 0 },
                                               .channel  = 123,
                                               .wiFiBand = WiFiBandEnum::k5g,
                                               .rssi     = 10,
                                           },
                                           WiFiScanResponse{
                                               .security = BitFlags<WiFiSecurityBitmap>(WiFiSecurityBitmap::kWpa2Personal),
                                               .ssid     = { 'b', 'a', 'r', 0 },
                                               .ssidLen  = 3,
                                               .bssid    = { 'x', 'y', 'z', 0 },
                                               .channel  = 321,
                                               .wiFiBand = WiFiBandEnum::k2g4,
                                               .rssi     = 20,
                                           },
                                           WiFiScanResponse{
                                               .security = BitFlags<WiFiSecurityBitmap>(WiFiSecurityBitmap::kWpa2Personal),
                                               .ssid     = { 'b', 'a', 0 },
                                               .ssidLen  = 2,
                                               .bssid    = { 'z', 'z', 'z', 0 },
                                               .channel  = 100,
                                               .wiFiBand = WiFiBandEnum::k2g4,
                                               .rssi     = 15,
                                           }

    };
    WifiScanResponseToTLV encoder(NetworkCommissioningStatusEnum::kSuccess, ""_span, &fakeResponses);

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

        ASSERT_TRUE(response.wiFiScanResults.HasValue());
        std::vector<WiFiInterfaceScanResultStruct::DecodableType> items;

        auto value = response.wiFiScanResults.Value().begin();
        while (value.Next())
        {
            items.push_back(value.GetValue());
        }

        // assert expectations:
        //   - values exist
        //   - Order is preserved (we assume drivers already sort the order so
        //     responses preserve order)
        //
        //   Actual spec requirement:
        //
        //   The order in which results are reported is implementation-specific.
        //   Results SHOULD be reported in decreasing RSSI order, even if RSSI
        //   is not reported in the response, to maximize the likelihood that
        //   most likely to be reachable elements are included within the
        //   size limits of the response
        ASSERT_EQ(items.size(), 3u);

        ASSERT_EQ(items[0].rssi, 10);
        ASSERT_EQ(items[0].channel, 123);

        ASSERT_EQ(items[1].rssi, 20);
        ASSERT_EQ(items[1].channel, 321);

        ASSERT_EQ(items[2].rssi, 15);
        ASSERT_EQ(items[2].channel, 100);
    }
}

} // namespace
