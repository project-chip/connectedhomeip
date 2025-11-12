/*
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
#pragma once

#include <app/AttributePathParams.h>
#include <app/data-model-provider/ProviderChangeListener.h>
#include <lib/support/TimerDelegate.h>
#include <system/SystemLayer.h>

#include <array>

namespace chip {
namespace app {
namespace DataModel {

/**
 * @brief A customized ProviderChangeListener that introduces a jitter delay before marking attributes dirty.
 *
 * This implementation adds a randomized delay (a constant base time plus jitter) before notifying the reporting engine that an
 * attribute path is dirty. The primary goal is to delay DataReports sending, thereby reducing overall network traffic. This
 * approach has proven effective in mitigating network congestion and reducing packet retransmissions, particularly on constrained
 * networks like Thread.
 */
class JitterDeferredProviderChangeListener : public ProviderChangeListener, public TimerContext
{
public:
    static constexpr uint32_t kMaxAttributePathsBufferSize       = 10;
    static constexpr uint32_t kDeferAttributePathBaseTimeoutMs   = 1000;
    static constexpr uint32_t kDeferAttributePathJitterTimeoutMs = 1000;

    JitterDeferredProviderChangeListener(ProviderChangeListener * aUnderlyingListener, TimerDelegate & aTimer,
                                         uint32_t aDeferAttributePathBaseTimeoutMs, uint32_t aDeferAttributePathJitterTimeoutMs) :
        mUnderlyingListener(aUnderlyingListener), mTimer(aTimer),
        mDeferAttributePathBaseTimeoutMs(aDeferAttributePathBaseTimeoutMs),
        mDeferAttributePathJitterTimeoutMs(aDeferAttributePathJitterTimeoutMs)
    {}

    JitterDeferredProviderChangeListener(ProviderChangeListener * aUnderlyingListener, TimerDelegate & aTimer) :
        JitterDeferredProviderChangeListener(aUnderlyingListener, aTimer, kDeferAttributePathBaseTimeoutMs,
                                             kDeferAttributePathJitterTimeoutMs)
    {}

    void MarkDirty(const AttributePathParams & path) override;
    /**
     * @brief Called when the identify timer fires.
     */
    void TimerFired() override;

private:
    void FlushDirtyPaths();

    ProviderChangeListener * mUnderlyingListener;
    TimerDelegate & mTimer;
    uint32_t mDeferAttributePathBaseTimeoutMs;
    uint32_t mDeferAttributePathJitterTimeoutMs;
    std::array<AttributePathParams, kMaxAttributePathsBufferSize> mAttributePaths;
    uint32_t mCurrentIndex = 0;
};

} // namespace DataModel
} // namespace app
} // namespace chip
