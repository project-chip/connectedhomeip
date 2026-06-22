/**
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "ambient-context-sensing-delegate-impl.h"
#include <cassert>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemClock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

constexpr uint8_t kDefACSDelegateCount = 2;

AmbientContextSensingDelegate & AmbientContextSensingDelegate::GetInstance()
{
    static AmbientContextSensingDelegateImpl Instance[kDefACSDelegateCount];
    static uint8_t UsedInstanceCount = 0;

    assert(UsedInstanceCount < kDefACSDelegateCount);
    return Instance[UsedInstanceCount++];
}

AmbientContextSensingDelegateImpl::AmbientContextSensingDelegateImpl()
{
    for (auto & v : mAmbientContextTypeSupportedBuf)
    {
        v = SemanticTagType{};
    }

    for (auto & v : mPredictActivityBuf)
    {
        v = PredictActivity{};
    }

    mPredictedActivityList = Span<PredictActivity>(mPredictActivityBuf, 0);
    for (auto & v : mAmbientContextTypeListUsed)
    {
        v = false;
    }
}

SemanticTagType * AmbientContextSensingDelegateImpl::GetAmbientContextTypeSupportedBuf(size_t size)
{
    VerifyOrReturnError(size <= kMaxACTypeSupported, nullptr);
    return mAmbientContextTypeSupportedBuf;
}

CHIP_ERROR AmbientContextSensingDelegateImpl::SetPredictedActivity(const Span<PredictedActivityType> & predictedActivityList)
{
    VerifyOrReturnError(predictedActivityList.size() <= kMaxPredictedActivity, CHIP_ERROR_INVALID_ARGUMENT);

    // Copy the input predicted activity to local array
    for (size_t i = 0; i < predictedActivityList.size(); i++)
    {
        const auto & src = predictedActivityList[i];
        auto & dst       = mPredictActivityBuf[i];
        dst.mInfo        = src;

        if (!src.ambientContextType.HasValue())
        {
            dst.mInfo.ambientContextType.ClearValue();
            continue;
        }

        // Copy tags
        const auto & acTypeList = src.ambientContextType.Value();
        const auto tagCount     = acTypeList.size();
        VerifyOrReturnError(tagCount <= kMaxPredictedACType, CHIP_ERROR_INVALID_ARGUMENT);
        dst.mOwnedTags = std::make_unique<SemanticTagType[]>(tagCount);

        for (size_t t = 0; t < tagCount; t++)
        {
            dst.mOwnedTags[t] = acTypeList[t];
        }

        dst.mInfo.ambientContextType.SetValue(
            DataModel::List<const SemanticTagType>(dst.mOwnedTags.get(), static_cast<uint16_t>(tagCount)));
    }
    mPredictedActivityList = Span<PredictActivity>(mPredictActivityBuf, predictedActivityList.size());

    return CHIP_NO_ERROR;
}

DetectFuncResult AmbientContextSensingDelegateImpl::FindAndUseAvailableDetection()
{
    uint8_t i;
    for (i = 0; i < kMaxSimultaneousDetectionLimit; i++)
    {
        if (mAmbientContextTypeListUsed[i] == false)
        {
            break;
        }
    }
    if (i >= kMaxSimultaneousDetectionLimit)
    {
        // Can't find the available space
        return { .res = CHIP_ERROR_INCORRECT_STATE };
    }
    mAmbientContextTypeListUsed[i] = true;

    return { .res = CHIP_NO_ERROR, .id = i };
}

AmbientContextSensed * AmbientContextSensingDelegateImpl::GetAllocedDetection(const uint8_t id)
{
    VerifyOrReturnError(id < kMaxSimultaneousDetectionLimit, nullptr);
    VerifyOrReturnError(mAmbientContextTypeListUsed[id] == true, nullptr);
    return &mAmbientContextTypeList[id];
}

CHIP_ERROR AmbientContextSensingDelegateImpl::DelDetection(const uint8_t & id)
{
    VerifyOrReturnError(id < kMaxSimultaneousDetectionLimit, CHIP_ERROR_INVALID_ARGUMENT);
    mAmbientContextTypeListUsed[id] = false;

    return CHIP_NO_ERROR;
}

uint64_t AmbientContextSensingDelegateImpl::GetEpochNow()
{
    using namespace chip::System::Clock;
    Milliseconds64 timestamp_ms(0);
    CHIP_ERROR err = System::SystemClock().GetClock_RealTimeMS(timestamp_ms);

    return (err == CHIP_NO_ERROR) ? (timestamp_ms.count()) : (0);
}
