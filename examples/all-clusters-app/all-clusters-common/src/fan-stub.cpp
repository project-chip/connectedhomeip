/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/clusters/fan-control-server/fan-control.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FanControl;
using namespace chip::app::Clusters::FanControl::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {

class FanManager : public Delegate
{
public:
    CHIP_ERROR Init();
    CHIP_ERROR HandleFanModeChange(FanModeEnum newMode);
    CHIP_ERROR HandleFanModeOff(DataModel::Nullable<Percent> newPercentSetting, DataModel::Nullable<uint8_t> newSpeedSetting);
    CHIP_ERROR HandleFanModeAuto(DataModel::Nullable<Percent> newPercentSetting, DataModel::Nullable<uint8_t> newSpeedSetting);
    CHIP_ERROR HandlePercentSpeedSettingChange(DataModel::Nullable<Percent> newPercentSetting,
                                               DataModel::Nullable<uint8_t> newSpeedSetting);
    CHIP_ERROR HandleRockSettingChange(BitMask<RockBitmap> newRockSetting);
    CHIP_ERROR HandleWindSettingChange(BitMask<WindBitmap> newWindSetting);
    CHIP_ERROR HandleAirflowDirectionChange(AirflowDirectionEnum newAirflowDirection);
    Status HandleStep(StepDirectionEnum stepDirection, chip::Optional<bool> pWrap, chip::Optional<bool> pLowestOff);

private:
    Status doUpdateSpeedSetting(uint8_t newSpeedSetting);
};

FanControl::Instance * gFanControlInstance;
FanManager gFanManager;

CHIP_ERROR FanManager::Init()
{
    ChipLogProgress(NotSpecified, "FanManager::Init()");
    return CHIP_NO_ERROR;
}

CHIP_ERROR FanManager::HandleFanModeChange(FanModeEnum newMode)
{
    Status status = Status::Success;
    ChipLogProgress(NotSpecified, "FanManager::HandleFanModeChange()");
    switch (newMode)
    {
    case FanModeEnum::kUnknownEnumValue:
    case FanModeEnum::kOff:
        status = doUpdateSpeedSetting(0);
        break;
    case FanModeEnum::kLow:
        status = doUpdateSpeedSetting(1);
        break;
    case FanModeEnum::kMedium:
        status = doUpdateSpeedSetting(5);
        break;
    case FanModeEnum::kOn:
    case FanModeEnum::kHigh:
        status = doUpdateSpeedSetting(10);
        break;
    case FanModeEnum::kSmart:
    case FanModeEnum::kAuto:
        status = doUpdateSpeedSetting(5);
        break;
    }
    return StatusIB(status).ToChipError();
}

CHIP_ERROR FanManager::HandleFanModeOff(DataModel::Nullable<Percent> newPercentSetting,
                                        DataModel::Nullable<uint8_t> newSpeedSetting)
{
    ChipLogProgress(NotSpecified, "FanManager::HandleFanModeOff()");
    doUpdateSpeedSetting(newSpeedSetting.Value());
    return CHIP_NO_ERROR;
}

CHIP_ERROR FanManager::HandleFanModeAuto(DataModel::Nullable<Percent> newPercentSetting,
                                         DataModel::Nullable<uint8_t> newSpeedSetting)
{
    ChipLogProgress(NotSpecified, "FanManager::HandleFanModeAuto()");
    doUpdateSpeedSetting(5);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FanManager::HandlePercentSpeedSettingChange(DataModel::Nullable<Percent> newPercentSetting,
                                                       DataModel::Nullable<uint8_t> newSpeedSetting)
{
    ChipLogProgress(NotSpecified, "FanManager::HandlePercentSpeedSettingChange()");
    UpdateClusterPercentageAndSpeedCurrent(newPercentSetting.Value());
    return CHIP_NO_ERROR;
}

CHIP_ERROR FanManager::HandleRockSettingChange(BitMask<RockBitmap> newRockSetting)
{
    ChipLogProgress(NotSpecified, "FanManager::HandleRockSettingChange()");
    return CHIP_NO_ERROR;
}

CHIP_ERROR FanManager::HandleWindSettingChange(BitMask<WindBitmap> newWindSetting)
{
    ChipLogProgress(NotSpecified, "FanManager::HandleWindSettingChange()");
    return CHIP_NO_ERROR;
}

CHIP_ERROR FanManager::HandleAirflowDirectionChange(AirflowDirectionEnum newAirflowDirection)
{
    ChipLogProgress(NotSpecified, "FanManager::HandleAirflowDirectionChange()");
    return CHIP_NO_ERROR;
}

Status FanManager::HandleStep(StepDirectionEnum stepDirection, chip::Optional<bool> pWrap, chip::Optional<bool> pLowestOff)
{
    // Example implementation of the possible effect of the application specific Step Command
    ChipLogProgress(NotSpecified, "FanManager::HandleStep()");
    Status status  = Status::Success;
    bool wrap      = false;
    bool lowestOff = false;

    if (pWrap.HasValue())
    {
        wrap = pWrap.Value();
    }

    if (pLowestOff.HasValue())
    {
        lowestOff = pLowestOff.Value();
    }

    uint8_t speedMax                          = GetClusterSpeedMax();
    DataModel::Nullable<uint8_t> speedSetting = GetClusterSpeedSetting();

    uint8_t newSpeedSetting;

    if ((speedSetting.Value() > 1) && (speedSetting.Value() < speedMax))
    {
        if (stepDirection == StepDirectionEnum::kIncrease)
        {
            newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() + 1);
            status          = doUpdateSpeedSetting(newSpeedSetting);
        }
        else if (stepDirection == StepDirectionEnum::kDecrease)
        {
            newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() - 1);
            status          = doUpdateSpeedSetting(newSpeedSetting);
        }
        else
        {
            status = Status::InvalidCommand;
        }
    }
    else if (speedSetting.Value() == 1)
    {
        if (stepDirection == StepDirectionEnum::kIncrease)
        {
            newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() + 1);
            status          = doUpdateSpeedSetting(newSpeedSetting);
        }
        else if (stepDirection == StepDirectionEnum::kDecrease)
        {
            if (lowestOff)
            {
                newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() - 1);
                status          = doUpdateSpeedSetting(newSpeedSetting);
            }
            else if (wrap)
            {
                newSpeedSetting = speedMax;
                status          = doUpdateSpeedSetting(newSpeedSetting);
            }
        }
        else
        {
            status = Status::InvalidCommand;
        }
    }
    else if (speedSetting.Value() == 0)
    {
        if (stepDirection == StepDirectionEnum::kIncrease)
        {
            newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() + 1);
            status          = doUpdateSpeedSetting(newSpeedSetting);
        }
        else if (stepDirection == StepDirectionEnum::kDecrease)
        {
            if (wrap)
            {
                newSpeedSetting = speedMax;
                status          = doUpdateSpeedSetting(newSpeedSetting);
            }
        }
        else
        {
            status = Status::InvalidCommand;
        }
    }
    else if (speedSetting.Value() == speedMax)
    {
        if (stepDirection == StepDirectionEnum::kIncrease)
        {
            if (wrap)
            {
                newSpeedSetting = lowestOff ? 0 : 1;
                status          = doUpdateSpeedSetting(newSpeedSetting);
            }
        }
        else if (stepDirection == StepDirectionEnum::kDecrease)
        {
            newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() - 1);
            status          = doUpdateSpeedSetting(newSpeedSetting);
        }
        else
        {
            status = Status::InvalidCommand;
        }
    }
    return status;
}

Status FanManager::doUpdateSpeedSetting(uint8_t newSpeedSetting)
{
    Status status = UpdateClusterSpeedAndPercentageSetting((DataModel::Nullable<uint8_t>) newSpeedSetting);
    HandlePercentSpeedSettingChange(GetClusterPercentSetting(), GetClusterSpeedSetting());

    return status;
}

} // anonymous namespace

void emberAfFanControlClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(NotSpecified, "FanControl Cluster init: endpoint %" PRIx16, endpoint);

    // Maximum speed is 10
    uint8_t speedMax = 10;

    // Support for Rocking Left/Right, Up/Down, and Round
    BitMask<RockBitmap> rockSupport;
    rockSupport.Set(RockBitmap::kRockLeftRight);
    rockSupport.Set(RockBitmap::kRockUpDown);
    rockSupport.Set(RockBitmap::kRockRound);

    // Support for Sleep and Natural Wind
    BitMask<WindBitmap> windSupport;
    windSupport.Set(WindBitmap::kSleepWind);
    windSupport.Set(WindBitmap::kNaturalWind);

    // All features are on
    uint32_t featureMap = 0b111111;

    gFanControlInstance = new Instance(endpoint, FanModeSequenceEnum::kOffLowMedHighAuto, speedMax, rockSupport, windSupport,
                                       featureMap, &gFanManager);
}
