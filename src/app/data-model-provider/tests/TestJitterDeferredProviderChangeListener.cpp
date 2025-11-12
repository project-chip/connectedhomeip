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

#include <app/data-model-provider/JitterDeferredProviderChangeListener.h>
#include <app/data-model-provider/ProviderChangeListener.h>
#include <lib/support/TimerDelegateMock.h>
#include <platform/CHIPDeviceLayer.h>
#include <pw_unit_test/framework.h>
#include <system/SystemClock.h>
#include <system/SystemTimer.h>

#include <array>
#include <cstdint>
#include <vector>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;

constexpr size_t kTestPathsSize                                      = 10;
constexpr std::array<AttributePathParams, kTestPathsSize> kTestPaths = {
    AttributePathParams(1, 1, 1), AttributePathParams(2, 2, 2),    AttributePathParams(3, 3, 3), AttributePathParams(4, 4, 4),
    AttributePathParams(5, 5, 5), AttributePathParams(6, 6, 6),    AttributePathParams(7, 7, 7), AttributePathParams(8, 8, 8),
    AttributePathParams(9, 9, 9), AttributePathParams(10, 10, 10),
};
constexpr AttributePathParams kOverflowPath(1, 2, 3);

constexpr uint32_t kBaseTimeoutMs   = 1000;
constexpr uint32_t kJitterTimeoutMs = 1000;

class MockProviderChangeListener : public ProviderChangeListener
{
public:
    void MarkDirty(const AttributePathParams & path) override
    {
        mPaths.push_back(path);
        if (mPaths.size() == kTestPathsSize || path == kOverflowPath)
        {
            chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
        }
    }

    void Reset() { mPaths.clear(); }

    std::vector<AttributePathParams> mPaths;
};

class TestJitterDeferredProviderChangeListener : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        chip::Platform::MemoryShutdown();
        chip::DeviceLayer::PlatformMgr().Shutdown();
    }
    TimerDelegateMock mMockTimerDelegate;
};

TEST_F(TestJitterDeferredProviderChangeListener, TestJitteryDelay)
{
    MockProviderChangeListener underlyingListener;
    JitterDeferredProviderChangeListener jitteryListener(&underlyingListener, mMockTimerDelegate, kBaseTimeoutMs, kJitterTimeoutMs);

    for (size_t i = 0; i < kTestPathsSize; i++)
    {
        jitteryListener.MarkDirty(kTestPaths[i]);
    }

    // The underlying listener should not have been called yet.
    EXPECT_EQ(underlyingListener.mPaths.size(), 0u);

    mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds32(kBaseTimeoutMs + kJitterTimeoutMs));

    // Now the underlying listener should have been called.
    EXPECT_EQ(underlyingListener.mPaths.size(), kTestPathsSize);
    for (size_t i = 0; i < kTestPathsSize; i++)
    {
        EXPECT_EQ(underlyingListener.mPaths[i], kTestPaths[i]);
    }
}

TEST_F(TestJitterDeferredProviderChangeListener, TestBufferFullFlush)
{
    MockProviderChangeListener underlyingListener;
    JitterDeferredProviderChangeListener jitteryListener(&underlyingListener, mMockTimerDelegate, kBaseTimeoutMs, kJitterTimeoutMs);

    for (size_t i = 0; i < kTestPathsSize; i++)
    {
        jitteryListener.MarkDirty(kTestPaths[i]);
    }

    // The underlying listener should not have been called yet.
    EXPECT_EQ(underlyingListener.mPaths.size(), 0u);

    // This call should overflow the buffer and trigger an immediate flush.
    jitteryListener.MarkDirty(kOverflowPath);
    // The first batch of paths should be flushed immediately.
    for (size_t i = 0; i < kTestPathsSize; i++)
    {
        EXPECT_EQ(underlyingListener.mPaths[i], kTestPaths[i]);
    }
    EXPECT_EQ(underlyingListener.mPaths.size(), JitterDeferredProviderChangeListener::kMaxAttributePathsBufferSize);

    mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds32(kBaseTimeoutMs + kJitterTimeoutMs));

    EXPECT_EQ(underlyingListener.mPaths.size(), JitterDeferredProviderChangeListener::kMaxAttributePathsBufferSize + 1);
    EXPECT_EQ(underlyingListener.mPaths.back(), kOverflowPath);
}

} // namespace
