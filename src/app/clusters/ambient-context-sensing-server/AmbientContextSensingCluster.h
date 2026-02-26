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
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <platform/DefaultTimerDelegate.h>
#include <lib/support/Span.h>
#include <vector>
#include <list>
#include "ambient-context-sensing-namespace.h"

namespace chip::app::Clusters {
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
constexpr uint8_t kMaxPredictedActivity          = 20;

using SemanticTagType = Globals::Structs::SemanticTagStruct::Type;
using AmbientContextSensingType = AmbientContextSensing::Structs::AmbientContextTypeStruct::Type;
using ObjectCountConfigType = AmbientContextSensing::Structs::ObjectCountConfigStruct::Type;
using PredictedActivityType = AmbientContextSensing::Structs::PredictedActivityStruct::Type;

class AmbientContextSensingCluster : public DefaultServerCluster, public TimerContext
{
public:
    struct Config
    {
        Config(EndpointId endpointId) : mEndpointId(endpointId) {}

        Config & WithFeatures(AmbientContextSensing::Feature featureMap)
        {
            mFeatureMap = featureMap;
            return *this;
        }

        Config & WithHoldTime(uint16_t aHoldTime, const AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type & aHoldTimeLimits,
                              TimerDelegate & aTimerDelegate)
        {
            mHoldTime       = aHoldTime;
            mHoldTimeLimits = aHoldTimeLimits;
            mHoldTimeDelegate = &aTimerDelegate;
            return *this;
        }

        Config & WithAmbientContextSupported(chip::Span <const SemanticTagType> acsTypes)
        {
            mAmbientContextTypeSupportedList.assign(acsTypes.begin(), acsTypes.end());
            return *this;
        }

        EndpointId mEndpointId;
        BitMask<AmbientContextSensing::Feature> mFeatureMap = 0;
        ObjectCountConfigType mObjectCountConfig            = {
            .countingObject  = {
                .namespaceID = kNamespaceIdentifiedObject,
                .tag         = static_cast<uint8_t>(TagIdentifiedObject::kUnknown),
            },
            .objectCountThreshold = kDefaultCountThreshold,
            };
        uint16_t mHoldTime                                                         = kDefaultHoldTimeDefault;
        AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type mHoldTimeLimits = {
            .holdTimeMin     = kDefaultHoldTimeMin,
            .holdTimeMax     = kDefaultHoldTimeMax,
            .holdTimeDefault = kDefaultHoldTimeDefault
            };
        TimerDelegate * mHoldTimeDelegate = nullptr;
        std::vector<SemanticTagType> mAmbientContextTypeSupportedList;

    };

    struct AmbientContextSensed
    {
        AmbientContextSensingType mInfo;
        std::vector<SemanticTagType> mOwnedTags;
        System::Clock::Timestamp mStartTimestamp = System::Clock::Milliseconds64(0);
        System::Clock::Timestamp mEndTimestamp;
    };

    struct PredictActivity
    {
        PredictedActivityType mInfo;
        std::vector<SemanticTagType> mOwnedTags;
    };

    AmbientContextSensingCluster(const Config & config);
    ~AmbientContextSensingCluster() = default;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType shutdownType) override;

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR SetAmbientContextTypeSupported(const std::vector<SemanticTagType> & ACTypeList);
    CHIP_ERROR AddDetection(const AmbientContextSensingType & sensedEvent);
    DataModel::ActionReturnStatus SetObjectCountConfig(const ObjectCountConfigType & objectCountConfig);
    CHIP_ERROR SetObjectCount(uint16_t objectCount);
    DataModel::ActionReturnStatus SetSimultaneousDetectionLimit(const uint16_t simultaneousDetectionLimit);
    DataModel::ActionReturnStatus SetHoldTime(uint16_t holdTime);
    uint16_t GetHoldTime() const { return mHoldTime; }
    void SetHoldTimeLimits(const AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type & holdTimeLimits);
    CHIP_ERROR SetPredictedActivity(const std::vector<PredictedActivityType> & predictedActivity);
    void TimerFired() override;

private:
    bool CompareAmbientContextSensed(const AmbientContextSensingType & sensedEvent, const AmbientContextSensingType & newEvent);
    CHIP_ERROR ReadAmbientContextTypeSupported(BitFlags<AmbientContextSensing::Feature> features, AttributeValueEncoder & encoder);
    CHIP_ERROR ReadAmbientContextType(AttributeValueEncoder & encoder);
    void UpdateDetectionAttributes();
    void UpdateEventTimeout();
    CHIP_ERROR CheckInputSupportedType(const std::vector<SemanticTagType> & ACTSupportedList);
    bool IsSupportedEvent(const AmbientContextSensingType & sensedEvent);
    void RemoveExpiredItems(std::list<AmbientContextSensed> & eventList, const System::Clock::Timestamp & now);
    System::Clock::Timestamp FindEarliestEndTimestamp(const std::list<AmbientContextSensed> & eventList);
    CHIP_ERROR CheckPredictedActivity(const std::vector<PredictedActivityType> & predictedActivityList);
    CHIP_ERROR ReadPredictedActivity(AttributeValueEncoder & encoder);

    BitMask<AmbientContextSensing::Feature> mFeatureMap;
    bool mHumanActivityDetected = false;
    bool mObjectIdentified      = false;
    bool mAudioContextDetected  = false;
    // Event is set while one of HADetected, ObjectIdentified, AudioContexted or ObjectCountReached is from false to true
    chip::EventNumber mEventNum[4] = {};

    std::vector<SemanticTagType> mAmbientContextTypeSupportedList;
    std::list<AmbientContextSensed> mAmbientContextTypeList;

    uint8_t mSimultaneousDetectionLimit = kDefaultSimultaneousDetectionLimit;
    bool mObjectCountReached = false;
    ObjectCountConfigType mObjectCountConfig = {
            .countingObject  = {
                .namespaceID = kNamespaceIdentifiedObject,
                .tag         = static_cast<uint8_t>(TagIdentifiedObject::kUnknown),
            },
            .objectCountThreshold = kDefaultCountThreshold,
        };
    uint16_t mObjectCount = 0;
    System::Clock::Timestamp mObjectCountEndTime;
    uint16_t mHoldTime = kDefaultHoldTimeDefault;
    AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type mHoldTimeLimits = {
            .holdTimeMin     = kDefaultHoldTimeMin,
            .holdTimeMax     = kDefaultHoldTimeMax,
            .holdTimeDefault = kDefaultHoldTimeDefault
        };
    TimerDelegate * mHoldTimeDelegate;

    std::vector<PredictActivity> mPredictedActivityList;
};

} // namespace chip::app::Clusters
