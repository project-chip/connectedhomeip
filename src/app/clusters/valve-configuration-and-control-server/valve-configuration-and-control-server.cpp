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

DataModel::Nullable<uint32_t> gRemainingDuration[kValveConfigurationAndControlDelegateTableSize];
Delegate * gDelegateTable[kValveConfigurationAndControlDelegateTableSize] = { nullptr };

DataModel::Nullable<uint32_t> GetRemainingDuration(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, ValveConfigurationAndControl::Id,
                                                       EMBER_AF_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kValveConfigurationAndControlDelegateTableSize ? DataModel::Nullable<uint32_t>() : gRemainingDuration[ep]);
}

void SetRemainingDuration(EndpointId endpoint, DataModel::Nullable<uint32_t> duration)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, ValveConfigurationAndControl::Id,
                                                       EMBER_AF_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kValveConfigurationAndControlDelegateTableSize)
    {
        gRemainingDuration[ep] = duration;
    }
}

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

class ValveConfigAndControlAttrAccess : public AttributeAccessInterface
{
public:
    ValveConfigAndControlAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), ValveConfigurationAndControl::Id)
    {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadRemainingDuration(EndpointId endpoint, AttributeValueEncoder & aEncoder);
};

ValveConfigAndControlAttrAccess gAttrAccess;

CHIP_ERROR ValveConfigAndControlAttrAccess::ReadRemainingDuration(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(GetRemainingDuration(endpoint));
}

CHIP_ERROR ValveConfigAndControlAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aPath.mClusterId != ValveConfigurationAndControl::Id)
    {
        return CHIP_ERROR_INVALID_PATH_LIST;
    }

    switch (aPath.mAttributeId)
    {
    case RemainingDuration::Id: {
        return ReadRemainingDuration(aPath.mEndpointId, aEncoder);
    }
    default: {
        break;
    }
    }

    return err;
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
    VerifyOrReturn(delegate != nullptr);

    EndpointId ep = delegate->mEndpoint;

    if (delegate->mRemainingDuration > 0)
    {
        delegate->mRemainingDuration--;
        if (delegate->mRemainingDuration < 5 || !(delegate->mRemainingDuration % 5))
        {
            SetRemainingDuration(ep, DataModel::MakeNullable<uint32_t>(delegate->mRemainingDuration));
        }
        startRemainingDurationTick(ep);
    }
    else
    {
        SetRemainingDuration(ep, DataModel::MakeNullable<uint32_t>());
    }
}

void startRemainingDurationTick(EndpointId ep)
{
    DataModel::Nullable<uint32_t> oDuration;
    Delegate * delegate = GetDelegate(ep);

    VerifyOrReturn(!isDelegateNull(delegate, ep));
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == OpenDuration::Get(ep, oDuration), ChipLogError(Zcl, "Error reading OpenDuration"));
    if (oDuration.IsNull())
    {
        delegate->mRemainingDuration = 0;
        return;
    }

    if (delegate->mRemainingDuration > 0)
    {
        (void) chip::DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onValveConfigurationAndControlTick,
                                                           delegate);
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
    SetRemainingDuration(ep, DataModel::MakeNullable<uint32_t>());

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

CHIP_ERROR SetValveLevel(chip::EndpointId ep, DataModel::Nullable<chip::Percent> level, DataModel::Nullable<uint32_t> openDuration)
{
    Delegate * delegate     = GetDelegate(ep);
    Optional<Status> status = Optional<Status>::Missing();
    DataModel::Nullable<chip::Percent> openLevel;
    DataModel::Nullable<uint64_t> autoCloseTime;
    CHIP_ERROR attribute_error = CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);

    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == OpenDuration::Set(ep, openDuration), attribute_error);
    SetRemainingDuration(ep, openDuration);

    // if has timesync feature and autoclosetime available, set autoclosetime to current UTC + openduration field or attribute
    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kTimeSync) &&
        EMBER_ZCL_STATUS_SUCCESS == AutoCloseTime::Get(ep, autoCloseTime))
    {
        System::Clock::Microseconds64 utcTime;
        ReturnErrorOnFailure(System::SystemClock().GetClock_RealTime(utcTime));

        if (!openDuration.IsNull())
        {
            uint64_t time = openDuration.Value() * chip::kMicrosecondsPerSecond;
            autoCloseTime.SetNonNull(utcTime.count() + time);
            VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AutoCloseTime::Set(ep, autoCloseTime), attribute_error);
        }
        else
        {
            VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AutoCloseTime::SetNull(ep), attribute_error);
        }
    }

    // level can only be null if LVL feature is not supported
    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kLevel) && !level.IsNull())
    {
        VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == TargetLevel::Set(ep, level), attribute_error);
    }

    // start movement towards target ( delegate )
    if (!isDelegateNull(delegate, ep))
    {
        delegate->mRemainingDuration = openDuration.IsNull() ? 0 : openDuration.Value();
        startRemainingDurationTick(ep);
        DataModel::Nullable<chip::Percent> cLevel = delegate->HandleOpenValve(level);
        if (HasFeature(ep, ValveConfigurationAndControl::Feature::kLevel))
        {
            VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == CurrentLevel::Set(ep, level), attribute_error);
        }
    }

    // set targetstate to open
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == TargetState::Set(ep, ValveConfigurationAndControl::ValveStateEnum::kOpen),
                        attribute_error);
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS ==
                            CurrentState::Set(ep, ValveConfigurationAndControl::ValveStateEnum::kTransitioning),
                        attribute_error);

    // if has level feature set targetlevel to openlevel

    // start movement and countdown
    // if level feature, update currentLevel at the start of movement
    // when movement is complete, update currentState to Open
    emitValveStateChangedEvent(ep, ValveConfigurationAndControl::ValveStateEnum::kTransitioning);

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
    const auto & targetLevel  = commandData.targetLevel;
    const auto & ep           = commandPath.mEndpointId;
    DataModel::Nullable<chip::Percent> level;
    DataModel::Nullable<uint32_t> duration;
    BitMask<ValveConfigurationAndControl::ValveFaultBitmap> fault(0);
    Optional<Status> status = Optional<Status>::Missing();

    // if fault is registered return FailureDueToFault
    if (EMBER_ZCL_STATUS_SUCCESS == ValveFault::Get(ep, &fault) && fault.HasAny())
    {
        commandObj->AddClusterSpecificFailure(commandPath,
                                              to_underlying(ValveConfigurationAndControl::StatusCodeEnum::kFailureDueToFault));
        return true;
    }

    // verify min 1 requirement
    VerifyOrExit(targetLevel.HasValue() ? targetLevel.Value() > 0 : true, status.Emplace(Status::ConstraintError));
    if (openDuration.HasValue())
    {
        bool validOpenDuration = openDuration.Value().IsNull() ? true : openDuration.Value().Value() > 0;
        // verify min 1 requirement
        VerifyOrExit(validOpenDuration, status.Emplace(Status::ConstraintError));

        duration = openDuration.Value();
    }
    else
    {
        VerifyOrExit(EMBER_ZCL_STATUS_SUCCESS == DefaultOpenDuration::Get(ep, duration), status.Emplace(Status::Failure));
    }

    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kLevel))
    {
        chip::Percent defOpenLevel;
        if (targetLevel.HasValue())
        {
            level.SetNonNull(targetLevel.Value());
        }
        else if (EMBER_ZCL_STATUS_SUCCESS == DefaultOpenLevel::Get(ep, &defOpenLevel))
        {
            level.SetNonNull(defOpenLevel);
        }
        else
        {
            level.SetNonNull(chip::Percent(100));
        }
    }

    VerifyOrExit(CHIP_NO_ERROR == ValveConfigurationAndControl::SetValveLevel(ep, level, duration),
                 status.Emplace(Status::Failure));

exit:
    if (status.HasValue())
    {
        BitMask<chip::app::Clusters::ValveConfigurationAndControl::ValveFaultBitmap> gFault(
            ValveConfigurationAndControl::ValveFaultBitmap::kGeneralFault);
        emitValveFaultEvent(ep, gFault);
        commandObj->AddStatus(commandPath, status.Value());
    }
    else
    {
        commandObj->AddStatus(commandPath, Status::Success);
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

#if 0
bool emberAfValveConfigurationAndControlClusterSetLevelCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const ValveConfigurationAndControl::Commands::SetLevel::DecodableType & commandData)
{
    const auto & level        = commandData.level;
    const auto & openDuration = commandData.openDuration;
    const auto & ep = commandPath.mEndpointId;
    Optional<Status> status   = Optional<Status>::Missing();
    DataModel::Nullable<chip::Percent> cLevel;
	chip::Percent defOpenLevel;



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
#endif

void MatterValveConfigurationAndControlClusterServerAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath)
{
    // TODO
}

void MatterValveConfigurationAndControlPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
