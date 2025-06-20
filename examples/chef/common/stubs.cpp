#include "DeviceTypes.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app/data-model/Nullable.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <lib/core/DataModelTypes.h>

using chip::app::DataModel::Nullable;

using namespace chef;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

#ifdef MATTER_DM_PLUGIN_AIR_QUALITY_SERVER
#include "chef-air-quality.h"
#endif // MATTER_DM_PLUGIN_AIR_QUALITY_SERVER
#if defined(MATTER_DM_PLUGIN_CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                  \
    defined(MATTER_DM_PLUGIN_CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                   \
    defined(MATTER_DM_PLUGIN_NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                 \
    defined(MATTER_DM_PLUGIN_OZONE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                            \
    defined(MATTER_DM_PLUGIN_PM2__5_CONCENTRATION_MEASUREMENT_SERVER) ||                                                           \
    defined(MATTER_DM_PLUGIN_FORMALDEHYDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                     \
    defined(MATTER_DM_PLUGIN_PM1_CONCENTRATION_MEASUREMENT_SERVER) ||                                                              \
    defined(MATTER_DM_PLUGIN_PM10_CONCENTRATION_MEASUREMENT_SERVER) ||                                                             \
    defined(MATTER_DM_PLUGIN_TOTAL_VOLATILE_ORGANIC_COMPOUNDS_CONCENTRATION_MEASUREMENT_SERVER) ||                                 \
    defined(MATTER_DM_PLUGIN_RADON_CONCENTRATION_MEASUREMENT_SERVER)
#include "chef-concentration-measurement.h"
#endif
#if defined(MATTER_DM_PLUGIN_HEPA_FILTER_MONITORING_SERVER) || defined(MATTER_DM_PLUGIN_ACTIVATED_CARBON_FILTER_MONITORING_SERVER)
#include "resource-monitoring/chef-resource-monitoring-delegates.h"
#endif

#if defined(MATTER_DM_PLUGIN_RVC_RUN_MODE_SERVER) || defined(MATTER_DM_PLUGIN_RVC_CLEAN_MODE_SERVER)
#include "chef-rvc-mode-delegate.h"
#endif

#ifdef MATTER_DM_PLUGIN_REFRIGERATOR_AND_TEMPERATURE_CONTROLLED_CABINET_MODE_SERVER
#include "refrigerator-and-temperature-controlled-cabinet-mode/tcc-mode.h"
#endif // MATTER_DM_PLUGIN_REFRIGERATOR_AND_TEMPERATURE_CONTROLLED_CABINET_MODE_SERVER

#ifdef MATTER_DM_PLUGIN_PUMP_CONFIGURATION_AND_CONTROL_SERVER
#ifdef MATTER_DM_PLUGIN_ON_OFF_SERVER
#include "chef-pump.h"
#endif // MATTER_DM_PLUGIN_ON_OFF_SERVER
#endif // MATTER_DM_PLUGIN_PUMP_CONFIGURATION_AND_CONTROL_SERVER

namespace {

// Please refer to https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces
constexpr const uint8_t kNamespaceRefrigerator = 0x41;
// Refrigerator Namespace: 0x41, tag 0x00 (Refrigerator)
constexpr const uint8_t kTagRefrigerator = 0x00;
// Refrigerator Namespace: 0x41, tag 0x01 (Freezer)
constexpr const uint8_t kTagFreezer                                                 = 0x01;
const Clusters::Descriptor::Structs::SemanticTagStruct::Type gRefrigeratorTagList[] = { { .namespaceID = kNamespaceRefrigerator,
                                                                                          .tag         = kTagRefrigerator } };
const Clusters::Descriptor::Structs::SemanticTagStruct::Type gFreezerTagList[]      = { { .namespaceID = kNamespaceRefrigerator,
                                                                                          .tag         = kTagFreezer } };
} // namespace

namespace PostionSemanticTag {

constexpr const uint8_t kNamespace                                   = 0x08; // Common Position Namespace
const Clusters::Descriptor::Structs::SemanticTagStruct::Type kLeft   = { .namespaceID = kNamespace, .tag = 0x00 };
const Clusters::Descriptor::Structs::SemanticTagStruct::Type kRight  = { .namespaceID = kNamespace, .tag = 0x01 };
const Clusters::Descriptor::Structs::SemanticTagStruct::Type kTop    = { .namespaceID = kNamespace, .tag = 0x02 };
const Clusters::Descriptor::Structs::SemanticTagStruct::Type kBottom = { .namespaceID = kNamespace, .tag = 0x03 };
const Clusters::Descriptor::Structs::SemanticTagStruct::Type kMiddle = { .namespaceID = kNamespace, .tag = 0x04 };

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kTopTagList[]  = { PostionSemanticTag::kTop };
const Clusters::Descriptor::Structs::SemanticTagStruct::Type kLeftTagList[] = { PostionSemanticTag::kLeft };
} // namespace PostionSemanticTag

#ifdef MATTER_DM_PLUGIN_RVC_OPERATIONAL_STATE_SERVER
#include "chef-rvc-operational-state-delegate.h"
#endif

#ifdef MATTER_DM_PLUGIN_DISHWASHER_MODE_SERVER
#include "chef-dishwasher-mode-delegate-impl.h"
#endif // MATTER_DM_PLUGIN_DISHWASHER_MODE_SERVER

#ifdef MATTER_DM_PLUGIN_LAUNDRY_WASHER_MODE_SERVER
#include "chef-laundry-washer-mode.h"
#endif // MATTER_DM_PLUGIN_LAUNDRY_WASHER_MODE_SERVER

#ifdef MATTER_DM_PLUGIN_LAUNDRY_WASHER_CONTROLS_SERVER
#include "chef-laundry-washer-controls-delegate-impl.h"
#endif // MATTER_DM_PLUGIN_LAUNDRY_WASHER_CONTROLS_SERVER

#ifdef MATTER_DM_PLUGIN_OPERATIONAL_STATE_SERVER
#include "chef-operational-state-delegate-impl.h"
#endif // MATTER_DM_PLUGIN_OPERATIONAL_STATE_SERVER

#ifdef MATTER_DM_PLUGIN_FAN_CONTROL_SERVER
#include "chef-fan-control-manager.h"
#endif // MATTER_DM_PLUGIN_FAN_CONTROL_SERVER
#ifdef MATTER_DM_PLUGIN_TEMPERATURE_CONTROL_SERVER
#include "temperature-control/static-supported-temperature-levels.h"
#endif // MATTER_DM_PLUGIN_TEMPERATURE_CONTROL_SERVER

#ifdef MATTER_DM_PLUGIN_WINDOW_COVERING_SERVER
#include "window-covering/chef-window-covering.h"
#endif // MATTER_DM_PLUGIN_WINDOW_COVERING_SERVER

#ifdef MATTER_DM_PLUGIN_OVEN_MODE_SERVER
#include "oven-mode/chef-oven-mode.h"
#endif // MATTER_DM_PLUGIN_OVEN_MODE_SERVER

#ifdef MATTER_DM_PLUGIN_OVEN_CAVITY_OPERATIONAL_STATE_SERVER
#include "oven-cavity-operational-state/chef-oven-cavity-operational-state.h"
#endif // MATTER_DM_PLUGIN_OVEN_CAVITY_OPERATIONAL_STATE_SERVER

Protocols::InteractionModel::Status emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                                         const EmberAfAttributeMetadata * attributeMetadata,
                                                                         uint8_t * buffer, uint16_t maxReadLength)
{
    switch (clusterId)
    {
#ifdef MATTER_DM_PLUGIN_AIR_QUALITY_SERVER
    case chip::app::Clusters::AirQuality::Id:
        return chefAirQualityReadCallback(endpoint, clusterId, attributeMetadata, buffer, maxReadLength);
#endif
#if defined(MATTER_DM_PLUGIN_CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                  \
    defined(MATTER_DM_PLUGIN_CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                   \
    defined(MATTER_DM_PLUGIN_NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                 \
    defined(MATTER_DM_PLUGIN_OZONE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                            \
    defined(MATTER_DM_PLUGIN_PM2__5_CONCENTRATION_MEASUREMENT_SERVER) ||                                                           \
    defined(MATTER_DM_PLUGIN_FORMALDEHYDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                     \
    defined(MATTER_DM_PLUGIN_PM1_CONCENTRATION_MEASUREMENT_SERVER) ||                                                              \
    defined(MATTER_DM_PLUGIN_PM10_CONCENTRATION_MEASUREMENT_SERVER) ||                                                             \
    defined(MATTER_DM_PLUGIN_TOTAL_VOLATILE_ORGANIC_COMPOUNDS_CONCENTRATION_MEASUREMENT_SERVER) ||                                 \
    defined(MATTER_DM_PLUGIN_RADON_CONCENTRATION_MEASUREMENT_SERVER)
    case chip::app::Clusters::CarbonMonoxideConcentrationMeasurement::Id:
    case chip::app::Clusters::CarbonDioxideConcentrationMeasurement::Id:
    case chip::app::Clusters::NitrogenDioxideConcentrationMeasurement::Id:
    case chip::app::Clusters::OzoneConcentrationMeasurement::Id:
    case chip::app::Clusters::FormaldehydeConcentrationMeasurement::Id:
    case chip::app::Clusters::Pm1ConcentrationMeasurement::Id:
    case chip::app::Clusters::Pm25ConcentrationMeasurement::Id:
    case chip::app::Clusters::Pm10ConcentrationMeasurement::Id:
    case chip::app::Clusters::RadonConcentrationMeasurement::Id:
    case chip::app::Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::Id:
        return chefConcentrationMeasurementReadCallback(endpoint, clusterId, attributeMetadata, buffer, maxReadLength);
#endif
#if defined(MATTER_DM_PLUGIN_HEPA_FILTER_MONITORING_SERVER) || defined(MATTER_DM_PLUGIN_ACTIVATED_CARBON_FILTER_MONITORING_SERVER)
    case chip::app::Clusters::HepaFilterMonitoring::Id:
    case chip::app::Clusters::ActivatedCarbonFilterMonitoring::Id:
        return chefResourceMonitoringExternalReadCallback(endpoint, clusterId, attributeMetadata, buffer, maxReadLength);
#endif
#ifdef MATTER_DM_PLUGIN_RVC_RUN_MODE_SERVER
    case chip::app::Clusters::RvcRunMode::Id:
        return chefRvcRunModeReadCallback(endpoint, clusterId, attributeMetadata, buffer, maxReadLength);
#endif
#ifdef MATTER_DM_PLUGIN_RVC_CLEAN_MODE_SERVER
    case chip::app::Clusters::RvcCleanMode::Id:
        return chefRvcCleanModeReadCallback(endpoint, clusterId, attributeMetadata, buffer, maxReadLength);
#endif
#ifdef MATTER_DM_PLUGIN_RVC_OPERATIONAL_STATE_SERVER
    case chip::app::Clusters::RvcOperationalState::Id:
        return chefRvcOperationalStateReadCallback(endpoint, clusterId, attributeMetadata, buffer, maxReadLength);
#endif
#ifdef MATTER_DM_PLUGIN_REFRIGERATOR_AND_TEMPERATURE_CONTROLLED_CABINET_MODE_SERVER
    case chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Id:
        return chefRefrigeratorAndTemperatureControlledCabinetModeExternalReadCallback(endpoint, clusterId, attributeMetadata,
                                                                                       buffer, maxReadLength);
#endif
#ifdef MATTER_DM_PLUGIN_DISHWASHER_MODE_SERVER
    case chip::app::Clusters::DishwasherMode::Id:
        return chefDishwasherModeReadCallback(endpoint, clusterId, attributeMetadata, buffer, maxReadLength);
#endif // MATTER_DM_PLUGIN_DISHWASHER_MODE_SERVER
#ifdef MATTER_DM_PLUGIN_LAUNDRY_WASHER_MODE_SERVER
    case chip::app::Clusters::LaundryWasherMode::Id:
        return chefLaundryWasherModeReadCallback(endpoint, clusterId, attributeMetadata, buffer, maxReadLength);
#endif // MATTER_DM_PLUGIN_LAUNDRY_WASHER_MODE_SERVER
#ifdef MATTER_DM_PLUGIN_OPERATIONAL_STATE_SERVER
    case chip::app::Clusters::OperationalState::Id:
        return chefOperationalStateReadCallback(endpoint, clusterId, attributeMetadata, buffer, maxReadLength);
#endif // MATTER_DM_PLUGIN_OPERATIONAL_STATE_SERVER
    default:
        break;
    }
    return Protocols::InteractionModel::Status::Success;
}

/*
 *
Thread 3 "rootnode_airqua" hit Breakpoint 1, emberAfExternalAttributeWriteCallback (endpoint=1, clusterId=91,
    attributeMetadata=0x555555791f30 <(anonymous namespace)::generatedAttributes+1904>, buffer=0x7ffff68464ac "\001")
    at /home/erwinpan/matter/erwinpan1/master_1124_airqualitysensor/examples/chef/common/stubs.cpp:22
(gdb) p *attributeMetadata
$1 = {defaultValue = {ptrToDefaultValue = 0x0, defaultValue = 0, ptrToMinMaxValue = 0x0}, attributeId = 0, size = 1, attributeType =
48 '0', mask = 16 '\020'} (gdb)
*/

Protocols::InteractionModel::Status emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                                          const EmberAfAttributeMetadata * attributeMetadata,
                                                                          uint8_t * buffer)
{
    switch (clusterId)
    {
#ifdef MATTER_DM_PLUGIN_AIR_QUALITY_SERVER
    case chip::app::Clusters::AirQuality::Id:
        return chefAirQualityWriteCallback(endpoint, clusterId, attributeMetadata, buffer);
#endif
#if defined(MATTER_DM_PLUGIN_CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                  \
    defined(MATTER_DM_PLUGIN_CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                   \
    defined(MATTER_DM_PLUGIN_NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                 \
    defined(MATTER_DM_PLUGIN_OZONE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                            \
    defined(MATTER_DM_PLUGIN_PM2__5_CONCENTRATION_MEASUREMENT_SERVER) ||                                                           \
    defined(MATTER_DM_PLUGIN_FORMALDEHYDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                     \
    defined(MATTER_DM_PLUGIN_PM1_CONCENTRATION_MEASUREMENT_SERVER) ||                                                              \
    defined(MATTER_DM_PLUGIN_PM10_CONCENTRATION_MEASUREMENT_SERVER) ||                                                             \
    defined(MATTER_DM_PLUGIN_TOTAL_VOLATILE_ORGANIC_COMPOUNDS_CONCENTRATION_MEASUREMENT_SERVER) ||                                 \
    defined(MATTER_DM_PLUGIN_RADON_CONCENTRATION_MEASUREMENT_SERVER)
    case chip::app::Clusters::CarbonMonoxideConcentrationMeasurement::Id:
    case chip::app::Clusters::CarbonDioxideConcentrationMeasurement::Id:
    case chip::app::Clusters::NitrogenDioxideConcentrationMeasurement::Id:
    case chip::app::Clusters::OzoneConcentrationMeasurement::Id:
    case chip::app::Clusters::FormaldehydeConcentrationMeasurement::Id:
    case chip::app::Clusters::Pm1ConcentrationMeasurement::Id:
    case chip::app::Clusters::Pm25ConcentrationMeasurement::Id:
    case chip::app::Clusters::Pm10ConcentrationMeasurement::Id:
    case chip::app::Clusters::RadonConcentrationMeasurement::Id:
    case chip::app::Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::Id:
        return chefConcentrationMeasurementWriteCallback(endpoint, clusterId, attributeMetadata, buffer);
#endif
#if defined(MATTER_DM_PLUGIN_HEPA_FILTER_MONITORING_SERVER) || defined(MATTER_DM_PLUGIN_ACTIVATED_CARBON_FILTER_MONITORING_SERVER)
    case chip::app::Clusters::HepaFilterMonitoring::Id:
    case chip::app::Clusters::ActivatedCarbonFilterMonitoring::Id:
        return chefResourceMonitoringExternalWriteCallback(endpoint, clusterId, attributeMetadata, buffer);
#endif
#ifdef MATTER_DM_PLUGIN_RVC_RUN_MODE_SERVER
    case chip::app::Clusters::RvcRunMode::Id:
        return chefRvcRunModeWriteCallback(endpoint, clusterId, attributeMetadata, buffer);
#endif
#ifdef MATTER_DM_PLUGIN_RVC_CLEAN_MODE_SERVER
    case chip::app::Clusters::RvcCleanMode::Id:
        return chefRvcCleanModeWriteCallback(endpoint, clusterId, attributeMetadata, buffer);
#endif
#ifdef MATTER_DM_PLUGIN_RVC_OPERATIONAL_STATE_SERVER
    case chip::app::Clusters::RvcOperationalState::Id:
        return chefRvcOperationalStateWriteCallback(endpoint, clusterId, attributeMetadata, buffer);
#endif
#ifdef MATTER_DM_PLUGIN_REFRIGERATOR_AND_TEMPERATURE_CONTROLLED_CABINET_MODE_SERVER
    case chip::app::Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Id:
        return chefRefrigeratorAndTemperatureControlledCabinetModeExternalWriteCallback(endpoint, clusterId, attributeMetadata,
                                                                                        buffer);
#endif
#ifdef MATTER_DM_PLUGIN_DISHWASHER_MODE_SERVER
    case chip::app::Clusters::DishwasherMode::Id:
        return chefDishwasherModeWriteCallback(endpoint, clusterId, attributeMetadata, buffer);
#endif // MATTER_DM_PLUGIN_DISHWASHER_MODE_SERVER
#ifdef MATTER_DM_PLUGIN_LAUNDRY_WASHER_MODE_SERVER
    case chip::app::Clusters::LaundryWasherMode::Id:
        return chefLaundryWasherModeWriteCallback(endpoint, clusterId, attributeMetadata, buffer);
#endif // MATTER_DM_PLUGIN_LAUNDRY_WASHER_MODE_SERVER
#ifdef MATTER_DM_PLUGIN_OPERATIONAL_STATE_SERVER
    case chip::app::Clusters::OperationalState::Id:
        return chefOperationalStateWriteCallback(endpoint, clusterId, attributeMetadata, buffer);
#endif // MATTER_DM_PLUGIN_OPERATIONAL_STATE_SERVER
    default:
        break;
    }
    return Protocols::InteractionModel::Status::Success;
}

void emberAfPluginSmokeCoAlarmSelfTestRequestCommand(EndpointId endpointId) {}

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    ChipLogProgress(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    if (clusterId == OnOff::Id && attributeId == OnOff::Attributes::OnOff::Id)
    {
        ChipLogProgress(Zcl, "OnOff attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u", ChipLogValueMEI(attributeId),
                        type, *value, size);
#ifdef MATTER_DM_PLUGIN_FAN_CONTROL_SERVER // Handle OnOff for fan
#ifdef MATTER_DM_PLUGIN_ON_OFF_SERVER
        HandleOnOffAttributeChangeForFan(attributePath.mEndpointId, bool(*value));
#endif // MATTER_DM_PLUGIN_ON_OFF_SERVER
#endif // MATTER_DM_PLUGIN_FAN_CONTROL_SERVER

#ifdef MATTER_DM_PLUGIN_PUMP_CONFIGURATION_AND_CONTROL_SERVER
#ifdef MATTER_DM_PLUGIN_ON_OFF_SERVER
        if (chef::DeviceTypes::EndpointHasDeviceType(attributePath.mEndpointId, chef::DeviceTypes::kPumpDeviceId))
        {
            chef::pump::postOnOff(attributePath.mEndpointId, bool(*value));
        }
#endif // #ifdef MATTER_DM_PLUGIN_ON_OFF_SERVER
#endif // MATTER_DM_PLUGIN_PUMP_CONFIGURATION_AND_CONTROL_SERVER
    }
    else if (clusterId == LevelControl::Id)
    {
        ChipLogProgress(Zcl, "Level Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);

#ifdef MATTER_DM_PLUGIN_PUMP_CONFIGURATION_AND_CONTROL_SERVER
#ifdef MATTER_DM_PLUGIN_ON_OFF_SERVER
#ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER
        if (chef::DeviceTypes::EndpointHasDeviceType(attributePath.mEndpointId, chef::DeviceTypes::kPumpDeviceId))
        {
            chef::pump::postMoveToLevel(attributePath.mEndpointId, *value);
        }
#endif // MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER
#endif // #ifdef MATTER_DM_PLUGIN_ON_OFF_SERVER
#endif // MATTER_DM_PLUGIN_PUMP_CONFIGURATION_AND_CONTROL_SERVER

        // WIP Apply attribute change to Light
    }
#ifdef MATTER_DM_PLUGIN_FAN_CONTROL_SERVER
    else if (clusterId == FanControl::Id)
    {
        HandleFanControlAttributeChange(attributeId, type, size, value);
    }
#endif // MATTER_DM_PLUGIN_FAN_CONTROL_SERVER
}

/** @brief OnOff Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * TODO Issue #3841
 * emberAfOnOffClusterInitCallback happens before the stack initialize the cluster
 * attributes to the default value.
 * The logic here expects something similar to the deprecated Plugins callback
 * emberAfPluginOnOffClusterServerPostInitCallback.
 *
 */
void emberAfOnOffClusterInitCallback(EndpointId endpoint) {}

#ifdef MATTER_DM_PLUGIN_CHANNEL_SERVER
#include "channel/ChannelManager.h"
static ChannelManager channelManager[MATTER_DM_CHANNEL_CLUSTER_SERVER_ENDPOINT_COUNT];

void emberAfChannelClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: Channel::SetDefaultDelegate");
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, Channel::Id, MATTER_DM_CHANNEL_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < MATTER_DM_CHANNEL_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        channelManager[ep].SetEndpoint(endpoint);
        Channel::SetDefaultDelegate(endpoint, &channelManager[ep]);
    }
}
#endif

#ifdef MATTER_DM_PLUGIN_KEYPAD_INPUT_SERVER
#include "keypad-input/KeypadInputManager.h"
static KeypadInputManager keypadInputManager;

void emberAfKeypadInputClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: KeypadInput::SetDefaultDelegate");
    KeypadInput::SetDefaultDelegate(endpoint, &keypadInputManager);
}
#endif

#ifdef MATTER_DM_PLUGIN_LOW_POWER_SERVER
#include "low-power/LowPowerManager.h"
static LowPowerManager lowPowerManager;

void emberAfLowPowerClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: LowPower::SetDefaultDelegate");
    LowPower::SetDefaultDelegate(endpoint, &lowPowerManager);
}
#endif

#ifdef MATTER_DM_PLUGIN_TARGET_NAVIGATOR_SERVER
#include "target-navigator/TargetNavigatorManager.h"
static TargetNavigatorManager targetNavigatorManager;

void emberAfTargetNavigatorClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: TargetNavigator::SetDefaultDelegate");
    TargetNavigator::SetDefaultDelegate(endpoint, &targetNavigatorManager);
}
#endif

#ifdef MATTER_DM_PLUGIN_WAKE_ON_LAN_SERVER
#include "wake-on-lan/WakeOnLanManager.h"
static WakeOnLanManager wakeOnLanManager;

void emberAfWakeOnLanClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: WakeOnLanManager::SetDefaultDelegate");
    WakeOnLan::SetDefaultDelegate(endpoint, &wakeOnLanManager);
}
#endif

/**
 * This initializer is for the application having refrigerator on EP1 and child
 * temperatureControlledCabinet endpoints on EP2 and EP3.
 */
void RefrigeratorTemperatureControlledCabinetInit()
{
    EndpointId kRefEndpointId           = DeviceTypes::ExpectedEndpointId::kRefrigerator;
    EndpointId kColdCabinetEndpointId   = DeviceTypes::ExpectedEndpointId::kColdCabinetPartOfRefrigerator;
    EndpointId kFreezeCabinetEndpointId = DeviceTypes::ExpectedEndpointId::kFreezeCabinetPartOfRefrigerator;
    if (!DeviceTypes::EndpointHasDeviceType(kRefEndpointId, DeviceTypes::kRefrigeratorDeviceId))
    {
        return;
    }
    ChipLogDetail(NotSpecified, "Refrigerator device type on EP: %d", kRefEndpointId);
    SetTreeCompositionForEndpoint(kRefEndpointId);

    if (DeviceTypes::EndpointHasDeviceType(kColdCabinetEndpointId, DeviceTypes::kTemperatureControlledCabinetDeviceId))
    {
        ChipLogDetail(NotSpecified, "Temperature controlled cabinet device type on EP: %d", kColdCabinetEndpointId);
        SetParentEndpointForEndpoint(kColdCabinetEndpointId, kRefEndpointId);
        SetTagList(kColdCabinetEndpointId,
                   Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(gRefrigeratorTagList));
    }

    if (DeviceTypes::EndpointHasDeviceType(kFreezeCabinetEndpointId, DeviceTypes::kTemperatureControlledCabinetDeviceId))
    {
        ChipLogDetail(NotSpecified, "Temperature controlled cabinet device type on EP: %d", kFreezeCabinetEndpointId);
        SetParentEndpointForEndpoint(kFreezeCabinetEndpointId, kRefEndpointId);
        SetTagList(kFreezeCabinetEndpointId, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(gFreezerTagList));
    }
}

/**
 * This initializer is for the application having cooktop. The cooktop can be a part of an oven
 * or standalone cooktop.
 *     Standalone Cooktop: Cooktop on EP1 and optional CookSurface on EP2.
 *     Cooktop part of Oven: Oven on EP1, Cooktop on EP3 and optional CookSurface on EP4.
 */
void CooktopCookSurfaceInit(EndpointId kCooktopEpId)
{
    SetTreeCompositionForEndpoint(kCooktopEpId);
    switch (kCooktopEpId)
    {
    case DeviceTypes::ExpectedEndpointId::kCooktopStandAlone:
        if (DeviceTypes::EndpointHasDeviceType(kCooktopEpId, DeviceTypes::kCooktopDeviceId))
        {
            ChipLogDetail(NotSpecified, "Cooktop device type on EP: %d", kCooktopEpId);
            EndpointId kCookSurfaceEpId = DeviceTypes::ExpectedEndpointId::kCookSurfacePartOfCooktop;
            if (DeviceTypes::EndpointHasDeviceType(kCookSurfaceEpId, DeviceTypes::kCookSurfaceDeviceId))
            {
                ChipLogDetail(NotSpecified, "Cook Surface device type on EP: %d", kCookSurfaceEpId);
                SetParentEndpointForEndpoint(kCookSurfaceEpId, kCooktopEpId);
                SetTagList(kCookSurfaceEpId,
                           Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(PostionSemanticTag::kLeftTagList));
            }
        }
        break;
    case DeviceTypes::ExpectedEndpointId::kCooktopPartOfOven:
        EndpointId kOvenEpId = DeviceTypes::ExpectedEndpointId::kOven;
        if (DeviceTypes::EndpointHasDeviceType(kCooktopEpId, DeviceTypes::kCooktopDeviceId) &&
            DeviceTypes::EndpointHasDeviceType(kOvenEpId, DeviceTypes::kOvenDeviceId))
        {
            ChipLogDetail(NotSpecified, "Cooktop device type on EP: %d", kCooktopEpId);
            SetParentEndpointForEndpoint(kCooktopEpId, kOvenEpId);
            EndpointId kCookSurfaceEpId = DeviceTypes::ExpectedEndpointId::kCookSurfacePartOfCooktopOven;
            if (DeviceTypes::EndpointHasDeviceType(kCookSurfaceEpId, DeviceTypes::kCookSurfaceDeviceId))
            {
                ChipLogDetail(NotSpecified, "Cook Surface device type on EP: %d", kCookSurfaceEpId);
                SetParentEndpointForEndpoint(kCookSurfaceEpId, kCooktopEpId);
                SetTagList(kCookSurfaceEpId,
                           Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(PostionSemanticTag::kLeftTagList));
            }
        }
    }
}

/**
 * This initializer is for the application having oven on EP1 and child endpoints -
 * temperatureControlledCabinet on EP2, cooktop on EP3 and cooksurface on EP4.
 */
void OvenTemperatureControlledCabinetCooktopCookSurfaceInit()
{
    EndpointId kOvenEpId                         = DeviceTypes::ExpectedEndpointId::kOven;
    EndpointId kTemperatureControlledCabinetEpId = DeviceTypes::ExpectedEndpointId::kTopCabinetPartOfOven;
    EndpointId kCooktopEpId                      = DeviceTypes::ExpectedEndpointId::kCooktopPartOfOven;
    if (!DeviceTypes::EndpointHasDeviceType(kOvenEpId, DeviceTypes::kOvenDeviceId))
    {
        return;
    }

    ChipLogDetail(NotSpecified, "Oven device type on EP: %d", kOvenEpId);
    SetTreeCompositionForEndpoint(kOvenEpId);

    if (DeviceTypes::EndpointHasDeviceType(kTemperatureControlledCabinetEpId, DeviceTypes::kTemperatureControlledCabinetDeviceId))
    {
        ChipLogDetail(NotSpecified, "Temperature controlled cabinet device type on EP: %d", kTemperatureControlledCabinetEpId);
        SetParentEndpointForEndpoint(kTemperatureControlledCabinetEpId, kOvenEpId);
        SetTagList(kTemperatureControlledCabinetEpId,
                   Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(PostionSemanticTag::kTopTagList));
#ifdef MATTER_DM_PLUGIN_OVEN_CAVITY_OPERATIONAL_STATE_SERVER
        Clusters::OvenCavityOperationalState::InitChefOvenCavityOperationalStateCluster();
#endif // MATTER_DM_PLUGIN_OVEN_CAVITY_OPERATIONAL_STATE_SERVER
    }
    CooktopCookSurfaceInit(kCooktopEpId);
}

void ApplicationInit()
{
    ChipLogProgress(NotSpecified, "Chef Application Init !!!");

    RefrigeratorTemperatureControlledCabinetInit();
    OvenTemperatureControlledCabinetCooktopCookSurfaceInit();

#ifdef MATTER_DM_PLUGIN_PUMP_CONFIGURATION_AND_CONTROL_SERVER
#ifdef MATTER_DM_PLUGIN_ON_OFF_SERVER
    chef::pump::init();
#endif // MATTER_DM_PLUGIN_ON_OFF_SERVER
#endif // MATTER_DM_PLUGIN_PUMP_CONFIGURATION_AND_CONTROL_SERVER

#ifdef MATTER_DM_PLUGIN_WINDOW_COVERING_SERVER
    ChipLogProgress(NotSpecified, "Initializing WindowCovering cluster delegate.");
    ChefWindowCovering::InitChefWindowCoveringCluster();
#endif // MATTER_DM_PLUGIN_WINDOW_COVERING_SERVER

#ifdef MATTER_DM_PLUGIN_OVEN_MODE_SERVER
    ChipLogProgress(NotSpecified, "Initializing OvenMode cluster.");
    ChefOvenMode::InitChefOvenModeCluster();
#endif // MATTER_DM_PLUGIN_OVEN_MODE_SERVER
}

void ApplicationShutdown()
{
    ChipLogProgress(NotSpecified, "Chef Application Down !!!");
}

// No-op function, used to force linking this file,
// instead of the weak functions from other files
extern "C" void chef_include_stubs_impl(void) {}
