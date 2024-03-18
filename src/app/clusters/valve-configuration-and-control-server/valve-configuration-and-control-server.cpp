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

#include <app/util/config.h>
#ifdef ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER
// Need the `nogncheck` because it's inter-cluster dependency and this
// breaks GN deps checks since that doesn't know how to deal with #ifdef'd includes :(.
#include "app/clusters/time-synchronization-server/time-synchronization-server.h" // nogncheck

#endif // ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/data-model/Encode.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ValveConfigurationAndControl::Attributes;
using chip::app::Clusters::ValveConfigurationAndControl::Delegate;
using chip::Protocols::InteractionModel::Status;

static constexpr size_t kValveConfigurationAndControlDelegateTableSize =
    MATTER_DM_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kValveConfigurationAndControlDelegateTableSize <= kEmberInvalidEndpointIndex,
              "ValveConfigurationAndControl Delegate table size error");

namespace {

struct RemainingDurationTable
{
    EndpointId endpoint;
    DataModel::Nullable<uint32_t> remainingDuration;
};

RemainingDurationTable gRemainingDuration[kValveConfigurationAndControlDelegateTableSize];
Delegate * gDelegateTable[kValveConfigurationAndControlDelegateTableSize] = { nullptr };

bool GetRemainingDuration(EndpointId endpoint, DataModel::Nullable<uint32_t> & duration)
{
    uint16_t epIdx = emberAfGetClusterServerEndpointIndex(endpoint, ValveConfigurationAndControl::Id,
                                                          MATTER_DM_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    VerifyOrReturnValue(epIdx < kValveConfigurationAndControlDelegateTableSize, false);
    duration = gRemainingDuration[epIdx].remainingDuration;
    return true;
}

void SetRemainingDuration(EndpointId endpoint, DataModel::Nullable<uint32_t> duration)
{
    uint16_t epIdx = emberAfGetClusterServerEndpointIndex(endpoint, ValveConfigurationAndControl::Id,
                                                          MATTER_DM_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (epIdx < kValveConfigurationAndControlDelegateTableSize)
    {
        gRemainingDuration[epIdx].endpoint          = endpoint;
        gRemainingDuration[epIdx].remainingDuration = duration;
    }
}

void SetRemainingDurationNull(EndpointId endpoint)
{
    uint16_t epIdx = emberAfGetClusterServerEndpointIndex(endpoint, ValveConfigurationAndControl::Id,
                                                          MATTER_DM_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (epIdx < kValveConfigurationAndControlDelegateTableSize)
    {
        if (!gRemainingDuration[epIdx].remainingDuration.IsNull())
        {
            MatterReportingAttributeChangeCallback(endpoint, ValveConfigurationAndControl::Id, RemainingDuration::Id);
        }
        gRemainingDuration[epIdx].remainingDuration.SetNull();
    }
}

RemainingDurationTable * GetRemainingDurationItem(EndpointId endpoint)
{
    uint16_t epIdx = emberAfGetClusterServerEndpointIndex(endpoint, ValveConfigurationAndControl::Id,
                                                          MATTER_DM_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (epIdx < kValveConfigurationAndControlDelegateTableSize)
    {
        return &gRemainingDuration[epIdx];
    }
    return nullptr;
}

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t epIdx = emberAfGetClusterServerEndpointIndex(endpoint, ValveConfigurationAndControl::Id,
                                                          MATTER_DM_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (epIdx >= kValveConfigurationAndControlDelegateTableSize ? nullptr : gDelegateTable[epIdx]);
}

bool isDelegateNull(Delegate * delegate)
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
    DataModel::Nullable<uint32_t> rDuration;
    VerifyOrReturnError(GetRemainingDuration(endpoint, rDuration), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));

    return aEncoder.Encode(rDuration);
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

    ChipLogProgress(Zcl, "Emit ValveStateChanged event [ep=%d] %d", ep, to_underlying(state));
    return true;
}

static CHIP_ERROR emitValveFaultEvent(EndpointId ep, BitMask<ValveConfigurationAndControl::ValveFaultBitmap> fault)
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

static void onValveConfigurationAndControlTick(System::Layer * systemLayer, void * data)
{
    RemainingDurationTable * item = reinterpret_cast<RemainingDurationTable *>(data);
    VerifyOrReturn(item != nullptr, ChipLogError(Zcl, "Error retrieving RemainingDuration item"));

    DataModel::Nullable<uint32_t> rDuration = item->remainingDuration;
    VerifyOrReturn(!rDuration.IsNull());

    EndpointId ep = item->endpoint;

    if (rDuration.Value() > 0)
    {
        SetRemainingDuration(ep, DataModel::MakeNullable<uint32_t>(--rDuration.Value()));
        startRemainingDurationTick(ep);
    }
    else
    {
        SetRemainingDurationNull(ep);
    }
}

void startRemainingDurationTick(EndpointId ep)
{
    RemainingDurationTable * item = GetRemainingDurationItem(ep);
    VerifyOrReturn(item != nullptr, ChipLogError(Zcl, "Error retrieving RemainingDuration item"));

    DataModel::Nullable<uint32_t> rDuration = item->remainingDuration;
    VerifyOrReturn(!rDuration.IsNull());
    Delegate * delegate = GetDelegate(item->endpoint);
    VerifyOrReturn(!isDelegateNull(delegate));

    delegate->HandleRemainingDurationTick(rDuration.Value());
    if (rDuration.Value() > 0)
    {
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onValveConfigurationAndControlTick, item);
    }
    else
    {
        ValveConfigurationAndControl::CloseValve(ep);
        (void) DeviceLayer::SystemLayer().CancelTimer(onValveConfigurationAndControlTick, item);
    }
}

namespace chip {
namespace app {
namespace Clusters {
namespace ValveConfigurationAndControl {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, ValveConfigurationAndControl::Id,
                                                       MATTER_DM_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kValveConfigurationAndControlDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
}

Delegate * GetDefaultDelegate(EndpointId endpoint)
{
    return GetDelegate(endpoint);
}

CHIP_ERROR CloseValve(EndpointId ep)
{
    Delegate * delegate = GetDelegate(ep);
    DataModel::Nullable<uint32_t> rDuration;
    CHIP_ERROR attribute_error = CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);

    VerifyOrReturnError(Status::Success == TargetState::Set(ep, ValveConfigurationAndControl::ValveStateEnum::kClosed),
                        attribute_error);
    VerifyOrReturnError(Status::Success == CurrentState::Set(ep, ValveConfigurationAndControl::ValveStateEnum::kTransitioning),
                        attribute_error);
    VerifyOrReturnError(Status::Success == OpenDuration::SetNull(ep), attribute_error);
    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kLevel))
    {
        VerifyOrReturnError(Status::Success == TargetLevel::Set(ep, 0), attribute_error);
    }
    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kTimeSync))
    {
        VerifyOrReturnError(Status::Success == AutoCloseTime::SetNull(ep), attribute_error);
    }
    SetRemainingDurationNull(ep);
    RemainingDurationTable * item = GetRemainingDurationItem(ep);
    (void) DeviceLayer::SystemLayer().CancelTimer(onValveConfigurationAndControlTick, item);

    emitValveStateChangedEvent(ep, ValveConfigurationAndControl::ValveStateEnum::kTransitioning);
    if (!isDelegateNull(delegate))
    {
        delegate->HandleCloseValve();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetValveLevel(EndpointId ep, DataModel::Nullable<Percent> level, DataModel::Nullable<uint32_t> openDuration)
{
    Delegate * delegate     = GetDelegate(ep);
    Optional<Status> status = Optional<Status>::Missing();
    DataModel::Nullable<Percent> openLevel;
    DataModel::Nullable<uint64_t> autoCloseTime;
    CHIP_ERROR attribute_error = CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);

    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kTimeSync))
    {
#ifdef ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER
        if (!openDuration.IsNull() &&
            TimeSynchronization::TimeSynchronizationServer::Instance().GetGranularity() !=
                TimeSynchronization::GranularityEnum::kNoTimeGranularity)
        {
            System::Clock::Microseconds64 utcTime;
            uint64_t chipEpochTime;
            ReturnErrorOnFailure(System::SystemClock().GetClock_RealTime(utcTime));
            VerifyOrReturnError(UnixEpochToChipEpochMicros(utcTime.count(), chipEpochTime), CHIP_ERROR_INVALID_TIME);

            uint64_t time = openDuration.Value() * chip::kMicrosecondsPerSecond;
            autoCloseTime.SetNonNull(chipEpochTime + time);
            VerifyOrReturnError(Status::Success == AutoCloseTime::Set(ep, autoCloseTime), attribute_error);
        }
        else
        {
            VerifyOrReturnError(Status::Success == AutoCloseTime::SetNull(ep), attribute_error);
        }
#else
        return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER
    }

    // level can only be null if LVL feature is not supported
    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kLevel) && !level.IsNull())
    {
        VerifyOrReturnError(Status::Success == TargetLevel::Set(ep, level), attribute_error);
    }

    VerifyOrReturnError(Status::Success == OpenDuration::Set(ep, openDuration), attribute_error);

    SetRemainingDuration(ep, openDuration);
    // Trigger report for remainingduration
    MatterReportingAttributeChangeCallback(ep, ValveConfigurationAndControl::Id, RemainingDuration::Id);
    // set targetstate to open
    VerifyOrReturnError(Status::Success == TargetState::Set(ep, ValveConfigurationAndControl::ValveStateEnum::kOpen),
                        attribute_error);
    VerifyOrReturnError(Status::Success == CurrentState::Set(ep, ValveConfigurationAndControl::ValveStateEnum::kTransitioning),
                        attribute_error);

    // start movement towards target
    emitValveStateChangedEvent(ep, ValveConfigurationAndControl::ValveStateEnum::kTransitioning);
    if (!isDelegateNull(delegate))
    {
        DataModel::Nullable<Percent> cLevel = delegate->HandleOpenValve(level);
        if (HasFeature(ep, ValveConfigurationAndControl::Feature::kLevel))
        {
            VerifyOrReturnError(Status::Success == CurrentLevel::Set(ep, cLevel), attribute_error);
        }
    }
    // start countdown
    startRemainingDurationTick(ep);

    return CHIP_NO_ERROR;
}

CHIP_ERROR UpdateCurrentLevel(EndpointId ep, Percent currentLevel)
{
    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kLevel))
    {
        VerifyOrReturnError(Status::Success == CurrentLevel::Set(ep, currentLevel), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        return CHIP_NO_ERROR;
    }
    return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
}

CHIP_ERROR UpdateCurrentState(EndpointId ep, ValveConfigurationAndControl::ValveStateEnum currentState)
{
    VerifyOrReturnError(Status::Success == CurrentState::Set(ep, currentState), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    emitValveStateChangedEvent(ep, currentState);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EmitValveFault(EndpointId ep, BitMask<ValveConfigurationAndControl::ValveFaultBitmap> fault)
{
    ReturnErrorOnFailure(emitValveFaultEvent(ep, fault));
    return CHIP_NO_ERROR;
}

void UpdateAutoCloseTime(uint64_t time)
{
    for (auto & t : gRemainingDuration)
    {
        const auto & d = t.remainingDuration;
        if (!d.IsNull() && d.Value() != 0)
        {
            uint64_t closingTime = d.Value() * chip::kMicrosecondsPerSecond + time;
            if (Status::Success != AutoCloseTime::Set(t.endpoint, closingTime))
            {
                ChipLogError(Zcl, "Unable to update AutoCloseTime");
            }
        }
    }
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
    DataModel::Nullable<Percent> level;
    DataModel::Nullable<uint32_t> duration;
    BitMask<ValveConfigurationAndControl::ValveFaultBitmap> fault(0);
    Optional<Status> status = Optional<Status>::Missing();

    // if fault is registered return FailureDueToFault
    if (Status::Success == ValveFault::Get(ep, &fault) && fault.HasAny())
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
        VerifyOrExit(Status::Success == DefaultOpenDuration::Get(ep, duration), status.Emplace(Status::Failure));
    }

    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kLevel))
    {
        Percent defOpenLevel;
        if (targetLevel.HasValue())
        {
            level.SetNonNull(targetLevel.Value());
        }
        else if (Status::Success == DefaultOpenLevel::Get(ep, &defOpenLevel))
        {
            level.SetNonNull(defOpenLevel);
        }
        else
        {
            level.SetNonNull(Percent(100));
        }
    }

    VerifyOrExit(CHIP_NO_ERROR == ValveConfigurationAndControl::SetValveLevel(ep, level, duration),
                 status.Emplace(Status::Failure));

exit:
    if (status.HasValue())
    {
        BitMask<ValveConfigurationAndControl::ValveFaultBitmap> gFault(
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
    BitMask<ValveConfigurationAndControl::ValveFaultBitmap> fault(0);

    // if fault is registered return FailureDueToFault
    if (Status::Success == ValveFault::Get(ep, &fault) && fault.HasAny())
    {
        commandObj->AddClusterSpecificFailure(commandPath,
                                              to_underlying(ValveConfigurationAndControl::StatusCodeEnum::kFailureDueToFault));
        return true;
    }

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

void MatterValveConfigurationAndControlPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
