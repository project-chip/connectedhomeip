/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#import "MTRCommandTimedCheck.h"

#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>

using namespace chip;
using namespace chip::app;

static BOOL CommandNeedsTimedInvokeInIdentifyCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Identify;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInGroupsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Groups;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInOnOffCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OnOff;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInLevelControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LevelControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInPulseWidthModulationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PulseWidthModulation;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInDescriptorCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Descriptor;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInBindingCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Binding;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInAccessControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AccessControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInActionsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Actions;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInBasicInformationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BasicInformation;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInOTASoftwareUpdateProviderCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OtaSoftwareUpdateProvider;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInOTASoftwareUpdateRequestorCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OtaSoftwareUpdateRequestor;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInLocalizationConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LocalizationConfiguration;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInTimeFormatLocalizationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TimeFormatLocalization;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInUnitLocalizationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::UnitLocalization;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInPowerSourceConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PowerSourceConfiguration;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInPowerSourceCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PowerSource;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInGeneralCommissioningCluster(AttributeId aAttributeId)
{
    using namespace Clusters::GeneralCommissioning;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInNetworkCommissioningCluster(AttributeId aAttributeId)
{
    using namespace Clusters::NetworkCommissioning;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInDiagnosticLogsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DiagnosticLogs;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInGeneralDiagnosticsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::GeneralDiagnostics;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInSoftwareDiagnosticsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::SoftwareDiagnostics;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInThreadNetworkDiagnosticsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ThreadNetworkDiagnostics;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInWiFiNetworkDiagnosticsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WiFiNetworkDiagnostics;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInEthernetNetworkDiagnosticsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::EthernetNetworkDiagnostics;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInTimeSynchronizationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TimeSynchronization;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInBridgedDeviceBasicInformationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BridgedDeviceBasicInformation;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInSwitchCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Switch;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInAdministratorCommissioningCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AdministratorCommissioning;
    switch (aAttributeId) {
    case Commands::OpenCommissioningWindow::Id: {
        return YES;
    }
    case Commands::OpenBasicCommissioningWindow::Id: {
        return YES;
    }
    case Commands::RevokeCommissioning::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInOperationalCredentialsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OperationalCredentials;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInGroupKeyManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::GroupKeyManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInFixedLabelCluster(AttributeId aAttributeId)
{
    using namespace Clusters::FixedLabel;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInUserLabelCluster(AttributeId aAttributeId)
{
    using namespace Clusters::UserLabel;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInBooleanStateCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BooleanState;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInICDManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::IcdManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInTimerCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Timer;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInOvenCavityOperationalStateCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OvenCavityOperationalState;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInOvenModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OvenMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInLaundryDryerControlsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LaundryDryerControls;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInModeSelectCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ModeSelect;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInLaundryWasherModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LaundryWasherMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInRefrigeratorAndTemperatureControlledCabinetModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RefrigeratorAndTemperatureControlledCabinetMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInLaundryWasherControlsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LaundryWasherControls;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInRVCRunModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RvcRunMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInRVCCleanModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RvcCleanMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInTemperatureControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TemperatureControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInRefrigeratorAlarmCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RefrigeratorAlarm;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInDishwasherModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DishwasherMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInAirQualityCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AirQuality;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInSmokeCOAlarmCluster(AttributeId aAttributeId)
{
    using namespace Clusters::SmokeCoAlarm;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInDishwasherAlarmCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DishwasherAlarm;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInMicrowaveOvenModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::MicrowaveOvenMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInMicrowaveOvenControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::MicrowaveOvenControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInOperationalStateCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OperationalState;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInRVCOperationalStateCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RvcOperationalState;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInScenesManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ScenesManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInHEPAFilterMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::HepaFilterMonitoring;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInActivatedCarbonFilterMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ActivatedCarbonFilterMonitoring;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInBooleanStateConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BooleanStateConfiguration;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInValveConfigurationAndControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ValveConfigurationAndControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInElectricalPowerMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ElectricalPowerMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInElectricalEnergyMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ElectricalEnergyMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInWaterHeaterManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WaterHeaterManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInDemandResponseLoadControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DemandResponseLoadControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInMessagesCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Messages;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInDeviceEnergyManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DeviceEnergyManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInEnergyEVSECluster(AttributeId aAttributeId)
{
    using namespace Clusters::EnergyEvse;
    switch (aAttributeId) {
    case Commands::Disable::Id: {
        return YES;
    }
    case Commands::EnableCharging::Id: {
        return YES;
    }
    case Commands::EnableDischarging::Id: {
        return YES;
    }
    case Commands::StartDiagnostics::Id: {
        return YES;
    }
    case Commands::SetTargets::Id: {
        return YES;
    }
    case Commands::GetTargets::Id: {
        return YES;
    }
    case Commands::ClearTargets::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInEnergyPreferenceCluster(AttributeId aAttributeId)
{
    using namespace Clusters::EnergyPreference;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInPowerTopologyCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PowerTopology;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInEnergyEVSEModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::EnergyEvseMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInWaterHeaterModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WaterHeaterMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInDeviceEnergyManagementModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DeviceEnergyManagementMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInDoorLockCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DoorLock;
    switch (aAttributeId) {
    case Commands::LockDoor::Id: {
        return YES;
    }
    case Commands::UnlockDoor::Id: {
        return YES;
    }
    case Commands::UnlockWithTimeout::Id: {
        return YES;
    }
    case Commands::SetUser::Id: {
        return YES;
    }
    case Commands::ClearUser::Id: {
        return YES;
    }
    case Commands::SetCredential::Id: {
        return YES;
    }
    case Commands::ClearCredential::Id: {
        return YES;
    }
    case Commands::UnboltDoor::Id: {
        return YES;
    }
    case Commands::SetAliroReaderConfig::Id: {
        return YES;
    }
    case Commands::ClearAliroReaderConfig::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInWindowCoveringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WindowCovering;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInServiceAreaCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ServiceArea;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInPumpConfigurationAndControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PumpConfigurationAndControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInThermostatCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Thermostat;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInFanControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::FanControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInThermostatUserInterfaceConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ThermostatUserInterfaceConfiguration;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInColorControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ColorControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInBallastConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BallastConfiguration;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInIlluminanceMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::IlluminanceMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInTemperatureMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TemperatureMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInPressureMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PressureMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInFlowMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::FlowMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInRelativeHumidityMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RelativeHumidityMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInOccupancySensingCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OccupancySensing;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInCarbonMonoxideConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CarbonMonoxideConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInCarbonDioxideConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CarbonDioxideConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInNitrogenDioxideConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::NitrogenDioxideConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInOzoneConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OzoneConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInPM25ConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Pm25ConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInFormaldehydeConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::FormaldehydeConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInPM1ConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Pm1ConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInPM10ConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Pm10ConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInTotalVolatileOrganicCompoundsConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInRadonConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RadonConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInWiFiNetworkManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WiFiNetworkManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInThreadBorderRouterManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ThreadBorderRouterManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInThreadNetworkDirectoryCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ThreadNetworkDirectory;
    switch (aAttributeId) {
    case Commands::AddNetwork::Id: {
        return YES;
    }
    case Commands::RemoveNetwork::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInWakeOnLANCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WakeOnLan;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInChannelCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Channel;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInTargetNavigatorCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TargetNavigator;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInMediaPlaybackCluster(AttributeId aAttributeId)
{
    using namespace Clusters::MediaPlayback;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInMediaInputCluster(AttributeId aAttributeId)
{
    using namespace Clusters::MediaInput;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInLowPowerCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LowPower;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInKeypadInputCluster(AttributeId aAttributeId)
{
    using namespace Clusters::KeypadInput;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInContentLauncherCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ContentLauncher;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInAudioOutputCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AudioOutput;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInApplicationLauncherCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ApplicationLauncher;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInApplicationBasicCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ApplicationBasic;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInAccountLoginCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AccountLogin;
    switch (aAttributeId) {
    case Commands::GetSetupPIN::Id: {
        return YES;
    }
    case Commands::Login::Id: {
        return YES;
    }
    case Commands::Logout::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInContentControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ContentControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInContentAppObserverCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ContentAppObserver;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInZoneManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ZoneManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInCameraAVStreamManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CameraAvStreamManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInCameraAVSettingsUserLevelManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CameraAvSettingsUserLevelManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInWebRTCTransportProviderCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WebRTCTransportProvider;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInWebRTCTransportRequestorCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WebRTCTransportRequestor;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInPushAVStreamTransportCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PushAvStreamTransport;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInChimeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Chime;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInEcosystemInformationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::EcosystemInformation;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInCommissionerControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CommissionerControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInTLSCertificateManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TlsCertificateManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInUnitTestingCluster(AttributeId aAttributeId)
{
    using namespace Clusters::UnitTesting;
    switch (aAttributeId) {
    case Commands::TimedInvokeRequest::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL CommandNeedsTimedInvokeInSampleMEICluster(AttributeId aAttributeId)
{
    using namespace Clusters::SampleMei;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}

BOOL MTRCommandNeedsTimedInvoke(NSNumber * _Nonnull aClusterID, NSNumber * _Nonnull aCommandID)
{
    ClusterId clusterID = static_cast<ClusterId>(aClusterID.unsignedLongLongValue);
    CommandId commandID = static_cast<CommandId>(aCommandID.unsignedLongLongValue);

    switch (clusterID) {
    case Clusters::Identify::Id: {
        return CommandNeedsTimedInvokeInIdentifyCluster(commandID);
    }
    case Clusters::Groups::Id: {
        return CommandNeedsTimedInvokeInGroupsCluster(commandID);
    }
    case Clusters::OnOff::Id: {
        return CommandNeedsTimedInvokeInOnOffCluster(commandID);
    }
    case Clusters::LevelControl::Id: {
        return CommandNeedsTimedInvokeInLevelControlCluster(commandID);
    }
    case Clusters::PulseWidthModulation::Id: {
        return CommandNeedsTimedInvokeInPulseWidthModulationCluster(commandID);
    }
    case Clusters::Descriptor::Id: {
        return CommandNeedsTimedInvokeInDescriptorCluster(commandID);
    }
    case Clusters::Binding::Id: {
        return CommandNeedsTimedInvokeInBindingCluster(commandID);
    }
    case Clusters::AccessControl::Id: {
        return CommandNeedsTimedInvokeInAccessControlCluster(commandID);
    }
    case Clusters::Actions::Id: {
        return CommandNeedsTimedInvokeInActionsCluster(commandID);
    }
    case Clusters::BasicInformation::Id: {
        return CommandNeedsTimedInvokeInBasicInformationCluster(commandID);
    }
    case Clusters::OtaSoftwareUpdateProvider::Id: {
        return CommandNeedsTimedInvokeInOTASoftwareUpdateProviderCluster(commandID);
    }
    case Clusters::OtaSoftwareUpdateRequestor::Id: {
        return CommandNeedsTimedInvokeInOTASoftwareUpdateRequestorCluster(commandID);
    }
    case Clusters::LocalizationConfiguration::Id: {
        return CommandNeedsTimedInvokeInLocalizationConfigurationCluster(commandID);
    }
    case Clusters::TimeFormatLocalization::Id: {
        return CommandNeedsTimedInvokeInTimeFormatLocalizationCluster(commandID);
    }
    case Clusters::UnitLocalization::Id: {
        return CommandNeedsTimedInvokeInUnitLocalizationCluster(commandID);
    }
    case Clusters::PowerSourceConfiguration::Id: {
        return CommandNeedsTimedInvokeInPowerSourceConfigurationCluster(commandID);
    }
    case Clusters::PowerSource::Id: {
        return CommandNeedsTimedInvokeInPowerSourceCluster(commandID);
    }
    case Clusters::GeneralCommissioning::Id: {
        return CommandNeedsTimedInvokeInGeneralCommissioningCluster(commandID);
    }
    case Clusters::NetworkCommissioning::Id: {
        return CommandNeedsTimedInvokeInNetworkCommissioningCluster(commandID);
    }
    case Clusters::DiagnosticLogs::Id: {
        return CommandNeedsTimedInvokeInDiagnosticLogsCluster(commandID);
    }
    case Clusters::GeneralDiagnostics::Id: {
        return CommandNeedsTimedInvokeInGeneralDiagnosticsCluster(commandID);
    }
    case Clusters::SoftwareDiagnostics::Id: {
        return CommandNeedsTimedInvokeInSoftwareDiagnosticsCluster(commandID);
    }
    case Clusters::ThreadNetworkDiagnostics::Id: {
        return CommandNeedsTimedInvokeInThreadNetworkDiagnosticsCluster(commandID);
    }
    case Clusters::WiFiNetworkDiagnostics::Id: {
        return CommandNeedsTimedInvokeInWiFiNetworkDiagnosticsCluster(commandID);
    }
    case Clusters::EthernetNetworkDiagnostics::Id: {
        return CommandNeedsTimedInvokeInEthernetNetworkDiagnosticsCluster(commandID);
    }
    case Clusters::TimeSynchronization::Id: {
        return CommandNeedsTimedInvokeInTimeSynchronizationCluster(commandID);
    }
    case Clusters::BridgedDeviceBasicInformation::Id: {
        return CommandNeedsTimedInvokeInBridgedDeviceBasicInformationCluster(commandID);
    }
    case Clusters::Switch::Id: {
        return CommandNeedsTimedInvokeInSwitchCluster(commandID);
    }
    case Clusters::AdministratorCommissioning::Id: {
        return CommandNeedsTimedInvokeInAdministratorCommissioningCluster(commandID);
    }
    case Clusters::OperationalCredentials::Id: {
        return CommandNeedsTimedInvokeInOperationalCredentialsCluster(commandID);
    }
    case Clusters::GroupKeyManagement::Id: {
        return CommandNeedsTimedInvokeInGroupKeyManagementCluster(commandID);
    }
    case Clusters::FixedLabel::Id: {
        return CommandNeedsTimedInvokeInFixedLabelCluster(commandID);
    }
    case Clusters::UserLabel::Id: {
        return CommandNeedsTimedInvokeInUserLabelCluster(commandID);
    }
    case Clusters::BooleanState::Id: {
        return CommandNeedsTimedInvokeInBooleanStateCluster(commandID);
    }
    case Clusters::IcdManagement::Id: {
        return CommandNeedsTimedInvokeInICDManagementCluster(commandID);
    }
    case Clusters::Timer::Id: {
        return CommandNeedsTimedInvokeInTimerCluster(commandID);
    }
    case Clusters::OvenCavityOperationalState::Id: {
        return CommandNeedsTimedInvokeInOvenCavityOperationalStateCluster(commandID);
    }
    case Clusters::OvenMode::Id: {
        return CommandNeedsTimedInvokeInOvenModeCluster(commandID);
    }
    case Clusters::LaundryDryerControls::Id: {
        return CommandNeedsTimedInvokeInLaundryDryerControlsCluster(commandID);
    }
    case Clusters::ModeSelect::Id: {
        return CommandNeedsTimedInvokeInModeSelectCluster(commandID);
    }
    case Clusters::LaundryWasherMode::Id: {
        return CommandNeedsTimedInvokeInLaundryWasherModeCluster(commandID);
    }
    case Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Id: {
        return CommandNeedsTimedInvokeInRefrigeratorAndTemperatureControlledCabinetModeCluster(commandID);
    }
    case Clusters::LaundryWasherControls::Id: {
        return CommandNeedsTimedInvokeInLaundryWasherControlsCluster(commandID);
    }
    case Clusters::RvcRunMode::Id: {
        return CommandNeedsTimedInvokeInRVCRunModeCluster(commandID);
    }
    case Clusters::RvcCleanMode::Id: {
        return CommandNeedsTimedInvokeInRVCCleanModeCluster(commandID);
    }
    case Clusters::TemperatureControl::Id: {
        return CommandNeedsTimedInvokeInTemperatureControlCluster(commandID);
    }
    case Clusters::RefrigeratorAlarm::Id: {
        return CommandNeedsTimedInvokeInRefrigeratorAlarmCluster(commandID);
    }
    case Clusters::DishwasherMode::Id: {
        return CommandNeedsTimedInvokeInDishwasherModeCluster(commandID);
    }
    case Clusters::AirQuality::Id: {
        return CommandNeedsTimedInvokeInAirQualityCluster(commandID);
    }
    case Clusters::SmokeCoAlarm::Id: {
        return CommandNeedsTimedInvokeInSmokeCOAlarmCluster(commandID);
    }
    case Clusters::DishwasherAlarm::Id: {
        return CommandNeedsTimedInvokeInDishwasherAlarmCluster(commandID);
    }
    case Clusters::MicrowaveOvenMode::Id: {
        return CommandNeedsTimedInvokeInMicrowaveOvenModeCluster(commandID);
    }
    case Clusters::MicrowaveOvenControl::Id: {
        return CommandNeedsTimedInvokeInMicrowaveOvenControlCluster(commandID);
    }
    case Clusters::OperationalState::Id: {
        return CommandNeedsTimedInvokeInOperationalStateCluster(commandID);
    }
    case Clusters::RvcOperationalState::Id: {
        return CommandNeedsTimedInvokeInRVCOperationalStateCluster(commandID);
    }
    case Clusters::ScenesManagement::Id: {
        return CommandNeedsTimedInvokeInScenesManagementCluster(commandID);
    }
    case Clusters::HepaFilterMonitoring::Id: {
        return CommandNeedsTimedInvokeInHEPAFilterMonitoringCluster(commandID);
    }
    case Clusters::ActivatedCarbonFilterMonitoring::Id: {
        return CommandNeedsTimedInvokeInActivatedCarbonFilterMonitoringCluster(commandID);
    }
    case Clusters::BooleanStateConfiguration::Id: {
        return CommandNeedsTimedInvokeInBooleanStateConfigurationCluster(commandID);
    }
    case Clusters::ValveConfigurationAndControl::Id: {
        return CommandNeedsTimedInvokeInValveConfigurationAndControlCluster(commandID);
    }
    case Clusters::ElectricalPowerMeasurement::Id: {
        return CommandNeedsTimedInvokeInElectricalPowerMeasurementCluster(commandID);
    }
    case Clusters::ElectricalEnergyMeasurement::Id: {
        return CommandNeedsTimedInvokeInElectricalEnergyMeasurementCluster(commandID);
    }
    case Clusters::WaterHeaterManagement::Id: {
        return CommandNeedsTimedInvokeInWaterHeaterManagementCluster(commandID);
    }
    case Clusters::DemandResponseLoadControl::Id: {
        return CommandNeedsTimedInvokeInDemandResponseLoadControlCluster(commandID);
    }
    case Clusters::Messages::Id: {
        return CommandNeedsTimedInvokeInMessagesCluster(commandID);
    }
    case Clusters::DeviceEnergyManagement::Id: {
        return CommandNeedsTimedInvokeInDeviceEnergyManagementCluster(commandID);
    }
    case Clusters::EnergyEvse::Id: {
        return CommandNeedsTimedInvokeInEnergyEVSECluster(commandID);
    }
    case Clusters::EnergyPreference::Id: {
        return CommandNeedsTimedInvokeInEnergyPreferenceCluster(commandID);
    }
    case Clusters::PowerTopology::Id: {
        return CommandNeedsTimedInvokeInPowerTopologyCluster(commandID);
    }
    case Clusters::EnergyEvseMode::Id: {
        return CommandNeedsTimedInvokeInEnergyEVSEModeCluster(commandID);
    }
    case Clusters::WaterHeaterMode::Id: {
        return CommandNeedsTimedInvokeInWaterHeaterModeCluster(commandID);
    }
    case Clusters::DeviceEnergyManagementMode::Id: {
        return CommandNeedsTimedInvokeInDeviceEnergyManagementModeCluster(commandID);
    }
    case Clusters::DoorLock::Id: {
        return CommandNeedsTimedInvokeInDoorLockCluster(commandID);
    }
    case Clusters::WindowCovering::Id: {
        return CommandNeedsTimedInvokeInWindowCoveringCluster(commandID);
    }
    case Clusters::ServiceArea::Id: {
        return CommandNeedsTimedInvokeInServiceAreaCluster(commandID);
    }
    case Clusters::PumpConfigurationAndControl::Id: {
        return CommandNeedsTimedInvokeInPumpConfigurationAndControlCluster(commandID);
    }
    case Clusters::Thermostat::Id: {
        return CommandNeedsTimedInvokeInThermostatCluster(commandID);
    }
    case Clusters::FanControl::Id: {
        return CommandNeedsTimedInvokeInFanControlCluster(commandID);
    }
    case Clusters::ThermostatUserInterfaceConfiguration::Id: {
        return CommandNeedsTimedInvokeInThermostatUserInterfaceConfigurationCluster(commandID);
    }
    case Clusters::ColorControl::Id: {
        return CommandNeedsTimedInvokeInColorControlCluster(commandID);
    }
    case Clusters::BallastConfiguration::Id: {
        return CommandNeedsTimedInvokeInBallastConfigurationCluster(commandID);
    }
    case Clusters::IlluminanceMeasurement::Id: {
        return CommandNeedsTimedInvokeInIlluminanceMeasurementCluster(commandID);
    }
    case Clusters::TemperatureMeasurement::Id: {
        return CommandNeedsTimedInvokeInTemperatureMeasurementCluster(commandID);
    }
    case Clusters::PressureMeasurement::Id: {
        return CommandNeedsTimedInvokeInPressureMeasurementCluster(commandID);
    }
    case Clusters::FlowMeasurement::Id: {
        return CommandNeedsTimedInvokeInFlowMeasurementCluster(commandID);
    }
    case Clusters::RelativeHumidityMeasurement::Id: {
        return CommandNeedsTimedInvokeInRelativeHumidityMeasurementCluster(commandID);
    }
    case Clusters::OccupancySensing::Id: {
        return CommandNeedsTimedInvokeInOccupancySensingCluster(commandID);
    }
    case Clusters::CarbonMonoxideConcentrationMeasurement::Id: {
        return CommandNeedsTimedInvokeInCarbonMonoxideConcentrationMeasurementCluster(commandID);
    }
    case Clusters::CarbonDioxideConcentrationMeasurement::Id: {
        return CommandNeedsTimedInvokeInCarbonDioxideConcentrationMeasurementCluster(commandID);
    }
    case Clusters::NitrogenDioxideConcentrationMeasurement::Id: {
        return CommandNeedsTimedInvokeInNitrogenDioxideConcentrationMeasurementCluster(commandID);
    }
    case Clusters::OzoneConcentrationMeasurement::Id: {
        return CommandNeedsTimedInvokeInOzoneConcentrationMeasurementCluster(commandID);
    }
    case Clusters::Pm25ConcentrationMeasurement::Id: {
        return CommandNeedsTimedInvokeInPM25ConcentrationMeasurementCluster(commandID);
    }
    case Clusters::FormaldehydeConcentrationMeasurement::Id: {
        return CommandNeedsTimedInvokeInFormaldehydeConcentrationMeasurementCluster(commandID);
    }
    case Clusters::Pm1ConcentrationMeasurement::Id: {
        return CommandNeedsTimedInvokeInPM1ConcentrationMeasurementCluster(commandID);
    }
    case Clusters::Pm10ConcentrationMeasurement::Id: {
        return CommandNeedsTimedInvokeInPM10ConcentrationMeasurementCluster(commandID);
    }
    case Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::Id: {
        return CommandNeedsTimedInvokeInTotalVolatileOrganicCompoundsConcentrationMeasurementCluster(commandID);
    }
    case Clusters::RadonConcentrationMeasurement::Id: {
        return CommandNeedsTimedInvokeInRadonConcentrationMeasurementCluster(commandID);
    }
    case Clusters::WiFiNetworkManagement::Id: {
        return CommandNeedsTimedInvokeInWiFiNetworkManagementCluster(commandID);
    }
    case Clusters::ThreadBorderRouterManagement::Id: {
        return CommandNeedsTimedInvokeInThreadBorderRouterManagementCluster(commandID);
    }
    case Clusters::ThreadNetworkDirectory::Id: {
        return CommandNeedsTimedInvokeInThreadNetworkDirectoryCluster(commandID);
    }
    case Clusters::WakeOnLan::Id: {
        return CommandNeedsTimedInvokeInWakeOnLANCluster(commandID);
    }
    case Clusters::Channel::Id: {
        return CommandNeedsTimedInvokeInChannelCluster(commandID);
    }
    case Clusters::TargetNavigator::Id: {
        return CommandNeedsTimedInvokeInTargetNavigatorCluster(commandID);
    }
    case Clusters::MediaPlayback::Id: {
        return CommandNeedsTimedInvokeInMediaPlaybackCluster(commandID);
    }
    case Clusters::MediaInput::Id: {
        return CommandNeedsTimedInvokeInMediaInputCluster(commandID);
    }
    case Clusters::LowPower::Id: {
        return CommandNeedsTimedInvokeInLowPowerCluster(commandID);
    }
    case Clusters::KeypadInput::Id: {
        return CommandNeedsTimedInvokeInKeypadInputCluster(commandID);
    }
    case Clusters::ContentLauncher::Id: {
        return CommandNeedsTimedInvokeInContentLauncherCluster(commandID);
    }
    case Clusters::AudioOutput::Id: {
        return CommandNeedsTimedInvokeInAudioOutputCluster(commandID);
    }
    case Clusters::ApplicationLauncher::Id: {
        return CommandNeedsTimedInvokeInApplicationLauncherCluster(commandID);
    }
    case Clusters::ApplicationBasic::Id: {
        return CommandNeedsTimedInvokeInApplicationBasicCluster(commandID);
    }
    case Clusters::AccountLogin::Id: {
        return CommandNeedsTimedInvokeInAccountLoginCluster(commandID);
    }
    case Clusters::ContentControl::Id: {
        return CommandNeedsTimedInvokeInContentControlCluster(commandID);
    }
    case Clusters::ContentAppObserver::Id: {
        return CommandNeedsTimedInvokeInContentAppObserverCluster(commandID);
    }
    case Clusters::ZoneManagement::Id: {
        return CommandNeedsTimedInvokeInZoneManagementCluster(commandID);
    }
    case Clusters::CameraAvStreamManagement::Id: {
        return CommandNeedsTimedInvokeInCameraAVStreamManagementCluster(commandID);
    }
    case Clusters::CameraAvSettingsUserLevelManagement::Id: {
        return CommandNeedsTimedInvokeInCameraAVSettingsUserLevelManagementCluster(commandID);
    }
    case Clusters::WebRTCTransportProvider::Id: {
        return CommandNeedsTimedInvokeInWebRTCTransportProviderCluster(commandID);
    }
    case Clusters::WebRTCTransportRequestor::Id: {
        return CommandNeedsTimedInvokeInWebRTCTransportRequestorCluster(commandID);
    }
    case Clusters::PushAvStreamTransport::Id: {
        return CommandNeedsTimedInvokeInPushAVStreamTransportCluster(commandID);
    }
    case Clusters::Chime::Id: {
        return CommandNeedsTimedInvokeInChimeCluster(commandID);
    }
    case Clusters::EcosystemInformation::Id: {
        return CommandNeedsTimedInvokeInEcosystemInformationCluster(commandID);
    }
    case Clusters::CommissionerControl::Id: {
        return CommandNeedsTimedInvokeInCommissionerControlCluster(commandID);
    }
    case Clusters::TlsCertificateManagement::Id: {
        return CommandNeedsTimedInvokeInTLSCertificateManagementCluster(commandID);
    }
    case Clusters::UnitTesting::Id: {
        return CommandNeedsTimedInvokeInUnitTestingCluster(commandID);
    }
    case Clusters::SampleMei::Id: {
        return CommandNeedsTimedInvokeInSampleMEICluster(commandID);
    }
    default: {
        return NO;
    }
    }
}
