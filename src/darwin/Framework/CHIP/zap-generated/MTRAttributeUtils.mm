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

#import "MTRAttributeUtils.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

using namespace chip;
using namespace chip::app;

static BOOL MTRAttributeIsSpecifiedInIdentifyCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInGroupsCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInScenesCluster(AttributeId aAttributeId)
{
    using namespace Clusters::Scenes;
    switch (aAttributeId) {
    case Attributes::SceneCount::Id: {
        return YES;
    }
    case Attributes::CurrentScene::Id: {
        return YES;
    }
    case Attributes::CurrentGroup::Id: {
        return YES;
    }
    case Attributes::SceneValid::Id: {
        return YES;
    }
    case Attributes::NameSupport::Id: {
        return YES;
    }
    case Attributes::LastConfiguredBy::Id: {
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
static BOOL MTRAttributeIsSpecifiedInOnOffCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInOnOffSwitchConfigurationCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInLevelControlCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInBinaryInputBasicCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInDescriptorCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInBindingCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInAccessControlCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInActionsCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInBasicInformationCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInOTASoftwareUpdateProviderCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInOTASoftwareUpdateRequestorCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInLocalizationConfigurationCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInTimeFormatLocalizationCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInUnitLocalizationCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInPowerSourceConfigurationCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInPowerSourceCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInGeneralCommissioningCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInNetworkCommissioningCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInDiagnosticLogsCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInGeneralDiagnosticsCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInSoftwareDiagnosticsCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInThreadNetworkDiagnosticsCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInWiFiNetworkDiagnosticsCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInEthernetNetworkDiagnosticsCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInBridgedDeviceBasicInformationCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInSwitchCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInAdministratorCommissioningCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInOperationalCredentialsCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInGroupKeyManagementCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInFixedLabelCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInUserLabelCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInBooleanStateCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInModeSelectCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInTemperatureControlCluster(AttributeId aAttributeId)
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
    case Attributes::CurrentTemperatureLevelIndex::Id: {
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
static BOOL MTRAttributeIsSpecifiedInRefrigeratorAlarmCluster(AttributeId aAttributeId)
{
    using namespace Clusters::RefrigeratorAlarm;
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
static BOOL MTRAttributeIsSpecifiedInAirQualityCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInSmokeCOAlarmCluster(AttributeId aAttributeId)
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
    case Attributes::SensitivityLevel::Id: {
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
static BOOL MTRAttributeIsSpecifiedInHEPAFilterMonitoringCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInActivatedCarbonFilterMonitoringCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInCeramicFilterMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::CeramicFilterMonitoring;
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
static BOOL MTRAttributeIsSpecifiedInElectrostaticFilterMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ElectrostaticFilterMonitoring;
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
static BOOL MTRAttributeIsSpecifiedInUVFilterMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::UvFilterMonitoring;
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
static BOOL MTRAttributeIsSpecifiedInIonizingFilterMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::IonizingFilterMonitoring;
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
static BOOL MTRAttributeIsSpecifiedInZeoliteFilterMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::ZeoliteFilterMonitoring;
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
static BOOL MTRAttributeIsSpecifiedInOzoneFilterMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::OzoneFilterMonitoring;
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
static BOOL MTRAttributeIsSpecifiedInWaterTankMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WaterTankMonitoring;
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
static BOOL MTRAttributeIsSpecifiedInFuelTankMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::FuelTankMonitoring;
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
static BOOL MTRAttributeIsSpecifiedInInkCartridgeMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::InkCartridgeMonitoring;
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
static BOOL MTRAttributeIsSpecifiedInTonerCartridgeMonitoringCluster(AttributeId aAttributeId)
{
    using namespace Clusters::TonerCartridgeMonitoring;
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
static BOOL MTRAttributeIsSpecifiedInDoorLockCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInWindowCoveringCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInBarrierControlCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInPumpConfigurationAndControlCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInThermostatCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInFanControlCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInThermostatUserInterfaceConfigurationCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInColorControlCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInBallastConfigurationCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInIlluminanceMeasurementCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInTemperatureMeasurementCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInPressureMeasurementCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInFlowMeasurementCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInRelativeHumidityMeasurementCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInOccupancySensingCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInWakeOnLANCluster(AttributeId aAttributeId)
{
    using namespace Clusters::WakeOnLan;
    switch (aAttributeId) {
    case Attributes::MACAddress::Id: {
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
static BOOL MTRAttributeIsSpecifiedInChannelCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInTargetNavigatorCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInMediaPlaybackCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInMediaInputCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInLowPowerCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInKeypadInputCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInContentLauncherCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInAudioOutputCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInApplicationLauncherCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInApplicationBasicCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInAccountLoginCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInElectricalMeasurementCluster(AttributeId aAttributeId)
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
static BOOL MTRAttributeIsSpecifiedInUnitTestingCluster(AttributeId aAttributeId)
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
    default: {
        return NO;
    }
    }
}

BOOL MTRAttributeIsSpecified(ClusterId aClusterId, AttributeId aAttributeId)
{
    switch (aClusterId) {
    case Clusters::Identify::Id: {
        return MTRAttributeIsSpecifiedInIdentifyCluster(aAttributeId);
    }
    case Clusters::Groups::Id: {
        return MTRAttributeIsSpecifiedInGroupsCluster(aAttributeId);
    }
    case Clusters::Scenes::Id: {
        return MTRAttributeIsSpecifiedInScenesCluster(aAttributeId);
    }
    case Clusters::OnOff::Id: {
        return MTRAttributeIsSpecifiedInOnOffCluster(aAttributeId);
    }
    case Clusters::OnOffSwitchConfiguration::Id: {
        return MTRAttributeIsSpecifiedInOnOffSwitchConfigurationCluster(aAttributeId);
    }
    case Clusters::LevelControl::Id: {
        return MTRAttributeIsSpecifiedInLevelControlCluster(aAttributeId);
    }
    case Clusters::BinaryInputBasic::Id: {
        return MTRAttributeIsSpecifiedInBinaryInputBasicCluster(aAttributeId);
    }
    case Clusters::Descriptor::Id: {
        return MTRAttributeIsSpecifiedInDescriptorCluster(aAttributeId);
    }
    case Clusters::Binding::Id: {
        return MTRAttributeIsSpecifiedInBindingCluster(aAttributeId);
    }
    case Clusters::AccessControl::Id: {
        return MTRAttributeIsSpecifiedInAccessControlCluster(aAttributeId);
    }
    case Clusters::Actions::Id: {
        return MTRAttributeIsSpecifiedInActionsCluster(aAttributeId);
    }
    case Clusters::BasicInformation::Id: {
        return MTRAttributeIsSpecifiedInBasicInformationCluster(aAttributeId);
    }
    case Clusters::OtaSoftwareUpdateProvider::Id: {
        return MTRAttributeIsSpecifiedInOTASoftwareUpdateProviderCluster(aAttributeId);
    }
    case Clusters::OtaSoftwareUpdateRequestor::Id: {
        return MTRAttributeIsSpecifiedInOTASoftwareUpdateRequestorCluster(aAttributeId);
    }
    case Clusters::LocalizationConfiguration::Id: {
        return MTRAttributeIsSpecifiedInLocalizationConfigurationCluster(aAttributeId);
    }
    case Clusters::TimeFormatLocalization::Id: {
        return MTRAttributeIsSpecifiedInTimeFormatLocalizationCluster(aAttributeId);
    }
    case Clusters::UnitLocalization::Id: {
        return MTRAttributeIsSpecifiedInUnitLocalizationCluster(aAttributeId);
    }
    case Clusters::PowerSourceConfiguration::Id: {
        return MTRAttributeIsSpecifiedInPowerSourceConfigurationCluster(aAttributeId);
    }
    case Clusters::PowerSource::Id: {
        return MTRAttributeIsSpecifiedInPowerSourceCluster(aAttributeId);
    }
    case Clusters::GeneralCommissioning::Id: {
        return MTRAttributeIsSpecifiedInGeneralCommissioningCluster(aAttributeId);
    }
    case Clusters::NetworkCommissioning::Id: {
        return MTRAttributeIsSpecifiedInNetworkCommissioningCluster(aAttributeId);
    }
    case Clusters::DiagnosticLogs::Id: {
        return MTRAttributeIsSpecifiedInDiagnosticLogsCluster(aAttributeId);
    }
    case Clusters::GeneralDiagnostics::Id: {
        return MTRAttributeIsSpecifiedInGeneralDiagnosticsCluster(aAttributeId);
    }
    case Clusters::SoftwareDiagnostics::Id: {
        return MTRAttributeIsSpecifiedInSoftwareDiagnosticsCluster(aAttributeId);
    }
    case Clusters::ThreadNetworkDiagnostics::Id: {
        return MTRAttributeIsSpecifiedInThreadNetworkDiagnosticsCluster(aAttributeId);
    }
    case Clusters::WiFiNetworkDiagnostics::Id: {
        return MTRAttributeIsSpecifiedInWiFiNetworkDiagnosticsCluster(aAttributeId);
    }
    case Clusters::EthernetNetworkDiagnostics::Id: {
        return MTRAttributeIsSpecifiedInEthernetNetworkDiagnosticsCluster(aAttributeId);
    }
    case Clusters::BridgedDeviceBasicInformation::Id: {
        return MTRAttributeIsSpecifiedInBridgedDeviceBasicInformationCluster(aAttributeId);
    }
    case Clusters::Switch::Id: {
        return MTRAttributeIsSpecifiedInSwitchCluster(aAttributeId);
    }
    case Clusters::AdministratorCommissioning::Id: {
        return MTRAttributeIsSpecifiedInAdministratorCommissioningCluster(aAttributeId);
    }
    case Clusters::OperationalCredentials::Id: {
        return MTRAttributeIsSpecifiedInOperationalCredentialsCluster(aAttributeId);
    }
    case Clusters::GroupKeyManagement::Id: {
        return MTRAttributeIsSpecifiedInGroupKeyManagementCluster(aAttributeId);
    }
    case Clusters::FixedLabel::Id: {
        return MTRAttributeIsSpecifiedInFixedLabelCluster(aAttributeId);
    }
    case Clusters::UserLabel::Id: {
        return MTRAttributeIsSpecifiedInUserLabelCluster(aAttributeId);
    }
    case Clusters::BooleanState::Id: {
        return MTRAttributeIsSpecifiedInBooleanStateCluster(aAttributeId);
    }
    case Clusters::ModeSelect::Id: {
        return MTRAttributeIsSpecifiedInModeSelectCluster(aAttributeId);
    }
    case Clusters::TemperatureControl::Id: {
        return MTRAttributeIsSpecifiedInTemperatureControlCluster(aAttributeId);
    }
    case Clusters::RefrigeratorAlarm::Id: {
        return MTRAttributeIsSpecifiedInRefrigeratorAlarmCluster(aAttributeId);
    }
    case Clusters::AirQuality::Id: {
        return MTRAttributeIsSpecifiedInAirQualityCluster(aAttributeId);
    }
    case Clusters::SmokeCoAlarm::Id: {
        return MTRAttributeIsSpecifiedInSmokeCOAlarmCluster(aAttributeId);
    }
    case Clusters::HepaFilterMonitoring::Id: {
        return MTRAttributeIsSpecifiedInHEPAFilterMonitoringCluster(aAttributeId);
    }
    case Clusters::ActivatedCarbonFilterMonitoring::Id: {
        return MTRAttributeIsSpecifiedInActivatedCarbonFilterMonitoringCluster(aAttributeId);
    }
    case Clusters::CeramicFilterMonitoring::Id: {
        return MTRAttributeIsSpecifiedInCeramicFilterMonitoringCluster(aAttributeId);
    }
    case Clusters::ElectrostaticFilterMonitoring::Id: {
        return MTRAttributeIsSpecifiedInElectrostaticFilterMonitoringCluster(aAttributeId);
    }
    case Clusters::UvFilterMonitoring::Id: {
        return MTRAttributeIsSpecifiedInUVFilterMonitoringCluster(aAttributeId);
    }
    case Clusters::IonizingFilterMonitoring::Id: {
        return MTRAttributeIsSpecifiedInIonizingFilterMonitoringCluster(aAttributeId);
    }
    case Clusters::ZeoliteFilterMonitoring::Id: {
        return MTRAttributeIsSpecifiedInZeoliteFilterMonitoringCluster(aAttributeId);
    }
    case Clusters::OzoneFilterMonitoring::Id: {
        return MTRAttributeIsSpecifiedInOzoneFilterMonitoringCluster(aAttributeId);
    }
    case Clusters::WaterTankMonitoring::Id: {
        return MTRAttributeIsSpecifiedInWaterTankMonitoringCluster(aAttributeId);
    }
    case Clusters::FuelTankMonitoring::Id: {
        return MTRAttributeIsSpecifiedInFuelTankMonitoringCluster(aAttributeId);
    }
    case Clusters::InkCartridgeMonitoring::Id: {
        return MTRAttributeIsSpecifiedInInkCartridgeMonitoringCluster(aAttributeId);
    }
    case Clusters::TonerCartridgeMonitoring::Id: {
        return MTRAttributeIsSpecifiedInTonerCartridgeMonitoringCluster(aAttributeId);
    }
    case Clusters::DoorLock::Id: {
        return MTRAttributeIsSpecifiedInDoorLockCluster(aAttributeId);
    }
    case Clusters::WindowCovering::Id: {
        return MTRAttributeIsSpecifiedInWindowCoveringCluster(aAttributeId);
    }
    case Clusters::BarrierControl::Id: {
        return MTRAttributeIsSpecifiedInBarrierControlCluster(aAttributeId);
    }
    case Clusters::PumpConfigurationAndControl::Id: {
        return MTRAttributeIsSpecifiedInPumpConfigurationAndControlCluster(aAttributeId);
    }
    case Clusters::Thermostat::Id: {
        return MTRAttributeIsSpecifiedInThermostatCluster(aAttributeId);
    }
    case Clusters::FanControl::Id: {
        return MTRAttributeIsSpecifiedInFanControlCluster(aAttributeId);
    }
    case Clusters::ThermostatUserInterfaceConfiguration::Id: {
        return MTRAttributeIsSpecifiedInThermostatUserInterfaceConfigurationCluster(aAttributeId);
    }
    case Clusters::ColorControl::Id: {
        return MTRAttributeIsSpecifiedInColorControlCluster(aAttributeId);
    }
    case Clusters::BallastConfiguration::Id: {
        return MTRAttributeIsSpecifiedInBallastConfigurationCluster(aAttributeId);
    }
    case Clusters::IlluminanceMeasurement::Id: {
        return MTRAttributeIsSpecifiedInIlluminanceMeasurementCluster(aAttributeId);
    }
    case Clusters::TemperatureMeasurement::Id: {
        return MTRAttributeIsSpecifiedInTemperatureMeasurementCluster(aAttributeId);
    }
    case Clusters::PressureMeasurement::Id: {
        return MTRAttributeIsSpecifiedInPressureMeasurementCluster(aAttributeId);
    }
    case Clusters::FlowMeasurement::Id: {
        return MTRAttributeIsSpecifiedInFlowMeasurementCluster(aAttributeId);
    }
    case Clusters::RelativeHumidityMeasurement::Id: {
        return MTRAttributeIsSpecifiedInRelativeHumidityMeasurementCluster(aAttributeId);
    }
    case Clusters::OccupancySensing::Id: {
        return MTRAttributeIsSpecifiedInOccupancySensingCluster(aAttributeId);
    }
    case Clusters::WakeOnLan::Id: {
        return MTRAttributeIsSpecifiedInWakeOnLANCluster(aAttributeId);
    }
    case Clusters::Channel::Id: {
        return MTRAttributeIsSpecifiedInChannelCluster(aAttributeId);
    }
    case Clusters::TargetNavigator::Id: {
        return MTRAttributeIsSpecifiedInTargetNavigatorCluster(aAttributeId);
    }
    case Clusters::MediaPlayback::Id: {
        return MTRAttributeIsSpecifiedInMediaPlaybackCluster(aAttributeId);
    }
    case Clusters::MediaInput::Id: {
        return MTRAttributeIsSpecifiedInMediaInputCluster(aAttributeId);
    }
    case Clusters::LowPower::Id: {
        return MTRAttributeIsSpecifiedInLowPowerCluster(aAttributeId);
    }
    case Clusters::KeypadInput::Id: {
        return MTRAttributeIsSpecifiedInKeypadInputCluster(aAttributeId);
    }
    case Clusters::ContentLauncher::Id: {
        return MTRAttributeIsSpecifiedInContentLauncherCluster(aAttributeId);
    }
    case Clusters::AudioOutput::Id: {
        return MTRAttributeIsSpecifiedInAudioOutputCluster(aAttributeId);
    }
    case Clusters::ApplicationLauncher::Id: {
        return MTRAttributeIsSpecifiedInApplicationLauncherCluster(aAttributeId);
    }
    case Clusters::ApplicationBasic::Id: {
        return MTRAttributeIsSpecifiedInApplicationBasicCluster(aAttributeId);
    }
    case Clusters::AccountLogin::Id: {
        return MTRAttributeIsSpecifiedInAccountLoginCluster(aAttributeId);
    }
    case Clusters::ElectricalMeasurement::Id: {
        return MTRAttributeIsSpecifiedInElectricalMeasurementCluster(aAttributeId);
    }
    case Clusters::UnitTesting::Id: {
        return MTRAttributeIsSpecifiedInUnitTestingCluster(aAttributeId);
    }
    default: {
        return NO;
    }
    }
}
