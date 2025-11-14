/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <lib/support/TimerDelegate.h>

namespace chip::app::Clusters {

class OccupancySensingCluster;

class OccupancySensingDelegate
{
public:
    virtual ~OccupancySensingDelegate() = default;

    /**
     * @brief Called when the occupancy state changes.
     * @param occupied The new occupancy state.
     */
    virtual void OnOccupancyChanged(bool occupied) = 0;

    /**
     * @brief Called when the hold time changes.
     * @param holdTime The new hold time.
     */
    virtual void OnHoldTimeChanged(uint16_t holdTime) = 0;
};

class OccupancySensingCluster : public DefaultServerCluster, public TimerContext
{
public:
    struct Config
    {
        Config(EndpointId endpointId) : mEndpointId(endpointId) {}

        Config & WithFeatures(OccupancySensing::Feature featureMap)
        {
            mFeatureMap = featureMap;
            return *this;
        }

        Config & WithHoldTime(uint16_t aHoldTime, const OccupancySensing::Structs::HoldTimeLimitsStruct::Type & aHoldTimeLimits,
                              TimerDelegate & aTimerDelegate)
        {
            mHasHoldTime    = true;
            mHoldTime       = aHoldTime;
            mHoldTimeLimits = aHoldTimeLimits;
            mTimerDelegate  = &aTimerDelegate;
            return *this;
        }

        Config & WithDelegate(OccupancySensingDelegate * delegate)
        {
            mDelegate = delegate;
            return *this;
        }

        EndpointId mEndpointId;
        BitMask<OccupancySensing::Feature> mFeatureMap                        = 0;
        bool mHasHoldTime                                                     = false;
        uint16_t mHoldTime                                                    = 10;
        OccupancySensing::Structs::HoldTimeLimitsStruct::Type mHoldTimeLimits = { .holdTimeMin     = 1,
                                                                                  .holdTimeMax     = 300,
                                                                                  .holdTimeDefault = 10 };
        TimerDelegate * mTimerDelegate                                        = nullptr;
        OccupancySensingDelegate * mDelegate                                  = nullptr;
    };

    OccupancySensingCluster(const Config & config);
    ~OccupancySensingCluster() = default;

    CHIP_ERROR Startup(ServerClusterContext & context) override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & aDecoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    void TimerFired() override;

    DataModel::ActionReturnStatus SetHoldTime(uint16_t holdTime);
    void SetHoldTimeLimits(const OccupancySensing::Structs::HoldTimeLimitsStruct::Type & holdTimeLimits);
    void SetOccupancy(bool occupied);
    bool IsOccupied() const;

    uint16_t GetHoldTime() const;
    const OccupancySensing::Structs::HoldTimeLimitsStruct::Type & GetHoldTimeLimits() const;

private:
    void DoSetOccupancy(bool occupied);

    TimerDelegate * mTimerDelegate;
    OccupancySensingDelegate * mDelegate;
    BitMask<OccupancySensing::Feature> mFeatureMap;
    bool mHasHoldTime = false;
    uint16_t mHoldTime;
    OccupancySensing::Structs::HoldTimeLimitsStruct::Type mHoldTimeLimits;
    BitMask<OccupancySensing::OccupancyBitmap> mOccupancy = 0;
    System::Clock::Timestamp mTimerStartedTimestamp       = System::Clock::Milliseconds64(0);
};

} // namespace chip::app::Clusters
