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
class ChefFanControlManager : public AttributeAccessInterface, public Delegate
{
public:
    ChefFanControlManager(EndpointId aEndpointId) :
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), FanControl::Id), Delegate(aEndpointId)
    {}

    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    Status HandleStep(StepDirectionEnum aDirection, bool aWrap, bool aLowestOff) override;

private:
    Nullable<uint8_t> mPercentSetting{};
    Nullable<uint8_t> mSpeedSetting{};
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

CHIP_ERROR ChefFanControlManager::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == FanControl::Id);
    VerifyOrDie(aPath.mEndpointId == mEndpoint);

    switch (aPath.mAttributeId)
    {
    case SpeedSetting::Id: {
        Nullable<uint8_t> newSpeedSetting;
        ReturnErrorOnFailure(aDecoder.Decode(newSpeedSetting));

        // Ensure new speed is in bounds
        {
            uint8_t maxSpeedSetting                    = 0;
            Protocols::InteractionModel::Status status = SpeedMax::Get(mEndpoint, &maxSpeedSetting);
            VerifyOrReturnError(status == Protocols::InteractionModel::Status::Success, CHIP_IM_GLOBAL_STATUS(Failure));

            if (!newSpeedSetting.IsNull() && newSpeedSetting.Value() > maxSpeedSetting)
            {
                return CHIP_IM_GLOBAL_STATUS(ConstraintError);
            }
        }

        // Only act on changed.
        if (newSpeedSetting != mSpeedSetting)
        {
            mSpeedSetting = newSpeedSetting;

            // Mark both the setting AND the current dirty, since the current always
            // tracks the target for our product.
            MatterReportingAttributeChangeCallback(mEndpoint, FanControl::Id, Attributes::SpeedSetting::Id);
            MatterReportingAttributeChangeCallback(mEndpoint, FanControl::Id, Attributes::SpeedCurrent::Id);
        }

        break;
    }
    case PercentSetting::Id: {
        Nullable<uint8_t> newPercentSetting;
        ReturnErrorOnFailure(aDecoder.Decode(newPercentSetting));

        // Ensure new speed in percent is valid.
        if (!newPercentSetting.IsNull() && newPercentSetting.Value() > 100)
        {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }

        // Only act on changed.
        if (newPercentSetting != mPercentSetting)
        {
            mPercentSetting = newPercentSetting;

            // Mark both the setting AND the current dirty, since the current always
            // tracks the target for our product.
            MatterReportingAttributeChangeCallback(mEndpoint, FanControl::Id, Attributes::PercentSetting::Id);
            MatterReportingAttributeChangeCallback(mEndpoint, FanControl::Id, Attributes::PercentCurrent::Id);
        }

        break;
    }
    default:
        break;
    }

    // Fall through goes to attribute store legacy handling.
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChefFanControlManager::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == FanControl::Id);
    VerifyOrDie(aPath.mEndpointId == mEndpoint);

    switch (aPath.mAttributeId)
    {
    case PercentCurrent::Id: {
        // Current percents always tracks setting immediately in our implementation.
        return aEncoder.Encode(mPercentSetting.ValueOr(0));
    }
    case PercentSetting::Id: {
        return aEncoder.Encode(mPercentSetting);
    }
    case SpeedCurrent::Id: {
        // Current speed always tracks setting immediately in our implementation.
        return aEncoder.Encode(mSpeedSetting.ValueOr(0));
    }
    case SpeedSetting::Id: {
        return aEncoder.Encode(mSpeedSetting);
    }
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

} // anonymous namespace

void emberAfFanControlClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(!mFanControlManager);
    mFanControlManager = std::make_unique<ChefFanControlManager>(endpoint);
    AttributeAccessInterfaceRegistry::Instance().Register(mFanControlManager.get());
    FanControl::SetDefaultDelegate(endpoint, mFanControlManager.get());
}
