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

#include "boolean-sensor-configuration-server.h"

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
using namespace chip::app::Clusters::BooleanSensorConfiguration::Attributes;
using chip::app::Clusters::BooleanSensorConfiguration::Delegate;
using chip::Protocols::InteractionModel::Status;

static constexpr size_t kBooleanSensorConfigurationDelegateTableSize =
    EMBER_AF_BOOLEAN_SENSOR_CONFIGURATION_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

static_assert(kBooleanSensorConfigurationDelegateTableSize <= kEmberInvalidEndpointIndex,
              "BooleanSensorConfiguration Delegate table size error");

namespace {
Delegate * gDelegateTable[kBooleanSensorConfigurationDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, BooleanSensorConfiguration::Id,
                                                       EMBER_AF_BOOLEAN_SENSOR_CONFIGURATION_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kBooleanSensorConfigurationDelegateTableSize ? nullptr : gDelegateTable[ep]);
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
    BooleanSensorConfiguration::Events::AlarmsStateChanged::Type event;
    chip::BitMask<BooleanSensorConfiguration::AlarmModeBitmap> active;
    VerifyOrReturnValue(EMBER_ZCL_STATUS_SUCCESS == AlarmsActive::Get(ep, &active), false);
    event.alarmsActive = active;

    // TODO add compile time feature-map check to reduce code size
    if (HasFeature(ep, BooleanSensorConfiguration::Feature::kAlarmSuppress))
    {
        chip::BitMask<BooleanSensorConfiguration::AlarmModeBitmap> suppressed;
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

static bool emitSensorFaultEvent(EndpointId ep)
{
    BooleanSensorConfiguration::Events::SensorFault::Type event;
    EventNumber eventNumber;

    CHIP_ERROR error = LogEvent(event, ep, eventNumber);

    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Zcl, "Unable to emit SensorFault event [ep=%d]", ep);
        return false;
    }

    ChipLogProgress(Zcl, "Emit SensorFault event [ep=%d]", ep);
    return true;
}

namespace chip {
namespace app {
namespace Clusters {
namespace BooleanSensorConfiguration {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, BooleanSensorConfiguration::Id,
                                                       EMBER_AF_BOOLEAN_SENSOR_CONFIGURATION_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kBooleanSensorConfigurationDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
}

Delegate * GetDefaultDelegate(EndpointId endpoint)
{
    return GetDelegate(endpoint);
}

CHIP_ERROR SetAlarmsActive(EndpointId ep, chip::BitMask<BooleanSensorConfiguration::AlarmModeBitmap> alarms)
{
    if (HasFeature(ep, BooleanSensorConfiguration::Feature::kVisual) ||
        HasFeature(ep, BooleanSensorConfiguration::Feature::kAudible))
    {
        chip::BitMask<BooleanSensorConfiguration::AlarmModeBitmap, uint8_t> alarmsEnabled;
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
    chip::BitMask<BooleanSensorConfiguration::AlarmModeBitmap, uint8_t> alarmsActive, alarmsSuppressed;
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

CHIP_ERROR SuppressAlarms(EndpointId ep, chip::BitMask<BooleanSensorConfiguration::AlarmModeBitmap> alarm)
{
    CHIP_ERROR attribute_error = CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);

    if (!HasFeature(ep, BooleanSensorConfiguration::Feature::kAlarmSuppress))
    {
        return attribute_error;
    }

    chip::BitMask<BooleanSensorConfiguration::AlarmModeBitmap, uint8_t> alarmsActive, alarmsSuppressed;

    if (HasFeature(ep, BooleanSensorConfiguration::Feature::kVisual) ||
        HasFeature(ep, BooleanSensorConfiguration::Feature::kAudible))
    {
        VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AlarmsActive::Get(ep, &alarmsActive), attribute_error);
        if (!alarmsActive.Has(alarm))
        {
            CHIP_IM_GLOBAL_STATUS(InvalidInState);
        }
    }
    else
    {
        CHIP_IM_GLOBAL_STATUS(InvalidInState);
    }

    Delegate * delegate = GetDelegate(ep);
    if (delegate != nullptr)
    {
        delegate->HandleSuppressRequest(alarm);
    }

    alarmsActive.Clear(alarm);
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AlarmsActive::Set(ep, alarmsActive), attribute_error);

    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AlarmsSuppressed::Get(ep, &alarmsSuppressed), attribute_error);
    alarmsSuppressed.Set(alarm);
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == AlarmsSuppressed::Set(ep, alarmsSuppressed), attribute_error);

    emitAlarmsStateChangedEvent(ep);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetSensitivityLevel(EndpointId ep, BooleanSensorConfiguration::SensitivityEnum level)
{
    VerifyOrReturnError(EMBER_ZCL_STATUS_SUCCESS == SensitivityLevel::Set(ep, level), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    return CHIP_NO_ERROR;
}

bool EmitSensorFault(EndpointId ep)
{
    return emitSensorFaultEvent(ep);
}

} // namespace BooleanSensorConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip

namespace {

class BooleanSensorConfigurationAttrAccess : public AttributeAccessInterface
{
public:
    BooleanSensorConfigurationAttrAccess() :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), BooleanSensorConfiguration::Id)
    {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadSensitivityLevel(AttributeValueEncoder & aEncoder, Delegate * delegate);
};

BooleanSensorConfigurationAttrAccess gBooleanSensorConfigurationAttrAccess;

CHIP_ERROR BooleanSensorConfigurationAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    if (isDelegateNull(delegate, endpoint))
    {
        return CHIP_NO_ERROR;
    }

    switch (aPath.mAttributeId)
    {
    case SensitivityLevel::Id: {
        return ReadSensitivityLevel(aEncoder, delegate);
    }
    break;

    default:
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BooleanSensorConfigurationAttrAccess::ReadSensitivityLevel(AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    // TODO this needs to be implemented if the type of this attribute type changes to struct in
    // https://github.com/CHIP-Specifications/connectedhomeip-spec/pull/7845
    return CHIP_NO_ERROR;
}
} // namespace

bool emberAfBooleanSensorConfigurationClusterSuppressRequestCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const BooleanSensorConfiguration::Commands::SuppressRequest::DecodableType & commandData)
{
    const auto & alarm = commandData.alarmsToSuppress;
    CHIP_ERROR err     = BooleanSensorConfiguration::SuppressAlarms(commandPath.mEndpointId, alarm);
    if (err == CHIP_IM_GLOBAL_STATUS(InvalidInState))
    {
        commandObj->AddStatus(commandPath, Status::InvalidInState);
        return true;
    }
    else if (err == CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute))
    {
        return false;
    }

    commandObj->AddStatus(commandPath, Status::Success);
    return true;
}

void MatterBooleanSensorConfigurationPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gBooleanSensorConfigurationAttrAccess);
}
