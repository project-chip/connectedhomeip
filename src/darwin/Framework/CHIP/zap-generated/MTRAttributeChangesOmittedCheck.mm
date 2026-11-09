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

#import "MTRAttributeChangesOmittedCheck.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

using namespace chip;
using namespace chip::app;

static BOOL AttributeHasChangesOmittedQualityInIdentifyCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Identify;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInGroupsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Groups;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInOnOffCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OnOff;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInLevelControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LevelControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInPulseWidthModulationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PulseWidthModulation;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInDescriptorCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Descriptor;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInBindingCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Binding;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInAccessControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AccessControl;
    switch (aAttributeId) {
    case Attributes::AuxiliaryACL::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInActionsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Actions;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInBasicInformationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BasicInformation;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInOTASoftwareUpdateProviderCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OtaSoftwareUpdateProvider;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInOTASoftwareUpdateRequestorCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OtaSoftwareUpdateRequestor;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInLocalizationConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LocalizationConfiguration;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInTimeFormatLocalizationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TimeFormatLocalization;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInUnitLocalizationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::UnitLocalization;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInPowerSourceConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PowerSourceConfiguration;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInPowerSourceCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PowerSource;
    switch (aAttributeId) {
    case Attributes::WiredAssessedInputVoltage::Id: {
        return YES;
    }
    case Attributes::WiredAssessedInputFrequency::Id: {
        return YES;
    }
    case Attributes::WiredAssessedCurrent::Id: {
        return YES;
    }
    case Attributes::BatVoltage::Id: {
        return YES;
    }
    case Attributes::BatChargingCurrent::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInGeneralCommissioningCluster(AttributeId aAttributeId)
{
    using namespace Clusters::GeneralCommissioning;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInNetworkCommissioningCluster(AttributeId aAttributeId)
{
    using namespace Clusters::NetworkCommissioning;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInDiagnosticLogsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DiagnosticLogs;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInGeneralDiagnosticsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::GeneralDiagnostics;
    switch (aAttributeId) {
    case Attributes::UpTime::Id: {
        return YES;
    }
    case Attributes::TotalOperationalHours::Id: {
        return YES;
    }
    case Attributes::DeviceLoadStatus::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInSoftwareDiagnosticsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::SoftwareDiagnostics;
    switch (aAttributeId) {
    case Attributes::ThreadMetrics::Id: {
        return YES;
    }
    case Attributes::CurrentHeapFree::Id: {
        return YES;
    }
    case Attributes::CurrentHeapUsed::Id: {
        return YES;
    }
    case Attributes::CurrentHeapHighWatermark::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInThreadNetworkDiagnosticsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ThreadNetworkDiagnostics;
    switch (aAttributeId) {
    case Attributes::OverrunCount::Id: {
        return YES;
    }
    case Attributes::DetachedRoleCount::Id: {
        return YES;
    }
    case Attributes::ChildRoleCount::Id: {
        return YES;
    }
    case Attributes::RouterRoleCount::Id: {
        return YES;
    }
    case Attributes::LeaderRoleCount::Id: {
        return YES;
    }
    case Attributes::AttachAttemptCount::Id: {
        return YES;
    }
    case Attributes::PartitionIdChangeCount::Id: {
        return YES;
    }
    case Attributes::BetterPartitionAttachAttemptCount::Id: {
        return YES;
    }
    case Attributes::ParentChangeCount::Id: {
        return YES;
    }
    case Attributes::TxTotalCount::Id: {
        return YES;
    }
    case Attributes::TxUnicastCount::Id: {
        return YES;
    }
    case Attributes::TxBroadcastCount::Id: {
        return YES;
    }
    case Attributes::TxAckRequestedCount::Id: {
        return YES;
    }
    case Attributes::TxAckedCount::Id: {
        return YES;
    }
    case Attributes::TxNoAckRequestedCount::Id: {
        return YES;
    }
    case Attributes::TxDataCount::Id: {
        return YES;
    }
    case Attributes::TxDataPollCount::Id: {
        return YES;
    }
    case Attributes::TxBeaconCount::Id: {
        return YES;
    }
    case Attributes::TxBeaconRequestCount::Id: {
        return YES;
    }
    case Attributes::TxOtherCount::Id: {
        return YES;
    }
    case Attributes::TxRetryCount::Id: {
        return YES;
    }
    case Attributes::TxDirectMaxRetryExpiryCount::Id: {
        return YES;
    }
    case Attributes::TxIndirectMaxRetryExpiryCount::Id: {
        return YES;
    }
    case Attributes::TxErrCcaCount::Id: {
        return YES;
    }
    case Attributes::TxErrAbortCount::Id: {
        return YES;
    }
    case Attributes::TxErrBusyChannelCount::Id: {
        return YES;
    }
    case Attributes::RxTotalCount::Id: {
        return YES;
    }
    case Attributes::RxUnicastCount::Id: {
        return YES;
    }
    case Attributes::RxBroadcastCount::Id: {
        return YES;
    }
    case Attributes::RxDataCount::Id: {
        return YES;
    }
    case Attributes::RxDataPollCount::Id: {
        return YES;
    }
    case Attributes::RxBeaconCount::Id: {
        return YES;
    }
    case Attributes::RxBeaconRequestCount::Id: {
        return YES;
    }
    case Attributes::RxOtherCount::Id: {
        return YES;
    }
    case Attributes::RxAddressFilteredCount::Id: {
        return YES;
    }
    case Attributes::RxDestAddrFilteredCount::Id: {
        return YES;
    }
    case Attributes::RxDuplicatedCount::Id: {
        return YES;
    }
    case Attributes::RxErrNoFrameCount::Id: {
        return YES;
    }
    case Attributes::RxErrUnknownNeighborCount::Id: {
        return YES;
    }
    case Attributes::RxErrInvalidSrcAddrCount::Id: {
        return YES;
    }
    case Attributes::RxErrSecCount::Id: {
        return YES;
    }
    case Attributes::RxErrFcsCount::Id: {
        return YES;
    }
    case Attributes::RxErrOtherCount::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInWiFiNetworkDiagnosticsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WiFiNetworkDiagnostics;
    switch (aAttributeId) {
    case Attributes::Rssi::Id: {
        return YES;
    }
    case Attributes::BeaconLostCount::Id: {
        return YES;
    }
    case Attributes::BeaconRxCount::Id: {
        return YES;
    }
    case Attributes::PacketMulticastRxCount::Id: {
        return YES;
    }
    case Attributes::PacketMulticastTxCount::Id: {
        return YES;
    }
    case Attributes::PacketUnicastRxCount::Id: {
        return YES;
    }
    case Attributes::PacketUnicastTxCount::Id: {
        return YES;
    }
    case Attributes::CurrentMaxRate::Id: {
        return YES;
    }
    case Attributes::OverrunCount::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInEthernetNetworkDiagnosticsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::EthernetNetworkDiagnostics;
    switch (aAttributeId) {
    case Attributes::PacketRxCount::Id: {
        return YES;
    }
    case Attributes::PacketTxCount::Id: {
        return YES;
    }
    case Attributes::TxErrCount::Id: {
        return YES;
    }
    case Attributes::CollisionCount::Id: {
        return YES;
    }
    case Attributes::OverrunCount::Id: {
        return YES;
    }
    case Attributes::CarrierDetect::Id: {
        return YES;
    }
    case Attributes::TimeSinceReset::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInTimeSynchronizationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TimeSynchronization;
    switch (aAttributeId) {
    case Attributes::UTCTime::Id: {
        return YES;
    }
    case Attributes::LocalTime::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInBridgedDeviceBasicInformationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BridgedDeviceBasicInformation;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInSwitchCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Switch;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInAdministratorCommissioningCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AdministratorCommissioning;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInOperationalCredentialsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OperationalCredentials;
    switch (aAttributeId) {
    case Attributes::NOCs::Id: {
        return YES;
    }
    case Attributes::TrustedRootCertificates::Id: {
        return YES;
    }
    case Attributes::PQCDeviceAttestationProfile::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInGroupKeyManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::GroupKeyManagement;
    switch (aAttributeId) {
    case Attributes::GroupKeyMap::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInFixedLabelCluster(AttributeId aAttributeId)
{
    using namespace Clusters::FixedLabel;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInUserLabelCluster(AttributeId aAttributeId)
{
    using namespace Clusters::UserLabel;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInBooleanStateCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BooleanState;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInICDManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::IcdManagement;
    switch (aAttributeId) {
    case Attributes::ICDCounter::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInOvenCavityOperationalStateCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OvenCavityOperationalState;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInOvenModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OvenMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInLaundryDryerControlsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LaundryDryerControls;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInTemperatureControlledCabinetTopologyCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TemperatureControlledCabinetTopology;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInModeSelectCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ModeSelect;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInLaundryWasherModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LaundryWasherMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInRefrigeratorAndTemperatureControlledCabinetModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RefrigeratorAndTemperatureControlledCabinetMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInLaundryWasherControlsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LaundryWasherControls;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInRVCRunModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RvcRunMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInRVCCleanModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RvcCleanMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInTemperatureControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TemperatureControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInRefrigeratorAlarmCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RefrigeratorAlarm;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInDishwasherModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DishwasherMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInAirQualityCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AirQuality;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInSmokeCOAlarmCluster(AttributeId aAttributeId)
{
    using namespace Clusters::SmokeCoAlarm;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInDishwasherAlarmCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DishwasherAlarm;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInMicrowaveOvenModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::MicrowaveOvenMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInMicrowaveOvenControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::MicrowaveOvenControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInOperationalStateCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OperationalState;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInRVCOperationalStateCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RvcOperationalState;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInScenesManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ScenesManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInThermostatModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ThermostatMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInGroupcastCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Groupcast;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInHEPAFilterMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::HepaFilterMonitoring;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInActivatedCarbonFilterMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ActivatedCarbonFilterMonitoring;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInWaterTankLevelMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WaterTankLevelMonitoring;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInBooleanStateConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BooleanStateConfiguration;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInValveConfigurationAndControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ValveConfigurationAndControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInElectricalPowerMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ElectricalPowerMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInElectricalEnergyMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ElectricalEnergyMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInWaterHeaterManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WaterHeaterManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInCommodityPriceCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CommodityPrice;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInMessagesCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Messages;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInDeviceEnergyManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DeviceEnergyManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInEnergyEVSECluster(AttributeId aAttributeId)
{
    using namespace Clusters::EnergyEvse;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInEnergyPreferenceCluster(AttributeId aAttributeId)
{
    using namespace Clusters::EnergyPreference;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInPowerTopologyCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PowerTopology;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInEnergyEVSEModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::EnergyEvseMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInWaterHeaterModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WaterHeaterMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInDeviceEnergyManagementModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DeviceEnergyManagementMode;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInElectricalGridConditionsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ElectricalGridConditions;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInElectricalAlarmCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ElectricalAlarm;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInElectricalDistributionCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ElectricalDistribution;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInElectricalProtectionAlarmCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ElectricalProtectionAlarm;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInDoorLockCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DoorLock;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInWindowCoveringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WindowCovering;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInClosureControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ClosureControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInClosureDimensionCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ClosureDimension;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInServiceAreaCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ServiceArea;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInPumpConfigurationAndControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PumpConfigurationAndControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInThermostatCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Thermostat;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInFanControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::FanControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInThermostatUserInterfaceConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ThermostatUserInterfaceConfiguration;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInHumidistatCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Humidistat;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInColorControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ColorControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInBallastConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BallastConfiguration;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInDynamicLightingCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DynamicLighting;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInIlluminanceMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::IlluminanceMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInTemperatureMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TemperatureMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInPressureMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PressureMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInFlowMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::FlowMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInRelativeHumidityMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RelativeHumidityMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInOccupancySensingCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OccupancySensing;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInCarbonMonoxideConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CarbonMonoxideConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInCarbonDioxideConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CarbonDioxideConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInNitrogenDioxideConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::NitrogenDioxideConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInOzoneConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OzoneConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInPM25ConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Pm25ConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInFormaldehydeConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::FormaldehydeConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInPM1ConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Pm1ConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInPM10ConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Pm10ConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInTotalVolatileOrganicCompoundsConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInRadonConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RadonConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInSoilMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::SoilMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInAmbientContextSensingCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AmbientContextSensing;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInAmbientSensingUnionCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AmbientSensingUnion;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInProximityRangingCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ProximityRanging;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInSmokeConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::SmokeConcentrationMeasurement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInNetworkIdentityManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::NetworkIdentityManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInWiFiNetworkManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WiFiNetworkManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInThreadBorderRouterManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ThreadBorderRouterManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInThreadNetworkDirectoryCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ThreadNetworkDirectory;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInCommissioningProxyCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CommissioningProxy;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInWakeOnLANCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WakeOnLan;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInChannelCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Channel;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInTargetNavigatorCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TargetNavigator;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInMediaPlaybackCluster(AttributeId aAttributeId)
{
    using namespace Clusters::MediaPlayback;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInMediaInputCluster(AttributeId aAttributeId)
{
    using namespace Clusters::MediaInput;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInLowPowerCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LowPower;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInKeypadInputCluster(AttributeId aAttributeId)
{
    using namespace Clusters::KeypadInput;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInContentLauncherCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ContentLauncher;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInAudioOutputCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AudioOutput;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInApplicationLauncherCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ApplicationLauncher;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInApplicationBasicCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ApplicationBasic;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInAccountLoginCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AccountLogin;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInContentControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ContentControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInContentAppObserverCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ContentAppObserver;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInMediaFileManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::MediaFileManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInAudioControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AudioControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInZoneManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ZoneManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInCameraAVStreamManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CameraAvStreamManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInCameraAVSettingsUserLevelManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CameraAvSettingsUserLevelManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInWebRTCTransportProviderCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WebRTCTransportProvider;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInWebRTCTransportRequestorCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WebRTCTransportRequestor;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInPushAVStreamTransportCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PushAvStreamTransport;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInChimeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Chime;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInAVAnalysisCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AvAnalysis;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInCommodityTariffCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CommodityTariff;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInEcosystemInformationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::EcosystemInformation;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInCommissionerControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CommissionerControl;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInJointFabricDatastoreCluster(AttributeId aAttributeId)
{
    using namespace Clusters::JointFabricDatastore;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInJointFabricAdministratorCluster(AttributeId aAttributeId)
{
    using namespace Clusters::JointFabricAdministrator;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInTLSCertificateManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TlsCertificateManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInTLSClientManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TlsClientManagement;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInMeterIdentificationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::MeterIdentification;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInCommodityMeteringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CommodityMetering;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInUnitTestingCluster(AttributeId aAttributeId)
{
    using namespace Clusters::UnitTesting;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInSampleMEICluster(AttributeId aAttributeId)
{
    using namespace Clusters::SampleMei;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}
static BOOL AttributeHasChangesOmittedQualityInTestHiddenManufacturerSpecificCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TestHiddenManufacturerSpecific;
    switch (aAttributeId) {
    default: {
        return NO;
    }
    }
}

BOOL MTRAttributeHasChangesOmittedQuality(ClusterId aClusterId, AttributeId aAttributeId)
{
    switch (aClusterId) {
    case Clusters::Identify::Id: {
        return AttributeHasChangesOmittedQualityInIdentifyCluster(aAttributeId);
    }
    case Clusters::Groups::Id: {
        return AttributeHasChangesOmittedQualityInGroupsCluster(aAttributeId);
    }
    case Clusters::OnOff::Id: {
        return AttributeHasChangesOmittedQualityInOnOffCluster(aAttributeId);
    }
    case Clusters::LevelControl::Id: {
        return AttributeHasChangesOmittedQualityInLevelControlCluster(aAttributeId);
    }
    case Clusters::PulseWidthModulation::Id: {
        return AttributeHasChangesOmittedQualityInPulseWidthModulationCluster(aAttributeId);
    }
    case Clusters::Descriptor::Id: {
        return AttributeHasChangesOmittedQualityInDescriptorCluster(aAttributeId);
    }
    case Clusters::Binding::Id: {
        return AttributeHasChangesOmittedQualityInBindingCluster(aAttributeId);
    }
    case Clusters::AccessControl::Id: {
        return AttributeHasChangesOmittedQualityInAccessControlCluster(aAttributeId);
    }
    case Clusters::Actions::Id: {
        return AttributeHasChangesOmittedQualityInActionsCluster(aAttributeId);
    }
    case Clusters::BasicInformation::Id: {
        return AttributeHasChangesOmittedQualityInBasicInformationCluster(aAttributeId);
    }
    case Clusters::OtaSoftwareUpdateProvider::Id: {
        return AttributeHasChangesOmittedQualityInOTASoftwareUpdateProviderCluster(aAttributeId);
    }
    case Clusters::OtaSoftwareUpdateRequestor::Id: {
        return AttributeHasChangesOmittedQualityInOTASoftwareUpdateRequestorCluster(aAttributeId);
    }
    case Clusters::LocalizationConfiguration::Id: {
        return AttributeHasChangesOmittedQualityInLocalizationConfigurationCluster(aAttributeId);
    }
    case Clusters::TimeFormatLocalization::Id: {
        return AttributeHasChangesOmittedQualityInTimeFormatLocalizationCluster(aAttributeId);
    }
    case Clusters::UnitLocalization::Id: {
        return AttributeHasChangesOmittedQualityInUnitLocalizationCluster(aAttributeId);
    }
    case Clusters::PowerSourceConfiguration::Id: {
        return AttributeHasChangesOmittedQualityInPowerSourceConfigurationCluster(aAttributeId);
    }
    case Clusters::PowerSource::Id: {
        return AttributeHasChangesOmittedQualityInPowerSourceCluster(aAttributeId);
    }
    case Clusters::GeneralCommissioning::Id: {
        return AttributeHasChangesOmittedQualityInGeneralCommissioningCluster(aAttributeId);
    }
    case Clusters::NetworkCommissioning::Id: {
        return AttributeHasChangesOmittedQualityInNetworkCommissioningCluster(aAttributeId);
    }
    case Clusters::DiagnosticLogs::Id: {
        return AttributeHasChangesOmittedQualityInDiagnosticLogsCluster(aAttributeId);
    }
    case Clusters::GeneralDiagnostics::Id: {
        return AttributeHasChangesOmittedQualityInGeneralDiagnosticsCluster(aAttributeId);
    }
    case Clusters::SoftwareDiagnostics::Id: {
        return AttributeHasChangesOmittedQualityInSoftwareDiagnosticsCluster(aAttributeId);
    }
    case Clusters::ThreadNetworkDiagnostics::Id: {
        return AttributeHasChangesOmittedQualityInThreadNetworkDiagnosticsCluster(aAttributeId);
    }
    case Clusters::WiFiNetworkDiagnostics::Id: {
        return AttributeHasChangesOmittedQualityInWiFiNetworkDiagnosticsCluster(aAttributeId);
    }
    case Clusters::EthernetNetworkDiagnostics::Id: {
        return AttributeHasChangesOmittedQualityInEthernetNetworkDiagnosticsCluster(aAttributeId);
    }
    case Clusters::TimeSynchronization::Id: {
        return AttributeHasChangesOmittedQualityInTimeSynchronizationCluster(aAttributeId);
    }
    case Clusters::BridgedDeviceBasicInformation::Id: {
        return AttributeHasChangesOmittedQualityInBridgedDeviceBasicInformationCluster(aAttributeId);
    }
    case Clusters::Switch::Id: {
        return AttributeHasChangesOmittedQualityInSwitchCluster(aAttributeId);
    }
    case Clusters::AdministratorCommissioning::Id: {
        return AttributeHasChangesOmittedQualityInAdministratorCommissioningCluster(aAttributeId);
    }
    case Clusters::OperationalCredentials::Id: {
        return AttributeHasChangesOmittedQualityInOperationalCredentialsCluster(aAttributeId);
    }
    case Clusters::GroupKeyManagement::Id: {
        return AttributeHasChangesOmittedQualityInGroupKeyManagementCluster(aAttributeId);
    }
    case Clusters::FixedLabel::Id: {
        return AttributeHasChangesOmittedQualityInFixedLabelCluster(aAttributeId);
    }
    case Clusters::UserLabel::Id: {
        return AttributeHasChangesOmittedQualityInUserLabelCluster(aAttributeId);
    }
    case Clusters::BooleanState::Id: {
        return AttributeHasChangesOmittedQualityInBooleanStateCluster(aAttributeId);
    }
    case Clusters::IcdManagement::Id: {
        return AttributeHasChangesOmittedQualityInICDManagementCluster(aAttributeId);
    }
    case Clusters::OvenCavityOperationalState::Id: {
        return AttributeHasChangesOmittedQualityInOvenCavityOperationalStateCluster(aAttributeId);
    }
    case Clusters::OvenMode::Id: {
        return AttributeHasChangesOmittedQualityInOvenModeCluster(aAttributeId);
    }
    case Clusters::LaundryDryerControls::Id: {
        return AttributeHasChangesOmittedQualityInLaundryDryerControlsCluster(aAttributeId);
    }
    case Clusters::TemperatureControlledCabinetTopology::Id: {
        return AttributeHasChangesOmittedQualityInTemperatureControlledCabinetTopologyCluster(aAttributeId);
    }
    case Clusters::ModeSelect::Id: {
        return AttributeHasChangesOmittedQualityInModeSelectCluster(aAttributeId);
    }
    case Clusters::LaundryWasherMode::Id: {
        return AttributeHasChangesOmittedQualityInLaundryWasherModeCluster(aAttributeId);
    }
    case Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Id: {
        return AttributeHasChangesOmittedQualityInRefrigeratorAndTemperatureControlledCabinetModeCluster(aAttributeId);
    }
    case Clusters::LaundryWasherControls::Id: {
        return AttributeHasChangesOmittedQualityInLaundryWasherControlsCluster(aAttributeId);
    }
    case Clusters::RvcRunMode::Id: {
        return AttributeHasChangesOmittedQualityInRVCRunModeCluster(aAttributeId);
    }
    case Clusters::RvcCleanMode::Id: {
        return AttributeHasChangesOmittedQualityInRVCCleanModeCluster(aAttributeId);
    }
    case Clusters::TemperatureControl::Id: {
        return AttributeHasChangesOmittedQualityInTemperatureControlCluster(aAttributeId);
    }
    case Clusters::RefrigeratorAlarm::Id: {
        return AttributeHasChangesOmittedQualityInRefrigeratorAlarmCluster(aAttributeId);
    }
    case Clusters::DishwasherMode::Id: {
        return AttributeHasChangesOmittedQualityInDishwasherModeCluster(aAttributeId);
    }
    case Clusters::AirQuality::Id: {
        return AttributeHasChangesOmittedQualityInAirQualityCluster(aAttributeId);
    }
    case Clusters::SmokeCoAlarm::Id: {
        return AttributeHasChangesOmittedQualityInSmokeCOAlarmCluster(aAttributeId);
    }
    case Clusters::DishwasherAlarm::Id: {
        return AttributeHasChangesOmittedQualityInDishwasherAlarmCluster(aAttributeId);
    }
    case Clusters::MicrowaveOvenMode::Id: {
        return AttributeHasChangesOmittedQualityInMicrowaveOvenModeCluster(aAttributeId);
    }
    case Clusters::MicrowaveOvenControl::Id: {
        return AttributeHasChangesOmittedQualityInMicrowaveOvenControlCluster(aAttributeId);
    }
    case Clusters::OperationalState::Id: {
        return AttributeHasChangesOmittedQualityInOperationalStateCluster(aAttributeId);
    }
    case Clusters::RvcOperationalState::Id: {
        return AttributeHasChangesOmittedQualityInRVCOperationalStateCluster(aAttributeId);
    }
    case Clusters::ScenesManagement::Id: {
        return AttributeHasChangesOmittedQualityInScenesManagementCluster(aAttributeId);
    }
    case Clusters::ThermostatMode::Id: {
        return AttributeHasChangesOmittedQualityInThermostatModeCluster(aAttributeId);
    }
    case Clusters::Groupcast::Id: {
        return AttributeHasChangesOmittedQualityInGroupcastCluster(aAttributeId);
    }
    case Clusters::HepaFilterMonitoring::Id: {
        return AttributeHasChangesOmittedQualityInHEPAFilterMonitoringCluster(aAttributeId);
    }
    case Clusters::ActivatedCarbonFilterMonitoring::Id: {
        return AttributeHasChangesOmittedQualityInActivatedCarbonFilterMonitoringCluster(aAttributeId);
    }
    case Clusters::WaterTankLevelMonitoring::Id: {
        return AttributeHasChangesOmittedQualityInWaterTankLevelMonitoringCluster(aAttributeId);
    }
    case Clusters::BooleanStateConfiguration::Id: {
        return AttributeHasChangesOmittedQualityInBooleanStateConfigurationCluster(aAttributeId);
    }
    case Clusters::ValveConfigurationAndControl::Id: {
        return AttributeHasChangesOmittedQualityInValveConfigurationAndControlCluster(aAttributeId);
    }
    case Clusters::ElectricalPowerMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInElectricalPowerMeasurementCluster(aAttributeId);
    }
    case Clusters::ElectricalEnergyMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInElectricalEnergyMeasurementCluster(aAttributeId);
    }
    case Clusters::WaterHeaterManagement::Id: {
        return AttributeHasChangesOmittedQualityInWaterHeaterManagementCluster(aAttributeId);
    }
    case Clusters::CommodityPrice::Id: {
        return AttributeHasChangesOmittedQualityInCommodityPriceCluster(aAttributeId);
    }
    case Clusters::Messages::Id: {
        return AttributeHasChangesOmittedQualityInMessagesCluster(aAttributeId);
    }
    case Clusters::DeviceEnergyManagement::Id: {
        return AttributeHasChangesOmittedQualityInDeviceEnergyManagementCluster(aAttributeId);
    }
    case Clusters::EnergyEvse::Id: {
        return AttributeHasChangesOmittedQualityInEnergyEVSECluster(aAttributeId);
    }
    case Clusters::EnergyPreference::Id: {
        return AttributeHasChangesOmittedQualityInEnergyPreferenceCluster(aAttributeId);
    }
    case Clusters::PowerTopology::Id: {
        return AttributeHasChangesOmittedQualityInPowerTopologyCluster(aAttributeId);
    }
    case Clusters::EnergyEvseMode::Id: {
        return AttributeHasChangesOmittedQualityInEnergyEVSEModeCluster(aAttributeId);
    }
    case Clusters::WaterHeaterMode::Id: {
        return AttributeHasChangesOmittedQualityInWaterHeaterModeCluster(aAttributeId);
    }
    case Clusters::DeviceEnergyManagementMode::Id: {
        return AttributeHasChangesOmittedQualityInDeviceEnergyManagementModeCluster(aAttributeId);
    }
    case Clusters::ElectricalGridConditions::Id: {
        return AttributeHasChangesOmittedQualityInElectricalGridConditionsCluster(aAttributeId);
    }
    case Clusters::ElectricalAlarm::Id: {
        return AttributeHasChangesOmittedQualityInElectricalAlarmCluster(aAttributeId);
    }
    case Clusters::ElectricalDistribution::Id: {
        return AttributeHasChangesOmittedQualityInElectricalDistributionCluster(aAttributeId);
    }
    case Clusters::ElectricalProtectionAlarm::Id: {
        return AttributeHasChangesOmittedQualityInElectricalProtectionAlarmCluster(aAttributeId);
    }
    case Clusters::DoorLock::Id: {
        return AttributeHasChangesOmittedQualityInDoorLockCluster(aAttributeId);
    }
    case Clusters::WindowCovering::Id: {
        return AttributeHasChangesOmittedQualityInWindowCoveringCluster(aAttributeId);
    }
    case Clusters::ClosureControl::Id: {
        return AttributeHasChangesOmittedQualityInClosureControlCluster(aAttributeId);
    }
    case Clusters::ClosureDimension::Id: {
        return AttributeHasChangesOmittedQualityInClosureDimensionCluster(aAttributeId);
    }
    case Clusters::ServiceArea::Id: {
        return AttributeHasChangesOmittedQualityInServiceAreaCluster(aAttributeId);
    }
    case Clusters::PumpConfigurationAndControl::Id: {
        return AttributeHasChangesOmittedQualityInPumpConfigurationAndControlCluster(aAttributeId);
    }
    case Clusters::Thermostat::Id: {
        return AttributeHasChangesOmittedQualityInThermostatCluster(aAttributeId);
    }
    case Clusters::FanControl::Id: {
        return AttributeHasChangesOmittedQualityInFanControlCluster(aAttributeId);
    }
    case Clusters::ThermostatUserInterfaceConfiguration::Id: {
        return AttributeHasChangesOmittedQualityInThermostatUserInterfaceConfigurationCluster(aAttributeId);
    }
    case Clusters::Humidistat::Id: {
        return AttributeHasChangesOmittedQualityInHumidistatCluster(aAttributeId);
    }
    case Clusters::ColorControl::Id: {
        return AttributeHasChangesOmittedQualityInColorControlCluster(aAttributeId);
    }
    case Clusters::BallastConfiguration::Id: {
        return AttributeHasChangesOmittedQualityInBallastConfigurationCluster(aAttributeId);
    }
    case Clusters::DynamicLighting::Id: {
        return AttributeHasChangesOmittedQualityInDynamicLightingCluster(aAttributeId);
    }
    case Clusters::IlluminanceMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInIlluminanceMeasurementCluster(aAttributeId);
    }
    case Clusters::TemperatureMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInTemperatureMeasurementCluster(aAttributeId);
    }
    case Clusters::PressureMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInPressureMeasurementCluster(aAttributeId);
    }
    case Clusters::FlowMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInFlowMeasurementCluster(aAttributeId);
    }
    case Clusters::RelativeHumidityMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInRelativeHumidityMeasurementCluster(aAttributeId);
    }
    case Clusters::OccupancySensing::Id: {
        return AttributeHasChangesOmittedQualityInOccupancySensingCluster(aAttributeId);
    }
    case Clusters::CarbonMonoxideConcentrationMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInCarbonMonoxideConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::CarbonDioxideConcentrationMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInCarbonDioxideConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::NitrogenDioxideConcentrationMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInNitrogenDioxideConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::OzoneConcentrationMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInOzoneConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::Pm25ConcentrationMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInPM25ConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::FormaldehydeConcentrationMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInFormaldehydeConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::Pm1ConcentrationMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInPM1ConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::Pm10ConcentrationMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInPM10ConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInTotalVolatileOrganicCompoundsConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::RadonConcentrationMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInRadonConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::SoilMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInSoilMeasurementCluster(aAttributeId);
    }
    case Clusters::AmbientContextSensing::Id: {
        return AttributeHasChangesOmittedQualityInAmbientContextSensingCluster(aAttributeId);
    }
    case Clusters::AmbientSensingUnion::Id: {
        return AttributeHasChangesOmittedQualityInAmbientSensingUnionCluster(aAttributeId);
    }
    case Clusters::ProximityRanging::Id: {
        return AttributeHasChangesOmittedQualityInProximityRangingCluster(aAttributeId);
    }
    case Clusters::SmokeConcentrationMeasurement::Id: {
        return AttributeHasChangesOmittedQualityInSmokeConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::NetworkIdentityManagement::Id: {
        return AttributeHasChangesOmittedQualityInNetworkIdentityManagementCluster(aAttributeId);
    }
    case Clusters::WiFiNetworkManagement::Id: {
        return AttributeHasChangesOmittedQualityInWiFiNetworkManagementCluster(aAttributeId);
    }
    case Clusters::ThreadBorderRouterManagement::Id: {
        return AttributeHasChangesOmittedQualityInThreadBorderRouterManagementCluster(aAttributeId);
    }
    case Clusters::ThreadNetworkDirectory::Id: {
        return AttributeHasChangesOmittedQualityInThreadNetworkDirectoryCluster(aAttributeId);
    }
    case Clusters::CommissioningProxy::Id: {
        return AttributeHasChangesOmittedQualityInCommissioningProxyCluster(aAttributeId);
    }
    case Clusters::WakeOnLan::Id: {
        return AttributeHasChangesOmittedQualityInWakeOnLANCluster(aAttributeId);
    }
    case Clusters::Channel::Id: {
        return AttributeHasChangesOmittedQualityInChannelCluster(aAttributeId);
    }
    case Clusters::TargetNavigator::Id: {
        return AttributeHasChangesOmittedQualityInTargetNavigatorCluster(aAttributeId);
    }
    case Clusters::MediaPlayback::Id: {
        return AttributeHasChangesOmittedQualityInMediaPlaybackCluster(aAttributeId);
    }
    case Clusters::MediaInput::Id: {
        return AttributeHasChangesOmittedQualityInMediaInputCluster(aAttributeId);
    }
    case Clusters::LowPower::Id: {
        return AttributeHasChangesOmittedQualityInLowPowerCluster(aAttributeId);
    }
    case Clusters::KeypadInput::Id: {
        return AttributeHasChangesOmittedQualityInKeypadInputCluster(aAttributeId);
    }
    case Clusters::ContentLauncher::Id: {
        return AttributeHasChangesOmittedQualityInContentLauncherCluster(aAttributeId);
    }
    case Clusters::AudioOutput::Id: {
        return AttributeHasChangesOmittedQualityInAudioOutputCluster(aAttributeId);
    }
    case Clusters::ApplicationLauncher::Id: {
        return AttributeHasChangesOmittedQualityInApplicationLauncherCluster(aAttributeId);
    }
    case Clusters::ApplicationBasic::Id: {
        return AttributeHasChangesOmittedQualityInApplicationBasicCluster(aAttributeId);
    }
    case Clusters::AccountLogin::Id: {
        return AttributeHasChangesOmittedQualityInAccountLoginCluster(aAttributeId);
    }
    case Clusters::ContentControl::Id: {
        return AttributeHasChangesOmittedQualityInContentControlCluster(aAttributeId);
    }
    case Clusters::ContentAppObserver::Id: {
        return AttributeHasChangesOmittedQualityInContentAppObserverCluster(aAttributeId);
    }
    case Clusters::MediaFileManagement::Id: {
        return AttributeHasChangesOmittedQualityInMediaFileManagementCluster(aAttributeId);
    }
    case Clusters::AudioControl::Id: {
        return AttributeHasChangesOmittedQualityInAudioControlCluster(aAttributeId);
    }
    case Clusters::ZoneManagement::Id: {
        return AttributeHasChangesOmittedQualityInZoneManagementCluster(aAttributeId);
    }
    case Clusters::CameraAvStreamManagement::Id: {
        return AttributeHasChangesOmittedQualityInCameraAVStreamManagementCluster(aAttributeId);
    }
    case Clusters::CameraAvSettingsUserLevelManagement::Id: {
        return AttributeHasChangesOmittedQualityInCameraAVSettingsUserLevelManagementCluster(aAttributeId);
    }
    case Clusters::WebRTCTransportProvider::Id: {
        return AttributeHasChangesOmittedQualityInWebRTCTransportProviderCluster(aAttributeId);
    }
    case Clusters::WebRTCTransportRequestor::Id: {
        return AttributeHasChangesOmittedQualityInWebRTCTransportRequestorCluster(aAttributeId);
    }
    case Clusters::PushAvStreamTransport::Id: {
        return AttributeHasChangesOmittedQualityInPushAVStreamTransportCluster(aAttributeId);
    }
    case Clusters::Chime::Id: {
        return AttributeHasChangesOmittedQualityInChimeCluster(aAttributeId);
    }
    case Clusters::AvAnalysis::Id: {
        return AttributeHasChangesOmittedQualityInAVAnalysisCluster(aAttributeId);
    }
    case Clusters::CommodityTariff::Id: {
        return AttributeHasChangesOmittedQualityInCommodityTariffCluster(aAttributeId);
    }
    case Clusters::EcosystemInformation::Id: {
        return AttributeHasChangesOmittedQualityInEcosystemInformationCluster(aAttributeId);
    }
    case Clusters::CommissionerControl::Id: {
        return AttributeHasChangesOmittedQualityInCommissionerControlCluster(aAttributeId);
    }
    case Clusters::JointFabricDatastore::Id: {
        return AttributeHasChangesOmittedQualityInJointFabricDatastoreCluster(aAttributeId);
    }
    case Clusters::JointFabricAdministrator::Id: {
        return AttributeHasChangesOmittedQualityInJointFabricAdministratorCluster(aAttributeId);
    }
    case Clusters::TlsCertificateManagement::Id: {
        return AttributeHasChangesOmittedQualityInTLSCertificateManagementCluster(aAttributeId);
    }
    case Clusters::TlsClientManagement::Id: {
        return AttributeHasChangesOmittedQualityInTLSClientManagementCluster(aAttributeId);
    }
    case Clusters::MeterIdentification::Id: {
        return AttributeHasChangesOmittedQualityInMeterIdentificationCluster(aAttributeId);
    }
    case Clusters::CommodityMetering::Id: {
        return AttributeHasChangesOmittedQualityInCommodityMeteringCluster(aAttributeId);
    }
    case Clusters::UnitTesting::Id: {
        return AttributeHasChangesOmittedQualityInUnitTestingCluster(aAttributeId);
    }
    case Clusters::SampleMei::Id: {
        return AttributeHasChangesOmittedQualityInSampleMEICluster(aAttributeId);
    }
    case Clusters::TestHiddenManufacturerSpecific::Id: {
        return AttributeHasChangesOmittedQualityInTestHiddenManufacturerSpecificCluster(aAttributeId);
    }
    default: {
        return NO;
    }
    }
}
