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

#import "MTRAttributeSpecifiedCheck.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

using namespace chip;
using namespace chip::app;

static BOOL AttributeIsSpecifiedInIdentifyCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Identify;
    switch (aAttributeId) {
    case Attributes::IdentifyTime::Id: {
        return YES;
    }
    case Attributes::IdentifyType::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInGroupsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Groups;
    switch (aAttributeId) {
    case Attributes::NameSupport::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInOnOffCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OnOff;
    switch (aAttributeId) {
    case Attributes::OnOff::Id: {
        return YES;
    }
    case Attributes::GlobalSceneControl::Id: {
        return YES;
    }
    case Attributes::OnTime::Id: {
        return YES;
    }
    case Attributes::OffWaitTime::Id: {
        return YES;
    }
    case Attributes::StartUpOnOff::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInOnOffSwitchConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OnOffSwitchConfiguration;
    switch (aAttributeId) {
    case Attributes::SwitchType::Id: {
        return YES;
    }
    case Attributes::SwitchActions::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInLevelControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LevelControl;
    switch (aAttributeId) {
    case Attributes::CurrentLevel::Id: {
        return YES;
    }
    case Attributes::RemainingTime::Id: {
        return YES;
    }
    case Attributes::MinLevel::Id: {
        return YES;
    }
    case Attributes::MaxLevel::Id: {
        return YES;
    }
    case Attributes::CurrentFrequency::Id: {
        return YES;
    }
    case Attributes::MinFrequency::Id: {
        return YES;
    }
    case Attributes::MaxFrequency::Id: {
        return YES;
    }
    case Attributes::Options::Id: {
        return YES;
    }
    case Attributes::OnOffTransitionTime::Id: {
        return YES;
    }
    case Attributes::OnLevel::Id: {
        return YES;
    }
    case Attributes::OnTransitionTime::Id: {
        return YES;
    }
    case Attributes::OffTransitionTime::Id: {
        return YES;
    }
    case Attributes::DefaultMoveRate::Id: {
        return YES;
    }
    case Attributes::StartUpCurrentLevel::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInBinaryInputBasicCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BinaryInputBasic;
    switch (aAttributeId) {
    case Attributes::ActiveText::Id: {
        return YES;
    }
    case Attributes::Description::Id: {
        return YES;
    }
    case Attributes::InactiveText::Id: {
        return YES;
    }
    case Attributes::OutOfService::Id: {
        return YES;
    }
    case Attributes::Polarity::Id: {
        return YES;
    }
    case Attributes::PresentValue::Id: {
        return YES;
    }
    case Attributes::Reliability::Id: {
        return YES;
    }
    case Attributes::StatusFlags::Id: {
        return YES;
    }
    case Attributes::ApplicationType::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInPulseWidthModulationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PulseWidthModulation;
    switch (aAttributeId) {
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInDescriptorCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Descriptor;
    switch (aAttributeId) {
    case Attributes::DeviceTypeList::Id: {
        return YES;
    }
    case Attributes::ServerList::Id: {
        return YES;
    }
    case Attributes::ClientList::Id: {
        return YES;
    }
    case Attributes::PartsList::Id: {
        return YES;
    }
    case Attributes::TagList::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInBindingCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Binding;
    switch (aAttributeId) {
    case Attributes::Binding::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInAccessControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AccessControl;
    switch (aAttributeId) {
    case Attributes::Acl::Id: {
        return YES;
    }
    case Attributes::Extension::Id: {
        return YES;
    }
    case Attributes::SubjectsPerAccessControlEntry::Id: {
        return YES;
    }
    case Attributes::TargetsPerAccessControlEntry::Id: {
        return YES;
    }
    case Attributes::AccessControlEntriesPerFabric::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInActionsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Actions;
    switch (aAttributeId) {
    case Attributes::ActionList::Id: {
        return YES;
    }
    case Attributes::EndpointLists::Id: {
        return YES;
    }
    case Attributes::SetupURL::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInBasicInformationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BasicInformation;
    switch (aAttributeId) {
    case Attributes::DataModelRevision::Id: {
        return YES;
    }
    case Attributes::VendorName::Id: {
        return YES;
    }
    case Attributes::VendorID::Id: {
        return YES;
    }
    case Attributes::ProductName::Id: {
        return YES;
    }
    case Attributes::ProductID::Id: {
        return YES;
    }
    case Attributes::NodeLabel::Id: {
        return YES;
    }
    case Attributes::Location::Id: {
        return YES;
    }
    case Attributes::HardwareVersion::Id: {
        return YES;
    }
    case Attributes::HardwareVersionString::Id: {
        return YES;
    }
    case Attributes::SoftwareVersion::Id: {
        return YES;
    }
    case Attributes::SoftwareVersionString::Id: {
        return YES;
    }
    case Attributes::ManufacturingDate::Id: {
        return YES;
    }
    case Attributes::PartNumber::Id: {
        return YES;
    }
    case Attributes::ProductURL::Id: {
        return YES;
    }
    case Attributes::ProductLabel::Id: {
        return YES;
    }
    case Attributes::SerialNumber::Id: {
        return YES;
    }
    case Attributes::LocalConfigDisabled::Id: {
        return YES;
    }
    case Attributes::Reachable::Id: {
        return YES;
    }
    case Attributes::UniqueID::Id: {
        return YES;
    }
    case Attributes::CapabilityMinima::Id: {
        return YES;
    }
    case Attributes::ProductAppearance::Id: {
        return YES;
    }
    case Attributes::SpecificationVersion::Id: {
        return YES;
    }
    case Attributes::MaxPathsPerInvoke::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInOTASoftwareUpdateProviderCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OtaSoftwareUpdateProvider;
    switch (aAttributeId) {
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInOTASoftwareUpdateRequestorCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OtaSoftwareUpdateRequestor;
    switch (aAttributeId) {
    case Attributes::DefaultOTAProviders::Id: {
        return YES;
    }
    case Attributes::UpdatePossible::Id: {
        return YES;
    }
    case Attributes::UpdateState::Id: {
        return YES;
    }
    case Attributes::UpdateStateProgress::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInLocalizationConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LocalizationConfiguration;
    switch (aAttributeId) {
    case Attributes::ActiveLocale::Id: {
        return YES;
    }
    case Attributes::SupportedLocales::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInTimeFormatLocalizationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TimeFormatLocalization;
    switch (aAttributeId) {
    case Attributes::HourFormat::Id: {
        return YES;
    }
    case Attributes::ActiveCalendarType::Id: {
        return YES;
    }
    case Attributes::SupportedCalendarTypes::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInUnitLocalizationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::UnitLocalization;
    switch (aAttributeId) {
    case Attributes::TemperatureUnit::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInPowerSourceConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PowerSourceConfiguration;
    switch (aAttributeId) {
    case Attributes::Sources::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInPowerSourceCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PowerSource;
    switch (aAttributeId) {
    case Attributes::Status::Id: {
        return YES;
    }
    case Attributes::Order::Id: {
        return YES;
    }
    case Attributes::Description::Id: {
        return YES;
    }
    case Attributes::WiredAssessedInputVoltage::Id: {
        return YES;
    }
    case Attributes::WiredAssessedInputFrequency::Id: {
        return YES;
    }
    case Attributes::WiredCurrentType::Id: {
        return YES;
    }
    case Attributes::WiredAssessedCurrent::Id: {
        return YES;
    }
    case Attributes::WiredNominalVoltage::Id: {
        return YES;
    }
    case Attributes::WiredMaximumCurrent::Id: {
        return YES;
    }
    case Attributes::WiredPresent::Id: {
        return YES;
    }
    case Attributes::ActiveWiredFaults::Id: {
        return YES;
    }
    case Attributes::BatVoltage::Id: {
        return YES;
    }
    case Attributes::BatPercentRemaining::Id: {
        return YES;
    }
    case Attributes::BatTimeRemaining::Id: {
        return YES;
    }
    case Attributes::BatChargeLevel::Id: {
        return YES;
    }
    case Attributes::BatReplacementNeeded::Id: {
        return YES;
    }
    case Attributes::BatReplaceability::Id: {
        return YES;
    }
    case Attributes::BatPresent::Id: {
        return YES;
    }
    case Attributes::ActiveBatFaults::Id: {
        return YES;
    }
    case Attributes::BatReplacementDescription::Id: {
        return YES;
    }
    case Attributes::BatCommonDesignation::Id: {
        return YES;
    }
    case Attributes::BatANSIDesignation::Id: {
        return YES;
    }
    case Attributes::BatIECDesignation::Id: {
        return YES;
    }
    case Attributes::BatApprovedChemistry::Id: {
        return YES;
    }
    case Attributes::BatCapacity::Id: {
        return YES;
    }
    case Attributes::BatQuantity::Id: {
        return YES;
    }
    case Attributes::BatChargeState::Id: {
        return YES;
    }
    case Attributes::BatTimeToFullCharge::Id: {
        return YES;
    }
    case Attributes::BatFunctionalWhileCharging::Id: {
        return YES;
    }
    case Attributes::BatChargingCurrent::Id: {
        return YES;
    }
    case Attributes::ActiveBatChargeFaults::Id: {
        return YES;
    }
    case Attributes::EndpointList::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInGeneralCommissioningCluster(AttributeId aAttributeId)
{
    using namespace Clusters::GeneralCommissioning;
    switch (aAttributeId) {
    case Attributes::Breadcrumb::Id: {
        return YES;
    }
    case Attributes::BasicCommissioningInfo::Id: {
        return YES;
    }
    case Attributes::RegulatoryConfig::Id: {
        return YES;
    }
    case Attributes::LocationCapability::Id: {
        return YES;
    }
    case Attributes::SupportsConcurrentConnection::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInNetworkCommissioningCluster(AttributeId aAttributeId)
{
    using namespace Clusters::NetworkCommissioning;
    switch (aAttributeId) {
    case Attributes::MaxNetworks::Id: {
        return YES;
    }
    case Attributes::Networks::Id: {
        return YES;
    }
    case Attributes::ScanMaxTimeSeconds::Id: {
        return YES;
    }
    case Attributes::ConnectMaxTimeSeconds::Id: {
        return YES;
    }
    case Attributes::InterfaceEnabled::Id: {
        return YES;
    }
    case Attributes::LastNetworkingStatus::Id: {
        return YES;
    }
    case Attributes::LastNetworkID::Id: {
        return YES;
    }
    case Attributes::LastConnectErrorValue::Id: {
        return YES;
    }
    case Attributes::SupportedWiFiBands::Id: {
        return YES;
    }
    case Attributes::SupportedThreadFeatures::Id: {
        return YES;
    }
    case Attributes::ThreadVersion::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInDiagnosticLogsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DiagnosticLogs;
    switch (aAttributeId) {
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInGeneralDiagnosticsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::GeneralDiagnostics;
    switch (aAttributeId) {
    case Attributes::NetworkInterfaces::Id: {
        return YES;
    }
    case Attributes::RebootCount::Id: {
        return YES;
    }
    case Attributes::UpTime::Id: {
        return YES;
    }
    case Attributes::TotalOperationalHours::Id: {
        return YES;
    }
    case Attributes::BootReason::Id: {
        return YES;
    }
    case Attributes::ActiveHardwareFaults::Id: {
        return YES;
    }
    case Attributes::ActiveRadioFaults::Id: {
        return YES;
    }
    case Attributes::ActiveNetworkFaults::Id: {
        return YES;
    }
    case Attributes::TestEventTriggersEnabled::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInSoftwareDiagnosticsCluster(AttributeId aAttributeId)
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
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInThreadNetworkDiagnosticsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ThreadNetworkDiagnostics;
    switch (aAttributeId) {
    case Attributes::Channel::Id: {
        return YES;
    }
    case Attributes::RoutingRole::Id: {
        return YES;
    }
    case Attributes::NetworkName::Id: {
        return YES;
    }
    case Attributes::PanId::Id: {
        return YES;
    }
    case Attributes::ExtendedPanId::Id: {
        return YES;
    }
    case Attributes::MeshLocalPrefix::Id: {
        return YES;
    }
    case Attributes::OverrunCount::Id: {
        return YES;
    }
    case Attributes::NeighborTable::Id: {
        return YES;
    }
    case Attributes::RouteTable::Id: {
        return YES;
    }
    case Attributes::PartitionId::Id: {
        return YES;
    }
    case Attributes::Weighting::Id: {
        return YES;
    }
    case Attributes::DataVersion::Id: {
        return YES;
    }
    case Attributes::StableDataVersion::Id: {
        return YES;
    }
    case Attributes::LeaderRouterId::Id: {
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
    case Attributes::ActiveTimestamp::Id: {
        return YES;
    }
    case Attributes::PendingTimestamp::Id: {
        return YES;
    }
    case Attributes::Delay::Id: {
        return YES;
    }
    case Attributes::SecurityPolicy::Id: {
        return YES;
    }
    case Attributes::ChannelPage0Mask::Id: {
        return YES;
    }
    case Attributes::OperationalDatasetComponents::Id: {
        return YES;
    }
    case Attributes::ActiveNetworkFaultsList::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInWiFiNetworkDiagnosticsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WiFiNetworkDiagnostics;
    switch (aAttributeId) {
    case Attributes::Bssid::Id: {
        return YES;
    }
    case Attributes::SecurityType::Id: {
        return YES;
    }
    case Attributes::WiFiVersion::Id: {
        return YES;
    }
    case Attributes::ChannelNumber::Id: {
        return YES;
    }
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
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInEthernetNetworkDiagnosticsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::EthernetNetworkDiagnostics;
    switch (aAttributeId) {
    case Attributes::PHYRate::Id: {
        return YES;
    }
    case Attributes::FullDuplex::Id: {
        return YES;
    }
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
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInTimeSynchronizationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TimeSynchronization;
    switch (aAttributeId) {
    case Attributes::UTCTime::Id: {
        return YES;
    }
    case Attributes::Granularity::Id: {
        return YES;
    }
    case Attributes::TimeSource::Id: {
        return YES;
    }
    case Attributes::TrustedTimeSource::Id: {
        return YES;
    }
    case Attributes::DefaultNTP::Id: {
        return YES;
    }
    case Attributes::TimeZone::Id: {
        return YES;
    }
    case Attributes::DSTOffset::Id: {
        return YES;
    }
    case Attributes::LocalTime::Id: {
        return YES;
    }
    case Attributes::TimeZoneDatabase::Id: {
        return YES;
    }
    case Attributes::NTPServerAvailable::Id: {
        return YES;
    }
    case Attributes::TimeZoneListMaxSize::Id: {
        return YES;
    }
    case Attributes::DSTOffsetListMaxSize::Id: {
        return YES;
    }
    case Attributes::SupportsDNSResolve::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInBridgedDeviceBasicInformationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BridgedDeviceBasicInformation;
    switch (aAttributeId) {
    case Attributes::VendorName::Id: {
        return YES;
    }
    case Attributes::VendorID::Id: {
        return YES;
    }
    case Attributes::ProductName::Id: {
        return YES;
    }
    case Attributes::NodeLabel::Id: {
        return YES;
    }
    case Attributes::HardwareVersion::Id: {
        return YES;
    }
    case Attributes::HardwareVersionString::Id: {
        return YES;
    }
    case Attributes::SoftwareVersion::Id: {
        return YES;
    }
    case Attributes::SoftwareVersionString::Id: {
        return YES;
    }
    case Attributes::ManufacturingDate::Id: {
        return YES;
    }
    case Attributes::PartNumber::Id: {
        return YES;
    }
    case Attributes::ProductURL::Id: {
        return YES;
    }
    case Attributes::ProductLabel::Id: {
        return YES;
    }
    case Attributes::SerialNumber::Id: {
        return YES;
    }
    case Attributes::Reachable::Id: {
        return YES;
    }
    case Attributes::UniqueID::Id: {
        return YES;
    }
    case Attributes::ProductAppearance::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInSwitchCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Switch;
    switch (aAttributeId) {
    case Attributes::NumberOfPositions::Id: {
        return YES;
    }
    case Attributes::CurrentPosition::Id: {
        return YES;
    }
    case Attributes::MultiPressMax::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInAdministratorCommissioningCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AdministratorCommissioning;
    switch (aAttributeId) {
    case Attributes::WindowStatus::Id: {
        return YES;
    }
    case Attributes::AdminFabricIndex::Id: {
        return YES;
    }
    case Attributes::AdminVendorId::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInOperationalCredentialsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OperationalCredentials;
    switch (aAttributeId) {
    case Attributes::NOCs::Id: {
        return YES;
    }
    case Attributes::Fabrics::Id: {
        return YES;
    }
    case Attributes::SupportedFabrics::Id: {
        return YES;
    }
    case Attributes::CommissionedFabrics::Id: {
        return YES;
    }
    case Attributes::TrustedRootCertificates::Id: {
        return YES;
    }
    case Attributes::CurrentFabricIndex::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInGroupKeyManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::GroupKeyManagement;
    switch (aAttributeId) {
    case Attributes::GroupKeyMap::Id: {
        return YES;
    }
    case Attributes::GroupTable::Id: {
        return YES;
    }
    case Attributes::MaxGroupsPerFabric::Id: {
        return YES;
    }
    case Attributes::MaxGroupKeysPerFabric::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInFixedLabelCluster(AttributeId aAttributeId)
{
    using namespace Clusters::FixedLabel;
    switch (aAttributeId) {
    case Attributes::LabelList::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInUserLabelCluster(AttributeId aAttributeId)
{
    using namespace Clusters::UserLabel;
    switch (aAttributeId) {
    case Attributes::LabelList::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInBooleanStateCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BooleanState;
    switch (aAttributeId) {
    case Attributes::StateValue::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInICDManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::IcdManagement;
    switch (aAttributeId) {
    case Attributes::IdleModeDuration::Id: {
        return YES;
    }
    case Attributes::ActiveModeDuration::Id: {
        return YES;
    }
    case Attributes::ActiveModeThreshold::Id: {
        return YES;
    }
    case Attributes::RegisteredClients::Id: {
        return YES;
    }
    case Attributes::ICDCounter::Id: {
        return YES;
    }
    case Attributes::ClientsSupportedPerFabric::Id: {
        return YES;
    }
    case Attributes::UserActiveModeTriggerHint::Id: {
        return YES;
    }
    case Attributes::UserActiveModeTriggerInstruction::Id: {
        return YES;
    }
    case Attributes::OperatingMode::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInTimerCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Timer;
    switch (aAttributeId) {
    case Attributes::SetTime::Id: {
        return YES;
    }
    case Attributes::TimeRemaining::Id: {
        return YES;
    }
    case Attributes::TimerState::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInOvenCavityOperationalStateCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OvenCavityOperationalState;
    switch (aAttributeId) {
    case Attributes::PhaseList::Id: {
        return YES;
    }
    case Attributes::CurrentPhase::Id: {
        return YES;
    }
    case Attributes::CountdownTime::Id: {
        return YES;
    }
    case Attributes::OperationalStateList::Id: {
        return YES;
    }
    case Attributes::OperationalState::Id: {
        return YES;
    }
    case Attributes::OperationalError::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInOvenModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OvenMode;
    switch (aAttributeId) {
    case Attributes::SupportedModes::Id: {
        return YES;
    }
    case Attributes::CurrentMode::Id: {
        return YES;
    }
    case Attributes::StartUpMode::Id: {
        return YES;
    }
    case Attributes::OnMode::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInLaundryDryerControlsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LaundryDryerControls;
    switch (aAttributeId) {
    case Attributes::SupportedDrynessLevels::Id: {
        return YES;
    }
    case Attributes::SelectedDrynessLevel::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInModeSelectCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ModeSelect;
    switch (aAttributeId) {
    case Attributes::Description::Id: {
        return YES;
    }
    case Attributes::StandardNamespace::Id: {
        return YES;
    }
    case Attributes::SupportedModes::Id: {
        return YES;
    }
    case Attributes::CurrentMode::Id: {
        return YES;
    }
    case Attributes::StartUpMode::Id: {
        return YES;
    }
    case Attributes::OnMode::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInLaundryWasherModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LaundryWasherMode;
    switch (aAttributeId) {
    case Attributes::SupportedModes::Id: {
        return YES;
    }
    case Attributes::CurrentMode::Id: {
        return YES;
    }
    case Attributes::StartUpMode::Id: {
        return YES;
    }
    case Attributes::OnMode::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInRefrigeratorAndTemperatureControlledCabinetModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RefrigeratorAndTemperatureControlledCabinetMode;
    switch (aAttributeId) {
    case Attributes::SupportedModes::Id: {
        return YES;
    }
    case Attributes::CurrentMode::Id: {
        return YES;
    }
    case Attributes::StartUpMode::Id: {
        return YES;
    }
    case Attributes::OnMode::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInLaundryWasherControlsCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LaundryWasherControls;
    switch (aAttributeId) {
    case Attributes::SpinSpeeds::Id: {
        return YES;
    }
    case Attributes::SpinSpeedCurrent::Id: {
        return YES;
    }
    case Attributes::NumberOfRinses::Id: {
        return YES;
    }
    case Attributes::SupportedRinses::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInRVCRunModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RvcRunMode;
    switch (aAttributeId) {
    case Attributes::SupportedModes::Id: {
        return YES;
    }
    case Attributes::CurrentMode::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInRVCCleanModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RvcCleanMode;
    switch (aAttributeId) {
    case Attributes::SupportedModes::Id: {
        return YES;
    }
    case Attributes::CurrentMode::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInTemperatureControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TemperatureControl;
    switch (aAttributeId) {
    case Attributes::TemperatureSetpoint::Id: {
        return YES;
    }
    case Attributes::MinTemperature::Id: {
        return YES;
    }
    case Attributes::MaxTemperature::Id: {
        return YES;
    }
    case Attributes::Step::Id: {
        return YES;
    }
    case Attributes::SelectedTemperatureLevel::Id: {
        return YES;
    }
    case Attributes::SupportedTemperatureLevels::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInRefrigeratorAlarmCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RefrigeratorAlarm;
    switch (aAttributeId) {
    case Attributes::Mask::Id: {
        return YES;
    }
    case Attributes::State::Id: {
        return YES;
    }
    case Attributes::Supported::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInDishwasherModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DishwasherMode;
    switch (aAttributeId) {
    case Attributes::SupportedModes::Id: {
        return YES;
    }
    case Attributes::CurrentMode::Id: {
        return YES;
    }
    case Attributes::StartUpMode::Id: {
        return YES;
    }
    case Attributes::OnMode::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInAirQualityCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AirQuality;
    switch (aAttributeId) {
    case Attributes::AirQuality::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInSmokeCOAlarmCluster(AttributeId aAttributeId)
{
    using namespace Clusters::SmokeCoAlarm;
    switch (aAttributeId) {
    case Attributes::ExpressedState::Id: {
        return YES;
    }
    case Attributes::SmokeState::Id: {
        return YES;
    }
    case Attributes::COState::Id: {
        return YES;
    }
    case Attributes::BatteryAlert::Id: {
        return YES;
    }
    case Attributes::DeviceMuted::Id: {
        return YES;
    }
    case Attributes::TestInProgress::Id: {
        return YES;
    }
    case Attributes::HardwareFaultAlert::Id: {
        return YES;
    }
    case Attributes::EndOfServiceAlert::Id: {
        return YES;
    }
    case Attributes::InterconnectSmokeAlarm::Id: {
        return YES;
    }
    case Attributes::InterconnectCOAlarm::Id: {
        return YES;
    }
    case Attributes::ContaminationState::Id: {
        return YES;
    }
    case Attributes::SmokeSensitivityLevel::Id: {
        return YES;
    }
    case Attributes::ExpiryDate::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInDishwasherAlarmCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DishwasherAlarm;
    switch (aAttributeId) {
    case Attributes::Mask::Id: {
        return YES;
    }
    case Attributes::Latch::Id: {
        return YES;
    }
    case Attributes::State::Id: {
        return YES;
    }
    case Attributes::Supported::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInMicrowaveOvenModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::MicrowaveOvenMode;
    switch (aAttributeId) {
    case Attributes::SupportedModes::Id: {
        return YES;
    }
    case Attributes::CurrentMode::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInMicrowaveOvenControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::MicrowaveOvenControl;
    switch (aAttributeId) {
    case Attributes::CookTime::Id: {
        return YES;
    }
    case Attributes::MaxCookTime::Id: {
        return YES;
    }
    case Attributes::PowerSetting::Id: {
        return YES;
    }
    case Attributes::MinPower::Id: {
        return YES;
    }
    case Attributes::MaxPower::Id: {
        return YES;
    }
    case Attributes::PowerStep::Id: {
        return YES;
    }
    case Attributes::SupportedWatts::Id: {
        return YES;
    }
    case Attributes::SelectedWattIndex::Id: {
        return YES;
    }
    case Attributes::WattRating::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInOperationalStateCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OperationalState;
    switch (aAttributeId) {
    case Attributes::PhaseList::Id: {
        return YES;
    }
    case Attributes::CurrentPhase::Id: {
        return YES;
    }
    case Attributes::CountdownTime::Id: {
        return YES;
    }
    case Attributes::OperationalStateList::Id: {
        return YES;
    }
    case Attributes::OperationalState::Id: {
        return YES;
    }
    case Attributes::OperationalError::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInRVCOperationalStateCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RvcOperationalState;
    switch (aAttributeId) {
    case Attributes::PhaseList::Id: {
        return YES;
    }
    case Attributes::CurrentPhase::Id: {
        return YES;
    }
    case Attributes::CountdownTime::Id: {
        return YES;
    }
    case Attributes::OperationalStateList::Id: {
        return YES;
    }
    case Attributes::OperationalState::Id: {
        return YES;
    }
    case Attributes::OperationalError::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInScenesManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ScenesManagement;
    switch (aAttributeId) {
    case Attributes::LastConfiguredBy::Id: {
        return YES;
    }
    case Attributes::SceneTableSize::Id: {
        return YES;
    }
    case Attributes::FabricSceneInfo::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInHEPAFilterMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::HepaFilterMonitoring;
    switch (aAttributeId) {
    case Attributes::Condition::Id: {
        return YES;
    }
    case Attributes::DegradationDirection::Id: {
        return YES;
    }
    case Attributes::ChangeIndication::Id: {
        return YES;
    }
    case Attributes::InPlaceIndicator::Id: {
        return YES;
    }
    case Attributes::LastChangedTime::Id: {
        return YES;
    }
    case Attributes::ReplacementProductList::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInActivatedCarbonFilterMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ActivatedCarbonFilterMonitoring;
    switch (aAttributeId) {
    case Attributes::Condition::Id: {
        return YES;
    }
    case Attributes::DegradationDirection::Id: {
        return YES;
    }
    case Attributes::ChangeIndication::Id: {
        return YES;
    }
    case Attributes::InPlaceIndicator::Id: {
        return YES;
    }
    case Attributes::LastChangedTime::Id: {
        return YES;
    }
    case Attributes::ReplacementProductList::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInBooleanStateConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BooleanStateConfiguration;
    switch (aAttributeId) {
    case Attributes::CurrentSensitivityLevel::Id: {
        return YES;
    }
    case Attributes::SupportedSensitivityLevels::Id: {
        return YES;
    }
    case Attributes::DefaultSensitivityLevel::Id: {
        return YES;
    }
    case Attributes::AlarmsActive::Id: {
        return YES;
    }
    case Attributes::AlarmsSuppressed::Id: {
        return YES;
    }
    case Attributes::AlarmsEnabled::Id: {
        return YES;
    }
    case Attributes::AlarmsSupported::Id: {
        return YES;
    }
    case Attributes::SensorFault::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInValveConfigurationAndControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ValveConfigurationAndControl;
    switch (aAttributeId) {
    case Attributes::OpenDuration::Id: {
        return YES;
    }
    case Attributes::DefaultOpenDuration::Id: {
        return YES;
    }
    case Attributes::AutoCloseTime::Id: {
        return YES;
    }
    case Attributes::RemainingDuration::Id: {
        return YES;
    }
    case Attributes::CurrentState::Id: {
        return YES;
    }
    case Attributes::TargetState::Id: {
        return YES;
    }
    case Attributes::CurrentLevel::Id: {
        return YES;
    }
    case Attributes::TargetLevel::Id: {
        return YES;
    }
    case Attributes::DefaultOpenLevel::Id: {
        return YES;
    }
    case Attributes::ValveFault::Id: {
        return YES;
    }
    case Attributes::LevelStep::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInElectricalPowerMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ElectricalPowerMeasurement;
    switch (aAttributeId) {
    case Attributes::PowerMode::Id: {
        return YES;
    }
    case Attributes::NumberOfMeasurementTypes::Id: {
        return YES;
    }
    case Attributes::Accuracy::Id: {
        return YES;
    }
    case Attributes::Ranges::Id: {
        return YES;
    }
    case Attributes::Voltage::Id: {
        return YES;
    }
    case Attributes::ActiveCurrent::Id: {
        return YES;
    }
    case Attributes::ReactiveCurrent::Id: {
        return YES;
    }
    case Attributes::ApparentCurrent::Id: {
        return YES;
    }
    case Attributes::ActivePower::Id: {
        return YES;
    }
    case Attributes::ReactivePower::Id: {
        return YES;
    }
    case Attributes::ApparentPower::Id: {
        return YES;
    }
    case Attributes::RMSVoltage::Id: {
        return YES;
    }
    case Attributes::RMSCurrent::Id: {
        return YES;
    }
    case Attributes::RMSPower::Id: {
        return YES;
    }
    case Attributes::Frequency::Id: {
        return YES;
    }
    case Attributes::HarmonicCurrents::Id: {
        return YES;
    }
    case Attributes::HarmonicPhases::Id: {
        return YES;
    }
    case Attributes::PowerFactor::Id: {
        return YES;
    }
    case Attributes::NeutralCurrent::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInElectricalEnergyMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ElectricalEnergyMeasurement;
    switch (aAttributeId) {
    case Attributes::Accuracy::Id: {
        return YES;
    }
    case Attributes::CumulativeEnergyImported::Id: {
        return YES;
    }
    case Attributes::CumulativeEnergyExported::Id: {
        return YES;
    }
    case Attributes::PeriodicEnergyImported::Id: {
        return YES;
    }
    case Attributes::PeriodicEnergyExported::Id: {
        return YES;
    }
    case Attributes::CumulativeEnergyReset::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInDemandResponseLoadControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DemandResponseLoadControl;
    switch (aAttributeId) {
    case Attributes::LoadControlPrograms::Id: {
        return YES;
    }
    case Attributes::NumberOfLoadControlPrograms::Id: {
        return YES;
    }
    case Attributes::Events::Id: {
        return YES;
    }
    case Attributes::ActiveEvents::Id: {
        return YES;
    }
    case Attributes::NumberOfEventsPerProgram::Id: {
        return YES;
    }
    case Attributes::NumberOfTransitions::Id: {
        return YES;
    }
    case Attributes::DefaultRandomStart::Id: {
        return YES;
    }
    case Attributes::DefaultRandomDuration::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInMessagesCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Messages;
    switch (aAttributeId) {
    case Attributes::Messages::Id: {
        return YES;
    }
    case Attributes::ActiveMessageIDs::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInDeviceEnergyManagementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DeviceEnergyManagement;
    switch (aAttributeId) {
    case Attributes::ESAType::Id: {
        return YES;
    }
    case Attributes::ESACanGenerate::Id: {
        return YES;
    }
    case Attributes::ESAState::Id: {
        return YES;
    }
    case Attributes::AbsMinPower::Id: {
        return YES;
    }
    case Attributes::AbsMaxPower::Id: {
        return YES;
    }
    case Attributes::PowerAdjustmentCapability::Id: {
        return YES;
    }
    case Attributes::Forecast::Id: {
        return YES;
    }
    case Attributes::OptOutState::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInEnergyEVSECluster(AttributeId aAttributeId)
{
    using namespace Clusters::EnergyEvse;
    switch (aAttributeId) {
    case Attributes::State::Id: {
        return YES;
    }
    case Attributes::SupplyState::Id: {
        return YES;
    }
    case Attributes::FaultState::Id: {
        return YES;
    }
    case Attributes::ChargingEnabledUntil::Id: {
        return YES;
    }
    case Attributes::DischargingEnabledUntil::Id: {
        return YES;
    }
    case Attributes::CircuitCapacity::Id: {
        return YES;
    }
    case Attributes::MinimumChargeCurrent::Id: {
        return YES;
    }
    case Attributes::MaximumChargeCurrent::Id: {
        return YES;
    }
    case Attributes::MaximumDischargeCurrent::Id: {
        return YES;
    }
    case Attributes::UserMaximumChargeCurrent::Id: {
        return YES;
    }
    case Attributes::RandomizationDelayWindow::Id: {
        return YES;
    }
    case Attributes::NextChargeStartTime::Id: {
        return YES;
    }
    case Attributes::NextChargeTargetTime::Id: {
        return YES;
    }
    case Attributes::NextChargeRequiredEnergy::Id: {
        return YES;
    }
    case Attributes::NextChargeTargetSoC::Id: {
        return YES;
    }
    case Attributes::ApproximateEVEfficiency::Id: {
        return YES;
    }
    case Attributes::StateOfCharge::Id: {
        return YES;
    }
    case Attributes::BatteryCapacity::Id: {
        return YES;
    }
    case Attributes::VehicleID::Id: {
        return YES;
    }
    case Attributes::SessionID::Id: {
        return YES;
    }
    case Attributes::SessionDuration::Id: {
        return YES;
    }
    case Attributes::SessionEnergyCharged::Id: {
        return YES;
    }
    case Attributes::SessionEnergyDischarged::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInEnergyPreferenceCluster(AttributeId aAttributeId)
{
    using namespace Clusters::EnergyPreference;
    switch (aAttributeId) {
    case Attributes::EnergyBalances::Id: {
        return YES;
    }
    case Attributes::CurrentEnergyBalance::Id: {
        return YES;
    }
    case Attributes::EnergyPriorities::Id: {
        return YES;
    }
    case Attributes::LowPowerModeSensitivities::Id: {
        return YES;
    }
    case Attributes::CurrentLowPowerModeSensitivity::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInPowerTopologyCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PowerTopology;
    switch (aAttributeId) {
    case Attributes::AvailableEndpoints::Id: {
        return YES;
    }
    case Attributes::ActiveEndpoints::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInEnergyEVSEModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::EnergyEvseMode;
    switch (aAttributeId) {
    case Attributes::SupportedModes::Id: {
        return YES;
    }
    case Attributes::CurrentMode::Id: {
        return YES;
    }
    case Attributes::StartUpMode::Id: {
        return YES;
    }
    case Attributes::OnMode::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInDeviceEnergyManagementModeCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DeviceEnergyManagementMode;
    switch (aAttributeId) {
    case Attributes::SupportedModes::Id: {
        return YES;
    }
    case Attributes::CurrentMode::Id: {
        return YES;
    }
    case Attributes::StartUpMode::Id: {
        return YES;
    }
    case Attributes::OnMode::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInDoorLockCluster(AttributeId aAttributeId)
{
    using namespace Clusters::DoorLock;
    switch (aAttributeId) {
    case Attributes::LockState::Id: {
        return YES;
    }
    case Attributes::LockType::Id: {
        return YES;
    }
    case Attributes::ActuatorEnabled::Id: {
        return YES;
    }
    case Attributes::DoorState::Id: {
        return YES;
    }
    case Attributes::DoorOpenEvents::Id: {
        return YES;
    }
    case Attributes::DoorClosedEvents::Id: {
        return YES;
    }
    case Attributes::OpenPeriod::Id: {
        return YES;
    }
    case Attributes::NumberOfTotalUsersSupported::Id: {
        return YES;
    }
    case Attributes::NumberOfPINUsersSupported::Id: {
        return YES;
    }
    case Attributes::NumberOfRFIDUsersSupported::Id: {
        return YES;
    }
    case Attributes::NumberOfWeekDaySchedulesSupportedPerUser::Id: {
        return YES;
    }
    case Attributes::NumberOfYearDaySchedulesSupportedPerUser::Id: {
        return YES;
    }
    case Attributes::NumberOfHolidaySchedulesSupported::Id: {
        return YES;
    }
    case Attributes::MaxPINCodeLength::Id: {
        return YES;
    }
    case Attributes::MinPINCodeLength::Id: {
        return YES;
    }
    case Attributes::MaxRFIDCodeLength::Id: {
        return YES;
    }
    case Attributes::MinRFIDCodeLength::Id: {
        return YES;
    }
    case Attributes::CredentialRulesSupport::Id: {
        return YES;
    }
    case Attributes::NumberOfCredentialsSupportedPerUser::Id: {
        return YES;
    }
    case Attributes::Language::Id: {
        return YES;
    }
    case Attributes::LEDSettings::Id: {
        return YES;
    }
    case Attributes::AutoRelockTime::Id: {
        return YES;
    }
    case Attributes::SoundVolume::Id: {
        return YES;
    }
    case Attributes::OperatingMode::Id: {
        return YES;
    }
    case Attributes::SupportedOperatingModes::Id: {
        return YES;
    }
    case Attributes::DefaultConfigurationRegister::Id: {
        return YES;
    }
    case Attributes::EnableLocalProgramming::Id: {
        return YES;
    }
    case Attributes::EnableOneTouchLocking::Id: {
        return YES;
    }
    case Attributes::EnableInsideStatusLED::Id: {
        return YES;
    }
    case Attributes::EnablePrivacyModeButton::Id: {
        return YES;
    }
    case Attributes::LocalProgrammingFeatures::Id: {
        return YES;
    }
    case Attributes::WrongCodeEntryLimit::Id: {
        return YES;
    }
    case Attributes::UserCodeTemporaryDisableTime::Id: {
        return YES;
    }
    case Attributes::SendPINOverTheAir::Id: {
        return YES;
    }
    case Attributes::RequirePINforRemoteOperation::Id: {
        return YES;
    }
    case Attributes::ExpiringUserTimeout::Id: {
        return YES;
    }
    case Attributes::AliroReaderVerificationKey::Id: {
        return YES;
    }
    case Attributes::AliroReaderGroupIdentifier::Id: {
        return YES;
    }
    case Attributes::AliroReaderGroupSubIdentifier::Id: {
        return YES;
    }
    case Attributes::AliroExpeditedTransactionSupportedProtocolVersions::Id: {
        return YES;
    }
    case Attributes::AliroGroupResolvingKey::Id: {
        return YES;
    }
    case Attributes::AliroSupportedBLEUWBProtocolVersions::Id: {
        return YES;
    }
    case Attributes::AliroBLEAdvertisingVersion::Id: {
        return YES;
    }
    case Attributes::NumberOfAliroCredentialIssuerKeysSupported::Id: {
        return YES;
    }
    case Attributes::NumberOfAliroEndpointKeysSupported::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInWindowCoveringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WindowCovering;
    switch (aAttributeId) {
    case Attributes::Type::Id: {
        return YES;
    }
    case Attributes::PhysicalClosedLimitLift::Id: {
        return YES;
    }
    case Attributes::PhysicalClosedLimitTilt::Id: {
        return YES;
    }
    case Attributes::CurrentPositionLift::Id: {
        return YES;
    }
    case Attributes::CurrentPositionTilt::Id: {
        return YES;
    }
    case Attributes::NumberOfActuationsLift::Id: {
        return YES;
    }
    case Attributes::NumberOfActuationsTilt::Id: {
        return YES;
    }
    case Attributes::ConfigStatus::Id: {
        return YES;
    }
    case Attributes::CurrentPositionLiftPercentage::Id: {
        return YES;
    }
    case Attributes::CurrentPositionTiltPercentage::Id: {
        return YES;
    }
    case Attributes::OperationalStatus::Id: {
        return YES;
    }
    case Attributes::TargetPositionLiftPercent100ths::Id: {
        return YES;
    }
    case Attributes::TargetPositionTiltPercent100ths::Id: {
        return YES;
    }
    case Attributes::EndProductType::Id: {
        return YES;
    }
    case Attributes::CurrentPositionLiftPercent100ths::Id: {
        return YES;
    }
    case Attributes::CurrentPositionTiltPercent100ths::Id: {
        return YES;
    }
    case Attributes::InstalledOpenLimitLift::Id: {
        return YES;
    }
    case Attributes::InstalledClosedLimitLift::Id: {
        return YES;
    }
    case Attributes::InstalledOpenLimitTilt::Id: {
        return YES;
    }
    case Attributes::InstalledClosedLimitTilt::Id: {
        return YES;
    }
    case Attributes::Mode::Id: {
        return YES;
    }
    case Attributes::SafetyStatus::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInBarrierControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BarrierControl;
    switch (aAttributeId) {
    case Attributes::BarrierMovingState::Id: {
        return YES;
    }
    case Attributes::BarrierSafetyStatus::Id: {
        return YES;
    }
    case Attributes::BarrierCapabilities::Id: {
        return YES;
    }
    case Attributes::BarrierOpenEvents::Id: {
        return YES;
    }
    case Attributes::BarrierCloseEvents::Id: {
        return YES;
    }
    case Attributes::BarrierCommandOpenEvents::Id: {
        return YES;
    }
    case Attributes::BarrierCommandCloseEvents::Id: {
        return YES;
    }
    case Attributes::BarrierOpenPeriod::Id: {
        return YES;
    }
    case Attributes::BarrierClosePeriod::Id: {
        return YES;
    }
    case Attributes::BarrierPosition::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInPumpConfigurationAndControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PumpConfigurationAndControl;
    switch (aAttributeId) {
    case Attributes::MaxPressure::Id: {
        return YES;
    }
    case Attributes::MaxSpeed::Id: {
        return YES;
    }
    case Attributes::MaxFlow::Id: {
        return YES;
    }
    case Attributes::MinConstPressure::Id: {
        return YES;
    }
    case Attributes::MaxConstPressure::Id: {
        return YES;
    }
    case Attributes::MinCompPressure::Id: {
        return YES;
    }
    case Attributes::MaxCompPressure::Id: {
        return YES;
    }
    case Attributes::MinConstSpeed::Id: {
        return YES;
    }
    case Attributes::MaxConstSpeed::Id: {
        return YES;
    }
    case Attributes::MinConstFlow::Id: {
        return YES;
    }
    case Attributes::MaxConstFlow::Id: {
        return YES;
    }
    case Attributes::MinConstTemp::Id: {
        return YES;
    }
    case Attributes::MaxConstTemp::Id: {
        return YES;
    }
    case Attributes::PumpStatus::Id: {
        return YES;
    }
    case Attributes::EffectiveOperationMode::Id: {
        return YES;
    }
    case Attributes::EffectiveControlMode::Id: {
        return YES;
    }
    case Attributes::Capacity::Id: {
        return YES;
    }
    case Attributes::Speed::Id: {
        return YES;
    }
    case Attributes::LifetimeRunningHours::Id: {
        return YES;
    }
    case Attributes::Power::Id: {
        return YES;
    }
    case Attributes::LifetimeEnergyConsumed::Id: {
        return YES;
    }
    case Attributes::OperationMode::Id: {
        return YES;
    }
    case Attributes::ControlMode::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInThermostatCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Thermostat;
    switch (aAttributeId) {
    case Attributes::LocalTemperature::Id: {
        return YES;
    }
    case Attributes::OutdoorTemperature::Id: {
        return YES;
    }
    case Attributes::Occupancy::Id: {
        return YES;
    }
    case Attributes::AbsMinHeatSetpointLimit::Id: {
        return YES;
    }
    case Attributes::AbsMaxHeatSetpointLimit::Id: {
        return YES;
    }
    case Attributes::AbsMinCoolSetpointLimit::Id: {
        return YES;
    }
    case Attributes::AbsMaxCoolSetpointLimit::Id: {
        return YES;
    }
    case Attributes::PICoolingDemand::Id: {
        return YES;
    }
    case Attributes::PIHeatingDemand::Id: {
        return YES;
    }
    case Attributes::HVACSystemTypeConfiguration::Id: {
        return YES;
    }
    case Attributes::LocalTemperatureCalibration::Id: {
        return YES;
    }
    case Attributes::OccupiedCoolingSetpoint::Id: {
        return YES;
    }
    case Attributes::OccupiedHeatingSetpoint::Id: {
        return YES;
    }
    case Attributes::UnoccupiedCoolingSetpoint::Id: {
        return YES;
    }
    case Attributes::UnoccupiedHeatingSetpoint::Id: {
        return YES;
    }
    case Attributes::MinHeatSetpointLimit::Id: {
        return YES;
    }
    case Attributes::MaxHeatSetpointLimit::Id: {
        return YES;
    }
    case Attributes::MinCoolSetpointLimit::Id: {
        return YES;
    }
    case Attributes::MaxCoolSetpointLimit::Id: {
        return YES;
    }
    case Attributes::MinSetpointDeadBand::Id: {
        return YES;
    }
    case Attributes::RemoteSensing::Id: {
        return YES;
    }
    case Attributes::ControlSequenceOfOperation::Id: {
        return YES;
    }
    case Attributes::SystemMode::Id: {
        return YES;
    }
    case Attributes::ThermostatRunningMode::Id: {
        return YES;
    }
    case Attributes::StartOfWeek::Id: {
        return YES;
    }
    case Attributes::NumberOfWeeklyTransitions::Id: {
        return YES;
    }
    case Attributes::NumberOfDailyTransitions::Id: {
        return YES;
    }
    case Attributes::TemperatureSetpointHold::Id: {
        return YES;
    }
    case Attributes::TemperatureSetpointHoldDuration::Id: {
        return YES;
    }
    case Attributes::ThermostatProgrammingOperationMode::Id: {
        return YES;
    }
    case Attributes::ThermostatRunningState::Id: {
        return YES;
    }
    case Attributes::SetpointChangeSource::Id: {
        return YES;
    }
    case Attributes::SetpointChangeAmount::Id: {
        return YES;
    }
    case Attributes::SetpointChangeSourceTimestamp::Id: {
        return YES;
    }
    case Attributes::OccupiedSetback::Id: {
        return YES;
    }
    case Attributes::OccupiedSetbackMin::Id: {
        return YES;
    }
    case Attributes::OccupiedSetbackMax::Id: {
        return YES;
    }
    case Attributes::UnoccupiedSetback::Id: {
        return YES;
    }
    case Attributes::UnoccupiedSetbackMin::Id: {
        return YES;
    }
    case Attributes::UnoccupiedSetbackMax::Id: {
        return YES;
    }
    case Attributes::EmergencyHeatDelta::Id: {
        return YES;
    }
    case Attributes::ACType::Id: {
        return YES;
    }
    case Attributes::ACCapacity::Id: {
        return YES;
    }
    case Attributes::ACRefrigerantType::Id: {
        return YES;
    }
    case Attributes::ACCompressorType::Id: {
        return YES;
    }
    case Attributes::ACErrorCode::Id: {
        return YES;
    }
    case Attributes::ACLouverPosition::Id: {
        return YES;
    }
    case Attributes::ACCoilTemperature::Id: {
        return YES;
    }
    case Attributes::ACCapacityformat::Id: {
        return YES;
    }
    case Attributes::PresetTypes::Id: {
        return YES;
    }
    case Attributes::ScheduleTypes::Id: {
        return YES;
    }
    case Attributes::NumberOfPresets::Id: {
        return YES;
    }
    case Attributes::NumberOfSchedules::Id: {
        return YES;
    }
    case Attributes::NumberOfScheduleTransitions::Id: {
        return YES;
    }
    case Attributes::NumberOfScheduleTransitionPerDay::Id: {
        return YES;
    }
    case Attributes::ActivePresetHandle::Id: {
        return YES;
    }
    case Attributes::ActiveScheduleHandle::Id: {
        return YES;
    }
    case Attributes::Presets::Id: {
        return YES;
    }
    case Attributes::Schedules::Id: {
        return YES;
    }
    case Attributes::PresetsSchedulesEditable::Id: {
        return YES;
    }
    case Attributes::TemperatureSetpointHoldPolicy::Id: {
        return YES;
    }
    case Attributes::SetpointHoldExpiryTimestamp::Id: {
        return YES;
    }
    case Attributes::QueuedPreset::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInFanControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::FanControl;
    switch (aAttributeId) {
    case Attributes::FanMode::Id: {
        return YES;
    }
    case Attributes::FanModeSequence::Id: {
        return YES;
    }
    case Attributes::PercentSetting::Id: {
        return YES;
    }
    case Attributes::PercentCurrent::Id: {
        return YES;
    }
    case Attributes::SpeedMax::Id: {
        return YES;
    }
    case Attributes::SpeedSetting::Id: {
        return YES;
    }
    case Attributes::SpeedCurrent::Id: {
        return YES;
    }
    case Attributes::RockSupport::Id: {
        return YES;
    }
    case Attributes::RockSetting::Id: {
        return YES;
    }
    case Attributes::WindSupport::Id: {
        return YES;
    }
    case Attributes::WindSetting::Id: {
        return YES;
    }
    case Attributes::AirflowDirection::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInThermostatUserInterfaceConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ThermostatUserInterfaceConfiguration;
    switch (aAttributeId) {
    case Attributes::TemperatureDisplayMode::Id: {
        return YES;
    }
    case Attributes::KeypadLockout::Id: {
        return YES;
    }
    case Attributes::ScheduleProgrammingVisibility::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInColorControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ColorControl;
    switch (aAttributeId) {
    case Attributes::CurrentHue::Id: {
        return YES;
    }
    case Attributes::CurrentSaturation::Id: {
        return YES;
    }
    case Attributes::RemainingTime::Id: {
        return YES;
    }
    case Attributes::CurrentX::Id: {
        return YES;
    }
    case Attributes::CurrentY::Id: {
        return YES;
    }
    case Attributes::DriftCompensation::Id: {
        return YES;
    }
    case Attributes::CompensationText::Id: {
        return YES;
    }
    case Attributes::ColorTemperatureMireds::Id: {
        return YES;
    }
    case Attributes::ColorMode::Id: {
        return YES;
    }
    case Attributes::Options::Id: {
        return YES;
    }
    case Attributes::NumberOfPrimaries::Id: {
        return YES;
    }
    case Attributes::Primary1X::Id: {
        return YES;
    }
    case Attributes::Primary1Y::Id: {
        return YES;
    }
    case Attributes::Primary1Intensity::Id: {
        return YES;
    }
    case Attributes::Primary2X::Id: {
        return YES;
    }
    case Attributes::Primary2Y::Id: {
        return YES;
    }
    case Attributes::Primary2Intensity::Id: {
        return YES;
    }
    case Attributes::Primary3X::Id: {
        return YES;
    }
    case Attributes::Primary3Y::Id: {
        return YES;
    }
    case Attributes::Primary3Intensity::Id: {
        return YES;
    }
    case Attributes::Primary4X::Id: {
        return YES;
    }
    case Attributes::Primary4Y::Id: {
        return YES;
    }
    case Attributes::Primary4Intensity::Id: {
        return YES;
    }
    case Attributes::Primary5X::Id: {
        return YES;
    }
    case Attributes::Primary5Y::Id: {
        return YES;
    }
    case Attributes::Primary5Intensity::Id: {
        return YES;
    }
    case Attributes::Primary6X::Id: {
        return YES;
    }
    case Attributes::Primary6Y::Id: {
        return YES;
    }
    case Attributes::Primary6Intensity::Id: {
        return YES;
    }
    case Attributes::WhitePointX::Id: {
        return YES;
    }
    case Attributes::WhitePointY::Id: {
        return YES;
    }
    case Attributes::ColorPointRX::Id: {
        return YES;
    }
    case Attributes::ColorPointRY::Id: {
        return YES;
    }
    case Attributes::ColorPointRIntensity::Id: {
        return YES;
    }
    case Attributes::ColorPointGX::Id: {
        return YES;
    }
    case Attributes::ColorPointGY::Id: {
        return YES;
    }
    case Attributes::ColorPointGIntensity::Id: {
        return YES;
    }
    case Attributes::ColorPointBX::Id: {
        return YES;
    }
    case Attributes::ColorPointBY::Id: {
        return YES;
    }
    case Attributes::ColorPointBIntensity::Id: {
        return YES;
    }
    case Attributes::EnhancedCurrentHue::Id: {
        return YES;
    }
    case Attributes::EnhancedColorMode::Id: {
        return YES;
    }
    case Attributes::ColorLoopActive::Id: {
        return YES;
    }
    case Attributes::ColorLoopDirection::Id: {
        return YES;
    }
    case Attributes::ColorLoopTime::Id: {
        return YES;
    }
    case Attributes::ColorLoopStartEnhancedHue::Id: {
        return YES;
    }
    case Attributes::ColorLoopStoredEnhancedHue::Id: {
        return YES;
    }
    case Attributes::ColorCapabilities::Id: {
        return YES;
    }
    case Attributes::ColorTempPhysicalMinMireds::Id: {
        return YES;
    }
    case Attributes::ColorTempPhysicalMaxMireds::Id: {
        return YES;
    }
    case Attributes::CoupleColorTempToLevelMinMireds::Id: {
        return YES;
    }
    case Attributes::StartUpColorTemperatureMireds::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInBallastConfigurationCluster(AttributeId aAttributeId)
{
    using namespace Clusters::BallastConfiguration;
    switch (aAttributeId) {
    case Attributes::PhysicalMinLevel::Id: {
        return YES;
    }
    case Attributes::PhysicalMaxLevel::Id: {
        return YES;
    }
    case Attributes::BallastStatus::Id: {
        return YES;
    }
    case Attributes::MinLevel::Id: {
        return YES;
    }
    case Attributes::MaxLevel::Id: {
        return YES;
    }
    case Attributes::IntrinsicBallastFactor::Id: {
        return YES;
    }
    case Attributes::BallastFactorAdjustment::Id: {
        return YES;
    }
    case Attributes::LampQuantity::Id: {
        return YES;
    }
    case Attributes::LampType::Id: {
        return YES;
    }
    case Attributes::LampManufacturer::Id: {
        return YES;
    }
    case Attributes::LampRatedHours::Id: {
        return YES;
    }
    case Attributes::LampBurnHours::Id: {
        return YES;
    }
    case Attributes::LampAlarmMode::Id: {
        return YES;
    }
    case Attributes::LampBurnHoursTripPoint::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInIlluminanceMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::IlluminanceMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasuredValue::Id: {
        return YES;
    }
    case Attributes::MinMeasuredValue::Id: {
        return YES;
    }
    case Attributes::MaxMeasuredValue::Id: {
        return YES;
    }
    case Attributes::Tolerance::Id: {
        return YES;
    }
    case Attributes::LightSensorType::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInTemperatureMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TemperatureMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasuredValue::Id: {
        return YES;
    }
    case Attributes::MinMeasuredValue::Id: {
        return YES;
    }
    case Attributes::MaxMeasuredValue::Id: {
        return YES;
    }
    case Attributes::Tolerance::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInPressureMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::PressureMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasuredValue::Id: {
        return YES;
    }
    case Attributes::MinMeasuredValue::Id: {
        return YES;
    }
    case Attributes::MaxMeasuredValue::Id: {
        return YES;
    }
    case Attributes::Tolerance::Id: {
        return YES;
    }
    case Attributes::ScaledValue::Id: {
        return YES;
    }
    case Attributes::MinScaledValue::Id: {
        return YES;
    }
    case Attributes::MaxScaledValue::Id: {
        return YES;
    }
    case Attributes::ScaledTolerance::Id: {
        return YES;
    }
    case Attributes::Scale::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInFlowMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::FlowMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasuredValue::Id: {
        return YES;
    }
    case Attributes::MinMeasuredValue::Id: {
        return YES;
    }
    case Attributes::MaxMeasuredValue::Id: {
        return YES;
    }
    case Attributes::Tolerance::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInRelativeHumidityMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RelativeHumidityMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasuredValue::Id: {
        return YES;
    }
    case Attributes::MinMeasuredValue::Id: {
        return YES;
    }
    case Attributes::MaxMeasuredValue::Id: {
        return YES;
    }
    case Attributes::Tolerance::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInOccupancySensingCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OccupancySensing;
    switch (aAttributeId) {
    case Attributes::Occupancy::Id: {
        return YES;
    }
    case Attributes::OccupancySensorType::Id: {
        return YES;
    }
    case Attributes::OccupancySensorTypeBitmap::Id: {
        return YES;
    }
    case Attributes::PIROccupiedToUnoccupiedDelay::Id: {
        return YES;
    }
    case Attributes::PIRUnoccupiedToOccupiedDelay::Id: {
        return YES;
    }
    case Attributes::PIRUnoccupiedToOccupiedThreshold::Id: {
        return YES;
    }
    case Attributes::UltrasonicOccupiedToUnoccupiedDelay::Id: {
        return YES;
    }
    case Attributes::UltrasonicUnoccupiedToOccupiedDelay::Id: {
        return YES;
    }
    case Attributes::UltrasonicUnoccupiedToOccupiedThreshold::Id: {
        return YES;
    }
    case Attributes::PhysicalContactOccupiedToUnoccupiedDelay::Id: {
        return YES;
    }
    case Attributes::PhysicalContactUnoccupiedToOccupiedDelay::Id: {
        return YES;
    }
    case Attributes::PhysicalContactUnoccupiedToOccupiedThreshold::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInCarbonMonoxideConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CarbonMonoxideConcentrationMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasuredValue::Id: {
        return YES;
    }
    case Attributes::MinMeasuredValue::Id: {
        return YES;
    }
    case Attributes::MaxMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValue::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::Uncertainty::Id: {
        return YES;
    }
    case Attributes::MeasurementUnit::Id: {
        return YES;
    }
    case Attributes::MeasurementMedium::Id: {
        return YES;
    }
    case Attributes::LevelValue::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInCarbonDioxideConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CarbonDioxideConcentrationMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasuredValue::Id: {
        return YES;
    }
    case Attributes::MinMeasuredValue::Id: {
        return YES;
    }
    case Attributes::MaxMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValue::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::Uncertainty::Id: {
        return YES;
    }
    case Attributes::MeasurementUnit::Id: {
        return YES;
    }
    case Attributes::MeasurementMedium::Id: {
        return YES;
    }
    case Attributes::LevelValue::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInNitrogenDioxideConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::NitrogenDioxideConcentrationMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasuredValue::Id: {
        return YES;
    }
    case Attributes::MinMeasuredValue::Id: {
        return YES;
    }
    case Attributes::MaxMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValue::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::Uncertainty::Id: {
        return YES;
    }
    case Attributes::MeasurementUnit::Id: {
        return YES;
    }
    case Attributes::MeasurementMedium::Id: {
        return YES;
    }
    case Attributes::LevelValue::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInOzoneConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OzoneConcentrationMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasuredValue::Id: {
        return YES;
    }
    case Attributes::MinMeasuredValue::Id: {
        return YES;
    }
    case Attributes::MaxMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValue::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::Uncertainty::Id: {
        return YES;
    }
    case Attributes::MeasurementUnit::Id: {
        return YES;
    }
    case Attributes::MeasurementMedium::Id: {
        return YES;
    }
    case Attributes::LevelValue::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInPM25ConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Pm25ConcentrationMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasuredValue::Id: {
        return YES;
    }
    case Attributes::MinMeasuredValue::Id: {
        return YES;
    }
    case Attributes::MaxMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValue::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::Uncertainty::Id: {
        return YES;
    }
    case Attributes::MeasurementUnit::Id: {
        return YES;
    }
    case Attributes::MeasurementMedium::Id: {
        return YES;
    }
    case Attributes::LevelValue::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInFormaldehydeConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::FormaldehydeConcentrationMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasuredValue::Id: {
        return YES;
    }
    case Attributes::MinMeasuredValue::Id: {
        return YES;
    }
    case Attributes::MaxMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValue::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::Uncertainty::Id: {
        return YES;
    }
    case Attributes::MeasurementUnit::Id: {
        return YES;
    }
    case Attributes::MeasurementMedium::Id: {
        return YES;
    }
    case Attributes::LevelValue::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInPM1ConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Pm1ConcentrationMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasuredValue::Id: {
        return YES;
    }
    case Attributes::MinMeasuredValue::Id: {
        return YES;
    }
    case Attributes::MaxMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValue::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::Uncertainty::Id: {
        return YES;
    }
    case Attributes::MeasurementUnit::Id: {
        return YES;
    }
    case Attributes::MeasurementMedium::Id: {
        return YES;
    }
    case Attributes::LevelValue::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInPM10ConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Pm10ConcentrationMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasuredValue::Id: {
        return YES;
    }
    case Attributes::MinMeasuredValue::Id: {
        return YES;
    }
    case Attributes::MaxMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValue::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::Uncertainty::Id: {
        return YES;
    }
    case Attributes::MeasurementUnit::Id: {
        return YES;
    }
    case Attributes::MeasurementMedium::Id: {
        return YES;
    }
    case Attributes::LevelValue::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInTotalVolatileOrganicCompoundsConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasuredValue::Id: {
        return YES;
    }
    case Attributes::MinMeasuredValue::Id: {
        return YES;
    }
    case Attributes::MaxMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValue::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::Uncertainty::Id: {
        return YES;
    }
    case Attributes::MeasurementUnit::Id: {
        return YES;
    }
    case Attributes::MeasurementMedium::Id: {
        return YES;
    }
    case Attributes::LevelValue::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInRadonConcentrationMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RadonConcentrationMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasuredValue::Id: {
        return YES;
    }
    case Attributes::MinMeasuredValue::Id: {
        return YES;
    }
    case Attributes::MaxMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValue::Id: {
        return YES;
    }
    case Attributes::PeakMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValue::Id: {
        return YES;
    }
    case Attributes::AverageMeasuredValueWindow::Id: {
        return YES;
    }
    case Attributes::Uncertainty::Id: {
        return YES;
    }
    case Attributes::MeasurementUnit::Id: {
        return YES;
    }
    case Attributes::MeasurementMedium::Id: {
        return YES;
    }
    case Attributes::LevelValue::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInWakeOnLANCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WakeOnLan;
    switch (aAttributeId) {
    case Attributes::MACAddress::Id: {
        return YES;
    }
    case Attributes::LinkLocalAddress::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInChannelCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Channel;
    switch (aAttributeId) {
    case Attributes::ChannelList::Id: {
        return YES;
    }
    case Attributes::Lineup::Id: {
        return YES;
    }
    case Attributes::CurrentChannel::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInTargetNavigatorCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TargetNavigator;
    switch (aAttributeId) {
    case Attributes::TargetList::Id: {
        return YES;
    }
    case Attributes::CurrentTarget::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInMediaPlaybackCluster(AttributeId aAttributeId)
{
    using namespace Clusters::MediaPlayback;
    switch (aAttributeId) {
    case Attributes::CurrentState::Id: {
        return YES;
    }
    case Attributes::StartTime::Id: {
        return YES;
    }
    case Attributes::Duration::Id: {
        return YES;
    }
    case Attributes::SampledPosition::Id: {
        return YES;
    }
    case Attributes::PlaybackSpeed::Id: {
        return YES;
    }
    case Attributes::SeekRangeEnd::Id: {
        return YES;
    }
    case Attributes::SeekRangeStart::Id: {
        return YES;
    }
    case Attributes::ActiveAudioTrack::Id: {
        return YES;
    }
    case Attributes::AvailableAudioTracks::Id: {
        return YES;
    }
    case Attributes::ActiveTextTrack::Id: {
        return YES;
    }
    case Attributes::AvailableTextTracks::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInMediaInputCluster(AttributeId aAttributeId)
{
    using namespace Clusters::MediaInput;
    switch (aAttributeId) {
    case Attributes::InputList::Id: {
        return YES;
    }
    case Attributes::CurrentInput::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInLowPowerCluster(AttributeId aAttributeId)
{
    using namespace Clusters::LowPower;
    switch (aAttributeId) {
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInKeypadInputCluster(AttributeId aAttributeId)
{
    using namespace Clusters::KeypadInput;
    switch (aAttributeId) {
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInContentLauncherCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ContentLauncher;
    switch (aAttributeId) {
    case Attributes::AcceptHeader::Id: {
        return YES;
    }
    case Attributes::SupportedStreamingProtocols::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInAudioOutputCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AudioOutput;
    switch (aAttributeId) {
    case Attributes::OutputList::Id: {
        return YES;
    }
    case Attributes::CurrentOutput::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInApplicationLauncherCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ApplicationLauncher;
    switch (aAttributeId) {
    case Attributes::CatalogList::Id: {
        return YES;
    }
    case Attributes::CurrentApp::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInApplicationBasicCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ApplicationBasic;
    switch (aAttributeId) {
    case Attributes::VendorName::Id: {
        return YES;
    }
    case Attributes::VendorID::Id: {
        return YES;
    }
    case Attributes::ApplicationName::Id: {
        return YES;
    }
    case Attributes::ProductID::Id: {
        return YES;
    }
    case Attributes::Application::Id: {
        return YES;
    }
    case Attributes::Status::Id: {
        return YES;
    }
    case Attributes::ApplicationVersion::Id: {
        return YES;
    }
    case Attributes::AllowedVendorList::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInAccountLoginCluster(AttributeId aAttributeId)
{
    using namespace Clusters::AccountLogin;
    switch (aAttributeId) {
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInContentControlCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ContentControl;
    switch (aAttributeId) {
    case Attributes::Enabled::Id: {
        return YES;
    }
    case Attributes::OnDemandRatings::Id: {
        return YES;
    }
    case Attributes::OnDemandRatingThreshold::Id: {
        return YES;
    }
    case Attributes::ScheduledContentRatings::Id: {
        return YES;
    }
    case Attributes::ScheduledContentRatingThreshold::Id: {
        return YES;
    }
    case Attributes::ScreenDailyTime::Id: {
        return YES;
    }
    case Attributes::RemainingScreenTime::Id: {
        return YES;
    }
    case Attributes::BlockUnrated::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInContentAppObserverCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ContentAppObserver;
    switch (aAttributeId) {
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInElectricalMeasurementCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ElectricalMeasurement;
    switch (aAttributeId) {
    case Attributes::MeasurementType::Id: {
        return YES;
    }
    case Attributes::DcVoltage::Id: {
        return YES;
    }
    case Attributes::DcVoltageMin::Id: {
        return YES;
    }
    case Attributes::DcVoltageMax::Id: {
        return YES;
    }
    case Attributes::DcCurrent::Id: {
        return YES;
    }
    case Attributes::DcCurrentMin::Id: {
        return YES;
    }
    case Attributes::DcCurrentMax::Id: {
        return YES;
    }
    case Attributes::DcPower::Id: {
        return YES;
    }
    case Attributes::DcPowerMin::Id: {
        return YES;
    }
    case Attributes::DcPowerMax::Id: {
        return YES;
    }
    case Attributes::DcVoltageMultiplier::Id: {
        return YES;
    }
    case Attributes::DcVoltageDivisor::Id: {
        return YES;
    }
    case Attributes::DcCurrentMultiplier::Id: {
        return YES;
    }
    case Attributes::DcCurrentDivisor::Id: {
        return YES;
    }
    case Attributes::DcPowerMultiplier::Id: {
        return YES;
    }
    case Attributes::DcPowerDivisor::Id: {
        return YES;
    }
    case Attributes::AcFrequency::Id: {
        return YES;
    }
    case Attributes::AcFrequencyMin::Id: {
        return YES;
    }
    case Attributes::AcFrequencyMax::Id: {
        return YES;
    }
    case Attributes::NeutralCurrent::Id: {
        return YES;
    }
    case Attributes::TotalActivePower::Id: {
        return YES;
    }
    case Attributes::TotalReactivePower::Id: {
        return YES;
    }
    case Attributes::TotalApparentPower::Id: {
        return YES;
    }
    case Attributes::Measured1stHarmonicCurrent::Id: {
        return YES;
    }
    case Attributes::Measured3rdHarmonicCurrent::Id: {
        return YES;
    }
    case Attributes::Measured5thHarmonicCurrent::Id: {
        return YES;
    }
    case Attributes::Measured7thHarmonicCurrent::Id: {
        return YES;
    }
    case Attributes::Measured9thHarmonicCurrent::Id: {
        return YES;
    }
    case Attributes::Measured11thHarmonicCurrent::Id: {
        return YES;
    }
    case Attributes::MeasuredPhase1stHarmonicCurrent::Id: {
        return YES;
    }
    case Attributes::MeasuredPhase3rdHarmonicCurrent::Id: {
        return YES;
    }
    case Attributes::MeasuredPhase5thHarmonicCurrent::Id: {
        return YES;
    }
    case Attributes::MeasuredPhase7thHarmonicCurrent::Id: {
        return YES;
    }
    case Attributes::MeasuredPhase9thHarmonicCurrent::Id: {
        return YES;
    }
    case Attributes::MeasuredPhase11thHarmonicCurrent::Id: {
        return YES;
    }
    case Attributes::AcFrequencyMultiplier::Id: {
        return YES;
    }
    case Attributes::AcFrequencyDivisor::Id: {
        return YES;
    }
    case Attributes::PowerMultiplier::Id: {
        return YES;
    }
    case Attributes::PowerDivisor::Id: {
        return YES;
    }
    case Attributes::HarmonicCurrentMultiplier::Id: {
        return YES;
    }
    case Attributes::PhaseHarmonicCurrentMultiplier::Id: {
        return YES;
    }
    case Attributes::InstantaneousVoltage::Id: {
        return YES;
    }
    case Attributes::InstantaneousLineCurrent::Id: {
        return YES;
    }
    case Attributes::InstantaneousActiveCurrent::Id: {
        return YES;
    }
    case Attributes::InstantaneousReactiveCurrent::Id: {
        return YES;
    }
    case Attributes::InstantaneousPower::Id: {
        return YES;
    }
    case Attributes::RmsVoltage::Id: {
        return YES;
    }
    case Attributes::RmsVoltageMin::Id: {
        return YES;
    }
    case Attributes::RmsVoltageMax::Id: {
        return YES;
    }
    case Attributes::RmsCurrent::Id: {
        return YES;
    }
    case Attributes::RmsCurrentMin::Id: {
        return YES;
    }
    case Attributes::RmsCurrentMax::Id: {
        return YES;
    }
    case Attributes::ActivePower::Id: {
        return YES;
    }
    case Attributes::ActivePowerMin::Id: {
        return YES;
    }
    case Attributes::ActivePowerMax::Id: {
        return YES;
    }
    case Attributes::ReactivePower::Id: {
        return YES;
    }
    case Attributes::ApparentPower::Id: {
        return YES;
    }
    case Attributes::PowerFactor::Id: {
        return YES;
    }
    case Attributes::AverageRmsVoltageMeasurementPeriod::Id: {
        return YES;
    }
    case Attributes::AverageRmsUnderVoltageCounter::Id: {
        return YES;
    }
    case Attributes::RmsExtremeOverVoltagePeriod::Id: {
        return YES;
    }
    case Attributes::RmsExtremeUnderVoltagePeriod::Id: {
        return YES;
    }
    case Attributes::RmsVoltageSagPeriod::Id: {
        return YES;
    }
    case Attributes::RmsVoltageSwellPeriod::Id: {
        return YES;
    }
    case Attributes::AcVoltageMultiplier::Id: {
        return YES;
    }
    case Attributes::AcVoltageDivisor::Id: {
        return YES;
    }
    case Attributes::AcCurrentMultiplier::Id: {
        return YES;
    }
    case Attributes::AcCurrentDivisor::Id: {
        return YES;
    }
    case Attributes::AcPowerMultiplier::Id: {
        return YES;
    }
    case Attributes::AcPowerDivisor::Id: {
        return YES;
    }
    case Attributes::OverloadAlarmsMask::Id: {
        return YES;
    }
    case Attributes::VoltageOverload::Id: {
        return YES;
    }
    case Attributes::CurrentOverload::Id: {
        return YES;
    }
    case Attributes::AcOverloadAlarmsMask::Id: {
        return YES;
    }
    case Attributes::AcVoltageOverload::Id: {
        return YES;
    }
    case Attributes::AcCurrentOverload::Id: {
        return YES;
    }
    case Attributes::AcActivePowerOverload::Id: {
        return YES;
    }
    case Attributes::AcReactivePowerOverload::Id: {
        return YES;
    }
    case Attributes::AverageRmsOverVoltage::Id: {
        return YES;
    }
    case Attributes::AverageRmsUnderVoltage::Id: {
        return YES;
    }
    case Attributes::RmsExtremeOverVoltage::Id: {
        return YES;
    }
    case Attributes::RmsExtremeUnderVoltage::Id: {
        return YES;
    }
    case Attributes::RmsVoltageSag::Id: {
        return YES;
    }
    case Attributes::RmsVoltageSwell::Id: {
        return YES;
    }
    case Attributes::LineCurrentPhaseB::Id: {
        return YES;
    }
    case Attributes::ActiveCurrentPhaseB::Id: {
        return YES;
    }
    case Attributes::ReactiveCurrentPhaseB::Id: {
        return YES;
    }
    case Attributes::RmsVoltagePhaseB::Id: {
        return YES;
    }
    case Attributes::RmsVoltageMinPhaseB::Id: {
        return YES;
    }
    case Attributes::RmsVoltageMaxPhaseB::Id: {
        return YES;
    }
    case Attributes::RmsCurrentPhaseB::Id: {
        return YES;
    }
    case Attributes::RmsCurrentMinPhaseB::Id: {
        return YES;
    }
    case Attributes::RmsCurrentMaxPhaseB::Id: {
        return YES;
    }
    case Attributes::ActivePowerPhaseB::Id: {
        return YES;
    }
    case Attributes::ActivePowerMinPhaseB::Id: {
        return YES;
    }
    case Attributes::ActivePowerMaxPhaseB::Id: {
        return YES;
    }
    case Attributes::ReactivePowerPhaseB::Id: {
        return YES;
    }
    case Attributes::ApparentPowerPhaseB::Id: {
        return YES;
    }
    case Attributes::PowerFactorPhaseB::Id: {
        return YES;
    }
    case Attributes::AverageRmsVoltageMeasurementPeriodPhaseB::Id: {
        return YES;
    }
    case Attributes::AverageRmsOverVoltageCounterPhaseB::Id: {
        return YES;
    }
    case Attributes::AverageRmsUnderVoltageCounterPhaseB::Id: {
        return YES;
    }
    case Attributes::RmsExtremeOverVoltagePeriodPhaseB::Id: {
        return YES;
    }
    case Attributes::RmsExtremeUnderVoltagePeriodPhaseB::Id: {
        return YES;
    }
    case Attributes::RmsVoltageSagPeriodPhaseB::Id: {
        return YES;
    }
    case Attributes::RmsVoltageSwellPeriodPhaseB::Id: {
        return YES;
    }
    case Attributes::LineCurrentPhaseC::Id: {
        return YES;
    }
    case Attributes::ActiveCurrentPhaseC::Id: {
        return YES;
    }
    case Attributes::ReactiveCurrentPhaseC::Id: {
        return YES;
    }
    case Attributes::RmsVoltagePhaseC::Id: {
        return YES;
    }
    case Attributes::RmsVoltageMinPhaseC::Id: {
        return YES;
    }
    case Attributes::RmsVoltageMaxPhaseC::Id: {
        return YES;
    }
    case Attributes::RmsCurrentPhaseC::Id: {
        return YES;
    }
    case Attributes::RmsCurrentMinPhaseC::Id: {
        return YES;
    }
    case Attributes::RmsCurrentMaxPhaseC::Id: {
        return YES;
    }
    case Attributes::ActivePowerPhaseC::Id: {
        return YES;
    }
    case Attributes::ActivePowerMinPhaseC::Id: {
        return YES;
    }
    case Attributes::ActivePowerMaxPhaseC::Id: {
        return YES;
    }
    case Attributes::ReactivePowerPhaseC::Id: {
        return YES;
    }
    case Attributes::ApparentPowerPhaseC::Id: {
        return YES;
    }
    case Attributes::PowerFactorPhaseC::Id: {
        return YES;
    }
    case Attributes::AverageRmsVoltageMeasurementPeriodPhaseC::Id: {
        return YES;
    }
    case Attributes::AverageRmsOverVoltageCounterPhaseC::Id: {
        return YES;
    }
    case Attributes::AverageRmsUnderVoltageCounterPhaseC::Id: {
        return YES;
    }
    case Attributes::RmsExtremeOverVoltagePeriodPhaseC::Id: {
        return YES;
    }
    case Attributes::RmsExtremeUnderVoltagePeriodPhaseC::Id: {
        return YES;
    }
    case Attributes::RmsVoltageSagPeriodPhaseC::Id: {
        return YES;
    }
    case Attributes::RmsVoltageSwellPeriodPhaseC::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInUnitTestingCluster(AttributeId aAttributeId)
{
    using namespace Clusters::UnitTesting;
    switch (aAttributeId) {
    case Attributes::Boolean::Id: {
        return YES;
    }
    case Attributes::Bitmap8::Id: {
        return YES;
    }
    case Attributes::Bitmap16::Id: {
        return YES;
    }
    case Attributes::Bitmap32::Id: {
        return YES;
    }
    case Attributes::Bitmap64::Id: {
        return YES;
    }
    case Attributes::Int8u::Id: {
        return YES;
    }
    case Attributes::Int16u::Id: {
        return YES;
    }
    case Attributes::Int24u::Id: {
        return YES;
    }
    case Attributes::Int32u::Id: {
        return YES;
    }
    case Attributes::Int40u::Id: {
        return YES;
    }
    case Attributes::Int48u::Id: {
        return YES;
    }
    case Attributes::Int56u::Id: {
        return YES;
    }
    case Attributes::Int64u::Id: {
        return YES;
    }
    case Attributes::Int8s::Id: {
        return YES;
    }
    case Attributes::Int16s::Id: {
        return YES;
    }
    case Attributes::Int24s::Id: {
        return YES;
    }
    case Attributes::Int32s::Id: {
        return YES;
    }
    case Attributes::Int40s::Id: {
        return YES;
    }
    case Attributes::Int48s::Id: {
        return YES;
    }
    case Attributes::Int56s::Id: {
        return YES;
    }
    case Attributes::Int64s::Id: {
        return YES;
    }
    case Attributes::Enum8::Id: {
        return YES;
    }
    case Attributes::Enum16::Id: {
        return YES;
    }
    case Attributes::FloatSingle::Id: {
        return YES;
    }
    case Attributes::FloatDouble::Id: {
        return YES;
    }
    case Attributes::OctetString::Id: {
        return YES;
    }
    case Attributes::ListInt8u::Id: {
        return YES;
    }
    case Attributes::ListOctetString::Id: {
        return YES;
    }
    case Attributes::ListStructOctetString::Id: {
        return YES;
    }
    case Attributes::LongOctetString::Id: {
        return YES;
    }
    case Attributes::CharString::Id: {
        return YES;
    }
    case Attributes::LongCharString::Id: {
        return YES;
    }
    case Attributes::EpochUs::Id: {
        return YES;
    }
    case Attributes::EpochS::Id: {
        return YES;
    }
    case Attributes::VendorId::Id: {
        return YES;
    }
    case Attributes::ListNullablesAndOptionalsStruct::Id: {
        return YES;
    }
    case Attributes::EnumAttr::Id: {
        return YES;
    }
    case Attributes::StructAttr::Id: {
        return YES;
    }
    case Attributes::RangeRestrictedInt8u::Id: {
        return YES;
    }
    case Attributes::RangeRestrictedInt8s::Id: {
        return YES;
    }
    case Attributes::RangeRestrictedInt16u::Id: {
        return YES;
    }
    case Attributes::RangeRestrictedInt16s::Id: {
        return YES;
    }
    case Attributes::ListLongOctetString::Id: {
        return YES;
    }
    case Attributes::ListFabricScoped::Id: {
        return YES;
    }
    case Attributes::TimedWriteBoolean::Id: {
        return YES;
    }
    case Attributes::GeneralErrorBoolean::Id: {
        return YES;
    }
    case Attributes::ClusterErrorBoolean::Id: {
        return YES;
    }
    case Attributes::Unsupported::Id: {
        return YES;
    }
    case Attributes::NullableBoolean::Id: {
        return YES;
    }
    case Attributes::NullableBitmap8::Id: {
        return YES;
    }
    case Attributes::NullableBitmap16::Id: {
        return YES;
    }
    case Attributes::NullableBitmap32::Id: {
        return YES;
    }
    case Attributes::NullableBitmap64::Id: {
        return YES;
    }
    case Attributes::NullableInt8u::Id: {
        return YES;
    }
    case Attributes::NullableInt16u::Id: {
        return YES;
    }
    case Attributes::NullableInt24u::Id: {
        return YES;
    }
    case Attributes::NullableInt32u::Id: {
        return YES;
    }
    case Attributes::NullableInt40u::Id: {
        return YES;
    }
    case Attributes::NullableInt48u::Id: {
        return YES;
    }
    case Attributes::NullableInt56u::Id: {
        return YES;
    }
    case Attributes::NullableInt64u::Id: {
        return YES;
    }
    case Attributes::NullableInt8s::Id: {
        return YES;
    }
    case Attributes::NullableInt16s::Id: {
        return YES;
    }
    case Attributes::NullableInt24s::Id: {
        return YES;
    }
    case Attributes::NullableInt32s::Id: {
        return YES;
    }
    case Attributes::NullableInt40s::Id: {
        return YES;
    }
    case Attributes::NullableInt48s::Id: {
        return YES;
    }
    case Attributes::NullableInt56s::Id: {
        return YES;
    }
    case Attributes::NullableInt64s::Id: {
        return YES;
    }
    case Attributes::NullableEnum8::Id: {
        return YES;
    }
    case Attributes::NullableEnum16::Id: {
        return YES;
    }
    case Attributes::NullableFloatSingle::Id: {
        return YES;
    }
    case Attributes::NullableFloatDouble::Id: {
        return YES;
    }
    case Attributes::NullableOctetString::Id: {
        return YES;
    }
    case Attributes::NullableCharString::Id: {
        return YES;
    }
    case Attributes::NullableEnumAttr::Id: {
        return YES;
    }
    case Attributes::NullableStruct::Id: {
        return YES;
    }
    case Attributes::NullableRangeRestrictedInt8u::Id: {
        return YES;
    }
    case Attributes::NullableRangeRestrictedInt8s::Id: {
        return YES;
    }
    case Attributes::NullableRangeRestrictedInt16u::Id: {
        return YES;
    }
    case Attributes::NullableRangeRestrictedInt16s::Id: {
        return YES;
    }
    case Attributes::WriteOnlyInt8u::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    case Attributes::MeiInt8u::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}
static BOOL AttributeIsSpecifiedInSampleMEICluster(AttributeId aAttributeId)
{
    using namespace Clusters::SampleMei;
    switch (aAttributeId) {
    case Attributes::FlipFlop::Id: {
        return YES;
    }
    case Attributes::GeneratedCommandList::Id: {
        return YES;
    }
    case Attributes::AcceptedCommandList::Id: {
        return YES;
    }
    case Attributes::EventList::Id: {
        return YES;
    }
    case Attributes::AttributeList::Id: {
        return YES;
    }
    case Attributes::FeatureMap::Id: {
        return YES;
    }
    case Attributes::ClusterRevision::Id: {
        return YES;
    }
    default: {
        return NO;
    }
    }
}

BOOL MTRAttributeIsSpecified(ClusterId aClusterId, AttributeId aAttributeId)
{
    switch (aClusterId) {
    case Clusters::Identify::Id: {
        return AttributeIsSpecifiedInIdentifyCluster(aAttributeId);
    }
    case Clusters::Groups::Id: {
        return AttributeIsSpecifiedInGroupsCluster(aAttributeId);
    }
    case Clusters::OnOff::Id: {
        return AttributeIsSpecifiedInOnOffCluster(aAttributeId);
    }
    case Clusters::OnOffSwitchConfiguration::Id: {
        return AttributeIsSpecifiedInOnOffSwitchConfigurationCluster(aAttributeId);
    }
    case Clusters::LevelControl::Id: {
        return AttributeIsSpecifiedInLevelControlCluster(aAttributeId);
    }
    case Clusters::BinaryInputBasic::Id: {
        return AttributeIsSpecifiedInBinaryInputBasicCluster(aAttributeId);
    }
    case Clusters::PulseWidthModulation::Id: {
        return AttributeIsSpecifiedInPulseWidthModulationCluster(aAttributeId);
    }
    case Clusters::Descriptor::Id: {
        return AttributeIsSpecifiedInDescriptorCluster(aAttributeId);
    }
    case Clusters::Binding::Id: {
        return AttributeIsSpecifiedInBindingCluster(aAttributeId);
    }
    case Clusters::AccessControl::Id: {
        return AttributeIsSpecifiedInAccessControlCluster(aAttributeId);
    }
    case Clusters::Actions::Id: {
        return AttributeIsSpecifiedInActionsCluster(aAttributeId);
    }
    case Clusters::BasicInformation::Id: {
        return AttributeIsSpecifiedInBasicInformationCluster(aAttributeId);
    }
    case Clusters::OtaSoftwareUpdateProvider::Id: {
        return AttributeIsSpecifiedInOTASoftwareUpdateProviderCluster(aAttributeId);
    }
    case Clusters::OtaSoftwareUpdateRequestor::Id: {
        return AttributeIsSpecifiedInOTASoftwareUpdateRequestorCluster(aAttributeId);
    }
    case Clusters::LocalizationConfiguration::Id: {
        return AttributeIsSpecifiedInLocalizationConfigurationCluster(aAttributeId);
    }
    case Clusters::TimeFormatLocalization::Id: {
        return AttributeIsSpecifiedInTimeFormatLocalizationCluster(aAttributeId);
    }
    case Clusters::UnitLocalization::Id: {
        return AttributeIsSpecifiedInUnitLocalizationCluster(aAttributeId);
    }
    case Clusters::PowerSourceConfiguration::Id: {
        return AttributeIsSpecifiedInPowerSourceConfigurationCluster(aAttributeId);
    }
    case Clusters::PowerSource::Id: {
        return AttributeIsSpecifiedInPowerSourceCluster(aAttributeId);
    }
    case Clusters::GeneralCommissioning::Id: {
        return AttributeIsSpecifiedInGeneralCommissioningCluster(aAttributeId);
    }
    case Clusters::NetworkCommissioning::Id: {
        return AttributeIsSpecifiedInNetworkCommissioningCluster(aAttributeId);
    }
    case Clusters::DiagnosticLogs::Id: {
        return AttributeIsSpecifiedInDiagnosticLogsCluster(aAttributeId);
    }
    case Clusters::GeneralDiagnostics::Id: {
        return AttributeIsSpecifiedInGeneralDiagnosticsCluster(aAttributeId);
    }
    case Clusters::SoftwareDiagnostics::Id: {
        return AttributeIsSpecifiedInSoftwareDiagnosticsCluster(aAttributeId);
    }
    case Clusters::ThreadNetworkDiagnostics::Id: {
        return AttributeIsSpecifiedInThreadNetworkDiagnosticsCluster(aAttributeId);
    }
    case Clusters::WiFiNetworkDiagnostics::Id: {
        return AttributeIsSpecifiedInWiFiNetworkDiagnosticsCluster(aAttributeId);
    }
    case Clusters::EthernetNetworkDiagnostics::Id: {
        return AttributeIsSpecifiedInEthernetNetworkDiagnosticsCluster(aAttributeId);
    }
    case Clusters::TimeSynchronization::Id: {
        return AttributeIsSpecifiedInTimeSynchronizationCluster(aAttributeId);
    }
    case Clusters::BridgedDeviceBasicInformation::Id: {
        return AttributeIsSpecifiedInBridgedDeviceBasicInformationCluster(aAttributeId);
    }
    case Clusters::Switch::Id: {
        return AttributeIsSpecifiedInSwitchCluster(aAttributeId);
    }
    case Clusters::AdministratorCommissioning::Id: {
        return AttributeIsSpecifiedInAdministratorCommissioningCluster(aAttributeId);
    }
    case Clusters::OperationalCredentials::Id: {
        return AttributeIsSpecifiedInOperationalCredentialsCluster(aAttributeId);
    }
    case Clusters::GroupKeyManagement::Id: {
        return AttributeIsSpecifiedInGroupKeyManagementCluster(aAttributeId);
    }
    case Clusters::FixedLabel::Id: {
        return AttributeIsSpecifiedInFixedLabelCluster(aAttributeId);
    }
    case Clusters::UserLabel::Id: {
        return AttributeIsSpecifiedInUserLabelCluster(aAttributeId);
    }
    case Clusters::BooleanState::Id: {
        return AttributeIsSpecifiedInBooleanStateCluster(aAttributeId);
    }
    case Clusters::IcdManagement::Id: {
        return AttributeIsSpecifiedInICDManagementCluster(aAttributeId);
    }
    case Clusters::Timer::Id: {
        return AttributeIsSpecifiedInTimerCluster(aAttributeId);
    }
    case Clusters::OvenCavityOperationalState::Id: {
        return AttributeIsSpecifiedInOvenCavityOperationalStateCluster(aAttributeId);
    }
    case Clusters::OvenMode::Id: {
        return AttributeIsSpecifiedInOvenModeCluster(aAttributeId);
    }
    case Clusters::LaundryDryerControls::Id: {
        return AttributeIsSpecifiedInLaundryDryerControlsCluster(aAttributeId);
    }
    case Clusters::ModeSelect::Id: {
        return AttributeIsSpecifiedInModeSelectCluster(aAttributeId);
    }
    case Clusters::LaundryWasherMode::Id: {
        return AttributeIsSpecifiedInLaundryWasherModeCluster(aAttributeId);
    }
    case Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Id: {
        return AttributeIsSpecifiedInRefrigeratorAndTemperatureControlledCabinetModeCluster(aAttributeId);
    }
    case Clusters::LaundryWasherControls::Id: {
        return AttributeIsSpecifiedInLaundryWasherControlsCluster(aAttributeId);
    }
    case Clusters::RvcRunMode::Id: {
        return AttributeIsSpecifiedInRVCRunModeCluster(aAttributeId);
    }
    case Clusters::RvcCleanMode::Id: {
        return AttributeIsSpecifiedInRVCCleanModeCluster(aAttributeId);
    }
    case Clusters::TemperatureControl::Id: {
        return AttributeIsSpecifiedInTemperatureControlCluster(aAttributeId);
    }
    case Clusters::RefrigeratorAlarm::Id: {
        return AttributeIsSpecifiedInRefrigeratorAlarmCluster(aAttributeId);
    }
    case Clusters::DishwasherMode::Id: {
        return AttributeIsSpecifiedInDishwasherModeCluster(aAttributeId);
    }
    case Clusters::AirQuality::Id: {
        return AttributeIsSpecifiedInAirQualityCluster(aAttributeId);
    }
    case Clusters::SmokeCoAlarm::Id: {
        return AttributeIsSpecifiedInSmokeCOAlarmCluster(aAttributeId);
    }
    case Clusters::DishwasherAlarm::Id: {
        return AttributeIsSpecifiedInDishwasherAlarmCluster(aAttributeId);
    }
    case Clusters::MicrowaveOvenMode::Id: {
        return AttributeIsSpecifiedInMicrowaveOvenModeCluster(aAttributeId);
    }
    case Clusters::MicrowaveOvenControl::Id: {
        return AttributeIsSpecifiedInMicrowaveOvenControlCluster(aAttributeId);
    }
    case Clusters::OperationalState::Id: {
        return AttributeIsSpecifiedInOperationalStateCluster(aAttributeId);
    }
    case Clusters::RvcOperationalState::Id: {
        return AttributeIsSpecifiedInRVCOperationalStateCluster(aAttributeId);
    }
    case Clusters::ScenesManagement::Id: {
        return AttributeIsSpecifiedInScenesManagementCluster(aAttributeId);
    }
    case Clusters::HepaFilterMonitoring::Id: {
        return AttributeIsSpecifiedInHEPAFilterMonitoringCluster(aAttributeId);
    }
    case Clusters::ActivatedCarbonFilterMonitoring::Id: {
        return AttributeIsSpecifiedInActivatedCarbonFilterMonitoringCluster(aAttributeId);
    }
    case Clusters::BooleanStateConfiguration::Id: {
        return AttributeIsSpecifiedInBooleanStateConfigurationCluster(aAttributeId);
    }
    case Clusters::ValveConfigurationAndControl::Id: {
        return AttributeIsSpecifiedInValveConfigurationAndControlCluster(aAttributeId);
    }
    case Clusters::ElectricalPowerMeasurement::Id: {
        return AttributeIsSpecifiedInElectricalPowerMeasurementCluster(aAttributeId);
    }
    case Clusters::ElectricalEnergyMeasurement::Id: {
        return AttributeIsSpecifiedInElectricalEnergyMeasurementCluster(aAttributeId);
    }
    case Clusters::DemandResponseLoadControl::Id: {
        return AttributeIsSpecifiedInDemandResponseLoadControlCluster(aAttributeId);
    }
    case Clusters::Messages::Id: {
        return AttributeIsSpecifiedInMessagesCluster(aAttributeId);
    }
    case Clusters::DeviceEnergyManagement::Id: {
        return AttributeIsSpecifiedInDeviceEnergyManagementCluster(aAttributeId);
    }
    case Clusters::EnergyEvse::Id: {
        return AttributeIsSpecifiedInEnergyEVSECluster(aAttributeId);
    }
    case Clusters::EnergyPreference::Id: {
        return AttributeIsSpecifiedInEnergyPreferenceCluster(aAttributeId);
    }
    case Clusters::PowerTopology::Id: {
        return AttributeIsSpecifiedInPowerTopologyCluster(aAttributeId);
    }
    case Clusters::EnergyEvseMode::Id: {
        return AttributeIsSpecifiedInEnergyEVSEModeCluster(aAttributeId);
    }
    case Clusters::DeviceEnergyManagementMode::Id: {
        return AttributeIsSpecifiedInDeviceEnergyManagementModeCluster(aAttributeId);
    }
    case Clusters::DoorLock::Id: {
        return AttributeIsSpecifiedInDoorLockCluster(aAttributeId);
    }
    case Clusters::WindowCovering::Id: {
        return AttributeIsSpecifiedInWindowCoveringCluster(aAttributeId);
    }
    case Clusters::BarrierControl::Id: {
        return AttributeIsSpecifiedInBarrierControlCluster(aAttributeId);
    }
    case Clusters::PumpConfigurationAndControl::Id: {
        return AttributeIsSpecifiedInPumpConfigurationAndControlCluster(aAttributeId);
    }
    case Clusters::Thermostat::Id: {
        return AttributeIsSpecifiedInThermostatCluster(aAttributeId);
    }
    case Clusters::FanControl::Id: {
        return AttributeIsSpecifiedInFanControlCluster(aAttributeId);
    }
    case Clusters::ThermostatUserInterfaceConfiguration::Id: {
        return AttributeIsSpecifiedInThermostatUserInterfaceConfigurationCluster(aAttributeId);
    }
    case Clusters::ColorControl::Id: {
        return AttributeIsSpecifiedInColorControlCluster(aAttributeId);
    }
    case Clusters::BallastConfiguration::Id: {
        return AttributeIsSpecifiedInBallastConfigurationCluster(aAttributeId);
    }
    case Clusters::IlluminanceMeasurement::Id: {
        return AttributeIsSpecifiedInIlluminanceMeasurementCluster(aAttributeId);
    }
    case Clusters::TemperatureMeasurement::Id: {
        return AttributeIsSpecifiedInTemperatureMeasurementCluster(aAttributeId);
    }
    case Clusters::PressureMeasurement::Id: {
        return AttributeIsSpecifiedInPressureMeasurementCluster(aAttributeId);
    }
    case Clusters::FlowMeasurement::Id: {
        return AttributeIsSpecifiedInFlowMeasurementCluster(aAttributeId);
    }
    case Clusters::RelativeHumidityMeasurement::Id: {
        return AttributeIsSpecifiedInRelativeHumidityMeasurementCluster(aAttributeId);
    }
    case Clusters::OccupancySensing::Id: {
        return AttributeIsSpecifiedInOccupancySensingCluster(aAttributeId);
    }
    case Clusters::CarbonMonoxideConcentrationMeasurement::Id: {
        return AttributeIsSpecifiedInCarbonMonoxideConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::CarbonDioxideConcentrationMeasurement::Id: {
        return AttributeIsSpecifiedInCarbonDioxideConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::NitrogenDioxideConcentrationMeasurement::Id: {
        return AttributeIsSpecifiedInNitrogenDioxideConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::OzoneConcentrationMeasurement::Id: {
        return AttributeIsSpecifiedInOzoneConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::Pm25ConcentrationMeasurement::Id: {
        return AttributeIsSpecifiedInPM25ConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::FormaldehydeConcentrationMeasurement::Id: {
        return AttributeIsSpecifiedInFormaldehydeConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::Pm1ConcentrationMeasurement::Id: {
        return AttributeIsSpecifiedInPM1ConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::Pm10ConcentrationMeasurement::Id: {
        return AttributeIsSpecifiedInPM10ConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::Id: {
        return AttributeIsSpecifiedInTotalVolatileOrganicCompoundsConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::RadonConcentrationMeasurement::Id: {
        return AttributeIsSpecifiedInRadonConcentrationMeasurementCluster(aAttributeId);
    }
    case Clusters::WakeOnLan::Id: {
        return AttributeIsSpecifiedInWakeOnLANCluster(aAttributeId);
    }
    case Clusters::Channel::Id: {
        return AttributeIsSpecifiedInChannelCluster(aAttributeId);
    }
    case Clusters::TargetNavigator::Id: {
        return AttributeIsSpecifiedInTargetNavigatorCluster(aAttributeId);
    }
    case Clusters::MediaPlayback::Id: {
        return AttributeIsSpecifiedInMediaPlaybackCluster(aAttributeId);
    }
    case Clusters::MediaInput::Id: {
        return AttributeIsSpecifiedInMediaInputCluster(aAttributeId);
    }
    case Clusters::LowPower::Id: {
        return AttributeIsSpecifiedInLowPowerCluster(aAttributeId);
    }
    case Clusters::KeypadInput::Id: {
        return AttributeIsSpecifiedInKeypadInputCluster(aAttributeId);
    }
    case Clusters::ContentLauncher::Id: {
        return AttributeIsSpecifiedInContentLauncherCluster(aAttributeId);
    }
    case Clusters::AudioOutput::Id: {
        return AttributeIsSpecifiedInAudioOutputCluster(aAttributeId);
    }
    case Clusters::ApplicationLauncher::Id: {
        return AttributeIsSpecifiedInApplicationLauncherCluster(aAttributeId);
    }
    case Clusters::ApplicationBasic::Id: {
        return AttributeIsSpecifiedInApplicationBasicCluster(aAttributeId);
    }
    case Clusters::AccountLogin::Id: {
        return AttributeIsSpecifiedInAccountLoginCluster(aAttributeId);
    }
    case Clusters::ContentControl::Id: {
        return AttributeIsSpecifiedInContentControlCluster(aAttributeId);
    }
    case Clusters::ContentAppObserver::Id: {
        return AttributeIsSpecifiedInContentAppObserverCluster(aAttributeId);
    }
    case Clusters::ElectricalMeasurement::Id: {
        return AttributeIsSpecifiedInElectricalMeasurementCluster(aAttributeId);
    }
    case Clusters::UnitTesting::Id: {
        return AttributeIsSpecifiedInUnitTestingCluster(aAttributeId);
    }
    case Clusters::SampleMei::Id: {
        return AttributeIsSpecifiedInSampleMEICluster(aAttributeId);
    }
    default: {
        return NO;
    }
    }
}
