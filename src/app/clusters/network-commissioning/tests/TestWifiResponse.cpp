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

#include "lib/support/BitFlags.h"
#include "platform/NetworkCommissioning.h"
#include <pw_unit_test/framework.h>

#include <app/clusters/network-commissioning/WifiScanResponse.h>
#include <lib/core/TLV.h>

#include <array>

namespace {

using namespace chip;
using namespace chip::TLV;
using namespace chip::app::Clusters::NetworkCommissioning;
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
    TestResponseIterator<2> fakeResponses{ WiFiScanResponse{
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
                                           }

    };
    WifiScanResponseToTLV encoder(NetworkCommissioningStatusEnum::kSuccess, ""_span, nullptr);

    TLVWriter writer;
    uint8_t buffer[128];

    writer.Init(buffer, sizeof(buffer));

    TLVType container;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container), CHIP_NO_ERROR);
    ASSERT_EQ(encoder.EncodeTo(writer, kEncodeTag), CHIP_NO_ERROR);
    ASSERT_EQ(writer.EndContainer(container), CHIP_NO_ERROR);
}

} // namespace
