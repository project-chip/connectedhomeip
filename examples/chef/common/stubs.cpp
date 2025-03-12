#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app/data-model/Nullable.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <lib/core/DataModelTypes.h>

using chip::app::DataModel::Nullable;

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

#ifdef MATTER_DM_PLUGIN_REFRIGERATOR_ALARM_SERVER
namespace {

// Please refer to https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces
constexpr const uint8_t kNamespaceRefrigerator = 0x41;
// Refrigerator Namespace: 0x41, tag 0x00 (Refrigerator)
constexpr const uint8_t kTagRefrigerator = 0x00;
// Refrigerator Namespace: 0x41, tag 0x01 (Freezer)
constexpr const uint8_t kTagFreezer                                                = 0x01;
const Clusters::Descriptor::Structs::SemanticTagStruct::Type refrigeratorTagList[] = { { .namespaceID = kNamespaceRefrigerator,
                                                                                         .tag         = kTagRefrigerator } };
const Clusters::Descriptor::Structs::SemanticTagStruct::Type freezerTagList[]      = { { .namespaceID = kNamespaceRefrigerator,
                                                                                         .tag         = kTagFreezer } };
} // namespace
#endif // MATTER_DM_PLUGIN_REFRIGERATOR_ALARM_SERVER

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
    }
    else if (clusterId == LevelControl::Id)
    {
        ChipLogProgress(Zcl, "Level Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);

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
static ChannelManager channelManager;

void emberAfChannelClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: Channel::SetDefaultDelegate");
    Channel::SetDefaultDelegate(endpoint, &channelManager);
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

void ApplicationInit()
{
    ChipLogProgress(NotSpecified, "Chef Application Init !!!");

#ifdef MATTER_DM_PLUGIN_REFRIGERATOR_ALARM_SERVER
    // set Parent Endpoint and Composition Type for an Endpoint
    EndpointId kRefEndpointId           = 1;
    EndpointId kColdCabinetEndpointId   = 2;
    EndpointId kFreezeCabinetEndpointId = 3;
    SetTreeCompositionForEndpoint(kRefEndpointId);
    SetParentEndpointForEndpoint(kColdCabinetEndpointId, kRefEndpointId);
    SetParentEndpointForEndpoint(kFreezeCabinetEndpointId, kRefEndpointId);
    // set TagList
    SetTagList(kColdCabinetEndpointId, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(refrigeratorTagList));
    SetTagList(kFreezeCabinetEndpointId, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(freezerTagList));
#endif // MATTER_DM_PLUGIN_REFRIGERATOR_ALARM_SERVER

#ifdef MATTER_DM_PLUGIN_WINDOW_COVERING_SERVER
    ChipLogProgress(NotSpecified, "Initializing WindowCovering cluster delegate.");
    ChefWindowCovering::InitChefWindowCoveringCluster();
#endif // MATTER_DM_PLUGIN_WINDOW_COVERING_SERVER
}

void ApplicationShutdown()
{
    ChipLogProgress(NotSpecified, "Chef Application Down !!!");
}

// No-op function, used to force linking this file,
// instead of the weak functions from other files
extern "C" void chef_include_stubs_impl(void) {}
