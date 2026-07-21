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

#include "AmbientContextSensingDelegate.h"
#include "ambient-context-sensing-namespace.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/Span.h>
#include <platform/DefaultTimerDelegate.h>

namespace chip::app::Clusters {

class AmbientContextSensingCluster : public DefaultServerCluster, public TimerContext
{
public:
    struct Config
    {
        Config(TimerDelegate & timerDelegate) : mHoldTimeDelegate(timerDelegate) {}

        Config & WithFeatures(BitMask<AmbientContextSensing::Feature> featureMap)
        {
            mFeatureMap = featureMap;
            return *this;
        }

        Config & WithOptionalAttributes(uint32_t bits)
        {
            mOptionalAttributeBits = bits;
            return *this;
        }

        Config & WithHoldTime(uint16_t aHoldTime,
                              const AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type & aHoldTimeLimits)
        {
            mHoldTime       = aHoldTime;
            mHoldTimeLimits = aHoldTimeLimits;
            return *this;
        }

        BitMask<AmbientContextSensing::Feature> mFeatureMap = 0;
        uint32_t mOptionalAttributeBits                     = 0;
        AmbientContextSensing::ObjectCountConfigType mObjectCountConfig            = {
            .countingObject  = {
                .namespaceID = kNamespaceIdentifiedObject,
                .tag         = static_cast<uint8_t>(TagIdentifiedObject::kUnknown),
            },
            .objectCountThreshold = AmbientContextSensing::kDefaultCountThreshold,
            };
        uint16_t mHoldTime                                                         = AmbientContextSensing::kDefaultHoldTimeDefault;
        AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type mHoldTimeLimits = {
            .holdTimeMin     = AmbientContextSensing::kDefaultHoldTimeMin,
            .holdTimeMax     = AmbientContextSensing::kDefaultHoldTimeMax,
            .holdTimeDefault = AmbientContextSensing::kDefaultHoldTimeDefault
        };
        TimerDelegate & mHoldTimeDelegate;
    };

    using OptionalAttributeSet = chip::app::OptionalAttributeSet<AmbientContextSensing::Attributes::ObjectCount::Id>;

    AmbientContextSensingCluster(EndpointId endpointId, const Config & config);
    ~AmbientContextSensingCluster() = default;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown(ClusterShutdownType shutdownType) override;
    void SetDelegate(AmbientContextSensing::AmbientContextSensingDelegate * pDelegate) { mACSDelegate = pDelegate; }

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    const BitMask<AmbientContextSensing::Feature> & GetFeatures() const { return mFeatureMap; }

    bool GetHumanActivityDetected() { return mHumanActivityDetected; }
    bool GetObjectIdentified() { return mObjectIdentified; }
    bool GetAudioContextDetected() { return mAudioContextDetected; }
    bool GetObjectCountThresholdReached() { return mObjectCountThresholdReached; }
    CHIP_ERROR SetAmbientContextTypeSupported(const Span<AmbientContextSensing::SemanticTagType> & ACTypeList);
    CHIP_ERROR AddDetection(const AmbientContextSensing::AmbientContextSensingType & sensedEvent);
    DataModel::ActionReturnStatus SetObjectCountConfig(const AmbientContextSensing::ObjectCountConfigType & objectCountConfig);
    AmbientContextSensing::ObjectCountConfigType GetObjectCountConfig() { return mObjectCountConfig; }
    CHIP_ERROR SetObjectCount(uint16_t objectCount);
    uint16_t GetObjectCount() { return mObjectCount; }
    DataModel::ActionReturnStatus SetSimultaneousDetectionLimit(const uint8_t simultaneousDetectionLimit);
    uint8_t GetSimultaneousDetectionLimit() { return mSimultaneousDetectionLimit; }
    DataModel::ActionReturnStatus SetHoldTime(uint16_t holdTime);
    uint16_t GetHoldTime() const { return mHoldTime; }
    void SetHoldTimeLimits(const AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type & holdTimeLimits);
    AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type GetHoldTimeLimits() { return mHoldTimeLimits; }
    CHIP_ERROR SetPredictedActivity(const Span<AmbientContextSensing::PredictedActivityType> & predictedActivity);
    void TimerFired() override;

private:
    bool CompareAmbientContextSensed(const AmbientContextSensing::AmbientContextSensingType & sensedEvent,
                                     const AmbientContextSensing::AmbientContextSensingType & newEvent);
    CHIP_ERROR ReadAmbientContextTypeSupported(AttributeValueEncoder & encoder);
    CHIP_ERROR ReadAmbientContextType(AttributeValueEncoder & encoder);
    void SendDetectStartEvent(const AmbientContextSensing::AmbientContextSensed & ACSItem);
    void SendDetectStartEvent(const bool objectCountReached, const uint16_t objectCount);
    void SendDetectEndEvent(const uint64_t eventStartTimePos, const uint64_t eventStartTimeSys);
    void UpdateDetectionAttributes();
    void UpdateEventTimeout();
    CHIP_ERROR CheckInputSupportedType(const Span<AmbientContextSensing::SemanticTagType> & ACTSupportedList);
    bool IsSupportedEvent(const AmbientContextSensing::AmbientContextSensingType & sensedEvent);
    void RemoveExpiredItems(IntrusiveList<AmbientContextSensing::AmbientContextSensed> & eventList, uint8_t & listSize,
                            const System::Clock::Timestamp & now);

    System::Clock::Timestamp FindEarliestEndTimestamp();
    CHIP_ERROR CheckPredictedActivity(const Span<AmbientContextSensing::PredictedActivityType> & predictedActivityList);
    CHIP_ERROR ReadPredictedActivity(AttributeValueEncoder & encoder);

    const BitMask<AmbientContextSensing::Feature> mFeatureMap;
    const OptionalAttributeSet mOptionalAttributeSet;
    bool mHumanActivityDetected                                         = false;
    bool mObjectIdentified                                              = false;
    bool mAudioContextDetected                                          = false;
    AmbientContextSensing::AmbientContextSensingDelegate * mACSDelegate = nullptr;

    Span<AmbientContextSensing::SemanticTagType> mAmbientContextTypeSupportedList;

    IntrusiveList<AmbientContextSensing::AmbientContextSensed> mAmbientContextTypeList;
    uint8_t mAmbientContextTypeListSize = 0;

    Span<AmbientContextSensing::PredictActivity> mPredictedActivityList;

    uint8_t mSimultaneousDetectionLimit = AmbientContextSensing::kDefaultSimultaneousDetectionLimit;
    bool mObjectCountThresholdReached   = false;
    AmbientContextSensing::ObjectCountConfigType mObjectCountConfig = {
            .countingObject  = {
                .namespaceID = kNamespaceIdentifiedObject,
                .tag         = static_cast<uint8_t>(TagIdentifiedObject::kUnknown),
            },
            .objectCountThreshold = AmbientContextSensing::kDefaultCountThreshold,
        };
    uint16_t mObjectCount = 0;
    System::Clock::Timestamp mObjectCountStartTime;
    uint64_t mObjectCountStartEpoch;
    System::Clock::Timestamp mObjectCountEndTime;
    uint16_t mHoldTime                                                         = AmbientContextSensing::kDefaultHoldTimeDefault;
    AmbientContextSensing::Structs::HoldTimeLimitsStruct::Type mHoldTimeLimits = {
        .holdTimeMin     = AmbientContextSensing::kDefaultHoldTimeMin,
        .holdTimeMax     = AmbientContextSensing::kDefaultHoldTimeMax,
        .holdTimeDefault = AmbientContextSensing::kDefaultHoldTimeDefault
    };
    TimerDelegate & mHoldTimeDelegate;
};

} // namespace chip::app::Clusters
