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

#import <Matter/MTRClusterNames.h>

// @implementation MTRClusterAttributeIDType

#pragma mark - Clusters IDs

NSString * MTRClusterNameForID(MTRClusterIDType clusterID)
{
    NSString * result = nil;

    switch (clusterID) {

    case MTRClusterIDTypeIdentifyID:
        result = @"Identify";
        break;
    case MTRClusterIDTypeGroupsID:
        result = @"Groups";
        break;
    case MTRClusterIDTypeOnOffID:
        result = @"OnOff";
        break;
    case MTRClusterIDTypeOnOffSwitchConfigurationID:
        result = @"OnOffSwitchConfiguration";
        break;
    case MTRClusterIDTypeLevelControlID:
        result = @"LevelControl";
        break;
    case MTRClusterIDTypeBinaryInputBasicID:
        result = @"BinaryInputBasic";
        break;
    case MTRClusterIDTypePulseWidthModulationID:
        result = @"PulseWidthModulation";
        break;
    case MTRClusterIDTypeDescriptorID:
        result = @"Descriptor";
        break;
    case MTRClusterIDTypeBindingID:
        result = @"Binding";
        break;
    case MTRClusterIDTypeAccessControlID:
        result = @"AccessControl";
        break;
    case MTRClusterIDTypeActionsID:
        result = @"Actions";
        break;
    case MTRClusterIDTypeBasicInformationID:
        result = @"BasicInformation";
        break;
    case MTRClusterIDTypeOTASoftwareUpdateProviderID:
        result = @"OTASoftwareUpdateProvider";
        break;
    case MTRClusterIDTypeOTASoftwareUpdateRequestorID:
        result = @"OTASoftwareUpdateRequestor";
        break;
    case MTRClusterIDTypeLocalizationConfigurationID:
        result = @"LocalizationConfiguration";
        break;
    case MTRClusterIDTypeTimeFormatLocalizationID:
        result = @"TimeFormatLocalization";
        break;
    case MTRClusterIDTypeUnitLocalizationID:
        result = @"UnitLocalization";
        break;
    case MTRClusterIDTypePowerSourceConfigurationID:
        result = @"PowerSourceConfiguration";
        break;
    case MTRClusterIDTypePowerSourceID:
        result = @"PowerSource";
        break;
    case MTRClusterIDTypeGeneralCommissioningID:
        result = @"GeneralCommissioning";
        break;
    case MTRClusterIDTypeNetworkCommissioningID:
        result = @"NetworkCommissioning";
        break;
    case MTRClusterIDTypeDiagnosticLogsID:
        result = @"DiagnosticLogs";
        break;
    case MTRClusterIDTypeGeneralDiagnosticsID:
        result = @"GeneralDiagnostics";
        break;
    case MTRClusterIDTypeSoftwareDiagnosticsID:
        result = @"SoftwareDiagnostics";
        break;
    case MTRClusterIDTypeThreadNetworkDiagnosticsID:
        result = @"ThreadNetworkDiagnostics";
        break;
    case MTRClusterIDTypeWiFiNetworkDiagnosticsID:
        result = @"WiFiNetworkDiagnostics";
        break;
    case MTRClusterIDTypeEthernetNetworkDiagnosticsID:
        result = @"EthernetNetworkDiagnostics";
        break;
    case MTRClusterIDTypeTimeSynchronizationID:
        result = @"TimeSynchronization";
        break;
    case MTRClusterIDTypeBridgedDeviceBasicInformationID:
        result = @"BridgedDeviceBasicInformation";
        break;
    case MTRClusterIDTypeSwitchID:
        result = @"Switch";
        break;
    case MTRClusterIDTypeAdministratorCommissioningID:
        result = @"AdministratorCommissioning";
        break;
    case MTRClusterIDTypeOperationalCredentialsID:
        result = @"OperationalCredentials";
        break;
    case MTRClusterIDTypeGroupKeyManagementID:
        result = @"GroupKeyManagement";
        break;
    case MTRClusterIDTypeFixedLabelID:
        result = @"FixedLabel";
        break;
    case MTRClusterIDTypeUserLabelID:
        result = @"UserLabel";
        break;
    case MTRClusterIDTypeBooleanStateID:
        result = @"BooleanState";
        break;
    case MTRClusterIDTypeICDManagementID:
        result = @"ICDManagement";
        break;
    case MTRClusterIDTypeTimerID:
        result = @"Timer";
        break;
    case MTRClusterIDTypeOvenCavityOperationalStateID:
        result = @"OvenCavityOperationalState";
        break;
    case MTRClusterIDTypeOvenModeID:
        result = @"OvenMode";
        break;
    case MTRClusterIDTypeLaundryDryerControlsID:
        result = @"LaundryDryerControls";
        break;
    case MTRClusterIDTypeModeSelectID:
        result = @"ModeSelect";
        break;
    case MTRClusterIDTypeLaundryWasherModeID:
        result = @"LaundryWasherMode";
        break;
    case MTRClusterIDTypeRefrigeratorAndTemperatureControlledCabinetModeID:
        result = @"RefrigeratorAndTemperatureControlledCabinetMode";
        break;
    case MTRClusterIDTypeLaundryWasherControlsID:
        result = @"LaundryWasherControls";
        break;
    case MTRClusterIDTypeRVCRunModeID:
        result = @"RVCRunMode";
        break;
    case MTRClusterIDTypeRVCCleanModeID:
        result = @"RVCCleanMode";
        break;
    case MTRClusterIDTypeTemperatureControlID:
        result = @"TemperatureControl";
        break;
    case MTRClusterIDTypeRefrigeratorAlarmID:
        result = @"RefrigeratorAlarm";
        break;
    case MTRClusterIDTypeDishwasherModeID:
        result = @"DishwasherMode";
        break;
    case MTRClusterIDTypeAirQualityID:
        result = @"AirQuality";
        break;
    case MTRClusterIDTypeSmokeCOAlarmID:
        result = @"SmokeCOAlarm";
        break;
    case MTRClusterIDTypeDishwasherAlarmID:
        result = @"DishwasherAlarm";
        break;
    case MTRClusterIDTypeMicrowaveOvenModeID:
        result = @"MicrowaveOvenMode";
        break;
    case MTRClusterIDTypeMicrowaveOvenControlID:
        result = @"MicrowaveOvenControl";
        break;
    case MTRClusterIDTypeOperationalStateID:
        result = @"OperationalState";
        break;
    case MTRClusterIDTypeRVCOperationalStateID:
        result = @"RVCOperationalState";
        break;
    case MTRClusterIDTypeScenesManagementID:
        result = @"ScenesManagement";
        break;
    case MTRClusterIDTypeHEPAFilterMonitoringID:
        result = @"HEPAFilterMonitoring";
        break;
    case MTRClusterIDTypeActivatedCarbonFilterMonitoringID:
        result = @"ActivatedCarbonFilterMonitoring";
        break;
    case MTRClusterIDTypeBooleanStateConfigurationID:
        result = @"BooleanStateConfiguration";
        break;
    case MTRClusterIDTypeValveConfigurationAndControlID:
        result = @"ValveConfigurationAndControl";
        break;
    case MTRClusterIDTypeElectricalPowerMeasurementID:
        result = @"ElectricalPowerMeasurement";
        break;
    case MTRClusterIDTypeElectricalEnergyMeasurementID:
        result = @"ElectricalEnergyMeasurement";
        break;
    case MTRClusterIDTypeDemandResponseLoadControlID:
        result = @"DemandResponseLoadControl";
        break;
    case MTRClusterIDTypeMessagesID:
        result = @"Messages";
        break;
    case MTRClusterIDTypeDeviceEnergyManagementID:
        result = @"DeviceEnergyManagement";
        break;
    case MTRClusterIDTypeEnergyEVSEID:
        result = @"EnergyEVSE";
        break;
    case MTRClusterIDTypeEnergyPreferenceID:
        result = @"EnergyPreference";
        break;
    case MTRClusterIDTypePowerTopologyID:
        result = @"PowerTopology";
        break;
    case MTRClusterIDTypeEnergyEVSEModeID:
        result = @"EnergyEVSEMode";
        break;
    case MTRClusterIDTypeDeviceEnergyManagementModeID:
        result = @"DeviceEnergyManagementMode";
        break;
    case MTRClusterIDTypeDoorLockID:
        result = @"DoorLock";
        break;
    case MTRClusterIDTypeWindowCoveringID:
        result = @"WindowCovering";
        break;
    case MTRClusterIDTypeBarrierControlID:
        result = @"BarrierControl";
        break;
    case MTRClusterIDTypePumpConfigurationAndControlID:
        result = @"PumpConfigurationAndControl";
        break;
    case MTRClusterIDTypeThermostatID:
        result = @"Thermostat";
        break;
    case MTRClusterIDTypeFanControlID:
        result = @"FanControl";
        break;
    case MTRClusterIDTypeThermostatUserInterfaceConfigurationID:
        result = @"ThermostatUserInterfaceConfiguration";
        break;
    case MTRClusterIDTypeColorControlID:
        result = @"ColorControl";
        break;
    case MTRClusterIDTypeBallastConfigurationID:
        result = @"BallastConfiguration";
        break;
    case MTRClusterIDTypeIlluminanceMeasurementID:
        result = @"IlluminanceMeasurement";
        break;
    case MTRClusterIDTypeTemperatureMeasurementID:
        result = @"TemperatureMeasurement";
        break;
    case MTRClusterIDTypePressureMeasurementID:
        result = @"PressureMeasurement";
        break;
    case MTRClusterIDTypeFlowMeasurementID:
        result = @"FlowMeasurement";
        break;
    case MTRClusterIDTypeRelativeHumidityMeasurementID:
        result = @"RelativeHumidityMeasurement";
        break;
    case MTRClusterIDTypeOccupancySensingID:
        result = @"OccupancySensing";
        break;
    case MTRClusterIDTypeCarbonMonoxideConcentrationMeasurementID:
        result = @"CarbonMonoxideConcentrationMeasurement";
        break;
    case MTRClusterIDTypeCarbonDioxideConcentrationMeasurementID:
        result = @"CarbonDioxideConcentrationMeasurement";
        break;
    case MTRClusterIDTypeNitrogenDioxideConcentrationMeasurementID:
        result = @"NitrogenDioxideConcentrationMeasurement";
        break;
    case MTRClusterIDTypeOzoneConcentrationMeasurementID:
        result = @"OzoneConcentrationMeasurement";
        break;
    case MTRClusterIDTypePM25ConcentrationMeasurementID:
        result = @"PM25ConcentrationMeasurement";
        break;
    case MTRClusterIDTypeFormaldehydeConcentrationMeasurementID:
        result = @"FormaldehydeConcentrationMeasurement";
        break;
    case MTRClusterIDTypePM1ConcentrationMeasurementID:
        result = @"PM1ConcentrationMeasurement";
        break;
    case MTRClusterIDTypePM10ConcentrationMeasurementID:
        result = @"PM10ConcentrationMeasurement";
        break;
    case MTRClusterIDTypeTotalVolatileOrganicCompoundsConcentrationMeasurementID:
        result = @"TotalVolatileOrganicCompoundsConcentrationMeasurement";
        break;
    case MTRClusterIDTypeRadonConcentrationMeasurementID:
        result = @"RadonConcentrationMeasurement";
        break;
    case MTRClusterIDTypeWakeOnLANID:
        result = @"WakeOnLAN";
        break;
    case MTRClusterIDTypeChannelID:
        result = @"Channel";
        break;
    case MTRClusterIDTypeTargetNavigatorID:
        result = @"TargetNavigator";
        break;
    case MTRClusterIDTypeMediaPlaybackID:
        result = @"MediaPlayback";
        break;
    case MTRClusterIDTypeMediaInputID:
        result = @"MediaInput";
        break;
    case MTRClusterIDTypeLowPowerID:
        result = @"LowPower";
        break;
    case MTRClusterIDTypeKeypadInputID:
        result = @"KeypadInput";
        break;
    case MTRClusterIDTypeContentLauncherID:
        result = @"ContentLauncher";
        break;
    case MTRClusterIDTypeAudioOutputID:
        result = @"AudioOutput";
        break;
    case MTRClusterIDTypeApplicationLauncherID:
        result = @"ApplicationLauncher";
        break;
    case MTRClusterIDTypeApplicationBasicID:
        result = @"ApplicationBasic";
        break;
    case MTRClusterIDTypeAccountLoginID:
        result = @"AccountLogin";
        break;
    case MTRClusterIDTypeContentControlID:
        result = @"ContentControl";
        break;
    case MTRClusterIDTypeContentAppObserverID:
        result = @"ContentAppObserver";
        break;
    case MTRClusterIDTypeElectricalMeasurementID:
        result = @"ElectricalMeasurement";
        break;
    case MTRClusterIDTypeUnitTestingID:
        result = @"UnitTesting";
        break;
    case MTRClusterIDTypeSampleMEIID:
        result = @"SampleMEI";
        break;

    default:
        result = [NSString stringWithFormat:@"<Unknown clusterID %d>", clusterID];
        break;
    }

    return result;
}

#pragma mark - Attributes IDs

NSString * MTRAttributeNameForID(MTRClusterIDType clusterID, MTRAttributeIDType attributeID)
{
    NSString * result = nil;

    switch (clusterID) {

    case MTRClusterIDTypeIdentifyID:

        switch (attributeID) {

            // Cluster Identify attributes
        case MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTimeID:
            result = @"IdentifyTime";
            break;

        case MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTypeID:
            result = @"IdentifyType";
            break;

        case MTRAttributeIDTypeClusterIdentifyAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterIdentifyAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterIdentifyAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterIdentifyAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterIdentifyAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterIdentifyAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeGroupsID:

        switch (attributeID) {

            // Cluster Groups attributes
        case MTRAttributeIDTypeClusterGroupsAttributeNameSupportID:
            result = @"NameSupport";
            break;

        case MTRAttributeIDTypeClusterGroupsAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterGroupsAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterGroupsAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterGroupsAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterGroupsAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterGroupsAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeOnOffID:

        switch (attributeID) {

            // Cluster OnOff attributes
        case MTRAttributeIDTypeClusterOnOffAttributeOnOffID:
            result = @"OnOff";
            break;

        case MTRAttributeIDTypeClusterOnOffAttributeGlobalSceneControlID:
            result = @"GlobalSceneControl";
            break;

        case MTRAttributeIDTypeClusterOnOffAttributeOnTimeID:
            result = @"OnTime";
            break;

        case MTRAttributeIDTypeClusterOnOffAttributeOffWaitTimeID:
            result = @"OffWaitTime";
            break;

        case MTRAttributeIDTypeClusterOnOffAttributeStartUpOnOffID:
            result = @"StartUpOnOff";
            break;

        case MTRAttributeIDTypeClusterOnOffAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterOnOffAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterOnOffAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterOnOffAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterOnOffAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterOnOffAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeOnOffSwitchConfigurationID:

        switch (attributeID) {

            // Cluster OnOffSwitchConfiguration attributes
        case MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchTypeID:
            result = @"SwitchType";
            break;

        case MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchActionsID:
            result = @"SwitchActions";
            break;

        case MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeLevelControlID:

        switch (attributeID) {

            // Cluster LevelControl attributes
        case MTRAttributeIDTypeClusterLevelControlAttributeCurrentLevelID:
            result = @"CurrentLevel";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeRemainingTimeID:
            result = @"RemainingTime";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeMinLevelID:
            result = @"MinLevel";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeMaxLevelID:
            result = @"MaxLevel";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeCurrentFrequencyID:
            result = @"CurrentFrequency";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeMinFrequencyID:
            result = @"MinFrequency";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeMaxFrequencyID:
            result = @"MaxFrequency";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeOptionsID:
            result = @"Options";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeOnOffTransitionTimeID:
            result = @"OnOffTransitionTime";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeOnLevelID:
            result = @"OnLevel";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeOnTransitionTimeID:
            result = @"OnTransitionTime";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeOffTransitionTimeID:
            result = @"OffTransitionTime";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeDefaultMoveRateID:
            result = @"DefaultMoveRate";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeStartUpCurrentLevelID:
            result = @"StartUpCurrentLevel";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterLevelControlAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeBinaryInputBasicID:

        switch (attributeID) {

            // Cluster BinaryInputBasic attributes
        case MTRAttributeIDTypeClusterBinaryInputBasicAttributeActiveTextID:
            result = @"ActiveText";
            break;

        case MTRAttributeIDTypeClusterBinaryInputBasicAttributeDescriptionID:
            result = @"Description";
            break;

        case MTRAttributeIDTypeClusterBinaryInputBasicAttributeInactiveTextID:
            result = @"InactiveText";
            break;

        case MTRAttributeIDTypeClusterBinaryInputBasicAttributeOutOfServiceID:
            result = @"OutOfService";
            break;

        case MTRAttributeIDTypeClusterBinaryInputBasicAttributePolarityID:
            result = @"Polarity";
            break;

        case MTRAttributeIDTypeClusterBinaryInputBasicAttributePresentValueID:
            result = @"PresentValue";
            break;

        case MTRAttributeIDTypeClusterBinaryInputBasicAttributeReliabilityID:
            result = @"Reliability";
            break;

        case MTRAttributeIDTypeClusterBinaryInputBasicAttributeStatusFlagsID:
            result = @"StatusFlags";
            break;

        case MTRAttributeIDTypeClusterBinaryInputBasicAttributeApplicationTypeID:
            result = @"ApplicationType";
            break;

        case MTRAttributeIDTypeClusterBinaryInputBasicAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterBinaryInputBasicAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterBinaryInputBasicAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterBinaryInputBasicAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterBinaryInputBasicAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterBinaryInputBasicAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypePulseWidthModulationID:

        switch (attributeID) {

            // Cluster PulseWidthModulation attributes
        case MTRAttributeIDTypeClusterPulseWidthModulationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterPulseWidthModulationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterPulseWidthModulationAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterPulseWidthModulationAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterPulseWidthModulationAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterPulseWidthModulationAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeDescriptorID:

        switch (attributeID) {

            // Cluster Descriptor attributes
        case MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID:
            result = @"DeviceTypeList";
            break;

        case MTRAttributeIDTypeClusterDescriptorAttributeServerListID:
            result = @"ServerList";
            break;

        case MTRAttributeIDTypeClusterDescriptorAttributeClientListID:
            result = @"ClientList";
            break;

        case MTRAttributeIDTypeClusterDescriptorAttributePartsListID:
            result = @"PartsList";
            break;

        case MTRAttributeIDTypeClusterDescriptorAttributeTagListID:
            result = @"TagList";
            break;

        case MTRAttributeIDTypeClusterDescriptorAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterDescriptorAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterDescriptorAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterDescriptorAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterDescriptorAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterDescriptorAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeBindingID:

        switch (attributeID) {

            // Cluster Binding attributes
        case MTRAttributeIDTypeClusterBindingAttributeBindingID:
            result = @"Binding";
            break;

        case MTRAttributeIDTypeClusterBindingAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterBindingAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterBindingAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterBindingAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterBindingAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterBindingAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeAccessControlID:

        switch (attributeID) {

            // Cluster AccessControl attributes
        case MTRAttributeIDTypeClusterAccessControlAttributeACLID:
            result = @"ACL";
            break;

        case MTRAttributeIDTypeClusterAccessControlAttributeExtensionID:
            result = @"Extension";
            break;

        case MTRAttributeIDTypeClusterAccessControlAttributeSubjectsPerAccessControlEntryID:
            result = @"SubjectsPerAccessControlEntry";
            break;

        case MTRAttributeIDTypeClusterAccessControlAttributeTargetsPerAccessControlEntryID:
            result = @"TargetsPerAccessControlEntry";
            break;

        case MTRAttributeIDTypeClusterAccessControlAttributeAccessControlEntriesPerFabricID:
            result = @"AccessControlEntriesPerFabric";
            break;

        case MTRAttributeIDTypeClusterAccessControlAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterAccessControlAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterAccessControlAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterAccessControlAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterAccessControlAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterAccessControlAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeActionsID:

        switch (attributeID) {

            // Cluster Actions attributes
        case MTRAttributeIDTypeClusterActionsAttributeActionListID:
            result = @"ActionList";
            break;

        case MTRAttributeIDTypeClusterActionsAttributeEndpointListsID:
            result = @"EndpointLists";
            break;

        case MTRAttributeIDTypeClusterActionsAttributeSetupURLID:
            result = @"SetupURL";
            break;

        case MTRAttributeIDTypeClusterActionsAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterActionsAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterActionsAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterActionsAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterActionsAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterActionsAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeBasicInformationID:

        switch (attributeID) {

            // Cluster BasicInformation attributes
        case MTRAttributeIDTypeClusterBasicInformationAttributeDataModelRevisionID:
            result = @"DataModelRevision";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeVendorNameID:
            result = @"VendorName";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeVendorIDID:
            result = @"VendorID";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeProductNameID:
            result = @"ProductName";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeProductIDID:
            result = @"ProductID";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeNodeLabelID:
            result = @"NodeLabel";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeLocationID:
            result = @"Location";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeHardwareVersionID:
            result = @"HardwareVersion";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeHardwareVersionStringID:
            result = @"HardwareVersionString";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeSoftwareVersionID:
            result = @"SoftwareVersion";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeSoftwareVersionStringID:
            result = @"SoftwareVersionString";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeManufacturingDateID:
            result = @"ManufacturingDate";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributePartNumberID:
            result = @"PartNumber";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeProductURLID:
            result = @"ProductURL";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeProductLabelID:
            result = @"ProductLabel";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeSerialNumberID:
            result = @"SerialNumber";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeLocalConfigDisabledID:
            result = @"LocalConfigDisabled";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeReachableID:
            result = @"Reachable";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeUniqueIDID:
            result = @"UniqueID";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeCapabilityMinimaID:
            result = @"CapabilityMinima";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeProductAppearanceID:
            result = @"ProductAppearance";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeSpecificationVersionID:
            result = @"SpecificationVersion";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeMaxPathsPerInvokeID:
            result = @"MaxPathsPerInvoke";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeOTASoftwareUpdateProviderID:

        switch (attributeID) {

            // Cluster OTASoftwareUpdateProvider attributes
        case MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeOTASoftwareUpdateRequestorID:

        switch (attributeID) {

            // Cluster OTASoftwareUpdateRequestor attributes
        case MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeDefaultOTAProvidersID:
            result = @"DefaultOTAProviders";
            break;

        case MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdatePossibleID:
            result = @"UpdatePossible";
            break;

        case MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateID:
            result = @"UpdateState";
            break;

        case MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateProgressID:
            result = @"UpdateStateProgress";
            break;

        case MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeLocalizationConfigurationID:

        switch (attributeID) {

            // Cluster LocalizationConfiguration attributes
        case MTRAttributeIDTypeClusterLocalizationConfigurationAttributeActiveLocaleID:
            result = @"ActiveLocale";
            break;

        case MTRAttributeIDTypeClusterLocalizationConfigurationAttributeSupportedLocalesID:
            result = @"SupportedLocales";
            break;

        case MTRAttributeIDTypeClusterLocalizationConfigurationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterLocalizationConfigurationAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterLocalizationConfigurationAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterLocalizationConfigurationAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeTimeFormatLocalizationID:

        switch (attributeID) {

            // Cluster TimeFormatLocalization attributes
        case MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeHourFormatID:
            result = @"HourFormat";
            break;

        case MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeActiveCalendarTypeID:
            result = @"ActiveCalendarType";
            break;

        case MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeSupportedCalendarTypesID:
            result = @"SupportedCalendarTypes";
            break;

        case MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeUnitLocalizationID:

        switch (attributeID) {

            // Cluster UnitLocalization attributes
        case MTRAttributeIDTypeClusterUnitLocalizationAttributeTemperatureUnitID:
            result = @"TemperatureUnit";
            break;

        case MTRAttributeIDTypeClusterUnitLocalizationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterUnitLocalizationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterUnitLocalizationAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterUnitLocalizationAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterUnitLocalizationAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterUnitLocalizationAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypePowerSourceConfigurationID:

        switch (attributeID) {

            // Cluster PowerSourceConfiguration attributes
        case MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeSourcesID:
            result = @"Sources";
            break;

        case MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypePowerSourceID:

        switch (attributeID) {

            // Cluster PowerSource attributes
        case MTRAttributeIDTypeClusterPowerSourceAttributeStatusID:
            result = @"Status";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeOrderID:
            result = @"Order";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeDescriptionID:
            result = @"Description";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputVoltageID:
            result = @"WiredAssessedInputVoltage";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputFrequencyID:
            result = @"WiredAssessedInputFrequency";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeWiredCurrentTypeID:
            result = @"WiredCurrentType";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedCurrentID:
            result = @"WiredAssessedCurrent";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeWiredNominalVoltageID:
            result = @"WiredNominalVoltage";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeWiredMaximumCurrentID:
            result = @"WiredMaximumCurrent";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeWiredPresentID:
            result = @"WiredPresent";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeActiveWiredFaultsID:
            result = @"ActiveWiredFaults";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatVoltageID:
            result = @"BatVoltage";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatPercentRemainingID:
            result = @"BatPercentRemaining";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeRemainingID:
            result = @"BatTimeRemaining";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeLevelID:
            result = @"BatChargeLevel";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementNeededID:
            result = @"BatReplacementNeeded";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatReplaceabilityID:
            result = @"BatReplaceability";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatPresentID:
            result = @"BatPresent";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatFaultsID:
            result = @"ActiveBatFaults";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementDescriptionID:
            result = @"BatReplacementDescription";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatCommonDesignationID:
            result = @"BatCommonDesignation";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatANSIDesignationID:
            result = @"BatANSIDesignation";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatIECDesignationID:
            result = @"BatIECDesignation";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatApprovedChemistryID:
            result = @"BatApprovedChemistry";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatCapacityID:
            result = @"BatCapacity";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatQuantityID:
            result = @"BatQuantity";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeStateID:
            result = @"BatChargeState";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeToFullChargeID:
            result = @"BatTimeToFullCharge";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatFunctionalWhileChargingID:
            result = @"BatFunctionalWhileCharging";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeBatChargingCurrentID:
            result = @"BatChargingCurrent";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatChargeFaultsID:
            result = @"ActiveBatChargeFaults";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeEndpointListID:
            result = @"EndpointList";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterPowerSourceAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeGeneralCommissioningID:

        switch (attributeID) {

            // Cluster GeneralCommissioning attributes
        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeBreadcrumbID:
            result = @"Breadcrumb";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeBasicCommissioningInfoID:
            result = @"BasicCommissioningInfo";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeRegulatoryConfigID:
            result = @"RegulatoryConfig";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeLocationCapabilityID:
            result = @"LocationCapability";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeSupportsConcurrentConnectionID:
            result = @"SupportsConcurrentConnection";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeNetworkCommissioningID:

        switch (attributeID) {

            // Cluster NetworkCommissioning attributes
        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeMaxNetworksID:
            result = @"MaxNetworks";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeNetworksID:
            result = @"Networks";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeScanMaxTimeSecondsID:
            result = @"ScanMaxTimeSeconds";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeConnectMaxTimeSecondsID:
            result = @"ConnectMaxTimeSeconds";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeInterfaceEnabledID:
            result = @"InterfaceEnabled";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkingStatusID:
            result = @"LastNetworkingStatus";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkIDID:
            result = @"LastNetworkID";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastConnectErrorValueID:
            result = @"LastConnectErrorValue";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeSupportedWiFiBandsID:
            result = @"SupportedWiFiBands";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeSupportedThreadFeaturesID:
            result = @"SupportedThreadFeatures";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeThreadVersionID:
            result = @"ThreadVersion";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterNetworkCommissioningAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeDiagnosticLogsID:

        switch (attributeID) {

            // Cluster DiagnosticLogs attributes
        case MTRAttributeIDTypeClusterDiagnosticLogsAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterDiagnosticLogsAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterDiagnosticLogsAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterDiagnosticLogsAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterDiagnosticLogsAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterDiagnosticLogsAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeGeneralDiagnosticsID:

        switch (attributeID) {

            // Cluster GeneralDiagnostics attributes
        case MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeNetworkInterfacesID:
            result = @"NetworkInterfaces";
            break;

        case MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeRebootCountID:
            result = @"RebootCount";
            break;

        case MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeUpTimeID:
            result = @"UpTime";
            break;

        case MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTotalOperationalHoursID:
            result = @"TotalOperationalHours";
            break;

        case MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeBootReasonID:
            result = @"BootReason";
            break;

        case MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveHardwareFaultsID:
            result = @"ActiveHardwareFaults";
            break;

        case MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveRadioFaultsID:
            result = @"ActiveRadioFaults";
            break;

        case MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveNetworkFaultsID:
            result = @"ActiveNetworkFaults";
            break;

        case MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTestEventTriggersEnabledID:
            result = @"TestEventTriggersEnabled";
            break;

        case MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeSoftwareDiagnosticsID:

        switch (attributeID) {

            // Cluster SoftwareDiagnostics attributes
        case MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeThreadMetricsID:
            result = @"ThreadMetrics";
            break;

        case MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapFreeID:
            result = @"CurrentHeapFree";
            break;

        case MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapUsedID:
            result = @"CurrentHeapUsed";
            break;

        case MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapHighWatermarkID:
            result = @"CurrentHeapHighWatermark";
            break;

        case MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeThreadNetworkDiagnosticsID:

        switch (attributeID) {

            // Cluster ThreadNetworkDiagnostics attributes
        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelID:
            result = @"Channel";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRoutingRoleID:
            result = @"RoutingRole";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNetworkNameID:
            result = @"NetworkName";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePanIdID:
            result = @"PanId";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeExtendedPanIdID:
            result = @"ExtendedPanId";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeMeshLocalPrefixID:
            result = @"MeshLocalPrefix";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOverrunCountID:
            result = @"OverrunCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNeighborTableID:
            result = @"NeighborTable";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouteTableID:
            result = @"RouteTable";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdID:
            result = @"PartitionId";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeWeightingID:
            result = @"Weighting";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDataVersionID:
            result = @"DataVersion";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeStableDataVersionID:
            result = @"StableDataVersion";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRouterIdID:
            result = @"LeaderRouterId";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDetachedRoleCountID:
            result = @"DetachedRoleCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChildRoleCountID:
            result = @"ChildRoleCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouterRoleCountID:
            result = @"RouterRoleCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRoleCountID:
            result = @"LeaderRoleCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttachAttemptCountID:
            result = @"AttachAttemptCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdChangeCountID:
            result = @"PartitionIdChangeCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeBetterPartitionAttachAttemptCountID:
            result = @"BetterPartitionAttachAttemptCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeParentChangeCountID:
            result = @"ParentChangeCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxTotalCountID:
            result = @"TxTotalCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxUnicastCountID:
            result = @"TxUnicastCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBroadcastCountID:
            result = @"TxBroadcastCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckRequestedCountID:
            result = @"TxAckRequestedCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckedCountID:
            result = @"TxAckedCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxNoAckRequestedCountID:
            result = @"TxNoAckRequestedCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataCountID:
            result = @"TxDataCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataPollCountID:
            result = @"TxDataPollCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconCountID:
            result = @"TxBeaconCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconRequestCountID:
            result = @"TxBeaconRequestCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxOtherCountID:
            result = @"TxOtherCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxRetryCountID:
            result = @"TxRetryCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDirectMaxRetryExpiryCountID:
            result = @"TxDirectMaxRetryExpiryCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxIndirectMaxRetryExpiryCountID:
            result = @"TxIndirectMaxRetryExpiryCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrCcaCountID:
            result = @"TxErrCcaCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrAbortCountID:
            result = @"TxErrAbortCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrBusyChannelCountID:
            result = @"TxErrBusyChannelCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxTotalCountID:
            result = @"RxTotalCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxUnicastCountID:
            result = @"RxUnicastCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBroadcastCountID:
            result = @"RxBroadcastCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataCountID:
            result = @"RxDataCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataPollCountID:
            result = @"RxDataPollCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconCountID:
            result = @"RxBeaconCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconRequestCountID:
            result = @"RxBeaconRequestCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxOtherCountID:
            result = @"RxOtherCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxAddressFilteredCountID:
            result = @"RxAddressFilteredCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDestAddrFilteredCountID:
            result = @"RxDestAddrFilteredCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDuplicatedCountID:
            result = @"RxDuplicatedCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrNoFrameCountID:
            result = @"RxErrNoFrameCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrUnknownNeighborCountID:
            result = @"RxErrUnknownNeighborCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrInvalidSrcAddrCountID:
            result = @"RxErrInvalidSrcAddrCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrSecCountID:
            result = @"RxErrSecCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrFcsCountID:
            result = @"RxErrFcsCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrOtherCountID:
            result = @"RxErrOtherCount";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveTimestampID:
            result = @"ActiveTimestamp";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePendingTimestampID:
            result = @"PendingTimestamp";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDelayID:
            result = @"Delay";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeSecurityPolicyID:
            result = @"SecurityPolicy";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelPage0MaskID:
            result = @"ChannelPage0Mask";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOperationalDatasetComponentsID:
            result = @"OperationalDatasetComponents";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveNetworkFaultsListID:
            result = @"ActiveNetworkFaultsList";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeWiFiNetworkDiagnosticsID:

        switch (attributeID) {

            // Cluster WiFiNetworkDiagnostics attributes
        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBSSIDID:
            result = @"BSSID";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeSecurityTypeID:
            result = @"SecurityType";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeWiFiVersionID:
            result = @"WiFiVersion";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeChannelNumberID:
            result = @"ChannelNumber";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeRSSIID:
            result = @"RSSI";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconLostCountID:
            result = @"BeaconLostCount";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconRxCountID:
            result = @"BeaconRxCount";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastRxCountID:
            result = @"PacketMulticastRxCount";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastTxCountID:
            result = @"PacketMulticastTxCount";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastRxCountID:
            result = @"PacketUnicastRxCount";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastTxCountID:
            result = @"PacketUnicastTxCount";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeCurrentMaxRateID:
            result = @"CurrentMaxRate";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeOverrunCountID:
            result = @"OverrunCount";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeEthernetNetworkDiagnosticsID:

        switch (attributeID) {

            // Cluster EthernetNetworkDiagnostics attributes
        case MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePHYRateID:
            result = @"PHYRate";
            break;

        case MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFullDuplexID:
            result = @"FullDuplex";
            break;

        case MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketRxCountID:
            result = @"PacketRxCount";
            break;

        case MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketTxCountID:
            result = @"PacketTxCount";
            break;

        case MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTxErrCountID:
            result = @"TxErrCount";
            break;

        case MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCollisionCountID:
            result = @"CollisionCount";
            break;

        case MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeOverrunCountID:
            result = @"OverrunCount";
            break;

        case MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCarrierDetectID:
            result = @"CarrierDetect";
            break;

        case MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTimeSinceResetID:
            result = @"TimeSinceReset";
            break;

        case MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeTimeSynchronizationID:

        switch (attributeID) {

            // Cluster TimeSynchronization attributes
        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeUTCTimeID:
            result = @"UTCTime";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeGranularityID:
            result = @"Granularity";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeSourceID:
            result = @"TimeSource";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeTrustedTimeSourceID:
            result = @"TrustedTimeSource";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeDefaultNTPID:
            result = @"DefaultNTP";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneID:
            result = @"TimeZone";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeDSTOffsetID:
            result = @"DSTOffset";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeLocalTimeID:
            result = @"LocalTime";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneDatabaseID:
            result = @"TimeZoneDatabase";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeNTPServerAvailableID:
            result = @"NTPServerAvailable";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneListMaxSizeID:
            result = @"TimeZoneListMaxSize";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeDSTOffsetListMaxSizeID:
            result = @"DSTOffsetListMaxSize";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeSupportsDNSResolveID:
            result = @"SupportsDNSResolve";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterTimeSynchronizationAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeBridgedDeviceBasicInformationID:

        switch (attributeID) {

            // Cluster BridgedDeviceBasicInformation attributes
        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeVendorNameID:
            result = @"VendorName";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeVendorIDID:
            result = @"VendorID";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductNameID:
            result = @"ProductName";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeNodeLabelID:
            result = @"NodeLabel";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeHardwareVersionID:
            result = @"HardwareVersion";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeHardwareVersionStringID:
            result = @"HardwareVersionString";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSoftwareVersionID:
            result = @"SoftwareVersion";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSoftwareVersionStringID:
            result = @"SoftwareVersionString";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeManufacturingDateID:
            result = @"ManufacturingDate";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributePartNumberID:
            result = @"PartNumber";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductURLID:
            result = @"ProductURL";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductLabelID:
            result = @"ProductLabel";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSerialNumberID:
            result = @"SerialNumber";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeReachableID:
            result = @"Reachable";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeUniqueIDID:
            result = @"UniqueID";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductAppearanceID:
            result = @"ProductAppearance";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeSwitchID:

        switch (attributeID) {

            // Cluster Switch attributes
        case MTRAttributeIDTypeClusterSwitchAttributeNumberOfPositionsID:
            result = @"NumberOfPositions";
            break;

        case MTRAttributeIDTypeClusterSwitchAttributeCurrentPositionID:
            result = @"CurrentPosition";
            break;

        case MTRAttributeIDTypeClusterSwitchAttributeMultiPressMaxID:
            result = @"MultiPressMax";
            break;

        case MTRAttributeIDTypeClusterSwitchAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterSwitchAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterSwitchAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterSwitchAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterSwitchAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterSwitchAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeAdministratorCommissioningID:

        switch (attributeID) {

            // Cluster AdministratorCommissioning attributes
        case MTRAttributeIDTypeClusterAdministratorCommissioningAttributeWindowStatusID:
            result = @"WindowStatus";
            break;

        case MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminFabricIndexID:
            result = @"AdminFabricIndex";
            break;

        case MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminVendorIdID:
            result = @"AdminVendorId";
            break;

        case MTRAttributeIDTypeClusterAdministratorCommissioningAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterAdministratorCommissioningAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterAdministratorCommissioningAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterAdministratorCommissioningAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeOperationalCredentialsID:

        switch (attributeID) {

            // Cluster OperationalCredentials attributes
        case MTRAttributeIDTypeClusterOperationalCredentialsAttributeNOCsID:
            result = @"NOCs";
            break;

        case MTRAttributeIDTypeClusterOperationalCredentialsAttributeFabricsID:
            result = @"Fabrics";
            break;

        case MTRAttributeIDTypeClusterOperationalCredentialsAttributeSupportedFabricsID:
            result = @"SupportedFabrics";
            break;

        case MTRAttributeIDTypeClusterOperationalCredentialsAttributeCommissionedFabricsID:
            result = @"CommissionedFabrics";
            break;

        case MTRAttributeIDTypeClusterOperationalCredentialsAttributeTrustedRootCertificatesID:
            result = @"TrustedRootCertificates";
            break;

        case MTRAttributeIDTypeClusterOperationalCredentialsAttributeCurrentFabricIndexID:
            result = @"CurrentFabricIndex";
            break;

        case MTRAttributeIDTypeClusterOperationalCredentialsAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterOperationalCredentialsAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterOperationalCredentialsAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterOperationalCredentialsAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterOperationalCredentialsAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterOperationalCredentialsAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeGroupKeyManagementID:

        switch (attributeID) {

            // Cluster GroupKeyManagement attributes
        case MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupKeyMapID:
            result = @"GroupKeyMap";
            break;

        case MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupTableID:
            result = @"GroupTable";
            break;

        case MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupsPerFabricID:
            result = @"MaxGroupsPerFabric";
            break;

        case MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupKeysPerFabricID:
            result = @"MaxGroupKeysPerFabric";
            break;

        case MTRAttributeIDTypeClusterGroupKeyManagementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterGroupKeyManagementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterGroupKeyManagementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterGroupKeyManagementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterGroupKeyManagementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterGroupKeyManagementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeFixedLabelID:

        switch (attributeID) {

            // Cluster FixedLabel attributes
        case MTRAttributeIDTypeClusterFixedLabelAttributeLabelListID:
            result = @"LabelList";
            break;

        case MTRAttributeIDTypeClusterFixedLabelAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterFixedLabelAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterFixedLabelAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterFixedLabelAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterFixedLabelAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterFixedLabelAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeUserLabelID:

        switch (attributeID) {

            // Cluster UserLabel attributes
        case MTRAttributeIDTypeClusterUserLabelAttributeLabelListID:
            result = @"LabelList";
            break;

        case MTRAttributeIDTypeClusterUserLabelAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterUserLabelAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterUserLabelAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterUserLabelAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterUserLabelAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterUserLabelAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeBooleanStateID:

        switch (attributeID) {

            // Cluster BooleanState attributes
        case MTRAttributeIDTypeClusterBooleanStateAttributeStateValueID:
            result = @"StateValue";
            break;

        case MTRAttributeIDTypeClusterBooleanStateAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterBooleanStateAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterBooleanStateAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterBooleanStateAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterBooleanStateAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterBooleanStateAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeICDManagementID:

        switch (attributeID) {

            // Cluster ICDManagement attributes
        case MTRAttributeIDTypeClusterICDManagementAttributeIdleModeDurationID:
            result = @"IdleModeDuration";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeActiveModeDurationID:
            result = @"ActiveModeDuration";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeActiveModeThresholdID:
            result = @"ActiveModeThreshold";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeRegisteredClientsID:
            result = @"RegisteredClients";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeICDCounterID:
            result = @"ICDCounter";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeClientsSupportedPerFabricID:
            result = @"ClientsSupportedPerFabric";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeUserActiveModeTriggerHintID:
            result = @"UserActiveModeTriggerHint";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeUserActiveModeTriggerInstructionID:
            result = @"UserActiveModeTriggerInstruction";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeOperatingModeID:
            result = @"OperatingMode";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeTimerID:

        switch (attributeID) {

            // Cluster Timer attributes
        case MTRAttributeIDTypeClusterTimerAttributeSetTimeID:
            result = @"SetTime";
            break;

        case MTRAttributeIDTypeClusterTimerAttributeTimeRemainingID:
            result = @"TimeRemaining";
            break;

        case MTRAttributeIDTypeClusterTimerAttributeTimerStateID:
            result = @"TimerState";
            break;

        case MTRAttributeIDTypeClusterTimerAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterTimerAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterTimerAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterTimerAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterTimerAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterTimerAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeOvenCavityOperationalStateID:

        switch (attributeID) {

            // Cluster OvenCavityOperationalState attributes
        case MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributePhaseListID:
            result = @"PhaseList";
            break;

        case MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeCurrentPhaseID:
            result = @"CurrentPhase";
            break;

        case MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeCountdownTimeID:
            result = @"CountdownTime";
            break;

        case MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeOperationalStateListID:
            result = @"OperationalStateList";
            break;

        case MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeOperationalStateID:
            result = @"OperationalState";
            break;

        case MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeOperationalErrorID:
            result = @"OperationalError";
            break;

        case MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeOvenModeID:

        switch (attributeID) {

            // Cluster OvenMode attributes
        case MTRAttributeIDTypeClusterOvenModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterOvenModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterOvenModeAttributeStartUpModeID:
            result = @"StartUpMode";
            break;

        case MTRAttributeIDTypeClusterOvenModeAttributeOnModeID:
            result = @"OnMode";
            break;

        case MTRAttributeIDTypeClusterOvenModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterOvenModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterOvenModeAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterOvenModeAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterOvenModeAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterOvenModeAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeLaundryDryerControlsID:

        switch (attributeID) {

            // Cluster LaundryDryerControls attributes
        case MTRAttributeIDTypeClusterLaundryDryerControlsAttributeSupportedDrynessLevelsID:
            result = @"SupportedDrynessLevels";
            break;

        case MTRAttributeIDTypeClusterLaundryDryerControlsAttributeSelectedDrynessLevelID:
            result = @"SelectedDrynessLevel";
            break;

        case MTRAttributeIDTypeClusterLaundryDryerControlsAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterLaundryDryerControlsAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterLaundryDryerControlsAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterLaundryDryerControlsAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterLaundryDryerControlsAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterLaundryDryerControlsAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeModeSelectID:

        switch (attributeID) {

            // Cluster ModeSelect attributes
        case MTRAttributeIDTypeClusterModeSelectAttributeDescriptionID:
            result = @"Description";
            break;

        case MTRAttributeIDTypeClusterModeSelectAttributeStandardNamespaceID:
            result = @"StandardNamespace";
            break;

        case MTRAttributeIDTypeClusterModeSelectAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterModeSelectAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterModeSelectAttributeStartUpModeID:
            result = @"StartUpMode";
            break;

        case MTRAttributeIDTypeClusterModeSelectAttributeOnModeID:
            result = @"OnMode";
            break;

        case MTRAttributeIDTypeClusterModeSelectAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterModeSelectAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterModeSelectAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterModeSelectAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterModeSelectAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterModeSelectAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeLaundryWasherModeID:

        switch (attributeID) {

            // Cluster LaundryWasherMode attributes
        case MTRAttributeIDTypeClusterLaundryWasherModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherModeAttributeStartUpModeID:
            result = @"StartUpMode";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherModeAttributeOnModeID:
            result = @"OnMode";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherModeAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherModeAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherModeAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherModeAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeRefrigeratorAndTemperatureControlledCabinetModeID:

        switch (attributeID) {

            // Cluster RefrigeratorAndTemperatureControlledCabinetMode attributes
        case MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeStartUpModeID:
            result = @"StartUpMode";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeOnModeID:
            result = @"OnMode";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeLaundryWasherControlsID:

        switch (attributeID) {

            // Cluster LaundryWasherControls attributes
        case MTRAttributeIDTypeClusterLaundryWasherControlsAttributeSpinSpeedsID:
            result = @"SpinSpeeds";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherControlsAttributeSpinSpeedCurrentID:
            result = @"SpinSpeedCurrent";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherControlsAttributeNumberOfRinsesID:
            result = @"NumberOfRinses";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherControlsAttributeSupportedRinsesID:
            result = @"SupportedRinses";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherControlsAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherControlsAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherControlsAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherControlsAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherControlsAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherControlsAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeRVCRunModeID:

        switch (attributeID) {

            // Cluster RVCRunMode attributes
        case MTRAttributeIDTypeClusterRVCRunModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterRVCRunModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterRVCRunModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterRVCRunModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterRVCRunModeAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterRVCRunModeAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterRVCRunModeAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterRVCRunModeAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeRVCCleanModeID:

        switch (attributeID) {

            // Cluster RVCCleanMode attributes
        case MTRAttributeIDTypeClusterRVCCleanModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterRVCCleanModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterRVCCleanModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterRVCCleanModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterRVCCleanModeAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterRVCCleanModeAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterRVCCleanModeAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterRVCCleanModeAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeTemperatureControlID:

        switch (attributeID) {

            // Cluster TemperatureControl attributes
        case MTRAttributeIDTypeClusterTemperatureControlAttributeTemperatureSetpointID:
            result = @"TemperatureSetpoint";
            break;

        case MTRAttributeIDTypeClusterTemperatureControlAttributeMinTemperatureID:
            result = @"MinTemperature";
            break;

        case MTRAttributeIDTypeClusterTemperatureControlAttributeMaxTemperatureID:
            result = @"MaxTemperature";
            break;

        case MTRAttributeIDTypeClusterTemperatureControlAttributeStepID:
            result = @"Step";
            break;

        case MTRAttributeIDTypeClusterTemperatureControlAttributeSelectedTemperatureLevelID:
            result = @"SelectedTemperatureLevel";
            break;

        case MTRAttributeIDTypeClusterTemperatureControlAttributeSupportedTemperatureLevelsID:
            result = @"SupportedTemperatureLevels";
            break;

        case MTRAttributeIDTypeClusterTemperatureControlAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterTemperatureControlAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterTemperatureControlAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterTemperatureControlAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterTemperatureControlAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterTemperatureControlAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeRefrigeratorAlarmID:

        switch (attributeID) {

            // Cluster RefrigeratorAlarm attributes
        case MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeMaskID:
            result = @"Mask";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeStateID:
            result = @"State";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeSupportedID:
            result = @"Supported";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeDishwasherModeID:

        switch (attributeID) {

            // Cluster DishwasherMode attributes
        case MTRAttributeIDTypeClusterDishwasherModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterDishwasherModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterDishwasherModeAttributeStartUpModeID:
            result = @"StartUpMode";
            break;

        case MTRAttributeIDTypeClusterDishwasherModeAttributeOnModeID:
            result = @"OnMode";
            break;

        case MTRAttributeIDTypeClusterDishwasherModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterDishwasherModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterDishwasherModeAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterDishwasherModeAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterDishwasherModeAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterDishwasherModeAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeAirQualityID:

        switch (attributeID) {

            // Cluster AirQuality attributes
        case MTRAttributeIDTypeClusterAirQualityAttributeAirQualityID:
            result = @"AirQuality";
            break;

        case MTRAttributeIDTypeClusterAirQualityAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterAirQualityAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterAirQualityAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterAirQualityAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterAirQualityAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterAirQualityAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeSmokeCOAlarmID:

        switch (attributeID) {

            // Cluster SmokeCOAlarm attributes
        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeExpressedStateID:
            result = @"ExpressedState";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeSmokeStateID:
            result = @"SmokeState";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeCOStateID:
            result = @"COState";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeBatteryAlertID:
            result = @"BatteryAlert";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeDeviceMutedID:
            result = @"DeviceMuted";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeTestInProgressID:
            result = @"TestInProgress";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeHardwareFaultAlertID:
            result = @"HardwareFaultAlert";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeEndOfServiceAlertID:
            result = @"EndOfServiceAlert";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeInterconnectSmokeAlarmID:
            result = @"InterconnectSmokeAlarm";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeInterconnectCOAlarmID:
            result = @"InterconnectCOAlarm";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeContaminationStateID:
            result = @"ContaminationState";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeSmokeSensitivityLevelID:
            result = @"SmokeSensitivityLevel";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeExpiryDateID:
            result = @"ExpiryDate";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterSmokeCOAlarmAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeDishwasherAlarmID:

        switch (attributeID) {

            // Cluster DishwasherAlarm attributes
        case MTRAttributeIDTypeClusterDishwasherAlarmAttributeMaskID:
            result = @"Mask";
            break;

        case MTRAttributeIDTypeClusterDishwasherAlarmAttributeLatchID:
            result = @"Latch";
            break;

        case MTRAttributeIDTypeClusterDishwasherAlarmAttributeStateID:
            result = @"State";
            break;

        case MTRAttributeIDTypeClusterDishwasherAlarmAttributeSupportedID:
            result = @"Supported";
            break;

        case MTRAttributeIDTypeClusterDishwasherAlarmAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterDishwasherAlarmAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterDishwasherAlarmAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterDishwasherAlarmAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterDishwasherAlarmAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterDishwasherAlarmAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeMicrowaveOvenModeID:

        switch (attributeID) {

            // Cluster MicrowaveOvenMode attributes
        case MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeMicrowaveOvenControlID:

        switch (attributeID) {

            // Cluster MicrowaveOvenControl attributes
        case MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeCookTimeID:
            result = @"CookTime";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeMaxCookTimeID:
            result = @"MaxCookTime";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenControlAttributePowerSettingID:
            result = @"PowerSetting";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeMinPowerID:
            result = @"MinPower";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeMaxPowerID:
            result = @"MaxPower";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenControlAttributePowerStepID:
            result = @"PowerStep";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeSupportedWattsID:
            result = @"SupportedWatts";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeSelectedWattIndexID:
            result = @"SelectedWattIndex";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeWattRatingID:
            result = @"WattRating";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeOperationalStateID:

        switch (attributeID) {

            // Cluster OperationalState attributes
        case MTRAttributeIDTypeClusterOperationalStateAttributePhaseListID:
            result = @"PhaseList";
            break;

        case MTRAttributeIDTypeClusterOperationalStateAttributeCurrentPhaseID:
            result = @"CurrentPhase";
            break;

        case MTRAttributeIDTypeClusterOperationalStateAttributeCountdownTimeID:
            result = @"CountdownTime";
            break;

        case MTRAttributeIDTypeClusterOperationalStateAttributeOperationalStateListID:
            result = @"OperationalStateList";
            break;

        case MTRAttributeIDTypeClusterOperationalStateAttributeOperationalStateID:
            result = @"OperationalState";
            break;

        case MTRAttributeIDTypeClusterOperationalStateAttributeOperationalErrorID:
            result = @"OperationalError";
            break;

        case MTRAttributeIDTypeClusterOperationalStateAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterOperationalStateAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterOperationalStateAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterOperationalStateAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterOperationalStateAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterOperationalStateAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeRVCOperationalStateID:

        switch (attributeID) {

            // Cluster RVCOperationalState attributes
        case MTRAttributeIDTypeClusterRVCOperationalStateAttributePhaseListID:
            result = @"PhaseList";
            break;

        case MTRAttributeIDTypeClusterRVCOperationalStateAttributeCurrentPhaseID:
            result = @"CurrentPhase";
            break;

        case MTRAttributeIDTypeClusterRVCOperationalStateAttributeCountdownTimeID:
            result = @"CountdownTime";
            break;

        case MTRAttributeIDTypeClusterRVCOperationalStateAttributeOperationalStateListID:
            result = @"OperationalStateList";
            break;

        case MTRAttributeIDTypeClusterRVCOperationalStateAttributeOperationalStateID:
            result = @"OperationalState";
            break;

        case MTRAttributeIDTypeClusterRVCOperationalStateAttributeOperationalErrorID:
            result = @"OperationalError";
            break;

        case MTRAttributeIDTypeClusterRVCOperationalStateAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterRVCOperationalStateAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterRVCOperationalStateAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterRVCOperationalStateAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterRVCOperationalStateAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterRVCOperationalStateAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeScenesManagementID:

        switch (attributeID) {

            // Cluster ScenesManagement attributes
        case MTRAttributeIDTypeClusterScenesManagementAttributeLastConfiguredByID:
            result = @"LastConfiguredBy";
            break;

        case MTRAttributeIDTypeClusterScenesManagementAttributeSceneTableSizeID:
            result = @"SceneTableSize";
            break;

        case MTRAttributeIDTypeClusterScenesManagementAttributeFabricSceneInfoID:
            result = @"FabricSceneInfo";
            break;

        case MTRAttributeIDTypeClusterScenesManagementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterScenesManagementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterScenesManagementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterScenesManagementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterScenesManagementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterScenesManagementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeHEPAFilterMonitoringID:

        switch (attributeID) {

            // Cluster HEPAFilterMonitoring attributes
        case MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeConditionID:
            result = @"Condition";
            break;

        case MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeDegradationDirectionID:
            result = @"DegradationDirection";
            break;

        case MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeChangeIndicationID:
            result = @"ChangeIndication";
            break;

        case MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeInPlaceIndicatorID:
            result = @"InPlaceIndicator";
            break;

        case MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeLastChangedTimeID:
            result = @"LastChangedTime";
            break;

        case MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeReplacementProductListID:
            result = @"ReplacementProductList";
            break;

        case MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeActivatedCarbonFilterMonitoringID:

        switch (attributeID) {

            // Cluster ActivatedCarbonFilterMonitoring attributes
        case MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeConditionID:
            result = @"Condition";
            break;

        case MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeDegradationDirectionID:
            result = @"DegradationDirection";
            break;

        case MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeChangeIndicationID:
            result = @"ChangeIndication";
            break;

        case MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeInPlaceIndicatorID:
            result = @"InPlaceIndicator";
            break;

        case MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeLastChangedTimeID:
            result = @"LastChangedTime";
            break;

        case MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeReplacementProductListID:
            result = @"ReplacementProductList";
            break;

        case MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeBooleanStateConfigurationID:

        switch (attributeID) {

            // Cluster BooleanStateConfiguration attributes
        case MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeCurrentSensitivityLevelID:
            result = @"CurrentSensitivityLevel";
            break;

        case MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeSupportedSensitivityLevelsID:
            result = @"SupportedSensitivityLevels";
            break;

        case MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeDefaultSensitivityLevelID:
            result = @"DefaultSensitivityLevel";
            break;

        case MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeAlarmsActiveID:
            result = @"AlarmsActive";
            break;

        case MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeAlarmsSuppressedID:
            result = @"AlarmsSuppressed";
            break;

        case MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeAlarmsEnabledID:
            result = @"AlarmsEnabled";
            break;

        case MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeAlarmsSupportedID:
            result = @"AlarmsSupported";
            break;

        case MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeSensorFaultID:
            result = @"SensorFault";
            break;

        case MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeValveConfigurationAndControlID:

        switch (attributeID) {

            // Cluster ValveConfigurationAndControl attributes
        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeOpenDurationID:
            result = @"OpenDuration";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeDefaultOpenDurationID:
            result = @"DefaultOpenDuration";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeAutoCloseTimeID:
            result = @"AutoCloseTime";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeRemainingDurationID:
            result = @"RemainingDuration";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeCurrentStateID:
            result = @"CurrentState";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeTargetStateID:
            result = @"TargetState";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeCurrentLevelID:
            result = @"CurrentLevel";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeTargetLevelID:
            result = @"TargetLevel";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeDefaultOpenLevelID:
            result = @"DefaultOpenLevel";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeValveFaultID:
            result = @"ValveFault";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeLevelStepID:
            result = @"LevelStep";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeElectricalPowerMeasurementID:

        switch (attributeID) {

            // Cluster ElectricalPowerMeasurement attributes
        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributePowerModeID:
            result = @"PowerMode";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeNumberOfMeasurementTypesID:
            result = @"NumberOfMeasurementTypes";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeAccuracyID:
            result = @"Accuracy";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeRangesID:
            result = @"Ranges";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeVoltageID:
            result = @"Voltage";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeActiveCurrentID:
            result = @"ActiveCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeReactiveCurrentID:
            result = @"ReactiveCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeApparentCurrentID:
            result = @"ApparentCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeActivePowerID:
            result = @"ActivePower";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeReactivePowerID:
            result = @"ReactivePower";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeApparentPowerID:
            result = @"ApparentPower";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeRMSVoltageID:
            result = @"RMSVoltage";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeRMSCurrentID:
            result = @"RMSCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeRMSPowerID:
            result = @"RMSPower";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeFrequencyID:
            result = @"Frequency";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeHarmonicCurrentsID:
            result = @"HarmonicCurrents";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeHarmonicPhasesID:
            result = @"HarmonicPhases";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributePowerFactorID:
            result = @"PowerFactor";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeNeutralCurrentID:
            result = @"NeutralCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterElectricalPowerMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeElectricalEnergyMeasurementID:

        switch (attributeID) {

            // Cluster ElectricalEnergyMeasurement attributes
        case MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeAccuracyID:
            result = @"Accuracy";
            break;

        case MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeCumulativeEnergyImportedID:
            result = @"CumulativeEnergyImported";
            break;

        case MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeCumulativeEnergyExportedID:
            result = @"CumulativeEnergyExported";
            break;

        case MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributePeriodicEnergyImportedID:
            result = @"PeriodicEnergyImported";
            break;

        case MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributePeriodicEnergyExportedID:
            result = @"PeriodicEnergyExported";
            break;

        case MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeCumulativeEnergyResetID:
            result = @"CumulativeEnergyReset";
            break;

        case MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeDemandResponseLoadControlID:

        switch (attributeID) {

            // Cluster DemandResponseLoadControl attributes
        case MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeLoadControlProgramsID:
            result = @"LoadControlPrograms";
            break;

        case MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeNumberOfLoadControlProgramsID:
            result = @"NumberOfLoadControlPrograms";
            break;

        case MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeEventsID:
            result = @"Events";
            break;

        case MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeActiveEventsID:
            result = @"ActiveEvents";
            break;

        case MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeNumberOfEventsPerProgramID:
            result = @"NumberOfEventsPerProgram";
            break;

        case MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeNumberOfTransitionsID:
            result = @"NumberOfTransitions";
            break;

        case MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeDefaultRandomStartID:
            result = @"DefaultRandomStart";
            break;

        case MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeDefaultRandomDurationID:
            result = @"DefaultRandomDuration";
            break;

        case MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeMessagesID:

        switch (attributeID) {

            // Cluster Messages attributes
        case MTRAttributeIDTypeClusterMessagesAttributeMessagesID:
            result = @"Messages";
            break;

        case MTRAttributeIDTypeClusterMessagesAttributeActiveMessageIDsID:
            result = @"ActiveMessageIDs";
            break;

        case MTRAttributeIDTypeClusterMessagesAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterMessagesAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterMessagesAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterMessagesAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterMessagesAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterMessagesAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeDeviceEnergyManagementID:

        switch (attributeID) {

            // Cluster DeviceEnergyManagement attributes
        case MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeESATypeID:
            result = @"ESAType";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeESACanGenerateID:
            result = @"ESACanGenerate";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeESAStateID:
            result = @"ESAState";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeAbsMinPowerID:
            result = @"AbsMinPower";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeAbsMaxPowerID:
            result = @"AbsMaxPower";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementAttributePowerAdjustmentCapabilityID:
            result = @"PowerAdjustmentCapability";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeForecastID:
            result = @"Forecast";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeOptOutStateID:
            result = @"OptOutState";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeEnergyEVSEID:

        switch (attributeID) {

            // Cluster EnergyEVSE attributes
        case MTRAttributeIDTypeClusterEnergyEVSEAttributeStateID:
            result = @"State";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeSupplyStateID:
            result = @"SupplyState";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeFaultStateID:
            result = @"FaultState";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeChargingEnabledUntilID:
            result = @"ChargingEnabledUntil";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeDischargingEnabledUntilID:
            result = @"DischargingEnabledUntil";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeCircuitCapacityID:
            result = @"CircuitCapacity";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeMinimumChargeCurrentID:
            result = @"MinimumChargeCurrent";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeMaximumChargeCurrentID:
            result = @"MaximumChargeCurrent";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeMaximumDischargeCurrentID:
            result = @"MaximumDischargeCurrent";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeUserMaximumChargeCurrentID:
            result = @"UserMaximumChargeCurrent";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeRandomizationDelayWindowID:
            result = @"RandomizationDelayWindow";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeNextChargeStartTimeID:
            result = @"NextChargeStartTime";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeNextChargeTargetTimeID:
            result = @"NextChargeTargetTime";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeNextChargeRequiredEnergyID:
            result = @"NextChargeRequiredEnergy";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeNextChargeTargetSoCID:
            result = @"NextChargeTargetSoC";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeApproximateEVEfficiencyID:
            result = @"ApproximateEVEfficiency";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeStateOfChargeID:
            result = @"StateOfCharge";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeBatteryCapacityID:
            result = @"BatteryCapacity";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeVehicleIDID:
            result = @"VehicleID";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeSessionIDID:
            result = @"SessionID";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeSessionDurationID:
            result = @"SessionDuration";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeSessionEnergyChargedID:
            result = @"SessionEnergyCharged";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeSessionEnergyDischargedID:
            result = @"SessionEnergyDischarged";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeEnergyPreferenceID:

        switch (attributeID) {

            // Cluster EnergyPreference attributes
        case MTRAttributeIDTypeClusterEnergyPreferenceAttributeEnergyBalancesID:
            result = @"EnergyBalances";
            break;

        case MTRAttributeIDTypeClusterEnergyPreferenceAttributeCurrentEnergyBalanceID:
            result = @"CurrentEnergyBalance";
            break;

        case MTRAttributeIDTypeClusterEnergyPreferenceAttributeEnergyPrioritiesID:
            result = @"EnergyPriorities";
            break;

        case MTRAttributeIDTypeClusterEnergyPreferenceAttributeLowPowerModeSensitivitiesID:
            result = @"LowPowerModeSensitivities";
            break;

        case MTRAttributeIDTypeClusterEnergyPreferenceAttributeCurrentLowPowerModeSensitivityID:
            result = @"CurrentLowPowerModeSensitivity";
            break;

        case MTRAttributeIDTypeClusterEnergyPreferenceAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterEnergyPreferenceAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterEnergyPreferenceAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterEnergyPreferenceAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterEnergyPreferenceAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterEnergyPreferenceAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypePowerTopologyID:

        switch (attributeID) {

            // Cluster PowerTopology attributes
        case MTRAttributeIDTypeClusterPowerTopologyAttributeAvailableEndpointsID:
            result = @"AvailableEndpoints";
            break;

        case MTRAttributeIDTypeClusterPowerTopologyAttributeActiveEndpointsID:
            result = @"ActiveEndpoints";
            break;

        case MTRAttributeIDTypeClusterPowerTopologyAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterPowerTopologyAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterPowerTopologyAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterPowerTopologyAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterPowerTopologyAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterPowerTopologyAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeEnergyEVSEModeID:

        switch (attributeID) {

            // Cluster EnergyEVSEMode attributes
        case MTRAttributeIDTypeClusterEnergyEVSEModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEModeAttributeStartUpModeID:
            result = @"StartUpMode";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEModeAttributeOnModeID:
            result = @"OnMode";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEModeAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEModeAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEModeAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEModeAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeDeviceEnergyManagementModeID:

        switch (attributeID) {

            // Cluster DeviceEnergyManagementMode attributes
        case MTRAttributeIDTypeClusterDeviceEnergyManagementModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementModeAttributeStartUpModeID:
            result = @"StartUpMode";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementModeAttributeOnModeID:
            result = @"OnMode";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementModeAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementModeAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementModeAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementModeAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeDoorLockID:

        switch (attributeID) {

            // Cluster DoorLock attributes
        case MTRAttributeIDTypeClusterDoorLockAttributeLockStateID:
            result = @"LockState";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeLockTypeID:
            result = @"LockType";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeActuatorEnabledID:
            result = @"ActuatorEnabled";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeDoorStateID:
            result = @"DoorState";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeDoorOpenEventsID:
            result = @"DoorOpenEvents";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeDoorClosedEventsID:
            result = @"DoorClosedEvents";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeOpenPeriodID:
            result = @"OpenPeriod";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeNumberOfTotalUsersSupportedID:
            result = @"NumberOfTotalUsersSupported";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeNumberOfPINUsersSupportedID:
            result = @"NumberOfPINUsersSupported";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeNumberOfRFIDUsersSupportedID:
            result = @"NumberOfRFIDUsersSupported";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeNumberOfWeekDaySchedulesSupportedPerUserID:
            result = @"NumberOfWeekDaySchedulesSupportedPerUser";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeNumberOfYearDaySchedulesSupportedPerUserID:
            result = @"NumberOfYearDaySchedulesSupportedPerUser";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeNumberOfHolidaySchedulesSupportedID:
            result = @"NumberOfHolidaySchedulesSupported";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeMaxPINCodeLengthID:
            result = @"MaxPINCodeLength";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeMinPINCodeLengthID:
            result = @"MinPINCodeLength";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeMaxRFIDCodeLengthID:
            result = @"MaxRFIDCodeLength";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeMinRFIDCodeLengthID:
            result = @"MinRFIDCodeLength";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeCredentialRulesSupportID:
            result = @"CredentialRulesSupport";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeNumberOfCredentialsSupportedPerUserID:
            result = @"NumberOfCredentialsSupportedPerUser";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeLanguageID:
            result = @"Language";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeLEDSettingsID:
            result = @"LEDSettings";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeAutoRelockTimeID:
            result = @"AutoRelockTime";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeSoundVolumeID:
            result = @"SoundVolume";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeOperatingModeID:
            result = @"OperatingMode";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeSupportedOperatingModesID:
            result = @"SupportedOperatingModes";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeDefaultConfigurationRegisterID:
            result = @"DefaultConfigurationRegister";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeEnableLocalProgrammingID:
            result = @"EnableLocalProgramming";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeEnableOneTouchLockingID:
            result = @"EnableOneTouchLocking";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeEnableInsideStatusLEDID:
            result = @"EnableInsideStatusLED";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeEnablePrivacyModeButtonID:
            result = @"EnablePrivacyModeButton";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeLocalProgrammingFeaturesID:
            result = @"LocalProgrammingFeatures";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeWrongCodeEntryLimitID:
            result = @"WrongCodeEntryLimit";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeUserCodeTemporaryDisableTimeID:
            result = @"UserCodeTemporaryDisableTime";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeSendPINOverTheAirID:
            result = @"SendPINOverTheAir";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeRequirePINforRemoteOperationID:
            result = @"RequirePINforRemoteOperation";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeExpiringUserTimeoutID:
            result = @"ExpiringUserTimeout";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeAliroReaderVerificationKeyID:
            result = @"AliroReaderVerificationKey";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeAliroReaderGroupIdentifierID:
            result = @"AliroReaderGroupIdentifier";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeAliroReaderGroupSubIdentifierID:
            result = @"AliroReaderGroupSubIdentifier";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeAliroExpeditedTransactionSupportedProtocolVersionsID:
            result = @"AliroExpeditedTransactionSupportedProtocolVersions";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeAliroGroupResolvingKeyID:
            result = @"AliroGroupResolvingKey";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeAliroSupportedBLEUWBProtocolVersionsID:
            result = @"AliroSupportedBLEUWBProtocolVersions";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeAliroBLEAdvertisingVersionID:
            result = @"AliroBLEAdvertisingVersion";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeNumberOfAliroCredentialIssuerKeysSupportedID:
            result = @"NumberOfAliroCredentialIssuerKeysSupported";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeNumberOfAliroEndpointKeysSupportedID:
            result = @"NumberOfAliroEndpointKeysSupported";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterDoorLockAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeWindowCoveringID:

        switch (attributeID) {

            // Cluster WindowCovering attributes
        case MTRAttributeIDTypeClusterWindowCoveringAttributeTypeID:
            result = @"Type";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitLiftID:
            result = @"PhysicalClosedLimitLift";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitTiltID:
            result = @"PhysicalClosedLimitTilt";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftID:
            result = @"CurrentPositionLift";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltID:
            result = @"CurrentPositionTilt";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsLiftID:
            result = @"NumberOfActuationsLift";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsTiltID:
            result = @"NumberOfActuationsTilt";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeConfigStatusID:
            result = @"ConfigStatus";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercentageID:
            result = @"CurrentPositionLiftPercentage";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercentageID:
            result = @"CurrentPositionTiltPercentage";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeOperationalStatusID:
            result = @"OperationalStatus";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionLiftPercent100thsID:
            result = @"TargetPositionLiftPercent100ths";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionTiltPercent100thsID:
            result = @"TargetPositionTiltPercent100ths";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeEndProductTypeID:
            result = @"EndProductType";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercent100thsID:
            result = @"CurrentPositionLiftPercent100ths";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercent100thsID:
            result = @"CurrentPositionTiltPercent100ths";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitLiftID:
            result = @"InstalledOpenLimitLift";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitLiftID:
            result = @"InstalledClosedLimitLift";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitTiltID:
            result = @"InstalledOpenLimitTilt";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitTiltID:
            result = @"InstalledClosedLimitTilt";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeModeID:
            result = @"Mode";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeSafetyStatusID:
            result = @"SafetyStatus";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterWindowCoveringAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeBarrierControlID:

        switch (attributeID) {

            // Cluster BarrierControl attributes
        case MTRAttributeIDTypeClusterBarrierControlAttributeBarrierMovingStateID:
            result = @"BarrierMovingState";
            break;

        case MTRAttributeIDTypeClusterBarrierControlAttributeBarrierSafetyStatusID:
            result = @"BarrierSafetyStatus";
            break;

        case MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCapabilitiesID:
            result = @"BarrierCapabilities";
            break;

        case MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenEventsID:
            result = @"BarrierOpenEvents";
            break;

        case MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCloseEventsID:
            result = @"BarrierCloseEvents";
            break;

        case MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandOpenEventsID:
            result = @"BarrierCommandOpenEvents";
            break;

        case MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandCloseEventsID:
            result = @"BarrierCommandCloseEvents";
            break;

        case MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenPeriodID:
            result = @"BarrierOpenPeriod";
            break;

        case MTRAttributeIDTypeClusterBarrierControlAttributeBarrierClosePeriodID:
            result = @"BarrierClosePeriod";
            break;

        case MTRAttributeIDTypeClusterBarrierControlAttributeBarrierPositionID:
            result = @"BarrierPosition";
            break;

        case MTRAttributeIDTypeClusterBarrierControlAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterBarrierControlAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterBarrierControlAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterBarrierControlAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterBarrierControlAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterBarrierControlAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypePumpConfigurationAndControlID:

        switch (attributeID) {

            // Cluster PumpConfigurationAndControl attributes
        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxPressureID:
            result = @"MaxPressure";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxSpeedID:
            result = @"MaxSpeed";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxFlowID:
            result = @"MaxFlow";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstPressureID:
            result = @"MinConstPressure";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstPressureID:
            result = @"MaxConstPressure";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinCompPressureID:
            result = @"MinCompPressure";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxCompPressureID:
            result = @"MaxCompPressure";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstSpeedID:
            result = @"MinConstSpeed";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstSpeedID:
            result = @"MaxConstSpeed";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstFlowID:
            result = @"MinConstFlow";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstFlowID:
            result = @"MaxConstFlow";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstTempID:
            result = @"MinConstTemp";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstTempID:
            result = @"MaxConstTemp";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePumpStatusID:
            result = @"PumpStatus";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveOperationModeID:
            result = @"EffectiveOperationMode";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveControlModeID:
            result = @"EffectiveControlMode";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeCapacityID:
            result = @"Capacity";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeSpeedID:
            result = @"Speed";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeRunningHoursID:
            result = @"LifetimeRunningHours";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePowerID:
            result = @"Power";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeEnergyConsumedID:
            result = @"LifetimeEnergyConsumed";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeOperationModeID:
            result = @"OperationMode";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeControlModeID:
            result = @"ControlMode";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeThermostatID:

        switch (attributeID) {

            // Cluster Thermostat attributes
        case MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureID:
            result = @"LocalTemperature";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeOutdoorTemperatureID:
            result = @"OutdoorTemperature";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeOccupancyID:
            result = @"Occupancy";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeAbsMinHeatSetpointLimitID:
            result = @"AbsMinHeatSetpointLimit";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeAbsMaxHeatSetpointLimitID:
            result = @"AbsMaxHeatSetpointLimit";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeAbsMinCoolSetpointLimitID:
            result = @"AbsMinCoolSetpointLimit";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeAbsMaxCoolSetpointLimitID:
            result = @"AbsMaxCoolSetpointLimit";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributePICoolingDemandID:
            result = @"PICoolingDemand";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributePIHeatingDemandID:
            result = @"PIHeatingDemand";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeHVACSystemTypeConfigurationID:
            result = @"HVACSystemTypeConfiguration";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureCalibrationID:
            result = @"LocalTemperatureCalibration";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeOccupiedCoolingSetpointID:
            result = @"OccupiedCoolingSetpoint";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeOccupiedHeatingSetpointID:
            result = @"OccupiedHeatingSetpoint";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedCoolingSetpointID:
            result = @"UnoccupiedCoolingSetpoint";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedHeatingSetpointID:
            result = @"UnoccupiedHeatingSetpoint";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeMinHeatSetpointLimitID:
            result = @"MinHeatSetpointLimit";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeMaxHeatSetpointLimitID:
            result = @"MaxHeatSetpointLimit";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeMinCoolSetpointLimitID:
            result = @"MinCoolSetpointLimit";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeMaxCoolSetpointLimitID:
            result = @"MaxCoolSetpointLimit";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeMinSetpointDeadBandID:
            result = @"MinSetpointDeadBand";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeRemoteSensingID:
            result = @"RemoteSensing";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeControlSequenceOfOperationID:
            result = @"ControlSequenceOfOperation";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeSystemModeID:
            result = @"SystemMode";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningModeID:
            result = @"ThermostatRunningMode";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeStartOfWeekID:
            result = @"StartOfWeek";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeNumberOfWeeklyTransitionsID:
            result = @"NumberOfWeeklyTransitions";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeNumberOfDailyTransitionsID:
            result = @"NumberOfDailyTransitions";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldID:
            result = @"TemperatureSetpointHold";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldDurationID:
            result = @"TemperatureSetpointHoldDuration";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeThermostatProgrammingOperationModeID:
            result = @"ThermostatProgrammingOperationMode";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningStateID:
            result = @"ThermostatRunningState";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceID:
            result = @"SetpointChangeSource";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeAmountID:
            result = @"SetpointChangeAmount";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceTimestampID:
            result = @"SetpointChangeSourceTimestamp";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackID:
            result = @"OccupiedSetback";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMinID:
            result = @"OccupiedSetbackMin";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMaxID:
            result = @"OccupiedSetbackMax";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackID:
            result = @"UnoccupiedSetback";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMinID:
            result = @"UnoccupiedSetbackMin";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMaxID:
            result = @"UnoccupiedSetbackMax";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeEmergencyHeatDeltaID:
            result = @"EmergencyHeatDelta";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeACTypeID:
            result = @"ACType";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeACCapacityID:
            result = @"ACCapacity";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeACRefrigerantTypeID:
            result = @"ACRefrigerantType";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeACCompressorTypeID:
            result = @"ACCompressorType";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeACErrorCodeID:
            result = @"ACErrorCode";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeACLouverPositionID:
            result = @"ACLouverPosition";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeACCoilTemperatureID:
            result = @"ACCoilTemperature";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeACCapacityformatID:
            result = @"ACCapacityformat";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributePresetTypesID:
            result = @"PresetTypes";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeScheduleTypesID:
            result = @"ScheduleTypes";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeNumberOfPresetsID:
            result = @"NumberOfPresets";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeNumberOfSchedulesID:
            result = @"NumberOfSchedules";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeNumberOfScheduleTransitionsID:
            result = @"NumberOfScheduleTransitions";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeNumberOfScheduleTransitionPerDayID:
            result = @"NumberOfScheduleTransitionPerDay";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeActivePresetHandleID:
            result = @"ActivePresetHandle";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeActiveScheduleHandleID:
            result = @"ActiveScheduleHandle";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributePresetsID:
            result = @"Presets";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeSchedulesID:
            result = @"Schedules";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributePresetsSchedulesEditableID:
            result = @"PresetsSchedulesEditable";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldPolicyID:
            result = @"TemperatureSetpointHoldPolicy";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeSetpointHoldExpiryTimestampID:
            result = @"SetpointHoldExpiryTimestamp";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeQueuedPresetID:
            result = @"QueuedPreset";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeFanControlID:

        switch (attributeID) {

            // Cluster FanControl attributes
        case MTRAttributeIDTypeClusterFanControlAttributeFanModeID:
            result = @"FanMode";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributeFanModeSequenceID:
            result = @"FanModeSequence";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributePercentSettingID:
            result = @"PercentSetting";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributePercentCurrentID:
            result = @"PercentCurrent";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributeSpeedMaxID:
            result = @"SpeedMax";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributeSpeedSettingID:
            result = @"SpeedSetting";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributeSpeedCurrentID:
            result = @"SpeedCurrent";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributeRockSupportID:
            result = @"RockSupport";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributeRockSettingID:
            result = @"RockSetting";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributeWindSupportID:
            result = @"WindSupport";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributeWindSettingID:
            result = @"WindSetting";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributeAirflowDirectionID:
            result = @"AirflowDirection";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterFanControlAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeThermostatUserInterfaceConfigurationID:

        switch (attributeID) {

            // Cluster ThermostatUserInterfaceConfiguration attributes
        case MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeTemperatureDisplayModeID:
            result = @"TemperatureDisplayMode";
            break;

        case MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeKeypadLockoutID:
            result = @"KeypadLockout";
            break;

        case MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeScheduleProgrammingVisibilityID:
            result = @"ScheduleProgrammingVisibility";
            break;

        case MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeColorControlID:

        switch (attributeID) {

            // Cluster ColorControl attributes
        case MTRAttributeIDTypeClusterColorControlAttributeCurrentHueID:
            result = @"CurrentHue";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeCurrentSaturationID:
            result = @"CurrentSaturation";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeRemainingTimeID:
            result = @"RemainingTime";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeCurrentXID:
            result = @"CurrentX";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeCurrentYID:
            result = @"CurrentY";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeDriftCompensationID:
            result = @"DriftCompensation";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeCompensationTextID:
            result = @"CompensationText";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorTemperatureMiredsID:
            result = @"ColorTemperatureMireds";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorModeID:
            result = @"ColorMode";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeOptionsID:
            result = @"Options";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeNumberOfPrimariesID:
            result = @"NumberOfPrimaries";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary1XID:
            result = @"Primary1X";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary1YID:
            result = @"Primary1Y";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary1IntensityID:
            result = @"Primary1Intensity";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary2XID:
            result = @"Primary2X";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary2YID:
            result = @"Primary2Y";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary2IntensityID:
            result = @"Primary2Intensity";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary3XID:
            result = @"Primary3X";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary3YID:
            result = @"Primary3Y";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary3IntensityID:
            result = @"Primary3Intensity";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary4XID:
            result = @"Primary4X";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary4YID:
            result = @"Primary4Y";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary4IntensityID:
            result = @"Primary4Intensity";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary5XID:
            result = @"Primary5X";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary5YID:
            result = @"Primary5Y";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary5IntensityID:
            result = @"Primary5Intensity";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary6XID:
            result = @"Primary6X";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary6YID:
            result = @"Primary6Y";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributePrimary6IntensityID:
            result = @"Primary6Intensity";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeWhitePointXID:
            result = @"WhitePointX";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeWhitePointYID:
            result = @"WhitePointY";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorPointRXID:
            result = @"ColorPointRX";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorPointRYID:
            result = @"ColorPointRY";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorPointRIntensityID:
            result = @"ColorPointRIntensity";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorPointGXID:
            result = @"ColorPointGX";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorPointGYID:
            result = @"ColorPointGY";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorPointGIntensityID:
            result = @"ColorPointGIntensity";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorPointBXID:
            result = @"ColorPointBX";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorPointBYID:
            result = @"ColorPointBY";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorPointBIntensityID:
            result = @"ColorPointBIntensity";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeEnhancedCurrentHueID:
            result = @"EnhancedCurrentHue";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeEnhancedColorModeID:
            result = @"EnhancedColorMode";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorLoopActiveID:
            result = @"ColorLoopActive";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorLoopDirectionID:
            result = @"ColorLoopDirection";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorLoopTimeID:
            result = @"ColorLoopTime";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorLoopStartEnhancedHueID:
            result = @"ColorLoopStartEnhancedHue";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorLoopStoredEnhancedHueID:
            result = @"ColorLoopStoredEnhancedHue";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorCapabilitiesID:
            result = @"ColorCapabilities";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMinMiredsID:
            result = @"ColorTempPhysicalMinMireds";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMaxMiredsID:
            result = @"ColorTempPhysicalMaxMireds";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeCoupleColorTempToLevelMinMiredsID:
            result = @"CoupleColorTempToLevelMinMireds";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeStartUpColorTemperatureMiredsID:
            result = @"StartUpColorTemperatureMireds";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterColorControlAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeBallastConfigurationID:

        switch (attributeID) {

            // Cluster BallastConfiguration attributes
        case MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMinLevelID:
            result = @"PhysicalMinLevel";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMaxLevelID:
            result = @"PhysicalMaxLevel";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastStatusID:
            result = @"BallastStatus";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeMinLevelID:
            result = @"MinLevel";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeMaxLevelID:
            result = @"MaxLevel";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeIntrinsicBallastFactorID:
            result = @"IntrinsicBallastFactor";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastFactorAdjustmentID:
            result = @"BallastFactorAdjustment";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeLampQuantityID:
            result = @"LampQuantity";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeLampTypeID:
            result = @"LampType";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeLampManufacturerID:
            result = @"LampManufacturer";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeLampRatedHoursID:
            result = @"LampRatedHours";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursID:
            result = @"LampBurnHours";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeLampAlarmModeID:
            result = @"LampAlarmMode";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursTripPointID:
            result = @"LampBurnHoursTripPoint";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterBallastConfigurationAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeIlluminanceMeasurementID:

        switch (attributeID) {

            // Cluster IlluminanceMeasurement attributes
        case MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMeasuredValueID:
            result = @"MeasuredValue";
            break;

        case MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMinMeasuredValueID:
            result = @"MinMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMaxMeasuredValueID:
            result = @"MaxMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeToleranceID:
            result = @"Tolerance";
            break;

        case MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeLightSensorTypeID:
            result = @"LightSensorType";
            break;

        case MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeTemperatureMeasurementID:

        switch (attributeID) {

            // Cluster TemperatureMeasurement attributes
        case MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMeasuredValueID:
            result = @"MeasuredValue";
            break;

        case MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMinMeasuredValueID:
            result = @"MinMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMaxMeasuredValueID:
            result = @"MaxMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterTemperatureMeasurementAttributeToleranceID:
            result = @"Tolerance";
            break;

        case MTRAttributeIDTypeClusterTemperatureMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterTemperatureMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterTemperatureMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterTemperatureMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypePressureMeasurementID:

        switch (attributeID) {

            // Cluster PressureMeasurement attributes
        case MTRAttributeIDTypeClusterPressureMeasurementAttributeMeasuredValueID:
            result = @"MeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPressureMeasurementAttributeMinMeasuredValueID:
            result = @"MinMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxMeasuredValueID:
            result = @"MaxMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPressureMeasurementAttributeToleranceID:
            result = @"Tolerance";
            break;

        case MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledValueID:
            result = @"ScaledValue";
            break;

        case MTRAttributeIDTypeClusterPressureMeasurementAttributeMinScaledValueID:
            result = @"MinScaledValue";
            break;

        case MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxScaledValueID:
            result = @"MaxScaledValue";
            break;

        case MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledToleranceID:
            result = @"ScaledTolerance";
            break;

        case MTRAttributeIDTypeClusterPressureMeasurementAttributeScaleID:
            result = @"Scale";
            break;

        case MTRAttributeIDTypeClusterPressureMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterPressureMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterPressureMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterPressureMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterPressureMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterPressureMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeFlowMeasurementID:

        switch (attributeID) {

            // Cluster FlowMeasurement attributes
        case MTRAttributeIDTypeClusterFlowMeasurementAttributeMeasuredValueID:
            result = @"MeasuredValue";
            break;

        case MTRAttributeIDTypeClusterFlowMeasurementAttributeMinMeasuredValueID:
            result = @"MinMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterFlowMeasurementAttributeMaxMeasuredValueID:
            result = @"MaxMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterFlowMeasurementAttributeToleranceID:
            result = @"Tolerance";
            break;

        case MTRAttributeIDTypeClusterFlowMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterFlowMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterFlowMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterFlowMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterFlowMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterFlowMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeRelativeHumidityMeasurementID:

        switch (attributeID) {

            // Cluster RelativeHumidityMeasurement attributes
        case MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMeasuredValueID:
            result = @"MeasuredValue";
            break;

        case MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMinMeasuredValueID:
            result = @"MinMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMaxMeasuredValueID:
            result = @"MaxMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeToleranceID:
            result = @"Tolerance";
            break;

        case MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeOccupancySensingID:

        switch (attributeID) {

            // Cluster OccupancySensing attributes
        case MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancyID:
            result = @"Occupancy";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeID:
            result = @"OccupancySensorType";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeBitmapID:
            result = @"OccupancySensorTypeBitmap";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributePIROccupiedToUnoccupiedDelayID:
            result = @"PIROccupiedToUnoccupiedDelay";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedDelayID:
            result = @"PIRUnoccupiedToOccupiedDelay";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedThresholdID:
            result = @"PIRUnoccupiedToOccupiedThreshold";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicOccupiedToUnoccupiedDelayID:
            result = @"UltrasonicOccupiedToUnoccupiedDelay";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedDelayID:
            result = @"UltrasonicUnoccupiedToOccupiedDelay";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedThresholdID:
            result = @"UltrasonicUnoccupiedToOccupiedThreshold";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactOccupiedToUnoccupiedDelayID:
            result = @"PhysicalContactOccupiedToUnoccupiedDelay";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedDelayID:
            result = @"PhysicalContactUnoccupiedToOccupiedDelay";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedThresholdID:
            result = @"PhysicalContactUnoccupiedToOccupiedThreshold";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeCarbonMonoxideConcentrationMeasurementID:

        switch (attributeID) {

            // Cluster CarbonMonoxideConcentrationMeasurement attributes
        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeMeasuredValueID:
            result = @"MeasuredValue";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeMinMeasuredValueID:
            result = @"MinMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeMaxMeasuredValueID:
            result = @"MaxMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributePeakMeasuredValueID:
            result = @"PeakMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributePeakMeasuredValueWindowID:
            result = @"PeakMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeAverageMeasuredValueID:
            result = @"AverageMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeAverageMeasuredValueWindowID:
            result = @"AverageMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeUncertaintyID:
            result = @"Uncertainty";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeMeasurementUnitID:
            result = @"MeasurementUnit";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeMeasurementMediumID:
            result = @"MeasurementMedium";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeLevelValueID:
            result = @"LevelValue";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeCarbonDioxideConcentrationMeasurementID:

        switch (attributeID) {

            // Cluster CarbonDioxideConcentrationMeasurement attributes
        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeMeasuredValueID:
            result = @"MeasuredValue";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeMinMeasuredValueID:
            result = @"MinMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeMaxMeasuredValueID:
            result = @"MaxMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributePeakMeasuredValueID:
            result = @"PeakMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributePeakMeasuredValueWindowID:
            result = @"PeakMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeAverageMeasuredValueID:
            result = @"AverageMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeAverageMeasuredValueWindowID:
            result = @"AverageMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeUncertaintyID:
            result = @"Uncertainty";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeMeasurementUnitID:
            result = @"MeasurementUnit";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeMeasurementMediumID:
            result = @"MeasurementMedium";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeLevelValueID:
            result = @"LevelValue";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeNitrogenDioxideConcentrationMeasurementID:

        switch (attributeID) {

            // Cluster NitrogenDioxideConcentrationMeasurement attributes
        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeMeasuredValueID:
            result = @"MeasuredValue";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeMinMeasuredValueID:
            result = @"MinMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeMaxMeasuredValueID:
            result = @"MaxMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributePeakMeasuredValueID:
            result = @"PeakMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributePeakMeasuredValueWindowID:
            result = @"PeakMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeAverageMeasuredValueID:
            result = @"AverageMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeAverageMeasuredValueWindowID:
            result = @"AverageMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeUncertaintyID:
            result = @"Uncertainty";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeMeasurementUnitID:
            result = @"MeasurementUnit";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeMeasurementMediumID:
            result = @"MeasurementMedium";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeLevelValueID:
            result = @"LevelValue";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeOzoneConcentrationMeasurementID:

        switch (attributeID) {

            // Cluster OzoneConcentrationMeasurement attributes
        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeMeasuredValueID:
            result = @"MeasuredValue";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeMinMeasuredValueID:
            result = @"MinMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeMaxMeasuredValueID:
            result = @"MaxMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributePeakMeasuredValueID:
            result = @"PeakMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributePeakMeasuredValueWindowID:
            result = @"PeakMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeAverageMeasuredValueID:
            result = @"AverageMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeAverageMeasuredValueWindowID:
            result = @"AverageMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeUncertaintyID:
            result = @"Uncertainty";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeMeasurementUnitID:
            result = @"MeasurementUnit";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeMeasurementMediumID:
            result = @"MeasurementMedium";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeLevelValueID:
            result = @"LevelValue";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypePM25ConcentrationMeasurementID:

        switch (attributeID) {

            // Cluster PM25ConcentrationMeasurement attributes
        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeMeasuredValueID:
            result = @"MeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeMinMeasuredValueID:
            result = @"MinMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeMaxMeasuredValueID:
            result = @"MaxMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributePeakMeasuredValueID:
            result = @"PeakMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributePeakMeasuredValueWindowID:
            result = @"PeakMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeAverageMeasuredValueID:
            result = @"AverageMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeAverageMeasuredValueWindowID:
            result = @"AverageMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeUncertaintyID:
            result = @"Uncertainty";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeMeasurementUnitID:
            result = @"MeasurementUnit";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeMeasurementMediumID:
            result = @"MeasurementMedium";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeLevelValueID:
            result = @"LevelValue";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeFormaldehydeConcentrationMeasurementID:

        switch (attributeID) {

            // Cluster FormaldehydeConcentrationMeasurement attributes
        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeMeasuredValueID:
            result = @"MeasuredValue";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeMinMeasuredValueID:
            result = @"MinMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeMaxMeasuredValueID:
            result = @"MaxMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributePeakMeasuredValueID:
            result = @"PeakMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributePeakMeasuredValueWindowID:
            result = @"PeakMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeAverageMeasuredValueID:
            result = @"AverageMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeAverageMeasuredValueWindowID:
            result = @"AverageMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeUncertaintyID:
            result = @"Uncertainty";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeMeasurementUnitID:
            result = @"MeasurementUnit";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeMeasurementMediumID:
            result = @"MeasurementMedium";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeLevelValueID:
            result = @"LevelValue";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypePM1ConcentrationMeasurementID:

        switch (attributeID) {

            // Cluster PM1ConcentrationMeasurement attributes
        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeMeasuredValueID:
            result = @"MeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeMinMeasuredValueID:
            result = @"MinMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeMaxMeasuredValueID:
            result = @"MaxMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributePeakMeasuredValueID:
            result = @"PeakMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributePeakMeasuredValueWindowID:
            result = @"PeakMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeAverageMeasuredValueID:
            result = @"AverageMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeAverageMeasuredValueWindowID:
            result = @"AverageMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeUncertaintyID:
            result = @"Uncertainty";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeMeasurementUnitID:
            result = @"MeasurementUnit";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeMeasurementMediumID:
            result = @"MeasurementMedium";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeLevelValueID:
            result = @"LevelValue";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypePM10ConcentrationMeasurementID:

        switch (attributeID) {

            // Cluster PM10ConcentrationMeasurement attributes
        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeMeasuredValueID:
            result = @"MeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeMinMeasuredValueID:
            result = @"MinMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeMaxMeasuredValueID:
            result = @"MaxMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributePeakMeasuredValueID:
            result = @"PeakMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributePeakMeasuredValueWindowID:
            result = @"PeakMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeAverageMeasuredValueID:
            result = @"AverageMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeAverageMeasuredValueWindowID:
            result = @"AverageMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeUncertaintyID:
            result = @"Uncertainty";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeMeasurementUnitID:
            result = @"MeasurementUnit";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeMeasurementMediumID:
            result = @"MeasurementMedium";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeLevelValueID:
            result = @"LevelValue";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeTotalVolatileOrganicCompoundsConcentrationMeasurementID:

        switch (attributeID) {

            // Cluster TotalVolatileOrganicCompoundsConcentrationMeasurement attributes
        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeMeasuredValueID:
            result = @"MeasuredValue";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeMinMeasuredValueID:
            result = @"MinMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeMaxMeasuredValueID:
            result = @"MaxMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributePeakMeasuredValueID:
            result = @"PeakMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributePeakMeasuredValueWindowID:
            result = @"PeakMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeAverageMeasuredValueID:
            result = @"AverageMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeAverageMeasuredValueWindowID:
            result = @"AverageMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeUncertaintyID:
            result = @"Uncertainty";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeMeasurementUnitID:
            result = @"MeasurementUnit";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeMeasurementMediumID:
            result = @"MeasurementMedium";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeLevelValueID:
            result = @"LevelValue";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeRadonConcentrationMeasurementID:

        switch (attributeID) {

            // Cluster RadonConcentrationMeasurement attributes
        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeMeasuredValueID:
            result = @"MeasuredValue";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeMinMeasuredValueID:
            result = @"MinMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeMaxMeasuredValueID:
            result = @"MaxMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributePeakMeasuredValueID:
            result = @"PeakMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributePeakMeasuredValueWindowID:
            result = @"PeakMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeAverageMeasuredValueID:
            result = @"AverageMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeAverageMeasuredValueWindowID:
            result = @"AverageMeasuredValueWindow";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeUncertaintyID:
            result = @"Uncertainty";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeMeasurementUnitID:
            result = @"MeasurementUnit";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeMeasurementMediumID:
            result = @"MeasurementMedium";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeLevelValueID:
            result = @"LevelValue";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeWakeOnLANID:

        switch (attributeID) {

            // Cluster WakeOnLAN attributes
        case MTRAttributeIDTypeClusterWakeOnLANAttributeMACAddressID:
            result = @"MACAddress";
            break;

        case MTRAttributeIDTypeClusterWakeOnLANAttributeLinkLocalAddressID:
            result = @"LinkLocalAddress";
            break;

        case MTRAttributeIDTypeClusterWakeOnLANAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterWakeOnLANAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterWakeOnLANAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterWakeOnLANAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterWakeOnLANAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterWakeOnLANAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeChannelID:

        switch (attributeID) {

            // Cluster Channel attributes
        case MTRAttributeIDTypeClusterChannelAttributeChannelListID:
            result = @"ChannelList";
            break;

        case MTRAttributeIDTypeClusterChannelAttributeLineupID:
            result = @"Lineup";
            break;

        case MTRAttributeIDTypeClusterChannelAttributeCurrentChannelID:
            result = @"CurrentChannel";
            break;

        case MTRAttributeIDTypeClusterChannelAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterChannelAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterChannelAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterChannelAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterChannelAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterChannelAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeTargetNavigatorID:

        switch (attributeID) {

            // Cluster TargetNavigator attributes
        case MTRAttributeIDTypeClusterTargetNavigatorAttributeTargetListID:
            result = @"TargetList";
            break;

        case MTRAttributeIDTypeClusterTargetNavigatorAttributeCurrentTargetID:
            result = @"CurrentTarget";
            break;

        case MTRAttributeIDTypeClusterTargetNavigatorAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterTargetNavigatorAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterTargetNavigatorAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterTargetNavigatorAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterTargetNavigatorAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterTargetNavigatorAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeMediaPlaybackID:

        switch (attributeID) {

            // Cluster MediaPlayback attributes
        case MTRAttributeIDTypeClusterMediaPlaybackAttributeCurrentStateID:
            result = @"CurrentState";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributeStartTimeID:
            result = @"StartTime";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributeDurationID:
            result = @"Duration";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributeSampledPositionID:
            result = @"SampledPosition";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributePlaybackSpeedID:
            result = @"PlaybackSpeed";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeEndID:
            result = @"SeekRangeEnd";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeStartID:
            result = @"SeekRangeStart";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributeActiveAudioTrackID:
            result = @"ActiveAudioTrack";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributeAvailableAudioTracksID:
            result = @"AvailableAudioTracks";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributeActiveTextTrackID:
            result = @"ActiveTextTrack";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributeAvailableTextTracksID:
            result = @"AvailableTextTracks";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterMediaPlaybackAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeMediaInputID:

        switch (attributeID) {

            // Cluster MediaInput attributes
        case MTRAttributeIDTypeClusterMediaInputAttributeInputListID:
            result = @"InputList";
            break;

        case MTRAttributeIDTypeClusterMediaInputAttributeCurrentInputID:
            result = @"CurrentInput";
            break;

        case MTRAttributeIDTypeClusterMediaInputAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterMediaInputAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterMediaInputAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterMediaInputAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterMediaInputAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterMediaInputAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeLowPowerID:

        switch (attributeID) {

            // Cluster LowPower attributes
        case MTRAttributeIDTypeClusterLowPowerAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterLowPowerAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterLowPowerAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterLowPowerAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterLowPowerAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterLowPowerAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeKeypadInputID:

        switch (attributeID) {

            // Cluster KeypadInput attributes
        case MTRAttributeIDTypeClusterKeypadInputAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterKeypadInputAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterKeypadInputAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterKeypadInputAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterKeypadInputAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterKeypadInputAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeContentLauncherID:

        switch (attributeID) {

            // Cluster ContentLauncher attributes
        case MTRAttributeIDTypeClusterContentLauncherAttributeAcceptHeaderID:
            result = @"AcceptHeader";
            break;

        case MTRAttributeIDTypeClusterContentLauncherAttributeSupportedStreamingProtocolsID:
            result = @"SupportedStreamingProtocols";
            break;

        case MTRAttributeIDTypeClusterContentLauncherAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterContentLauncherAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterContentLauncherAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterContentLauncherAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterContentLauncherAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterContentLauncherAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeAudioOutputID:

        switch (attributeID) {

            // Cluster AudioOutput attributes
        case MTRAttributeIDTypeClusterAudioOutputAttributeOutputListID:
            result = @"OutputList";
            break;

        case MTRAttributeIDTypeClusterAudioOutputAttributeCurrentOutputID:
            result = @"CurrentOutput";
            break;

        case MTRAttributeIDTypeClusterAudioOutputAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterAudioOutputAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterAudioOutputAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterAudioOutputAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterAudioOutputAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterAudioOutputAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeApplicationLauncherID:

        switch (attributeID) {

            // Cluster ApplicationLauncher attributes
        case MTRAttributeIDTypeClusterApplicationLauncherAttributeCatalogListID:
            result = @"CatalogList";
            break;

        case MTRAttributeIDTypeClusterApplicationLauncherAttributeCurrentAppID:
            result = @"CurrentApp";
            break;

        case MTRAttributeIDTypeClusterApplicationLauncherAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterApplicationLauncherAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterApplicationLauncherAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterApplicationLauncherAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterApplicationLauncherAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterApplicationLauncherAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeApplicationBasicID:

        switch (attributeID) {

            // Cluster ApplicationBasic attributes
        case MTRAttributeIDTypeClusterApplicationBasicAttributeVendorNameID:
            result = @"VendorName";
            break;

        case MTRAttributeIDTypeClusterApplicationBasicAttributeVendorIDID:
            result = @"VendorID";
            break;

        case MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationNameID:
            result = @"ApplicationName";
            break;

        case MTRAttributeIDTypeClusterApplicationBasicAttributeProductIDID:
            result = @"ProductID";
            break;

        case MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationID:
            result = @"Application";
            break;

        case MTRAttributeIDTypeClusterApplicationBasicAttributeStatusID:
            result = @"Status";
            break;

        case MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationVersionID:
            result = @"ApplicationVersion";
            break;

        case MTRAttributeIDTypeClusterApplicationBasicAttributeAllowedVendorListID:
            result = @"AllowedVendorList";
            break;

        case MTRAttributeIDTypeClusterApplicationBasicAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterApplicationBasicAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterApplicationBasicAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterApplicationBasicAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterApplicationBasicAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterApplicationBasicAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeAccountLoginID:

        switch (attributeID) {

            // Cluster AccountLogin attributes
        case MTRAttributeIDTypeClusterAccountLoginAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterAccountLoginAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterAccountLoginAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterAccountLoginAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterAccountLoginAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterAccountLoginAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeContentControlID:

        switch (attributeID) {

            // Cluster ContentControl attributes
        case MTRAttributeIDTypeClusterContentControlAttributeEnabledID:
            result = @"Enabled";
            break;

        case MTRAttributeIDTypeClusterContentControlAttributeOnDemandRatingsID:
            result = @"OnDemandRatings";
            break;

        case MTRAttributeIDTypeClusterContentControlAttributeOnDemandRatingThresholdID:
            result = @"OnDemandRatingThreshold";
            break;

        case MTRAttributeIDTypeClusterContentControlAttributeScheduledContentRatingsID:
            result = @"ScheduledContentRatings";
            break;

        case MTRAttributeIDTypeClusterContentControlAttributeScheduledContentRatingThresholdID:
            result = @"ScheduledContentRatingThreshold";
            break;

        case MTRAttributeIDTypeClusterContentControlAttributeScreenDailyTimeID:
            result = @"ScreenDailyTime";
            break;

        case MTRAttributeIDTypeClusterContentControlAttributeRemainingScreenTimeID:
            result = @"RemainingScreenTime";
            break;

        case MTRAttributeIDTypeClusterContentControlAttributeBlockUnratedID:
            result = @"BlockUnrated";
            break;

        case MTRAttributeIDTypeClusterContentControlAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterContentControlAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterContentControlAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterContentControlAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterContentControlAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterContentControlAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeContentAppObserverID:

        switch (attributeID) {

            // Cluster ContentAppObserver attributes
        case MTRAttributeIDTypeClusterContentAppObserverAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterContentAppObserverAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterContentAppObserverAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterContentAppObserverAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterContentAppObserverAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterContentAppObserverAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeElectricalMeasurementID:

        switch (attributeID) {

            // Cluster ElectricalMeasurement attributes
        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasurementTypeID:
            result = @"MeasurementType";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageID:
            result = @"DcVoltage";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMinID:
            result = @"DcVoltageMin";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMaxID:
            result = @"DcVoltageMax";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentID:
            result = @"DcCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMinID:
            result = @"DcCurrentMin";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMaxID:
            result = @"DcCurrentMax";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerID:
            result = @"DcPower";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMinID:
            result = @"DcPowerMin";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMaxID:
            result = @"DcPowerMax";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMultiplierID:
            result = @"DcVoltageMultiplier";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageDivisorID:
            result = @"DcVoltageDivisor";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMultiplierID:
            result = @"DcCurrentMultiplier";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentDivisorID:
            result = @"DcCurrentDivisor";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMultiplierID:
            result = @"DcPowerMultiplier";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerDivisorID:
            result = @"DcPowerDivisor";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyID:
            result = @"AcFrequency";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMinID:
            result = @"AcFrequencyMin";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMaxID:
            result = @"AcFrequencyMax";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeNeutralCurrentID:
            result = @"NeutralCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalActivePowerID:
            result = @"TotalActivePower";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalReactivePowerID:
            result = @"TotalReactivePower";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalApparentPowerID:
            result = @"TotalApparentPower";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured1stHarmonicCurrentID:
            result = @"Measured1stHarmonicCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured3rdHarmonicCurrentID:
            result = @"Measured3rdHarmonicCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured5thHarmonicCurrentID:
            result = @"Measured5thHarmonicCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured7thHarmonicCurrentID:
            result = @"Measured7thHarmonicCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured9thHarmonicCurrentID:
            result = @"Measured9thHarmonicCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured11thHarmonicCurrentID:
            result = @"Measured11thHarmonicCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase1stHarmonicCurrentID:
            result = @"MeasuredPhase1stHarmonicCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase3rdHarmonicCurrentID:
            result = @"MeasuredPhase3rdHarmonicCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase5thHarmonicCurrentID:
            result = @"MeasuredPhase5thHarmonicCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase7thHarmonicCurrentID:
            result = @"MeasuredPhase7thHarmonicCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase9thHarmonicCurrentID:
            result = @"MeasuredPhase9thHarmonicCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase11thHarmonicCurrentID:
            result = @"MeasuredPhase11thHarmonicCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMultiplierID:
            result = @"AcFrequencyMultiplier";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyDivisorID:
            result = @"AcFrequencyDivisor";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerMultiplierID:
            result = @"PowerMultiplier";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerDivisorID:
            result = @"PowerDivisor";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeHarmonicCurrentMultiplierID:
            result = @"HarmonicCurrentMultiplier";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributePhaseHarmonicCurrentMultiplierID:
            result = @"PhaseHarmonicCurrentMultiplier";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousVoltageID:
            result = @"InstantaneousVoltage";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousLineCurrentID:
            result = @"InstantaneousLineCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousActiveCurrentID:
            result = @"InstantaneousActiveCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousReactiveCurrentID:
            result = @"InstantaneousReactiveCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousPowerID:
            result = @"InstantaneousPower";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageID:
            result = @"RmsVoltage";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinID:
            result = @"RmsVoltageMin";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxID:
            result = @"RmsVoltageMax";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentID:
            result = @"RmsCurrent";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinID:
            result = @"RmsCurrentMin";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxID:
            result = @"RmsCurrentMax";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerID:
            result = @"ActivePower";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinID:
            result = @"ActivePowerMin";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxID:
            result = @"ActivePowerMax";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerID:
            result = @"ReactivePower";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerID:
            result = @"ApparentPower";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorID:
            result = @"PowerFactor";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodID:
            result = @"AverageRmsVoltageMeasurementPeriod";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterID:
            result = @"AverageRmsUnderVoltageCounter";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodID:
            result = @"RmsExtremeOverVoltagePeriod";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodID:
            result = @"RmsExtremeUnderVoltagePeriod";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodID:
            result = @"RmsVoltageSagPeriod";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodID:
            result = @"RmsVoltageSwellPeriod";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageMultiplierID:
            result = @"AcVoltageMultiplier";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageDivisorID:
            result = @"AcVoltageDivisor";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentMultiplierID:
            result = @"AcCurrentMultiplier";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentDivisorID:
            result = @"AcCurrentDivisor";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerMultiplierID:
            result = @"AcPowerMultiplier";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerDivisorID:
            result = @"AcPowerDivisor";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeOverloadAlarmsMaskID:
            result = @"OverloadAlarmsMask";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeVoltageOverloadID:
            result = @"VoltageOverload";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeCurrentOverloadID:
            result = @"CurrentOverload";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcOverloadAlarmsMaskID:
            result = @"AcOverloadAlarmsMask";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageOverloadID:
            result = @"AcVoltageOverload";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentOverloadID:
            result = @"AcCurrentOverload";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcActivePowerOverloadID:
            result = @"AcActivePowerOverload";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcReactivePowerOverloadID:
            result = @"AcReactivePowerOverload";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageID:
            result = @"AverageRmsOverVoltage";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageID:
            result = @"AverageRmsUnderVoltage";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltageID:
            result = @"RmsExtremeOverVoltage";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltageID:
            result = @"RmsExtremeUnderVoltage";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagID:
            result = @"RmsVoltageSag";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellID:
            result = @"RmsVoltageSwell";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseBID:
            result = @"LineCurrentPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseBID:
            result = @"ActiveCurrentPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseBID:
            result = @"ReactiveCurrentPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseBID:
            result = @"RmsVoltagePhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseBID:
            result = @"RmsVoltageMinPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseBID:
            result = @"RmsVoltageMaxPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseBID:
            result = @"RmsCurrentPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseBID:
            result = @"RmsCurrentMinPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseBID:
            result = @"RmsCurrentMaxPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseBID:
            result = @"ActivePowerPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseBID:
            result = @"ActivePowerMinPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseBID:
            result = @"ActivePowerMaxPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseBID:
            result = @"ReactivePowerPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseBID:
            result = @"ApparentPowerPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseBID:
            result = @"PowerFactorPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseBID:
            result = @"AverageRmsVoltageMeasurementPeriodPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseBID:
            result = @"AverageRmsOverVoltageCounterPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseBID:
            result = @"AverageRmsUnderVoltageCounterPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseBID:
            result = @"RmsExtremeOverVoltagePeriodPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseBID:
            result = @"RmsExtremeUnderVoltagePeriodPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseBID:
            result = @"RmsVoltageSagPeriodPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseBID:
            result = @"RmsVoltageSwellPeriodPhaseB";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseCID:
            result = @"LineCurrentPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseCID:
            result = @"ActiveCurrentPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseCID:
            result = @"ReactiveCurrentPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseCID:
            result = @"RmsVoltagePhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseCID:
            result = @"RmsVoltageMinPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseCID:
            result = @"RmsVoltageMaxPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseCID:
            result = @"RmsCurrentPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseCID:
            result = @"RmsCurrentMinPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseCID:
            result = @"RmsCurrentMaxPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseCID:
            result = @"ActivePowerPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseCID:
            result = @"ActivePowerMinPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseCID:
            result = @"ActivePowerMaxPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseCID:
            result = @"ReactivePowerPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseCID:
            result = @"ApparentPowerPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseCID:
            result = @"PowerFactorPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseCID:
            result = @"AverageRmsVoltageMeasurementPeriodPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseCID:
            result = @"AverageRmsOverVoltageCounterPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseCID:
            result = @"AverageRmsUnderVoltageCounterPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseCID:
            result = @"RmsExtremeOverVoltagePeriodPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseCID:
            result = @"RmsExtremeUnderVoltagePeriodPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseCID:
            result = @"RmsVoltageSagPeriodPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseCID:
            result = @"RmsVoltageSwellPeriodPhaseC";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterElectricalMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeUnitTestingID:

        switch (attributeID) {

            // Cluster UnitTesting attributes
        case MTRAttributeIDTypeClusterUnitTestingAttributeBooleanID:
            result = @"Boolean";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeBitmap8ID:
            result = @"Bitmap8";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeBitmap16ID:
            result = @"Bitmap16";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeBitmap32ID:
            result = @"Bitmap32";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeBitmap64ID:
            result = @"Bitmap64";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt8uID:
            result = @"Int8u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt16uID:
            result = @"Int16u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt24uID:
            result = @"Int24u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt32uID:
            result = @"Int32u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt40uID:
            result = @"Int40u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt48uID:
            result = @"Int48u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt56uID:
            result = @"Int56u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt64uID:
            result = @"Int64u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt8sID:
            result = @"Int8s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt16sID:
            result = @"Int16s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt24sID:
            result = @"Int24s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt32sID:
            result = @"Int32s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt40sID:
            result = @"Int40s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt48sID:
            result = @"Int48s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt56sID:
            result = @"Int56s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeInt64sID:
            result = @"Int64s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeEnum8ID:
            result = @"Enum8";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeEnum16ID:
            result = @"Enum16";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeFloatSingleID:
            result = @"FloatSingle";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeFloatDoubleID:
            result = @"FloatDouble";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeOctetStringID:
            result = @"OctetString";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeListInt8uID:
            result = @"ListInt8u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeListOctetStringID:
            result = @"ListOctetString";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeListStructOctetStringID:
            result = @"ListStructOctetString";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeLongOctetStringID:
            result = @"LongOctetString";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeCharStringID:
            result = @"CharString";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeLongCharStringID:
            result = @"LongCharString";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeEpochUsID:
            result = @"EpochUs";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeEpochSID:
            result = @"EpochS";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeVendorIdID:
            result = @"VendorId";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeListNullablesAndOptionalsStructID:
            result = @"ListNullablesAndOptionalsStruct";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeEnumAttrID:
            result = @"EnumAttr";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeStructAttrID:
            result = @"StructAttr";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8uID:
            result = @"RangeRestrictedInt8u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8sID:
            result = @"RangeRestrictedInt8s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16uID:
            result = @"RangeRestrictedInt16u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16sID:
            result = @"RangeRestrictedInt16s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeListLongOctetStringID:
            result = @"ListLongOctetString";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeListFabricScopedID:
            result = @"ListFabricScoped";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeTimedWriteBooleanID:
            result = @"TimedWriteBoolean";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeGeneralErrorBooleanID:
            result = @"GeneralErrorBoolean";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeClusterErrorBooleanID:
            result = @"ClusterErrorBoolean";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeUnsupportedID:
            result = @"Unsupported";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableBooleanID:
            result = @"NullableBoolean";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap8ID:
            result = @"NullableBitmap8";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap16ID:
            result = @"NullableBitmap16";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap32ID:
            result = @"NullableBitmap32";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap64ID:
            result = @"NullableBitmap64";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8uID:
            result = @"NullableInt8u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16uID:
            result = @"NullableInt16u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24uID:
            result = @"NullableInt24u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32uID:
            result = @"NullableInt32u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40uID:
            result = @"NullableInt40u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48uID:
            result = @"NullableInt48u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56uID:
            result = @"NullableInt56u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64uID:
            result = @"NullableInt64u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8sID:
            result = @"NullableInt8s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16sID:
            result = @"NullableInt16s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24sID:
            result = @"NullableInt24s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32sID:
            result = @"NullableInt32s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40sID:
            result = @"NullableInt40s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48sID:
            result = @"NullableInt48s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56sID:
            result = @"NullableInt56s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64sID:
            result = @"NullableInt64s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum8ID:
            result = @"NullableEnum8";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum16ID:
            result = @"NullableEnum16";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatSingleID:
            result = @"NullableFloatSingle";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatDoubleID:
            result = @"NullableFloatDouble";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableOctetStringID:
            result = @"NullableOctetString";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableCharStringID:
            result = @"NullableCharString";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnumAttrID:
            result = @"NullableEnumAttr";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableStructID:
            result = @"NullableStruct";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8uID:
            result = @"NullableRangeRestrictedInt8u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8sID:
            result = @"NullableRangeRestrictedInt8s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16uID:
            result = @"NullableRangeRestrictedInt16u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16sID:
            result = @"NullableRangeRestrictedInt16s";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeWriteOnlyInt8uID:
            result = @"WriteOnlyInt8u";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeMeiInt8uID:
            result = @"MeiInt8u";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    case MTRClusterIDTypeSampleMEIID:

        switch (attributeID) {

            // Cluster SampleMEI attributes
        case MTRAttributeIDTypeClusterSampleMEIAttributeFlipFlopID:
            result = @"FlipFlop";
            break;

        case MTRAttributeIDTypeClusterSampleMEIAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterSampleMEIAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterSampleMEIAttributeEventListID:
            result = @"EventList";
            break;

        case MTRAttributeIDTypeClusterSampleMEIAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterSampleMEIAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterSampleMEIAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown attributeID %d>", attributeID];
            break;
        }

    default:
        result = [NSString stringWithFormat:@"<Unknown clusterID %d>", clusterID];
        break;
    }

    return result;
}

// @end
