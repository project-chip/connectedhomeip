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

#include <app/clusters/water-heater-management-server/water-heater-management-server.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {
namespace Chef {

const BitMask<WaterHeaterHeatSourceBitmap> kDefaultHeatDemand = 0;
const uint16_t kDefaultTankVolumeLiters                       = 150;
const Percent kDefaultTankPercentage                          = 50;

class ChefDelegate : public Delegate
{
public:
    ChefDelegate() :
        Delegate(), mHeatDemand(kDefaultHeatDemand), mTankVolume(kDefaultTankVolumeLiters), mTankPercentage(kDefaultTankPercentage)
    {}

    Protocols::InteractionModel::Status HandleBoost(uint32_t durationSeconds, Optional<bool> oneShot, Optional<bool> emergencyBoost,
                                                    Optional<int16_t> temporarySetpoint, Optional<Percent> targetPercentage,
                                                    Optional<Percent> targetReheat) override;
    Protocols::InteractionModel::Status HandleCancelBoost() override;

    BitMask<WaterHeaterHeatSourceBitmap> GetHeaterTypes() override { return mHeaterTypes; }
    BitMask<WaterHeaterHeatSourceBitmap> GetHeatDemand() override { return mHeatDemand; }
    uint16_t GetTankVolume() override { return mTankVolume; }
    Energy_mWh GetEstimatedHeatRequired() override;
    Percent GetTankPercentage() override { return mTankPercentage; }
    BoostStateEnum GetBoostState() override { return mBoostState; }
    void TimerTick();

private:
    BitMask<WaterHeaterHeatSourceBitmap> mHeaterTypes =
        to_underlying(WaterHeaterHeatSourceBitmap::kImmersionElement1) | to_underlying(WaterHeaterHeatSourceBitmap::kHeatPump);
    BitMask<WaterHeaterHeatSourceBitmap> mHeatDemand;
    uint16_t mTankVolume;
    Percent mTankPercentage;
    BoostStateEnum mBoostState = BoostStateEnum::kInactive;

    Optional<Structs::WaterHeaterBoostInfoStruct::Type> mBoostInfo;
};

} // namespace Chef
} // namespace WaterHeaterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
