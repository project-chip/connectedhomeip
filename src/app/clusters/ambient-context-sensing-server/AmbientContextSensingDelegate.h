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
#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/IntrusiveList.h>

namespace chip::app::Clusters::AmbientContextSensing {

using SemanticTagType           = Globals::Structs::SemanticTagStruct::Type;
using AmbientContextSensingType = AmbientContextSensing::Structs::AmbientContextTypeStruct::Type;
using ObjectCountConfigType     = AmbientContextSensing::Structs::ObjectCountConfigStruct::Type;
using PredictedActivityType     = AmbientContextSensing::Structs::PredictedActivityStruct::Type;

// Default value for implementation sake. They should be set by device vendor
constexpr uint8_t kDefaultSimultaneousDetectionLimit = 5;
constexpr uint16_t kDefaultCountThreshold            = 5;
constexpr uint16_t kDefaultHoldTimeMin               = 10;
constexpr uint16_t kDefaultHoldTimeMax               = 300;
constexpr uint16_t kDefaultHoldTimeDefault           = 50;

// Default value in spec
constexpr uint8_t kMaxACSensed                   = 2;
constexpr uint8_t kMaxACTypeSupported            = 50;
constexpr uint8_t kMaxSimultaneousDetectionLimit = 10;
constexpr uint16_t kMinObjectCount               = 1;
constexpr uint8_t kMaxPredictedACType            = 100;
constexpr uint8_t kMaxPredictedActivity          = 20;

struct AmbientContextSensed : public IntrusiveListNodeBase<>
{
    AmbientContextSensingType mInfo;
    SemanticTagType mOwnedTags[kMaxACSensed];
    System::Clock::Timestamp mStartTimestamp = System::Clock::Milliseconds64(0);
    System::Clock::Timestamp mEndTimestamp;
    uint64_t mStartEpoch;
    uint8_t id;
};

struct PredictActivity
{
    PredictedActivityType mInfo;
    std::unique_ptr<SemanticTagType[]> mOwnedTags;
};

struct DetectFuncResult
{
    // result of the function
    CHIP_ERROR res;
    // The id of the object
    uint8_t id;
};

/*
    This is the delegate of AmbientContextSensing server. It will own the buffers to keep the attributes:
        - AmbientContextType,
        - AmbientContextTypeSupported
        - PredictedActivity
*/
class AmbientContextSensingDelegate
{
public:
    virtual ~AmbientContextSensingDelegate() = default;

    static AmbientContextSensingDelegate & GetInstance();

    // Buffer to keep the AmbientContextTypeSupported attribute passed from the caller
    virtual SemanticTagType * GetAmbientContextTypeSupportedBuf(size_t size) = 0;

    // Save the PredictedActivity attribute passed from the caller
    virtual CHIP_ERROR SetPredictedActivity(const Span<PredictedActivityType> & predictedActivityList) = 0;

    // Return the stored PredictedActivity
    virtual Span<PredictActivity> & GetPredictedActivity() = 0;

    // Retrieve the id of an available AmbientContextType from delegate and mark it as allocated
    virtual DetectFuncResult FindAndUseAvailableDetection() = 0;
    // Get the pointer of the space from the returned id in FindAndUseAvailableDetection()
    virtual AmbientContextSensed * GetAllocedDetection(const uint8_t id) = 0;
    // Return the space by passing the id
    virtual CHIP_ERROR DelDetection(const uint8_t & id) = 0;

    // Return the current epoch
    virtual uint64_t GetEpochNow() = 0;
};

} // namespace chip::app::Clusters::AmbientContextSensing
