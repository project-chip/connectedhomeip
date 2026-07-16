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
#pragma once

#include <app/clusters/ambient-context-sensing-server/AmbientContextSensingCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/types/ambient-context-sensor/AmbientContextSensor.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app::Clusters::AmbientContextSensing {

/**
 * @brief A basic implementation of an Ambient Context Sensor Device.
 *
 * This class serves as a simple example of an ambient context sensor.
 */
class LoggingAmbientContextSensor : public AmbientContextSensor, public AmbientContextSensingDelegate
{
public:
    LoggingAmbientContextSensor(TimerDelegate & timerDelegate);
    ~LoggingAmbientContextSensor() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    SemanticTagType * GetAmbientContextTypeSupportedBuf(size_t size) override;
    CHIP_ERROR SetPredictedActivity(const Span<PredictedActivityType> & predictedActivityList) override;
    PredictActivity * GetPredictedActivityBuf() override { return mPredictActivityBuf; };
    AmbientContextSensed * AllocDetection() override;
    CHIP_ERROR DelDetection(AmbientContextSensed * pitem) override;
    uint64_t GetEpochNow() override;

    // Use the smaller value for not consuming too much resources
    static constexpr uint8_t kMaxACTypeSupportedForLog   = 20;
    static constexpr uint8_t kMaxPredictedActivityForLog = 3;
    static constexpr BitFlags<Feature> kFeatureAllForLog{ Feature::kHumanActivity, Feature::kObjectCounting,
                                                          Feature::kObjectIdentification, Feature::kSoundIdentification,
                                                          Feature::kPredictedActivity };

private:
    SemanticTagType mAmbientContextTypeSupportedBuf[kMaxACTypeSupportedForLog];

    PredictActivity mPredictActivityBuf[kMaxPredictedActivityForLog];

    // From spec, constraint of AmbientContextType is 1 to SimultaneousDetectionLimit.
    std::unique_ptr<AmbientContextSensed> mAmbientContextTypeList[kMaxSimultaneousDetectionLimit];
};

} // namespace chip::app::Clusters::AmbientContextSensing
