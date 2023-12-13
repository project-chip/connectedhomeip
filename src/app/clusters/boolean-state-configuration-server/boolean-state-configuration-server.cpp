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

#include "boolean-state-configuration-server.h"

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
using namespace chip::app::Clusters::BooleanStateConfiguration::Attributes;
using chip::app::Clusters::BooleanStateConfiguration::Delegate;
using chip::Protocols::InteractionModel::Status;

static constexpr size_t kBooleanStateConfigurationDelegateTableSize =
    EMBER_AF_BOOLEAN_STATE_CONFIGURATION_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kBooleanStateConfigurationDelegateTableSize <= kEmberInvalidEndpointIndex,
              "BooleanStateConfiguration Delegate table size error");

namespace {
Delegate * gDelegateTable[kBooleanStateConfigurationDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, BooleanStateConfiguration::Id,
                                                       EMBER_AF_BOOLEAN_STATE_CONFIGURATION_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kBooleanStateConfigurationDelegateTableSize ? nullptr : gDelegateTable[ep]);
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

static bool emitAlarmsStateChangedEvent(EndpointId ep)
{
    if (!HasFeature(ep, BooleanStateConfiguration::Feature::kAudible) &&
        !HasFeature(ep, BooleanStateConfiguration::Feature::kVisual))
    {
        return false;
    }

    BooleanStateConfiguration::Events::AlarmsStateChanged::Type event;
    BitMask<BooleanStateConfiguration::AlarmModeBitmap> active;
    VerifyOrReturnValue(EMBER_ZCL_STATUS_SUCCESS == AlarmsActive::Get(ep, &active), false);
    event.alarmsActive = active;

    if (HasFeature(ep, BooleanStateConfiguration::Feature::kAlarmSuppress))
    {
        BitMask<BooleanStateConfiguration::AlarmModeBitmap> suppressed;
        VerifyOrReturnValue(EMBER_ZCL_STATUS_SUCCESS == AlarmsSuppressed::Get(ep, &suppressed), false);
        event.alarmsSuppressed.SetValue(suppressed);
    }

    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to emit AlarmsStateChanged event [ep=%d]", ep);
        return false;
    }

    ChipLogProgress(Zcl, "Emit AlarmsStateChanged event [ep=%d]", ep);
    return true;
}

static CHIP_ERROR emitSensorFaultEvent(EndpointId ep)
{
    BooleanStateConfiguration::Events::SensorFault::Type event;
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to emit SensorFault event [ep=%d]", ep);
        return error;
    }

    ChipLogProgress(Zcl, "Emit SensorFault event [ep=%d]", ep);
    return CHIP_NO_ERROR;
}

namespace chip {
namespace app {
namespace Clusters {
namespace BooleanStateConfiguration {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, BooleanStateConfiguration::Id,
                                                       EMBER_AF_BOOLEAN_STATE_CONFIGURATION_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kBooleanStateConfigurationDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
}

Delegate * GetDefaultDelegate(EndpointId endpoint)
{
    return GetDelegate(endpoint);
}

CHIP_ERROR SetAlarmsActive(EndpointId ep, BitMask<BooleanStateConfiguration::AlarmModeBitmap> alarms)
{
    if (HasFeature(ep, BooleanStateConfiguration::Feature::kVisual) || HasFeature(ep, BooleanStateConfiguration::Feature::kAudible))
    {
        BitMask<BooleanStateConfiguration::AlarmModeBitmap> alarmsEnabled;
        VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AlarmsEnabled::Get(ep, &alarmsEnabled),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
        if (!alarmsEnabled.Has(alarms))
        {
            return CHIP_NO_ERROR;
        }
    }

    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AlarmsActive::Set(ep, alarms), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    emitAlarmsStateChangedEvent(ep);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClearAllAlarms(EndpointId ep)
{
    BitMask<BooleanStateConfiguration::AlarmModeBitmap> alarmsActive, alarmsSuppressed;
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AlarmsActive::Get(ep, &alarmsActive),
                        CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AlarmsSuppressed::Get(ep, &alarmsSuppressed),
                        CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));

    if (alarmsActive.HasAny() || alarmsSuppressed.HasAny())
    {
        alarmsActive.ClearAll();
        alarmsSuppressed.ClearAll();
        VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AlarmsActive::Set(ep, alarmsActive),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
        VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AlarmsSuppressed::Set(ep, alarmsSuppressed),
                            CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
        emitAlarmsStateChangedEvent(ep);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR SuppressAlarms(EndpointId ep, BitMask<BooleanStateConfiguration::AlarmModeBitmap> alarm)
{
    CHIP_ERROR attribute_error = CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);

    if (!HasFeature(ep, BooleanStateConfiguration::Feature::kAlarmSuppress))
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedCommand);
    }

    BitMask<BooleanStateConfiguration::AlarmModeBitmap> alarmsActive, alarmsSuppressed;

    if (HasFeature(ep, BooleanStateConfiguration::Feature::kVisual) || HasFeature(ep, BooleanStateConfiguration::Feature::kAudible))
    {
        VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AlarmsActive::Get(ep, &alarmsActive), attribute_error);
        if (!alarmsActive.Has(alarm))
        {
            return CHIP_IM_GLOBAL_STATUS(InvalidInState);
        }
    }
    else
    {
        return CHIP_IM_GLOBAL_STATUS(InvalidInState);
    }

    Delegate * delegate = GetDelegate(ep);
    if (!isDelegateNull(delegate, ep))
    {
        delegate->HandleSuppressAlarm(alarm);
    }

    alarmsActive.Clear(alarm);
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AlarmsActive::Set(ep, alarmsActive), attribute_error);

    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AlarmsSuppressed::Get(ep, &alarmsSuppressed), attribute_error);
    alarmsSuppressed.Set(alarm);
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AlarmsSuppressed::Set(ep, alarmsSuppressed), attribute_error);

    emitAlarmsStateChangedEvent(ep);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetCurrentSensitivityLevel(EndpointId ep, uint8_t level)
{
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == CurrentSensitivityLevel::Set(ep, level),
                        CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    return CHIP_NO_ERROR;
}

CHIP_ERROR EmitSensorFault(EndpointId ep)
{
    ReturnErrorOnFailure(emitSensorFaultEvent(ep));
    return CHIP_NO_ERROR;
}

} // namespace BooleanStateConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip

bool emberAfBooleanStateConfigurationClusterSuppressAlarmCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const BooleanStateConfiguration::Commands::SuppressAlarm::DecodableType & commandData)
{
    const auto & alarm = commandData.alarmsToSuppress;
    CHIP_ERROR err     = BooleanStateConfiguration::SuppressAlarms(commandPath.mEndpointId, alarm);
    if (err == CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Status::Success);
    }
    else if (err == CHIP_IM_GLOBAL_STATUS(InvalidInState))
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
    }
    else if (err == CHIP_IM_GLOBAL_STATUS(UnsupportedCommand))
    {
        return false;
    }
    else
    {
        commandObj->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfBooleanStateConfigurationClusterEnableDisableAlarmCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const BooleanStateConfiguration::Commands::EnableDisableAlarm::DecodableType & commandData)
{
    return true;
}

void MatterBooleanStateConfigurationPluginServerInitCallback() {}
