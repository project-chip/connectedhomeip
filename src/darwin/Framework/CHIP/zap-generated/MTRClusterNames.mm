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
    case MTRClusterIDTypeLevelControlID:
        result = @"LevelControl";
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
    case MTRClusterIDTypeWaterHeaterManagementID:
        result = @"WaterHeaterManagement";
        break;
    case MTRClusterIDTypeCommodityPriceID:
        result = @"CommodityPrice";
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
    case MTRClusterIDTypeWaterHeaterModeID:
        result = @"WaterHeaterMode";
        break;
    case MTRClusterIDTypeDeviceEnergyManagementModeID:
        result = @"DeviceEnergyManagementMode";
        break;
    case MTRClusterIDTypeElectricalGridConditionsID:
        result = @"ElectricalGridConditions";
        break;
    case MTRClusterIDTypeDoorLockID:
        result = @"DoorLock";
        break;
    case MTRClusterIDTypeWindowCoveringID:
        result = @"WindowCovering";
        break;
    case MTRClusterIDTypeClosureControlID:
        result = @"ClosureControl";
        break;
    case MTRClusterIDTypeClosureDimensionID:
        result = @"ClosureDimension";
        break;
    case MTRClusterIDTypeServiceAreaID:
        result = @"ServiceArea";
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
    case MTRClusterIDTypeSoilMeasurementID:
        result = @"SoilMeasurement";
        break;
    case MTRClusterIDTypeWiFiNetworkManagementID:
        result = @"WiFiNetworkManagement";
        break;
    case MTRClusterIDTypeThreadBorderRouterManagementID:
        result = @"ThreadBorderRouterManagement";
        break;
    case MTRClusterIDTypeThreadNetworkDirectoryID:
        result = @"ThreadNetworkDirectory";
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
    case MTRClusterIDTypeZoneManagementID:
        result = @"ZoneManagement";
        break;
    case MTRClusterIDTypeCameraAVStreamManagementID:
        result = @"CameraAVStreamManagement";
        break;
    case MTRClusterIDTypeCameraAVSettingsUserLevelManagementID:
        result = @"CameraAVSettingsUserLevelManagement";
        break;
    case MTRClusterIDTypeWebRTCTransportProviderID:
        result = @"WebRTCTransportProvider";
        break;
    case MTRClusterIDTypeWebRTCTransportRequestorID:
        result = @"WebRTCTransportRequestor";
        break;
    case MTRClusterIDTypePushAVStreamTransportID:
        result = @"PushAVStreamTransport";
        break;
    case MTRClusterIDTypeChimeID:
        result = @"Chime";
        break;
    case MTRClusterIDTypeCommodityTariffID:
        result = @"CommodityTariff";
        break;
    case MTRClusterIDTypeEcosystemInformationID:
        result = @"EcosystemInformation";
        break;
    case MTRClusterIDTypeCommissionerControlID:
        result = @"CommissionerControl";
        break;
    case MTRClusterIDTypeJointFabricDatastoreID:
        result = @"JointFabricDatastore";
        break;
    case MTRClusterIDTypeJointFabricAdministratorID:
        result = @"JointFabricAdministrator";
        break;
    case MTRClusterIDTypeTLSCertificateManagementID:
        result = @"TLSCertificateManagement";
        break;
    case MTRClusterIDTypeTLSClientManagementID:
        result = @"TLSClientManagement";
        break;
    case MTRClusterIDTypeMeterIdentificationID:
        result = @"MeterIdentification";
        break;
    case MTRClusterIDTypeCommodityMeteringID:
        result = @"CommodityMetering";
        break;
    case MTRClusterIDTypeUnitTestingID:
        result = @"UnitTesting";
        break;
    case MTRClusterIDTypeSampleMEIID:
        result = @"SampleMEI";
        break;

    default:
        result = [NSString stringWithFormat:@"<Unknown clusterID %u>", clusterID];
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
            // Not a known Identify attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known Groups attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known OnOff attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known LevelControl attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypePulseWidthModulationID:

        switch (attributeID) {

            // Cluster PulseWidthModulation attributes
        case MTRAttributeIDTypeClusterPulseWidthModulationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterPulseWidthModulationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known PulseWidthModulation attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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

        case MTRAttributeIDTypeClusterDescriptorAttributeEndpointUniqueIDID:
            result = @"EndpointUniqueID";
            break;

        case MTRAttributeIDTypeClusterDescriptorAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterDescriptorAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known Descriptor attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known Binding attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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

        case MTRAttributeIDTypeClusterAccessControlAttributeCommissioningARLID:
            result = @"CommissioningARL";
            break;

        case MTRAttributeIDTypeClusterAccessControlAttributeARLID:
            result = @"ARL";
            break;

        case MTRAttributeIDTypeClusterAccessControlAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterAccessControlAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known AccessControl attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known Actions attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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

        case MTRAttributeIDTypeClusterBasicInformationAttributeConfigurationVersionID:
            result = @"ConfigurationVersion";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterBasicInformationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known BasicInformation attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeOTASoftwareUpdateProviderID:

        switch (attributeID) {

            // Cluster OTASoftwareUpdateProvider attributes
        case MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known OTASoftwareUpdateProvider attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known OTASoftwareUpdateRequestor attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known LocalizationConfiguration attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known TimeFormatLocalization attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeUnitLocalizationID:

        switch (attributeID) {

            // Cluster UnitLocalization attributes
        case MTRAttributeIDTypeClusterUnitLocalizationAttributeTemperatureUnitID:
            result = @"TemperatureUnit";
            break;

        case MTRAttributeIDTypeClusterUnitLocalizationAttributeSupportedTemperatureUnitsID:
            result = @"SupportedTemperatureUnits";
            break;

        case MTRAttributeIDTypeClusterUnitLocalizationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterUnitLocalizationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known UnitLocalization attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known PowerSourceConfiguration attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known PowerSource attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeTCAcceptedVersionID:
            result = @"TCAcceptedVersion";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeTCMinRequiredVersionID:
            result = @"TCMinRequiredVersion";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeTCAcknowledgementsID:
            result = @"TCAcknowledgements";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeTCAcknowledgementsRequiredID:
            result = @"TCAcknowledgementsRequired";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeTCUpdateDeadlineID:
            result = @"TCUpdateDeadline";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeRecoveryIdentifierID:
            result = @"RecoveryIdentifier";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeNetworkRecoveryReasonID:
            result = @"NetworkRecoveryReason";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeIsCommissioningWithoutPowerID:
            result = @"IsCommissioningWithoutPower";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterGeneralCommissioningAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known GeneralCommissioning attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known NetworkCommissioning attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeDiagnosticLogsID:

        switch (attributeID) {

            // Cluster DiagnosticLogs attributes
        case MTRAttributeIDTypeClusterDiagnosticLogsAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterDiagnosticLogsAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known DiagnosticLogs attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known GeneralDiagnostics attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known SoftwareDiagnostics attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeExtAddressID:
            result = @"ExtAddress";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRloc16ID:
            result = @"Rloc16";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known ThreadNetworkDiagnostics attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known WiFiNetworkDiagnostics attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known EthernetNetworkDiagnostics attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known TimeSynchronization attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductIDID:
            result = @"ProductID";
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

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeConfigurationVersionID:
            result = @"ConfigurationVersion";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known BridgedDeviceBasicInformation attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known Switch attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known AdministratorCommissioning attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known OperationalCredentials attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known GroupKeyManagement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known FixedLabel attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known UserLabel attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known BooleanState attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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

        case MTRAttributeIDTypeClusterICDManagementAttributeMaximumCheckInBackOffID:
            result = @"MaximumCheckInBackOff";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterICDManagementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known ICDManagement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known Timer attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known OvenCavityOperationalState attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeOvenModeID:

        switch (attributeID) {

            // Cluster OvenMode attributes
        case MTRAttributeIDTypeClusterOvenModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterOvenModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterOvenModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterOvenModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known OvenMode attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known LaundryDryerControls attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known ModeSelect attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeLaundryWasherModeID:

        switch (attributeID) {

            // Cluster LaundryWasherMode attributes
        case MTRAttributeIDTypeClusterLaundryWasherModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterLaundryWasherModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known LaundryWasherMode attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeRefrigeratorAndTemperatureControlledCabinetModeID:

        switch (attributeID) {

            // Cluster RefrigeratorAndTemperatureControlledCabinetMode attributes
        case MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known RefrigeratorAndTemperatureControlledCabinetMode attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known LaundryWasherControls attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known RVCRunMode attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known RVCCleanMode attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known TemperatureControl attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known RefrigeratorAlarm attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeDishwasherModeID:

        switch (attributeID) {

            // Cluster DishwasherMode attributes
        case MTRAttributeIDTypeClusterDishwasherModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterDishwasherModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterDishwasherModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterDishwasherModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known DishwasherMode attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known AirQuality attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known SmokeCOAlarm attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known DishwasherAlarm attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known MicrowaveOvenMode attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known MicrowaveOvenControl attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known OperationalState attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known RVCOperationalState attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeScenesManagementID:

        switch (attributeID) {

            // Cluster ScenesManagement attributes
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
            // Not a known ScenesManagement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known HEPAFilterMonitoring attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known ActivatedCarbonFilterMonitoring attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known BooleanStateConfiguration attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known ValveConfigurationAndControl attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known ElectricalPowerMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known ElectricalEnergyMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeWaterHeaterManagementID:

        switch (attributeID) {

            // Cluster WaterHeaterManagement attributes
        case MTRAttributeIDTypeClusterWaterHeaterManagementAttributeHeaterTypesID:
            result = @"HeaterTypes";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterManagementAttributeHeatDemandID:
            result = @"HeatDemand";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterManagementAttributeTankVolumeID:
            result = @"TankVolume";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterManagementAttributeEstimatedHeatRequiredID:
            result = @"EstimatedHeatRequired";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterManagementAttributeTankPercentageID:
            result = @"TankPercentage";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterManagementAttributeBoostStateID:
            result = @"BoostState";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterManagementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterManagementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterManagementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterManagementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterManagementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known WaterHeaterManagement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeCommodityPriceID:

        switch (attributeID) {

            // Cluster CommodityPrice attributes
        case MTRAttributeIDTypeClusterCommodityPriceAttributeTariffUnitID:
            result = @"TariffUnit";
            break;

        case MTRAttributeIDTypeClusterCommodityPriceAttributeCurrencyID:
            result = @"Currency";
            break;

        case MTRAttributeIDTypeClusterCommodityPriceAttributeCurrentPriceID:
            result = @"CurrentPrice";
            break;

        case MTRAttributeIDTypeClusterCommodityPriceAttributePriceForecastID:
            result = @"PriceForecast";
            break;

        case MTRAttributeIDTypeClusterCommodityPriceAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterCommodityPriceAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterCommodityPriceAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterCommodityPriceAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterCommodityPriceAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known CommodityPrice attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known Messages attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known DeviceEnergyManagement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known EnergyEVSE attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known EnergyPreference attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known PowerTopology attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeEnergyEVSEModeID:

        switch (attributeID) {

            // Cluster EnergyEVSEMode attributes
        case MTRAttributeIDTypeClusterEnergyEVSEModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterEnergyEVSEModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known EnergyEVSEMode attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeWaterHeaterModeID:

        switch (attributeID) {

            // Cluster WaterHeaterMode attributes
        case MTRAttributeIDTypeClusterWaterHeaterModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterModeAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterModeAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterWaterHeaterModeAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known WaterHeaterMode attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeDeviceEnergyManagementModeID:

        switch (attributeID) {

            // Cluster DeviceEnergyManagementMode attributes
        case MTRAttributeIDTypeClusterDeviceEnergyManagementModeAttributeSupportedModesID:
            result = @"SupportedModes";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementModeAttributeCurrentModeID:
            result = @"CurrentMode";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementModeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterDeviceEnergyManagementModeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known DeviceEnergyManagementMode attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeElectricalGridConditionsID:

        switch (attributeID) {

            // Cluster ElectricalGridConditions attributes
        case MTRAttributeIDTypeClusterElectricalGridConditionsAttributeLocalGenerationAvailableID:
            result = @"LocalGenerationAvailable";
            break;

        case MTRAttributeIDTypeClusterElectricalGridConditionsAttributeCurrentConditionsID:
            result = @"CurrentConditions";
            break;

        case MTRAttributeIDTypeClusterElectricalGridConditionsAttributeForecastConditionsID:
            result = @"ForecastConditions";
            break;

        case MTRAttributeIDTypeClusterElectricalGridConditionsAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterElectricalGridConditionsAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterElectricalGridConditionsAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterElectricalGridConditionsAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterElectricalGridConditionsAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known ElectricalGridConditions attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known DoorLock attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known WindowCovering attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeClosureControlID:

        switch (attributeID) {

            // Cluster ClosureControl attributes
        case MTRAttributeIDTypeClusterClosureControlAttributeCountdownTimeID:
            result = @"CountdownTime";
            break;

        case MTRAttributeIDTypeClusterClosureControlAttributeMainStateID:
            result = @"MainState";
            break;

        case MTRAttributeIDTypeClusterClosureControlAttributeCurrentErrorListID:
            result = @"CurrentErrorList";
            break;

        case MTRAttributeIDTypeClusterClosureControlAttributeOverallCurrentStateID:
            result = @"OverallCurrentState";
            break;

        case MTRAttributeIDTypeClusterClosureControlAttributeOverallTargetStateID:
            result = @"OverallTargetState";
            break;

        case MTRAttributeIDTypeClusterClosureControlAttributeLatchControlModesID:
            result = @"LatchControlModes";
            break;

        case MTRAttributeIDTypeClusterClosureControlAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterClosureControlAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterClosureControlAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterClosureControlAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterClosureControlAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known ClosureControl attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeClosureDimensionID:

        switch (attributeID) {

            // Cluster ClosureDimension attributes
        case MTRAttributeIDTypeClusterClosureDimensionAttributeCurrentStateID:
            result = @"CurrentState";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeTargetStateID:
            result = @"TargetState";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeResolutionID:
            result = @"Resolution";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeStepValueID:
            result = @"StepValue";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeUnitID:
            result = @"Unit";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeUnitRangeID:
            result = @"UnitRange";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeLimitRangeID:
            result = @"LimitRange";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeTranslationDirectionID:
            result = @"TranslationDirection";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeRotationAxisID:
            result = @"RotationAxis";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeOverflowID:
            result = @"Overflow";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeModulationTypeID:
            result = @"ModulationType";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeLatchControlModesID:
            result = @"LatchControlModes";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterClosureDimensionAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known ClosureDimension attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeServiceAreaID:

        switch (attributeID) {

            // Cluster ServiceArea attributes
        case MTRAttributeIDTypeClusterServiceAreaAttributeSupportedAreasID:
            result = @"SupportedAreas";
            break;

        case MTRAttributeIDTypeClusterServiceAreaAttributeSupportedMapsID:
            result = @"SupportedMaps";
            break;

        case MTRAttributeIDTypeClusterServiceAreaAttributeSelectedAreasID:
            result = @"SelectedAreas";
            break;

        case MTRAttributeIDTypeClusterServiceAreaAttributeCurrentAreaID:
            result = @"CurrentArea";
            break;

        case MTRAttributeIDTypeClusterServiceAreaAttributeEstimatedEndTimeID:
            result = @"EstimatedEndTime";
            break;

        case MTRAttributeIDTypeClusterServiceAreaAttributeProgressID:
            result = @"Progress";
            break;

        case MTRAttributeIDTypeClusterServiceAreaAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterServiceAreaAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterServiceAreaAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterServiceAreaAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterServiceAreaAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known ServiceArea attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known PumpConfigurationAndControl attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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

        case MTRAttributeIDTypeClusterThermostatAttributeSetpointHoldExpiryTimestampID:
            result = @"SetpointHoldExpiryTimestamp";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeMaxThermostatSuggestionsID:
            result = @"MaxThermostatSuggestions";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeThermostatSuggestionsID:
            result = @"ThermostatSuggestions";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeCurrentThermostatSuggestionID:
            result = @"CurrentThermostatSuggestion";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeThermostatSuggestionNotFollowingReasonID:
            result = @"ThermostatSuggestionNotFollowingReason";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterThermostatAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known Thermostat attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known FanControl attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known ThermostatUserInterfaceConfiguration attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known ColorControl attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known BallastConfiguration attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known IlluminanceMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known TemperatureMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known PressureMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known FlowMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known RelativeHumidityMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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

        case MTRAttributeIDTypeClusterOccupancySensingAttributeHoldTimeID:
            result = @"HoldTime";
            break;

        case MTRAttributeIDTypeClusterOccupancySensingAttributeHoldTimeLimitsID:
            result = @"HoldTimeLimits";
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
            // Not a known OccupancySensing attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known CarbonMonoxideConcentrationMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known CarbonDioxideConcentrationMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known NitrogenDioxideConcentrationMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known OzoneConcentrationMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known PM25ConcentrationMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known FormaldehydeConcentrationMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known PM1ConcentrationMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known PM10ConcentrationMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known TotalVolatileOrganicCompoundsConcentrationMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known RadonConcentrationMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeSoilMeasurementID:

        switch (attributeID) {

            // Cluster SoilMeasurement attributes
        case MTRAttributeIDTypeClusterSoilMeasurementAttributeSoilMoistureMeasurementLimitsID:
            result = @"SoilMoistureMeasurementLimits";
            break;

        case MTRAttributeIDTypeClusterSoilMeasurementAttributeSoilMoistureMeasuredValueID:
            result = @"SoilMoistureMeasuredValue";
            break;

        case MTRAttributeIDTypeClusterSoilMeasurementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterSoilMeasurementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterSoilMeasurementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterSoilMeasurementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterSoilMeasurementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known SoilMeasurement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeWiFiNetworkManagementID:

        switch (attributeID) {

            // Cluster WiFiNetworkManagement attributes
        case MTRAttributeIDTypeClusterWiFiNetworkManagementAttributeSSIDID:
            result = @"SSID";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkManagementAttributePassphraseSurrogateID:
            result = @"PassphraseSurrogate";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkManagementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkManagementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkManagementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkManagementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterWiFiNetworkManagementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known WiFiNetworkManagement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeThreadBorderRouterManagementID:

        switch (attributeID) {

            // Cluster ThreadBorderRouterManagement attributes
        case MTRAttributeIDTypeClusterThreadBorderRouterManagementAttributeBorderRouterNameID:
            result = @"BorderRouterName";
            break;

        case MTRAttributeIDTypeClusterThreadBorderRouterManagementAttributeBorderAgentIDID:
            result = @"BorderAgentID";
            break;

        case MTRAttributeIDTypeClusterThreadBorderRouterManagementAttributeThreadVersionID:
            result = @"ThreadVersion";
            break;

        case MTRAttributeIDTypeClusterThreadBorderRouterManagementAttributeInterfaceEnabledID:
            result = @"InterfaceEnabled";
            break;

        case MTRAttributeIDTypeClusterThreadBorderRouterManagementAttributeActiveDatasetTimestampID:
            result = @"ActiveDatasetTimestamp";
            break;

        case MTRAttributeIDTypeClusterThreadBorderRouterManagementAttributePendingDatasetTimestampID:
            result = @"PendingDatasetTimestamp";
            break;

        case MTRAttributeIDTypeClusterThreadBorderRouterManagementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterThreadBorderRouterManagementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterThreadBorderRouterManagementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterThreadBorderRouterManagementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterThreadBorderRouterManagementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known ThreadBorderRouterManagement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeThreadNetworkDirectoryID:

        switch (attributeID) {

            // Cluster ThreadNetworkDirectory attributes
        case MTRAttributeIDTypeClusterThreadNetworkDirectoryAttributePreferredExtendedPanIDID:
            result = @"PreferredExtendedPanID";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDirectoryAttributeThreadNetworksID:
            result = @"ThreadNetworks";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDirectoryAttributeThreadNetworkTableSizeID:
            result = @"ThreadNetworkTableSize";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDirectoryAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDirectoryAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDirectoryAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDirectoryAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterThreadNetworkDirectoryAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known ThreadNetworkDirectory attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known WakeOnLAN attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known Channel attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known TargetNavigator attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known MediaPlayback attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known MediaInput attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeLowPowerID:

        switch (attributeID) {

            // Cluster LowPower attributes
        case MTRAttributeIDTypeClusterLowPowerAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterLowPowerAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known LowPower attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeKeypadInputID:

        switch (attributeID) {

            // Cluster KeypadInput attributes
        case MTRAttributeIDTypeClusterKeypadInputAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterKeypadInputAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known KeypadInput attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known ContentLauncher attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known AudioOutput attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known ApplicationLauncher attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known ApplicationBasic attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeAccountLoginID:

        switch (attributeID) {

            // Cluster AccountLogin attributes
        case MTRAttributeIDTypeClusterAccountLoginAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterAccountLoginAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known AccountLogin attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known ContentControl attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeContentAppObserverID:

        switch (attributeID) {

            // Cluster ContentAppObserver attributes
        case MTRAttributeIDTypeClusterContentAppObserverAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterContentAppObserverAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known ContentAppObserver attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeZoneManagementID:

        switch (attributeID) {

            // Cluster ZoneManagement attributes
        case MTRAttributeIDTypeClusterZoneManagementAttributeMaxUserDefinedZonesID:
            result = @"MaxUserDefinedZones";
            break;

        case MTRAttributeIDTypeClusterZoneManagementAttributeMaxZonesID:
            result = @"MaxZones";
            break;

        case MTRAttributeIDTypeClusterZoneManagementAttributeZonesID:
            result = @"Zones";
            break;

        case MTRAttributeIDTypeClusterZoneManagementAttributeTriggersID:
            result = @"Triggers";
            break;

        case MTRAttributeIDTypeClusterZoneManagementAttributeSensitivityMaxID:
            result = @"SensitivityMax";
            break;

        case MTRAttributeIDTypeClusterZoneManagementAttributeSensitivityID:
            result = @"Sensitivity";
            break;

        case MTRAttributeIDTypeClusterZoneManagementAttributeTwoDCartesianMaxID:
            result = @"TwoDCartesianMax";
            break;

        case MTRAttributeIDTypeClusterZoneManagementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterZoneManagementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterZoneManagementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterZoneManagementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterZoneManagementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known ZoneManagement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeCameraAVStreamManagementID:

        switch (attributeID) {

            // Cluster CameraAVStreamManagement attributes
        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeMaxConcurrentEncodersID:
            result = @"MaxConcurrentEncoders";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeMaxEncodedPixelRateID:
            result = @"MaxEncodedPixelRate";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeVideoSensorParamsID:
            result = @"VideoSensorParams";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeNightVisionUsesInfraredID:
            result = @"NightVisionUsesInfrared";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeMinViewportResolutionID:
            result = @"MinViewportResolution";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeRateDistortionTradeOffPointsID:
            result = @"RateDistortionTradeOffPoints";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeMaxContentBufferSizeID:
            result = @"MaxContentBufferSize";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeMicrophoneCapabilitiesID:
            result = @"MicrophoneCapabilities";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeSpeakerCapabilitiesID:
            result = @"SpeakerCapabilities";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeTwoWayTalkSupportID:
            result = @"TwoWayTalkSupport";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeSnapshotCapabilitiesID:
            result = @"SnapshotCapabilities";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeMaxNetworkBandwidthID:
            result = @"MaxNetworkBandwidth";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeCurrentFrameRateID:
            result = @"CurrentFrameRate";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeHDRModeEnabledID:
            result = @"HDRModeEnabled";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeSupportedStreamUsagesID:
            result = @"SupportedStreamUsages";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeAllocatedVideoStreamsID:
            result = @"AllocatedVideoStreams";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeAllocatedAudioStreamsID:
            result = @"AllocatedAudioStreams";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeAllocatedSnapshotStreamsID:
            result = @"AllocatedSnapshotStreams";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeStreamUsagePrioritiesID:
            result = @"StreamUsagePriorities";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeSoftRecordingPrivacyModeEnabledID:
            result = @"SoftRecordingPrivacyModeEnabled";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeSoftLivestreamPrivacyModeEnabledID:
            result = @"SoftLivestreamPrivacyModeEnabled";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeHardPrivacyModeOnID:
            result = @"HardPrivacyModeOn";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeNightVisionID:
            result = @"NightVision";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeNightVisionIllumID:
            result = @"NightVisionIllum";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeViewportID:
            result = @"Viewport";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeSpeakerMutedID:
            result = @"SpeakerMuted";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeSpeakerVolumeLevelID:
            result = @"SpeakerVolumeLevel";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeSpeakerMaxLevelID:
            result = @"SpeakerMaxLevel";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeSpeakerMinLevelID:
            result = @"SpeakerMinLevel";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeMicrophoneMutedID:
            result = @"MicrophoneMuted";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeMicrophoneVolumeLevelID:
            result = @"MicrophoneVolumeLevel";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeMicrophoneMaxLevelID:
            result = @"MicrophoneMaxLevel";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeMicrophoneMinLevelID:
            result = @"MicrophoneMinLevel";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeMicrophoneAGCEnabledID:
            result = @"MicrophoneAGCEnabled";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeImageRotationID:
            result = @"ImageRotation";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeImageFlipHorizontalID:
            result = @"ImageFlipHorizontal";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeImageFlipVerticalID:
            result = @"ImageFlipVertical";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeLocalVideoRecordingEnabledID:
            result = @"LocalVideoRecordingEnabled";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeLocalSnapshotRecordingEnabledID:
            result = @"LocalSnapshotRecordingEnabled";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeStatusLightEnabledID:
            result = @"StatusLightEnabled";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeStatusLightBrightnessID:
            result = @"StatusLightBrightness";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterCameraAVStreamManagementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known CameraAVStreamManagement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeCameraAVSettingsUserLevelManagementID:

        switch (attributeID) {

            // Cluster CameraAVSettingsUserLevelManagement attributes
        case MTRAttributeIDTypeClusterCameraAVSettingsUserLevelManagementAttributeMPTZPositionID:
            result = @"MPTZPosition";
            break;

        case MTRAttributeIDTypeClusterCameraAVSettingsUserLevelManagementAttributeMaxPresetsID:
            result = @"MaxPresets";
            break;

        case MTRAttributeIDTypeClusterCameraAVSettingsUserLevelManagementAttributeMPTZPresetsID:
            result = @"MPTZPresets";
            break;

        case MTRAttributeIDTypeClusterCameraAVSettingsUserLevelManagementAttributeDPTZStreamsID:
            result = @"DPTZStreams";
            break;

        case MTRAttributeIDTypeClusterCameraAVSettingsUserLevelManagementAttributeZoomMaxID:
            result = @"ZoomMax";
            break;

        case MTRAttributeIDTypeClusterCameraAVSettingsUserLevelManagementAttributeTiltMinID:
            result = @"TiltMin";
            break;

        case MTRAttributeIDTypeClusterCameraAVSettingsUserLevelManagementAttributeTiltMaxID:
            result = @"TiltMax";
            break;

        case MTRAttributeIDTypeClusterCameraAVSettingsUserLevelManagementAttributePanMinID:
            result = @"PanMin";
            break;

        case MTRAttributeIDTypeClusterCameraAVSettingsUserLevelManagementAttributePanMaxID:
            result = @"PanMax";
            break;

        case MTRAttributeIDTypeClusterCameraAVSettingsUserLevelManagementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterCameraAVSettingsUserLevelManagementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterCameraAVSettingsUserLevelManagementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterCameraAVSettingsUserLevelManagementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterCameraAVSettingsUserLevelManagementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known CameraAVSettingsUserLevelManagement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeWebRTCTransportProviderID:

        switch (attributeID) {

            // Cluster WebRTCTransportProvider attributes
        case MTRAttributeIDTypeClusterWebRTCTransportProviderAttributeCurrentSessionsID:
            result = @"CurrentSessions";
            break;

        case MTRAttributeIDTypeClusterWebRTCTransportProviderAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterWebRTCTransportProviderAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterWebRTCTransportProviderAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterWebRTCTransportProviderAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterWebRTCTransportProviderAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known WebRTCTransportProvider attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeWebRTCTransportRequestorID:

        switch (attributeID) {

            // Cluster WebRTCTransportRequestor attributes
        case MTRAttributeIDTypeClusterWebRTCTransportRequestorAttributeCurrentSessionsID:
            result = @"CurrentSessions";
            break;

        case MTRAttributeIDTypeClusterWebRTCTransportRequestorAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterWebRTCTransportRequestorAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterWebRTCTransportRequestorAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterWebRTCTransportRequestorAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterWebRTCTransportRequestorAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known WebRTCTransportRequestor attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypePushAVStreamTransportID:

        switch (attributeID) {

            // Cluster PushAVStreamTransport attributes
        case MTRAttributeIDTypeClusterPushAVStreamTransportAttributeSupportedFormatsID:
            result = @"SupportedFormats";
            break;

        case MTRAttributeIDTypeClusterPushAVStreamTransportAttributeCurrentConnectionsID:
            result = @"CurrentConnections";
            break;

        case MTRAttributeIDTypeClusterPushAVStreamTransportAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterPushAVStreamTransportAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterPushAVStreamTransportAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterPushAVStreamTransportAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterPushAVStreamTransportAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known PushAVStreamTransport attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeChimeID:

        switch (attributeID) {

            // Cluster Chime attributes
        case MTRAttributeIDTypeClusterChimeAttributeInstalledChimeSoundsID:
            result = @"InstalledChimeSounds";
            break;

        case MTRAttributeIDTypeClusterChimeAttributeSelectedChimeID:
            result = @"SelectedChime";
            break;

        case MTRAttributeIDTypeClusterChimeAttributeEnabledID:
            result = @"Enabled";
            break;

        case MTRAttributeIDTypeClusterChimeAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterChimeAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterChimeAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterChimeAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterChimeAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known Chime attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeCommodityTariffID:

        switch (attributeID) {

            // Cluster CommodityTariff attributes
        case MTRAttributeIDTypeClusterCommodityTariffAttributeTariffInfoID:
            result = @"TariffInfo";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeTariffUnitID:
            result = @"TariffUnit";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeStartDateID:
            result = @"StartDate";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeDayEntriesID:
            result = @"DayEntries";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeDayPatternsID:
            result = @"DayPatterns";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeCalendarPeriodsID:
            result = @"CalendarPeriods";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeIndividualDaysID:
            result = @"IndividualDays";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeCurrentDayID:
            result = @"CurrentDay";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeNextDayID:
            result = @"NextDay";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeCurrentDayEntryID:
            result = @"CurrentDayEntry";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeCurrentDayEntryDateID:
            result = @"CurrentDayEntryDate";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeNextDayEntryID:
            result = @"NextDayEntry";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeNextDayEntryDateID:
            result = @"NextDayEntryDate";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeTariffComponentsID:
            result = @"TariffComponents";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeTariffPeriodsID:
            result = @"TariffPeriods";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeCurrentTariffComponentsID:
            result = @"CurrentTariffComponents";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeNextTariffComponentsID:
            result = @"NextTariffComponents";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeDefaultRandomizationOffsetID:
            result = @"DefaultRandomizationOffset";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeDefaultRandomizationTypeID:
            result = @"DefaultRandomizationType";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterCommodityTariffAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known CommodityTariff attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeEcosystemInformationID:

        switch (attributeID) {

            // Cluster EcosystemInformation attributes
        case MTRAttributeIDTypeClusterEcosystemInformationAttributeDeviceDirectoryID:
            result = @"DeviceDirectory";
            break;

        case MTRAttributeIDTypeClusterEcosystemInformationAttributeLocationDirectoryID:
            result = @"LocationDirectory";
            break;

        case MTRAttributeIDTypeClusterEcosystemInformationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterEcosystemInformationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterEcosystemInformationAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterEcosystemInformationAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterEcosystemInformationAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known EcosystemInformation attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeCommissionerControlID:

        switch (attributeID) {

            // Cluster CommissionerControl attributes
        case MTRAttributeIDTypeClusterCommissionerControlAttributeSupportedDeviceCategoriesID:
            result = @"SupportedDeviceCategories";
            break;

        case MTRAttributeIDTypeClusterCommissionerControlAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterCommissionerControlAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterCommissionerControlAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterCommissionerControlAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterCommissionerControlAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known CommissionerControl attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeJointFabricDatastoreID:

        switch (attributeID) {

            // Cluster JointFabricDatastore attributes
        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeAnchorRootCAID:
            result = @"AnchorRootCA";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeAnchorNodeIDID:
            result = @"AnchorNodeID";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeAnchorVendorIDID:
            result = @"AnchorVendorID";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeFriendlyNameID:
            result = @"FriendlyName";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeGroupKeySetListID:
            result = @"GroupKeySetList";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeGroupListID:
            result = @"GroupList";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeNodeListID:
            result = @"NodeList";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeAdminListID:
            result = @"AdminList";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeStatusID:
            result = @"Status";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeEndpointGroupIDListID:
            result = @"EndpointGroupIDList";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeEndpointBindingListID:
            result = @"EndpointBindingList";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeNodeKeySetListID:
            result = @"NodeKeySetList";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeNodeACLListID:
            result = @"NodeACLList";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeNodeEndpointListID:
            result = @"NodeEndpointList";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterJointFabricDatastoreAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known JointFabricDatastore attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeJointFabricAdministratorID:

        switch (attributeID) {

            // Cluster JointFabricAdministrator attributes
        case MTRAttributeIDTypeClusterJointFabricAdministratorAttributeAdministratorFabricIndexID:
            result = @"AdministratorFabricIndex";
            break;

        case MTRAttributeIDTypeClusterJointFabricAdministratorAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterJointFabricAdministratorAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterJointFabricAdministratorAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterJointFabricAdministratorAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterJointFabricAdministratorAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known JointFabricAdministrator attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeTLSCertificateManagementID:

        switch (attributeID) {

            // Cluster TLSCertificateManagement attributes
        case MTRAttributeIDTypeClusterTLSCertificateManagementAttributeMaxRootCertificatesID:
            result = @"MaxRootCertificates";
            break;

        case MTRAttributeIDTypeClusterTLSCertificateManagementAttributeProvisionedRootCertificatesID:
            result = @"ProvisionedRootCertificates";
            break;

        case MTRAttributeIDTypeClusterTLSCertificateManagementAttributeMaxClientCertificatesID:
            result = @"MaxClientCertificates";
            break;

        case MTRAttributeIDTypeClusterTLSCertificateManagementAttributeProvisionedClientCertificatesID:
            result = @"ProvisionedClientCertificates";
            break;

        case MTRAttributeIDTypeClusterTLSCertificateManagementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterTLSCertificateManagementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterTLSCertificateManagementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterTLSCertificateManagementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterTLSCertificateManagementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known TLSCertificateManagement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeTLSClientManagementID:

        switch (attributeID) {

            // Cluster TLSClientManagement attributes
        case MTRAttributeIDTypeClusterTLSClientManagementAttributeMaxProvisionedID:
            result = @"MaxProvisioned";
            break;

        case MTRAttributeIDTypeClusterTLSClientManagementAttributeProvisionedEndpointsID:
            result = @"ProvisionedEndpoints";
            break;

        case MTRAttributeIDTypeClusterTLSClientManagementAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterTLSClientManagementAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterTLSClientManagementAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterTLSClientManagementAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterTLSClientManagementAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known TLSClientManagement attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeMeterIdentificationID:

        switch (attributeID) {

            // Cluster MeterIdentification attributes
        case MTRAttributeIDTypeClusterMeterIdentificationAttributeMeterTypeID:
            result = @"MeterType";
            break;

        case MTRAttributeIDTypeClusterMeterIdentificationAttributePointOfDeliveryID:
            result = @"PointOfDelivery";
            break;

        case MTRAttributeIDTypeClusterMeterIdentificationAttributeMeterSerialNumberID:
            result = @"MeterSerialNumber";
            break;

        case MTRAttributeIDTypeClusterMeterIdentificationAttributeProtocolVersionID:
            result = @"ProtocolVersion";
            break;

        case MTRAttributeIDTypeClusterMeterIdentificationAttributePowerThresholdID:
            result = @"PowerThreshold";
            break;

        case MTRAttributeIDTypeClusterMeterIdentificationAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterMeterIdentificationAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterMeterIdentificationAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterMeterIdentificationAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterMeterIdentificationAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known MeterIdentification attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    case MTRClusterIDTypeCommodityMeteringID:

        switch (attributeID) {

            // Cluster CommodityMetering attributes
        case MTRAttributeIDTypeClusterCommodityMeteringAttributeMeteredQuantityID:
            result = @"MeteredQuantity";
            break;

        case MTRAttributeIDTypeClusterCommodityMeteringAttributeMeteredQuantityTimestampID:
            result = @"MeteredQuantityTimestamp";
            break;

        case MTRAttributeIDTypeClusterCommodityMeteringAttributeTariffUnitID:
            result = @"TariffUnit";
            break;

        case MTRAttributeIDTypeClusterCommodityMeteringAttributeMaximumMeteredQuantitiesID:
            result = @"MaximumMeteredQuantities";
            break;

        case MTRAttributeIDTypeClusterCommodityMeteringAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterCommodityMeteringAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
            break;

        case MTRAttributeIDTypeClusterCommodityMeteringAttributeAttributeListID:
            result = @"AttributeList";
            break;

        case MTRAttributeIDTypeClusterCommodityMeteringAttributeFeatureMapID:
            result = @"FeatureMap";
            break;

        case MTRAttributeIDTypeClusterCommodityMeteringAttributeClusterRevisionID:
            result = @"ClusterRevision";
            break;

        default:
            // Not a known CommodityMetering attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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

        case MTRAttributeIDTypeClusterUnitTestingAttributeGlobalEnumID:
            result = @"GlobalEnum";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeGlobalStructID:
            result = @"GlobalStruct";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeUnsupportedAttributeRequiringAdminPrivilegeID:
            result = @"UnsupportedAttributeRequiringAdminPrivilege";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeUnsupportedID:
            result = @"Unsupported";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeReadFailureCodeID:
            result = @"ReadFailureCode";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeFailureInt32UID:
            result = @"FailureInt32U";
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

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableGlobalEnumID:
            result = @"NullableGlobalEnum";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeNullableGlobalStructID:
            result = @"NullableGlobalStruct";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeGeneratedCommandListID:
            result = @"GeneratedCommandList";
            break;

        case MTRAttributeIDTypeClusterUnitTestingAttributeAcceptedCommandListID:
            result = @"AcceptedCommandList";
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
            // Not a known UnitTesting attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

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
            // Not a known SampleMEI attribute.
            result = [NSString stringWithFormat:@"<Unknown attributeID %u>", attributeID];
            break;
        }
        break;

    default:
        result = [NSString stringWithFormat:@"<Unknown clusterID %u>", clusterID];
        break;
    }

    return result;
}

#pragma mark - Command IDs

NSString * MTRRequestCommandNameForID(MTRClusterIDType clusterID, MTRCommandIDType commandID)
{
    NSString * result = nil;

    switch (clusterID) {

    case MTRClusterIDTypeIdentifyID:

        switch (commandID) {

        case MTRCommandIDTypeClusterIdentifyCommandIdentifyID:
            result = @"Identify";
            break;

        case MTRCommandIDTypeClusterIdentifyCommandTriggerEffectID:
            result = @"TriggerEffect";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeGroupsID:

        switch (commandID) {

        case MTRCommandIDTypeClusterGroupsCommandAddGroupID:
            result = @"AddGroup";
            break;

        case MTRCommandIDTypeClusterGroupsCommandViewGroupID:
            result = @"ViewGroup";
            break;

        case MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipID:
            result = @"GetGroupMembership";
            break;

        case MTRCommandIDTypeClusterGroupsCommandRemoveGroupID:
            result = @"RemoveGroup";
            break;

        case MTRCommandIDTypeClusterGroupsCommandRemoveAllGroupsID:
            result = @"RemoveAllGroups";
            break;

        case MTRCommandIDTypeClusterGroupsCommandAddGroupIfIdentifyingID:
            result = @"AddGroupIfIdentifying";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOnOffID:

        switch (commandID) {

        case MTRCommandIDTypeClusterOnOffCommandOffID:
            result = @"Off";
            break;

        case MTRCommandIDTypeClusterOnOffCommandOnID:
            result = @"On";
            break;

        case MTRCommandIDTypeClusterOnOffCommandToggleID:
            result = @"Toggle";
            break;

        case MTRCommandIDTypeClusterOnOffCommandOffWithEffectID:
            result = @"OffWithEffect";
            break;

        case MTRCommandIDTypeClusterOnOffCommandOnWithRecallGlobalSceneID:
            result = @"OnWithRecallGlobalScene";
            break;

        case MTRCommandIDTypeClusterOnOffCommandOnWithTimedOffID:
            result = @"OnWithTimedOff";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeLevelControlID:

        switch (commandID) {

        case MTRCommandIDTypeClusterLevelControlCommandMoveToLevelID:
            result = @"MoveToLevel";
            break;

        case MTRCommandIDTypeClusterLevelControlCommandMoveID:
            result = @"Move";
            break;

        case MTRCommandIDTypeClusterLevelControlCommandStepID:
            result = @"Step";
            break;

        case MTRCommandIDTypeClusterLevelControlCommandStopID:
            result = @"Stop";
            break;

        case MTRCommandIDTypeClusterLevelControlCommandMoveToLevelWithOnOffID:
            result = @"MoveToLevelWithOnOff";
            break;

        case MTRCommandIDTypeClusterLevelControlCommandMoveWithOnOffID:
            result = @"MoveWithOnOff";
            break;

        case MTRCommandIDTypeClusterLevelControlCommandStepWithOnOffID:
            result = @"StepWithOnOff";
            break;

        case MTRCommandIDTypeClusterLevelControlCommandStopWithOnOffID:
            result = @"StopWithOnOff";
            break;

        case MTRCommandIDTypeClusterLevelControlCommandMoveToClosestFrequencyID:
            result = @"MoveToClosestFrequency";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePulseWidthModulationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeDescriptorID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeBindingID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeAccessControlID:

        switch (commandID) {

        case MTRCommandIDTypeClusterAccessControlCommandReviewFabricRestrictionsID:
            result = @"ReviewFabricRestrictions";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeActionsID:

        switch (commandID) {

        case MTRCommandIDTypeClusterActionsCommandInstantActionID:
            result = @"InstantAction";
            break;

        case MTRCommandIDTypeClusterActionsCommandInstantActionWithTransitionID:
            result = @"InstantActionWithTransition";
            break;

        case MTRCommandIDTypeClusterActionsCommandStartActionID:
            result = @"StartAction";
            break;

        case MTRCommandIDTypeClusterActionsCommandStartActionWithDurationID:
            result = @"StartActionWithDuration";
            break;

        case MTRCommandIDTypeClusterActionsCommandStopActionID:
            result = @"StopAction";
            break;

        case MTRCommandIDTypeClusterActionsCommandPauseActionID:
            result = @"PauseAction";
            break;

        case MTRCommandIDTypeClusterActionsCommandPauseActionWithDurationID:
            result = @"PauseActionWithDuration";
            break;

        case MTRCommandIDTypeClusterActionsCommandResumeActionID:
            result = @"ResumeAction";
            break;

        case MTRCommandIDTypeClusterActionsCommandEnableActionID:
            result = @"EnableAction";
            break;

        case MTRCommandIDTypeClusterActionsCommandEnableActionWithDurationID:
            result = @"EnableActionWithDuration";
            break;

        case MTRCommandIDTypeClusterActionsCommandDisableActionID:
            result = @"DisableAction";
            break;

        case MTRCommandIDTypeClusterActionsCommandDisableActionWithDurationID:
            result = @"DisableActionWithDuration";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeBasicInformationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOTASoftwareUpdateProviderID:

        switch (commandID) {

        case MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageID:
            result = @"QueryImage";
            break;

        case MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateRequestID:
            result = @"ApplyUpdateRequest";
            break;

        case MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandNotifyUpdateAppliedID:
            result = @"NotifyUpdateApplied";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOTASoftwareUpdateRequestorID:

        switch (commandID) {

        case MTRCommandIDTypeClusterOTASoftwareUpdateRequestorCommandAnnounceOTAProviderID:
            result = @"AnnounceOTAProvider";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeLocalizationConfigurationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTimeFormatLocalizationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeUnitLocalizationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePowerSourceConfigurationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePowerSourceID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeGeneralCommissioningID:

        switch (commandID) {

        case MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeID:
            result = @"ArmFailSafe";
            break;

        case MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigID:
            result = @"SetRegulatoryConfig";
            break;

        case MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteID:
            result = @"CommissioningComplete";
            break;

        case MTRCommandIDTypeClusterGeneralCommissioningCommandSetTCAcknowledgementsID:
            result = @"SetTCAcknowledgements";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeNetworkCommissioningID:

        switch (commandID) {

        case MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksID:
            result = @"ScanNetworks";
            break;

        case MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateWiFiNetworkID:
            result = @"AddOrUpdateWiFiNetwork";
            break;

        case MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateThreadNetworkID:
            result = @"AddOrUpdateThreadNetwork";
            break;

        case MTRCommandIDTypeClusterNetworkCommissioningCommandRemoveNetworkID:
            result = @"RemoveNetwork";
            break;

        case MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkID:
            result = @"ConnectNetwork";
            break;

        case MTRCommandIDTypeClusterNetworkCommissioningCommandReorderNetworkID:
            result = @"ReorderNetwork";
            break;

        case MTRCommandIDTypeClusterNetworkCommissioningCommandQueryIdentityID:
            result = @"QueryIdentity";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeDiagnosticLogsID:

        switch (commandID) {

        case MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsRequestID:
            result = @"RetrieveLogsRequest";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeGeneralDiagnosticsID:

        switch (commandID) {

        case MTRCommandIDTypeClusterGeneralDiagnosticsCommandTestEventTriggerID:
            result = @"TestEventTrigger";
            break;

        case MTRCommandIDTypeClusterGeneralDiagnosticsCommandTimeSnapshotID:
            result = @"TimeSnapshot";
            break;

        case MTRCommandIDTypeClusterGeneralDiagnosticsCommandPayloadTestRequestID:
            result = @"PayloadTestRequest";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeSoftwareDiagnosticsID:

        switch (commandID) {

        case MTRCommandIDTypeClusterSoftwareDiagnosticsCommandResetWatermarksID:
            result = @"ResetWatermarks";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeThreadNetworkDiagnosticsID:

        switch (commandID) {

        case MTRCommandIDTypeClusterThreadNetworkDiagnosticsCommandResetCountsID:
            result = @"ResetCounts";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWiFiNetworkDiagnosticsID:

        switch (commandID) {

        case MTRCommandIDTypeClusterWiFiNetworkDiagnosticsCommandResetCountsID:
            result = @"ResetCounts";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeEthernetNetworkDiagnosticsID:

        switch (commandID) {

        case MTRCommandIDTypeClusterEthernetNetworkDiagnosticsCommandResetCountsID:
            result = @"ResetCounts";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTimeSynchronizationID:

        switch (commandID) {

        case MTRCommandIDTypeClusterTimeSynchronizationCommandSetUTCTimeID:
            result = @"SetUTCTime";
            break;

        case MTRCommandIDTypeClusterTimeSynchronizationCommandSetTrustedTimeSourceID:
            result = @"SetTrustedTimeSource";
            break;

        case MTRCommandIDTypeClusterTimeSynchronizationCommandSetTimeZoneID:
            result = @"SetTimeZone";
            break;

        case MTRCommandIDTypeClusterTimeSynchronizationCommandSetDSTOffsetID:
            result = @"SetDSTOffset";
            break;

        case MTRCommandIDTypeClusterTimeSynchronizationCommandSetDefaultNTPID:
            result = @"SetDefaultNTP";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeBridgedDeviceBasicInformationID:

        switch (commandID) {

        case MTRCommandIDTypeClusterBridgedDeviceBasicInformationCommandKeepActiveID:
            result = @"KeepActive";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeSwitchID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeAdministratorCommissioningID:

        switch (commandID) {

        case MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenCommissioningWindowID:
            result = @"OpenCommissioningWindow";
            break;

        case MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenBasicCommissioningWindowID:
            result = @"OpenBasicCommissioningWindow";
            break;

        case MTRCommandIDTypeClusterAdministratorCommissioningCommandRevokeCommissioningID:
            result = @"RevokeCommissioning";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOperationalCredentialsID:

        switch (commandID) {

        case MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationRequestID:
            result = @"AttestationRequest";
            break;

        case MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainRequestID:
            result = @"CertificateChainRequest";
            break;

        case MTRCommandIDTypeClusterOperationalCredentialsCommandCSRRequestID:
            result = @"CSRRequest";
            break;

        case MTRCommandIDTypeClusterOperationalCredentialsCommandAddNOCID:
            result = @"AddNOC";
            break;

        case MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateNOCID:
            result = @"UpdateNOC";
            break;

        case MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateFabricLabelID:
            result = @"UpdateFabricLabel";
            break;

        case MTRCommandIDTypeClusterOperationalCredentialsCommandRemoveFabricID:
            result = @"RemoveFabric";
            break;

        case MTRCommandIDTypeClusterOperationalCredentialsCommandAddTrustedRootCertificateID:
            result = @"AddTrustedRootCertificate";
            break;

        case MTRCommandIDTypeClusterOperationalCredentialsCommandSetVIDVerificationStatementID:
            result = @"SetVIDVerificationStatement";
            break;

        case MTRCommandIDTypeClusterOperationalCredentialsCommandSignVIDVerificationRequestID:
            result = @"SignVIDVerificationRequest";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeGroupKeyManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetWriteID:
            result = @"KeySetWrite";
            break;

        case MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadID:
            result = @"KeySetRead";
            break;

        case MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetRemoveID:
            result = @"KeySetRemove";
            break;

        case MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesID:
            result = @"KeySetReadAllIndices";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeFixedLabelID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeUserLabelID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeBooleanStateID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeICDManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterICDManagementCommandRegisterClientID:
            result = @"RegisterClient";
            break;

        case MTRCommandIDTypeClusterICDManagementCommandUnregisterClientID:
            result = @"UnregisterClient";
            break;

        case MTRCommandIDTypeClusterICDManagementCommandStayActiveRequestID:
            result = @"StayActiveRequest";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTimerID:

        switch (commandID) {

        case MTRCommandIDTypeClusterTimerCommandSetTimerID:
            result = @"SetTimer";
            break;

        case MTRCommandIDTypeClusterTimerCommandResetTimerID:
            result = @"ResetTimer";
            break;

        case MTRCommandIDTypeClusterTimerCommandAddTimeID:
            result = @"AddTime";
            break;

        case MTRCommandIDTypeClusterTimerCommandReduceTimeID:
            result = @"ReduceTime";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOvenCavityOperationalStateID:

        switch (commandID) {

        case MTRCommandIDTypeClusterOvenCavityOperationalStateCommandStopID:
            result = @"Stop";
            break;

        case MTRCommandIDTypeClusterOvenCavityOperationalStateCommandStartID:
            result = @"Start";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOvenModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterOvenModeCommandChangeToModeID:
            result = @"ChangeToMode";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeLaundryDryerControlsID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeModeSelectID:

        switch (commandID) {

        case MTRCommandIDTypeClusterModeSelectCommandChangeToModeID:
            result = @"ChangeToMode";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeLaundryWasherModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterLaundryWasherModeCommandChangeToModeID:
            result = @"ChangeToMode";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeRefrigeratorAndTemperatureControlledCabinetModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeCommandChangeToModeID:
            result = @"ChangeToMode";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeLaundryWasherControlsID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeRVCRunModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterRVCRunModeCommandChangeToModeID:
            result = @"ChangeToMode";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeRVCCleanModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterRVCCleanModeCommandChangeToModeID:
            result = @"ChangeToMode";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTemperatureControlID:

        switch (commandID) {

        case MTRCommandIDTypeClusterTemperatureControlCommandSetTemperatureID:
            result = @"SetTemperature";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeRefrigeratorAlarmID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeDishwasherModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterDishwasherModeCommandChangeToModeID:
            result = @"ChangeToMode";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeAirQualityID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeSmokeCOAlarmID:

        switch (commandID) {

        case MTRCommandIDTypeClusterSmokeCOAlarmCommandSelfTestRequestID:
            result = @"SelfTestRequest";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeDishwasherAlarmID:

        switch (commandID) {

        case MTRCommandIDTypeClusterDishwasherAlarmCommandResetID:
            result = @"Reset";
            break;

        case MTRCommandIDTypeClusterDishwasherAlarmCommandModifyEnabledAlarmsID:
            result = @"ModifyEnabledAlarms";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeMicrowaveOvenModeID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeMicrowaveOvenControlID:

        switch (commandID) {

        case MTRCommandIDTypeClusterMicrowaveOvenControlCommandSetCookingParametersID:
            result = @"SetCookingParameters";
            break;

        case MTRCommandIDTypeClusterMicrowaveOvenControlCommandAddMoreTimeID:
            result = @"AddMoreTime";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOperationalStateID:

        switch (commandID) {

        case MTRCommandIDTypeClusterOperationalStateCommandPauseID:
            result = @"Pause";
            break;

        case MTRCommandIDTypeClusterOperationalStateCommandStopID:
            result = @"Stop";
            break;

        case MTRCommandIDTypeClusterOperationalStateCommandStartID:
            result = @"Start";
            break;

        case MTRCommandIDTypeClusterOperationalStateCommandResumeID:
            result = @"Resume";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeRVCOperationalStateID:

        switch (commandID) {

        case MTRCommandIDTypeClusterRVCOperationalStateCommandPauseID:
            result = @"Pause";
            break;

        case MTRCommandIDTypeClusterRVCOperationalStateCommandResumeID:
            result = @"Resume";
            break;

        case MTRCommandIDTypeClusterRVCOperationalStateCommandGoHomeID:
            result = @"GoHome";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeScenesManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterScenesManagementCommandAddSceneID:
            result = @"AddScene";
            break;

        case MTRCommandIDTypeClusterScenesManagementCommandViewSceneID:
            result = @"ViewScene";
            break;

        case MTRCommandIDTypeClusterScenesManagementCommandRemoveSceneID:
            result = @"RemoveScene";
            break;

        case MTRCommandIDTypeClusterScenesManagementCommandRemoveAllScenesID:
            result = @"RemoveAllScenes";
            break;

        case MTRCommandIDTypeClusterScenesManagementCommandStoreSceneID:
            result = @"StoreScene";
            break;

        case MTRCommandIDTypeClusterScenesManagementCommandRecallSceneID:
            result = @"RecallScene";
            break;

        case MTRCommandIDTypeClusterScenesManagementCommandGetSceneMembershipID:
            result = @"GetSceneMembership";
            break;

        case MTRCommandIDTypeClusterScenesManagementCommandCopySceneID:
            result = @"CopyScene";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeHEPAFilterMonitoringID:

        switch (commandID) {

        case MTRCommandIDTypeClusterHEPAFilterMonitoringCommandResetConditionID:
            result = @"ResetCondition";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeActivatedCarbonFilterMonitoringID:

        switch (commandID) {

        case MTRCommandIDTypeClusterActivatedCarbonFilterMonitoringCommandResetConditionID:
            result = @"ResetCondition";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeBooleanStateConfigurationID:

        switch (commandID) {

        case MTRCommandIDTypeClusterBooleanStateConfigurationCommandSuppressAlarmID:
            result = @"SuppressAlarm";
            break;

        case MTRCommandIDTypeClusterBooleanStateConfigurationCommandEnableDisableAlarmID:
            result = @"EnableDisableAlarm";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeValveConfigurationAndControlID:

        switch (commandID) {

        case MTRCommandIDTypeClusterValveConfigurationAndControlCommandOpenID:
            result = @"Open";
            break;

        case MTRCommandIDTypeClusterValveConfigurationAndControlCommandCloseID:
            result = @"Close";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeElectricalPowerMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeElectricalEnergyMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWaterHeaterManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterWaterHeaterManagementCommandBoostID:
            result = @"Boost";
            break;

        case MTRCommandIDTypeClusterWaterHeaterManagementCommandCancelBoostID:
            result = @"CancelBoost";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCommodityPriceID:

        switch (commandID) {

        case MTRCommandIDTypeClusterCommodityPriceCommandGetDetailedPriceRequestID:
            result = @"GetDetailedPriceRequest";
            break;

        case MTRCommandIDTypeClusterCommodityPriceCommandGetDetailedForecastRequestID:
            result = @"GetDetailedForecastRequest";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeMessagesID:

        switch (commandID) {

        case MTRCommandIDTypeClusterMessagesCommandPresentMessagesRequestID:
            result = @"PresentMessagesRequest";
            break;

        case MTRCommandIDTypeClusterMessagesCommandCancelMessagesRequestID:
            result = @"CancelMessagesRequest";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeDeviceEnergyManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterDeviceEnergyManagementCommandPowerAdjustRequestID:
            result = @"PowerAdjustRequest";
            break;

        case MTRCommandIDTypeClusterDeviceEnergyManagementCommandCancelPowerAdjustRequestID:
            result = @"CancelPowerAdjustRequest";
            break;

        case MTRCommandIDTypeClusterDeviceEnergyManagementCommandStartTimeAdjustRequestID:
            result = @"StartTimeAdjustRequest";
            break;

        case MTRCommandIDTypeClusterDeviceEnergyManagementCommandPauseRequestID:
            result = @"PauseRequest";
            break;

        case MTRCommandIDTypeClusterDeviceEnergyManagementCommandResumeRequestID:
            result = @"ResumeRequest";
            break;

        case MTRCommandIDTypeClusterDeviceEnergyManagementCommandModifyForecastRequestID:
            result = @"ModifyForecastRequest";
            break;

        case MTRCommandIDTypeClusterDeviceEnergyManagementCommandRequestConstraintBasedForecastID:
            result = @"RequestConstraintBasedForecast";
            break;

        case MTRCommandIDTypeClusterDeviceEnergyManagementCommandCancelRequestID:
            result = @"CancelRequest";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeEnergyEVSEID:

        switch (commandID) {

        case MTRCommandIDTypeClusterEnergyEVSECommandDisableID:
            result = @"Disable";
            break;

        case MTRCommandIDTypeClusterEnergyEVSECommandEnableChargingID:
            result = @"EnableCharging";
            break;

        case MTRCommandIDTypeClusterEnergyEVSECommandEnableDischargingID:
            result = @"EnableDischarging";
            break;

        case MTRCommandIDTypeClusterEnergyEVSECommandStartDiagnosticsID:
            result = @"StartDiagnostics";
            break;

        case MTRCommandIDTypeClusterEnergyEVSECommandSetTargetsID:
            result = @"SetTargets";
            break;

        case MTRCommandIDTypeClusterEnergyEVSECommandGetTargetsID:
            result = @"GetTargets";
            break;

        case MTRCommandIDTypeClusterEnergyEVSECommandClearTargetsID:
            result = @"ClearTargets";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeEnergyPreferenceID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePowerTopologyID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeEnergyEVSEModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterEnergyEVSEModeCommandChangeToModeID:
            result = @"ChangeToMode";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWaterHeaterModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterWaterHeaterModeCommandChangeToModeID:
            result = @"ChangeToMode";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeDeviceEnergyManagementModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterDeviceEnergyManagementModeCommandChangeToModeID:
            result = @"ChangeToMode";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeElectricalGridConditionsID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeDoorLockID:

        switch (commandID) {

        case MTRCommandIDTypeClusterDoorLockCommandLockDoorID:
            result = @"LockDoor";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandUnlockDoorID:
            result = @"UnlockDoor";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandUnlockWithTimeoutID:
            result = @"UnlockWithTimeout";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandSetWeekDayScheduleID:
            result = @"SetWeekDaySchedule";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleID:
            result = @"GetWeekDaySchedule";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandClearWeekDayScheduleID:
            result = @"ClearWeekDaySchedule";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandSetYearDayScheduleID:
            result = @"SetYearDaySchedule";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleID:
            result = @"GetYearDaySchedule";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandClearYearDayScheduleID:
            result = @"ClearYearDaySchedule";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandSetHolidayScheduleID:
            result = @"SetHolidaySchedule";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleID:
            result = @"GetHolidaySchedule";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandClearHolidayScheduleID:
            result = @"ClearHolidaySchedule";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandSetUserID:
            result = @"SetUser";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandGetUserID:
            result = @"GetUser";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandClearUserID:
            result = @"ClearUser";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandSetCredentialID:
            result = @"SetCredential";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusID:
            result = @"GetCredentialStatus";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandClearCredentialID:
            result = @"ClearCredential";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandUnboltDoorID:
            result = @"UnboltDoor";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandSetAliroReaderConfigID:
            result = @"SetAliroReaderConfig";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandClearAliroReaderConfigID:
            result = @"ClearAliroReaderConfig";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWindowCoveringID:

        switch (commandID) {

        case MTRCommandIDTypeClusterWindowCoveringCommandUpOrOpenID:
            result = @"UpOrOpen";
            break;

        case MTRCommandIDTypeClusterWindowCoveringCommandDownOrCloseID:
            result = @"DownOrClose";
            break;

        case MTRCommandIDTypeClusterWindowCoveringCommandStopMotionID:
            result = @"StopMotion";
            break;

        case MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftValueID:
            result = @"GoToLiftValue";
            break;

        case MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftPercentageID:
            result = @"GoToLiftPercentage";
            break;

        case MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltValueID:
            result = @"GoToTiltValue";
            break;

        case MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltPercentageID:
            result = @"GoToTiltPercentage";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeClosureControlID:

        switch (commandID) {

        case MTRCommandIDTypeClusterClosureControlCommandStopID:
            result = @"Stop";
            break;

        case MTRCommandIDTypeClusterClosureControlCommandMoveToID:
            result = @"MoveTo";
            break;

        case MTRCommandIDTypeClusterClosureControlCommandCalibrateID:
            result = @"Calibrate";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeClosureDimensionID:

        switch (commandID) {

        case MTRCommandIDTypeClusterClosureDimensionCommandSetTargetID:
            result = @"SetTarget";
            break;

        case MTRCommandIDTypeClusterClosureDimensionCommandStepID:
            result = @"Step";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeServiceAreaID:

        switch (commandID) {

        case MTRCommandIDTypeClusterServiceAreaCommandSelectAreasID:
            result = @"SelectAreas";
            break;

        case MTRCommandIDTypeClusterServiceAreaCommandSkipAreaID:
            result = @"SkipArea";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePumpConfigurationAndControlID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeThermostatID:

        switch (commandID) {

        case MTRCommandIDTypeClusterThermostatCommandSetpointRaiseLowerID:
            result = @"SetpointRaiseLower";
            break;

        case MTRCommandIDTypeClusterThermostatCommandSetWeeklyScheduleID:
            result = @"SetWeeklySchedule";
            break;

        case MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleID:
            result = @"GetWeeklySchedule";
            break;

        case MTRCommandIDTypeClusterThermostatCommandClearWeeklyScheduleID:
            result = @"ClearWeeklySchedule";
            break;

        case MTRCommandIDTypeClusterThermostatCommandSetActiveScheduleRequestID:
            result = @"SetActiveScheduleRequest";
            break;

        case MTRCommandIDTypeClusterThermostatCommandSetActivePresetRequestID:
            result = @"SetActivePresetRequest";
            break;

        case MTRCommandIDTypeClusterThermostatCommandAddThermostatSuggestionID:
            result = @"AddThermostatSuggestion";
            break;

        case MTRCommandIDTypeClusterThermostatCommandRemoveThermostatSuggestionID:
            result = @"RemoveThermostatSuggestion";
            break;

        case MTRCommandIDTypeClusterThermostatCommandAtomicRequestID:
            result = @"AtomicRequest";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeFanControlID:

        switch (commandID) {

        case MTRCommandIDTypeClusterFanControlCommandStepID:
            result = @"Step";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeThermostatUserInterfaceConfigurationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeColorControlID:

        switch (commandID) {

        case MTRCommandIDTypeClusterColorControlCommandMoveToHueID:
            result = @"MoveToHue";
            break;

        case MTRCommandIDTypeClusterColorControlCommandMoveHueID:
            result = @"MoveHue";
            break;

        case MTRCommandIDTypeClusterColorControlCommandStepHueID:
            result = @"StepHue";
            break;

        case MTRCommandIDTypeClusterColorControlCommandMoveToSaturationID:
            result = @"MoveToSaturation";
            break;

        case MTRCommandIDTypeClusterColorControlCommandMoveSaturationID:
            result = @"MoveSaturation";
            break;

        case MTRCommandIDTypeClusterColorControlCommandStepSaturationID:
            result = @"StepSaturation";
            break;

        case MTRCommandIDTypeClusterColorControlCommandMoveToHueAndSaturationID:
            result = @"MoveToHueAndSaturation";
            break;

        case MTRCommandIDTypeClusterColorControlCommandMoveToColorID:
            result = @"MoveToColor";
            break;

        case MTRCommandIDTypeClusterColorControlCommandMoveColorID:
            result = @"MoveColor";
            break;

        case MTRCommandIDTypeClusterColorControlCommandStepColorID:
            result = @"StepColor";
            break;

        case MTRCommandIDTypeClusterColorControlCommandMoveToColorTemperatureID:
            result = @"MoveToColorTemperature";
            break;

        case MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueID:
            result = @"EnhancedMoveToHue";
            break;

        case MTRCommandIDTypeClusterColorControlCommandEnhancedMoveHueID:
            result = @"EnhancedMoveHue";
            break;

        case MTRCommandIDTypeClusterColorControlCommandEnhancedStepHueID:
            result = @"EnhancedStepHue";
            break;

        case MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueAndSaturationID:
            result = @"EnhancedMoveToHueAndSaturation";
            break;

        case MTRCommandIDTypeClusterColorControlCommandColorLoopSetID:
            result = @"ColorLoopSet";
            break;

        case MTRCommandIDTypeClusterColorControlCommandStopMoveStepID:
            result = @"StopMoveStep";
            break;

        case MTRCommandIDTypeClusterColorControlCommandMoveColorTemperatureID:
            result = @"MoveColorTemperature";
            break;

        case MTRCommandIDTypeClusterColorControlCommandStepColorTemperatureID:
            result = @"StepColorTemperature";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeBallastConfigurationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeIlluminanceMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTemperatureMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePressureMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeFlowMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeRelativeHumidityMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOccupancySensingID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCarbonMonoxideConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCarbonDioxideConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeNitrogenDioxideConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOzoneConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePM25ConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeFormaldehydeConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePM1ConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePM10ConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTotalVolatileOrganicCompoundsConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeRadonConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeSoilMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWiFiNetworkManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterWiFiNetworkManagementCommandNetworkPassphraseRequestID:
            result = @"NetworkPassphraseRequest";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeThreadBorderRouterManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterThreadBorderRouterManagementCommandGetActiveDatasetRequestID:
            result = @"GetActiveDatasetRequest";
            break;

        case MTRCommandIDTypeClusterThreadBorderRouterManagementCommandGetPendingDatasetRequestID:
            result = @"GetPendingDatasetRequest";
            break;

        case MTRCommandIDTypeClusterThreadBorderRouterManagementCommandSetActiveDatasetRequestID:
            result = @"SetActiveDatasetRequest";
            break;

        case MTRCommandIDTypeClusterThreadBorderRouterManagementCommandSetPendingDatasetRequestID:
            result = @"SetPendingDatasetRequest";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeThreadNetworkDirectoryID:

        switch (commandID) {

        case MTRCommandIDTypeClusterThreadNetworkDirectoryCommandAddNetworkID:
            result = @"AddNetwork";
            break;

        case MTRCommandIDTypeClusterThreadNetworkDirectoryCommandRemoveNetworkID:
            result = @"RemoveNetwork";
            break;

        case MTRCommandIDTypeClusterThreadNetworkDirectoryCommandGetOperationalDatasetID:
            result = @"GetOperationalDataset";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWakeOnLANID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeChannelID:

        switch (commandID) {

        case MTRCommandIDTypeClusterChannelCommandChangeChannelID:
            result = @"ChangeChannel";
            break;

        case MTRCommandIDTypeClusterChannelCommandChangeChannelByNumberID:
            result = @"ChangeChannelByNumber";
            break;

        case MTRCommandIDTypeClusterChannelCommandSkipChannelID:
            result = @"SkipChannel";
            break;

        case MTRCommandIDTypeClusterChannelCommandGetProgramGuideID:
            result = @"GetProgramGuide";
            break;

        case MTRCommandIDTypeClusterChannelCommandRecordProgramID:
            result = @"RecordProgram";
            break;

        case MTRCommandIDTypeClusterChannelCommandCancelRecordProgramID:
            result = @"CancelRecordProgram";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTargetNavigatorID:

        switch (commandID) {

        case MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetID:
            result = @"NavigateTarget";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeMediaPlaybackID:

        switch (commandID) {

        case MTRCommandIDTypeClusterMediaPlaybackCommandPlayID:
            result = @"Play";
            break;

        case MTRCommandIDTypeClusterMediaPlaybackCommandPauseID:
            result = @"Pause";
            break;

        case MTRCommandIDTypeClusterMediaPlaybackCommandStopID:
            result = @"Stop";
            break;

        case MTRCommandIDTypeClusterMediaPlaybackCommandStartOverID:
            result = @"StartOver";
            break;

        case MTRCommandIDTypeClusterMediaPlaybackCommandPreviousID:
            result = @"Previous";
            break;

        case MTRCommandIDTypeClusterMediaPlaybackCommandNextID:
            result = @"Next";
            break;

        case MTRCommandIDTypeClusterMediaPlaybackCommandRewindID:
            result = @"Rewind";
            break;

        case MTRCommandIDTypeClusterMediaPlaybackCommandFastForwardID:
            result = @"FastForward";
            break;

        case MTRCommandIDTypeClusterMediaPlaybackCommandSkipForwardID:
            result = @"SkipForward";
            break;

        case MTRCommandIDTypeClusterMediaPlaybackCommandSkipBackwardID:
            result = @"SkipBackward";
            break;

        case MTRCommandIDTypeClusterMediaPlaybackCommandSeekID:
            result = @"Seek";
            break;

        case MTRCommandIDTypeClusterMediaPlaybackCommandActivateAudioTrackID:
            result = @"ActivateAudioTrack";
            break;

        case MTRCommandIDTypeClusterMediaPlaybackCommandActivateTextTrackID:
            result = @"ActivateTextTrack";
            break;

        case MTRCommandIDTypeClusterMediaPlaybackCommandDeactivateTextTrackID:
            result = @"DeactivateTextTrack";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeMediaInputID:

        switch (commandID) {

        case MTRCommandIDTypeClusterMediaInputCommandSelectInputID:
            result = @"SelectInput";
            break;

        case MTRCommandIDTypeClusterMediaInputCommandShowInputStatusID:
            result = @"ShowInputStatus";
            break;

        case MTRCommandIDTypeClusterMediaInputCommandHideInputStatusID:
            result = @"HideInputStatus";
            break;

        case MTRCommandIDTypeClusterMediaInputCommandRenameInputID:
            result = @"RenameInput";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeLowPowerID:

        switch (commandID) {

        case MTRCommandIDTypeClusterLowPowerCommandSleepID:
            result = @"Sleep";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeKeypadInputID:

        switch (commandID) {

        case MTRCommandIDTypeClusterKeypadInputCommandSendKeyID:
            result = @"SendKey";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeContentLauncherID:

        switch (commandID) {

        case MTRCommandIDTypeClusterContentLauncherCommandLaunchContentID:
            result = @"LaunchContent";
            break;

        case MTRCommandIDTypeClusterContentLauncherCommandLaunchURLID:
            result = @"LaunchURL";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeAudioOutputID:

        switch (commandID) {

        case MTRCommandIDTypeClusterAudioOutputCommandSelectOutputID:
            result = @"SelectOutput";
            break;

        case MTRCommandIDTypeClusterAudioOutputCommandRenameOutputID:
            result = @"RenameOutput";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeApplicationLauncherID:

        switch (commandID) {

        case MTRCommandIDTypeClusterApplicationLauncherCommandLaunchAppID:
            result = @"LaunchApp";
            break;

        case MTRCommandIDTypeClusterApplicationLauncherCommandStopAppID:
            result = @"StopApp";
            break;

        case MTRCommandIDTypeClusterApplicationLauncherCommandHideAppID:
            result = @"HideApp";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeApplicationBasicID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeAccountLoginID:

        switch (commandID) {

        case MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINID:
            result = @"GetSetupPIN";
            break;

        case MTRCommandIDTypeClusterAccountLoginCommandLoginID:
            result = @"Login";
            break;

        case MTRCommandIDTypeClusterAccountLoginCommandLogoutID:
            result = @"Logout";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeContentControlID:

        switch (commandID) {

        case MTRCommandIDTypeClusterContentControlCommandUpdatePINID:
            result = @"UpdatePIN";
            break;

        case MTRCommandIDTypeClusterContentControlCommandResetPINID:
            result = @"ResetPIN";
            break;

        case MTRCommandIDTypeClusterContentControlCommandEnableID:
            result = @"Enable";
            break;

        case MTRCommandIDTypeClusterContentControlCommandDisableID:
            result = @"Disable";
            break;

        case MTRCommandIDTypeClusterContentControlCommandAddBonusTimeID:
            result = @"AddBonusTime";
            break;

        case MTRCommandIDTypeClusterContentControlCommandSetScreenDailyTimeID:
            result = @"SetScreenDailyTime";
            break;

        case MTRCommandIDTypeClusterContentControlCommandBlockUnratedContentID:
            result = @"BlockUnratedContent";
            break;

        case MTRCommandIDTypeClusterContentControlCommandUnblockUnratedContentID:
            result = @"UnblockUnratedContent";
            break;

        case MTRCommandIDTypeClusterContentControlCommandSetOnDemandRatingThresholdID:
            result = @"SetOnDemandRatingThreshold";
            break;

        case MTRCommandIDTypeClusterContentControlCommandSetScheduledContentRatingThresholdID:
            result = @"SetScheduledContentRatingThreshold";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeContentAppObserverID:

        switch (commandID) {

        case MTRCommandIDTypeClusterContentAppObserverCommandContentAppMessageID:
            result = @"ContentAppMessage";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeZoneManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterZoneManagementCommandCreateTwoDCartesianZoneID:
            result = @"CreateTwoDCartesianZone";
            break;

        case MTRCommandIDTypeClusterZoneManagementCommandUpdateTwoDCartesianZoneID:
            result = @"UpdateTwoDCartesianZone";
            break;

        case MTRCommandIDTypeClusterZoneManagementCommandRemoveZoneID:
            result = @"RemoveZone";
            break;

        case MTRCommandIDTypeClusterZoneManagementCommandCreateOrUpdateTriggerID:
            result = @"CreateOrUpdateTrigger";
            break;

        case MTRCommandIDTypeClusterZoneManagementCommandRemoveTriggerID:
            result = @"RemoveTrigger";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCameraAVStreamManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterCameraAVStreamManagementCommandAudioStreamAllocateID:
            result = @"AudioStreamAllocate";
            break;

        case MTRCommandIDTypeClusterCameraAVStreamManagementCommandAudioStreamDeallocateID:
            result = @"AudioStreamDeallocate";
            break;

        case MTRCommandIDTypeClusterCameraAVStreamManagementCommandVideoStreamAllocateID:
            result = @"VideoStreamAllocate";
            break;

        case MTRCommandIDTypeClusterCameraAVStreamManagementCommandVideoStreamModifyID:
            result = @"VideoStreamModify";
            break;

        case MTRCommandIDTypeClusterCameraAVStreamManagementCommandVideoStreamDeallocateID:
            result = @"VideoStreamDeallocate";
            break;

        case MTRCommandIDTypeClusterCameraAVStreamManagementCommandSnapshotStreamAllocateID:
            result = @"SnapshotStreamAllocate";
            break;

        case MTRCommandIDTypeClusterCameraAVStreamManagementCommandSnapshotStreamModifyID:
            result = @"SnapshotStreamModify";
            break;

        case MTRCommandIDTypeClusterCameraAVStreamManagementCommandSnapshotStreamDeallocateID:
            result = @"SnapshotStreamDeallocate";
            break;

        case MTRCommandIDTypeClusterCameraAVStreamManagementCommandSetStreamPrioritiesID:
            result = @"SetStreamPriorities";
            break;

        case MTRCommandIDTypeClusterCameraAVStreamManagementCommandCaptureSnapshotID:
            result = @"CaptureSnapshot";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCameraAVSettingsUserLevelManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterCameraAVSettingsUserLevelManagementCommandMPTZSetPositionID:
            result = @"MPTZSetPosition";
            break;

        case MTRCommandIDTypeClusterCameraAVSettingsUserLevelManagementCommandMPTZRelativeMoveID:
            result = @"MPTZRelativeMove";
            break;

        case MTRCommandIDTypeClusterCameraAVSettingsUserLevelManagementCommandMPTZMoveToPresetID:
            result = @"MPTZMoveToPreset";
            break;

        case MTRCommandIDTypeClusterCameraAVSettingsUserLevelManagementCommandMPTZSavePresetID:
            result = @"MPTZSavePreset";
            break;

        case MTRCommandIDTypeClusterCameraAVSettingsUserLevelManagementCommandMPTZRemovePresetID:
            result = @"MPTZRemovePreset";
            break;

        case MTRCommandIDTypeClusterCameraAVSettingsUserLevelManagementCommandDPTZSetViewportID:
            result = @"DPTZSetViewport";
            break;

        case MTRCommandIDTypeClusterCameraAVSettingsUserLevelManagementCommandDPTZRelativeMoveID:
            result = @"DPTZRelativeMove";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWebRTCTransportProviderID:

        switch (commandID) {

        case MTRCommandIDTypeClusterWebRTCTransportProviderCommandSolicitOfferID:
            result = @"SolicitOffer";
            break;

        case MTRCommandIDTypeClusterWebRTCTransportProviderCommandProvideOfferID:
            result = @"ProvideOffer";
            break;

        case MTRCommandIDTypeClusterWebRTCTransportProviderCommandProvideAnswerID:
            result = @"ProvideAnswer";
            break;

        case MTRCommandIDTypeClusterWebRTCTransportProviderCommandProvideICECandidatesID:
            result = @"ProvideICECandidates";
            break;

        case MTRCommandIDTypeClusterWebRTCTransportProviderCommandEndSessionID:
            result = @"EndSession";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWebRTCTransportRequestorID:

        switch (commandID) {

        case MTRCommandIDTypeClusterWebRTCTransportRequestorCommandOfferID:
            result = @"Offer";
            break;

        case MTRCommandIDTypeClusterWebRTCTransportRequestorCommandAnswerID:
            result = @"Answer";
            break;

        case MTRCommandIDTypeClusterWebRTCTransportRequestorCommandICECandidatesID:
            result = @"ICECandidates";
            break;

        case MTRCommandIDTypeClusterWebRTCTransportRequestorCommandEndID:
            result = @"End";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePushAVStreamTransportID:

        switch (commandID) {

        case MTRCommandIDTypeClusterPushAVStreamTransportCommandAllocatePushTransportID:
            result = @"AllocatePushTransport";
            break;

        case MTRCommandIDTypeClusterPushAVStreamTransportCommandDeallocatePushTransportID:
            result = @"DeallocatePushTransport";
            break;

        case MTRCommandIDTypeClusterPushAVStreamTransportCommandModifyPushTransportID:
            result = @"ModifyPushTransport";
            break;

        case MTRCommandIDTypeClusterPushAVStreamTransportCommandSetTransportStatusID:
            result = @"SetTransportStatus";
            break;

        case MTRCommandIDTypeClusterPushAVStreamTransportCommandManuallyTriggerTransportID:
            result = @"ManuallyTriggerTransport";
            break;

        case MTRCommandIDTypeClusterPushAVStreamTransportCommandFindTransportID:
            result = @"FindTransport";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeChimeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterChimeCommandPlayChimeSoundID:
            result = @"PlayChimeSound";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCommodityTariffID:

        switch (commandID) {

        case MTRCommandIDTypeClusterCommodityTariffCommandGetTariffComponentID:
            result = @"GetTariffComponent";
            break;

        case MTRCommandIDTypeClusterCommodityTariffCommandGetDayEntryID:
            result = @"GetDayEntry";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeEcosystemInformationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCommissionerControlID:

        switch (commandID) {

        case MTRCommandIDTypeClusterCommissionerControlCommandRequestCommissioningApprovalID:
            result = @"RequestCommissioningApproval";
            break;

        case MTRCommandIDTypeClusterCommissionerControlCommandCommissionNodeID:
            result = @"CommissionNode";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeJointFabricDatastoreID:

        switch (commandID) {

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandAddKeySetID:
            result = @"AddKeySet";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandUpdateKeySetID:
            result = @"UpdateKeySet";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandRemoveKeySetID:
            result = @"RemoveKeySet";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandAddGroupID:
            result = @"AddGroup";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandUpdateGroupID:
            result = @"UpdateGroup";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandRemoveGroupID:
            result = @"RemoveGroup";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandAddAdminID:
            result = @"AddAdmin";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandUpdateAdminID:
            result = @"UpdateAdmin";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandRemoveAdminID:
            result = @"RemoveAdmin";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandAddPendingNodeID:
            result = @"AddPendingNode";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandRefreshNodeID:
            result = @"RefreshNode";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandUpdateNodeID:
            result = @"UpdateNode";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandRemoveNodeID:
            result = @"RemoveNode";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandUpdateEndpointForNodeID:
            result = @"UpdateEndpointForNode";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandAddGroupIDToEndpointForNodeID:
            result = @"AddGroupIDToEndpointForNode";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandRemoveGroupIDFromEndpointForNodeID:
            result = @"RemoveGroupIDFromEndpointForNode";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandAddBindingToEndpointForNodeID:
            result = @"AddBindingToEndpointForNode";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandRemoveBindingFromEndpointForNodeID:
            result = @"RemoveBindingFromEndpointForNode";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandAddACLToNodeID:
            result = @"AddACLToNode";
            break;

        case MTRCommandIDTypeClusterJointFabricDatastoreCommandRemoveACLFromNodeID:
            result = @"RemoveACLFromNode";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeJointFabricAdministratorID:

        switch (commandID) {

        case MTRCommandIDTypeClusterJointFabricAdministratorCommandICACCSRRequestID:
            result = @"ICACCSRRequest";
            break;

        case MTRCommandIDTypeClusterJointFabricAdministratorCommandAddICACID:
            result = @"AddICAC";
            break;

        case MTRCommandIDTypeClusterJointFabricAdministratorCommandOpenJointCommissioningWindowID:
            result = @"OpenJointCommissioningWindow";
            break;

        case MTRCommandIDTypeClusterJointFabricAdministratorCommandTransferAnchorRequestID:
            result = @"TransferAnchorRequest";
            break;

        case MTRCommandIDTypeClusterJointFabricAdministratorCommandTransferAnchorCompleteID:
            result = @"TransferAnchorComplete";
            break;

        case MTRCommandIDTypeClusterJointFabricAdministratorCommandAnnounceJointFabricAdministratorID:
            result = @"AnnounceJointFabricAdministrator";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTLSCertificateManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterTLSCertificateManagementCommandProvisionRootCertificateID:
            result = @"ProvisionRootCertificate";
            break;

        case MTRCommandIDTypeClusterTLSCertificateManagementCommandFindRootCertificateID:
            result = @"FindRootCertificate";
            break;

        case MTRCommandIDTypeClusterTLSCertificateManagementCommandLookupRootCertificateID:
            result = @"LookupRootCertificate";
            break;

        case MTRCommandIDTypeClusterTLSCertificateManagementCommandRemoveRootCertificateID:
            result = @"RemoveRootCertificate";
            break;

        case MTRCommandIDTypeClusterTLSCertificateManagementCommandTLSClientCSRID:
            result = @"TLSClientCSR";
            break;

        case MTRCommandIDTypeClusterTLSCertificateManagementCommandProvisionClientCertificateID:
            result = @"ProvisionClientCertificate";
            break;

        case MTRCommandIDTypeClusterTLSCertificateManagementCommandFindClientCertificateID:
            result = @"FindClientCertificate";
            break;

        case MTRCommandIDTypeClusterTLSCertificateManagementCommandLookupClientCertificateID:
            result = @"LookupClientCertificate";
            break;

        case MTRCommandIDTypeClusterTLSCertificateManagementCommandRemoveClientCertificateID:
            result = @"RemoveClientCertificate";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTLSClientManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterTLSClientManagementCommandProvisionEndpointID:
            result = @"ProvisionEndpoint";
            break;

        case MTRCommandIDTypeClusterTLSClientManagementCommandFindEndpointID:
            result = @"FindEndpoint";
            break;

        case MTRCommandIDTypeClusterTLSClientManagementCommandRemoveEndpointID:
            result = @"RemoveEndpoint";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeMeterIdentificationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCommodityMeteringID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeUnitTestingID:

        switch (commandID) {

        case MTRCommandIDTypeClusterUnitTestingCommandTestID:
            result = @"Test";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestNotHandledID:
            result = @"TestNotHandled";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestSpecificID:
            result = @"TestSpecific";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestUnknownCommandID:
            result = @"TestUnknownCommand";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsID:
            result = @"TestAddArguments";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentRequestID:
            result = @"TestSimpleArgumentRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentRequestID:
            result = @"TestStructArrayArgumentRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestStructArgumentRequestID:
            result = @"TestStructArgumentRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructArgumentRequestID:
            result = @"TestNestedStructArgumentRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestListStructArgumentRequestID:
            result = @"TestListStructArgumentRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UArgumentRequestID:
            result = @"TestListInt8UArgumentRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructListArgumentRequestID:
            result = @"TestNestedStructListArgumentRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestListNestedStructListArgumentRequestID:
            result = @"TestListNestedStructListArgumentRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseRequestID:
            result = @"TestListInt8UReverseRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestEnumsRequestID:
            result = @"TestEnumsRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalRequestID:
            result = @"TestNullableOptionalRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalRequestID:
            result = @"TestComplexNullableOptionalRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandSimpleStructEchoRequestID:
            result = @"SimpleStructEchoRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTimedInvokeRequestID:
            result = @"TimedInvokeRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestSimpleOptionalArgumentRequestID:
            result = @"TestSimpleOptionalArgumentRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventRequestID:
            result = @"TestEmitTestEventRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventRequestID:
            result = @"TestEmitTestFabricScopedEventRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestBatchHelperRequestID:
            result = @"TestBatchHelperRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestSecondBatchHelperRequestID:
            result = @"TestSecondBatchHelperRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandStringEchoRequestID:
            result = @"StringEchoRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandGlobalEchoRequestID:
            result = @"GlobalEchoRequest";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestCheckCommandFlagsID:
            result = @"TestCheckCommandFlags";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestDifferentVendorMeiRequestID:
            result = @"TestDifferentVendorMeiRequest";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeSampleMEIID:

        switch (commandID) {

        case MTRCommandIDTypeClusterSampleMEICommandPingID:
            result = @"Ping";
            break;

        case MTRCommandIDTypeClusterSampleMEICommandAddArgumentsID:
            result = @"AddArguments";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    default:
        result = [NSString stringWithFormat:@"<Unknown clusterID %u>", clusterID];
        break;
    }

    return result;
}

NSString * MTRResponseCommandNameForID(MTRClusterIDType clusterID, MTRCommandIDType commandID)
{
    NSString * result = nil;

    switch (clusterID) {

    case MTRClusterIDTypeIdentifyID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeGroupsID:

        switch (commandID) {

        case MTRCommandIDTypeClusterGroupsCommandAddGroupResponseID:
            result = @"AddGroupResponse";
            break;

        case MTRCommandIDTypeClusterGroupsCommandViewGroupResponseID:
            result = @"ViewGroupResponse";
            break;

        case MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipResponseID:
            result = @"GetGroupMembershipResponse";
            break;

        case MTRCommandIDTypeClusterGroupsCommandRemoveGroupResponseID:
            result = @"RemoveGroupResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOnOffID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeLevelControlID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePulseWidthModulationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeDescriptorID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeBindingID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeAccessControlID:

        switch (commandID) {

        case MTRCommandIDTypeClusterAccessControlCommandReviewFabricRestrictionsResponseID:
            result = @"ReviewFabricRestrictionsResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeActionsID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeBasicInformationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOTASoftwareUpdateProviderID:

        switch (commandID) {

        case MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageResponseID:
            result = @"QueryImageResponse";
            break;

        case MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateResponseID:
            result = @"ApplyUpdateResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOTASoftwareUpdateRequestorID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeLocalizationConfigurationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTimeFormatLocalizationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeUnitLocalizationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePowerSourceConfigurationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePowerSourceID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeGeneralCommissioningID:

        switch (commandID) {

        case MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeResponseID:
            result = @"ArmFailSafeResponse";
            break;

        case MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigResponseID:
            result = @"SetRegulatoryConfigResponse";
            break;

        case MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteResponseID:
            result = @"CommissioningCompleteResponse";
            break;

        case MTRCommandIDTypeClusterGeneralCommissioningCommandSetTCAcknowledgementsResponseID:
            result = @"SetTCAcknowledgementsResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeNetworkCommissioningID:

        switch (commandID) {

        case MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksResponseID:
            result = @"ScanNetworksResponse";
            break;

        case MTRCommandIDTypeClusterNetworkCommissioningCommandNetworkConfigResponseID:
            result = @"NetworkConfigResponse";
            break;

        case MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkResponseID:
            result = @"ConnectNetworkResponse";
            break;

        case MTRCommandIDTypeClusterNetworkCommissioningCommandQueryIdentityResponseID:
            result = @"QueryIdentityResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeDiagnosticLogsID:

        switch (commandID) {

        case MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsResponseID:
            result = @"RetrieveLogsResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeGeneralDiagnosticsID:

        switch (commandID) {

        case MTRCommandIDTypeClusterGeneralDiagnosticsCommandTimeSnapshotResponseID:
            result = @"TimeSnapshotResponse";
            break;

        case MTRCommandIDTypeClusterGeneralDiagnosticsCommandPayloadTestResponseID:
            result = @"PayloadTestResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeSoftwareDiagnosticsID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeThreadNetworkDiagnosticsID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWiFiNetworkDiagnosticsID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeEthernetNetworkDiagnosticsID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTimeSynchronizationID:

        switch (commandID) {

        case MTRCommandIDTypeClusterTimeSynchronizationCommandSetTimeZoneResponseID:
            result = @"SetTimeZoneResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeBridgedDeviceBasicInformationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeSwitchID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeAdministratorCommissioningID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOperationalCredentialsID:

        switch (commandID) {

        case MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationResponseID:
            result = @"AttestationResponse";
            break;

        case MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainResponseID:
            result = @"CertificateChainResponse";
            break;

        case MTRCommandIDTypeClusterOperationalCredentialsCommandCSRResponseID:
            result = @"CSRResponse";
            break;

        case MTRCommandIDTypeClusterOperationalCredentialsCommandNOCResponseID:
            result = @"NOCResponse";
            break;

        case MTRCommandIDTypeClusterOperationalCredentialsCommandSignVIDVerificationResponseID:
            result = @"SignVIDVerificationResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeGroupKeyManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadResponseID:
            result = @"KeySetReadResponse";
            break;

        case MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesResponseID:
            result = @"KeySetReadAllIndicesResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeFixedLabelID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeUserLabelID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeBooleanStateID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeICDManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterICDManagementCommandRegisterClientResponseID:
            result = @"RegisterClientResponse";
            break;

        case MTRCommandIDTypeClusterICDManagementCommandStayActiveResponseID:
            result = @"StayActiveResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTimerID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOvenCavityOperationalStateID:

        switch (commandID) {

        case MTRCommandIDTypeClusterOvenCavityOperationalStateCommandOperationalCommandResponseID:
            result = @"OperationalCommandResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOvenModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterOvenModeCommandChangeToModeResponseID:
            result = @"ChangeToModeResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeLaundryDryerControlsID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeModeSelectID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeLaundryWasherModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterLaundryWasherModeCommandChangeToModeResponseID:
            result = @"ChangeToModeResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeRefrigeratorAndTemperatureControlledCabinetModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeCommandChangeToModeResponseID:
            result = @"ChangeToModeResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeLaundryWasherControlsID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeRVCRunModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterRVCRunModeCommandChangeToModeResponseID:
            result = @"ChangeToModeResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeRVCCleanModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterRVCCleanModeCommandChangeToModeResponseID:
            result = @"ChangeToModeResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTemperatureControlID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeRefrigeratorAlarmID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeDishwasherModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterDishwasherModeCommandChangeToModeResponseID:
            result = @"ChangeToModeResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeAirQualityID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeSmokeCOAlarmID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeDishwasherAlarmID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeMicrowaveOvenModeID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeMicrowaveOvenControlID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOperationalStateID:

        switch (commandID) {

        case MTRCommandIDTypeClusterOperationalStateCommandOperationalCommandResponseID:
            result = @"OperationalCommandResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeRVCOperationalStateID:

        switch (commandID) {

        case MTRCommandIDTypeClusterRVCOperationalStateCommandOperationalCommandResponseID:
            result = @"OperationalCommandResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeScenesManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterScenesManagementCommandAddSceneResponseID:
            result = @"AddSceneResponse";
            break;

        case MTRCommandIDTypeClusterScenesManagementCommandViewSceneResponseID:
            result = @"ViewSceneResponse";
            break;

        case MTRCommandIDTypeClusterScenesManagementCommandRemoveSceneResponseID:
            result = @"RemoveSceneResponse";
            break;

        case MTRCommandIDTypeClusterScenesManagementCommandRemoveAllScenesResponseID:
            result = @"RemoveAllScenesResponse";
            break;

        case MTRCommandIDTypeClusterScenesManagementCommandStoreSceneResponseID:
            result = @"StoreSceneResponse";
            break;

        case MTRCommandIDTypeClusterScenesManagementCommandGetSceneMembershipResponseID:
            result = @"GetSceneMembershipResponse";
            break;

        case MTRCommandIDTypeClusterScenesManagementCommandCopySceneResponseID:
            result = @"CopySceneResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeHEPAFilterMonitoringID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeActivatedCarbonFilterMonitoringID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeBooleanStateConfigurationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeValveConfigurationAndControlID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeElectricalPowerMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeElectricalEnergyMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWaterHeaterManagementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCommodityPriceID:

        switch (commandID) {

        case MTRCommandIDTypeClusterCommodityPriceCommandGetDetailedPriceResponseID:
            result = @"GetDetailedPriceResponse";
            break;

        case MTRCommandIDTypeClusterCommodityPriceCommandGetDetailedForecastResponseID:
            result = @"GetDetailedForecastResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeMessagesID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeDeviceEnergyManagementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeEnergyEVSEID:

        switch (commandID) {

        case MTRCommandIDTypeClusterEnergyEVSECommandGetTargetsResponseID:
            result = @"GetTargetsResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeEnergyPreferenceID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePowerTopologyID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeEnergyEVSEModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterEnergyEVSEModeCommandChangeToModeResponseID:
            result = @"ChangeToModeResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWaterHeaterModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterWaterHeaterModeCommandChangeToModeResponseID:
            result = @"ChangeToModeResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeDeviceEnergyManagementModeID:

        switch (commandID) {

        case MTRCommandIDTypeClusterDeviceEnergyManagementModeCommandChangeToModeResponseID:
            result = @"ChangeToModeResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeElectricalGridConditionsID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeDoorLockID:

        switch (commandID) {

        case MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleResponseID:
            result = @"GetWeekDayScheduleResponse";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleResponseID:
            result = @"GetYearDayScheduleResponse";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleResponseID:
            result = @"GetHolidayScheduleResponse";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandGetUserResponseID:
            result = @"GetUserResponse";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandSetCredentialResponseID:
            result = @"SetCredentialResponse";
            break;

        case MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusResponseID:
            result = @"GetCredentialStatusResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWindowCoveringID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeClosureControlID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeClosureDimensionID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeServiceAreaID:

        switch (commandID) {

        case MTRCommandIDTypeClusterServiceAreaCommandSelectAreasResponseID:
            result = @"SelectAreasResponse";
            break;

        case MTRCommandIDTypeClusterServiceAreaCommandSkipAreaResponseID:
            result = @"SkipAreaResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePumpConfigurationAndControlID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeThermostatID:

        switch (commandID) {

        case MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleResponseID:
            result = @"GetWeeklyScheduleResponse";
            break;

        case MTRCommandIDTypeClusterThermostatCommandAddThermostatSuggestionResponseID:
            result = @"AddThermostatSuggestionResponse";
            break;

        case MTRCommandIDTypeClusterThermostatCommandAtomicResponseID:
            result = @"AtomicResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeFanControlID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeThermostatUserInterfaceConfigurationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeColorControlID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeBallastConfigurationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeIlluminanceMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTemperatureMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePressureMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeFlowMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeRelativeHumidityMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOccupancySensingID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCarbonMonoxideConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCarbonDioxideConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeNitrogenDioxideConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeOzoneConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePM25ConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeFormaldehydeConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePM1ConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePM10ConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTotalVolatileOrganicCompoundsConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeRadonConcentrationMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeSoilMeasurementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWiFiNetworkManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterWiFiNetworkManagementCommandNetworkPassphraseResponseID:
            result = @"NetworkPassphraseResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeThreadBorderRouterManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterThreadBorderRouterManagementCommandDatasetResponseID:
            result = @"DatasetResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeThreadNetworkDirectoryID:

        switch (commandID) {

        case MTRCommandIDTypeClusterThreadNetworkDirectoryCommandOperationalDatasetResponseID:
            result = @"OperationalDatasetResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWakeOnLANID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeChannelID:

        switch (commandID) {

        case MTRCommandIDTypeClusterChannelCommandChangeChannelResponseID:
            result = @"ChangeChannelResponse";
            break;

        case MTRCommandIDTypeClusterChannelCommandProgramGuideResponseID:
            result = @"ProgramGuideResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTargetNavigatorID:

        switch (commandID) {

        case MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetResponseID:
            result = @"NavigateTargetResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeMediaPlaybackID:

        switch (commandID) {

        case MTRCommandIDTypeClusterMediaPlaybackCommandPlaybackResponseID:
            result = @"PlaybackResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeMediaInputID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeLowPowerID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeKeypadInputID:

        switch (commandID) {

        case MTRCommandIDTypeClusterKeypadInputCommandSendKeyResponseID:
            result = @"SendKeyResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeContentLauncherID:

        switch (commandID) {

        case MTRCommandIDTypeClusterContentLauncherCommandLauncherResponseID:
            result = @"LauncherResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeAudioOutputID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeApplicationLauncherID:

        switch (commandID) {

        case MTRCommandIDTypeClusterApplicationLauncherCommandLauncherResponseID:
            result = @"LauncherResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeApplicationBasicID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeAccountLoginID:

        switch (commandID) {

        case MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINResponseID:
            result = @"GetSetupPINResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeContentControlID:

        switch (commandID) {

        case MTRCommandIDTypeClusterContentControlCommandResetPINResponseID:
            result = @"ResetPINResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeContentAppObserverID:

        switch (commandID) {

        case MTRCommandIDTypeClusterContentAppObserverCommandContentAppMessageResponseID:
            result = @"ContentAppMessageResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeZoneManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterZoneManagementCommandCreateTwoDCartesianZoneResponseID:
            result = @"CreateTwoDCartesianZoneResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCameraAVStreamManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterCameraAVStreamManagementCommandAudioStreamAllocateResponseID:
            result = @"AudioStreamAllocateResponse";
            break;

        case MTRCommandIDTypeClusterCameraAVStreamManagementCommandVideoStreamAllocateResponseID:
            result = @"VideoStreamAllocateResponse";
            break;

        case MTRCommandIDTypeClusterCameraAVStreamManagementCommandSnapshotStreamAllocateResponseID:
            result = @"SnapshotStreamAllocateResponse";
            break;

        case MTRCommandIDTypeClusterCameraAVStreamManagementCommandCaptureSnapshotResponseID:
            result = @"CaptureSnapshotResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCameraAVSettingsUserLevelManagementID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWebRTCTransportProviderID:

        switch (commandID) {

        case MTRCommandIDTypeClusterWebRTCTransportProviderCommandSolicitOfferResponseID:
            result = @"SolicitOfferResponse";
            break;

        case MTRCommandIDTypeClusterWebRTCTransportProviderCommandProvideOfferResponseID:
            result = @"ProvideOfferResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeWebRTCTransportRequestorID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypePushAVStreamTransportID:

        switch (commandID) {

        case MTRCommandIDTypeClusterPushAVStreamTransportCommandAllocatePushTransportResponseID:
            result = @"AllocatePushTransportResponse";
            break;

        case MTRCommandIDTypeClusterPushAVStreamTransportCommandFindTransportResponseID:
            result = @"FindTransportResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeChimeID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCommodityTariffID:

        switch (commandID) {

        case MTRCommandIDTypeClusterCommodityTariffCommandGetTariffComponentResponseID:
            result = @"GetTariffComponentResponse";
            break;

        case MTRCommandIDTypeClusterCommodityTariffCommandGetDayEntryResponseID:
            result = @"GetDayEntryResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeEcosystemInformationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCommissionerControlID:

        switch (commandID) {

        case MTRCommandIDTypeClusterCommissionerControlCommandReverseOpenCommissioningWindowID:
            result = @"ReverseOpenCommissioningWindow";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeJointFabricDatastoreID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeJointFabricAdministratorID:

        switch (commandID) {

        case MTRCommandIDTypeClusterJointFabricAdministratorCommandICACCSRResponseID:
            result = @"ICACCSRResponse";
            break;

        case MTRCommandIDTypeClusterJointFabricAdministratorCommandICACResponseID:
            result = @"ICACResponse";
            break;

        case MTRCommandIDTypeClusterJointFabricAdministratorCommandTransferAnchorResponseID:
            result = @"TransferAnchorResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTLSCertificateManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterTLSCertificateManagementCommandProvisionRootCertificateResponseID:
            result = @"ProvisionRootCertificateResponse";
            break;

        case MTRCommandIDTypeClusterTLSCertificateManagementCommandFindRootCertificateResponseID:
            result = @"FindRootCertificateResponse";
            break;

        case MTRCommandIDTypeClusterTLSCertificateManagementCommandLookupRootCertificateResponseID:
            result = @"LookupRootCertificateResponse";
            break;

        case MTRCommandIDTypeClusterTLSCertificateManagementCommandTLSClientCSRResponseID:
            result = @"TLSClientCSRResponse";
            break;

        case MTRCommandIDTypeClusterTLSCertificateManagementCommandFindClientCertificateResponseID:
            result = @"FindClientCertificateResponse";
            break;

        case MTRCommandIDTypeClusterTLSCertificateManagementCommandLookupClientCertificateResponseID:
            result = @"LookupClientCertificateResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeTLSClientManagementID:

        switch (commandID) {

        case MTRCommandIDTypeClusterTLSClientManagementCommandProvisionEndpointResponseID:
            result = @"ProvisionEndpointResponse";
            break;

        case MTRCommandIDTypeClusterTLSClientManagementCommandFindEndpointResponseID:
            result = @"FindEndpointResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeMeterIdentificationID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeCommodityMeteringID:

        switch (commandID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeUnitTestingID:

        switch (commandID) {

        case MTRCommandIDTypeClusterUnitTestingCommandTestSpecificResponseID:
            result = @"TestSpecificResponse";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsResponseID:
            result = @"TestAddArgumentsResponse";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentResponseID:
            result = @"TestSimpleArgumentResponse";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentResponseID:
            result = @"TestStructArrayArgumentResponse";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseResponseID:
            result = @"TestListInt8UReverseResponse";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestEnumsResponseID:
            result = @"TestEnumsResponse";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalResponseID:
            result = @"TestNullableOptionalResponse";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalResponseID:
            result = @"TestComplexNullableOptionalResponse";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandBooleanResponseID:
            result = @"BooleanResponse";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandSimpleStructResponseID:
            result = @"SimpleStructResponse";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventResponseID:
            result = @"TestEmitTestEventResponse";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventResponseID:
            result = @"TestEmitTestFabricScopedEventResponse";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestBatchHelperResponseID:
            result = @"TestBatchHelperResponse";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandStringEchoResponseID:
            result = @"StringEchoResponse";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandGlobalEchoResponseID:
            result = @"GlobalEchoResponse";
            break;

        case MTRCommandIDTypeClusterUnitTestingCommandTestDifferentVendorMeiResponseID:
            result = @"TestDifferentVendorMeiResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    case MTRClusterIDTypeSampleMEIID:

        switch (commandID) {

        case MTRCommandIDTypeClusterSampleMEICommandAddArgumentsResponseID:
            result = @"AddArgumentsResponse";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown commandID %u>", commandID];
            break;
        }
        break;

    default:
        result = [NSString stringWithFormat:@"<Unknown clusterID %u>", clusterID];
        break;
    }

    return result;
}

#pragma mark - Event IDs

NSString * MTREventNameForID(MTRClusterIDType clusterID, MTREventIDType eventID)
{
    NSString * result = nil;

    switch (clusterID) {

    case MTRClusterIDTypeIdentifyID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeGroupsID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeOnOffID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeLevelControlID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypePulseWidthModulationID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeDescriptorID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeBindingID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeAccessControlID:

        switch (eventID) {

            // Cluster AccessControl events
        case MTREventIDTypeClusterAccessControlEventAccessControlEntryChangedID:
            result = @"AccessControlEntryChanged";
            break;

        case MTREventIDTypeClusterAccessControlEventAccessControlExtensionChangedID:
            result = @"AccessControlExtensionChanged";
            break;

        case MTREventIDTypeClusterAccessControlEventFabricRestrictionReviewUpdateID:
            result = @"FabricRestrictionReviewUpdate";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeActionsID:

        switch (eventID) {

            // Cluster Actions events
        case MTREventIDTypeClusterActionsEventStateChangedID:
            result = @"StateChanged";
            break;

        case MTREventIDTypeClusterActionsEventActionFailedID:
            result = @"ActionFailed";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeBasicInformationID:

        switch (eventID) {

            // Cluster BasicInformation events
        case MTREventIDTypeClusterBasicInformationEventStartUpID:
            result = @"StartUp";
            break;

        case MTREventIDTypeClusterBasicInformationEventShutDownID:
            result = @"ShutDown";
            break;

        case MTREventIDTypeClusterBasicInformationEventLeaveID:
            result = @"Leave";
            break;

        case MTREventIDTypeClusterBasicInformationEventReachableChangedID:
            result = @"ReachableChanged";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeOTASoftwareUpdateProviderID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeOTASoftwareUpdateRequestorID:

        switch (eventID) {

            // Cluster OTASoftwareUpdateRequestor events
        case MTREventIDTypeClusterOTASoftwareUpdateRequestorEventStateTransitionID:
            result = @"StateTransition";
            break;

        case MTREventIDTypeClusterOTASoftwareUpdateRequestorEventVersionAppliedID:
            result = @"VersionApplied";
            break;

        case MTREventIDTypeClusterOTASoftwareUpdateRequestorEventDownloadErrorID:
            result = @"DownloadError";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeLocalizationConfigurationID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeTimeFormatLocalizationID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeUnitLocalizationID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypePowerSourceConfigurationID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypePowerSourceID:

        switch (eventID) {

            // Cluster PowerSource events
        case MTREventIDTypeClusterPowerSourceEventWiredFaultChangeID:
            result = @"WiredFaultChange";
            break;

        case MTREventIDTypeClusterPowerSourceEventBatFaultChangeID:
            result = @"BatFaultChange";
            break;

        case MTREventIDTypeClusterPowerSourceEventBatChargeFaultChangeID:
            result = @"BatChargeFaultChange";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeGeneralCommissioningID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeNetworkCommissioningID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeDiagnosticLogsID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeGeneralDiagnosticsID:

        switch (eventID) {

            // Cluster GeneralDiagnostics events
        case MTREventIDTypeClusterGeneralDiagnosticsEventHardwareFaultChangeID:
            result = @"HardwareFaultChange";
            break;

        case MTREventIDTypeClusterGeneralDiagnosticsEventRadioFaultChangeID:
            result = @"RadioFaultChange";
            break;

        case MTREventIDTypeClusterGeneralDiagnosticsEventNetworkFaultChangeID:
            result = @"NetworkFaultChange";
            break;

        case MTREventIDTypeClusterGeneralDiagnosticsEventBootReasonID:
            result = @"BootReason";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeSoftwareDiagnosticsID:

        switch (eventID) {

            // Cluster SoftwareDiagnostics events
        case MTREventIDTypeClusterSoftwareDiagnosticsEventSoftwareFaultID:
            result = @"SoftwareFault";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeThreadNetworkDiagnosticsID:

        switch (eventID) {

            // Cluster ThreadNetworkDiagnostics events
        case MTREventIDTypeClusterThreadNetworkDiagnosticsEventConnectionStatusID:
            result = @"ConnectionStatus";
            break;

        case MTREventIDTypeClusterThreadNetworkDiagnosticsEventNetworkFaultChangeID:
            result = @"NetworkFaultChange";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeWiFiNetworkDiagnosticsID:

        switch (eventID) {

            // Cluster WiFiNetworkDiagnostics events
        case MTREventIDTypeClusterWiFiNetworkDiagnosticsEventDisconnectionID:
            result = @"Disconnection";
            break;

        case MTREventIDTypeClusterWiFiNetworkDiagnosticsEventAssociationFailureID:
            result = @"AssociationFailure";
            break;

        case MTREventIDTypeClusterWiFiNetworkDiagnosticsEventConnectionStatusID:
            result = @"ConnectionStatus";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeEthernetNetworkDiagnosticsID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeTimeSynchronizationID:

        switch (eventID) {

            // Cluster TimeSynchronization events
        case MTREventIDTypeClusterTimeSynchronizationEventDSTTableEmptyID:
            result = @"DSTTableEmpty";
            break;

        case MTREventIDTypeClusterTimeSynchronizationEventDSTStatusID:
            result = @"DSTStatus";
            break;

        case MTREventIDTypeClusterTimeSynchronizationEventTimeZoneStatusID:
            result = @"TimeZoneStatus";
            break;

        case MTREventIDTypeClusterTimeSynchronizationEventTimeFailureID:
            result = @"TimeFailure";
            break;

        case MTREventIDTypeClusterTimeSynchronizationEventMissingTrustedTimeSourceID:
            result = @"MissingTrustedTimeSource";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeBridgedDeviceBasicInformationID:

        switch (eventID) {

            // Cluster BridgedDeviceBasicInformation events
        case MTREventIDTypeClusterBridgedDeviceBasicInformationEventStartUpID:
            result = @"StartUp";
            break;

        case MTREventIDTypeClusterBridgedDeviceBasicInformationEventShutDownID:
            result = @"ShutDown";
            break;

        case MTREventIDTypeClusterBridgedDeviceBasicInformationEventLeaveID:
            result = @"Leave";
            break;

        case MTREventIDTypeClusterBridgedDeviceBasicInformationEventReachableChangedID:
            result = @"ReachableChanged";
            break;

        case MTREventIDTypeClusterBridgedDeviceBasicInformationEventActiveChangedID:
            result = @"ActiveChanged";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeSwitchID:

        switch (eventID) {

            // Cluster Switch events
        case MTREventIDTypeClusterSwitchEventSwitchLatchedID:
            result = @"SwitchLatched";
            break;

        case MTREventIDTypeClusterSwitchEventInitialPressID:
            result = @"InitialPress";
            break;

        case MTREventIDTypeClusterSwitchEventLongPressID:
            result = @"LongPress";
            break;

        case MTREventIDTypeClusterSwitchEventShortReleaseID:
            result = @"ShortRelease";
            break;

        case MTREventIDTypeClusterSwitchEventLongReleaseID:
            result = @"LongRelease";
            break;

        case MTREventIDTypeClusterSwitchEventMultiPressOngoingID:
            result = @"MultiPressOngoing";
            break;

        case MTREventIDTypeClusterSwitchEventMultiPressCompleteID:
            result = @"MultiPressComplete";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeAdministratorCommissioningID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeOperationalCredentialsID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeGroupKeyManagementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeFixedLabelID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeUserLabelID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeBooleanStateID:

        switch (eventID) {

            // Cluster BooleanState events
        case MTREventIDTypeClusterBooleanStateEventStateChangeID:
            result = @"StateChange";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeICDManagementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeTimerID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeOvenCavityOperationalStateID:

        switch (eventID) {

            // Cluster OvenCavityOperationalState events
        case MTREventIDTypeClusterOvenCavityOperationalStateEventOperationalErrorID:
            result = @"OperationalError";
            break;

        case MTREventIDTypeClusterOvenCavityOperationalStateEventOperationCompletionID:
            result = @"OperationCompletion";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeOvenModeID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeLaundryDryerControlsID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeModeSelectID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeLaundryWasherModeID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeRefrigeratorAndTemperatureControlledCabinetModeID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeLaundryWasherControlsID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeRVCRunModeID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeRVCCleanModeID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeTemperatureControlID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeRefrigeratorAlarmID:

        switch (eventID) {

            // Cluster RefrigeratorAlarm events
        case MTREventIDTypeClusterRefrigeratorAlarmEventNotifyID:
            result = @"Notify";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeDishwasherModeID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeAirQualityID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeSmokeCOAlarmID:

        switch (eventID) {

            // Cluster SmokeCOAlarm events
        case MTREventIDTypeClusterSmokeCOAlarmEventSmokeAlarmID:
            result = @"SmokeAlarm";
            break;

        case MTREventIDTypeClusterSmokeCOAlarmEventCOAlarmID:
            result = @"COAlarm";
            break;

        case MTREventIDTypeClusterSmokeCOAlarmEventLowBatteryID:
            result = @"LowBattery";
            break;

        case MTREventIDTypeClusterSmokeCOAlarmEventHardwareFaultID:
            result = @"HardwareFault";
            break;

        case MTREventIDTypeClusterSmokeCOAlarmEventEndOfServiceID:
            result = @"EndOfService";
            break;

        case MTREventIDTypeClusterSmokeCOAlarmEventSelfTestCompleteID:
            result = @"SelfTestComplete";
            break;

        case MTREventIDTypeClusterSmokeCOAlarmEventAlarmMutedID:
            result = @"AlarmMuted";
            break;

        case MTREventIDTypeClusterSmokeCOAlarmEventMuteEndedID:
            result = @"MuteEnded";
            break;

        case MTREventIDTypeClusterSmokeCOAlarmEventInterconnectSmokeAlarmID:
            result = @"InterconnectSmokeAlarm";
            break;

        case MTREventIDTypeClusterSmokeCOAlarmEventInterconnectCOAlarmID:
            result = @"InterconnectCOAlarm";
            break;

        case MTREventIDTypeClusterSmokeCOAlarmEventAllClearID:
            result = @"AllClear";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeDishwasherAlarmID:

        switch (eventID) {

            // Cluster DishwasherAlarm events
        case MTREventIDTypeClusterDishwasherAlarmEventNotifyID:
            result = @"Notify";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeMicrowaveOvenModeID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeMicrowaveOvenControlID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeOperationalStateID:

        switch (eventID) {

            // Cluster OperationalState events
        case MTREventIDTypeClusterOperationalStateEventOperationalErrorID:
            result = @"OperationalError";
            break;

        case MTREventIDTypeClusterOperationalStateEventOperationCompletionID:
            result = @"OperationCompletion";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeRVCOperationalStateID:

        switch (eventID) {

            // Cluster RVCOperationalState events
        case MTREventIDTypeClusterRVCOperationalStateEventOperationalErrorID:
            result = @"OperationalError";
            break;

        case MTREventIDTypeClusterRVCOperationalStateEventOperationCompletionID:
            result = @"OperationCompletion";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeScenesManagementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeHEPAFilterMonitoringID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeActivatedCarbonFilterMonitoringID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeBooleanStateConfigurationID:

        switch (eventID) {

            // Cluster BooleanStateConfiguration events
        case MTREventIDTypeClusterBooleanStateConfigurationEventAlarmsStateChangedID:
            result = @"AlarmsStateChanged";
            break;

        case MTREventIDTypeClusterBooleanStateConfigurationEventSensorFaultID:
            result = @"SensorFault";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeValveConfigurationAndControlID:

        switch (eventID) {

            // Cluster ValveConfigurationAndControl events
        case MTREventIDTypeClusterValveConfigurationAndControlEventValveStateChangedID:
            result = @"ValveStateChanged";
            break;

        case MTREventIDTypeClusterValveConfigurationAndControlEventValveFaultID:
            result = @"ValveFault";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeElectricalPowerMeasurementID:

        switch (eventID) {

            // Cluster ElectricalPowerMeasurement events
        case MTREventIDTypeClusterElectricalPowerMeasurementEventMeasurementPeriodRangesID:
            result = @"MeasurementPeriodRanges";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeElectricalEnergyMeasurementID:

        switch (eventID) {

            // Cluster ElectricalEnergyMeasurement events
        case MTREventIDTypeClusterElectricalEnergyMeasurementEventCumulativeEnergyMeasuredID:
            result = @"CumulativeEnergyMeasured";
            break;

        case MTREventIDTypeClusterElectricalEnergyMeasurementEventPeriodicEnergyMeasuredID:
            result = @"PeriodicEnergyMeasured";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeWaterHeaterManagementID:

        switch (eventID) {

            // Cluster WaterHeaterManagement events
        case MTREventIDTypeClusterWaterHeaterManagementEventBoostStartedID:
            result = @"BoostStarted";
            break;

        case MTREventIDTypeClusterWaterHeaterManagementEventBoostEndedID:
            result = @"BoostEnded";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeCommodityPriceID:

        switch (eventID) {

            // Cluster CommodityPrice events
        case MTREventIDTypeClusterCommodityPriceEventPriceChangeID:
            result = @"PriceChange";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeMessagesID:

        switch (eventID) {

            // Cluster Messages events
        case MTREventIDTypeClusterMessagesEventMessageQueuedID:
            result = @"MessageQueued";
            break;

        case MTREventIDTypeClusterMessagesEventMessagePresentedID:
            result = @"MessagePresented";
            break;

        case MTREventIDTypeClusterMessagesEventMessageCompleteID:
            result = @"MessageComplete";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeDeviceEnergyManagementID:

        switch (eventID) {

            // Cluster DeviceEnergyManagement events
        case MTREventIDTypeClusterDeviceEnergyManagementEventPowerAdjustStartID:
            result = @"PowerAdjustStart";
            break;

        case MTREventIDTypeClusterDeviceEnergyManagementEventPowerAdjustEndID:
            result = @"PowerAdjustEnd";
            break;

        case MTREventIDTypeClusterDeviceEnergyManagementEventPausedID:
            result = @"Paused";
            break;

        case MTREventIDTypeClusterDeviceEnergyManagementEventResumedID:
            result = @"Resumed";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeEnergyEVSEID:

        switch (eventID) {

            // Cluster EnergyEVSE events
        case MTREventIDTypeClusterEnergyEVSEEventEVConnectedID:
            result = @"EVConnected";
            break;

        case MTREventIDTypeClusterEnergyEVSEEventEVNotDetectedID:
            result = @"EVNotDetected";
            break;

        case MTREventIDTypeClusterEnergyEVSEEventEnergyTransferStartedID:
            result = @"EnergyTransferStarted";
            break;

        case MTREventIDTypeClusterEnergyEVSEEventEnergyTransferStoppedID:
            result = @"EnergyTransferStopped";
            break;

        case MTREventIDTypeClusterEnergyEVSEEventFaultID:
            result = @"Fault";
            break;

        case MTREventIDTypeClusterEnergyEVSEEventRFIDID:
            result = @"RFID";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeEnergyPreferenceID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypePowerTopologyID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeEnergyEVSEModeID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeWaterHeaterModeID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeDeviceEnergyManagementModeID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeElectricalGridConditionsID:

        switch (eventID) {

            // Cluster ElectricalGridConditions events
        case MTREventIDTypeClusterElectricalGridConditionsEventCurrentConditionsChangedID:
            result = @"CurrentConditionsChanged";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeDoorLockID:

        switch (eventID) {

            // Cluster DoorLock events
        case MTREventIDTypeClusterDoorLockEventDoorLockAlarmID:
            result = @"DoorLockAlarm";
            break;

        case MTREventIDTypeClusterDoorLockEventDoorStateChangeID:
            result = @"DoorStateChange";
            break;

        case MTREventIDTypeClusterDoorLockEventLockOperationID:
            result = @"LockOperation";
            break;

        case MTREventIDTypeClusterDoorLockEventLockOperationErrorID:
            result = @"LockOperationError";
            break;

        case MTREventIDTypeClusterDoorLockEventLockUserChangeID:
            result = @"LockUserChange";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeWindowCoveringID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeClosureControlID:

        switch (eventID) {

            // Cluster ClosureControl events
        case MTREventIDTypeClusterClosureControlEventOperationalErrorID:
            result = @"OperationalError";
            break;

        case MTREventIDTypeClusterClosureControlEventMovementCompletedID:
            result = @"MovementCompleted";
            break;

        case MTREventIDTypeClusterClosureControlEventEngageStateChangedID:
            result = @"EngageStateChanged";
            break;

        case MTREventIDTypeClusterClosureControlEventSecureStateChangedID:
            result = @"SecureStateChanged";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeClosureDimensionID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeServiceAreaID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypePumpConfigurationAndControlID:

        switch (eventID) {

            // Cluster PumpConfigurationAndControl events
        case MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageLowID:
            result = @"SupplyVoltageLow";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageHighID:
            result = @"SupplyVoltageHigh";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventPowerMissingPhaseID:
            result = @"PowerMissingPhase";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureLowID:
            result = @"SystemPressureLow";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureHighID:
            result = @"SystemPressureHigh";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventDryRunningID:
            result = @"DryRunning";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventMotorTemperatureHighID:
            result = @"MotorTemperatureHigh";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventPumpMotorFatalFailureID:
            result = @"PumpMotorFatalFailure";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicTemperatureHighID:
            result = @"ElectronicTemperatureHigh";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventPumpBlockedID:
            result = @"PumpBlocked";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventSensorFailureID:
            result = @"SensorFailure";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicNonFatalFailureID:
            result = @"ElectronicNonFatalFailure";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicFatalFailureID:
            result = @"ElectronicFatalFailure";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventGeneralFaultID:
            result = @"GeneralFault";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventLeakageID:
            result = @"Leakage";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventAirDetectionID:
            result = @"AirDetection";
            break;

        case MTREventIDTypeClusterPumpConfigurationAndControlEventTurbineOperationID:
            result = @"TurbineOperation";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeThermostatID:

        switch (eventID) {

            // Cluster Thermostat events
        case MTREventIDTypeClusterThermostatEventSystemModeChangeID:
            result = @"SystemModeChange";
            break;

        case MTREventIDTypeClusterThermostatEventLocalTemperatureChangeID:
            result = @"LocalTemperatureChange";
            break;

        case MTREventIDTypeClusterThermostatEventOccupancyChangeID:
            result = @"OccupancyChange";
            break;

        case MTREventIDTypeClusterThermostatEventSetpointChangeID:
            result = @"SetpointChange";
            break;

        case MTREventIDTypeClusterThermostatEventRunningStateChangeID:
            result = @"RunningStateChange";
            break;

        case MTREventIDTypeClusterThermostatEventRunningModeChangeID:
            result = @"RunningModeChange";
            break;

        case MTREventIDTypeClusterThermostatEventActiveScheduleChangeID:
            result = @"ActiveScheduleChange";
            break;

        case MTREventIDTypeClusterThermostatEventActivePresetChangeID:
            result = @"ActivePresetChange";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeFanControlID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeThermostatUserInterfaceConfigurationID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeColorControlID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeBallastConfigurationID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeIlluminanceMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeTemperatureMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypePressureMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeFlowMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeRelativeHumidityMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeOccupancySensingID:

        switch (eventID) {

            // Cluster OccupancySensing events
        case MTREventIDTypeClusterOccupancySensingEventOccupancyChangedID:
            result = @"OccupancyChanged";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeCarbonMonoxideConcentrationMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeCarbonDioxideConcentrationMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeNitrogenDioxideConcentrationMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeOzoneConcentrationMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypePM25ConcentrationMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeFormaldehydeConcentrationMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypePM1ConcentrationMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypePM10ConcentrationMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeTotalVolatileOrganicCompoundsConcentrationMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeRadonConcentrationMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeSoilMeasurementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeWiFiNetworkManagementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeThreadBorderRouterManagementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeThreadNetworkDirectoryID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeWakeOnLANID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeChannelID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeTargetNavigatorID:

        switch (eventID) {

            // Cluster TargetNavigator events
        case MTREventIDTypeClusterTargetNavigatorEventTargetUpdatedID:
            result = @"TargetUpdated";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeMediaPlaybackID:

        switch (eventID) {

            // Cluster MediaPlayback events
        case MTREventIDTypeClusterMediaPlaybackEventStateChangedID:
            result = @"StateChanged";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeMediaInputID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeLowPowerID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeKeypadInputID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeContentLauncherID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeAudioOutputID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeApplicationLauncherID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeApplicationBasicID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeAccountLoginID:

        switch (eventID) {

            // Cluster AccountLogin events
        case MTREventIDTypeClusterAccountLoginEventLoggedOutID:
            result = @"LoggedOut";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeContentControlID:

        switch (eventID) {

            // Cluster ContentControl events
        case MTREventIDTypeClusterContentControlEventRemainingScreenTimeExpiredID:
            result = @"RemainingScreenTimeExpired";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeContentAppObserverID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeZoneManagementID:

        switch (eventID) {

            // Cluster ZoneManagement events
        case MTREventIDTypeClusterZoneManagementEventZoneTriggeredID:
            result = @"ZoneTriggered";
            break;

        case MTREventIDTypeClusterZoneManagementEventZoneStoppedID:
            result = @"ZoneStopped";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeCameraAVStreamManagementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeCameraAVSettingsUserLevelManagementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeWebRTCTransportProviderID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeWebRTCTransportRequestorID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypePushAVStreamTransportID:

        switch (eventID) {

            // Cluster PushAVStreamTransport events
        case MTREventIDTypeClusterPushAVStreamTransportEventPushTransportBeginID:
            result = @"PushTransportBegin";
            break;

        case MTREventIDTypeClusterPushAVStreamTransportEventPushTransportEndID:
            result = @"PushTransportEnd";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeChimeID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeCommodityTariffID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeEcosystemInformationID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeCommissionerControlID:

        switch (eventID) {

            // Cluster CommissionerControl events
        case MTREventIDTypeClusterCommissionerControlEventCommissioningRequestResultID:
            result = @"CommissioningRequestResult";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeJointFabricDatastoreID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeJointFabricAdministratorID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeTLSCertificateManagementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeTLSClientManagementID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeMeterIdentificationID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeCommodityMeteringID:

        switch (eventID) {

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeUnitTestingID:

        switch (eventID) {

            // Cluster UnitTesting events
        case MTREventIDTypeClusterUnitTestingEventTestEventID:
            result = @"TestEvent";
            break;

        case MTREventIDTypeClusterUnitTestingEventTestFabricScopedEventID:
            result = @"TestFabricScopedEvent";
            break;

        case MTREventIDTypeClusterUnitTestingEventTestDifferentVendorMeiEventID:
            result = @"TestDifferentVendorMeiEvent";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    case MTRClusterIDTypeSampleMEIID:

        switch (eventID) {

            // Cluster SampleMEI events
        case MTREventIDTypeClusterSampleMEIEventPingCountEventID:
            result = @"PingCountEvent";
            break;

        default:
            result = [NSString stringWithFormat:@"<Unknown eventID %u>", eventID];
            break;
        }
        break;

    default:
        result = [NSString stringWithFormat:@"<Unknown clusterID %u>", clusterID];
        break;
    }

    return result;
}

// @end
