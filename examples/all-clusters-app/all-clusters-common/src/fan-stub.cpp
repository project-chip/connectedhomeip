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
using namespace chip::app::Clusters::FanControl::Attributes;

namespace {

class FanManager : public FanControl::Delegate
{
public:
    CHIP_ERROR Init();
    CHIP_ERROR HandleFanModeChange(FanModeEnum newMode);
    CHIP_ERROR HandleFanModeOff(DataModel::Nullable<Percent> newPercentSetting, DataModel::Nullable<uint8_t> newSpeedSetting);
    CHIP_ERROR HandleFanModeAuto(DataModel::Nullable<Percent> newPercentSetting, DataModel::Nullable<uint8_t> newSpeedSetting);
    CHIP_ERROR HandlePercentSpeedSettingChange(DataModel::Nullable<Percent> newPercentSetting,
                                               DataModel::Nullable<uint8_t> newSpeedSetting);
    CHIP_ERROR HandleRockSettingChange(RockBitmap newRockSetting);
    CHIP_ERROR HandleWindSettingChange(WindBitmap newWindSetting);
    CHIP_ERROR HandleAirflowDirectionChange(AirflowDirectionEnum newAirflowDirection);
    Status HandleStep(StepDirectionEnum stepDirection, bool wrap, bool lowestOff);

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
    ChipLogProgress(NotSpecified, "FanManager::HandleFanModeChange()");
    switch (newMode)
    {
    case FanModeEnum::kLow:
        return doUpdateSpeedSetting(GetClusterSpeedMax() / 3);
    case FanModeEnum::kMedium:
        return doUpdateSpeedSetting(GetClusterSpeedMax() / 2);
    case FanModeEnum::kHigh:
        return doUpdateSpeedSetting(GetClusterSpeedMax());
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR FanManager::HandleFanModeOff(DataModel::Nullable<Percent> newPercentSetting,
                                        DataModel::Nullable<uint8_t> newSpeedSetting)
{
    ChipLogProgress(NotSpecified, "FanManager::HandleFanModeOff()");
    UpdateClusterPercentageAndSpeedCurrent(newPercentSetting.Value());
    return CHIP_NO_ERROR;
}

CHIP_ERROR FanManager::HandleFanModeAuto(DataModel::Nullable<Percent> newPercentSetting,
                                         DataModel::Nullable<uint8_t> newSpeedSetting)
{
    ChipLogProgress(NotSpecified, "FanManager::HandleFanModeAuto()");
    UpdateClusterPercentageAndSpeedCurrent((Percent) 50);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FanManager::HandlePercentSpeedSettingChange(DataModel::Nullable<Percent> newPercentSetting,
                                                       DataModel::Nullable<uint8_t> newSpeedSetting)
{
    ChipLogProgress(NotSpecified, "FanManager::HandlePercentSpeedChange()");
    UpdateClusterPercentageAndSpeedCurrent(newPercentSetting.Value());
    return CHIP_NO_ERROR;
}

CHIP_ERROR FanManager::HandleRockSettingChange(RockBitmap newRockSetting)
{
    ChipLogProgress(NotSpecified, "FanManager::HandleRockSettingChange()");
    return CHIP_NO_ERROR;
}

CHIP_ERROR FanManager::HandleWindSettingChange(WindBitmap newWindSetting)
{
    ChipLogProgress(NotSpecified, "FanManager::HandleWindSettingChange()");
    return CHIP_NO_ERROR;
}

CHIP_ERROR FanManager::HandleAirflowDirectionChange(AirflowDirectionEnum newAirflowDirection)
{
    ChipLogProgress(NotSpecified, "FanManager::HandleAirflowDirectionChange()");
    return CHIP_NO_ERROR;
}

Status FanManager::HandleStep(StepDirectionEnum stepDirection, bool wrap, bool lowestOff)
{
    // Example implementation of the possible effect of the application specific Step Command
    ChipLogProgress(NotSpecified, "FanManager::HandleStep()");
    Status status = Status::Success;

    uint8_t speedMax                          = GetClusterSpeedMax();
    DataModel::Nullable<uint8_t> speedSetting = GetClusterSpeedSetting();

    uint8_t newSpeedSetting;

    if ((speedSetting.Value() > 1) && (speedSetting.Value() < speedMax))
    {
        if (direction == DirectionEnum::kIncrease)
        {
            newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() + 1);
            status          = doUpdateSpeedSetting(newSpeedSetting);
        }
        else if (direction == DirectionEnum::kDecrease)
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
        if (direction == DirectionEnum::kIncrease)
        {
            newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() + 1);
            status          = doUpdateSpeedSetting(newSpeedSetting);
        }
        else if (direction == DirectionEnum::kDecrease)
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
        if (direction == DirectionEnum::kIncrease)
        {
            newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() + 1);
            status          = doUpdateSpeedSetting(newSpeedSetting);
        }
        else if (direction == DirectionEnum::kDecrease)
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
        if (direction == DirectionEnum::kIncrease)
        {
            if (wrap)
            {
                newSpeedSetting = lowestOff ? 0 : 1;
                status          = doUpdateSpeedSetting(newSpeedSetting);
            }
        }
        else if (direction == DirectionEnum::kDecrease)
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
    Status status = UpdateClusterPercentageAndSpeedSetting(newSpeedSetting);
    HandlePercentSpeedSettingChange(GetClusterPercentSetting(), GetClusterSpeedSetting());

    return status
}
} // anonymous namespace

void emberAfFanControlClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(NotSpecified, "FanControl Cluster init: endpoint %" PRIx16, endpoint);
    uint8_t speedMax = 10;

    gFanControlInstance = new FanControl::Instance(endpoint, FanModeSequenceEnum::kOffLowMedHighAuto, 10, 7, 3, &gFanManager);
}
