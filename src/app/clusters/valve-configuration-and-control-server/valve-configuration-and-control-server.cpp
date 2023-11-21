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

static bool emitValveStateChangedEvent(EndpointId ep)
{
    ValveConfigurationAndControl::Events::ValveStateChanged::Type event;

    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to emit ValveStateChanged event [ep=%d]", ep);
        return false;
    }

    ChipLogProgress(Zcl, "Emit ValveStateChanged event [ep=%d]", ep);
    return true;
}

static bool emitValveFaultEvent(EndpointId ep)
{
    ValveConfigurationAndControl::Events::ValveFault::Type event;
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to emit ValveFault event [ep=%d]", ep);
        return false;
    }

    ChipLogProgress(Zcl, "Emit ValveFault event [ep=%d]", ep);
    return true;
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
        gDelegateTable[ep] = delegate;
    }
}

Delegate * GetDefaultDelegate(EndpointId endpoint)
{
    return GetDelegate(endpoint);
}

} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip

bool emberAfValveConfigurationAndControlClusterOpenCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const ValveConfigurationAndControl::Commands::Open::DecodableType & commandData)
{
    emitValveStateChangedEvent(commandPath.mEndpointId); // TODO remove
    emitValveFaultEvent(commandPath.mEndpointId);        // TODO remove

    const auto & openDuration = commandData.openDuration;
    const auto & ep           = commandPath.mEndpointId;

    Delegate * delegate     = GetDelegate(ep);
    Optional<Status> status = Optional<Status>::Missing();
    DataModel::Nullable<chip::Percent> openLevel;
    DataModel::Nullable<uint64_t> autoCloseTime;
    DataModel::Nullable<uint32_t> oDuration, rDuration;

    // set targetstate to open
    VerifyOrExit(EMBER_ZCL_STATUS_SUCCESS == TargetState::Set(ep, ValveConfigurationAndControl::ValveStateEnum::kOpen),
                 status.Emplace(Status::Failure));

    // if has level feature set targetlevel to openlevel
    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kLevel) &&
        EMBER_ZCL_STATUS_SUCCESS == OpenLevel::Get(ep, openLevel) && !openLevel.IsNull())
    {
        VerifyOrExit(EMBER_ZCL_STATUS_SUCCESS == TargetLevel::Set(ep, openLevel), status.Emplace(Status::Failure));
    }

    // if has timesync feature and autoclosetime available, set autoclosetime to current UTC + openduration field or attribute
    if (HasFeature(ep, ValveConfigurationAndControl::Feature::kTimeSync) &&
        EMBER_ZCL_STATUS_SUCCESS == AutoCloseTime::Get(ep, autoCloseTime))
    {
        System::Clock::Microseconds64 utcTime;
        VerifyOrExit(CHIP_NO_ERROR == System::SystemClock().GetClock_RealTime(utcTime), status.Emplace(Status::Failure));
        if (openDuration.HasValue())
        {
            oDuration.SetNonNull(openDuration.Value());
        }
        else
        {
            VerifyOrExit(EMBER_ZCL_STATUS_SUCCESS == OpenDuration::Get(ep, oDuration), status.Emplace(Status::Failure));
        }

        uint64_t time = oDuration.Value() * chip::kMicrosecondsPerSecond;
        autoCloseTime.SetNonNull(utcTime.count() + time);
        VerifyOrExit(EMBER_ZCL_STATUS_SUCCESS == AutoCloseTime::Set(ep, autoCloseTime), status.Emplace(Status::Failure));
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
            VerifyOrExit(EMBER_ZCL_STATUS_SUCCESS == OpenDuration::Get(ep, rDuration), status.Emplace(Status::Failure));
        }
        VerifyOrExit(EMBER_ZCL_STATUS_SUCCESS == RemainingDuration::Set(ep, rDuration), status.Emplace(Status::Failure));
    }

    if (openDuration.HasValue())
    {
        VerifyOrExit(EMBER_ZCL_STATUS_SUCCESS == OpenDuration::Set(ep, openDuration.Value()), status.Emplace(Status::Failure));
    }

    // start movement towards targets ( delegate )
    if (!isDelegateNull(delegate, commandPath.mEndpointId))
    {
        delegate->HandleOpenValve();
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

bool emberAfValveConfigurationAndControlClusterCloseCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const ValveConfigurationAndControl::Commands::Close::DecodableType & commandData)
{
    return true;
}

bool emberAfValveConfigurationAndControlClusterSetLevelCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const ValveConfigurationAndControl::Commands::SetLevel::DecodableType & commandData)
{
    return true;
}

void MatterValveConfigurationAndControlPluginServerInitCallback() {}
