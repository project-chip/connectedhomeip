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
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/clusters/fan-control-server/fan-control-server.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <functional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FanControl;
using namespace chip::app::Clusters::FanControl::Attributes;
using Protocols::InteractionModel::Status;

namespace {
class ChefFanControlManager : public Delegate
{
public:
    ChefFanControlManager(EndpointId aEndpointId) : Delegate(aEndpointId) {}

    void Init();
    void HandleFanControlAttributeChange(AttributeId attributeId, uint8_t type, uint16_t size, uint8_t * value);
    Status HandleStep(StepDirectionEnum aDirection, bool aWrap, bool aLowestOff) override;
    DataModel::Nullable<uint8_t> GetSpeedSetting();
    DataModel::Nullable<Percent> GetPercentSetting();

private:
    uint8_t mPercentCurrent;
    uint8_t mSpeedCurrent;

    // Fan Mode Limits
    static constexpr int FAN_MODE_LOW_LOWER_BOUND    = 1;
    static constexpr int FAN_MODE_LOW_UPPER_BOUND    = 3;
    static constexpr int FAN_MODE_MEDIUM_LOWER_BOUND = 4;
    static constexpr int FAN_MODE_MEDIUM_UPPER_BOUND = 7;
    static constexpr int FAN_MODE_HIGH_LOWER_BOUND   = 8;
    static constexpr int FAN_MODE_HIGH_UPPER_BOUND   = 10;

    void FanModeWriteCallback(FanControl::FanModeEnum aNewFanMode);
    void SpeedSettingWriteCallback(uint8_t aNewSpeedSetting);
    void PercentSettingWriteCallback(uint8_t aNewPercentSetting);
    void SetSpeedSetting(DataModel::Nullable<uint8_t> aNewSpeedSetting);
};

static std::unique_ptr<ChefFanControlManager> mFanControlManager;

Status ChefFanControlManager::HandleStep(StepDirectionEnum aDirection, bool aWrap, bool aLowestOff)
{
    ChipLogProgress(NotSpecified, "ChefFanControlManager::HandleStep aDirection %d, aWrap %d, aLowestOff %d",
                    to_underlying(aDirection), aWrap, aLowestOff);

    VerifyOrReturnError(aDirection != StepDirectionEnum::kUnknownEnumValue, Status::InvalidCommand);

    Protocols::InteractionModel::Status status;

    uint8_t speedMax;
    status = SpeedMax::Get(mEndpoint, &speedMax);
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, Status::InvalidCommand);

    uint8_t speedCurrent;
    status = SpeedCurrent::Get(mEndpoint, &speedCurrent);
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, Status::InvalidCommand);

    DataModel::Nullable<uint8_t> speedSetting;
    status = SpeedSetting::Get(mEndpoint, speedSetting);
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, Status::InvalidCommand);

    uint8_t newSpeedSetting    = speedSetting.ValueOr(0);
    uint8_t speedValue         = speedSetting.ValueOr(speedCurrent);
    const uint8_t kLowestSpeed = aLowestOff ? 0 : 1;

    if (aDirection == StepDirectionEnum::kIncrease)
    {
        newSpeedSetting = std::invoke([&]() -> uint8_t {
            VerifyOrReturnValue(speedValue < speedMax, (aWrap ? kLowestSpeed : speedMax));
            return static_cast<uint8_t>(speedValue + 1);
        });
    }
    else if (aDirection == StepDirectionEnum::kDecrease)
    {
        newSpeedSetting = std::invoke([&]() -> uint8_t {
            VerifyOrReturnValue(speedValue > kLowestSpeed, aWrap ? speedMax : kLowestSpeed);
            return static_cast<uint8_t>(speedValue - 1);
        });
    }

    return SpeedSetting::Set(mEndpoint, newSpeedSetting);
}

void ChefFanControlManager::HandleFanControlAttributeChange(AttributeId attributeId, uint8_t type, uint16_t size, uint8_t * value)
{
    ChipLogProgress(NotSpecified, "ChefFanControlManager::HandleFanControlAttributeChange");
    switch (attributeId)
    {
    case FanControl::Attributes::PercentSetting::Id: {
        ChipLogProgress(NotSpecified, "ChefFanControlManager::HandleFanControlAttributeChange  PercentSetting");
        DataModel::Nullable<Percent> percentSetting = static_cast<DataModel::Nullable<uint8_t>>(*value);
        if (percentSetting.IsNull())
        {
            PercentSettingWriteCallback(0);
        }
        else
        {
            PercentSettingWriteCallback(percentSetting.Value());
        }
        break;
    }

    case FanControl::Attributes::SpeedSetting::Id: {
        ChipLogProgress(NotSpecified, "ChefFanControlManager::HandleFanControlAttributeChange  SpeedSetting");

        DataModel::Nullable<uint8_t> speedSetting = static_cast<DataModel::Nullable<uint8_t>>(*value);
        if (speedSetting.IsNull())
        {
            SpeedSettingWriteCallback(0);
        }
        else
        {
            SpeedSettingWriteCallback(speedSetting.Value());
        }
        break;
    }

    case FanControl::Attributes::FanMode::Id: {
        ChipLogProgress(NotSpecified, "ChefFanControlManager::HandleFanControlAttributeChange  FanMode");

        FanControl::FanModeEnum fanMode = static_cast<FanControl::FanModeEnum>(*value);
        FanModeWriteCallback(fanMode);
        break;
    }

    default: {
        break;
    }
    }
}

void ChefFanControlManager::PercentSettingWriteCallback(uint8_t aNewPercentSetting)
{
    if (aNewPercentSetting != mPercentCurrent)
    {
        ChipLogDetail(NotSpecified, "ChefFanControlManager::PercentSettingWriteCallback: %d", aNewPercentSetting);
        mPercentCurrent = aNewPercentSetting;
        Status status   = FanControl::Attributes::PercentCurrent::Set(mEndpoint, mPercentCurrent);
        if (status != Status::Success)
        {
            ChipLogError(NotSpecified,
                         "ChefFanControlManager::PercentSettingWriteCallback: failed to set PercentCurrent attribute: %d",
                         to_underlying(status));
            return;
        }
    }
}

void ChefFanControlManager::SpeedSettingWriteCallback(uint8_t aNewSpeedSetting)
{
    if (aNewSpeedSetting != mSpeedCurrent)
    {
        ChipLogDetail(NotSpecified, "ChefFanControlManager::SpeedSettingWriteCallback: %d", aNewSpeedSetting);
        mSpeedCurrent = aNewSpeedSetting;
        Status status = FanControl::Attributes::SpeedCurrent::Set(mEndpoint, aNewSpeedSetting);
        if (status != Status::Success)
        {
            ChipLogError(NotSpecified, "ChefFanControlManager::SpeedSettingWriteCallback: failed to set SpeedCurrent attribute: %d",
                         to_underlying(status));
            return;
        }

        // Determine if the speed change should also change the fan mode
        if (mSpeedCurrent == 0)
        {
            FanControl::Attributes::FanMode::Set(mEndpoint, FanControl::FanModeEnum::kOff);
        }
        else if (mSpeedCurrent <= FAN_MODE_LOW_UPPER_BOUND)
        {
            FanControl::Attributes::FanMode::Set(mEndpoint, FanControl::FanModeEnum::kLow);
        }
        else if (mSpeedCurrent <= FAN_MODE_MEDIUM_UPPER_BOUND)
        {
            FanControl::Attributes::FanMode::Set(mEndpoint, FanControl::FanModeEnum::kMedium);
        }
        else if (mSpeedCurrent <= FAN_MODE_HIGH_UPPER_BOUND)
        {
            FanControl::Attributes::FanMode::Set(mEndpoint, FanControl::FanModeEnum::kHigh);
        }
    }
}

void ChefFanControlManager::FanModeWriteCallback(FanControl::FanModeEnum aNewFanMode)
{
    ChipLogDetail(NotSpecified, "ChefFanControlManager::FanModeWriteCallback: %d", (uint8_t) aNewFanMode);
    switch (aNewFanMode)
    {
    case FanControl::FanModeEnum::kOff: {
        if (mSpeedCurrent != 0)
        {
            DataModel::Nullable<uint8_t> speedSetting(0);
            SetSpeedSetting(speedSetting);
        }
        break;
    }
    case FanControl::FanModeEnum::kLow: {
        if (mSpeedCurrent < FAN_MODE_LOW_LOWER_BOUND || mSpeedCurrent > FAN_MODE_LOW_UPPER_BOUND)
        {
            DataModel::Nullable<uint8_t> speedSetting(FAN_MODE_LOW_LOWER_BOUND);
            SetSpeedSetting(speedSetting);
        }
        break;
    }
    case FanControl::FanModeEnum::kMedium: {
        if (mSpeedCurrent < FAN_MODE_MEDIUM_LOWER_BOUND || mSpeedCurrent > FAN_MODE_MEDIUM_UPPER_BOUND)
        {
            DataModel::Nullable<uint8_t> speedSetting(FAN_MODE_MEDIUM_LOWER_BOUND);
            SetSpeedSetting(speedSetting);
        }
        break;
    }
    case FanControl::FanModeEnum::kOn:
    case FanControl::FanModeEnum::kHigh: {
        if (mSpeedCurrent < FAN_MODE_HIGH_LOWER_BOUND || mSpeedCurrent > FAN_MODE_HIGH_UPPER_BOUND)
        {
            DataModel::Nullable<uint8_t> speedSetting(FAN_MODE_HIGH_LOWER_BOUND);
            SetSpeedSetting(speedSetting);
        }
        break;
    }
    case FanControl::FanModeEnum::kSmart:
    case FanControl::FanModeEnum::kAuto: {
        ChipLogProgress(NotSpecified, "ChefFanControlManager::FanModeWriteCallback: Auto");
        break;
    }
    case FanControl::FanModeEnum::kUnknownEnumValue: {
        ChipLogProgress(NotSpecified, "ChefFanControlManager::FanModeWriteCallback: Unknown");
        break;
    }
    }
}

void ChefFanControlManager::SetSpeedSetting(DataModel::Nullable<uint8_t> aNewSpeedSetting)
{
    if (aNewSpeedSetting.IsNull())
    {
        ChipLogError(NotSpecified, "ChefFanControlManager::SetSpeedSetting: invalid value");
        return;
    }

    if (aNewSpeedSetting.Value() != mSpeedCurrent)
    {
        Status status = FanControl::Attributes::SpeedSetting::Set(mEndpoint, aNewSpeedSetting);
        if (status != Status::Success)
        {
            ChipLogError(NotSpecified, "ChefFanControlManager::SetSpeedSetting: failed to set SpeedSetting attribute: %d",
                         to_underlying(status));
        }
    }
}

void ChefFanControlManager::Init()
{
    DataModel::Nullable<Percent> percentSetting = GetPercentSetting();
    if (percentSetting.IsNull())
    {
        PercentSettingWriteCallback(0);
    }
    else
    {
        PercentSettingWriteCallback(percentSetting.Value());
    }

    DataModel::Nullable<uint8_t> speedSetting = GetSpeedSetting();
    if (speedSetting.IsNull())
    {
        SpeedSettingWriteCallback(0);
    }
    else
    {
        SpeedSettingWriteCallback(speedSetting.Value());
    }
}

DataModel::Nullable<Percent> ChefFanControlManager::GetPercentSetting()
{
    DataModel::Nullable<Percent> percentSetting;
    Status status = FanControl::Attributes::PercentSetting::Get(mEndpoint, percentSetting);

    if (status != Status::Success)
    {
        ChipLogError(NotSpecified, "ChefFanControlManager::GetPercentSetting: failed to get PercentSetting attribute: %d",
                     to_underlying(status));
    }

    return percentSetting;
}

DataModel::Nullable<uint8_t> ChefFanControlManager::GetSpeedSetting()
{
    DataModel::Nullable<uint8_t> speedSetting;
    Status status = FanControl::Attributes::SpeedSetting::Get(mEndpoint, speedSetting);

    if (status != Status::Success)
    {
        ChipLogError(NotSpecified, "ChefFanControlManager::GetSpeedSetting: failed to get SpeedSetting attribute: %d",
                     to_underlying(status));
    }

    return speedSetting;
}

} // anonymous namespace

void emberAfFanControlClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(!mFanControlManager);
    mFanControlManager = std::make_unique<ChefFanControlManager>(endpoint);
    FanControl::SetDefaultDelegate(endpoint, mFanControlManager.get());
    mFanControlManager->Init();
}

void HandleFanControlAttributeChange(AttributeId attributeId, uint8_t type, uint16_t size, uint8_t * value)
{
    mFanControlManager->HandleFanControlAttributeChange(attributeId, type, size, value);
}
