/*
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
#include <crypto/RandUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemLayer.h>
#include <system/SystemTimer.h>

namespace chip {
namespace app {
namespace DataModel {

void JitterDeferredAttributeChangeListener::OnAttributeChanged(const ConcreteAttributePath & path, AttributeChangeType type)
{
    // Quiet changes don't trigger report updates, but let's check if the path is EP0
    if (path.mEndpointId == kRootEndpointId || type == AttributeChangeType::kQuiet)
    {
        if (mUnderlyingListener != nullptr)
        {
            mUnderlyingListener->OnAttributeChanged(path, type);
        }
        return;
    }

    if (mCurrentIndex >= kMaxAttributePathsBufferSize)
    {
        FlushDirtyPaths();
    }

    for (uint32_t i = 0; i < mCurrentIndex; ++i)
    {
        if (mAttributePaths[i] == path)
        {
            return;
        }
    }
    mAttributePaths[mCurrentIndex++] = path;
    if (!mTimer.IsTimerActive(this))
    {
        uint32_t jitterMs =
            (mDeferAttributePathJitterTimeoutMs == 0) ? 0 : Crypto::GetRandU32() % mDeferAttributePathJitterTimeoutMs;
        (void) mTimer.StartTimer(this, System::Clock::Milliseconds32(mDeferAttributePathBaseTimeoutMs + jitterMs));
    }
}

void JitterDeferredAttributeChangeListener::OnEndpointChanged(EndpointId endpointId, EndpointChangeType type)
{
    if (mUnderlyingListener != nullptr)
    {
        mUnderlyingListener->OnEndpointChanged(endpointId, type);
    }
}

void JitterDeferredAttributeChangeListener::FlushDirtyPaths()
{
    if (mUnderlyingListener != nullptr)
    {
        for (uint32_t i = 0; i < mCurrentIndex; i++)
        {
            mUnderlyingListener->OnAttributeChanged(mAttributePaths[i], AttributeChangeType::kReportable);
        }
    }
    mCurrentIndex = 0;
}

void JitterDeferredAttributeChangeListener::TimerFired()
{
    FlushDirtyPaths();
    mDeferAttributePathBaseTimeoutMs   = kDefaultDeferAttributePathBaseTimeoutMs;
    mDeferAttributePathJitterTimeoutMs = kDefaultDeferAttributePathJitterTimeoutMs;
}

void JitterDeferredAttributeChangeListener::UpdateListenerConfiguration(const AttributeChangeListenerConfiguration & config)
{
    if (config.delay.has_value())
    {
        uint32_t delay = config.delay.value();
        mDeferAttributePathBaseTimeoutMs   = (uint16_t) (delay >> 16);
        mDeferAttributePathJitterTimeoutMs = (uint16_t) (delay & 0xFFFF);

        if (mTimer.IsTimerActive(this))
        {
            mTimer.CancelTimer(this);
            uint32_t jitterMs =
                (mDeferAttributePathJitterTimeoutMs == 0) ? 0 : Crypto::GetRandU32() % mDeferAttributePathJitterTimeoutMs;
            (void) mTimer.StartTimer(this, System::Clock::Milliseconds32(mDeferAttributePathBaseTimeoutMs + jitterMs));
        }
    }
}

} // namespace DataModel
} // namespace app
} // namespace chip
