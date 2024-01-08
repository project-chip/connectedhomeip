/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

// This file is generated automatically. Don't try to change something here.
// To add support for new clusters, modify addon-helper.js in uic/applications/matter_bridge/src/zap

// Component include
#include <matter_device_translator.hpp>

// Standard library
#include <optional>
#include <string>
#include <unordered_map>

// Matter includes
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>

// Cluster mapping Matter
namespace chip {
namespace app {
    namespace Clusters {

        static const std::unordered_map<std::string, chip::ClusterId> matter_cluster_id_map {
            { "Identify", Identify::Id },
            { "Groups", Groups::Id },
            { "Scenes", Scenes::Id },
            { "OnOff", OnOff::Id },
            { "OnOffSwitchConfiguration", OnOffSwitchConfiguration::Id },
            { "LevelControl", LevelControl::Id },
            { "BinaryInputBasic", BinaryInputBasic::Id },
            { "PulseWidthModulation", PulseWidthModulation::Id },
            { "Descriptor", Descriptor::Id },
            { "Binding", Binding::Id },
            { "AccessControl", AccessControl::Id },
            { "Actions", Actions::Id },
            { "BasicInformation", BasicInformation::Id },
            { "OtaSoftwareUpdateProvider", OtaSoftwareUpdateProvider::Id },
            { "OtaSoftwareUpdateRequestor", OtaSoftwareUpdateRequestor::Id },
            { "LocalizationConfiguration", LocalizationConfiguration::Id },
            { "TimeFormatLocalization", TimeFormatLocalization::Id },
            { "UnitLocalization", UnitLocalization::Id },
            { "PowerSourceConfiguration", PowerSourceConfiguration::Id },
            { "PowerSource", PowerSource::Id },
            { "GeneralCommissioning", GeneralCommissioning::Id },
            { "NetworkCommissioning", NetworkCommissioning::Id },
            { "DiagnosticLogs", DiagnosticLogs::Id },
            { "GeneralDiagnostics", GeneralDiagnostics::Id },
            { "SoftwareDiagnostics", SoftwareDiagnostics::Id },
            { "ThreadNetworkDiagnostics", ThreadNetworkDiagnostics::Id },
            { "WiFiNetworkDiagnostics", WiFiNetworkDiagnostics::Id },
            { "EthernetNetworkDiagnostics", EthernetNetworkDiagnostics::Id },
            { "TimeSynchronization", TimeSynchronization::Id },
            { "BridgedDeviceBasicInformation", BridgedDeviceBasicInformation::Id },
            { "Switch", Switch::Id },
            { "AdministratorCommissioning", AdministratorCommissioning::Id },
            { "OperationalCredentials", OperationalCredentials::Id },
            { "GroupKeyManagement", GroupKeyManagement::Id },
            { "FixedLabel", FixedLabel::Id },
            { "UserLabel", UserLabel::Id },
            { "ProxyConfiguration", ProxyConfiguration::Id },
            { "ProxyDiscovery", ProxyDiscovery::Id },
            { "ProxyValid", ProxyValid::Id },
            { "BooleanState", BooleanState::Id },
            { "IcdManagement", IcdManagement::Id },
            { "Timer", Timer::Id },
            { "OvenCavityOperationalState", OvenCavityOperationalState::Id },
            { "OvenMode", OvenMode::Id },
            { "LaundryDryerControls", LaundryDryerControls::Id },
            { "ModeSelect", ModeSelect::Id },
            { "LaundryWasherMode", LaundryWasherMode::Id },
            { "RefrigeratorAndTemperatureControlledCabinetMode", RefrigeratorAndTemperatureControlledCabinetMode::Id },
            { "LaundryWasherControls", LaundryWasherControls::Id },
            { "RvcRunMode", RvcRunMode::Id },
            { "RvcCleanMode", RvcCleanMode::Id },
            { "TemperatureControl", TemperatureControl::Id },
            { "RefrigeratorAlarm", RefrigeratorAlarm::Id },
            { "DishwasherMode", DishwasherMode::Id },
            { "AirQuality", AirQuality::Id },
            { "SmokeCoAlarm", SmokeCoAlarm::Id },
            { "DishwasherAlarm", DishwasherAlarm::Id },
            { "MicrowaveOvenMode", MicrowaveOvenMode::Id },
            { "MicrowaveOvenControl", MicrowaveOvenControl::Id },
            { "OperationalState", OperationalState::Id },
            { "RvcOperationalState", RvcOperationalState::Id },
            { "HepaFilterMonitoring", HepaFilterMonitoring::Id },
            { "ActivatedCarbonFilterMonitoring", ActivatedCarbonFilterMonitoring::Id },
            { "BooleanStateConfiguration", BooleanStateConfiguration::Id },
            { "ValveConfigurationAndControl", ValveConfigurationAndControl::Id },
            { "ElectricalEnergyMeasurement", ElectricalEnergyMeasurement::Id },
            { "DemandResponseLoadControl", DemandResponseLoadControl::Id },
            { "DeviceEnergyManagement", DeviceEnergyManagement::Id },
            { "EnergyEvse", EnergyEvse::Id },
            { "EnergyPreference", EnergyPreference::Id },
            { "DoorLock", DoorLock::Id },
            { "WindowCovering", WindowCovering::Id },
            { "BarrierControl", BarrierControl::Id },
            { "PumpConfigurationAndControl", PumpConfigurationAndControl::Id },
            { "Thermostat", Thermostat::Id },
            { "FanControl", FanControl::Id },
            { "ThermostatUserInterfaceConfiguration", ThermostatUserInterfaceConfiguration::Id },
            { "ColorControl", ColorControl::Id },
            { "BallastConfiguration", BallastConfiguration::Id },
            { "IlluminanceMeasurement", IlluminanceMeasurement::Id },
            { "TemperatureMeasurement", TemperatureMeasurement::Id },
            { "PressureMeasurement", PressureMeasurement::Id },
            { "FlowMeasurement", FlowMeasurement::Id },
            { "RelativeHumidityMeasurement", RelativeHumidityMeasurement::Id },
            { "OccupancySensing", OccupancySensing::Id },
            { "CarbonMonoxideConcentrationMeasurement", CarbonMonoxideConcentrationMeasurement::Id },
            { "CarbonDioxideConcentrationMeasurement", CarbonDioxideConcentrationMeasurement::Id },
            { "NitrogenDioxideConcentrationMeasurement", NitrogenDioxideConcentrationMeasurement::Id },
            { "OzoneConcentrationMeasurement", OzoneConcentrationMeasurement::Id },
            { "Pm25ConcentrationMeasurement", Pm25ConcentrationMeasurement::Id },
            { "FormaldehydeConcentrationMeasurement", FormaldehydeConcentrationMeasurement::Id },
            { "Pm1ConcentrationMeasurement", Pm1ConcentrationMeasurement::Id },
            { "Pm10ConcentrationMeasurement", Pm10ConcentrationMeasurement::Id },
            { "TotalVolatileOrganicCompoundsConcentrationMeasurement", TotalVolatileOrganicCompoundsConcentrationMeasurement::Id },
            { "RadonConcentrationMeasurement", RadonConcentrationMeasurement::Id },
            { "WakeOnLan", WakeOnLan::Id },
            { "Channel", Channel::Id },
            { "TargetNavigator", TargetNavigator::Id },
            { "MediaPlayback", MediaPlayback::Id },
            { "MediaInput", MediaInput::Id },
            { "LowPower", LowPower::Id },
            { "KeypadInput", KeypadInput::Id },
            { "ContentLauncher", ContentLauncher::Id },
            { "AudioOutput", AudioOutput::Id },
            { "ApplicationLauncher", ApplicationLauncher::Id },
            { "ApplicationBasic", ApplicationBasic::Id },
            { "AccountLogin", AccountLogin::Id },
            { "ContentControl", ContentControl::Id },
            { "ContentAppObserver", ContentAppObserver::Id },
            { "ElectricalMeasurement", ElectricalMeasurement::Id },
            { "UnitTesting", UnitTesting::Id },
            { "FaultInjection", FaultInjection::Id },
            { "SampleMei", SampleMei::Id }
        };

        // Attribute mapping Matter
        // Attribute structure for each cluster
        namespace Identify {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "IdentifyTime", IdentifyTime::Id },
                    { "IdentifyType", IdentifyType::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace Identify
        namespace Groups {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "NameSupport", NameSupport::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace Groups
        namespace Scenes {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "SceneCount", SceneCount::Id },
                    { "CurrentScene", CurrentScene::Id },
                    { "CurrentGroup", CurrentGroup::Id },
                    { "SceneValid", SceneValid::Id },
                    { "NameSupport", NameSupport::Id },
                    { "LastConfiguredBy", LastConfiguredBy::Id },
                    { "SceneTableSize", SceneTableSize::Id },
                    { "FabricSceneInfo", FabricSceneInfo::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace Scenes
        namespace OnOff {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "OnOff", OnOff::Id },
                    { "GlobalSceneControl", GlobalSceneControl::Id },
                    { "OnTime", OnTime::Id },
                    { "OffWaitTime", OffWaitTime::Id },
                    { "StartUpOnOff", StartUpOnOff::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace OnOff
        namespace OnOffSwitchConfiguration {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "SwitchType", SwitchType::Id },
                    { "SwitchActions", SwitchActions::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace OnOffSwitchConfiguration
        namespace LevelControl {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "CurrentLevel", CurrentLevel::Id },
                    { "RemainingTime", RemainingTime::Id },
                    { "MinLevel", MinLevel::Id },
                    { "MaxLevel", MaxLevel::Id },
                    { "CurrentFrequency", CurrentFrequency::Id },
                    { "MinFrequency", MinFrequency::Id },
                    { "MaxFrequency", MaxFrequency::Id },
                    { "Options", Options::Id },
                    { "OnOffTransitionTime", OnOffTransitionTime::Id },
                    { "OnLevel", OnLevel::Id },
                    { "OnTransitionTime", OnTransitionTime::Id },
                    { "OffTransitionTime", OffTransitionTime::Id },
                    { "DefaultMoveRate", DefaultMoveRate::Id },
                    { "StartUpCurrentLevel", StartUpCurrentLevel::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace LevelControl
        namespace BinaryInputBasic {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "ActiveText", ActiveText::Id },
                    { "Description", Description::Id },
                    { "InactiveText", InactiveText::Id },
                    { "OutOfService", OutOfService::Id },
                    { "Polarity", Polarity::Id },
                    { "PresentValue", PresentValue::Id },
                    { "Reliability", Reliability::Id },
                    { "StatusFlags", StatusFlags::Id },
                    { "ApplicationType", ApplicationType::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace BinaryInputBasic
        namespace PulseWidthModulation {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace PulseWidthModulation
        namespace Descriptor {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "DeviceTypeList", DeviceTypeList::Id },
                    { "ServerList", ServerList::Id },
                    { "ClientList", ClientList::Id },
                    { "PartsList", PartsList::Id },
                    { "TagList", TagList::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace Descriptor
        namespace Binding {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Binding", Binding::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace Binding
        namespace AccessControl {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Acl", Acl::Id },
                    { "Extension", Extension::Id },
                    { "SubjectsPerAccessControlEntry", SubjectsPerAccessControlEntry::Id },
                    { "TargetsPerAccessControlEntry", TargetsPerAccessControlEntry::Id },
                    { "AccessControlEntriesPerFabric", AccessControlEntriesPerFabric::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace AccessControl
        namespace Actions {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "ActionList", ActionList::Id },
                    { "EndpointLists", EndpointLists::Id },
                    { "SetupURL", SetupURL::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace Actions
        namespace BasicInformation {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "DataModelRevision", DataModelRevision::Id },
                    { "VendorName", VendorName::Id },
                    { "VendorID", VendorID::Id },
                    { "ProductName", ProductName::Id },
                    { "ProductID", ProductID::Id },
                    { "NodeLabel", NodeLabel::Id },
                    { "Location", Location::Id },
                    { "HardwareVersion", HardwareVersion::Id },
                    { "HardwareVersionString", HardwareVersionString::Id },
                    { "SoftwareVersion", SoftwareVersion::Id },
                    { "SoftwareVersionString", SoftwareVersionString::Id },
                    { "ManufacturingDate", ManufacturingDate::Id },
                    { "PartNumber", PartNumber::Id },
                    { "ProductURL", ProductURL::Id },
                    { "ProductLabel", ProductLabel::Id },
                    { "SerialNumber", SerialNumber::Id },
                    { "LocalConfigDisabled", LocalConfigDisabled::Id },
                    { "Reachable", Reachable::Id },
                    { "UniqueID", UniqueID::Id },
                    { "CapabilityMinima", CapabilityMinima::Id },
                    { "ProductAppearance", ProductAppearance::Id },
                    { "SpecificationVersion", SpecificationVersion::Id },
                    { "MaxPathsPerInvoke", MaxPathsPerInvoke::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace BasicInformation
        namespace OtaSoftwareUpdateProvider {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace OtaSoftwareUpdateProvider
        namespace OtaSoftwareUpdateRequestor {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "DefaultOTAProviders", DefaultOTAProviders::Id },
                    { "UpdatePossible", UpdatePossible::Id },
                    { "UpdateState", UpdateState::Id },
                    { "UpdateStateProgress", UpdateStateProgress::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace OtaSoftwareUpdateRequestor
        namespace LocalizationConfiguration {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "ActiveLocale", ActiveLocale::Id },
                    { "SupportedLocales", SupportedLocales::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace LocalizationConfiguration
        namespace TimeFormatLocalization {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "HourFormat", HourFormat::Id },
                    { "ActiveCalendarType", ActiveCalendarType::Id },
                    { "SupportedCalendarTypes", SupportedCalendarTypes::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace TimeFormatLocalization
        namespace UnitLocalization {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "TemperatureUnit", TemperatureUnit::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace UnitLocalization
        namespace PowerSourceConfiguration {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Sources", Sources::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace PowerSourceConfiguration
        namespace PowerSource {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Status", Status::Id },
                    { "Order", Order::Id },
                    { "Description", Description::Id },
                    { "WiredAssessedInputVoltage", WiredAssessedInputVoltage::Id },
                    { "WiredAssessedInputFrequency", WiredAssessedInputFrequency::Id },
                    { "WiredCurrentType", WiredCurrentType::Id },
                    { "WiredAssessedCurrent", WiredAssessedCurrent::Id },
                    { "WiredNominalVoltage", WiredNominalVoltage::Id },
                    { "WiredMaximumCurrent", WiredMaximumCurrent::Id },
                    { "WiredPresent", WiredPresent::Id },
                    { "ActiveWiredFaults", ActiveWiredFaults::Id },
                    { "BatVoltage", BatVoltage::Id },
                    { "BatPercentRemaining", BatPercentRemaining::Id },
                    { "BatTimeRemaining", BatTimeRemaining::Id },
                    { "BatChargeLevel", BatChargeLevel::Id },
                    { "BatReplacementNeeded", BatReplacementNeeded::Id },
                    { "BatReplaceability", BatReplaceability::Id },
                    { "BatPresent", BatPresent::Id },
                    { "ActiveBatFaults", ActiveBatFaults::Id },
                    { "BatReplacementDescription", BatReplacementDescription::Id },
                    { "BatCommonDesignation", BatCommonDesignation::Id },
                    { "BatANSIDesignation", BatANSIDesignation::Id },
                    { "BatIECDesignation", BatIECDesignation::Id },
                    { "BatApprovedChemistry", BatApprovedChemistry::Id },
                    { "BatCapacity", BatCapacity::Id },
                    { "BatQuantity", BatQuantity::Id },
                    { "BatChargeState", BatChargeState::Id },
                    { "BatTimeToFullCharge", BatTimeToFullCharge::Id },
                    { "BatFunctionalWhileCharging", BatFunctionalWhileCharging::Id },
                    { "BatChargingCurrent", BatChargingCurrent::Id },
                    { "ActiveBatChargeFaults", ActiveBatChargeFaults::Id },
                    { "EndpointList", EndpointList::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace PowerSource
        namespace GeneralCommissioning {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Breadcrumb", Breadcrumb::Id },
                    { "BasicCommissioningInfo", BasicCommissioningInfo::Id },
                    { "RegulatoryConfig", RegulatoryConfig::Id },
                    { "LocationCapability", LocationCapability::Id },
                    { "SupportsConcurrentConnection", SupportsConcurrentConnection::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace GeneralCommissioning
        namespace NetworkCommissioning {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MaxNetworks", MaxNetworks::Id },
                    { "Networks", Networks::Id },
                    { "ScanMaxTimeSeconds", ScanMaxTimeSeconds::Id },
                    { "ConnectMaxTimeSeconds", ConnectMaxTimeSeconds::Id },
                    { "InterfaceEnabled", InterfaceEnabled::Id },
                    { "LastNetworkingStatus", LastNetworkingStatus::Id },
                    { "LastNetworkID", LastNetworkID::Id },
                    { "LastConnectErrorValue", LastConnectErrorValue::Id },
                    { "SupportedWiFiBands", SupportedWiFiBands::Id },
                    { "SupportedThreadFeatures", SupportedThreadFeatures::Id },
                    { "ThreadVersion", ThreadVersion::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace NetworkCommissioning
        namespace DiagnosticLogs {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace DiagnosticLogs
        namespace GeneralDiagnostics {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "NetworkInterfaces", NetworkInterfaces::Id },
                    { "RebootCount", RebootCount::Id },
                    { "UpTime", UpTime::Id },
                    { "TotalOperationalHours", TotalOperationalHours::Id },
                    { "BootReason", BootReason::Id },
                    { "ActiveHardwareFaults", ActiveHardwareFaults::Id },
                    { "ActiveRadioFaults", ActiveRadioFaults::Id },
                    { "ActiveNetworkFaults", ActiveNetworkFaults::Id },
                    { "TestEventTriggersEnabled", TestEventTriggersEnabled::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace GeneralDiagnostics
        namespace SoftwareDiagnostics {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "ThreadMetrics", ThreadMetrics::Id },
                    { "CurrentHeapFree", CurrentHeapFree::Id },
                    { "CurrentHeapUsed", CurrentHeapUsed::Id },
                    { "CurrentHeapHighWatermark", CurrentHeapHighWatermark::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace SoftwareDiagnostics
        namespace ThreadNetworkDiagnostics {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Channel", Channel::Id },
                    { "RoutingRole", RoutingRole::Id },
                    { "NetworkName", NetworkName::Id },
                    { "PanId", PanId::Id },
                    { "ExtendedPanId", ExtendedPanId::Id },
                    { "MeshLocalPrefix", MeshLocalPrefix::Id },
                    { "OverrunCount", OverrunCount::Id },
                    { "NeighborTable", NeighborTable::Id },
                    { "RouteTable", RouteTable::Id },
                    { "PartitionId", PartitionId::Id },
                    { "Weighting", Weighting::Id },
                    { "DataVersion", DataVersion::Id },
                    { "StableDataVersion", StableDataVersion::Id },
                    { "LeaderRouterId", LeaderRouterId::Id },
                    { "DetachedRoleCount", DetachedRoleCount::Id },
                    { "ChildRoleCount", ChildRoleCount::Id },
                    { "RouterRoleCount", RouterRoleCount::Id },
                    { "LeaderRoleCount", LeaderRoleCount::Id },
                    { "AttachAttemptCount", AttachAttemptCount::Id },
                    { "PartitionIdChangeCount", PartitionIdChangeCount::Id },
                    { "BetterPartitionAttachAttemptCount", BetterPartitionAttachAttemptCount::Id },
                    { "ParentChangeCount", ParentChangeCount::Id },
                    { "TxTotalCount", TxTotalCount::Id },
                    { "TxUnicastCount", TxUnicastCount::Id },
                    { "TxBroadcastCount", TxBroadcastCount::Id },
                    { "TxAckRequestedCount", TxAckRequestedCount::Id },
                    { "TxAckedCount", TxAckedCount::Id },
                    { "TxNoAckRequestedCount", TxNoAckRequestedCount::Id },
                    { "TxDataCount", TxDataCount::Id },
                    { "TxDataPollCount", TxDataPollCount::Id },
                    { "TxBeaconCount", TxBeaconCount::Id },
                    { "TxBeaconRequestCount", TxBeaconRequestCount::Id },
                    { "TxOtherCount", TxOtherCount::Id },
                    { "TxRetryCount", TxRetryCount::Id },
                    { "TxDirectMaxRetryExpiryCount", TxDirectMaxRetryExpiryCount::Id },
                    { "TxIndirectMaxRetryExpiryCount", TxIndirectMaxRetryExpiryCount::Id },
                    { "TxErrCcaCount", TxErrCcaCount::Id },
                    { "TxErrAbortCount", TxErrAbortCount::Id },
                    { "TxErrBusyChannelCount", TxErrBusyChannelCount::Id },
                    { "RxTotalCount", RxTotalCount::Id },
                    { "RxUnicastCount", RxUnicastCount::Id },
                    { "RxBroadcastCount", RxBroadcastCount::Id },
                    { "RxDataCount", RxDataCount::Id },
                    { "RxDataPollCount", RxDataPollCount::Id },
                    { "RxBeaconCount", RxBeaconCount::Id },
                    { "RxBeaconRequestCount", RxBeaconRequestCount::Id },
                    { "RxOtherCount", RxOtherCount::Id },
                    { "RxAddressFilteredCount", RxAddressFilteredCount::Id },
                    { "RxDestAddrFilteredCount", RxDestAddrFilteredCount::Id },
                    { "RxDuplicatedCount", RxDuplicatedCount::Id },
                    { "RxErrNoFrameCount", RxErrNoFrameCount::Id },
                    { "RxErrUnknownNeighborCount", RxErrUnknownNeighborCount::Id },
                    { "RxErrInvalidSrcAddrCount", RxErrInvalidSrcAddrCount::Id },
                    { "RxErrSecCount", RxErrSecCount::Id },
                    { "RxErrFcsCount", RxErrFcsCount::Id },
                    { "RxErrOtherCount", RxErrOtherCount::Id },
                    { "ActiveTimestamp", ActiveTimestamp::Id },
                    { "PendingTimestamp", PendingTimestamp::Id },
                    { "Delay", Delay::Id },
                    { "SecurityPolicy", SecurityPolicy::Id },
                    { "ChannelPage0Mask", ChannelPage0Mask::Id },
                    { "OperationalDatasetComponents", OperationalDatasetComponents::Id },
                    { "ActiveNetworkFaultsList", ActiveNetworkFaultsList::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ThreadNetworkDiagnostics
        namespace WiFiNetworkDiagnostics {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Bssid", Bssid::Id },
                    { "SecurityType", SecurityType::Id },
                    { "WiFiVersion", WiFiVersion::Id },
                    { "ChannelNumber", ChannelNumber::Id },
                    { "Rssi", Rssi::Id },
                    { "BeaconLostCount", BeaconLostCount::Id },
                    { "BeaconRxCount", BeaconRxCount::Id },
                    { "PacketMulticastRxCount", PacketMulticastRxCount::Id },
                    { "PacketMulticastTxCount", PacketMulticastTxCount::Id },
                    { "PacketUnicastRxCount", PacketUnicastRxCount::Id },
                    { "PacketUnicastTxCount", PacketUnicastTxCount::Id },
                    { "CurrentMaxRate", CurrentMaxRate::Id },
                    { "OverrunCount", OverrunCount::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace WiFiNetworkDiagnostics
        namespace EthernetNetworkDiagnostics {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "PHYRate", PHYRate::Id },
                    { "FullDuplex", FullDuplex::Id },
                    { "PacketRxCount", PacketRxCount::Id },
                    { "PacketTxCount", PacketTxCount::Id },
                    { "TxErrCount", TxErrCount::Id },
                    { "CollisionCount", CollisionCount::Id },
                    { "OverrunCount", OverrunCount::Id },
                    { "CarrierDetect", CarrierDetect::Id },
                    { "TimeSinceReset", TimeSinceReset::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace EthernetNetworkDiagnostics
        namespace TimeSynchronization {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "UTCTime", UTCTime::Id },
                    { "Granularity", Granularity::Id },
                    { "TimeSource", TimeSource::Id },
                    { "TrustedTimeSource", TrustedTimeSource::Id },
                    { "DefaultNTP", DefaultNTP::Id },
                    { "TimeZone", TimeZone::Id },
                    { "DSTOffset", DSTOffset::Id },
                    { "LocalTime", LocalTime::Id },
                    { "TimeZoneDatabase", TimeZoneDatabase::Id },
                    { "NTPServerAvailable", NTPServerAvailable::Id },
                    { "TimeZoneListMaxSize", TimeZoneListMaxSize::Id },
                    { "DSTOffsetListMaxSize", DSTOffsetListMaxSize::Id },
                    { "SupportsDNSResolve", SupportsDNSResolve::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace TimeSynchronization
        namespace BridgedDeviceBasicInformation {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "VendorName", VendorName::Id },
                    { "VendorID", VendorID::Id },
                    { "ProductName", ProductName::Id },
                    { "NodeLabel", NodeLabel::Id },
                    { "HardwareVersion", HardwareVersion::Id },
                    { "HardwareVersionString", HardwareVersionString::Id },
                    { "SoftwareVersion", SoftwareVersion::Id },
                    { "SoftwareVersionString", SoftwareVersionString::Id },
                    { "ManufacturingDate", ManufacturingDate::Id },
                    { "PartNumber", PartNumber::Id },
                    { "ProductURL", ProductURL::Id },
                    { "ProductLabel", ProductLabel::Id },
                    { "SerialNumber", SerialNumber::Id },
                    { "Reachable", Reachable::Id },
                    { "UniqueID", UniqueID::Id },
                    { "ProductAppearance", ProductAppearance::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace BridgedDeviceBasicInformation
        namespace Switch {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "NumberOfPositions", NumberOfPositions::Id },
                    { "CurrentPosition", CurrentPosition::Id },
                    { "MultiPressMax", MultiPressMax::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace Switch
        namespace AdministratorCommissioning {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "WindowStatus", WindowStatus::Id },
                    { "AdminFabricIndex", AdminFabricIndex::Id },
                    { "AdminVendorId", AdminVendorId::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace AdministratorCommissioning
        namespace OperationalCredentials {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "NOCs", NOCs::Id },
                    { "Fabrics", Fabrics::Id },
                    { "SupportedFabrics", SupportedFabrics::Id },
                    { "CommissionedFabrics", CommissionedFabrics::Id },
                    { "TrustedRootCertificates", TrustedRootCertificates::Id },
                    { "CurrentFabricIndex", CurrentFabricIndex::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace OperationalCredentials
        namespace GroupKeyManagement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "GroupKeyMap", GroupKeyMap::Id },
                    { "GroupTable", GroupTable::Id },
                    { "MaxGroupsPerFabric", MaxGroupsPerFabric::Id },
                    { "MaxGroupKeysPerFabric", MaxGroupKeysPerFabric::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace GroupKeyManagement
        namespace FixedLabel {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "LabelList", LabelList::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace FixedLabel
        namespace UserLabel {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "LabelList", LabelList::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace UserLabel
        namespace ProxyConfiguration {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ProxyConfiguration
        namespace ProxyDiscovery {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ProxyDiscovery
        namespace ProxyValid {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ProxyValid
        namespace BooleanState {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "StateValue", StateValue::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace BooleanState
        namespace IcdManagement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "IdleModeDuration", IdleModeDuration::Id },
                    { "ActiveModeDuration", ActiveModeDuration::Id },
                    { "ActiveModeThreshold", ActiveModeThreshold::Id },
                    { "RegisteredClients", RegisteredClients::Id },
                    { "ICDCounter", ICDCounter::Id },
                    { "ClientsSupportedPerFabric", ClientsSupportedPerFabric::Id },
                    { "UserActiveModeTriggerHint", UserActiveModeTriggerHint::Id },
                    { "UserActiveModeTriggerInstruction", UserActiveModeTriggerInstruction::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace IcdManagement
        namespace Timer {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "SetTime", SetTime::Id },
                    { "TimeRemaining", TimeRemaining::Id },
                    { "TimerState", TimerState::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace Timer
        namespace OvenCavityOperationalState {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "PhaseList", PhaseList::Id },
                    { "CurrentPhase", CurrentPhase::Id },
                    { "CountdownTime", CountdownTime::Id },
                    { "OperationalStateList", OperationalStateList::Id },
                    { "OperationalState", OperationalState::Id },
                    { "OperationalError", OperationalError::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace OvenCavityOperationalState
        namespace OvenMode {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "SupportedModes", SupportedModes::Id },
                    { "CurrentMode", CurrentMode::Id },
                    { "StartUpMode", StartUpMode::Id },
                    { "OnMode", OnMode::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace OvenMode
        namespace LaundryDryerControls {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "SupportedDrynessLevels", SupportedDrynessLevels::Id },
                    { "SelectedDrynessLevel", SelectedDrynessLevel::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace LaundryDryerControls
        namespace ModeSelect {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Description", Description::Id },
                    { "StandardNamespace", StandardNamespace::Id },
                    { "SupportedModes", SupportedModes::Id },
                    { "CurrentMode", CurrentMode::Id },
                    { "StartUpMode", StartUpMode::Id },
                    { "OnMode", OnMode::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ModeSelect
        namespace LaundryWasherMode {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "SupportedModes", SupportedModes::Id },
                    { "CurrentMode", CurrentMode::Id },
                    { "StartUpMode", StartUpMode::Id },
                    { "OnMode", OnMode::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace LaundryWasherMode
        namespace RefrigeratorAndTemperatureControlledCabinetMode {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "SupportedModes", SupportedModes::Id },
                    { "CurrentMode", CurrentMode::Id },
                    { "StartUpMode", StartUpMode::Id },
                    { "OnMode", OnMode::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace RefrigeratorAndTemperatureControlledCabinetMode
        namespace LaundryWasherControls {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "SpinSpeeds", SpinSpeeds::Id },
                    { "SpinSpeedCurrent", SpinSpeedCurrent::Id },
                    { "NumberOfRinses", NumberOfRinses::Id },
                    { "SupportedRinses", SupportedRinses::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace LaundryWasherControls
        namespace RvcRunMode {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "SupportedModes", SupportedModes::Id },
                    { "CurrentMode", CurrentMode::Id },
                    { "OnMode", OnMode::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace RvcRunMode
        namespace RvcCleanMode {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "SupportedModes", SupportedModes::Id },
                    { "CurrentMode", CurrentMode::Id },
                    { "OnMode", OnMode::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace RvcCleanMode
        namespace TemperatureControl {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "TemperatureSetpoint", TemperatureSetpoint::Id },
                    { "MinTemperature", MinTemperature::Id },
                    { "MaxTemperature", MaxTemperature::Id },
                    { "Step", Step::Id },
                    { "SelectedTemperatureLevel", SelectedTemperatureLevel::Id },
                    { "SupportedTemperatureLevels", SupportedTemperatureLevels::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace TemperatureControl
        namespace RefrigeratorAlarm {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Mask", Mask::Id },
                    { "State", State::Id },
                    { "Supported", Supported::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace RefrigeratorAlarm
        namespace DishwasherMode {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "SupportedModes", SupportedModes::Id },
                    { "CurrentMode", CurrentMode::Id },
                    { "StartUpMode", StartUpMode::Id },
                    { "OnMode", OnMode::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace DishwasherMode
        namespace AirQuality {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "AirQuality", AirQuality::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace AirQuality
        namespace SmokeCoAlarm {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "ExpressedState", ExpressedState::Id },
                    { "SmokeState", SmokeState::Id },
                    { "COState", COState::Id },
                    { "BatteryAlert", BatteryAlert::Id },
                    { "DeviceMuted", DeviceMuted::Id },
                    { "TestInProgress", TestInProgress::Id },
                    { "HardwareFaultAlert", HardwareFaultAlert::Id },
                    { "EndOfServiceAlert", EndOfServiceAlert::Id },
                    { "InterconnectSmokeAlarm", InterconnectSmokeAlarm::Id },
                    { "InterconnectCOAlarm", InterconnectCOAlarm::Id },
                    { "ContaminationState", ContaminationState::Id },
                    { "SmokeSensitivityLevel", SmokeSensitivityLevel::Id },
                    { "ExpiryDate", ExpiryDate::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace SmokeCoAlarm
        namespace DishwasherAlarm {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Mask", Mask::Id },
                    { "Latch", Latch::Id },
                    { "State", State::Id },
                    { "Supported", Supported::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace DishwasherAlarm
        namespace MicrowaveOvenMode {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "SupportedModes", SupportedModes::Id },
                    { "CurrentMode", CurrentMode::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace MicrowaveOvenMode
        namespace MicrowaveOvenControl {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "CookTime", CookTime::Id },
                    { "PowerSetting", PowerSetting::Id },
                    { "MinPower", MinPower::Id },
                    { "MaxPower", MaxPower::Id },
                    { "PowerStep", PowerStep::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace MicrowaveOvenControl
        namespace OperationalState {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "PhaseList", PhaseList::Id },
                    { "CurrentPhase", CurrentPhase::Id },
                    { "CountdownTime", CountdownTime::Id },
                    { "OperationalStateList", OperationalStateList::Id },
                    { "OperationalState", OperationalState::Id },
                    { "OperationalError", OperationalError::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace OperationalState
        namespace RvcOperationalState {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "PhaseList", PhaseList::Id },
                    { "CurrentPhase", CurrentPhase::Id },
                    { "CountdownTime", CountdownTime::Id },
                    { "OperationalStateList", OperationalStateList::Id },
                    { "OperationalState", OperationalState::Id },
                    { "OperationalError", OperationalError::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace RvcOperationalState
        namespace HepaFilterMonitoring {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Condition", Condition::Id },
                    { "DegradationDirection", DegradationDirection::Id },
                    { "ChangeIndication", ChangeIndication::Id },
                    { "InPlaceIndicator", InPlaceIndicator::Id },
                    { "LastChangedTime", LastChangedTime::Id },
                    { "ReplacementProductList", ReplacementProductList::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace HepaFilterMonitoring
        namespace ActivatedCarbonFilterMonitoring {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Condition", Condition::Id },
                    { "DegradationDirection", DegradationDirection::Id },
                    { "ChangeIndication", ChangeIndication::Id },
                    { "InPlaceIndicator", InPlaceIndicator::Id },
                    { "LastChangedTime", LastChangedTime::Id },
                    { "ReplacementProductList", ReplacementProductList::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ActivatedCarbonFilterMonitoring
        namespace BooleanStateConfiguration {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "CurrentSensitivityLevel", CurrentSensitivityLevel::Id },
                    { "SupportedSensitivityLevels", SupportedSensitivityLevels::Id },
                    { "DefaultSensitivityLevel", DefaultSensitivityLevel::Id },
                    { "AlarmsActive", AlarmsActive::Id },
                    { "AlarmsSuppressed", AlarmsSuppressed::Id },
                    { "AlarmsEnabled", AlarmsEnabled::Id },
                    { "AlarmsSupported", AlarmsSupported::Id },
                    { "SensorFault", SensorFault::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace BooleanStateConfiguration
        namespace ValveConfigurationAndControl {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "OpenDuration", OpenDuration::Id },
                    { "DefaultOpenDuration", DefaultOpenDuration::Id },
                    { "AutoCloseTime", AutoCloseTime::Id },
                    { "RemainingDuration", RemainingDuration::Id },
                    { "CurrentState", CurrentState::Id },
                    { "TargetState", TargetState::Id },
                    { "CurrentLevel", CurrentLevel::Id },
                    { "TargetLevel", TargetLevel::Id },
                    { "DefaultOpenLevel", DefaultOpenLevel::Id },
                    { "ValveFault", ValveFault::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ValveConfigurationAndControl
        namespace ElectricalEnergyMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Accuracy", Accuracy::Id },
                    { "CumulativeEnergyImported", CumulativeEnergyImported::Id },
                    { "CumulativeEnergyExported", CumulativeEnergyExported::Id },
                    { "PeriodicEnergyImported", PeriodicEnergyImported::Id },
                    { "PeriodicEnergyExported", PeriodicEnergyExported::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ElectricalEnergyMeasurement
        namespace DemandResponseLoadControl {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "LoadControlPrograms", LoadControlPrograms::Id },
                    { "NumberOfLoadControlPrograms", NumberOfLoadControlPrograms::Id },
                    { "Events", Events::Id },
                    { "ActiveEvents", ActiveEvents::Id },
                    { "NumberOfEventsPerProgram", NumberOfEventsPerProgram::Id },
                    { "NumberOfTransitions", NumberOfTransitions::Id },
                    { "DefaultRandomStart", DefaultRandomStart::Id },
                    { "DefaultRandomDuration", DefaultRandomDuration::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace DemandResponseLoadControl
        namespace DeviceEnergyManagement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "ESAType", ESAType::Id },
                    { "ESACanGenerate", ESACanGenerate::Id },
                    { "ESAState", ESAState::Id },
                    { "AbsMinPower", AbsMinPower::Id },
                    { "AbsMaxPower", AbsMaxPower::Id },
                    { "PowerAdjustmentCapability", PowerAdjustmentCapability::Id },
                    { "Forecast", Forecast::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace DeviceEnergyManagement
        namespace EnergyEvse {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "State", State::Id },
                    { "SupplyState", SupplyState::Id },
                    { "FaultState", FaultState::Id },
                    { "ChargingEnabledUntil", ChargingEnabledUntil::Id },
                    { "DischargingEnabledUntil", DischargingEnabledUntil::Id },
                    { "CircuitCapacity", CircuitCapacity::Id },
                    { "MinimumChargeCurrent", MinimumChargeCurrent::Id },
                    { "MaximumChargeCurrent", MaximumChargeCurrent::Id },
                    { "MaximumDischargeCurrent", MaximumDischargeCurrent::Id },
                    { "UserMaximumChargeCurrent", UserMaximumChargeCurrent::Id },
                    { "RandomizationDelayWindow", RandomizationDelayWindow::Id },
                    { "NumberOfWeeklyTargets", NumberOfWeeklyTargets::Id },
                    { "NumberOfDailyTargets", NumberOfDailyTargets::Id },
                    { "NextChargeStartTime", NextChargeStartTime::Id },
                    { "NextChargeTargetTime", NextChargeTargetTime::Id },
                    { "NextChargeRequiredEnergy", NextChargeRequiredEnergy::Id },
                    { "NextChargeTargetSoC", NextChargeTargetSoC::Id },
                    { "ApproximateEVEfficiency", ApproximateEVEfficiency::Id },
                    { "StateOfCharge", StateOfCharge::Id },
                    { "BatteryCapacity", BatteryCapacity::Id },
                    { "VehicleID", VehicleID::Id },
                    { "SessionID", SessionID::Id },
                    { "SessionDuration", SessionDuration::Id },
                    { "SessionEnergyCharged", SessionEnergyCharged::Id },
                    { "SessionEnergyDischarged", SessionEnergyDischarged::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace EnergyEvse
        namespace EnergyPreference {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "EnergyBalances", EnergyBalances::Id },
                    { "CurrentEnergyBalance", CurrentEnergyBalance::Id },
                    { "EnergyPriorities", EnergyPriorities::Id },
                    { "LowPowerModeSensitivities", LowPowerModeSensitivities::Id },
                    { "CurrentLowPowerModeSensitivity", CurrentLowPowerModeSensitivity::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace EnergyPreference
        namespace DoorLock {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "LockState", LockState::Id },
                    { "LockType", LockType::Id },
                    { "ActuatorEnabled", ActuatorEnabled::Id },
                    { "DoorState", DoorState::Id },
                    { "DoorOpenEvents", DoorOpenEvents::Id },
                    { "DoorClosedEvents", DoorClosedEvents::Id },
                    { "OpenPeriod", OpenPeriod::Id },
                    { "NumberOfTotalUsersSupported", NumberOfTotalUsersSupported::Id },
                    { "NumberOfPINUsersSupported", NumberOfPINUsersSupported::Id },
                    { "NumberOfRFIDUsersSupported", NumberOfRFIDUsersSupported::Id },
                    { "NumberOfWeekDaySchedulesSupportedPerUser", NumberOfWeekDaySchedulesSupportedPerUser::Id },
                    { "NumberOfYearDaySchedulesSupportedPerUser", NumberOfYearDaySchedulesSupportedPerUser::Id },
                    { "NumberOfHolidaySchedulesSupported", NumberOfHolidaySchedulesSupported::Id },
                    { "MaxPINCodeLength", MaxPINCodeLength::Id },
                    { "MinPINCodeLength", MinPINCodeLength::Id },
                    { "MaxRFIDCodeLength", MaxRFIDCodeLength::Id },
                    { "MinRFIDCodeLength", MinRFIDCodeLength::Id },
                    { "CredentialRulesSupport", CredentialRulesSupport::Id },
                    { "NumberOfCredentialsSupportedPerUser", NumberOfCredentialsSupportedPerUser::Id },
                    { "Language", Language::Id },
                    { "LEDSettings", LEDSettings::Id },
                    { "AutoRelockTime", AutoRelockTime::Id },
                    { "SoundVolume", SoundVolume::Id },
                    { "OperatingMode", OperatingMode::Id },
                    { "SupportedOperatingModes", SupportedOperatingModes::Id },
                    { "DefaultConfigurationRegister", DefaultConfigurationRegister::Id },
                    { "EnableLocalProgramming", EnableLocalProgramming::Id },
                    { "EnableOneTouchLocking", EnableOneTouchLocking::Id },
                    { "EnableInsideStatusLED", EnableInsideStatusLED::Id },
                    { "EnablePrivacyModeButton", EnablePrivacyModeButton::Id },
                    { "LocalProgrammingFeatures", LocalProgrammingFeatures::Id },
                    { "WrongCodeEntryLimit", WrongCodeEntryLimit::Id },
                    { "UserCodeTemporaryDisableTime", UserCodeTemporaryDisableTime::Id },
                    { "SendPINOverTheAir", SendPINOverTheAir::Id },
                    { "RequirePINforRemoteOperation", RequirePINforRemoteOperation::Id },
                    { "ExpiringUserTimeout", ExpiringUserTimeout::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace DoorLock
        namespace WindowCovering {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Type", Type::Id },
                    { "PhysicalClosedLimitLift", PhysicalClosedLimitLift::Id },
                    { "PhysicalClosedLimitTilt", PhysicalClosedLimitTilt::Id },
                    { "CurrentPositionLift", CurrentPositionLift::Id },
                    { "CurrentPositionTilt", CurrentPositionTilt::Id },
                    { "NumberOfActuationsLift", NumberOfActuationsLift::Id },
                    { "NumberOfActuationsTilt", NumberOfActuationsTilt::Id },
                    { "ConfigStatus", ConfigStatus::Id },
                    { "CurrentPositionLiftPercentage", CurrentPositionLiftPercentage::Id },
                    { "CurrentPositionTiltPercentage", CurrentPositionTiltPercentage::Id },
                    { "OperationalStatus", OperationalStatus::Id },
                    { "TargetPositionLiftPercent100ths", TargetPositionLiftPercent100ths::Id },
                    { "TargetPositionTiltPercent100ths", TargetPositionTiltPercent100ths::Id },
                    { "EndProductType", EndProductType::Id },
                    { "CurrentPositionLiftPercent100ths", CurrentPositionLiftPercent100ths::Id },
                    { "CurrentPositionTiltPercent100ths", CurrentPositionTiltPercent100ths::Id },
                    { "InstalledOpenLimitLift", InstalledOpenLimitLift::Id },
                    { "InstalledClosedLimitLift", InstalledClosedLimitLift::Id },
                    { "InstalledOpenLimitTilt", InstalledOpenLimitTilt::Id },
                    { "InstalledClosedLimitTilt", InstalledClosedLimitTilt::Id },
                    { "Mode", Mode::Id },
                    { "SafetyStatus", SafetyStatus::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace WindowCovering
        namespace BarrierControl {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "BarrierMovingState", BarrierMovingState::Id },
                    { "BarrierSafetyStatus", BarrierSafetyStatus::Id },
                    { "BarrierCapabilities", BarrierCapabilities::Id },
                    { "BarrierOpenEvents", BarrierOpenEvents::Id },
                    { "BarrierCloseEvents", BarrierCloseEvents::Id },
                    { "BarrierCommandOpenEvents", BarrierCommandOpenEvents::Id },
                    { "BarrierCommandCloseEvents", BarrierCommandCloseEvents::Id },
                    { "BarrierOpenPeriod", BarrierOpenPeriod::Id },
                    { "BarrierClosePeriod", BarrierClosePeriod::Id },
                    { "BarrierPosition", BarrierPosition::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace BarrierControl
        namespace PumpConfigurationAndControl {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MaxPressure", MaxPressure::Id },
                    { "MaxSpeed", MaxSpeed::Id },
                    { "MaxFlow", MaxFlow::Id },
                    { "MinConstPressure", MinConstPressure::Id },
                    { "MaxConstPressure", MaxConstPressure::Id },
                    { "MinCompPressure", MinCompPressure::Id },
                    { "MaxCompPressure", MaxCompPressure::Id },
                    { "MinConstSpeed", MinConstSpeed::Id },
                    { "MaxConstSpeed", MaxConstSpeed::Id },
                    { "MinConstFlow", MinConstFlow::Id },
                    { "MaxConstFlow", MaxConstFlow::Id },
                    { "MinConstTemp", MinConstTemp::Id },
                    { "MaxConstTemp", MaxConstTemp::Id },
                    { "PumpStatus", PumpStatus::Id },
                    { "EffectiveOperationMode", EffectiveOperationMode::Id },
                    { "EffectiveControlMode", EffectiveControlMode::Id },
                    { "Capacity", Capacity::Id },
                    { "Speed", Speed::Id },
                    { "LifetimeRunningHours", LifetimeRunningHours::Id },
                    { "Power", Power::Id },
                    { "LifetimeEnergyConsumed", LifetimeEnergyConsumed::Id },
                    { "OperationMode", OperationMode::Id },
                    { "ControlMode", ControlMode::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace PumpConfigurationAndControl
        namespace Thermostat {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "LocalTemperature", LocalTemperature::Id },
                    { "OutdoorTemperature", OutdoorTemperature::Id },
                    { "Occupancy", Occupancy::Id },
                    { "AbsMinHeatSetpointLimit", AbsMinHeatSetpointLimit::Id },
                    { "AbsMaxHeatSetpointLimit", AbsMaxHeatSetpointLimit::Id },
                    { "AbsMinCoolSetpointLimit", AbsMinCoolSetpointLimit::Id },
                    { "AbsMaxCoolSetpointLimit", AbsMaxCoolSetpointLimit::Id },
                    { "PICoolingDemand", PICoolingDemand::Id },
                    { "PIHeatingDemand", PIHeatingDemand::Id },
                    { "HVACSystemTypeConfiguration", HVACSystemTypeConfiguration::Id },
                    { "LocalTemperatureCalibration", LocalTemperatureCalibration::Id },
                    { "OccupiedCoolingSetpoint", OccupiedCoolingSetpoint::Id },
                    { "OccupiedHeatingSetpoint", OccupiedHeatingSetpoint::Id },
                    { "UnoccupiedCoolingSetpoint", UnoccupiedCoolingSetpoint::Id },
                    { "UnoccupiedHeatingSetpoint", UnoccupiedHeatingSetpoint::Id },
                    { "MinHeatSetpointLimit", MinHeatSetpointLimit::Id },
                    { "MaxHeatSetpointLimit", MaxHeatSetpointLimit::Id },
                    { "MinCoolSetpointLimit", MinCoolSetpointLimit::Id },
                    { "MaxCoolSetpointLimit", MaxCoolSetpointLimit::Id },
                    { "MinSetpointDeadBand", MinSetpointDeadBand::Id },
                    { "RemoteSensing", RemoteSensing::Id },
                    { "ControlSequenceOfOperation", ControlSequenceOfOperation::Id },
                    { "SystemMode", SystemMode::Id },
                    { "ThermostatRunningMode", ThermostatRunningMode::Id },
                    { "StartOfWeek", StartOfWeek::Id },
                    { "NumberOfWeeklyTransitions", NumberOfWeeklyTransitions::Id },
                    { "NumberOfDailyTransitions", NumberOfDailyTransitions::Id },
                    { "TemperatureSetpointHold", TemperatureSetpointHold::Id },
                    { "TemperatureSetpointHoldDuration", TemperatureSetpointHoldDuration::Id },
                    { "ThermostatProgrammingOperationMode", ThermostatProgrammingOperationMode::Id },
                    { "ThermostatRunningState", ThermostatRunningState::Id },
                    { "SetpointChangeSource", SetpointChangeSource::Id },
                    { "SetpointChangeAmount", SetpointChangeAmount::Id },
                    { "SetpointChangeSourceTimestamp", SetpointChangeSourceTimestamp::Id },
                    { "OccupiedSetback", OccupiedSetback::Id },
                    { "OccupiedSetbackMin", OccupiedSetbackMin::Id },
                    { "OccupiedSetbackMax", OccupiedSetbackMax::Id },
                    { "UnoccupiedSetback", UnoccupiedSetback::Id },
                    { "UnoccupiedSetbackMin", UnoccupiedSetbackMin::Id },
                    { "UnoccupiedSetbackMax", UnoccupiedSetbackMax::Id },
                    { "EmergencyHeatDelta", EmergencyHeatDelta::Id },
                    { "ACType", ACType::Id },
                    { "ACCapacity", ACCapacity::Id },
                    { "ACRefrigerantType", ACRefrigerantType::Id },
                    { "ACCompressorType", ACCompressorType::Id },
                    { "ACErrorCode", ACErrorCode::Id },
                    { "ACLouverPosition", ACLouverPosition::Id },
                    { "ACCoilTemperature", ACCoilTemperature::Id },
                    { "ACCapacityformat", ACCapacityformat::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace Thermostat
        namespace FanControl {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "FanMode", FanMode::Id },
                    { "FanModeSequence", FanModeSequence::Id },
                    { "PercentSetting", PercentSetting::Id },
                    { "PercentCurrent", PercentCurrent::Id },
                    { "SpeedMax", SpeedMax::Id },
                    { "SpeedSetting", SpeedSetting::Id },
                    { "SpeedCurrent", SpeedCurrent::Id },
                    { "RockSupport", RockSupport::Id },
                    { "RockSetting", RockSetting::Id },
                    { "WindSupport", WindSupport::Id },
                    { "WindSetting", WindSetting::Id },
                    { "AirflowDirection", AirflowDirection::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace FanControl
        namespace ThermostatUserInterfaceConfiguration {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "TemperatureDisplayMode", TemperatureDisplayMode::Id },
                    { "KeypadLockout", KeypadLockout::Id },
                    { "ScheduleProgrammingVisibility", ScheduleProgrammingVisibility::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ThermostatUserInterfaceConfiguration
        namespace ColorControl {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "CurrentHue", CurrentHue::Id },
                    { "CurrentSaturation", CurrentSaturation::Id },
                    { "RemainingTime", RemainingTime::Id },
                    { "CurrentX", CurrentX::Id },
                    { "CurrentY", CurrentY::Id },
                    { "DriftCompensation", DriftCompensation::Id },
                    { "CompensationText", CompensationText::Id },
                    { "ColorTemperatureMireds", ColorTemperatureMireds::Id },
                    { "ColorMode", ColorMode::Id },
                    { "Options", Options::Id },
                    { "NumberOfPrimaries", NumberOfPrimaries::Id },
                    { "Primary1X", Primary1X::Id },
                    { "Primary1Y", Primary1Y::Id },
                    { "Primary1Intensity", Primary1Intensity::Id },
                    { "Primary2X", Primary2X::Id },
                    { "Primary2Y", Primary2Y::Id },
                    { "Primary2Intensity", Primary2Intensity::Id },
                    { "Primary3X", Primary3X::Id },
                    { "Primary3Y", Primary3Y::Id },
                    { "Primary3Intensity", Primary3Intensity::Id },
                    { "Primary4X", Primary4X::Id },
                    { "Primary4Y", Primary4Y::Id },
                    { "Primary4Intensity", Primary4Intensity::Id },
                    { "Primary5X", Primary5X::Id },
                    { "Primary5Y", Primary5Y::Id },
                    { "Primary5Intensity", Primary5Intensity::Id },
                    { "Primary6X", Primary6X::Id },
                    { "Primary6Y", Primary6Y::Id },
                    { "Primary6Intensity", Primary6Intensity::Id },
                    { "WhitePointX", WhitePointX::Id },
                    { "WhitePointY", WhitePointY::Id },
                    { "ColorPointRX", ColorPointRX::Id },
                    { "ColorPointRY", ColorPointRY::Id },
                    { "ColorPointRIntensity", ColorPointRIntensity::Id },
                    { "ColorPointGX", ColorPointGX::Id },
                    { "ColorPointGY", ColorPointGY::Id },
                    { "ColorPointGIntensity", ColorPointGIntensity::Id },
                    { "ColorPointBX", ColorPointBX::Id },
                    { "ColorPointBY", ColorPointBY::Id },
                    { "ColorPointBIntensity", ColorPointBIntensity::Id },
                    { "EnhancedCurrentHue", EnhancedCurrentHue::Id },
                    { "EnhancedColorMode", EnhancedColorMode::Id },
                    { "ColorLoopActive", ColorLoopActive::Id },
                    { "ColorLoopDirection", ColorLoopDirection::Id },
                    { "ColorLoopTime", ColorLoopTime::Id },
                    { "ColorLoopStartEnhancedHue", ColorLoopStartEnhancedHue::Id },
                    { "ColorLoopStoredEnhancedHue", ColorLoopStoredEnhancedHue::Id },
                    { "ColorCapabilities", ColorCapabilities::Id },
                    { "ColorTempPhysicalMinMireds", ColorTempPhysicalMinMireds::Id },
                    { "ColorTempPhysicalMaxMireds", ColorTempPhysicalMaxMireds::Id },
                    { "CoupleColorTempToLevelMinMireds", CoupleColorTempToLevelMinMireds::Id },
                    { "StartUpColorTemperatureMireds", StartUpColorTemperatureMireds::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ColorControl
        namespace BallastConfiguration {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "PhysicalMinLevel", PhysicalMinLevel::Id },
                    { "PhysicalMaxLevel", PhysicalMaxLevel::Id },
                    { "BallastStatus", BallastStatus::Id },
                    { "MinLevel", MinLevel::Id },
                    { "MaxLevel", MaxLevel::Id },
                    { "IntrinsicBallastFactor", IntrinsicBallastFactor::Id },
                    { "BallastFactorAdjustment", BallastFactorAdjustment::Id },
                    { "LampQuantity", LampQuantity::Id },
                    { "LampType", LampType::Id },
                    { "LampManufacturer", LampManufacturer::Id },
                    { "LampRatedHours", LampRatedHours::Id },
                    { "LampBurnHours", LampBurnHours::Id },
                    { "LampAlarmMode", LampAlarmMode::Id },
                    { "LampBurnHoursTripPoint", LampBurnHoursTripPoint::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace BallastConfiguration
        namespace IlluminanceMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasuredValue", MeasuredValue::Id },
                    { "MinMeasuredValue", MinMeasuredValue::Id },
                    { "MaxMeasuredValue", MaxMeasuredValue::Id },
                    { "Tolerance", Tolerance::Id },
                    { "LightSensorType", LightSensorType::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace IlluminanceMeasurement
        namespace TemperatureMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasuredValue", MeasuredValue::Id },
                    { "MinMeasuredValue", MinMeasuredValue::Id },
                    { "MaxMeasuredValue", MaxMeasuredValue::Id },
                    { "Tolerance", Tolerance::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace TemperatureMeasurement
        namespace PressureMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasuredValue", MeasuredValue::Id },
                    { "MinMeasuredValue", MinMeasuredValue::Id },
                    { "MaxMeasuredValue", MaxMeasuredValue::Id },
                    { "Tolerance", Tolerance::Id },
                    { "ScaledValue", ScaledValue::Id },
                    { "MinScaledValue", MinScaledValue::Id },
                    { "MaxScaledValue", MaxScaledValue::Id },
                    { "ScaledTolerance", ScaledTolerance::Id },
                    { "Scale", Scale::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace PressureMeasurement
        namespace FlowMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasuredValue", MeasuredValue::Id },
                    { "MinMeasuredValue", MinMeasuredValue::Id },
                    { "MaxMeasuredValue", MaxMeasuredValue::Id },
                    { "Tolerance", Tolerance::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace FlowMeasurement
        namespace RelativeHumidityMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasuredValue", MeasuredValue::Id },
                    { "MinMeasuredValue", MinMeasuredValue::Id },
                    { "MaxMeasuredValue", MaxMeasuredValue::Id },
                    { "Tolerance", Tolerance::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace RelativeHumidityMeasurement
        namespace OccupancySensing {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Occupancy", Occupancy::Id },
                    { "OccupancySensorType", OccupancySensorType::Id },
                    { "OccupancySensorTypeBitmap", OccupancySensorTypeBitmap::Id },
                    { "PIROccupiedToUnoccupiedDelay", PIROccupiedToUnoccupiedDelay::Id },
                    { "PIRUnoccupiedToOccupiedDelay", PIRUnoccupiedToOccupiedDelay::Id },
                    { "PIRUnoccupiedToOccupiedThreshold", PIRUnoccupiedToOccupiedThreshold::Id },
                    { "UltrasonicOccupiedToUnoccupiedDelay", UltrasonicOccupiedToUnoccupiedDelay::Id },
                    { "UltrasonicUnoccupiedToOccupiedDelay", UltrasonicUnoccupiedToOccupiedDelay::Id },
                    { "UltrasonicUnoccupiedToOccupiedThreshold", UltrasonicUnoccupiedToOccupiedThreshold::Id },
                    { "PhysicalContactOccupiedToUnoccupiedDelay", PhysicalContactOccupiedToUnoccupiedDelay::Id },
                    { "PhysicalContactUnoccupiedToOccupiedDelay", PhysicalContactUnoccupiedToOccupiedDelay::Id },
                    { "PhysicalContactUnoccupiedToOccupiedThreshold", PhysicalContactUnoccupiedToOccupiedThreshold::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace OccupancySensing
        namespace CarbonMonoxideConcentrationMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasuredValue", MeasuredValue::Id },
                    { "MinMeasuredValue", MinMeasuredValue::Id },
                    { "MaxMeasuredValue", MaxMeasuredValue::Id },
                    { "PeakMeasuredValue", PeakMeasuredValue::Id },
                    { "PeakMeasuredValueWindow", PeakMeasuredValueWindow::Id },
                    { "AverageMeasuredValue", AverageMeasuredValue::Id },
                    { "AverageMeasuredValueWindow", AverageMeasuredValueWindow::Id },
                    { "Uncertainty", Uncertainty::Id },
                    { "MeasurementUnit", MeasurementUnit::Id },
                    { "MeasurementMedium", MeasurementMedium::Id },
                    { "LevelValue", LevelValue::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace CarbonMonoxideConcentrationMeasurement
        namespace CarbonDioxideConcentrationMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasuredValue", MeasuredValue::Id },
                    { "MinMeasuredValue", MinMeasuredValue::Id },
                    { "MaxMeasuredValue", MaxMeasuredValue::Id },
                    { "PeakMeasuredValue", PeakMeasuredValue::Id },
                    { "PeakMeasuredValueWindow", PeakMeasuredValueWindow::Id },
                    { "AverageMeasuredValue", AverageMeasuredValue::Id },
                    { "AverageMeasuredValueWindow", AverageMeasuredValueWindow::Id },
                    { "Uncertainty", Uncertainty::Id },
                    { "MeasurementUnit", MeasurementUnit::Id },
                    { "MeasurementMedium", MeasurementMedium::Id },
                    { "LevelValue", LevelValue::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace CarbonDioxideConcentrationMeasurement
        namespace NitrogenDioxideConcentrationMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasuredValue", MeasuredValue::Id },
                    { "MinMeasuredValue", MinMeasuredValue::Id },
                    { "MaxMeasuredValue", MaxMeasuredValue::Id },
                    { "PeakMeasuredValue", PeakMeasuredValue::Id },
                    { "PeakMeasuredValueWindow", PeakMeasuredValueWindow::Id },
                    { "AverageMeasuredValue", AverageMeasuredValue::Id },
                    { "AverageMeasuredValueWindow", AverageMeasuredValueWindow::Id },
                    { "Uncertainty", Uncertainty::Id },
                    { "MeasurementUnit", MeasurementUnit::Id },
                    { "MeasurementMedium", MeasurementMedium::Id },
                    { "LevelValue", LevelValue::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace NitrogenDioxideConcentrationMeasurement
        namespace OzoneConcentrationMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasuredValue", MeasuredValue::Id },
                    { "MinMeasuredValue", MinMeasuredValue::Id },
                    { "MaxMeasuredValue", MaxMeasuredValue::Id },
                    { "PeakMeasuredValue", PeakMeasuredValue::Id },
                    { "PeakMeasuredValueWindow", PeakMeasuredValueWindow::Id },
                    { "AverageMeasuredValue", AverageMeasuredValue::Id },
                    { "AverageMeasuredValueWindow", AverageMeasuredValueWindow::Id },
                    { "Uncertainty", Uncertainty::Id },
                    { "MeasurementUnit", MeasurementUnit::Id },
                    { "MeasurementMedium", MeasurementMedium::Id },
                    { "LevelValue", LevelValue::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace OzoneConcentrationMeasurement
        namespace Pm25ConcentrationMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasuredValue", MeasuredValue::Id },
                    { "MinMeasuredValue", MinMeasuredValue::Id },
                    { "MaxMeasuredValue", MaxMeasuredValue::Id },
                    { "PeakMeasuredValue", PeakMeasuredValue::Id },
                    { "PeakMeasuredValueWindow", PeakMeasuredValueWindow::Id },
                    { "AverageMeasuredValue", AverageMeasuredValue::Id },
                    { "AverageMeasuredValueWindow", AverageMeasuredValueWindow::Id },
                    { "Uncertainty", Uncertainty::Id },
                    { "MeasurementUnit", MeasurementUnit::Id },
                    { "MeasurementMedium", MeasurementMedium::Id },
                    { "LevelValue", LevelValue::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace Pm25ConcentrationMeasurement
        namespace FormaldehydeConcentrationMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasuredValue", MeasuredValue::Id },
                    { "MinMeasuredValue", MinMeasuredValue::Id },
                    { "MaxMeasuredValue", MaxMeasuredValue::Id },
                    { "PeakMeasuredValue", PeakMeasuredValue::Id },
                    { "PeakMeasuredValueWindow", PeakMeasuredValueWindow::Id },
                    { "AverageMeasuredValue", AverageMeasuredValue::Id },
                    { "AverageMeasuredValueWindow", AverageMeasuredValueWindow::Id },
                    { "Uncertainty", Uncertainty::Id },
                    { "MeasurementUnit", MeasurementUnit::Id },
                    { "MeasurementMedium", MeasurementMedium::Id },
                    { "LevelValue", LevelValue::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace FormaldehydeConcentrationMeasurement
        namespace Pm1ConcentrationMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasuredValue", MeasuredValue::Id },
                    { "MinMeasuredValue", MinMeasuredValue::Id },
                    { "MaxMeasuredValue", MaxMeasuredValue::Id },
                    { "PeakMeasuredValue", PeakMeasuredValue::Id },
                    { "PeakMeasuredValueWindow", PeakMeasuredValueWindow::Id },
                    { "AverageMeasuredValue", AverageMeasuredValue::Id },
                    { "AverageMeasuredValueWindow", AverageMeasuredValueWindow::Id },
                    { "Uncertainty", Uncertainty::Id },
                    { "MeasurementUnit", MeasurementUnit::Id },
                    { "MeasurementMedium", MeasurementMedium::Id },
                    { "LevelValue", LevelValue::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace Pm1ConcentrationMeasurement
        namespace Pm10ConcentrationMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasuredValue", MeasuredValue::Id },
                    { "MinMeasuredValue", MinMeasuredValue::Id },
                    { "MaxMeasuredValue", MaxMeasuredValue::Id },
                    { "PeakMeasuredValue", PeakMeasuredValue::Id },
                    { "PeakMeasuredValueWindow", PeakMeasuredValueWindow::Id },
                    { "AverageMeasuredValue", AverageMeasuredValue::Id },
                    { "AverageMeasuredValueWindow", AverageMeasuredValueWindow::Id },
                    { "Uncertainty", Uncertainty::Id },
                    { "MeasurementUnit", MeasurementUnit::Id },
                    { "MeasurementMedium", MeasurementMedium::Id },
                    { "LevelValue", LevelValue::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace Pm10ConcentrationMeasurement
        namespace TotalVolatileOrganicCompoundsConcentrationMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasuredValue", MeasuredValue::Id },
                    { "MinMeasuredValue", MinMeasuredValue::Id },
                    { "MaxMeasuredValue", MaxMeasuredValue::Id },
                    { "PeakMeasuredValue", PeakMeasuredValue::Id },
                    { "PeakMeasuredValueWindow", PeakMeasuredValueWindow::Id },
                    { "AverageMeasuredValue", AverageMeasuredValue::Id },
                    { "AverageMeasuredValueWindow", AverageMeasuredValueWindow::Id },
                    { "Uncertainty", Uncertainty::Id },
                    { "MeasurementUnit", MeasurementUnit::Id },
                    { "MeasurementMedium", MeasurementMedium::Id },
                    { "LevelValue", LevelValue::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace TotalVolatileOrganicCompoundsConcentrationMeasurement
        namespace RadonConcentrationMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasuredValue", MeasuredValue::Id },
                    { "MinMeasuredValue", MinMeasuredValue::Id },
                    { "MaxMeasuredValue", MaxMeasuredValue::Id },
                    { "PeakMeasuredValue", PeakMeasuredValue::Id },
                    { "PeakMeasuredValueWindow", PeakMeasuredValueWindow::Id },
                    { "AverageMeasuredValue", AverageMeasuredValue::Id },
                    { "AverageMeasuredValueWindow", AverageMeasuredValueWindow::Id },
                    { "Uncertainty", Uncertainty::Id },
                    { "MeasurementUnit", MeasurementUnit::Id },
                    { "MeasurementMedium", MeasurementMedium::Id },
                    { "LevelValue", LevelValue::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace RadonConcentrationMeasurement
        namespace WakeOnLan {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MACAddress", MACAddress::Id },
                    { "LinkLocalAddress", LinkLocalAddress::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace WakeOnLan
        namespace Channel {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "ChannelList", ChannelList::Id },
                    { "Lineup", Lineup::Id },
                    { "CurrentChannel", CurrentChannel::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace Channel
        namespace TargetNavigator {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "TargetList", TargetList::Id },
                    { "CurrentTarget", CurrentTarget::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace TargetNavigator
        namespace MediaPlayback {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "CurrentState", CurrentState::Id },
                    { "StartTime", StartTime::Id },
                    { "Duration", Duration::Id },
                    { "SampledPosition", SampledPosition::Id },
                    { "PlaybackSpeed", PlaybackSpeed::Id },
                    { "SeekRangeEnd", SeekRangeEnd::Id },
                    { "SeekRangeStart", SeekRangeStart::Id },
                    { "ActiveAudioTrack", ActiveAudioTrack::Id },
                    { "AvailableAudioTracks", AvailableAudioTracks::Id },
                    { "ActiveTextTrack", ActiveTextTrack::Id },
                    { "AvailableTextTracks", AvailableTextTracks::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace MediaPlayback
        namespace MediaInput {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "InputList", InputList::Id },
                    { "CurrentInput", CurrentInput::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace MediaInput
        namespace LowPower {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace LowPower
        namespace KeypadInput {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace KeypadInput
        namespace ContentLauncher {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "AcceptHeader", AcceptHeader::Id },
                    { "SupportedStreamingProtocols", SupportedStreamingProtocols::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ContentLauncher
        namespace AudioOutput {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "OutputList", OutputList::Id },
                    { "CurrentOutput", CurrentOutput::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace AudioOutput
        namespace ApplicationLauncher {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "CatalogList", CatalogList::Id },
                    { "CurrentApp", CurrentApp::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ApplicationLauncher
        namespace ApplicationBasic {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "VendorName", VendorName::Id },
                    { "VendorID", VendorID::Id },
                    { "ApplicationName", ApplicationName::Id },
                    { "ProductID", ProductID::Id },
                    { "Application", Application::Id },
                    { "Status", Status::Id },
                    { "ApplicationVersion", ApplicationVersion::Id },
                    { "AllowedVendorList", AllowedVendorList::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ApplicationBasic
        namespace AccountLogin {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace AccountLogin
        namespace ContentControl {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Enabled", Enabled::Id },
                    { "OnDemandRatings", OnDemandRatings::Id },
                    { "OnDemandRatingThreshold", OnDemandRatingThreshold::Id },
                    { "ScheduledContentRatings", ScheduledContentRatings::Id },
                    { "ScheduledContentRatingThreshold", ScheduledContentRatingThreshold::Id },
                    { "ScreenDailyTime", ScreenDailyTime::Id },
                    { "RemainingScreenTime", RemainingScreenTime::Id },
                    { "BlockUnrated", BlockUnrated::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ContentControl
        namespace ContentAppObserver {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ContentAppObserver
        namespace ElectricalMeasurement {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "MeasurementType", MeasurementType::Id },
                    { "DcVoltage", DcVoltage::Id },
                    { "DcVoltageMin", DcVoltageMin::Id },
                    { "DcVoltageMax", DcVoltageMax::Id },
                    { "DcCurrent", DcCurrent::Id },
                    { "DcCurrentMin", DcCurrentMin::Id },
                    { "DcCurrentMax", DcCurrentMax::Id },
                    { "DcPower", DcPower::Id },
                    { "DcPowerMin", DcPowerMin::Id },
                    { "DcPowerMax", DcPowerMax::Id },
                    { "DcVoltageMultiplier", DcVoltageMultiplier::Id },
                    { "DcVoltageDivisor", DcVoltageDivisor::Id },
                    { "DcCurrentMultiplier", DcCurrentMultiplier::Id },
                    { "DcCurrentDivisor", DcCurrentDivisor::Id },
                    { "DcPowerMultiplier", DcPowerMultiplier::Id },
                    { "DcPowerDivisor", DcPowerDivisor::Id },
                    { "AcFrequency", AcFrequency::Id },
                    { "AcFrequencyMin", AcFrequencyMin::Id },
                    { "AcFrequencyMax", AcFrequencyMax::Id },
                    { "NeutralCurrent", NeutralCurrent::Id },
                    { "TotalActivePower", TotalActivePower::Id },
                    { "TotalReactivePower", TotalReactivePower::Id },
                    { "TotalApparentPower", TotalApparentPower::Id },
                    { "Measured1stHarmonicCurrent", Measured1stHarmonicCurrent::Id },
                    { "Measured3rdHarmonicCurrent", Measured3rdHarmonicCurrent::Id },
                    { "Measured5thHarmonicCurrent", Measured5thHarmonicCurrent::Id },
                    { "Measured7thHarmonicCurrent", Measured7thHarmonicCurrent::Id },
                    { "Measured9thHarmonicCurrent", Measured9thHarmonicCurrent::Id },
                    { "Measured11thHarmonicCurrent", Measured11thHarmonicCurrent::Id },
                    { "MeasuredPhase1stHarmonicCurrent", MeasuredPhase1stHarmonicCurrent::Id },
                    { "MeasuredPhase3rdHarmonicCurrent", MeasuredPhase3rdHarmonicCurrent::Id },
                    { "MeasuredPhase5thHarmonicCurrent", MeasuredPhase5thHarmonicCurrent::Id },
                    { "MeasuredPhase7thHarmonicCurrent", MeasuredPhase7thHarmonicCurrent::Id },
                    { "MeasuredPhase9thHarmonicCurrent", MeasuredPhase9thHarmonicCurrent::Id },
                    { "MeasuredPhase11thHarmonicCurrent", MeasuredPhase11thHarmonicCurrent::Id },
                    { "AcFrequencyMultiplier", AcFrequencyMultiplier::Id },
                    { "AcFrequencyDivisor", AcFrequencyDivisor::Id },
                    { "PowerMultiplier", PowerMultiplier::Id },
                    { "PowerDivisor", PowerDivisor::Id },
                    { "HarmonicCurrentMultiplier", HarmonicCurrentMultiplier::Id },
                    { "PhaseHarmonicCurrentMultiplier", PhaseHarmonicCurrentMultiplier::Id },
                    { "InstantaneousVoltage", InstantaneousVoltage::Id },
                    { "InstantaneousLineCurrent", InstantaneousLineCurrent::Id },
                    { "InstantaneousActiveCurrent", InstantaneousActiveCurrent::Id },
                    { "InstantaneousReactiveCurrent", InstantaneousReactiveCurrent::Id },
                    { "InstantaneousPower", InstantaneousPower::Id },
                    { "RmsVoltage", RmsVoltage::Id },
                    { "RmsVoltageMin", RmsVoltageMin::Id },
                    { "RmsVoltageMax", RmsVoltageMax::Id },
                    { "RmsCurrent", RmsCurrent::Id },
                    { "RmsCurrentMin", RmsCurrentMin::Id },
                    { "RmsCurrentMax", RmsCurrentMax::Id },
                    { "ActivePower", ActivePower::Id },
                    { "ActivePowerMin", ActivePowerMin::Id },
                    { "ActivePowerMax", ActivePowerMax::Id },
                    { "ReactivePower", ReactivePower::Id },
                    { "ApparentPower", ApparentPower::Id },
                    { "PowerFactor", PowerFactor::Id },
                    { "AverageRmsVoltageMeasurementPeriod", AverageRmsVoltageMeasurementPeriod::Id },
                    { "AverageRmsUnderVoltageCounter", AverageRmsUnderVoltageCounter::Id },
                    { "RmsExtremeOverVoltagePeriod", RmsExtremeOverVoltagePeriod::Id },
                    { "RmsExtremeUnderVoltagePeriod", RmsExtremeUnderVoltagePeriod::Id },
                    { "RmsVoltageSagPeriod", RmsVoltageSagPeriod::Id },
                    { "RmsVoltageSwellPeriod", RmsVoltageSwellPeriod::Id },
                    { "AcVoltageMultiplier", AcVoltageMultiplier::Id },
                    { "AcVoltageDivisor", AcVoltageDivisor::Id },
                    { "AcCurrentMultiplier", AcCurrentMultiplier::Id },
                    { "AcCurrentDivisor", AcCurrentDivisor::Id },
                    { "AcPowerMultiplier", AcPowerMultiplier::Id },
                    { "AcPowerDivisor", AcPowerDivisor::Id },
                    { "OverloadAlarmsMask", OverloadAlarmsMask::Id },
                    { "VoltageOverload", VoltageOverload::Id },
                    { "CurrentOverload", CurrentOverload::Id },
                    { "AcOverloadAlarmsMask", AcOverloadAlarmsMask::Id },
                    { "AcVoltageOverload", AcVoltageOverload::Id },
                    { "AcCurrentOverload", AcCurrentOverload::Id },
                    { "AcActivePowerOverload", AcActivePowerOverload::Id },
                    { "AcReactivePowerOverload", AcReactivePowerOverload::Id },
                    { "AverageRmsOverVoltage", AverageRmsOverVoltage::Id },
                    { "AverageRmsUnderVoltage", AverageRmsUnderVoltage::Id },
                    { "RmsExtremeOverVoltage", RmsExtremeOverVoltage::Id },
                    { "RmsExtremeUnderVoltage", RmsExtremeUnderVoltage::Id },
                    { "RmsVoltageSag", RmsVoltageSag::Id },
                    { "RmsVoltageSwell", RmsVoltageSwell::Id },
                    { "LineCurrentPhaseB", LineCurrentPhaseB::Id },
                    { "ActiveCurrentPhaseB", ActiveCurrentPhaseB::Id },
                    { "ReactiveCurrentPhaseB", ReactiveCurrentPhaseB::Id },
                    { "RmsVoltagePhaseB", RmsVoltagePhaseB::Id },
                    { "RmsVoltageMinPhaseB", RmsVoltageMinPhaseB::Id },
                    { "RmsVoltageMaxPhaseB", RmsVoltageMaxPhaseB::Id },
                    { "RmsCurrentPhaseB", RmsCurrentPhaseB::Id },
                    { "RmsCurrentMinPhaseB", RmsCurrentMinPhaseB::Id },
                    { "RmsCurrentMaxPhaseB", RmsCurrentMaxPhaseB::Id },
                    { "ActivePowerPhaseB", ActivePowerPhaseB::Id },
                    { "ActivePowerMinPhaseB", ActivePowerMinPhaseB::Id },
                    { "ActivePowerMaxPhaseB", ActivePowerMaxPhaseB::Id },
                    { "ReactivePowerPhaseB", ReactivePowerPhaseB::Id },
                    { "ApparentPowerPhaseB", ApparentPowerPhaseB::Id },
                    { "PowerFactorPhaseB", PowerFactorPhaseB::Id },
                    { "AverageRmsVoltageMeasurementPeriodPhaseB", AverageRmsVoltageMeasurementPeriodPhaseB::Id },
                    { "AverageRmsOverVoltageCounterPhaseB", AverageRmsOverVoltageCounterPhaseB::Id },
                    { "AverageRmsUnderVoltageCounterPhaseB", AverageRmsUnderVoltageCounterPhaseB::Id },
                    { "RmsExtremeOverVoltagePeriodPhaseB", RmsExtremeOverVoltagePeriodPhaseB::Id },
                    { "RmsExtremeUnderVoltagePeriodPhaseB", RmsExtremeUnderVoltagePeriodPhaseB::Id },
                    { "RmsVoltageSagPeriodPhaseB", RmsVoltageSagPeriodPhaseB::Id },
                    { "RmsVoltageSwellPeriodPhaseB", RmsVoltageSwellPeriodPhaseB::Id },
                    { "LineCurrentPhaseC", LineCurrentPhaseC::Id },
                    { "ActiveCurrentPhaseC", ActiveCurrentPhaseC::Id },
                    { "ReactiveCurrentPhaseC", ReactiveCurrentPhaseC::Id },
                    { "RmsVoltagePhaseC", RmsVoltagePhaseC::Id },
                    { "RmsVoltageMinPhaseC", RmsVoltageMinPhaseC::Id },
                    { "RmsVoltageMaxPhaseC", RmsVoltageMaxPhaseC::Id },
                    { "RmsCurrentPhaseC", RmsCurrentPhaseC::Id },
                    { "RmsCurrentMinPhaseC", RmsCurrentMinPhaseC::Id },
                    { "RmsCurrentMaxPhaseC", RmsCurrentMaxPhaseC::Id },
                    { "ActivePowerPhaseC", ActivePowerPhaseC::Id },
                    { "ActivePowerMinPhaseC", ActivePowerMinPhaseC::Id },
                    { "ActivePowerMaxPhaseC", ActivePowerMaxPhaseC::Id },
                    { "ReactivePowerPhaseC", ReactivePowerPhaseC::Id },
                    { "ApparentPowerPhaseC", ApparentPowerPhaseC::Id },
                    { "PowerFactorPhaseC", PowerFactorPhaseC::Id },
                    { "AverageRmsVoltageMeasurementPeriodPhaseC", AverageRmsVoltageMeasurementPeriodPhaseC::Id },
                    { "AverageRmsOverVoltageCounterPhaseC", AverageRmsOverVoltageCounterPhaseC::Id },
                    { "AverageRmsUnderVoltageCounterPhaseC", AverageRmsUnderVoltageCounterPhaseC::Id },
                    { "RmsExtremeOverVoltagePeriodPhaseC", RmsExtremeOverVoltagePeriodPhaseC::Id },
                    { "RmsExtremeUnderVoltagePeriodPhaseC", RmsExtremeUnderVoltagePeriodPhaseC::Id },
                    { "RmsVoltageSagPeriodPhaseC", RmsVoltageSagPeriodPhaseC::Id },
                    { "RmsVoltageSwellPeriodPhaseC", RmsVoltageSwellPeriodPhaseC::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace ElectricalMeasurement
        namespace UnitTesting {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "Boolean", Boolean::Id },
                    { "Bitmap8", Bitmap8::Id },
                    { "Bitmap16", Bitmap16::Id },
                    { "Bitmap32", Bitmap32::Id },
                    { "Bitmap64", Bitmap64::Id },
                    { "Int8u", Int8u::Id },
                    { "Int16u", Int16u::Id },
                    { "Int24u", Int24u::Id },
                    { "Int32u", Int32u::Id },
                    { "Int40u", Int40u::Id },
                    { "Int48u", Int48u::Id },
                    { "Int56u", Int56u::Id },
                    { "Int64u", Int64u::Id },
                    { "Int8s", Int8s::Id },
                    { "Int16s", Int16s::Id },
                    { "Int24s", Int24s::Id },
                    { "Int32s", Int32s::Id },
                    { "Int40s", Int40s::Id },
                    { "Int48s", Int48s::Id },
                    { "Int56s", Int56s::Id },
                    { "Int64s", Int64s::Id },
                    { "Enum8", Enum8::Id },
                    { "Enum16", Enum16::Id },
                    { "FloatSingle", FloatSingle::Id },
                    { "FloatDouble", FloatDouble::Id },
                    { "OctetString", OctetString::Id },
                    { "ListInt8u", ListInt8u::Id },
                    { "ListOctetString", ListOctetString::Id },
                    { "ListStructOctetString", ListStructOctetString::Id },
                    { "LongOctetString", LongOctetString::Id },
                    { "CharString", CharString::Id },
                    { "LongCharString", LongCharString::Id },
                    { "EpochUs", EpochUs::Id },
                    { "EpochS", EpochS::Id },
                    { "VendorId", VendorId::Id },
                    { "ListNullablesAndOptionalsStruct", ListNullablesAndOptionalsStruct::Id },
                    { "EnumAttr", EnumAttr::Id },
                    { "StructAttr", StructAttr::Id },
                    { "RangeRestrictedInt8u", RangeRestrictedInt8u::Id },
                    { "RangeRestrictedInt8s", RangeRestrictedInt8s::Id },
                    { "RangeRestrictedInt16u", RangeRestrictedInt16u::Id },
                    { "RangeRestrictedInt16s", RangeRestrictedInt16s::Id },
                    { "ListLongOctetString", ListLongOctetString::Id },
                    { "ListFabricScoped", ListFabricScoped::Id },
                    { "TimedWriteBoolean", TimedWriteBoolean::Id },
                    { "GeneralErrorBoolean", GeneralErrorBoolean::Id },
                    { "ClusterErrorBoolean", ClusterErrorBoolean::Id },
                    { "Unsupported", Unsupported::Id },
                    { "NullableBoolean", NullableBoolean::Id },
                    { "NullableBitmap8", NullableBitmap8::Id },
                    { "NullableBitmap16", NullableBitmap16::Id },
                    { "NullableBitmap32", NullableBitmap32::Id },
                    { "NullableBitmap64", NullableBitmap64::Id },
                    { "NullableInt8u", NullableInt8u::Id },
                    { "NullableInt16u", NullableInt16u::Id },
                    { "NullableInt24u", NullableInt24u::Id },
                    { "NullableInt32u", NullableInt32u::Id },
                    { "NullableInt40u", NullableInt40u::Id },
                    { "NullableInt48u", NullableInt48u::Id },
                    { "NullableInt56u", NullableInt56u::Id },
                    { "NullableInt64u", NullableInt64u::Id },
                    { "NullableInt8s", NullableInt8s::Id },
                    { "NullableInt16s", NullableInt16s::Id },
                    { "NullableInt24s", NullableInt24s::Id },
                    { "NullableInt32s", NullableInt32s::Id },
                    { "NullableInt40s", NullableInt40s::Id },
                    { "NullableInt48s", NullableInt48s::Id },
                    { "NullableInt56s", NullableInt56s::Id },
                    { "NullableInt64s", NullableInt64s::Id },
                    { "NullableEnum8", NullableEnum8::Id },
                    { "NullableEnum16", NullableEnum16::Id },
                    { "NullableFloatSingle", NullableFloatSingle::Id },
                    { "NullableFloatDouble", NullableFloatDouble::Id },
                    { "NullableOctetString", NullableOctetString::Id },
                    { "NullableCharString", NullableCharString::Id },
                    { "NullableEnumAttr", NullableEnumAttr::Id },
                    { "NullableStruct", NullableStruct::Id },
                    { "NullableRangeRestrictedInt8u", NullableRangeRestrictedInt8u::Id },
                    { "NullableRangeRestrictedInt8s", NullableRangeRestrictedInt8s::Id },
                    { "NullableRangeRestrictedInt16u", NullableRangeRestrictedInt16u::Id },
                    { "NullableRangeRestrictedInt16s", NullableRangeRestrictedInt16s::Id },
                    { "WriteOnlyInt8u", WriteOnlyInt8u::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace UnitTesting
        namespace FaultInjection {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace FaultInjection
        namespace SampleMei {
            namespace Attributes {
                static const std::unordered_map<std::string, chip::AttributeId> matter_attribute_id_cluster_map {
                    { "FlipFlop", FlipFlop::Id },
                    { "GeneratedCommandList", GeneratedCommandList::Id },
                    { "AcceptedCommandList", AcceptedCommandList::Id },
                    { "EventList", EventList::Id },
                    { "AttributeList", AttributeList::Id },
                    { "FeatureMap", FeatureMap::Id },
                    { "ClusterRevision", ClusterRevision::Id }
                };
            } // namespace Attributes
        } // namespace SampleMei

        // Global map of each clusters attributes
        static const std::unordered_map<std::string, std::unordered_map<std::string, chip::AttributeId>> matter_attribute_id_map {
            { "Identify", Identify::Attributes::matter_attribute_id_cluster_map },
            { "Groups", Groups::Attributes::matter_attribute_id_cluster_map },
            { "Scenes", Scenes::Attributes::matter_attribute_id_cluster_map },
            { "OnOff", OnOff::Attributes::matter_attribute_id_cluster_map },
            { "OnOffSwitchConfiguration", OnOffSwitchConfiguration::Attributes::matter_attribute_id_cluster_map },
            { "LevelControl", LevelControl::Attributes::matter_attribute_id_cluster_map },
            { "BinaryInputBasic", BinaryInputBasic::Attributes::matter_attribute_id_cluster_map },
            { "PulseWidthModulation", PulseWidthModulation::Attributes::matter_attribute_id_cluster_map },
            { "Descriptor", Descriptor::Attributes::matter_attribute_id_cluster_map },
            { "Binding", Binding::Attributes::matter_attribute_id_cluster_map },
            { "AccessControl", AccessControl::Attributes::matter_attribute_id_cluster_map },
            { "Actions", Actions::Attributes::matter_attribute_id_cluster_map },
            { "BasicInformation", BasicInformation::Attributes::matter_attribute_id_cluster_map },
            { "OtaSoftwareUpdateProvider", OtaSoftwareUpdateProvider::Attributes::matter_attribute_id_cluster_map },
            { "OtaSoftwareUpdateRequestor", OtaSoftwareUpdateRequestor::Attributes::matter_attribute_id_cluster_map },
            { "LocalizationConfiguration", LocalizationConfiguration::Attributes::matter_attribute_id_cluster_map },
            { "TimeFormatLocalization", TimeFormatLocalization::Attributes::matter_attribute_id_cluster_map },
            { "UnitLocalization", UnitLocalization::Attributes::matter_attribute_id_cluster_map },
            { "PowerSourceConfiguration", PowerSourceConfiguration::Attributes::matter_attribute_id_cluster_map },
            { "PowerSource", PowerSource::Attributes::matter_attribute_id_cluster_map },
            { "GeneralCommissioning", GeneralCommissioning::Attributes::matter_attribute_id_cluster_map },
            { "NetworkCommissioning", NetworkCommissioning::Attributes::matter_attribute_id_cluster_map },
            { "DiagnosticLogs", DiagnosticLogs::Attributes::matter_attribute_id_cluster_map },
            { "GeneralDiagnostics", GeneralDiagnostics::Attributes::matter_attribute_id_cluster_map },
            { "SoftwareDiagnostics", SoftwareDiagnostics::Attributes::matter_attribute_id_cluster_map },
            { "ThreadNetworkDiagnostics", ThreadNetworkDiagnostics::Attributes::matter_attribute_id_cluster_map },
            { "WiFiNetworkDiagnostics", WiFiNetworkDiagnostics::Attributes::matter_attribute_id_cluster_map },
            { "EthernetNetworkDiagnostics", EthernetNetworkDiagnostics::Attributes::matter_attribute_id_cluster_map },
            { "TimeSynchronization", TimeSynchronization::Attributes::matter_attribute_id_cluster_map },
            { "BridgedDeviceBasicInformation", BridgedDeviceBasicInformation::Attributes::matter_attribute_id_cluster_map },
            { "Switch", Switch::Attributes::matter_attribute_id_cluster_map },
            { "AdministratorCommissioning", AdministratorCommissioning::Attributes::matter_attribute_id_cluster_map },
            { "OperationalCredentials", OperationalCredentials::Attributes::matter_attribute_id_cluster_map },
            { "GroupKeyManagement", GroupKeyManagement::Attributes::matter_attribute_id_cluster_map },
            { "FixedLabel", FixedLabel::Attributes::matter_attribute_id_cluster_map },
            { "UserLabel", UserLabel::Attributes::matter_attribute_id_cluster_map },
            { "ProxyConfiguration", ProxyConfiguration::Attributes::matter_attribute_id_cluster_map },
            { "ProxyDiscovery", ProxyDiscovery::Attributes::matter_attribute_id_cluster_map },
            { "ProxyValid", ProxyValid::Attributes::matter_attribute_id_cluster_map },
            { "BooleanState", BooleanState::Attributes::matter_attribute_id_cluster_map },
            { "IcdManagement", IcdManagement::Attributes::matter_attribute_id_cluster_map },
            { "Timer", Timer::Attributes::matter_attribute_id_cluster_map },
            { "OvenCavityOperationalState", OvenCavityOperationalState::Attributes::matter_attribute_id_cluster_map },
            { "OvenMode", OvenMode::Attributes::matter_attribute_id_cluster_map },
            { "LaundryDryerControls", LaundryDryerControls::Attributes::matter_attribute_id_cluster_map },
            { "ModeSelect", ModeSelect::Attributes::matter_attribute_id_cluster_map },
            { "LaundryWasherMode", LaundryWasherMode::Attributes::matter_attribute_id_cluster_map },
            { "RefrigeratorAndTemperatureControlledCabinetMode", RefrigeratorAndTemperatureControlledCabinetMode::Attributes::matter_attribute_id_cluster_map },
            { "LaundryWasherControls", LaundryWasherControls::Attributes::matter_attribute_id_cluster_map },
            { "RvcRunMode", RvcRunMode::Attributes::matter_attribute_id_cluster_map },
            { "RvcCleanMode", RvcCleanMode::Attributes::matter_attribute_id_cluster_map },
            { "TemperatureControl", TemperatureControl::Attributes::matter_attribute_id_cluster_map },
            { "RefrigeratorAlarm", RefrigeratorAlarm::Attributes::matter_attribute_id_cluster_map },
            { "DishwasherMode", DishwasherMode::Attributes::matter_attribute_id_cluster_map },
            { "AirQuality", AirQuality::Attributes::matter_attribute_id_cluster_map },
            { "SmokeCoAlarm", SmokeCoAlarm::Attributes::matter_attribute_id_cluster_map },
            { "DishwasherAlarm", DishwasherAlarm::Attributes::matter_attribute_id_cluster_map },
            { "MicrowaveOvenMode", MicrowaveOvenMode::Attributes::matter_attribute_id_cluster_map },
            { "MicrowaveOvenControl", MicrowaveOvenControl::Attributes::matter_attribute_id_cluster_map },
            { "OperationalState", OperationalState::Attributes::matter_attribute_id_cluster_map },
            { "RvcOperationalState", RvcOperationalState::Attributes::matter_attribute_id_cluster_map },
            { "HepaFilterMonitoring", HepaFilterMonitoring::Attributes::matter_attribute_id_cluster_map },
            { "ActivatedCarbonFilterMonitoring", ActivatedCarbonFilterMonitoring::Attributes::matter_attribute_id_cluster_map },
            { "BooleanStateConfiguration", BooleanStateConfiguration::Attributes::matter_attribute_id_cluster_map },
            { "ValveConfigurationAndControl", ValveConfigurationAndControl::Attributes::matter_attribute_id_cluster_map },
            { "ElectricalEnergyMeasurement", ElectricalEnergyMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "DemandResponseLoadControl", DemandResponseLoadControl::Attributes::matter_attribute_id_cluster_map },
            { "DeviceEnergyManagement", DeviceEnergyManagement::Attributes::matter_attribute_id_cluster_map },
            { "EnergyEvse", EnergyEvse::Attributes::matter_attribute_id_cluster_map },
            { "EnergyPreference", EnergyPreference::Attributes::matter_attribute_id_cluster_map },
            { "DoorLock", DoorLock::Attributes::matter_attribute_id_cluster_map },
            { "WindowCovering", WindowCovering::Attributes::matter_attribute_id_cluster_map },
            { "BarrierControl", BarrierControl::Attributes::matter_attribute_id_cluster_map },
            { "PumpConfigurationAndControl", PumpConfigurationAndControl::Attributes::matter_attribute_id_cluster_map },
            { "Thermostat", Thermostat::Attributes::matter_attribute_id_cluster_map },
            { "FanControl", FanControl::Attributes::matter_attribute_id_cluster_map },
            { "ThermostatUserInterfaceConfiguration", ThermostatUserInterfaceConfiguration::Attributes::matter_attribute_id_cluster_map },
            { "ColorControl", ColorControl::Attributes::matter_attribute_id_cluster_map },
            { "BallastConfiguration", BallastConfiguration::Attributes::matter_attribute_id_cluster_map },
            { "IlluminanceMeasurement", IlluminanceMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "TemperatureMeasurement", TemperatureMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "PressureMeasurement", PressureMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "FlowMeasurement", FlowMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "RelativeHumidityMeasurement", RelativeHumidityMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "OccupancySensing", OccupancySensing::Attributes::matter_attribute_id_cluster_map },
            { "CarbonMonoxideConcentrationMeasurement", CarbonMonoxideConcentrationMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "CarbonDioxideConcentrationMeasurement", CarbonDioxideConcentrationMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "NitrogenDioxideConcentrationMeasurement", NitrogenDioxideConcentrationMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "OzoneConcentrationMeasurement", OzoneConcentrationMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "Pm25ConcentrationMeasurement", Pm25ConcentrationMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "FormaldehydeConcentrationMeasurement", FormaldehydeConcentrationMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "Pm1ConcentrationMeasurement", Pm1ConcentrationMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "Pm10ConcentrationMeasurement", Pm10ConcentrationMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "TotalVolatileOrganicCompoundsConcentrationMeasurement", TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "RadonConcentrationMeasurement", RadonConcentrationMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "WakeOnLan", WakeOnLan::Attributes::matter_attribute_id_cluster_map },
            { "Channel", Channel::Attributes::matter_attribute_id_cluster_map },
            { "TargetNavigator", TargetNavigator::Attributes::matter_attribute_id_cluster_map },
            { "MediaPlayback", MediaPlayback::Attributes::matter_attribute_id_cluster_map },
            { "MediaInput", MediaInput::Attributes::matter_attribute_id_cluster_map },
            { "LowPower", LowPower::Attributes::matter_attribute_id_cluster_map },
            { "KeypadInput", KeypadInput::Attributes::matter_attribute_id_cluster_map },
            { "ContentLauncher", ContentLauncher::Attributes::matter_attribute_id_cluster_map },
            { "AudioOutput", AudioOutput::Attributes::matter_attribute_id_cluster_map },
            { "ApplicationLauncher", ApplicationLauncher::Attributes::matter_attribute_id_cluster_map },
            { "ApplicationBasic", ApplicationBasic::Attributes::matter_attribute_id_cluster_map },
            { "AccountLogin", AccountLogin::Attributes::matter_attribute_id_cluster_map },
            { "ContentControl", ContentControl::Attributes::matter_attribute_id_cluster_map },
            { "ContentAppObserver", ContentAppObserver::Attributes::matter_attribute_id_cluster_map },
            { "ElectricalMeasurement", ElectricalMeasurement::Attributes::matter_attribute_id_cluster_map },
            { "UnitTesting", UnitTesting::Attributes::matter_attribute_id_cluster_map },
            { "FaultInjection", FaultInjection::Attributes::matter_attribute_id_cluster_map },
            { "SampleMei", SampleMei::Attributes::matter_attribute_id_cluster_map }
        };

        // Command mapping Matter
        // Command structure for each cluster
        namespace Identify {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "Identify", Identify::Id },
                    { "TriggerEffect", TriggerEffect::Id }
                };
            } // namespace Commands
        } // namespace Identify
        namespace Groups {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "AddGroup", AddGroup::Id },
                    { "AddGroupResponse", AddGroupResponse::Id },
                    { "ViewGroup", ViewGroup::Id },
                    { "ViewGroupResponse", ViewGroupResponse::Id },
                    { "GetGroupMembership", GetGroupMembership::Id },
                    { "GetGroupMembershipResponse", GetGroupMembershipResponse::Id },
                    { "RemoveGroup", RemoveGroup::Id },
                    { "RemoveGroupResponse", RemoveGroupResponse::Id },
                    { "RemoveAllGroups", RemoveAllGroups::Id },
                    { "AddGroupIfIdentifying", AddGroupIfIdentifying::Id }
                };
            } // namespace Commands
        } // namespace Groups
        namespace Scenes {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "AddScene", AddScene::Id },
                    { "AddSceneResponse", AddSceneResponse::Id },
                    { "ViewScene", ViewScene::Id },
                    { "ViewSceneResponse", ViewSceneResponse::Id },
                    { "RemoveScene", RemoveScene::Id },
                    { "RemoveSceneResponse", RemoveSceneResponse::Id },
                    { "RemoveAllScenes", RemoveAllScenes::Id },
                    { "RemoveAllScenesResponse", RemoveAllScenesResponse::Id },
                    { "StoreScene", StoreScene::Id },
                    { "StoreSceneResponse", StoreSceneResponse::Id },
                    { "RecallScene", RecallScene::Id },
                    { "GetSceneMembership", GetSceneMembership::Id },
                    { "GetSceneMembershipResponse", GetSceneMembershipResponse::Id },
                    { "EnhancedAddScene", EnhancedAddScene::Id },
                    { "EnhancedAddSceneResponse", EnhancedAddSceneResponse::Id },
                    { "EnhancedViewScene", EnhancedViewScene::Id },
                    { "EnhancedViewSceneResponse", EnhancedViewSceneResponse::Id },
                    { "CopyScene", CopyScene::Id },
                    { "CopySceneResponse", CopySceneResponse::Id }
                };
            } // namespace Commands
        } // namespace Scenes
        namespace OnOff {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "Off", Off::Id },
                    { "On", On::Id },
                    { "Toggle", Toggle::Id },
                    { "OffWithEffect", OffWithEffect::Id },
                    { "OnWithRecallGlobalScene", OnWithRecallGlobalScene::Id },
                    { "OnWithTimedOff", OnWithTimedOff::Id }
                };
            } // namespace Commands
        } // namespace OnOff
        namespace OnOffSwitchConfiguration {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace OnOffSwitchConfiguration
        namespace LevelControl {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "MoveToLevel", MoveToLevel::Id },
                    { "Move", Move::Id },
                    { "Step", Step::Id },
                    { "Stop", Stop::Id },
                    { "MoveToLevelWithOnOff", MoveToLevelWithOnOff::Id },
                    { "MoveWithOnOff", MoveWithOnOff::Id },
                    { "StepWithOnOff", StepWithOnOff::Id },
                    { "StopWithOnOff", StopWithOnOff::Id },
                    { "MoveToClosestFrequency", MoveToClosestFrequency::Id }
                };
            } // namespace Commands
        } // namespace LevelControl
        namespace BinaryInputBasic {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace BinaryInputBasic
        namespace PulseWidthModulation {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace PulseWidthModulation
        namespace Descriptor {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace Descriptor
        namespace Binding {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace Binding
        namespace AccessControl {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace AccessControl
        namespace Actions {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "InstantAction", InstantAction::Id },
                    { "InstantActionWithTransition", InstantActionWithTransition::Id },
                    { "StartAction", StartAction::Id },
                    { "StartActionWithDuration", StartActionWithDuration::Id },
                    { "StopAction", StopAction::Id },
                    { "PauseAction", PauseAction::Id },
                    { "PauseActionWithDuration", PauseActionWithDuration::Id },
                    { "ResumeAction", ResumeAction::Id },
                    { "EnableAction", EnableAction::Id },
                    { "EnableActionWithDuration", EnableActionWithDuration::Id },
                    { "DisableAction", DisableAction::Id },
                    { "DisableActionWithDuration", DisableActionWithDuration::Id }
                };
            } // namespace Commands
        } // namespace Actions
        namespace BasicInformation {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "MfgSpecificPing", MfgSpecificPing::Id }
                };
            } // namespace Commands
        } // namespace BasicInformation
        namespace OtaSoftwareUpdateProvider {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "QueryImage", QueryImage::Id },
                    { "QueryImageResponse", QueryImageResponse::Id },
                    { "ApplyUpdateRequest", ApplyUpdateRequest::Id },
                    { "ApplyUpdateResponse", ApplyUpdateResponse::Id },
                    { "NotifyUpdateApplied", NotifyUpdateApplied::Id }
                };
            } // namespace Commands
        } // namespace OtaSoftwareUpdateProvider
        namespace OtaSoftwareUpdateRequestor {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "AnnounceOTAProvider", AnnounceOTAProvider::Id }
                };
            } // namespace Commands
        } // namespace OtaSoftwareUpdateRequestor
        namespace LocalizationConfiguration {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace LocalizationConfiguration
        namespace TimeFormatLocalization {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace TimeFormatLocalization
        namespace UnitLocalization {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace UnitLocalization
        namespace PowerSourceConfiguration {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace PowerSourceConfiguration
        namespace PowerSource {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace PowerSource
        namespace GeneralCommissioning {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ArmFailSafe", ArmFailSafe::Id },
                    { "ArmFailSafeResponse", ArmFailSafeResponse::Id },
                    { "SetRegulatoryConfig", SetRegulatoryConfig::Id },
                    { "SetRegulatoryConfigResponse", SetRegulatoryConfigResponse::Id },
                    { "CommissioningComplete", CommissioningComplete::Id },
                    { "CommissioningCompleteResponse", CommissioningCompleteResponse::Id }
                };
            } // namespace Commands
        } // namespace GeneralCommissioning
        namespace NetworkCommissioning {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ScanNetworks", ScanNetworks::Id },
                    { "ScanNetworksResponse", ScanNetworksResponse::Id },
                    { "AddOrUpdateWiFiNetwork", AddOrUpdateWiFiNetwork::Id },
                    { "AddOrUpdateThreadNetwork", AddOrUpdateThreadNetwork::Id },
                    { "RemoveNetwork", RemoveNetwork::Id },
                    { "NetworkConfigResponse", NetworkConfigResponse::Id },
                    { "ConnectNetwork", ConnectNetwork::Id },
                    { "ConnectNetworkResponse", ConnectNetworkResponse::Id },
                    { "ReorderNetwork", ReorderNetwork::Id },
                    { "QueryIdentity", QueryIdentity::Id },
                    { "QueryIdentityResponse", QueryIdentityResponse::Id }
                };
            } // namespace Commands
        } // namespace NetworkCommissioning
        namespace DiagnosticLogs {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "RetrieveLogsRequest", RetrieveLogsRequest::Id },
                    { "RetrieveLogsResponse", RetrieveLogsResponse::Id }
                };
            } // namespace Commands
        } // namespace DiagnosticLogs
        namespace GeneralDiagnostics {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "TestEventTrigger", TestEventTrigger::Id },
                    { "TimeSnapshot", TimeSnapshot::Id },
                    { "TimeSnapshotResponse", TimeSnapshotResponse::Id }
                };
            } // namespace Commands
        } // namespace GeneralDiagnostics
        namespace SoftwareDiagnostics {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ResetWatermarks", ResetWatermarks::Id }
                };
            } // namespace Commands
        } // namespace SoftwareDiagnostics
        namespace ThreadNetworkDiagnostics {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ResetCounts", ResetCounts::Id }
                };
            } // namespace Commands
        } // namespace ThreadNetworkDiagnostics
        namespace WiFiNetworkDiagnostics {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ResetCounts", ResetCounts::Id }
                };
            } // namespace Commands
        } // namespace WiFiNetworkDiagnostics
        namespace EthernetNetworkDiagnostics {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ResetCounts", ResetCounts::Id }
                };
            } // namespace Commands
        } // namespace EthernetNetworkDiagnostics
        namespace TimeSynchronization {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "SetUTCTime", SetUTCTime::Id },
                    { "SetTrustedTimeSource", SetTrustedTimeSource::Id },
                    { "SetTimeZone", SetTimeZone::Id },
                    { "SetTimeZoneResponse", SetTimeZoneResponse::Id },
                    { "SetDSTOffset", SetDSTOffset::Id },
                    { "SetDefaultNTP", SetDefaultNTP::Id }
                };
            } // namespace Commands
        } // namespace TimeSynchronization
        namespace BridgedDeviceBasicInformation {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace BridgedDeviceBasicInformation
        namespace Switch {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace Switch
        namespace AdministratorCommissioning {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "OpenCommissioningWindow", OpenCommissioningWindow::Id },
                    { "OpenBasicCommissioningWindow", OpenBasicCommissioningWindow::Id },
                    { "RevokeCommissioning", RevokeCommissioning::Id }
                };
            } // namespace Commands
        } // namespace AdministratorCommissioning
        namespace OperationalCredentials {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "AttestationRequest", AttestationRequest::Id },
                    { "AttestationResponse", AttestationResponse::Id },
                    { "CertificateChainRequest", CertificateChainRequest::Id },
                    { "CertificateChainResponse", CertificateChainResponse::Id },
                    { "CSRRequest", CSRRequest::Id },
                    { "CSRResponse", CSRResponse::Id },
                    { "AddNOC", AddNOC::Id },
                    { "UpdateNOC", UpdateNOC::Id },
                    { "NOCResponse", NOCResponse::Id },
                    { "UpdateFabricLabel", UpdateFabricLabel::Id },
                    { "RemoveFabric", RemoveFabric::Id },
                    { "AddTrustedRootCertificate", AddTrustedRootCertificate::Id }
                };
            } // namespace Commands
        } // namespace OperationalCredentials
        namespace GroupKeyManagement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "KeySetWrite", KeySetWrite::Id },
                    { "KeySetRead", KeySetRead::Id },
                    { "KeySetReadResponse", KeySetReadResponse::Id },
                    { "KeySetRemove", KeySetRemove::Id },
                    { "KeySetReadAllIndices", KeySetReadAllIndices::Id },
                    { "KeySetReadAllIndicesResponse", KeySetReadAllIndicesResponse::Id }
                };
            } // namespace Commands
        } // namespace GroupKeyManagement
        namespace FixedLabel {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace FixedLabel
        namespace UserLabel {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace UserLabel
        namespace ProxyConfiguration {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace ProxyConfiguration
        namespace ProxyDiscovery {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace ProxyDiscovery
        namespace ProxyValid {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace ProxyValid
        namespace BooleanState {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace BooleanState
        namespace IcdManagement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "RegisterClient", RegisterClient::Id },
                    { "RegisterClientResponse", RegisterClientResponse::Id },
                    { "UnregisterClient", UnregisterClient::Id },
                    { "StayActiveRequest", StayActiveRequest::Id },
                    { "StayActiveResponse", StayActiveResponse::Id }
                };
            } // namespace Commands
        } // namespace IcdManagement
        namespace Timer {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "SetTimer", SetTimer::Id },
                    { "ResetTimer", ResetTimer::Id },
                    { "AddTime", AddTime::Id },
                    { "ReduceTime", ReduceTime::Id }
                };
            } // namespace Commands
        } // namespace Timer
        namespace OvenCavityOperationalState {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "Pause", Pause::Id },
                    { "Stop", Stop::Id },
                    { "Start", Start::Id },
                    { "Resume", Resume::Id },
                    { "OperationalCommandResponse", OperationalCommandResponse::Id }
                };
            } // namespace Commands
        } // namespace OvenCavityOperationalState
        namespace OvenMode {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ChangeToMode", ChangeToMode::Id },
                    { "ChangeToModeResponse", ChangeToModeResponse::Id }
                };
            } // namespace Commands
        } // namespace OvenMode
        namespace LaundryDryerControls {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace LaundryDryerControls
        namespace ModeSelect {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ChangeToMode", ChangeToMode::Id }
                };
            } // namespace Commands
        } // namespace ModeSelect
        namespace LaundryWasherMode {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ChangeToMode", ChangeToMode::Id },
                    { "ChangeToModeResponse", ChangeToModeResponse::Id }
                };
            } // namespace Commands
        } // namespace LaundryWasherMode
        namespace RefrigeratorAndTemperatureControlledCabinetMode {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ChangeToMode", ChangeToMode::Id },
                    { "ChangeToModeResponse", ChangeToModeResponse::Id }
                };
            } // namespace Commands
        } // namespace RefrigeratorAndTemperatureControlledCabinetMode
        namespace LaundryWasherControls {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace LaundryWasherControls
        namespace RvcRunMode {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ChangeToMode", ChangeToMode::Id },
                    { "ChangeToModeResponse", ChangeToModeResponse::Id }
                };
            } // namespace Commands
        } // namespace RvcRunMode
        namespace RvcCleanMode {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ChangeToMode", ChangeToMode::Id },
                    { "ChangeToModeResponse", ChangeToModeResponse::Id }
                };
            } // namespace Commands
        } // namespace RvcCleanMode
        namespace TemperatureControl {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "SetTemperature", SetTemperature::Id }
                };
            } // namespace Commands
        } // namespace TemperatureControl
        namespace RefrigeratorAlarm {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace RefrigeratorAlarm
        namespace DishwasherMode {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ChangeToMode", ChangeToMode::Id },
                    { "ChangeToModeResponse", ChangeToModeResponse::Id }
                };
            } // namespace Commands
        } // namespace DishwasherMode
        namespace AirQuality {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace AirQuality
        namespace SmokeCoAlarm {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "SelfTestRequest", SelfTestRequest::Id }
                };
            } // namespace Commands
        } // namespace SmokeCoAlarm
        namespace DishwasherAlarm {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "Reset", Reset::Id },
                    { "ModifyEnabledAlarms", ModifyEnabledAlarms::Id }
                };
            } // namespace Commands
        } // namespace DishwasherAlarm
        namespace MicrowaveOvenMode {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace MicrowaveOvenMode
        namespace MicrowaveOvenControl {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "SetCookingParameters", SetCookingParameters::Id },
                    { "AddMoreTime", AddMoreTime::Id }
                };
            } // namespace Commands
        } // namespace MicrowaveOvenControl
        namespace OperationalState {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "Pause", Pause::Id },
                    { "Stop", Stop::Id },
                    { "Start", Start::Id },
                    { "Resume", Resume::Id },
                    { "OperationalCommandResponse", OperationalCommandResponse::Id }
                };
            } // namespace Commands
        } // namespace OperationalState
        namespace RvcOperationalState {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "Pause", Pause::Id },
                    { "Stop", Stop::Id },
                    { "Start", Start::Id },
                    { "Resume", Resume::Id },
                    { "OperationalCommandResponse", OperationalCommandResponse::Id }
                };
            } // namespace Commands
        } // namespace RvcOperationalState
        namespace HepaFilterMonitoring {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ResetCondition", ResetCondition::Id }
                };
            } // namespace Commands
        } // namespace HepaFilterMonitoring
        namespace ActivatedCarbonFilterMonitoring {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ResetCondition", ResetCondition::Id }
                };
            } // namespace Commands
        } // namespace ActivatedCarbonFilterMonitoring
        namespace BooleanStateConfiguration {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "SuppressAlarm", SuppressAlarm::Id },
                    { "EnableDisableAlarm", EnableDisableAlarm::Id }
                };
            } // namespace Commands
        } // namespace BooleanStateConfiguration
        namespace ValveConfigurationAndControl {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "Open", Open::Id },
                    { "Close", Close::Id }
                };
            } // namespace Commands
        } // namespace ValveConfigurationAndControl
        namespace ElectricalEnergyMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace ElectricalEnergyMeasurement
        namespace DemandResponseLoadControl {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "RegisterLoadControlProgramRequest", RegisterLoadControlProgramRequest::Id },
                    { "UnregisterLoadControlProgramRequest", UnregisterLoadControlProgramRequest::Id },
                    { "AddLoadControlEventRequest", AddLoadControlEventRequest::Id },
                    { "RemoveLoadControlEventRequest", RemoveLoadControlEventRequest::Id },
                    { "ClearLoadControlEventsRequest", ClearLoadControlEventsRequest::Id }
                };
            } // namespace Commands
        } // namespace DemandResponseLoadControl
        namespace DeviceEnergyManagement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "PowerAdjustRequest", PowerAdjustRequest::Id },
                    { "CancelPowerAdjustRequest", CancelPowerAdjustRequest::Id },
                    { "StartTimeAdjustRequest", StartTimeAdjustRequest::Id },
                    { "PauseRequest", PauseRequest::Id },
                    { "ResumeRequest", ResumeRequest::Id },
                    { "ModifyForecastRequest", ModifyForecastRequest::Id },
                    { "RequestConstraintBasedForecast", RequestConstraintBasedForecast::Id }
                };
            } // namespace Commands
        } // namespace DeviceEnergyManagement
        namespace EnergyEvse {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "GetTargetsResponse", GetTargetsResponse::Id },
                    { "Disable", Disable::Id },
                    { "EnableCharging", EnableCharging::Id },
                    { "EnableDischarging", EnableDischarging::Id },
                    { "StartDiagnostics", StartDiagnostics::Id },
                    { "SetTargets", SetTargets::Id },
                    { "GetTargets", GetTargets::Id },
                    { "ClearTargets", ClearTargets::Id }
                };
            } // namespace Commands
        } // namespace EnergyEvse
        namespace EnergyPreference {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace EnergyPreference
        namespace DoorLock {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "LockDoor", LockDoor::Id },
                    { "UnlockDoor", UnlockDoor::Id },
                    { "UnlockWithTimeout", UnlockWithTimeout::Id },
                    { "SetWeekDaySchedule", SetWeekDaySchedule::Id },
                    { "GetWeekDaySchedule", GetWeekDaySchedule::Id },
                    { "GetWeekDayScheduleResponse", GetWeekDayScheduleResponse::Id },
                    { "ClearWeekDaySchedule", ClearWeekDaySchedule::Id },
                    { "SetYearDaySchedule", SetYearDaySchedule::Id },
                    { "GetYearDaySchedule", GetYearDaySchedule::Id },
                    { "GetYearDayScheduleResponse", GetYearDayScheduleResponse::Id },
                    { "ClearYearDaySchedule", ClearYearDaySchedule::Id },
                    { "SetHolidaySchedule", SetHolidaySchedule::Id },
                    { "GetHolidaySchedule", GetHolidaySchedule::Id },
                    { "GetHolidayScheduleResponse", GetHolidayScheduleResponse::Id },
                    { "ClearHolidaySchedule", ClearHolidaySchedule::Id },
                    { "SetUser", SetUser::Id },
                    { "GetUser", GetUser::Id },
                    { "GetUserResponse", GetUserResponse::Id },
                    { "ClearUser", ClearUser::Id },
                    { "SetCredential", SetCredential::Id },
                    { "SetCredentialResponse", SetCredentialResponse::Id },
                    { "GetCredentialStatus", GetCredentialStatus::Id },
                    { "GetCredentialStatusResponse", GetCredentialStatusResponse::Id },
                    { "ClearCredential", ClearCredential::Id },
                    { "UnboltDoor", UnboltDoor::Id }
                };
            } // namespace Commands
        } // namespace DoorLock
        namespace WindowCovering {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "UpOrOpen", UpOrOpen::Id },
                    { "DownOrClose", DownOrClose::Id },
                    { "StopMotion", StopMotion::Id },
                    { "GoToLiftValue", GoToLiftValue::Id },
                    { "GoToLiftPercentage", GoToLiftPercentage::Id },
                    { "GoToTiltValue", GoToTiltValue::Id },
                    { "GoToTiltPercentage", GoToTiltPercentage::Id }
                };
            } // namespace Commands
        } // namespace WindowCovering
        namespace BarrierControl {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "BarrierControlGoToPercent", BarrierControlGoToPercent::Id },
                    { "BarrierControlStop", BarrierControlStop::Id }
                };
            } // namespace Commands
        } // namespace BarrierControl
        namespace PumpConfigurationAndControl {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace PumpConfigurationAndControl
        namespace Thermostat {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "SetpointRaiseLower", SetpointRaiseLower::Id },
                    { "GetWeeklyScheduleResponse", GetWeeklyScheduleResponse::Id },
                    { "SetWeeklySchedule", SetWeeklySchedule::Id },
                    { "GetWeeklySchedule", GetWeeklySchedule::Id },
                    { "ClearWeeklySchedule", ClearWeeklySchedule::Id }
                };
            } // namespace Commands
        } // namespace Thermostat
        namespace FanControl {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "Step", Step::Id }
                };
            } // namespace Commands
        } // namespace FanControl
        namespace ThermostatUserInterfaceConfiguration {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace ThermostatUserInterfaceConfiguration
        namespace ColorControl {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "MoveToHue", MoveToHue::Id },
                    { "MoveHue", MoveHue::Id },
                    { "StepHue", StepHue::Id },
                    { "MoveToSaturation", MoveToSaturation::Id },
                    { "MoveSaturation", MoveSaturation::Id },
                    { "StepSaturation", StepSaturation::Id },
                    { "MoveToHueAndSaturation", MoveToHueAndSaturation::Id },
                    { "MoveToColor", MoveToColor::Id },
                    { "MoveColor", MoveColor::Id },
                    { "StepColor", StepColor::Id },
                    { "MoveToColorTemperature", MoveToColorTemperature::Id },
                    { "EnhancedMoveToHue", EnhancedMoveToHue::Id },
                    { "EnhancedMoveHue", EnhancedMoveHue::Id },
                    { "EnhancedStepHue", EnhancedStepHue::Id },
                    { "EnhancedMoveToHueAndSaturation", EnhancedMoveToHueAndSaturation::Id },
                    { "ColorLoopSet", ColorLoopSet::Id },
                    { "StopMoveStep", StopMoveStep::Id },
                    { "MoveColorTemperature", MoveColorTemperature::Id },
                    { "StepColorTemperature", StepColorTemperature::Id }
                };
            } // namespace Commands
        } // namespace ColorControl
        namespace BallastConfiguration {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace BallastConfiguration
        namespace IlluminanceMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace IlluminanceMeasurement
        namespace TemperatureMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace TemperatureMeasurement
        namespace PressureMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace PressureMeasurement
        namespace FlowMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace FlowMeasurement
        namespace RelativeHumidityMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace RelativeHumidityMeasurement
        namespace OccupancySensing {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace OccupancySensing
        namespace CarbonMonoxideConcentrationMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace CarbonMonoxideConcentrationMeasurement
        namespace CarbonDioxideConcentrationMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace CarbonDioxideConcentrationMeasurement
        namespace NitrogenDioxideConcentrationMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace NitrogenDioxideConcentrationMeasurement
        namespace OzoneConcentrationMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace OzoneConcentrationMeasurement
        namespace Pm25ConcentrationMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace Pm25ConcentrationMeasurement
        namespace FormaldehydeConcentrationMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace FormaldehydeConcentrationMeasurement
        namespace Pm1ConcentrationMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace Pm1ConcentrationMeasurement
        namespace Pm10ConcentrationMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace Pm10ConcentrationMeasurement
        namespace TotalVolatileOrganicCompoundsConcentrationMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace TotalVolatileOrganicCompoundsConcentrationMeasurement
        namespace RadonConcentrationMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace RadonConcentrationMeasurement
        namespace WakeOnLan {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace WakeOnLan
        namespace Channel {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ChangeChannel", ChangeChannel::Id },
                    { "ChangeChannelResponse", ChangeChannelResponse::Id },
                    { "ChangeChannelByNumber", ChangeChannelByNumber::Id },
                    { "SkipChannel", SkipChannel::Id },
                    { "GetProgramGuide", GetProgramGuide::Id },
                    { "ProgramGuideResponse", ProgramGuideResponse::Id },
                    { "RecordProgram", RecordProgram::Id },
                    { "CancelRecordProgram", CancelRecordProgram::Id }
                };
            } // namespace Commands
        } // namespace Channel
        namespace TargetNavigator {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "NavigateTarget", NavigateTarget::Id },
                    { "NavigateTargetResponse", NavigateTargetResponse::Id }
                };
            } // namespace Commands
        } // namespace TargetNavigator
        namespace MediaPlayback {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "Play", Play::Id },
                    { "Pause", Pause::Id },
                    { "Stop", Stop::Id },
                    { "StartOver", StartOver::Id },
                    { "Previous", Previous::Id },
                    { "Next", Next::Id },
                    { "Rewind", Rewind::Id },
                    { "FastForward", FastForward::Id },
                    { "SkipForward", SkipForward::Id },
                    { "SkipBackward", SkipBackward::Id },
                    { "PlaybackResponse", PlaybackResponse::Id },
                    { "Seek", Seek::Id },
                    { "ActivateAudioTrack", ActivateAudioTrack::Id },
                    { "ActivateTextTrack", ActivateTextTrack::Id },
                    { "DeactivateTextTrack", DeactivateTextTrack::Id }
                };
            } // namespace Commands
        } // namespace MediaPlayback
        namespace MediaInput {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "SelectInput", SelectInput::Id },
                    { "ShowInputStatus", ShowInputStatus::Id },
                    { "HideInputStatus", HideInputStatus::Id },
                    { "RenameInput", RenameInput::Id }
                };
            } // namespace Commands
        } // namespace MediaInput
        namespace LowPower {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "Sleep", Sleep::Id }
                };
            } // namespace Commands
        } // namespace LowPower
        namespace KeypadInput {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "SendKey", SendKey::Id },
                    { "SendKeyResponse", SendKeyResponse::Id }
                };
            } // namespace Commands
        } // namespace KeypadInput
        namespace ContentLauncher {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "LaunchContent", LaunchContent::Id },
                    { "LaunchURL", LaunchURL::Id },
                    { "LauncherResponse", LauncherResponse::Id }
                };
            } // namespace Commands
        } // namespace ContentLauncher
        namespace AudioOutput {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "SelectOutput", SelectOutput::Id },
                    { "RenameOutput", RenameOutput::Id }
                };
            } // namespace Commands
        } // namespace AudioOutput
        namespace ApplicationLauncher {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "LaunchApp", LaunchApp::Id },
                    { "StopApp", StopApp::Id },
                    { "HideApp", HideApp::Id },
                    { "LauncherResponse", LauncherResponse::Id }
                };
            } // namespace Commands
        } // namespace ApplicationLauncher
        namespace ApplicationBasic {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {};
            } // namespace Commands
        } // namespace ApplicationBasic
        namespace AccountLogin {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "GetSetupPIN", GetSetupPIN::Id },
                    { "GetSetupPINResponse", GetSetupPINResponse::Id },
                    { "Login", Login::Id },
                    { "Logout", Logout::Id }
                };
            } // namespace Commands
        } // namespace AccountLogin
        namespace ContentControl {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "UpdatePIN", UpdatePIN::Id },
                    { "ResetPIN", ResetPIN::Id },
                    { "ResetPINResponse", ResetPINResponse::Id },
                    { "Enable", Enable::Id },
                    { "Disable", Disable::Id },
                    { "AddBonusTime", AddBonusTime::Id },
                    { "SetScreenDailyTime", SetScreenDailyTime::Id },
                    { "BlockUnratedContent", BlockUnratedContent::Id },
                    { "UnblockUnratedContent", UnblockUnratedContent::Id },
                    { "SetOnDemandRatingThreshold", SetOnDemandRatingThreshold::Id },
                    { "SetScheduledContentRatingThreshold", SetScheduledContentRatingThreshold::Id }
                };
            } // namespace Commands
        } // namespace ContentControl
        namespace ContentAppObserver {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "ContentAppMessage", ContentAppMessage::Id },
                    { "ContentAppMessageResponse", ContentAppMessageResponse::Id }
                };
            } // namespace Commands
        } // namespace ContentAppObserver
        namespace ElectricalMeasurement {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "GetProfileInfoResponseCommand", GetProfileInfoResponseCommand::Id },
                    { "GetProfileInfoCommand", GetProfileInfoCommand::Id },
                    { "GetMeasurementProfileResponseCommand", GetMeasurementProfileResponseCommand::Id },
                    { "GetMeasurementProfileCommand", GetMeasurementProfileCommand::Id }
                };
            } // namespace Commands
        } // namespace ElectricalMeasurement
        namespace UnitTesting {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "Test", Test::Id },
                    { "TestSpecificResponse", TestSpecificResponse::Id },
                    { "TestNotHandled", TestNotHandled::Id },
                    { "TestAddArgumentsResponse", TestAddArgumentsResponse::Id },
                    { "TestSpecific", TestSpecific::Id },
                    { "TestSimpleArgumentResponse", TestSimpleArgumentResponse::Id },
                    { "TestUnknownCommand", TestUnknownCommand::Id },
                    { "TestStructArrayArgumentResponse", TestStructArrayArgumentResponse::Id },
                    { "TestAddArguments", TestAddArguments::Id },
                    { "TestListInt8UReverseResponse", TestListInt8UReverseResponse::Id },
                    { "TestSimpleArgumentRequest", TestSimpleArgumentRequest::Id },
                    { "TestEnumsResponse", TestEnumsResponse::Id },
                    { "TestStructArrayArgumentRequest", TestStructArrayArgumentRequest::Id },
                    { "TestNullableOptionalResponse", TestNullableOptionalResponse::Id },
                    { "TestStructArgumentRequest", TestStructArgumentRequest::Id },
                    { "TestComplexNullableOptionalResponse", TestComplexNullableOptionalResponse::Id },
                    { "TestNestedStructArgumentRequest", TestNestedStructArgumentRequest::Id },
                    { "BooleanResponse", BooleanResponse::Id },
                    { "TestListStructArgumentRequest", TestListStructArgumentRequest::Id },
                    { "SimpleStructResponse", SimpleStructResponse::Id },
                    { "TestListInt8UArgumentRequest", TestListInt8UArgumentRequest::Id },
                    { "TestEmitTestEventResponse", TestEmitTestEventResponse::Id },
                    { "TestNestedStructListArgumentRequest", TestNestedStructListArgumentRequest::Id },
                    { "TestEmitTestFabricScopedEventResponse", TestEmitTestFabricScopedEventResponse::Id },
                    { "TestListNestedStructListArgumentRequest", TestListNestedStructListArgumentRequest::Id },
                    { "TestListInt8UReverseRequest", TestListInt8UReverseRequest::Id },
                    { "TestEnumsRequest", TestEnumsRequest::Id },
                    { "TestNullableOptionalRequest", TestNullableOptionalRequest::Id },
                    { "TestComplexNullableOptionalRequest", TestComplexNullableOptionalRequest::Id },
                    { "SimpleStructEchoRequest", SimpleStructEchoRequest::Id },
                    { "TimedInvokeRequest", TimedInvokeRequest::Id },
                    { "TestSimpleOptionalArgumentRequest", TestSimpleOptionalArgumentRequest::Id },
                    { "TestEmitTestEventRequest", TestEmitTestEventRequest::Id },
                    { "TestEmitTestFabricScopedEventRequest", TestEmitTestFabricScopedEventRequest::Id }
                };
            } // namespace Commands
        } // namespace UnitTesting
        namespace FaultInjection {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "FailAtFault", FailAtFault::Id },
                    { "FailRandomlyAtFault", FailRandomlyAtFault::Id }
                };
            } // namespace Commands
        } // namespace FaultInjection
        namespace SampleMei {
            namespace Commands {
                static const std::unordered_map<std::string, chip::CommandId> matter_command_id_cluster_map {
                    { "Ping", Ping::Id },
                    { "AddArgumentsResponse", AddArgumentsResponse::Id },
                    { "AddArguments", AddArguments::Id }
                };
            } // namespace Commands
        } // namespace SampleMei

        // Global map of each clusters commands
        static const std::unordered_map<std::string, std::unordered_map<std::string, chip::CommandId>> matter_command_id_map {
            { "Identify", Identify::Commands::matter_command_id_cluster_map },
            { "Groups", Groups::Commands::matter_command_id_cluster_map },
            { "Scenes", Scenes::Commands::matter_command_id_cluster_map },
            { "OnOff", OnOff::Commands::matter_command_id_cluster_map },
            { "OnOffSwitchConfiguration", OnOffSwitchConfiguration::Commands::matter_command_id_cluster_map },
            { "LevelControl", LevelControl::Commands::matter_command_id_cluster_map },
            { "BinaryInputBasic", BinaryInputBasic::Commands::matter_command_id_cluster_map },
            { "PulseWidthModulation", PulseWidthModulation::Commands::matter_command_id_cluster_map },
            { "Descriptor", Descriptor::Commands::matter_command_id_cluster_map },
            { "Binding", Binding::Commands::matter_command_id_cluster_map },
            { "AccessControl", AccessControl::Commands::matter_command_id_cluster_map },
            { "Actions", Actions::Commands::matter_command_id_cluster_map },
            { "BasicInformation", BasicInformation::Commands::matter_command_id_cluster_map },
            { "OtaSoftwareUpdateProvider", OtaSoftwareUpdateProvider::Commands::matter_command_id_cluster_map },
            { "OtaSoftwareUpdateRequestor", OtaSoftwareUpdateRequestor::Commands::matter_command_id_cluster_map },
            { "LocalizationConfiguration", LocalizationConfiguration::Commands::matter_command_id_cluster_map },
            { "TimeFormatLocalization", TimeFormatLocalization::Commands::matter_command_id_cluster_map },
            { "UnitLocalization", UnitLocalization::Commands::matter_command_id_cluster_map },
            { "PowerSourceConfiguration", PowerSourceConfiguration::Commands::matter_command_id_cluster_map },
            { "PowerSource", PowerSource::Commands::matter_command_id_cluster_map },
            { "GeneralCommissioning", GeneralCommissioning::Commands::matter_command_id_cluster_map },
            { "NetworkCommissioning", NetworkCommissioning::Commands::matter_command_id_cluster_map },
            { "DiagnosticLogs", DiagnosticLogs::Commands::matter_command_id_cluster_map },
            { "GeneralDiagnostics", GeneralDiagnostics::Commands::matter_command_id_cluster_map },
            { "SoftwareDiagnostics", SoftwareDiagnostics::Commands::matter_command_id_cluster_map },
            { "ThreadNetworkDiagnostics", ThreadNetworkDiagnostics::Commands::matter_command_id_cluster_map },
            { "WiFiNetworkDiagnostics", WiFiNetworkDiagnostics::Commands::matter_command_id_cluster_map },
            { "EthernetNetworkDiagnostics", EthernetNetworkDiagnostics::Commands::matter_command_id_cluster_map },
            { "TimeSynchronization", TimeSynchronization::Commands::matter_command_id_cluster_map },
            { "BridgedDeviceBasicInformation", BridgedDeviceBasicInformation::Commands::matter_command_id_cluster_map },
            { "Switch", Switch::Commands::matter_command_id_cluster_map },
            { "AdministratorCommissioning", AdministratorCommissioning::Commands::matter_command_id_cluster_map },
            { "OperationalCredentials", OperationalCredentials::Commands::matter_command_id_cluster_map },
            { "GroupKeyManagement", GroupKeyManagement::Commands::matter_command_id_cluster_map },
            { "FixedLabel", FixedLabel::Commands::matter_command_id_cluster_map },
            { "UserLabel", UserLabel::Commands::matter_command_id_cluster_map },
            { "ProxyConfiguration", ProxyConfiguration::Commands::matter_command_id_cluster_map },
            { "ProxyDiscovery", ProxyDiscovery::Commands::matter_command_id_cluster_map },
            { "ProxyValid", ProxyValid::Commands::matter_command_id_cluster_map },
            { "BooleanState", BooleanState::Commands::matter_command_id_cluster_map },
            { "IcdManagement", IcdManagement::Commands::matter_command_id_cluster_map },
            { "Timer", Timer::Commands::matter_command_id_cluster_map },
            { "OvenCavityOperationalState", OvenCavityOperationalState::Commands::matter_command_id_cluster_map },
            { "OvenMode", OvenMode::Commands::matter_command_id_cluster_map },
            { "LaundryDryerControls", LaundryDryerControls::Commands::matter_command_id_cluster_map },
            { "ModeSelect", ModeSelect::Commands::matter_command_id_cluster_map },
            { "LaundryWasherMode", LaundryWasherMode::Commands::matter_command_id_cluster_map },
            { "RefrigeratorAndTemperatureControlledCabinetMode", RefrigeratorAndTemperatureControlledCabinetMode::Commands::matter_command_id_cluster_map },
            { "LaundryWasherControls", LaundryWasherControls::Commands::matter_command_id_cluster_map },
            { "RvcRunMode", RvcRunMode::Commands::matter_command_id_cluster_map },
            { "RvcCleanMode", RvcCleanMode::Commands::matter_command_id_cluster_map },
            { "TemperatureControl", TemperatureControl::Commands::matter_command_id_cluster_map },
            { "RefrigeratorAlarm", RefrigeratorAlarm::Commands::matter_command_id_cluster_map },
            { "DishwasherMode", DishwasherMode::Commands::matter_command_id_cluster_map },
            { "AirQuality", AirQuality::Commands::matter_command_id_cluster_map },
            { "SmokeCoAlarm", SmokeCoAlarm::Commands::matter_command_id_cluster_map },
            { "DishwasherAlarm", DishwasherAlarm::Commands::matter_command_id_cluster_map },
            { "MicrowaveOvenMode", MicrowaveOvenMode::Commands::matter_command_id_cluster_map },
            { "MicrowaveOvenControl", MicrowaveOvenControl::Commands::matter_command_id_cluster_map },
            { "OperationalState", OperationalState::Commands::matter_command_id_cluster_map },
            { "RvcOperationalState", RvcOperationalState::Commands::matter_command_id_cluster_map },
            { "HepaFilterMonitoring", HepaFilterMonitoring::Commands::matter_command_id_cluster_map },
            { "ActivatedCarbonFilterMonitoring", ActivatedCarbonFilterMonitoring::Commands::matter_command_id_cluster_map },
            { "BooleanStateConfiguration", BooleanStateConfiguration::Commands::matter_command_id_cluster_map },
            { "ValveConfigurationAndControl", ValveConfigurationAndControl::Commands::matter_command_id_cluster_map },
            { "ElectricalEnergyMeasurement", ElectricalEnergyMeasurement::Commands::matter_command_id_cluster_map },
            { "DemandResponseLoadControl", DemandResponseLoadControl::Commands::matter_command_id_cluster_map },
            { "DeviceEnergyManagement", DeviceEnergyManagement::Commands::matter_command_id_cluster_map },
            { "EnergyEvse", EnergyEvse::Commands::matter_command_id_cluster_map },
            { "EnergyPreference", EnergyPreference::Commands::matter_command_id_cluster_map },
            { "DoorLock", DoorLock::Commands::matter_command_id_cluster_map },
            { "WindowCovering", WindowCovering::Commands::matter_command_id_cluster_map },
            { "BarrierControl", BarrierControl::Commands::matter_command_id_cluster_map },
            { "PumpConfigurationAndControl", PumpConfigurationAndControl::Commands::matter_command_id_cluster_map },
            { "Thermostat", Thermostat::Commands::matter_command_id_cluster_map },
            { "FanControl", FanControl::Commands::matter_command_id_cluster_map },
            { "ThermostatUserInterfaceConfiguration", ThermostatUserInterfaceConfiguration::Commands::matter_command_id_cluster_map },
            { "ColorControl", ColorControl::Commands::matter_command_id_cluster_map },
            { "BallastConfiguration", BallastConfiguration::Commands::matter_command_id_cluster_map },
            { "IlluminanceMeasurement", IlluminanceMeasurement::Commands::matter_command_id_cluster_map },
            { "TemperatureMeasurement", TemperatureMeasurement::Commands::matter_command_id_cluster_map },
            { "PressureMeasurement", PressureMeasurement::Commands::matter_command_id_cluster_map },
            { "FlowMeasurement", FlowMeasurement::Commands::matter_command_id_cluster_map },
            { "RelativeHumidityMeasurement", RelativeHumidityMeasurement::Commands::matter_command_id_cluster_map },
            { "OccupancySensing", OccupancySensing::Commands::matter_command_id_cluster_map },
            { "CarbonMonoxideConcentrationMeasurement", CarbonMonoxideConcentrationMeasurement::Commands::matter_command_id_cluster_map },
            { "CarbonDioxideConcentrationMeasurement", CarbonDioxideConcentrationMeasurement::Commands::matter_command_id_cluster_map },
            { "NitrogenDioxideConcentrationMeasurement", NitrogenDioxideConcentrationMeasurement::Commands::matter_command_id_cluster_map },
            { "OzoneConcentrationMeasurement", OzoneConcentrationMeasurement::Commands::matter_command_id_cluster_map },
            { "Pm25ConcentrationMeasurement", Pm25ConcentrationMeasurement::Commands::matter_command_id_cluster_map },
            { "FormaldehydeConcentrationMeasurement", FormaldehydeConcentrationMeasurement::Commands::matter_command_id_cluster_map },
            { "Pm1ConcentrationMeasurement", Pm1ConcentrationMeasurement::Commands::matter_command_id_cluster_map },
            { "Pm10ConcentrationMeasurement", Pm10ConcentrationMeasurement::Commands::matter_command_id_cluster_map },
            { "TotalVolatileOrganicCompoundsConcentrationMeasurement", TotalVolatileOrganicCompoundsConcentrationMeasurement::Commands::matter_command_id_cluster_map },
            { "RadonConcentrationMeasurement", RadonConcentrationMeasurement::Commands::matter_command_id_cluster_map },
            { "WakeOnLan", WakeOnLan::Commands::matter_command_id_cluster_map },
            { "Channel", Channel::Commands::matter_command_id_cluster_map },
            { "TargetNavigator", TargetNavigator::Commands::matter_command_id_cluster_map },
            { "MediaPlayback", MediaPlayback::Commands::matter_command_id_cluster_map },
            { "MediaInput", MediaInput::Commands::matter_command_id_cluster_map },
            { "LowPower", LowPower::Commands::matter_command_id_cluster_map },
            { "KeypadInput", KeypadInput::Commands::matter_command_id_cluster_map },
            { "ContentLauncher", ContentLauncher::Commands::matter_command_id_cluster_map },
            { "AudioOutput", AudioOutput::Commands::matter_command_id_cluster_map },
            { "ApplicationLauncher", ApplicationLauncher::Commands::matter_command_id_cluster_map },
            { "ApplicationBasic", ApplicationBasic::Commands::matter_command_id_cluster_map },
            { "AccountLogin", AccountLogin::Commands::matter_command_id_cluster_map },
            { "ContentControl", ContentControl::Commands::matter_command_id_cluster_map },
            { "ContentAppObserver", ContentAppObserver::Commands::matter_command_id_cluster_map },
            { "ElectricalMeasurement", ElectricalMeasurement::Commands::matter_command_id_cluster_map },
            { "UnitTesting", UnitTesting::Commands::matter_command_id_cluster_map },
            { "FaultInjection", FaultInjection::Commands::matter_command_id_cluster_map },
            { "SampleMei", SampleMei::Commands::matter_command_id_cluster_map }
        };

    } // namespace Clusters
} // namespace app
} // namespace chip
