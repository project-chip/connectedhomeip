/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/data-model-provider/JitterDeferredAttributeChangeListener.h>
#include <app/data-model-provider/AttributeChangeListener.h>
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
constexpr std::array<ConcreteAttributePath, kTestPathsSize> kTestPaths = {
    ConcreteAttributePath(1, 1, 1), ConcreteAttributePath(2, 2, 2),    ConcreteAttributePath(3, 3, 3), ConcreteAttributePath(4, 4, 4),
    ConcreteAttributePath(5, 5, 5), ConcreteAttributePath(6, 6, 6),    ConcreteAttributePath(7, 7, 7), ConcreteAttributePath(8, 8, 8),
    ConcreteAttributePath(9, 9, 9), ConcreteAttributePath(10, 10, 10),
};
constexpr ConcreteAttributePath kOverflowPath(1, 2, 3);

constexpr uint32_t kBaseTimeoutMs   = 1000;
constexpr uint32_t kJitterTimeoutMs = 1000;

class MockAttributeChangeListener : public AttributeChangeListener
{
public:
    void OnAttributeChanged(const ConcreteAttributePath & path, AttributeChangeType type) override
    {
        mPaths.push_back(path);
        if (mPaths.size() == kTestPathsSize || path == kOverflowPath)
        {
            (void) chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
        }
    }

    void Reset() { mPaths.clear(); }

    std::vector<ConcreteAttributePath> mPaths;
};

class TestJitterDeferredAttributeChangeListener : public ::testing::Test
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

TEST_F(TestJitterDeferredAttributeChangeListener, TestJitteryDelay)
{
    MockAttributeChangeListener underlyingListener;
    JitterDeferredAttributeChangeListener jitteryListener(&underlyingListener, mMockTimerDelegate, kBaseTimeoutMs, kJitterTimeoutMs);

    for (size_t i = 0; i < kTestPathsSize; i++)
    {
        jitteryListener.OnAttributeChanged(kTestPaths[i], AttributeChangeType::kReportable);
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

TEST_F(TestJitterDeferredAttributeChangeListener, TestBufferFullFlush)
{
    MockAttributeChangeListener underlyingListener;
    JitterDeferredAttributeChangeListener jitteryListener(&underlyingListener, mMockTimerDelegate, kBaseTimeoutMs, kJitterTimeoutMs);

    for (size_t i = 0; i < kTestPathsSize; i++)
    {
        jitteryListener.OnAttributeChanged(kTestPaths[i], AttributeChangeType::kReportable);
    }

    // The underlying listener should not have been called yet.
    EXPECT_EQ(underlyingListener.mPaths.size(), 0u);

    // This call should overflow the buffer and trigger an immediate flush.
    jitteryListener.OnAttributeChanged(kOverflowPath, AttributeChangeType::kReportable);
    // The first batch of paths should be flushed immediately.
    for (size_t i = 0; i < kTestPathsSize; i++)
    {
        EXPECT_EQ(underlyingListener.mPaths[i], kTestPaths[i]);
    }
    EXPECT_EQ(underlyingListener.mPaths.size(), JitterDeferredAttributeChangeListener::kMaxAttributePathsBufferSize);

    mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds32(kBaseTimeoutMs + kJitterTimeoutMs));

    EXPECT_EQ(underlyingListener.mPaths.size(), JitterDeferredAttributeChangeListener::kMaxAttributePathsBufferSize + 1);
    EXPECT_EQ(underlyingListener.mPaths.back(), kOverflowPath);
}

TEST_F(TestJitterDeferredAttributeChangeListener, TestUpdateListenerConfiguration)
{
    MockAttributeChangeListener underlyingListener;
    JitterDeferredAttributeChangeListener jitteryListener(&underlyingListener, mMockTimerDelegate, kBaseTimeoutMs, 0);

    jitteryListener.OnAttributeChanged(kTestPaths[0], AttributeChangeType::kReportable);

    // Verify configuration updates dynamically
    AttributeChangeListenerConfiguration newConfig;
    newConfig.delay = (5000 << 16) | 0;
    jitteryListener.UpdateListenerConfiguration(newConfig);

    // Advance clock past original timeout of 1000ms. It shouldn't trigger since the new timeout is 5000ms.
    mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds32(1500));
    EXPECT_EQ(underlyingListener.mPaths.size(), 0u);

    // Advance past the new 5000ms timeout (cumulative 6500ms since start).
    mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds32(5000));
    EXPECT_EQ(underlyingListener.mPaths.size(), 1u);
    EXPECT_EQ(underlyingListener.mPaths[0], kTestPaths[0]);
}

} // namespace
