/**
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
 *
 */

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/microwave-oven-control-server/microwave-oven-control-server.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/error-mapping.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::MicrowaveOvenControl;
using namespace chip::app::Clusters::MicrowaveOvenControl::Attributes;
using Status = Protocols::InteractionModel::Status;

namespace {

constexpr size_t kMicrowaveOvenControlInstanceTableSize =
    EMBER_AF_MICROWAVE_OVEN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kMicrowaveOvenControlInstanceTableSize <= kEmberInvalidEndpointIndex,
              "Microwave Oven Control Instance table size error");

OperationalState::Instance * gOPInstanceTable[kMicrowaveOvenControlInstanceTableSize] = { nullptr };

OperationalState::Instance * gOperationalStateInstance = nullptr;

} // anonymous namespace

namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenControl {

Instance::Instance(Delegate * aDelegate, EndpointId aEndpointId, ClusterId aClusterId) :
    CommandHandlerInterface(MakeOptional(aEndpointId), aClusterId), AttributeAccessInterface(MakeOptional(aEndpointId), aClusterId),
    mDelegate(aDelegate), mEndpointId(aEndpointId), mClusterId(aClusterId)
{
    mDelegate->SetInstance(this);
}

Instance::~Instance()
{
    gOperationalStateInstance = nullptr;
    InteractionModelEngine::GetInstance()->UnregisterCommandHandler(this);
    unregisterAttributeAccessOverride(this);
}

CHIP_ERROR Instance::Init()
{
    // Check if the cluster has been selected in zap
    if (!emberAfContainsServer(mEndpointId, mClusterId))
    {
        ChipLogError(Zcl, "Microwave Oven Control: The cluster with ID %lu was not enabled in zap.", long(mClusterId));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

uint32_t Instance::GetCookTime() const
{
    return mCookTime;
}

uint8_t Instance::GetPowerSetting() const
{
    return mPowerSettng;
}

void Instance::SetCookTime(uint32_t cookTime)
{
    if (mCookTime != cookTime)
    {
        MatterReportingAttributeChangeCallback(ConcreteAttributePath(mEndpointId, mClusterId, Attributes::CookTime::Id));
    }
    mCookTime = cookTime;
}

void Instance::SetPowerSetting(uint8_t powerSetting)
{
    if (mPowerSettng != powerSetting)
    {
        MatterReportingAttributeChangeCallback(ConcreteAttributePath(mEndpointId, mClusterId, Attributes::PowerSetting::Id));
    }
    mPowerSettng = powerSetting;
}

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    ChipLogError(Zcl, "OperationalState: Reading");
    switch (aPath.mAttributeId)
    {
    case MicrowaveOvenControl::Attributes::CookTime::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(GetCookTime()));
    }
    break;

    case MicrowaveOvenControl::Attributes::PowerSetting::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(GetPowerSetting()));
    }
    break;

    case MicrowaveOvenControl::Attributes::MinPower::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetMinPower()));
    }
    break;

    case MicrowaveOvenControl::Attributes::MaxPower::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetMaxPower()));
    }
    break;

    case MicrowaveOvenControl::Attributes::PowerStep::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mDelegate->GetPowerStep()));
    }
    break;
    }
    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogDetail(Zcl, "MicrowaveOvenControl: InvokeCommand");
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::SetCookingParameters::Id:
        ChipLogDetail(Zcl, "MicrowaveOvenControl:  Entering SetCookingParameters");

        CommandHandlerInterface::HandleCommand<Commands::SetCookingParameters::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleSetCookingParameters(ctx, req); });
        break;

    case Commands::AddMoreTime::Id:
        ChipLogDetail(Zcl, "MicrowaveOvenControl:  Entering AddMoreTime");

        CommandHandlerInterface::HandleCommand<Commands::AddMoreTime::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleAddMoreTime(ctx, req); });
        break;
    }
}

void Instance::HandleSetCookingParameters(HandlerContext & ctx, const Commands::SetCookingParameters::DecodableType & req)
{
    ChipLogDetail(Zcl, "MicrowaveOvenControl:  HandleSetCookingParameters");
    Status status;
    uint8_t opState;
    auto & CookMode     = req.cookMode;
    auto & CookTime     = req.cookTime;
    auto & PowerSetting = req.powerSetting;

    // TODO: check if the input cook mode value is invalid

    if (CookTime.HasValue() && (!IsCookTimeInRange(CookTime.Value())))
    {
        status = Status::InvalidCommand;
        ChipLogError(Zcl, "Failed to set cookTime, cookTime value is out of range");
        goto exit;
    }

    if (PowerSetting.HasValue() &&
        (!IsPowerSettingInRange(PowerSetting.Value(), mDelegate->GetMinPower(), mDelegate->GetMaxPower())))
    {
        status = Status::InvalidCommand;
        ChipLogError(Zcl, "Failed to set cookPower, cookPower value is out of range");
        goto exit;
    }

    // Get Operational State instance
    gOperationalStateInstance = GetOPInstance(mEndpointId);
    if (gOperationalStateInstance == nullptr)
    {
        status = Status::InvalidInState;
        ChipLogError(Zcl, "Microwave Oven Control: Operational State instance does not exist");
        goto exit;
    }

    opState = gOperationalStateInstance->GetCurrentOperationalState();
    if (opState == to_underlying(OperationalStateEnum::kStopped))
    {
        uint8_t reqCookMode     = 0;
        uint32_t reqCookTime    = 0;
        uint8_t reqPowerSetting = 0;
        if (CookMode.HasValue())
        {
            // TODO: set Microwave Oven cooking mode by CookMode.Value().
        }
        else
        {
            // TODO: set Microwave Oven cooking mode to normal mode.
        }

        if (CookTime.HasValue())
        {
            reqCookTime = CookTime.Value();
        }
        else
        {
            // set Microwave Oven cooking time to 30 seconds(default).
            reqCookTime = MicrowaveOvenControl::kDefaultCookTime;
        }

        if (PowerSetting.HasValue())
        {
            reqPowerSetting = PowerSetting.Value();
        }
        else
        {
            // set Microwave Oven cooking power to max power(default).
            reqCookTime = mDelegate->GetMaxPower();
        }
        status = mDelegate->HandleSetCookingParametersCallback(reqCookMode, reqCookTime, reqPowerSetting);
        goto exit;
    }
    else
    {
        status = Status::InvalidInState;
        goto exit;
    }

exit:
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleAddMoreTime(HandlerContext & ctx, const Commands::AddMoreTime::DecodableType & req)
{
    ChipLogDetail(Zcl, "MicrowaveOvenControl:  HandleAddMoreTime");
    Status status;
    uint8_t opState;

    // Get Operational State instance
    gOperationalStateInstance = GetOPInstance(mEndpointId);
    if (gOperationalStateInstance == nullptr)
    {
        status = Status::InvalidInState;
        ChipLogError(Zcl, "Microwave Oven Control: Operational State instance does not exist");
        goto exit;
    }

    opState = gOperationalStateInstance->GetCurrentOperationalState();
    if (opState == to_underlying(OperationalStateEnum::kStopped) || opState == to_underlying(OperationalStateEnum::kRunning) ||
        opState == to_underlying(OperationalStateEnum::kPaused))
    {
        uint32_t addedCookTime;

        addedCookTime = GetCookTime() + req.timeToAdd;
        // if the added cooking time is greater than the max cooking time, the cooking time stay unchanged.
        if (addedCookTime < kMaxCookTime)
        {
            status = mDelegate->HandleAddMoreTimeCallback(addedCookTime);
            goto exit;
        }
        else
        {
            ChipLogError(Zcl, "Failed to set cookTime, cookTime value is out of range");
            status = Status::ConstraintError;
            goto exit;
        }
    }
    else // operational state is in error
    {
        status = Status::InvalidInState;
        goto exit;
    }

exit:
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

bool IsCookTimeInRange(uint32_t cookTime)
{
    return (cookTime < MicrowaveOvenControl::kMinCookTime || cookTime > MicrowaveOvenControl::kMaxCookTime) ? false : true;
}

bool IsPowerSettingInRange(uint8_t powerSetting, uint8_t minCookPower, uint8_t maxCookPower)
{
    return (powerSetting < minCookPower || powerSetting > maxCookPower) ? false : true;
}

void SetOPInstance(EndpointId aEndpoint, OperationalState::Instance * aInstance)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(aEndpoint, MicrowaveOvenControl::Id,
                                                       EMBER_AF_MICROWAVE_OVEN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kMicrowaveOvenControlInstanceTableSize)
    {
        gOPInstanceTable[aEndpoint] = aInstance;
    }
}

OperationalState::Instance * GetOPInstance(EndpointId aEndpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(aEndpoint, MicrowaveOvenControl::Id,
                                                       EMBER_AF_MICROWAVE_OVEN_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kMicrowaveOvenControlInstanceTableSize ? nullptr : gOPInstanceTable[aEndpoint]);
}

} // namespace MicrowaveOvenControl
} // namespace Clusters
} // namespace app
} // namespace chip

/** @brief Microwave Oven Control Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void MatterMicrowaveOvenControlPluginServerInitCallback() {}
