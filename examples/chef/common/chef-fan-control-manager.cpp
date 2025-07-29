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
    Protocols::InteractionModel::Status OnCommand(EndpointId endpointId);
    Protocols::InteractionModel::Status OffCommand(EndpointId endpointId);

private:
    uint8_t mPercentCurrent = 0;
    uint8_t mSpeedCurrent   = 0;

    // Fan Mode Limits
    struct Range
    {
        bool Contains(int value) const { return value >= low && value <= high; }
        int Low() const { return low; }
        int High() const { return high; }

        int low;
        int high;
    };
    static constexpr Range kFanModeLowSpeedRange    = { 1, 3 };
    static constexpr Range kFanModeMediumSpeedRange = { 4, 7 };
    static constexpr Range kFanModeHighSpeedRange   = { 8, 10 };

    static_assert(kFanModeLowSpeedRange.low <= kFanModeLowSpeedRange.high);
    static_assert(kFanModeLowSpeedRange.high + 1 == kFanModeMediumSpeedRange.low);
    static_assert(kFanModeMediumSpeedRange.high + 1 == kFanModeHighSpeedRange.low);
    static_assert(kFanModeHighSpeedRange.low <= kFanModeHighSpeedRange.high);

    void FanModeWriteCallback(FanControl::FanModeEnum aNewFanMode);
    void SetSpeedCurrent(uint8_t aNewSpeedCurrent);
    void SetPercentCurrent(uint8_t aNewPercentCurrent);
    void SetSpeedSetting(DataModel::Nullable<uint8_t> aNewSpeedSetting);
    static FanControl::FanModeEnum SpeedToFanMode(uint8_t speed);
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
        DataModel::Nullable<Percent> percentSetting;
        if (!NumericAttributeTraits<Percent>::IsNullValue(*value))
        {
            percentSetting.SetNonNull(NumericAttributeTraits<Percent>::StorageToWorking(*value));
        }
        else
        {
            percentSetting.SetNull();
        }

        // The cluster code in fan-control-server.cpp is the only one allowed to set PercentSetting to null.
        // This happens as a consequence of setting the FanMode to kAuto. In auto mode, percentCurrent should continue to report the
        // real fan speed percentage. In this example, we set PercentCurrent to 0 here as we don't have a real value for the Fan
        // speed or a FanAutoMode simulator.
        // When not Null, SpeedCurrent tracks SpeedSetting's value.
        SetPercentCurrent(percentSetting.ValueOr(0));
        break;
    }

    case FanControl::Attributes::SpeedSetting::Id: {
        ChipLogProgress(NotSpecified, "ChefFanControlManager::HandleFanControlAttributeChange  SpeedSetting");
        DataModel::Nullable<uint8_t> speedSetting;
        if (!NumericAttributeTraits<uint8_t>::IsNullValue(*value))
        {
            speedSetting.SetNonNull(NumericAttributeTraits<uint8_t>::StorageToWorking(*value));
        }
        else
        {
            speedSetting.SetNull();
        }

        // The cluster code in fan-control-server.cpp is the only one allowed to set speedSetting to null.
        // This happens as a consequence of setting the FanMode to kAuto. In auto mode, speedCurrent should continue to report the
        // real fan speed. In this example, we set SpeedCurrent to 0 here as we don't have a real value for the Fan speed or a
        // FanAutoMode simulator.
        // When not Null, SpeedCurrent tracks SpeedSetting's value.
        SetSpeedCurrent(speedSetting.ValueOr(0));
        // Determine if the speed change should also change the fan mode
        FanControl::Attributes::FanMode::Set(mEndpoint, SpeedToFanMode(mSpeedCurrent));
        break;
    }

    case FanControl::Attributes::FanMode::Id: {
        ChipLogProgress(NotSpecified, "ChefFanControlManager::HandleFanControlAttributeChange  FanMode");

        static_assert(sizeof(FanControl::FanModeEnum) == 1);
        FanControl::FanModeEnum fanMode = static_cast<FanControl::FanModeEnum>(*value);
        FanModeWriteCallback(fanMode);
        break;
    }

    default: {
        break;
    }
    }
}

FanControl::FanModeEnum ChefFanControlManager::SpeedToFanMode(uint8_t speed)
{
    if (speed == 0)
    {
        return FanControl::FanModeEnum::kOff;
    }
    if (kFanModeLowSpeedRange.Contains(speed))
    {
        return FanControl::FanModeEnum::kLow;
    }
    if (kFanModeMediumSpeedRange.Contains(speed))
    {
        return FanControl::FanModeEnum::kMedium;
    }
    return FanControl::FanModeEnum::kHigh;
}

void ChefFanControlManager::SetPercentCurrent(uint8_t aNewPercentCurrent)
{
    ChipLogDetail(NotSpecified, "ChefFanControlManager::SetPercentCurrent: %d", aNewPercentCurrent);
    mPercentCurrent = aNewPercentCurrent;
    Status status   = FanControl::Attributes::PercentCurrent::Set(mEndpoint, mPercentCurrent);
    if (status != Status::Success)
    {
        ChipLogError(NotSpecified, "ChefFanControlManager::SetPercentCurrent: failed to set PercentCurrent attribute: %d",
                     to_underlying(status));
    }
}

void ChefFanControlManager::SetSpeedCurrent(uint8_t aNewSpeedCurrent)
{
    mSpeedCurrent = aNewSpeedCurrent;
    Status status = FanControl::Attributes::SpeedCurrent::Set(mEndpoint, aNewSpeedCurrent);
    if (status != Status::Success)
    {
        ChipLogError(NotSpecified, "ChefFanControlManager::SetSpeedCurrent: failed to set SpeedCurrent attribute: %d",
                     to_underlying(status));
    }
}

void ChefFanControlManager::FanModeWriteCallback(FanControl::FanModeEnum aNewFanMode)
{
    ChipLogDetail(NotSpecified, "ChefFanControlManager::FanModeWriteCallback: %d", to_underlying(aNewFanMode));
    switch (aNewFanMode)
    {
    case FanControl::FanModeEnum::kOff: {
        DataModel::Nullable<uint8_t> speedSetting(0);
        SetSpeedSetting(speedSetting);
        break;
    }
    case FanControl::FanModeEnum::kLow: {
        if (!kFanModeLowSpeedRange.Contains(mSpeedCurrent))
        {
            DataModel::Nullable<uint8_t> speedSetting(kFanModeLowSpeedRange.Low());
            SetSpeedSetting(speedSetting);
        }
        break;
    }
    case FanControl::FanModeEnum::kMedium: {
        if (!kFanModeMediumSpeedRange.Contains(mSpeedCurrent))
        {
            DataModel::Nullable<uint8_t> speedSetting(kFanModeMediumSpeedRange.Low());
            SetSpeedSetting(speedSetting);
        }
        break;
    }
    case FanControl::FanModeEnum::kOn:
    case FanControl::FanModeEnum::kHigh: {
        if (!kFanModeHighSpeedRange.Contains(mSpeedCurrent))
        {
            DataModel::Nullable<uint8_t> speedSetting(kFanModeHighSpeedRange.Low());
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
    Status status = FanControl::Attributes::SpeedSetting::Set(mEndpoint, aNewSpeedSetting);
    if (status != Status::Success)
    {
        ChipLogError(NotSpecified, "ChefFanControlManager::SetSpeedSetting: failed to set SpeedSetting attribute: %d",
                     to_underlying(status));
    }
}

void ChefFanControlManager::Init()
{
    SetPercentCurrent(GetPercentSetting().ValueOr(0));
    SetSpeedCurrent(GetSpeedSetting().ValueOr(0));
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

Protocols::InteractionModel::Status ChefFanControlManager::OnCommand(EndpointId endpointId)
{
    ChipLogProgress(DeviceLayer, "ChefFanControlManager::OnCommand");

    FanControl::FanModeEnum fanMode;
    FanControl::Attributes::FanMode::Get(endpointId, &fanMode);

    if (fanMode == FanControl::FanModeEnum::kOff) // Off mode implies Speed/Percent setting values are 0. Set fan to HIGH.
    {
        uint8_t speedMax;
        Status status = SpeedMax::Get(mEndpoint, &speedMax);
        if (status == Status::Success)
        {
            status = FanControl::Attributes::SpeedSetting::Set(mEndpoint, speedMax);
            if (status == Status::Success)
            {
                // Atribute change handler sets SpeedCurrent equal to SpeedSetting and updates FanMode.
                MatterReportingAttributeChangeCallback(endpointId, FanControl::Id, FanControl::Attributes::SpeedSetting::Id);
            }
            else
            {
                ChipLogError(DeviceLayer, "Error setting SpeedSetting: %d", to_underlying(status));
                return status; // Speed is enabled since SpeedMax read was successful. So return failed status.
            }
        }
        else
        {
            // Not returning error as speed is optional.
            ChipLogError(DeviceLayer, "Error getting SpeedMax: %d", to_underlying(status));
        }
        status = FanControl::Attributes::PercentSetting::Set(mEndpoint, 100);
        if (status == Status::Success)
        {
            // Atribute change handler sets PercentCurrent equal to PercentSetting.
            MatterReportingAttributeChangeCallback(endpointId, FanControl::Id, FanControl::Attributes::PercentSetting::Id);
        }
        else
        {
            return status; // Percent is mandatory. So return failed status.
        }
    }

    Status status;

    DataModel::Nullable<uint8_t> speedSetting(GetSpeedSetting());

    if (!speedSetting.IsNull() && speedSetting.Value())
    {
        status = FanControl::Attributes::SpeedCurrent::Set(endpointId, speedSetting.Value());
        if (status != Status::Success)
        {
            ChipLogError(DeviceLayer, "Error setting SpeedCurrent: %d", to_underlying(status));
            return status;
        }
        MatterReportingAttributeChangeCallback(endpointId, FanControl::Id, FanControl::Attributes::SpeedCurrent::Id);
    }

    DataModel::Nullable<uint8_t> percentSetting(GetPercentSetting());

    if (!percentSetting.IsNull() && percentSetting.Value())
    {
        status = FanControl::Attributes::PercentCurrent::Set(endpointId, percentSetting.Value());
        if (status != Status::Success)
        {
            ChipLogError(DeviceLayer, "Error setting PercentCurrent: %d", to_underlying(status));
            return status;
        }
        MatterReportingAttributeChangeCallback(endpointId, FanControl::Id, FanControl::Attributes::PercentCurrent::Id);
    }

    return Status::Success;
}

Protocols::InteractionModel::Status ChefFanControlManager::OffCommand(EndpointId endpointId)
{
    ChipLogProgress(DeviceLayer, "ChefFanControlManager::OffCommand");

    FanControl::FanModeEnum fanMode;
    FanControl::Attributes::FanMode::Get(endpointId, &fanMode);

    if (fanMode == FanControl::FanModeEnum::kOff) // Off mode implies Speed/Percent current values are 0.
    {
        return Status::Success;
    }

    Status status;

    uint8_t speedCurrent;
    status = SpeedCurrent::Get(endpointId, &speedCurrent);

    if (status == Protocols::InteractionModel::Status::Success && speedCurrent)
    {
        status = FanControl::Attributes::SpeedCurrent::Set(endpointId, 0);
        if (status != Status::Success)
        {
            ChipLogError(DeviceLayer, "Error setting SpeedCurrent: %d", to_underlying(status));
            return status;
        }
        MatterReportingAttributeChangeCallback(endpointId, FanControl::Id, FanControl::Attributes::SpeedCurrent::Id);
    }

    uint8_t percentCurrent;
    status = PercentCurrent::Get(endpointId, &percentCurrent);
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);

    if (percentCurrent)
    {
        status = FanControl::Attributes::PercentCurrent::Set(endpointId, 0);
        if (status != Status::Success)
        {
            ChipLogError(DeviceLayer, "Error setting PercentCurrent: %d", to_underlying(status));
            return status;
        }
        MatterReportingAttributeChangeCallback(endpointId, FanControl::Id, FanControl::Attributes::PercentCurrent::Id);
    }

    return Status::Success;
}

void HandleOnOffAttributeChangeForFan(EndpointId endpointId, bool value)
{
    if (value)
        mFanControlManager->OnCommand(endpointId);
    else
        mFanControlManager->OffCommand(endpointId);
}

void HandleFanControlAttributeChange(AttributeId attributeId, uint8_t type, uint16_t size, uint8_t * value)
{
    mFanControlManager->HandleFanControlAttributeChange(attributeId, type, size, value);
}
