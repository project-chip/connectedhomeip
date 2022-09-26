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

#import <Foundation/Foundation.h>
#import <stdint.h>

#pragma mark - Clusters IDs

typedef NS_ENUM(uint32_t, MTRClusterIDType) {
    MTRClusterIDTypeIdentifyID = 0x00000003,
    MTRClusterIDTypeGroupsID = 0x00000004,
    MTRClusterIDTypeScenesID = 0x00000005,
    MTRClusterIDTypeOnOffID = 0x00000006,
    MTRClusterIDTypeOnOffSwitchConfigurationID = 0x00000007,
    MTRClusterIDTypeLevelControlID = 0x00000008,
    MTRClusterIDTypeBinaryInputBasicID = 0x0000000F,
    MTRClusterIDTypePulseWidthModulationID = 0x0000001C,
    MTRClusterIDTypeDescriptorID = 0x0000001D,
    MTRClusterIDTypeBindingID = 0x0000001E,
    MTRClusterIDTypeAccessControlID = 0x0000001F,
    MTRClusterIDTypeActionsID = 0x00000025,
    MTRClusterIDTypeBasicID = 0x00000028,
    MTRClusterIDTypeOTASoftwareUpdateProviderID = 0x00000029,
    MTRClusterIDTypeOTASoftwareUpdateRequestorID = 0x0000002A,
    MTRClusterIDTypeLocalizationConfigurationID = 0x0000002B,
    MTRClusterIDTypeTimeFormatLocalizationID = 0x0000002C,
    MTRClusterIDTypeUnitLocalizationID = 0x0000002D,
    MTRClusterIDTypePowerSourceConfigurationID = 0x0000002E,
    MTRClusterIDTypePowerSourceID = 0x0000002F,
    MTRClusterIDTypeGeneralCommissioningID = 0x00000030,
    MTRClusterIDTypeNetworkCommissioningID = 0x00000031,
    MTRClusterIDTypeDiagnosticLogsID = 0x00000032,
    MTRClusterIDTypeGeneralDiagnosticsID = 0x00000033,
    MTRClusterIDTypeSoftwareDiagnosticsID = 0x00000034,
    MTRClusterIDTypeThreadNetworkDiagnosticsID = 0x00000035,
    MTRClusterIDTypeWiFiNetworkDiagnosticsID = 0x00000036,
    MTRClusterIDTypeEthernetNetworkDiagnosticsID = 0x00000037,
    MTRClusterIDTypeTimeSynchronizationID = 0x00000038,
    MTRClusterIDTypeBridgedDeviceBasicID = 0x00000039,
    MTRClusterIDTypeSwitchID = 0x0000003B,
    MTRClusterIDTypeAdministratorCommissioningID = 0x0000003C,
    MTRClusterIDTypeOperationalCredentialsID = 0x0000003E,
    MTRClusterIDTypeGroupKeyManagementID = 0x0000003F,
    MTRClusterIDTypeFixedLabelID = 0x00000040,
    MTRClusterIDTypeUserLabelID = 0x00000041,
    MTRClusterIDTypeProxyConfigurationID = 0x00000042,
    MTRClusterIDTypeProxyDiscoveryID = 0x00000043,
    MTRClusterIDTypeProxyValidID = 0x00000044,
    MTRClusterIDTypeBooleanStateID = 0x00000045,
    MTRClusterIDTypeModeSelectID = 0x00000050,
    MTRClusterIDTypeDoorLockID = 0x00000101,
    MTRClusterIDTypeWindowCoveringID = 0x00000102,
    MTRClusterIDTypeBarrierControlID = 0x00000103,
    MTRClusterIDTypePumpConfigurationAndControlID = 0x00000200,
    MTRClusterIDTypeThermostatID = 0x00000201,
    MTRClusterIDTypeFanControlID = 0x00000202,
    MTRClusterIDTypeThermostatUserInterfaceConfigurationID = 0x00000204,
    MTRClusterIDTypeColorControlID = 0x00000300,
    MTRClusterIDTypeBallastConfigurationID = 0x00000301,
    MTRClusterIDTypeIlluminanceMeasurementID = 0x00000400,
    MTRClusterIDTypeTemperatureMeasurementID = 0x00000402,
    MTRClusterIDTypePressureMeasurementID = 0x00000403,
    MTRClusterIDTypeFlowMeasurementID = 0x00000404,
    MTRClusterIDTypeRelativeHumidityMeasurementID = 0x00000405,
    MTRClusterIDTypeOccupancySensingID = 0x00000406,
    MTRClusterIDTypeWakeOnLANID = 0x00000503,
    MTRClusterIDTypeChannelID = 0x00000504,
    MTRClusterIDTypeTargetNavigatorID = 0x00000505,
    MTRClusterIDTypeMediaPlaybackID = 0x00000506,
    MTRClusterIDTypeMediaInputID = 0x00000507,
    MTRClusterIDTypeLowPowerID = 0x00000508,
    MTRClusterIDTypeKeypadInputID = 0x00000509,
    MTRClusterIDTypeContentLauncherID = 0x0000050A,
    MTRClusterIDTypeAudioOutputID = 0x0000050B,
    MTRClusterIDTypeApplicationLauncherID = 0x0000050C,
    MTRClusterIDTypeApplicationBasicID = 0x0000050D,
    MTRClusterIDTypeAccountLoginID = 0x0000050E,
    MTRClusterIDTypeElectricalMeasurementID = 0x00000B04,
    MTRClusterIDTypeTestClusterID = 0xFFF1FC05,
    MTRClusterIDTypeFaultInjectionID = 0xFFF1FC06,
};

#pragma mark - Attributes IDs

typedef NS_ENUM(uint32_t, MTRAttributeIDType) {
    // Global attributes
    MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID = 0x0000FFF8,
    MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID = 0x0000FFF9,
    MTRAttributeIDTypeGlobalAttributeAttributeListID = 0x0000FFFB,
    MTRAttributeIDTypeGlobalAttributeFeatureMapID = 0x0000FFFC,
    MTRAttributeIDTypeGlobalAttributeClusterRevisionID = 0x0000FFFD,

    // Cluster Identify attributes
    MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTimeID = 0x00000000,
    MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTypeID = 0x00000001,
    MTRAttributeIDTypeClusterIdentifyAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterIdentifyAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterIdentifyAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterIdentifyAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterIdentifyAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Groups attributes
    MTRAttributeIDTypeClusterGroupsAttributeNameSupportID = 0x00000000,
    MTRAttributeIDTypeClusterGroupsAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGroupsAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGroupsAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGroupsAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGroupsAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Scenes attributes
    MTRAttributeIDTypeClusterScenesAttributeSceneCountID = 0x00000000,
    MTRAttributeIDTypeClusterScenesAttributeCurrentSceneID = 0x00000001,
    MTRAttributeIDTypeClusterScenesAttributeCurrentGroupID = 0x00000002,
    MTRAttributeIDTypeClusterScenesAttributeSceneValidID = 0x00000003,
    MTRAttributeIDTypeClusterScenesAttributeNameSupportID = 0x00000004,
    MTRAttributeIDTypeClusterScenesAttributeLastConfiguredByID = 0x00000005,
    MTRAttributeIDTypeClusterScenesAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterScenesAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterScenesAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterScenesAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterScenesAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OnOff attributes
    MTRAttributeIDTypeClusterOnOffAttributeOnOffID = 0x00000000,
    MTRAttributeIDTypeClusterOnOffAttributeGlobalSceneControlID = 0x00004000,
    MTRAttributeIDTypeClusterOnOffAttributeOnTimeID = 0x00004001,
    MTRAttributeIDTypeClusterOnOffAttributeOffWaitTimeID = 0x00004002,
    MTRAttributeIDTypeClusterOnOffAttributeStartUpOnOffID = 0x00004003,
    MTRAttributeIDTypeClusterOnOffAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOnOffAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOnOffAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOnOffAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOnOffAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OnOffSwitchConfiguration attributes
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchTypeID = 0x00000000,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchActionsID = 0x00000010,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeClusterRevisionID
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster LevelControl attributes
    MTRAttributeIDTypeClusterLevelControlAttributeCurrentLevelID = 0x00000000,
    MTRAttributeIDTypeClusterLevelControlAttributeRemainingTimeID = 0x00000001,
    MTRAttributeIDTypeClusterLevelControlAttributeMinLevelID = 0x00000002,
    MTRAttributeIDTypeClusterLevelControlAttributeMaxLevelID = 0x00000003,
    MTRAttributeIDTypeClusterLevelControlAttributeCurrentFrequencyID = 0x00000004,
    MTRAttributeIDTypeClusterLevelControlAttributeMinFrequencyID = 0x00000005,
    MTRAttributeIDTypeClusterLevelControlAttributeMaxFrequencyID = 0x00000006,
    MTRAttributeIDTypeClusterLevelControlAttributeOptionsID = 0x0000000F,
    MTRAttributeIDTypeClusterLevelControlAttributeOnOffTransitionTimeID = 0x00000010,
    MTRAttributeIDTypeClusterLevelControlAttributeOnLevelID = 0x00000011,
    MTRAttributeIDTypeClusterLevelControlAttributeOnTransitionTimeID = 0x00000012,
    MTRAttributeIDTypeClusterLevelControlAttributeOffTransitionTimeID = 0x00000013,
    MTRAttributeIDTypeClusterLevelControlAttributeDefaultMoveRateID = 0x00000014,
    MTRAttributeIDTypeClusterLevelControlAttributeStartUpCurrentLevelID = 0x00004000,
    MTRAttributeIDTypeClusterLevelControlAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterLevelControlAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterLevelControlAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterLevelControlAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterLevelControlAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BinaryInputBasic attributes
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeActiveTextID = 0x00000004,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeDescriptionID = 0x0000001C,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeInactiveTextID = 0x0000002E,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeOutOfServiceID = 0x00000051,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributePolarityID = 0x00000054,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributePresentValueID = 0x00000055,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeReliabilityID = 0x00000067,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeStatusFlagsID = 0x0000006F,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeApplicationTypeID = 0x00000100,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PulseWidthModulation attributes
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Descriptor attributes
    MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID = 0x00000000,
    MTRAttributeIDTypeClusterDescriptorAttributeServerListID = 0x00000001,
    MTRAttributeIDTypeClusterDescriptorAttributeClientListID = 0x00000002,
    MTRAttributeIDTypeClusterDescriptorAttributePartsListID = 0x00000003,
    MTRAttributeIDTypeClusterDescriptorAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDescriptorAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDescriptorAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDescriptorAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDescriptorAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Binding attributes
    MTRAttributeIDTypeClusterBindingAttributeBindingID = 0x00000000,
    MTRAttributeIDTypeClusterBindingAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBindingAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBindingAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBindingAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBindingAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AccessControl attributes
    MTRAttributeIDTypeClusterAccessControlAttributeACLID = 0x00000000,
    MTRAttributeIDTypeClusterAccessControlAttributeExtensionID = 0x00000001,
    MTRAttributeIDTypeClusterAccessControlAttributeSubjectsPerAccessControlEntryID = 0x00000002,
    MTRAttributeIDTypeClusterAccessControlAttributeTargetsPerAccessControlEntryID = 0x00000003,
    MTRAttributeIDTypeClusterAccessControlAttributeAccessControlEntriesPerFabricID = 0x00000004,
    MTRAttributeIDTypeClusterAccessControlAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAccessControlAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAccessControlAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAccessControlAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAccessControlAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Actions attributes
    MTRAttributeIDTypeClusterActionsAttributeActionListID = 0x00000000,
    MTRAttributeIDTypeClusterActionsAttributeEndpointListsID = 0x00000001,
    MTRAttributeIDTypeClusterActionsAttributeSetupURLID = 0x00000002,
    MTRAttributeIDTypeClusterActionsAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterActionsAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterActionsAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterActionsAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterActionsAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Basic attributes
    MTRAttributeIDTypeClusterBasicAttributeDataModelRevisionID = 0x00000000,
    MTRAttributeIDTypeClusterBasicAttributeVendorNameID = 0x00000001,
    MTRAttributeIDTypeClusterBasicAttributeVendorIDID = 0x00000002,
    MTRAttributeIDTypeClusterBasicAttributeProductNameID = 0x00000003,
    MTRAttributeIDTypeClusterBasicAttributeProductIDID = 0x00000004,
    MTRAttributeIDTypeClusterBasicAttributeNodeLabelID = 0x00000005,
    MTRAttributeIDTypeClusterBasicAttributeLocationID = 0x00000006,
    MTRAttributeIDTypeClusterBasicAttributeHardwareVersionID = 0x00000007,
    MTRAttributeIDTypeClusterBasicAttributeHardwareVersionStringID = 0x00000008,
    MTRAttributeIDTypeClusterBasicAttributeSoftwareVersionID = 0x00000009,
    MTRAttributeIDTypeClusterBasicAttributeSoftwareVersionStringID = 0x0000000A,
    MTRAttributeIDTypeClusterBasicAttributeManufacturingDateID = 0x0000000B,
    MTRAttributeIDTypeClusterBasicAttributePartNumberID = 0x0000000C,
    MTRAttributeIDTypeClusterBasicAttributeProductURLID = 0x0000000D,
    MTRAttributeIDTypeClusterBasicAttributeProductLabelID = 0x0000000E,
    MTRAttributeIDTypeClusterBasicAttributeSerialNumberID = 0x0000000F,
    MTRAttributeIDTypeClusterBasicAttributeLocalConfigDisabledID = 0x00000010,
    MTRAttributeIDTypeClusterBasicAttributeReachableID = 0x00000011,
    MTRAttributeIDTypeClusterBasicAttributeUniqueIDID = 0x00000012,
    MTRAttributeIDTypeClusterBasicAttributeCapabilityMinimaID = 0x00000013,
    MTRAttributeIDTypeClusterBasicAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBasicAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBasicAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBasicAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBasicAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OTASoftwareUpdateProvider attributes
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeClusterRevisionID
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OTASoftwareUpdateRequestor attributes
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeDefaultOtaProvidersID = 0x00000000,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdatePossibleID = 0x00000001,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateID = 0x00000002,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateProgressID = 0x00000003,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeClusterRevisionID
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster LocalizationConfiguration attributes
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeActiveLocaleID = 0x00000000,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeSupportedLocalesID = 0x00000001,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeClusterRevisionID
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TimeFormatLocalization attributes
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeHourFormatID = 0x00000000,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeActiveCalendarTypeID = 0x00000001,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeSupportedCalendarTypesID = 0x00000002,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster UnitLocalization attributes
    MTRAttributeIDTypeClusterUnitLocalizationAttributeTemperatureUnitID = 0x00000000,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PowerSourceConfiguration attributes
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeSourcesID = 0x00000000,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeClusterRevisionID
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PowerSource attributes
    MTRAttributeIDTypeClusterPowerSourceAttributeStatusID = 0x00000000,
    MTRAttributeIDTypeClusterPowerSourceAttributeOrderID = 0x00000001,
    MTRAttributeIDTypeClusterPowerSourceAttributeDescriptionID = 0x00000002,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputVoltageID = 0x00000003,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputFrequencyID = 0x00000004,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredCurrentTypeID = 0x00000005,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedCurrentID = 0x00000006,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredNominalVoltageID = 0x00000007,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredMaximumCurrentID = 0x00000008,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredPresentID = 0x00000009,
    MTRAttributeIDTypeClusterPowerSourceAttributeActiveWiredFaultsID = 0x0000000A,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatVoltageID = 0x0000000B,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatPercentRemainingID = 0x0000000C,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeRemainingID = 0x0000000D,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeLevelID = 0x0000000E,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementNeededID = 0x0000000F,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatReplaceabilityID = 0x00000010,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatPresentID = 0x00000011,
    MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatFaultsID = 0x00000012,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementDescriptionID = 0x00000013,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatCommonDesignationID = 0x00000014,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatANSIDesignationID = 0x00000015,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatIECDesignationID = 0x00000016,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatApprovedChemistryID = 0x00000017,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatCapacityID = 0x00000018,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatQuantityID = 0x00000019,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeStateID = 0x0000001A,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeToFullChargeID = 0x0000001B,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatFunctionalWhileChargingID = 0x0000001C,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatChargingCurrentID = 0x0000001D,
    MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatChargeFaultsID = 0x0000001E,
    MTRAttributeIDTypeClusterPowerSourceAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPowerSourceAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPowerSourceAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster GeneralCommissioning attributes
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeBreadcrumbID = 0x00000000,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeBasicCommissioningInfoID = 0x00000001,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeRegulatoryConfigID = 0x00000002,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeLocationCapabilityID = 0x00000003,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeSupportsConcurrentConnectionID = 0x00000004,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster NetworkCommissioning attributes
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeMaxNetworksID = 0x00000000,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeNetworksID = 0x00000001,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeScanMaxTimeSecondsID = 0x00000002,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeConnectMaxTimeSecondsID = 0x00000003,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeInterfaceEnabledID = 0x00000004,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkingStatusID = 0x00000005,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkIDID = 0x00000006,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastConnectErrorValueID = 0x00000007,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster DiagnosticLogs attributes
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster GeneralDiagnostics attributes
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeNetworkInterfacesID = 0x00000000,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeRebootCountID = 0x00000001,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeUpTimeID = 0x00000002,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTotalOperationalHoursID = 0x00000003,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeBootReasonsID = 0x00000004,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveHardwareFaultsID = 0x00000005,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveRadioFaultsID = 0x00000006,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveNetworkFaultsID = 0x00000007,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTestEventTriggersEnabledID = 0x00000008,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster SoftwareDiagnostics attributes
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeThreadMetricsID = 0x00000000,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapFreeID = 0x00000001,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapUsedID = 0x00000002,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapHighWatermarkID = 0x00000003,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ThreadNetworkDiagnostics attributes
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelID = 0x00000000,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRoutingRoleID = 0x00000001,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNetworkNameID = 0x00000002,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePanIdID = 0x00000003,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeExtendedPanIdID = 0x00000004,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeMeshLocalPrefixID = 0x00000005,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOverrunCountID = 0x00000006,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNeighborTableListID = 0x00000007,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouteTableListID = 0x00000008,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdID = 0x00000009,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeWeightingID = 0x0000000A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDataVersionID = 0x0000000B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeStableDataVersionID = 0x0000000C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRouterIdID = 0x0000000D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDetachedRoleCountID = 0x0000000E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChildRoleCountID = 0x0000000F,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouterRoleCountID = 0x00000010,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRoleCountID = 0x00000011,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttachAttemptCountID = 0x00000012,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdChangeCountID = 0x00000013,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeBetterPartitionAttachAttemptCountID = 0x00000014,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeParentChangeCountID = 0x00000015,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxTotalCountID = 0x00000016,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxUnicastCountID = 0x00000017,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBroadcastCountID = 0x00000018,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckRequestedCountID = 0x00000019,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckedCountID = 0x0000001A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxNoAckRequestedCountID = 0x0000001B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataCountID = 0x0000001C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataPollCountID = 0x0000001D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconCountID = 0x0000001E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconRequestCountID = 0x0000001F,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxOtherCountID = 0x00000020,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxRetryCountID = 0x00000021,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDirectMaxRetryExpiryCountID = 0x00000022,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxIndirectMaxRetryExpiryCountID = 0x00000023,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrCcaCountID = 0x00000024,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrAbortCountID = 0x00000025,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrBusyChannelCountID = 0x00000026,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxTotalCountID = 0x00000027,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxUnicastCountID = 0x00000028,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBroadcastCountID = 0x00000029,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataCountID = 0x0000002A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataPollCountID = 0x0000002B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconCountID = 0x0000002C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconRequestCountID = 0x0000002D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxOtherCountID = 0x0000002E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxAddressFilteredCountID = 0x0000002F,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDestAddrFilteredCountID = 0x00000030,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDuplicatedCountID = 0x00000031,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrNoFrameCountID = 0x00000032,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrUnknownNeighborCountID = 0x00000033,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrInvalidSrcAddrCountID = 0x00000034,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrSecCountID = 0x00000035,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrFcsCountID = 0x00000036,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrOtherCountID = 0x00000037,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveTimestampID = 0x00000038,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePendingTimestampID = 0x00000039,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDelayID = 0x0000003A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeSecurityPolicyID = 0x0000003B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelPage0MaskID = 0x0000003C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOperationalDatasetComponentsID = 0x0000003D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveNetworkFaultsListID = 0x0000003E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeClusterRevisionID
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster WiFiNetworkDiagnostics attributes
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBssidID = 0x00000000,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeSecurityTypeID = 0x00000001,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeWiFiVersionID = 0x00000002,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeChannelNumberID = 0x00000003,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeRssiID = 0x00000004,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconLostCountID = 0x00000005,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconRxCountID = 0x00000006,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastRxCountID = 0x00000007,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastTxCountID = 0x00000008,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastRxCountID = 0x00000009,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastTxCountID = 0x0000000A,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeCurrentMaxRateID = 0x0000000B,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeOverrunCountID = 0x0000000C,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster EthernetNetworkDiagnostics attributes
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePHYRateID = 0x00000000,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFullDuplexID = 0x00000001,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketRxCountID = 0x00000002,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketTxCountID = 0x00000003,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTxErrCountID = 0x00000004,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCollisionCountID = 0x00000005,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeOverrunCountID = 0x00000006,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCarrierDetectID = 0x00000007,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTimeSinceResetID = 0x00000008,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeClusterRevisionID
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TimeSynchronization attributes
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeUTCTimeID = 0x00000000,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeGranularityID = 0x00000001,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeSourceID = 0x00000002,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTrustedTimeNodeIdID = 0x00000003,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeDefaultNtpID = 0x00000004,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneID = 0x00000005,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeDstOffsetID = 0x00000006,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeLocalTimeID = 0x00000007,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneDatabaseID = 0x00000008,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeNtpServerPortID = 0x00000009,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BridgedDeviceBasic attributes
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeVendorNameID = 0x00000001,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeVendorIDID = 0x00000002,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeProductNameID = 0x00000003,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeNodeLabelID = 0x00000005,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeHardwareVersionID = 0x00000007,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeHardwareVersionStringID = 0x00000008,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeSoftwareVersionID = 0x00000009,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeSoftwareVersionStringID = 0x0000000A,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeManufacturingDateID = 0x0000000B,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributePartNumberID = 0x0000000C,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeProductURLID = 0x0000000D,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeProductLabelID = 0x0000000E,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeSerialNumberID = 0x0000000F,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeReachableID = 0x00000011,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeUniqueIDID = 0x00000012,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Switch attributes
    MTRAttributeIDTypeClusterSwitchAttributeNumberOfPositionsID = 0x00000000,
    MTRAttributeIDTypeClusterSwitchAttributeCurrentPositionID = 0x00000001,
    MTRAttributeIDTypeClusterSwitchAttributeMultiPressMaxID = 0x00000002,
    MTRAttributeIDTypeClusterSwitchAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterSwitchAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterSwitchAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterSwitchAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterSwitchAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AdministratorCommissioning attributes
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeWindowStatusID = 0x00000000,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminFabricIndexID = 0x00000001,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminVendorIdID = 0x00000002,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeClusterRevisionID
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OperationalCredentials attributes
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeNOCsID = 0x00000000,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeFabricsID = 0x00000001,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeSupportedFabricsID = 0x00000002,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeCommissionedFabricsID = 0x00000003,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeTrustedRootCertificatesID = 0x00000004,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeCurrentFabricIndexID = 0x00000005,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster GroupKeyManagement attributes
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupKeyMapID = 0x00000000,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupTableID = 0x00000001,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupsPerFabricID = 0x00000002,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupKeysPerFabricID = 0x00000003,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FixedLabel attributes
    MTRAttributeIDTypeClusterFixedLabelAttributeLabelListID = 0x00000000,
    MTRAttributeIDTypeClusterFixedLabelAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFixedLabelAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFixedLabelAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFixedLabelAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFixedLabelAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster UserLabel attributes
    MTRAttributeIDTypeClusterUserLabelAttributeLabelListID = 0x00000000,
    MTRAttributeIDTypeClusterUserLabelAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterUserLabelAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterUserLabelAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterUserLabelAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterUserLabelAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ProxyConfiguration attributes
    MTRAttributeIDTypeClusterProxyConfigurationAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterProxyConfigurationAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterProxyConfigurationAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterProxyConfigurationAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterProxyConfigurationAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ProxyDiscovery attributes
    MTRAttributeIDTypeClusterProxyDiscoveryAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterProxyDiscoveryAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterProxyDiscoveryAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterProxyDiscoveryAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterProxyDiscoveryAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ProxyValid attributes
    MTRAttributeIDTypeClusterProxyValidAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterProxyValidAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterProxyValidAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterProxyValidAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterProxyValidAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BooleanState attributes
    MTRAttributeIDTypeClusterBooleanStateAttributeStateValueID = 0x00000000,
    MTRAttributeIDTypeClusterBooleanStateAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBooleanStateAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBooleanStateAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBooleanStateAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBooleanStateAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ModeSelect attributes
    MTRAttributeIDTypeClusterModeSelectAttributeDescriptionID = 0x00000000,
    MTRAttributeIDTypeClusterModeSelectAttributeStandardNamespaceID = 0x00000001,
    MTRAttributeIDTypeClusterModeSelectAttributeSupportedModesID = 0x00000002,
    MTRAttributeIDTypeClusterModeSelectAttributeCurrentModeID = 0x00000003,
    MTRAttributeIDTypeClusterModeSelectAttributeStartUpModeID = 0x00000004,
    MTRAttributeIDTypeClusterModeSelectAttributeOnModeID = 0x00000005,
    MTRAttributeIDTypeClusterModeSelectAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterModeSelectAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterModeSelectAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterModeSelectAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterModeSelectAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster DoorLock attributes
    MTRAttributeIDTypeClusterDoorLockAttributeLockStateID = 0x00000000,
    MTRAttributeIDTypeClusterDoorLockAttributeLockTypeID = 0x00000001,
    MTRAttributeIDTypeClusterDoorLockAttributeActuatorEnabledID = 0x00000002,
    MTRAttributeIDTypeClusterDoorLockAttributeDoorStateID = 0x00000003,
    MTRAttributeIDTypeClusterDoorLockAttributeDoorOpenEventsID = 0x00000004,
    MTRAttributeIDTypeClusterDoorLockAttributeDoorClosedEventsID = 0x00000005,
    MTRAttributeIDTypeClusterDoorLockAttributeOpenPeriodID = 0x00000006,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfTotalUsersSupportedID = 0x00000011,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfPINUsersSupportedID = 0x00000012,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfRFIDUsersSupportedID = 0x00000013,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfWeekDaySchedulesSupportedPerUserID = 0x00000014,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfYearDaySchedulesSupportedPerUserID = 0x00000015,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfHolidaySchedulesSupportedID = 0x00000016,
    MTRAttributeIDTypeClusterDoorLockAttributeMaxPINCodeLengthID = 0x00000017,
    MTRAttributeIDTypeClusterDoorLockAttributeMinPINCodeLengthID = 0x00000018,
    MTRAttributeIDTypeClusterDoorLockAttributeMaxRFIDCodeLengthID = 0x00000019,
    MTRAttributeIDTypeClusterDoorLockAttributeMinRFIDCodeLengthID = 0x0000001A,
    MTRAttributeIDTypeClusterDoorLockAttributeCredentialRulesSupportID = 0x0000001B,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfCredentialsSupportedPerUserID = 0x0000001C,
    MTRAttributeIDTypeClusterDoorLockAttributeLanguageID = 0x00000021,
    MTRAttributeIDTypeClusterDoorLockAttributeLEDSettingsID = 0x00000022,
    MTRAttributeIDTypeClusterDoorLockAttributeAutoRelockTimeID = 0x00000023,
    MTRAttributeIDTypeClusterDoorLockAttributeSoundVolumeID = 0x00000024,
    MTRAttributeIDTypeClusterDoorLockAttributeOperatingModeID = 0x00000025,
    MTRAttributeIDTypeClusterDoorLockAttributeSupportedOperatingModesID = 0x00000026,
    MTRAttributeIDTypeClusterDoorLockAttributeDefaultConfigurationRegisterID = 0x00000027,
    MTRAttributeIDTypeClusterDoorLockAttributeEnableLocalProgrammingID = 0x00000028,
    MTRAttributeIDTypeClusterDoorLockAttributeEnableOneTouchLockingID = 0x00000029,
    MTRAttributeIDTypeClusterDoorLockAttributeEnableInsideStatusLEDID = 0x0000002A,
    MTRAttributeIDTypeClusterDoorLockAttributeEnablePrivacyModeButtonID = 0x0000002B,
    MTRAttributeIDTypeClusterDoorLockAttributeLocalProgrammingFeaturesID = 0x0000002C,
    MTRAttributeIDTypeClusterDoorLockAttributeWrongCodeEntryLimitID = 0x00000030,
    MTRAttributeIDTypeClusterDoorLockAttributeUserCodeTemporaryDisableTimeID = 0x00000031,
    MTRAttributeIDTypeClusterDoorLockAttributeSendPINOverTheAirID = 0x00000032,
    MTRAttributeIDTypeClusterDoorLockAttributeRequirePINforRemoteOperationID = 0x00000033,
    MTRAttributeIDTypeClusterDoorLockAttributeExpiringUserTimeoutID = 0x00000035,
    MTRAttributeIDTypeClusterDoorLockAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDoorLockAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDoorLockAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDoorLockAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDoorLockAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster WindowCovering attributes
    MTRAttributeIDTypeClusterWindowCoveringAttributeTypeID = 0x00000000,
    MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitLiftID = 0x00000001,
    MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitTiltID = 0x00000002,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftID = 0x00000003,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltID = 0x00000004,
    MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsLiftID = 0x00000005,
    MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsTiltID = 0x00000006,
    MTRAttributeIDTypeClusterWindowCoveringAttributeConfigStatusID = 0x00000007,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercentageID = 0x00000008,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercentageID = 0x00000009,
    MTRAttributeIDTypeClusterWindowCoveringAttributeOperationalStatusID = 0x0000000A,
    MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionLiftPercent100thsID = 0x0000000B,
    MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionTiltPercent100thsID = 0x0000000C,
    MTRAttributeIDTypeClusterWindowCoveringAttributeEndProductTypeID = 0x0000000D,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercent100thsID = 0x0000000E,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercent100thsID = 0x0000000F,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitLiftID = 0x00000010,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitLiftID = 0x00000011,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitTiltID = 0x00000012,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitTiltID = 0x00000013,
    MTRAttributeIDTypeClusterWindowCoveringAttributeModeID = 0x00000017,
    MTRAttributeIDTypeClusterWindowCoveringAttributeSafetyStatusID = 0x0000001A,
    MTRAttributeIDTypeClusterWindowCoveringAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BarrierControl attributes
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierMovingStateID = 0x00000001,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierSafetyStatusID = 0x00000002,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCapabilitiesID = 0x00000003,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenEventsID = 0x00000004,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCloseEventsID = 0x00000005,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandOpenEventsID = 0x00000006,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandCloseEventsID = 0x00000007,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenPeriodID = 0x00000008,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierClosePeriodID = 0x00000009,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierPositionID = 0x0000000A,
    MTRAttributeIDTypeClusterBarrierControlAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBarrierControlAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBarrierControlAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBarrierControlAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBarrierControlAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PumpConfigurationAndControl attributes
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxPressureID = 0x00000000,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxSpeedID = 0x00000001,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxFlowID = 0x00000002,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstPressureID = 0x00000003,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstPressureID = 0x00000004,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinCompPressureID = 0x00000005,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxCompPressureID = 0x00000006,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstSpeedID = 0x00000007,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstSpeedID = 0x00000008,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstFlowID = 0x00000009,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstFlowID = 0x0000000A,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstTempID = 0x0000000B,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstTempID = 0x0000000C,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePumpStatusID = 0x00000010,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveOperationModeID = 0x00000011,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveControlModeID = 0x00000012,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeCapacityID = 0x00000013,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeSpeedID = 0x00000014,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeRunningHoursID = 0x00000015,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePowerID = 0x00000016,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeEnergyConsumedID = 0x00000017,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeOperationModeID = 0x00000020,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeControlModeID = 0x00000021,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeClusterRevisionID
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Thermostat attributes
    MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureID = 0x00000000,
    MTRAttributeIDTypeClusterThermostatAttributeOutdoorTemperatureID = 0x00000001,
    MTRAttributeIDTypeClusterThermostatAttributeOccupancyID = 0x00000002,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMinHeatSetpointLimitID = 0x00000003,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMaxHeatSetpointLimitID = 0x00000004,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMinCoolSetpointLimitID = 0x00000005,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMaxCoolSetpointLimitID = 0x00000006,
    MTRAttributeIDTypeClusterThermostatAttributePICoolingDemandID = 0x00000007,
    MTRAttributeIDTypeClusterThermostatAttributePIHeatingDemandID = 0x00000008,
    MTRAttributeIDTypeClusterThermostatAttributeHVACSystemTypeConfigurationID = 0x00000009,
    MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureCalibrationID = 0x00000010,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedCoolingSetpointID = 0x00000011,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedHeatingSetpointID = 0x00000012,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedCoolingSetpointID = 0x00000013,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedHeatingSetpointID = 0x00000014,
    MTRAttributeIDTypeClusterThermostatAttributeMinHeatSetpointLimitID = 0x00000015,
    MTRAttributeIDTypeClusterThermostatAttributeMaxHeatSetpointLimitID = 0x00000016,
    MTRAttributeIDTypeClusterThermostatAttributeMinCoolSetpointLimitID = 0x00000017,
    MTRAttributeIDTypeClusterThermostatAttributeMaxCoolSetpointLimitID = 0x00000018,
    MTRAttributeIDTypeClusterThermostatAttributeMinSetpointDeadBandID = 0x00000019,
    MTRAttributeIDTypeClusterThermostatAttributeRemoteSensingID = 0x0000001A,
    MTRAttributeIDTypeClusterThermostatAttributeControlSequenceOfOperationID = 0x0000001B,
    MTRAttributeIDTypeClusterThermostatAttributeSystemModeID = 0x0000001C,
    MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningModeID = 0x0000001E,
    MTRAttributeIDTypeClusterThermostatAttributeStartOfWeekID = 0x00000020,
    MTRAttributeIDTypeClusterThermostatAttributeNumberOfWeeklyTransitionsID = 0x00000021,
    MTRAttributeIDTypeClusterThermostatAttributeNumberOfDailyTransitionsID = 0x00000022,
    MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldID = 0x00000023,
    MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldDurationID = 0x00000024,
    MTRAttributeIDTypeClusterThermostatAttributeThermostatProgrammingOperationModeID = 0x00000025,
    MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningStateID = 0x00000029,
    MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceID = 0x00000030,
    MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeAmountID = 0x00000031,
    MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceTimestampID = 0x00000032,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackID = 0x00000034,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMinID = 0x00000035,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMaxID = 0x00000036,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackID = 0x00000037,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMinID = 0x00000038,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMaxID = 0x00000039,
    MTRAttributeIDTypeClusterThermostatAttributeEmergencyHeatDeltaID = 0x0000003A,
    MTRAttributeIDTypeClusterThermostatAttributeACTypeID = 0x00000040,
    MTRAttributeIDTypeClusterThermostatAttributeACCapacityID = 0x00000041,
    MTRAttributeIDTypeClusterThermostatAttributeACRefrigerantTypeID = 0x00000042,
    MTRAttributeIDTypeClusterThermostatAttributeACCompressorTypeID = 0x00000043,
    MTRAttributeIDTypeClusterThermostatAttributeACErrorCodeID = 0x00000044,
    MTRAttributeIDTypeClusterThermostatAttributeACLouverPositionID = 0x00000045,
    MTRAttributeIDTypeClusterThermostatAttributeACCoilTemperatureID = 0x00000046,
    MTRAttributeIDTypeClusterThermostatAttributeACCapacityformatID = 0x00000047,
    MTRAttributeIDTypeClusterThermostatAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterThermostatAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterThermostatAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterThermostatAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterThermostatAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FanControl attributes
    MTRAttributeIDTypeClusterFanControlAttributeFanModeID = 0x00000000,
    MTRAttributeIDTypeClusterFanControlAttributeFanModeSequenceID = 0x00000001,
    MTRAttributeIDTypeClusterFanControlAttributePercentSettingID = 0x00000002,
    MTRAttributeIDTypeClusterFanControlAttributePercentCurrentID = 0x00000003,
    MTRAttributeIDTypeClusterFanControlAttributeSpeedMaxID = 0x00000004,
    MTRAttributeIDTypeClusterFanControlAttributeSpeedSettingID = 0x00000005,
    MTRAttributeIDTypeClusterFanControlAttributeSpeedCurrentID = 0x00000006,
    MTRAttributeIDTypeClusterFanControlAttributeRockSupportID = 0x00000007,
    MTRAttributeIDTypeClusterFanControlAttributeRockSettingID = 0x00000008,
    MTRAttributeIDTypeClusterFanControlAttributeWindSupportID = 0x00000009,
    MTRAttributeIDTypeClusterFanControlAttributeWindSettingID = 0x0000000A,
    MTRAttributeIDTypeClusterFanControlAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFanControlAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFanControlAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFanControlAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFanControlAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ThermostatUserInterfaceConfiguration attributes
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeTemperatureDisplayModeID = 0x00000000,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeKeypadLockoutID = 0x00000001,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeScheduleProgrammingVisibilityID = 0x00000002,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAttributeListID
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeFeatureMapID
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeClusterRevisionID
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ColorControl attributes
    MTRAttributeIDTypeClusterColorControlAttributeCurrentHueID = 0x00000000,
    MTRAttributeIDTypeClusterColorControlAttributeCurrentSaturationID = 0x00000001,
    MTRAttributeIDTypeClusterColorControlAttributeRemainingTimeID = 0x00000002,
    MTRAttributeIDTypeClusterColorControlAttributeCurrentXID = 0x00000003,
    MTRAttributeIDTypeClusterColorControlAttributeCurrentYID = 0x00000004,
    MTRAttributeIDTypeClusterColorControlAttributeDriftCompensationID = 0x00000005,
    MTRAttributeIDTypeClusterColorControlAttributeCompensationTextID = 0x00000006,
    MTRAttributeIDTypeClusterColorControlAttributeColorTemperatureMiredsID = 0x00000007,
    MTRAttributeIDTypeClusterColorControlAttributeColorModeID = 0x00000008,
    MTRAttributeIDTypeClusterColorControlAttributeOptionsID = 0x0000000F,
    MTRAttributeIDTypeClusterColorControlAttributeNumberOfPrimariesID = 0x00000010,
    MTRAttributeIDTypeClusterColorControlAttributePrimary1XID = 0x00000011,
    MTRAttributeIDTypeClusterColorControlAttributePrimary1YID = 0x00000012,
    MTRAttributeIDTypeClusterColorControlAttributePrimary1IntensityID = 0x00000013,
    MTRAttributeIDTypeClusterColorControlAttributePrimary2XID = 0x00000015,
    MTRAttributeIDTypeClusterColorControlAttributePrimary2YID = 0x00000016,
    MTRAttributeIDTypeClusterColorControlAttributePrimary2IntensityID = 0x00000017,
    MTRAttributeIDTypeClusterColorControlAttributePrimary3XID = 0x00000019,
    MTRAttributeIDTypeClusterColorControlAttributePrimary3YID = 0x0000001A,
    MTRAttributeIDTypeClusterColorControlAttributePrimary3IntensityID = 0x0000001B,
    MTRAttributeIDTypeClusterColorControlAttributePrimary4XID = 0x00000020,
    MTRAttributeIDTypeClusterColorControlAttributePrimary4YID = 0x00000021,
    MTRAttributeIDTypeClusterColorControlAttributePrimary4IntensityID = 0x00000022,
    MTRAttributeIDTypeClusterColorControlAttributePrimary5XID = 0x00000024,
    MTRAttributeIDTypeClusterColorControlAttributePrimary5YID = 0x00000025,
    MTRAttributeIDTypeClusterColorControlAttributePrimary5IntensityID = 0x00000026,
    MTRAttributeIDTypeClusterColorControlAttributePrimary6XID = 0x00000028,
    MTRAttributeIDTypeClusterColorControlAttributePrimary6YID = 0x00000029,
    MTRAttributeIDTypeClusterColorControlAttributePrimary6IntensityID = 0x0000002A,
    MTRAttributeIDTypeClusterColorControlAttributeWhitePointXID = 0x00000030,
    MTRAttributeIDTypeClusterColorControlAttributeWhitePointYID = 0x00000031,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointRXID = 0x00000032,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointRYID = 0x00000033,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointRIntensityID = 0x00000034,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointGXID = 0x00000036,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointGYID = 0x00000037,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointGIntensityID = 0x00000038,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointBXID = 0x0000003A,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointBYID = 0x0000003B,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointBIntensityID = 0x0000003C,
    MTRAttributeIDTypeClusterColorControlAttributeEnhancedCurrentHueID = 0x00004000,
    MTRAttributeIDTypeClusterColorControlAttributeEnhancedColorModeID = 0x00004001,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopActiveID = 0x00004002,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopDirectionID = 0x00004003,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopTimeID = 0x00004004,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopStartEnhancedHueID = 0x00004005,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopStoredEnhancedHueID = 0x00004006,
    MTRAttributeIDTypeClusterColorControlAttributeColorCapabilitiesID = 0x0000400A,
    MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMinMiredsID = 0x0000400B,
    MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMaxMiredsID = 0x0000400C,
    MTRAttributeIDTypeClusterColorControlAttributeCoupleColorTempToLevelMinMiredsID = 0x0000400D,
    MTRAttributeIDTypeClusterColorControlAttributeStartUpColorTemperatureMiredsID = 0x00004010,
    MTRAttributeIDTypeClusterColorControlAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterColorControlAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterColorControlAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterColorControlAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterColorControlAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BallastConfiguration attributes
    MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMinLevelID = 0x00000000,
    MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMaxLevelID = 0x00000001,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastStatusID = 0x00000002,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeMinLevelID = 0x00000010,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeMaxLevelID = 0x00000011,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeIntrinsicBalanceFactorID = 0x00000014,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastFactorAdjustmentID = 0x00000015,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampQuantityID = 0x00000020,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampTypeID = 0x00000030,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampManufacturerID = 0x00000031,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampRatedHoursID = 0x00000032,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursID = 0x00000033,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampAlarmModeID = 0x00000034,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursTripPointID = 0x00000035,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster IlluminanceMeasurement attributes
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMeasuredValueID = 0x00000000,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMinMeasuredValueID = 0x00000001,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMaxMeasuredValueID = 0x00000002,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeToleranceID = 0x00000003,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeLightSensorTypeID = 0x00000004,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TemperatureMeasurement attributes
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMeasuredValueID = 0x00000000,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMinMeasuredValueID = 0x00000001,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMaxMeasuredValueID = 0x00000002,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeToleranceID = 0x00000003,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PressureMeasurement attributes
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMeasuredValueID = 0x00000000,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMinMeasuredValueID = 0x00000001,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxMeasuredValueID = 0x00000002,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeToleranceID = 0x00000003,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledValueID = 0x00000010,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMinScaledValueID = 0x00000011,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxScaledValueID = 0x00000012,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledToleranceID = 0x00000013,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeScaleID = 0x00000014,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FlowMeasurement attributes
    MTRAttributeIDTypeClusterFlowMeasurementAttributeMeasuredValueID = 0x00000000,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeMinMeasuredValueID = 0x00000001,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeMaxMeasuredValueID = 0x00000002,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeToleranceID = 0x00000003,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster RelativeHumidityMeasurement attributes
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMeasuredValueID = 0x00000000,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMinMeasuredValueID = 0x00000001,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMaxMeasuredValueID = 0x00000002,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeToleranceID = 0x00000003,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeClusterRevisionID
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OccupancySensing attributes
    MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancyID = 0x00000000,
    MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeID = 0x00000001,
    MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeBitmapID = 0x00000002,
    MTRAttributeIDTypeClusterOccupancySensingAttributePIROccupiedToUnoccupiedDelayID = 0x00000010,
    MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedDelayID = 0x00000011,
    MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedThresholdID = 0x00000012,
    MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicOccupiedToUnoccupiedDelayID = 0x00000020,
    MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedDelayID = 0x00000021,
    MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedThresholdID = 0x00000022,
    MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactOccupiedToUnoccupiedDelayID = 0x00000030,
    MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedDelayID = 0x00000031,
    MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedThresholdID = 0x00000032,
    MTRAttributeIDTypeClusterOccupancySensingAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster WakeOnLAN attributes
    MTRAttributeIDTypeClusterWakeOnLANAttributeMACAddressID = 0x00000000,
    MTRAttributeIDTypeClusterWakeOnLANAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Channel attributes
    MTRAttributeIDTypeClusterChannelAttributeChannelListID = 0x00000000,
    MTRAttributeIDTypeClusterChannelAttributeLineupID = 0x00000001,
    MTRAttributeIDTypeClusterChannelAttributeCurrentChannelID = 0x00000002,
    MTRAttributeIDTypeClusterChannelAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterChannelAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterChannelAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterChannelAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterChannelAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TargetNavigator attributes
    MTRAttributeIDTypeClusterTargetNavigatorAttributeTargetListID = 0x00000000,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeCurrentTargetID = 0x00000001,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster MediaPlayback attributes
    MTRAttributeIDTypeClusterMediaPlaybackAttributeCurrentStateID = 0x00000000,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeStartTimeID = 0x00000001,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeDurationID = 0x00000002,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeSampledPositionID = 0x00000003,
    MTRAttributeIDTypeClusterMediaPlaybackAttributePlaybackSpeedID = 0x00000004,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeEndID = 0x00000005,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeStartID = 0x00000006,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster MediaInput attributes
    MTRAttributeIDTypeClusterMediaInputAttributeInputListID = 0x00000000,
    MTRAttributeIDTypeClusterMediaInputAttributeCurrentInputID = 0x00000001,
    MTRAttributeIDTypeClusterMediaInputAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterMediaInputAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterMediaInputAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterMediaInputAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterMediaInputAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster LowPower attributes
    MTRAttributeIDTypeClusterLowPowerAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterLowPowerAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterLowPowerAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterLowPowerAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterLowPowerAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster KeypadInput attributes
    MTRAttributeIDTypeClusterKeypadInputAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterKeypadInputAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterKeypadInputAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterKeypadInputAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterKeypadInputAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ContentLauncher attributes
    MTRAttributeIDTypeClusterContentLauncherAttributeAcceptHeaderID = 0x00000000,
    MTRAttributeIDTypeClusterContentLauncherAttributeSupportedStreamingProtocolsID = 0x00000001,
    MTRAttributeIDTypeClusterContentLauncherAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterContentLauncherAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterContentLauncherAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterContentLauncherAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterContentLauncherAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AudioOutput attributes
    MTRAttributeIDTypeClusterAudioOutputAttributeOutputListID = 0x00000000,
    MTRAttributeIDTypeClusterAudioOutputAttributeCurrentOutputID = 0x00000001,
    MTRAttributeIDTypeClusterAudioOutputAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAudioOutputAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAudioOutputAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAudioOutputAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAudioOutputAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ApplicationLauncher attributes
    MTRAttributeIDTypeClusterApplicationLauncherAttributeCatalogListID = 0x00000000,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeCurrentAppID = 0x00000001,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ApplicationBasic attributes
    MTRAttributeIDTypeClusterApplicationBasicAttributeVendorNameID = 0x00000000,
    MTRAttributeIDTypeClusterApplicationBasicAttributeVendorIDID = 0x00000001,
    MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationNameID = 0x00000002,
    MTRAttributeIDTypeClusterApplicationBasicAttributeProductIDID = 0x00000003,
    MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationID = 0x00000004,
    MTRAttributeIDTypeClusterApplicationBasicAttributeStatusID = 0x00000005,
    MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationVersionID = 0x00000006,
    MTRAttributeIDTypeClusterApplicationBasicAttributeAllowedVendorListID = 0x00000007,
    MTRAttributeIDTypeClusterApplicationBasicAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AccountLogin attributes
    MTRAttributeIDTypeClusterAccountLoginAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAccountLoginAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAccountLoginAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAccountLoginAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAccountLoginAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ElectricalMeasurement attributes
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasurementTypeID = 0x00000000,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageID = 0x00000100,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMinID = 0x00000101,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMaxID = 0x00000102,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentID = 0x00000103,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMinID = 0x00000104,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMaxID = 0x00000105,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerID = 0x00000106,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMinID = 0x00000107,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMaxID = 0x00000108,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMultiplierID = 0x00000200,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageDivisorID = 0x00000201,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMultiplierID = 0x00000202,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentDivisorID = 0x00000203,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMultiplierID = 0x00000204,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerDivisorID = 0x00000205,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyID = 0x00000300,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMinID = 0x00000301,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMaxID = 0x00000302,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeNeutralCurrentID = 0x00000303,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalActivePowerID = 0x00000304,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalReactivePowerID = 0x00000305,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalApparentPowerID = 0x00000306,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured1stHarmonicCurrentID = 0x00000307,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured3rdHarmonicCurrentID = 0x00000308,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured5thHarmonicCurrentID = 0x00000309,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured7thHarmonicCurrentID = 0x0000030A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured9thHarmonicCurrentID = 0x0000030B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured11thHarmonicCurrentID = 0x0000030C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase1stHarmonicCurrentID = 0x0000030D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase3rdHarmonicCurrentID = 0x0000030E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase5thHarmonicCurrentID = 0x0000030F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase7thHarmonicCurrentID = 0x00000310,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase9thHarmonicCurrentID = 0x00000311,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase11thHarmonicCurrentID = 0x00000312,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMultiplierID = 0x00000400,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyDivisorID = 0x00000401,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerMultiplierID = 0x00000402,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerDivisorID = 0x00000403,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeHarmonicCurrentMultiplierID = 0x00000404,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePhaseHarmonicCurrentMultiplierID = 0x00000405,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousVoltageID = 0x00000500,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousLineCurrentID = 0x00000501,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousActiveCurrentID = 0x00000502,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousReactiveCurrentID = 0x00000503,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousPowerID = 0x00000504,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageID = 0x00000505,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinID = 0x00000506,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxID = 0x00000507,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentID = 0x00000508,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinID = 0x00000509,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxID = 0x0000050A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerID = 0x0000050B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinID = 0x0000050C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxID = 0x0000050D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerID = 0x0000050E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerID = 0x0000050F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorID = 0x00000510,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodID = 0x00000511,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterID = 0x00000513,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodID = 0x00000514,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodID = 0x00000515,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodID = 0x00000516,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodID = 0x00000517,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageMultiplierID = 0x00000600,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageDivisorID = 0x00000601,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentMultiplierID = 0x00000602,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentDivisorID = 0x00000603,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerMultiplierID = 0x00000604,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerDivisorID = 0x00000605,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeOverloadAlarmsMaskID = 0x00000700,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeVoltageOverloadID = 0x00000701,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeCurrentOverloadID = 0x00000702,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcOverloadAlarmsMaskID = 0x00000800,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageOverloadID = 0x00000801,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentOverloadID = 0x00000802,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcActivePowerOverloadID = 0x00000803,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcReactivePowerOverloadID = 0x00000804,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageID = 0x00000805,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageID = 0x00000806,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltageID = 0x00000807,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltageID = 0x00000808,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagID = 0x00000809,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellID = 0x0000080A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseBID = 0x00000901,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseBID = 0x00000902,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseBID = 0x00000903,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseBID = 0x00000905,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseBID = 0x00000906,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseBID = 0x00000907,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseBID = 0x00000908,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseBID = 0x00000909,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseBID = 0x0000090A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseBID = 0x0000090B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseBID = 0x0000090C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseBID = 0x0000090D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseBID = 0x0000090E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseBID = 0x0000090F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseBID = 0x00000910,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseBID = 0x00000911,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseBID = 0x00000912,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseBID = 0x00000913,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseBID = 0x00000914,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseBID = 0x00000915,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseBID = 0x00000916,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseBID = 0x00000917,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseCID = 0x00000A01,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseCID = 0x00000A02,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseCID = 0x00000A03,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseCID = 0x00000A05,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseCID = 0x00000A06,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseCID = 0x00000A07,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseCID = 0x00000A08,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseCID = 0x00000A09,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseCID = 0x00000A0A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseCID = 0x00000A0B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseCID = 0x00000A0C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseCID = 0x00000A0D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseCID = 0x00000A0E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseCID = 0x00000A0F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseCID = 0x00000A10,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseCID = 0x00000A11,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseCID = 0x00000A12,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseCID = 0x00000A13,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseCID = 0x00000A14,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseCID = 0x00000A15,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseCID = 0x00000A16,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseCID = 0x00000A17,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcceptedCommandListID
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TestCluster attributes
    MTRAttributeIDTypeClusterTestClusterAttributeBooleanID = 0x00000000,
    MTRAttributeIDTypeClusterTestClusterAttributeBitmap8ID = 0x00000001,
    MTRAttributeIDTypeClusterTestClusterAttributeBitmap16ID = 0x00000002,
    MTRAttributeIDTypeClusterTestClusterAttributeBitmap32ID = 0x00000003,
    MTRAttributeIDTypeClusterTestClusterAttributeBitmap64ID = 0x00000004,
    MTRAttributeIDTypeClusterTestClusterAttributeInt8uID = 0x00000005,
    MTRAttributeIDTypeClusterTestClusterAttributeInt16uID = 0x00000006,
    MTRAttributeIDTypeClusterTestClusterAttributeInt24uID = 0x00000007,
    MTRAttributeIDTypeClusterTestClusterAttributeInt32uID = 0x00000008,
    MTRAttributeIDTypeClusterTestClusterAttributeInt40uID = 0x00000009,
    MTRAttributeIDTypeClusterTestClusterAttributeInt48uID = 0x0000000A,
    MTRAttributeIDTypeClusterTestClusterAttributeInt56uID = 0x0000000B,
    MTRAttributeIDTypeClusterTestClusterAttributeInt64uID = 0x0000000C,
    MTRAttributeIDTypeClusterTestClusterAttributeInt8sID = 0x0000000D,
    MTRAttributeIDTypeClusterTestClusterAttributeInt16sID = 0x0000000E,
    MTRAttributeIDTypeClusterTestClusterAttributeInt24sID = 0x0000000F,
    MTRAttributeIDTypeClusterTestClusterAttributeInt32sID = 0x00000010,
    MTRAttributeIDTypeClusterTestClusterAttributeInt40sID = 0x00000011,
    MTRAttributeIDTypeClusterTestClusterAttributeInt48sID = 0x00000012,
    MTRAttributeIDTypeClusterTestClusterAttributeInt56sID = 0x00000013,
    MTRAttributeIDTypeClusterTestClusterAttributeInt64sID = 0x00000014,
    MTRAttributeIDTypeClusterTestClusterAttributeEnum8ID = 0x00000015,
    MTRAttributeIDTypeClusterTestClusterAttributeEnum16ID = 0x00000016,
    MTRAttributeIDTypeClusterTestClusterAttributeFloatSingleID = 0x00000017,
    MTRAttributeIDTypeClusterTestClusterAttributeFloatDoubleID = 0x00000018,
    MTRAttributeIDTypeClusterTestClusterAttributeOctetStringID = 0x00000019,
    MTRAttributeIDTypeClusterTestClusterAttributeListInt8uID = 0x0000001A,
    MTRAttributeIDTypeClusterTestClusterAttributeListOctetStringID = 0x0000001B,
    MTRAttributeIDTypeClusterTestClusterAttributeListStructOctetStringID = 0x0000001C,
    MTRAttributeIDTypeClusterTestClusterAttributeLongOctetStringID = 0x0000001D,
    MTRAttributeIDTypeClusterTestClusterAttributeCharStringID = 0x0000001E,
    MTRAttributeIDTypeClusterTestClusterAttributeLongCharStringID = 0x0000001F,
    MTRAttributeIDTypeClusterTestClusterAttributeEpochUsID = 0x00000020,
    MTRAttributeIDTypeClusterTestClusterAttributeEpochSID = 0x00000021,
    MTRAttributeIDTypeClusterTestClusterAttributeVendorIdID = 0x00000022,
    MTRAttributeIDTypeClusterTestClusterAttributeListNullablesAndOptionalsStructID = 0x00000023,
    MTRAttributeIDTypeClusterTestClusterAttributeEnumAttrID = 0x00000024,
    MTRAttributeIDTypeClusterTestClusterAttributeStructAttrID = 0x00000025,
    MTRAttributeIDTypeClusterTestClusterAttributeRangeRestrictedInt8uID = 0x00000026,
    MTRAttributeIDTypeClusterTestClusterAttributeRangeRestrictedInt8sID = 0x00000027,
    MTRAttributeIDTypeClusterTestClusterAttributeRangeRestrictedInt16uID = 0x00000028,
    MTRAttributeIDTypeClusterTestClusterAttributeRangeRestrictedInt16sID = 0x00000029,
    MTRAttributeIDTypeClusterTestClusterAttributeListLongOctetStringID = 0x0000002A,
    MTRAttributeIDTypeClusterTestClusterAttributeListFabricScopedID = 0x0000002B,
    MTRAttributeIDTypeClusterTestClusterAttributeTimedWriteBooleanID = 0x00000030,
    MTRAttributeIDTypeClusterTestClusterAttributeGeneralErrorBooleanID = 0x00000031,
    MTRAttributeIDTypeClusterTestClusterAttributeClusterErrorBooleanID = 0x00000032,
    MTRAttributeIDTypeClusterTestClusterAttributeUnsupportedID = 0x000000FF,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableBooleanID = 0x00004000,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableBitmap8ID = 0x00004001,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableBitmap16ID = 0x00004002,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableBitmap32ID = 0x00004003,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableBitmap64ID = 0x00004004,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt8uID = 0x00004005,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt16uID = 0x00004006,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt24uID = 0x00004007,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt32uID = 0x00004008,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt40uID = 0x00004009,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt48uID = 0x0000400A,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt56uID = 0x0000400B,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt64uID = 0x0000400C,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt8sID = 0x0000400D,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt16sID = 0x0000400E,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt24sID = 0x0000400F,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt32sID = 0x00004010,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt40sID = 0x00004011,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt48sID = 0x00004012,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt56sID = 0x00004013,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableInt64sID = 0x00004014,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableEnum8ID = 0x00004015,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableEnum16ID = 0x00004016,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableFloatSingleID = 0x00004017,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableFloatDoubleID = 0x00004018,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableOctetStringID = 0x00004019,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableCharStringID = 0x0000401E,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableEnumAttrID = 0x00004024,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableStructID = 0x00004025,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableRangeRestrictedInt8uID = 0x00004026,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableRangeRestrictedInt8sID = 0x00004027,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableRangeRestrictedInt16uID = 0x00004028,
    MTRAttributeIDTypeClusterTestClusterAttributeNullableRangeRestrictedInt16sID = 0x00004029,
    MTRAttributeIDTypeClusterTestClusterAttributeWriteOnlyInt8uID = 0x0000402A,
    MTRAttributeIDTypeClusterTestClusterAttributeGeneratedCommandListID = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTestClusterAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTestClusterAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTestClusterAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTestClusterAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FaultInjection attributes
    MTRAttributeIDTypeClusterFaultInjectionAttributeGeneratedCommandListID
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFaultInjectionAttributeAcceptedCommandListID = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFaultInjectionAttributeAttributeListID = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFaultInjectionAttributeFeatureMapID = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFaultInjectionAttributeClusterRevisionID = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

};

#pragma mark - Commands IDs

typedef NS_ENUM(uint32_t, MTRCommandIDType) {
    // Cluster Identify commands
    MTRCommandIDTypeClusterIdentifyCommandIdentifyID = 0x00000000,
    MTRCommandIDTypeClusterIdentifyCommandTriggerEffectID = 0x00000040,

    // Cluster Groups commands
    MTRCommandIDTypeClusterGroupsCommandAddGroupID = 0x00000000,
    MTRCommandIDTypeClusterGroupsCommandAddGroupResponseID = 0x00000000,
    MTRCommandIDTypeClusterGroupsCommandViewGroupID = 0x00000001,
    MTRCommandIDTypeClusterGroupsCommandViewGroupResponseID = 0x00000001,
    MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipID = 0x00000002,
    MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipResponseID = 0x00000002,
    MTRCommandIDTypeClusterGroupsCommandRemoveGroupID = 0x00000003,
    MTRCommandIDTypeClusterGroupsCommandRemoveGroupResponseID = 0x00000003,
    MTRCommandIDTypeClusterGroupsCommandRemoveAllGroupsID = 0x00000004,
    MTRCommandIDTypeClusterGroupsCommandAddGroupIfIdentifyingID = 0x00000005,

    // Cluster Scenes commands
    MTRCommandIDTypeClusterScenesCommandAddSceneID = 0x00000000,
    MTRCommandIDTypeClusterScenesCommandAddSceneResponseID = 0x00000000,
    MTRCommandIDTypeClusterScenesCommandViewSceneID = 0x00000001,
    MTRCommandIDTypeClusterScenesCommandViewSceneResponseID = 0x00000001,
    MTRCommandIDTypeClusterScenesCommandRemoveSceneID = 0x00000002,
    MTRCommandIDTypeClusterScenesCommandRemoveSceneResponseID = 0x00000002,
    MTRCommandIDTypeClusterScenesCommandRemoveAllScenesID = 0x00000003,
    MTRCommandIDTypeClusterScenesCommandRemoveAllScenesResponseID = 0x00000003,
    MTRCommandIDTypeClusterScenesCommandStoreSceneID = 0x00000004,
    MTRCommandIDTypeClusterScenesCommandStoreSceneResponseID = 0x00000004,
    MTRCommandIDTypeClusterScenesCommandRecallSceneID = 0x00000005,
    MTRCommandIDTypeClusterScenesCommandGetSceneMembershipID = 0x00000006,
    MTRCommandIDTypeClusterScenesCommandGetSceneMembershipResponseID = 0x00000006,
    MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneID = 0x00000040,
    MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneResponseID = 0x00000040,
    MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneID = 0x00000041,
    MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneResponseID = 0x00000041,
    MTRCommandIDTypeClusterScenesCommandCopySceneID = 0x00000042,
    MTRCommandIDTypeClusterScenesCommandCopySceneResponseID = 0x00000042,

    // Cluster OnOff commands
    MTRCommandIDTypeClusterOnOffCommandOffID = 0x00000000,
    MTRCommandIDTypeClusterOnOffCommandOnID = 0x00000001,
    MTRCommandIDTypeClusterOnOffCommandToggleID = 0x00000002,
    MTRCommandIDTypeClusterOnOffCommandOffWithEffectID = 0x00000040,
    MTRCommandIDTypeClusterOnOffCommandOnWithRecallGlobalSceneID = 0x00000041,
    MTRCommandIDTypeClusterOnOffCommandOnWithTimedOffID = 0x00000042,

    // Cluster LevelControl commands
    MTRCommandIDTypeClusterLevelControlCommandMoveToLevelID = 0x00000000,
    MTRCommandIDTypeClusterLevelControlCommandMoveID = 0x00000001,
    MTRCommandIDTypeClusterLevelControlCommandStepID = 0x00000002,
    MTRCommandIDTypeClusterLevelControlCommandStopID = 0x00000003,
    MTRCommandIDTypeClusterLevelControlCommandMoveToLevelWithOnOffID = 0x00000004,
    MTRCommandIDTypeClusterLevelControlCommandMoveWithOnOffID = 0x00000005,
    MTRCommandIDTypeClusterLevelControlCommandStepWithOnOffID = 0x00000006,
    MTRCommandIDTypeClusterLevelControlCommandStopWithOnOffID = 0x00000007,
    MTRCommandIDTypeClusterLevelControlCommandMoveToClosestFrequencyID = 0x00000008,

    // Cluster Actions commands
    MTRCommandIDTypeClusterActionsCommandInstantActionID = 0x00000000,
    MTRCommandIDTypeClusterActionsCommandInstantActionWithTransitionID = 0x00000001,
    MTRCommandIDTypeClusterActionsCommandStartActionID = 0x00000002,
    MTRCommandIDTypeClusterActionsCommandStartActionWithDurationID = 0x00000003,
    MTRCommandIDTypeClusterActionsCommandStopActionID = 0x00000004,
    MTRCommandIDTypeClusterActionsCommandPauseActionID = 0x00000005,
    MTRCommandIDTypeClusterActionsCommandPauseActionWithDurationID = 0x00000006,
    MTRCommandIDTypeClusterActionsCommandResumeActionID = 0x00000007,
    MTRCommandIDTypeClusterActionsCommandEnableActionID = 0x00000008,
    MTRCommandIDTypeClusterActionsCommandEnableActionWithDurationID = 0x00000009,
    MTRCommandIDTypeClusterActionsCommandDisableActionID = 0x0000000A,
    MTRCommandIDTypeClusterActionsCommandDisableActionWithDurationID = 0x0000000B,

    // Cluster Basic commands
    MTRCommandIDTypeClusterBasicCommandMfgSpecificPingID = 0x10020000,

    // Cluster OTASoftwareUpdateProvider commands
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageID = 0x00000000,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageResponseID = 0x00000001,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateRequestID = 0x00000002,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateResponseID = 0x00000003,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandNotifyUpdateAppliedID = 0x00000004,

    // Cluster OTASoftwareUpdateRequestor commands
    MTRCommandIDTypeClusterOTASoftwareUpdateRequestorCommandAnnounceOtaProviderID = 0x00000000,

    // Cluster GeneralCommissioning commands
    MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeID = 0x00000000,
    MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeResponseID = 0x00000001,
    MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigID = 0x00000002,
    MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigResponseID = 0x00000003,
    MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteID = 0x00000004,
    MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteResponseID = 0x00000005,

    // Cluster NetworkCommissioning commands
    MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksID = 0x00000000,
    MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksResponseID = 0x00000001,
    MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateWiFiNetworkID = 0x00000002,
    MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateThreadNetworkID = 0x00000003,
    MTRCommandIDTypeClusterNetworkCommissioningCommandRemoveNetworkID = 0x00000004,
    MTRCommandIDTypeClusterNetworkCommissioningCommandNetworkConfigResponseID = 0x00000005,
    MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkID = 0x00000006,
    MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkResponseID = 0x00000007,
    MTRCommandIDTypeClusterNetworkCommissioningCommandReorderNetworkID = 0x00000008,

    // Cluster DiagnosticLogs commands
    MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsRequestID = 0x00000000,
    MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsResponseID = 0x00000001,

    // Cluster GeneralDiagnostics commands
    MTRCommandIDTypeClusterGeneralDiagnosticsCommandTestEventTriggerID = 0x00000000,

    // Cluster SoftwareDiagnostics commands
    MTRCommandIDTypeClusterSoftwareDiagnosticsCommandResetWatermarksID = 0x00000000,

    // Cluster ThreadNetworkDiagnostics commands
    MTRCommandIDTypeClusterThreadNetworkDiagnosticsCommandResetCountsID = 0x00000000,

    // Cluster WiFiNetworkDiagnostics commands
    MTRCommandIDTypeClusterWiFiNetworkDiagnosticsCommandResetCountsID = 0x00000000,

    // Cluster EthernetNetworkDiagnostics commands
    MTRCommandIDTypeClusterEthernetNetworkDiagnosticsCommandResetCountsID = 0x00000000,

    // Cluster TimeSynchronization commands
    MTRCommandIDTypeClusterTimeSynchronizationCommandSetUtcTimeID = 0x00000000,

    // Cluster AdministratorCommissioning commands
    MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenCommissioningWindowID = 0x00000000,
    MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenBasicCommissioningWindowID = 0x00000001,
    MTRCommandIDTypeClusterAdministratorCommissioningCommandRevokeCommissioningID = 0x00000002,

    // Cluster OperationalCredentials commands
    MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationRequestID = 0x00000000,
    MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationResponseID = 0x00000001,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainRequestID = 0x00000002,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainResponseID = 0x00000003,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCSRRequestID = 0x00000004,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCSRResponseID = 0x00000005,
    MTRCommandIDTypeClusterOperationalCredentialsCommandAddNOCID = 0x00000006,
    MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateNOCID = 0x00000007,
    MTRCommandIDTypeClusterOperationalCredentialsCommandNOCResponseID = 0x00000008,
    MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateFabricLabelID = 0x00000009,
    MTRCommandIDTypeClusterOperationalCredentialsCommandRemoveFabricID = 0x0000000A,
    MTRCommandIDTypeClusterOperationalCredentialsCommandAddTrustedRootCertificateID = 0x0000000B,

    // Cluster GroupKeyManagement commands
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetWriteID = 0x00000000,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadID = 0x00000001,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadResponseID = 0x00000002,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetRemoveID = 0x00000003,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesID = 0x00000004,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesResponseID = 0x00000005,

    // Cluster ModeSelect commands
    MTRCommandIDTypeClusterModeSelectCommandChangeToModeID = 0x00000000,

    // Cluster DoorLock commands
    MTRCommandIDTypeClusterDoorLockCommandLockDoorID = 0x00000000,
    MTRCommandIDTypeClusterDoorLockCommandUnlockDoorID = 0x00000001,
    MTRCommandIDTypeClusterDoorLockCommandUnlockWithTimeoutID = 0x00000003,
    MTRCommandIDTypeClusterDoorLockCommandSetWeekDayScheduleID = 0x0000000B,
    MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleID = 0x0000000C,
    MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleResponseID = 0x0000000C,
    MTRCommandIDTypeClusterDoorLockCommandClearWeekDayScheduleID = 0x0000000D,
    MTRCommandIDTypeClusterDoorLockCommandSetYearDayScheduleID = 0x0000000E,
    MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleID = 0x0000000F,
    MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleResponseID = 0x0000000F,
    MTRCommandIDTypeClusterDoorLockCommandClearYearDayScheduleID = 0x00000010,
    MTRCommandIDTypeClusterDoorLockCommandSetHolidayScheduleID = 0x00000011,
    MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleID = 0x00000012,
    MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleResponseID = 0x00000012,
    MTRCommandIDTypeClusterDoorLockCommandClearHolidayScheduleID = 0x00000013,
    MTRCommandIDTypeClusterDoorLockCommandSetUserID = 0x0000001A,
    MTRCommandIDTypeClusterDoorLockCommandGetUserID = 0x0000001B,
    MTRCommandIDTypeClusterDoorLockCommandGetUserResponseID = 0x0000001C,
    MTRCommandIDTypeClusterDoorLockCommandClearUserID = 0x0000001D,
    MTRCommandIDTypeClusterDoorLockCommandSetCredentialID = 0x00000022,
    MTRCommandIDTypeClusterDoorLockCommandSetCredentialResponseID = 0x00000023,
    MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusID = 0x00000024,
    MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusResponseID = 0x00000025,
    MTRCommandIDTypeClusterDoorLockCommandClearCredentialID = 0x00000026,

    // Cluster WindowCovering commands
    MTRCommandIDTypeClusterWindowCoveringCommandUpOrOpenID = 0x00000000,
    MTRCommandIDTypeClusterWindowCoveringCommandDownOrCloseID = 0x00000001,
    MTRCommandIDTypeClusterWindowCoveringCommandStopMotionID = 0x00000002,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftValueID = 0x00000004,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftPercentageID = 0x00000005,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltValueID = 0x00000007,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltPercentageID = 0x00000008,

    // Cluster BarrierControl commands
    MTRCommandIDTypeClusterBarrierControlCommandBarrierControlGoToPercentID = 0x00000000,
    MTRCommandIDTypeClusterBarrierControlCommandBarrierControlStopID = 0x00000001,

    // Cluster Thermostat commands
    MTRCommandIDTypeClusterThermostatCommandSetpointRaiseLowerID = 0x00000000,
    MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleResponseID = 0x00000000,
    MTRCommandIDTypeClusterThermostatCommandSetWeeklyScheduleID = 0x00000001,
    MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleID = 0x00000002,
    MTRCommandIDTypeClusterThermostatCommandClearWeeklyScheduleID = 0x00000003,

    // Cluster ColorControl commands
    MTRCommandIDTypeClusterColorControlCommandMoveToHueID = 0x00000000,
    MTRCommandIDTypeClusterColorControlCommandMoveHueID = 0x00000001,
    MTRCommandIDTypeClusterColorControlCommandStepHueID = 0x00000002,
    MTRCommandIDTypeClusterColorControlCommandMoveToSaturationID = 0x00000003,
    MTRCommandIDTypeClusterColorControlCommandMoveSaturationID = 0x00000004,
    MTRCommandIDTypeClusterColorControlCommandStepSaturationID = 0x00000005,
    MTRCommandIDTypeClusterColorControlCommandMoveToHueAndSaturationID = 0x00000006,
    MTRCommandIDTypeClusterColorControlCommandMoveToColorID = 0x00000007,
    MTRCommandIDTypeClusterColorControlCommandMoveColorID = 0x00000008,
    MTRCommandIDTypeClusterColorControlCommandStepColorID = 0x00000009,
    MTRCommandIDTypeClusterColorControlCommandMoveToColorTemperatureID = 0x0000000A,
    MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueID = 0x00000040,
    MTRCommandIDTypeClusterColorControlCommandEnhancedMoveHueID = 0x00000041,
    MTRCommandIDTypeClusterColorControlCommandEnhancedStepHueID = 0x00000042,
    MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueAndSaturationID = 0x00000043,
    MTRCommandIDTypeClusterColorControlCommandColorLoopSetID = 0x00000044,
    MTRCommandIDTypeClusterColorControlCommandStopMoveStepID = 0x00000047,
    MTRCommandIDTypeClusterColorControlCommandMoveColorTemperatureID = 0x0000004B,
    MTRCommandIDTypeClusterColorControlCommandStepColorTemperatureID = 0x0000004C,

    // Cluster Channel commands
    MTRCommandIDTypeClusterChannelCommandChangeChannelID = 0x00000000,
    MTRCommandIDTypeClusterChannelCommandChangeChannelResponseID = 0x00000001,
    MTRCommandIDTypeClusterChannelCommandChangeChannelByNumberID = 0x00000002,
    MTRCommandIDTypeClusterChannelCommandSkipChannelID = 0x00000003,

    // Cluster TargetNavigator commands
    MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetID = 0x00000000,
    MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetResponseID = 0x00000001,

    // Cluster MediaPlayback commands
    MTRCommandIDTypeClusterMediaPlaybackCommandPlayID = 0x00000000,
    MTRCommandIDTypeClusterMediaPlaybackCommandPauseID = 0x00000001,
    MTRCommandIDTypeClusterMediaPlaybackCommandStopPlaybackID = 0x00000002,
    MTRCommandIDTypeClusterMediaPlaybackCommandStartOverID = 0x00000003,
    MTRCommandIDTypeClusterMediaPlaybackCommandPreviousID = 0x00000004,
    MTRCommandIDTypeClusterMediaPlaybackCommandNextID = 0x00000005,
    MTRCommandIDTypeClusterMediaPlaybackCommandRewindID = 0x00000006,
    MTRCommandIDTypeClusterMediaPlaybackCommandFastForwardID = 0x00000007,
    MTRCommandIDTypeClusterMediaPlaybackCommandSkipForwardID = 0x00000008,
    MTRCommandIDTypeClusterMediaPlaybackCommandSkipBackwardID = 0x00000009,
    MTRCommandIDTypeClusterMediaPlaybackCommandPlaybackResponseID = 0x0000000A,
    MTRCommandIDTypeClusterMediaPlaybackCommandSeekID = 0x0000000B,

    // Cluster MediaInput commands
    MTRCommandIDTypeClusterMediaInputCommandSelectInputID = 0x00000000,
    MTRCommandIDTypeClusterMediaInputCommandShowInputStatusID = 0x00000001,
    MTRCommandIDTypeClusterMediaInputCommandHideInputStatusID = 0x00000002,
    MTRCommandIDTypeClusterMediaInputCommandRenameInputID = 0x00000003,

    // Cluster LowPower commands
    MTRCommandIDTypeClusterLowPowerCommandSleepID = 0x00000000,

    // Cluster KeypadInput commands
    MTRCommandIDTypeClusterKeypadInputCommandSendKeyID = 0x00000000,
    MTRCommandIDTypeClusterKeypadInputCommandSendKeyResponseID = 0x00000001,

    // Cluster ContentLauncher commands
    MTRCommandIDTypeClusterContentLauncherCommandLaunchContentID = 0x00000000,
    MTRCommandIDTypeClusterContentLauncherCommandLaunchURLID = 0x00000001,
    MTRCommandIDTypeClusterContentLauncherCommandLaunchResponseID = 0x00000002,

    // Cluster AudioOutput commands
    MTRCommandIDTypeClusterAudioOutputCommandSelectOutputID = 0x00000000,
    MTRCommandIDTypeClusterAudioOutputCommandRenameOutputID = 0x00000001,

    // Cluster ApplicationLauncher commands
    MTRCommandIDTypeClusterApplicationLauncherCommandLaunchAppID = 0x00000000,
    MTRCommandIDTypeClusterApplicationLauncherCommandStopAppID = 0x00000001,
    MTRCommandIDTypeClusterApplicationLauncherCommandHideAppID = 0x00000002,
    MTRCommandIDTypeClusterApplicationLauncherCommandLauncherResponseID = 0x00000003,

    // Cluster AccountLogin commands
    MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINID = 0x00000000,
    MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINResponseID = 0x00000001,
    MTRCommandIDTypeClusterAccountLoginCommandLoginID = 0x00000002,
    MTRCommandIDTypeClusterAccountLoginCommandLogoutID = 0x00000003,

    // Cluster ElectricalMeasurement commands
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoResponseCommandID = 0x00000000,
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoCommandID = 0x00000000,
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileResponseCommandID = 0x00000001,
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileCommandID = 0x00000001,

    // Cluster TestCluster commands
    MTRCommandIDTypeClusterTestClusterCommandTestID = 0x00000000,
    MTRCommandIDTypeClusterTestClusterCommandTestSpecificResponseID = 0x00000000,
    MTRCommandIDTypeClusterTestClusterCommandTestNotHandledID = 0x00000001,
    MTRCommandIDTypeClusterTestClusterCommandTestAddArgumentsResponseID = 0x00000001,
    MTRCommandIDTypeClusterTestClusterCommandTestSpecificID = 0x00000002,
    MTRCommandIDTypeClusterTestClusterCommandTestSimpleArgumentResponseID = 0x00000002,
    MTRCommandIDTypeClusterTestClusterCommandTestUnknownCommandID = 0x00000003,
    MTRCommandIDTypeClusterTestClusterCommandTestStructArrayArgumentResponseID = 0x00000003,
    MTRCommandIDTypeClusterTestClusterCommandTestAddArgumentsID = 0x00000004,
    MTRCommandIDTypeClusterTestClusterCommandTestListInt8UReverseResponseID = 0x00000004,
    MTRCommandIDTypeClusterTestClusterCommandTestSimpleArgumentRequestID = 0x00000005,
    MTRCommandIDTypeClusterTestClusterCommandTestEnumsResponseID = 0x00000005,
    MTRCommandIDTypeClusterTestClusterCommandTestStructArrayArgumentRequestID = 0x00000006,
    MTRCommandIDTypeClusterTestClusterCommandTestNullableOptionalResponseID = 0x00000006,
    MTRCommandIDTypeClusterTestClusterCommandTestStructArgumentRequestID = 0x00000007,
    MTRCommandIDTypeClusterTestClusterCommandTestComplexNullableOptionalResponseID = 0x00000007,
    MTRCommandIDTypeClusterTestClusterCommandTestNestedStructArgumentRequestID = 0x00000008,
    MTRCommandIDTypeClusterTestClusterCommandBooleanResponseID = 0x00000008,
    MTRCommandIDTypeClusterTestClusterCommandTestListStructArgumentRequestID = 0x00000009,
    MTRCommandIDTypeClusterTestClusterCommandSimpleStructResponseID = 0x00000009,
    MTRCommandIDTypeClusterTestClusterCommandTestListInt8UArgumentRequestID = 0x0000000A,
    MTRCommandIDTypeClusterTestClusterCommandTestEmitTestEventResponseID = 0x0000000A,
    MTRCommandIDTypeClusterTestClusterCommandTestNestedStructListArgumentRequestID = 0x0000000B,
    MTRCommandIDTypeClusterTestClusterCommandTestEmitTestFabricScopedEventResponseID = 0x0000000B,
    MTRCommandIDTypeClusterTestClusterCommandTestListNestedStructListArgumentRequestID = 0x0000000C,
    MTRCommandIDTypeClusterTestClusterCommandTestListInt8UReverseRequestID = 0x0000000D,
    MTRCommandIDTypeClusterTestClusterCommandTestEnumsRequestID = 0x0000000E,
    MTRCommandIDTypeClusterTestClusterCommandTestNullableOptionalRequestID = 0x0000000F,
    MTRCommandIDTypeClusterTestClusterCommandTestComplexNullableOptionalRequestID = 0x00000010,
    MTRCommandIDTypeClusterTestClusterCommandSimpleStructEchoRequestID = 0x00000011,
    MTRCommandIDTypeClusterTestClusterCommandTimedInvokeRequestID = 0x00000012,
    MTRCommandIDTypeClusterTestClusterCommandTestSimpleOptionalArgumentRequestID = 0x00000013,
    MTRCommandIDTypeClusterTestClusterCommandTestEmitTestEventRequestID = 0x00000014,
    MTRCommandIDTypeClusterTestClusterCommandTestEmitTestFabricScopedEventRequestID = 0x00000015,

    // Cluster FaultInjection commands
    MTRCommandIDTypeClusterFaultInjectionCommandFailAtFaultID = 0x00000000,
    MTRCommandIDTypeClusterFaultInjectionCommandFailRandomlyAtFaultID = 0x00000001,

};

#pragma mark - Events IDs

typedef NS_ENUM(uint32_t, MTREventIDType) {
    // Cluster AccessControl events
    MTREventIDTypeClusterAccessControlEventAccessControlEntryChangedID = 0x00000000,
    MTREventIDTypeClusterAccessControlEventAccessControlExtensionChangedID = 0x00000001,

    // Cluster Actions events
    MTREventIDTypeClusterActionsEventStateChangedID = 0x00000000,
    MTREventIDTypeClusterActionsEventActionFailedID = 0x00000001,

    // Cluster Basic events
    MTREventIDTypeClusterBasicEventStartUpID = 0x00000000,
    MTREventIDTypeClusterBasicEventShutDownID = 0x00000001,
    MTREventIDTypeClusterBasicEventLeaveID = 0x00000002,
    MTREventIDTypeClusterBasicEventReachableChangedID = 0x00000003,

    // Cluster OTASoftwareUpdateRequestor events
    MTREventIDTypeClusterOTASoftwareUpdateRequestorEventStateTransitionID = 0x00000000,
    MTREventIDTypeClusterOTASoftwareUpdateRequestorEventVersionAppliedID = 0x00000001,
    MTREventIDTypeClusterOTASoftwareUpdateRequestorEventDownloadErrorID = 0x00000002,

    // Cluster GeneralDiagnostics events
    MTREventIDTypeClusterGeneralDiagnosticsEventHardwareFaultChangeID = 0x00000000,
    MTREventIDTypeClusterGeneralDiagnosticsEventRadioFaultChangeID = 0x00000001,
    MTREventIDTypeClusterGeneralDiagnosticsEventNetworkFaultChangeID = 0x00000002,
    MTREventIDTypeClusterGeneralDiagnosticsEventBootReasonID = 0x00000003,

    // Cluster SoftwareDiagnostics events
    MTREventIDTypeClusterSoftwareDiagnosticsEventSoftwareFaultID = 0x00000000,

    // Cluster ThreadNetworkDiagnostics events
    MTREventIDTypeClusterThreadNetworkDiagnosticsEventConnectionStatusID = 0x00000000,
    MTREventIDTypeClusterThreadNetworkDiagnosticsEventNetworkFaultChangeID = 0x00000001,

    // Cluster WiFiNetworkDiagnostics events
    MTREventIDTypeClusterWiFiNetworkDiagnosticsEventDisconnectionID = 0x00000000,
    MTREventIDTypeClusterWiFiNetworkDiagnosticsEventAssociationFailureID = 0x00000001,
    MTREventIDTypeClusterWiFiNetworkDiagnosticsEventConnectionStatusID = 0x00000002,

    // Cluster BridgedDeviceBasic events
    MTREventIDTypeClusterBridgedDeviceBasicEventStartUpID = 0x00000000,
    MTREventIDTypeClusterBridgedDeviceBasicEventShutDownID = 0x00000001,
    MTREventIDTypeClusterBridgedDeviceBasicEventLeaveID = 0x00000002,
    MTREventIDTypeClusterBridgedDeviceBasicEventReachableChangedID = 0x00000003,

    // Cluster Switch events
    MTREventIDTypeClusterSwitchEventSwitchLatchedID = 0x00000000,
    MTREventIDTypeClusterSwitchEventInitialPressID = 0x00000001,
    MTREventIDTypeClusterSwitchEventLongPressID = 0x00000002,
    MTREventIDTypeClusterSwitchEventShortReleaseID = 0x00000003,
    MTREventIDTypeClusterSwitchEventLongReleaseID = 0x00000004,
    MTREventIDTypeClusterSwitchEventMultiPressOngoingID = 0x00000005,
    MTREventIDTypeClusterSwitchEventMultiPressCompleteID = 0x00000006,

    // Cluster BooleanState events
    MTREventIDTypeClusterBooleanStateEventStateChangeID = 0x00000000,

    // Cluster DoorLock events
    MTREventIDTypeClusterDoorLockEventDoorLockAlarmID = 0x00000000,
    MTREventIDTypeClusterDoorLockEventDoorStateChangeID = 0x00000001,
    MTREventIDTypeClusterDoorLockEventLockOperationID = 0x00000002,
    MTREventIDTypeClusterDoorLockEventLockOperationErrorID = 0x00000003,
    MTREventIDTypeClusterDoorLockEventLockUserChangeID = 0x00000004,

    // Cluster PumpConfigurationAndControl events
    MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageLowID = 0x00000000,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageHighID = 0x00000001,
    MTREventIDTypeClusterPumpConfigurationAndControlEventPowerMissingPhaseID = 0x00000002,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureLowID = 0x00000003,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureHighID = 0x00000004,
    MTREventIDTypeClusterPumpConfigurationAndControlEventDryRunningID = 0x00000005,
    MTREventIDTypeClusterPumpConfigurationAndControlEventMotorTemperatureHighID = 0x00000006,
    MTREventIDTypeClusterPumpConfigurationAndControlEventPumpMotorFatalFailureID = 0x00000007,
    MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicTemperatureHighID = 0x00000008,
    MTREventIDTypeClusterPumpConfigurationAndControlEventPumpBlockedID = 0x00000009,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSensorFailureID = 0x0000000A,
    MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicNonFatalFailureID = 0x0000000B,
    MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicFatalFailureID = 0x0000000C,
    MTREventIDTypeClusterPumpConfigurationAndControlEventGeneralFaultID = 0x0000000D,
    MTREventIDTypeClusterPumpConfigurationAndControlEventLeakageID = 0x0000000E,
    MTREventIDTypeClusterPumpConfigurationAndControlEventAirDetectionID = 0x0000000F,
    MTREventIDTypeClusterPumpConfigurationAndControlEventTurbineOperationID = 0x00000010,

    // Cluster TestCluster events
    MTREventIDTypeClusterTestClusterEventTestEventID = 0x00000001,
    MTREventIDTypeClusterTestClusterEventTestFabricScopedEventID = 0x00000002,

};
