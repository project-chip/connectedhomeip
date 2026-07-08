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
            (mDeferAttributePathJitterWindowMs == 0) ? 0 : Crypto::GetRandU32() % mDeferAttributePathJitterWindowMs;
        CHIP_ERROR err = mTimer.StartTimer(this, System::Clock::Milliseconds32(mDeferAttributePathMinMs + jitterMs));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DataManagement, "Failed to start deferred attribute change timer: %" CHIP_ERROR_FORMAT, err.Format());
            FlushDirtyPaths();
        }
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
    mDeferAttributePathMinMs          = 0;
    mDeferAttributePathJitterWindowMs = 0;
}

void JitterDeferredAttributeChangeListener::UpdateListenerConfiguration(const AttributeChangeListenerConfiguration & config)
{
    bool isConfigurationChanged = false;

    if (config.delayMinMs.has_value())
    {
        mDeferAttributePathMinMs = config.delayMinMs.value();
        isConfigurationChanged = true;
    }
    if (config.delayJitterWindowMs.has_value())
    {
        mDeferAttributePathJitterWindowMs = config.delayJitterWindowMs.value();
        isConfigurationChanged = true;
    }

    if (isConfigurationChanged && mTimer.IsTimerActive(this))
    {
        mTimer.CancelTimer(this);
        uint32_t jitterMs =
            (mDeferAttributePathJitterWindowMs == 0) ? 0 : Crypto::GetRandU32() % mDeferAttributePathJitterWindowMs;
        CHIP_ERROR err = mTimer.StartTimer(this, System::Clock::Milliseconds32(mDeferAttributePathMinMs + jitterMs));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DataManagement, "Failed to start deferred attribute change timer: %" CHIP_ERROR_FORMAT, err.Format());
            FlushDirtyPaths();
        }
    }
}

} // namespace DataModel
} // namespace app
} // namespace chip
