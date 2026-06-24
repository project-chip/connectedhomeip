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
#include <devices/ambient-context-sensor/AmbientContextSensorDevice.h>
#include <lib/support/TimerDelegate.h>

namespace chip::app::Clusters::AmbientContextSensing {

// Use the smaller value for not consuming too much resources
constexpr uint8_t kMaxACTypeSupported_s   = 20;
constexpr uint8_t kMaxPredictedActivity_s = 3;


/**
 * @brief A basic implementation of an Ambient Context Sensor Device.
 *
 * This class serves as a simple example of an ambient context sensor.
 */
class LoggingAmbientContextSensorDevice : public AmbientContextSensorDevice, public AmbientContextSensingDelegate
{
public:
    LoggingAmbientContextSensorDevice(TimerDelegate & timerDelegate);
    ~LoggingAmbientContextSensorDevice() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    SemanticTagType * GetAmbientContextTypeSupportedBuf(size_t size) override;
    CHIP_ERROR SetPredictedActivity(const Span<PredictedActivityType> & predictedActivityList) override;
    Span<PredictActivity> & GetPredictedActivity() override { return mPredictedActivityList; };
    DetectFuncResult FindAndUseAvailableDetection() override;
    AmbientContextSensed * GetAllocedDetection(const uint8_t id) override;
    CHIP_ERROR DelDetection(const uint8_t & id) override;
    uint64_t GetEpochNow() override;

private:
    SemanticTagType mAmbientContextTypeSupportedBuf[kMaxACTypeSupported_s];

    PredictActivity mPredictActivityBuf[kMaxPredictedActivity_s];
    Span<PredictActivity> mPredictedActivityList;

    // From spec, constraint of AmbientContextType is 1 to SimultaneousDetectionLimit.
    AmbientContextSensed mAmbientContextTypeList[kMaxSimultaneousDetectionLimit];
    bool mAmbientContextTypeListUsed[kMaxSimultaneousDetectionLimit];
};

} //chip::app::Clusters::AmbientContextSensing
