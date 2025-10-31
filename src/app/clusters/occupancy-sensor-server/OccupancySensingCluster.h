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

namespace chip::app::Clusters {

constexpr uint32_t kDefaultFeatureMap = 0;

class OccupancySensingCluster : public DefaultServerCluster
{
public:
    struct Config
    {
        Config(EndpointId aEndpointId) : endpointId(aEndpointId) {}

        Config & WithFeature(BitMask<OccupancySensing::Feature> aFeature)
        {
            this->feature = aFeature;
            return *this;
        }

        Config & WithHoldTime(uint16_t aHoldTime)
        {
            this->holdTime = aHoldTime;
            return *this;
        }

        Config & WithHoldTimeLimits(const OccupancySensing::Structs::HoldTimeLimitsStruct::Type & aHoldTimeLimits)
        {
            this->holdTimeLimits = aHoldTimeLimits;
            return *this;
        }

        Config & WithOccupancySensorTypeBitmap(BitMask<OccupancySensing::OccupancySensorTypeBitmap> aOccupancySensorTypeBitmap)
        {
            this->occupancySensorTypeBitmap = aOccupancySensorTypeBitmap;
            return *this;
        }

        Config & WithPIRUnoccupiedToOccupiedDelay(uint16_t aDelay)
        {
            this->pirUnoccupiedToOccupiedDelay = aDelay;
            return *this;
        }

        Config & WithPIRUnoccupiedToOccupiedThreshold(uint8_t aThreshold)
        {
            this->pirUnoccupiedToOccupiedThreshold = aThreshold;
            return *this;
        }

        Config & WithUltrasonicUnoccupiedToOccupiedDelay(uint16_t aDelay)
        {
            this->ultrasonicUnoccupiedToOccupiedDelay = aDelay;
            return *this;
        }

        Config & WithUltrasonicUnoccupiedToOccupiedThreshold(uint8_t aThreshold)
        {
            this->ultrasonicUnoccupiedToOccupiedThreshold = aThreshold;
            return *this;
        }

        Config & WithPhysicalContactUnoccupiedToOccupiedDelay(uint16_t aDelay)
        {
            this->physicalContactUnoccupiedToOccupiedDelay = aDelay;
            return *this;
        }

        Config & WithPhysicalContactUnoccupiedToOccupiedThreshold(uint8_t aThreshold)
        {
            this->physicalContactUnoccupiedToOccupiedThreshold = aThreshold;
            return *this;
        }

        EndpointId endpointId;
        BitMask<OccupancySensing::Feature> feature = 0;
        uint16_t holdTime = 1;
        OccupancySensing::Structs::HoldTimeLimitsStruct::Type holdTimeLimits = { .holdTimeMin = 0, .holdTimeMax = 0, .holdTimeDefault = 0 };
        BitMask<OccupancySensing::OccupancySensorTypeBitmap> occupancySensorTypeBitmap = OccupancySensing::OccupancySensorTypeBitmap::kPir;
        uint16_t pirUnoccupiedToOccupiedDelay = 0;
        uint8_t pirUnoccupiedToOccupiedThreshold = 1;
        uint16_t ultrasonicUnoccupiedToOccupiedDelay = 0;
        uint8_t ultrasonicUnoccupiedToOccupiedThreshold = 1;
        uint16_t physicalContactUnoccupiedToOccupiedDelay = 0;
        uint8_t physicalContactUnoccupiedToOccupiedThreshold = 1;
    };

    OccupancySensingCluster(const Config & config);

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

private:
    BitMask<OccupancySensing::Feature> mFeature;
    uint16_t mHoldTime;
    OccupancySensing::Structs::HoldTimeLimitsStruct::Type mHoldTimeLimits;
    BitMask<OccupancySensing::OccupancyBitmap> mOccupancy = 0;
    BitMask<OccupancySensing::OccupancySensorTypeBitmap> mOccupancySensorTypeBitmap;
    uint16_t mPIRUnoccupiedToOccupiedDelay;
    uint8_t mPIRUnoccupiedToOccupiedThreshold;
    uint16_t mUltrasonicUnoccupiedToOccupiedDelay;
    uint8_t mUltrasonicUnoccupiedToOccupiedThreshold;
    uint16_t mPhysicalContactUnoccupiedToOccupiedDelay;
    uint8_t mPhysicalContactUnoccupiedToOccupiedThreshold;
};

} // namespace chip::app::Clusters