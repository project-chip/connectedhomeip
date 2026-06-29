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
#include <lib/support/logging/CHIPLogging.h>
#include <memory>
#include <system/SystemClock.h>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

AmbientContextSensingDelegate & AmbientContextSensingDelegate::AllocateInstance()
{
    static std::vector<std::unique_ptr<AmbientContextSensingDelegateImpl>> instances;

    instances.push_back(std::make_unique<AmbientContextSensingDelegateImpl>());
    return *instances.back();
}

AmbientContextSensingDelegateImpl::AmbientContextSensingDelegateImpl()
    : mAmbientContextTypeSupportedBuf{},
      mPredictActivityBuf{},
      mPredictedActivityList(mPredictActivityBuf, 0)
{
}

SemanticTagType * AmbientContextSensingDelegateImpl::GetAmbientContextTypeSupportedBuf(size_t size)
{
    VerifyOrReturnError(size <= kMaxACTypeSupported_s, nullptr);
    return mAmbientContextTypeSupportedBuf;
}

CHIP_ERROR AmbientContextSensingDelegateImpl::SetPredictedActivity(const Span<PredictedActivityType> & predictedActivityList)
{
    VerifyOrReturnError(predictedActivityList.size() <= kMaxPredictedActivity_s, CHIP_ERROR_INVALID_ARGUMENT);

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

AmbientContextSensed * AmbientContextSensingDelegateImpl::AllocDetection()
{
    for (uint8_t id = 0; id < kMaxSimultaneousDetectionLimit; id++)
    {
        if (mAmbientContextTypeList[id] == nullptr)
        {
            auto ptr = std::make_unique<AmbientContextSensed>();
            ptr->id = id;
            auto raw = ptr.get();
            mAmbientContextTypeList[id] = std::move(ptr);
            return raw;
        }
    }
    return nullptr;
}

CHIP_ERROR AmbientContextSensingDelegateImpl::DelDetection(AmbientContextSensed * pitem)
{
    VerifyOrReturnError(pitem != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    const uint8_t id = pitem->id;
    VerifyOrReturnError(id < kMaxSimultaneousDetectionLimit, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((mAmbientContextTypeList[id] != nullptr) &&
        (mAmbientContextTypeList[id].get() == pitem),
        CHIP_ERROR_INVALID_ARGUMENT);
    mAmbientContextTypeList[id].reset();

    return CHIP_NO_ERROR;
}

uint64_t AmbientContextSensingDelegateImpl::GetEpochNow()
{
    using namespace chip::System::Clock;
    Milliseconds64 timestamp_ms(0);
    CHIP_ERROR err = System::SystemClock().GetClock_RealTimeMS(timestamp_ms);

    return (err == CHIP_NO_ERROR) ? (timestamp_ms.count()) : (0);
}
