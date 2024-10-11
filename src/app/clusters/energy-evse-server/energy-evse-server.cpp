/*
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

#include "energy-evse-server.h"

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using namespace chip::app::Clusters::EnergyEvse::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

CHIP_ERROR Instance::Init()
{
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

bool Instance::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

bool Instance::SupportsOptAttr(OptionalAttributes aOptionalAttrs) const
{
    return mOptionalAttrs.Has(aOptionalAttrs);
}

bool Instance::SupportsOptCmd(OptionalCommands aOptionalCmds) const
{
    return mOptionalCmds.Has(aOptionalCmds);
}

// AttributeAccessInterface
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case State::Id:
        return aEncoder.Encode(mDelegate.GetState());
    case SupplyState::Id:
        return aEncoder.Encode(mDelegate.GetSupplyState());
    case FaultState::Id:
        return aEncoder.Encode(mDelegate.GetFaultState());
    case ChargingEnabledUntil::Id:
        return aEncoder.Encode(mDelegate.GetChargingEnabledUntil());
    case DischargingEnabledUntil::Id:
        /* V2X */
        return aEncoder.Encode(mDelegate.GetDischargingEnabledUntil());
    case CircuitCapacity::Id:
        return aEncoder.Encode(mDelegate.GetCircuitCapacity());
    case MinimumChargeCurrent::Id:
        return aEncoder.Encode(mDelegate.GetMinimumChargeCurrent());
    case MaximumChargeCurrent::Id:
        return aEncoder.Encode(mDelegate.GetMaximumChargeCurrent());
    case MaximumDischargeCurrent::Id:
        /* V2X */
        return aEncoder.Encode(mDelegate.GetMaximumDischargeCurrent());

    case UserMaximumChargeCurrent::Id:
        return aEncoder.Encode(mDelegate.GetUserMaximumChargeCurrent());
    case RandomizationDelayWindow::Id:
        /* Optional */
        return aEncoder.Encode(mDelegate.GetRandomizationDelayWindow());
    /* PREF - ChargingPreferences attributes */
    case NextChargeStartTime::Id:
        return aEncoder.Encode(mDelegate.GetNextChargeStartTime());
    case NextChargeTargetTime::Id:
        return aEncoder.Encode(mDelegate.GetNextChargeTargetTime());
    case NextChargeRequiredEnergy::Id:
        return aEncoder.Encode(mDelegate.GetNextChargeRequiredEnergy());
    case NextChargeTargetSoC::Id:
        return aEncoder.Encode(mDelegate.GetNextChargeTargetSoC());
    case ApproximateEVEfficiency::Id:
        return aEncoder.Encode(mDelegate.GetApproximateEVEfficiency());
    /* SOC attributes */
    case StateOfCharge::Id:
        return aEncoder.Encode(mDelegate.GetStateOfCharge());
    case BatteryCapacity::Id:
        return aEncoder.Encode(mDelegate.GetBatteryCapacity());
    /* PNC attributes*/
    case VehicleID::Id:
        return aEncoder.Encode(mDelegate.GetVehicleID());
    /* Session SESS attributes */
    case SessionID::Id:
        return aEncoder.Encode(mDelegate.GetSessionID());
    case SessionDuration::Id:
        return aEncoder.Encode(mDelegate.GetSessionDuration());
    case SessionEnergyCharged::Id:
        return aEncoder.Encode(mDelegate.GetSessionEnergyCharged());
    case SessionEnergyDischarged::Id:
        return aEncoder.Encode(mDelegate.GetSessionEnergyDischarged());

    /* FeatureMap - is held locally */
    case FeatureMap::Id:
        return aEncoder.Encode(mFeature);
    }
    /* Allow all other unhandled attributes to fall through to Ember */
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case UserMaximumChargeCurrent::Id: {
        // Optional Attribute
        if (!SupportsOptAttr(OptionalAttributes::kSupportsUserMaximumChargingCurrent))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }

        int64_t newValue;
        ReturnErrorOnFailure(aDecoder.Decode(newValue));
        ReturnErrorOnFailure(mDelegate.SetUserMaximumChargeCurrent(newValue));
        return CHIP_NO_ERROR;
    }
    case RandomizationDelayWindow::Id: {
        // Optional Attribute
        if (!SupportsOptAttr(OptionalAttributes::kSupportsRandomizationWindow))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        uint32_t newValue;
        ReturnErrorOnFailure(aDecoder.Decode(newValue));
        ReturnErrorOnFailure(mDelegate.SetRandomizationDelayWindow(newValue));
        return CHIP_NO_ERROR;
    }
    case ApproximateEVEfficiency::Id: {
        // Optional Attribute if ChargingPreferences is supported
        if ((!HasFeature(Feature::kChargingPreferences)) ||
            (!SupportsOptAttr(OptionalAttributes::kSupportsApproximateEvEfficiency)))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        uint16_t newValue;
        ReturnErrorOnFailure(aDecoder.Decode(newValue));
        ReturnErrorOnFailure(mDelegate.SetApproximateEVEfficiency(MakeNullable(newValue)));
        return CHIP_NO_ERROR;
    }

    default:
        // Unknown attribute; return error.  None of the other attributes for
        // this cluster are writable, so should not be ending up in this code to
        // start with.
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

// CommandHandlerInterface
CHIP_ERROR Instance::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
{
    using namespace Commands;

    for (auto && cmd : {
             Disable::Id,
             EnableCharging::Id,
         })
    {
        VerifyOrExit(callback(cmd, context) == Loop::Continue, /**/);
    }

    if (HasFeature(Feature::kV2x))
    {
        VerifyOrExit(callback(EnableDischarging::Id, context) == Loop::Continue, /**/);
    }

    if (HasFeature(Feature::kChargingPreferences))
    {
        for (auto && cmd : {
                 SetTargets::Id,
                 GetTargets::Id,
                 ClearTargets::Id,
             })
        {
            VerifyOrExit(callback(cmd, context) == Loop::Continue, /**/);
        }
    }

    if (SupportsOptCmd(OptionalCommands::kSupportsStartDiagnostics))
    {
        callback(StartDiagnostics::Id, context);
    }

exit:
    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    using namespace Commands;

    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Disable::Id:
        HandleCommand<Disable::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleDisable(ctx, commandData); });
        return;
    case EnableCharging::Id:
        HandleCommand<EnableCharging::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleEnableCharging(ctx, commandData); });
        return;
    case EnableDischarging::Id:
        if (!HasFeature(Feature::kV2x))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<EnableDischarging::DecodableType>(handlerContext, [this](HandlerContext & ctx, const auto & commandData) {
                HandleEnableDischarging(ctx, commandData);
            });
        }
        return;
    case StartDiagnostics::Id:
        if (!SupportsOptCmd(OptionalCommands::kSupportsStartDiagnostics))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<StartDiagnostics::DecodableType>(handlerContext, [this](HandlerContext & ctx, const auto & commandData) {
                HandleStartDiagnostics(ctx, commandData);
            });
        }
        return;
    case SetTargets::Id:
        if (!HasFeature(Feature::kChargingPreferences))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<SetTargets::DecodableType>(
                handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleSetTargets(ctx, commandData); });
        }
        return;
    case GetTargets::Id:
        if (!HasFeature(Feature::kChargingPreferences))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<GetTargets::DecodableType>(
                handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleGetTargets(ctx, commandData); });
        }
        return;
    case ClearTargets::Id:
        if (!HasFeature(Feature::kChargingPreferences))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<ClearTargets::DecodableType>(
                handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleClearTargets(ctx, commandData); });
        }
        return;
    }
}

void Instance::HandleDisable(HandlerContext & ctx, const Commands::Disable::DecodableType & commandData)
{
    // No parameters for this command
    // Call the delegate
    Status status = mDelegate.Disable();

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleEnableCharging(HandlerContext & ctx, const Commands::EnableCharging::DecodableType & commandData)
{
    auto & chargingEnabledUntil = commandData.chargingEnabledUntil;
    auto & minimumChargeCurrent = commandData.minimumChargeCurrent;
    auto & maximumChargeCurrent = commandData.maximumChargeCurrent;

    if (minimumChargeCurrent < kMinimumChargeCurrent)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    if (maximumChargeCurrent < kMinimumChargeCurrent)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    if (minimumChargeCurrent > maximumChargeCurrent)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    // Call the delegate
    Status status = mDelegate.EnableCharging(chargingEnabledUntil, minimumChargeCurrent, maximumChargeCurrent);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleEnableDischarging(HandlerContext & ctx, const Commands::EnableDischarging::DecodableType & commandData)
{

    auto & dischargingEnabledUntil = commandData.dischargingEnabledUntil;
    auto & maximumDischargeCurrent = commandData.maximumDischargeCurrent;

    if (maximumDischargeCurrent < kMinimumChargeCurrent)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    // Call the delegate
    Status status = mDelegate.EnableDischarging(dischargingEnabledUntil, maximumDischargeCurrent);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}
void Instance::HandleStartDiagnostics(HandlerContext & ctx, const Commands::StartDiagnostics::DecodableType & commandData)
{
    // No parameters for this command
    // Call the delegate
    Status status = mDelegate.StartDiagnostics();

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleSetTargets(HandlerContext & ctx, const Commands::SetTargets::DecodableType & commandData)
{
    // Call the delegate
    auto & chargingTargetSchedules = commandData.chargingTargetSchedules;

    Status status = ValidateTargets(chargingTargetSchedules);
    if (status != Status::Success)
    {
        ChipLogError(AppServer, "SetTargets contained invalid data - Rejecting");
    }
    else
    {
        status = mDelegate.SetTargets(chargingTargetSchedules);
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

Status Instance::ValidateTargets(
    const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & chargingTargetSchedules)
{
    /* A) check that the targets are valid
     *  1) each target must be within valid range (TargetTimeMinutesPastMidnight < 1440)
     *  2) each target must be within valid range (TargetSoC percent 0 - 100)
     *      If SOC feature not supported then this MUST be 100 or not present
     *  3) each target must be within valid range (AddedEnergy >= 0)
     * B) Day of Week is only allowed to be included once
     */

    uint8_t dayOfWeekBitmap = 0;

    auto iter = chargingTargetSchedules.begin();
    while (iter.Next())
    {
        auto & entry    = iter.GetValue();
        uint8_t bitmask = entry.dayOfWeekForSequence.GetField(static_cast<TargetDayOfWeekBitmap>(0x7F));
        ChipLogProgress(AppServer, "DayOfWeekForSequence = 0x%02x", bitmask);

        if ((dayOfWeekBitmap & bitmask) != 0)
        {
            // A bit has already been set - Return ConstraintError
            ChipLogError(AppServer, "DayOfWeekForSequence has a bit set which has already been set in another entry.");
            return Status::ConstraintError;
        }
        dayOfWeekBitmap |= bitmask; // add this day Of week to the previously seen days

        auto iterInner   = entry.chargingTargets.begin();
        uint8_t innerIdx = 0;
        while (iterInner.Next())
        {
            auto & targetStruct          = iterInner.GetValue();
            uint16_t minutesPastMidnight = targetStruct.targetTimeMinutesPastMidnight;
            ChipLogProgress(AppServer, "[%d] MinutesPastMidnight : %d", innerIdx,
                            static_cast<short unsigned int>(minutesPastMidnight));

            if (minutesPastMidnight > 1439)
            {
                ChipLogError(AppServer, "MinutesPastMidnight has invalid value (%d)", static_cast<int>(minutesPastMidnight));
                return Status::ConstraintError;
            }

            // If SocReporting is supported, targetSoc must have a value in the range [0, 100]
            if (HasFeature(Feature::kSoCReporting))
            {
                if (!targetStruct.targetSoC.HasValue())
                {
                    ChipLogError(AppServer, "kSoCReporting is supported but TargetSoC does not have a value");
                    return Status::Failure;
                }

                if (targetStruct.targetSoC.Value() > 100)
                {
                    ChipLogError(AppServer, "TargetSoC has invalid value (%d)", static_cast<int>(targetStruct.targetSoC.Value()));
                    return Status::ConstraintError;
                }
            }
            else if (targetStruct.targetSoC.HasValue() && targetStruct.targetSoC.Value() != 100)
            {
                // If SocReporting is not supported but targetSoc has a value, it must be 100
                ChipLogError(AppServer, "TargetSoC has can only be 100%% if SOC feature is not supported");
                return Status::ConstraintError;
            }

            // One or both of targetSoc and addedEnergy must be specified
            if (!(targetStruct.targetSoC.HasValue()) && !(targetStruct.addedEnergy.HasValue()))
            {
                ChipLogError(AppServer, "Must have one of AddedEnergy or TargetSoC");
                return Status::Failure;
            }

            // Validate the value of addedEnergy, if specified is >= 0
            if (targetStruct.addedEnergy.HasValue() && targetStruct.addedEnergy.Value() < 0)
            {
                ChipLogError(AppServer, "AddedEnergy has invalid value (%ld)",
                             static_cast<signed long int>(targetStruct.addedEnergy.Value()));
                return Status::ConstraintError;
            }
            innerIdx++;
        }

        if (iterInner.GetStatus() != CHIP_NO_ERROR)
        {
            return Status::InvalidCommand;
        }
    }

    if (iter.GetStatus() != CHIP_NO_ERROR)
    {
        return Status::InvalidCommand;
    }

    return Status::Success;
}

void Instance::HandleGetTargets(HandlerContext & ctx, const Commands::GetTargets::DecodableType & commandData)
{
    Commands::GetTargetsResponse::Type response;

    Status status = mDelegate.GetTargets(response.chargingTargetSchedules);
    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}

void Instance::HandleClearTargets(HandlerContext & ctx, const Commands::ClearTargets::DecodableType & commandData)
{
    // Call the delegate
    Status status = mDelegate.ClearTargets();

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterEnergyEvsePluginServerInitCallback() {}
