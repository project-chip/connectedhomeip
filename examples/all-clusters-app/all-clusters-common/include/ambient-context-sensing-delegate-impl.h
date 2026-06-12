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

#pragma once

#include <app/clusters/ambient-context-sensing-server/AmbientContextSensingDelegate.h>

namespace chip {
namespace app {
namespace Clusters {

using namespace chip::app::Clusters::AmbientContextSensing;

// Use the smaller value for not consuming too much resources
constexpr uint8_t kMaxACTypeSupported_s   = 20;
constexpr uint8_t kMaxPredictedActivity_s = 3;

class AmbientContextSensingDelegateImpl : public AmbientContextSensingDelegate
{
public:
    AmbientContextSensingDelegateImpl();
    ~AmbientContextSensingDelegateImpl() = default;

    SemanticTagType * GetAmbientContextTypeSupportedBuf(size_t size) override;

    CHIP_ERROR SetPredictedActivity(const Span<PredictedActivityType> & predictedActivityList) override;
    Span<PredictActivity> & GetPredictedActivity() override { return mPredictedActivityList; };

    DetectFuncResult FindAndUseAvailableDetection() override;
    AmbientContextSensed * GetAllocedDetection(const uint8_t id) override;
    CHIP_ERROR DelDetection(const uint8_t & id) override;

    uint64_t GetEpochNow() override;

    AmbientContextSensingDelegateImpl(const AmbientContextSensingDelegateImpl &)             = delete;
    AmbientContextSensingDelegateImpl & operator=(const AmbientContextSensingDelegateImpl &) = delete;
    AmbientContextSensingDelegateImpl(AmbientContextSensingDelegateImpl &&)                  = delete;
    AmbientContextSensingDelegateImpl & operator=(AmbientContextSensingDelegateImpl &&)      = delete;

private:
    SemanticTagType mAmbientContextTypeSupportedBuf[kMaxACTypeSupported_s];

    PredictActivity mPredictActivityBuf[kMaxPredictedActivity_s];
    Span<PredictActivity> mPredictedActivityList;

    // From spec, constraint of AmbientContextType is 1 to SimultaneousDetectionLimit.
    AmbientContextSensed mAmbientContextTypeList[kMaxSimultaneousDetectionLimit];
    bool mAmbientContextTypeListUsed[kMaxSimultaneousDetectionLimit];
};

} // namespace Clusters
} // namespace app
} // namespace chip
