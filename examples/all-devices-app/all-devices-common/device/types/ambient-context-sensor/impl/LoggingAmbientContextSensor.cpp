/*
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
#include "LoggingAmbientContextSensor.h"
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app::Clusters::AmbientContextSensing {

LoggingAmbientContextSensor::LoggingAmbientContextSensor(TimerDelegate & timerDelegate) :
    AmbientContextSensor(AmbientContextSensingConfig{ timerDelegate }
                             .WithFeatures(BitMask<AmbientContextSensing::Feature>(kFeatureAllForLog))
                             .WithHoldTime(10,
                                           {
                                               .holdTimeMin     = 1,
                                               .holdTimeMax     = 300,
                                               .holdTimeDefault = 10,
                                           }),
                         timerDelegate, *this),
    mAmbientContextTypeSupportedBuf{}, mPredictActivityBuf{}
{}

CHIP_ERROR LoggingAmbientContextSensor::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                 EndpointComposition composition)
{
    return AmbientContextSensor::Register(endpoint, provider, composition);
}

void LoggingAmbientContextSensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    AmbientContextSensor::Unregister(provider);
}

// AmbientContextSensingDelegate implementation
SemanticTagType * LoggingAmbientContextSensor::GetAmbientContextTypeSupportedBuf(size_t size)
{
    VerifyOrReturnError(size <= kMaxACTypeSupportedForLog, nullptr);
    return mAmbientContextTypeSupportedBuf;
}

CHIP_ERROR LoggingAmbientContextSensor::SetPredictedActivity(const Span<PredictedActivityType> & predictedActivityList)
{
    VerifyOrReturnError(predictedActivityList.size() <= kMaxPredictedActivityForLog, CHIP_ERROR_INVALID_ARGUMENT);

    // Copy the input predicted activity to local array
    for (size_t i = 0; i < predictedActivityList.size(); i++)
    {
        const auto & src = predictedActivityList[i];
        auto & dst       = mPredictActivityBuf[i];
        dst.mInfo        = src;

        if (!src.ambientContextType.HasValue())
        {
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

    return CHIP_NO_ERROR;
}

AmbientContextSensed * LoggingAmbientContextSensor::AllocDetection()
{
    for (uint8_t id = 0; id < kMaxSimultaneousDetectionLimit; id++)
    {
        if (mAmbientContextTypeList[id] == nullptr)
        {
            auto ptr                    = std::make_unique<AmbientContextSensed>();
            ptr->id                     = id;
            auto raw                    = ptr.get();
            mAmbientContextTypeList[id] = std::move(ptr);
            return raw;
        }
    }
    return nullptr;
}

CHIP_ERROR LoggingAmbientContextSensor::DelDetection(AmbientContextSensed * pitem)
{
    VerifyOrReturnError(pitem != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    const uint8_t id = pitem->id;
    VerifyOrReturnError(id < kMaxSimultaneousDetectionLimit, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((mAmbientContextTypeList[id] != nullptr) && (mAmbientContextTypeList[id].get() == pitem),
                        CHIP_ERROR_INVALID_ARGUMENT);
    mAmbientContextTypeList[id].reset();

    return CHIP_NO_ERROR;
}

uint64_t LoggingAmbientContextSensor::GetEpochNow()
{
    using namespace chip::System::Clock;
    Milliseconds64 timestamp_ms(0);
    CHIP_ERROR err = System::SystemClock().GetClock_RealTimeMS(timestamp_ms);

    return (err == CHIP_NO_ERROR) ? timestamp_ms.count() : 0;
}

} // namespace chip::app::Clusters::AmbientContextSensing
