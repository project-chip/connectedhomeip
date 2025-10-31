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

#include <app/data-model-provider/JitterDeferredProviderChangeListener.h>
#include <crypto/RandUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemTimer.h>

namespace chip {
namespace app {
namespace DataModel {

void JitterDeferredProviderChangeListener::MarkDirty(const AttributePathParams & path)
{
    if (mCurrentIndex >= kMaxAttributePathsBufferSize)
    {
        // When attribute path buffer is full, flush and process existing paths to make room
        FlushDirtyPaths();
    }
    mAttributePaths[mCurrentIndex++] = path;
    if (!mTimerActive)
    {
        uint32_t jitterMs =
            (mDeferAttributePathJitterTimeoutMs == 0) ? 0 : Crypto::GetRandU32() % mDeferAttributePathJitterTimeoutMs;
        DeviceLayer::SystemLayer().StartTimer(
            System::Clock::Milliseconds32(mDeferAttributePathBaseTimeoutMs + jitterMs),
            [](System::Layer *, void * me) {
                DeviceLayer::SystemLayer().ScheduleLambda(
                    [me] { static_cast<JitterDeferredProviderChangeListener *>(me)->TimerCallback(); });
            },
            this);
        mTimerActive = true;
    }
}

void JitterDeferredProviderChangeListener::FlushDirtyPaths()
{
    if (mUnderlyingListener != nullptr)
    {
        for (uint32_t i = 0; i < mCurrentIndex; i++)
        {
            mUnderlyingListener->MarkDirty(mAttributePaths[i]);
        }
    }
    mCurrentIndex = 0;
}

void JitterDeferredProviderChangeListener::TimerCallback()
{
    FlushDirtyPaths();
    mTimerActive = false;
}

} // namespace DataModel
} // namespace app
} // namespace chip
