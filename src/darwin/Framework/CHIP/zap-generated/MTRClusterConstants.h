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

#import <Matter/MTRDefines.h>
#import <stdint.h>

#pragma mark - Clusters IDs

typedef NS_ENUM(uint32_t, MTRClusterIDType) {
    MTRClusterIdentifyID MTR_DEPRECATED("Please use MTRClusterIDTypeIdentifyID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000003,
    MTRClusterGroupsID MTR_DEPRECATED("Please use MTRClusterIDTypeGroupsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000004,
    MTRClusterScenesID MTR_DEPRECATED("Please use MTRClusterIDTypeScenesID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000005,
    MTRClusterOnOffID MTR_DEPRECATED("Please use MTRClusterIDTypeOnOffID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000006,
    MTRClusterOnOffSwitchConfigurationID MTR_DEPRECATED("Please use MTRClusterIDTypeOnOffSwitchConfigurationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000007,
    MTRClusterLevelControlID MTR_DEPRECATED("Please use MTRClusterIDTypeLevelControlID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000008,
    MTRClusterBinaryInputBasicID MTR_DEPRECATED("Please use MTRClusterIDTypeBinaryInputBasicID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000000F,
    MTRClusterPulseWidthModulationID MTR_DEPRECATED("Please use MTRClusterIDTypePulseWidthModulationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000001C,
    MTRClusterDescriptorID MTR_DEPRECATED("Please use MTRClusterIDTypeDescriptorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000001D,
    MTRClusterBindingID MTR_DEPRECATED("Please use MTRClusterIDTypeBindingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000001E,
    MTRClusterAccessControlID MTR_DEPRECATED("Please use MTRClusterIDTypeAccessControlID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000001F,
    MTRClusterActionsID MTR_DEPRECATED("Please use MTRClusterIDTypeActionsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000025,
    MTRClusterBasicID MTR_DEPRECATED("Please use MTRClusterIDTypeBasicInformationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000028,
    MTRClusterOtaSoftwareUpdateProviderID MTR_DEPRECATED("Please use MTRClusterIDTypeOTASoftwareUpdateProviderID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000029,
    MTRClusterOtaSoftwareUpdateRequestorID MTR_DEPRECATED("Please use MTRClusterIDTypeOTASoftwareUpdateRequestorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000002A,
    MTRClusterLocalizationConfigurationID MTR_DEPRECATED("Please use MTRClusterIDTypeLocalizationConfigurationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000002B,
    MTRClusterTimeFormatLocalizationID MTR_DEPRECATED("Please use MTRClusterIDTypeTimeFormatLocalizationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000002C,
    MTRClusterUnitLocalizationID MTR_DEPRECATED("Please use MTRClusterIDTypeUnitLocalizationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000002D,
    MTRClusterPowerSourceConfigurationID MTR_DEPRECATED("Please use MTRClusterIDTypePowerSourceConfigurationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000002E,
    MTRClusterPowerSourceID MTR_DEPRECATED("Please use MTRClusterIDTypePowerSourceID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000002F,
    MTRClusterGeneralCommissioningID MTR_DEPRECATED("Please use MTRClusterIDTypeGeneralCommissioningID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000030,
    MTRClusterNetworkCommissioningID MTR_DEPRECATED("Please use MTRClusterIDTypeNetworkCommissioningID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000031,
    MTRClusterDiagnosticLogsID MTR_DEPRECATED("Please use MTRClusterIDTypeDiagnosticLogsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000032,
    MTRClusterGeneralDiagnosticsID MTR_DEPRECATED("Please use MTRClusterIDTypeGeneralDiagnosticsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000033,
    MTRClusterSoftwareDiagnosticsID MTR_DEPRECATED("Please use MTRClusterIDTypeSoftwareDiagnosticsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000034,
    MTRClusterThreadNetworkDiagnosticsID MTR_DEPRECATED("Please use MTRClusterIDTypeThreadNetworkDiagnosticsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000035,
    MTRClusterWiFiNetworkDiagnosticsID MTR_DEPRECATED("Please use MTRClusterIDTypeWiFiNetworkDiagnosticsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000036,
    MTRClusterEthernetNetworkDiagnosticsID MTR_DEPRECATED("Please use MTRClusterIDTypeEthernetNetworkDiagnosticsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000037,
    MTRClusterTimeSynchronizationID MTR_DEPRECATED("Please use MTRClusterIDTypeTimeSynchronizationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000038,
    MTRClusterBridgedDeviceBasicID MTR_DEPRECATED("Please use MTRClusterIDTypeBridgedDeviceBasicInformationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000039,
    MTRClusterSwitchID MTR_DEPRECATED("Please use MTRClusterIDTypeSwitchID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000003B,
    MTRClusterAdministratorCommissioningID MTR_DEPRECATED("Please use MTRClusterIDTypeAdministratorCommissioningID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000003C,
    MTRClusterOperationalCredentialsID MTR_DEPRECATED("Please use MTRClusterIDTypeOperationalCredentialsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000003E,
    MTRClusterGroupKeyManagementID MTR_DEPRECATED("Please use MTRClusterIDTypeGroupKeyManagementID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000003F,
    MTRClusterFixedLabelID MTR_DEPRECATED("Please use MTRClusterIDTypeFixedLabelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000040,
    MTRClusterUserLabelID MTR_DEPRECATED("Please use MTRClusterIDTypeUserLabelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000041,
    MTRClusterBooleanStateID MTR_DEPRECATED("Please use MTRClusterIDTypeBooleanStateID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000045,
    MTRClusterModeSelectID MTR_DEPRECATED("Please use MTRClusterIDTypeModeSelectID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000050,
    MTRClusterDoorLockID MTR_DEPRECATED("Please use MTRClusterIDTypeDoorLockID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000101,
    MTRClusterWindowCoveringID MTR_DEPRECATED("Please use MTRClusterIDTypeWindowCoveringID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000102,
    MTRClusterBarrierControlID MTR_DEPRECATED("Please use MTRClusterIDTypeBarrierControlID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000103,
    MTRClusterPumpConfigurationAndControlID MTR_DEPRECATED("Please use MTRClusterIDTypePumpConfigurationAndControlID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000200,
    MTRClusterThermostatID MTR_DEPRECATED("Please use MTRClusterIDTypeThermostatID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000201,
    MTRClusterFanControlID MTR_DEPRECATED("Please use MTRClusterIDTypeFanControlID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000202,
    MTRClusterThermostatUserInterfaceConfigurationID MTR_DEPRECATED("Please use MTRClusterIDTypeThermostatUserInterfaceConfigurationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000204,
    MTRClusterColorControlID MTR_DEPRECATED("Please use MTRClusterIDTypeColorControlID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000300,
    MTRClusterBallastConfigurationID MTR_DEPRECATED("Please use MTRClusterIDTypeBallastConfigurationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000301,
    MTRClusterIlluminanceMeasurementID MTR_DEPRECATED("Please use MTRClusterIDTypeIlluminanceMeasurementID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000400,
    MTRClusterTemperatureMeasurementID MTR_DEPRECATED("Please use MTRClusterIDTypeTemperatureMeasurementID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000402,
    MTRClusterPressureMeasurementID MTR_DEPRECATED("Please use MTRClusterIDTypePressureMeasurementID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000403,
    MTRClusterFlowMeasurementID MTR_DEPRECATED("Please use MTRClusterIDTypeFlowMeasurementID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000404,
    MTRClusterRelativeHumidityMeasurementID MTR_DEPRECATED("Please use MTRClusterIDTypeRelativeHumidityMeasurementID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000405,
    MTRClusterOccupancySensingID MTR_DEPRECATED("Please use MTRClusterIDTypeOccupancySensingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000406,
    MTRClusterWakeOnLanID MTR_DEPRECATED("Please use MTRClusterIDTypeWakeOnLANID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000503,
    MTRClusterChannelID MTR_DEPRECATED("Please use MTRClusterIDTypeChannelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000504,
    MTRClusterTargetNavigatorID MTR_DEPRECATED("Please use MTRClusterIDTypeTargetNavigatorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000505,
    MTRClusterMediaPlaybackID MTR_DEPRECATED("Please use MTRClusterIDTypeMediaPlaybackID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000506,
    MTRClusterMediaInputID MTR_DEPRECATED("Please use MTRClusterIDTypeMediaInputID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000507,
    MTRClusterLowPowerID MTR_DEPRECATED("Please use MTRClusterIDTypeLowPowerID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000508,
    MTRClusterKeypadInputID MTR_DEPRECATED("Please use MTRClusterIDTypeKeypadInputID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000509,
    MTRClusterContentLauncherID MTR_DEPRECATED("Please use MTRClusterIDTypeContentLauncherID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000050A,
    MTRClusterAudioOutputID MTR_DEPRECATED("Please use MTRClusterIDTypeAudioOutputID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000050B,
    MTRClusterApplicationLauncherID MTR_DEPRECATED("Please use MTRClusterIDTypeApplicationLauncherID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000050C,
    MTRClusterApplicationBasicID MTR_DEPRECATED("Please use MTRClusterIDTypeApplicationBasicID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000050D,
    MTRClusterAccountLoginID MTR_DEPRECATED("Please use MTRClusterIDTypeAccountLoginID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x0000050E,
    MTRClusterElectricalMeasurementID MTR_DEPRECATED("Please use MTRClusterIDTypeElectricalMeasurementID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0x00000B04,
    MTRClusterTestClusterID MTR_DEPRECATED("Please use MTRClusterIDTypeUnitTestingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4)) = 0xFFF1FC05,
    MTRClusterIDTypeIdentifyID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRClusterIDTypeGroupsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRClusterIDTypeScenesID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRClusterIDTypeOnOffID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRClusterIDTypeOnOffSwitchConfigurationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRClusterIDTypeLevelControlID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRClusterIDTypeBinaryInputBasicID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000F,
    MTRClusterIDTypePulseWidthModulationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001C,
    MTRClusterIDTypeDescriptorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001D,
    MTRClusterIDTypeBindingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001E,
    MTRClusterIDTypeAccessControlID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001F,
    MTRClusterIDTypeActionsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000025,
    MTRClusterIDTypeBasicInformationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000028,
    MTRClusterIDTypeOTASoftwareUpdateProviderID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000029,
    MTRClusterIDTypeOTASoftwareUpdateRequestorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002A,
    MTRClusterIDTypeLocalizationConfigurationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002B,
    MTRClusterIDTypeTimeFormatLocalizationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002C,
    MTRClusterIDTypeUnitLocalizationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002D,
    MTRClusterIDTypePowerSourceConfigurationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002E,
    MTRClusterIDTypePowerSourceID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002F,
    MTRClusterIDTypeGeneralCommissioningID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000030,
    MTRClusterIDTypeNetworkCommissioningID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000031,
    MTRClusterIDTypeDiagnosticLogsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000032,
    MTRClusterIDTypeGeneralDiagnosticsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000033,
    MTRClusterIDTypeSoftwareDiagnosticsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000034,
    MTRClusterIDTypeThreadNetworkDiagnosticsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000035,
    MTRClusterIDTypeWiFiNetworkDiagnosticsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000036,
    MTRClusterIDTypeEthernetNetworkDiagnosticsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000037,
    MTRClusterIDTypeTimeSynchronizationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000038,
    MTRClusterIDTypeBridgedDeviceBasicInformationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000039,
    MTRClusterIDTypeSwitchID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003B,
    MTRClusterIDTypeAdministratorCommissioningID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003C,
    MTRClusterIDTypeOperationalCredentialsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003E,
    MTRClusterIDTypeGroupKeyManagementID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003F,
    MTRClusterIDTypeFixedLabelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000040,
    MTRClusterIDTypeUserLabelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000041,
    MTRClusterIDTypeBooleanStateID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000045,
    MTRClusterIDTypeICDManagementID MTR_PROVISIONALLY_AVAILABLE = 0x00000046,
    MTRClusterIDTypeTimerID MTR_PROVISIONALLY_AVAILABLE = 0x00000047,
    MTRClusterIDTypeOvenCavityOperationalStateID MTR_PROVISIONALLY_AVAILABLE = 0x00000048,
    MTRClusterIDTypeOvenModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000049,
    MTRClusterIDTypeLaundryDryerControlsID MTR_PROVISIONALLY_AVAILABLE = 0x0000004A,
    MTRClusterIDTypeModeSelectID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000050,
    MTRClusterIDTypeLaundryWasherModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000051,
    MTRClusterIDTypeRefrigeratorAndTemperatureControlledCabinetModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000052,
    MTRClusterIDTypeLaundryWasherControlsID MTR_PROVISIONALLY_AVAILABLE = 0x00000053,
    MTRClusterIDTypeRVCRunModeID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000054,
    MTRClusterIDTypeRVCCleanModeID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000055,
    MTRClusterIDTypeTemperatureControlID MTR_PROVISIONALLY_AVAILABLE = 0x00000056,
    MTRClusterIDTypeRefrigeratorAlarmID MTR_PROVISIONALLY_AVAILABLE = 0x00000057,
    MTRClusterIDTypeDishwasherModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000059,
    MTRClusterIDTypeAirQualityID MTR_PROVISIONALLY_AVAILABLE = 0x0000005B,
    MTRClusterIDTypeSmokeCOAlarmID MTR_PROVISIONALLY_AVAILABLE = 0x0000005C,
    MTRClusterIDTypeDishwasherAlarmID MTR_PROVISIONALLY_AVAILABLE = 0x0000005D,
    MTRClusterIDTypeMicrowaveOvenModeID MTR_PROVISIONALLY_AVAILABLE = 0x0000005E,
    MTRClusterIDTypeMicrowaveOvenControlID MTR_PROVISIONALLY_AVAILABLE = 0x0000005F,
    MTRClusterIDTypeOperationalStateID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000060,
    MTRClusterIDTypeRVCOperationalStateID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000061,
    MTRClusterIDTypeHEPAFilterMonitoringID MTR_PROVISIONALLY_AVAILABLE = 0x00000071,
    MTRClusterIDTypeActivatedCarbonFilterMonitoringID MTR_PROVISIONALLY_AVAILABLE = 0x00000072,
    MTRClusterIDTypeBooleanStateConfigurationID MTR_PROVISIONALLY_AVAILABLE = 0x00000080,
    MTRClusterIDTypeValveConfigurationAndControlID MTR_PROVISIONALLY_AVAILABLE = 0x00000081,
    MTRClusterIDTypeElectricalEnergyMeasurementID MTR_PROVISIONALLY_AVAILABLE = 0x00000091,
    MTRClusterIDTypeDemandResponseLoadControlID MTR_PROVISIONALLY_AVAILABLE = 0x00000096,
    MTRClusterIDTypeDeviceEnergyManagementID MTR_PROVISIONALLY_AVAILABLE = 0x00000098,
    MTRClusterIDTypeEnergyEVSEID MTR_PROVISIONALLY_AVAILABLE = 0x00000099,
    MTRClusterIDTypeEnergyPreferenceID MTR_PROVISIONALLY_AVAILABLE = 0x0000009B,
    MTRClusterIDTypeDoorLockID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000101,
    MTRClusterIDTypeWindowCoveringID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000102,
    MTRClusterIDTypeBarrierControlID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000103,
    MTRClusterIDTypePumpConfigurationAndControlID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000200,
    MTRClusterIDTypeThermostatID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000201,
    MTRClusterIDTypeFanControlID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000202,
    MTRClusterIDTypeThermostatUserInterfaceConfigurationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000204,
    MTRClusterIDTypeColorControlID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000300,
    MTRClusterIDTypeBallastConfigurationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000301,
    MTRClusterIDTypeIlluminanceMeasurementID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000400,
    MTRClusterIDTypeTemperatureMeasurementID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000402,
    MTRClusterIDTypePressureMeasurementID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000403,
    MTRClusterIDTypeFlowMeasurementID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000404,
    MTRClusterIDTypeRelativeHumidityMeasurementID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000405,
    MTRClusterIDTypeOccupancySensingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000406,
    MTRClusterIDTypeCarbonMonoxideConcentrationMeasurementID MTR_PROVISIONALLY_AVAILABLE = 0x0000040C,
    MTRClusterIDTypeCarbonDioxideConcentrationMeasurementID MTR_PROVISIONALLY_AVAILABLE = 0x0000040D,
    MTRClusterIDTypeNitrogenDioxideConcentrationMeasurementID MTR_PROVISIONALLY_AVAILABLE = 0x00000413,
    MTRClusterIDTypeOzoneConcentrationMeasurementID MTR_PROVISIONALLY_AVAILABLE = 0x00000415,
    MTRClusterIDTypePM25ConcentrationMeasurementID MTR_PROVISIONALLY_AVAILABLE = 0x0000042A,
    MTRClusterIDTypeFormaldehydeConcentrationMeasurementID MTR_PROVISIONALLY_AVAILABLE = 0x0000042B,
    MTRClusterIDTypePM1ConcentrationMeasurementID MTR_PROVISIONALLY_AVAILABLE = 0x0000042C,
    MTRClusterIDTypePM10ConcentrationMeasurementID MTR_PROVISIONALLY_AVAILABLE = 0x0000042D,
    MTRClusterIDTypeTotalVolatileOrganicCompoundsConcentrationMeasurementID MTR_PROVISIONALLY_AVAILABLE = 0x0000042E,
    MTRClusterIDTypeRadonConcentrationMeasurementID MTR_PROVISIONALLY_AVAILABLE = 0x0000042F,
    MTRClusterIDTypeWakeOnLANID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000503,
    MTRClusterIDTypeChannelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000504,
    MTRClusterIDTypeTargetNavigatorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000505,
    MTRClusterIDTypeMediaPlaybackID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000506,
    MTRClusterIDTypeMediaInputID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000507,
    MTRClusterIDTypeLowPowerID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000508,
    MTRClusterIDTypeKeypadInputID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000509,
    MTRClusterIDTypeContentLauncherID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050A,
    MTRClusterIDTypeAudioOutputID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050B,
    MTRClusterIDTypeApplicationLauncherID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050C,
    MTRClusterIDTypeApplicationBasicID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050D,
    MTRClusterIDTypeAccountLoginID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050E,
    MTRClusterIDTypeContentControlID MTR_PROVISIONALLY_AVAILABLE = 0x0000050F,
    MTRClusterIDTypeContentAppObserverID MTR_PROVISIONALLY_AVAILABLE = 0x00000510,
    MTRClusterIDTypeElectricalMeasurementID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000B04,
    MTRClusterIDTypeUnitTestingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0xFFF1FC05,
    MTRClusterIDTypeSampleMEIID MTR_PROVISIONALLY_AVAILABLE = 0xFFF1FC20,
};

#pragma mark - Attributes IDs

typedef NS_ENUM(uint32_t, MTRAttributeIDType) {
    // Deprecated global attribute names
    MTRClusterGlobalAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000FFF8,
    MTRClusterGlobalAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000FFF9,
    MTRClusterGlobalAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000FFFB,
    MTRClusterGlobalAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000FFFC,
    MTRClusterGlobalAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000FFFD,

    // Global attributes
    MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000FFF8,
    MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000FFF9,
    MTRAttributeIDTypeGlobalAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = 0x0000FFFA,
    MTRAttributeIDTypeGlobalAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000FFFB,
    MTRAttributeIDTypeGlobalAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000FFFC,
    MTRAttributeIDTypeGlobalAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000FFFD,

    // Cluster Identify deprecated attribute names
    MTRClusterIdentifyAttributeIdentifyTimeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterIdentifyAttributeIdentifyTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterIdentifyAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterIdentifyAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterIdentifyAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterIdentifyAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterIdentifyAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Identify attributes
    MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterIdentifyAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterIdentifyAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterIdentifyAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterIdentifyAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterIdentifyAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterIdentifyAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Groups deprecated attribute names
    MTRClusterGroupsAttributeNameSupportID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeNameSupportID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGroupsAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGroupsAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGroupsAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGroupsAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGroupsAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Groups attributes
    MTRAttributeIDTypeClusterGroupsAttributeNameSupportID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterGroupsAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGroupsAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGroupsAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterGroupsAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGroupsAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGroupsAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Scenes deprecated attribute names
    MTRClusterScenesAttributeSceneCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeSceneCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterScenesAttributeCurrentSceneID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeCurrentSceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterScenesAttributeCurrentGroupID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeCurrentGroupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterScenesAttributeSceneValidID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeSceneValidID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterScenesAttributeNameSupportID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeNameSupportID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterScenesAttributeLastConfiguredByID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeLastConfiguredByID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterScenesAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterScenesAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterScenesAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterScenesAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterScenesAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Scenes attributes
    MTRAttributeIDTypeClusterScenesAttributeSceneCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterScenesAttributeCurrentSceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterScenesAttributeCurrentGroupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterScenesAttributeSceneValidID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterScenesAttributeNameSupportID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterScenesAttributeLastConfiguredByID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterScenesAttributeSceneTableSizeID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterScenesAttributeFabricSceneInfoID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterScenesAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterScenesAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterScenesAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterScenesAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterScenesAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterScenesAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OnOff deprecated attribute names
    MTRClusterOnOffAttributeOnOffID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeOnOffID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOnOffAttributeGlobalSceneControlID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeGlobalSceneControlID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004000,
    MTRClusterOnOffAttributeOnTimeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeOnTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004001,
    MTRClusterOnOffAttributeOffWaitTimeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeOffWaitTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004002,
    MTRClusterOnOffAttributeStartUpOnOffID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeStartUpOnOffID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004003,
    MTRClusterOnOffAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOnOffAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOnOffAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOnOffAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOnOffAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OnOff attributes
    MTRAttributeIDTypeClusterOnOffAttributeOnOffID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterOnOffAttributeGlobalSceneControlID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004000,
    MTRAttributeIDTypeClusterOnOffAttributeOnTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004001,
    MTRAttributeIDTypeClusterOnOffAttributeOffWaitTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004002,
    MTRAttributeIDTypeClusterOnOffAttributeStartUpOnOffID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004003,
    MTRAttributeIDTypeClusterOnOffAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOnOffAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOnOffAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterOnOffAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOnOffAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOnOffAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OnOffSwitchConfiguration deprecated attribute names
    MTRClusterOnOffSwitchConfigurationAttributeSwitchTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOnOffSwitchConfigurationAttributeSwitchActionsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchActionsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterOnOffSwitchConfigurationAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOnOffSwitchConfigurationAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOnOffSwitchConfigurationAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOnOffSwitchConfigurationAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOnOffSwitchConfigurationAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OnOffSwitchConfiguration attributes
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchActionsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster LevelControl deprecated attribute names
    MTRClusterLevelControlAttributeCurrentLevelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeCurrentLevelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterLevelControlAttributeRemainingTimeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeRemainingTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterLevelControlAttributeMinLevelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeMinLevelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterLevelControlAttributeMaxLevelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeMaxLevelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterLevelControlAttributeCurrentFrequencyID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeCurrentFrequencyID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterLevelControlAttributeMinFrequencyID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeMinFrequencyID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterLevelControlAttributeMaxFrequencyID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeMaxFrequencyID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterLevelControlAttributeOptionsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeOptionsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterLevelControlAttributeOnOffTransitionTimeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeOnOffTransitionTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterLevelControlAttributeOnLevelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeOnLevelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterLevelControlAttributeOnTransitionTimeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeOnTransitionTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterLevelControlAttributeOffTransitionTimeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeOffTransitionTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterLevelControlAttributeDefaultMoveRateID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeDefaultMoveRateID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterLevelControlAttributeStartUpCurrentLevelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeStartUpCurrentLevelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004000,
    MTRClusterLevelControlAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterLevelControlAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterLevelControlAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterLevelControlAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterLevelControlAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster LevelControl attributes
    MTRAttributeIDTypeClusterLevelControlAttributeCurrentLevelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterLevelControlAttributeRemainingTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterLevelControlAttributeMinLevelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterLevelControlAttributeMaxLevelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterLevelControlAttributeCurrentFrequencyID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterLevelControlAttributeMinFrequencyID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterLevelControlAttributeMaxFrequencyID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterLevelControlAttributeOptionsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000F,
    MTRAttributeIDTypeClusterLevelControlAttributeOnOffTransitionTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,
    MTRAttributeIDTypeClusterLevelControlAttributeOnLevelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRAttributeIDTypeClusterLevelControlAttributeOnTransitionTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRAttributeIDTypeClusterLevelControlAttributeOffTransitionTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000013,
    MTRAttributeIDTypeClusterLevelControlAttributeDefaultMoveRateID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000014,
    MTRAttributeIDTypeClusterLevelControlAttributeStartUpCurrentLevelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004000,
    MTRAttributeIDTypeClusterLevelControlAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterLevelControlAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterLevelControlAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterLevelControlAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterLevelControlAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterLevelControlAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BinaryInputBasic deprecated attribute names
    MTRClusterBinaryInputBasicAttributeActiveTextID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeActiveTextID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterBinaryInputBasicAttributeDescriptionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeDescriptionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001C,
    MTRClusterBinaryInputBasicAttributeInactiveTextID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeInactiveTextID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002E,
    MTRClusterBinaryInputBasicAttributeOutOfServiceID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeOutOfServiceID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000051,
    MTRClusterBinaryInputBasicAttributePolarityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributePolarityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000054,
    MTRClusterBinaryInputBasicAttributePresentValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributePresentValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000055,
    MTRClusterBinaryInputBasicAttributeReliabilityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeReliabilityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000067,
    MTRClusterBinaryInputBasicAttributeStatusFlagsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeStatusFlagsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000006F,
    MTRClusterBinaryInputBasicAttributeApplicationTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeApplicationTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000100,
    MTRClusterBinaryInputBasicAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBinaryInputBasicAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBinaryInputBasicAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBinaryInputBasicAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBinaryInputBasicAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BinaryInputBasic attributes
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeActiveTextID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeDescriptionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001C,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeInactiveTextID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002E,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeOutOfServiceID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000051,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributePolarityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000054,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributePresentValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000055,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeReliabilityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000067,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeStatusFlagsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000006F,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeApplicationTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000100,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBinaryInputBasicAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PulseWidthModulation deprecated attribute names
    MTRClusterPulseWidthModulationAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPulseWidthModulationAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPulseWidthModulationAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPulseWidthModulationAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPulseWidthModulationAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster PulseWidthModulation attributes
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPulseWidthModulationAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Descriptor deprecated attribute names
    MTRClusterDescriptorAttributeDeviceTypeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID", ios(16.2, 16.4), macos(13.1, 13.3), watchos(9.2, 9.4), tvos(16.2, 16.4))
    = 0x00000000,
    MTRClusterDescriptorAttributeDeviceListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterDescriptorAttributeServerListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeServerListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterDescriptorAttributeClientListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeClientListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterDescriptorAttributePartsListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributePartsListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterDescriptorAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterDescriptorAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterDescriptorAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterDescriptorAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterDescriptorAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Descriptor attributes
    MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterDescriptorAttributeServerListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterDescriptorAttributeClientListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterDescriptorAttributePartsListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterDescriptorAttributeTagListID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterDescriptorAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDescriptorAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDescriptorAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterDescriptorAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDescriptorAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDescriptorAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Binding deprecated attribute names
    MTRClusterBindingAttributeBindingID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeBindingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterBindingAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBindingAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBindingAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBindingAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBindingAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Binding attributes
    MTRAttributeIDTypeClusterBindingAttributeBindingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterBindingAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBindingAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBindingAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterBindingAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBindingAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBindingAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AccessControl deprecated attribute names
    MTRClusterAccessControlAttributeAclID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeACLID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterAccessControlAttributeExtensionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeExtensionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterAccessControlAttributeSubjectsPerAccessControlEntryID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeSubjectsPerAccessControlEntryID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterAccessControlAttributeTargetsPerAccessControlEntryID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeTargetsPerAccessControlEntryID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterAccessControlAttributeAccessControlEntriesPerFabricID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeAccessControlEntriesPerFabricID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterAccessControlAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAccessControlAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAccessControlAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAccessControlAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAccessControlAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster AccessControl attributes
    MTRAttributeIDTypeClusterAccessControlAttributeACLID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterAccessControlAttributeExtensionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterAccessControlAttributeSubjectsPerAccessControlEntryID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterAccessControlAttributeTargetsPerAccessControlEntryID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterAccessControlAttributeAccessControlEntriesPerFabricID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterAccessControlAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAccessControlAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAccessControlAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterAccessControlAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAccessControlAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAccessControlAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Actions deprecated attribute names
    MTRClusterActionsAttributeActionListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeActionListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterActionsAttributeEndpointListsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeEndpointListsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterActionsAttributeSetupURLID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeSetupURLID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterActionsAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterActionsAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterActionsAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterActionsAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterActionsAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Actions attributes
    MTRAttributeIDTypeClusterActionsAttributeActionListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterActionsAttributeEndpointListsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterActionsAttributeSetupURLID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterActionsAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterActionsAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterActionsAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterActionsAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterActionsAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterActionsAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Basic deprecated attribute names
    MTRClusterBasicAttributeDataModelRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeDataModelRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterBasicAttributeVendorNameID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeVendorNameID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterBasicAttributeVendorIDID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeVendorIDID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterBasicAttributeProductNameID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeProductNameID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterBasicAttributeProductIDID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeProductIDID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterBasicAttributeNodeLabelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeNodeLabelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterBasicAttributeLocationID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeLocationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterBasicAttributeHardwareVersionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeHardwareVersionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterBasicAttributeHardwareVersionStringID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeHardwareVersionStringID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterBasicAttributeSoftwareVersionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeSoftwareVersionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterBasicAttributeSoftwareVersionStringID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeSoftwareVersionStringID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterBasicAttributeManufacturingDateID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeManufacturingDateID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterBasicAttributePartNumberID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributePartNumberID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterBasicAttributeProductURLID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeProductURLID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterBasicAttributeProductLabelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeProductLabelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterBasicAttributeSerialNumberID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeSerialNumberID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterBasicAttributeLocalConfigDisabledID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeLocalConfigDisabledID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterBasicAttributeReachableID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeReachableID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterBasicAttributeUniqueIDID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeUniqueIDID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterBasicAttributeCapabilityMinimaID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeCapabilityMinimaID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterBasicAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBasicAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBasicAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBasicAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBasicAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BasicInformation attributes
    MTRAttributeIDTypeClusterBasicInformationAttributeDataModelRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterBasicInformationAttributeVendorNameID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterBasicInformationAttributeVendorIDID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterBasicInformationAttributeProductNameID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterBasicInformationAttributeProductIDID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterBasicInformationAttributeNodeLabelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterBasicInformationAttributeLocationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterBasicInformationAttributeHardwareVersionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterBasicInformationAttributeHardwareVersionStringID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRAttributeIDTypeClusterBasicInformationAttributeSoftwareVersionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRAttributeIDTypeClusterBasicInformationAttributeSoftwareVersionStringID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRAttributeIDTypeClusterBasicInformationAttributeManufacturingDateID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,
    MTRAttributeIDTypeClusterBasicInformationAttributePartNumberID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000C,
    MTRAttributeIDTypeClusterBasicInformationAttributeProductURLID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000D,
    MTRAttributeIDTypeClusterBasicInformationAttributeProductLabelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000E,
    MTRAttributeIDTypeClusterBasicInformationAttributeSerialNumberID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000F,
    MTRAttributeIDTypeClusterBasicInformationAttributeLocalConfigDisabledID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,
    MTRAttributeIDTypeClusterBasicInformationAttributeReachableID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRAttributeIDTypeClusterBasicInformationAttributeUniqueIDID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRAttributeIDTypeClusterBasicInformationAttributeCapabilityMinimaID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000013,
    MTRAttributeIDTypeClusterBasicInformationAttributeProductAppearanceID MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0)) = 0x00000014,
    MTRAttributeIDTypeClusterBasicInformationAttributeSpecificationVersionID MTR_PROVISIONALLY_AVAILABLE = 0x00000015,
    MTRAttributeIDTypeClusterBasicInformationAttributeMaxPathsPerInvokeID MTR_PROVISIONALLY_AVAILABLE = 0x00000016,
    MTRAttributeIDTypeClusterBasicInformationAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBasicInformationAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBasicInformationAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterBasicInformationAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBasicInformationAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBasicInformationAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OtaSoftwareUpdateProvider deprecated attribute names
    MTRClusterOtaSoftwareUpdateProviderAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOtaSoftwareUpdateProviderAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOtaSoftwareUpdateProviderAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOtaSoftwareUpdateProviderAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOtaSoftwareUpdateProviderAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OTASoftwareUpdateProvider attributes
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OtaSoftwareUpdateRequestor deprecated attribute names
    MTRClusterOtaSoftwareUpdateRequestorAttributeDefaultOtaProvidersID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeDefaultOTAProvidersID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOtaSoftwareUpdateRequestorAttributeUpdatePossibleID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdatePossibleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterOtaSoftwareUpdateRequestorAttributeUpdateStateID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterOtaSoftwareUpdateRequestorAttributeUpdateStateProgressID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateProgressID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterOtaSoftwareUpdateRequestorAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OTASoftwareUpdateRequestor attributes
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeDefaultOTAProvidersID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdatePossibleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateProgressID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster LocalizationConfiguration deprecated attribute names
    MTRClusterLocalizationConfigurationAttributeActiveLocaleID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeActiveLocaleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterLocalizationConfigurationAttributeSupportedLocalesID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeSupportedLocalesID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterLocalizationConfigurationAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterLocalizationConfigurationAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterLocalizationConfigurationAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterLocalizationConfigurationAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterLocalizationConfigurationAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster LocalizationConfiguration attributes
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeActiveLocaleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeSupportedLocalesID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterLocalizationConfigurationAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TimeFormatLocalization deprecated attribute names
    MTRClusterTimeFormatLocalizationAttributeHourFormatID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeHourFormatID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTimeFormatLocalizationAttributeActiveCalendarTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeActiveCalendarTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTimeFormatLocalizationAttributeSupportedCalendarTypesID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeSupportedCalendarTypesID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterTimeFormatLocalizationAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTimeFormatLocalizationAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTimeFormatLocalizationAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTimeFormatLocalizationAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTimeFormatLocalizationAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster TimeFormatLocalization attributes
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeHourFormatID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeActiveCalendarTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeSupportedCalendarTypesID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster UnitLocalization deprecated attribute names
    MTRClusterUnitLocalizationAttributeTemperatureUnitID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeTemperatureUnitID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterUnitLocalizationAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterUnitLocalizationAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterUnitLocalizationAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterUnitLocalizationAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterUnitLocalizationAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster UnitLocalization attributes
    MTRAttributeIDTypeClusterUnitLocalizationAttributeTemperatureUnitID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterUnitLocalizationAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PowerSourceConfiguration deprecated attribute names
    MTRClusterPowerSourceConfigurationAttributeSourcesID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeSourcesID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterPowerSourceConfigurationAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPowerSourceConfigurationAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPowerSourceConfigurationAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPowerSourceConfigurationAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPowerSourceConfigurationAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster PowerSourceConfiguration attributes
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeSourcesID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PowerSource deprecated attribute names
    MTRClusterPowerSourceAttributeStatusID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeStatusID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterPowerSourceAttributeOrderID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeOrderID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterPowerSourceAttributeDescriptionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeDescriptionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterPowerSourceAttributeWiredAssessedInputVoltageID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputVoltageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterPowerSourceAttributeWiredAssessedInputFrequencyID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputFrequencyID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterPowerSourceAttributeWiredCurrentTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredCurrentTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterPowerSourceAttributeWiredAssessedCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterPowerSourceAttributeWiredNominalVoltageID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredNominalVoltageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterPowerSourceAttributeWiredMaximumCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredMaximumCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterPowerSourceAttributeWiredPresentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredPresentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterPowerSourceAttributeActiveWiredFaultsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeActiveWiredFaultsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterPowerSourceAttributeBatVoltageID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatVoltageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterPowerSourceAttributeBatPercentRemainingID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatPercentRemainingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterPowerSourceAttributeBatTimeRemainingID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeRemainingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterPowerSourceAttributeBatChargeLevelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeLevelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterPowerSourceAttributeBatReplacementNeededID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementNeededID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterPowerSourceAttributeBatReplaceabilityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatReplaceabilityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterPowerSourceAttributeBatPresentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatPresentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterPowerSourceAttributeActiveBatFaultsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatFaultsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterPowerSourceAttributeBatReplacementDescriptionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementDescriptionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterPowerSourceAttributeBatCommonDesignationID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatCommonDesignationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterPowerSourceAttributeBatANSIDesignationID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatANSIDesignationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterPowerSourceAttributeBatIECDesignationID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatIECDesignationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000016,
    MTRClusterPowerSourceAttributeBatApprovedChemistryID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatApprovedChemistryID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterPowerSourceAttributeBatCapacityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatCapacityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000018,
    MTRClusterPowerSourceAttributeBatQuantityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatQuantityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000019,
    MTRClusterPowerSourceAttributeBatChargeStateID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeStateID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterPowerSourceAttributeBatTimeToFullChargeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeToFullChargeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001B,
    MTRClusterPowerSourceAttributeBatFunctionalWhileChargingID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatFunctionalWhileChargingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001C,
    MTRClusterPowerSourceAttributeBatChargingCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatChargingCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001D,
    MTRClusterPowerSourceAttributeActiveBatChargeFaultsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatChargeFaultsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001E,
    MTRClusterPowerSourceAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPowerSourceAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPowerSourceAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPowerSourceAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPowerSourceAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster PowerSource attributes
    MTRAttributeIDTypeClusterPowerSourceAttributeStatusID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterPowerSourceAttributeOrderID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterPowerSourceAttributeDescriptionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputVoltageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputFrequencyID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredCurrentTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredNominalVoltageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredMaximumCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRAttributeIDTypeClusterPowerSourceAttributeWiredPresentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRAttributeIDTypeClusterPowerSourceAttributeActiveWiredFaultsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatVoltageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatPercentRemainingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000C,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeRemainingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000D,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeLevelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000E,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementNeededID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000F,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatReplaceabilityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatPresentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatFaultsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementDescriptionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000013,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatCommonDesignationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000014,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatANSIDesignationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000015,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatIECDesignationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000016,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatApprovedChemistryID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000017,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatCapacityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000018,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatQuantityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000019,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeStateID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001A,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeToFullChargeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001B,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatFunctionalWhileChargingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001C,
    MTRAttributeIDTypeClusterPowerSourceAttributeBatChargingCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001D,
    MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatChargeFaultsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001E,
    MTRAttributeIDTypeClusterPowerSourceAttributeEndpointListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x0000001F,
    MTRAttributeIDTypeClusterPowerSourceAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPowerSourceAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterPowerSourceAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPowerSourceAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPowerSourceAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster GeneralCommissioning deprecated attribute names
    MTRClusterGeneralCommissioningAttributeBreadcrumbID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeBreadcrumbID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGeneralCommissioningAttributeBasicCommissioningInfoID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeBasicCommissioningInfoID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGeneralCommissioningAttributeRegulatoryConfigID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeRegulatoryConfigID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGeneralCommissioningAttributeLocationCapabilityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeLocationCapabilityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterGeneralCommissioningAttributeSupportsConcurrentConnectionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeSupportsConcurrentConnectionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterGeneralCommissioningAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGeneralCommissioningAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGeneralCommissioningAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGeneralCommissioningAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGeneralCommissioningAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster GeneralCommissioning attributes
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeBreadcrumbID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeBasicCommissioningInfoID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeRegulatoryConfigID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeLocationCapabilityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeSupportsConcurrentConnectionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGeneralCommissioningAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster NetworkCommissioning deprecated attribute names
    MTRClusterNetworkCommissioningAttributeMaxNetworksID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeMaxNetworksID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterNetworkCommissioningAttributeNetworksID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeNetworksID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterNetworkCommissioningAttributeScanMaxTimeSecondsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeScanMaxTimeSecondsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterNetworkCommissioningAttributeConnectMaxTimeSecondsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeConnectMaxTimeSecondsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterNetworkCommissioningAttributeInterfaceEnabledID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeInterfaceEnabledID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterNetworkCommissioningAttributeLastNetworkingStatusID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkingStatusID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterNetworkCommissioningAttributeLastNetworkIDID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkIDID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterNetworkCommissioningAttributeLastConnectErrorValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastConnectErrorValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterNetworkCommissioningAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterNetworkCommissioningAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterNetworkCommissioningAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterNetworkCommissioningAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterNetworkCommissioningAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster NetworkCommissioning attributes
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeMaxNetworksID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeNetworksID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeScanMaxTimeSecondsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeConnectMaxTimeSecondsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeInterfaceEnabledID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkingStatusID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkIDID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastConnectErrorValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeSupportedWiFiBandsID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeSupportedThreadFeaturesID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeThreadVersionID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterNetworkCommissioningAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster DiagnosticLogs deprecated attribute names
    MTRClusterDiagnosticLogsAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterDiagnosticLogsAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterDiagnosticLogsAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterDiagnosticLogsAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterDiagnosticLogsAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster DiagnosticLogs attributes
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDiagnosticLogsAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster GeneralDiagnostics deprecated attribute names
    MTRClusterGeneralDiagnosticsAttributeNetworkInterfacesID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeNetworkInterfacesID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGeneralDiagnosticsAttributeRebootCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeRebootCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGeneralDiagnosticsAttributeUpTimeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeUpTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGeneralDiagnosticsAttributeTotalOperationalHoursID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTotalOperationalHoursID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterGeneralDiagnosticsAttributeBootReasonsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeBootReasonID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterGeneralDiagnosticsAttributeActiveHardwareFaultsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveHardwareFaultsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterGeneralDiagnosticsAttributeActiveRadioFaultsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveRadioFaultsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterGeneralDiagnosticsAttributeActiveNetworkFaultsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveNetworkFaultsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterGeneralDiagnosticsAttributeTestEventTriggersEnabledID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTestEventTriggersEnabledID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterGeneralDiagnosticsAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGeneralDiagnosticsAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGeneralDiagnosticsAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGeneralDiagnosticsAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGeneralDiagnosticsAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster GeneralDiagnostics attributes
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeNetworkInterfacesID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeRebootCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeUpTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTotalOperationalHoursID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeBootReasonID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveHardwareFaultsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveRadioFaultsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveNetworkFaultsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTestEventTriggersEnabledID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster SoftwareDiagnostics deprecated attribute names
    MTRClusterSoftwareDiagnosticsAttributeThreadMetricsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeThreadMetricsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterSoftwareDiagnosticsAttributeCurrentHeapFreeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapFreeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterSoftwareDiagnosticsAttributeCurrentHeapUsedID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapUsedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterSoftwareDiagnosticsAttributeCurrentHeapHighWatermarkID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapHighWatermarkID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterSoftwareDiagnosticsAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterSoftwareDiagnosticsAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterSoftwareDiagnosticsAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterSoftwareDiagnosticsAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterSoftwareDiagnosticsAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster SoftwareDiagnostics attributes
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeThreadMetricsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapFreeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapUsedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapHighWatermarkID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ThreadNetworkDiagnostics deprecated attribute names
    MTRClusterThreadNetworkDiagnosticsAttributeChannelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterThreadNetworkDiagnosticsAttributeRoutingRoleID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRoutingRoleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterThreadNetworkDiagnosticsAttributeNetworkNameID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNetworkNameID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterThreadNetworkDiagnosticsAttributePanIdID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePanIdID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterThreadNetworkDiagnosticsAttributeExtendedPanIdID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeExtendedPanIdID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterThreadNetworkDiagnosticsAttributeMeshLocalPrefixID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeMeshLocalPrefixID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterThreadNetworkDiagnosticsAttributeOverrunCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOverrunCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterThreadNetworkDiagnosticsAttributeNeighborTableListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNeighborTableID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterThreadNetworkDiagnosticsAttributeRouteTableListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouteTableID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterThreadNetworkDiagnosticsAttributePartitionIdID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterThreadNetworkDiagnosticsAttributeWeightingID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeWeightingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterThreadNetworkDiagnosticsAttributeDataVersionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDataVersionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterThreadNetworkDiagnosticsAttributeStableDataVersionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeStableDataVersionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterThreadNetworkDiagnosticsAttributeLeaderRouterIdID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRouterIdID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterThreadNetworkDiagnosticsAttributeDetachedRoleCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDetachedRoleCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterThreadNetworkDiagnosticsAttributeChildRoleCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChildRoleCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterThreadNetworkDiagnosticsAttributeRouterRoleCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouterRoleCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterThreadNetworkDiagnosticsAttributeLeaderRoleCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRoleCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterThreadNetworkDiagnosticsAttributeAttachAttemptCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttachAttemptCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterThreadNetworkDiagnosticsAttributePartitionIdChangeCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdChangeCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterThreadNetworkDiagnosticsAttributeBetterPartitionAttachAttemptCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeBetterPartitionAttachAttemptCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterThreadNetworkDiagnosticsAttributeParentChangeCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeParentChangeCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterThreadNetworkDiagnosticsAttributeTxTotalCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxTotalCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000016,
    MTRClusterThreadNetworkDiagnosticsAttributeTxUnicastCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxUnicastCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterThreadNetworkDiagnosticsAttributeTxBroadcastCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBroadcastCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000018,
    MTRClusterThreadNetworkDiagnosticsAttributeTxAckRequestedCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckRequestedCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000019,
    MTRClusterThreadNetworkDiagnosticsAttributeTxAckedCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckedCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterThreadNetworkDiagnosticsAttributeTxNoAckRequestedCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxNoAckRequestedCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001B,
    MTRClusterThreadNetworkDiagnosticsAttributeTxDataCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001C,
    MTRClusterThreadNetworkDiagnosticsAttributeTxDataPollCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataPollCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001D,
    MTRClusterThreadNetworkDiagnosticsAttributeTxBeaconCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001E,
    MTRClusterThreadNetworkDiagnosticsAttributeTxBeaconRequestCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconRequestCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001F,
    MTRClusterThreadNetworkDiagnosticsAttributeTxOtherCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxOtherCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000020,
    MTRClusterThreadNetworkDiagnosticsAttributeTxRetryCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxRetryCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000021,
    MTRClusterThreadNetworkDiagnosticsAttributeTxDirectMaxRetryExpiryCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDirectMaxRetryExpiryCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000022,
    MTRClusterThreadNetworkDiagnosticsAttributeTxIndirectMaxRetryExpiryCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxIndirectMaxRetryExpiryCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000023,
    MTRClusterThreadNetworkDiagnosticsAttributeTxErrCcaCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrCcaCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000024,
    MTRClusterThreadNetworkDiagnosticsAttributeTxErrAbortCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrAbortCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000025,
    MTRClusterThreadNetworkDiagnosticsAttributeTxErrBusyChannelCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrBusyChannelCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000026,
    MTRClusterThreadNetworkDiagnosticsAttributeRxTotalCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxTotalCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000027,
    MTRClusterThreadNetworkDiagnosticsAttributeRxUnicastCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxUnicastCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000028,
    MTRClusterThreadNetworkDiagnosticsAttributeRxBroadcastCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBroadcastCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000029,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDataCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002A,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDataPollCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataPollCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002B,
    MTRClusterThreadNetworkDiagnosticsAttributeRxBeaconCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002C,
    MTRClusterThreadNetworkDiagnosticsAttributeRxBeaconRequestCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconRequestCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002D,
    MTRClusterThreadNetworkDiagnosticsAttributeRxOtherCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxOtherCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002E,
    MTRClusterThreadNetworkDiagnosticsAttributeRxAddressFilteredCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxAddressFilteredCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002F,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDestAddrFilteredCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDestAddrFilteredCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000030,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDuplicatedCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDuplicatedCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000031,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrNoFrameCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrNoFrameCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000032,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrUnknownNeighborCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrUnknownNeighborCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000033,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrInvalidSrcAddrCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrInvalidSrcAddrCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000034,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrSecCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrSecCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000035,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrFcsCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrFcsCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000036,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrOtherCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrOtherCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000037,
    MTRClusterThreadNetworkDiagnosticsAttributeActiveTimestampID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveTimestampID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000038,
    MTRClusterThreadNetworkDiagnosticsAttributePendingTimestampID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePendingTimestampID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000039,
    MTRClusterThreadNetworkDiagnosticsAttributeDelayID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDelayID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003A,
    MTRClusterThreadNetworkDiagnosticsAttributeSecurityPolicyID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeSecurityPolicyID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003B,
    MTRClusterThreadNetworkDiagnosticsAttributeChannelPage0MaskID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelPage0MaskID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003C,
    MTRClusterThreadNetworkDiagnosticsAttributeOperationalDatasetComponentsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOperationalDatasetComponentsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003D,
    MTRClusterThreadNetworkDiagnosticsAttributeActiveNetworkFaultsListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveNetworkFaultsListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003E,
    MTRClusterThreadNetworkDiagnosticsAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterThreadNetworkDiagnosticsAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterThreadNetworkDiagnosticsAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterThreadNetworkDiagnosticsAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterThreadNetworkDiagnosticsAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ThreadNetworkDiagnostics attributes
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRoutingRoleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNetworkNameID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePanIdID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeExtendedPanIdID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeMeshLocalPrefixID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOverrunCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNeighborTableID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouteTableID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeWeightingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDataVersionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeStableDataVersionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRouterIdID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDetachedRoleCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChildRoleCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000F,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouterRoleCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRoleCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttachAttemptCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdChangeCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000013,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeBetterPartitionAttachAttemptCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000014,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeParentChangeCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000015,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxTotalCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000016,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxUnicastCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000017,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBroadcastCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000018,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckRequestedCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000019,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckedCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxNoAckRequestedCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataPollCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconRequestCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001F,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxOtherCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000020,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxRetryCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000021,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDirectMaxRetryExpiryCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000022,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxIndirectMaxRetryExpiryCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000023,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrCcaCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000024,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrAbortCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000025,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrBusyChannelCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000026,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxTotalCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000027,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxUnicastCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000028,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBroadcastCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000029,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataPollCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconRequestCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxOtherCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxAddressFilteredCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002F,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDestAddrFilteredCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000030,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDuplicatedCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000031,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrNoFrameCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000032,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrUnknownNeighborCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000033,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrInvalidSrcAddrCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000034,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrSecCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000035,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrFcsCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000036,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrOtherCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000037,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveTimestampID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000038,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePendingTimestampID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000039,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDelayID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003A,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeSecurityPolicyID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003B,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelPage0MaskID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003C,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOperationalDatasetComponentsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003D,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveNetworkFaultsListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003E,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster WiFiNetworkDiagnostics deprecated attribute names
    MTRClusterWiFiNetworkDiagnosticsAttributeBssidID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBSSIDID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterWiFiNetworkDiagnosticsAttributeSecurityTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeSecurityTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterWiFiNetworkDiagnosticsAttributeWiFiVersionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeWiFiVersionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterWiFiNetworkDiagnosticsAttributeChannelNumberID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeChannelNumberID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterWiFiNetworkDiagnosticsAttributeRssiID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeRSSIID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterWiFiNetworkDiagnosticsAttributeBeaconLostCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconLostCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterWiFiNetworkDiagnosticsAttributeBeaconRxCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconRxCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketMulticastRxCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastRxCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketMulticastTxCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastTxCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketUnicastRxCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastRxCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketUnicastTxCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastTxCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterWiFiNetworkDiagnosticsAttributeCurrentMaxRateID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeCurrentMaxRateID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterWiFiNetworkDiagnosticsAttributeOverrunCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeOverrunCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterWiFiNetworkDiagnosticsAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterWiFiNetworkDiagnosticsAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterWiFiNetworkDiagnosticsAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterWiFiNetworkDiagnosticsAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterWiFiNetworkDiagnosticsAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster WiFiNetworkDiagnostics attributes
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBSSIDID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeSecurityTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeWiFiVersionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeChannelNumberID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeRSSIID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconLostCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconRxCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastRxCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastTxCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastRxCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastTxCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeCurrentMaxRateID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeOverrunCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000C,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster EthernetNetworkDiagnostics deprecated attribute names
    MTRClusterEthernetNetworkDiagnosticsAttributePHYRateID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePHYRateID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterEthernetNetworkDiagnosticsAttributeFullDuplexID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFullDuplexID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterEthernetNetworkDiagnosticsAttributePacketRxCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketRxCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterEthernetNetworkDiagnosticsAttributePacketTxCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketTxCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterEthernetNetworkDiagnosticsAttributeTxErrCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTxErrCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterEthernetNetworkDiagnosticsAttributeCollisionCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCollisionCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterEthernetNetworkDiagnosticsAttributeOverrunCountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeOverrunCountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterEthernetNetworkDiagnosticsAttributeCarrierDetectID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCarrierDetectID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterEthernetNetworkDiagnosticsAttributeTimeSinceResetID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTimeSinceResetID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterEthernetNetworkDiagnosticsAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterEthernetNetworkDiagnosticsAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterEthernetNetworkDiagnosticsAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterEthernetNetworkDiagnosticsAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterEthernetNetworkDiagnosticsAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster EthernetNetworkDiagnostics attributes
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePHYRateID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFullDuplexID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketRxCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketTxCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTxErrCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCollisionCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeOverrunCountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCarrierDetectID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTimeSinceResetID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TimeSynchronization deprecated attribute names
    MTRClusterTimeSynchronizationAttributeUTCTimeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeUTCTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTimeSynchronizationAttributeGranularityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeGranularityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTimeSynchronizationAttributeTimeSourceID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeSourceID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterTimeSynchronizationAttributeTrustedTimeNodeIdID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTrustedTimeSourceID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterTimeSynchronizationAttributeDefaultNtpID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeDefaultNTPID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterTimeSynchronizationAttributeTimeZoneID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterTimeSynchronizationAttributeDstOffsetID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeDSTOffsetID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterTimeSynchronizationAttributeLocalTimeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeLocalTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterTimeSynchronizationAttributeTimeZoneDatabaseID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneDatabaseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterTimeSynchronizationAttributeNtpServerPortID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeNTPServerAvailableID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterTimeSynchronizationAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTimeSynchronizationAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTimeSynchronizationAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTimeSynchronizationAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTimeSynchronizationAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster TimeSynchronization attributes
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeUTCTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeGranularityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeSourceID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTrustedTimeSourceID MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0)) = 0x00000003,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTrustedTimeNodeIdID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeClusterTimeSynchronizationAttributeTrustedTimeSourceID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeDefaultNTPID MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0)) = 0x00000004,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeDefaultNtpID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeClusterTimeSynchronizationAttributeDefaultNTPID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeDSTOffsetID MTR_AVAILABLE(ios(16.5), macos(13.4), watchos(9.5), tvos(16.5)) = 0x00000006,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeDstOffsetID MTR_DEPRECATED("Please use MTRAttributeIDTypeTimeSynchronizationAttributeDSTOffsetID", ios(16.4, 16.5), macos(13.3, 13.4), watchos(9.4, 9.5), tvos(16.4, 16.5)) = MTRAttributeIDTypeClusterTimeSynchronizationAttributeDSTOffsetID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeLocalTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneDatabaseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeNTPServerAvailableID MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0)) = 0x00000009,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeNtpServerPortID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeClusterTimeSynchronizationAttributeNTPServerAvailableID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneListMaxSizeID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeDSTOffsetListMaxSizeID MTR_PROVISIONALLY_AVAILABLE = 0x0000000B,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeSupportsDNSResolveID MTR_PROVISIONALLY_AVAILABLE = 0x0000000C,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTimeSynchronizationAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BridgedDeviceBasic deprecated attribute names
    MTRClusterBridgedDeviceBasicAttributeVendorNameID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeVendorNameID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterBridgedDeviceBasicAttributeVendorIDID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeVendorIDID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterBridgedDeviceBasicAttributeProductNameID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductNameID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterBridgedDeviceBasicAttributeNodeLabelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeNodeLabelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterBridgedDeviceBasicAttributeHardwareVersionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeHardwareVersionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterBridgedDeviceBasicAttributeHardwareVersionStringID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeHardwareVersionStringID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterBridgedDeviceBasicAttributeSoftwareVersionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSoftwareVersionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterBridgedDeviceBasicAttributeSoftwareVersionStringID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSoftwareVersionStringID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterBridgedDeviceBasicAttributeManufacturingDateID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeManufacturingDateID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterBridgedDeviceBasicAttributePartNumberID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributePartNumberID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterBridgedDeviceBasicAttributeProductURLID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductURLID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterBridgedDeviceBasicAttributeProductLabelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductLabelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterBridgedDeviceBasicAttributeSerialNumberID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSerialNumberID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterBridgedDeviceBasicAttributeReachableID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeReachableID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterBridgedDeviceBasicAttributeUniqueIDID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeUniqueIDID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterBridgedDeviceBasicAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBridgedDeviceBasicAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBridgedDeviceBasicAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBridgedDeviceBasicAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBridgedDeviceBasicAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BridgedDeviceBasicInformation attributes
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeVendorNameID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeVendorIDID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductNameID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeNodeLabelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeHardwareVersionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeHardwareVersionStringID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSoftwareVersionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSoftwareVersionStringID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeManufacturingDateID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributePartNumberID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000C,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductURLID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000D,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductLabelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000E,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSerialNumberID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000F,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeReachableID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeUniqueIDID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductAppearanceID MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0)) = 0x00000014,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Switch deprecated attribute names
    MTRClusterSwitchAttributeNumberOfPositionsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeNumberOfPositionsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterSwitchAttributeCurrentPositionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeCurrentPositionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterSwitchAttributeMultiPressMaxID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeMultiPressMaxID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterSwitchAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterSwitchAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterSwitchAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterSwitchAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterSwitchAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Switch attributes
    MTRAttributeIDTypeClusterSwitchAttributeNumberOfPositionsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterSwitchAttributeCurrentPositionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterSwitchAttributeMultiPressMaxID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterSwitchAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterSwitchAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterSwitchAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterSwitchAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterSwitchAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterSwitchAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AdministratorCommissioning deprecated attribute names
    MTRClusterAdministratorCommissioningAttributeWindowStatusID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeWindowStatusID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterAdministratorCommissioningAttributeAdminFabricIndexID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminFabricIndexID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterAdministratorCommissioningAttributeAdminVendorIdID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminVendorIdID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterAdministratorCommissioningAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAdministratorCommissioningAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAdministratorCommissioningAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAdministratorCommissioningAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAdministratorCommissioningAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster AdministratorCommissioning attributes
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeWindowStatusID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminFabricIndexID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminVendorIdID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAdministratorCommissioningAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OperationalCredentials deprecated attribute names
    MTRClusterOperationalCredentialsAttributeNOCsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeNOCsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOperationalCredentialsAttributeFabricsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeFabricsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterOperationalCredentialsAttributeSupportedFabricsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeSupportedFabricsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterOperationalCredentialsAttributeCommissionedFabricsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeCommissionedFabricsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterOperationalCredentialsAttributeTrustedRootCertificatesID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeTrustedRootCertificatesID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterOperationalCredentialsAttributeCurrentFabricIndexID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeCurrentFabricIndexID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterOperationalCredentialsAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOperationalCredentialsAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOperationalCredentialsAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOperationalCredentialsAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOperationalCredentialsAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OperationalCredentials attributes
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeNOCsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeFabricsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeSupportedFabricsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeCommissionedFabricsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeTrustedRootCertificatesID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeCurrentFabricIndexID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOperationalCredentialsAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster GroupKeyManagement deprecated attribute names
    MTRClusterGroupKeyManagementAttributeGroupKeyMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupKeyMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGroupKeyManagementAttributeGroupTableID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupTableID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGroupKeyManagementAttributeMaxGroupsPerFabricID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupsPerFabricID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGroupKeyManagementAttributeMaxGroupKeysPerFabricID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupKeysPerFabricID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterGroupKeyManagementAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGroupKeyManagementAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGroupKeyManagementAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGroupKeyManagementAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGroupKeyManagementAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster GroupKeyManagement attributes
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupKeyMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupTableID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupsPerFabricID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupKeysPerFabricID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterGroupKeyManagementAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FixedLabel deprecated attribute names
    MTRClusterFixedLabelAttributeLabelListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFixedLabelAttributeLabelListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterFixedLabelAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFixedLabelAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterFixedLabelAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFixedLabelAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterFixedLabelAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFixedLabelAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterFixedLabelAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFixedLabelAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterFixedLabelAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFixedLabelAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster FixedLabel attributes
    MTRAttributeIDTypeClusterFixedLabelAttributeLabelListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterFixedLabelAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFixedLabelAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFixedLabelAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterFixedLabelAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFixedLabelAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFixedLabelAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster UserLabel deprecated attribute names
    MTRClusterUserLabelAttributeLabelListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUserLabelAttributeLabelListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterUserLabelAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUserLabelAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterUserLabelAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUserLabelAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterUserLabelAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUserLabelAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterUserLabelAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUserLabelAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterUserLabelAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUserLabelAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster UserLabel attributes
    MTRAttributeIDTypeClusterUserLabelAttributeLabelListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterUserLabelAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterUserLabelAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterUserLabelAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterUserLabelAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterUserLabelAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterUserLabelAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BooleanState deprecated attribute names
    MTRClusterBooleanStateAttributeStateValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBooleanStateAttributeStateValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterBooleanStateAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBooleanStateAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBooleanStateAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBooleanStateAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBooleanStateAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBooleanStateAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBooleanStateAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBooleanStateAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBooleanStateAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBooleanStateAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BooleanState attributes
    MTRAttributeIDTypeClusterBooleanStateAttributeStateValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterBooleanStateAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBooleanStateAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBooleanStateAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterBooleanStateAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBooleanStateAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBooleanStateAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ICDManagement attributes
    MTRAttributeIDTypeClusterICDManagementAttributeIdleModeDurationID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterICDManagementAttributeActiveModeDurationID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterICDManagementAttributeActiveModeThresholdID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterICDManagementAttributeRegisteredClientsID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterICDManagementAttributeICDCounterID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterICDManagementAttributeClientsSupportedPerFabricID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterICDManagementAttributeUserActiveModeTriggerHintID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterICDManagementAttributeUserActiveModeTriggerInstructionID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterICDManagementAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterICDManagementAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterICDManagementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterICDManagementAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterICDManagementAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterICDManagementAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Timer attributes
    MTRAttributeIDTypeClusterTimerAttributeSetTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterTimerAttributeTimeRemainingID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterTimerAttributeTimerStateID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterTimerAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTimerAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTimerAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterTimerAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTimerAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTimerAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OvenCavityOperationalState attributes
    MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributePhaseListID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeCurrentPhaseID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeCountdownTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeOperationalStateListID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeOperationalStateID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeOperationalErrorID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOvenCavityOperationalStateAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OvenMode attributes
    MTRAttributeIDTypeClusterOvenModeAttributeSupportedModesID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterOvenModeAttributeCurrentModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterOvenModeAttributeStartUpModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterOvenModeAttributeOnModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterOvenModeAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOvenModeAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOvenModeAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterOvenModeAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOvenModeAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOvenModeAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster LaundryDryerControls attributes
    MTRAttributeIDTypeClusterLaundryDryerControlsAttributeSupportedDrynessLevelsID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterLaundryDryerControlsAttributeSelectedDrynessLevelID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterLaundryDryerControlsAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterLaundryDryerControlsAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterLaundryDryerControlsAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterLaundryDryerControlsAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterLaundryDryerControlsAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterLaundryDryerControlsAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ModeSelect deprecated attribute names
    MTRClusterModeSelectAttributeDescriptionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeDescriptionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterModeSelectAttributeStandardNamespaceID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeStandardNamespaceID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterModeSelectAttributeSupportedModesID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeSupportedModesID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterModeSelectAttributeCurrentModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeCurrentModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterModeSelectAttributeStartUpModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeStartUpModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterModeSelectAttributeOnModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeOnModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterModeSelectAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterModeSelectAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterModeSelectAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterModeSelectAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterModeSelectAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ModeSelect attributes
    MTRAttributeIDTypeClusterModeSelectAttributeDescriptionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterModeSelectAttributeStandardNamespaceID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterModeSelectAttributeSupportedModesID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterModeSelectAttributeCurrentModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterModeSelectAttributeStartUpModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterModeSelectAttributeOnModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterModeSelectAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterModeSelectAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterModeSelectAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterModeSelectAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterModeSelectAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterModeSelectAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster LaundryWasherMode attributes
    MTRAttributeIDTypeClusterLaundryWasherModeAttributeSupportedModesID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterLaundryWasherModeAttributeCurrentModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterLaundryWasherModeAttributeStartUpModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterLaundryWasherModeAttributeOnModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterLaundryWasherModeAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterLaundryWasherModeAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterLaundryWasherModeAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterLaundryWasherModeAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterLaundryWasherModeAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterLaundryWasherModeAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster RefrigeratorAndTemperatureControlledCabinetMode attributes
    MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeSupportedModesID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeCurrentModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeStartUpModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeOnModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster LaundryWasherControls attributes
    MTRAttributeIDTypeClusterLaundryWasherControlsAttributeSpinSpeedsID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterLaundryWasherControlsAttributeSpinSpeedCurrentID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterLaundryWasherControlsAttributeNumberOfRinsesID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterLaundryWasherControlsAttributeSupportedRinsesID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterLaundryWasherControlsAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterLaundryWasherControlsAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterLaundryWasherControlsAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterLaundryWasherControlsAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterLaundryWasherControlsAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterLaundryWasherControlsAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster RVCRunMode attributes
    MTRAttributeIDTypeClusterRVCRunModeAttributeSupportedModesID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000000,
    MTRAttributeIDTypeClusterRVCRunModeAttributeCurrentModeID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000001,
    MTRAttributeIDTypeClusterRVCRunModeAttributeOnModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterRVCRunModeAttributeGeneratedCommandListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterRVCRunModeAttributeAcceptedCommandListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterRVCRunModeAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterRVCRunModeAttributeAttributeListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterRVCRunModeAttributeFeatureMapID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterRVCRunModeAttributeClusterRevisionID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster RVCCleanMode attributes
    MTRAttributeIDTypeClusterRVCCleanModeAttributeSupportedModesID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000000,
    MTRAttributeIDTypeClusterRVCCleanModeAttributeCurrentModeID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000001,
    MTRAttributeIDTypeClusterRVCCleanModeAttributeOnModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterRVCCleanModeAttributeGeneratedCommandListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterRVCCleanModeAttributeAcceptedCommandListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterRVCCleanModeAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterRVCCleanModeAttributeAttributeListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterRVCCleanModeAttributeFeatureMapID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterRVCCleanModeAttributeClusterRevisionID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TemperatureControl attributes
    MTRAttributeIDTypeClusterTemperatureControlAttributeTemperatureSetpointID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterTemperatureControlAttributeMinTemperatureID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterTemperatureControlAttributeMaxTemperatureID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterTemperatureControlAttributeStepID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterTemperatureControlAttributeSelectedTemperatureLevelID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterTemperatureControlAttributeSupportedTemperatureLevelsID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterTemperatureControlAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTemperatureControlAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTemperatureControlAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterTemperatureControlAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTemperatureControlAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTemperatureControlAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster RefrigeratorAlarm attributes
    MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeMaskID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeStateID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeSupportedID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterRefrigeratorAlarmAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster DishwasherMode attributes
    MTRAttributeIDTypeClusterDishwasherModeAttributeSupportedModesID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterDishwasherModeAttributeCurrentModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterDishwasherModeAttributeStartUpModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterDishwasherModeAttributeOnModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterDishwasherModeAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDishwasherModeAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDishwasherModeAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterDishwasherModeAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDishwasherModeAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDishwasherModeAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AirQuality attributes
    MTRAttributeIDTypeClusterAirQualityAttributeAirQualityID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterAirQualityAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAirQualityAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAirQualityAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterAirQualityAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAirQualityAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAirQualityAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster SmokeCOAlarm attributes
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeExpressedStateID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeSmokeStateID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeCOStateID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeBatteryAlertID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeDeviceMutedID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeTestInProgressID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeHardwareFaultAlertID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeEndOfServiceAlertID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeInterconnectSmokeAlarmID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeInterconnectCOAlarmID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeContaminationStateID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeSmokeSensitivityLevelID MTR_PROVISIONALLY_AVAILABLE = 0x0000000B,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeExpiryDateID MTR_PROVISIONALLY_AVAILABLE = 0x0000000C,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterSmokeCOAlarmAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster DishwasherAlarm attributes
    MTRAttributeIDTypeClusterDishwasherAlarmAttributeMaskID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterDishwasherAlarmAttributeLatchID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterDishwasherAlarmAttributeStateID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterDishwasherAlarmAttributeSupportedID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterDishwasherAlarmAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDishwasherAlarmAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDishwasherAlarmAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterDishwasherAlarmAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDishwasherAlarmAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDishwasherAlarmAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster MicrowaveOvenMode attributes
    MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeSupportedModesID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeCurrentModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterMicrowaveOvenModeAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster MicrowaveOvenControl attributes
    MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeCookTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterMicrowaveOvenControlAttributePowerSettingID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeMinPowerID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeMaxPowerID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterMicrowaveOvenControlAttributePowerStepID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterMicrowaveOvenControlAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OperationalState attributes
    MTRAttributeIDTypeClusterOperationalStateAttributePhaseListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000000,
    MTRAttributeIDTypeClusterOperationalStateAttributeCurrentPhaseID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000001,
    MTRAttributeIDTypeClusterOperationalStateAttributeCountdownTimeID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000002,
    MTRAttributeIDTypeClusterOperationalStateAttributeOperationalStateListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000003,
    MTRAttributeIDTypeClusterOperationalStateAttributeOperationalStateID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000004,
    MTRAttributeIDTypeClusterOperationalStateAttributeOperationalErrorID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000005,
    MTRAttributeIDTypeClusterOperationalStateAttributeGeneratedCommandListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOperationalStateAttributeAcceptedCommandListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOperationalStateAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterOperationalStateAttributeAttributeListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOperationalStateAttributeFeatureMapID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOperationalStateAttributeClusterRevisionID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster RVCOperationalState attributes
    MTRAttributeIDTypeClusterRVCOperationalStateAttributePhaseListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000000,
    MTRAttributeIDTypeClusterRVCOperationalStateAttributeCurrentPhaseID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000001,
    MTRAttributeIDTypeClusterRVCOperationalStateAttributeCountdownTimeID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000002,
    MTRAttributeIDTypeClusterRVCOperationalStateAttributeOperationalStateListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000003,
    MTRAttributeIDTypeClusterRVCOperationalStateAttributeOperationalStateID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000004,
    MTRAttributeIDTypeClusterRVCOperationalStateAttributeOperationalErrorID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000005,
    MTRAttributeIDTypeClusterRVCOperationalStateAttributeGeneratedCommandListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterRVCOperationalStateAttributeAcceptedCommandListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterRVCOperationalStateAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterRVCOperationalStateAttributeAttributeListID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterRVCOperationalStateAttributeFeatureMapID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterRVCOperationalStateAttributeClusterRevisionID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster HEPAFilterMonitoring attributes
    MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeConditionID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeDegradationDirectionID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeChangeIndicationID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeInPlaceIndicatorID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeLastChangedTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeReplacementProductListID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterHEPAFilterMonitoringAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ActivatedCarbonFilterMonitoring attributes
    MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeConditionID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeDegradationDirectionID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeChangeIndicationID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeInPlaceIndicatorID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeLastChangedTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeReplacementProductListID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterActivatedCarbonFilterMonitoringAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BooleanStateConfiguration attributes
    MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeCurrentSensitivityLevelID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeSupportedSensitivityLevelsID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeDefaultSensitivityLevelID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeAlarmsActiveID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeAlarmsSuppressedID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeAlarmsEnabledID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeAlarmsSupportedID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeSensorFaultID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBooleanStateConfigurationAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ValveConfigurationAndControl attributes
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeOpenDurationID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeDefaultOpenDurationID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeAutoCloseTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeRemainingDurationID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeCurrentStateID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeTargetStateID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeCurrentLevelID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeTargetLevelID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeDefaultOpenLevelID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeValveFaultID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterValveConfigurationAndControlAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ElectricalEnergyMeasurement attributes
    MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeAccuracyID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeCumulativeEnergyImportedID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeCumulativeEnergyExportedID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributePeriodicEnergyImportedID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributePeriodicEnergyExportedID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterElectricalEnergyMeasurementAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster DemandResponseLoadControl attributes
    MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeLoadControlProgramsID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeNumberOfLoadControlProgramsID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeEventsID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeActiveEventsID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeNumberOfEventsPerProgramID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeNumberOfTransitionsID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeDefaultRandomStartID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeDefaultRandomDurationID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDemandResponseLoadControlAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster DeviceEnergyManagement attributes
    MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeESATypeID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeESACanGenerateID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeESAStateID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeAbsMinPowerID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeAbsMaxPowerID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterDeviceEnergyManagementAttributePowerAdjustmentCapabilityID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeForecastID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDeviceEnergyManagementAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster EnergyEVSE attributes
    MTRAttributeIDTypeClusterEnergyEVSEAttributeStateID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeSupplyStateID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeFaultStateID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeChargingEnabledUntilID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeDischargingEnabledUntilID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeCircuitCapacityID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeMinimumChargeCurrentID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeMaximumChargeCurrentID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeMaximumDischargeCurrentID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeUserMaximumChargeCurrentID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeRandomizationDelayWindowID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeNumberOfWeeklyTargetsID MTR_PROVISIONALLY_AVAILABLE = 0x00000021,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeNumberOfDailyTargetsID MTR_PROVISIONALLY_AVAILABLE = 0x00000022,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeNextChargeStartTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000023,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeNextChargeTargetTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000024,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeNextChargeRequiredEnergyID MTR_PROVISIONALLY_AVAILABLE = 0x00000025,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeNextChargeTargetSoCID MTR_PROVISIONALLY_AVAILABLE = 0x00000026,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeApproximateEVEfficiencyID MTR_PROVISIONALLY_AVAILABLE = 0x00000027,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeStateOfChargeID MTR_PROVISIONALLY_AVAILABLE = 0x00000030,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeBatteryCapacityID MTR_PROVISIONALLY_AVAILABLE = 0x00000031,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeVehicleIDID MTR_PROVISIONALLY_AVAILABLE = 0x00000032,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeSessionIDID MTR_PROVISIONALLY_AVAILABLE = 0x00000040,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeSessionDurationID MTR_PROVISIONALLY_AVAILABLE = 0x00000041,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeSessionEnergyChargedID MTR_PROVISIONALLY_AVAILABLE = 0x00000042,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeSessionEnergyDischargedID MTR_PROVISIONALLY_AVAILABLE = 0x00000043,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterEnergyEVSEAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster EnergyPreference attributes
    MTRAttributeIDTypeClusterEnergyPreferenceAttributeEnergyBalancesID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterEnergyPreferenceAttributeCurrentEnergyBalanceID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterEnergyPreferenceAttributeEnergyPrioritiesID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterEnergyPreferenceAttributeLowPowerModeSensitivitiesID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterEnergyPreferenceAttributeCurrentLowPowerModeSensitivityID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterEnergyPreferenceAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterEnergyPreferenceAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterEnergyPreferenceAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterEnergyPreferenceAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterEnergyPreferenceAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterEnergyPreferenceAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster DoorLock deprecated attribute names
    MTRClusterDoorLockAttributeLockStateID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLockStateID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterDoorLockAttributeLockTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLockTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterDoorLockAttributeActuatorEnabledID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeActuatorEnabledID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterDoorLockAttributeDoorStateID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeDoorStateID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterDoorLockAttributeDoorOpenEventsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeDoorOpenEventsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterDoorLockAttributeDoorClosedEventsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeDoorClosedEventsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterDoorLockAttributeOpenPeriodID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeOpenPeriodID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterDoorLockAttributeNumberOfTotalUsersSupportedID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfTotalUsersSupportedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterDoorLockAttributeNumberOfPINUsersSupportedID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfPINUsersSupportedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterDoorLockAttributeNumberOfRFIDUsersSupportedID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfRFIDUsersSupportedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterDoorLockAttributeNumberOfWeekDaySchedulesSupportedPerUserID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfWeekDaySchedulesSupportedPerUserID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterDoorLockAttributeNumberOfYearDaySchedulesSupportedPerUserID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfYearDaySchedulesSupportedPerUserID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterDoorLockAttributeNumberOfHolidaySchedulesSupportedID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfHolidaySchedulesSupportedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000016,
    MTRClusterDoorLockAttributeMaxPINCodeLengthID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeMaxPINCodeLengthID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterDoorLockAttributeMinPINCodeLengthID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeMinPINCodeLengthID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000018,
    MTRClusterDoorLockAttributeMaxRFIDCodeLengthID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeMaxRFIDCodeLengthID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000019,
    MTRClusterDoorLockAttributeMinRFIDCodeLengthID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeMinRFIDCodeLengthID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterDoorLockAttributeCredentialRulesSupportID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeCredentialRulesSupportID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001B,
    MTRClusterDoorLockAttributeNumberOfCredentialsSupportedPerUserID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfCredentialsSupportedPerUserID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001C,
    MTRClusterDoorLockAttributeLanguageID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLanguageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000021,
    MTRClusterDoorLockAttributeLEDSettingsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLEDSettingsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000022,
    MTRClusterDoorLockAttributeAutoRelockTimeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeAutoRelockTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000023,
    MTRClusterDoorLockAttributeSoundVolumeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeSoundVolumeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000024,
    MTRClusterDoorLockAttributeOperatingModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeOperatingModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000025,
    MTRClusterDoorLockAttributeSupportedOperatingModesID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeSupportedOperatingModesID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000026,
    MTRClusterDoorLockAttributeDefaultConfigurationRegisterID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeDefaultConfigurationRegisterID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000027,
    MTRClusterDoorLockAttributeEnableLocalProgrammingID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeEnableLocalProgrammingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000028,
    MTRClusterDoorLockAttributeEnableOneTouchLockingID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeEnableOneTouchLockingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000029,
    MTRClusterDoorLockAttributeEnableInsideStatusLEDID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeEnableInsideStatusLEDID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002A,
    MTRClusterDoorLockAttributeEnablePrivacyModeButtonID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeEnablePrivacyModeButtonID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002B,
    MTRClusterDoorLockAttributeLocalProgrammingFeaturesID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLocalProgrammingFeaturesID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002C,
    MTRClusterDoorLockAttributeWrongCodeEntryLimitID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeWrongCodeEntryLimitID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000030,
    MTRClusterDoorLockAttributeUserCodeTemporaryDisableTimeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeUserCodeTemporaryDisableTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000031,
    MTRClusterDoorLockAttributeSendPINOverTheAirID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeSendPINOverTheAirID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000032,
    MTRClusterDoorLockAttributeRequirePINforRemoteOperationID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeRequirePINforRemoteOperationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000033,
    MTRClusterDoorLockAttributeExpiringUserTimeoutID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeExpiringUserTimeoutID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000035,
    MTRClusterDoorLockAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterDoorLockAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterDoorLockAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterDoorLockAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterDoorLockAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster DoorLock attributes
    MTRAttributeIDTypeClusterDoorLockAttributeLockStateID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterDoorLockAttributeLockTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterDoorLockAttributeActuatorEnabledID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterDoorLockAttributeDoorStateID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterDoorLockAttributeDoorOpenEventsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterDoorLockAttributeDoorClosedEventsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterDoorLockAttributeOpenPeriodID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfTotalUsersSupportedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfPINUsersSupportedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfRFIDUsersSupportedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000013,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfWeekDaySchedulesSupportedPerUserID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000014,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfYearDaySchedulesSupportedPerUserID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000015,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfHolidaySchedulesSupportedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000016,
    MTRAttributeIDTypeClusterDoorLockAttributeMaxPINCodeLengthID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000017,
    MTRAttributeIDTypeClusterDoorLockAttributeMinPINCodeLengthID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000018,
    MTRAttributeIDTypeClusterDoorLockAttributeMaxRFIDCodeLengthID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000019,
    MTRAttributeIDTypeClusterDoorLockAttributeMinRFIDCodeLengthID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001A,
    MTRAttributeIDTypeClusterDoorLockAttributeCredentialRulesSupportID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001B,
    MTRAttributeIDTypeClusterDoorLockAttributeNumberOfCredentialsSupportedPerUserID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001C,
    MTRAttributeIDTypeClusterDoorLockAttributeLanguageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000021,
    MTRAttributeIDTypeClusterDoorLockAttributeLEDSettingsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000022,
    MTRAttributeIDTypeClusterDoorLockAttributeAutoRelockTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000023,
    MTRAttributeIDTypeClusterDoorLockAttributeSoundVolumeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000024,
    MTRAttributeIDTypeClusterDoorLockAttributeOperatingModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000025,
    MTRAttributeIDTypeClusterDoorLockAttributeSupportedOperatingModesID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000026,
    MTRAttributeIDTypeClusterDoorLockAttributeDefaultConfigurationRegisterID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000027,
    MTRAttributeIDTypeClusterDoorLockAttributeEnableLocalProgrammingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000028,
    MTRAttributeIDTypeClusterDoorLockAttributeEnableOneTouchLockingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000029,
    MTRAttributeIDTypeClusterDoorLockAttributeEnableInsideStatusLEDID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002A,
    MTRAttributeIDTypeClusterDoorLockAttributeEnablePrivacyModeButtonID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002B,
    MTRAttributeIDTypeClusterDoorLockAttributeLocalProgrammingFeaturesID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002C,
    MTRAttributeIDTypeClusterDoorLockAttributeWrongCodeEntryLimitID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000030,
    MTRAttributeIDTypeClusterDoorLockAttributeUserCodeTemporaryDisableTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000031,
    MTRAttributeIDTypeClusterDoorLockAttributeSendPINOverTheAirID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000032,
    MTRAttributeIDTypeClusterDoorLockAttributeRequirePINforRemoteOperationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000033,
    MTRAttributeIDTypeClusterDoorLockAttributeExpiringUserTimeoutID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000035,
    MTRAttributeIDTypeClusterDoorLockAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterDoorLockAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterDoorLockAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterDoorLockAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterDoorLockAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterDoorLockAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster WindowCovering deprecated attribute names
    MTRClusterWindowCoveringAttributeTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterWindowCoveringAttributePhysicalClosedLimitLiftID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitLiftID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterWindowCoveringAttributePhysicalClosedLimitTiltID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitTiltID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterWindowCoveringAttributeCurrentPositionLiftID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterWindowCoveringAttributeCurrentPositionTiltID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterWindowCoveringAttributeNumberOfActuationsLiftID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsLiftID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterWindowCoveringAttributeNumberOfActuationsTiltID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsTiltID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterWindowCoveringAttributeConfigStatusID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeConfigStatusID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterWindowCoveringAttributeCurrentPositionLiftPercentageID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercentageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterWindowCoveringAttributeCurrentPositionTiltPercentageID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercentageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterWindowCoveringAttributeOperationalStatusID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeOperationalStatusID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterWindowCoveringAttributeTargetPositionLiftPercent100thsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionLiftPercent100thsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterWindowCoveringAttributeTargetPositionTiltPercent100thsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionTiltPercent100thsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterWindowCoveringAttributeEndProductTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeEndProductTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterWindowCoveringAttributeCurrentPositionLiftPercent100thsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercent100thsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterWindowCoveringAttributeCurrentPositionTiltPercent100thsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercent100thsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterWindowCoveringAttributeInstalledOpenLimitLiftID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitLiftID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterWindowCoveringAttributeInstalledClosedLimitLiftID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitLiftID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterWindowCoveringAttributeInstalledOpenLimitTiltID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitTiltID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterWindowCoveringAttributeInstalledClosedLimitTiltID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitTiltID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterWindowCoveringAttributeModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterWindowCoveringAttributeSafetyStatusID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeSafetyStatusID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterWindowCoveringAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterWindowCoveringAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterWindowCoveringAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterWindowCoveringAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterWindowCoveringAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster WindowCovering attributes
    MTRAttributeIDTypeClusterWindowCoveringAttributeTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitLiftID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitTiltID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsLiftID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsTiltID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterWindowCoveringAttributeConfigStatusID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercentageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercentageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRAttributeIDTypeClusterWindowCoveringAttributeOperationalStatusID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionLiftPercent100thsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,
    MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionTiltPercent100thsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000C,
    MTRAttributeIDTypeClusterWindowCoveringAttributeEndProductTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000D,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercent100thsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000E,
    MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercent100thsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000F,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitLiftID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitLiftID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitTiltID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitTiltID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000013,
    MTRAttributeIDTypeClusterWindowCoveringAttributeModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000017,
    MTRAttributeIDTypeClusterWindowCoveringAttributeSafetyStatusID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001A,
    MTRAttributeIDTypeClusterWindowCoveringAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterWindowCoveringAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BarrierControl deprecated attribute names
    MTRClusterBarrierControlAttributeBarrierMovingStateID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierMovingStateID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterBarrierControlAttributeBarrierSafetyStatusID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierSafetyStatusID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterBarrierControlAttributeBarrierCapabilitiesID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCapabilitiesID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterBarrierControlAttributeBarrierOpenEventsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenEventsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterBarrierControlAttributeBarrierCloseEventsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCloseEventsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterBarrierControlAttributeBarrierCommandOpenEventsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandOpenEventsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterBarrierControlAttributeBarrierCommandCloseEventsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandCloseEventsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterBarrierControlAttributeBarrierOpenPeriodID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenPeriodID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterBarrierControlAttributeBarrierClosePeriodID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierClosePeriodID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterBarrierControlAttributeBarrierPositionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierPositionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterBarrierControlAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBarrierControlAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBarrierControlAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBarrierControlAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBarrierControlAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BarrierControl attributes
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierMovingStateID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierSafetyStatusID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCapabilitiesID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenEventsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCloseEventsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandOpenEventsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandCloseEventsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenPeriodID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierClosePeriodID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRAttributeIDTypeClusterBarrierControlAttributeBarrierPositionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRAttributeIDTypeClusterBarrierControlAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBarrierControlAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBarrierControlAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterBarrierControlAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBarrierControlAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBarrierControlAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PumpConfigurationAndControl deprecated attribute names
    MTRClusterPumpConfigurationAndControlAttributeMaxPressureID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxPressureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterPumpConfigurationAndControlAttributeMaxSpeedID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxSpeedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterPumpConfigurationAndControlAttributeMaxFlowID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxFlowID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterPumpConfigurationAndControlAttributeMinConstPressureID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstPressureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstPressureID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstPressureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterPumpConfigurationAndControlAttributeMinCompPressureID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinCompPressureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterPumpConfigurationAndControlAttributeMaxCompPressureID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxCompPressureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterPumpConfigurationAndControlAttributeMinConstSpeedID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstSpeedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstSpeedID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstSpeedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterPumpConfigurationAndControlAttributeMinConstFlowID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstFlowID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstFlowID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstFlowID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterPumpConfigurationAndControlAttributeMinConstTempID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstTempID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstTempID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstTempID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterPumpConfigurationAndControlAttributePumpStatusID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePumpStatusID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterPumpConfigurationAndControlAttributeEffectiveOperationModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveOperationModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterPumpConfigurationAndControlAttributeEffectiveControlModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveControlModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterPumpConfigurationAndControlAttributeCapacityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeCapacityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterPumpConfigurationAndControlAttributeSpeedID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeSpeedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterPumpConfigurationAndControlAttributeLifetimeRunningHoursID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeRunningHoursID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterPumpConfigurationAndControlAttributePowerID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePowerID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000016,
    MTRClusterPumpConfigurationAndControlAttributeLifetimeEnergyConsumedID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeEnergyConsumedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterPumpConfigurationAndControlAttributeOperationModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeOperationModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000020,
    MTRClusterPumpConfigurationAndControlAttributeControlModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeControlModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000021,
    MTRClusterPumpConfigurationAndControlAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPumpConfigurationAndControlAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPumpConfigurationAndControlAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPumpConfigurationAndControlAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPumpConfigurationAndControlAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster PumpConfigurationAndControl attributes
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxPressureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxSpeedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxFlowID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstPressureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstPressureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinCompPressureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxCompPressureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstSpeedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstSpeedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstFlowID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstFlowID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstTempID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstTempID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000C,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePumpStatusID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveOperationModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveControlModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeCapacityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000013,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeSpeedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000014,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeRunningHoursID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000015,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePowerID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000016,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeEnergyConsumedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000017,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeOperationModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000020,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeControlModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000021,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Thermostat deprecated attribute names
    MTRClusterThermostatAttributeLocalTemperatureID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterThermostatAttributeOutdoorTemperatureID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeOutdoorTemperatureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterThermostatAttributeOccupancyID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeOccupancyID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterThermostatAttributeAbsMinHeatSetpointLimitID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMinHeatSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterThermostatAttributeAbsMaxHeatSetpointLimitID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMaxHeatSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterThermostatAttributeAbsMinCoolSetpointLimitID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMinCoolSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterThermostatAttributeAbsMaxCoolSetpointLimitID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMaxCoolSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterThermostatAttributePICoolingDemandID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributePICoolingDemandID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterThermostatAttributePIHeatingDemandID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributePIHeatingDemandID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterThermostatAttributeHVACSystemTypeConfigurationID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeHVACSystemTypeConfigurationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterThermostatAttributeLocalTemperatureCalibrationID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureCalibrationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterThermostatAttributeOccupiedCoolingSetpointID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedCoolingSetpointID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterThermostatAttributeOccupiedHeatingSetpointID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedHeatingSetpointID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterThermostatAttributeUnoccupiedCoolingSetpointID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedCoolingSetpointID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterThermostatAttributeUnoccupiedHeatingSetpointID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedHeatingSetpointID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterThermostatAttributeMinHeatSetpointLimitID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeMinHeatSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterThermostatAttributeMaxHeatSetpointLimitID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeMaxHeatSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000016,
    MTRClusterThermostatAttributeMinCoolSetpointLimitID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeMinCoolSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterThermostatAttributeMaxCoolSetpointLimitID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeMaxCoolSetpointLimitID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000018,
    MTRClusterThermostatAttributeMinSetpointDeadBandID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeMinSetpointDeadBandID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000019,
    MTRClusterThermostatAttributeRemoteSensingID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeRemoteSensingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterThermostatAttributeControlSequenceOfOperationID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeControlSequenceOfOperationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001B,
    MTRClusterThermostatAttributeSystemModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeSystemModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001C,
    MTRClusterThermostatAttributeThermostatRunningModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001E,
    MTRClusterThermostatAttributeStartOfWeekID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeStartOfWeekID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000020,
    MTRClusterThermostatAttributeNumberOfWeeklyTransitionsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeNumberOfWeeklyTransitionsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000021,
    MTRClusterThermostatAttributeNumberOfDailyTransitionsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeNumberOfDailyTransitionsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000022,
    MTRClusterThermostatAttributeTemperatureSetpointHoldID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000023,
    MTRClusterThermostatAttributeTemperatureSetpointHoldDurationID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldDurationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000024,
    MTRClusterThermostatAttributeThermostatProgrammingOperationModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeThermostatProgrammingOperationModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000025,
    MTRClusterThermostatAttributeThermostatRunningStateID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningStateID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000029,
    MTRClusterThermostatAttributeSetpointChangeSourceID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000030,
    MTRClusterThermostatAttributeSetpointChangeAmountID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeAmountID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000031,
    MTRClusterThermostatAttributeSetpointChangeSourceTimestampID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceTimestampID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000032,
    MTRClusterThermostatAttributeOccupiedSetbackID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000034,
    MTRClusterThermostatAttributeOccupiedSetbackMinID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMinID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000035,
    MTRClusterThermostatAttributeOccupiedSetbackMaxID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMaxID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000036,
    MTRClusterThermostatAttributeUnoccupiedSetbackID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000037,
    MTRClusterThermostatAttributeUnoccupiedSetbackMinID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMinID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000038,
    MTRClusterThermostatAttributeUnoccupiedSetbackMaxID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMaxID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000039,
    MTRClusterThermostatAttributeEmergencyHeatDeltaID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeEmergencyHeatDeltaID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003A,
    MTRClusterThermostatAttributeACTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000040,
    MTRClusterThermostatAttributeACCapacityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACCapacityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000041,
    MTRClusterThermostatAttributeACRefrigerantTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACRefrigerantTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000042,
    MTRClusterThermostatAttributeACCompressorTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACCompressorTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000043,
    MTRClusterThermostatAttributeACErrorCodeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACErrorCodeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000044,
    MTRClusterThermostatAttributeACLouverPositionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACLouverPositionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000045,
    MTRClusterThermostatAttributeACCoilTemperatureID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACCoilTemperatureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000046,
    MTRClusterThermostatAttributeACCapacityformatID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACCapacityformatID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000047,
    MTRClusterThermostatAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterThermostatAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterThermostatAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterThermostatAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterThermostatAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Thermostat attributes
    MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterThermostatAttributeOutdoorTemperatureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterThermostatAttributeOccupancyID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMinHeatSetpointLimitID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMaxHeatSetpointLimitID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMinCoolSetpointLimitID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterThermostatAttributeAbsMaxCoolSetpointLimitID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterThermostatAttributePICoolingDemandID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterThermostatAttributePIHeatingDemandID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRAttributeIDTypeClusterThermostatAttributeHVACSystemTypeConfigurationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureCalibrationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedCoolingSetpointID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedHeatingSetpointID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedCoolingSetpointID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000013,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedHeatingSetpointID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000014,
    MTRAttributeIDTypeClusterThermostatAttributeMinHeatSetpointLimitID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000015,
    MTRAttributeIDTypeClusterThermostatAttributeMaxHeatSetpointLimitID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000016,
    MTRAttributeIDTypeClusterThermostatAttributeMinCoolSetpointLimitID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000017,
    MTRAttributeIDTypeClusterThermostatAttributeMaxCoolSetpointLimitID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000018,
    MTRAttributeIDTypeClusterThermostatAttributeMinSetpointDeadBandID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000019,
    MTRAttributeIDTypeClusterThermostatAttributeRemoteSensingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001A,
    MTRAttributeIDTypeClusterThermostatAttributeControlSequenceOfOperationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001B,
    MTRAttributeIDTypeClusterThermostatAttributeSystemModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001C,
    MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001E,
    MTRAttributeIDTypeClusterThermostatAttributeStartOfWeekID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000020,
    MTRAttributeIDTypeClusterThermostatAttributeNumberOfWeeklyTransitionsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000021,
    MTRAttributeIDTypeClusterThermostatAttributeNumberOfDailyTransitionsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000022,
    MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000023,
    MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldDurationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000024,
    MTRAttributeIDTypeClusterThermostatAttributeThermostatProgrammingOperationModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000025,
    MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningStateID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000029,
    MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000030,
    MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeAmountID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000031,
    MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceTimestampID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000032,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000034,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMinID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000035,
    MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMaxID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000036,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000037,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMinID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000038,
    MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMaxID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000039,
    MTRAttributeIDTypeClusterThermostatAttributeEmergencyHeatDeltaID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003A,
    MTRAttributeIDTypeClusterThermostatAttributeACTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000040,
    MTRAttributeIDTypeClusterThermostatAttributeACCapacityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000041,
    MTRAttributeIDTypeClusterThermostatAttributeACRefrigerantTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000042,
    MTRAttributeIDTypeClusterThermostatAttributeACCompressorTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000043,
    MTRAttributeIDTypeClusterThermostatAttributeACErrorCodeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000044,
    MTRAttributeIDTypeClusterThermostatAttributeACLouverPositionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000045,
    MTRAttributeIDTypeClusterThermostatAttributeACCoilTemperatureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000046,
    MTRAttributeIDTypeClusterThermostatAttributeACCapacityformatID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000047,
    MTRAttributeIDTypeClusterThermostatAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterThermostatAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterThermostatAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterThermostatAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterThermostatAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterThermostatAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FanControl deprecated attribute names
    MTRClusterFanControlAttributeFanModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeFanModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterFanControlAttributeFanModeSequenceID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeFanModeSequenceID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterFanControlAttributePercentSettingID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributePercentSettingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterFanControlAttributePercentCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributePercentCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterFanControlAttributeSpeedMaxID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeSpeedMaxID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterFanControlAttributeSpeedSettingID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeSpeedSettingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterFanControlAttributeSpeedCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeSpeedCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterFanControlAttributeRockSupportID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeRockSupportID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterFanControlAttributeRockSettingID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeRockSettingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterFanControlAttributeWindSupportID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeWindSupportID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterFanControlAttributeWindSettingID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeWindSettingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterFanControlAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterFanControlAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterFanControlAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterFanControlAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterFanControlAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster FanControl attributes
    MTRAttributeIDTypeClusterFanControlAttributeFanModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterFanControlAttributeFanModeSequenceID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterFanControlAttributePercentSettingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterFanControlAttributePercentCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterFanControlAttributeSpeedMaxID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterFanControlAttributeSpeedSettingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterFanControlAttributeSpeedCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterFanControlAttributeRockSupportID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterFanControlAttributeRockSettingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRAttributeIDTypeClusterFanControlAttributeWindSupportID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRAttributeIDTypeClusterFanControlAttributeWindSettingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRAttributeIDTypeClusterFanControlAttributeAirflowDirectionID MTR_PROVISIONALLY_AVAILABLE = 0x0000000B,
    MTRAttributeIDTypeClusterFanControlAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFanControlAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFanControlAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterFanControlAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFanControlAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFanControlAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ThermostatUserInterfaceConfiguration deprecated attribute names
    MTRClusterThermostatUserInterfaceConfigurationAttributeTemperatureDisplayModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeTemperatureDisplayModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterThermostatUserInterfaceConfigurationAttributeKeypadLockoutID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeKeypadLockoutID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterThermostatUserInterfaceConfigurationAttributeScheduleProgrammingVisibilityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeScheduleProgrammingVisibilityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterThermostatUserInterfaceConfigurationAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ThermostatUserInterfaceConfiguration attributes
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeTemperatureDisplayModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeKeypadLockoutID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeScheduleProgrammingVisibilityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ColorControl deprecated attribute names
    MTRClusterColorControlAttributeCurrentHueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentHueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterColorControlAttributeCurrentSaturationID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentSaturationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterColorControlAttributeRemainingTimeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeRemainingTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterColorControlAttributeCurrentXID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentXID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterColorControlAttributeCurrentYID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentYID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterColorControlAttributeDriftCompensationID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeDriftCompensationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterColorControlAttributeCompensationTextID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeCompensationTextID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterColorControlAttributeColorTemperatureMiredsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorTemperatureMiredsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterColorControlAttributeColorModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterColorControlAttributeOptionsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeOptionsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterColorControlAttributeNumberOfPrimariesID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeNumberOfPrimariesID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterColorControlAttributePrimary1XID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary1XID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterColorControlAttributePrimary1YID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary1YID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterColorControlAttributePrimary1IntensityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary1IntensityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterColorControlAttributePrimary2XID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary2XID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterColorControlAttributePrimary2YID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary2YID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000016,
    MTRClusterColorControlAttributePrimary2IntensityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary2IntensityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterColorControlAttributePrimary3XID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary3XID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000019,
    MTRClusterColorControlAttributePrimary3YID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary3YID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterColorControlAttributePrimary3IntensityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary3IntensityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001B,
    MTRClusterColorControlAttributePrimary4XID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary4XID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000020,
    MTRClusterColorControlAttributePrimary4YID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary4YID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000021,
    MTRClusterColorControlAttributePrimary4IntensityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary4IntensityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000022,
    MTRClusterColorControlAttributePrimary5XID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary5XID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000024,
    MTRClusterColorControlAttributePrimary5YID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary5YID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000025,
    MTRClusterColorControlAttributePrimary5IntensityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary5IntensityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000026,
    MTRClusterColorControlAttributePrimary6XID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary6XID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000028,
    MTRClusterColorControlAttributePrimary6YID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary6YID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000029,
    MTRClusterColorControlAttributePrimary6IntensityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary6IntensityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002A,
    MTRClusterColorControlAttributeWhitePointXID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeWhitePointXID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000030,
    MTRClusterColorControlAttributeWhitePointYID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeWhitePointYID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000031,
    MTRClusterColorControlAttributeColorPointRXID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointRXID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000032,
    MTRClusterColorControlAttributeColorPointRYID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointRYID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000033,
    MTRClusterColorControlAttributeColorPointRIntensityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointRIntensityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000034,
    MTRClusterColorControlAttributeColorPointGXID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointGXID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000036,
    MTRClusterColorControlAttributeColorPointGYID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointGYID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000037,
    MTRClusterColorControlAttributeColorPointGIntensityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointGIntensityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000038,
    MTRClusterColorControlAttributeColorPointBXID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointBXID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003A,
    MTRClusterColorControlAttributeColorPointBYID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointBYID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003B,
    MTRClusterColorControlAttributeColorPointBIntensityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointBIntensityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000003C,
    MTRClusterColorControlAttributeEnhancedCurrentHueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeEnhancedCurrentHueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004000,
    MTRClusterColorControlAttributeEnhancedColorModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeEnhancedColorModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004001,
    MTRClusterColorControlAttributeColorLoopActiveID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopActiveID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004002,
    MTRClusterColorControlAttributeColorLoopDirectionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopDirectionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004003,
    MTRClusterColorControlAttributeColorLoopTimeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004004,
    MTRClusterColorControlAttributeColorLoopStartEnhancedHueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopStartEnhancedHueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004005,
    MTRClusterColorControlAttributeColorLoopStoredEnhancedHueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopStoredEnhancedHueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004006,
    MTRClusterColorControlAttributeColorCapabilitiesID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorCapabilitiesID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400A,
    MTRClusterColorControlAttributeColorTempPhysicalMinMiredsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMinMiredsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400B,
    MTRClusterColorControlAttributeColorTempPhysicalMaxMiredsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMaxMiredsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400C,
    MTRClusterColorControlAttributeCoupleColorTempToLevelMinMiredsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeCoupleColorTempToLevelMinMiredsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400D,
    MTRClusterColorControlAttributeStartUpColorTemperatureMiredsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeStartUpColorTemperatureMiredsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004010,
    MTRClusterColorControlAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterColorControlAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterColorControlAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterColorControlAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterColorControlAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ColorControl attributes
    MTRAttributeIDTypeClusterColorControlAttributeCurrentHueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterColorControlAttributeCurrentSaturationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterColorControlAttributeRemainingTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterColorControlAttributeCurrentXID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterColorControlAttributeCurrentYID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterColorControlAttributeDriftCompensationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterColorControlAttributeCompensationTextID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterColorControlAttributeColorTemperatureMiredsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterColorControlAttributeColorModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRAttributeIDTypeClusterColorControlAttributeOptionsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000F,
    MTRAttributeIDTypeClusterColorControlAttributeNumberOfPrimariesID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,
    MTRAttributeIDTypeClusterColorControlAttributePrimary1XID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRAttributeIDTypeClusterColorControlAttributePrimary1YID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRAttributeIDTypeClusterColorControlAttributePrimary1IntensityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000013,
    MTRAttributeIDTypeClusterColorControlAttributePrimary2XID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000015,
    MTRAttributeIDTypeClusterColorControlAttributePrimary2YID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000016,
    MTRAttributeIDTypeClusterColorControlAttributePrimary2IntensityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000017,
    MTRAttributeIDTypeClusterColorControlAttributePrimary3XID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000019,
    MTRAttributeIDTypeClusterColorControlAttributePrimary3YID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001A,
    MTRAttributeIDTypeClusterColorControlAttributePrimary3IntensityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001B,
    MTRAttributeIDTypeClusterColorControlAttributePrimary4XID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000020,
    MTRAttributeIDTypeClusterColorControlAttributePrimary4YID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000021,
    MTRAttributeIDTypeClusterColorControlAttributePrimary4IntensityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000022,
    MTRAttributeIDTypeClusterColorControlAttributePrimary5XID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000024,
    MTRAttributeIDTypeClusterColorControlAttributePrimary5YID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000025,
    MTRAttributeIDTypeClusterColorControlAttributePrimary5IntensityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000026,
    MTRAttributeIDTypeClusterColorControlAttributePrimary6XID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000028,
    MTRAttributeIDTypeClusterColorControlAttributePrimary6YID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000029,
    MTRAttributeIDTypeClusterColorControlAttributePrimary6IntensityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002A,
    MTRAttributeIDTypeClusterColorControlAttributeWhitePointXID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000030,
    MTRAttributeIDTypeClusterColorControlAttributeWhitePointYID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000031,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointRXID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000032,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointRYID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000033,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointRIntensityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000034,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointGXID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000036,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointGYID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000037,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointGIntensityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000038,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointBXID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003A,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointBYID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003B,
    MTRAttributeIDTypeClusterColorControlAttributeColorPointBIntensityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000003C,
    MTRAttributeIDTypeClusterColorControlAttributeEnhancedCurrentHueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004000,
    MTRAttributeIDTypeClusterColorControlAttributeEnhancedColorModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004001,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopActiveID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004002,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopDirectionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004003,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004004,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopStartEnhancedHueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004005,
    MTRAttributeIDTypeClusterColorControlAttributeColorLoopStoredEnhancedHueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004006,
    MTRAttributeIDTypeClusterColorControlAttributeColorCapabilitiesID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000400A,
    MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMinMiredsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000400B,
    MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMaxMiredsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000400C,
    MTRAttributeIDTypeClusterColorControlAttributeCoupleColorTempToLevelMinMiredsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000400D,
    MTRAttributeIDTypeClusterColorControlAttributeStartUpColorTemperatureMiredsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004010,
    MTRAttributeIDTypeClusterColorControlAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterColorControlAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterColorControlAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterColorControlAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterColorControlAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterColorControlAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster BallastConfiguration deprecated attribute names
    MTRClusterBallastConfigurationAttributePhysicalMinLevelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMinLevelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterBallastConfigurationAttributePhysicalMaxLevelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMaxLevelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterBallastConfigurationAttributeBallastStatusID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastStatusID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterBallastConfigurationAttributeMinLevelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeMinLevelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterBallastConfigurationAttributeMaxLevelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeMaxLevelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterBallastConfigurationAttributeIntrinsicBalanceFactorID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeIntrinsicBallastFactorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterBallastConfigurationAttributeBallastFactorAdjustmentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastFactorAdjustmentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterBallastConfigurationAttributeLampQuantityID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampQuantityID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000020,
    MTRClusterBallastConfigurationAttributeLampTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000030,
    MTRClusterBallastConfigurationAttributeLampManufacturerID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampManufacturerID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000031,
    MTRClusterBallastConfigurationAttributeLampRatedHoursID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampRatedHoursID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000032,
    MTRClusterBallastConfigurationAttributeLampBurnHoursID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000033,
    MTRClusterBallastConfigurationAttributeLampAlarmModeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampAlarmModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000034,
    MTRClusterBallastConfigurationAttributeLampBurnHoursTripPointID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursTripPointID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000035,
    MTRClusterBallastConfigurationAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBallastConfigurationAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBallastConfigurationAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBallastConfigurationAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBallastConfigurationAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BallastConfiguration attributes
    MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMinLevelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMaxLevelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastStatusID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeMinLevelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeMaxLevelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeIntrinsicBallastFactorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000014,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastFactorAdjustmentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000015,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampQuantityID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000020,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000030,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampManufacturerID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000031,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampRatedHoursID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000032,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000033,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampAlarmModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000034,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursTripPointID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000035,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBallastConfigurationAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster IlluminanceMeasurement deprecated attribute names
    MTRClusterIlluminanceMeasurementAttributeMeasuredValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterIlluminanceMeasurementAttributeMinMeasuredValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMinMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterIlluminanceMeasurementAttributeMaxMeasuredValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMaxMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterIlluminanceMeasurementAttributeToleranceID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeToleranceID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterIlluminanceMeasurementAttributeLightSensorTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeLightSensorTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterIlluminanceMeasurementAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterIlluminanceMeasurementAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterIlluminanceMeasurementAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterIlluminanceMeasurementAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterIlluminanceMeasurementAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster IlluminanceMeasurement attributes
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMeasuredValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMinMeasuredValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMaxMeasuredValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeToleranceID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeLightSensorTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TemperatureMeasurement deprecated attribute names
    MTRClusterTemperatureMeasurementAttributeMeasuredValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTemperatureMeasurementAttributeMinMeasuredValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMinMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTemperatureMeasurementAttributeMaxMeasuredValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMaxMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterTemperatureMeasurementAttributeToleranceID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeToleranceID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterTemperatureMeasurementAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTemperatureMeasurementAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTemperatureMeasurementAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTemperatureMeasurementAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTemperatureMeasurementAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster TemperatureMeasurement attributes
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMeasuredValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMinMeasuredValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMaxMeasuredValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeToleranceID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTemperatureMeasurementAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PressureMeasurement deprecated attribute names
    MTRClusterPressureMeasurementAttributeMeasuredValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterPressureMeasurementAttributeMinMeasuredValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMinMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterPressureMeasurementAttributeMaxMeasuredValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterPressureMeasurementAttributeToleranceID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeToleranceID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterPressureMeasurementAttributeScaledValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterPressureMeasurementAttributeMinScaledValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMinScaledValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterPressureMeasurementAttributeMaxScaledValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxScaledValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterPressureMeasurementAttributeScaledToleranceID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledToleranceID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterPressureMeasurementAttributeScaleID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeScaleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterPressureMeasurementAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPressureMeasurementAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPressureMeasurementAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPressureMeasurementAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPressureMeasurementAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster PressureMeasurement attributes
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMeasuredValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMinMeasuredValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxMeasuredValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeToleranceID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMinScaledValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxScaledValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledToleranceID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000013,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeScaleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000014,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPressureMeasurementAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FlowMeasurement deprecated attribute names
    MTRClusterFlowMeasurementAttributeMeasuredValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterFlowMeasurementAttributeMinMeasuredValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeMinMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterFlowMeasurementAttributeMaxMeasuredValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeMaxMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterFlowMeasurementAttributeToleranceID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeToleranceID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterFlowMeasurementAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterFlowMeasurementAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterFlowMeasurementAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterFlowMeasurementAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterFlowMeasurementAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster FlowMeasurement attributes
    MTRAttributeIDTypeClusterFlowMeasurementAttributeMeasuredValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeMinMeasuredValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeMaxMeasuredValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeToleranceID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFlowMeasurementAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster RelativeHumidityMeasurement deprecated attribute names
    MTRClusterRelativeHumidityMeasurementAttributeMeasuredValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterRelativeHumidityMeasurementAttributeMinMeasuredValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMinMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterRelativeHumidityMeasurementAttributeMaxMeasuredValueID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMaxMeasuredValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterRelativeHumidityMeasurementAttributeToleranceID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeToleranceID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterRelativeHumidityMeasurementAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterRelativeHumidityMeasurementAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterRelativeHumidityMeasurementAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterRelativeHumidityMeasurementAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterRelativeHumidityMeasurementAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster RelativeHumidityMeasurement attributes
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMeasuredValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMinMeasuredValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMaxMeasuredValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeToleranceID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OccupancySensing deprecated attribute names
    MTRClusterOccupancySensingAttributeOccupancyID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancyID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOccupancySensingAttributeOccupancySensorTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterOccupancySensingAttributeOccupancySensorTypeBitmapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeBitmapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterOccupancySensingAttributePirOccupiedToUnoccupiedDelayID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributePIROccupiedToUnoccupiedDelayID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterOccupancySensingAttributePirUnoccupiedToOccupiedDelayID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedDelayID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterOccupancySensingAttributePirUnoccupiedToOccupiedThresholdID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedThresholdID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterOccupancySensingAttributeUltrasonicOccupiedToUnoccupiedDelayID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicOccupiedToUnoccupiedDelayID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000020,
    MTRClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedDelayID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedDelayID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000021,
    MTRClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedThresholdID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedThresholdID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000022,
    MTRClusterOccupancySensingAttributePhysicalContactOccupiedToUnoccupiedDelayID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactOccupiedToUnoccupiedDelayID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000030,
    MTRClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedDelayID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedDelayID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000031,
    MTRClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedThresholdID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedThresholdID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000032,
    MTRClusterOccupancySensingAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOccupancySensingAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOccupancySensingAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOccupancySensingAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOccupancySensingAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OccupancySensing attributes
    MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancyID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeBitmapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterOccupancySensingAttributePIROccupiedToUnoccupiedDelayID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,
    MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedDelayID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedThresholdID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicOccupiedToUnoccupiedDelayID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000020,
    MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedDelayID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000021,
    MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedThresholdID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000022,
    MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactOccupiedToUnoccupiedDelayID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000030,
    MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedDelayID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000031,
    MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedThresholdID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000032,
    MTRAttributeIDTypeClusterOccupancySensingAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOccupancySensingAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster CarbonMonoxideConcentrationMeasurement attributes
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeMinMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeMaxMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributePeakMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributePeakMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeAverageMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeAverageMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeUncertaintyID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeMeasurementUnitID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeMeasurementMediumID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeLevelValueID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterCarbonMonoxideConcentrationMeasurementAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster CarbonDioxideConcentrationMeasurement attributes
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeMinMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeMaxMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributePeakMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributePeakMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeAverageMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeAverageMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeUncertaintyID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeMeasurementUnitID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeMeasurementMediumID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeLevelValueID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterCarbonDioxideConcentrationMeasurementAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster NitrogenDioxideConcentrationMeasurement attributes
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeMinMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeMaxMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributePeakMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributePeakMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeAverageMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeAverageMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeUncertaintyID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeMeasurementUnitID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeMeasurementMediumID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeLevelValueID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterNitrogenDioxideConcentrationMeasurementAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OzoneConcentrationMeasurement attributes
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeMinMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeMaxMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributePeakMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributePeakMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeAverageMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeAverageMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeUncertaintyID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeMeasurementUnitID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeMeasurementMediumID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeLevelValueID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterOzoneConcentrationMeasurementAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PM25ConcentrationMeasurement attributes
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeMinMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeMaxMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributePeakMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributePeakMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeAverageMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeAverageMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeUncertaintyID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeMeasurementUnitID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeMeasurementMediumID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeLevelValueID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPM25ConcentrationMeasurementAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster FormaldehydeConcentrationMeasurement attributes
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeMinMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeMaxMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributePeakMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributePeakMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeAverageMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeAverageMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeUncertaintyID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeMeasurementUnitID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeMeasurementMediumID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeLevelValueID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterFormaldehydeConcentrationMeasurementAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PM1ConcentrationMeasurement attributes
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeMinMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeMaxMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributePeakMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributePeakMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeAverageMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeAverageMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeUncertaintyID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeMeasurementUnitID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeMeasurementMediumID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeLevelValueID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPM1ConcentrationMeasurementAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster PM10ConcentrationMeasurement attributes
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeMinMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeMaxMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributePeakMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributePeakMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeAverageMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeAverageMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeUncertaintyID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeMeasurementUnitID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeMeasurementMediumID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeLevelValueID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterPM10ConcentrationMeasurementAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TotalVolatileOrganicCompoundsConcentrationMeasurement attributes
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeMinMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeMaxMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributePeakMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributePeakMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeAverageMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeAverageMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeUncertaintyID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeMeasurementUnitID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeMeasurementMediumID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeLevelValueID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTotalVolatileOrganicCompoundsConcentrationMeasurementAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster RadonConcentrationMeasurement attributes
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeMinMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeMaxMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributePeakMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributePeakMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeAverageMeasuredValueID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeAverageMeasuredValueWindowID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeUncertaintyID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeMeasurementUnitID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeMeasurementMediumID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeLevelValueID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterRadonConcentrationMeasurementAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster WakeOnLan deprecated attribute names
    MTRClusterWakeOnLanAttributeMACAddressID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWakeOnLANAttributeMACAddressID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterWakeOnLanAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWakeOnLANAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterWakeOnLanAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWakeOnLANAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterWakeOnLanAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWakeOnLANAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterWakeOnLanAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWakeOnLANAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterWakeOnLanAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterWakeOnLANAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster WakeOnLAN attributes
    MTRAttributeIDTypeClusterWakeOnLANAttributeMACAddressID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterWakeOnLANAttributeLinkLocalAddressID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterWakeOnLANAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterWakeOnLANAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Channel deprecated attribute names
    MTRClusterChannelAttributeChannelListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeChannelListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterChannelAttributeLineupID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeLineupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterChannelAttributeCurrentChannelID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeCurrentChannelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterChannelAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterChannelAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterChannelAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterChannelAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterChannelAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster Channel attributes
    MTRAttributeIDTypeClusterChannelAttributeChannelListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterChannelAttributeLineupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterChannelAttributeCurrentChannelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterChannelAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterChannelAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterChannelAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterChannelAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterChannelAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterChannelAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TargetNavigator deprecated attribute names
    MTRClusterTargetNavigatorAttributeTargetListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeTargetListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTargetNavigatorAttributeCurrentTargetID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeCurrentTargetID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTargetNavigatorAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTargetNavigatorAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTargetNavigatorAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTargetNavigatorAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTargetNavigatorAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster TargetNavigator attributes
    MTRAttributeIDTypeClusterTargetNavigatorAttributeTargetListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeCurrentTargetID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterTargetNavigatorAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster MediaPlayback deprecated attribute names
    MTRClusterMediaPlaybackAttributeCurrentStateID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeCurrentStateID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterMediaPlaybackAttributeStartTimeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeStartTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterMediaPlaybackAttributeDurationID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeDurationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterMediaPlaybackAttributeSampledPositionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeSampledPositionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterMediaPlaybackAttributePlaybackSpeedID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributePlaybackSpeedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterMediaPlaybackAttributeSeekRangeEndID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeEndID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterMediaPlaybackAttributeSeekRangeStartID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeStartID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterMediaPlaybackAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterMediaPlaybackAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterMediaPlaybackAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterMediaPlaybackAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterMediaPlaybackAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster MediaPlayback attributes
    MTRAttributeIDTypeClusterMediaPlaybackAttributeCurrentStateID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeStartTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeDurationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeSampledPositionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterMediaPlaybackAttributePlaybackSpeedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeEndID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeStartID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeActiveAudioTrackID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeAvailableAudioTracksID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeActiveTextTrackID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeAvailableTextTracksID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterMediaPlaybackAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster MediaInput deprecated attribute names
    MTRClusterMediaInputAttributeInputListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeInputListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterMediaInputAttributeCurrentInputID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeCurrentInputID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterMediaInputAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterMediaInputAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterMediaInputAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterMediaInputAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterMediaInputAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster MediaInput attributes
    MTRAttributeIDTypeClusterMediaInputAttributeInputListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterMediaInputAttributeCurrentInputID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterMediaInputAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterMediaInputAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterMediaInputAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterMediaInputAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterMediaInputAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterMediaInputAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster LowPower deprecated attribute names
    MTRClusterLowPowerAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLowPowerAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterLowPowerAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLowPowerAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterLowPowerAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLowPowerAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterLowPowerAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLowPowerAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterLowPowerAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterLowPowerAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster LowPower attributes
    MTRAttributeIDTypeClusterLowPowerAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterLowPowerAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterLowPowerAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterLowPowerAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterLowPowerAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterLowPowerAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster KeypadInput deprecated attribute names
    MTRClusterKeypadInputAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterKeypadInputAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterKeypadInputAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterKeypadInputAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterKeypadInputAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterKeypadInputAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterKeypadInputAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterKeypadInputAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterKeypadInputAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterKeypadInputAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster KeypadInput attributes
    MTRAttributeIDTypeClusterKeypadInputAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterKeypadInputAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterKeypadInputAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterKeypadInputAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterKeypadInputAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterKeypadInputAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ContentLauncher deprecated attribute names
    MTRClusterContentLauncherAttributeAcceptHeaderID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterContentLauncherAttributeAcceptHeaderID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterContentLauncherAttributeSupportedStreamingProtocolsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterContentLauncherAttributeSupportedStreamingProtocolsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterContentLauncherAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterContentLauncherAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterContentLauncherAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterContentLauncherAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterContentLauncherAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterContentLauncherAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterContentLauncherAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterContentLauncherAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterContentLauncherAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterContentLauncherAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ContentLauncher attributes
    MTRAttributeIDTypeClusterContentLauncherAttributeAcceptHeaderID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterContentLauncherAttributeSupportedStreamingProtocolsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterContentLauncherAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterContentLauncherAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterContentLauncherAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterContentLauncherAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterContentLauncherAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterContentLauncherAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AudioOutput deprecated attribute names
    MTRClusterAudioOutputAttributeOutputListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAudioOutputAttributeOutputListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterAudioOutputAttributeCurrentOutputID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAudioOutputAttributeCurrentOutputID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterAudioOutputAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAudioOutputAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAudioOutputAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAudioOutputAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAudioOutputAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAudioOutputAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAudioOutputAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAudioOutputAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAudioOutputAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAudioOutputAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster AudioOutput attributes
    MTRAttributeIDTypeClusterAudioOutputAttributeOutputListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterAudioOutputAttributeCurrentOutputID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterAudioOutputAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAudioOutputAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAudioOutputAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterAudioOutputAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAudioOutputAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAudioOutputAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ApplicationLauncher deprecated attribute names
    MTRClusterApplicationLauncherAttributeCatalogListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeCatalogListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterApplicationLauncherAttributeCurrentAppID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeCurrentAppID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterApplicationLauncherAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterApplicationLauncherAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterApplicationLauncherAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterApplicationLauncherAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterApplicationLauncherAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ApplicationLauncher attributes
    MTRAttributeIDTypeClusterApplicationLauncherAttributeCatalogListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeCurrentAppID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterApplicationLauncherAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ApplicationBasic deprecated attribute names
    MTRClusterApplicationBasicAttributeVendorNameID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeVendorNameID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterApplicationBasicAttributeVendorIDID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeVendorIDID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterApplicationBasicAttributeApplicationNameID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationNameID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterApplicationBasicAttributeProductIDID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeProductIDID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterApplicationBasicAttributeApplicationID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterApplicationBasicAttributeStatusID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeStatusID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterApplicationBasicAttributeApplicationVersionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationVersionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterApplicationBasicAttributeAllowedVendorListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeAllowedVendorListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterApplicationBasicAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterApplicationBasicAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterApplicationBasicAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterApplicationBasicAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterApplicationBasicAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ApplicationBasic attributes
    MTRAttributeIDTypeClusterApplicationBasicAttributeVendorNameID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterApplicationBasicAttributeVendorIDID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationNameID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterApplicationBasicAttributeProductIDID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterApplicationBasicAttributeStatusID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationVersionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterApplicationBasicAttributeAllowedVendorListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterApplicationBasicAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterApplicationBasicAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster AccountLogin deprecated attribute names
    MTRClusterAccountLoginAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAccountLoginAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAccountLoginAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAccountLoginAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAccountLoginAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAccountLoginAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAccountLoginAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAccountLoginAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAccountLoginAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterAccountLoginAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster AccountLogin attributes
    MTRAttributeIDTypeClusterAccountLoginAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterAccountLoginAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterAccountLoginAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterAccountLoginAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterAccountLoginAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterAccountLoginAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ContentControl attributes
    MTRAttributeIDTypeClusterContentControlAttributeEnabledID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterContentControlAttributeOnDemandRatingsID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterContentControlAttributeOnDemandRatingThresholdID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterContentControlAttributeScheduledContentRatingsID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterContentControlAttributeScheduledContentRatingThresholdID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterContentControlAttributeScreenDailyTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterContentControlAttributeRemainingScreenTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterContentControlAttributeBlockUnratedID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterContentControlAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterContentControlAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterContentControlAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterContentControlAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterContentControlAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterContentControlAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ContentAppObserver attributes
    MTRAttributeIDTypeClusterContentAppObserverAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterContentAppObserverAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterContentAppObserverAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterContentAppObserverAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterContentAppObserverAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterContentAppObserverAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster ElectricalMeasurement deprecated attribute names
    MTRClusterElectricalMeasurementAttributeMeasurementTypeID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasurementTypeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterElectricalMeasurementAttributeDcVoltageID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000100,
    MTRClusterElectricalMeasurementAttributeDcVoltageMinID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMinID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000101,
    MTRClusterElectricalMeasurementAttributeDcVoltageMaxID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMaxID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000102,
    MTRClusterElectricalMeasurementAttributeDcCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000103,
    MTRClusterElectricalMeasurementAttributeDcCurrentMinID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMinID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000104,
    MTRClusterElectricalMeasurementAttributeDcCurrentMaxID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMaxID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000105,
    MTRClusterElectricalMeasurementAttributeDcPowerID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000106,
    MTRClusterElectricalMeasurementAttributeDcPowerMinID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMinID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000107,
    MTRClusterElectricalMeasurementAttributeDcPowerMaxID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMaxID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000108,
    MTRClusterElectricalMeasurementAttributeDcVoltageMultiplierID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMultiplierID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000200,
    MTRClusterElectricalMeasurementAttributeDcVoltageDivisorID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageDivisorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000201,
    MTRClusterElectricalMeasurementAttributeDcCurrentMultiplierID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMultiplierID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000202,
    MTRClusterElectricalMeasurementAttributeDcCurrentDivisorID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentDivisorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000203,
    MTRClusterElectricalMeasurementAttributeDcPowerMultiplierID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMultiplierID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000204,
    MTRClusterElectricalMeasurementAttributeDcPowerDivisorID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerDivisorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000205,
    MTRClusterElectricalMeasurementAttributeAcFrequencyID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000300,
    MTRClusterElectricalMeasurementAttributeAcFrequencyMinID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMinID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000301,
    MTRClusterElectricalMeasurementAttributeAcFrequencyMaxID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMaxID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000302,
    MTRClusterElectricalMeasurementAttributeNeutralCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeNeutralCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000303,
    MTRClusterElectricalMeasurementAttributeTotalActivePowerID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalActivePowerID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000304,
    MTRClusterElectricalMeasurementAttributeTotalReactivePowerID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalReactivePowerID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000305,
    MTRClusterElectricalMeasurementAttributeTotalApparentPowerID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalApparentPowerID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000306,
    MTRClusterElectricalMeasurementAttributeMeasured1stHarmonicCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured1stHarmonicCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000307,
    MTRClusterElectricalMeasurementAttributeMeasured3rdHarmonicCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured3rdHarmonicCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000308,
    MTRClusterElectricalMeasurementAttributeMeasured5thHarmonicCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured5thHarmonicCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000309,
    MTRClusterElectricalMeasurementAttributeMeasured7thHarmonicCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured7thHarmonicCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000030A,
    MTRClusterElectricalMeasurementAttributeMeasured9thHarmonicCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured9thHarmonicCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000030B,
    MTRClusterElectricalMeasurementAttributeMeasured11thHarmonicCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured11thHarmonicCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000030C,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase1stHarmonicCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase1stHarmonicCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000030D,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase3rdHarmonicCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase3rdHarmonicCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000030E,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase5thHarmonicCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase5thHarmonicCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000030F,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase7thHarmonicCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase7thHarmonicCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000310,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase9thHarmonicCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase9thHarmonicCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000311,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase11thHarmonicCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase11thHarmonicCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000312,
    MTRClusterElectricalMeasurementAttributeAcFrequencyMultiplierID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMultiplierID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000400,
    MTRClusterElectricalMeasurementAttributeAcFrequencyDivisorID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyDivisorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000401,
    MTRClusterElectricalMeasurementAttributePowerMultiplierID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerMultiplierID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000402,
    MTRClusterElectricalMeasurementAttributePowerDivisorID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerDivisorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000403,
    MTRClusterElectricalMeasurementAttributeHarmonicCurrentMultiplierID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeHarmonicCurrentMultiplierID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000404,
    MTRClusterElectricalMeasurementAttributePhaseHarmonicCurrentMultiplierID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePhaseHarmonicCurrentMultiplierID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000405,
    MTRClusterElectricalMeasurementAttributeInstantaneousVoltageID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousVoltageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000500,
    MTRClusterElectricalMeasurementAttributeInstantaneousLineCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousLineCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000501,
    MTRClusterElectricalMeasurementAttributeInstantaneousActiveCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousActiveCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000502,
    MTRClusterElectricalMeasurementAttributeInstantaneousReactiveCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousReactiveCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000503,
    MTRClusterElectricalMeasurementAttributeInstantaneousPowerID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousPowerID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000504,
    MTRClusterElectricalMeasurementAttributeRmsVoltageID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000505,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMinID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000506,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMaxID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000507,
    MTRClusterElectricalMeasurementAttributeRmsCurrentID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000508,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMinID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000509,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMaxID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050A,
    MTRClusterElectricalMeasurementAttributeActivePowerID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050B,
    MTRClusterElectricalMeasurementAttributeActivePowerMinID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050C,
    MTRClusterElectricalMeasurementAttributeActivePowerMaxID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050D,
    MTRClusterElectricalMeasurementAttributeReactivePowerID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050E,
    MTRClusterElectricalMeasurementAttributeApparentPowerID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000050F,
    MTRClusterElectricalMeasurementAttributePowerFactorID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000510,
    MTRClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000511,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000513,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000514,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000515,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagPeriodID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000516,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000517,
    MTRClusterElectricalMeasurementAttributeAcVoltageMultiplierID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageMultiplierID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000600,
    MTRClusterElectricalMeasurementAttributeAcVoltageDivisorID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageDivisorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000601,
    MTRClusterElectricalMeasurementAttributeAcCurrentMultiplierID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentMultiplierID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000602,
    MTRClusterElectricalMeasurementAttributeAcCurrentDivisorID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentDivisorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000603,
    MTRClusterElectricalMeasurementAttributeAcPowerMultiplierID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerMultiplierID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000604,
    MTRClusterElectricalMeasurementAttributeAcPowerDivisorID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerDivisorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000605,
    MTRClusterElectricalMeasurementAttributeOverloadAlarmsMaskID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeOverloadAlarmsMaskID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000700,
    MTRClusterElectricalMeasurementAttributeVoltageOverloadID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeVoltageOverloadID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000701,
    MTRClusterElectricalMeasurementAttributeCurrentOverloadID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeCurrentOverloadID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000702,
    MTRClusterElectricalMeasurementAttributeAcOverloadAlarmsMaskID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcOverloadAlarmsMaskID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000800,
    MTRClusterElectricalMeasurementAttributeAcVoltageOverloadID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageOverloadID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000801,
    MTRClusterElectricalMeasurementAttributeAcCurrentOverloadID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentOverloadID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000802,
    MTRClusterElectricalMeasurementAttributeAcActivePowerOverloadID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcActivePowerOverloadID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000803,
    MTRClusterElectricalMeasurementAttributeAcReactivePowerOverloadID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcReactivePowerOverloadID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000804,
    MTRClusterElectricalMeasurementAttributeAverageRmsOverVoltageID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000805,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000806,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltageID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000807,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltageID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000808,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000809,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000080A,
    MTRClusterElectricalMeasurementAttributeLineCurrentPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000901,
    MTRClusterElectricalMeasurementAttributeActiveCurrentPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000902,
    MTRClusterElectricalMeasurementAttributeReactiveCurrentPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000903,
    MTRClusterElectricalMeasurementAttributeRmsVoltagePhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000905,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMinPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000906,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000907,
    MTRClusterElectricalMeasurementAttributeRmsCurrentPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000908,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMinPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000909,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000090A,
    MTRClusterElectricalMeasurementAttributeActivePowerPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000090B,
    MTRClusterElectricalMeasurementAttributeActivePowerMinPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000090C,
    MTRClusterElectricalMeasurementAttributeActivePowerMaxPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000090D,
    MTRClusterElectricalMeasurementAttributeReactivePowerPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000090E,
    MTRClusterElectricalMeasurementAttributeApparentPowerPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000090F,
    MTRClusterElectricalMeasurementAttributePowerFactorPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000910,
    MTRClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000911,
    MTRClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000912,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000913,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000914,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000915,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000916,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseBID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseBID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000917,
    MTRClusterElectricalMeasurementAttributeLineCurrentPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A01,
    MTRClusterElectricalMeasurementAttributeActiveCurrentPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A02,
    MTRClusterElectricalMeasurementAttributeReactiveCurrentPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A03,
    MTRClusterElectricalMeasurementAttributeRmsVoltagePhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A05,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMinPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A06,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A07,
    MTRClusterElectricalMeasurementAttributeRmsCurrentPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A08,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMinPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A09,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A0A,
    MTRClusterElectricalMeasurementAttributeActivePowerPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A0B,
    MTRClusterElectricalMeasurementAttributeActivePowerMinPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A0C,
    MTRClusterElectricalMeasurementAttributeActivePowerMaxPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A0D,
    MTRClusterElectricalMeasurementAttributeReactivePowerPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A0E,
    MTRClusterElectricalMeasurementAttributeApparentPowerPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A0F,
    MTRClusterElectricalMeasurementAttributePowerFactorPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A10,
    MTRClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A11,
    MTRClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A12,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A13,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A14,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A15,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A16,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseCID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000A17,
    MTRClusterElectricalMeasurementAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterElectricalMeasurementAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterElectricalMeasurementAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterElectricalMeasurementAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterElectricalMeasurementAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ElectricalMeasurement attributes
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasurementTypeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000100,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMinID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000101,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMaxID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000102,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000103,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMinID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000104,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMaxID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000105,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000106,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMinID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000107,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMaxID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000108,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMultiplierID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000200,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageDivisorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000201,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMultiplierID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000202,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentDivisorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000203,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMultiplierID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000204,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerDivisorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000205,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000300,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMinID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000301,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMaxID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000302,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeNeutralCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000303,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalActivePowerID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000304,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalReactivePowerID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000305,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalApparentPowerID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000306,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured1stHarmonicCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000307,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured3rdHarmonicCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000308,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured5thHarmonicCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000309,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured7thHarmonicCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000030A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured9thHarmonicCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000030B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured11thHarmonicCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000030C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase1stHarmonicCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000030D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase3rdHarmonicCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000030E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase5thHarmonicCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000030F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase7thHarmonicCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000310,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase9thHarmonicCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000311,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase11thHarmonicCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000312,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMultiplierID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000400,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyDivisorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000401,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerMultiplierID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000402,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerDivisorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000403,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeHarmonicCurrentMultiplierID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000404,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePhaseHarmonicCurrentMultiplierID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000405,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousVoltageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000500,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousLineCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000501,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousActiveCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000502,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousReactiveCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000503,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousPowerID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000504,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000505,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000506,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000507,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000508,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000509,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000050F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000510,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000511,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000513,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000514,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000515,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000516,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000517,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageMultiplierID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000600,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageDivisorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000601,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentMultiplierID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000602,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentDivisorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000603,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerMultiplierID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000604,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerDivisorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000605,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeOverloadAlarmsMaskID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000700,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeVoltageOverloadID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000701,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeCurrentOverloadID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000702,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcOverloadAlarmsMaskID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000800,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageOverloadID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000801,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentOverloadID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000802,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcActivePowerOverloadID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000803,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcReactivePowerOverloadID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000804,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000805,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000806,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000807,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000808,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000809,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000080A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000901,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000902,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000903,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000905,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000906,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000907,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000908,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000909,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000090A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000090B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000090C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000090D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000090E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000090F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000910,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000911,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000912,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000913,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000914,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000915,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000916,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseBID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000917,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A01,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A02,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A03,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A05,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A06,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A07,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A08,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A09,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A0A,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A0B,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A0C,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A0D,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A0E,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A0F,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A10,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A11,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A12,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A13,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A14,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A15,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A16,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000A17,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterElectricalMeasurementAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster TestCluster deprecated attribute names
    MTRClusterTestClusterAttributeBooleanID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeBooleanID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTestClusterAttributeBitmap8ID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap8ID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTestClusterAttributeBitmap16ID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap16ID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterTestClusterAttributeBitmap32ID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap32ID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterTestClusterAttributeBitmap64ID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap64ID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterTestClusterAttributeInt8uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt8uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterTestClusterAttributeInt16uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt16uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterTestClusterAttributeInt24uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt24uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterTestClusterAttributeInt32uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt32uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterTestClusterAttributeInt40uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt40uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterTestClusterAttributeInt48uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt48uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterTestClusterAttributeInt56uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt56uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterTestClusterAttributeInt64uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt64uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterTestClusterAttributeInt8sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt8sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterTestClusterAttributeInt16sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt16sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterTestClusterAttributeInt24sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt24sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterTestClusterAttributeInt32sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt32sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterTestClusterAttributeInt40sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt40sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterTestClusterAttributeInt48sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt48sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterTestClusterAttributeInt56sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt56sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterTestClusterAttributeInt64sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt64sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterTestClusterAttributeEnum8ID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEnum8ID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,
    MTRClusterTestClusterAttributeEnum16ID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEnum16ID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000016,
    MTRClusterTestClusterAttributeFloatSingleID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeFloatSingleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000017,
    MTRClusterTestClusterAttributeFloatDoubleID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeFloatDoubleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000018,
    MTRClusterTestClusterAttributeOctetStringID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeOctetStringID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000019,
    MTRClusterTestClusterAttributeListInt8uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeListInt8uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterTestClusterAttributeListOctetStringID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeListOctetStringID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001B,
    MTRClusterTestClusterAttributeListStructOctetStringID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeListStructOctetStringID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001C,
    MTRClusterTestClusterAttributeLongOctetStringID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeLongOctetStringID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001D,
    MTRClusterTestClusterAttributeCharStringID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeCharStringID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001E,
    MTRClusterTestClusterAttributeLongCharStringID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeLongCharStringID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001F,
    MTRClusterTestClusterAttributeEpochUsID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEpochUsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000020,
    MTRClusterTestClusterAttributeEpochSID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEpochSID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000021,
    MTRClusterTestClusterAttributeVendorIdID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeVendorIdID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000022,
    MTRClusterTestClusterAttributeListNullablesAndOptionalsStructID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeListNullablesAndOptionalsStructID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000023,
    MTRClusterTestClusterAttributeEnumAttrID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEnumAttrID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000024,
    MTRClusterTestClusterAttributeStructAttrID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeStructAttrID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000025,
    MTRClusterTestClusterAttributeRangeRestrictedInt8uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000026,
    MTRClusterTestClusterAttributeRangeRestrictedInt8sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000027,
    MTRClusterTestClusterAttributeRangeRestrictedInt16uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000028,
    MTRClusterTestClusterAttributeRangeRestrictedInt16sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000029,
    MTRClusterTestClusterAttributeListLongOctetStringID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeListLongOctetStringID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002A,
    MTRClusterTestClusterAttributeListFabricScopedID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeListFabricScopedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000002B,
    MTRClusterTestClusterAttributeTimedWriteBooleanID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeTimedWriteBooleanID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000030,
    MTRClusterTestClusterAttributeGeneralErrorBooleanID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeGeneralErrorBooleanID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000031,
    MTRClusterTestClusterAttributeClusterErrorBooleanID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeClusterErrorBooleanID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000032,
    MTRClusterTestClusterAttributeUnsupportedID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeUnsupportedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x000000FF,
    MTRClusterTestClusterAttributeNullableBooleanID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBooleanID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004000,
    MTRClusterTestClusterAttributeNullableBitmap8ID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap8ID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004001,
    MTRClusterTestClusterAttributeNullableBitmap16ID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap16ID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004002,
    MTRClusterTestClusterAttributeNullableBitmap32ID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap32ID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004003,
    MTRClusterTestClusterAttributeNullableBitmap64ID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap64ID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004004,
    MTRClusterTestClusterAttributeNullableInt8uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004005,
    MTRClusterTestClusterAttributeNullableInt16uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004006,
    MTRClusterTestClusterAttributeNullableInt24uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004007,
    MTRClusterTestClusterAttributeNullableInt32uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004008,
    MTRClusterTestClusterAttributeNullableInt40uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004009,
    MTRClusterTestClusterAttributeNullableInt48uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400A,
    MTRClusterTestClusterAttributeNullableInt56uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400B,
    MTRClusterTestClusterAttributeNullableInt64uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400C,
    MTRClusterTestClusterAttributeNullableInt8sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400D,
    MTRClusterTestClusterAttributeNullableInt16sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400E,
    MTRClusterTestClusterAttributeNullableInt24sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000400F,
    MTRClusterTestClusterAttributeNullableInt32sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004010,
    MTRClusterTestClusterAttributeNullableInt40sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004011,
    MTRClusterTestClusterAttributeNullableInt48sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004012,
    MTRClusterTestClusterAttributeNullableInt56sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004013,
    MTRClusterTestClusterAttributeNullableInt64sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004014,
    MTRClusterTestClusterAttributeNullableEnum8ID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum8ID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004015,
    MTRClusterTestClusterAttributeNullableEnum16ID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum16ID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004016,
    MTRClusterTestClusterAttributeNullableFloatSingleID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatSingleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004017,
    MTRClusterTestClusterAttributeNullableFloatDoubleID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatDoubleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004018,
    MTRClusterTestClusterAttributeNullableOctetStringID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableOctetStringID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004019,
    MTRClusterTestClusterAttributeNullableCharStringID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableCharStringID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000401E,
    MTRClusterTestClusterAttributeNullableEnumAttrID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnumAttrID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004024,
    MTRClusterTestClusterAttributeNullableStructID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableStructID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004025,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt8uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004026,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt8sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004027,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt16uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16uID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004028,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt16sID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16sID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00004029,
    MTRClusterTestClusterAttributeWriteOnlyInt8uID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeWriteOnlyInt8uID", ios(16.2, 16.4), macos(13.1, 13.3), watchos(9.2, 9.4), tvos(16.2, 16.4))
    = 0x0000402A,
    MTRClusterTestClusterAttributeGeneratedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeGeneratedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTestClusterAttributeAcceptedCommandListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeAcceptedCommandListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTestClusterAttributeAttributeListID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeAttributeListID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTestClusterAttributeFeatureMapID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeFeatureMapID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTestClusterAttributeClusterRevisionID
        MTR_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeClusterRevisionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster UnitTesting attributes
    MTRAttributeIDTypeClusterUnitTestingAttributeBooleanID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRAttributeIDTypeClusterUnitTestingAttributeBitmap8ID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRAttributeIDTypeClusterUnitTestingAttributeBitmap16ID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRAttributeIDTypeClusterUnitTestingAttributeBitmap32ID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRAttributeIDTypeClusterUnitTestingAttributeBitmap64ID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt8uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt16uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt24uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt32uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt40uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt48uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt56uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt64uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000C,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt8sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000D,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt16sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000E,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt24sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000F,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt32sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt40sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt48sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt56sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000013,
    MTRAttributeIDTypeClusterUnitTestingAttributeInt64sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000014,
    MTRAttributeIDTypeClusterUnitTestingAttributeEnum8ID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000015,
    MTRAttributeIDTypeClusterUnitTestingAttributeEnum16ID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000016,
    MTRAttributeIDTypeClusterUnitTestingAttributeFloatSingleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000017,
    MTRAttributeIDTypeClusterUnitTestingAttributeFloatDoubleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000018,
    MTRAttributeIDTypeClusterUnitTestingAttributeOctetStringID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000019,
    MTRAttributeIDTypeClusterUnitTestingAttributeListInt8uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001A,
    MTRAttributeIDTypeClusterUnitTestingAttributeListOctetStringID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001B,
    MTRAttributeIDTypeClusterUnitTestingAttributeListStructOctetStringID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001C,
    MTRAttributeIDTypeClusterUnitTestingAttributeLongOctetStringID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001D,
    MTRAttributeIDTypeClusterUnitTestingAttributeCharStringID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001E,
    MTRAttributeIDTypeClusterUnitTestingAttributeLongCharStringID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001F,
    MTRAttributeIDTypeClusterUnitTestingAttributeEpochUsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000020,
    MTRAttributeIDTypeClusterUnitTestingAttributeEpochSID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000021,
    MTRAttributeIDTypeClusterUnitTestingAttributeVendorIdID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000022,
    MTRAttributeIDTypeClusterUnitTestingAttributeListNullablesAndOptionalsStructID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000023,
    MTRAttributeIDTypeClusterUnitTestingAttributeEnumAttrID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000024,
    MTRAttributeIDTypeClusterUnitTestingAttributeStructAttrID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000025,
    MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000026,
    MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000027,
    MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000028,
    MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000029,
    MTRAttributeIDTypeClusterUnitTestingAttributeListLongOctetStringID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002A,
    MTRAttributeIDTypeClusterUnitTestingAttributeListFabricScopedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000002B,
    MTRAttributeIDTypeClusterUnitTestingAttributeTimedWriteBooleanID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000030,
    MTRAttributeIDTypeClusterUnitTestingAttributeGeneralErrorBooleanID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000031,
    MTRAttributeIDTypeClusterUnitTestingAttributeClusterErrorBooleanID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000032,
    MTRAttributeIDTypeClusterUnitTestingAttributeUnsupportedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x000000FF,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableBooleanID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004000,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap8ID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004001,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap16ID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004002,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap32ID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004003,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap64ID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004004,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004005,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004006,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004007,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004008,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004009,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000400A,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000400B,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000400C,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000400D,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000400E,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000400F,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004010,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004011,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004012,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004013,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004014,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum8ID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004015,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum16ID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004016,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatSingleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004017,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatDoubleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004018,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableOctetStringID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004019,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableCharStringID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000401E,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnumAttrID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004024,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableStructID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004025,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004026,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004027,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004028,
    MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16sID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00004029,
    MTRAttributeIDTypeClusterUnitTestingAttributeWriteOnlyInt8uID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000402A,
    MTRAttributeIDTypeClusterUnitTestingAttributeGeneratedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterUnitTestingAttributeAcceptedCommandListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterUnitTestingAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterUnitTestingAttributeAttributeListID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterUnitTestingAttributeFeatureMapID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterUnitTestingAttributeClusterRevisionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster SampleMEI attributes
    MTRAttributeIDTypeClusterSampleMEIAttributeFlipFlopID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterSampleMEIAttributeGeneratedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterSampleMEIAttributeAcceptedCommandListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterSampleMEIAttributeEventListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeEventListID,
    MTRAttributeIDTypeClusterSampleMEIAttributeAttributeListID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterSampleMEIAttributeFeatureMapID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterSampleMEIAttributeClusterRevisionID MTR_PROVISIONALLY_AVAILABLE = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

};

#pragma mark - Commands IDs

typedef NS_ENUM(uint32_t, MTRCommandIDType) {

    // Cluster Identify deprecated command id names
    MTRClusterIdentifyCommandIdentifyID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterIdentifyCommandIdentifyID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterIdentifyCommandTriggerEffectID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterIdentifyCommandTriggerEffectID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000040,

    // Cluster Identify commands
    MTRCommandIDTypeClusterIdentifyCommandIdentifyID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterIdentifyCommandTriggerEffectID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000040,

    // Cluster Groups deprecated command id names
    MTRClusterGroupsCommandAddGroupID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandAddGroupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGroupsCommandAddGroupResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandAddGroupResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGroupsCommandViewGroupID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandViewGroupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGroupsCommandViewGroupResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandViewGroupResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGroupsCommandGetGroupMembershipID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGroupsCommandGetGroupMembershipResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGroupsCommandRemoveGroupID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandRemoveGroupID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterGroupsCommandRemoveGroupResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandRemoveGroupResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterGroupsCommandRemoveAllGroupsID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandRemoveAllGroupsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterGroupsCommandAddGroupIfIdentifyingID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandAddGroupIfIdentifyingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,

    // Cluster Groups commands
    MTRCommandIDTypeClusterGroupsCommandAddGroupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterGroupsCommandAddGroupResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterGroupsCommandViewGroupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterGroupsCommandViewGroupResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterGroupsCommandRemoveGroupID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterGroupsCommandRemoveGroupResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterGroupsCommandRemoveAllGroupsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterGroupsCommandAddGroupIfIdentifyingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,

    // Cluster Scenes deprecated command id names
    MTRClusterScenesCommandAddSceneID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandAddSceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterScenesCommandAddSceneResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandAddSceneResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterScenesCommandViewSceneID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandViewSceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterScenesCommandViewSceneResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandViewSceneResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterScenesCommandRemoveSceneID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandRemoveSceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterScenesCommandRemoveSceneResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandRemoveSceneResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterScenesCommandRemoveAllScenesID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandRemoveAllScenesID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterScenesCommandRemoveAllScenesResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandRemoveAllScenesResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterScenesCommandStoreSceneID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandStoreSceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterScenesCommandStoreSceneResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandStoreSceneResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterScenesCommandRecallSceneID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandRecallSceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterScenesCommandGetSceneMembershipID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandGetSceneMembershipID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterScenesCommandGetSceneMembershipResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandGetSceneMembershipResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterScenesCommandEnhancedAddSceneID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000040,
    MTRClusterScenesCommandEnhancedAddSceneResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000040,
    MTRClusterScenesCommandEnhancedViewSceneID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000041,
    MTRClusterScenesCommandEnhancedViewSceneResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000041,
    MTRClusterScenesCommandCopySceneID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandCopySceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000042,
    MTRClusterScenesCommandCopySceneResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandCopySceneResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000042,

    // Cluster Scenes commands
    MTRCommandIDTypeClusterScenesCommandAddSceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterScenesCommandAddSceneResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterScenesCommandViewSceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterScenesCommandViewSceneResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterScenesCommandRemoveSceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterScenesCommandRemoveSceneResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterScenesCommandRemoveAllScenesID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterScenesCommandRemoveAllScenesResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterScenesCommandStoreSceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterScenesCommandStoreSceneResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterScenesCommandRecallSceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRCommandIDTypeClusterScenesCommandGetSceneMembershipID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRCommandIDTypeClusterScenesCommandGetSceneMembershipResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000040,
    MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000040,
    MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000041,
    MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000041,
    MTRCommandIDTypeClusterScenesCommandCopySceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000042,
    MTRCommandIDTypeClusterScenesCommandCopySceneResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000042,

    // Cluster OnOff deprecated command id names
    MTRClusterOnOffCommandOffID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOffID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOnOffCommandOnID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOnID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterOnOffCommandToggleID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandToggleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterOnOffCommandOffWithEffectID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOffWithEffectID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000040,
    MTRClusterOnOffCommandOnWithRecallGlobalSceneID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOnWithRecallGlobalSceneID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000041,
    MTRClusterOnOffCommandOnWithTimedOffID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOnWithTimedOffID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000042,

    // Cluster OnOff commands
    MTRCommandIDTypeClusterOnOffCommandOffID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterOnOffCommandOnID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterOnOffCommandToggleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterOnOffCommandOffWithEffectID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000040,
    MTRCommandIDTypeClusterOnOffCommandOnWithRecallGlobalSceneID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000041,
    MTRCommandIDTypeClusterOnOffCommandOnWithTimedOffID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000042,

    // Cluster LevelControl deprecated command id names
    MTRClusterLevelControlCommandMoveToLevelID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandMoveToLevelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterLevelControlCommandMoveID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandMoveID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterLevelControlCommandStepID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandStepID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterLevelControlCommandStopID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandStopID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterLevelControlCommandMoveToLevelWithOnOffID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandMoveToLevelWithOnOffID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterLevelControlCommandMoveWithOnOffID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandMoveWithOnOffID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterLevelControlCommandStepWithOnOffID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandStepWithOnOffID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterLevelControlCommandStopWithOnOffID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandStopWithOnOffID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterLevelControlCommandMoveToClosestFrequencyID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandMoveToClosestFrequencyID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,

    // Cluster LevelControl commands
    MTRCommandIDTypeClusterLevelControlCommandMoveToLevelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterLevelControlCommandMoveID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterLevelControlCommandStepID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterLevelControlCommandStopID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterLevelControlCommandMoveToLevelWithOnOffID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterLevelControlCommandMoveWithOnOffID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRCommandIDTypeClusterLevelControlCommandStepWithOnOffID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRCommandIDTypeClusterLevelControlCommandStopWithOnOffID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRCommandIDTypeClusterLevelControlCommandMoveToClosestFrequencyID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,

    // Cluster Actions deprecated command id names
    MTRClusterActionsCommandInstantActionID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandInstantActionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterActionsCommandInstantActionWithTransitionID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandInstantActionWithTransitionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterActionsCommandStartActionID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandStartActionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterActionsCommandStartActionWithDurationID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandStartActionWithDurationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterActionsCommandStopActionID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandStopActionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterActionsCommandPauseActionID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandPauseActionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterActionsCommandPauseActionWithDurationID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandPauseActionWithDurationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterActionsCommandResumeActionID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandResumeActionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterActionsCommandEnableActionID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandEnableActionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterActionsCommandEnableActionWithDurationID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandEnableActionWithDurationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterActionsCommandDisableActionID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandDisableActionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterActionsCommandDisableActionWithDurationID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandDisableActionWithDurationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,

    // Cluster Actions commands
    MTRCommandIDTypeClusterActionsCommandInstantActionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterActionsCommandInstantActionWithTransitionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterActionsCommandStartActionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterActionsCommandStartActionWithDurationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterActionsCommandStopActionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterActionsCommandPauseActionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRCommandIDTypeClusterActionsCommandPauseActionWithDurationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRCommandIDTypeClusterActionsCommandResumeActionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRCommandIDTypeClusterActionsCommandEnableActionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRCommandIDTypeClusterActionsCommandEnableActionWithDurationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRCommandIDTypeClusterActionsCommandDisableActionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRCommandIDTypeClusterActionsCommandDisableActionWithDurationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,

    // Cluster Basic deprecated command id names
    MTRClusterBasicCommandMfgSpecificPingID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterBasicInformationCommandMfgSpecificPingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x10020000,

    // Cluster BasicInformation commands

    // Cluster OtaSoftwareUpdateProvider deprecated command id names
    MTRClusterOtaSoftwareUpdateProviderCommandQueryImageID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOtaSoftwareUpdateProviderCommandQueryImageResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterOtaSoftwareUpdateProviderCommandApplyUpdateRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterOtaSoftwareUpdateProviderCommandApplyUpdateResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterOtaSoftwareUpdateProviderCommandNotifyUpdateAppliedID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandNotifyUpdateAppliedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,

    // Cluster OTASoftwareUpdateProvider commands
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandNotifyUpdateAppliedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,

    // Cluster OtaSoftwareUpdateRequestor deprecated command id names
    MTRClusterOtaSoftwareUpdateRequestorCommandAnnounceOtaProviderID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOTASoftwareUpdateRequestorCommandAnnounceOTAProviderID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster OTASoftwareUpdateRequestor commands
    MTRCommandIDTypeClusterOTASoftwareUpdateRequestorCommandAnnounceOTAProviderID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,

    // Cluster GeneralCommissioning deprecated command id names
    MTRClusterGeneralCommissioningCommandArmFailSafeID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGeneralCommissioningCommandArmFailSafeResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGeneralCommissioningCommandSetRegulatoryConfigID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGeneralCommissioningCommandSetRegulatoryConfigResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterGeneralCommissioningCommandCommissioningCompleteID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterGeneralCommissioningCommandCommissioningCompleteResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,

    // Cluster GeneralCommissioning commands
    MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,

    // Cluster NetworkCommissioning deprecated command id names
    MTRClusterNetworkCommissioningCommandScanNetworksID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterNetworkCommissioningCommandScanNetworksResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterNetworkCommissioningCommandAddOrUpdateWiFiNetworkID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateWiFiNetworkID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterNetworkCommissioningCommandAddOrUpdateThreadNetworkID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateThreadNetworkID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterNetworkCommissioningCommandRemoveNetworkID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandRemoveNetworkID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterNetworkCommissioningCommandNetworkConfigResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandNetworkConfigResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterNetworkCommissioningCommandConnectNetworkID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterNetworkCommissioningCommandConnectNetworkResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterNetworkCommissioningCommandReorderNetworkID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandReorderNetworkID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,

    // Cluster NetworkCommissioning commands
    MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateWiFiNetworkID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateThreadNetworkID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterNetworkCommissioningCommandRemoveNetworkID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterNetworkCommissioningCommandNetworkConfigResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRCommandIDTypeClusterNetworkCommissioningCommandReorderNetworkID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRCommandIDTypeClusterNetworkCommissioningCommandQueryIdentityID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRCommandIDTypeClusterNetworkCommissioningCommandQueryIdentityResponseID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,

    // Cluster DiagnosticLogs deprecated command id names
    MTRClusterDiagnosticLogsCommandRetrieveLogsRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterDiagnosticLogsCommandRetrieveLogsResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster DiagnosticLogs commands
    MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,

    // Cluster GeneralDiagnostics deprecated command id names
    MTRClusterGeneralDiagnosticsCommandTestEventTriggerID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGeneralDiagnosticsCommandTestEventTriggerID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster GeneralDiagnostics commands
    MTRCommandIDTypeClusterGeneralDiagnosticsCommandTestEventTriggerID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterGeneralDiagnosticsCommandTimeSnapshotID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterGeneralDiagnosticsCommandTimeSnapshotResponseID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,

    // Cluster SoftwareDiagnostics deprecated command id names
    MTRClusterSoftwareDiagnosticsCommandResetWatermarksID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterSoftwareDiagnosticsCommandResetWatermarksID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster SoftwareDiagnostics commands
    MTRCommandIDTypeClusterSoftwareDiagnosticsCommandResetWatermarksID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,

    // Cluster ThreadNetworkDiagnostics deprecated command id names
    MTRClusterThreadNetworkDiagnosticsCommandResetCountsID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterThreadNetworkDiagnosticsCommandResetCountsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster ThreadNetworkDiagnostics commands
    MTRCommandIDTypeClusterThreadNetworkDiagnosticsCommandResetCountsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,

    // Cluster WiFiNetworkDiagnostics deprecated command id names
    MTRClusterWiFiNetworkDiagnosticsCommandResetCountsID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterWiFiNetworkDiagnosticsCommandResetCountsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster WiFiNetworkDiagnostics commands
    MTRCommandIDTypeClusterWiFiNetworkDiagnosticsCommandResetCountsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,

    // Cluster EthernetNetworkDiagnostics deprecated command id names
    MTRClusterEthernetNetworkDiagnosticsCommandResetCountsID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterEthernetNetworkDiagnosticsCommandResetCountsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster EthernetNetworkDiagnostics commands
    MTRCommandIDTypeClusterEthernetNetworkDiagnosticsCommandResetCountsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,

    // Cluster TimeSynchronization deprecated command id names
    MTRClusterTimeSynchronizationCommandSetUtcTimeID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterTimeSynchronizationCommandSetUTCTimeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster TimeSynchronization commands
    MTRCommandIDTypeClusterTimeSynchronizationCommandSetUTCTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterTimeSynchronizationCommandSetUtcTimeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterTimeSynchronizationCommandSetTrustedTimeSourceID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterTimeSynchronizationCommandSetTimeZoneID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterTimeSynchronizationCommandSetTimeZoneResponseID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterTimeSynchronizationCommandSetDSTOffsetID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterTimeSynchronizationCommandSetDefaultNTPID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,

    // Cluster AdministratorCommissioning deprecated command id names
    MTRClusterAdministratorCommissioningCommandOpenCommissioningWindowID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenCommissioningWindowID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterAdministratorCommissioningCommandOpenBasicCommissioningWindowID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenBasicCommissioningWindowID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterAdministratorCommissioningCommandRevokeCommissioningID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterAdministratorCommissioningCommandRevokeCommissioningID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,

    // Cluster AdministratorCommissioning commands
    MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenCommissioningWindowID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenBasicCommissioningWindowID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterAdministratorCommissioningCommandRevokeCommissioningID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,

    // Cluster OperationalCredentials deprecated command id names
    MTRClusterOperationalCredentialsCommandAttestationRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOperationalCredentialsCommandAttestationResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterOperationalCredentialsCommandCertificateChainRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterOperationalCredentialsCommandCertificateChainResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterOperationalCredentialsCommandCSRRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCSRRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterOperationalCredentialsCommandCSRResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCSRResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterOperationalCredentialsCommandAddNOCID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAddNOCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterOperationalCredentialsCommandUpdateNOCID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateNOCID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterOperationalCredentialsCommandNOCResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandNOCResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterOperationalCredentialsCommandUpdateFabricLabelID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateFabricLabelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterOperationalCredentialsCommandRemoveFabricID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandRemoveFabricID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterOperationalCredentialsCommandAddTrustedRootCertificateID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAddTrustedRootCertificateID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,

    // Cluster OperationalCredentials commands
    MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCSRRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterOperationalCredentialsCommandCSRResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRCommandIDTypeClusterOperationalCredentialsCommandAddNOCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateNOCID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRCommandIDTypeClusterOperationalCredentialsCommandNOCResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateFabricLabelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRCommandIDTypeClusterOperationalCredentialsCommandRemoveFabricID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRCommandIDTypeClusterOperationalCredentialsCommandAddTrustedRootCertificateID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,

    // Cluster GroupKeyManagement deprecated command id names
    MTRClusterGroupKeyManagementCommandKeySetWriteID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetWriteID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGroupKeyManagementCommandKeySetReadID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGroupKeyManagementCommandKeySetReadResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGroupKeyManagementCommandKeySetRemoveID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetRemoveID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterGroupKeyManagementCommandKeySetReadAllIndicesID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterGroupKeyManagementCommandKeySetReadAllIndicesResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,

    // Cluster GroupKeyManagement commands
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetWriteID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetRemoveID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,

    // Cluster ICDManagement commands
    MTRCommandIDTypeClusterICDManagementCommandRegisterClientID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterICDManagementCommandRegisterClientResponseID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterICDManagementCommandUnregisterClientID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterICDManagementCommandStayActiveRequestID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterICDManagementCommandStayActiveResponseID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,

    // Cluster Timer commands
    MTRCommandIDTypeClusterTimerCommandSetTimerID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterTimerCommandResetTimerID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterTimerCommandAddTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterTimerCommandReduceTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,

    // Cluster OvenCavityOperationalState commands
    MTRCommandIDTypeClusterOvenCavityOperationalStateCommandPauseID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterOvenCavityOperationalStateCommandStopID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterOvenCavityOperationalStateCommandStartID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterOvenCavityOperationalStateCommandResumeID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterOvenCavityOperationalStateCommandOperationalCommandResponseID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,

    // Cluster OvenMode commands
    MTRCommandIDTypeClusterOvenModeCommandChangeToModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterOvenModeCommandChangeToModeResponseID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,

    // Cluster ModeSelect deprecated command id names
    MTRClusterModeSelectCommandChangeToModeID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterModeSelectCommandChangeToModeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster ModeSelect commands
    MTRCommandIDTypeClusterModeSelectCommandChangeToModeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,

    // Cluster LaundryWasherMode commands
    MTRCommandIDTypeClusterLaundryWasherModeCommandChangeToModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterLaundryWasherModeCommandChangeToModeResponseID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,

    // Cluster RefrigeratorAndTemperatureControlledCabinetMode commands
    MTRCommandIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeCommandChangeToModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterRefrigeratorAndTemperatureControlledCabinetModeCommandChangeToModeResponseID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,

    // Cluster RVCRunMode commands
    MTRCommandIDTypeClusterRVCRunModeCommandChangeToModeID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000000,
    MTRCommandIDTypeClusterRVCRunModeCommandChangeToModeResponseID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000001,

    // Cluster RVCCleanMode commands
    MTRCommandIDTypeClusterRVCCleanModeCommandChangeToModeID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000000,
    MTRCommandIDTypeClusterRVCCleanModeCommandChangeToModeResponseID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000001,

    // Cluster TemperatureControl commands
    MTRCommandIDTypeClusterTemperatureControlCommandSetTemperatureID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,

    // Cluster DishwasherMode commands
    MTRCommandIDTypeClusterDishwasherModeCommandChangeToModeID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterDishwasherModeCommandChangeToModeResponseID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,

    // Cluster SmokeCOAlarm commands
    MTRCommandIDTypeClusterSmokeCOAlarmCommandSelfTestRequestID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,

    // Cluster DishwasherAlarm commands
    MTRCommandIDTypeClusterDishwasherAlarmCommandResetID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterDishwasherAlarmCommandModifyEnabledAlarmsID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,

    // Cluster MicrowaveOvenControl commands
    MTRCommandIDTypeClusterMicrowaveOvenControlCommandSetCookingParametersID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterMicrowaveOvenControlCommandAddMoreTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,

    // Cluster OperationalState commands
    MTRCommandIDTypeClusterOperationalStateCommandPauseID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000000,
    MTRCommandIDTypeClusterOperationalStateCommandStopID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000001,
    MTRCommandIDTypeClusterOperationalStateCommandStartID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000002,
    MTRCommandIDTypeClusterOperationalStateCommandResumeID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000003,
    MTRCommandIDTypeClusterOperationalStateCommandOperationalCommandResponseID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000004,

    // Cluster RVCOperationalState commands
    MTRCommandIDTypeClusterRVCOperationalStateCommandPauseID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000000,
    MTRCommandIDTypeClusterRVCOperationalStateCommandStopID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterRVCOperationalStateCommandStartID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterRVCOperationalStateCommandResumeID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000003,
    MTRCommandIDTypeClusterRVCOperationalStateCommandOperationalCommandResponseID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000004,

    // Cluster HEPAFilterMonitoring commands
    MTRCommandIDTypeClusterHEPAFilterMonitoringCommandResetConditionID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,

    // Cluster ActivatedCarbonFilterMonitoring commands
    MTRCommandIDTypeClusterActivatedCarbonFilterMonitoringCommandResetConditionID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,

    // Cluster BooleanStateConfiguration commands
    MTRCommandIDTypeClusterBooleanStateConfigurationCommandSuppressAlarmID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterBooleanStateConfigurationCommandEnableDisableAlarmID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,

    // Cluster ValveConfigurationAndControl commands
    MTRCommandIDTypeClusterValveConfigurationAndControlCommandOpenID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterValveConfigurationAndControlCommandCloseID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,

    // Cluster DemandResponseLoadControl commands
    MTRCommandIDTypeClusterDemandResponseLoadControlCommandRegisterLoadControlProgramRequestID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterDemandResponseLoadControlCommandUnregisterLoadControlProgramRequestID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterDemandResponseLoadControlCommandAddLoadControlEventRequestID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterDemandResponseLoadControlCommandRemoveLoadControlEventRequestID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterDemandResponseLoadControlCommandClearLoadControlEventsRequestID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,

    // Cluster DeviceEnergyManagement commands
    MTRCommandIDTypeClusterDeviceEnergyManagementCommandPowerAdjustRequestID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterDeviceEnergyManagementCommandCancelPowerAdjustRequestID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterDeviceEnergyManagementCommandStartTimeAdjustRequestID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterDeviceEnergyManagementCommandPauseRequestID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterDeviceEnergyManagementCommandResumeRequestID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterDeviceEnergyManagementCommandModifyForecastRequestID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRCommandIDTypeClusterDeviceEnergyManagementCommandRequestConstraintBasedForecastID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,

    // Cluster EnergyEVSE commands
    MTRCommandIDTypeClusterEnergyEVSECommandGetTargetsResponseID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterEnergyEVSECommandDisableID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterEnergyEVSECommandEnableChargingID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterEnergyEVSECommandEnableDischargingID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterEnergyEVSECommandStartDiagnosticsID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterEnergyEVSECommandSetTargetsID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRCommandIDTypeClusterEnergyEVSECommandGetTargetsID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRCommandIDTypeClusterEnergyEVSECommandClearTargetsID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,

    // Cluster DoorLock deprecated command id names
    MTRClusterDoorLockCommandLockDoorID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandLockDoorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterDoorLockCommandUnlockDoorID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandUnlockDoorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterDoorLockCommandUnlockWithTimeoutID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandUnlockWithTimeoutID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterDoorLockCommandSetWeekDayScheduleID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandSetWeekDayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterDoorLockCommandGetWeekDayScheduleID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterDoorLockCommandGetWeekDayScheduleResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterDoorLockCommandClearWeekDayScheduleID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandClearWeekDayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterDoorLockCommandSetYearDayScheduleID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandSetYearDayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterDoorLockCommandGetYearDayScheduleID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterDoorLockCommandGetYearDayScheduleResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterDoorLockCommandClearYearDayScheduleID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandClearYearDayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterDoorLockCommandSetHolidayScheduleID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandSetHolidayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterDoorLockCommandGetHolidayScheduleID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterDoorLockCommandGetHolidayScheduleResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterDoorLockCommandClearHolidayScheduleID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandClearHolidayScheduleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterDoorLockCommandSetUserID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandSetUserID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001A,
    MTRClusterDoorLockCommandGetUserID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetUserID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001B,
    MTRClusterDoorLockCommandGetUserResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetUserResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001C,
    MTRClusterDoorLockCommandClearUserID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandClearUserID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000001D,
    MTRClusterDoorLockCommandSetCredentialID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandSetCredentialID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000022,
    MTRClusterDoorLockCommandSetCredentialResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandSetCredentialResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000023,
    MTRClusterDoorLockCommandGetCredentialStatusID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000024,
    MTRClusterDoorLockCommandGetCredentialStatusResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000025,
    MTRClusterDoorLockCommandClearCredentialID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandClearCredentialID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000026,

    // Cluster DoorLock commands
    MTRCommandIDTypeClusterDoorLockCommandLockDoorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterDoorLockCommandUnlockDoorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterDoorLockCommandUnlockWithTimeoutID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterDoorLockCommandSetWeekDayScheduleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,
    MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000C,
    MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000C,
    MTRCommandIDTypeClusterDoorLockCommandClearWeekDayScheduleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000D,
    MTRCommandIDTypeClusterDoorLockCommandSetYearDayScheduleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000E,
    MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000F,
    MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000F,
    MTRCommandIDTypeClusterDoorLockCommandClearYearDayScheduleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,
    MTRCommandIDTypeClusterDoorLockCommandSetHolidayScheduleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRCommandIDTypeClusterDoorLockCommandClearHolidayScheduleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000013,
    MTRCommandIDTypeClusterDoorLockCommandSetUserID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001A,
    MTRCommandIDTypeClusterDoorLockCommandGetUserID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001B,
    MTRCommandIDTypeClusterDoorLockCommandGetUserResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001C,
    MTRCommandIDTypeClusterDoorLockCommandClearUserID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000001D,
    MTRCommandIDTypeClusterDoorLockCommandSetCredentialID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000022,
    MTRCommandIDTypeClusterDoorLockCommandSetCredentialResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000023,
    MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000024,
    MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000025,
    MTRCommandIDTypeClusterDoorLockCommandClearCredentialID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000026,
    MTRCommandIDTypeClusterDoorLockCommandUnboltDoorID MTR_PROVISIONALLY_AVAILABLE = 0x00000027,

    // Cluster WindowCovering deprecated command id names
    MTRClusterWindowCoveringCommandUpOrOpenID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterWindowCoveringCommandUpOrOpenID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterWindowCoveringCommandDownOrCloseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterWindowCoveringCommandDownOrCloseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterWindowCoveringCommandStopMotionID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterWindowCoveringCommandStopMotionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterWindowCoveringCommandGoToLiftValueID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterWindowCoveringCommandGoToLiftPercentageID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftPercentageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterWindowCoveringCommandGoToTiltValueID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltValueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterWindowCoveringCommandGoToTiltPercentageID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltPercentageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,

    // Cluster WindowCovering commands
    MTRCommandIDTypeClusterWindowCoveringCommandUpOrOpenID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterWindowCoveringCommandDownOrCloseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterWindowCoveringCommandStopMotionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftPercentageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltValueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltPercentageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,

    // Cluster BarrierControl deprecated command id names
    MTRClusterBarrierControlCommandBarrierControlGoToPercentID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterBarrierControlCommandBarrierControlGoToPercentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterBarrierControlCommandBarrierControlStopID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterBarrierControlCommandBarrierControlStopID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster BarrierControl commands
    MTRCommandIDTypeClusterBarrierControlCommandBarrierControlGoToPercentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterBarrierControlCommandBarrierControlStopID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,

    // Cluster Thermostat deprecated command id names
    MTRClusterThermostatCommandSetpointRaiseLowerID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterThermostatCommandSetpointRaiseLowerID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterThermostatCommandGetWeeklyScheduleResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterThermostatCommandSetWeeklyScheduleID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterThermostatCommandSetWeeklyScheduleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterThermostatCommandGetWeeklyScheduleID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterThermostatCommandClearWeeklyScheduleID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterThermostatCommandClearWeeklyScheduleID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster Thermostat commands
    MTRCommandIDTypeClusterThermostatCommandSetpointRaiseLowerID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterThermostatCommandSetWeeklyScheduleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterThermostatCommandClearWeeklyScheduleID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,

    // Cluster FanControl deprecated command id names

    // Cluster FanControl commands
    MTRCommandIDTypeClusterFanControlCommandStepID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,

    // Cluster ColorControl deprecated command id names
    MTRClusterColorControlCommandMoveToHueID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveToHueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterColorControlCommandMoveHueID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveHueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterColorControlCommandStepHueID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandStepHueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterColorControlCommandMoveToSaturationID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveToSaturationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterColorControlCommandMoveSaturationID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveSaturationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterColorControlCommandStepSaturationID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandStepSaturationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterColorControlCommandMoveToHueAndSaturationID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveToHueAndSaturationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterColorControlCommandMoveToColorID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveToColorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterColorControlCommandMoveColorID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveColorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterColorControlCommandStepColorID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandStepColorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterColorControlCommandMoveToColorTemperatureID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveToColorTemperatureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterColorControlCommandEnhancedMoveToHueID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000040,
    MTRClusterColorControlCommandEnhancedMoveHueID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandEnhancedMoveHueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000041,
    MTRClusterColorControlCommandEnhancedStepHueID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandEnhancedStepHueID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000042,
    MTRClusterColorControlCommandEnhancedMoveToHueAndSaturationID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueAndSaturationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000043,
    MTRClusterColorControlCommandColorLoopSetID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandColorLoopSetID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000044,
    MTRClusterColorControlCommandStopMoveStepID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandStopMoveStepID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000047,
    MTRClusterColorControlCommandMoveColorTemperatureID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveColorTemperatureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000004B,
    MTRClusterColorControlCommandStepColorTemperatureID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandStepColorTemperatureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000004C,

    // Cluster ColorControl commands
    MTRCommandIDTypeClusterColorControlCommandMoveToHueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterColorControlCommandMoveHueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterColorControlCommandStepHueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterColorControlCommandMoveToSaturationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterColorControlCommandMoveSaturationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterColorControlCommandStepSaturationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRCommandIDTypeClusterColorControlCommandMoveToHueAndSaturationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRCommandIDTypeClusterColorControlCommandMoveToColorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRCommandIDTypeClusterColorControlCommandMoveColorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRCommandIDTypeClusterColorControlCommandStepColorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRCommandIDTypeClusterColorControlCommandMoveToColorTemperatureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000040,
    MTRCommandIDTypeClusterColorControlCommandEnhancedMoveHueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000041,
    MTRCommandIDTypeClusterColorControlCommandEnhancedStepHueID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000042,
    MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueAndSaturationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000043,
    MTRCommandIDTypeClusterColorControlCommandColorLoopSetID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000044,
    MTRCommandIDTypeClusterColorControlCommandStopMoveStepID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000047,
    MTRCommandIDTypeClusterColorControlCommandMoveColorTemperatureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000004B,
    MTRCommandIDTypeClusterColorControlCommandStepColorTemperatureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000004C,

    // Cluster Channel deprecated command id names
    MTRClusterChannelCommandChangeChannelID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterChannelCommandChangeChannelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterChannelCommandChangeChannelResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterChannelCommandChangeChannelResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterChannelCommandChangeChannelByNumberID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterChannelCommandChangeChannelByNumberID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterChannelCommandSkipChannelID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterChannelCommandSkipChannelID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster Channel commands
    MTRCommandIDTypeClusterChannelCommandChangeChannelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterChannelCommandChangeChannelResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterChannelCommandChangeChannelByNumberID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterChannelCommandSkipChannelID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterChannelCommandGetProgramGuideID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterChannelCommandProgramGuideResponseID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRCommandIDTypeClusterChannelCommandRecordProgramID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRCommandIDTypeClusterChannelCommandCancelRecordProgramID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,

    // Cluster TargetNavigator deprecated command id names
    MTRClusterTargetNavigatorCommandNavigateTargetID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTargetNavigatorCommandNavigateTargetResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster TargetNavigator commands
    MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,

    // Cluster MediaPlayback deprecated command id names
    MTRClusterMediaPlaybackCommandPlayID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandPlayID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterMediaPlaybackCommandPauseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandPauseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterMediaPlaybackCommandStopPlaybackID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandStopID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterMediaPlaybackCommandStartOverID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandStartOverID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterMediaPlaybackCommandPreviousID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandPreviousID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterMediaPlaybackCommandNextID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandNextID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterMediaPlaybackCommandRewindID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandRewindID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterMediaPlaybackCommandFastForwardID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandFastForwardID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterMediaPlaybackCommandSkipForwardID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandSkipForwardID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterMediaPlaybackCommandSkipBackwardID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandSkipBackwardID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterMediaPlaybackCommandPlaybackResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandPlaybackResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterMediaPlaybackCommandSeekID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandSeekID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,

    // Cluster MediaPlayback commands
    MTRCommandIDTypeClusterMediaPlaybackCommandPlayID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterMediaPlaybackCommandPauseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterMediaPlaybackCommandStopID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterMediaPlaybackCommandStartOverID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterMediaPlaybackCommandPreviousID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterMediaPlaybackCommandNextID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRCommandIDTypeClusterMediaPlaybackCommandRewindID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRCommandIDTypeClusterMediaPlaybackCommandFastForwardID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRCommandIDTypeClusterMediaPlaybackCommandSkipForwardID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRCommandIDTypeClusterMediaPlaybackCommandSkipBackwardID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRCommandIDTypeClusterMediaPlaybackCommandPlaybackResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRCommandIDTypeClusterMediaPlaybackCommandSeekID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,
    MTRCommandIDTypeClusterMediaPlaybackCommandActivateAudioTrackID MTR_PROVISIONALLY_AVAILABLE = 0x0000000C,
    MTRCommandIDTypeClusterMediaPlaybackCommandActivateTextTrackID MTR_PROVISIONALLY_AVAILABLE = 0x0000000D,
    MTRCommandIDTypeClusterMediaPlaybackCommandDeactivateTextTrackID MTR_PROVISIONALLY_AVAILABLE = 0x0000000E,

    // Cluster MediaInput deprecated command id names
    MTRClusterMediaInputCommandSelectInputID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaInputCommandSelectInputID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterMediaInputCommandShowInputStatusID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaInputCommandShowInputStatusID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterMediaInputCommandHideInputStatusID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaInputCommandHideInputStatusID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterMediaInputCommandRenameInputID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterMediaInputCommandRenameInputID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster MediaInput commands
    MTRCommandIDTypeClusterMediaInputCommandSelectInputID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterMediaInputCommandShowInputStatusID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterMediaInputCommandHideInputStatusID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterMediaInputCommandRenameInputID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,

    // Cluster LowPower deprecated command id names
    MTRClusterLowPowerCommandSleepID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterLowPowerCommandSleepID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster LowPower commands
    MTRCommandIDTypeClusterLowPowerCommandSleepID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,

    // Cluster KeypadInput deprecated command id names
    MTRClusterKeypadInputCommandSendKeyID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterKeypadInputCommandSendKeyID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterKeypadInputCommandSendKeyResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterKeypadInputCommandSendKeyResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster KeypadInput commands
    MTRCommandIDTypeClusterKeypadInputCommandSendKeyID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterKeypadInputCommandSendKeyResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,

    // Cluster ContentLauncher deprecated command id names
    MTRClusterContentLauncherCommandLaunchContentID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterContentLauncherCommandLaunchContentID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterContentLauncherCommandLaunchURLID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterContentLauncherCommandLaunchURLID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterContentLauncherCommandLaunchResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterContentLauncherCommandLauncherResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,

    // Cluster ContentLauncher commands
    MTRCommandIDTypeClusterContentLauncherCommandLaunchContentID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterContentLauncherCommandLaunchURLID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterContentLauncherCommandLauncherResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,

    // Cluster AudioOutput deprecated command id names
    MTRClusterAudioOutputCommandSelectOutputID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterAudioOutputCommandSelectOutputID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterAudioOutputCommandRenameOutputID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterAudioOutputCommandRenameOutputID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster AudioOutput commands
    MTRCommandIDTypeClusterAudioOutputCommandSelectOutputID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterAudioOutputCommandRenameOutputID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,

    // Cluster ApplicationLauncher deprecated command id names
    MTRClusterApplicationLauncherCommandLaunchAppID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterApplicationLauncherCommandLaunchAppID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterApplicationLauncherCommandStopAppID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterApplicationLauncherCommandStopAppID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterApplicationLauncherCommandHideAppID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterApplicationLauncherCommandHideAppID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterApplicationLauncherCommandLauncherResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterApplicationLauncherCommandLauncherResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster ApplicationLauncher commands
    MTRCommandIDTypeClusterApplicationLauncherCommandLaunchAppID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterApplicationLauncherCommandStopAppID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterApplicationLauncherCommandHideAppID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterApplicationLauncherCommandLauncherResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,

    // Cluster AccountLogin deprecated command id names
    MTRClusterAccountLoginCommandGetSetupPINID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterAccountLoginCommandGetSetupPINResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterAccountLoginCommandLoginID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterAccountLoginCommandLoginID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterAccountLoginCommandLogoutID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterAccountLoginCommandLogoutID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster AccountLogin commands
    MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterAccountLoginCommandLoginID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterAccountLoginCommandLogoutID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,

    // Cluster ContentControl commands
    MTRCommandIDTypeClusterContentControlCommandUpdatePINID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterContentControlCommandResetPINID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterContentControlCommandResetPINResponseID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterContentControlCommandEnableID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterContentControlCommandDisableID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterContentControlCommandAddBonusTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTRCommandIDTypeClusterContentControlCommandSetScreenDailyTimeID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTRCommandIDTypeClusterContentControlCommandBlockUnratedContentID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTRCommandIDTypeClusterContentControlCommandUnblockUnratedContentID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTRCommandIDTypeClusterContentControlCommandSetOnDemandRatingThresholdID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTRCommandIDTypeClusterContentControlCommandSetScheduledContentRatingThresholdID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,

    // Cluster ContentAppObserver commands
    MTRCommandIDTypeClusterContentAppObserverCommandContentAppMessageID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterContentAppObserverCommandContentAppMessageResponseID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,

    // Cluster ElectricalMeasurement deprecated command id names
    MTRClusterElectricalMeasurementCommandGetProfileInfoResponseCommandID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoResponseCommandID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterElectricalMeasurementCommandGetProfileInfoCommandID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoCommandID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterElectricalMeasurementCommandGetMeasurementProfileResponseCommandID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileResponseCommandID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterElectricalMeasurementCommandGetMeasurementProfileCommandID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileCommandID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster ElectricalMeasurement commands
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoResponseCommandID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoCommandID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileResponseCommandID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileCommandID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,

    // Cluster TestCluster deprecated command id names
    MTRClusterTestClusterCommandTestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTestClusterCommandTestSpecificResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestSpecificResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterTestClusterCommandTestNotHandledID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestNotHandledID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTestClusterCommandTestAddArgumentsResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTestClusterCommandTestSpecificID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestSpecificID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterTestClusterCommandTestSimpleArgumentResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterTestClusterCommandTestUnknownCommandID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestUnknownCommandID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterTestClusterCommandTestStructArrayArgumentResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterTestClusterCommandTestAddArgumentsID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterTestClusterCommandTestListInt8UReverseResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterTestClusterCommandTestSimpleArgumentRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterTestClusterCommandTestEnumsResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestEnumsResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterTestClusterCommandTestStructArrayArgumentRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterTestClusterCommandTestNullableOptionalResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterTestClusterCommandTestStructArgumentRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestStructArgumentRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterTestClusterCommandTestComplexNullableOptionalResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterTestClusterCommandTestNestedStructArgumentRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructArgumentRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterTestClusterCommandBooleanResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandBooleanResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterTestClusterCommandTestListStructArgumentRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestListStructArgumentRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterTestClusterCommandSimpleStructResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandSimpleStructResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterTestClusterCommandTestListInt8UArgumentRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UArgumentRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterTestClusterCommandTestEmitTestEventResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterTestClusterCommandTestNestedStructListArgumentRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructListArgumentRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterTestClusterCommandTestEmitTestFabricScopedEventResponseID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventResponseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterTestClusterCommandTestListNestedStructListArgumentRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestListNestedStructListArgumentRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterTestClusterCommandTestListInt8UReverseRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterTestClusterCommandTestEnumsRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestEnumsRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterTestClusterCommandTestNullableOptionalRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterTestClusterCommandTestComplexNullableOptionalRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,
    MTRClusterTestClusterCommandSimpleStructEchoRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandSimpleStructEchoRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000011,
    MTRClusterTestClusterCommandTimedInvokeRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTimedInvokeRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000012,
    MTRClusterTestClusterCommandTestSimpleOptionalArgumentRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestSimpleOptionalArgumentRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000013,
    MTRClusterTestClusterCommandTestEmitTestEventRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000014,
    MTRClusterTestClusterCommandTestEmitTestFabricScopedEventRequestID
        MTR_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventRequestID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000015,

    // Cluster UnitTesting commands
    MTRCommandIDTypeClusterUnitTestingCommandTestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterUnitTestingCommandTestSpecificResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTRCommandIDTypeClusterUnitTestingCommandTestNotHandledID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTRCommandIDTypeClusterUnitTestingCommandTestSpecificID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTRCommandIDTypeClusterUnitTestingCommandTestUnknownCommandID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRCommandIDTypeClusterUnitTestingCommandTestEnumsResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTRCommandIDTypeClusterUnitTestingCommandTestStructArgumentRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructArgumentRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRCommandIDTypeClusterUnitTestingCommandBooleanResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTRCommandIDTypeClusterUnitTestingCommandTestListStructArgumentRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRCommandIDTypeClusterUnitTestingCommandSimpleStructResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UArgumentRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructListArgumentRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,
    MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventResponseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,
    MTRCommandIDTypeClusterUnitTestingCommandTestListNestedStructListArgumentRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000C,
    MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000D,
    MTRCommandIDTypeClusterUnitTestingCommandTestEnumsRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000E,
    MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000F,
    MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,
    MTRCommandIDTypeClusterUnitTestingCommandSimpleStructEchoRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000011,
    MTRCommandIDTypeClusterUnitTestingCommandTimedInvokeRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000012,
    MTRCommandIDTypeClusterUnitTestingCommandTestSimpleOptionalArgumentRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000013,
    MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000014,
    MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventRequestID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000015,

    // Cluster SampleMEI commands
    MTRCommandIDTypeClusterSampleMEICommandPingID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterSampleMEICommandAddArgumentsResponseID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterSampleMEICommandAddArgumentsID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,

};

#pragma mark - Events IDs

typedef NS_ENUM(uint32_t, MTREventIDType) {

    // Cluster AccessControl deprecated event names
    MTRClusterAccessControlEventAccessControlEntryChangedID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterAccessControlEventAccessControlEntryChangedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterAccessControlEventAccessControlExtensionChangedID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterAccessControlEventAccessControlExtensionChangedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster AccessControl events
    MTREventIDTypeClusterAccessControlEventAccessControlEntryChangedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTREventIDTypeClusterAccessControlEventAccessControlExtensionChangedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,

    // Cluster Actions deprecated event names
    MTRClusterActionsEventStateChangedID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterActionsEventStateChangedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterActionsEventActionFailedID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterActionsEventActionFailedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster Actions events
    MTREventIDTypeClusterActionsEventStateChangedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTREventIDTypeClusterActionsEventActionFailedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,

    // Cluster Basic deprecated event names
    MTRClusterBasicEventStartUpID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterBasicInformationEventStartUpID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterBasicEventShutDownID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterBasicInformationEventShutDownID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterBasicEventLeaveID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterBasicInformationEventLeaveID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterBasicEventReachableChangedID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterBasicInformationEventReachableChangedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster BasicInformation events
    MTREventIDTypeClusterBasicInformationEventStartUpID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTREventIDTypeClusterBasicInformationEventShutDownID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTREventIDTypeClusterBasicInformationEventLeaveID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTREventIDTypeClusterBasicInformationEventReachableChangedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,

    // Cluster OtaSoftwareUpdateRequestor deprecated event names
    MTRClusterOtaSoftwareUpdateRequestorEventStateTransitionID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterOTASoftwareUpdateRequestorEventStateTransitionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterOtaSoftwareUpdateRequestorEventVersionAppliedID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterOTASoftwareUpdateRequestorEventVersionAppliedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterOtaSoftwareUpdateRequestorEventDownloadErrorID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterOTASoftwareUpdateRequestorEventDownloadErrorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,

    // Cluster OTASoftwareUpdateRequestor events
    MTREventIDTypeClusterOTASoftwareUpdateRequestorEventStateTransitionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTREventIDTypeClusterOTASoftwareUpdateRequestorEventVersionAppliedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTREventIDTypeClusterOTASoftwareUpdateRequestorEventDownloadErrorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,

    // Cluster PowerSource deprecated event names

    // Cluster PowerSource events
    MTREventIDTypeClusterPowerSourceEventWiredFaultChangeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTREventIDTypeClusterPowerSourceEventBatFaultChangeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTREventIDTypeClusterPowerSourceEventBatChargeFaultChangeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,

    // Cluster GeneralDiagnostics deprecated event names
    MTRClusterGeneralDiagnosticsEventHardwareFaultChangeID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterGeneralDiagnosticsEventHardwareFaultChangeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterGeneralDiagnosticsEventRadioFaultChangeID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterGeneralDiagnosticsEventRadioFaultChangeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterGeneralDiagnosticsEventNetworkFaultChangeID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterGeneralDiagnosticsEventNetworkFaultChangeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterGeneralDiagnosticsEventBootReasonID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterGeneralDiagnosticsEventBootReasonID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster GeneralDiagnostics events
    MTREventIDTypeClusterGeneralDiagnosticsEventHardwareFaultChangeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTREventIDTypeClusterGeneralDiagnosticsEventRadioFaultChangeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTREventIDTypeClusterGeneralDiagnosticsEventNetworkFaultChangeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTREventIDTypeClusterGeneralDiagnosticsEventBootReasonID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,

    // Cluster SoftwareDiagnostics deprecated event names
    MTRClusterSoftwareDiagnosticsEventSoftwareFaultID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterSoftwareDiagnosticsEventSoftwareFaultID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster SoftwareDiagnostics events
    MTREventIDTypeClusterSoftwareDiagnosticsEventSoftwareFaultID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,

    // Cluster ThreadNetworkDiagnostics deprecated event names
    MTRClusterThreadNetworkDiagnosticsEventConnectionStatusID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterThreadNetworkDiagnosticsEventConnectionStatusID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterThreadNetworkDiagnosticsEventNetworkFaultChangeID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterThreadNetworkDiagnosticsEventNetworkFaultChangeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,

    // Cluster ThreadNetworkDiagnostics events
    MTREventIDTypeClusterThreadNetworkDiagnosticsEventConnectionStatusID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTREventIDTypeClusterThreadNetworkDiagnosticsEventNetworkFaultChangeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,

    // Cluster WiFiNetworkDiagnostics deprecated event names
    MTRClusterWiFiNetworkDiagnosticsEventDisconnectionID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterWiFiNetworkDiagnosticsEventDisconnectionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterWiFiNetworkDiagnosticsEventAssociationFailureID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterWiFiNetworkDiagnosticsEventAssociationFailureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterWiFiNetworkDiagnosticsEventConnectionStatusID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterWiFiNetworkDiagnosticsEventConnectionStatusID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,

    // Cluster WiFiNetworkDiagnostics events
    MTREventIDTypeClusterWiFiNetworkDiagnosticsEventDisconnectionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTREventIDTypeClusterWiFiNetworkDiagnosticsEventAssociationFailureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTREventIDTypeClusterWiFiNetworkDiagnosticsEventConnectionStatusID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,

    // Cluster TimeSynchronization deprecated event names

    // Cluster TimeSynchronization events
    MTREventIDTypeClusterTimeSynchronizationEventDSTTableEmptyID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterTimeSynchronizationEventDSTStatusID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterTimeSynchronizationEventTimeZoneStatusID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTREventIDTypeClusterTimeSynchronizationEventTimeFailureID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTREventIDTypeClusterTimeSynchronizationEventMissingTrustedTimeSourceID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,

    // Cluster BridgedDeviceBasic deprecated event names
    MTRClusterBridgedDeviceBasicEventStartUpID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterBridgedDeviceBasicInformationEventStartUpID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterBridgedDeviceBasicEventShutDownID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterBridgedDeviceBasicInformationEventShutDownID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterBridgedDeviceBasicEventLeaveID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterBridgedDeviceBasicInformationEventLeaveID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterBridgedDeviceBasicEventReachableChangedID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterBridgedDeviceBasicInformationEventReachableChangedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,

    // Cluster BridgedDeviceBasicInformation events
    MTREventIDTypeClusterBridgedDeviceBasicInformationEventStartUpID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTREventIDTypeClusterBridgedDeviceBasicInformationEventShutDownID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTREventIDTypeClusterBridgedDeviceBasicInformationEventLeaveID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTREventIDTypeClusterBridgedDeviceBasicInformationEventReachableChangedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,

    // Cluster Switch deprecated event names
    MTRClusterSwitchEventSwitchLatchedID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventSwitchLatchedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterSwitchEventInitialPressID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventInitialPressID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterSwitchEventLongPressID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventLongPressID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterSwitchEventShortReleaseID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventShortReleaseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterSwitchEventLongReleaseID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventLongReleaseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterSwitchEventMultiPressOngoingID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventMultiPressOngoingID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterSwitchEventMultiPressCompleteID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventMultiPressCompleteID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,

    // Cluster Switch events
    MTREventIDTypeClusterSwitchEventSwitchLatchedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTREventIDTypeClusterSwitchEventInitialPressID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTREventIDTypeClusterSwitchEventLongPressID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTREventIDTypeClusterSwitchEventShortReleaseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTREventIDTypeClusterSwitchEventLongReleaseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTREventIDTypeClusterSwitchEventMultiPressOngoingID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTREventIDTypeClusterSwitchEventMultiPressCompleteID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,

    // Cluster BooleanState deprecated event names
    MTRClusterBooleanStateEventStateChangeID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterBooleanStateEventStateChangeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,

    // Cluster BooleanState events
    MTREventIDTypeClusterBooleanStateEventStateChangeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,

    // Cluster OvenCavityOperationalState events
    MTREventIDTypeClusterOvenCavityOperationalStateEventOperationalErrorID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterOvenCavityOperationalStateEventOperationCompletionID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,

    // Cluster RefrigeratorAlarm events
    MTREventIDTypeClusterRefrigeratorAlarmEventNotifyID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,

    // Cluster SmokeCOAlarm events
    MTREventIDTypeClusterSmokeCOAlarmEventSmokeAlarmID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterSmokeCOAlarmEventCOAlarmID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterSmokeCOAlarmEventLowBatteryID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTREventIDTypeClusterSmokeCOAlarmEventHardwareFaultID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTREventIDTypeClusterSmokeCOAlarmEventEndOfServiceID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTREventIDTypeClusterSmokeCOAlarmEventSelfTestCompleteID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,
    MTREventIDTypeClusterSmokeCOAlarmEventAlarmMutedID MTR_PROVISIONALLY_AVAILABLE = 0x00000006,
    MTREventIDTypeClusterSmokeCOAlarmEventMuteEndedID MTR_PROVISIONALLY_AVAILABLE = 0x00000007,
    MTREventIDTypeClusterSmokeCOAlarmEventInterconnectSmokeAlarmID MTR_PROVISIONALLY_AVAILABLE = 0x00000008,
    MTREventIDTypeClusterSmokeCOAlarmEventInterconnectCOAlarmID MTR_PROVISIONALLY_AVAILABLE = 0x00000009,
    MTREventIDTypeClusterSmokeCOAlarmEventAllClearID MTR_PROVISIONALLY_AVAILABLE = 0x0000000A,

    // Cluster DishwasherAlarm events
    MTREventIDTypeClusterDishwasherAlarmEventNotifyID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,

    // Cluster OperationalState events
    MTREventIDTypeClusterOperationalStateEventOperationalErrorID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000000,
    MTREventIDTypeClusterOperationalStateEventOperationCompletionID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000001,

    // Cluster RVCOperationalState events
    MTREventIDTypeClusterRVCOperationalStateEventOperationalErrorID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000000,
    MTREventIDTypeClusterRVCOperationalStateEventOperationCompletionID MTR_AVAILABLE(ios(17.4), macos(14.4), watchos(10.4), tvos(17.4)) = 0x00000001,

    // Cluster BooleanStateConfiguration events
    MTREventIDTypeClusterBooleanStateConfigurationEventAlarmsStateChangedID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterBooleanStateConfigurationEventSensorFaultID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,

    // Cluster ValveConfigurationAndControl events
    MTREventIDTypeClusterValveConfigurationAndControlEventValveStateChangedID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterValveConfigurationAndControlEventValveFaultID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,

    // Cluster ElectricalEnergyMeasurement events
    MTREventIDTypeClusterElectricalEnergyMeasurementEventCumulativeEnergyMeasuredID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterElectricalEnergyMeasurementEventPeriodicEnergyMeasuredID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,

    // Cluster DemandResponseLoadControl events
    MTREventIDTypeClusterDemandResponseLoadControlEventLoadControlEventStatusChangeID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,

    // Cluster DeviceEnergyManagement events
    MTREventIDTypeClusterDeviceEnergyManagementEventPowerAdjustStartID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterDeviceEnergyManagementEventPowerAdjustEndID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterDeviceEnergyManagementEventPausedID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTREventIDTypeClusterDeviceEnergyManagementEventResumedID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,

    // Cluster EnergyEVSE events
    MTREventIDTypeClusterEnergyEVSEEventEVConnectedID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterEnergyEVSEEventEVNotDetectedID MTR_PROVISIONALLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterEnergyEVSEEventEnergyTransferStartedID MTR_PROVISIONALLY_AVAILABLE = 0x00000002,
    MTREventIDTypeClusterEnergyEVSEEventEnergyTransferStoppedID MTR_PROVISIONALLY_AVAILABLE = 0x00000003,
    MTREventIDTypeClusterEnergyEVSEEventFaultID MTR_PROVISIONALLY_AVAILABLE = 0x00000004,
    MTREventIDTypeClusterEnergyEVSEEventRFIDID MTR_PROVISIONALLY_AVAILABLE = 0x00000005,

    // Cluster DoorLock deprecated event names
    MTRClusterDoorLockEventDoorLockAlarmID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventDoorLockAlarmID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterDoorLockEventDoorStateChangeID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventDoorStateChangeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterDoorLockEventLockOperationID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventLockOperationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterDoorLockEventLockOperationErrorID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventLockOperationErrorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterDoorLockEventLockUserChangeID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventLockUserChangeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,

    // Cluster DoorLock events
    MTREventIDTypeClusterDoorLockEventDoorLockAlarmID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTREventIDTypeClusterDoorLockEventDoorStateChangeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTREventIDTypeClusterDoorLockEventLockOperationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTREventIDTypeClusterDoorLockEventLockOperationErrorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTREventIDTypeClusterDoorLockEventLockUserChangeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,

    // Cluster PumpConfigurationAndControl deprecated event names
    MTRClusterPumpConfigurationAndControlEventSupplyVoltageLowID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageLowID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000000,
    MTRClusterPumpConfigurationAndControlEventSupplyVoltageHighID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageHighID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterPumpConfigurationAndControlEventPowerMissingPhaseID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventPowerMissingPhaseID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,
    MTRClusterPumpConfigurationAndControlEventSystemPressureLowID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureLowID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000003,
    MTRClusterPumpConfigurationAndControlEventSystemPressureHighID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureHighID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000004,
    MTRClusterPumpConfigurationAndControlEventDryRunningID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventDryRunningID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000005,
    MTRClusterPumpConfigurationAndControlEventMotorTemperatureHighID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventMotorTemperatureHighID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000006,
    MTRClusterPumpConfigurationAndControlEventPumpMotorFatalFailureID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventPumpMotorFatalFailureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000007,
    MTRClusterPumpConfigurationAndControlEventElectronicTemperatureHighID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicTemperatureHighID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000008,
    MTRClusterPumpConfigurationAndControlEventPumpBlockedID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventPumpBlockedID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000009,
    MTRClusterPumpConfigurationAndControlEventSensorFailureID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSensorFailureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000A,
    MTRClusterPumpConfigurationAndControlEventElectronicNonFatalFailureID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicNonFatalFailureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000B,
    MTRClusterPumpConfigurationAndControlEventElectronicFatalFailureID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicFatalFailureID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000C,
    MTRClusterPumpConfigurationAndControlEventGeneralFaultID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventGeneralFaultID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000D,
    MTRClusterPumpConfigurationAndControlEventLeakageID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventLeakageID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000E,
    MTRClusterPumpConfigurationAndControlEventAirDetectionID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventAirDetectionID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x0000000F,
    MTRClusterPumpConfigurationAndControlEventTurbineOperationID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventTurbineOperationID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000010,

    // Cluster PumpConfigurationAndControl events
    MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageLowID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000000,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageHighID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTREventIDTypeClusterPumpConfigurationAndControlEventPowerMissingPhaseID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureLowID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000003,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureHighID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000004,
    MTREventIDTypeClusterPumpConfigurationAndControlEventDryRunningID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000005,
    MTREventIDTypeClusterPumpConfigurationAndControlEventMotorTemperatureHighID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000006,
    MTREventIDTypeClusterPumpConfigurationAndControlEventPumpMotorFatalFailureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000007,
    MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicTemperatureHighID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000008,
    MTREventIDTypeClusterPumpConfigurationAndControlEventPumpBlockedID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000009,
    MTREventIDTypeClusterPumpConfigurationAndControlEventSensorFailureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000A,
    MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicNonFatalFailureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000B,
    MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicFatalFailureID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000C,
    MTREventIDTypeClusterPumpConfigurationAndControlEventGeneralFaultID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000D,
    MTREventIDTypeClusterPumpConfigurationAndControlEventLeakageID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000E,
    MTREventIDTypeClusterPumpConfigurationAndControlEventAirDetectionID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x0000000F,
    MTREventIDTypeClusterPumpConfigurationAndControlEventTurbineOperationID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000010,

    // Cluster TargetNavigator deprecated event names

    // Cluster TargetNavigator events
    MTREventIDTypeClusterTargetNavigatorEventTargetUpdatedID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,

    // Cluster MediaPlayback deprecated event names

    // Cluster MediaPlayback events
    MTREventIDTypeClusterMediaPlaybackEventStateChangedID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,

    // Cluster AccountLogin deprecated event names

    // Cluster AccountLogin events
    MTREventIDTypeClusterAccountLoginEventLoggedOutID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,

    // Cluster ContentControl events
    MTREventIDTypeClusterContentControlEventRemainingScreenTimeExpiredID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,

    // Cluster TestCluster deprecated event names
    MTRClusterTestClusterEventTestEventID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterUnitTestingEventTestEventID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000001,
    MTRClusterTestClusterEventTestFabricScopedEventID
        MTR_DEPRECATED("Please use MTREventIDTypeClusterUnitTestingEventTestFabricScopedEventID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0x00000002,

    // Cluster UnitTesting events
    MTREventIDTypeClusterUnitTestingEventTestEventID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000001,
    MTREventIDTypeClusterUnitTestingEventTestFabricScopedEventID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4)) = 0x00000002,

    // Cluster SampleMEI events
    MTREventIDTypeClusterSampleMEIEventPingCountEventID MTR_PROVISIONALLY_AVAILABLE = 0x00000000,

};
