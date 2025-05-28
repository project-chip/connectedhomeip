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

#include "platform/NetworkCommissioning.h"
#include <pw_unit_test/framework.h>

#include <app/clusters/network-commissioning/ThreadScanResponse.h>

namespace {

using namespace chip;
using namespace chip::TLV;
using namespace chip::app::Clusters::NetworkCommissioning;
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
    TestResponseIterator<2> fakeResponses{
        ThreadScanResponse{
            .panId           = 123,
            .extendedPanId   = 234,
            .networkName     = { 'f', 'o', 'o', 'b', 'a', 'r', 'b', 'a', 'z', 0 },
            .networkNameLen  = 9,
            .channel         = 12,
            .version         = 10,
            .extendedAddress = 2345,
            .rssi            = 10,
            .lqi             = 1,
        },
        ThreadScanResponse{
            .panId           = 321,
            .extendedPanId   = 112233,
            .networkName     = { 't', 'e', 's', 't', 0 },
            .networkNameLen  = 4,
            .channel         = 1,
            .version         = 100,
            .extendedAddress = 0x1234,
            .rssi            = 20,
            .lqi             = 2,
        },
    };
    ThreadScanResponseToTLV encoder(NetworkCommissioningStatusEnum::kSuccess, ""_span, nullptr);

    TLVWriter writer;
    uint8_t buffer[128];

    writer.Init(buffer, sizeof(buffer));

    TLVType container;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container), CHIP_NO_ERROR);
    ASSERT_EQ(encoder.EncodeTo(writer, kEncodeTag), CHIP_NO_ERROR);
    ASSERT_EQ(writer.EndContainer(container), CHIP_NO_ERROR);
}

} // namespace
