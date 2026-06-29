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
#include "LoggingAmbientContextSensorDevice.h"
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app::Clusters::AmbientContextSensing {

constexpr uint32_t g_kFeatures_all = static_cast<uint32_t>(Feature::kHumanActivity) |
    static_cast<uint32_t>(Feature::kObjectCounting) | static_cast<uint32_t>(Feature::kObjectIdentification) |
    static_cast<uint32_t>(Feature::kSoundIdentification) | static_cast<uint32_t>(Feature::kPredictedActivity);

LoggingAmbientContextSensorDevice::LoggingAmbientContextSensorDevice(TimerDelegate & timerDelegate) :
    AmbientContextSensorDevice(
        // Initialize with kInvalidEndpointId. The actual endpoint ID will be set
        // when Register() is called by the application with a valid endpoint ID.
        AmbientContextSensingConfig{ kInvalidEndpointId, *this, timerDelegate }
            .WithFeatures(AmbientContextSensing::Feature(g_kFeatures_all))
            .WithHoldTime(10,
                          {
                              .holdTimeMin     = 1,
                              .holdTimeMax     = 300,
                              .holdTimeDefault = 10,
                          }),
        timerDelegate),
    mAmbientContextTypeSupportedBuf{}, mPredictActivityBuf{}, mPredictedActivityList(mPredictActivityBuf, 0),
    mAmbientContextTypeListUsed{}
{}

CHIP_ERROR LoggingAmbientContextSensorDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                       EndpointComposition composition)
{
    return AmbientContextSensorDevice::Register(endpoint, provider, composition);
}

void LoggingAmbientContextSensorDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    AmbientContextSensorDevice::Unregister(provider);
}

// AmbientContextSensingDelegate implementation
SemanticTagType * LoggingAmbientContextSensorDevice::GetAmbientContextTypeSupportedBuf(size_t size)
{
    VerifyOrReturnError(size <= kMaxACTypeSupported_s, nullptr);
    return mAmbientContextTypeSupportedBuf;
}

CHIP_ERROR LoggingAmbientContextSensorDevice::SetPredictedActivity(const Span<PredictedActivityType> & predictedActivityList)
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

DetectFuncResult LoggingAmbientContextSensorDevice::FindAndUseAvailableDetection()
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

AmbientContextSensed * LoggingAmbientContextSensorDevice::GetAllocedDetection(const uint8_t id)
{
    VerifyOrReturnError(id < kMaxSimultaneousDetectionLimit, nullptr);
    VerifyOrReturnError(mAmbientContextTypeListUsed[id] == true, nullptr);
    return &mAmbientContextTypeList[id];
}

CHIP_ERROR LoggingAmbientContextSensorDevice::DelDetection(const uint8_t & id)
{
    VerifyOrReturnError(id < kMaxSimultaneousDetectionLimit, CHIP_ERROR_INVALID_ARGUMENT);
    mAmbientContextTypeListUsed[id] = false;

    return CHIP_NO_ERROR;
}

uint64_t LoggingAmbientContextSensorDevice::GetEpochNow()
{
    using namespace chip::System::Clock;
    Milliseconds64 timestamp_ms(0);
    CHIP_ERROR err = System::SystemClock().GetClock_RealTimeMS(timestamp_ms);

    return (err == CHIP_NO_ERROR) ? (timestamp_ms.count()) : (0);
}

} // namespace chip::app::Clusters::AmbientContextSensing
