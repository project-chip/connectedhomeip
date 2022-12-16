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
    MTRClusterIdentifyID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeIdentifyID") = 0x00000003,
    MTRClusterGroupsID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeGroupsID") = 0x00000004,
    MTRClusterScenesID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeScenesID") = 0x00000005,
    MTRClusterOnOffID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeOnOffID") = 0x00000006,
    MTRClusterOnOffSwitchConfigurationID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeOnOffSwitchConfigurationID") = 0x00000007,
    MTRClusterLevelControlID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeLevelControlID") = 0x00000008,
    MTRClusterBinaryInputBasicID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeBinaryInputBasicID") = 0x0000000F,
    MTRClusterPulseWidthModulationID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypePulseWidthModulationID") = 0x0000001C,
    MTRClusterDescriptorID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeDescriptorID") = 0x0000001D,
    MTRClusterBindingID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeBindingID") = 0x0000001E,
    MTRClusterAccessControlID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeAccessControlID") = 0x0000001F,
    MTRClusterActionsID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeActionsID") = 0x00000025,
    MTRClusterBasicID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeBasicID") = 0x00000028,
    MTRClusterOtaSoftwareUpdateProviderID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeOtaSoftwareUpdateProviderID")
    = 0x00000029,
    MTRClusterOtaSoftwareUpdateRequestorID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeOtaSoftwareUpdateRequestorID")
    = 0x0000002A,
    MTRClusterLocalizationConfigurationID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeLocalizationConfigurationID")
    = 0x0000002B,
    MTRClusterTimeFormatLocalizationID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeTimeFormatLocalizationID") = 0x0000002C,
    MTRClusterUnitLocalizationID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeUnitLocalizationID") = 0x0000002D,
    MTRClusterPowerSourceConfigurationID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypePowerSourceConfigurationID") = 0x0000002E,
    MTRClusterPowerSourceID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypePowerSourceID") = 0x0000002F,
    MTRClusterGeneralCommissioningID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeGeneralCommissioningID") = 0x00000030,
    MTRClusterNetworkCommissioningID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeNetworkCommissioningID") = 0x00000031,
    MTRClusterDiagnosticLogsID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeDiagnosticLogsID") = 0x00000032,
    MTRClusterGeneralDiagnosticsID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeGeneralDiagnosticsID") = 0x00000033,
    MTRClusterSoftwareDiagnosticsID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeSoftwareDiagnosticsID") = 0x00000034,
    MTRClusterThreadNetworkDiagnosticsID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeThreadNetworkDiagnosticsID") = 0x00000035,
    MTRClusterWiFiNetworkDiagnosticsID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeWiFiNetworkDiagnosticsID") = 0x00000036,
    MTRClusterEthernetNetworkDiagnosticsID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeEthernetNetworkDiagnosticsID")
    = 0x00000037,
    MTRClusterTimeSynchronizationID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeTimeSynchronizationID") = 0x00000038,
    MTRClusterBridgedDeviceBasicID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeBridgedDeviceBasicID") = 0x00000039,
    MTRClusterSwitchID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeSwitchID") = 0x0000003B,
    MTRClusterAdministratorCommissioningID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeAdministratorCommissioningID")
    = 0x0000003C,
    MTRClusterOperationalCredentialsID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeOperationalCredentialsID") = 0x0000003E,
    MTRClusterGroupKeyManagementID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeGroupKeyManagementID") = 0x0000003F,
    MTRClusterFixedLabelID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeFixedLabelID") = 0x00000040,
    MTRClusterUserLabelID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeUserLabelID") = 0x00000041,
    MTRClusterProxyConfigurationID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeProxyConfigurationID") = 0x00000042,
    MTRClusterProxyDiscoveryID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeProxyDiscoveryID") = 0x00000043,
    MTRClusterProxyValidID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeProxyValidID") = 0x00000044,
    MTRClusterBooleanStateID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeBooleanStateID") = 0x00000045,
    MTRClusterModeSelectID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeModeSelectID") = 0x00000050,
    MTRClusterDoorLockID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeDoorLockID") = 0x00000101,
    MTRClusterWindowCoveringID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeWindowCoveringID") = 0x00000102,
    MTRClusterBarrierControlID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeBarrierControlID") = 0x00000103,
    MTRClusterPumpConfigurationAndControlID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypePumpConfigurationAndControlID")
    = 0x00000200,
    MTRClusterThermostatID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeThermostatID") = 0x00000201,
    MTRClusterFanControlID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeFanControlID") = 0x00000202,
    MTRClusterThermostatUserInterfaceConfigurationID MTR_NEWLY_DEPRECATED(
        "Please use MTRClusterIDTypeThermostatUserInterfaceConfigurationID")
    = 0x00000204,
    MTRClusterColorControlID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeColorControlID") = 0x00000300,
    MTRClusterBallastConfigurationID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeBallastConfigurationID") = 0x00000301,
    MTRClusterIlluminanceMeasurementID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeIlluminanceMeasurementID") = 0x00000400,
    MTRClusterTemperatureMeasurementID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeTemperatureMeasurementID") = 0x00000402,
    MTRClusterPressureMeasurementID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypePressureMeasurementID") = 0x00000403,
    MTRClusterFlowMeasurementID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeFlowMeasurementID") = 0x00000404,
    MTRClusterRelativeHumidityMeasurementID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeRelativeHumidityMeasurementID")
    = 0x00000405,
    MTRClusterOccupancySensingID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeOccupancySensingID") = 0x00000406,
    MTRClusterWakeOnLanID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeWakeOnLanID") = 0x00000503,
    MTRClusterChannelID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeChannelID") = 0x00000504,
    MTRClusterTargetNavigatorID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeTargetNavigatorID") = 0x00000505,
    MTRClusterMediaPlaybackID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeMediaPlaybackID") = 0x00000506,
    MTRClusterMediaInputID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeMediaInputID") = 0x00000507,
    MTRClusterLowPowerID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeLowPowerID") = 0x00000508,
    MTRClusterKeypadInputID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeKeypadInputID") = 0x00000509,
    MTRClusterContentLauncherID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeContentLauncherID") = 0x0000050A,
    MTRClusterAudioOutputID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeAudioOutputID") = 0x0000050B,
    MTRClusterApplicationLauncherID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeApplicationLauncherID") = 0x0000050C,
    MTRClusterApplicationBasicID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeApplicationBasicID") = 0x0000050D,
    MTRClusterAccountLoginID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeAccountLoginID") = 0x0000050E,
    MTRClusterElectricalMeasurementID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeElectricalMeasurementID") = 0x00000B04,
    MTRClusterTestClusterID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeUnitTestingID") = 0xFFF1FC05,
    MTRClusterFaultInjectionID MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeFaultInjectionID") = 0xFFF1FC06,
    MTRClusterIDTypeIdentifyID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRClusterIDTypeGroupsID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRClusterIDTypeScenesID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRClusterIDTypeOnOffID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRClusterIDTypeOnOffSwitchConfigurationID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRClusterIDTypeLevelControlID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRClusterIDTypeBinaryInputBasicID MTR_NEWLY_AVAILABLE = 0x0000000F,
    MTRClusterIDTypePulseWidthModulationID MTR_NEWLY_AVAILABLE = 0x0000001C,
    MTRClusterIDTypeDescriptorID MTR_NEWLY_AVAILABLE = 0x0000001D,
    MTRClusterIDTypeBindingID MTR_NEWLY_AVAILABLE = 0x0000001E,
    MTRClusterIDTypeAccessControlID MTR_NEWLY_AVAILABLE = 0x0000001F,
    MTRClusterIDTypeActionsID MTR_NEWLY_AVAILABLE = 0x00000025,
    MTRClusterIDTypeBasicID MTR_NEWLY_AVAILABLE = 0x00000028,
    MTRClusterIDTypeOTASoftwareUpdateProviderID MTR_NEWLY_AVAILABLE = 0x00000029,
    MTRClusterIDTypeOTASoftwareUpdateRequestorID MTR_NEWLY_AVAILABLE = 0x0000002A,
    MTRClusterIDTypeLocalizationConfigurationID MTR_NEWLY_AVAILABLE = 0x0000002B,
    MTRClusterIDTypeTimeFormatLocalizationID MTR_NEWLY_AVAILABLE = 0x0000002C,
    MTRClusterIDTypeUnitLocalizationID MTR_NEWLY_AVAILABLE = 0x0000002D,
    MTRClusterIDTypePowerSourceConfigurationID MTR_NEWLY_AVAILABLE = 0x0000002E,
    MTRClusterIDTypePowerSourceID MTR_NEWLY_AVAILABLE = 0x0000002F,
    MTRClusterIDTypeGeneralCommissioningID MTR_NEWLY_AVAILABLE = 0x00000030,
    MTRClusterIDTypeNetworkCommissioningID MTR_NEWLY_AVAILABLE = 0x00000031,
    MTRClusterIDTypeDiagnosticLogsID MTR_NEWLY_AVAILABLE = 0x00000032,
    MTRClusterIDTypeGeneralDiagnosticsID MTR_NEWLY_AVAILABLE = 0x00000033,
    MTRClusterIDTypeSoftwareDiagnosticsID MTR_NEWLY_AVAILABLE = 0x00000034,
    MTRClusterIDTypeThreadNetworkDiagnosticsID MTR_NEWLY_AVAILABLE = 0x00000035,
    MTRClusterIDTypeWiFiNetworkDiagnosticsID MTR_NEWLY_AVAILABLE = 0x00000036,
    MTRClusterIDTypeEthernetNetworkDiagnosticsID MTR_NEWLY_AVAILABLE = 0x00000037,
    MTRClusterIDTypeTimeSynchronizationID MTR_NEWLY_AVAILABLE = 0x00000038,
    MTRClusterIDTypeBridgedDeviceBasicID MTR_NEWLY_AVAILABLE = 0x00000039,
    MTRClusterIDTypeSwitchID MTR_NEWLY_AVAILABLE = 0x0000003B,
    MTRClusterIDTypeAdministratorCommissioningID MTR_NEWLY_AVAILABLE = 0x0000003C,
    MTRClusterIDTypeOperationalCredentialsID MTR_NEWLY_AVAILABLE = 0x0000003E,
    MTRClusterIDTypeGroupKeyManagementID MTR_NEWLY_AVAILABLE = 0x0000003F,
    MTRClusterIDTypeFixedLabelID MTR_NEWLY_AVAILABLE = 0x00000040,
    MTRClusterIDTypeUserLabelID MTR_NEWLY_AVAILABLE = 0x00000041,
    MTRClusterIDTypeProxyConfigurationID MTR_NEWLY_AVAILABLE = 0x00000042,
    MTRClusterIDTypeProxyDiscoveryID MTR_NEWLY_AVAILABLE = 0x00000043,
    MTRClusterIDTypeProxyValidID MTR_NEWLY_AVAILABLE = 0x00000044,
    MTRClusterIDTypeBooleanStateID MTR_NEWLY_AVAILABLE = 0x00000045,
    MTRClusterIDTypeModeSelectID MTR_NEWLY_AVAILABLE = 0x00000050,
    MTRClusterIDTypeDoorLockID MTR_NEWLY_AVAILABLE = 0x00000101,
    MTRClusterIDTypeWindowCoveringID MTR_NEWLY_AVAILABLE = 0x00000102,
    MTRClusterIDTypeBarrierControlID MTR_NEWLY_AVAILABLE = 0x00000103,
    MTRClusterIDTypePumpConfigurationAndControlID MTR_NEWLY_AVAILABLE = 0x00000200,
    MTRClusterIDTypeThermostatID MTR_NEWLY_AVAILABLE = 0x00000201,
    MTRClusterIDTypeFanControlID MTR_NEWLY_AVAILABLE = 0x00000202,
    MTRClusterIDTypeThermostatUserInterfaceConfigurationID MTR_NEWLY_AVAILABLE = 0x00000204,
    MTRClusterIDTypeColorControlID MTR_NEWLY_AVAILABLE = 0x00000300,
    MTRClusterIDTypeBallastConfigurationID MTR_NEWLY_AVAILABLE = 0x00000301,
    MTRClusterIDTypeIlluminanceMeasurementID MTR_NEWLY_AVAILABLE = 0x00000400,
    MTRClusterIDTypeTemperatureMeasurementID MTR_NEWLY_AVAILABLE = 0x00000402,
    MTRClusterIDTypePressureMeasurementID MTR_NEWLY_AVAILABLE = 0x00000403,
    MTRClusterIDTypeFlowMeasurementID MTR_NEWLY_AVAILABLE = 0x00000404,
    MTRClusterIDTypeRelativeHumidityMeasurementID MTR_NEWLY_AVAILABLE = 0x00000405,
    MTRClusterIDTypeOccupancySensingID MTR_NEWLY_AVAILABLE = 0x00000406,
    MTRClusterIDTypeWakeOnLANID MTR_NEWLY_AVAILABLE = 0x00000503,
    MTRClusterIDTypeChannelID MTR_NEWLY_AVAILABLE = 0x00000504,
    MTRClusterIDTypeTargetNavigatorID MTR_NEWLY_AVAILABLE = 0x00000505,
    MTRClusterIDTypeMediaPlaybackID MTR_NEWLY_AVAILABLE = 0x00000506,
    MTRClusterIDTypeMediaInputID MTR_NEWLY_AVAILABLE = 0x00000507,
    MTRClusterIDTypeLowPowerID MTR_NEWLY_AVAILABLE = 0x00000508,
    MTRClusterIDTypeKeypadInputID MTR_NEWLY_AVAILABLE = 0x00000509,
    MTRClusterIDTypeContentLauncherID MTR_NEWLY_AVAILABLE = 0x0000050A,
    MTRClusterIDTypeAudioOutputID MTR_NEWLY_AVAILABLE = 0x0000050B,
    MTRClusterIDTypeApplicationLauncherID MTR_NEWLY_AVAILABLE = 0x0000050C,
    MTRClusterIDTypeApplicationBasicID MTR_NEWLY_AVAILABLE = 0x0000050D,
    MTRClusterIDTypeAccountLoginID MTR_NEWLY_AVAILABLE = 0x0000050E,
    MTRClusterIDTypeElectricalMeasurementID MTR_NEWLY_AVAILABLE = 0x00000B04,
    MTRClusterIDTypeClientMonitoringID MTR_NEWLY_AVAILABLE = 0x00001046,
    MTRClusterIDTypeUnitTestingID MTR_NEWLY_AVAILABLE = 0xFFF1FC05,
    MTRClusterIDTypeFaultInjectionID MTR_NEWLY_AVAILABLE = 0xFFF1FC06,
};

#pragma mark - Attributes IDs

typedef NS_ENUM(uint32_t, MTRAttributeIDType) {
    // Deprecated global attribute names
    MTRClusterGlobalAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID")
    = 0x0000FFF8,
    MTRClusterGlobalAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID")
    = 0x0000FFF9,
    MTRClusterGlobalAttributeAttributeListID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeAttributeListID")
    = 0x0000FFFB,
    MTRClusterGlobalAttributeFeatureMapID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeFeatureMapID")
    = 0x0000FFFC,
    MTRClusterGlobalAttributeClusterRevisionID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeClusterRevisionID")
    = 0x0000FFFD,

    // Global attributes
    MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE = 0x0000FFF8,
    MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE = 0x0000FFF9,
    MTRAttributeIDTypeGlobalAttributeAttributeListID MTR_NEWLY_AVAILABLE = 0x0000FFFB,
    MTRAttributeIDTypeGlobalAttributeFeatureMapID MTR_NEWLY_AVAILABLE = 0x0000FFFC,
    MTRAttributeIDTypeGlobalAttributeClusterRevisionID MTR_NEWLY_AVAILABLE = 0x0000FFFD,

    // Cluster Identify deprecated attribute names
    MTRClusterIdentifyAttributeIdentifyTimeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTimeID")
    = 0x00000000,
    MTRClusterIdentifyAttributeIdentifyTypeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTypeID")
    = 0x00000001,
    MTRClusterIdentifyAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIdentifyAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterIdentifyAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIdentifyAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterIdentifyAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIdentifyAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterIdentifyAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIdentifyAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterIdentifyAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIdentifyAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Identify attributes
    MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTimeID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTypeID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterIdentifyAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterIdentifyAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterIdentifyAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterIdentifyAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterIdentifyAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Groups deprecated attribute names
    MTRClusterGroupsAttributeNameSupportID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeNameSupportID")
    = 0x00000000,
    MTRClusterGroupsAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGroupsAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGroupsAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGroupsAttributeFeatureMapID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGroupsAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupsAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Groups attributes
    MTRAttributeIDTypeClusterGroupsAttributeNameSupportID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterGroupsAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGroupsAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGroupsAttributeAttributeListID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGroupsAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGroupsAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Scenes deprecated attribute names
    MTRClusterScenesAttributeSceneCountID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeSceneCountID")
    = 0x00000000,
    MTRClusterScenesAttributeCurrentSceneID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterScenesAttributeCurrentSceneID")
    = 0x00000001,
    MTRClusterScenesAttributeCurrentGroupID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterScenesAttributeCurrentGroupID")
    = 0x00000002,
    MTRClusterScenesAttributeSceneValidID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeSceneValidID")
    = 0x00000003,
    MTRClusterScenesAttributeNameSupportID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeNameSupportID")
    = 0x00000004,
    MTRClusterScenesAttributeLastConfiguredByID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterScenesAttributeLastConfiguredByID")
    = 0x00000005,
    MTRClusterScenesAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterScenesAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterScenesAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterScenesAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterScenesAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterScenesAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterScenesAttributeFeatureMapID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterScenesAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterScenesAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Scenes attributes
    MTRAttributeIDTypeClusterScenesAttributeSceneCountID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterScenesAttributeCurrentSceneID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterScenesAttributeCurrentGroupID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterScenesAttributeSceneValidID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterScenesAttributeNameSupportID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterScenesAttributeLastConfiguredByID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterScenesAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterScenesAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterScenesAttributeAttributeListID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterScenesAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterScenesAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OnOff deprecated attribute names
    MTRClusterOnOffAttributeOnOffID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeOnOffID") = 0x00000000,
    MTRClusterOnOffAttributeGlobalSceneControlID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffAttributeGlobalSceneControlID")
    = 0x00004000,
    MTRClusterOnOffAttributeOnTimeID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeOnTimeID")
    = 0x00004001,
    MTRClusterOnOffAttributeOffWaitTimeID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeOffWaitTimeID")
    = 0x00004002,
    MTRClusterOnOffAttributeStartUpOnOffID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeStartUpOnOffID")
    = 0x00004003,
    MTRClusterOnOffAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOnOffAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOnOffAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOnOffAttributeFeatureMapID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOnOffAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OnOff attributes
    MTRAttributeIDTypeClusterOnOffAttributeOnOffID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterOnOffAttributeGlobalSceneControlID MTR_NEWLY_AVAILABLE = 0x00004000,
    MTRAttributeIDTypeClusterOnOffAttributeOnTimeID MTR_NEWLY_AVAILABLE = 0x00004001,
    MTRAttributeIDTypeClusterOnOffAttributeOffWaitTimeID MTR_NEWLY_AVAILABLE = 0x00004002,
    MTRAttributeIDTypeClusterOnOffAttributeStartUpOnOffID MTR_NEWLY_AVAILABLE = 0x00004003,
    MTRAttributeIDTypeClusterOnOffAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOnOffAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOnOffAttributeAttributeListID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOnOffAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOnOffAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OnOffSwitchConfiguration deprecated attribute names
    MTRClusterOnOffSwitchConfigurationAttributeSwitchTypeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchTypeID")
    = 0x00000000,
    MTRClusterOnOffSwitchConfigurationAttributeSwitchActionsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchActionsID")
    = 0x00000010,
    MTRClusterOnOffSwitchConfigurationAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOnOffSwitchConfigurationAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOnOffSwitchConfigurationAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOnOffSwitchConfigurationAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOnOffSwitchConfigurationAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OnOffSwitchConfiguration attributes
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchTypeID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchActionsID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster LevelControl deprecated attribute names
    MTRClusterLevelControlAttributeCurrentLevelID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeCurrentLevelID")
    = 0x00000000,
    MTRClusterLevelControlAttributeRemainingTimeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeRemainingTimeID")
    = 0x00000001,
    MTRClusterLevelControlAttributeMinLevelID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeMinLevelID")
    = 0x00000002,
    MTRClusterLevelControlAttributeMaxLevelID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeMaxLevelID")
    = 0x00000003,
    MTRClusterLevelControlAttributeCurrentFrequencyID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeCurrentFrequencyID")
    = 0x00000004,
    MTRClusterLevelControlAttributeMinFrequencyID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeMinFrequencyID")
    = 0x00000005,
    MTRClusterLevelControlAttributeMaxFrequencyID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeMaxFrequencyID")
    = 0x00000006,
    MTRClusterLevelControlAttributeOptionsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeOptionsID")
    = 0x0000000F,
    MTRClusterLevelControlAttributeOnOffTransitionTimeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeOnOffTransitionTimeID")
    = 0x00000010,
    MTRClusterLevelControlAttributeOnLevelID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeOnLevelID")
    = 0x00000011,
    MTRClusterLevelControlAttributeOnTransitionTimeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeOnTransitionTimeID")
    = 0x00000012,
    MTRClusterLevelControlAttributeOffTransitionTimeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeOffTransitionTimeID")
    = 0x00000013,
    MTRClusterLevelControlAttributeDefaultMoveRateID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeDefaultMoveRateID")
    = 0x00000014,
    MTRClusterLevelControlAttributeStartUpCurrentLevelID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeStartUpCurrentLevelID")
    = 0x00004000,
    MTRClusterLevelControlAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterLevelControlAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterLevelControlAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterLevelControlAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterLevelControlAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLevelControlAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster LevelControl attributes
    MTRAttributeIDTypeClusterLevelControlAttributeCurrentLevelID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterLevelControlAttributeRemainingTimeID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterLevelControlAttributeMinLevelID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterLevelControlAttributeMaxLevelID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterLevelControlAttributeCurrentFrequencyID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterLevelControlAttributeMinFrequencyID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterLevelControlAttributeMaxFrequencyID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterLevelControlAttributeOptionsID MTR_NEWLY_AVAILABLE = 0x0000000F,
    MTRAttributeIDTypeClusterLevelControlAttributeOnOffTransitionTimeID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRAttributeIDTypeClusterLevelControlAttributeOnLevelID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterLevelControlAttributeOnTransitionTimeID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRAttributeIDTypeClusterLevelControlAttributeOffTransitionTimeID MTR_NEWLY_AVAILABLE = 0x00000013,
    MTRAttributeIDTypeClusterLevelControlAttributeDefaultMoveRateID MTR_NEWLY_AVAILABLE = 0x00000014,
    MTRAttributeIDTypeClusterLevelControlAttributeStartUpCurrentLevelID MTR_NEWLY_AVAILABLE = 0x00004000,
    MTRAttributeIDTypeClusterLevelControlAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterLevelControlAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterLevelControlAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterLevelControlAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterLevelControlAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BinaryInputBasic deprecated attribute names
    MTRClusterBinaryInputBasicAttributeActiveTextID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeActiveTextID")
    = 0x00000004,
    MTRClusterBinaryInputBasicAttributeDescriptionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeDescriptionID")
    = 0x0000001C,
    MTRClusterBinaryInputBasicAttributeInactiveTextID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeInactiveTextID")
    = 0x0000002E,
    MTRClusterBinaryInputBasicAttributeOutOfServiceID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeOutOfServiceID")
    = 0x00000051,
    MTRClusterBinaryInputBasicAttributePolarityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributePolarityID")
    = 0x00000054,
    MTRClusterBinaryInputBasicAttributePresentValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributePresentValueID")
    = 0x00000055,
    MTRClusterBinaryInputBasicAttributeReliabilityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeReliabilityID")
    = 0x00000067,
    MTRClusterBinaryInputBasicAttributeStatusFlagsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeStatusFlagsID")
    = 0x0000006F,
    MTRClusterBinaryInputBasicAttributeApplicationTypeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeApplicationTypeID")
    = 0x00000100,
    MTRClusterBinaryInputBasicAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBinaryInputBasicAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBinaryInputBasicAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBinaryInputBasicAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBinaryInputBasicAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BinaryInputBasic attributes
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeActiveTextID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeDescriptionID MTR_NEWLY_AVAILABLE = 0x0000001C,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeInactiveTextID MTR_NEWLY_AVAILABLE = 0x0000002E,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeOutOfServiceID MTR_NEWLY_AVAILABLE = 0x00000051,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributePolarityID MTR_NEWLY_AVAILABLE = 0x00000054,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributePresentValueID MTR_NEWLY_AVAILABLE = 0x00000055,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeReliabilityID MTR_NEWLY_AVAILABLE = 0x00000067,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeStatusFlagsID MTR_NEWLY_AVAILABLE = 0x0000006F,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeApplicationTypeID MTR_NEWLY_AVAILABLE = 0x00000100,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PulseWidthModulation deprecated attribute names
    MTRClusterPulseWidthModulationAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPulseWidthModulationAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPulseWidthModulationAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPulseWidthModulationAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPulseWidthModulationAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster PulseWidthModulation attributes
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Descriptor deprecated attribute names
    MTRClusterDescriptorAttributeDeviceTypeListID API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID")
    = 0x00000000,
    MTRClusterDescriptorAttributeDeviceListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID")
    = 0x00000000,
    MTRClusterDescriptorAttributeServerListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDescriptorAttributeServerListID")
    = 0x00000001,
    MTRClusterDescriptorAttributeClientListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDescriptorAttributeClientListID")
    = 0x00000002,
    MTRClusterDescriptorAttributePartsListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDescriptorAttributePartsListID")
    = 0x00000003,
    MTRClusterDescriptorAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDescriptorAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterDescriptorAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDescriptorAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterDescriptorAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDescriptorAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterDescriptorAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDescriptorAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterDescriptorAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDescriptorAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Descriptor attributes
    MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterDescriptorAttributeServerListID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterDescriptorAttributeClientListID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterDescriptorAttributePartsListID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterDescriptorAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDescriptorAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDescriptorAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDescriptorAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDescriptorAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Binding deprecated attribute names
    MTRClusterBindingAttributeBindingID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeBindingID")
    = 0x00000000,
    MTRClusterBindingAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBindingAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBindingAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBindingAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBindingAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBindingAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBindingAttributeFeatureMapID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBindingAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBindingAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Binding attributes
    MTRAttributeIDTypeClusterBindingAttributeBindingID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterBindingAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBindingAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBindingAttributeAttributeListID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBindingAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBindingAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AccessControl deprecated attribute names
    MTRClusterAccessControlAttributeAclID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeAclID")
    = 0x00000000,
    MTRClusterAccessControlAttributeExtensionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeExtensionID")
    = 0x00000001,
    MTRClusterAccessControlAttributeSubjectsPerAccessControlEntryID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeSubjectsPerAccessControlEntryID")
    = 0x00000002,
    MTRClusterAccessControlAttributeTargetsPerAccessControlEntryID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeTargetsPerAccessControlEntryID")
    = 0x00000003,
    MTRClusterAccessControlAttributeAccessControlEntriesPerFabricID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeAccessControlEntriesPerFabricID")
    = 0x00000004,
    MTRClusterAccessControlAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAccessControlAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAccessControlAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAccessControlAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAccessControlAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccessControlAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster AccessControl attributes
    MTRAttributeIDTypeClusterAccessControlAttributeACLID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterAccessControlAttributeExtensionID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterAccessControlAttributeSubjectsPerAccessControlEntryID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterAccessControlAttributeTargetsPerAccessControlEntryID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterAccessControlAttributeAccessControlEntriesPerFabricID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterAccessControlAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAccessControlAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAccessControlAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAccessControlAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAccessControlAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Actions deprecated attribute names
    MTRClusterActionsAttributeActionListID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeActionListID")
    = 0x00000000,
    MTRClusterActionsAttributeEndpointListsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterActionsAttributeEndpointListsID")
    = 0x00000001,
    MTRClusterActionsAttributeSetupURLID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeSetupURLID")
    = 0x00000002,
    MTRClusterActionsAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterActionsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterActionsAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterActionsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterActionsAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterActionsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterActionsAttributeFeatureMapID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterActionsAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterActionsAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Actions attributes
    MTRAttributeIDTypeClusterActionsAttributeActionListID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterActionsAttributeEndpointListsID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterActionsAttributeSetupURLID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterActionsAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterActionsAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterActionsAttributeAttributeListID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterActionsAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterActionsAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Basic deprecated attribute names
    MTRClusterBasicAttributeDataModelRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicAttributeDataModelRevisionID")
    = 0x00000000,
    MTRClusterBasicAttributeVendorNameID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicAttributeVendorNameID")
    = 0x00000001,
    MTRClusterBasicAttributeVendorIDID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicAttributeVendorIDID")
    = 0x00000002,
    MTRClusterBasicAttributeProductNameID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicAttributeProductNameID")
    = 0x00000003,
    MTRClusterBasicAttributeProductIDID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicAttributeProductIDID")
    = 0x00000004,
    MTRClusterBasicAttributeNodeLabelID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicAttributeNodeLabelID")
    = 0x00000005,
    MTRClusterBasicAttributeLocationID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicAttributeLocationID")
    = 0x00000006,
    MTRClusterBasicAttributeHardwareVersionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicAttributeHardwareVersionID")
    = 0x00000007,
    MTRClusterBasicAttributeHardwareVersionStringID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicAttributeHardwareVersionStringID")
    = 0x00000008,
    MTRClusterBasicAttributeSoftwareVersionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicAttributeSoftwareVersionID")
    = 0x00000009,
    MTRClusterBasicAttributeSoftwareVersionStringID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicAttributeSoftwareVersionStringID")
    = 0x0000000A,
    MTRClusterBasicAttributeManufacturingDateID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicAttributeManufacturingDateID")
    = 0x0000000B,
    MTRClusterBasicAttributePartNumberID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicAttributePartNumberID")
    = 0x0000000C,
    MTRClusterBasicAttributeProductURLID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicAttributeProductURLID")
    = 0x0000000D,
    MTRClusterBasicAttributeProductLabelID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicAttributeProductLabelID")
    = 0x0000000E,
    MTRClusterBasicAttributeSerialNumberID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicAttributeSerialNumberID")
    = 0x0000000F,
    MTRClusterBasicAttributeLocalConfigDisabledID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicAttributeLocalConfigDisabledID")
    = 0x00000010,
    MTRClusterBasicAttributeReachableID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicAttributeReachableID")
    = 0x00000011,
    MTRClusterBasicAttributeUniqueIDID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicAttributeUniqueIDID")
    = 0x00000012,
    MTRClusterBasicAttributeCapabilityMinimaID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicAttributeCapabilityMinimaID")
    = 0x00000013,
    MTRClusterBasicAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBasicAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBasicAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBasicAttributeFeatureMapID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBasicAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBasicAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Basic attributes
    MTRAttributeIDTypeClusterBasicAttributeDataModelRevisionID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterBasicAttributeVendorNameID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterBasicAttributeVendorIDID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterBasicAttributeProductNameID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterBasicAttributeProductIDID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterBasicAttributeNodeLabelID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterBasicAttributeLocationID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterBasicAttributeHardwareVersionID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterBasicAttributeHardwareVersionStringID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterBasicAttributeSoftwareVersionID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterBasicAttributeSoftwareVersionStringID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterBasicAttributeManufacturingDateID MTR_NEWLY_AVAILABLE = 0x0000000B,
    MTRAttributeIDTypeClusterBasicAttributePartNumberID MTR_NEWLY_AVAILABLE = 0x0000000C,
    MTRAttributeIDTypeClusterBasicAttributeProductURLID MTR_NEWLY_AVAILABLE = 0x0000000D,
    MTRAttributeIDTypeClusterBasicAttributeProductLabelID MTR_NEWLY_AVAILABLE = 0x0000000E,
    MTRAttributeIDTypeClusterBasicAttributeSerialNumberID MTR_NEWLY_AVAILABLE = 0x0000000F,
    MTRAttributeIDTypeClusterBasicAttributeLocalConfigDisabledID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRAttributeIDTypeClusterBasicAttributeReachableID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterBasicAttributeUniqueIDID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRAttributeIDTypeClusterBasicAttributeCapabilityMinimaID MTR_NEWLY_AVAILABLE = 0x00000013,
    MTRAttributeIDTypeClusterBasicAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBasicAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBasicAttributeAttributeListID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBasicAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBasicAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OtaSoftwareUpdateProvider deprecated attribute names
    MTRClusterOtaSoftwareUpdateProviderAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOtaSoftwareUpdateProviderAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOtaSoftwareUpdateProviderAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOtaSoftwareUpdateProviderAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOtaSoftwareUpdateProviderAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOtaSoftwareUpdateProviderAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOtaSoftwareUpdateProviderAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOtaSoftwareUpdateProviderAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOtaSoftwareUpdateProviderAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOtaSoftwareUpdateProviderAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OTASoftwareUpdateProvider attributes
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OtaSoftwareUpdateRequestor deprecated attribute names
    MTRClusterOtaSoftwareUpdateRequestorAttributeDefaultOtaProvidersID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOtaSoftwareUpdateRequestorAttributeDefaultOtaProvidersID")
    = 0x00000000,
    MTRClusterOtaSoftwareUpdateRequestorAttributeUpdatePossibleID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOtaSoftwareUpdateRequestorAttributeUpdatePossibleID")
    = 0x00000001,
    MTRClusterOtaSoftwareUpdateRequestorAttributeUpdateStateID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOtaSoftwareUpdateRequestorAttributeUpdateStateID")
    = 0x00000002,
    MTRClusterOtaSoftwareUpdateRequestorAttributeUpdateStateProgressID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOtaSoftwareUpdateRequestorAttributeUpdateStateProgressID")
    = 0x00000003,
    MTRClusterOtaSoftwareUpdateRequestorAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOtaSoftwareUpdateRequestorAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOtaSoftwareUpdateRequestorAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOtaSoftwareUpdateRequestorAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOtaSoftwareUpdateRequestorAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOtaSoftwareUpdateRequestorAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OTASoftwareUpdateRequestor attributes
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeDefaultOtaProvidersID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdatePossibleID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateProgressID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster LocalizationConfiguration deprecated attribute names
    MTRClusterLocalizationConfigurationAttributeActiveLocaleID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeActiveLocaleID")
    = 0x00000000,
    MTRClusterLocalizationConfigurationAttributeSupportedLocalesID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeSupportedLocalesID")
    = 0x00000001,
    MTRClusterLocalizationConfigurationAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterLocalizationConfigurationAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterLocalizationConfigurationAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterLocalizationConfigurationAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterLocalizationConfigurationAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster LocalizationConfiguration attributes
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeActiveLocaleID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeSupportedLocalesID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TimeFormatLocalization deprecated attribute names
    MTRClusterTimeFormatLocalizationAttributeHourFormatID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeHourFormatID")
    = 0x00000000,
    MTRClusterTimeFormatLocalizationAttributeActiveCalendarTypeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeActiveCalendarTypeID")
    = 0x00000001,
    MTRClusterTimeFormatLocalizationAttributeSupportedCalendarTypesID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeSupportedCalendarTypesID")
    = 0x00000002,
    MTRClusterTimeFormatLocalizationAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTimeFormatLocalizationAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTimeFormatLocalizationAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTimeFormatLocalizationAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTimeFormatLocalizationAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster TimeFormatLocalization attributes
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeHourFormatID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeActiveCalendarTypeID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeSupportedCalendarTypesID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster UnitLocalization deprecated attribute names
    MTRClusterUnitLocalizationAttributeTemperatureUnitID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeTemperatureUnitID")
    = 0x00000000,
    MTRClusterUnitLocalizationAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterUnitLocalizationAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterUnitLocalizationAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterUnitLocalizationAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterUnitLocalizationAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster UnitLocalization attributes
    MTRAttributeIDTypeClusterUnitLocalizationAttributeTemperatureUnitID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PowerSourceConfiguration deprecated attribute names
    MTRClusterPowerSourceConfigurationAttributeSourcesID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeSourcesID")
    = 0x00000000,
    MTRClusterPowerSourceConfigurationAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPowerSourceConfigurationAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPowerSourceConfigurationAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPowerSourceConfigurationAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPowerSourceConfigurationAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster PowerSourceConfiguration attributes
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeSourcesID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PowerSource deprecated attribute names
    MTRClusterPowerSourceAttributeStatusID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeStatusID")
    = 0x00000000,
    MTRClusterPowerSourceAttributeOrderID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeOrderID")
    = 0x00000001,
    MTRClusterPowerSourceAttributeDescriptionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeDescriptionID")
    = 0x00000002,
    MTRClusterPowerSourceAttributeWiredAssessedInputVoltageID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputVoltageID")
    = 0x00000003,
    MTRClusterPowerSourceAttributeWiredAssessedInputFrequencyID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputFrequencyID")
    = 0x00000004,
    MTRClusterPowerSourceAttributeWiredCurrentTypeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredCurrentTypeID")
    = 0x00000005,
    MTRClusterPowerSourceAttributeWiredAssessedCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedCurrentID")
    = 0x00000006,
    MTRClusterPowerSourceAttributeWiredNominalVoltageID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredNominalVoltageID")
    = 0x00000007,
    MTRClusterPowerSourceAttributeWiredMaximumCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredMaximumCurrentID")
    = 0x00000008,
    MTRClusterPowerSourceAttributeWiredPresentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredPresentID")
    = 0x00000009,
    MTRClusterPowerSourceAttributeActiveWiredFaultsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeActiveWiredFaultsID")
    = 0x0000000A,
    MTRClusterPowerSourceAttributeBatVoltageID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatVoltageID")
    = 0x0000000B,
    MTRClusterPowerSourceAttributeBatPercentRemainingID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatPercentRemainingID")
    = 0x0000000C,
    MTRClusterPowerSourceAttributeBatTimeRemainingID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeRemainingID")
    = 0x0000000D,
    MTRClusterPowerSourceAttributeBatChargeLevelID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeLevelID")
    = 0x0000000E,
    MTRClusterPowerSourceAttributeBatReplacementNeededID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementNeededID")
    = 0x0000000F,
    MTRClusterPowerSourceAttributeBatReplaceabilityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatReplaceabilityID")
    = 0x00000010,
    MTRClusterPowerSourceAttributeBatPresentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatPresentID")
    = 0x00000011,
    MTRClusterPowerSourceAttributeActiveBatFaultsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatFaultsID")
    = 0x00000012,
    MTRClusterPowerSourceAttributeBatReplacementDescriptionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementDescriptionID")
    = 0x00000013,
    MTRClusterPowerSourceAttributeBatCommonDesignationID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatCommonDesignationID")
    = 0x00000014,
    MTRClusterPowerSourceAttributeBatANSIDesignationID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatANSIDesignationID")
    = 0x00000015,
    MTRClusterPowerSourceAttributeBatIECDesignationID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatIECDesignationID")
    = 0x00000016,
    MTRClusterPowerSourceAttributeBatApprovedChemistryID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatApprovedChemistryID")
    = 0x00000017,
    MTRClusterPowerSourceAttributeBatCapacityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatCapacityID")
    = 0x00000018,
    MTRClusterPowerSourceAttributeBatQuantityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatQuantityID")
    = 0x00000019,
    MTRClusterPowerSourceAttributeBatChargeStateID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeStateID")
    = 0x0000001A,
    MTRClusterPowerSourceAttributeBatTimeToFullChargeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeToFullChargeID")
    = 0x0000001B,
    MTRClusterPowerSourceAttributeBatFunctionalWhileChargingID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatFunctionalWhileChargingID")
    = 0x0000001C,
    MTRClusterPowerSourceAttributeBatChargingCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatChargingCurrentID")
    = 0x0000001D,
    MTRClusterPowerSourceAttributeActiveBatChargeFaultsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatChargeFaultsID")
    = 0x0000001E,
    MTRClusterPowerSourceAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPowerSourceAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPowerSourceAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPowerSourceAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPowerSourceAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPowerSourceAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster PowerSource attributes
    MTRAttributeIDTypeClusterPowerSourceAttributeStatusID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterPowerSourceAttributeOrderID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterPowerSourceAttributeDescriptionID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputVoltageID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputFrequencyID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredCurrentTypeID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedCurrentID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredNominalVoltageID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredMaximumCurrentID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredPresentID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterPowerSourceAttributeActiveWiredFaultsID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatVoltageID MTR_NEWLY_AVAILABLE = 0x0000000B,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatPercentRemainingID MTR_NEWLY_AVAILABLE = 0x0000000C,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeRemainingID MTR_NEWLY_AVAILABLE = 0x0000000D,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeLevelID MTR_NEWLY_AVAILABLE = 0x0000000E,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementNeededID MTR_NEWLY_AVAILABLE = 0x0000000F,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatReplaceabilityID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatPresentID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatFaultsID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementDescriptionID MTR_NEWLY_AVAILABLE = 0x00000013,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatCommonDesignationID MTR_NEWLY_AVAILABLE = 0x00000014,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatANSIDesignationID MTR_NEWLY_AVAILABLE = 0x00000015,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatIECDesignationID MTR_NEWLY_AVAILABLE = 0x00000016,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatApprovedChemistryID MTR_NEWLY_AVAILABLE = 0x00000017,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatCapacityID MTR_NEWLY_AVAILABLE = 0x00000018,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatQuantityID MTR_NEWLY_AVAILABLE = 0x00000019,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeStateID MTR_NEWLY_AVAILABLE = 0x0000001A,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeToFullChargeID MTR_NEWLY_AVAILABLE = 0x0000001B,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatFunctionalWhileChargingID MTR_NEWLY_AVAILABLE = 0x0000001C,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatChargingCurrentID MTR_NEWLY_AVAILABLE = 0x0000001D,
    MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatChargeFaultsID MTR_NEWLY_AVAILABLE = 0x0000001E,
    MTRAttributeIDTypeClusterPowerSourceAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPowerSourceAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPowerSourceAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster GeneralCommissioning deprecated attribute names
    MTRClusterGeneralCommissioningAttributeBreadcrumbID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeBreadcrumbID")
    = 0x00000000,
    MTRClusterGeneralCommissioningAttributeBasicCommissioningInfoID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeBasicCommissioningInfoID")
    = 0x00000001,
    MTRClusterGeneralCommissioningAttributeRegulatoryConfigID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeRegulatoryConfigID")
    = 0x00000002,
    MTRClusterGeneralCommissioningAttributeLocationCapabilityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeLocationCapabilityID")
    = 0x00000003,
    MTRClusterGeneralCommissioningAttributeSupportsConcurrentConnectionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeSupportsConcurrentConnectionID")
    = 0x00000004,
    MTRClusterGeneralCommissioningAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGeneralCommissioningAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGeneralCommissioningAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGeneralCommissioningAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGeneralCommissioningAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster GeneralCommissioning attributes
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeBreadcrumbID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeBasicCommissioningInfoID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeRegulatoryConfigID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeLocationCapabilityID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeSupportsConcurrentConnectionID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster NetworkCommissioning deprecated attribute names
    MTRClusterNetworkCommissioningAttributeMaxNetworksID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeMaxNetworksID")
    = 0x00000000,
    MTRClusterNetworkCommissioningAttributeNetworksID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeNetworksID")
    = 0x00000001,
    MTRClusterNetworkCommissioningAttributeScanMaxTimeSecondsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeScanMaxTimeSecondsID")
    = 0x00000002,
    MTRClusterNetworkCommissioningAttributeConnectMaxTimeSecondsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeConnectMaxTimeSecondsID")
    = 0x00000003,
    MTRClusterNetworkCommissioningAttributeInterfaceEnabledID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeInterfaceEnabledID")
    = 0x00000004,
    MTRClusterNetworkCommissioningAttributeLastNetworkingStatusID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkingStatusID")
    = 0x00000005,
    MTRClusterNetworkCommissioningAttributeLastNetworkIDID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkIDID")
    = 0x00000006,
    MTRClusterNetworkCommissioningAttributeLastConnectErrorValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastConnectErrorValueID")
    = 0x00000007,
    MTRClusterNetworkCommissioningAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterNetworkCommissioningAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterNetworkCommissioningAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterNetworkCommissioningAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterNetworkCommissioningAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster NetworkCommissioning attributes
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeMaxNetworksID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeNetworksID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeScanMaxTimeSecondsID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeConnectMaxTimeSecondsID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeInterfaceEnabledID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkingStatusID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkIDID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastConnectErrorValueID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster DiagnosticLogs deprecated attribute names
    MTRClusterDiagnosticLogsAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterDiagnosticLogsAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterDiagnosticLogsAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterDiagnosticLogsAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterDiagnosticLogsAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster DiagnosticLogs attributes
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster GeneralDiagnostics deprecated attribute names
    MTRClusterGeneralDiagnosticsAttributeNetworkInterfacesID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeNetworkInterfacesID")
    = 0x00000000,
    MTRClusterGeneralDiagnosticsAttributeRebootCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeRebootCountID")
    = 0x00000001,
    MTRClusterGeneralDiagnosticsAttributeUpTimeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeUpTimeID")
    = 0x00000002,
    MTRClusterGeneralDiagnosticsAttributeTotalOperationalHoursID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTotalOperationalHoursID")
    = 0x00000003,
    MTRClusterGeneralDiagnosticsAttributeBootReasonsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeBootReasonsID")
    = 0x00000004,
    MTRClusterGeneralDiagnosticsAttributeActiveHardwareFaultsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveHardwareFaultsID")
    = 0x00000005,
    MTRClusterGeneralDiagnosticsAttributeActiveRadioFaultsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveRadioFaultsID")
    = 0x00000006,
    MTRClusterGeneralDiagnosticsAttributeActiveNetworkFaultsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveNetworkFaultsID")
    = 0x00000007,
    MTRClusterGeneralDiagnosticsAttributeTestEventTriggersEnabledID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTestEventTriggersEnabledID")
    = 0x00000008,
    MTRClusterGeneralDiagnosticsAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGeneralDiagnosticsAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGeneralDiagnosticsAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGeneralDiagnosticsAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGeneralDiagnosticsAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster GeneralDiagnostics attributes
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeNetworkInterfacesID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeRebootCountID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeUpTimeID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTotalOperationalHoursID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeBootReasonsID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveHardwareFaultsID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveRadioFaultsID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveNetworkFaultsID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTestEventTriggersEnabledID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster SoftwareDiagnostics deprecated attribute names
    MTRClusterSoftwareDiagnosticsAttributeThreadMetricsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeThreadMetricsID")
    = 0x00000000,
    MTRClusterSoftwareDiagnosticsAttributeCurrentHeapFreeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapFreeID")
    = 0x00000001,
    MTRClusterSoftwareDiagnosticsAttributeCurrentHeapUsedID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapUsedID")
    = 0x00000002,
    MTRClusterSoftwareDiagnosticsAttributeCurrentHeapHighWatermarkID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapHighWatermarkID")
    = 0x00000003,
    MTRClusterSoftwareDiagnosticsAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterSoftwareDiagnosticsAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterSoftwareDiagnosticsAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterSoftwareDiagnosticsAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterSoftwareDiagnosticsAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster SoftwareDiagnostics attributes
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeThreadMetricsID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapFreeID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapUsedID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapHighWatermarkID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ThreadNetworkDiagnostics deprecated attribute names
    MTRClusterThreadNetworkDiagnosticsAttributeChannelID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelID")
    = 0x00000000,
    MTRClusterThreadNetworkDiagnosticsAttributeRoutingRoleID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRoutingRoleID")
    = 0x00000001,
    MTRClusterThreadNetworkDiagnosticsAttributeNetworkNameID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNetworkNameID")
    = 0x00000002,
    MTRClusterThreadNetworkDiagnosticsAttributePanIdID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePanIdID")
    = 0x00000003,
    MTRClusterThreadNetworkDiagnosticsAttributeExtendedPanIdID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeExtendedPanIdID")
    = 0x00000004,
    MTRClusterThreadNetworkDiagnosticsAttributeMeshLocalPrefixID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeMeshLocalPrefixID")
    = 0x00000005,
    MTRClusterThreadNetworkDiagnosticsAttributeOverrunCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOverrunCountID")
    = 0x00000006,
    MTRClusterThreadNetworkDiagnosticsAttributeNeighborTableListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNeighborTableListID")
    = 0x00000007,
    MTRClusterThreadNetworkDiagnosticsAttributeRouteTableListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouteTableListID")
    = 0x00000008,
    MTRClusterThreadNetworkDiagnosticsAttributePartitionIdID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdID")
    = 0x00000009,
    MTRClusterThreadNetworkDiagnosticsAttributeWeightingID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeWeightingID")
    = 0x0000000A,
    MTRClusterThreadNetworkDiagnosticsAttributeDataVersionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDataVersionID")
    = 0x0000000B,
    MTRClusterThreadNetworkDiagnosticsAttributeStableDataVersionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeStableDataVersionID")
    = 0x0000000C,
    MTRClusterThreadNetworkDiagnosticsAttributeLeaderRouterIdID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRouterIdID")
    = 0x0000000D,
    MTRClusterThreadNetworkDiagnosticsAttributeDetachedRoleCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDetachedRoleCountID")
    = 0x0000000E,
    MTRClusterThreadNetworkDiagnosticsAttributeChildRoleCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChildRoleCountID")
    = 0x0000000F,
    MTRClusterThreadNetworkDiagnosticsAttributeRouterRoleCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouterRoleCountID")
    = 0x00000010,
    MTRClusterThreadNetworkDiagnosticsAttributeLeaderRoleCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRoleCountID")
    = 0x00000011,
    MTRClusterThreadNetworkDiagnosticsAttributeAttachAttemptCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttachAttemptCountID")
    = 0x00000012,
    MTRClusterThreadNetworkDiagnosticsAttributePartitionIdChangeCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdChangeCountID")
    = 0x00000013,
    MTRClusterThreadNetworkDiagnosticsAttributeBetterPartitionAttachAttemptCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeBetterPartitionAttachAttemptCountID")
    = 0x00000014,
    MTRClusterThreadNetworkDiagnosticsAttributeParentChangeCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeParentChangeCountID")
    = 0x00000015,
    MTRClusterThreadNetworkDiagnosticsAttributeTxTotalCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxTotalCountID")
    = 0x00000016,
    MTRClusterThreadNetworkDiagnosticsAttributeTxUnicastCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxUnicastCountID")
    = 0x00000017,
    MTRClusterThreadNetworkDiagnosticsAttributeTxBroadcastCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBroadcastCountID")
    = 0x00000018,
    MTRClusterThreadNetworkDiagnosticsAttributeTxAckRequestedCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckRequestedCountID")
    = 0x00000019,
    MTRClusterThreadNetworkDiagnosticsAttributeTxAckedCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckedCountID")
    = 0x0000001A,
    MTRClusterThreadNetworkDiagnosticsAttributeTxNoAckRequestedCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxNoAckRequestedCountID")
    = 0x0000001B,
    MTRClusterThreadNetworkDiagnosticsAttributeTxDataCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataCountID")
    = 0x0000001C,
    MTRClusterThreadNetworkDiagnosticsAttributeTxDataPollCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataPollCountID")
    = 0x0000001D,
    MTRClusterThreadNetworkDiagnosticsAttributeTxBeaconCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconCountID")
    = 0x0000001E,
    MTRClusterThreadNetworkDiagnosticsAttributeTxBeaconRequestCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconRequestCountID")
    = 0x0000001F,
    MTRClusterThreadNetworkDiagnosticsAttributeTxOtherCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxOtherCountID")
    = 0x00000020,
    MTRClusterThreadNetworkDiagnosticsAttributeTxRetryCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxRetryCountID")
    = 0x00000021,
    MTRClusterThreadNetworkDiagnosticsAttributeTxDirectMaxRetryExpiryCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDirectMaxRetryExpiryCountID")
    = 0x00000022,
    MTRClusterThreadNetworkDiagnosticsAttributeTxIndirectMaxRetryExpiryCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxIndirectMaxRetryExpiryCountID")
    = 0x00000023,
    MTRClusterThreadNetworkDiagnosticsAttributeTxErrCcaCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrCcaCountID")
    = 0x00000024,
    MTRClusterThreadNetworkDiagnosticsAttributeTxErrAbortCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrAbortCountID")
    = 0x00000025,
    MTRClusterThreadNetworkDiagnosticsAttributeTxErrBusyChannelCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrBusyChannelCountID")
    = 0x00000026,
    MTRClusterThreadNetworkDiagnosticsAttributeRxTotalCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxTotalCountID")
    = 0x00000027,
    MTRClusterThreadNetworkDiagnosticsAttributeRxUnicastCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxUnicastCountID")
    = 0x00000028,
    MTRClusterThreadNetworkDiagnosticsAttributeRxBroadcastCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBroadcastCountID")
    = 0x00000029,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDataCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataCountID")
    = 0x0000002A,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDataPollCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataPollCountID")
    = 0x0000002B,
    MTRClusterThreadNetworkDiagnosticsAttributeRxBeaconCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconCountID")
    = 0x0000002C,
    MTRClusterThreadNetworkDiagnosticsAttributeRxBeaconRequestCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconRequestCountID")
    = 0x0000002D,
    MTRClusterThreadNetworkDiagnosticsAttributeRxOtherCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxOtherCountID")
    = 0x0000002E,
    MTRClusterThreadNetworkDiagnosticsAttributeRxAddressFilteredCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxAddressFilteredCountID")
    = 0x0000002F,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDestAddrFilteredCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDestAddrFilteredCountID")
    = 0x00000030,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDuplicatedCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDuplicatedCountID")
    = 0x00000031,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrNoFrameCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrNoFrameCountID")
    = 0x00000032,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrUnknownNeighborCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrUnknownNeighborCountID")
    = 0x00000033,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrInvalidSrcAddrCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrInvalidSrcAddrCountID")
    = 0x00000034,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrSecCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrSecCountID")
    = 0x00000035,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrFcsCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrFcsCountID")
    = 0x00000036,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrOtherCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrOtherCountID")
    = 0x00000037,
    MTRClusterThreadNetworkDiagnosticsAttributeActiveTimestampID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveTimestampID")
    = 0x00000038,
    MTRClusterThreadNetworkDiagnosticsAttributePendingTimestampID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePendingTimestampID")
    = 0x00000039,
    MTRClusterThreadNetworkDiagnosticsAttributeDelayID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDelayID")
    = 0x0000003A,
    MTRClusterThreadNetworkDiagnosticsAttributeSecurityPolicyID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeSecurityPolicyID")
    = 0x0000003B,
    MTRClusterThreadNetworkDiagnosticsAttributeChannelPage0MaskID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelPage0MaskID")
    = 0x0000003C,
    MTRClusterThreadNetworkDiagnosticsAttributeOperationalDatasetComponentsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOperationalDatasetComponentsID")
    = 0x0000003D,
    MTRClusterThreadNetworkDiagnosticsAttributeActiveNetworkFaultsListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveNetworkFaultsListID")
    = 0x0000003E,
    MTRClusterThreadNetworkDiagnosticsAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterThreadNetworkDiagnosticsAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterThreadNetworkDiagnosticsAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterThreadNetworkDiagnosticsAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterThreadNetworkDiagnosticsAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ThreadNetworkDiagnostics attributes
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRoutingRoleID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNetworkNameID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePanIdID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeExtendedPanIdID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeMeshLocalPrefixID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOverrunCountID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNeighborTableListID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouteTableListID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeWeightingID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDataVersionID MTR_NEWLY_AVAILABLE = 0x0000000B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeStableDataVersionID MTR_NEWLY_AVAILABLE = 0x0000000C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRouterIdID MTR_NEWLY_AVAILABLE = 0x0000000D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDetachedRoleCountID MTR_NEWLY_AVAILABLE = 0x0000000E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChildRoleCountID MTR_NEWLY_AVAILABLE = 0x0000000F,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouterRoleCountID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRoleCountID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttachAttemptCountID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdChangeCountID MTR_NEWLY_AVAILABLE = 0x00000013,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeBetterPartitionAttachAttemptCountID MTR_NEWLY_AVAILABLE = 0x00000014,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeParentChangeCountID MTR_NEWLY_AVAILABLE = 0x00000015,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxTotalCountID MTR_NEWLY_AVAILABLE = 0x00000016,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxUnicastCountID MTR_NEWLY_AVAILABLE = 0x00000017,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBroadcastCountID MTR_NEWLY_AVAILABLE = 0x00000018,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckRequestedCountID MTR_NEWLY_AVAILABLE = 0x00000019,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckedCountID MTR_NEWLY_AVAILABLE = 0x0000001A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxNoAckRequestedCountID MTR_NEWLY_AVAILABLE = 0x0000001B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataCountID MTR_NEWLY_AVAILABLE = 0x0000001C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataPollCountID MTR_NEWLY_AVAILABLE = 0x0000001D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconCountID MTR_NEWLY_AVAILABLE = 0x0000001E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconRequestCountID MTR_NEWLY_AVAILABLE = 0x0000001F,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxOtherCountID MTR_NEWLY_AVAILABLE = 0x00000020,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxRetryCountID MTR_NEWLY_AVAILABLE = 0x00000021,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDirectMaxRetryExpiryCountID MTR_NEWLY_AVAILABLE = 0x00000022,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxIndirectMaxRetryExpiryCountID MTR_NEWLY_AVAILABLE = 0x00000023,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrCcaCountID MTR_NEWLY_AVAILABLE = 0x00000024,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrAbortCountID MTR_NEWLY_AVAILABLE = 0x00000025,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrBusyChannelCountID MTR_NEWLY_AVAILABLE = 0x00000026,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxTotalCountID MTR_NEWLY_AVAILABLE = 0x00000027,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxUnicastCountID MTR_NEWLY_AVAILABLE = 0x00000028,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBroadcastCountID MTR_NEWLY_AVAILABLE = 0x00000029,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataCountID MTR_NEWLY_AVAILABLE = 0x0000002A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataPollCountID MTR_NEWLY_AVAILABLE = 0x0000002B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconCountID MTR_NEWLY_AVAILABLE = 0x0000002C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconRequestCountID MTR_NEWLY_AVAILABLE = 0x0000002D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxOtherCountID MTR_NEWLY_AVAILABLE = 0x0000002E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxAddressFilteredCountID MTR_NEWLY_AVAILABLE = 0x0000002F,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDestAddrFilteredCountID MTR_NEWLY_AVAILABLE = 0x00000030,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDuplicatedCountID MTR_NEWLY_AVAILABLE = 0x00000031,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrNoFrameCountID MTR_NEWLY_AVAILABLE = 0x00000032,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrUnknownNeighborCountID MTR_NEWLY_AVAILABLE = 0x00000033,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrInvalidSrcAddrCountID MTR_NEWLY_AVAILABLE = 0x00000034,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrSecCountID MTR_NEWLY_AVAILABLE = 0x00000035,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrFcsCountID MTR_NEWLY_AVAILABLE = 0x00000036,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrOtherCountID MTR_NEWLY_AVAILABLE = 0x00000037,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveTimestampID MTR_NEWLY_AVAILABLE = 0x00000038,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePendingTimestampID MTR_NEWLY_AVAILABLE = 0x00000039,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDelayID MTR_NEWLY_AVAILABLE = 0x0000003A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeSecurityPolicyID MTR_NEWLY_AVAILABLE = 0x0000003B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelPage0MaskID MTR_NEWLY_AVAILABLE = 0x0000003C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOperationalDatasetComponentsID MTR_NEWLY_AVAILABLE = 0x0000003D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveNetworkFaultsListID MTR_NEWLY_AVAILABLE = 0x0000003E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster WiFiNetworkDiagnostics deprecated attribute names
    MTRClusterWiFiNetworkDiagnosticsAttributeBssidID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBssidID")
    = 0x00000000,
    MTRClusterWiFiNetworkDiagnosticsAttributeSecurityTypeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeSecurityTypeID")
    = 0x00000001,
    MTRClusterWiFiNetworkDiagnosticsAttributeWiFiVersionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeWiFiVersionID")
    = 0x00000002,
    MTRClusterWiFiNetworkDiagnosticsAttributeChannelNumberID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeChannelNumberID")
    = 0x00000003,
    MTRClusterWiFiNetworkDiagnosticsAttributeRssiID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeRssiID")
    = 0x00000004,
    MTRClusterWiFiNetworkDiagnosticsAttributeBeaconLostCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconLostCountID")
    = 0x00000005,
    MTRClusterWiFiNetworkDiagnosticsAttributeBeaconRxCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconRxCountID")
    = 0x00000006,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketMulticastRxCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastRxCountID")
    = 0x00000007,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketMulticastTxCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastTxCountID")
    = 0x00000008,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketUnicastRxCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastRxCountID")
    = 0x00000009,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketUnicastTxCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastTxCountID")
    = 0x0000000A,
    MTRClusterWiFiNetworkDiagnosticsAttributeCurrentMaxRateID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeCurrentMaxRateID")
    = 0x0000000B,
    MTRClusterWiFiNetworkDiagnosticsAttributeOverrunCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeOverrunCountID")
    = 0x0000000C,
    MTRClusterWiFiNetworkDiagnosticsAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterWiFiNetworkDiagnosticsAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterWiFiNetworkDiagnosticsAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterWiFiNetworkDiagnosticsAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterWiFiNetworkDiagnosticsAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster WiFiNetworkDiagnostics attributes
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBssidID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeSecurityTypeID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeWiFiVersionID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeChannelNumberID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeRssiID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconLostCountID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconRxCountID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastRxCountID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastTxCountID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastRxCountID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastTxCountID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeCurrentMaxRateID MTR_NEWLY_AVAILABLE = 0x0000000B,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeOverrunCountID MTR_NEWLY_AVAILABLE = 0x0000000C,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster EthernetNetworkDiagnostics deprecated attribute names
    MTRClusterEthernetNetworkDiagnosticsAttributePHYRateID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePHYRateID")
    = 0x00000000,
    MTRClusterEthernetNetworkDiagnosticsAttributeFullDuplexID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFullDuplexID")
    = 0x00000001,
    MTRClusterEthernetNetworkDiagnosticsAttributePacketRxCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketRxCountID")
    = 0x00000002,
    MTRClusterEthernetNetworkDiagnosticsAttributePacketTxCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketTxCountID")
    = 0x00000003,
    MTRClusterEthernetNetworkDiagnosticsAttributeTxErrCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTxErrCountID")
    = 0x00000004,
    MTRClusterEthernetNetworkDiagnosticsAttributeCollisionCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCollisionCountID")
    = 0x00000005,
    MTRClusterEthernetNetworkDiagnosticsAttributeOverrunCountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeOverrunCountID")
    = 0x00000006,
    MTRClusterEthernetNetworkDiagnosticsAttributeCarrierDetectID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCarrierDetectID")
    = 0x00000007,
    MTRClusterEthernetNetworkDiagnosticsAttributeTimeSinceResetID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTimeSinceResetID")
    = 0x00000008,
    MTRClusterEthernetNetworkDiagnosticsAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterEthernetNetworkDiagnosticsAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterEthernetNetworkDiagnosticsAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterEthernetNetworkDiagnosticsAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterEthernetNetworkDiagnosticsAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster EthernetNetworkDiagnostics attributes
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePHYRateID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFullDuplexID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketRxCountID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketTxCountID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTxErrCountID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCollisionCountID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeOverrunCountID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCarrierDetectID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTimeSinceResetID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TimeSynchronization deprecated attribute names
    MTRClusterTimeSynchronizationAttributeUTCTimeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeUTCTimeID")
    = 0x00000000,
    MTRClusterTimeSynchronizationAttributeGranularityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeGranularityID")
    = 0x00000001,
    MTRClusterTimeSynchronizationAttributeTimeSourceID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeSourceID")
    = 0x00000002,
    MTRClusterTimeSynchronizationAttributeTrustedTimeNodeIdID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTrustedTimeNodeIdID")
    = 0x00000003,
    MTRClusterTimeSynchronizationAttributeDefaultNtpID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeDefaultNtpID")
    = 0x00000004,
    MTRClusterTimeSynchronizationAttributeTimeZoneID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneID")
    = 0x00000005,
    MTRClusterTimeSynchronizationAttributeDstOffsetID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeDstOffsetID")
    = 0x00000006,
    MTRClusterTimeSynchronizationAttributeLocalTimeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeLocalTimeID")
    = 0x00000007,
    MTRClusterTimeSynchronizationAttributeTimeZoneDatabaseID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneDatabaseID")
    = 0x00000008,
    MTRClusterTimeSynchronizationAttributeNtpServerPortID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeNtpServerPortID")
    = 0x00000009,
    MTRClusterTimeSynchronizationAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTimeSynchronizationAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTimeSynchronizationAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTimeSynchronizationAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTimeSynchronizationAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster TimeSynchronization attributes
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeUTCTimeID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeGranularityID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeSourceID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTrustedTimeNodeIdID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeDefaultNtpID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeDstOffsetID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeLocalTimeID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneDatabaseID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeNtpServerPortID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BridgedDeviceBasic deprecated attribute names
    MTRClusterBridgedDeviceBasicAttributeVendorNameID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeVendorNameID")
    = 0x00000001,
    MTRClusterBridgedDeviceBasicAttributeVendorIDID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeVendorIDID")
    = 0x00000002,
    MTRClusterBridgedDeviceBasicAttributeProductNameID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeProductNameID")
    = 0x00000003,
    MTRClusterBridgedDeviceBasicAttributeNodeLabelID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeNodeLabelID")
    = 0x00000005,
    MTRClusterBridgedDeviceBasicAttributeHardwareVersionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeHardwareVersionID")
    = 0x00000007,
    MTRClusterBridgedDeviceBasicAttributeHardwareVersionStringID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeHardwareVersionStringID")
    = 0x00000008,
    MTRClusterBridgedDeviceBasicAttributeSoftwareVersionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeSoftwareVersionID")
    = 0x00000009,
    MTRClusterBridgedDeviceBasicAttributeSoftwareVersionStringID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeSoftwareVersionStringID")
    = 0x0000000A,
    MTRClusterBridgedDeviceBasicAttributeManufacturingDateID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeManufacturingDateID")
    = 0x0000000B,
    MTRClusterBridgedDeviceBasicAttributePartNumberID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributePartNumberID")
    = 0x0000000C,
    MTRClusterBridgedDeviceBasicAttributeProductURLID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeProductURLID")
    = 0x0000000D,
    MTRClusterBridgedDeviceBasicAttributeProductLabelID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeProductLabelID")
    = 0x0000000E,
    MTRClusterBridgedDeviceBasicAttributeSerialNumberID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeSerialNumberID")
    = 0x0000000F,
    MTRClusterBridgedDeviceBasicAttributeReachableID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeReachableID")
    = 0x00000011,
    MTRClusterBridgedDeviceBasicAttributeUniqueIDID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeUniqueIDID")
    = 0x00000012,
    MTRClusterBridgedDeviceBasicAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBridgedDeviceBasicAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBridgedDeviceBasicAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBridgedDeviceBasicAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBridgedDeviceBasicAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BridgedDeviceBasic attributes
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeVendorNameID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeVendorIDID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeProductNameID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeNodeLabelID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeHardwareVersionID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeHardwareVersionStringID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeSoftwareVersionID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeSoftwareVersionStringID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeManufacturingDateID MTR_NEWLY_AVAILABLE = 0x0000000B,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributePartNumberID MTR_NEWLY_AVAILABLE = 0x0000000C,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeProductURLID MTR_NEWLY_AVAILABLE = 0x0000000D,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeProductLabelID MTR_NEWLY_AVAILABLE = 0x0000000E,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeSerialNumberID MTR_NEWLY_AVAILABLE = 0x0000000F,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeReachableID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeUniqueIDID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Switch deprecated attribute names
    MTRClusterSwitchAttributeNumberOfPositionsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSwitchAttributeNumberOfPositionsID")
    = 0x00000000,
    MTRClusterSwitchAttributeCurrentPositionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSwitchAttributeCurrentPositionID")
    = 0x00000001,
    MTRClusterSwitchAttributeMultiPressMaxID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSwitchAttributeMultiPressMaxID")
    = 0x00000002,
    MTRClusterSwitchAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSwitchAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterSwitchAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSwitchAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterSwitchAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSwitchAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterSwitchAttributeFeatureMapID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterSwitchAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterSwitchAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Switch attributes
    MTRAttributeIDTypeClusterSwitchAttributeNumberOfPositionsID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterSwitchAttributeCurrentPositionID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterSwitchAttributeMultiPressMaxID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterSwitchAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterSwitchAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterSwitchAttributeAttributeListID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterSwitchAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterSwitchAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AdministratorCommissioning deprecated attribute names
    MTRClusterAdministratorCommissioningAttributeWindowStatusID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeWindowStatusID")
    = 0x00000000,
    MTRClusterAdministratorCommissioningAttributeAdminFabricIndexID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminFabricIndexID")
    = 0x00000001,
    MTRClusterAdministratorCommissioningAttributeAdminVendorIdID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminVendorIdID")
    = 0x00000002,
    MTRClusterAdministratorCommissioningAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAdministratorCommissioningAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAdministratorCommissioningAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAdministratorCommissioningAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAdministratorCommissioningAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster AdministratorCommissioning attributes
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeWindowStatusID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminFabricIndexID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminVendorIdID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OperationalCredentials deprecated attribute names
    MTRClusterOperationalCredentialsAttributeNOCsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeNOCsID")
    = 0x00000000,
    MTRClusterOperationalCredentialsAttributeFabricsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeFabricsID")
    = 0x00000001,
    MTRClusterOperationalCredentialsAttributeSupportedFabricsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeSupportedFabricsID")
    = 0x00000002,
    MTRClusterOperationalCredentialsAttributeCommissionedFabricsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeCommissionedFabricsID")
    = 0x00000003,
    MTRClusterOperationalCredentialsAttributeTrustedRootCertificatesID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeTrustedRootCertificatesID")
    = 0x00000004,
    MTRClusterOperationalCredentialsAttributeCurrentFabricIndexID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeCurrentFabricIndexID")
    = 0x00000005,
    MTRClusterOperationalCredentialsAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOperationalCredentialsAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOperationalCredentialsAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOperationalCredentialsAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOperationalCredentialsAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OperationalCredentials attributes
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeNOCsID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeFabricsID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeSupportedFabricsID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeCommissionedFabricsID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeTrustedRootCertificatesID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeCurrentFabricIndexID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster GroupKeyManagement deprecated attribute names
    MTRClusterGroupKeyManagementAttributeGroupKeyMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupKeyMapID")
    = 0x00000000,
    MTRClusterGroupKeyManagementAttributeGroupTableID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupTableID")
    = 0x00000001,
    MTRClusterGroupKeyManagementAttributeMaxGroupsPerFabricID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupsPerFabricID")
    = 0x00000002,
    MTRClusterGroupKeyManagementAttributeMaxGroupKeysPerFabricID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupKeysPerFabricID")
    = 0x00000003,
    MTRClusterGroupKeyManagementAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGroupKeyManagementAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGroupKeyManagementAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGroupKeyManagementAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGroupKeyManagementAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster GroupKeyManagement attributes
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupKeyMapID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupTableID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupsPerFabricID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupKeysPerFabricID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FixedLabel deprecated attribute names
    MTRClusterFixedLabelAttributeLabelListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFixedLabelAttributeLabelListID")
    = 0x00000000,
    MTRClusterFixedLabelAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFixedLabelAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterFixedLabelAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFixedLabelAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterFixedLabelAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFixedLabelAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterFixedLabelAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFixedLabelAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterFixedLabelAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFixedLabelAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster FixedLabel attributes
    MTRAttributeIDTypeClusterFixedLabelAttributeLabelListID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterFixedLabelAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFixedLabelAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFixedLabelAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFixedLabelAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFixedLabelAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster UserLabel deprecated attribute names
    MTRClusterUserLabelAttributeLabelListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUserLabelAttributeLabelListID")
    = 0x00000000,
    MTRClusterUserLabelAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUserLabelAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterUserLabelAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUserLabelAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterUserLabelAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUserLabelAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterUserLabelAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUserLabelAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterUserLabelAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUserLabelAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster UserLabel attributes
    MTRAttributeIDTypeClusterUserLabelAttributeLabelListID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterUserLabelAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterUserLabelAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterUserLabelAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterUserLabelAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterUserLabelAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ProxyConfiguration deprecated attribute names
    MTRClusterProxyConfigurationAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyConfigurationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterProxyConfigurationAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyConfigurationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterProxyConfigurationAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyConfigurationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterProxyConfigurationAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyConfigurationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterProxyConfigurationAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyConfigurationAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ProxyConfiguration attributes
    MTRAttributeIDTypeClusterProxyConfigurationAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterProxyConfigurationAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterProxyConfigurationAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterProxyConfigurationAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterProxyConfigurationAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ProxyDiscovery deprecated attribute names
    MTRClusterProxyDiscoveryAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyDiscoveryAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterProxyDiscoveryAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyDiscoveryAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterProxyDiscoveryAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyDiscoveryAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterProxyDiscoveryAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyDiscoveryAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterProxyDiscoveryAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyDiscoveryAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ProxyDiscovery attributes
    MTRAttributeIDTypeClusterProxyDiscoveryAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterProxyDiscoveryAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterProxyDiscoveryAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterProxyDiscoveryAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterProxyDiscoveryAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ProxyValid deprecated attribute names
    MTRClusterProxyValidAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyValidAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterProxyValidAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyValidAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterProxyValidAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyValidAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterProxyValidAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyValidAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterProxyValidAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterProxyValidAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ProxyValid attributes
    MTRAttributeIDTypeClusterProxyValidAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterProxyValidAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterProxyValidAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterProxyValidAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterProxyValidAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BooleanState deprecated attribute names
    MTRClusterBooleanStateAttributeStateValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBooleanStateAttributeStateValueID")
    = 0x00000000,
    MTRClusterBooleanStateAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBooleanStateAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBooleanStateAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBooleanStateAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBooleanStateAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBooleanStateAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBooleanStateAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBooleanStateAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBooleanStateAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBooleanStateAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BooleanState attributes
    MTRAttributeIDTypeClusterBooleanStateAttributeStateValueID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterBooleanStateAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBooleanStateAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBooleanStateAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBooleanStateAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBooleanStateAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ModeSelect deprecated attribute names
    MTRClusterModeSelectAttributeDescriptionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeDescriptionID")
    = 0x00000000,
    MTRClusterModeSelectAttributeStandardNamespaceID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeStandardNamespaceID")
    = 0x00000001,
    MTRClusterModeSelectAttributeSupportedModesID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeSupportedModesID")
    = 0x00000002,
    MTRClusterModeSelectAttributeCurrentModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeCurrentModeID")
    = 0x00000003,
    MTRClusterModeSelectAttributeStartUpModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeStartUpModeID")
    = 0x00000004,
    MTRClusterModeSelectAttributeOnModeID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeOnModeID")
    = 0x00000005,
    MTRClusterModeSelectAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterModeSelectAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterModeSelectAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterModeSelectAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterModeSelectAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterModeSelectAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ModeSelect attributes
    MTRAttributeIDTypeClusterModeSelectAttributeDescriptionID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterModeSelectAttributeStandardNamespaceID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterModeSelectAttributeSupportedModesID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterModeSelectAttributeCurrentModeID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterModeSelectAttributeStartUpModeID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterModeSelectAttributeOnModeID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterModeSelectAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterModeSelectAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterModeSelectAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterModeSelectAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterModeSelectAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster DoorLock deprecated attribute names
    MTRClusterDoorLockAttributeLockStateID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLockStateID")
    = 0x00000000,
    MTRClusterDoorLockAttributeLockTypeID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLockTypeID")
    = 0x00000001,
    MTRClusterDoorLockAttributeActuatorEnabledID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeActuatorEnabledID")
    = 0x00000002,
    MTRClusterDoorLockAttributeDoorStateID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeDoorStateID")
    = 0x00000003,
    MTRClusterDoorLockAttributeDoorOpenEventsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeDoorOpenEventsID")
    = 0x00000004,
    MTRClusterDoorLockAttributeDoorClosedEventsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeDoorClosedEventsID")
    = 0x00000005,
    MTRClusterDoorLockAttributeOpenPeriodID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeOpenPeriodID")
    = 0x00000006,
    MTRClusterDoorLockAttributeNumberOfTotalUsersSupportedID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfTotalUsersSupportedID")
    = 0x00000011,
    MTRClusterDoorLockAttributeNumberOfPINUsersSupportedID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfPINUsersSupportedID")
    = 0x00000012,
    MTRClusterDoorLockAttributeNumberOfRFIDUsersSupportedID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfRFIDUsersSupportedID")
    = 0x00000013,
    MTRClusterDoorLockAttributeNumberOfWeekDaySchedulesSupportedPerUserID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfWeekDaySchedulesSupportedPerUserID")
    = 0x00000014,
    MTRClusterDoorLockAttributeNumberOfYearDaySchedulesSupportedPerUserID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfYearDaySchedulesSupportedPerUserID")
    = 0x00000015,
    MTRClusterDoorLockAttributeNumberOfHolidaySchedulesSupportedID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfHolidaySchedulesSupportedID")
    = 0x00000016,
    MTRClusterDoorLockAttributeMaxPINCodeLengthID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeMaxPINCodeLengthID")
    = 0x00000017,
    MTRClusterDoorLockAttributeMinPINCodeLengthID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeMinPINCodeLengthID")
    = 0x00000018,
    MTRClusterDoorLockAttributeMaxRFIDCodeLengthID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeMaxRFIDCodeLengthID")
    = 0x00000019,
    MTRClusterDoorLockAttributeMinRFIDCodeLengthID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeMinRFIDCodeLengthID")
    = 0x0000001A,
    MTRClusterDoorLockAttributeCredentialRulesSupportID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeCredentialRulesSupportID")
    = 0x0000001B,
    MTRClusterDoorLockAttributeNumberOfCredentialsSupportedPerUserID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfCredentialsSupportedPerUserID")
    = 0x0000001C,
    MTRClusterDoorLockAttributeLanguageID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLanguageID")
    = 0x00000021,
    MTRClusterDoorLockAttributeLEDSettingsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeLEDSettingsID")
    = 0x00000022,
    MTRClusterDoorLockAttributeAutoRelockTimeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeAutoRelockTimeID")
    = 0x00000023,
    MTRClusterDoorLockAttributeSoundVolumeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeSoundVolumeID")
    = 0x00000024,
    MTRClusterDoorLockAttributeOperatingModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeOperatingModeID")
    = 0x00000025,
    MTRClusterDoorLockAttributeSupportedOperatingModesID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeSupportedOperatingModesID")
    = 0x00000026,
    MTRClusterDoorLockAttributeDefaultConfigurationRegisterID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeDefaultConfigurationRegisterID")
    = 0x00000027,
    MTRClusterDoorLockAttributeEnableLocalProgrammingID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeEnableLocalProgrammingID")
    = 0x00000028,
    MTRClusterDoorLockAttributeEnableOneTouchLockingID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeEnableOneTouchLockingID")
    = 0x00000029,
    MTRClusterDoorLockAttributeEnableInsideStatusLEDID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeEnableInsideStatusLEDID")
    = 0x0000002A,
    MTRClusterDoorLockAttributeEnablePrivacyModeButtonID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeEnablePrivacyModeButtonID")
    = 0x0000002B,
    MTRClusterDoorLockAttributeLocalProgrammingFeaturesID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeLocalProgrammingFeaturesID")
    = 0x0000002C,
    MTRClusterDoorLockAttributeWrongCodeEntryLimitID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeWrongCodeEntryLimitID")
    = 0x00000030,
    MTRClusterDoorLockAttributeUserCodeTemporaryDisableTimeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeUserCodeTemporaryDisableTimeID")
    = 0x00000031,
    MTRClusterDoorLockAttributeSendPINOverTheAirID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeSendPINOverTheAirID")
    = 0x00000032,
    MTRClusterDoorLockAttributeRequirePINforRemoteOperationID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeRequirePINforRemoteOperationID")
    = 0x00000033,
    MTRClusterDoorLockAttributeExpiringUserTimeoutID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeExpiringUserTimeoutID")
    = 0x00000035,
    MTRClusterDoorLockAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterDoorLockAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterDoorLockAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterDoorLockAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterDoorLockAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterDoorLockAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster DoorLock attributes
    MTRAttributeIDTypeClusterDoorLockAttributeLockStateID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterDoorLockAttributeLockTypeID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterDoorLockAttributeActuatorEnabledID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterDoorLockAttributeDoorStateID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterDoorLockAttributeDoorOpenEventsID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterDoorLockAttributeDoorClosedEventsID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterDoorLockAttributeOpenPeriodID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfTotalUsersSupportedID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfPINUsersSupportedID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfRFIDUsersSupportedID MTR_NEWLY_AVAILABLE = 0x00000013,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfWeekDaySchedulesSupportedPerUserID MTR_NEWLY_AVAILABLE = 0x00000014,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfYearDaySchedulesSupportedPerUserID MTR_NEWLY_AVAILABLE = 0x00000015,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfHolidaySchedulesSupportedID MTR_NEWLY_AVAILABLE = 0x00000016,
    MTRAttributeIDTypeClusterDoorLockAttributeMaxPINCodeLengthID MTR_NEWLY_AVAILABLE = 0x00000017,
    MTRAttributeIDTypeClusterDoorLockAttributeMinPINCodeLengthID MTR_NEWLY_AVAILABLE = 0x00000018,
    MTRAttributeIDTypeClusterDoorLockAttributeMaxRFIDCodeLengthID MTR_NEWLY_AVAILABLE = 0x00000019,
    MTRAttributeIDTypeClusterDoorLockAttributeMinRFIDCodeLengthID MTR_NEWLY_AVAILABLE = 0x0000001A,
    MTRAttributeIDTypeClusterDoorLockAttributeCredentialRulesSupportID MTR_NEWLY_AVAILABLE = 0x0000001B,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfCredentialsSupportedPerUserID MTR_NEWLY_AVAILABLE = 0x0000001C,
    MTRAttributeIDTypeClusterDoorLockAttributeLanguageID MTR_NEWLY_AVAILABLE = 0x00000021,
    MTRAttributeIDTypeClusterDoorLockAttributeLEDSettingsID MTR_NEWLY_AVAILABLE = 0x00000022,
    MTRAttributeIDTypeClusterDoorLockAttributeAutoRelockTimeID MTR_NEWLY_AVAILABLE = 0x00000023,
    MTRAttributeIDTypeClusterDoorLockAttributeSoundVolumeID MTR_NEWLY_AVAILABLE = 0x00000024,
    MTRAttributeIDTypeClusterDoorLockAttributeOperatingModeID MTR_NEWLY_AVAILABLE = 0x00000025,
    MTRAttributeIDTypeClusterDoorLockAttributeSupportedOperatingModesID MTR_NEWLY_AVAILABLE = 0x00000026,
    MTRAttributeIDTypeClusterDoorLockAttributeDefaultConfigurationRegisterID MTR_NEWLY_AVAILABLE = 0x00000027,
    MTRAttributeIDTypeClusterDoorLockAttributeEnableLocalProgrammingID MTR_NEWLY_AVAILABLE = 0x00000028,
    MTRAttributeIDTypeClusterDoorLockAttributeEnableOneTouchLockingID MTR_NEWLY_AVAILABLE = 0x00000029,
    MTRAttributeIDTypeClusterDoorLockAttributeEnableInsideStatusLEDID MTR_NEWLY_AVAILABLE = 0x0000002A,
    MTRAttributeIDTypeClusterDoorLockAttributeEnablePrivacyModeButtonID MTR_NEWLY_AVAILABLE = 0x0000002B,
    MTRAttributeIDTypeClusterDoorLockAttributeLocalProgrammingFeaturesID MTR_NEWLY_AVAILABLE = 0x0000002C,
    MTRAttributeIDTypeClusterDoorLockAttributeWrongCodeEntryLimitID MTR_NEWLY_AVAILABLE = 0x00000030,
    MTRAttributeIDTypeClusterDoorLockAttributeUserCodeTemporaryDisableTimeID MTR_NEWLY_AVAILABLE = 0x00000031,
    MTRAttributeIDTypeClusterDoorLockAttributeSendPINOverTheAirID MTR_NEWLY_AVAILABLE = 0x00000032,
    MTRAttributeIDTypeClusterDoorLockAttributeRequirePINforRemoteOperationID MTR_NEWLY_AVAILABLE = 0x00000033,
    MTRAttributeIDTypeClusterDoorLockAttributeExpiringUserTimeoutID MTR_NEWLY_AVAILABLE = 0x00000035,
    MTRAttributeIDTypeClusterDoorLockAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDoorLockAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDoorLockAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDoorLockAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDoorLockAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster WindowCovering deprecated attribute names
    MTRClusterWindowCoveringAttributeTypeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeTypeID")
    = 0x00000000,
    MTRClusterWindowCoveringAttributePhysicalClosedLimitLiftID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitLiftID")
    = 0x00000001,
    MTRClusterWindowCoveringAttributePhysicalClosedLimitTiltID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitTiltID")
    = 0x00000002,
    MTRClusterWindowCoveringAttributeCurrentPositionLiftID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftID")
    = 0x00000003,
    MTRClusterWindowCoveringAttributeCurrentPositionTiltID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltID")
    = 0x00000004,
    MTRClusterWindowCoveringAttributeNumberOfActuationsLiftID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsLiftID")
    = 0x00000005,
    MTRClusterWindowCoveringAttributeNumberOfActuationsTiltID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsTiltID")
    = 0x00000006,
    MTRClusterWindowCoveringAttributeConfigStatusID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeConfigStatusID")
    = 0x00000007,
    MTRClusterWindowCoveringAttributeCurrentPositionLiftPercentageID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercentageID")
    = 0x00000008,
    MTRClusterWindowCoveringAttributeCurrentPositionTiltPercentageID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercentageID")
    = 0x00000009,
    MTRClusterWindowCoveringAttributeOperationalStatusID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeOperationalStatusID")
    = 0x0000000A,
    MTRClusterWindowCoveringAttributeTargetPositionLiftPercent100thsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionLiftPercent100thsID")
    = 0x0000000B,
    MTRClusterWindowCoveringAttributeTargetPositionTiltPercent100thsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionTiltPercent100thsID")
    = 0x0000000C,
    MTRClusterWindowCoveringAttributeEndProductTypeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeEndProductTypeID")
    = 0x0000000D,
    MTRClusterWindowCoveringAttributeCurrentPositionLiftPercent100thsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercent100thsID")
    = 0x0000000E,
    MTRClusterWindowCoveringAttributeCurrentPositionTiltPercent100thsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercent100thsID")
    = 0x0000000F,
    MTRClusterWindowCoveringAttributeInstalledOpenLimitLiftID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitLiftID")
    = 0x00000010,
    MTRClusterWindowCoveringAttributeInstalledClosedLimitLiftID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitLiftID")
    = 0x00000011,
    MTRClusterWindowCoveringAttributeInstalledOpenLimitTiltID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitTiltID")
    = 0x00000012,
    MTRClusterWindowCoveringAttributeInstalledClosedLimitTiltID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitTiltID")
    = 0x00000013,
    MTRClusterWindowCoveringAttributeModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeModeID")
    = 0x00000017,
    MTRClusterWindowCoveringAttributeSafetyStatusID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeSafetyStatusID")
    = 0x0000001A,
    MTRClusterWindowCoveringAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterWindowCoveringAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterWindowCoveringAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterWindowCoveringAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterWindowCoveringAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWindowCoveringAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster WindowCovering attributes
    MTRAttributeIDTypeClusterWindowCoveringAttributeTypeID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitLiftID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitTiltID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsLiftID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsTiltID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterWindowCoveringAttributeConfigStatusID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercentageID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercentageID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterWindowCoveringAttributeOperationalStatusID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionLiftPercent100thsID MTR_NEWLY_AVAILABLE = 0x0000000B,
    MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionTiltPercent100thsID MTR_NEWLY_AVAILABLE = 0x0000000C,
    MTRAttributeIDTypeClusterWindowCoveringAttributeEndProductTypeID MTR_NEWLY_AVAILABLE = 0x0000000D,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercent100thsID MTR_NEWLY_AVAILABLE = 0x0000000E,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercent100thsID MTR_NEWLY_AVAILABLE = 0x0000000F,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitLiftID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitLiftID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitTiltID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitTiltID MTR_NEWLY_AVAILABLE = 0x00000013,
    MTRAttributeIDTypeClusterWindowCoveringAttributeModeID MTR_NEWLY_AVAILABLE = 0x00000017,
    MTRAttributeIDTypeClusterWindowCoveringAttributeSafetyStatusID MTR_NEWLY_AVAILABLE = 0x0000001A,
    MTRAttributeIDTypeClusterWindowCoveringAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BarrierControl deprecated attribute names
    MTRClusterBarrierControlAttributeBarrierMovingStateID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierMovingStateID")
    = 0x00000001,
    MTRClusterBarrierControlAttributeBarrierSafetyStatusID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierSafetyStatusID")
    = 0x00000002,
    MTRClusterBarrierControlAttributeBarrierCapabilitiesID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCapabilitiesID")
    = 0x00000003,
    MTRClusterBarrierControlAttributeBarrierOpenEventsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenEventsID")
    = 0x00000004,
    MTRClusterBarrierControlAttributeBarrierCloseEventsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCloseEventsID")
    = 0x00000005,
    MTRClusterBarrierControlAttributeBarrierCommandOpenEventsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandOpenEventsID")
    = 0x00000006,
    MTRClusterBarrierControlAttributeBarrierCommandCloseEventsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandCloseEventsID")
    = 0x00000007,
    MTRClusterBarrierControlAttributeBarrierOpenPeriodID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenPeriodID")
    = 0x00000008,
    MTRClusterBarrierControlAttributeBarrierClosePeriodID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierClosePeriodID")
    = 0x00000009,
    MTRClusterBarrierControlAttributeBarrierPositionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierPositionID")
    = 0x0000000A,
    MTRClusterBarrierControlAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBarrierControlAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBarrierControlAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBarrierControlAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBarrierControlAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBarrierControlAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BarrierControl attributes
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierMovingStateID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierSafetyStatusID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCapabilitiesID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenEventsID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCloseEventsID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandOpenEventsID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandCloseEventsID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenPeriodID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierClosePeriodID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierPositionID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterBarrierControlAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBarrierControlAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBarrierControlAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBarrierControlAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBarrierControlAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PumpConfigurationAndControl deprecated attribute names
    MTRClusterPumpConfigurationAndControlAttributeMaxPressureID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxPressureID")
    = 0x00000000,
    MTRClusterPumpConfigurationAndControlAttributeMaxSpeedID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxSpeedID")
    = 0x00000001,
    MTRClusterPumpConfigurationAndControlAttributeMaxFlowID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxFlowID")
    = 0x00000002,
    MTRClusterPumpConfigurationAndControlAttributeMinConstPressureID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstPressureID")
    = 0x00000003,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstPressureID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstPressureID")
    = 0x00000004,
    MTRClusterPumpConfigurationAndControlAttributeMinCompPressureID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinCompPressureID")
    = 0x00000005,
    MTRClusterPumpConfigurationAndControlAttributeMaxCompPressureID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxCompPressureID")
    = 0x00000006,
    MTRClusterPumpConfigurationAndControlAttributeMinConstSpeedID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstSpeedID")
    = 0x00000007,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstSpeedID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstSpeedID")
    = 0x00000008,
    MTRClusterPumpConfigurationAndControlAttributeMinConstFlowID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstFlowID")
    = 0x00000009,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstFlowID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstFlowID")
    = 0x0000000A,
    MTRClusterPumpConfigurationAndControlAttributeMinConstTempID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstTempID")
    = 0x0000000B,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstTempID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstTempID")
    = 0x0000000C,
    MTRClusterPumpConfigurationAndControlAttributePumpStatusID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePumpStatusID")
    = 0x00000010,
    MTRClusterPumpConfigurationAndControlAttributeEffectiveOperationModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveOperationModeID")
    = 0x00000011,
    MTRClusterPumpConfigurationAndControlAttributeEffectiveControlModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveControlModeID")
    = 0x00000012,
    MTRClusterPumpConfigurationAndControlAttributeCapacityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeCapacityID")
    = 0x00000013,
    MTRClusterPumpConfigurationAndControlAttributeSpeedID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeSpeedID")
    = 0x00000014,
    MTRClusterPumpConfigurationAndControlAttributeLifetimeRunningHoursID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeRunningHoursID")
    = 0x00000015,
    MTRClusterPumpConfigurationAndControlAttributePowerID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePowerID")
    = 0x00000016,
    MTRClusterPumpConfigurationAndControlAttributeLifetimeEnergyConsumedID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeEnergyConsumedID")
    = 0x00000017,
    MTRClusterPumpConfigurationAndControlAttributeOperationModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeOperationModeID")
    = 0x00000020,
    MTRClusterPumpConfigurationAndControlAttributeControlModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeControlModeID")
    = 0x00000021,
    MTRClusterPumpConfigurationAndControlAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPumpConfigurationAndControlAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPumpConfigurationAndControlAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPumpConfigurationAndControlAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPumpConfigurationAndControlAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster PumpConfigurationAndControl attributes
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxPressureID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxSpeedID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxFlowID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstPressureID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstPressureID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinCompPressureID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxCompPressureID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstSpeedID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstSpeedID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstFlowID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstFlowID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstTempID MTR_NEWLY_AVAILABLE = 0x0000000B,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstTempID MTR_NEWLY_AVAILABLE = 0x0000000C,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePumpStatusID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveOperationModeID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveControlModeID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeCapacityID MTR_NEWLY_AVAILABLE = 0x00000013,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeSpeedID MTR_NEWLY_AVAILABLE = 0x00000014,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeRunningHoursID MTR_NEWLY_AVAILABLE = 0x00000015,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePowerID MTR_NEWLY_AVAILABLE = 0x00000016,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeEnergyConsumedID MTR_NEWLY_AVAILABLE = 0x00000017,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeOperationModeID MTR_NEWLY_AVAILABLE = 0x00000020,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeControlModeID MTR_NEWLY_AVAILABLE = 0x00000021,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Thermostat deprecated attribute names
    MTRClusterThermostatAttributeLocalTemperatureID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureID")
    = 0x00000000,
    MTRClusterThermostatAttributeOutdoorTemperatureID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeOutdoorTemperatureID")
    = 0x00000001,
    MTRClusterThermostatAttributeOccupancyID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeOccupancyID")
    = 0x00000002,
    MTRClusterThermostatAttributeAbsMinHeatSetpointLimitID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMinHeatSetpointLimitID")
    = 0x00000003,
    MTRClusterThermostatAttributeAbsMaxHeatSetpointLimitID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMaxHeatSetpointLimitID")
    = 0x00000004,
    MTRClusterThermostatAttributeAbsMinCoolSetpointLimitID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMinCoolSetpointLimitID")
    = 0x00000005,
    MTRClusterThermostatAttributeAbsMaxCoolSetpointLimitID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMaxCoolSetpointLimitID")
    = 0x00000006,
    MTRClusterThermostatAttributePICoolingDemandID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributePICoolingDemandID")
    = 0x00000007,
    MTRClusterThermostatAttributePIHeatingDemandID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributePIHeatingDemandID")
    = 0x00000008,
    MTRClusterThermostatAttributeHVACSystemTypeConfigurationID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeHVACSystemTypeConfigurationID")
    = 0x00000009,
    MTRClusterThermostatAttributeLocalTemperatureCalibrationID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureCalibrationID")
    = 0x00000010,
    MTRClusterThermostatAttributeOccupiedCoolingSetpointID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedCoolingSetpointID")
    = 0x00000011,
    MTRClusterThermostatAttributeOccupiedHeatingSetpointID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedHeatingSetpointID")
    = 0x00000012,
    MTRClusterThermostatAttributeUnoccupiedCoolingSetpointID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedCoolingSetpointID")
    = 0x00000013,
    MTRClusterThermostatAttributeUnoccupiedHeatingSetpointID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedHeatingSetpointID")
    = 0x00000014,
    MTRClusterThermostatAttributeMinHeatSetpointLimitID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeMinHeatSetpointLimitID")
    = 0x00000015,
    MTRClusterThermostatAttributeMaxHeatSetpointLimitID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeMaxHeatSetpointLimitID")
    = 0x00000016,
    MTRClusterThermostatAttributeMinCoolSetpointLimitID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeMinCoolSetpointLimitID")
    = 0x00000017,
    MTRClusterThermostatAttributeMaxCoolSetpointLimitID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeMaxCoolSetpointLimitID")
    = 0x00000018,
    MTRClusterThermostatAttributeMinSetpointDeadBandID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeMinSetpointDeadBandID")
    = 0x00000019,
    MTRClusterThermostatAttributeRemoteSensingID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeRemoteSensingID")
    = 0x0000001A,
    MTRClusterThermostatAttributeControlSequenceOfOperationID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeControlSequenceOfOperationID")
    = 0x0000001B,
    MTRClusterThermostatAttributeSystemModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeSystemModeID")
    = 0x0000001C,
    MTRClusterThermostatAttributeThermostatRunningModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningModeID")
    = 0x0000001E,
    MTRClusterThermostatAttributeStartOfWeekID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeStartOfWeekID")
    = 0x00000020,
    MTRClusterThermostatAttributeNumberOfWeeklyTransitionsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeNumberOfWeeklyTransitionsID")
    = 0x00000021,
    MTRClusterThermostatAttributeNumberOfDailyTransitionsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeNumberOfDailyTransitionsID")
    = 0x00000022,
    MTRClusterThermostatAttributeTemperatureSetpointHoldID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldID")
    = 0x00000023,
    MTRClusterThermostatAttributeTemperatureSetpointHoldDurationID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldDurationID")
    = 0x00000024,
    MTRClusterThermostatAttributeThermostatProgrammingOperationModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeThermostatProgrammingOperationModeID")
    = 0x00000025,
    MTRClusterThermostatAttributeThermostatRunningStateID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningStateID")
    = 0x00000029,
    MTRClusterThermostatAttributeSetpointChangeSourceID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceID")
    = 0x00000030,
    MTRClusterThermostatAttributeSetpointChangeAmountID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeAmountID")
    = 0x00000031,
    MTRClusterThermostatAttributeSetpointChangeSourceTimestampID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceTimestampID")
    = 0x00000032,
    MTRClusterThermostatAttributeOccupiedSetbackID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackID")
    = 0x00000034,
    MTRClusterThermostatAttributeOccupiedSetbackMinID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMinID")
    = 0x00000035,
    MTRClusterThermostatAttributeOccupiedSetbackMaxID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMaxID")
    = 0x00000036,
    MTRClusterThermostatAttributeUnoccupiedSetbackID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackID")
    = 0x00000037,
    MTRClusterThermostatAttributeUnoccupiedSetbackMinID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMinID")
    = 0x00000038,
    MTRClusterThermostatAttributeUnoccupiedSetbackMaxID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMaxID")
    = 0x00000039,
    MTRClusterThermostatAttributeEmergencyHeatDeltaID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeEmergencyHeatDeltaID")
    = 0x0000003A,
    MTRClusterThermostatAttributeACTypeID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACTypeID")
    = 0x00000040,
    MTRClusterThermostatAttributeACCapacityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeACCapacityID")
    = 0x00000041,
    MTRClusterThermostatAttributeACRefrigerantTypeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeACRefrigerantTypeID")
    = 0x00000042,
    MTRClusterThermostatAttributeACCompressorTypeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeACCompressorTypeID")
    = 0x00000043,
    MTRClusterThermostatAttributeACErrorCodeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeACErrorCodeID")
    = 0x00000044,
    MTRClusterThermostatAttributeACLouverPositionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeACLouverPositionID")
    = 0x00000045,
    MTRClusterThermostatAttributeACCoilTemperatureID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeACCoilTemperatureID")
    = 0x00000046,
    MTRClusterThermostatAttributeACCapacityformatID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeACCapacityformatID")
    = 0x00000047,
    MTRClusterThermostatAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterThermostatAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterThermostatAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterThermostatAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterThermostatAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Thermostat attributes
    MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterThermostatAttributeOutdoorTemperatureID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterThermostatAttributeOccupancyID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMinHeatSetpointLimitID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMaxHeatSetpointLimitID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMinCoolSetpointLimitID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMaxCoolSetpointLimitID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterThermostatAttributePICoolingDemandID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterThermostatAttributePIHeatingDemandID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterThermostatAttributeHVACSystemTypeConfigurationID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureCalibrationID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedCoolingSetpointID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedHeatingSetpointID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedCoolingSetpointID MTR_NEWLY_AVAILABLE = 0x00000013,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedHeatingSetpointID MTR_NEWLY_AVAILABLE = 0x00000014,
    MTRAttributeIDTypeClusterThermostatAttributeMinHeatSetpointLimitID MTR_NEWLY_AVAILABLE = 0x00000015,
    MTRAttributeIDTypeClusterThermostatAttributeMaxHeatSetpointLimitID MTR_NEWLY_AVAILABLE = 0x00000016,
    MTRAttributeIDTypeClusterThermostatAttributeMinCoolSetpointLimitID MTR_NEWLY_AVAILABLE = 0x00000017,
    MTRAttributeIDTypeClusterThermostatAttributeMaxCoolSetpointLimitID MTR_NEWLY_AVAILABLE = 0x00000018,
    MTRAttributeIDTypeClusterThermostatAttributeMinSetpointDeadBandID MTR_NEWLY_AVAILABLE = 0x00000019,
    MTRAttributeIDTypeClusterThermostatAttributeRemoteSensingID MTR_NEWLY_AVAILABLE = 0x0000001A,
    MTRAttributeIDTypeClusterThermostatAttributeControlSequenceOfOperationID MTR_NEWLY_AVAILABLE = 0x0000001B,
    MTRAttributeIDTypeClusterThermostatAttributeSystemModeID MTR_NEWLY_AVAILABLE = 0x0000001C,
    MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningModeID MTR_NEWLY_AVAILABLE = 0x0000001E,
    MTRAttributeIDTypeClusterThermostatAttributeStartOfWeekID MTR_NEWLY_AVAILABLE = 0x00000020,
    MTRAttributeIDTypeClusterThermostatAttributeNumberOfWeeklyTransitionsID MTR_NEWLY_AVAILABLE = 0x00000021,
    MTRAttributeIDTypeClusterThermostatAttributeNumberOfDailyTransitionsID MTR_NEWLY_AVAILABLE = 0x00000022,
    MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldID MTR_NEWLY_AVAILABLE = 0x00000023,
    MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldDurationID MTR_NEWLY_AVAILABLE = 0x00000024,
    MTRAttributeIDTypeClusterThermostatAttributeThermostatProgrammingOperationModeID MTR_NEWLY_AVAILABLE = 0x00000025,
    MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningStateID MTR_NEWLY_AVAILABLE = 0x00000029,
    MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceID MTR_NEWLY_AVAILABLE = 0x00000030,
    MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeAmountID MTR_NEWLY_AVAILABLE = 0x00000031,
    MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceTimestampID MTR_NEWLY_AVAILABLE = 0x00000032,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackID MTR_NEWLY_AVAILABLE = 0x00000034,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMinID MTR_NEWLY_AVAILABLE = 0x00000035,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMaxID MTR_NEWLY_AVAILABLE = 0x00000036,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackID MTR_NEWLY_AVAILABLE = 0x00000037,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMinID MTR_NEWLY_AVAILABLE = 0x00000038,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMaxID MTR_NEWLY_AVAILABLE = 0x00000039,
    MTRAttributeIDTypeClusterThermostatAttributeEmergencyHeatDeltaID MTR_NEWLY_AVAILABLE = 0x0000003A,
    MTRAttributeIDTypeClusterThermostatAttributeACTypeID MTR_NEWLY_AVAILABLE = 0x00000040,
    MTRAttributeIDTypeClusterThermostatAttributeACCapacityID MTR_NEWLY_AVAILABLE = 0x00000041,
    MTRAttributeIDTypeClusterThermostatAttributeACRefrigerantTypeID MTR_NEWLY_AVAILABLE = 0x00000042,
    MTRAttributeIDTypeClusterThermostatAttributeACCompressorTypeID MTR_NEWLY_AVAILABLE = 0x00000043,
    MTRAttributeIDTypeClusterThermostatAttributeACErrorCodeID MTR_NEWLY_AVAILABLE = 0x00000044,
    MTRAttributeIDTypeClusterThermostatAttributeACLouverPositionID MTR_NEWLY_AVAILABLE = 0x00000045,
    MTRAttributeIDTypeClusterThermostatAttributeACCoilTemperatureID MTR_NEWLY_AVAILABLE = 0x00000046,
    MTRAttributeIDTypeClusterThermostatAttributeACCapacityformatID MTR_NEWLY_AVAILABLE = 0x00000047,
    MTRAttributeIDTypeClusterThermostatAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterThermostatAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterThermostatAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterThermostatAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterThermostatAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FanControl deprecated attribute names
    MTRClusterFanControlAttributeFanModeID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeFanModeID")
    = 0x00000000,
    MTRClusterFanControlAttributeFanModeSequenceID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeFanModeSequenceID")
    = 0x00000001,
    MTRClusterFanControlAttributePercentSettingID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributePercentSettingID")
    = 0x00000002,
    MTRClusterFanControlAttributePercentCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributePercentCurrentID")
    = 0x00000003,
    MTRClusterFanControlAttributeSpeedMaxID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeSpeedMaxID")
    = 0x00000004,
    MTRClusterFanControlAttributeSpeedSettingID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeSpeedSettingID")
    = 0x00000005,
    MTRClusterFanControlAttributeSpeedCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeSpeedCurrentID")
    = 0x00000006,
    MTRClusterFanControlAttributeRockSupportID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeRockSupportID")
    = 0x00000007,
    MTRClusterFanControlAttributeRockSettingID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeRockSettingID")
    = 0x00000008,
    MTRClusterFanControlAttributeWindSupportID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeWindSupportID")
    = 0x00000009,
    MTRClusterFanControlAttributeWindSettingID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeWindSettingID")
    = 0x0000000A,
    MTRClusterFanControlAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterFanControlAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterFanControlAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterFanControlAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterFanControlAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFanControlAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster FanControl attributes
    MTRAttributeIDTypeClusterFanControlAttributeFanModeID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterFanControlAttributeFanModeSequenceID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterFanControlAttributePercentSettingID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterFanControlAttributePercentCurrentID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterFanControlAttributeSpeedMaxID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterFanControlAttributeSpeedSettingID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterFanControlAttributeSpeedCurrentID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterFanControlAttributeRockSupportID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterFanControlAttributeRockSettingID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterFanControlAttributeWindSupportID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterFanControlAttributeWindSettingID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterFanControlAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFanControlAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFanControlAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFanControlAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFanControlAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ThermostatUserInterfaceConfiguration deprecated attribute names
    MTRClusterThermostatUserInterfaceConfigurationAttributeTemperatureDisplayModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeTemperatureDisplayModeID")
    = 0x00000000,
    MTRClusterThermostatUserInterfaceConfigurationAttributeKeypadLockoutID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeKeypadLockoutID")
    = 0x00000001,
    MTRClusterThermostatUserInterfaceConfigurationAttributeScheduleProgrammingVisibilityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeScheduleProgrammingVisibilityID")
    = 0x00000002,
    MTRClusterThermostatUserInterfaceConfigurationAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ThermostatUserInterfaceConfiguration attributes
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeTemperatureDisplayModeID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeKeypadLockoutID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeScheduleProgrammingVisibilityID MTR_NEWLY_AVAILABLE
    = 0x00000002,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ColorControl deprecated attribute names
    MTRClusterColorControlAttributeCurrentHueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentHueID")
    = 0x00000000,
    MTRClusterColorControlAttributeCurrentSaturationID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentSaturationID")
    = 0x00000001,
    MTRClusterColorControlAttributeRemainingTimeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeRemainingTimeID")
    = 0x00000002,
    MTRClusterColorControlAttributeCurrentXID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentXID")
    = 0x00000003,
    MTRClusterColorControlAttributeCurrentYID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentYID")
    = 0x00000004,
    MTRClusterColorControlAttributeDriftCompensationID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeDriftCompensationID")
    = 0x00000005,
    MTRClusterColorControlAttributeCompensationTextID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeCompensationTextID")
    = 0x00000006,
    MTRClusterColorControlAttributeColorTemperatureMiredsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorTemperatureMiredsID")
    = 0x00000007,
    MTRClusterColorControlAttributeColorModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorModeID")
    = 0x00000008,
    MTRClusterColorControlAttributeOptionsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeOptionsID")
    = 0x0000000F,
    MTRClusterColorControlAttributeNumberOfPrimariesID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeNumberOfPrimariesID")
    = 0x00000010,
    MTRClusterColorControlAttributePrimary1XID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary1XID")
    = 0x00000011,
    MTRClusterColorControlAttributePrimary1YID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary1YID")
    = 0x00000012,
    MTRClusterColorControlAttributePrimary1IntensityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary1IntensityID")
    = 0x00000013,
    MTRClusterColorControlAttributePrimary2XID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary2XID")
    = 0x00000015,
    MTRClusterColorControlAttributePrimary2YID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary2YID")
    = 0x00000016,
    MTRClusterColorControlAttributePrimary2IntensityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary2IntensityID")
    = 0x00000017,
    MTRClusterColorControlAttributePrimary3XID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary3XID")
    = 0x00000019,
    MTRClusterColorControlAttributePrimary3YID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary3YID")
    = 0x0000001A,
    MTRClusterColorControlAttributePrimary3IntensityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary3IntensityID")
    = 0x0000001B,
    MTRClusterColorControlAttributePrimary4XID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary4XID")
    = 0x00000020,
    MTRClusterColorControlAttributePrimary4YID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary4YID")
    = 0x00000021,
    MTRClusterColorControlAttributePrimary4IntensityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary4IntensityID")
    = 0x00000022,
    MTRClusterColorControlAttributePrimary5XID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary5XID")
    = 0x00000024,
    MTRClusterColorControlAttributePrimary5YID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary5YID")
    = 0x00000025,
    MTRClusterColorControlAttributePrimary5IntensityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary5IntensityID")
    = 0x00000026,
    MTRClusterColorControlAttributePrimary6XID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary6XID")
    = 0x00000028,
    MTRClusterColorControlAttributePrimary6YID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary6YID")
    = 0x00000029,
    MTRClusterColorControlAttributePrimary6IntensityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributePrimary6IntensityID")
    = 0x0000002A,
    MTRClusterColorControlAttributeWhitePointXID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeWhitePointXID")
    = 0x00000030,
    MTRClusterColorControlAttributeWhitePointYID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeWhitePointYID")
    = 0x00000031,
    MTRClusterColorControlAttributeColorPointRXID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointRXID")
    = 0x00000032,
    MTRClusterColorControlAttributeColorPointRYID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointRYID")
    = 0x00000033,
    MTRClusterColorControlAttributeColorPointRIntensityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointRIntensityID")
    = 0x00000034,
    MTRClusterColorControlAttributeColorPointGXID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointGXID")
    = 0x00000036,
    MTRClusterColorControlAttributeColorPointGYID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointGYID")
    = 0x00000037,
    MTRClusterColorControlAttributeColorPointGIntensityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointGIntensityID")
    = 0x00000038,
    MTRClusterColorControlAttributeColorPointBXID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointBXID")
    = 0x0000003A,
    MTRClusterColorControlAttributeColorPointBYID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointBYID")
    = 0x0000003B,
    MTRClusterColorControlAttributeColorPointBIntensityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointBIntensityID")
    = 0x0000003C,
    MTRClusterColorControlAttributeEnhancedCurrentHueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeEnhancedCurrentHueID")
    = 0x00004000,
    MTRClusterColorControlAttributeEnhancedColorModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeEnhancedColorModeID")
    = 0x00004001,
    MTRClusterColorControlAttributeColorLoopActiveID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopActiveID")
    = 0x00004002,
    MTRClusterColorControlAttributeColorLoopDirectionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopDirectionID")
    = 0x00004003,
    MTRClusterColorControlAttributeColorLoopTimeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopTimeID")
    = 0x00004004,
    MTRClusterColorControlAttributeColorLoopStartEnhancedHueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopStartEnhancedHueID")
    = 0x00004005,
    MTRClusterColorControlAttributeColorLoopStoredEnhancedHueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopStoredEnhancedHueID")
    = 0x00004006,
    MTRClusterColorControlAttributeColorCapabilitiesID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorCapabilitiesID")
    = 0x0000400A,
    MTRClusterColorControlAttributeColorTempPhysicalMinMiredsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMinMiredsID")
    = 0x0000400B,
    MTRClusterColorControlAttributeColorTempPhysicalMaxMiredsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMaxMiredsID")
    = 0x0000400C,
    MTRClusterColorControlAttributeCoupleColorTempToLevelMinMiredsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeCoupleColorTempToLevelMinMiredsID")
    = 0x0000400D,
    MTRClusterColorControlAttributeStartUpColorTemperatureMiredsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeStartUpColorTemperatureMiredsID")
    = 0x00004010,
    MTRClusterColorControlAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterColorControlAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterColorControlAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterColorControlAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterColorControlAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterColorControlAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ColorControl attributes
    MTRAttributeIDTypeClusterColorControlAttributeCurrentHueID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterColorControlAttributeCurrentSaturationID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterColorControlAttributeRemainingTimeID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterColorControlAttributeCurrentXID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterColorControlAttributeCurrentYID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterColorControlAttributeDriftCompensationID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterColorControlAttributeCompensationTextID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterColorControlAttributeColorTemperatureMiredsID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterColorControlAttributeColorModeID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterColorControlAttributeOptionsID MTR_NEWLY_AVAILABLE = 0x0000000F,
    MTRAttributeIDTypeClusterColorControlAttributeNumberOfPrimariesID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRAttributeIDTypeClusterColorControlAttributePrimary1XID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterColorControlAttributePrimary1YID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRAttributeIDTypeClusterColorControlAttributePrimary1IntensityID MTR_NEWLY_AVAILABLE = 0x00000013,
    MTRAttributeIDTypeClusterColorControlAttributePrimary2XID MTR_NEWLY_AVAILABLE = 0x00000015,
    MTRAttributeIDTypeClusterColorControlAttributePrimary2YID MTR_NEWLY_AVAILABLE = 0x00000016,
    MTRAttributeIDTypeClusterColorControlAttributePrimary2IntensityID MTR_NEWLY_AVAILABLE = 0x00000017,
    MTRAttributeIDTypeClusterColorControlAttributePrimary3XID MTR_NEWLY_AVAILABLE = 0x00000019,
    MTRAttributeIDTypeClusterColorControlAttributePrimary3YID MTR_NEWLY_AVAILABLE = 0x0000001A,
    MTRAttributeIDTypeClusterColorControlAttributePrimary3IntensityID MTR_NEWLY_AVAILABLE = 0x0000001B,
    MTRAttributeIDTypeClusterColorControlAttributePrimary4XID MTR_NEWLY_AVAILABLE = 0x00000020,
    MTRAttributeIDTypeClusterColorControlAttributePrimary4YID MTR_NEWLY_AVAILABLE = 0x00000021,
    MTRAttributeIDTypeClusterColorControlAttributePrimary4IntensityID MTR_NEWLY_AVAILABLE = 0x00000022,
    MTRAttributeIDTypeClusterColorControlAttributePrimary5XID MTR_NEWLY_AVAILABLE = 0x00000024,
    MTRAttributeIDTypeClusterColorControlAttributePrimary5YID MTR_NEWLY_AVAILABLE = 0x00000025,
    MTRAttributeIDTypeClusterColorControlAttributePrimary5IntensityID MTR_NEWLY_AVAILABLE = 0x00000026,
    MTRAttributeIDTypeClusterColorControlAttributePrimary6XID MTR_NEWLY_AVAILABLE = 0x00000028,
    MTRAttributeIDTypeClusterColorControlAttributePrimary6YID MTR_NEWLY_AVAILABLE = 0x00000029,
    MTRAttributeIDTypeClusterColorControlAttributePrimary6IntensityID MTR_NEWLY_AVAILABLE = 0x0000002A,
    MTRAttributeIDTypeClusterColorControlAttributeWhitePointXID MTR_NEWLY_AVAILABLE = 0x00000030,
    MTRAttributeIDTypeClusterColorControlAttributeWhitePointYID MTR_NEWLY_AVAILABLE = 0x00000031,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointRXID MTR_NEWLY_AVAILABLE = 0x00000032,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointRYID MTR_NEWLY_AVAILABLE = 0x00000033,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointRIntensityID MTR_NEWLY_AVAILABLE = 0x00000034,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointGXID MTR_NEWLY_AVAILABLE = 0x00000036,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointGYID MTR_NEWLY_AVAILABLE = 0x00000037,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointGIntensityID MTR_NEWLY_AVAILABLE = 0x00000038,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointBXID MTR_NEWLY_AVAILABLE = 0x0000003A,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointBYID MTR_NEWLY_AVAILABLE = 0x0000003B,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointBIntensityID MTR_NEWLY_AVAILABLE = 0x0000003C,
    MTRAttributeIDTypeClusterColorControlAttributeEnhancedCurrentHueID MTR_NEWLY_AVAILABLE = 0x00004000,
    MTRAttributeIDTypeClusterColorControlAttributeEnhancedColorModeID MTR_NEWLY_AVAILABLE = 0x00004001,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopActiveID MTR_NEWLY_AVAILABLE = 0x00004002,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopDirectionID MTR_NEWLY_AVAILABLE = 0x00004003,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopTimeID MTR_NEWLY_AVAILABLE = 0x00004004,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopStartEnhancedHueID MTR_NEWLY_AVAILABLE = 0x00004005,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopStoredEnhancedHueID MTR_NEWLY_AVAILABLE = 0x00004006,
    MTRAttributeIDTypeClusterColorControlAttributeColorCapabilitiesID MTR_NEWLY_AVAILABLE = 0x0000400A,
    MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMinMiredsID MTR_NEWLY_AVAILABLE = 0x0000400B,
    MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMaxMiredsID MTR_NEWLY_AVAILABLE = 0x0000400C,
    MTRAttributeIDTypeClusterColorControlAttributeCoupleColorTempToLevelMinMiredsID MTR_NEWLY_AVAILABLE = 0x0000400D,
    MTRAttributeIDTypeClusterColorControlAttributeStartUpColorTemperatureMiredsID MTR_NEWLY_AVAILABLE = 0x00004010,
    MTRAttributeIDTypeClusterColorControlAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterColorControlAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterColorControlAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterColorControlAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterColorControlAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BallastConfiguration deprecated attribute names
    MTRClusterBallastConfigurationAttributePhysicalMinLevelID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMinLevelID")
    = 0x00000000,
    MTRClusterBallastConfigurationAttributePhysicalMaxLevelID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMaxLevelID")
    = 0x00000001,
    MTRClusterBallastConfigurationAttributeBallastStatusID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastStatusID")
    = 0x00000002,
    MTRClusterBallastConfigurationAttributeMinLevelID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeMinLevelID")
    = 0x00000010,
    MTRClusterBallastConfigurationAttributeMaxLevelID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeMaxLevelID")
    = 0x00000011,
    MTRClusterBallastConfigurationAttributeIntrinsicBalanceFactorID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeIntrinsicBalanceFactorID")
    = 0x00000014,
    MTRClusterBallastConfigurationAttributeBallastFactorAdjustmentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastFactorAdjustmentID")
    = 0x00000015,
    MTRClusterBallastConfigurationAttributeLampQuantityID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampQuantityID")
    = 0x00000020,
    MTRClusterBallastConfigurationAttributeLampTypeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampTypeID")
    = 0x00000030,
    MTRClusterBallastConfigurationAttributeLampManufacturerID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampManufacturerID")
    = 0x00000031,
    MTRClusterBallastConfigurationAttributeLampRatedHoursID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampRatedHoursID")
    = 0x00000032,
    MTRClusterBallastConfigurationAttributeLampBurnHoursID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursID")
    = 0x00000033,
    MTRClusterBallastConfigurationAttributeLampAlarmModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampAlarmModeID")
    = 0x00000034,
    MTRClusterBallastConfigurationAttributeLampBurnHoursTripPointID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursTripPointID")
    = 0x00000035,
    MTRClusterBallastConfigurationAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBallastConfigurationAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBallastConfigurationAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBallastConfigurationAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBallastConfigurationAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BallastConfiguration attributes
    MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMinLevelID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMaxLevelID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastStatusID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeMinLevelID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeMaxLevelID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeIntrinsicBalanceFactorID MTR_NEWLY_AVAILABLE = 0x00000014,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastFactorAdjustmentID MTR_NEWLY_AVAILABLE = 0x00000015,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampQuantityID MTR_NEWLY_AVAILABLE = 0x00000020,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampTypeID MTR_NEWLY_AVAILABLE = 0x00000030,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampManufacturerID MTR_NEWLY_AVAILABLE = 0x00000031,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampRatedHoursID MTR_NEWLY_AVAILABLE = 0x00000032,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursID MTR_NEWLY_AVAILABLE = 0x00000033,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampAlarmModeID MTR_NEWLY_AVAILABLE = 0x00000034,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursTripPointID MTR_NEWLY_AVAILABLE = 0x00000035,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster IlluminanceMeasurement deprecated attribute names
    MTRClusterIlluminanceMeasurementAttributeMeasuredValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMeasuredValueID")
    = 0x00000000,
    MTRClusterIlluminanceMeasurementAttributeMinMeasuredValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMinMeasuredValueID")
    = 0x00000001,
    MTRClusterIlluminanceMeasurementAttributeMaxMeasuredValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMaxMeasuredValueID")
    = 0x00000002,
    MTRClusterIlluminanceMeasurementAttributeToleranceID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeToleranceID")
    = 0x00000003,
    MTRClusterIlluminanceMeasurementAttributeLightSensorTypeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeLightSensorTypeID")
    = 0x00000004,
    MTRClusterIlluminanceMeasurementAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterIlluminanceMeasurementAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterIlluminanceMeasurementAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterIlluminanceMeasurementAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterIlluminanceMeasurementAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster IlluminanceMeasurement attributes
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMeasuredValueID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMinMeasuredValueID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMaxMeasuredValueID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeToleranceID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeLightSensorTypeID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TemperatureMeasurement deprecated attribute names
    MTRClusterTemperatureMeasurementAttributeMeasuredValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMeasuredValueID")
    = 0x00000000,
    MTRClusterTemperatureMeasurementAttributeMinMeasuredValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMinMeasuredValueID")
    = 0x00000001,
    MTRClusterTemperatureMeasurementAttributeMaxMeasuredValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMaxMeasuredValueID")
    = 0x00000002,
    MTRClusterTemperatureMeasurementAttributeToleranceID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeToleranceID")
    = 0x00000003,
    MTRClusterTemperatureMeasurementAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTemperatureMeasurementAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTemperatureMeasurementAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTemperatureMeasurementAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTemperatureMeasurementAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster TemperatureMeasurement attributes
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMeasuredValueID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMinMeasuredValueID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMaxMeasuredValueID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeToleranceID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PressureMeasurement deprecated attribute names
    MTRClusterPressureMeasurementAttributeMeasuredValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMeasuredValueID")
    = 0x00000000,
    MTRClusterPressureMeasurementAttributeMinMeasuredValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMinMeasuredValueID")
    = 0x00000001,
    MTRClusterPressureMeasurementAttributeMaxMeasuredValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxMeasuredValueID")
    = 0x00000002,
    MTRClusterPressureMeasurementAttributeToleranceID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeToleranceID")
    = 0x00000003,
    MTRClusterPressureMeasurementAttributeScaledValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledValueID")
    = 0x00000010,
    MTRClusterPressureMeasurementAttributeMinScaledValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMinScaledValueID")
    = 0x00000011,
    MTRClusterPressureMeasurementAttributeMaxScaledValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxScaledValueID")
    = 0x00000012,
    MTRClusterPressureMeasurementAttributeScaledToleranceID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledToleranceID")
    = 0x00000013,
    MTRClusterPressureMeasurementAttributeScaleID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeScaleID")
    = 0x00000014,
    MTRClusterPressureMeasurementAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPressureMeasurementAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPressureMeasurementAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPressureMeasurementAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPressureMeasurementAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster PressureMeasurement attributes
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMeasuredValueID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMinMeasuredValueID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxMeasuredValueID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeToleranceID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledValueID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMinScaledValueID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxScaledValueID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledToleranceID MTR_NEWLY_AVAILABLE = 0x00000013,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeScaleID MTR_NEWLY_AVAILABLE = 0x00000014,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FlowMeasurement deprecated attribute names
    MTRClusterFlowMeasurementAttributeMeasuredValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeMeasuredValueID")
    = 0x00000000,
    MTRClusterFlowMeasurementAttributeMinMeasuredValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeMinMeasuredValueID")
    = 0x00000001,
    MTRClusterFlowMeasurementAttributeMaxMeasuredValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeMaxMeasuredValueID")
    = 0x00000002,
    MTRClusterFlowMeasurementAttributeToleranceID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeToleranceID")
    = 0x00000003,
    MTRClusterFlowMeasurementAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterFlowMeasurementAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterFlowMeasurementAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterFlowMeasurementAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterFlowMeasurementAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster FlowMeasurement attributes
    MTRAttributeIDTypeClusterFlowMeasurementAttributeMeasuredValueID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeMinMeasuredValueID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeMaxMeasuredValueID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeToleranceID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster RelativeHumidityMeasurement deprecated attribute names
    MTRClusterRelativeHumidityMeasurementAttributeMeasuredValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMeasuredValueID")
    = 0x00000000,
    MTRClusterRelativeHumidityMeasurementAttributeMinMeasuredValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMinMeasuredValueID")
    = 0x00000001,
    MTRClusterRelativeHumidityMeasurementAttributeMaxMeasuredValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMaxMeasuredValueID")
    = 0x00000002,
    MTRClusterRelativeHumidityMeasurementAttributeToleranceID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeToleranceID")
    = 0x00000003,
    MTRClusterRelativeHumidityMeasurementAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterRelativeHumidityMeasurementAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterRelativeHumidityMeasurementAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterRelativeHumidityMeasurementAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterRelativeHumidityMeasurementAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster RelativeHumidityMeasurement attributes
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMeasuredValueID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMinMeasuredValueID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMaxMeasuredValueID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeToleranceID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OccupancySensing deprecated attribute names
    MTRClusterOccupancySensingAttributeOccupancyID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancyID")
    = 0x00000000,
    MTRClusterOccupancySensingAttributeOccupancySensorTypeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeID")
    = 0x00000001,
    MTRClusterOccupancySensingAttributeOccupancySensorTypeBitmapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeBitmapID")
    = 0x00000002,
    MTRClusterOccupancySensingAttributePirOccupiedToUnoccupiedDelayID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributePirOccupiedToUnoccupiedDelayID")
    = 0x00000010,
    MTRClusterOccupancySensingAttributePirUnoccupiedToOccupiedDelayID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributePirUnoccupiedToOccupiedDelayID")
    = 0x00000011,
    MTRClusterOccupancySensingAttributePirUnoccupiedToOccupiedThresholdID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributePirUnoccupiedToOccupiedThresholdID")
    = 0x00000012,
    MTRClusterOccupancySensingAttributeUltrasonicOccupiedToUnoccupiedDelayID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicOccupiedToUnoccupiedDelayID")
    = 0x00000020,
    MTRClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedDelayID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedDelayID")
    = 0x00000021,
    MTRClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedThresholdID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedThresholdID")
    = 0x00000022,
    MTRClusterOccupancySensingAttributePhysicalContactOccupiedToUnoccupiedDelayID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactOccupiedToUnoccupiedDelayID")
    = 0x00000030,
    MTRClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedDelayID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedDelayID")
    = 0x00000031,
    MTRClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedThresholdID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedThresholdID")
    = 0x00000032,
    MTRClusterOccupancySensingAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOccupancySensingAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOccupancySensingAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOccupancySensingAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOccupancySensingAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OccupancySensing attributes
    MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancyID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeBitmapID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterOccupancySensingAttributePIROccupiedToUnoccupiedDelayID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedDelayID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedThresholdID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicOccupiedToUnoccupiedDelayID MTR_NEWLY_AVAILABLE = 0x00000020,
    MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedDelayID MTR_NEWLY_AVAILABLE = 0x00000021,
    MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedThresholdID MTR_NEWLY_AVAILABLE = 0x00000022,
    MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactOccupiedToUnoccupiedDelayID MTR_NEWLY_AVAILABLE = 0x00000030,
    MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedDelayID MTR_NEWLY_AVAILABLE = 0x00000031,
    MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedThresholdID MTR_NEWLY_AVAILABLE
    = 0x00000032,
    MTRAttributeIDTypeClusterOccupancySensingAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster WakeOnLan deprecated attribute names
    MTRClusterWakeOnLanAttributeMACAddressID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWakeOnLanAttributeMACAddressID")
    = 0x00000000,
    MTRClusterWakeOnLanAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWakeOnLanAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterWakeOnLanAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWakeOnLanAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterWakeOnLanAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWakeOnLanAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterWakeOnLanAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWakeOnLanAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterWakeOnLanAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterWakeOnLanAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster WakeOnLAN attributes
    MTRAttributeIDTypeClusterWakeOnLANAttributeMACAddressID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterWakeOnLANAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Channel deprecated attribute names
    MTRClusterChannelAttributeChannelListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterChannelAttributeChannelListID")
    = 0x00000000,
    MTRClusterChannelAttributeLineupID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeLineupID")
    = 0x00000001,
    MTRClusterChannelAttributeCurrentChannelID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterChannelAttributeCurrentChannelID")
    = 0x00000002,
    MTRClusterChannelAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterChannelAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterChannelAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterChannelAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterChannelAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterChannelAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterChannelAttributeFeatureMapID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterChannelAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterChannelAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Channel attributes
    MTRAttributeIDTypeClusterChannelAttributeChannelListID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterChannelAttributeLineupID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterChannelAttributeCurrentChannelID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterChannelAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterChannelAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterChannelAttributeAttributeListID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterChannelAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterChannelAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TargetNavigator deprecated attribute names
    MTRClusterTargetNavigatorAttributeTargetListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeTargetListID")
    = 0x00000000,
    MTRClusterTargetNavigatorAttributeCurrentTargetID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeCurrentTargetID")
    = 0x00000001,
    MTRClusterTargetNavigatorAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTargetNavigatorAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTargetNavigatorAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTargetNavigatorAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTargetNavigatorAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster TargetNavigator attributes
    MTRAttributeIDTypeClusterTargetNavigatorAttributeTargetListID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeCurrentTargetID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster MediaPlayback deprecated attribute names
    MTRClusterMediaPlaybackAttributeCurrentStateID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeCurrentStateID")
    = 0x00000000,
    MTRClusterMediaPlaybackAttributeStartTimeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeStartTimeID")
    = 0x00000001,
    MTRClusterMediaPlaybackAttributeDurationID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeDurationID")
    = 0x00000002,
    MTRClusterMediaPlaybackAttributeSampledPositionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeSampledPositionID")
    = 0x00000003,
    MTRClusterMediaPlaybackAttributePlaybackSpeedID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributePlaybackSpeedID")
    = 0x00000004,
    MTRClusterMediaPlaybackAttributeSeekRangeEndID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeEndID")
    = 0x00000005,
    MTRClusterMediaPlaybackAttributeSeekRangeStartID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeStartID")
    = 0x00000006,
    MTRClusterMediaPlaybackAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterMediaPlaybackAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterMediaPlaybackAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterMediaPlaybackAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterMediaPlaybackAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster MediaPlayback attributes
    MTRAttributeIDTypeClusterMediaPlaybackAttributeCurrentStateID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeStartTimeID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeDurationID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeSampledPositionID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterMediaPlaybackAttributePlaybackSpeedID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeEndID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeStartID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster MediaInput deprecated attribute names
    MTRClusterMediaInputAttributeInputListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaInputAttributeInputListID")
    = 0x00000000,
    MTRClusterMediaInputAttributeCurrentInputID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaInputAttributeCurrentInputID")
    = 0x00000001,
    MTRClusterMediaInputAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaInputAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterMediaInputAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaInputAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterMediaInputAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaInputAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterMediaInputAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaInputAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterMediaInputAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterMediaInputAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster MediaInput attributes
    MTRAttributeIDTypeClusterMediaInputAttributeInputListID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterMediaInputAttributeCurrentInputID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterMediaInputAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterMediaInputAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterMediaInputAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterMediaInputAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterMediaInputAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster LowPower deprecated attribute names
    MTRClusterLowPowerAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLowPowerAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterLowPowerAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLowPowerAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterLowPowerAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLowPowerAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterLowPowerAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLowPowerAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterLowPowerAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterLowPowerAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster LowPower attributes
    MTRAttributeIDTypeClusterLowPowerAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterLowPowerAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterLowPowerAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterLowPowerAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterLowPowerAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster KeypadInput deprecated attribute names
    MTRClusterKeypadInputAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterKeypadInputAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterKeypadInputAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterKeypadInputAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterKeypadInputAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterKeypadInputAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterKeypadInputAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterKeypadInputAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterKeypadInputAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterKeypadInputAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster KeypadInput attributes
    MTRAttributeIDTypeClusterKeypadInputAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterKeypadInputAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterKeypadInputAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterKeypadInputAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterKeypadInputAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ContentLauncher deprecated attribute names
    MTRClusterContentLauncherAttributeAcceptHeaderID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterContentLauncherAttributeAcceptHeaderID")
    = 0x00000000,
    MTRClusterContentLauncherAttributeSupportedStreamingProtocolsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterContentLauncherAttributeSupportedStreamingProtocolsID")
    = 0x00000001,
    MTRClusterContentLauncherAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterContentLauncherAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterContentLauncherAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterContentLauncherAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterContentLauncherAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterContentLauncherAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterContentLauncherAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterContentLauncherAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterContentLauncherAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterContentLauncherAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ContentLauncher attributes
    MTRAttributeIDTypeClusterContentLauncherAttributeAcceptHeaderID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterContentLauncherAttributeSupportedStreamingProtocolsID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterContentLauncherAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterContentLauncherAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterContentLauncherAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterContentLauncherAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterContentLauncherAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AudioOutput deprecated attribute names
    MTRClusterAudioOutputAttributeOutputListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAudioOutputAttributeOutputListID")
    = 0x00000000,
    MTRClusterAudioOutputAttributeCurrentOutputID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAudioOutputAttributeCurrentOutputID")
    = 0x00000001,
    MTRClusterAudioOutputAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAudioOutputAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAudioOutputAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAudioOutputAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAudioOutputAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAudioOutputAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAudioOutputAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAudioOutputAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAudioOutputAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAudioOutputAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster AudioOutput attributes
    MTRAttributeIDTypeClusterAudioOutputAttributeOutputListID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterAudioOutputAttributeCurrentOutputID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterAudioOutputAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAudioOutputAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAudioOutputAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAudioOutputAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAudioOutputAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ApplicationLauncher deprecated attribute names
    MTRClusterApplicationLauncherAttributeCatalogListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeCatalogListID")
    = 0x00000000,
    MTRClusterApplicationLauncherAttributeCurrentAppID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeCurrentAppID")
    = 0x00000001,
    MTRClusterApplicationLauncherAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterApplicationLauncherAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterApplicationLauncherAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterApplicationLauncherAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterApplicationLauncherAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ApplicationLauncher attributes
    MTRAttributeIDTypeClusterApplicationLauncherAttributeCatalogListID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeCurrentAppID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ApplicationBasic deprecated attribute names
    MTRClusterApplicationBasicAttributeVendorNameID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeVendorNameID")
    = 0x00000000,
    MTRClusterApplicationBasicAttributeVendorIDID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeVendorIDID")
    = 0x00000001,
    MTRClusterApplicationBasicAttributeApplicationNameID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationNameID")
    = 0x00000002,
    MTRClusterApplicationBasicAttributeProductIDID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeProductIDID")
    = 0x00000003,
    MTRClusterApplicationBasicAttributeApplicationID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationID")
    = 0x00000004,
    MTRClusterApplicationBasicAttributeStatusID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeStatusID")
    = 0x00000005,
    MTRClusterApplicationBasicAttributeApplicationVersionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationVersionID")
    = 0x00000006,
    MTRClusterApplicationBasicAttributeAllowedVendorListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeAllowedVendorListID")
    = 0x00000007,
    MTRClusterApplicationBasicAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterApplicationBasicAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterApplicationBasicAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterApplicationBasicAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterApplicationBasicAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterApplicationBasicAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ApplicationBasic attributes
    MTRAttributeIDTypeClusterApplicationBasicAttributeVendorNameID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterApplicationBasicAttributeVendorIDID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationNameID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterApplicationBasicAttributeProductIDID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterApplicationBasicAttributeStatusID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationVersionID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterApplicationBasicAttributeAllowedVendorListID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterApplicationBasicAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AccountLogin deprecated attribute names
    MTRClusterAccountLoginAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccountLoginAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAccountLoginAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccountLoginAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAccountLoginAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccountLoginAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAccountLoginAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccountLoginAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAccountLoginAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterAccountLoginAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster AccountLogin attributes
    MTRAttributeIDTypeClusterAccountLoginAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAccountLoginAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAccountLoginAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAccountLoginAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAccountLoginAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ElectricalMeasurement deprecated attribute names
    MTRClusterElectricalMeasurementAttributeMeasurementTypeID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasurementTypeID")
    = 0x00000000,
    MTRClusterElectricalMeasurementAttributeDcVoltageID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageID")
    = 0x00000100,
    MTRClusterElectricalMeasurementAttributeDcVoltageMinID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMinID")
    = 0x00000101,
    MTRClusterElectricalMeasurementAttributeDcVoltageMaxID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMaxID")
    = 0x00000102,
    MTRClusterElectricalMeasurementAttributeDcCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentID")
    = 0x00000103,
    MTRClusterElectricalMeasurementAttributeDcCurrentMinID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMinID")
    = 0x00000104,
    MTRClusterElectricalMeasurementAttributeDcCurrentMaxID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMaxID")
    = 0x00000105,
    MTRClusterElectricalMeasurementAttributeDcPowerID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerID")
    = 0x00000106,
    MTRClusterElectricalMeasurementAttributeDcPowerMinID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMinID")
    = 0x00000107,
    MTRClusterElectricalMeasurementAttributeDcPowerMaxID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMaxID")
    = 0x00000108,
    MTRClusterElectricalMeasurementAttributeDcVoltageMultiplierID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMultiplierID")
    = 0x00000200,
    MTRClusterElectricalMeasurementAttributeDcVoltageDivisorID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageDivisorID")
    = 0x00000201,
    MTRClusterElectricalMeasurementAttributeDcCurrentMultiplierID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMultiplierID")
    = 0x00000202,
    MTRClusterElectricalMeasurementAttributeDcCurrentDivisorID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentDivisorID")
    = 0x00000203,
    MTRClusterElectricalMeasurementAttributeDcPowerMultiplierID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMultiplierID")
    = 0x00000204,
    MTRClusterElectricalMeasurementAttributeDcPowerDivisorID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerDivisorID")
    = 0x00000205,
    MTRClusterElectricalMeasurementAttributeAcFrequencyID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyID")
    = 0x00000300,
    MTRClusterElectricalMeasurementAttributeAcFrequencyMinID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMinID")
    = 0x00000301,
    MTRClusterElectricalMeasurementAttributeAcFrequencyMaxID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMaxID")
    = 0x00000302,
    MTRClusterElectricalMeasurementAttributeNeutralCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeNeutralCurrentID")
    = 0x00000303,
    MTRClusterElectricalMeasurementAttributeTotalActivePowerID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalActivePowerID")
    = 0x00000304,
    MTRClusterElectricalMeasurementAttributeTotalReactivePowerID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalReactivePowerID")
    = 0x00000305,
    MTRClusterElectricalMeasurementAttributeTotalApparentPowerID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalApparentPowerID")
    = 0x00000306,
    MTRClusterElectricalMeasurementAttributeMeasured1stHarmonicCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured1stHarmonicCurrentID")
    = 0x00000307,
    MTRClusterElectricalMeasurementAttributeMeasured3rdHarmonicCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured3rdHarmonicCurrentID")
    = 0x00000308,
    MTRClusterElectricalMeasurementAttributeMeasured5thHarmonicCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured5thHarmonicCurrentID")
    = 0x00000309,
    MTRClusterElectricalMeasurementAttributeMeasured7thHarmonicCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured7thHarmonicCurrentID")
    = 0x0000030A,
    MTRClusterElectricalMeasurementAttributeMeasured9thHarmonicCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured9thHarmonicCurrentID")
    = 0x0000030B,
    MTRClusterElectricalMeasurementAttributeMeasured11thHarmonicCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured11thHarmonicCurrentID")
    = 0x0000030C,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase1stHarmonicCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase1stHarmonicCurrentID")
    = 0x0000030D,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase3rdHarmonicCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase3rdHarmonicCurrentID")
    = 0x0000030E,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase5thHarmonicCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase5thHarmonicCurrentID")
    = 0x0000030F,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase7thHarmonicCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase7thHarmonicCurrentID")
    = 0x00000310,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase9thHarmonicCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase9thHarmonicCurrentID")
    = 0x00000311,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase11thHarmonicCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase11thHarmonicCurrentID")
    = 0x00000312,
    MTRClusterElectricalMeasurementAttributeAcFrequencyMultiplierID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMultiplierID")
    = 0x00000400,
    MTRClusterElectricalMeasurementAttributeAcFrequencyDivisorID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyDivisorID")
    = 0x00000401,
    MTRClusterElectricalMeasurementAttributePowerMultiplierID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerMultiplierID")
    = 0x00000402,
    MTRClusterElectricalMeasurementAttributePowerDivisorID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerDivisorID")
    = 0x00000403,
    MTRClusterElectricalMeasurementAttributeHarmonicCurrentMultiplierID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeHarmonicCurrentMultiplierID")
    = 0x00000404,
    MTRClusterElectricalMeasurementAttributePhaseHarmonicCurrentMultiplierID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePhaseHarmonicCurrentMultiplierID")
    = 0x00000405,
    MTRClusterElectricalMeasurementAttributeInstantaneousVoltageID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousVoltageID")
    = 0x00000500,
    MTRClusterElectricalMeasurementAttributeInstantaneousLineCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousLineCurrentID")
    = 0x00000501,
    MTRClusterElectricalMeasurementAttributeInstantaneousActiveCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousActiveCurrentID")
    = 0x00000502,
    MTRClusterElectricalMeasurementAttributeInstantaneousReactiveCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousReactiveCurrentID")
    = 0x00000503,
    MTRClusterElectricalMeasurementAttributeInstantaneousPowerID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousPowerID")
    = 0x00000504,
    MTRClusterElectricalMeasurementAttributeRmsVoltageID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageID")
    = 0x00000505,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMinID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinID")
    = 0x00000506,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMaxID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxID")
    = 0x00000507,
    MTRClusterElectricalMeasurementAttributeRmsCurrentID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentID")
    = 0x00000508,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMinID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinID")
    = 0x00000509,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMaxID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxID")
    = 0x0000050A,
    MTRClusterElectricalMeasurementAttributeActivePowerID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerID")
    = 0x0000050B,
    MTRClusterElectricalMeasurementAttributeActivePowerMinID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinID")
    = 0x0000050C,
    MTRClusterElectricalMeasurementAttributeActivePowerMaxID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxID")
    = 0x0000050D,
    MTRClusterElectricalMeasurementAttributeReactivePowerID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerID")
    = 0x0000050E,
    MTRClusterElectricalMeasurementAttributeApparentPowerID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerID")
    = 0x0000050F,
    MTRClusterElectricalMeasurementAttributePowerFactorID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorID")
    = 0x00000510,
    MTRClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodID")
    = 0x00000511,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterID")
    = 0x00000513,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodID")
    = 0x00000514,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodID")
    = 0x00000515,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagPeriodID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodID")
    = 0x00000516,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodID")
    = 0x00000517,
    MTRClusterElectricalMeasurementAttributeAcVoltageMultiplierID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageMultiplierID")
    = 0x00000600,
    MTRClusterElectricalMeasurementAttributeAcVoltageDivisorID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageDivisorID")
    = 0x00000601,
    MTRClusterElectricalMeasurementAttributeAcCurrentMultiplierID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentMultiplierID")
    = 0x00000602,
    MTRClusterElectricalMeasurementAttributeAcCurrentDivisorID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentDivisorID")
    = 0x00000603,
    MTRClusterElectricalMeasurementAttributeAcPowerMultiplierID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerMultiplierID")
    = 0x00000604,
    MTRClusterElectricalMeasurementAttributeAcPowerDivisorID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerDivisorID")
    = 0x00000605,
    MTRClusterElectricalMeasurementAttributeOverloadAlarmsMaskID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeOverloadAlarmsMaskID")
    = 0x00000700,
    MTRClusterElectricalMeasurementAttributeVoltageOverloadID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeVoltageOverloadID")
    = 0x00000701,
    MTRClusterElectricalMeasurementAttributeCurrentOverloadID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeCurrentOverloadID")
    = 0x00000702,
    MTRClusterElectricalMeasurementAttributeAcOverloadAlarmsMaskID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcOverloadAlarmsMaskID")
    = 0x00000800,
    MTRClusterElectricalMeasurementAttributeAcVoltageOverloadID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageOverloadID")
    = 0x00000801,
    MTRClusterElectricalMeasurementAttributeAcCurrentOverloadID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentOverloadID")
    = 0x00000802,
    MTRClusterElectricalMeasurementAttributeAcActivePowerOverloadID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcActivePowerOverloadID")
    = 0x00000803,
    MTRClusterElectricalMeasurementAttributeAcReactivePowerOverloadID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcReactivePowerOverloadID")
    = 0x00000804,
    MTRClusterElectricalMeasurementAttributeAverageRmsOverVoltageID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageID")
    = 0x00000805,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageID")
    = 0x00000806,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltageID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltageID")
    = 0x00000807,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltageID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltageID")
    = 0x00000808,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagID")
    = 0x00000809,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellID")
    = 0x0000080A,
    MTRClusterElectricalMeasurementAttributeLineCurrentPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseBID")
    = 0x00000901,
    MTRClusterElectricalMeasurementAttributeActiveCurrentPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseBID")
    = 0x00000902,
    MTRClusterElectricalMeasurementAttributeReactiveCurrentPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseBID")
    = 0x00000903,
    MTRClusterElectricalMeasurementAttributeRmsVoltagePhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseBID")
    = 0x00000905,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMinPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseBID")
    = 0x00000906,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseBID")
    = 0x00000907,
    MTRClusterElectricalMeasurementAttributeRmsCurrentPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseBID")
    = 0x00000908,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMinPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseBID")
    = 0x00000909,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseBID")
    = 0x0000090A,
    MTRClusterElectricalMeasurementAttributeActivePowerPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseBID")
    = 0x0000090B,
    MTRClusterElectricalMeasurementAttributeActivePowerMinPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseBID")
    = 0x0000090C,
    MTRClusterElectricalMeasurementAttributeActivePowerMaxPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseBID")
    = 0x0000090D,
    MTRClusterElectricalMeasurementAttributeReactivePowerPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseBID")
    = 0x0000090E,
    MTRClusterElectricalMeasurementAttributeApparentPowerPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseBID")
    = 0x0000090F,
    MTRClusterElectricalMeasurementAttributePowerFactorPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseBID")
    = 0x00000910,
    MTRClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseBID")
    = 0x00000911,
    MTRClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseBID")
    = 0x00000912,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseBID")
    = 0x00000913,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseBID")
    = 0x00000914,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseBID")
    = 0x00000915,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseBID")
    = 0x00000916,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseBID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseBID")
    = 0x00000917,
    MTRClusterElectricalMeasurementAttributeLineCurrentPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseCID")
    = 0x00000A01,
    MTRClusterElectricalMeasurementAttributeActiveCurrentPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseCID")
    = 0x00000A02,
    MTRClusterElectricalMeasurementAttributeReactiveCurrentPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseCID")
    = 0x00000A03,
    MTRClusterElectricalMeasurementAttributeRmsVoltagePhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseCID")
    = 0x00000A05,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMinPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseCID")
    = 0x00000A06,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseCID")
    = 0x00000A07,
    MTRClusterElectricalMeasurementAttributeRmsCurrentPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseCID")
    = 0x00000A08,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMinPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseCID")
    = 0x00000A09,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseCID")
    = 0x00000A0A,
    MTRClusterElectricalMeasurementAttributeActivePowerPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseCID")
    = 0x00000A0B,
    MTRClusterElectricalMeasurementAttributeActivePowerMinPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseCID")
    = 0x00000A0C,
    MTRClusterElectricalMeasurementAttributeActivePowerMaxPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseCID")
    = 0x00000A0D,
    MTRClusterElectricalMeasurementAttributeReactivePowerPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseCID")
    = 0x00000A0E,
    MTRClusterElectricalMeasurementAttributeApparentPowerPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseCID")
    = 0x00000A0F,
    MTRClusterElectricalMeasurementAttributePowerFactorPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseCID")
    = 0x00000A10,
    MTRClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseCID")
    = 0x00000A11,
    MTRClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseCID")
    = 0x00000A12,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseCID")
    = 0x00000A13,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseCID")
    = 0x00000A14,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseCID")
    = 0x00000A15,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseCID")
    = 0x00000A16,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseCID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseCID")
    = 0x00000A17,
    MTRClusterElectricalMeasurementAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterElectricalMeasurementAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterElectricalMeasurementAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterElectricalMeasurementAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterElectricalMeasurementAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ElectricalMeasurement attributes
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasurementTypeID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageID MTR_NEWLY_AVAILABLE = 0x00000100,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMinID MTR_NEWLY_AVAILABLE = 0x00000101,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMaxID MTR_NEWLY_AVAILABLE = 0x00000102,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentID MTR_NEWLY_AVAILABLE = 0x00000103,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMinID MTR_NEWLY_AVAILABLE = 0x00000104,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMaxID MTR_NEWLY_AVAILABLE = 0x00000105,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerID MTR_NEWLY_AVAILABLE = 0x00000106,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMinID MTR_NEWLY_AVAILABLE = 0x00000107,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMaxID MTR_NEWLY_AVAILABLE = 0x00000108,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMultiplierID MTR_NEWLY_AVAILABLE = 0x00000200,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageDivisorID MTR_NEWLY_AVAILABLE = 0x00000201,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMultiplierID MTR_NEWLY_AVAILABLE = 0x00000202,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentDivisorID MTR_NEWLY_AVAILABLE = 0x00000203,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMultiplierID MTR_NEWLY_AVAILABLE = 0x00000204,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerDivisorID MTR_NEWLY_AVAILABLE = 0x00000205,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyID MTR_NEWLY_AVAILABLE = 0x00000300,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMinID MTR_NEWLY_AVAILABLE = 0x00000301,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMaxID MTR_NEWLY_AVAILABLE = 0x00000302,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeNeutralCurrentID MTR_NEWLY_AVAILABLE = 0x00000303,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalActivePowerID MTR_NEWLY_AVAILABLE = 0x00000304,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalReactivePowerID MTR_NEWLY_AVAILABLE = 0x00000305,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalApparentPowerID MTR_NEWLY_AVAILABLE = 0x00000306,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured1stHarmonicCurrentID MTR_NEWLY_AVAILABLE = 0x00000307,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured3rdHarmonicCurrentID MTR_NEWLY_AVAILABLE = 0x00000308,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured5thHarmonicCurrentID MTR_NEWLY_AVAILABLE = 0x00000309,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured7thHarmonicCurrentID MTR_NEWLY_AVAILABLE = 0x0000030A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured9thHarmonicCurrentID MTR_NEWLY_AVAILABLE = 0x0000030B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured11thHarmonicCurrentID MTR_NEWLY_AVAILABLE = 0x0000030C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase1stHarmonicCurrentID MTR_NEWLY_AVAILABLE = 0x0000030D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase3rdHarmonicCurrentID MTR_NEWLY_AVAILABLE = 0x0000030E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase5thHarmonicCurrentID MTR_NEWLY_AVAILABLE = 0x0000030F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase7thHarmonicCurrentID MTR_NEWLY_AVAILABLE = 0x00000310,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase9thHarmonicCurrentID MTR_NEWLY_AVAILABLE = 0x00000311,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase11thHarmonicCurrentID MTR_NEWLY_AVAILABLE = 0x00000312,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMultiplierID MTR_NEWLY_AVAILABLE = 0x00000400,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyDivisorID MTR_NEWLY_AVAILABLE = 0x00000401,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerMultiplierID MTR_NEWLY_AVAILABLE = 0x00000402,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerDivisorID MTR_NEWLY_AVAILABLE = 0x00000403,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeHarmonicCurrentMultiplierID MTR_NEWLY_AVAILABLE = 0x00000404,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePhaseHarmonicCurrentMultiplierID MTR_NEWLY_AVAILABLE = 0x00000405,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousVoltageID MTR_NEWLY_AVAILABLE = 0x00000500,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousLineCurrentID MTR_NEWLY_AVAILABLE = 0x00000501,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousActiveCurrentID MTR_NEWLY_AVAILABLE = 0x00000502,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousReactiveCurrentID MTR_NEWLY_AVAILABLE = 0x00000503,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousPowerID MTR_NEWLY_AVAILABLE = 0x00000504,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageID MTR_NEWLY_AVAILABLE = 0x00000505,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinID MTR_NEWLY_AVAILABLE = 0x00000506,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxID MTR_NEWLY_AVAILABLE = 0x00000507,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentID MTR_NEWLY_AVAILABLE = 0x00000508,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinID MTR_NEWLY_AVAILABLE = 0x00000509,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxID MTR_NEWLY_AVAILABLE = 0x0000050A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerID MTR_NEWLY_AVAILABLE = 0x0000050B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinID MTR_NEWLY_AVAILABLE = 0x0000050C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxID MTR_NEWLY_AVAILABLE = 0x0000050D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerID MTR_NEWLY_AVAILABLE = 0x0000050E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerID MTR_NEWLY_AVAILABLE = 0x0000050F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorID MTR_NEWLY_AVAILABLE = 0x00000510,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodID MTR_NEWLY_AVAILABLE = 0x00000511,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterID MTR_NEWLY_AVAILABLE = 0x00000513,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodID MTR_NEWLY_AVAILABLE = 0x00000514,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodID MTR_NEWLY_AVAILABLE = 0x00000515,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodID MTR_NEWLY_AVAILABLE = 0x00000516,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodID MTR_NEWLY_AVAILABLE = 0x00000517,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageMultiplierID MTR_NEWLY_AVAILABLE = 0x00000600,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageDivisorID MTR_NEWLY_AVAILABLE = 0x00000601,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentMultiplierID MTR_NEWLY_AVAILABLE = 0x00000602,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentDivisorID MTR_NEWLY_AVAILABLE = 0x00000603,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerMultiplierID MTR_NEWLY_AVAILABLE = 0x00000604,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerDivisorID MTR_NEWLY_AVAILABLE = 0x00000605,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeOverloadAlarmsMaskID MTR_NEWLY_AVAILABLE = 0x00000700,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeVoltageOverloadID MTR_NEWLY_AVAILABLE = 0x00000701,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeCurrentOverloadID MTR_NEWLY_AVAILABLE = 0x00000702,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcOverloadAlarmsMaskID MTR_NEWLY_AVAILABLE = 0x00000800,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageOverloadID MTR_NEWLY_AVAILABLE = 0x00000801,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentOverloadID MTR_NEWLY_AVAILABLE = 0x00000802,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcActivePowerOverloadID MTR_NEWLY_AVAILABLE = 0x00000803,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcReactivePowerOverloadID MTR_NEWLY_AVAILABLE = 0x00000804,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageID MTR_NEWLY_AVAILABLE = 0x00000805,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageID MTR_NEWLY_AVAILABLE = 0x00000806,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltageID MTR_NEWLY_AVAILABLE = 0x00000807,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltageID MTR_NEWLY_AVAILABLE = 0x00000808,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagID MTR_NEWLY_AVAILABLE = 0x00000809,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellID MTR_NEWLY_AVAILABLE = 0x0000080A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseBID MTR_NEWLY_AVAILABLE = 0x00000901,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseBID MTR_NEWLY_AVAILABLE = 0x00000902,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseBID MTR_NEWLY_AVAILABLE = 0x00000903,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseBID MTR_NEWLY_AVAILABLE = 0x00000905,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseBID MTR_NEWLY_AVAILABLE = 0x00000906,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseBID MTR_NEWLY_AVAILABLE = 0x00000907,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseBID MTR_NEWLY_AVAILABLE = 0x00000908,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseBID MTR_NEWLY_AVAILABLE = 0x00000909,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseBID MTR_NEWLY_AVAILABLE = 0x0000090A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseBID MTR_NEWLY_AVAILABLE = 0x0000090B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseBID MTR_NEWLY_AVAILABLE = 0x0000090C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseBID MTR_NEWLY_AVAILABLE = 0x0000090D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseBID MTR_NEWLY_AVAILABLE = 0x0000090E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseBID MTR_NEWLY_AVAILABLE = 0x0000090F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseBID MTR_NEWLY_AVAILABLE = 0x00000910,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseBID MTR_NEWLY_AVAILABLE
    = 0x00000911,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseBID MTR_NEWLY_AVAILABLE = 0x00000912,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseBID MTR_NEWLY_AVAILABLE = 0x00000913,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseBID MTR_NEWLY_AVAILABLE = 0x00000914,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseBID MTR_NEWLY_AVAILABLE = 0x00000915,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseBID MTR_NEWLY_AVAILABLE = 0x00000916,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseBID MTR_NEWLY_AVAILABLE = 0x00000917,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A01,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A02,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A03,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseCID MTR_NEWLY_AVAILABLE = 0x00000A05,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A06,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A07,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A08,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A09,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A0A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A0B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A0C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A0D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A0E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A0F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A10,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseCID MTR_NEWLY_AVAILABLE
    = 0x00000A11,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A12,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A13,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A14,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A15,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A16,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseCID MTR_NEWLY_AVAILABLE = 0x00000A17,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ClientMonitoring attributes
    MTRAttributeIDTypeClusterClientMonitoringAttributeIdleModeIntervalID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterClientMonitoringAttributeActiveModeIntervalID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterClientMonitoringAttributeActiveModeThresholdID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterClientMonitoringAttributeExpectedClientsID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterClientMonitoringAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterClientMonitoringAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterClientMonitoringAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterClientMonitoringAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterClientMonitoringAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TestCluster deprecated attribute names
    MTRClusterTestClusterAttributeBooleanID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeBooleanID")
    = 0x00000000,
    MTRClusterTestClusterAttributeBitmap8ID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap8ID")
    = 0x00000001,
    MTRClusterTestClusterAttributeBitmap16ID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap16ID")
    = 0x00000002,
    MTRClusterTestClusterAttributeBitmap32ID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap32ID")
    = 0x00000003,
    MTRClusterTestClusterAttributeBitmap64ID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap64ID")
    = 0x00000004,
    MTRClusterTestClusterAttributeInt8uID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt8uID")
    = 0x00000005,
    MTRClusterTestClusterAttributeInt16uID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt16uID")
    = 0x00000006,
    MTRClusterTestClusterAttributeInt24uID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt24uID")
    = 0x00000007,
    MTRClusterTestClusterAttributeInt32uID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt32uID")
    = 0x00000008,
    MTRClusterTestClusterAttributeInt40uID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt40uID")
    = 0x00000009,
    MTRClusterTestClusterAttributeInt48uID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt48uID")
    = 0x0000000A,
    MTRClusterTestClusterAttributeInt56uID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt56uID")
    = 0x0000000B,
    MTRClusterTestClusterAttributeInt64uID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt64uID")
    = 0x0000000C,
    MTRClusterTestClusterAttributeInt8sID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt8sID")
    = 0x0000000D,
    MTRClusterTestClusterAttributeInt16sID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt16sID")
    = 0x0000000E,
    MTRClusterTestClusterAttributeInt24sID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt24sID")
    = 0x0000000F,
    MTRClusterTestClusterAttributeInt32sID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt32sID")
    = 0x00000010,
    MTRClusterTestClusterAttributeInt40sID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt40sID")
    = 0x00000011,
    MTRClusterTestClusterAttributeInt48sID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt48sID")
    = 0x00000012,
    MTRClusterTestClusterAttributeInt56sID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt56sID")
    = 0x00000013,
    MTRClusterTestClusterAttributeInt64sID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt64sID")
    = 0x00000014,
    MTRClusterTestClusterAttributeEnum8ID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEnum8ID")
    = 0x00000015,
    MTRClusterTestClusterAttributeEnum16ID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEnum16ID")
    = 0x00000016,
    MTRClusterTestClusterAttributeFloatSingleID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeFloatSingleID")
    = 0x00000017,
    MTRClusterTestClusterAttributeFloatDoubleID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeFloatDoubleID")
    = 0x00000018,
    MTRClusterTestClusterAttributeOctetStringID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeOctetStringID")
    = 0x00000019,
    MTRClusterTestClusterAttributeListInt8uID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeListInt8uID")
    = 0x0000001A,
    MTRClusterTestClusterAttributeListOctetStringID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeListOctetStringID")
    = 0x0000001B,
    MTRClusterTestClusterAttributeListStructOctetStringID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeListStructOctetStringID")
    = 0x0000001C,
    MTRClusterTestClusterAttributeLongOctetStringID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeLongOctetStringID")
    = 0x0000001D,
    MTRClusterTestClusterAttributeCharStringID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeCharStringID")
    = 0x0000001E,
    MTRClusterTestClusterAttributeLongCharStringID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeLongCharStringID")
    = 0x0000001F,
    MTRClusterTestClusterAttributeEpochUsID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeEpochUsID")
    = 0x00000020,
    MTRClusterTestClusterAttributeEpochSID MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEpochSID")
    = 0x00000021,
    MTRClusterTestClusterAttributeVendorIdID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeVendorIdID")
    = 0x00000022,
    MTRClusterTestClusterAttributeListNullablesAndOptionalsStructID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeListNullablesAndOptionalsStructID")
    = 0x00000023,
    MTRClusterTestClusterAttributeEnumAttrID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeEnumAttrID")
    = 0x00000024,
    MTRClusterTestClusterAttributeStructAttrID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeStructAttrID")
    = 0x00000025,
    MTRClusterTestClusterAttributeRangeRestrictedInt8uID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8uID")
    = 0x00000026,
    MTRClusterTestClusterAttributeRangeRestrictedInt8sID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8sID")
    = 0x00000027,
    MTRClusterTestClusterAttributeRangeRestrictedInt16uID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16uID")
    = 0x00000028,
    MTRClusterTestClusterAttributeRangeRestrictedInt16sID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16sID")
    = 0x00000029,
    MTRClusterTestClusterAttributeListLongOctetStringID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeListLongOctetStringID")
    = 0x0000002A,
    MTRClusterTestClusterAttributeListFabricScopedID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeListFabricScopedID")
    = 0x0000002B,
    MTRClusterTestClusterAttributeTimedWriteBooleanID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeTimedWriteBooleanID")
    = 0x00000030,
    MTRClusterTestClusterAttributeGeneralErrorBooleanID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeGeneralErrorBooleanID")
    = 0x00000031,
    MTRClusterTestClusterAttributeClusterErrorBooleanID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeClusterErrorBooleanID")
    = 0x00000032,
    MTRClusterTestClusterAttributeUnsupportedID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeUnsupportedID")
    = 0x000000FF,
    MTRClusterTestClusterAttributeNullableBooleanID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBooleanID")
    = 0x00004000,
    MTRClusterTestClusterAttributeNullableBitmap8ID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap8ID")
    = 0x00004001,
    MTRClusterTestClusterAttributeNullableBitmap16ID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap16ID")
    = 0x00004002,
    MTRClusterTestClusterAttributeNullableBitmap32ID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap32ID")
    = 0x00004003,
    MTRClusterTestClusterAttributeNullableBitmap64ID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap64ID")
    = 0x00004004,
    MTRClusterTestClusterAttributeNullableInt8uID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8uID")
    = 0x00004005,
    MTRClusterTestClusterAttributeNullableInt16uID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16uID")
    = 0x00004006,
    MTRClusterTestClusterAttributeNullableInt24uID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24uID")
    = 0x00004007,
    MTRClusterTestClusterAttributeNullableInt32uID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32uID")
    = 0x00004008,
    MTRClusterTestClusterAttributeNullableInt40uID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40uID")
    = 0x00004009,
    MTRClusterTestClusterAttributeNullableInt48uID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48uID")
    = 0x0000400A,
    MTRClusterTestClusterAttributeNullableInt56uID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56uID")
    = 0x0000400B,
    MTRClusterTestClusterAttributeNullableInt64uID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64uID")
    = 0x0000400C,
    MTRClusterTestClusterAttributeNullableInt8sID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8sID")
    = 0x0000400D,
    MTRClusterTestClusterAttributeNullableInt16sID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16sID")
    = 0x0000400E,
    MTRClusterTestClusterAttributeNullableInt24sID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24sID")
    = 0x0000400F,
    MTRClusterTestClusterAttributeNullableInt32sID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32sID")
    = 0x00004010,
    MTRClusterTestClusterAttributeNullableInt40sID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40sID")
    = 0x00004011,
    MTRClusterTestClusterAttributeNullableInt48sID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48sID")
    = 0x00004012,
    MTRClusterTestClusterAttributeNullableInt56sID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56sID")
    = 0x00004013,
    MTRClusterTestClusterAttributeNullableInt64sID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64sID")
    = 0x00004014,
    MTRClusterTestClusterAttributeNullableEnum8ID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum8ID")
    = 0x00004015,
    MTRClusterTestClusterAttributeNullableEnum16ID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum16ID")
    = 0x00004016,
    MTRClusterTestClusterAttributeNullableFloatSingleID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatSingleID")
    = 0x00004017,
    MTRClusterTestClusterAttributeNullableFloatDoubleID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatDoubleID")
    = 0x00004018,
    MTRClusterTestClusterAttributeNullableOctetStringID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableOctetStringID")
    = 0x00004019,
    MTRClusterTestClusterAttributeNullableCharStringID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableCharStringID")
    = 0x0000401E,
    MTRClusterTestClusterAttributeNullableEnumAttrID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnumAttrID")
    = 0x00004024,
    MTRClusterTestClusterAttributeNullableStructID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableStructID")
    = 0x00004025,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt8uID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8uID")
    = 0x00004026,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt8sID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8sID")
    = 0x00004027,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt16uID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16uID")
    = 0x00004028,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt16sID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16sID")
    = 0x00004029,
    MTRClusterTestClusterAttributeWriteOnlyInt8uID API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeWriteOnlyInt8uID")
    = 0x0000402A,
    MTRClusterTestClusterAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTestClusterAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTestClusterAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTestClusterAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTestClusterAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterUnitTestingAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster UnitTesting attributes
    MTRAttributeIDTypeClusterUnitTestingAttributeBooleanID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterUnitTestingAttributeBitmap8ID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterUnitTestingAttributeBitmap16ID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterUnitTestingAttributeBitmap32ID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterUnitTestingAttributeBitmap64ID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt8uID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt16uID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt24uID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt32uID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt40uID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt48uID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt56uID MTR_NEWLY_AVAILABLE = 0x0000000B,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt64uID MTR_NEWLY_AVAILABLE = 0x0000000C,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt8sID MTR_NEWLY_AVAILABLE = 0x0000000D,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt16sID MTR_NEWLY_AVAILABLE = 0x0000000E,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt24sID MTR_NEWLY_AVAILABLE = 0x0000000F,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt32sID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt40sID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt48sID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt56sID MTR_NEWLY_AVAILABLE = 0x00000013,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt64sID MTR_NEWLY_AVAILABLE = 0x00000014,
    MTRAttributeIDTypeClusterUnitTestingAttributeEnum8ID MTR_NEWLY_AVAILABLE = 0x00000015,
    MTRAttributeIDTypeClusterUnitTestingAttributeEnum16ID MTR_NEWLY_AVAILABLE = 0x00000016,
    MTRAttributeIDTypeClusterUnitTestingAttributeFloatSingleID MTR_NEWLY_AVAILABLE = 0x00000017,
    MTRAttributeIDTypeClusterUnitTestingAttributeFloatDoubleID MTR_NEWLY_AVAILABLE = 0x00000018,
    MTRAttributeIDTypeClusterUnitTestingAttributeOctetStringID MTR_NEWLY_AVAILABLE = 0x00000019,
    MTRAttributeIDTypeClusterUnitTestingAttributeListInt8uID MTR_NEWLY_AVAILABLE = 0x0000001A,
    MTRAttributeIDTypeClusterUnitTestingAttributeListOctetStringID MTR_NEWLY_AVAILABLE = 0x0000001B,
    MTRAttributeIDTypeClusterUnitTestingAttributeListStructOctetStringID MTR_NEWLY_AVAILABLE = 0x0000001C,
    MTRAttributeIDTypeClusterUnitTestingAttributeLongOctetStringID MTR_NEWLY_AVAILABLE = 0x0000001D,
    MTRAttributeIDTypeClusterUnitTestingAttributeCharStringID MTR_NEWLY_AVAILABLE = 0x0000001E,
    MTRAttributeIDTypeClusterUnitTestingAttributeLongCharStringID MTR_NEWLY_AVAILABLE = 0x0000001F,
    MTRAttributeIDTypeClusterUnitTestingAttributeEpochUsID MTR_NEWLY_AVAILABLE = 0x00000020,
    MTRAttributeIDTypeClusterUnitTestingAttributeEpochSID MTR_NEWLY_AVAILABLE = 0x00000021,
    MTRAttributeIDTypeClusterUnitTestingAttributeVendorIdID MTR_NEWLY_AVAILABLE = 0x00000022,
    MTRAttributeIDTypeClusterUnitTestingAttributeListNullablesAndOptionalsStructID MTR_NEWLY_AVAILABLE = 0x00000023,
    MTRAttributeIDTypeClusterUnitTestingAttributeEnumAttrID MTR_NEWLY_AVAILABLE = 0x00000024,
    MTRAttributeIDTypeClusterUnitTestingAttributeStructAttrID MTR_NEWLY_AVAILABLE = 0x00000025,
    MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8uID MTR_NEWLY_AVAILABLE = 0x00000026,
    MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8sID MTR_NEWLY_AVAILABLE = 0x00000027,
    MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16uID MTR_NEWLY_AVAILABLE = 0x00000028,
    MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16sID MTR_NEWLY_AVAILABLE = 0x00000029,
    MTRAttributeIDTypeClusterUnitTestingAttributeListLongOctetStringID MTR_NEWLY_AVAILABLE = 0x0000002A,
    MTRAttributeIDTypeClusterUnitTestingAttributeListFabricScopedID MTR_NEWLY_AVAILABLE = 0x0000002B,
    MTRAttributeIDTypeClusterUnitTestingAttributeTimedWriteBooleanID MTR_NEWLY_AVAILABLE = 0x00000030,
    MTRAttributeIDTypeClusterUnitTestingAttributeGeneralErrorBooleanID MTR_NEWLY_AVAILABLE = 0x00000031,
    MTRAttributeIDTypeClusterUnitTestingAttributeClusterErrorBooleanID MTR_NEWLY_AVAILABLE = 0x00000032,
    MTRAttributeIDTypeClusterUnitTestingAttributeUnsupportedID MTR_NEWLY_AVAILABLE = 0x000000FF,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableBooleanID MTR_NEWLY_AVAILABLE = 0x00004000,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap8ID MTR_NEWLY_AVAILABLE = 0x00004001,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap16ID MTR_NEWLY_AVAILABLE = 0x00004002,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap32ID MTR_NEWLY_AVAILABLE = 0x00004003,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap64ID MTR_NEWLY_AVAILABLE = 0x00004004,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8uID MTR_NEWLY_AVAILABLE = 0x00004005,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16uID MTR_NEWLY_AVAILABLE = 0x00004006,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24uID MTR_NEWLY_AVAILABLE = 0x00004007,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32uID MTR_NEWLY_AVAILABLE = 0x00004008,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40uID MTR_NEWLY_AVAILABLE = 0x00004009,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48uID MTR_NEWLY_AVAILABLE = 0x0000400A,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56uID MTR_NEWLY_AVAILABLE = 0x0000400B,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64uID MTR_NEWLY_AVAILABLE = 0x0000400C,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8sID MTR_NEWLY_AVAILABLE = 0x0000400D,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16sID MTR_NEWLY_AVAILABLE = 0x0000400E,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24sID MTR_NEWLY_AVAILABLE = 0x0000400F,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32sID MTR_NEWLY_AVAILABLE = 0x00004010,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40sID MTR_NEWLY_AVAILABLE = 0x00004011,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48sID MTR_NEWLY_AVAILABLE = 0x00004012,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56sID MTR_NEWLY_AVAILABLE = 0x00004013,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64sID MTR_NEWLY_AVAILABLE = 0x00004014,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum8ID MTR_NEWLY_AVAILABLE = 0x00004015,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum16ID MTR_NEWLY_AVAILABLE = 0x00004016,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatSingleID MTR_NEWLY_AVAILABLE = 0x00004017,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatDoubleID MTR_NEWLY_AVAILABLE = 0x00004018,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableOctetStringID MTR_NEWLY_AVAILABLE = 0x00004019,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableCharStringID MTR_NEWLY_AVAILABLE = 0x0000401E,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnumAttrID MTR_NEWLY_AVAILABLE = 0x00004024,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableStructID MTR_NEWLY_AVAILABLE = 0x00004025,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8uID MTR_NEWLY_AVAILABLE = 0x00004026,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8sID MTR_NEWLY_AVAILABLE = 0x00004027,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16uID MTR_NEWLY_AVAILABLE = 0x00004028,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16sID MTR_NEWLY_AVAILABLE = 0x00004029,
    MTRAttributeIDTypeClusterUnitTestingAttributeWriteOnlyInt8uID MTR_NEWLY_AVAILABLE = 0x0000402A,
    MTRAttributeIDTypeClusterUnitTestingAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterUnitTestingAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterUnitTestingAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterUnitTestingAttributeFeatureMapID MTR_NEWLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterUnitTestingAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FaultInjection deprecated attribute names
    MTRClusterFaultInjectionAttributeGeneratedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFaultInjectionAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterFaultInjectionAttributeAcceptedCommandListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFaultInjectionAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterFaultInjectionAttributeAttributeListID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFaultInjectionAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterFaultInjectionAttributeFeatureMapID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFaultInjectionAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterFaultInjectionAttributeClusterRevisionID MTR_NEWLY_DEPRECATED(
        "Please use MTRAttributeIDTypeClusterFaultInjectionAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster FaultInjection attributes
    MTRAttributeIDTypeClusterFaultInjectionAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFaultInjectionAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFaultInjectionAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFaultInjectionAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFaultInjectionAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

};

#pragma mark - Commands IDs

typedef NS_ENUM(uint32_t, MTRCommandIDType) {

    // Cluster Identify deprecated command id names
    MTRClusterIdentifyCommandIdentifyID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterIdentifyCommandIdentifyID")
    = 0x00000000,
    MTRClusterIdentifyCommandTriggerEffectID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterIdentifyCommandTriggerEffectID")
    = 0x00000040,

    // Cluster Identify commands
    MTRCommandIDTypeClusterIdentifyCommandIdentifyID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterIdentifyCommandTriggerEffectID MTR_NEWLY_AVAILABLE = 0x00000040,

    // Cluster Groups deprecated command id names
    MTRClusterGroupsCommandAddGroupID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandAddGroupID")
    = 0x00000000,
    MTRClusterGroupsCommandAddGroupResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupsCommandAddGroupResponseID")
    = 0x00000000,
    MTRClusterGroupsCommandViewGroupID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandViewGroupID")
    = 0x00000001,
    MTRClusterGroupsCommandViewGroupResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupsCommandViewGroupResponseID")
    = 0x00000001,
    MTRClusterGroupsCommandGetGroupMembershipID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipID")
    = 0x00000002,
    MTRClusterGroupsCommandGetGroupMembershipResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipResponseID")
    = 0x00000002,
    MTRClusterGroupsCommandRemoveGroupID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandRemoveGroupID")
    = 0x00000003,
    MTRClusterGroupsCommandRemoveGroupResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupsCommandRemoveGroupResponseID")
    = 0x00000003,
    MTRClusterGroupsCommandRemoveAllGroupsID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupsCommandRemoveAllGroupsID")
    = 0x00000004,
    MTRClusterGroupsCommandAddGroupIfIdentifyingID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupsCommandAddGroupIfIdentifyingID")
    = 0x00000005,

    // Cluster Groups commands
    MTRCommandIDTypeClusterGroupsCommandAddGroupID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterGroupsCommandAddGroupResponseID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterGroupsCommandViewGroupID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterGroupsCommandViewGroupResponseID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipResponseID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterGroupsCommandRemoveGroupID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterGroupsCommandRemoveGroupResponseID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterGroupsCommandRemoveAllGroupsID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterGroupsCommandAddGroupIfIdentifyingID MTR_NEWLY_AVAILABLE = 0x00000005,

    // Cluster Scenes deprecated command id names
    MTRClusterScenesCommandAddSceneID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandAddSceneID")
    = 0x00000000,
    MTRClusterScenesCommandAddSceneResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandAddSceneResponseID")
    = 0x00000000,
    MTRClusterScenesCommandViewSceneID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandViewSceneID")
    = 0x00000001,
    MTRClusterScenesCommandViewSceneResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandViewSceneResponseID")
    = 0x00000001,
    MTRClusterScenesCommandRemoveSceneID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandRemoveSceneID")
    = 0x00000002,
    MTRClusterScenesCommandRemoveSceneResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandRemoveSceneResponseID")
    = 0x00000002,
    MTRClusterScenesCommandRemoveAllScenesID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandRemoveAllScenesID")
    = 0x00000003,
    MTRClusterScenesCommandRemoveAllScenesResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandRemoveAllScenesResponseID")
    = 0x00000003,
    MTRClusterScenesCommandStoreSceneID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandStoreSceneID")
    = 0x00000004,
    MTRClusterScenesCommandStoreSceneResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandStoreSceneResponseID")
    = 0x00000004,
    MTRClusterScenesCommandRecallSceneID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandRecallSceneID")
    = 0x00000005,
    MTRClusterScenesCommandGetSceneMembershipID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandGetSceneMembershipID")
    = 0x00000006,
    MTRClusterScenesCommandGetSceneMembershipResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandGetSceneMembershipResponseID")
    = 0x00000006,
    MTRClusterScenesCommandEnhancedAddSceneID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneID")
    = 0x00000040,
    MTRClusterScenesCommandEnhancedAddSceneResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneResponseID")
    = 0x00000040,
    MTRClusterScenesCommandEnhancedViewSceneID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneID")
    = 0x00000041,
    MTRClusterScenesCommandEnhancedViewSceneResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneResponseID")
    = 0x00000041,
    MTRClusterScenesCommandCopySceneID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandCopySceneID")
    = 0x00000042,
    MTRClusterScenesCommandCopySceneResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterScenesCommandCopySceneResponseID")
    = 0x00000042,

    // Cluster Scenes commands
    MTRCommandIDTypeClusterScenesCommandAddSceneID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterScenesCommandAddSceneResponseID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterScenesCommandViewSceneID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterScenesCommandViewSceneResponseID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterScenesCommandRemoveSceneID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterScenesCommandRemoveSceneResponseID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterScenesCommandRemoveAllScenesID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterScenesCommandRemoveAllScenesResponseID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterScenesCommandStoreSceneID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterScenesCommandStoreSceneResponseID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterScenesCommandRecallSceneID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRCommandIDTypeClusterScenesCommandGetSceneMembershipID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRCommandIDTypeClusterScenesCommandGetSceneMembershipResponseID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneID MTR_NEWLY_AVAILABLE = 0x00000040,
    MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneResponseID MTR_NEWLY_AVAILABLE = 0x00000040,
    MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneID MTR_NEWLY_AVAILABLE = 0x00000041,
    MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneResponseID MTR_NEWLY_AVAILABLE = 0x00000041,
    MTRCommandIDTypeClusterScenesCommandCopySceneID MTR_NEWLY_AVAILABLE = 0x00000042,
    MTRCommandIDTypeClusterScenesCommandCopySceneResponseID MTR_NEWLY_AVAILABLE = 0x00000042,

    // Cluster OnOff deprecated command id names
    MTRClusterOnOffCommandOffID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOffID") = 0x00000000,
    MTRClusterOnOffCommandOnID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOnID") = 0x00000001,
    MTRClusterOnOffCommandToggleID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandToggleID") = 0x00000002,
    MTRClusterOnOffCommandOffWithEffectID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOffWithEffectID")
    = 0x00000040,
    MTRClusterOnOffCommandOnWithRecallGlobalSceneID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOnOffCommandOnWithRecallGlobalSceneID")
    = 0x00000041,
    MTRClusterOnOffCommandOnWithTimedOffID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOnWithTimedOffID")
    = 0x00000042,

    // Cluster OnOff commands
    MTRCommandIDTypeClusterOnOffCommandOffID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterOnOffCommandOnID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterOnOffCommandToggleID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterOnOffCommandOffWithEffectID MTR_NEWLY_AVAILABLE = 0x00000040,
    MTRCommandIDTypeClusterOnOffCommandOnWithRecallGlobalSceneID MTR_NEWLY_AVAILABLE = 0x00000041,
    MTRCommandIDTypeClusterOnOffCommandOnWithTimedOffID MTR_NEWLY_AVAILABLE = 0x00000042,

    // Cluster LevelControl deprecated command id names
    MTRClusterLevelControlCommandMoveToLevelID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterLevelControlCommandMoveToLevelID")
    = 0x00000000,
    MTRClusterLevelControlCommandMoveID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandMoveID")
    = 0x00000001,
    MTRClusterLevelControlCommandStepID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandStepID")
    = 0x00000002,
    MTRClusterLevelControlCommandStopID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandStopID")
    = 0x00000003,
    MTRClusterLevelControlCommandMoveToLevelWithOnOffID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterLevelControlCommandMoveToLevelWithOnOffID")
    = 0x00000004,
    MTRClusterLevelControlCommandMoveWithOnOffID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterLevelControlCommandMoveWithOnOffID")
    = 0x00000005,
    MTRClusterLevelControlCommandStepWithOnOffID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterLevelControlCommandStepWithOnOffID")
    = 0x00000006,
    MTRClusterLevelControlCommandStopWithOnOffID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterLevelControlCommandStopWithOnOffID")
    = 0x00000007,
    MTRClusterLevelControlCommandMoveToClosestFrequencyID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterLevelControlCommandMoveToClosestFrequencyID")
    = 0x00000008,

    // Cluster LevelControl commands
    MTRCommandIDTypeClusterLevelControlCommandMoveToLevelID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterLevelControlCommandMoveID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterLevelControlCommandStepID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterLevelControlCommandStopID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterLevelControlCommandMoveToLevelWithOnOffID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterLevelControlCommandMoveWithOnOffID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRCommandIDTypeClusterLevelControlCommandStepWithOnOffID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRCommandIDTypeClusterLevelControlCommandStopWithOnOffID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRCommandIDTypeClusterLevelControlCommandMoveToClosestFrequencyID MTR_NEWLY_AVAILABLE = 0x00000008,

    // Cluster Actions deprecated command id names
    MTRClusterActionsCommandInstantActionID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandInstantActionID")
    = 0x00000000,
    MTRClusterActionsCommandInstantActionWithTransitionID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterActionsCommandInstantActionWithTransitionID")
    = 0x00000001,
    MTRClusterActionsCommandStartActionID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandStartActionID")
    = 0x00000002,
    MTRClusterActionsCommandStartActionWithDurationID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterActionsCommandStartActionWithDurationID")
    = 0x00000003,
    MTRClusterActionsCommandStopActionID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandStopActionID")
    = 0x00000004,
    MTRClusterActionsCommandPauseActionID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandPauseActionID")
    = 0x00000005,
    MTRClusterActionsCommandPauseActionWithDurationID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterActionsCommandPauseActionWithDurationID")
    = 0x00000006,
    MTRClusterActionsCommandResumeActionID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandResumeActionID")
    = 0x00000007,
    MTRClusterActionsCommandEnableActionID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandEnableActionID")
    = 0x00000008,
    MTRClusterActionsCommandEnableActionWithDurationID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterActionsCommandEnableActionWithDurationID")
    = 0x00000009,
    MTRClusterActionsCommandDisableActionID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandDisableActionID")
    = 0x0000000A,
    MTRClusterActionsCommandDisableActionWithDurationID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterActionsCommandDisableActionWithDurationID")
    = 0x0000000B,

    // Cluster Actions commands
    MTRCommandIDTypeClusterActionsCommandInstantActionID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterActionsCommandInstantActionWithTransitionID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterActionsCommandStartActionID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterActionsCommandStartActionWithDurationID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterActionsCommandStopActionID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterActionsCommandPauseActionID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRCommandIDTypeClusterActionsCommandPauseActionWithDurationID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRCommandIDTypeClusterActionsCommandResumeActionID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRCommandIDTypeClusterActionsCommandEnableActionID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRCommandIDTypeClusterActionsCommandEnableActionWithDurationID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRCommandIDTypeClusterActionsCommandDisableActionID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRCommandIDTypeClusterActionsCommandDisableActionWithDurationID MTR_NEWLY_AVAILABLE = 0x0000000B,

    // Cluster Basic deprecated command id names
    MTRClusterBasicCommandMfgSpecificPingID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterBasicCommandMfgSpecificPingID")
    = 0x10020000,

    // Cluster Basic commands
    MTRCommandIDTypeClusterBasicCommandMfgSpecificPingID MTR_NEWLY_AVAILABLE = 0x10020000,

    // Cluster OtaSoftwareUpdateProvider deprecated command id names
    MTRClusterOtaSoftwareUpdateProviderCommandQueryImageID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOtaSoftwareUpdateProviderCommandQueryImageID")
    = 0x00000000,
    MTRClusterOtaSoftwareUpdateProviderCommandQueryImageResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOtaSoftwareUpdateProviderCommandQueryImageResponseID")
    = 0x00000001,
    MTRClusterOtaSoftwareUpdateProviderCommandApplyUpdateRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOtaSoftwareUpdateProviderCommandApplyUpdateRequestID")
    = 0x00000002,
    MTRClusterOtaSoftwareUpdateProviderCommandApplyUpdateResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOtaSoftwareUpdateProviderCommandApplyUpdateResponseID")
    = 0x00000003,
    MTRClusterOtaSoftwareUpdateProviderCommandNotifyUpdateAppliedID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOtaSoftwareUpdateProviderCommandNotifyUpdateAppliedID")
    = 0x00000004,

    // Cluster OTASoftwareUpdateProvider commands
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageResponseID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateRequestID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateResponseID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandNotifyUpdateAppliedID MTR_NEWLY_AVAILABLE = 0x00000004,

    // Cluster OtaSoftwareUpdateRequestor deprecated command id names
    MTRClusterOtaSoftwareUpdateRequestorCommandAnnounceOtaProviderID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOtaSoftwareUpdateRequestorCommandAnnounceOtaProviderID")
    = 0x00000000,

    // Cluster OTASoftwareUpdateRequestor commands
    MTRCommandIDTypeClusterOTASoftwareUpdateRequestorCommandAnnounceOtaProviderID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster GeneralCommissioning deprecated command id names
    MTRClusterGeneralCommissioningCommandArmFailSafeID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeID")
    = 0x00000000,
    MTRClusterGeneralCommissioningCommandArmFailSafeResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeResponseID")
    = 0x00000001,
    MTRClusterGeneralCommissioningCommandSetRegulatoryConfigID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigID")
    = 0x00000002,
    MTRClusterGeneralCommissioningCommandSetRegulatoryConfigResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigResponseID")
    = 0x00000003,
    MTRClusterGeneralCommissioningCommandCommissioningCompleteID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteID")
    = 0x00000004,
    MTRClusterGeneralCommissioningCommandCommissioningCompleteResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteResponseID")
    = 0x00000005,

    // Cluster GeneralCommissioning commands
    MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeResponseID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigResponseID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteResponseID MTR_NEWLY_AVAILABLE = 0x00000005,

    // Cluster NetworkCommissioning deprecated command id names
    MTRClusterNetworkCommissioningCommandScanNetworksID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksID")
    = 0x00000000,
    MTRClusterNetworkCommissioningCommandScanNetworksResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksResponseID")
    = 0x00000001,
    MTRClusterNetworkCommissioningCommandAddOrUpdateWiFiNetworkID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateWiFiNetworkID")
    = 0x00000002,
    MTRClusterNetworkCommissioningCommandAddOrUpdateThreadNetworkID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateThreadNetworkID")
    = 0x00000003,
    MTRClusterNetworkCommissioningCommandRemoveNetworkID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandRemoveNetworkID")
    = 0x00000004,
    MTRClusterNetworkCommissioningCommandNetworkConfigResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandNetworkConfigResponseID")
    = 0x00000005,
    MTRClusterNetworkCommissioningCommandConnectNetworkID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkID")
    = 0x00000006,
    MTRClusterNetworkCommissioningCommandConnectNetworkResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkResponseID")
    = 0x00000007,
    MTRClusterNetworkCommissioningCommandReorderNetworkID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterNetworkCommissioningCommandReorderNetworkID")
    = 0x00000008,

    // Cluster NetworkCommissioning commands
    MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksResponseID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateWiFiNetworkID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateThreadNetworkID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterNetworkCommissioningCommandRemoveNetworkID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterNetworkCommissioningCommandNetworkConfigResponseID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkResponseID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRCommandIDTypeClusterNetworkCommissioningCommandReorderNetworkID MTR_NEWLY_AVAILABLE = 0x00000008,

    // Cluster DiagnosticLogs deprecated command id names
    MTRClusterDiagnosticLogsCommandRetrieveLogsRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsRequestID")
    = 0x00000000,
    MTRClusterDiagnosticLogsCommandRetrieveLogsResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsResponseID")
    = 0x00000001,

    // Cluster DiagnosticLogs commands
    MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsRequestID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsResponseID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster GeneralDiagnostics deprecated command id names
    MTRClusterGeneralDiagnosticsCommandTestEventTriggerID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGeneralDiagnosticsCommandTestEventTriggerID")
    = 0x00000000,

    // Cluster GeneralDiagnostics commands
    MTRCommandIDTypeClusterGeneralDiagnosticsCommandTestEventTriggerID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster SoftwareDiagnostics deprecated command id names
    MTRClusterSoftwareDiagnosticsCommandResetWatermarksID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterSoftwareDiagnosticsCommandResetWatermarksID")
    = 0x00000000,

    // Cluster SoftwareDiagnostics commands
    MTRCommandIDTypeClusterSoftwareDiagnosticsCommandResetWatermarksID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster ThreadNetworkDiagnostics deprecated command id names
    MTRClusterThreadNetworkDiagnosticsCommandResetCountsID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterThreadNetworkDiagnosticsCommandResetCountsID")
    = 0x00000000,

    // Cluster ThreadNetworkDiagnostics commands
    MTRCommandIDTypeClusterThreadNetworkDiagnosticsCommandResetCountsID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster WiFiNetworkDiagnostics deprecated command id names
    MTRClusterWiFiNetworkDiagnosticsCommandResetCountsID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterWiFiNetworkDiagnosticsCommandResetCountsID")
    = 0x00000000,

    // Cluster WiFiNetworkDiagnostics commands
    MTRCommandIDTypeClusterWiFiNetworkDiagnosticsCommandResetCountsID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster EthernetNetworkDiagnostics deprecated command id names
    MTRClusterEthernetNetworkDiagnosticsCommandResetCountsID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterEthernetNetworkDiagnosticsCommandResetCountsID")
    = 0x00000000,

    // Cluster EthernetNetworkDiagnostics commands
    MTRCommandIDTypeClusterEthernetNetworkDiagnosticsCommandResetCountsID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster TimeSynchronization deprecated command id names
    MTRClusterTimeSynchronizationCommandSetUtcTimeID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterTimeSynchronizationCommandSetUtcTimeID")
    = 0x00000000,

    // Cluster TimeSynchronization commands
    MTRCommandIDTypeClusterTimeSynchronizationCommandSetUtcTimeID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster AdministratorCommissioning deprecated command id names
    MTRClusterAdministratorCommissioningCommandOpenCommissioningWindowID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenCommissioningWindowID")
    = 0x00000000,
    MTRClusterAdministratorCommissioningCommandOpenBasicCommissioningWindowID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenBasicCommissioningWindowID")
    = 0x00000001,
    MTRClusterAdministratorCommissioningCommandRevokeCommissioningID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterAdministratorCommissioningCommandRevokeCommissioningID")
    = 0x00000002,

    // Cluster AdministratorCommissioning commands
    MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenCommissioningWindowID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenBasicCommissioningWindowID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterAdministratorCommissioningCommandRevokeCommissioningID MTR_NEWLY_AVAILABLE = 0x00000002,

    // Cluster OperationalCredentials deprecated command id names
    MTRClusterOperationalCredentialsCommandAttestationRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationRequestID")
    = 0x00000000,
    MTRClusterOperationalCredentialsCommandAttestationResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationResponseID")
    = 0x00000001,
    MTRClusterOperationalCredentialsCommandCertificateChainRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainRequestID")
    = 0x00000002,
    MTRClusterOperationalCredentialsCommandCertificateChainResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainResponseID")
    = 0x00000003,
    MTRClusterOperationalCredentialsCommandCSRRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCSRRequestID")
    = 0x00000004,
    MTRClusterOperationalCredentialsCommandCSRResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCSRResponseID")
    = 0x00000005,
    MTRClusterOperationalCredentialsCommandAddNOCID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAddNOCID")
    = 0x00000006,
    MTRClusterOperationalCredentialsCommandUpdateNOCID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateNOCID")
    = 0x00000007,
    MTRClusterOperationalCredentialsCommandNOCResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandNOCResponseID")
    = 0x00000008,
    MTRClusterOperationalCredentialsCommandUpdateFabricLabelID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateFabricLabelID")
    = 0x00000009,
    MTRClusterOperationalCredentialsCommandRemoveFabricID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandRemoveFabricID")
    = 0x0000000A,
    MTRClusterOperationalCredentialsCommandAddTrustedRootCertificateID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAddTrustedRootCertificateID")
    = 0x0000000B,

    // Cluster OperationalCredentials commands
    MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationRequestID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationResponseID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainRequestID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainResponseID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCSRRequestID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCSRResponseID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRCommandIDTypeClusterOperationalCredentialsCommandAddNOCID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateNOCID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRCommandIDTypeClusterOperationalCredentialsCommandNOCResponseID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateFabricLabelID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRCommandIDTypeClusterOperationalCredentialsCommandRemoveFabricID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRCommandIDTypeClusterOperationalCredentialsCommandAddTrustedRootCertificateID MTR_NEWLY_AVAILABLE = 0x0000000B,

    // Cluster GroupKeyManagement deprecated command id names
    MTRClusterGroupKeyManagementCommandKeySetWriteID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetWriteID")
    = 0x00000000,
    MTRClusterGroupKeyManagementCommandKeySetReadID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadID")
    = 0x00000001,
    MTRClusterGroupKeyManagementCommandKeySetReadResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadResponseID")
    = 0x00000002,
    MTRClusterGroupKeyManagementCommandKeySetRemoveID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetRemoveID")
    = 0x00000003,
    MTRClusterGroupKeyManagementCommandKeySetReadAllIndicesID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesID")
    = 0x00000004,
    MTRClusterGroupKeyManagementCommandKeySetReadAllIndicesResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesResponseID")
    = 0x00000005,

    // Cluster GroupKeyManagement commands
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetWriteID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadResponseID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetRemoveID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesResponseID MTR_NEWLY_AVAILABLE = 0x00000005,

    // Cluster ModeSelect deprecated command id names
    MTRClusterModeSelectCommandChangeToModeID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterModeSelectCommandChangeToModeID")
    = 0x00000000,

    // Cluster ModeSelect commands
    MTRCommandIDTypeClusterModeSelectCommandChangeToModeID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster DoorLock deprecated command id names
    MTRClusterDoorLockCommandLockDoorID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandLockDoorID")
    = 0x00000000,
    MTRClusterDoorLockCommandUnlockDoorID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandUnlockDoorID")
    = 0x00000001,
    MTRClusterDoorLockCommandUnlockWithTimeoutID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandUnlockWithTimeoutID")
    = 0x00000003,
    MTRClusterDoorLockCommandSetWeekDayScheduleID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandSetWeekDayScheduleID")
    = 0x0000000B,
    MTRClusterDoorLockCommandGetWeekDayScheduleID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleID")
    = 0x0000000C,
    MTRClusterDoorLockCommandGetWeekDayScheduleResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleResponseID")
    = 0x0000000C,
    MTRClusterDoorLockCommandClearWeekDayScheduleID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandClearWeekDayScheduleID")
    = 0x0000000D,
    MTRClusterDoorLockCommandSetYearDayScheduleID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandSetYearDayScheduleID")
    = 0x0000000E,
    MTRClusterDoorLockCommandGetYearDayScheduleID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleID")
    = 0x0000000F,
    MTRClusterDoorLockCommandGetYearDayScheduleResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleResponseID")
    = 0x0000000F,
    MTRClusterDoorLockCommandClearYearDayScheduleID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandClearYearDayScheduleID")
    = 0x00000010,
    MTRClusterDoorLockCommandSetHolidayScheduleID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandSetHolidayScheduleID")
    = 0x00000011,
    MTRClusterDoorLockCommandGetHolidayScheduleID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleID")
    = 0x00000012,
    MTRClusterDoorLockCommandGetHolidayScheduleResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleResponseID")
    = 0x00000012,
    MTRClusterDoorLockCommandClearHolidayScheduleID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandClearHolidayScheduleID")
    = 0x00000013,
    MTRClusterDoorLockCommandSetUserID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandSetUserID")
    = 0x0000001A,
    MTRClusterDoorLockCommandGetUserID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetUserID")
    = 0x0000001B,
    MTRClusterDoorLockCommandGetUserResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetUserResponseID")
    = 0x0000001C,
    MTRClusterDoorLockCommandClearUserID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandClearUserID")
    = 0x0000001D,
    MTRClusterDoorLockCommandSetCredentialID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandSetCredentialID")
    = 0x00000022,
    MTRClusterDoorLockCommandSetCredentialResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandSetCredentialResponseID")
    = 0x00000023,
    MTRClusterDoorLockCommandGetCredentialStatusID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusID")
    = 0x00000024,
    MTRClusterDoorLockCommandGetCredentialStatusResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusResponseID")
    = 0x00000025,
    MTRClusterDoorLockCommandClearCredentialID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterDoorLockCommandClearCredentialID")
    = 0x00000026,

    // Cluster DoorLock commands
    MTRCommandIDTypeClusterDoorLockCommandLockDoorID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterDoorLockCommandUnlockDoorID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterDoorLockCommandUnlockWithTimeoutID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterDoorLockCommandSetWeekDayScheduleID MTR_NEWLY_AVAILABLE = 0x0000000B,
    MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleID MTR_NEWLY_AVAILABLE = 0x0000000C,
    MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleResponseID MTR_NEWLY_AVAILABLE = 0x0000000C,
    MTRCommandIDTypeClusterDoorLockCommandClearWeekDayScheduleID MTR_NEWLY_AVAILABLE = 0x0000000D,
    MTRCommandIDTypeClusterDoorLockCommandSetYearDayScheduleID MTR_NEWLY_AVAILABLE = 0x0000000E,
    MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleID MTR_NEWLY_AVAILABLE = 0x0000000F,
    MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleResponseID MTR_NEWLY_AVAILABLE = 0x0000000F,
    MTRCommandIDTypeClusterDoorLockCommandClearYearDayScheduleID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRCommandIDTypeClusterDoorLockCommandSetHolidayScheduleID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleResponseID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRCommandIDTypeClusterDoorLockCommandClearHolidayScheduleID MTR_NEWLY_AVAILABLE = 0x00000013,
    MTRCommandIDTypeClusterDoorLockCommandSetUserID MTR_NEWLY_AVAILABLE = 0x0000001A,
    MTRCommandIDTypeClusterDoorLockCommandGetUserID MTR_NEWLY_AVAILABLE = 0x0000001B,
    MTRCommandIDTypeClusterDoorLockCommandGetUserResponseID MTR_NEWLY_AVAILABLE = 0x0000001C,
    MTRCommandIDTypeClusterDoorLockCommandClearUserID MTR_NEWLY_AVAILABLE = 0x0000001D,
    MTRCommandIDTypeClusterDoorLockCommandSetCredentialID MTR_NEWLY_AVAILABLE = 0x00000022,
    MTRCommandIDTypeClusterDoorLockCommandSetCredentialResponseID MTR_NEWLY_AVAILABLE = 0x00000023,
    MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusID MTR_NEWLY_AVAILABLE = 0x00000024,
    MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusResponseID MTR_NEWLY_AVAILABLE = 0x00000025,
    MTRCommandIDTypeClusterDoorLockCommandClearCredentialID MTR_NEWLY_AVAILABLE = 0x00000026,

    // Cluster WindowCovering deprecated command id names
    MTRClusterWindowCoveringCommandUpOrOpenID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterWindowCoveringCommandUpOrOpenID")
    = 0x00000000,
    MTRClusterWindowCoveringCommandDownOrCloseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterWindowCoveringCommandDownOrCloseID")
    = 0x00000001,
    MTRClusterWindowCoveringCommandStopMotionID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterWindowCoveringCommandStopMotionID")
    = 0x00000002,
    MTRClusterWindowCoveringCommandGoToLiftValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftValueID")
    = 0x00000004,
    MTRClusterWindowCoveringCommandGoToLiftPercentageID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftPercentageID")
    = 0x00000005,
    MTRClusterWindowCoveringCommandGoToTiltValueID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltValueID")
    = 0x00000007,
    MTRClusterWindowCoveringCommandGoToTiltPercentageID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltPercentageID")
    = 0x00000008,

    // Cluster WindowCovering commands
    MTRCommandIDTypeClusterWindowCoveringCommandUpOrOpenID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterWindowCoveringCommandDownOrCloseID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterWindowCoveringCommandStopMotionID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftValueID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftPercentageID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltValueID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltPercentageID MTR_NEWLY_AVAILABLE = 0x00000008,

    // Cluster BarrierControl deprecated command id names
    MTRClusterBarrierControlCommandBarrierControlGoToPercentID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterBarrierControlCommandBarrierControlGoToPercentID")
    = 0x00000000,
    MTRClusterBarrierControlCommandBarrierControlStopID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterBarrierControlCommandBarrierControlStopID")
    = 0x00000001,

    // Cluster BarrierControl commands
    MTRCommandIDTypeClusterBarrierControlCommandBarrierControlGoToPercentID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterBarrierControlCommandBarrierControlStopID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster Thermostat deprecated command id names
    MTRClusterThermostatCommandSetpointRaiseLowerID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterThermostatCommandSetpointRaiseLowerID")
    = 0x00000000,
    MTRClusterThermostatCommandGetWeeklyScheduleResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleResponseID")
    = 0x00000000,
    MTRClusterThermostatCommandSetWeeklyScheduleID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterThermostatCommandSetWeeklyScheduleID")
    = 0x00000001,
    MTRClusterThermostatCommandGetWeeklyScheduleID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleID")
    = 0x00000002,
    MTRClusterThermostatCommandClearWeeklyScheduleID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterThermostatCommandClearWeeklyScheduleID")
    = 0x00000003,

    // Cluster Thermostat commands
    MTRCommandIDTypeClusterThermostatCommandSetpointRaiseLowerID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleResponseID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterThermostatCommandSetWeeklyScheduleID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterThermostatCommandClearWeeklyScheduleID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster ColorControl deprecated command id names
    MTRClusterColorControlCommandMoveToHueID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandMoveToHueID")
    = 0x00000000,
    MTRClusterColorControlCommandMoveHueID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveHueID")
    = 0x00000001,
    MTRClusterColorControlCommandStepHueID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandStepHueID")
    = 0x00000002,
    MTRClusterColorControlCommandMoveToSaturationID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandMoveToSaturationID")
    = 0x00000003,
    MTRClusterColorControlCommandMoveSaturationID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandMoveSaturationID")
    = 0x00000004,
    MTRClusterColorControlCommandStepSaturationID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandStepSaturationID")
    = 0x00000005,
    MTRClusterColorControlCommandMoveToHueAndSaturationID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandMoveToHueAndSaturationID")
    = 0x00000006,
    MTRClusterColorControlCommandMoveToColorID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandMoveToColorID")
    = 0x00000007,
    MTRClusterColorControlCommandMoveColorID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandMoveColorID")
    = 0x00000008,
    MTRClusterColorControlCommandStepColorID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandStepColorID")
    = 0x00000009,
    MTRClusterColorControlCommandMoveToColorTemperatureID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandMoveToColorTemperatureID")
    = 0x0000000A,
    MTRClusterColorControlCommandEnhancedMoveToHueID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueID")
    = 0x00000040,
    MTRClusterColorControlCommandEnhancedMoveHueID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandEnhancedMoveHueID")
    = 0x00000041,
    MTRClusterColorControlCommandEnhancedStepHueID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandEnhancedStepHueID")
    = 0x00000042,
    MTRClusterColorControlCommandEnhancedMoveToHueAndSaturationID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueAndSaturationID")
    = 0x00000043,
    MTRClusterColorControlCommandColorLoopSetID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandColorLoopSetID")
    = 0x00000044,
    MTRClusterColorControlCommandStopMoveStepID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandStopMoveStepID")
    = 0x00000047,
    MTRClusterColorControlCommandMoveColorTemperatureID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandMoveColorTemperatureID")
    = 0x0000004B,
    MTRClusterColorControlCommandStepColorTemperatureID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterColorControlCommandStepColorTemperatureID")
    = 0x0000004C,

    // Cluster ColorControl commands
    MTRCommandIDTypeClusterColorControlCommandMoveToHueID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterColorControlCommandMoveHueID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterColorControlCommandStepHueID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterColorControlCommandMoveToSaturationID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterColorControlCommandMoveSaturationID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterColorControlCommandStepSaturationID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRCommandIDTypeClusterColorControlCommandMoveToHueAndSaturationID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRCommandIDTypeClusterColorControlCommandMoveToColorID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRCommandIDTypeClusterColorControlCommandMoveColorID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRCommandIDTypeClusterColorControlCommandStepColorID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRCommandIDTypeClusterColorControlCommandMoveToColorTemperatureID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueID MTR_NEWLY_AVAILABLE = 0x00000040,
    MTRCommandIDTypeClusterColorControlCommandEnhancedMoveHueID MTR_NEWLY_AVAILABLE = 0x00000041,
    MTRCommandIDTypeClusterColorControlCommandEnhancedStepHueID MTR_NEWLY_AVAILABLE = 0x00000042,
    MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueAndSaturationID MTR_NEWLY_AVAILABLE = 0x00000043,
    MTRCommandIDTypeClusterColorControlCommandColorLoopSetID MTR_NEWLY_AVAILABLE = 0x00000044,
    MTRCommandIDTypeClusterColorControlCommandStopMoveStepID MTR_NEWLY_AVAILABLE = 0x00000047,
    MTRCommandIDTypeClusterColorControlCommandMoveColorTemperatureID MTR_NEWLY_AVAILABLE = 0x0000004B,
    MTRCommandIDTypeClusterColorControlCommandStepColorTemperatureID MTR_NEWLY_AVAILABLE = 0x0000004C,

    // Cluster Channel deprecated command id names
    MTRClusterChannelCommandChangeChannelID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterChannelCommandChangeChannelID")
    = 0x00000000,
    MTRClusterChannelCommandChangeChannelResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterChannelCommandChangeChannelResponseID")
    = 0x00000001,
    MTRClusterChannelCommandChangeChannelByNumberID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterChannelCommandChangeChannelByNumberID")
    = 0x00000002,
    MTRClusterChannelCommandSkipChannelID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterChannelCommandSkipChannelID")
    = 0x00000003,

    // Cluster Channel commands
    MTRCommandIDTypeClusterChannelCommandChangeChannelID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterChannelCommandChangeChannelResponseID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterChannelCommandChangeChannelByNumberID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterChannelCommandSkipChannelID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster TargetNavigator deprecated command id names
    MTRClusterTargetNavigatorCommandNavigateTargetID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetID")
    = 0x00000000,
    MTRClusterTargetNavigatorCommandNavigateTargetResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetResponseID")
    = 0x00000001,

    // Cluster TargetNavigator commands
    MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetResponseID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster MediaPlayback deprecated command id names
    MTRClusterMediaPlaybackCommandPlayID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandPlayID")
    = 0x00000000,
    MTRClusterMediaPlaybackCommandPauseID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandPauseID")
    = 0x00000001,
    MTRClusterMediaPlaybackCommandStopPlaybackID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaPlaybackCommandStopPlaybackID")
    = 0x00000002,
    MTRClusterMediaPlaybackCommandStartOverID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaPlaybackCommandStartOverID")
    = 0x00000003,
    MTRClusterMediaPlaybackCommandPreviousID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaPlaybackCommandPreviousID")
    = 0x00000004,
    MTRClusterMediaPlaybackCommandNextID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandNextID")
    = 0x00000005,
    MTRClusterMediaPlaybackCommandRewindID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandRewindID")
    = 0x00000006,
    MTRClusterMediaPlaybackCommandFastForwardID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaPlaybackCommandFastForwardID")
    = 0x00000007,
    MTRClusterMediaPlaybackCommandSkipForwardID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaPlaybackCommandSkipForwardID")
    = 0x00000008,
    MTRClusterMediaPlaybackCommandSkipBackwardID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaPlaybackCommandSkipBackwardID")
    = 0x00000009,
    MTRClusterMediaPlaybackCommandPlaybackResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaPlaybackCommandPlaybackResponseID")
    = 0x0000000A,
    MTRClusterMediaPlaybackCommandSeekID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandSeekID")
    = 0x0000000B,

    // Cluster MediaPlayback commands
    MTRCommandIDTypeClusterMediaPlaybackCommandPlayID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterMediaPlaybackCommandPauseID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterMediaPlaybackCommandStopPlaybackID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterMediaPlaybackCommandStartOverID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterMediaPlaybackCommandPreviousID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterMediaPlaybackCommandNextID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRCommandIDTypeClusterMediaPlaybackCommandRewindID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRCommandIDTypeClusterMediaPlaybackCommandFastForwardID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRCommandIDTypeClusterMediaPlaybackCommandSkipForwardID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRCommandIDTypeClusterMediaPlaybackCommandSkipBackwardID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRCommandIDTypeClusterMediaPlaybackCommandPlaybackResponseID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRCommandIDTypeClusterMediaPlaybackCommandSeekID MTR_NEWLY_AVAILABLE = 0x0000000B,

    // Cluster MediaInput deprecated command id names
    MTRClusterMediaInputCommandSelectInputID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaInputCommandSelectInputID")
    = 0x00000000,
    MTRClusterMediaInputCommandShowInputStatusID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaInputCommandShowInputStatusID")
    = 0x00000001,
    MTRClusterMediaInputCommandHideInputStatusID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaInputCommandHideInputStatusID")
    = 0x00000002,
    MTRClusterMediaInputCommandRenameInputID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterMediaInputCommandRenameInputID")
    = 0x00000003,

    // Cluster MediaInput commands
    MTRCommandIDTypeClusterMediaInputCommandSelectInputID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterMediaInputCommandShowInputStatusID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterMediaInputCommandHideInputStatusID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterMediaInputCommandRenameInputID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster LowPower deprecated command id names
    MTRClusterLowPowerCommandSleepID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterLowPowerCommandSleepID") = 0x00000000,

    // Cluster LowPower commands
    MTRCommandIDTypeClusterLowPowerCommandSleepID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster KeypadInput deprecated command id names
    MTRClusterKeypadInputCommandSendKeyID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterKeypadInputCommandSendKeyID")
    = 0x00000000,
    MTRClusterKeypadInputCommandSendKeyResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterKeypadInputCommandSendKeyResponseID")
    = 0x00000001,

    // Cluster KeypadInput commands
    MTRCommandIDTypeClusterKeypadInputCommandSendKeyID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterKeypadInputCommandSendKeyResponseID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster ContentLauncher deprecated command id names
    MTRClusterContentLauncherCommandLaunchContentID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterContentLauncherCommandLaunchContentID")
    = 0x00000000,
    MTRClusterContentLauncherCommandLaunchURLID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterContentLauncherCommandLaunchURLID")
    = 0x00000001,
    MTRClusterContentLauncherCommandLaunchResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterContentLauncherCommandLaunchResponseID")
    = 0x00000002,

    // Cluster ContentLauncher commands
    MTRCommandIDTypeClusterContentLauncherCommandLaunchContentID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterContentLauncherCommandLaunchURLID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterContentLauncherCommandLaunchResponseID MTR_NEWLY_AVAILABLE = 0x00000002,

    // Cluster AudioOutput deprecated command id names
    MTRClusterAudioOutputCommandSelectOutputID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterAudioOutputCommandSelectOutputID")
    = 0x00000000,
    MTRClusterAudioOutputCommandRenameOutputID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterAudioOutputCommandRenameOutputID")
    = 0x00000001,

    // Cluster AudioOutput commands
    MTRCommandIDTypeClusterAudioOutputCommandSelectOutputID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterAudioOutputCommandRenameOutputID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster ApplicationLauncher deprecated command id names
    MTRClusterApplicationLauncherCommandLaunchAppID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterApplicationLauncherCommandLaunchAppID")
    = 0x00000000,
    MTRClusterApplicationLauncherCommandStopAppID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterApplicationLauncherCommandStopAppID")
    = 0x00000001,
    MTRClusterApplicationLauncherCommandHideAppID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterApplicationLauncherCommandHideAppID")
    = 0x00000002,
    MTRClusterApplicationLauncherCommandLauncherResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterApplicationLauncherCommandLauncherResponseID")
    = 0x00000003,

    // Cluster ApplicationLauncher commands
    MTRCommandIDTypeClusterApplicationLauncherCommandLaunchAppID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterApplicationLauncherCommandStopAppID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterApplicationLauncherCommandHideAppID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterApplicationLauncherCommandLauncherResponseID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster AccountLogin deprecated command id names
    MTRClusterAccountLoginCommandGetSetupPINID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINID")
    = 0x00000000,
    MTRClusterAccountLoginCommandGetSetupPINResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINResponseID")
    = 0x00000001,
    MTRClusterAccountLoginCommandLoginID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterAccountLoginCommandLoginID")
    = 0x00000002,
    MTRClusterAccountLoginCommandLogoutID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterAccountLoginCommandLogoutID")
    = 0x00000003,

    // Cluster AccountLogin commands
    MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINResponseID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterAccountLoginCommandLoginID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterAccountLoginCommandLogoutID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster ElectricalMeasurement deprecated command id names
    MTRClusterElectricalMeasurementCommandGetProfileInfoResponseCommandID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoResponseCommandID")
    = 0x00000000,
    MTRClusterElectricalMeasurementCommandGetProfileInfoCommandID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoCommandID")
    = 0x00000000,
    MTRClusterElectricalMeasurementCommandGetMeasurementProfileResponseCommandID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileResponseCommandID")
    = 0x00000001,
    MTRClusterElectricalMeasurementCommandGetMeasurementProfileCommandID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileCommandID")
    = 0x00000001,

    // Cluster ElectricalMeasurement commands
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoResponseCommandID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoCommandID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileResponseCommandID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileCommandID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster ClientMonitoring commands
    MTRCommandIDTypeClusterClientMonitoringCommandRegisterClientMonitoringID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterClientMonitoringCommandStayAwakeRequestID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster TestCluster deprecated command id names
    MTRClusterTestClusterCommandTestID MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestID")
    = 0x00000000,
    MTRClusterTestClusterCommandTestSpecificResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestSpecificResponseID")
    = 0x00000000,
    MTRClusterTestClusterCommandTestNotHandledID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestNotHandledID")
    = 0x00000001,
    MTRClusterTestClusterCommandTestAddArgumentsResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsResponseID")
    = 0x00000001,
    MTRClusterTestClusterCommandTestSpecificID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestSpecificID")
    = 0x00000002,
    MTRClusterTestClusterCommandTestSimpleArgumentResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentResponseID")
    = 0x00000002,
    MTRClusterTestClusterCommandTestUnknownCommandID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestUnknownCommandID")
    = 0x00000003,
    MTRClusterTestClusterCommandTestStructArrayArgumentResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentResponseID")
    = 0x00000003,
    MTRClusterTestClusterCommandTestAddArgumentsID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsID")
    = 0x00000004,
    MTRClusterTestClusterCommandTestListInt8UReverseResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseResponseID")
    = 0x00000004,
    MTRClusterTestClusterCommandTestSimpleArgumentRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentRequestID")
    = 0x00000005,
    MTRClusterTestClusterCommandTestEnumsResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestEnumsResponseID")
    = 0x00000005,
    MTRClusterTestClusterCommandTestStructArrayArgumentRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentRequestID")
    = 0x00000006,
    MTRClusterTestClusterCommandTestNullableOptionalResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalResponseID")
    = 0x00000006,
    MTRClusterTestClusterCommandTestStructArgumentRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestStructArgumentRequestID")
    = 0x00000007,
    MTRClusterTestClusterCommandTestComplexNullableOptionalResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalResponseID")
    = 0x00000007,
    MTRClusterTestClusterCommandTestNestedStructArgumentRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructArgumentRequestID")
    = 0x00000008,
    MTRClusterTestClusterCommandBooleanResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandBooleanResponseID")
    = 0x00000008,
    MTRClusterTestClusterCommandTestListStructArgumentRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestListStructArgumentRequestID")
    = 0x00000009,
    MTRClusterTestClusterCommandSimpleStructResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandSimpleStructResponseID")
    = 0x00000009,
    MTRClusterTestClusterCommandTestListInt8UArgumentRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UArgumentRequestID")
    = 0x0000000A,
    MTRClusterTestClusterCommandTestEmitTestEventResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventResponseID")
    = 0x0000000A,
    MTRClusterTestClusterCommandTestNestedStructListArgumentRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructListArgumentRequestID")
    = 0x0000000B,
    MTRClusterTestClusterCommandTestEmitTestFabricScopedEventResponseID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventResponseID")
    = 0x0000000B,
    MTRClusterTestClusterCommandTestListNestedStructListArgumentRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestListNestedStructListArgumentRequestID")
    = 0x0000000C,
    MTRClusterTestClusterCommandTestListInt8UReverseRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseRequestID")
    = 0x0000000D,
    MTRClusterTestClusterCommandTestEnumsRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestEnumsRequestID")
    = 0x0000000E,
    MTRClusterTestClusterCommandTestNullableOptionalRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalRequestID")
    = 0x0000000F,
    MTRClusterTestClusterCommandTestComplexNullableOptionalRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalRequestID")
    = 0x00000010,
    MTRClusterTestClusterCommandSimpleStructEchoRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandSimpleStructEchoRequestID")
    = 0x00000011,
    MTRClusterTestClusterCommandTimedInvokeRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTimedInvokeRequestID")
    = 0x00000012,
    MTRClusterTestClusterCommandTestSimpleOptionalArgumentRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestSimpleOptionalArgumentRequestID")
    = 0x00000013,
    MTRClusterTestClusterCommandTestEmitTestEventRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventRequestID")
    = 0x00000014,
    MTRClusterTestClusterCommandTestEmitTestFabricScopedEventRequestID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventRequestID")
    = 0x00000015,

    // Cluster UnitTesting commands
    MTRCommandIDTypeClusterUnitTestingCommandTestID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterUnitTestingCommandTestSpecificResponseID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterUnitTestingCommandTestNotHandledID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsResponseID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterUnitTestingCommandTestSpecificID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentResponseID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterUnitTestingCommandTestUnknownCommandID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentResponseID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseResponseID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentRequestID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRCommandIDTypeClusterUnitTestingCommandTestEnumsResponseID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentRequestID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalResponseID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRCommandIDTypeClusterUnitTestingCommandTestStructArgumentRequestID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalResponseID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructArgumentRequestID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRCommandIDTypeClusterUnitTestingCommandBooleanResponseID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRCommandIDTypeClusterUnitTestingCommandTestListStructArgumentRequestID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRCommandIDTypeClusterUnitTestingCommandSimpleStructResponseID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UArgumentRequestID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventResponseID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructListArgumentRequestID MTR_NEWLY_AVAILABLE = 0x0000000B,
    MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventResponseID MTR_NEWLY_AVAILABLE = 0x0000000B,
    MTRCommandIDTypeClusterUnitTestingCommandTestListNestedStructListArgumentRequestID MTR_NEWLY_AVAILABLE = 0x0000000C,
    MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseRequestID MTR_NEWLY_AVAILABLE = 0x0000000D,
    MTRCommandIDTypeClusterUnitTestingCommandTestEnumsRequestID MTR_NEWLY_AVAILABLE = 0x0000000E,
    MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalRequestID MTR_NEWLY_AVAILABLE = 0x0000000F,
    MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalRequestID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRCommandIDTypeClusterUnitTestingCommandSimpleStructEchoRequestID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRCommandIDTypeClusterUnitTestingCommandTimedInvokeRequestID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRCommandIDTypeClusterUnitTestingCommandTestSimpleOptionalArgumentRequestID MTR_NEWLY_AVAILABLE = 0x00000013,
    MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventRequestID MTR_NEWLY_AVAILABLE = 0x00000014,
    MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventRequestID MTR_NEWLY_AVAILABLE = 0x00000015,

    // Cluster FaultInjection deprecated command id names
    MTRClusterFaultInjectionCommandFailAtFaultID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterFaultInjectionCommandFailAtFaultID")
    = 0x00000000,
    MTRClusterFaultInjectionCommandFailRandomlyAtFaultID MTR_NEWLY_DEPRECATED(
        "Please use MTRCommandIDTypeClusterFaultInjectionCommandFailRandomlyAtFaultID")
    = 0x00000001,

    // Cluster FaultInjection commands
    MTRCommandIDTypeClusterFaultInjectionCommandFailAtFaultID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterFaultInjectionCommandFailRandomlyAtFaultID MTR_NEWLY_AVAILABLE = 0x00000001,

};

#pragma mark - Events IDs

typedef NS_ENUM(uint32_t, MTREventIDType) {

    // Cluster AccessControl deprecated event names
    MTRClusterAccessControlEventAccessControlEntryChangedID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterAccessControlEventAccessControlEntryChangedID")
    = 0x00000000,
    MTRClusterAccessControlEventAccessControlExtensionChangedID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterAccessControlEventAccessControlExtensionChangedID")
    = 0x00000001,

    // Cluster AccessControl events
    MTREventIDTypeClusterAccessControlEventAccessControlEntryChangedID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterAccessControlEventAccessControlExtensionChangedID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster Actions deprecated event names
    MTRClusterActionsEventStateChangedID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterActionsEventStateChangedID")
    = 0x00000000,
    MTRClusterActionsEventActionFailedID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterActionsEventActionFailedID")
    = 0x00000001,

    // Cluster Actions events
    MTREventIDTypeClusterActionsEventStateChangedID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterActionsEventActionFailedID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster Basic deprecated event names
    MTRClusterBasicEventStartUpID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterBasicEventStartUpID") = 0x00000000,
    MTRClusterBasicEventShutDownID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterBasicEventShutDownID") = 0x00000001,
    MTRClusterBasicEventLeaveID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterBasicEventLeaveID") = 0x00000002,
    MTRClusterBasicEventReachableChangedID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterBasicEventReachableChangedID")
    = 0x00000003,

    // Cluster Basic events
    MTREventIDTypeClusterBasicEventStartUpID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterBasicEventShutDownID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterBasicEventLeaveID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTREventIDTypeClusterBasicEventReachableChangedID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster OtaSoftwareUpdateRequestor deprecated event names
    MTRClusterOtaSoftwareUpdateRequestorEventStateTransitionID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterOtaSoftwareUpdateRequestorEventStateTransitionID")
    = 0x00000000,
    MTRClusterOtaSoftwareUpdateRequestorEventVersionAppliedID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterOtaSoftwareUpdateRequestorEventVersionAppliedID")
    = 0x00000001,
    MTRClusterOtaSoftwareUpdateRequestorEventDownloadErrorID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterOtaSoftwareUpdateRequestorEventDownloadErrorID")
    = 0x00000002,

    // Cluster OTASoftwareUpdateRequestor events
    MTREventIDTypeClusterOTASoftwareUpdateRequestorEventStateTransitionID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterOTASoftwareUpdateRequestorEventVersionAppliedID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterOTASoftwareUpdateRequestorEventDownloadErrorID MTR_NEWLY_AVAILABLE = 0x00000002,

    // Cluster GeneralDiagnostics deprecated event names
    MTRClusterGeneralDiagnosticsEventHardwareFaultChangeID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterGeneralDiagnosticsEventHardwareFaultChangeID")
    = 0x00000000,
    MTRClusterGeneralDiagnosticsEventRadioFaultChangeID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterGeneralDiagnosticsEventRadioFaultChangeID")
    = 0x00000001,
    MTRClusterGeneralDiagnosticsEventNetworkFaultChangeID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterGeneralDiagnosticsEventNetworkFaultChangeID")
    = 0x00000002,
    MTRClusterGeneralDiagnosticsEventBootReasonID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterGeneralDiagnosticsEventBootReasonID")
    = 0x00000003,

    // Cluster GeneralDiagnostics events
    MTREventIDTypeClusterGeneralDiagnosticsEventHardwareFaultChangeID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterGeneralDiagnosticsEventRadioFaultChangeID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterGeneralDiagnosticsEventNetworkFaultChangeID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTREventIDTypeClusterGeneralDiagnosticsEventBootReasonID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster SoftwareDiagnostics deprecated event names
    MTRClusterSoftwareDiagnosticsEventSoftwareFaultID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterSoftwareDiagnosticsEventSoftwareFaultID")
    = 0x00000000,

    // Cluster SoftwareDiagnostics events
    MTREventIDTypeClusterSoftwareDiagnosticsEventSoftwareFaultID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster ThreadNetworkDiagnostics deprecated event names
    MTRClusterThreadNetworkDiagnosticsEventConnectionStatusID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterThreadNetworkDiagnosticsEventConnectionStatusID")
    = 0x00000000,
    MTRClusterThreadNetworkDiagnosticsEventNetworkFaultChangeID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterThreadNetworkDiagnosticsEventNetworkFaultChangeID")
    = 0x00000001,

    // Cluster ThreadNetworkDiagnostics events
    MTREventIDTypeClusterThreadNetworkDiagnosticsEventConnectionStatusID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterThreadNetworkDiagnosticsEventNetworkFaultChangeID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster WiFiNetworkDiagnostics deprecated event names
    MTRClusterWiFiNetworkDiagnosticsEventDisconnectionID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterWiFiNetworkDiagnosticsEventDisconnectionID")
    = 0x00000000,
    MTRClusterWiFiNetworkDiagnosticsEventAssociationFailureID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterWiFiNetworkDiagnosticsEventAssociationFailureID")
    = 0x00000001,
    MTRClusterWiFiNetworkDiagnosticsEventConnectionStatusID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterWiFiNetworkDiagnosticsEventConnectionStatusID")
    = 0x00000002,

    // Cluster WiFiNetworkDiagnostics events
    MTREventIDTypeClusterWiFiNetworkDiagnosticsEventDisconnectionID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterWiFiNetworkDiagnosticsEventAssociationFailureID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterWiFiNetworkDiagnosticsEventConnectionStatusID MTR_NEWLY_AVAILABLE = 0x00000002,

    // Cluster BridgedDeviceBasic deprecated event names
    MTRClusterBridgedDeviceBasicEventStartUpID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterBridgedDeviceBasicEventStartUpID")
    = 0x00000000,
    MTRClusterBridgedDeviceBasicEventShutDownID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterBridgedDeviceBasicEventShutDownID")
    = 0x00000001,
    MTRClusterBridgedDeviceBasicEventLeaveID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterBridgedDeviceBasicEventLeaveID")
    = 0x00000002,
    MTRClusterBridgedDeviceBasicEventReachableChangedID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterBridgedDeviceBasicEventReachableChangedID")
    = 0x00000003,

    // Cluster BridgedDeviceBasic events
    MTREventIDTypeClusterBridgedDeviceBasicEventStartUpID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterBridgedDeviceBasicEventShutDownID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterBridgedDeviceBasicEventLeaveID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTREventIDTypeClusterBridgedDeviceBasicEventReachableChangedID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster Switch deprecated event names
    MTRClusterSwitchEventSwitchLatchedID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventSwitchLatchedID")
    = 0x00000000,
    MTRClusterSwitchEventInitialPressID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventInitialPressID")
    = 0x00000001,
    MTRClusterSwitchEventLongPressID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventLongPressID") = 0x00000002,
    MTRClusterSwitchEventShortReleaseID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventShortReleaseID")
    = 0x00000003,
    MTRClusterSwitchEventLongReleaseID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventLongReleaseID")
    = 0x00000004,
    MTRClusterSwitchEventMultiPressOngoingID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventMultiPressOngoingID")
    = 0x00000005,
    MTRClusterSwitchEventMultiPressCompleteID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterSwitchEventMultiPressCompleteID")
    = 0x00000006,

    // Cluster Switch events
    MTREventIDTypeClusterSwitchEventSwitchLatchedID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterSwitchEventInitialPressID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterSwitchEventLongPressID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTREventIDTypeClusterSwitchEventShortReleaseID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTREventIDTypeClusterSwitchEventLongReleaseID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTREventIDTypeClusterSwitchEventMultiPressOngoingID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTREventIDTypeClusterSwitchEventMultiPressCompleteID MTR_NEWLY_AVAILABLE = 0x00000006,

    // Cluster BooleanState deprecated event names
    MTRClusterBooleanStateEventStateChangeID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterBooleanStateEventStateChangeID")
    = 0x00000000,

    // Cluster BooleanState events
    MTREventIDTypeClusterBooleanStateEventStateChangeID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster DoorLock deprecated event names
    MTRClusterDoorLockEventDoorLockAlarmID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventDoorLockAlarmID")
    = 0x00000000,
    MTRClusterDoorLockEventDoorStateChangeID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventDoorStateChangeID")
    = 0x00000001,
    MTRClusterDoorLockEventLockOperationID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventLockOperationID")
    = 0x00000002,
    MTRClusterDoorLockEventLockOperationErrorID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterDoorLockEventLockOperationErrorID")
    = 0x00000003,
    MTRClusterDoorLockEventLockUserChangeID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventLockUserChangeID")
    = 0x00000004,

    // Cluster DoorLock events
    MTREventIDTypeClusterDoorLockEventDoorLockAlarmID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterDoorLockEventDoorStateChangeID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterDoorLockEventLockOperationID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTREventIDTypeClusterDoorLockEventLockOperationErrorID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTREventIDTypeClusterDoorLockEventLockUserChangeID MTR_NEWLY_AVAILABLE = 0x00000004,

    // Cluster PumpConfigurationAndControl deprecated event names
    MTRClusterPumpConfigurationAndControlEventSupplyVoltageLowID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageLowID")
    = 0x00000000,
    MTRClusterPumpConfigurationAndControlEventSupplyVoltageHighID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageHighID")
    = 0x00000001,
    MTRClusterPumpConfigurationAndControlEventPowerMissingPhaseID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventPowerMissingPhaseID")
    = 0x00000002,
    MTRClusterPumpConfigurationAndControlEventSystemPressureLowID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureLowID")
    = 0x00000003,
    MTRClusterPumpConfigurationAndControlEventSystemPressureHighID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureHighID")
    = 0x00000004,
    MTRClusterPumpConfigurationAndControlEventDryRunningID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventDryRunningID")
    = 0x00000005,
    MTRClusterPumpConfigurationAndControlEventMotorTemperatureHighID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventMotorTemperatureHighID")
    = 0x00000006,
    MTRClusterPumpConfigurationAndControlEventPumpMotorFatalFailureID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventPumpMotorFatalFailureID")
    = 0x00000007,
    MTRClusterPumpConfigurationAndControlEventElectronicTemperatureHighID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicTemperatureHighID")
    = 0x00000008,
    MTRClusterPumpConfigurationAndControlEventPumpBlockedID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventPumpBlockedID")
    = 0x00000009,
    MTRClusterPumpConfigurationAndControlEventSensorFailureID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSensorFailureID")
    = 0x0000000A,
    MTRClusterPumpConfigurationAndControlEventElectronicNonFatalFailureID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicNonFatalFailureID")
    = 0x0000000B,
    MTRClusterPumpConfigurationAndControlEventElectronicFatalFailureID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicFatalFailureID")
    = 0x0000000C,
    MTRClusterPumpConfigurationAndControlEventGeneralFaultID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventGeneralFaultID")
    = 0x0000000D,
    MTRClusterPumpConfigurationAndControlEventLeakageID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventLeakageID")
    = 0x0000000E,
    MTRClusterPumpConfigurationAndControlEventAirDetectionID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventAirDetectionID")
    = 0x0000000F,
    MTRClusterPumpConfigurationAndControlEventTurbineOperationID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterPumpConfigurationAndControlEventTurbineOperationID")
    = 0x00000010,

    // Cluster PumpConfigurationAndControl events
    MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageLowID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageHighID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterPumpConfigurationAndControlEventPowerMissingPhaseID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureLowID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureHighID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTREventIDTypeClusterPumpConfigurationAndControlEventDryRunningID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTREventIDTypeClusterPumpConfigurationAndControlEventMotorTemperatureHighID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTREventIDTypeClusterPumpConfigurationAndControlEventPumpMotorFatalFailureID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicTemperatureHighID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTREventIDTypeClusterPumpConfigurationAndControlEventPumpBlockedID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSensorFailureID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicNonFatalFailureID MTR_NEWLY_AVAILABLE = 0x0000000B,
    MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicFatalFailureID MTR_NEWLY_AVAILABLE = 0x0000000C,
    MTREventIDTypeClusterPumpConfigurationAndControlEventGeneralFaultID MTR_NEWLY_AVAILABLE = 0x0000000D,
    MTREventIDTypeClusterPumpConfigurationAndControlEventLeakageID MTR_NEWLY_AVAILABLE = 0x0000000E,
    MTREventIDTypeClusterPumpConfigurationAndControlEventAirDetectionID MTR_NEWLY_AVAILABLE = 0x0000000F,
    MTREventIDTypeClusterPumpConfigurationAndControlEventTurbineOperationID MTR_NEWLY_AVAILABLE = 0x00000010,

    // Cluster TestCluster deprecated event names
    MTRClusterTestClusterEventTestEventID MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterUnitTestingEventTestEventID")
    = 0x00000001,
    MTRClusterTestClusterEventTestFabricScopedEventID MTR_NEWLY_DEPRECATED(
        "Please use MTREventIDTypeClusterUnitTestingEventTestFabricScopedEventID")
    = 0x00000002,

    // Cluster UnitTesting events
    MTREventIDTypeClusterUnitTestingEventTestEventID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterUnitTestingEventTestFabricScopedEventID MTR_NEWLY_AVAILABLE = 0x00000002,

};
