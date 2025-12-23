// DO NOT EDIT MANUALLY - Generated file
//
// This file provides a function to query into the MetadataProviders without
// instantiating any unnecessary metadata.
// It will instatiate all metadata for the selected cluster commands, even unused commands.
//
// If used without parameters it will instatiate metadata
// for all clusters and might incur a big overhead.
//
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/AccessControl/Ids.h>
#include <clusters/AccessControl/MetadataProvider.h>
#include <clusters/AccountLogin/Ids.h>
#include <clusters/AccountLogin/MetadataProvider.h>
#include <clusters/Actions/Ids.h>
#include <clusters/Actions/MetadataProvider.h>
#include <clusters/ActivatedCarbonFilterMonitoring/Ids.h>
#include <clusters/ActivatedCarbonFilterMonitoring/MetadataProvider.h>
#include <clusters/AdministratorCommissioning/Ids.h>
#include <clusters/AdministratorCommissioning/MetadataProvider.h>
#include <clusters/AirQuality/Ids.h>
#include <clusters/AirQuality/MetadataProvider.h>
#include <clusters/AmbientContextSensing/Ids.h>
#include <clusters/AmbientContextSensing/MetadataProvider.h>
#include <clusters/AmbientSensingUnion/Ids.h>
#include <clusters/AmbientSensingUnion/MetadataProvider.h>
#include <clusters/ApplicationBasic/Ids.h>
#include <clusters/ApplicationBasic/MetadataProvider.h>
#include <clusters/ApplicationLauncher/Ids.h>
#include <clusters/ApplicationLauncher/MetadataProvider.h>
#include <clusters/AudioOutput/Ids.h>
#include <clusters/AudioOutput/MetadataProvider.h>
#include <clusters/BallastConfiguration/Ids.h>
#include <clusters/BallastConfiguration/MetadataProvider.h>
#include <clusters/BasicInformation/Ids.h>
#include <clusters/BasicInformation/MetadataProvider.h>
#include <clusters/Binding/Ids.h>
#include <clusters/Binding/MetadataProvider.h>
#include <clusters/BooleanState/Ids.h>
#include <clusters/BooleanState/MetadataProvider.h>
#include <clusters/BooleanStateConfiguration/Ids.h>
#include <clusters/BooleanStateConfiguration/MetadataProvider.h>
#include <clusters/BridgedDeviceBasicInformation/Ids.h>
#include <clusters/BridgedDeviceBasicInformation/MetadataProvider.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Ids.h>
#include <clusters/CameraAvSettingsUserLevelManagement/MetadataProvider.h>
#include <clusters/CameraAvStreamManagement/Ids.h>
#include <clusters/CameraAvStreamManagement/MetadataProvider.h>
#include <clusters/CarbonDioxideConcentrationMeasurement/Ids.h>
#include <clusters/CarbonDioxideConcentrationMeasurement/MetadataProvider.h>
#include <clusters/CarbonMonoxideConcentrationMeasurement/Ids.h>
#include <clusters/CarbonMonoxideConcentrationMeasurement/MetadataProvider.h>
#include <clusters/Channel/Ids.h>
#include <clusters/Channel/MetadataProvider.h>
#include <clusters/Chime/Ids.h>
#include <clusters/Chime/MetadataProvider.h>
#include <clusters/ClosureControl/Ids.h>
#include <clusters/ClosureControl/MetadataProvider.h>
#include <clusters/ClosureDimension/Ids.h>
#include <clusters/ClosureDimension/MetadataProvider.h>
#include <clusters/ColorControl/Ids.h>
#include <clusters/ColorControl/MetadataProvider.h>
#include <clusters/CommissionerControl/Ids.h>
#include <clusters/CommissionerControl/MetadataProvider.h>
#include <clusters/CommodityMetering/Ids.h>
#include <clusters/CommodityMetering/MetadataProvider.h>
#include <clusters/CommodityPrice/Ids.h>
#include <clusters/CommodityPrice/MetadataProvider.h>
#include <clusters/CommodityTariff/Ids.h>
#include <clusters/CommodityTariff/MetadataProvider.h>
#include <clusters/ContentAppObserver/Ids.h>
#include <clusters/ContentAppObserver/MetadataProvider.h>
#include <clusters/ContentControl/Ids.h>
#include <clusters/ContentControl/MetadataProvider.h>
#include <clusters/ContentLauncher/Ids.h>
#include <clusters/ContentLauncher/MetadataProvider.h>
#include <clusters/Descriptor/Ids.h>
#include <clusters/Descriptor/MetadataProvider.h>
#include <clusters/DeviceEnergyManagement/Ids.h>
#include <clusters/DeviceEnergyManagement/MetadataProvider.h>
#include <clusters/DeviceEnergyManagementMode/Ids.h>
#include <clusters/DeviceEnergyManagementMode/MetadataProvider.h>
#include <clusters/DiagnosticLogs/Ids.h>
#include <clusters/DiagnosticLogs/MetadataProvider.h>
#include <clusters/DishwasherAlarm/Ids.h>
#include <clusters/DishwasherAlarm/MetadataProvider.h>
#include <clusters/DishwasherMode/Ids.h>
#include <clusters/DishwasherMode/MetadataProvider.h>
#include <clusters/DoorLock/Ids.h>
#include <clusters/DoorLock/MetadataProvider.h>
#include <clusters/EcosystemInformation/Ids.h>
#include <clusters/EcosystemInformation/MetadataProvider.h>
#include <clusters/ElectricalEnergyMeasurement/Ids.h>
#include <clusters/ElectricalEnergyMeasurement/MetadataProvider.h>
#include <clusters/ElectricalGridConditions/Ids.h>
#include <clusters/ElectricalGridConditions/MetadataProvider.h>
#include <clusters/ElectricalPowerMeasurement/Ids.h>
#include <clusters/ElectricalPowerMeasurement/MetadataProvider.h>
#include <clusters/EnergyEvse/Ids.h>
#include <clusters/EnergyEvse/MetadataProvider.h>
#include <clusters/EnergyEvseMode/Ids.h>
#include <clusters/EnergyEvseMode/MetadataProvider.h>
#include <clusters/EnergyPreference/Ids.h>
#include <clusters/EnergyPreference/MetadataProvider.h>
#include <clusters/EthernetNetworkDiagnostics/Ids.h>
#include <clusters/EthernetNetworkDiagnostics/MetadataProvider.h>
#include <clusters/FanControl/Ids.h>
#include <clusters/FanControl/MetadataProvider.h>
#include <clusters/FaultInjection/Ids.h>
#include <clusters/FaultInjection/MetadataProvider.h>
#include <clusters/FixedLabel/Ids.h>
#include <clusters/FixedLabel/MetadataProvider.h>
#include <clusters/FlowMeasurement/Ids.h>
#include <clusters/FlowMeasurement/MetadataProvider.h>
#include <clusters/FormaldehydeConcentrationMeasurement/Ids.h>
#include <clusters/FormaldehydeConcentrationMeasurement/MetadataProvider.h>
#include <clusters/GeneralCommissioning/Ids.h>
#include <clusters/GeneralCommissioning/MetadataProvider.h>
#include <clusters/GeneralDiagnostics/Ids.h>
#include <clusters/GeneralDiagnostics/MetadataProvider.h>
#include <clusters/GroupKeyManagement/Ids.h>
#include <clusters/GroupKeyManagement/MetadataProvider.h>
#include <clusters/Groupcast/Ids.h>
#include <clusters/Groupcast/MetadataProvider.h>
#include <clusters/Groups/Ids.h>
#include <clusters/Groups/MetadataProvider.h>
#include <clusters/HepaFilterMonitoring/Ids.h>
#include <clusters/HepaFilterMonitoring/MetadataProvider.h>
#include <clusters/IcdManagement/Ids.h>
#include <clusters/IcdManagement/MetadataProvider.h>
#include <clusters/Identify/Ids.h>
#include <clusters/Identify/MetadataProvider.h>
#include <clusters/IlluminanceMeasurement/Ids.h>
#include <clusters/IlluminanceMeasurement/MetadataProvider.h>
#include <clusters/JointFabricAdministrator/Ids.h>
#include <clusters/JointFabricAdministrator/MetadataProvider.h>
#include <clusters/JointFabricDatastore/Ids.h>
#include <clusters/JointFabricDatastore/MetadataProvider.h>
#include <clusters/KeypadInput/Ids.h>
#include <clusters/KeypadInput/MetadataProvider.h>
#include <clusters/LaundryDryerControls/Ids.h>
#include <clusters/LaundryDryerControls/MetadataProvider.h>
#include <clusters/LaundryWasherControls/Ids.h>
#include <clusters/LaundryWasherControls/MetadataProvider.h>
#include <clusters/LaundryWasherMode/Ids.h>
#include <clusters/LaundryWasherMode/MetadataProvider.h>
#include <clusters/LevelControl/Ids.h>
#include <clusters/LevelControl/MetadataProvider.h>
#include <clusters/LocalizationConfiguration/Ids.h>
#include <clusters/LocalizationConfiguration/MetadataProvider.h>
#include <clusters/LowPower/Ids.h>
#include <clusters/LowPower/MetadataProvider.h>
#include <clusters/MediaInput/Ids.h>
#include <clusters/MediaInput/MetadataProvider.h>
#include <clusters/MediaPlayback/Ids.h>
#include <clusters/MediaPlayback/MetadataProvider.h>
#include <clusters/Messages/Ids.h>
#include <clusters/Messages/MetadataProvider.h>
#include <clusters/MeterIdentification/Ids.h>
#include <clusters/MeterIdentification/MetadataProvider.h>
#include <clusters/MicrowaveOvenControl/Ids.h>
#include <clusters/MicrowaveOvenControl/MetadataProvider.h>
#include <clusters/MicrowaveOvenMode/Ids.h>
#include <clusters/MicrowaveOvenMode/MetadataProvider.h>
#include <clusters/ModeSelect/Ids.h>
#include <clusters/ModeSelect/MetadataProvider.h>
#include <clusters/NetworkCommissioning/Ids.h>
#include <clusters/NetworkCommissioning/MetadataProvider.h>
#include <clusters/NitrogenDioxideConcentrationMeasurement/Ids.h>
#include <clusters/NitrogenDioxideConcentrationMeasurement/MetadataProvider.h>
#include <clusters/OccupancySensing/Ids.h>
#include <clusters/OccupancySensing/MetadataProvider.h>
#include <clusters/OnOff/Ids.h>
#include <clusters/OnOff/MetadataProvider.h>
#include <clusters/OperationalCredentials/Ids.h>
#include <clusters/OperationalCredentials/MetadataProvider.h>
#include <clusters/OperationalState/Ids.h>
#include <clusters/OperationalState/MetadataProvider.h>
#include <clusters/OtaSoftwareUpdateProvider/Ids.h>
#include <clusters/OtaSoftwareUpdateProvider/MetadataProvider.h>
#include <clusters/OtaSoftwareUpdateRequestor/Ids.h>
#include <clusters/OtaSoftwareUpdateRequestor/MetadataProvider.h>
#include <clusters/OvenCavityOperationalState/Ids.h>
#include <clusters/OvenCavityOperationalState/MetadataProvider.h>
#include <clusters/OvenMode/Ids.h>
#include <clusters/OvenMode/MetadataProvider.h>
#include <clusters/OzoneConcentrationMeasurement/Ids.h>
#include <clusters/OzoneConcentrationMeasurement/MetadataProvider.h>
#include <clusters/Pm10ConcentrationMeasurement/Ids.h>
#include <clusters/Pm10ConcentrationMeasurement/MetadataProvider.h>
#include <clusters/Pm1ConcentrationMeasurement/Ids.h>
#include <clusters/Pm1ConcentrationMeasurement/MetadataProvider.h>
#include <clusters/Pm25ConcentrationMeasurement/Ids.h>
#include <clusters/Pm25ConcentrationMeasurement/MetadataProvider.h>
#include <clusters/PowerSource/Ids.h>
#include <clusters/PowerSource/MetadataProvider.h>
#include <clusters/PowerSourceConfiguration/Ids.h>
#include <clusters/PowerSourceConfiguration/MetadataProvider.h>
#include <clusters/PowerTopology/Ids.h>
#include <clusters/PowerTopology/MetadataProvider.h>
#include <clusters/PressureMeasurement/Ids.h>
#include <clusters/PressureMeasurement/MetadataProvider.h>
#include <clusters/ProxyConfiguration/Ids.h>
#include <clusters/ProxyConfiguration/MetadataProvider.h>
#include <clusters/ProxyDiscovery/Ids.h>
#include <clusters/ProxyDiscovery/MetadataProvider.h>
#include <clusters/ProxyValid/Ids.h>
#include <clusters/ProxyValid/MetadataProvider.h>
#include <clusters/PulseWidthModulation/Ids.h>
#include <clusters/PulseWidthModulation/MetadataProvider.h>
#include <clusters/PumpConfigurationAndControl/Ids.h>
#include <clusters/PumpConfigurationAndControl/MetadataProvider.h>
#include <clusters/PushAvStreamTransport/Ids.h>
#include <clusters/PushAvStreamTransport/MetadataProvider.h>
#include <clusters/RadonConcentrationMeasurement/Ids.h>
#include <clusters/RadonConcentrationMeasurement/MetadataProvider.h>
#include <clusters/RefrigeratorAlarm/Ids.h>
#include <clusters/RefrigeratorAlarm/MetadataProvider.h>
#include <clusters/RefrigeratorAndTemperatureControlledCabinetMode/Ids.h>
#include <clusters/RefrigeratorAndTemperatureControlledCabinetMode/MetadataProvider.h>
#include <clusters/RelativeHumidityMeasurement/Ids.h>
#include <clusters/RelativeHumidityMeasurement/MetadataProvider.h>
#include <clusters/RvcCleanMode/Ids.h>
#include <clusters/RvcCleanMode/MetadataProvider.h>
#include <clusters/RvcOperationalState/Ids.h>
#include <clusters/RvcOperationalState/MetadataProvider.h>
#include <clusters/RvcRunMode/Ids.h>
#include <clusters/RvcRunMode/MetadataProvider.h>
#include <clusters/SampleMei/Ids.h>
#include <clusters/SampleMei/MetadataProvider.h>
#include <clusters/ScenesManagement/Ids.h>
#include <clusters/ScenesManagement/MetadataProvider.h>
#include <clusters/ServiceArea/Ids.h>
#include <clusters/ServiceArea/MetadataProvider.h>
#include <clusters/SmokeCoAlarm/Ids.h>
#include <clusters/SmokeCoAlarm/MetadataProvider.h>
#include <clusters/SoftwareDiagnostics/Ids.h>
#include <clusters/SoftwareDiagnostics/MetadataProvider.h>
#include <clusters/SoilMeasurement/Ids.h>
#include <clusters/SoilMeasurement/MetadataProvider.h>
#include <clusters/Switch/Ids.h>
#include <clusters/Switch/MetadataProvider.h>
#include <clusters/TargetNavigator/Ids.h>
#include <clusters/TargetNavigator/MetadataProvider.h>
#include <clusters/TemperatureControl/Ids.h>
#include <clusters/TemperatureControl/MetadataProvider.h>
#include <clusters/TemperatureMeasurement/Ids.h>
#include <clusters/TemperatureMeasurement/MetadataProvider.h>
#include <clusters/Thermostat/Ids.h>
#include <clusters/Thermostat/MetadataProvider.h>
#include <clusters/ThermostatUserInterfaceConfiguration/Ids.h>
#include <clusters/ThermostatUserInterfaceConfiguration/MetadataProvider.h>
#include <clusters/ThreadBorderRouterManagement/Ids.h>
#include <clusters/ThreadBorderRouterManagement/MetadataProvider.h>
#include <clusters/ThreadNetworkDiagnostics/Ids.h>
#include <clusters/ThreadNetworkDiagnostics/MetadataProvider.h>
#include <clusters/ThreadNetworkDirectory/Ids.h>
#include <clusters/ThreadNetworkDirectory/MetadataProvider.h>
#include <clusters/TimeFormatLocalization/Ids.h>
#include <clusters/TimeFormatLocalization/MetadataProvider.h>
#include <clusters/TimeSynchronization/Ids.h>
#include <clusters/TimeSynchronization/MetadataProvider.h>
#include <clusters/Timer/Ids.h>
#include <clusters/Timer/MetadataProvider.h>
#include <clusters/TlsCertificateManagement/Ids.h>
#include <clusters/TlsCertificateManagement/MetadataProvider.h>
#include <clusters/TlsClientManagement/Ids.h>
#include <clusters/TlsClientManagement/MetadataProvider.h>
#include <clusters/TotalVolatileOrganicCompoundsConcentrationMeasurement/Ids.h>
#include <clusters/TotalVolatileOrganicCompoundsConcentrationMeasurement/MetadataProvider.h>
#include <clusters/UnitLocalization/Ids.h>
#include <clusters/UnitLocalization/MetadataProvider.h>
#include <clusters/UnitTesting/Ids.h>
#include <clusters/UnitTesting/MetadataProvider.h>
#include <clusters/UserLabel/Ids.h>
#include <clusters/UserLabel/MetadataProvider.h>
#include <clusters/ValveConfigurationAndControl/Ids.h>
#include <clusters/ValveConfigurationAndControl/MetadataProvider.h>
#include <clusters/WakeOnLan/Ids.h>
#include <clusters/WakeOnLan/MetadataProvider.h>
#include <clusters/WaterHeaterManagement/Ids.h>
#include <clusters/WaterHeaterManagement/MetadataProvider.h>
#include <clusters/WaterHeaterMode/Ids.h>
#include <clusters/WaterHeaterMode/MetadataProvider.h>
#include <clusters/WaterTankLevelMonitoring/Ids.h>
#include <clusters/WaterTankLevelMonitoring/MetadataProvider.h>
#include <clusters/WebRTCTransportProvider/Ids.h>
#include <clusters/WebRTCTransportProvider/MetadataProvider.h>
#include <clusters/WebRTCTransportRequestor/Ids.h>
#include <clusters/WebRTCTransportRequestor/MetadataProvider.h>
#include <clusters/WiFiNetworkDiagnostics/Ids.h>
#include <clusters/WiFiNetworkDiagnostics/MetadataProvider.h>
#include <clusters/WiFiNetworkManagement/Ids.h>
#include <clusters/WiFiNetworkManagement/MetadataProvider.h>
#include <clusters/WindowCovering/Ids.h>
#include <clusters/WindowCovering/MetadataProvider.h>
#include <clusters/ZoneManagement/Ids.h>
#include <clusters/ZoneManagement/MetadataProvider.h>
#include <lib/core/DataModelTypes.h>
#include <optional>

namespace chip {
namespace app {
namespace DataModel {
namespace detail {

// Implements a search for the AcceptedCommandEntry in multiple clusters
// If no Clusters are provided it will search all clusters.
// We provide this function for convenience, however it is not expected to be used long-term.
template <ClusterId... TClusterIds>
std::optional<DataModel::AcceptedCommandEntry> AcceptedCommandEntryFor(ClusterId id, CommandId command)
{
    using namespace chip::app::Clusters;
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == AccessControl::Id) || ...))
    {
        if (id == AccessControl::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, AccessControl::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == AccountLogin::Id) || ...))
    {
        if (id == AccountLogin::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, AccountLogin::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == Actions::Id) || ...))
    {
        if (id == Actions::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Actions::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ActivatedCarbonFilterMonitoring::Id) || ...))
    {
        if (id == ActivatedCarbonFilterMonitoring::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ActivatedCarbonFilterMonitoring::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == AdministratorCommissioning::Id) || ...))
    {
        if (id == AdministratorCommissioning::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, AdministratorCommissioning::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == AirQuality::Id) || ...))
    {
        if (id == AirQuality::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, AirQuality::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == AmbientContextSensing::Id) || ...))
    {
        if (id == AmbientContextSensing::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, AmbientContextSensing::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == AmbientSensingUnion::Id) || ...))
    {
        if (id == AmbientSensingUnion::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, AmbientSensingUnion::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ApplicationBasic::Id) || ...))
    {
        if (id == ApplicationBasic::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ApplicationBasic::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ApplicationLauncher::Id) || ...))
    {
        if (id == ApplicationLauncher::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ApplicationLauncher::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == AudioOutput::Id) || ...))
    {
        if (id == AudioOutput::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, AudioOutput::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == BallastConfiguration::Id) || ...))
    {
        if (id == BallastConfiguration::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, BallastConfiguration::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == BasicInformation::Id) || ...))
    {
        if (id == BasicInformation::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, BasicInformation::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == Binding::Id) || ...))
    {
        if (id == Binding::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Binding::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == BooleanState::Id) || ...))
    {
        if (id == BooleanState::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, BooleanState::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == BooleanStateConfiguration::Id) || ...))
    {
        if (id == BooleanStateConfiguration::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, BooleanStateConfiguration::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == BridgedDeviceBasicInformation::Id) || ...))
    {
        if (id == BridgedDeviceBasicInformation::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, BridgedDeviceBasicInformation::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == CameraAvSettingsUserLevelManagement::Id) || ...))
    {
        if (id == CameraAvSettingsUserLevelManagement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, CameraAvSettingsUserLevelManagement::Id>::EntryFor(
                command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == CameraAvStreamManagement::Id) || ...))
    {
        if (id == CameraAvStreamManagement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, CameraAvStreamManagement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == CarbonDioxideConcentrationMeasurement::Id) || ...))
    {
        if (id == CarbonDioxideConcentrationMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, CarbonDioxideConcentrationMeasurement::Id>::EntryFor(
                command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == CarbonMonoxideConcentrationMeasurement::Id) || ...))
    {
        if (id == CarbonMonoxideConcentrationMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, CarbonMonoxideConcentrationMeasurement::Id>::EntryFor(
                command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == Channel::Id) || ...))
    {
        if (id == Channel::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Channel::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == Chime::Id) || ...))
    {
        if (id == Chime::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Chime::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ClosureControl::Id) || ...))
    {
        if (id == ClosureControl::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ClosureControl::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ClosureDimension::Id) || ...))
    {
        if (id == ClosureDimension::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ClosureDimension::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ColorControl::Id) || ...))
    {
        if (id == ColorControl::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ColorControl::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == CommissionerControl::Id) || ...))
    {
        if (id == CommissionerControl::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, CommissionerControl::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == CommodityMetering::Id) || ...))
    {
        if (id == CommodityMetering::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, CommodityMetering::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == CommodityPrice::Id) || ...))
    {
        if (id == CommodityPrice::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, CommodityPrice::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == CommodityTariff::Id) || ...))
    {
        if (id == CommodityTariff::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, CommodityTariff::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ContentAppObserver::Id) || ...))
    {
        if (id == ContentAppObserver::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ContentAppObserver::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ContentControl::Id) || ...))
    {
        if (id == ContentControl::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ContentControl::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ContentLauncher::Id) || ...))
    {
        if (id == ContentLauncher::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ContentLauncher::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == Descriptor::Id) || ...))
    {
        if (id == Descriptor::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Descriptor::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == DeviceEnergyManagement::Id) || ...))
    {
        if (id == DeviceEnergyManagement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, DeviceEnergyManagement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == DeviceEnergyManagementMode::Id) || ...))
    {
        if (id == DeviceEnergyManagementMode::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, DeviceEnergyManagementMode::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == DiagnosticLogs::Id) || ...))
    {
        if (id == DiagnosticLogs::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, DiagnosticLogs::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == DishwasherAlarm::Id) || ...))
    {
        if (id == DishwasherAlarm::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, DishwasherAlarm::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == DishwasherMode::Id) || ...))
    {
        if (id == DishwasherMode::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, DishwasherMode::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == DoorLock::Id) || ...))
    {
        if (id == DoorLock::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, DoorLock::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == EcosystemInformation::Id) || ...))
    {
        if (id == EcosystemInformation::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, EcosystemInformation::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ElectricalEnergyMeasurement::Id) || ...))
    {
        if (id == ElectricalEnergyMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ElectricalEnergyMeasurement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ElectricalGridConditions::Id) || ...))
    {
        if (id == ElectricalGridConditions::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ElectricalGridConditions::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ElectricalPowerMeasurement::Id) || ...))
    {
        if (id == ElectricalPowerMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ElectricalPowerMeasurement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == EnergyEvse::Id) || ...))
    {
        if (id == EnergyEvse::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, EnergyEvse::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == EnergyEvseMode::Id) || ...))
    {
        if (id == EnergyEvseMode::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, EnergyEvseMode::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == EnergyPreference::Id) || ...))
    {
        if (id == EnergyPreference::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, EnergyPreference::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == EthernetNetworkDiagnostics::Id) || ...))
    {
        if (id == EthernetNetworkDiagnostics::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, EthernetNetworkDiagnostics::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == FanControl::Id) || ...))
    {
        if (id == FanControl::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, FanControl::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == FaultInjection::Id) || ...))
    {
        if (id == FaultInjection::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, FaultInjection::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == FixedLabel::Id) || ...))
    {
        if (id == FixedLabel::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, FixedLabel::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == FlowMeasurement::Id) || ...))
    {
        if (id == FlowMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, FlowMeasurement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == FormaldehydeConcentrationMeasurement::Id) || ...))
    {
        if (id == FormaldehydeConcentrationMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, FormaldehydeConcentrationMeasurement::Id>::EntryFor(
                command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == GeneralCommissioning::Id) || ...))
    {
        if (id == GeneralCommissioning::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, GeneralCommissioning::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == GeneralDiagnostics::Id) || ...))
    {
        if (id == GeneralDiagnostics::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, GeneralDiagnostics::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == Groupcast::Id) || ...))
    {
        if (id == Groupcast::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Groupcast::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == GroupKeyManagement::Id) || ...))
    {
        if (id == GroupKeyManagement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, GroupKeyManagement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == Groups::Id) || ...))
    {
        if (id == Groups::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Groups::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == HepaFilterMonitoring::Id) || ...))
    {
        if (id == HepaFilterMonitoring::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, HepaFilterMonitoring::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == IcdManagement::Id) || ...))
    {
        if (id == IcdManagement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, IcdManagement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == Identify::Id) || ...))
    {
        if (id == Identify::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Identify::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == IlluminanceMeasurement::Id) || ...))
    {
        if (id == IlluminanceMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, IlluminanceMeasurement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == JointFabricAdministrator::Id) || ...))
    {
        if (id == JointFabricAdministrator::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, JointFabricAdministrator::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == JointFabricDatastore::Id) || ...))
    {
        if (id == JointFabricDatastore::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, JointFabricDatastore::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == KeypadInput::Id) || ...))
    {
        if (id == KeypadInput::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, KeypadInput::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == LaundryDryerControls::Id) || ...))
    {
        if (id == LaundryDryerControls::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, LaundryDryerControls::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == LaundryWasherControls::Id) || ...))
    {
        if (id == LaundryWasherControls::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, LaundryWasherControls::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == LaundryWasherMode::Id) || ...))
    {
        if (id == LaundryWasherMode::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, LaundryWasherMode::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == LevelControl::Id) || ...))
    {
        if (id == LevelControl::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, LevelControl::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == LocalizationConfiguration::Id) || ...))
    {
        if (id == LocalizationConfiguration::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, LocalizationConfiguration::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == LowPower::Id) || ...))
    {
        if (id == LowPower::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, LowPower::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == MediaInput::Id) || ...))
    {
        if (id == MediaInput::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, MediaInput::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == MediaPlayback::Id) || ...))
    {
        if (id == MediaPlayback::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, MediaPlayback::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == Messages::Id) || ...))
    {
        if (id == Messages::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Messages::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == MeterIdentification::Id) || ...))
    {
        if (id == MeterIdentification::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, MeterIdentification::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == MicrowaveOvenControl::Id) || ...))
    {
        if (id == MicrowaveOvenControl::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, MicrowaveOvenControl::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == MicrowaveOvenMode::Id) || ...))
    {
        if (id == MicrowaveOvenMode::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, MicrowaveOvenMode::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ModeSelect::Id) || ...))
    {
        if (id == ModeSelect::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ModeSelect::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == NetworkCommissioning::Id) || ...))
    {
        if (id == NetworkCommissioning::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, NetworkCommissioning::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == NitrogenDioxideConcentrationMeasurement::Id) || ...))
    {
        if (id == NitrogenDioxideConcentrationMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, NitrogenDioxideConcentrationMeasurement::Id>::EntryFor(
                command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == OccupancySensing::Id) || ...))
    {
        if (id == OccupancySensing::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, OccupancySensing::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == OnOff::Id) || ...))
    {
        if (id == OnOff::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, OnOff::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == OperationalCredentials::Id) || ...))
    {
        if (id == OperationalCredentials::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, OperationalCredentials::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == OperationalState::Id) || ...))
    {
        if (id == OperationalState::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, OperationalState::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == OtaSoftwareUpdateProvider::Id) || ...))
    {
        if (id == OtaSoftwareUpdateProvider::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, OtaSoftwareUpdateProvider::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == OtaSoftwareUpdateRequestor::Id) || ...))
    {
        if (id == OtaSoftwareUpdateRequestor::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, OtaSoftwareUpdateRequestor::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == OvenCavityOperationalState::Id) || ...))
    {
        if (id == OvenCavityOperationalState::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, OvenCavityOperationalState::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == OvenMode::Id) || ...))
    {
        if (id == OvenMode::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, OvenMode::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == OzoneConcentrationMeasurement::Id) || ...))
    {
        if (id == OzoneConcentrationMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, OzoneConcentrationMeasurement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == Pm10ConcentrationMeasurement::Id) || ...))
    {
        if (id == Pm10ConcentrationMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Pm10ConcentrationMeasurement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == Pm1ConcentrationMeasurement::Id) || ...))
    {
        if (id == Pm1ConcentrationMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Pm1ConcentrationMeasurement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == Pm25ConcentrationMeasurement::Id) || ...))
    {
        if (id == Pm25ConcentrationMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Pm25ConcentrationMeasurement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == PowerSource::Id) || ...))
    {
        if (id == PowerSource::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, PowerSource::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == PowerSourceConfiguration::Id) || ...))
    {
        if (id == PowerSourceConfiguration::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, PowerSourceConfiguration::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == PowerTopology::Id) || ...))
    {
        if (id == PowerTopology::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, PowerTopology::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == PressureMeasurement::Id) || ...))
    {
        if (id == PressureMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, PressureMeasurement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ProxyConfiguration::Id) || ...))
    {
        if (id == ProxyConfiguration::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ProxyConfiguration::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ProxyDiscovery::Id) || ...))
    {
        if (id == ProxyDiscovery::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ProxyDiscovery::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ProxyValid::Id) || ...))
    {
        if (id == ProxyValid::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ProxyValid::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == PulseWidthModulation::Id) || ...))
    {
        if (id == PulseWidthModulation::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, PulseWidthModulation::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == PumpConfigurationAndControl::Id) || ...))
    {
        if (id == PumpConfigurationAndControl::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, PumpConfigurationAndControl::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == PushAvStreamTransport::Id) || ...))
    {
        if (id == PushAvStreamTransport::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, PushAvStreamTransport::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == RadonConcentrationMeasurement::Id) || ...))
    {
        if (id == RadonConcentrationMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, RadonConcentrationMeasurement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == RefrigeratorAlarm::Id) || ...))
    {
        if (id == RefrigeratorAlarm::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, RefrigeratorAlarm::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == RefrigeratorAndTemperatureControlledCabinetMode::Id) || ...))
    {
        if (id == RefrigeratorAndTemperatureControlledCabinetMode::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry,
                                           RefrigeratorAndTemperatureControlledCabinetMode::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == RelativeHumidityMeasurement::Id) || ...))
    {
        if (id == RelativeHumidityMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, RelativeHumidityMeasurement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == RvcCleanMode::Id) || ...))
    {
        if (id == RvcCleanMode::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, RvcCleanMode::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == RvcOperationalState::Id) || ...))
    {
        if (id == RvcOperationalState::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, RvcOperationalState::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == RvcRunMode::Id) || ...))
    {
        if (id == RvcRunMode::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, RvcRunMode::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == SampleMei::Id) || ...))
    {
        if (id == SampleMei::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, SampleMei::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ScenesManagement::Id) || ...))
    {
        if (id == ScenesManagement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ScenesManagement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ServiceArea::Id) || ...))
    {
        if (id == ServiceArea::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ServiceArea::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == SmokeCoAlarm::Id) || ...))
    {
        if (id == SmokeCoAlarm::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, SmokeCoAlarm::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == SoftwareDiagnostics::Id) || ...))
    {
        if (id == SoftwareDiagnostics::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, SoftwareDiagnostics::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == SoilMeasurement::Id) || ...))
    {
        if (id == SoilMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, SoilMeasurement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == Switch::Id) || ...))
    {
        if (id == Switch::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Switch::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == TargetNavigator::Id) || ...))
    {
        if (id == TargetNavigator::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, TargetNavigator::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == TemperatureControl::Id) || ...))
    {
        if (id == TemperatureControl::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, TemperatureControl::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == TemperatureMeasurement::Id) || ...))
    {
        if (id == TemperatureMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, TemperatureMeasurement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == Thermostat::Id) || ...))
    {
        if (id == Thermostat::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Thermostat::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ThermostatUserInterfaceConfiguration::Id) || ...))
    {
        if (id == ThermostatUserInterfaceConfiguration::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ThermostatUserInterfaceConfiguration::Id>::EntryFor(
                command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ThreadBorderRouterManagement::Id) || ...))
    {
        if (id == ThreadBorderRouterManagement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ThreadBorderRouterManagement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ThreadNetworkDiagnostics::Id) || ...))
    {
        if (id == ThreadNetworkDiagnostics::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ThreadNetworkDiagnostics::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ThreadNetworkDirectory::Id) || ...))
    {
        if (id == ThreadNetworkDirectory::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ThreadNetworkDirectory::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == TimeFormatLocalization::Id) || ...))
    {
        if (id == TimeFormatLocalization::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, TimeFormatLocalization::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == Timer::Id) || ...))
    {
        if (id == Timer::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Timer::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == TimeSynchronization::Id) || ...))
    {
        if (id == TimeSynchronization::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, TimeSynchronization::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == TlsCertificateManagement::Id) || ...))
    {
        if (id == TlsCertificateManagement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, TlsCertificateManagement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == TlsClientManagement::Id) || ...))
    {
        if (id == TlsClientManagement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, TlsClientManagement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 ||
                  ((TClusterIds == TotalVolatileOrganicCompoundsConcentrationMeasurement::Id) || ...))
    {
        if (id == TotalVolatileOrganicCompoundsConcentrationMeasurement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry,
                                           TotalVolatileOrganicCompoundsConcentrationMeasurement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == UnitLocalization::Id) || ...))
    {
        if (id == UnitLocalization::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, UnitLocalization::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == UnitTesting::Id) || ...))
    {
        if (id == UnitTesting::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, UnitTesting::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == UserLabel::Id) || ...))
    {
        if (id == UserLabel::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, UserLabel::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ValveConfigurationAndControl::Id) || ...))
    {
        if (id == ValveConfigurationAndControl::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ValveConfigurationAndControl::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == WakeOnLan::Id) || ...))
    {
        if (id == WakeOnLan::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, WakeOnLan::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == WaterHeaterManagement::Id) || ...))
    {
        if (id == WaterHeaterManagement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, WaterHeaterManagement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == WaterHeaterMode::Id) || ...))
    {
        if (id == WaterHeaterMode::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, WaterHeaterMode::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == WaterTankLevelMonitoring::Id) || ...))
    {
        if (id == WaterTankLevelMonitoring::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, WaterTankLevelMonitoring::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == WebRTCTransportProvider::Id) || ...))
    {
        if (id == WebRTCTransportProvider::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, WebRTCTransportProvider::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == WebRTCTransportRequestor::Id) || ...))
    {
        if (id == WebRTCTransportRequestor::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, WebRTCTransportRequestor::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == WiFiNetworkDiagnostics::Id) || ...))
    {
        if (id == WiFiNetworkDiagnostics::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, WiFiNetworkDiagnostics::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == WiFiNetworkManagement::Id) || ...))
    {
        if (id == WiFiNetworkManagement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, WiFiNetworkManagement::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == WindowCovering::Id) || ...))
    {
        if (id == WindowCovering::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, WindowCovering::Id>::EntryFor(command);
    }
    if constexpr (sizeof...(TClusterIds) == 0 || ((TClusterIds == ZoneManagement::Id) || ...))
    {
        if (id == ZoneManagement::Id)
            return ClusterMetadataProvider<DataModel::AcceptedCommandEntry, ZoneManagement::Id>::EntryFor(command);
    }

    return std::nullopt;
}

} // namespace detail
} // namespace DataModel
} // namespace app
} // namespace chip
