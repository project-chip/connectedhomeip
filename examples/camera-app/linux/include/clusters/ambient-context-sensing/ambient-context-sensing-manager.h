/*
 *
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

#pragma once
#include <app/clusters/ambient-context-sensing-server/AmbientContextSensingDelegate.h>

namespace chip::app::Clusters::AmbientContextSensing {

/**
 * @brief Delegate implementation for the AmbientContextSensing cluster used by the camera app.
 *
 * Owns the backing buffers required by the cluster for the AmbientContextTypeSupported,
 * PredictedActivity and SensorFusionSupported attributes, and allocates the per-detection
 * bookkeeping objects used to track active ambient context detections.
 */
class AmbientContextSensingManager : public AmbientContextSensingDelegate
{
public:
    AmbientContextSensingManager()           = default;
    ~AmbientContextSensingManager() override = default;

    // AmbientContextSensingDelegate implementation
    SemanticTagType * GetAmbientContextTypeSupportedBuf(size_t size) override;
    CHIP_ERROR SetPredictedActivity(const Span<PredictedActivityType> & predictedActivityList) override;
    PredictActivity * GetPredictedActivityBuf() override { return mPredictedActivityBuf; }
    SemanticTagType * GetSensorFusionSupportedBuf(size_t size) override;
    AmbientContextSensed * AllocDetection() override;
    CHIP_ERROR DelDetection(AmbientContextSensed * pitem) override;
    uint64_t GetEpochNow() override;

private:
    static constexpr uint8_t kMaxAmbientContextTypeSupported = 20;
    static constexpr uint8_t kMaxPredictedActivity           = 5;
    static constexpr uint8_t kMaxSensorFusionSupported       = 20;

    SemanticTagType mAmbientContextTypeSupportedBuf[kMaxAmbientContextTypeSupported]{};
    PredictActivity mPredictedActivityBuf[kMaxPredictedActivity]{};
    SemanticTagType mSensorFusionSupportedBuf[kMaxSensorFusionSupported]{};

    // From spec, the constraint on AmbientContextType is 1 to SimultaneousDetectionLimit.
    std::unique_ptr<AmbientContextSensed> mAmbientContextTypeList[kMaxSimultaneousDetectionLimit];
};

} // namespace chip::app::Clusters::AmbientContextSensing
