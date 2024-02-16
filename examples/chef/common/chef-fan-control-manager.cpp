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
#include <app/clusters/fan-control-server/fan-control-server.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <functional>

using namespace chip;
using namespace chip::app;
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

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    Status HandleStep(StepDirectionEnum aDirection, bool aWrap, bool aLowestOff) override;

private:
    CHIP_ERROR ReadPercentCurrent(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSpeedCurrent(AttributeValueEncoder & aEncoder);
};

static std::unique_ptr<ChefFanControlManager> mFanControlManager;

CHIP_ERROR ChefFanControlManager::ReadPercentCurrent(AttributeValueEncoder & aEncoder)
{
    // Return PercentSetting attribute value for now
    DataModel::Nullable<Percent> percentSetting;
    Protocols::InteractionModel::Status status = PercentSetting::Get(mEndpoint, percentSetting);

    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, CHIP_ERROR_READ_FAILED);

    return aEncoder.Encode(percentSetting.ValueOr(0));
}

CHIP_ERROR ChefFanControlManager::ReadSpeedCurrent(AttributeValueEncoder & aEncoder)
{
    // Return SpeedCurrent attribute value for now
    DataModel::Nullable<uint8_t> speedSetting;
    Protocols::InteractionModel::Status status = SpeedSetting::Get(mEndpoint, speedSetting);

    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, CHIP_ERROR_READ_FAILED);

    return aEncoder.Encode(speedSetting.ValueOr(0));
}

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

CHIP_ERROR ChefFanControlManager::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == FanControl::Id);
    VerifyOrDie(aPath.mEndpointId == mEndpoint);

    switch (aPath.mAttributeId)
    {
    case SpeedCurrent::Id:
        return ReadSpeedCurrent(aEncoder);
    case PercentCurrent::Id:
        return ReadPercentCurrent(aEncoder);
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
    registerAttributeAccessOverride(mFanControlManager.get());
    FanControl::SetDefaultDelegate(endpoint, mFanControlManager.get());
}
