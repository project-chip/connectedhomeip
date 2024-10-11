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
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FanControl;
using namespace chip::app::Clusters::FanControl::Attributes;
using Protocols::InteractionModel::Status;

namespace {
class FanControlManager : public AttributeAccessInterface, public Delegate
{
public:
    // Register for the FanControl cluster on all endpoints.
    FanControlManager(EndpointId aEndpointId) :
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), FanControl::Id), Delegate(aEndpointId)
    {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    Status HandleStep(StepDirectionEnum aDirection, bool aWrap, bool aLowestOff) override;

private:
    CHIP_ERROR ReadPercentCurrent(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSpeedCurrent(AttributeValueEncoder & aEncoder);
};

static FanControlManager * mFanControlManager = nullptr;

CHIP_ERROR FanControlManager::ReadPercentCurrent(AttributeValueEncoder & aEncoder)
{
    // Return PercentSetting attribute value for now
    DataModel::Nullable<Percent> percentSetting;
    PercentSetting::Get(mEndpoint, percentSetting);
    Percent ret = 0;
    if (!percentSetting.IsNull())
    {
        ret = percentSetting.Value();
    }

    return aEncoder.Encode(ret);
}

CHIP_ERROR FanControlManager::ReadSpeedCurrent(AttributeValueEncoder & aEncoder)
{
    // Return SpeedCurrent attribute value for now
    DataModel::Nullable<uint8_t> speedSetting;
    SpeedSetting::Get(mEndpoint, speedSetting);
    uint8_t ret = 0;
    if (!speedSetting.IsNull())
    {
        ret = speedSetting.Value();
    }

    return aEncoder.Encode(ret);
}

Status FanControlManager::HandleStep(StepDirectionEnum aDirection, bool aWrap, bool aLowestOff)
{
    ChipLogProgress(NotSpecified, "FanControlManager::HandleStep aDirection %d, aWrap %d, aLowestOff %d", to_underlying(aDirection),
                    aWrap, aLowestOff);

    VerifyOrReturnError(aDirection != StepDirectionEnum::kUnknownEnumValue, Status::InvalidCommand);

    uint8_t speedMax;
    SpeedMax::Get(mEndpoint, &speedMax);

    DataModel::Nullable<uint8_t> speedSetting;
    SpeedSetting::Get(mEndpoint, speedSetting);

    uint8_t newSpeedSetting = speedSetting.IsNull() ? 0 : speedSetting.Value();

    if (aDirection == StepDirectionEnum::kIncrease)
    {
        if (speedSetting.IsNull())
        {
            newSpeedSetting = 1;
        }
        else if (speedSetting.Value() < speedMax)
        {
            newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() + 1);
        }
        else if (speedSetting.Value() == speedMax)
        {
            if (aWrap)
            {
                newSpeedSetting = aLowestOff ? 0 : 1;
            }
        }
    }
    else if (aDirection == StepDirectionEnum::kDecrease)
    {
        if (speedSetting.IsNull())
        {
            newSpeedSetting = aLowestOff ? 0 : 1;
        }
        else if ((speedSetting.Value() > 1) && (speedSetting.Value() <= speedMax))
        {
            newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() - 1);
        }
        else if (speedSetting.Value() == 1)
        {
            if (aLowestOff)
            {
                newSpeedSetting = static_cast<uint8_t>(speedSetting.Value() - 1);
            }
            else if (aWrap)
            {
                newSpeedSetting = speedMax;
            }
        }
        else if (speedSetting.Value() == 0)
        {
            if (aWrap)
            {
                newSpeedSetting = speedMax;
            }
            else if (!aLowestOff)
            {
                newSpeedSetting = 1;
            }
        }
    }

    return SpeedSetting::Set(mEndpoint, newSpeedSetting);
}

CHIP_ERROR FanControlManager::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
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
    VerifyOrDie(mFanControlManager == nullptr);
    mFanControlManager = new FanControlManager(endpoint);
    AttributeAccessInterfaceRegistry::Instance().Register(mFanControlManager);
    FanControl::SetDefaultDelegate(endpoint, mFanControlManager);
}
