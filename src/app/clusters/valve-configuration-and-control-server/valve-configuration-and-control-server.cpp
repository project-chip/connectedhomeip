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

#include "valve-configuration-and-control-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ValveConfigurationAndControl::Attributes;
using chip::app::Clusters::ValveConfigurationAndControl::Delegate;
using chip::Protocols::InteractionModel::Status;

static constexpr size_t kValveConfigurationAndControlDelegateTableSize =
    EMBER_AF_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kValveConfigurationAndControlDelegateTableSize <= kEmberInvalidEndpointIndex,
              "ValveConfigurationAndControl Delegate table size error");

namespace {
Delegate * gDelegateTable[kValveConfigurationAndControlDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, ValveConfigurationAndControl::Id,
                                                       EMBER_AF_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kValveConfigurationAndControlDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        return true;
    }
    return false;
}
} // namespace

static void startRemainingDurationTick(EndpointId ep);

static bool emitValveStateChangedEvent(EndpointId ep, ValveConfigurationAndControl::ValveStateEnum state)
{
    ValveConfigurationAndControl::Events::ValveStateChanged::Type event;
    EventNumber eventNumber;
    event.valveState = state;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to emit ValveStateChanged event [ep=%d]", ep);
        return false;
    }

    ChipLogProgress(Zcl, "Emit ValveStateChanged event [ep=%d]", ep);
    return true;
}

static CHIP_ERROR emitValveFaultEvent(EndpointId ep, chip::BitMask<ValveConfigurationAndControl::ValveFaultBitmap> fault)
{
    ValveConfigurationAndControl::Events::ValveFault::Type event;
    EventNumber eventNumber;
    event.valveFault = fault;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to emit ValveFault event [ep=%d]", ep);
        return error;
    }

    ChipLogProgress(Zcl, "Emit ValveFault event [ep=%d]", ep);
    return CHIP_NO_ERROR;
}

static void onValveConfigurationAndControlTick(chip::System::Layer * systemLayer, void * data)
{
    Delegate * delegate = reinterpret_cast<Delegate *>(data);
    if (delegate == nullptr)
        return;
    EndpointId ep = delegate->mEndpoint;

    if (delegate->mRemainingDuration > 0)
    {
        delegate->mRemainingDuration--;
        if (delegate->mRemainingDuration < 5 || !(delegate->mRemainingDuration % 5))
        {
            RemainingDuration::Set(ep, delegate->mRemainingDuration);
        }
        startRemainingDurationTick(ep);
    }
    else
    {
        RemainingDuration::SetNull(ep);
    }
}

void startRemainingDurationTick(EndpointId ep)
{
    DataModel::Nullable<uint32_t> oDuration;
    Delegate * delegate = GetDelegate(ep);

    VerifyOrReturn(!isDelegateNull(delegate, ep));
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == OpenDuration::Get(ep, oDuration), ChipLogError(Zcl, "Valve OpenDuration is null"));
    if (oDuration.IsNull())
    {
        delegate->mRemainingDuration = 0;
        RemainingDuration::SetNull(ep);
        return;
    }

    if (delegate->mRemainingDuration > 0)
    {
        (void) chip::DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onValveConfigurationAndControlTick,
                                                           delegate);
        return;
    }
    else
    {
        ValveConfigurationAndControl::CloseValve(ep);
        (void) chip::DeviceLayer::SystemLayer().CancelTimer(onValveConfigurationAndControlTick, delegate);
    }
}

namespace chip {
namespace app {
namespace Clusters {
namespace ValveConfigurationAndControl {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, ValveConfigurationAndControl::Id,
                                                       EMBER_AF_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kValveConfigurationAndControlDelegateTableSize)
    {
        delegate->mEndpoint = endpoint;
        gDelegateTable[ep]  = delegate;
    }
}

Delegate * GetDefaultDelegate(EndpointId endpoint)
{
    return GetDelegate(endpoint);
}

CHIP_ERROR CloseValve(EndpointId ep)
{
    Delegate * delegate = GetDelegate(ep);
    DataModel::Nullable<uint64_t> autoCloseTime;
    DataModel::Nullable<uint32_t> rDuration;
    CHIP_ERROR attribute_error = CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);

    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == TargetState::Set(ep, ValveConfigurationAndControl::ValveStateEnum::kClosed),
                        attribute_error);
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == CurrentState::Set(ep, ValveConfigurationAndControl::ValveStateEnum::kClosed),
                        attribute_error);
    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kLevel))
    {
        VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == TargetLevel::SetNull(ep), attribute_error);
    }
    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kTimeSync) &&
        EMBER_ZCL_STATUS_SUCCESS == AutoCloseTime::Get(ep, autoCloseTime))
    {
        VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AutoCloseTime::SetNull(ep), attribute_error);
    }
    if (EMBER_ZCL_STATUS_SUCCESS == RemainingDuration::Get(ep, rDuration))
    {
        VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == RemainingDuration::SetNull(ep), attribute_error);
    }

    if (!isDelegateNull(delegate, ep))
    {
        delegate->HandleCloseValve();
        (void) chip::DeviceLayer::SystemLayer().CancelTimer(onValveConfigurationAndControlTick, delegate);
        delegate->mRemainingDuration = 0;
    }
    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kLevel))
    {
        // TODO not in current spec but proposed in
        // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/8575
        VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == CurrentLevel::Set(ep, chip::Percent(0)), attribute_error);
    }
    emitValveStateChangedEvent(ep, ValveConfigurationAndControl::ValveStateEnum::kClosed);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetValveLevel(chip::EndpointId ep, DataModel::Nullable<chip::Percent> level, chip::Optional<uint32_t> openDuration)
{
    Delegate * delegate     = GetDelegate(ep);
    Optional<Status> status = Optional<Status>::Missing();
    DataModel::Nullable<chip::Percent> openLevel;
    DataModel::Nullable<uint64_t> autoCloseTime;
    DataModel::Nullable<uint32_t> oDuration, rDuration;
    CHIP_ERROR attribute_error = CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);

    // set targetstate to open
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == TargetState::Set(ep, ValveConfigurationAndControl::ValveStateEnum::kOpen),
                        attribute_error);

    // if has level feature set targetlevel to openlevel
    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kLevel) && !level.IsNull())
    {
        VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == TargetLevel::Set(ep, level), attribute_error);
    }

    // if has timesync feature and autoclosetime available, set autoclosetime to current UTC + openduration field or attribute
    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kTimeSync) &&
        EMBER_ZCL_STATUS_SUCCESS == AutoCloseTime::Get(ep, autoCloseTime))
    {
        System::Clock::Microseconds64 utcTime;
        ReturnErrorOnFailure(System::SystemClock().GetClock_RealTime(utcTime));
        if (openDuration.HasValue())
        {
            oDuration.SetNonNull(openDuration.Value());
            // TODO OpenDuration attribute shall be set with the value from this field
            // https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/8560
        }
        else
        {
            VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == OpenDuration::Get(ep, oDuration), attribute_error);
        }

        if (!oDuration.IsNull())
        {
            uint64_t time = oDuration.Value() * chip::kMicrosecondsPerSecond;
            autoCloseTime.SetNonNull(utcTime.count() + time);
            VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AutoCloseTime::Set(ep, autoCloseTime), attribute_error);
        }
    }

    // if remainingduration available set to openduration field or attribute
    if (EMBER_ZCL_STATUS_SUCCESS == RemainingDuration::Get(ep, rDuration))
    {
        if (openDuration.HasValue())
        {
            rDuration.SetNonNull(openDuration.Value());
        }
        else
        {
            VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == OpenDuration::Get(ep, rDuration), attribute_error);
        }
        VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == RemainingDuration::Set(ep, rDuration), attribute_error);
    }

    if (openDuration.HasValue())
    {
        VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == OpenDuration::Set(ep, openDuration.Value()), attribute_error);
    }

    // start movement towards targets ( delegate )
    if (!isDelegateNull(delegate, ep))
    {
        delegate->HandleOpenValve(level);
        delegate->mRemainingDuration = oDuration.IsNull() ? 0 : oDuration.Value();
        startRemainingDurationTick(ep);
    }
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == CurrentState::Set(ep, ValveConfigurationAndControl::ValveStateEnum::kOpen),
                        attribute_error);
    emitValveStateChangedEvent(ep, ValveConfigurationAndControl::ValveStateEnum::kOpen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR EmitValveFault(EndpointId ep, chip::BitMask<ValveConfigurationAndControl::ValveFaultBitmap> fault)
{
    ReturnErrorOnFailure(emitValveFaultEvent(ep, fault));
    return CHIP_NO_ERROR;
}

} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip

bool emberAfValveConfigurationAndControlClusterOpenCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const ValveConfigurationAndControl::Commands::Open::DecodableType & commandData)
{
    const auto & openDuration = commandData.openDuration;
    const auto & ep           = commandPath.mEndpointId;
    DataModel::Nullable<chip::Percent> openLevel;

    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kLevel))
    {
        if (EMBER_ZCL_STATUS_SUCCESS == OpenLevel::Get(ep, openLevel))
        {
            // a null value SHALL indicate that the valve SHALL open to the TargetLevel value
            if (openLevel.IsNull() && EMBER_ZCL_STATUS_SUCCESS != TargetLevel::Get(ep, openLevel))
            {
                commandObj->AddStatus(commandPath, Status::Failure);
                return true;
            }
        }
    }

    if (CHIP_NO_ERROR == ValveConfigurationAndControl::SetValveLevel(ep, openLevel, openDuration))
    {
        commandObj->AddStatus(commandPath, Status::Success);
    }
    else
    {
        chip::BitMask<chip::app::Clusters::ValveConfigurationAndControl::ValveFaultBitmap> fault(
            ValveConfigurationAndControl::ValveFaultBitmap::kGeneralFault);
        emitValveFaultEvent(ep, fault);
        commandObj->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfValveConfigurationAndControlClusterCloseCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const ValveConfigurationAndControl::Commands::Close::DecodableType & commandData)
{
    const auto & ep = commandPath.mEndpointId;

    if (CHIP_NO_ERROR == ValveConfigurationAndControl::CloseValve(ep))
    {
        commandObj->AddStatus(commandPath, Status::Success);
    }
    else
    {
        commandObj->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfValveConfigurationAndControlClusterSetLevelCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const ValveConfigurationAndControl::Commands::SetLevel::DecodableType & commandData)
{
    const auto & ep = commandPath.mEndpointId;
    if (!HasFeature(ep, ValveConfigurationAndControl::Feature::kLevel))
    {
        return false;
    }

    const auto & level        = commandData.level;
    const auto & openDuration = commandData.openDuration;
    Optional<Status> status   = Optional<Status>::Missing();
    DataModel::Nullable<chip::Percent> cLevel;

    VerifyOrExit(EMBER_ZCL_STATUS_SUCCESS == CurrentLevel::Get(ep, cLevel), status.Emplace(Status::Failure));
    if (level == 0)
    {
        // if level is 0 and CurrentLevel is not 0, do Close command
        if (!cLevel.IsNull() && cLevel.Value() == 0)
            VerifyOrExit(EMBER_ZCL_STATUS_SUCCESS == TargetLevel::Set(ep, level), status.Emplace(Status::Failure));
        else
            VerifyOrExit(CHIP_NO_ERROR == ValveConfigurationAndControl::CloseValve(ep), status.Emplace(Status::Failure));
    }
    else
    {
        VerifyOrExit(
            CHIP_NO_ERROR ==
                ValveConfigurationAndControl::SetValveLevel(ep, DataModel::MakeNullable<chip::Percent>(level), openDuration),
            status.Emplace(Status::Failure));
    }

exit:
    if (status.HasValue())
    {
        commandObj->AddStatus(commandPath, status.Value());
    }
    else
    {
        commandObj->AddStatus(commandPath, Status::Success);
    }

    return true;
}

void MatterValveConfigurationAndControlClusterServerAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath)
{
    // TODO
}

void MatterValveConfigurationAndControlPluginServerInitCallback() {}
