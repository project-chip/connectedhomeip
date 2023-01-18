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
    MTRClusterIdentifyID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeIdentifyID")
    = 0x00000003,
    MTRClusterGroupsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeGroupsID")
    = 0x00000004,
    MTRClusterScenesID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeScenesID")
    = 0x00000005,
    MTRClusterOnOffID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeOnOffID")
    = 0x00000006,
    MTRClusterOnOffSwitchConfigurationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeOnOffSwitchConfigurationID")
    = 0x00000007,
    MTRClusterLevelControlID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeLevelControlID")
    = 0x00000008,
    MTRClusterBinaryInputBasicID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeBinaryInputBasicID")
    = 0x0000000F,
    MTRClusterPulseWidthModulationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypePulseWidthModulationID")
    = 0x0000001C,
    MTRClusterDescriptorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeDescriptorID")
    = 0x0000001D,
    MTRClusterBindingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeBindingID")
    = 0x0000001E,
    MTRClusterAccessControlID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeAccessControlID")
    = 0x0000001F,
    MTRClusterActionsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeActionsID")
    = 0x00000025,
    MTRClusterBasicID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeBasicInformationID")
    = 0x00000028,
    MTRClusterOtaSoftwareUpdateProviderID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeOTASoftwareUpdateProviderID")
    = 0x00000029,
    MTRClusterOtaSoftwareUpdateRequestorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeOTASoftwareUpdateRequestorID")
    = 0x0000002A,
    MTRClusterLocalizationConfigurationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeLocalizationConfigurationID")
    = 0x0000002B,
    MTRClusterTimeFormatLocalizationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeTimeFormatLocalizationID")
    = 0x0000002C,
    MTRClusterUnitLocalizationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeUnitLocalizationID")
    = 0x0000002D,
    MTRClusterPowerSourceConfigurationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypePowerSourceConfigurationID")
    = 0x0000002E,
    MTRClusterPowerSourceID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypePowerSourceID")
    = 0x0000002F,
    MTRClusterGeneralCommissioningID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeGeneralCommissioningID")
    = 0x00000030,
    MTRClusterNetworkCommissioningID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeNetworkCommissioningID")
    = 0x00000031,
    MTRClusterDiagnosticLogsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeDiagnosticLogsID")
    = 0x00000032,
    MTRClusterGeneralDiagnosticsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeGeneralDiagnosticsID")
    = 0x00000033,
    MTRClusterSoftwareDiagnosticsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeSoftwareDiagnosticsID")
    = 0x00000034,
    MTRClusterThreadNetworkDiagnosticsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeThreadNetworkDiagnosticsID")
    = 0x00000035,
    MTRClusterWiFiNetworkDiagnosticsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeWiFiNetworkDiagnosticsID")
    = 0x00000036,
    MTRClusterEthernetNetworkDiagnosticsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeEthernetNetworkDiagnosticsID")
    = 0x00000037,
    MTRClusterTimeSynchronizationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeTimeSynchronizationID")
    = 0x00000038,
    MTRClusterBridgedDeviceBasicID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeBridgedDeviceBasicInformationID")
    = 0x00000039,
    MTRClusterSwitchID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeSwitchID")
    = 0x0000003B,
    MTRClusterAdministratorCommissioningID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeAdministratorCommissioningID")
    = 0x0000003C,
    MTRClusterOperationalCredentialsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeOperationalCredentialsID")
    = 0x0000003E,
    MTRClusterGroupKeyManagementID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeGroupKeyManagementID")
    = 0x0000003F,
    MTRClusterFixedLabelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeFixedLabelID")
    = 0x00000040,
    MTRClusterUserLabelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeUserLabelID")
    = 0x00000041,
    MTRClusterProxyConfigurationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeProxyConfigurationID")
    = 0x00000042,
    MTRClusterProxyDiscoveryID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeProxyDiscoveryID")
    = 0x00000043,
    MTRClusterProxyValidID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeProxyValidID")
    = 0x00000044,
    MTRClusterBooleanStateID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeBooleanStateID")
    = 0x00000045,
    MTRClusterModeSelectID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeModeSelectID")
    = 0x00000050,
    MTRClusterDoorLockID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeDoorLockID")
    = 0x00000101,
    MTRClusterWindowCoveringID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeWindowCoveringID")
    = 0x00000102,
    MTRClusterBarrierControlID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeBarrierControlID")
    = 0x00000103,
    MTRClusterPumpConfigurationAndControlID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypePumpConfigurationAndControlID")
    = 0x00000200,
    MTRClusterThermostatID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeThermostatID")
    = 0x00000201,
    MTRClusterFanControlID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeFanControlID")
    = 0x00000202,
    MTRClusterThermostatUserInterfaceConfigurationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeThermostatUserInterfaceConfigurationID")
    = 0x00000204,
    MTRClusterColorControlID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeColorControlID")
    = 0x00000300,
    MTRClusterBallastConfigurationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeBallastConfigurationID")
    = 0x00000301,
    MTRClusterIlluminanceMeasurementID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeIlluminanceMeasurementID")
    = 0x00000400,
    MTRClusterTemperatureMeasurementID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeTemperatureMeasurementID")
    = 0x00000402,
    MTRClusterPressureMeasurementID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypePressureMeasurementID")
    = 0x00000403,
    MTRClusterFlowMeasurementID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeFlowMeasurementID")
    = 0x00000404,
    MTRClusterRelativeHumidityMeasurementID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeRelativeHumidityMeasurementID")
    = 0x00000405,
    MTRClusterOccupancySensingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeOccupancySensingID")
    = 0x00000406,
    MTRClusterWakeOnLanID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeWakeOnLANID")
    = 0x00000503,
    MTRClusterChannelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeChannelID")
    = 0x00000504,
    MTRClusterTargetNavigatorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeTargetNavigatorID")
    = 0x00000505,
    MTRClusterMediaPlaybackID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeMediaPlaybackID")
    = 0x00000506,
    MTRClusterMediaInputID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeMediaInputID")
    = 0x00000507,
    MTRClusterLowPowerID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeLowPowerID")
    = 0x00000508,
    MTRClusterKeypadInputID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeKeypadInputID")
    = 0x00000509,
    MTRClusterContentLauncherID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeContentLauncherID")
    = 0x0000050A,
    MTRClusterAudioOutputID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeAudioOutputID")
    = 0x0000050B,
    MTRClusterApplicationLauncherID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeApplicationLauncherID")
    = 0x0000050C,
    MTRClusterApplicationBasicID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeApplicationBasicID")
    = 0x0000050D,
    MTRClusterAccountLoginID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeAccountLoginID")
    = 0x0000050E,
    MTRClusterElectricalMeasurementID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeElectricalMeasurementID")
    = 0x00000B04,
    MTRClusterTestClusterID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeUnitTestingID")
    = 0xFFF1FC05,
    MTRClusterFaultInjectionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRClusterIDTypeFaultInjectionID")
    = 0xFFF1FC06,
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
    MTRClusterIDTypeBasicInformationID MTR_NEWLY_AVAILABLE = 0x00000028,
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
    MTRClusterIDTypeBridgedDeviceBasicInformationID MTR_NEWLY_AVAILABLE = 0x00000039,
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
    MTRClusterIDTypeUnitTestingID MTR_NEWLY_AVAILABLE = 0xFFF1FC05,
    MTRClusterIDTypeFaultInjectionID MTR_NEWLY_AVAILABLE = 0xFFF1FC06,
};

#pragma mark - Attributes IDs

typedef NS_ENUM(uint32_t, MTRAttributeIDType) {
    // Deprecated global attribute names
    MTRClusterGlobalAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID")
    = 0x0000FFF8,
    MTRClusterGlobalAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID")
    = 0x0000FFF9,
    MTRClusterGlobalAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeAttributeListID")
    = 0x0000FFFB,
    MTRClusterGlobalAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeFeatureMapID")
    = 0x0000FFFC,
    MTRClusterGlobalAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeGlobalAttributeClusterRevisionID")
    = 0x0000FFFD,

    // Global attributes
    MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE = 0x0000FFF8,
    MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE = 0x0000FFF9,
    MTRAttributeIDTypeGlobalAttributeAttributeListID MTR_NEWLY_AVAILABLE = 0x0000FFFB,
    MTRAttributeIDTypeGlobalAttributeFeatureMapID MTR_NEWLY_AVAILABLE = 0x0000FFFC,
    MTRAttributeIDTypeGlobalAttributeClusterRevisionID MTR_NEWLY_AVAILABLE = 0x0000FFFD,

    // Cluster Identify deprecated attribute names
    MTRClusterIdentifyAttributeIdentifyTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTimeID")
    = 0x00000000,
    MTRClusterIdentifyAttributeIdentifyTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeIdentifyTypeID")
    = 0x00000001,
    MTRClusterIdentifyAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterIdentifyAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterIdentifyAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterIdentifyAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterIdentifyAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIdentifyAttributeClusterRevisionID")
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
    MTRClusterGroupsAttributeNameSupportID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeNameSupportID")
    = 0x00000000,
    MTRClusterGroupsAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGroupsAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGroupsAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGroupsAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGroupsAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupsAttributeClusterRevisionID")
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
    MTRClusterScenesAttributeSceneCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeSceneCountID")
    = 0x00000000,
    MTRClusterScenesAttributeCurrentSceneID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeCurrentSceneID")
    = 0x00000001,
    MTRClusterScenesAttributeCurrentGroupID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeCurrentGroupID")
    = 0x00000002,
    MTRClusterScenesAttributeSceneValidID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeSceneValidID")
    = 0x00000003,
    MTRClusterScenesAttributeNameSupportID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeNameSupportID")
    = 0x00000004,
    MTRClusterScenesAttributeLastConfiguredByID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeLastConfiguredByID")
    = 0x00000005,
    MTRClusterScenesAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterScenesAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterScenesAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterScenesAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterScenesAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterScenesAttributeClusterRevisionID")
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
    MTRClusterOnOffAttributeOnOffID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeOnOffID")
    = 0x00000000,
    MTRClusterOnOffAttributeGlobalSceneControlID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeGlobalSceneControlID")
    = 0x00004000,
    MTRClusterOnOffAttributeOnTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeOnTimeID")
    = 0x00004001,
    MTRClusterOnOffAttributeOffWaitTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeOffWaitTimeID")
    = 0x00004002,
    MTRClusterOnOffAttributeStartUpOnOffID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeStartUpOnOffID")
    = 0x00004003,
    MTRClusterOnOffAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOnOffAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOnOffAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOnOffAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOnOffAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffAttributeClusterRevisionID")
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
    MTRClusterOnOffSwitchConfigurationAttributeSwitchTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchTypeID")
    = 0x00000000,
    MTRClusterOnOffSwitchConfigurationAttributeSwitchActionsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeSwitchActionsID")
    = 0x00000010,
    MTRClusterOnOffSwitchConfigurationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOnOffSwitchConfigurationAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOnOffSwitchConfigurationAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOnOffSwitchConfigurationAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOnOffSwitchConfigurationAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOnOffSwitchConfigurationAttributeClusterRevisionID")
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
    MTRClusterLevelControlAttributeCurrentLevelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeCurrentLevelID")
    = 0x00000000,
    MTRClusterLevelControlAttributeRemainingTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeRemainingTimeID")
    = 0x00000001,
    MTRClusterLevelControlAttributeMinLevelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeMinLevelID")
    = 0x00000002,
    MTRClusterLevelControlAttributeMaxLevelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeMaxLevelID")
    = 0x00000003,
    MTRClusterLevelControlAttributeCurrentFrequencyID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeCurrentFrequencyID")
    = 0x00000004,
    MTRClusterLevelControlAttributeMinFrequencyID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeMinFrequencyID")
    = 0x00000005,
    MTRClusterLevelControlAttributeMaxFrequencyID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeMaxFrequencyID")
    = 0x00000006,
    MTRClusterLevelControlAttributeOptionsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeOptionsID")
    = 0x0000000F,
    MTRClusterLevelControlAttributeOnOffTransitionTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeOnOffTransitionTimeID")
    = 0x00000010,
    MTRClusterLevelControlAttributeOnLevelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeOnLevelID")
    = 0x00000011,
    MTRClusterLevelControlAttributeOnTransitionTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeOnTransitionTimeID")
    = 0x00000012,
    MTRClusterLevelControlAttributeOffTransitionTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeOffTransitionTimeID")
    = 0x00000013,
    MTRClusterLevelControlAttributeDefaultMoveRateID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeDefaultMoveRateID")
    = 0x00000014,
    MTRClusterLevelControlAttributeStartUpCurrentLevelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeStartUpCurrentLevelID")
    = 0x00004000,
    MTRClusterLevelControlAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterLevelControlAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterLevelControlAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterLevelControlAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterLevelControlAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLevelControlAttributeClusterRevisionID")
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
    MTRClusterBinaryInputBasicAttributeActiveTextID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeActiveTextID")
    = 0x00000004,
    MTRClusterBinaryInputBasicAttributeDescriptionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeDescriptionID")
    = 0x0000001C,
    MTRClusterBinaryInputBasicAttributeInactiveTextID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeInactiveTextID")
    = 0x0000002E,
    MTRClusterBinaryInputBasicAttributeOutOfServiceID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeOutOfServiceID")
    = 0x00000051,
    MTRClusterBinaryInputBasicAttributePolarityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributePolarityID")
    = 0x00000054,
    MTRClusterBinaryInputBasicAttributePresentValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributePresentValueID")
    = 0x00000055,
    MTRClusterBinaryInputBasicAttributeReliabilityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeReliabilityID")
    = 0x00000067,
    MTRClusterBinaryInputBasicAttributeStatusFlagsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeStatusFlagsID")
    = 0x0000006F,
    MTRClusterBinaryInputBasicAttributeApplicationTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeApplicationTypeID")
    = 0x00000100,
    MTRClusterBinaryInputBasicAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBinaryInputBasicAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBinaryInputBasicAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBinaryInputBasicAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBinaryInputBasicAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBinaryInputBasicAttributeClusterRevisionID")
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
    MTRClusterPulseWidthModulationAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPulseWidthModulationAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPulseWidthModulationAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPulseWidthModulationAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPulseWidthModulationAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPulseWidthModulationAttributeClusterRevisionID")
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
    MTRClusterDescriptorAttributeDeviceListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeDeviceTypeListID")
    = 0x00000000,
    MTRClusterDescriptorAttributeServerListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeServerListID")
    = 0x00000001,
    MTRClusterDescriptorAttributeClientListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeClientListID")
    = 0x00000002,
    MTRClusterDescriptorAttributePartsListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributePartsListID")
    = 0x00000003,
    MTRClusterDescriptorAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterDescriptorAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterDescriptorAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterDescriptorAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterDescriptorAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDescriptorAttributeClusterRevisionID")
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
    MTRClusterBindingAttributeBindingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeBindingID")
    = 0x00000000,
    MTRClusterBindingAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBindingAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBindingAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBindingAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBindingAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBindingAttributeClusterRevisionID")
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
    MTRClusterAccessControlAttributeAclID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeACLID")
    = 0x00000000,
    MTRClusterAccessControlAttributeExtensionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeExtensionID")
    = 0x00000001,
    MTRClusterAccessControlAttributeSubjectsPerAccessControlEntryID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeSubjectsPerAccessControlEntryID")
    = 0x00000002,
    MTRClusterAccessControlAttributeTargetsPerAccessControlEntryID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeTargetsPerAccessControlEntryID")
    = 0x00000003,
    MTRClusterAccessControlAttributeAccessControlEntriesPerFabricID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeAccessControlEntriesPerFabricID")
    = 0x00000004,
    MTRClusterAccessControlAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAccessControlAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAccessControlAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAccessControlAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAccessControlAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccessControlAttributeClusterRevisionID")
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
    MTRClusterActionsAttributeActionListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeActionListID")
    = 0x00000000,
    MTRClusterActionsAttributeEndpointListsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeEndpointListsID")
    = 0x00000001,
    MTRClusterActionsAttributeSetupURLID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeSetupURLID")
    = 0x00000002,
    MTRClusterActionsAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterActionsAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterActionsAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterActionsAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterActionsAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterActionsAttributeClusterRevisionID")
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
    MTRClusterBasicAttributeDataModelRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeDataModelRevisionID")
    = 0x00000000,
    MTRClusterBasicAttributeVendorNameID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeVendorNameID")
    = 0x00000001,
    MTRClusterBasicAttributeVendorIDID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeVendorIDID")
    = 0x00000002,
    MTRClusterBasicAttributeProductNameID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeProductNameID")
    = 0x00000003,
    MTRClusterBasicAttributeProductIDID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeProductIDID")
    = 0x00000004,
    MTRClusterBasicAttributeNodeLabelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeNodeLabelID")
    = 0x00000005,
    MTRClusterBasicAttributeLocationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeLocationID")
    = 0x00000006,
    MTRClusterBasicAttributeHardwareVersionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeHardwareVersionID")
    = 0x00000007,
    MTRClusterBasicAttributeHardwareVersionStringID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeHardwareVersionStringID")
    = 0x00000008,
    MTRClusterBasicAttributeSoftwareVersionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeSoftwareVersionID")
    = 0x00000009,
    MTRClusterBasicAttributeSoftwareVersionStringID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeSoftwareVersionStringID")
    = 0x0000000A,
    MTRClusterBasicAttributeManufacturingDateID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeManufacturingDateID")
    = 0x0000000B,
    MTRClusterBasicAttributePartNumberID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributePartNumberID")
    = 0x0000000C,
    MTRClusterBasicAttributeProductURLID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeProductURLID")
    = 0x0000000D,
    MTRClusterBasicAttributeProductLabelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeProductLabelID")
    = 0x0000000E,
    MTRClusterBasicAttributeSerialNumberID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeSerialNumberID")
    = 0x0000000F,
    MTRClusterBasicAttributeLocalConfigDisabledID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeLocalConfigDisabledID")
    = 0x00000010,
    MTRClusterBasicAttributeReachableID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeReachableID")
    = 0x00000011,
    MTRClusterBasicAttributeUniqueIDID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeUniqueIDID")
    = 0x00000012,
    MTRClusterBasicAttributeCapabilityMinimaID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeCapabilityMinimaID")
    = 0x00000013,
    MTRClusterBasicAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBasicAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBasicAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBasicAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBasicAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBasicInformationAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BasicInformation attributes
    MTRAttributeIDTypeClusterBasicInformationAttributeDataModelRevisionID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterBasicInformationAttributeVendorNameID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterBasicInformationAttributeVendorIDID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterBasicInformationAttributeProductNameID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterBasicInformationAttributeProductIDID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterBasicInformationAttributeNodeLabelID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterBasicInformationAttributeLocationID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterBasicInformationAttributeHardwareVersionID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterBasicInformationAttributeHardwareVersionStringID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterBasicInformationAttributeSoftwareVersionID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterBasicInformationAttributeSoftwareVersionStringID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterBasicInformationAttributeManufacturingDateID MTR_NEWLY_AVAILABLE = 0x0000000B,
    MTRAttributeIDTypeClusterBasicInformationAttributePartNumberID MTR_NEWLY_AVAILABLE = 0x0000000C,
    MTRAttributeIDTypeClusterBasicInformationAttributeProductURLID MTR_NEWLY_AVAILABLE = 0x0000000D,
    MTRAttributeIDTypeClusterBasicInformationAttributeProductLabelID MTR_NEWLY_AVAILABLE = 0x0000000E,
    MTRAttributeIDTypeClusterBasicInformationAttributeSerialNumberID MTR_NEWLY_AVAILABLE = 0x0000000F,
    MTRAttributeIDTypeClusterBasicInformationAttributeLocalConfigDisabledID MTR_NEWLY_AVAILABLE = 0x00000010,
    MTRAttributeIDTypeClusterBasicInformationAttributeReachableID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterBasicInformationAttributeUniqueIDID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRAttributeIDTypeClusterBasicInformationAttributeCapabilityMinimaID MTR_NEWLY_AVAILABLE = 0x00000013,
    MTRAttributeIDTypeClusterBasicInformationAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBasicInformationAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBasicInformationAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBasicInformationAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBasicInformationAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster OtaSoftwareUpdateProvider deprecated attribute names
    MTRClusterOtaSoftwareUpdateProviderAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOtaSoftwareUpdateProviderAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOtaSoftwareUpdateProviderAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOtaSoftwareUpdateProviderAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOtaSoftwareUpdateProviderAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateProviderAttributeClusterRevisionID")
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
    MTRClusterOtaSoftwareUpdateRequestorAttributeDefaultOtaProvidersID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeDefaultOTAProvidersID")
    = 0x00000000,
    MTRClusterOtaSoftwareUpdateRequestorAttributeUpdatePossibleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdatePossibleID")
    = 0x00000001,
    MTRClusterOtaSoftwareUpdateRequestorAttributeUpdateStateID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateID")
    = 0x00000002,
    MTRClusterOtaSoftwareUpdateRequestorAttributeUpdateStateProgressID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeUpdateStateProgressID")
    = 0x00000003,
    MTRClusterOtaSoftwareUpdateRequestorAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOtaSoftwareUpdateRequestorAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster OTASoftwareUpdateRequestor attributes
    MTRAttributeIDTypeClusterOTASoftwareUpdateRequestorAttributeDefaultOTAProvidersID MTR_NEWLY_AVAILABLE = 0x00000000,
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
    MTRClusterLocalizationConfigurationAttributeActiveLocaleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeActiveLocaleID")
    = 0x00000000,
    MTRClusterLocalizationConfigurationAttributeSupportedLocalesID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeSupportedLocalesID")
    = 0x00000001,
    MTRClusterLocalizationConfigurationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterLocalizationConfigurationAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterLocalizationConfigurationAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterLocalizationConfigurationAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterLocalizationConfigurationAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLocalizationConfigurationAttributeClusterRevisionID")
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
    MTRClusterTimeFormatLocalizationAttributeHourFormatID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeHourFormatID")
    = 0x00000000,
    MTRClusterTimeFormatLocalizationAttributeActiveCalendarTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeActiveCalendarTypeID")
    = 0x00000001,
    MTRClusterTimeFormatLocalizationAttributeSupportedCalendarTypesID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeSupportedCalendarTypesID")
    = 0x00000002,
    MTRClusterTimeFormatLocalizationAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTimeFormatLocalizationAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTimeFormatLocalizationAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTimeFormatLocalizationAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTimeFormatLocalizationAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeFormatLocalizationAttributeClusterRevisionID")
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
    MTRClusterUnitLocalizationAttributeTemperatureUnitID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeTemperatureUnitID")
    = 0x00000000,
    MTRClusterUnitLocalizationAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterUnitLocalizationAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterUnitLocalizationAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterUnitLocalizationAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterUnitLocalizationAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitLocalizationAttributeClusterRevisionID")
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
    MTRClusterPowerSourceConfigurationAttributeSourcesID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeSourcesID")
    = 0x00000000,
    MTRClusterPowerSourceConfigurationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPowerSourceConfigurationAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPowerSourceConfigurationAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPowerSourceConfigurationAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPowerSourceConfigurationAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceConfigurationAttributeClusterRevisionID")
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
    MTRClusterPowerSourceAttributeStatusID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeStatusID")
    = 0x00000000,
    MTRClusterPowerSourceAttributeOrderID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeOrderID")
    = 0x00000001,
    MTRClusterPowerSourceAttributeDescriptionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeDescriptionID")
    = 0x00000002,
    MTRClusterPowerSourceAttributeWiredAssessedInputVoltageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputVoltageID")
    = 0x00000003,
    MTRClusterPowerSourceAttributeWiredAssessedInputFrequencyID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedInputFrequencyID")
    = 0x00000004,
    MTRClusterPowerSourceAttributeWiredCurrentTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredCurrentTypeID")
    = 0x00000005,
    MTRClusterPowerSourceAttributeWiredAssessedCurrentID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredAssessedCurrentID")
    = 0x00000006,
    MTRClusterPowerSourceAttributeWiredNominalVoltageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredNominalVoltageID")
    = 0x00000007,
    MTRClusterPowerSourceAttributeWiredMaximumCurrentID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredMaximumCurrentID")
    = 0x00000008,
    MTRClusterPowerSourceAttributeWiredPresentID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeWiredPresentID")
    = 0x00000009,
    MTRClusterPowerSourceAttributeActiveWiredFaultsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeActiveWiredFaultsID")
    = 0x0000000A,
    MTRClusterPowerSourceAttributeBatVoltageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatVoltageID")
    = 0x0000000B,
    MTRClusterPowerSourceAttributeBatPercentRemainingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatPercentRemainingID")
    = 0x0000000C,
    MTRClusterPowerSourceAttributeBatTimeRemainingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeRemainingID")
    = 0x0000000D,
    MTRClusterPowerSourceAttributeBatChargeLevelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeLevelID")
    = 0x0000000E,
    MTRClusterPowerSourceAttributeBatReplacementNeededID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementNeededID")
    = 0x0000000F,
    MTRClusterPowerSourceAttributeBatReplaceabilityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatReplaceabilityID")
    = 0x00000010,
    MTRClusterPowerSourceAttributeBatPresentID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatPresentID")
    = 0x00000011,
    MTRClusterPowerSourceAttributeActiveBatFaultsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatFaultsID")
    = 0x00000012,
    MTRClusterPowerSourceAttributeBatReplacementDescriptionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatReplacementDescriptionID")
    = 0x00000013,
    MTRClusterPowerSourceAttributeBatCommonDesignationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatCommonDesignationID")
    = 0x00000014,
    MTRClusterPowerSourceAttributeBatANSIDesignationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatANSIDesignationID")
    = 0x00000015,
    MTRClusterPowerSourceAttributeBatIECDesignationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatIECDesignationID")
    = 0x00000016,
    MTRClusterPowerSourceAttributeBatApprovedChemistryID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatApprovedChemistryID")
    = 0x00000017,
    MTRClusterPowerSourceAttributeBatCapacityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatCapacityID")
    = 0x00000018,
    MTRClusterPowerSourceAttributeBatQuantityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatQuantityID")
    = 0x00000019,
    MTRClusterPowerSourceAttributeBatChargeStateID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatChargeStateID")
    = 0x0000001A,
    MTRClusterPowerSourceAttributeBatTimeToFullChargeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatTimeToFullChargeID")
    = 0x0000001B,
    MTRClusterPowerSourceAttributeBatFunctionalWhileChargingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatFunctionalWhileChargingID")
    = 0x0000001C,
    MTRClusterPowerSourceAttributeBatChargingCurrentID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeBatChargingCurrentID")
    = 0x0000001D,
    MTRClusterPowerSourceAttributeActiveBatChargeFaultsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeActiveBatChargeFaultsID")
    = 0x0000001E,
    MTRClusterPowerSourceAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPowerSourceAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPowerSourceAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPowerSourceAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPowerSourceAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPowerSourceAttributeClusterRevisionID")
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
    MTRClusterGeneralCommissioningAttributeBreadcrumbID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeBreadcrumbID")
    = 0x00000000,
    MTRClusterGeneralCommissioningAttributeBasicCommissioningInfoID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeBasicCommissioningInfoID")
    = 0x00000001,
    MTRClusterGeneralCommissioningAttributeRegulatoryConfigID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeRegulatoryConfigID")
    = 0x00000002,
    MTRClusterGeneralCommissioningAttributeLocationCapabilityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeLocationCapabilityID")
    = 0x00000003,
    MTRClusterGeneralCommissioningAttributeSupportsConcurrentConnectionID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeSupportsConcurrentConnectionID")
    = 0x00000004,
    MTRClusterGeneralCommissioningAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGeneralCommissioningAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGeneralCommissioningAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGeneralCommissioningAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGeneralCommissioningAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralCommissioningAttributeClusterRevisionID")
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
    MTRClusterNetworkCommissioningAttributeMaxNetworksID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeMaxNetworksID")
    = 0x00000000,
    MTRClusterNetworkCommissioningAttributeNetworksID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeNetworksID")
    = 0x00000001,
    MTRClusterNetworkCommissioningAttributeScanMaxTimeSecondsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeScanMaxTimeSecondsID")
    = 0x00000002,
    MTRClusterNetworkCommissioningAttributeConnectMaxTimeSecondsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeConnectMaxTimeSecondsID")
    = 0x00000003,
    MTRClusterNetworkCommissioningAttributeInterfaceEnabledID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeInterfaceEnabledID")
    = 0x00000004,
    MTRClusterNetworkCommissioningAttributeLastNetworkingStatusID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkingStatusID")
    = 0x00000005,
    MTRClusterNetworkCommissioningAttributeLastNetworkIDID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastNetworkIDID")
    = 0x00000006,
    MTRClusterNetworkCommissioningAttributeLastConnectErrorValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeLastConnectErrorValueID")
    = 0x00000007,
    MTRClusterNetworkCommissioningAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterNetworkCommissioningAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterNetworkCommissioningAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterNetworkCommissioningAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterNetworkCommissioningAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterNetworkCommissioningAttributeClusterRevisionID")
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
    MTRClusterDiagnosticLogsAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterDiagnosticLogsAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterDiagnosticLogsAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterDiagnosticLogsAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterDiagnosticLogsAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDiagnosticLogsAttributeClusterRevisionID")
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
    MTRClusterGeneralDiagnosticsAttributeNetworkInterfacesID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeNetworkInterfacesID")
    = 0x00000000,
    MTRClusterGeneralDiagnosticsAttributeRebootCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeRebootCountID")
    = 0x00000001,
    MTRClusterGeneralDiagnosticsAttributeUpTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeUpTimeID")
    = 0x00000002,
    MTRClusterGeneralDiagnosticsAttributeTotalOperationalHoursID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTotalOperationalHoursID")
    = 0x00000003,
    MTRClusterGeneralDiagnosticsAttributeBootReasonsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeBootReasonID")
    = 0x00000004,
    MTRClusterGeneralDiagnosticsAttributeActiveHardwareFaultsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveHardwareFaultsID")
    = 0x00000005,
    MTRClusterGeneralDiagnosticsAttributeActiveRadioFaultsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveRadioFaultsID")
    = 0x00000006,
    MTRClusterGeneralDiagnosticsAttributeActiveNetworkFaultsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeActiveNetworkFaultsID")
    = 0x00000007,
    MTRClusterGeneralDiagnosticsAttributeTestEventTriggersEnabledID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTestEventTriggersEnabledID")
    = 0x00000008,
    MTRClusterGeneralDiagnosticsAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGeneralDiagnosticsAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGeneralDiagnosticsAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGeneralDiagnosticsAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGeneralDiagnosticsAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster GeneralDiagnostics attributes
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeNetworkInterfacesID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeRebootCountID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeUpTimeID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeTotalOperationalHoursID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterGeneralDiagnosticsAttributeBootReasonID MTR_NEWLY_AVAILABLE = 0x00000004,
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
    MTRClusterSoftwareDiagnosticsAttributeThreadMetricsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeThreadMetricsID")
    = 0x00000000,
    MTRClusterSoftwareDiagnosticsAttributeCurrentHeapFreeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapFreeID")
    = 0x00000001,
    MTRClusterSoftwareDiagnosticsAttributeCurrentHeapUsedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapUsedID")
    = 0x00000002,
    MTRClusterSoftwareDiagnosticsAttributeCurrentHeapHighWatermarkID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeCurrentHeapHighWatermarkID")
    = 0x00000003,
    MTRClusterSoftwareDiagnosticsAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterSoftwareDiagnosticsAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterSoftwareDiagnosticsAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterSoftwareDiagnosticsAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterSoftwareDiagnosticsAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSoftwareDiagnosticsAttributeClusterRevisionID")
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
    MTRClusterThreadNetworkDiagnosticsAttributeChannelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelID")
    = 0x00000000,
    MTRClusterThreadNetworkDiagnosticsAttributeRoutingRoleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRoutingRoleID")
    = 0x00000001,
    MTRClusterThreadNetworkDiagnosticsAttributeNetworkNameID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNetworkNameID")
    = 0x00000002,
    MTRClusterThreadNetworkDiagnosticsAttributePanIdID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePanIdID")
    = 0x00000003,
    MTRClusterThreadNetworkDiagnosticsAttributeExtendedPanIdID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeExtendedPanIdID")
    = 0x00000004,
    MTRClusterThreadNetworkDiagnosticsAttributeMeshLocalPrefixID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeMeshLocalPrefixID")
    = 0x00000005,
    MTRClusterThreadNetworkDiagnosticsAttributeOverrunCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOverrunCountID")
    = 0x00000006,
    MTRClusterThreadNetworkDiagnosticsAttributeNeighborTableListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNeighborTableID")
    = 0x00000007,
    MTRClusterThreadNetworkDiagnosticsAttributeRouteTableListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouteTableID")
    = 0x00000008,
    MTRClusterThreadNetworkDiagnosticsAttributePartitionIdID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdID")
    = 0x00000009,
    MTRClusterThreadNetworkDiagnosticsAttributeWeightingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeWeightingID")
    = 0x0000000A,
    MTRClusterThreadNetworkDiagnosticsAttributeDataVersionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDataVersionID")
    = 0x0000000B,
    MTRClusterThreadNetworkDiagnosticsAttributeStableDataVersionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeStableDataVersionID")
    = 0x0000000C,
    MTRClusterThreadNetworkDiagnosticsAttributeLeaderRouterIdID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRouterIdID")
    = 0x0000000D,
    MTRClusterThreadNetworkDiagnosticsAttributeDetachedRoleCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDetachedRoleCountID")
    = 0x0000000E,
    MTRClusterThreadNetworkDiagnosticsAttributeChildRoleCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChildRoleCountID")
    = 0x0000000F,
    MTRClusterThreadNetworkDiagnosticsAttributeRouterRoleCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouterRoleCountID")
    = 0x00000010,
    MTRClusterThreadNetworkDiagnosticsAttributeLeaderRoleCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeLeaderRoleCountID")
    = 0x00000011,
    MTRClusterThreadNetworkDiagnosticsAttributeAttachAttemptCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttachAttemptCountID")
    = 0x00000012,
    MTRClusterThreadNetworkDiagnosticsAttributePartitionIdChangeCountID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePartitionIdChangeCountID")
    = 0x00000013,
    MTRClusterThreadNetworkDiagnosticsAttributeBetterPartitionAttachAttemptCountID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeBetterPartitionAttachAttemptCountID")
    = 0x00000014,
    MTRClusterThreadNetworkDiagnosticsAttributeParentChangeCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeParentChangeCountID")
    = 0x00000015,
    MTRClusterThreadNetworkDiagnosticsAttributeTxTotalCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxTotalCountID")
    = 0x00000016,
    MTRClusterThreadNetworkDiagnosticsAttributeTxUnicastCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxUnicastCountID")
    = 0x00000017,
    MTRClusterThreadNetworkDiagnosticsAttributeTxBroadcastCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBroadcastCountID")
    = 0x00000018,
    MTRClusterThreadNetworkDiagnosticsAttributeTxAckRequestedCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckRequestedCountID")
    = 0x00000019,
    MTRClusterThreadNetworkDiagnosticsAttributeTxAckedCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxAckedCountID")
    = 0x0000001A,
    MTRClusterThreadNetworkDiagnosticsAttributeTxNoAckRequestedCountID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxNoAckRequestedCountID")
    = 0x0000001B,
    MTRClusterThreadNetworkDiagnosticsAttributeTxDataCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataCountID")
    = 0x0000001C,
    MTRClusterThreadNetworkDiagnosticsAttributeTxDataPollCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDataPollCountID")
    = 0x0000001D,
    MTRClusterThreadNetworkDiagnosticsAttributeTxBeaconCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconCountID")
    = 0x0000001E,
    MTRClusterThreadNetworkDiagnosticsAttributeTxBeaconRequestCountID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxBeaconRequestCountID")
    = 0x0000001F,
    MTRClusterThreadNetworkDiagnosticsAttributeTxOtherCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxOtherCountID")
    = 0x00000020,
    MTRClusterThreadNetworkDiagnosticsAttributeTxRetryCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxRetryCountID")
    = 0x00000021,
    MTRClusterThreadNetworkDiagnosticsAttributeTxDirectMaxRetryExpiryCountID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxDirectMaxRetryExpiryCountID")
    = 0x00000022,
    MTRClusterThreadNetworkDiagnosticsAttributeTxIndirectMaxRetryExpiryCountID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxIndirectMaxRetryExpiryCountID")
    = 0x00000023,
    MTRClusterThreadNetworkDiagnosticsAttributeTxErrCcaCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrCcaCountID")
    = 0x00000024,
    MTRClusterThreadNetworkDiagnosticsAttributeTxErrAbortCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrAbortCountID")
    = 0x00000025,
    MTRClusterThreadNetworkDiagnosticsAttributeTxErrBusyChannelCountID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeTxErrBusyChannelCountID")
    = 0x00000026,
    MTRClusterThreadNetworkDiagnosticsAttributeRxTotalCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxTotalCountID")
    = 0x00000027,
    MTRClusterThreadNetworkDiagnosticsAttributeRxUnicastCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxUnicastCountID")
    = 0x00000028,
    MTRClusterThreadNetworkDiagnosticsAttributeRxBroadcastCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBroadcastCountID")
    = 0x00000029,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDataCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataCountID")
    = 0x0000002A,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDataPollCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDataPollCountID")
    = 0x0000002B,
    MTRClusterThreadNetworkDiagnosticsAttributeRxBeaconCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconCountID")
    = 0x0000002C,
    MTRClusterThreadNetworkDiagnosticsAttributeRxBeaconRequestCountID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxBeaconRequestCountID")
    = 0x0000002D,
    MTRClusterThreadNetworkDiagnosticsAttributeRxOtherCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxOtherCountID")
    = 0x0000002E,
    MTRClusterThreadNetworkDiagnosticsAttributeRxAddressFilteredCountID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxAddressFilteredCountID")
    = 0x0000002F,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDestAddrFilteredCountID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDestAddrFilteredCountID")
    = 0x00000030,
    MTRClusterThreadNetworkDiagnosticsAttributeRxDuplicatedCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxDuplicatedCountID")
    = 0x00000031,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrNoFrameCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrNoFrameCountID")
    = 0x00000032,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrUnknownNeighborCountID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrUnknownNeighborCountID")
    = 0x00000033,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrInvalidSrcAddrCountID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrInvalidSrcAddrCountID")
    = 0x00000034,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrSecCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrSecCountID")
    = 0x00000035,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrFcsCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrFcsCountID")
    = 0x00000036,
    MTRClusterThreadNetworkDiagnosticsAttributeRxErrOtherCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRxErrOtherCountID")
    = 0x00000037,
    MTRClusterThreadNetworkDiagnosticsAttributeActiveTimestampID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveTimestampID")
    = 0x00000038,
    MTRClusterThreadNetworkDiagnosticsAttributePendingTimestampID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePendingTimestampID")
    = 0x00000039,
    MTRClusterThreadNetworkDiagnosticsAttributeDelayID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeDelayID")
    = 0x0000003A,
    MTRClusterThreadNetworkDiagnosticsAttributeSecurityPolicyID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeSecurityPolicyID")
    = 0x0000003B,
    MTRClusterThreadNetworkDiagnosticsAttributeChannelPage0MaskID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelPage0MaskID")
    = 0x0000003C,
    MTRClusterThreadNetworkDiagnosticsAttributeOperationalDatasetComponentsID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOperationalDatasetComponentsID")
    = 0x0000003D,
    MTRClusterThreadNetworkDiagnosticsAttributeActiveNetworkFaultsListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeActiveNetworkFaultsListID")
    = 0x0000003E,
    MTRClusterThreadNetworkDiagnosticsAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterThreadNetworkDiagnosticsAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterThreadNetworkDiagnosticsAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterThreadNetworkDiagnosticsAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterThreadNetworkDiagnosticsAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster ThreadNetworkDiagnostics attributes
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeChannelID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRoutingRoleID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNetworkNameID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributePanIdID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeExtendedPanIdID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeMeshLocalPrefixID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeOverrunCountID MTR_NEWLY_AVAILABLE = 0x00000006,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeNeighborTableID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterThreadNetworkDiagnosticsAttributeRouteTableID MTR_NEWLY_AVAILABLE = 0x00000008,
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
    MTRClusterWiFiNetworkDiagnosticsAttributeBssidID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBSSIDID")
    = 0x00000000,
    MTRClusterWiFiNetworkDiagnosticsAttributeSecurityTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeSecurityTypeID")
    = 0x00000001,
    MTRClusterWiFiNetworkDiagnosticsAttributeWiFiVersionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeWiFiVersionID")
    = 0x00000002,
    MTRClusterWiFiNetworkDiagnosticsAttributeChannelNumberID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeChannelNumberID")
    = 0x00000003,
    MTRClusterWiFiNetworkDiagnosticsAttributeRssiID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeRSSIID")
    = 0x00000004,
    MTRClusterWiFiNetworkDiagnosticsAttributeBeaconLostCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconLostCountID")
    = 0x00000005,
    MTRClusterWiFiNetworkDiagnosticsAttributeBeaconRxCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBeaconRxCountID")
    = 0x00000006,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketMulticastRxCountID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastRxCountID")
    = 0x00000007,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketMulticastTxCountID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketMulticastTxCountID")
    = 0x00000008,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketUnicastRxCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastRxCountID")
    = 0x00000009,
    MTRClusterWiFiNetworkDiagnosticsAttributePacketUnicastTxCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributePacketUnicastTxCountID")
    = 0x0000000A,
    MTRClusterWiFiNetworkDiagnosticsAttributeCurrentMaxRateID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeCurrentMaxRateID")
    = 0x0000000B,
    MTRClusterWiFiNetworkDiagnosticsAttributeOverrunCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeOverrunCountID")
    = 0x0000000C,
    MTRClusterWiFiNetworkDiagnosticsAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterWiFiNetworkDiagnosticsAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterWiFiNetworkDiagnosticsAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterWiFiNetworkDiagnosticsAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterWiFiNetworkDiagnosticsAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster WiFiNetworkDiagnostics attributes
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeBSSIDID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeSecurityTypeID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeWiFiVersionID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeChannelNumberID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterWiFiNetworkDiagnosticsAttributeRSSIID MTR_NEWLY_AVAILABLE = 0x00000004,
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
    MTRClusterEthernetNetworkDiagnosticsAttributePHYRateID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePHYRateID")
    = 0x00000000,
    MTRClusterEthernetNetworkDiagnosticsAttributeFullDuplexID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFullDuplexID")
    = 0x00000001,
    MTRClusterEthernetNetworkDiagnosticsAttributePacketRxCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketRxCountID")
    = 0x00000002,
    MTRClusterEthernetNetworkDiagnosticsAttributePacketTxCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributePacketTxCountID")
    = 0x00000003,
    MTRClusterEthernetNetworkDiagnosticsAttributeTxErrCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTxErrCountID")
    = 0x00000004,
    MTRClusterEthernetNetworkDiagnosticsAttributeCollisionCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCollisionCountID")
    = 0x00000005,
    MTRClusterEthernetNetworkDiagnosticsAttributeOverrunCountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeOverrunCountID")
    = 0x00000006,
    MTRClusterEthernetNetworkDiagnosticsAttributeCarrierDetectID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeCarrierDetectID")
    = 0x00000007,
    MTRClusterEthernetNetworkDiagnosticsAttributeTimeSinceResetID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeTimeSinceResetID")
    = 0x00000008,
    MTRClusterEthernetNetworkDiagnosticsAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterEthernetNetworkDiagnosticsAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterEthernetNetworkDiagnosticsAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterEthernetNetworkDiagnosticsAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterEthernetNetworkDiagnosticsAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterEthernetNetworkDiagnosticsAttributeClusterRevisionID")
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
    MTRClusterTimeSynchronizationAttributeUTCTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeUTCTimeID")
    = 0x00000000,
    MTRClusterTimeSynchronizationAttributeGranularityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeGranularityID")
    = 0x00000001,
    MTRClusterTimeSynchronizationAttributeTimeSourceID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeSourceID")
    = 0x00000002,
    MTRClusterTimeSynchronizationAttributeTrustedTimeNodeIdID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTrustedTimeNodeIdID")
    = 0x00000003,
    MTRClusterTimeSynchronizationAttributeDefaultNtpID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeDefaultNtpID")
    = 0x00000004,
    MTRClusterTimeSynchronizationAttributeTimeZoneID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneID")
    = 0x00000005,
    MTRClusterTimeSynchronizationAttributeDstOffsetID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeDstOffsetID")
    = 0x00000006,
    MTRClusterTimeSynchronizationAttributeLocalTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeLocalTimeID")
    = 0x00000007,
    MTRClusterTimeSynchronizationAttributeTimeZoneDatabaseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeTimeZoneDatabaseID")
    = 0x00000008,
    MTRClusterTimeSynchronizationAttributeNtpServerPortID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeNtpServerPortID")
    = 0x00000009,
    MTRClusterTimeSynchronizationAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTimeSynchronizationAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTimeSynchronizationAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTimeSynchronizationAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTimeSynchronizationAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTimeSynchronizationAttributeClusterRevisionID")
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
    MTRClusterBridgedDeviceBasicAttributeVendorNameID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeVendorNameID")
    = 0x00000001,
    MTRClusterBridgedDeviceBasicAttributeVendorIDID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeVendorIDID")
    = 0x00000002,
    MTRClusterBridgedDeviceBasicAttributeProductNameID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductNameID")
    = 0x00000003,
    MTRClusterBridgedDeviceBasicAttributeNodeLabelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeNodeLabelID")
    = 0x00000005,
    MTRClusterBridgedDeviceBasicAttributeHardwareVersionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeHardwareVersionID")
    = 0x00000007,
    MTRClusterBridgedDeviceBasicAttributeHardwareVersionStringID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeHardwareVersionStringID")
    = 0x00000008,
    MTRClusterBridgedDeviceBasicAttributeSoftwareVersionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSoftwareVersionID")
    = 0x00000009,
    MTRClusterBridgedDeviceBasicAttributeSoftwareVersionStringID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSoftwareVersionStringID")
    = 0x0000000A,
    MTRClusterBridgedDeviceBasicAttributeManufacturingDateID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeManufacturingDateID")
    = 0x0000000B,
    MTRClusterBridgedDeviceBasicAttributePartNumberID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributePartNumberID")
    = 0x0000000C,
    MTRClusterBridgedDeviceBasicAttributeProductURLID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductURLID")
    = 0x0000000D,
    MTRClusterBridgedDeviceBasicAttributeProductLabelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductLabelID")
    = 0x0000000E,
    MTRClusterBridgedDeviceBasicAttributeSerialNumberID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSerialNumberID")
    = 0x0000000F,
    MTRClusterBridgedDeviceBasicAttributeReachableID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeReachableID")
    = 0x00000011,
    MTRClusterBridgedDeviceBasicAttributeUniqueIDID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeUniqueIDID")
    = 0x00000012,
    MTRClusterBridgedDeviceBasicAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBridgedDeviceBasicAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBridgedDeviceBasicAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBridgedDeviceBasicAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBridgedDeviceBasicAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeClusterRevisionID")
    = MTRClusterGlobalAttributeClusterRevisionID,

    // Cluster BridgedDeviceBasicInformation attributes
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeVendorNameID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeVendorIDID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductNameID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeNodeLabelID MTR_NEWLY_AVAILABLE = 0x00000005,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeHardwareVersionID MTR_NEWLY_AVAILABLE = 0x00000007,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeHardwareVersionStringID MTR_NEWLY_AVAILABLE = 0x00000008,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSoftwareVersionID MTR_NEWLY_AVAILABLE = 0x00000009,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSoftwareVersionStringID MTR_NEWLY_AVAILABLE = 0x0000000A,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeManufacturingDateID MTR_NEWLY_AVAILABLE = 0x0000000B,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributePartNumberID MTR_NEWLY_AVAILABLE = 0x0000000C,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductURLID MTR_NEWLY_AVAILABLE = 0x0000000D,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeProductLabelID MTR_NEWLY_AVAILABLE = 0x0000000E,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeSerialNumberID MTR_NEWLY_AVAILABLE = 0x0000000F,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeReachableID MTR_NEWLY_AVAILABLE = 0x00000011,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeUniqueIDID MTR_NEWLY_AVAILABLE = 0x00000012,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeGeneratedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeGeneratedCommandListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeAcceptedCommandListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAcceptedCommandListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeAttributeListID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeAttributeListID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeFeatureMapID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeFeatureMapID,
    MTRAttributeIDTypeClusterBridgedDeviceBasicInformationAttributeClusterRevisionID MTR_NEWLY_AVAILABLE
    = MTRAttributeIDTypeGlobalAttributeClusterRevisionID,

    // Cluster Switch deprecated attribute names
    MTRClusterSwitchAttributeNumberOfPositionsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeNumberOfPositionsID")
    = 0x00000000,
    MTRClusterSwitchAttributeCurrentPositionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeCurrentPositionID")
    = 0x00000001,
    MTRClusterSwitchAttributeMultiPressMaxID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeMultiPressMaxID")
    = 0x00000002,
    MTRClusterSwitchAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterSwitchAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterSwitchAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterSwitchAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterSwitchAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterSwitchAttributeClusterRevisionID")
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
    MTRClusterAdministratorCommissioningAttributeWindowStatusID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeWindowStatusID")
    = 0x00000000,
    MTRClusterAdministratorCommissioningAttributeAdminFabricIndexID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminFabricIndexID")
    = 0x00000001,
    MTRClusterAdministratorCommissioningAttributeAdminVendorIdID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAdminVendorIdID")
    = 0x00000002,
    MTRClusterAdministratorCommissioningAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAdministratorCommissioningAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAdministratorCommissioningAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAdministratorCommissioningAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAdministratorCommissioningAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAdministratorCommissioningAttributeClusterRevisionID")
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
    MTRClusterOperationalCredentialsAttributeNOCsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeNOCsID")
    = 0x00000000,
    MTRClusterOperationalCredentialsAttributeFabricsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeFabricsID")
    = 0x00000001,
    MTRClusterOperationalCredentialsAttributeSupportedFabricsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeSupportedFabricsID")
    = 0x00000002,
    MTRClusterOperationalCredentialsAttributeCommissionedFabricsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeCommissionedFabricsID")
    = 0x00000003,
    MTRClusterOperationalCredentialsAttributeTrustedRootCertificatesID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeTrustedRootCertificatesID")
    = 0x00000004,
    MTRClusterOperationalCredentialsAttributeCurrentFabricIndexID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeCurrentFabricIndexID")
    = 0x00000005,
    MTRClusterOperationalCredentialsAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOperationalCredentialsAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOperationalCredentialsAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOperationalCredentialsAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOperationalCredentialsAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOperationalCredentialsAttributeClusterRevisionID")
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
    MTRClusterGroupKeyManagementAttributeGroupKeyMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupKeyMapID")
    = 0x00000000,
    MTRClusterGroupKeyManagementAttributeGroupTableID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeGroupTableID")
    = 0x00000001,
    MTRClusterGroupKeyManagementAttributeMaxGroupsPerFabricID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupsPerFabricID")
    = 0x00000002,
    MTRClusterGroupKeyManagementAttributeMaxGroupKeysPerFabricID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeMaxGroupKeysPerFabricID")
    = 0x00000003,
    MTRClusterGroupKeyManagementAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterGroupKeyManagementAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterGroupKeyManagementAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterGroupKeyManagementAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterGroupKeyManagementAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterGroupKeyManagementAttributeClusterRevisionID")
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
    MTRClusterFixedLabelAttributeLabelListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFixedLabelAttributeLabelListID")
    = 0x00000000,
    MTRClusterFixedLabelAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFixedLabelAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterFixedLabelAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFixedLabelAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterFixedLabelAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFixedLabelAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterFixedLabelAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFixedLabelAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterFixedLabelAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFixedLabelAttributeClusterRevisionID")
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
    MTRClusterUserLabelAttributeLabelListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUserLabelAttributeLabelListID")
    = 0x00000000,
    MTRClusterUserLabelAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUserLabelAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterUserLabelAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUserLabelAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterUserLabelAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUserLabelAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterUserLabelAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUserLabelAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterUserLabelAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUserLabelAttributeClusterRevisionID")
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
    MTRClusterProxyConfigurationAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyConfigurationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterProxyConfigurationAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyConfigurationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterProxyConfigurationAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyConfigurationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterProxyConfigurationAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyConfigurationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterProxyConfigurationAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyConfigurationAttributeClusterRevisionID")
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
    MTRClusterProxyDiscoveryAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyDiscoveryAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterProxyDiscoveryAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyDiscoveryAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterProxyDiscoveryAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyDiscoveryAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterProxyDiscoveryAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyDiscoveryAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterProxyDiscoveryAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyDiscoveryAttributeClusterRevisionID")
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
    MTRClusterProxyValidAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyValidAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterProxyValidAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyValidAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterProxyValidAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyValidAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterProxyValidAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyValidAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterProxyValidAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterProxyValidAttributeClusterRevisionID")
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
    MTRClusterBooleanStateAttributeStateValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBooleanStateAttributeStateValueID")
    = 0x00000000,
    MTRClusterBooleanStateAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBooleanStateAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBooleanStateAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBooleanStateAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBooleanStateAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBooleanStateAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBooleanStateAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBooleanStateAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBooleanStateAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBooleanStateAttributeClusterRevisionID")
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
    MTRClusterModeSelectAttributeDescriptionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeDescriptionID")
    = 0x00000000,
    MTRClusterModeSelectAttributeStandardNamespaceID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeStandardNamespaceID")
    = 0x00000001,
    MTRClusterModeSelectAttributeSupportedModesID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeSupportedModesID")
    = 0x00000002,
    MTRClusterModeSelectAttributeCurrentModeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeCurrentModeID")
    = 0x00000003,
    MTRClusterModeSelectAttributeStartUpModeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeStartUpModeID")
    = 0x00000004,
    MTRClusterModeSelectAttributeOnModeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeOnModeID")
    = 0x00000005,
    MTRClusterModeSelectAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterModeSelectAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterModeSelectAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterModeSelectAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterModeSelectAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterModeSelectAttributeClusterRevisionID")
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
    MTRClusterDoorLockAttributeLockStateID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLockStateID")
    = 0x00000000,
    MTRClusterDoorLockAttributeLockTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLockTypeID")
    = 0x00000001,
    MTRClusterDoorLockAttributeActuatorEnabledID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeActuatorEnabledID")
    = 0x00000002,
    MTRClusterDoorLockAttributeDoorStateID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeDoorStateID")
    = 0x00000003,
    MTRClusterDoorLockAttributeDoorOpenEventsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeDoorOpenEventsID")
    = 0x00000004,
    MTRClusterDoorLockAttributeDoorClosedEventsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeDoorClosedEventsID")
    = 0x00000005,
    MTRClusterDoorLockAttributeOpenPeriodID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeOpenPeriodID")
    = 0x00000006,
    MTRClusterDoorLockAttributeNumberOfTotalUsersSupportedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfTotalUsersSupportedID")
    = 0x00000011,
    MTRClusterDoorLockAttributeNumberOfPINUsersSupportedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfPINUsersSupportedID")
    = 0x00000012,
    MTRClusterDoorLockAttributeNumberOfRFIDUsersSupportedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfRFIDUsersSupportedID")
    = 0x00000013,
    MTRClusterDoorLockAttributeNumberOfWeekDaySchedulesSupportedPerUserID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfWeekDaySchedulesSupportedPerUserID")
    = 0x00000014,
    MTRClusterDoorLockAttributeNumberOfYearDaySchedulesSupportedPerUserID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfYearDaySchedulesSupportedPerUserID")
    = 0x00000015,
    MTRClusterDoorLockAttributeNumberOfHolidaySchedulesSupportedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfHolidaySchedulesSupportedID")
    = 0x00000016,
    MTRClusterDoorLockAttributeMaxPINCodeLengthID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeMaxPINCodeLengthID")
    = 0x00000017,
    MTRClusterDoorLockAttributeMinPINCodeLengthID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeMinPINCodeLengthID")
    = 0x00000018,
    MTRClusterDoorLockAttributeMaxRFIDCodeLengthID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeMaxRFIDCodeLengthID")
    = 0x00000019,
    MTRClusterDoorLockAttributeMinRFIDCodeLengthID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeMinRFIDCodeLengthID")
    = 0x0000001A,
    MTRClusterDoorLockAttributeCredentialRulesSupportID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeCredentialRulesSupportID")
    = 0x0000001B,
    MTRClusterDoorLockAttributeNumberOfCredentialsSupportedPerUserID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeNumberOfCredentialsSupportedPerUserID")
    = 0x0000001C,
    MTRClusterDoorLockAttributeLanguageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLanguageID")
    = 0x00000021,
    MTRClusterDoorLockAttributeLEDSettingsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLEDSettingsID")
    = 0x00000022,
    MTRClusterDoorLockAttributeAutoRelockTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeAutoRelockTimeID")
    = 0x00000023,
    MTRClusterDoorLockAttributeSoundVolumeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeSoundVolumeID")
    = 0x00000024,
    MTRClusterDoorLockAttributeOperatingModeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeOperatingModeID")
    = 0x00000025,
    MTRClusterDoorLockAttributeSupportedOperatingModesID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeSupportedOperatingModesID")
    = 0x00000026,
    MTRClusterDoorLockAttributeDefaultConfigurationRegisterID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeDefaultConfigurationRegisterID")
    = 0x00000027,
    MTRClusterDoorLockAttributeEnableLocalProgrammingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeEnableLocalProgrammingID")
    = 0x00000028,
    MTRClusterDoorLockAttributeEnableOneTouchLockingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeEnableOneTouchLockingID")
    = 0x00000029,
    MTRClusterDoorLockAttributeEnableInsideStatusLEDID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeEnableInsideStatusLEDID")
    = 0x0000002A,
    MTRClusterDoorLockAttributeEnablePrivacyModeButtonID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeEnablePrivacyModeButtonID")
    = 0x0000002B,
    MTRClusterDoorLockAttributeLocalProgrammingFeaturesID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeLocalProgrammingFeaturesID")
    = 0x0000002C,
    MTRClusterDoorLockAttributeWrongCodeEntryLimitID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeWrongCodeEntryLimitID")
    = 0x00000030,
    MTRClusterDoorLockAttributeUserCodeTemporaryDisableTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeUserCodeTemporaryDisableTimeID")
    = 0x00000031,
    MTRClusterDoorLockAttributeSendPINOverTheAirID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeSendPINOverTheAirID")
    = 0x00000032,
    MTRClusterDoorLockAttributeRequirePINforRemoteOperationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeRequirePINforRemoteOperationID")
    = 0x00000033,
    MTRClusterDoorLockAttributeExpiringUserTimeoutID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeExpiringUserTimeoutID")
    = 0x00000035,
    MTRClusterDoorLockAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterDoorLockAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterDoorLockAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterDoorLockAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterDoorLockAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterDoorLockAttributeClusterRevisionID")
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
    MTRClusterWindowCoveringAttributeTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeTypeID")
    = 0x00000000,
    MTRClusterWindowCoveringAttributePhysicalClosedLimitLiftID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitLiftID")
    = 0x00000001,
    MTRClusterWindowCoveringAttributePhysicalClosedLimitTiltID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributePhysicalClosedLimitTiltID")
    = 0x00000002,
    MTRClusterWindowCoveringAttributeCurrentPositionLiftID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftID")
    = 0x00000003,
    MTRClusterWindowCoveringAttributeCurrentPositionTiltID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltID")
    = 0x00000004,
    MTRClusterWindowCoveringAttributeNumberOfActuationsLiftID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsLiftID")
    = 0x00000005,
    MTRClusterWindowCoveringAttributeNumberOfActuationsTiltID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeNumberOfActuationsTiltID")
    = 0x00000006,
    MTRClusterWindowCoveringAttributeConfigStatusID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeConfigStatusID")
    = 0x00000007,
    MTRClusterWindowCoveringAttributeCurrentPositionLiftPercentageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercentageID")
    = 0x00000008,
    MTRClusterWindowCoveringAttributeCurrentPositionTiltPercentageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercentageID")
    = 0x00000009,
    MTRClusterWindowCoveringAttributeOperationalStatusID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeOperationalStatusID")
    = 0x0000000A,
    MTRClusterWindowCoveringAttributeTargetPositionLiftPercent100thsID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionLiftPercent100thsID")
    = 0x0000000B,
    MTRClusterWindowCoveringAttributeTargetPositionTiltPercent100thsID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeTargetPositionTiltPercent100thsID")
    = 0x0000000C,
    MTRClusterWindowCoveringAttributeEndProductTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeEndProductTypeID")
    = 0x0000000D,
    MTRClusterWindowCoveringAttributeCurrentPositionLiftPercent100thsID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionLiftPercent100thsID")
    = 0x0000000E,
    MTRClusterWindowCoveringAttributeCurrentPositionTiltPercent100thsID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeCurrentPositionTiltPercent100thsID")
    = 0x0000000F,
    MTRClusterWindowCoveringAttributeInstalledOpenLimitLiftID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitLiftID")
    = 0x00000010,
    MTRClusterWindowCoveringAttributeInstalledClosedLimitLiftID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitLiftID")
    = 0x00000011,
    MTRClusterWindowCoveringAttributeInstalledOpenLimitTiltID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledOpenLimitTiltID")
    = 0x00000012,
    MTRClusterWindowCoveringAttributeInstalledClosedLimitTiltID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeInstalledClosedLimitTiltID")
    = 0x00000013,
    MTRClusterWindowCoveringAttributeModeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeModeID")
    = 0x00000017,
    MTRClusterWindowCoveringAttributeSafetyStatusID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeSafetyStatusID")
    = 0x0000001A,
    MTRClusterWindowCoveringAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterWindowCoveringAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterWindowCoveringAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterWindowCoveringAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterWindowCoveringAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWindowCoveringAttributeClusterRevisionID")
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
    MTRClusterBarrierControlAttributeBarrierMovingStateID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierMovingStateID")
    = 0x00000001,
    MTRClusterBarrierControlAttributeBarrierSafetyStatusID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierSafetyStatusID")
    = 0x00000002,
    MTRClusterBarrierControlAttributeBarrierCapabilitiesID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCapabilitiesID")
    = 0x00000003,
    MTRClusterBarrierControlAttributeBarrierOpenEventsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenEventsID")
    = 0x00000004,
    MTRClusterBarrierControlAttributeBarrierCloseEventsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCloseEventsID")
    = 0x00000005,
    MTRClusterBarrierControlAttributeBarrierCommandOpenEventsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandOpenEventsID")
    = 0x00000006,
    MTRClusterBarrierControlAttributeBarrierCommandCloseEventsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierCommandCloseEventsID")
    = 0x00000007,
    MTRClusterBarrierControlAttributeBarrierOpenPeriodID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierOpenPeriodID")
    = 0x00000008,
    MTRClusterBarrierControlAttributeBarrierClosePeriodID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierClosePeriodID")
    = 0x00000009,
    MTRClusterBarrierControlAttributeBarrierPositionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeBarrierPositionID")
    = 0x0000000A,
    MTRClusterBarrierControlAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBarrierControlAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBarrierControlAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBarrierControlAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBarrierControlAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBarrierControlAttributeClusterRevisionID")
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
    MTRClusterPumpConfigurationAndControlAttributeMaxPressureID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxPressureID")
    = 0x00000000,
    MTRClusterPumpConfigurationAndControlAttributeMaxSpeedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxSpeedID")
    = 0x00000001,
    MTRClusterPumpConfigurationAndControlAttributeMaxFlowID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxFlowID")
    = 0x00000002,
    MTRClusterPumpConfigurationAndControlAttributeMinConstPressureID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstPressureID")
    = 0x00000003,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstPressureID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstPressureID")
    = 0x00000004,
    MTRClusterPumpConfigurationAndControlAttributeMinCompPressureID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinCompPressureID")
    = 0x00000005,
    MTRClusterPumpConfigurationAndControlAttributeMaxCompPressureID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxCompPressureID")
    = 0x00000006,
    MTRClusterPumpConfigurationAndControlAttributeMinConstSpeedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstSpeedID")
    = 0x00000007,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstSpeedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstSpeedID")
    = 0x00000008,
    MTRClusterPumpConfigurationAndControlAttributeMinConstFlowID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstFlowID")
    = 0x00000009,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstFlowID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstFlowID")
    = 0x0000000A,
    MTRClusterPumpConfigurationAndControlAttributeMinConstTempID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMinConstTempID")
    = 0x0000000B,
    MTRClusterPumpConfigurationAndControlAttributeMaxConstTempID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeMaxConstTempID")
    = 0x0000000C,
    MTRClusterPumpConfigurationAndControlAttributePumpStatusID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePumpStatusID")
    = 0x00000010,
    MTRClusterPumpConfigurationAndControlAttributeEffectiveOperationModeID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveOperationModeID")
    = 0x00000011,
    MTRClusterPumpConfigurationAndControlAttributeEffectiveControlModeID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeEffectiveControlModeID")
    = 0x00000012,
    MTRClusterPumpConfigurationAndControlAttributeCapacityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeCapacityID")
    = 0x00000013,
    MTRClusterPumpConfigurationAndControlAttributeSpeedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeSpeedID")
    = 0x00000014,
    MTRClusterPumpConfigurationAndControlAttributeLifetimeRunningHoursID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeRunningHoursID")
    = 0x00000015,
    MTRClusterPumpConfigurationAndControlAttributePowerID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributePowerID")
    = 0x00000016,
    MTRClusterPumpConfigurationAndControlAttributeLifetimeEnergyConsumedID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeLifetimeEnergyConsumedID")
    = 0x00000017,
    MTRClusterPumpConfigurationAndControlAttributeOperationModeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeOperationModeID")
    = 0x00000020,
    MTRClusterPumpConfigurationAndControlAttributeControlModeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeControlModeID")
    = 0x00000021,
    MTRClusterPumpConfigurationAndControlAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPumpConfigurationAndControlAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPumpConfigurationAndControlAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPumpConfigurationAndControlAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPumpConfigurationAndControlAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPumpConfigurationAndControlAttributeClusterRevisionID")
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
    MTRClusterThermostatAttributeLocalTemperatureID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureID")
    = 0x00000000,
    MTRClusterThermostatAttributeOutdoorTemperatureID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeOutdoorTemperatureID")
    = 0x00000001,
    MTRClusterThermostatAttributeOccupancyID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeOccupancyID")
    = 0x00000002,
    MTRClusterThermostatAttributeAbsMinHeatSetpointLimitID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMinHeatSetpointLimitID")
    = 0x00000003,
    MTRClusterThermostatAttributeAbsMaxHeatSetpointLimitID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMaxHeatSetpointLimitID")
    = 0x00000004,
    MTRClusterThermostatAttributeAbsMinCoolSetpointLimitID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMinCoolSetpointLimitID")
    = 0x00000005,
    MTRClusterThermostatAttributeAbsMaxCoolSetpointLimitID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeAbsMaxCoolSetpointLimitID")
    = 0x00000006,
    MTRClusterThermostatAttributePICoolingDemandID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributePICoolingDemandID")
    = 0x00000007,
    MTRClusterThermostatAttributePIHeatingDemandID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributePIHeatingDemandID")
    = 0x00000008,
    MTRClusterThermostatAttributeHVACSystemTypeConfigurationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeHVACSystemTypeConfigurationID")
    = 0x00000009,
    MTRClusterThermostatAttributeLocalTemperatureCalibrationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeLocalTemperatureCalibrationID")
    = 0x00000010,
    MTRClusterThermostatAttributeOccupiedCoolingSetpointID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedCoolingSetpointID")
    = 0x00000011,
    MTRClusterThermostatAttributeOccupiedHeatingSetpointID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedHeatingSetpointID")
    = 0x00000012,
    MTRClusterThermostatAttributeUnoccupiedCoolingSetpointID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedCoolingSetpointID")
    = 0x00000013,
    MTRClusterThermostatAttributeUnoccupiedHeatingSetpointID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedHeatingSetpointID")
    = 0x00000014,
    MTRClusterThermostatAttributeMinHeatSetpointLimitID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeMinHeatSetpointLimitID")
    = 0x00000015,
    MTRClusterThermostatAttributeMaxHeatSetpointLimitID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeMaxHeatSetpointLimitID")
    = 0x00000016,
    MTRClusterThermostatAttributeMinCoolSetpointLimitID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeMinCoolSetpointLimitID")
    = 0x00000017,
    MTRClusterThermostatAttributeMaxCoolSetpointLimitID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeMaxCoolSetpointLimitID")
    = 0x00000018,
    MTRClusterThermostatAttributeMinSetpointDeadBandID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeMinSetpointDeadBandID")
    = 0x00000019,
    MTRClusterThermostatAttributeRemoteSensingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeRemoteSensingID")
    = 0x0000001A,
    MTRClusterThermostatAttributeControlSequenceOfOperationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeControlSequenceOfOperationID")
    = 0x0000001B,
    MTRClusterThermostatAttributeSystemModeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeSystemModeID")
    = 0x0000001C,
    MTRClusterThermostatAttributeThermostatRunningModeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningModeID")
    = 0x0000001E,
    MTRClusterThermostatAttributeStartOfWeekID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeStartOfWeekID")
    = 0x00000020,
    MTRClusterThermostatAttributeNumberOfWeeklyTransitionsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeNumberOfWeeklyTransitionsID")
    = 0x00000021,
    MTRClusterThermostatAttributeNumberOfDailyTransitionsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeNumberOfDailyTransitionsID")
    = 0x00000022,
    MTRClusterThermostatAttributeTemperatureSetpointHoldID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldID")
    = 0x00000023,
    MTRClusterThermostatAttributeTemperatureSetpointHoldDurationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeTemperatureSetpointHoldDurationID")
    = 0x00000024,
    MTRClusterThermostatAttributeThermostatProgrammingOperationModeID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeThermostatProgrammingOperationModeID")
    = 0x00000025,
    MTRClusterThermostatAttributeThermostatRunningStateID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeThermostatRunningStateID")
    = 0x00000029,
    MTRClusterThermostatAttributeSetpointChangeSourceID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceID")
    = 0x00000030,
    MTRClusterThermostatAttributeSetpointChangeAmountID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeAmountID")
    = 0x00000031,
    MTRClusterThermostatAttributeSetpointChangeSourceTimestampID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeSetpointChangeSourceTimestampID")
    = 0x00000032,
    MTRClusterThermostatAttributeOccupiedSetbackID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackID")
    = 0x00000034,
    MTRClusterThermostatAttributeOccupiedSetbackMinID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMinID")
    = 0x00000035,
    MTRClusterThermostatAttributeOccupiedSetbackMaxID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeOccupiedSetbackMaxID")
    = 0x00000036,
    MTRClusterThermostatAttributeUnoccupiedSetbackID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackID")
    = 0x00000037,
    MTRClusterThermostatAttributeUnoccupiedSetbackMinID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMinID")
    = 0x00000038,
    MTRClusterThermostatAttributeUnoccupiedSetbackMaxID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeUnoccupiedSetbackMaxID")
    = 0x00000039,
    MTRClusterThermostatAttributeEmergencyHeatDeltaID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeEmergencyHeatDeltaID")
    = 0x0000003A,
    MTRClusterThermostatAttributeACTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACTypeID")
    = 0x00000040,
    MTRClusterThermostatAttributeACCapacityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACCapacityID")
    = 0x00000041,
    MTRClusterThermostatAttributeACRefrigerantTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACRefrigerantTypeID")
    = 0x00000042,
    MTRClusterThermostatAttributeACCompressorTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACCompressorTypeID")
    = 0x00000043,
    MTRClusterThermostatAttributeACErrorCodeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACErrorCodeID")
    = 0x00000044,
    MTRClusterThermostatAttributeACLouverPositionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACLouverPositionID")
    = 0x00000045,
    MTRClusterThermostatAttributeACCoilTemperatureID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACCoilTemperatureID")
    = 0x00000046,
    MTRClusterThermostatAttributeACCapacityformatID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeACCapacityformatID")
    = 0x00000047,
    MTRClusterThermostatAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterThermostatAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterThermostatAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterThermostatAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterThermostatAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatAttributeClusterRevisionID")
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
    MTRClusterFanControlAttributeFanModeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeFanModeID")
    = 0x00000000,
    MTRClusterFanControlAttributeFanModeSequenceID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeFanModeSequenceID")
    = 0x00000001,
    MTRClusterFanControlAttributePercentSettingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributePercentSettingID")
    = 0x00000002,
    MTRClusterFanControlAttributePercentCurrentID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributePercentCurrentID")
    = 0x00000003,
    MTRClusterFanControlAttributeSpeedMaxID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeSpeedMaxID")
    = 0x00000004,
    MTRClusterFanControlAttributeSpeedSettingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeSpeedSettingID")
    = 0x00000005,
    MTRClusterFanControlAttributeSpeedCurrentID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeSpeedCurrentID")
    = 0x00000006,
    MTRClusterFanControlAttributeRockSupportID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeRockSupportID")
    = 0x00000007,
    MTRClusterFanControlAttributeRockSettingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeRockSettingID")
    = 0x00000008,
    MTRClusterFanControlAttributeWindSupportID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeWindSupportID")
    = 0x00000009,
    MTRClusterFanControlAttributeWindSettingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeWindSettingID")
    = 0x0000000A,
    MTRClusterFanControlAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterFanControlAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterFanControlAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterFanControlAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterFanControlAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFanControlAttributeClusterRevisionID")
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
    MTRClusterThermostatUserInterfaceConfigurationAttributeTemperatureDisplayModeID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeTemperatureDisplayModeID")
    = 0x00000000,
    MTRClusterThermostatUserInterfaceConfigurationAttributeKeypadLockoutID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeKeypadLockoutID")
    = 0x00000001,
    MTRClusterThermostatUserInterfaceConfigurationAttributeScheduleProgrammingVisibilityID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeScheduleProgrammingVisibilityID")
    = 0x00000002,
    MTRClusterThermostatUserInterfaceConfigurationAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeAttributeListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeFeatureMapID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterThermostatUserInterfaceConfigurationAttributeClusterRevisionID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterThermostatUserInterfaceConfigurationAttributeClusterRevisionID")
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
    MTRClusterColorControlAttributeCurrentHueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentHueID")
    = 0x00000000,
    MTRClusterColorControlAttributeCurrentSaturationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentSaturationID")
    = 0x00000001,
    MTRClusterColorControlAttributeRemainingTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeRemainingTimeID")
    = 0x00000002,
    MTRClusterColorControlAttributeCurrentXID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentXID")
    = 0x00000003,
    MTRClusterColorControlAttributeCurrentYID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeCurrentYID")
    = 0x00000004,
    MTRClusterColorControlAttributeDriftCompensationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeDriftCompensationID")
    = 0x00000005,
    MTRClusterColorControlAttributeCompensationTextID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeCompensationTextID")
    = 0x00000006,
    MTRClusterColorControlAttributeColorTemperatureMiredsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorTemperatureMiredsID")
    = 0x00000007,
    MTRClusterColorControlAttributeColorModeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorModeID")
    = 0x00000008,
    MTRClusterColorControlAttributeOptionsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeOptionsID")
    = 0x0000000F,
    MTRClusterColorControlAttributeNumberOfPrimariesID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeNumberOfPrimariesID")
    = 0x00000010,
    MTRClusterColorControlAttributePrimary1XID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary1XID")
    = 0x00000011,
    MTRClusterColorControlAttributePrimary1YID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary1YID")
    = 0x00000012,
    MTRClusterColorControlAttributePrimary1IntensityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary1IntensityID")
    = 0x00000013,
    MTRClusterColorControlAttributePrimary2XID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary2XID")
    = 0x00000015,
    MTRClusterColorControlAttributePrimary2YID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary2YID")
    = 0x00000016,
    MTRClusterColorControlAttributePrimary2IntensityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary2IntensityID")
    = 0x00000017,
    MTRClusterColorControlAttributePrimary3XID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary3XID")
    = 0x00000019,
    MTRClusterColorControlAttributePrimary3YID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary3YID")
    = 0x0000001A,
    MTRClusterColorControlAttributePrimary3IntensityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary3IntensityID")
    = 0x0000001B,
    MTRClusterColorControlAttributePrimary4XID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary4XID")
    = 0x00000020,
    MTRClusterColorControlAttributePrimary4YID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary4YID")
    = 0x00000021,
    MTRClusterColorControlAttributePrimary4IntensityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary4IntensityID")
    = 0x00000022,
    MTRClusterColorControlAttributePrimary5XID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary5XID")
    = 0x00000024,
    MTRClusterColorControlAttributePrimary5YID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary5YID")
    = 0x00000025,
    MTRClusterColorControlAttributePrimary5IntensityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary5IntensityID")
    = 0x00000026,
    MTRClusterColorControlAttributePrimary6XID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary6XID")
    = 0x00000028,
    MTRClusterColorControlAttributePrimary6YID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary6YID")
    = 0x00000029,
    MTRClusterColorControlAttributePrimary6IntensityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributePrimary6IntensityID")
    = 0x0000002A,
    MTRClusterColorControlAttributeWhitePointXID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeWhitePointXID")
    = 0x00000030,
    MTRClusterColorControlAttributeWhitePointYID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeWhitePointYID")
    = 0x00000031,
    MTRClusterColorControlAttributeColorPointRXID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointRXID")
    = 0x00000032,
    MTRClusterColorControlAttributeColorPointRYID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointRYID")
    = 0x00000033,
    MTRClusterColorControlAttributeColorPointRIntensityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointRIntensityID")
    = 0x00000034,
    MTRClusterColorControlAttributeColorPointGXID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointGXID")
    = 0x00000036,
    MTRClusterColorControlAttributeColorPointGYID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointGYID")
    = 0x00000037,
    MTRClusterColorControlAttributeColorPointGIntensityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointGIntensityID")
    = 0x00000038,
    MTRClusterColorControlAttributeColorPointBXID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointBXID")
    = 0x0000003A,
    MTRClusterColorControlAttributeColorPointBYID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointBYID")
    = 0x0000003B,
    MTRClusterColorControlAttributeColorPointBIntensityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorPointBIntensityID")
    = 0x0000003C,
    MTRClusterColorControlAttributeEnhancedCurrentHueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeEnhancedCurrentHueID")
    = 0x00004000,
    MTRClusterColorControlAttributeEnhancedColorModeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeEnhancedColorModeID")
    = 0x00004001,
    MTRClusterColorControlAttributeColorLoopActiveID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopActiveID")
    = 0x00004002,
    MTRClusterColorControlAttributeColorLoopDirectionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopDirectionID")
    = 0x00004003,
    MTRClusterColorControlAttributeColorLoopTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopTimeID")
    = 0x00004004,
    MTRClusterColorControlAttributeColorLoopStartEnhancedHueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopStartEnhancedHueID")
    = 0x00004005,
    MTRClusterColorControlAttributeColorLoopStoredEnhancedHueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorLoopStoredEnhancedHueID")
    = 0x00004006,
    MTRClusterColorControlAttributeColorCapabilitiesID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorCapabilitiesID")
    = 0x0000400A,
    MTRClusterColorControlAttributeColorTempPhysicalMinMiredsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMinMiredsID")
    = 0x0000400B,
    MTRClusterColorControlAttributeColorTempPhysicalMaxMiredsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeColorTempPhysicalMaxMiredsID")
    = 0x0000400C,
    MTRClusterColorControlAttributeCoupleColorTempToLevelMinMiredsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeCoupleColorTempToLevelMinMiredsID")
    = 0x0000400D,
    MTRClusterColorControlAttributeStartUpColorTemperatureMiredsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeStartUpColorTemperatureMiredsID")
    = 0x00004010,
    MTRClusterColorControlAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterColorControlAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterColorControlAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterColorControlAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterColorControlAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterColorControlAttributeClusterRevisionID")
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
    MTRClusterBallastConfigurationAttributePhysicalMinLevelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMinLevelID")
    = 0x00000000,
    MTRClusterBallastConfigurationAttributePhysicalMaxLevelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributePhysicalMaxLevelID")
    = 0x00000001,
    MTRClusterBallastConfigurationAttributeBallastStatusID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastStatusID")
    = 0x00000002,
    MTRClusterBallastConfigurationAttributeMinLevelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeMinLevelID")
    = 0x00000010,
    MTRClusterBallastConfigurationAttributeMaxLevelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeMaxLevelID")
    = 0x00000011,
    MTRClusterBallastConfigurationAttributeIntrinsicBalanceFactorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeIntrinsicBalanceFactorID")
    = 0x00000014,
    MTRClusterBallastConfigurationAttributeBallastFactorAdjustmentID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeBallastFactorAdjustmentID")
    = 0x00000015,
    MTRClusterBallastConfigurationAttributeLampQuantityID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampQuantityID")
    = 0x00000020,
    MTRClusterBallastConfigurationAttributeLampTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampTypeID")
    = 0x00000030,
    MTRClusterBallastConfigurationAttributeLampManufacturerID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampManufacturerID")
    = 0x00000031,
    MTRClusterBallastConfigurationAttributeLampRatedHoursID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampRatedHoursID")
    = 0x00000032,
    MTRClusterBallastConfigurationAttributeLampBurnHoursID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursID")
    = 0x00000033,
    MTRClusterBallastConfigurationAttributeLampAlarmModeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampAlarmModeID")
    = 0x00000034,
    MTRClusterBallastConfigurationAttributeLampBurnHoursTripPointID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeLampBurnHoursTripPointID")
    = 0x00000035,
    MTRClusterBallastConfigurationAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterBallastConfigurationAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterBallastConfigurationAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterBallastConfigurationAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterBallastConfigurationAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterBallastConfigurationAttributeClusterRevisionID")
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
    MTRClusterIlluminanceMeasurementAttributeMeasuredValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMeasuredValueID")
    = 0x00000000,
    MTRClusterIlluminanceMeasurementAttributeMinMeasuredValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMinMeasuredValueID")
    = 0x00000001,
    MTRClusterIlluminanceMeasurementAttributeMaxMeasuredValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeMaxMeasuredValueID")
    = 0x00000002,
    MTRClusterIlluminanceMeasurementAttributeToleranceID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeToleranceID")
    = 0x00000003,
    MTRClusterIlluminanceMeasurementAttributeLightSensorTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeLightSensorTypeID")
    = 0x00000004,
    MTRClusterIlluminanceMeasurementAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterIlluminanceMeasurementAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterIlluminanceMeasurementAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterIlluminanceMeasurementAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterIlluminanceMeasurementAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterIlluminanceMeasurementAttributeClusterRevisionID")
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
    MTRClusterTemperatureMeasurementAttributeMeasuredValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMeasuredValueID")
    = 0x00000000,
    MTRClusterTemperatureMeasurementAttributeMinMeasuredValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMinMeasuredValueID")
    = 0x00000001,
    MTRClusterTemperatureMeasurementAttributeMaxMeasuredValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeMaxMeasuredValueID")
    = 0x00000002,
    MTRClusterTemperatureMeasurementAttributeToleranceID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeToleranceID")
    = 0x00000003,
    MTRClusterTemperatureMeasurementAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTemperatureMeasurementAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTemperatureMeasurementAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTemperatureMeasurementAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTemperatureMeasurementAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTemperatureMeasurementAttributeClusterRevisionID")
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
    MTRClusterPressureMeasurementAttributeMeasuredValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMeasuredValueID")
    = 0x00000000,
    MTRClusterPressureMeasurementAttributeMinMeasuredValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMinMeasuredValueID")
    = 0x00000001,
    MTRClusterPressureMeasurementAttributeMaxMeasuredValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxMeasuredValueID")
    = 0x00000002,
    MTRClusterPressureMeasurementAttributeToleranceID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeToleranceID")
    = 0x00000003,
    MTRClusterPressureMeasurementAttributeScaledValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledValueID")
    = 0x00000010,
    MTRClusterPressureMeasurementAttributeMinScaledValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMinScaledValueID")
    = 0x00000011,
    MTRClusterPressureMeasurementAttributeMaxScaledValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeMaxScaledValueID")
    = 0x00000012,
    MTRClusterPressureMeasurementAttributeScaledToleranceID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeScaledToleranceID")
    = 0x00000013,
    MTRClusterPressureMeasurementAttributeScaleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeScaleID")
    = 0x00000014,
    MTRClusterPressureMeasurementAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterPressureMeasurementAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterPressureMeasurementAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterPressureMeasurementAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterPressureMeasurementAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterPressureMeasurementAttributeClusterRevisionID")
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
    MTRClusterFlowMeasurementAttributeMeasuredValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeMeasuredValueID")
    = 0x00000000,
    MTRClusterFlowMeasurementAttributeMinMeasuredValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeMinMeasuredValueID")
    = 0x00000001,
    MTRClusterFlowMeasurementAttributeMaxMeasuredValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeMaxMeasuredValueID")
    = 0x00000002,
    MTRClusterFlowMeasurementAttributeToleranceID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeToleranceID")
    = 0x00000003,
    MTRClusterFlowMeasurementAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterFlowMeasurementAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterFlowMeasurementAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterFlowMeasurementAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterFlowMeasurementAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFlowMeasurementAttributeClusterRevisionID")
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
    MTRClusterRelativeHumidityMeasurementAttributeMeasuredValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMeasuredValueID")
    = 0x00000000,
    MTRClusterRelativeHumidityMeasurementAttributeMinMeasuredValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMinMeasuredValueID")
    = 0x00000001,
    MTRClusterRelativeHumidityMeasurementAttributeMaxMeasuredValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeMaxMeasuredValueID")
    = 0x00000002,
    MTRClusterRelativeHumidityMeasurementAttributeToleranceID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeToleranceID")
    = 0x00000003,
    MTRClusterRelativeHumidityMeasurementAttributeGeneratedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterRelativeHumidityMeasurementAttributeAcceptedCommandListID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterRelativeHumidityMeasurementAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterRelativeHumidityMeasurementAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterRelativeHumidityMeasurementAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterRelativeHumidityMeasurementAttributeClusterRevisionID")
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
    MTRClusterOccupancySensingAttributeOccupancyID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancyID")
    = 0x00000000,
    MTRClusterOccupancySensingAttributeOccupancySensorTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeID")
    = 0x00000001,
    MTRClusterOccupancySensingAttributeOccupancySensorTypeBitmapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeOccupancySensorTypeBitmapID")
    = 0x00000002,
    MTRClusterOccupancySensingAttributePirOccupiedToUnoccupiedDelayID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributePIROccupiedToUnoccupiedDelayID")
    = 0x00000010,
    MTRClusterOccupancySensingAttributePirUnoccupiedToOccupiedDelayID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedDelayID")
    = 0x00000011,
    MTRClusterOccupancySensingAttributePirUnoccupiedToOccupiedThresholdID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributePIRUnoccupiedToOccupiedThresholdID")
    = 0x00000012,
    MTRClusterOccupancySensingAttributeUltrasonicOccupiedToUnoccupiedDelayID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicOccupiedToUnoccupiedDelayID")
    = 0x00000020,
    MTRClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedDelayID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedDelayID")
    = 0x00000021,
    MTRClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedThresholdID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterOccupancySensingAttributeUltrasonicUnoccupiedToOccupiedThresholdID")
    = 0x00000022,
    MTRClusterOccupancySensingAttributePhysicalContactOccupiedToUnoccupiedDelayID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactOccupiedToUnoccupiedDelayID")
    = 0x00000030,
    MTRClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedDelayID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedDelayID")
    = 0x00000031,
    MTRClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedThresholdID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterOccupancySensingAttributePhysicalContactUnoccupiedToOccupiedThresholdID")
    = 0x00000032,
    MTRClusterOccupancySensingAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterOccupancySensingAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterOccupancySensingAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterOccupancySensingAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterOccupancySensingAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterOccupancySensingAttributeClusterRevisionID")
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
    MTRClusterWakeOnLanAttributeMACAddressID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWakeOnLANAttributeMACAddressID")
    = 0x00000000,
    MTRClusterWakeOnLanAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWakeOnLANAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterWakeOnLanAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWakeOnLANAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterWakeOnLanAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWakeOnLANAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterWakeOnLanAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWakeOnLANAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterWakeOnLanAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterWakeOnLANAttributeClusterRevisionID")
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
    MTRClusterChannelAttributeChannelListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeChannelListID")
    = 0x00000000,
    MTRClusterChannelAttributeLineupID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeLineupID")
    = 0x00000001,
    MTRClusterChannelAttributeCurrentChannelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeCurrentChannelID")
    = 0x00000002,
    MTRClusterChannelAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterChannelAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterChannelAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterChannelAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterChannelAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterChannelAttributeClusterRevisionID")
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
    MTRClusterTargetNavigatorAttributeTargetListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeTargetListID")
    = 0x00000000,
    MTRClusterTargetNavigatorAttributeCurrentTargetID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeCurrentTargetID")
    = 0x00000001,
    MTRClusterTargetNavigatorAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTargetNavigatorAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTargetNavigatorAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTargetNavigatorAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTargetNavigatorAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterTargetNavigatorAttributeClusterRevisionID")
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
    MTRClusterMediaPlaybackAttributeCurrentStateID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeCurrentStateID")
    = 0x00000000,
    MTRClusterMediaPlaybackAttributeStartTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeStartTimeID")
    = 0x00000001,
    MTRClusterMediaPlaybackAttributeDurationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeDurationID")
    = 0x00000002,
    MTRClusterMediaPlaybackAttributeSampledPositionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeSampledPositionID")
    = 0x00000003,
    MTRClusterMediaPlaybackAttributePlaybackSpeedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributePlaybackSpeedID")
    = 0x00000004,
    MTRClusterMediaPlaybackAttributeSeekRangeEndID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeEndID")
    = 0x00000005,
    MTRClusterMediaPlaybackAttributeSeekRangeStartID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeSeekRangeStartID")
    = 0x00000006,
    MTRClusterMediaPlaybackAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterMediaPlaybackAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterMediaPlaybackAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterMediaPlaybackAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterMediaPlaybackAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaPlaybackAttributeClusterRevisionID")
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
    MTRClusterMediaInputAttributeInputListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeInputListID")
    = 0x00000000,
    MTRClusterMediaInputAttributeCurrentInputID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeCurrentInputID")
    = 0x00000001,
    MTRClusterMediaInputAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterMediaInputAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterMediaInputAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterMediaInputAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterMediaInputAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterMediaInputAttributeClusterRevisionID")
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
    MTRClusterLowPowerAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLowPowerAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterLowPowerAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLowPowerAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterLowPowerAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLowPowerAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterLowPowerAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLowPowerAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterLowPowerAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterLowPowerAttributeClusterRevisionID")
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
    MTRClusterKeypadInputAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterKeypadInputAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterKeypadInputAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterKeypadInputAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterKeypadInputAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterKeypadInputAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterKeypadInputAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterKeypadInputAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterKeypadInputAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterKeypadInputAttributeClusterRevisionID")
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
    MTRClusterContentLauncherAttributeAcceptHeaderID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterContentLauncherAttributeAcceptHeaderID")
    = 0x00000000,
    MTRClusterContentLauncherAttributeSupportedStreamingProtocolsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterContentLauncherAttributeSupportedStreamingProtocolsID")
    = 0x00000001,
    MTRClusterContentLauncherAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterContentLauncherAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterContentLauncherAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterContentLauncherAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterContentLauncherAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterContentLauncherAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterContentLauncherAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterContentLauncherAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterContentLauncherAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterContentLauncherAttributeClusterRevisionID")
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
    MTRClusterAudioOutputAttributeOutputListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAudioOutputAttributeOutputListID")
    = 0x00000000,
    MTRClusterAudioOutputAttributeCurrentOutputID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAudioOutputAttributeCurrentOutputID")
    = 0x00000001,
    MTRClusterAudioOutputAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAudioOutputAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAudioOutputAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAudioOutputAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAudioOutputAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAudioOutputAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAudioOutputAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAudioOutputAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAudioOutputAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAudioOutputAttributeClusterRevisionID")
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
    MTRClusterApplicationLauncherAttributeCatalogListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeCatalogListID")
    = 0x00000000,
    MTRClusterApplicationLauncherAttributeCurrentAppID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeCurrentAppID")
    = 0x00000001,
    MTRClusterApplicationLauncherAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterApplicationLauncherAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterApplicationLauncherAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterApplicationLauncherAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterApplicationLauncherAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationLauncherAttributeClusterRevisionID")
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
    MTRClusterApplicationBasicAttributeVendorNameID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeVendorNameID")
    = 0x00000000,
    MTRClusterApplicationBasicAttributeVendorIDID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeVendorIDID")
    = 0x00000001,
    MTRClusterApplicationBasicAttributeApplicationNameID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationNameID")
    = 0x00000002,
    MTRClusterApplicationBasicAttributeProductIDID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeProductIDID")
    = 0x00000003,
    MTRClusterApplicationBasicAttributeApplicationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationID")
    = 0x00000004,
    MTRClusterApplicationBasicAttributeStatusID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeStatusID")
    = 0x00000005,
    MTRClusterApplicationBasicAttributeApplicationVersionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeApplicationVersionID")
    = 0x00000006,
    MTRClusterApplicationBasicAttributeAllowedVendorListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeAllowedVendorListID")
    = 0x00000007,
    MTRClusterApplicationBasicAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterApplicationBasicAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterApplicationBasicAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterApplicationBasicAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterApplicationBasicAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterApplicationBasicAttributeClusterRevisionID")
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
    MTRClusterAccountLoginAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccountLoginAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterAccountLoginAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccountLoginAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterAccountLoginAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccountLoginAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterAccountLoginAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccountLoginAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterAccountLoginAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterAccountLoginAttributeClusterRevisionID")
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
    MTRClusterElectricalMeasurementAttributeMeasurementTypeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasurementTypeID")
    = 0x00000000,
    MTRClusterElectricalMeasurementAttributeDcVoltageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageID")
    = 0x00000100,
    MTRClusterElectricalMeasurementAttributeDcVoltageMinID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMinID")
    = 0x00000101,
    MTRClusterElectricalMeasurementAttributeDcVoltageMaxID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMaxID")
    = 0x00000102,
    MTRClusterElectricalMeasurementAttributeDcCurrentID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentID")
    = 0x00000103,
    MTRClusterElectricalMeasurementAttributeDcCurrentMinID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMinID")
    = 0x00000104,
    MTRClusterElectricalMeasurementAttributeDcCurrentMaxID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMaxID")
    = 0x00000105,
    MTRClusterElectricalMeasurementAttributeDcPowerID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerID")
    = 0x00000106,
    MTRClusterElectricalMeasurementAttributeDcPowerMinID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMinID")
    = 0x00000107,
    MTRClusterElectricalMeasurementAttributeDcPowerMaxID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMaxID")
    = 0x00000108,
    MTRClusterElectricalMeasurementAttributeDcVoltageMultiplierID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageMultiplierID")
    = 0x00000200,
    MTRClusterElectricalMeasurementAttributeDcVoltageDivisorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcVoltageDivisorID")
    = 0x00000201,
    MTRClusterElectricalMeasurementAttributeDcCurrentMultiplierID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentMultiplierID")
    = 0x00000202,
    MTRClusterElectricalMeasurementAttributeDcCurrentDivisorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcCurrentDivisorID")
    = 0x00000203,
    MTRClusterElectricalMeasurementAttributeDcPowerMultiplierID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerMultiplierID")
    = 0x00000204,
    MTRClusterElectricalMeasurementAttributeDcPowerDivisorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeDcPowerDivisorID")
    = 0x00000205,
    MTRClusterElectricalMeasurementAttributeAcFrequencyID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyID")
    = 0x00000300,
    MTRClusterElectricalMeasurementAttributeAcFrequencyMinID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMinID")
    = 0x00000301,
    MTRClusterElectricalMeasurementAttributeAcFrequencyMaxID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMaxID")
    = 0x00000302,
    MTRClusterElectricalMeasurementAttributeNeutralCurrentID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeNeutralCurrentID")
    = 0x00000303,
    MTRClusterElectricalMeasurementAttributeTotalActivePowerID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalActivePowerID")
    = 0x00000304,
    MTRClusterElectricalMeasurementAttributeTotalReactivePowerID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalReactivePowerID")
    = 0x00000305,
    MTRClusterElectricalMeasurementAttributeTotalApparentPowerID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeTotalApparentPowerID")
    = 0x00000306,
    MTRClusterElectricalMeasurementAttributeMeasured1stHarmonicCurrentID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured1stHarmonicCurrentID")
    = 0x00000307,
    MTRClusterElectricalMeasurementAttributeMeasured3rdHarmonicCurrentID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured3rdHarmonicCurrentID")
    = 0x00000308,
    MTRClusterElectricalMeasurementAttributeMeasured5thHarmonicCurrentID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured5thHarmonicCurrentID")
    = 0x00000309,
    MTRClusterElectricalMeasurementAttributeMeasured7thHarmonicCurrentID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured7thHarmonicCurrentID")
    = 0x0000030A,
    MTRClusterElectricalMeasurementAttributeMeasured9thHarmonicCurrentID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured9thHarmonicCurrentID")
    = 0x0000030B,
    MTRClusterElectricalMeasurementAttributeMeasured11thHarmonicCurrentID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasured11thHarmonicCurrentID")
    = 0x0000030C,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase1stHarmonicCurrentID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase1stHarmonicCurrentID")
    = 0x0000030D,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase3rdHarmonicCurrentID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase3rdHarmonicCurrentID")
    = 0x0000030E,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase5thHarmonicCurrentID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase5thHarmonicCurrentID")
    = 0x0000030F,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase7thHarmonicCurrentID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase7thHarmonicCurrentID")
    = 0x00000310,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase9thHarmonicCurrentID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase9thHarmonicCurrentID")
    = 0x00000311,
    MTRClusterElectricalMeasurementAttributeMeasuredPhase11thHarmonicCurrentID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeMeasuredPhase11thHarmonicCurrentID")
    = 0x00000312,
    MTRClusterElectricalMeasurementAttributeAcFrequencyMultiplierID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyMultiplierID")
    = 0x00000400,
    MTRClusterElectricalMeasurementAttributeAcFrequencyDivisorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcFrequencyDivisorID")
    = 0x00000401,
    MTRClusterElectricalMeasurementAttributePowerMultiplierID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerMultiplierID")
    = 0x00000402,
    MTRClusterElectricalMeasurementAttributePowerDivisorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerDivisorID")
    = 0x00000403,
    MTRClusterElectricalMeasurementAttributeHarmonicCurrentMultiplierID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeHarmonicCurrentMultiplierID")
    = 0x00000404,
    MTRClusterElectricalMeasurementAttributePhaseHarmonicCurrentMultiplierID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePhaseHarmonicCurrentMultiplierID")
    = 0x00000405,
    MTRClusterElectricalMeasurementAttributeInstantaneousVoltageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousVoltageID")
    = 0x00000500,
    MTRClusterElectricalMeasurementAttributeInstantaneousLineCurrentID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousLineCurrentID")
    = 0x00000501,
    MTRClusterElectricalMeasurementAttributeInstantaneousActiveCurrentID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousActiveCurrentID")
    = 0x00000502,
    MTRClusterElectricalMeasurementAttributeInstantaneousReactiveCurrentID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousReactiveCurrentID")
    = 0x00000503,
    MTRClusterElectricalMeasurementAttributeInstantaneousPowerID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeInstantaneousPowerID")
    = 0x00000504,
    MTRClusterElectricalMeasurementAttributeRmsVoltageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageID")
    = 0x00000505,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMinID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinID")
    = 0x00000506,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMaxID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxID")
    = 0x00000507,
    MTRClusterElectricalMeasurementAttributeRmsCurrentID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentID")
    = 0x00000508,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMinID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinID")
    = 0x00000509,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMaxID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxID")
    = 0x0000050A,
    MTRClusterElectricalMeasurementAttributeActivePowerID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerID")
    = 0x0000050B,
    MTRClusterElectricalMeasurementAttributeActivePowerMinID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinID")
    = 0x0000050C,
    MTRClusterElectricalMeasurementAttributeActivePowerMaxID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxID")
    = 0x0000050D,
    MTRClusterElectricalMeasurementAttributeReactivePowerID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerID")
    = 0x0000050E,
    MTRClusterElectricalMeasurementAttributeApparentPowerID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerID")
    = 0x0000050F,
    MTRClusterElectricalMeasurementAttributePowerFactorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorID")
    = 0x00000510,
    MTRClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodID")
    = 0x00000511,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterID")
    = 0x00000513,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodID")
    = 0x00000514,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodID")
    = 0x00000515,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagPeriodID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodID")
    = 0x00000516,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodID")
    = 0x00000517,
    MTRClusterElectricalMeasurementAttributeAcVoltageMultiplierID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageMultiplierID")
    = 0x00000600,
    MTRClusterElectricalMeasurementAttributeAcVoltageDivisorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageDivisorID")
    = 0x00000601,
    MTRClusterElectricalMeasurementAttributeAcCurrentMultiplierID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentMultiplierID")
    = 0x00000602,
    MTRClusterElectricalMeasurementAttributeAcCurrentDivisorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentDivisorID")
    = 0x00000603,
    MTRClusterElectricalMeasurementAttributeAcPowerMultiplierID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerMultiplierID")
    = 0x00000604,
    MTRClusterElectricalMeasurementAttributeAcPowerDivisorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcPowerDivisorID")
    = 0x00000605,
    MTRClusterElectricalMeasurementAttributeOverloadAlarmsMaskID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeOverloadAlarmsMaskID")
    = 0x00000700,
    MTRClusterElectricalMeasurementAttributeVoltageOverloadID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeVoltageOverloadID")
    = 0x00000701,
    MTRClusterElectricalMeasurementAttributeCurrentOverloadID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeCurrentOverloadID")
    = 0x00000702,
    MTRClusterElectricalMeasurementAttributeAcOverloadAlarmsMaskID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcOverloadAlarmsMaskID")
    = 0x00000800,
    MTRClusterElectricalMeasurementAttributeAcVoltageOverloadID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcVoltageOverloadID")
    = 0x00000801,
    MTRClusterElectricalMeasurementAttributeAcCurrentOverloadID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcCurrentOverloadID")
    = 0x00000802,
    MTRClusterElectricalMeasurementAttributeAcActivePowerOverloadID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcActivePowerOverloadID")
    = 0x00000803,
    MTRClusterElectricalMeasurementAttributeAcReactivePowerOverloadID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcReactivePowerOverloadID")
    = 0x00000804,
    MTRClusterElectricalMeasurementAttributeAverageRmsOverVoltageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageID")
    = 0x00000805,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageID")
    = 0x00000806,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltageID")
    = 0x00000807,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltageID")
    = 0x00000808,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagID")
    = 0x00000809,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellID")
    = 0x0000080A,
    MTRClusterElectricalMeasurementAttributeLineCurrentPhaseBID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseBID")
    = 0x00000901,
    MTRClusterElectricalMeasurementAttributeActiveCurrentPhaseBID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseBID")
    = 0x00000902,
    MTRClusterElectricalMeasurementAttributeReactiveCurrentPhaseBID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseBID")
    = 0x00000903,
    MTRClusterElectricalMeasurementAttributeRmsVoltagePhaseBID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseBID")
    = 0x00000905,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMinPhaseBID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseBID")
    = 0x00000906,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseBID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseBID")
    = 0x00000907,
    MTRClusterElectricalMeasurementAttributeRmsCurrentPhaseBID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseBID")
    = 0x00000908,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMinPhaseBID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseBID")
    = 0x00000909,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseBID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseBID")
    = 0x0000090A,
    MTRClusterElectricalMeasurementAttributeActivePowerPhaseBID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseBID")
    = 0x0000090B,
    MTRClusterElectricalMeasurementAttributeActivePowerMinPhaseBID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseBID")
    = 0x0000090C,
    MTRClusterElectricalMeasurementAttributeActivePowerMaxPhaseBID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseBID")
    = 0x0000090D,
    MTRClusterElectricalMeasurementAttributeReactivePowerPhaseBID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseBID")
    = 0x0000090E,
    MTRClusterElectricalMeasurementAttributeApparentPowerPhaseBID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseBID")
    = 0x0000090F,
    MTRClusterElectricalMeasurementAttributePowerFactorPhaseBID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseBID")
    = 0x00000910,
    MTRClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseBID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseBID")
    = 0x00000911,
    MTRClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseBID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseBID")
    = 0x00000912,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseBID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseBID")
    = 0x00000913,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseBID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseBID")
    = 0x00000914,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseBID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseBID")
    = 0x00000915,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseBID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseBID")
    = 0x00000916,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseBID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseBID")
    = 0x00000917,
    MTRClusterElectricalMeasurementAttributeLineCurrentPhaseCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeLineCurrentPhaseCID")
    = 0x00000A01,
    MTRClusterElectricalMeasurementAttributeActiveCurrentPhaseCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActiveCurrentPhaseCID")
    = 0x00000A02,
    MTRClusterElectricalMeasurementAttributeReactiveCurrentPhaseCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactiveCurrentPhaseCID")
    = 0x00000A03,
    MTRClusterElectricalMeasurementAttributeRmsVoltagePhaseCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltagePhaseCID")
    = 0x00000A05,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMinPhaseCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMinPhaseCID")
    = 0x00000A06,
    MTRClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageMaxPhaseCID")
    = 0x00000A07,
    MTRClusterElectricalMeasurementAttributeRmsCurrentPhaseCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentPhaseCID")
    = 0x00000A08,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMinPhaseCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMinPhaseCID")
    = 0x00000A09,
    MTRClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsCurrentMaxPhaseCID")
    = 0x00000A0A,
    MTRClusterElectricalMeasurementAttributeActivePowerPhaseCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerPhaseCID")
    = 0x00000A0B,
    MTRClusterElectricalMeasurementAttributeActivePowerMinPhaseCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMinPhaseCID")
    = 0x00000A0C,
    MTRClusterElectricalMeasurementAttributeActivePowerMaxPhaseCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeActivePowerMaxPhaseCID")
    = 0x00000A0D,
    MTRClusterElectricalMeasurementAttributeReactivePowerPhaseCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeReactivePowerPhaseCID")
    = 0x00000A0E,
    MTRClusterElectricalMeasurementAttributeApparentPowerPhaseCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeApparentPowerPhaseCID")
    = 0x00000A0F,
    MTRClusterElectricalMeasurementAttributePowerFactorPhaseCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributePowerFactorPhaseCID")
    = 0x00000A10,
    MTRClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseCID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsVoltageMeasurementPeriodPhaseCID")
    = 0x00000A11,
    MTRClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseCID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsOverVoltageCounterPhaseCID")
    = 0x00000A12,
    MTRClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseCID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAverageRmsUnderVoltageCounterPhaseCID")
    = 0x00000A13,
    MTRClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseCID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeOverVoltagePeriodPhaseCID")
    = 0x00000A14,
    MTRClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseCID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED(
            "Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsExtremeUnderVoltagePeriodPhaseCID")
    = 0x00000A15,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseCID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSagPeriodPhaseCID")
    = 0x00000A16,
    MTRClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseCID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeRmsVoltageSwellPeriodPhaseCID")
    = 0x00000A17,
    MTRClusterElectricalMeasurementAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterElectricalMeasurementAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterElectricalMeasurementAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterElectricalMeasurementAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterElectricalMeasurementAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterElectricalMeasurementAttributeClusterRevisionID")
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

    // Cluster TestCluster deprecated attribute names
    MTRClusterTestClusterAttributeBooleanID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeBooleanID")
    = 0x00000000,
    MTRClusterTestClusterAttributeBitmap8ID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap8ID")
    = 0x00000001,
    MTRClusterTestClusterAttributeBitmap16ID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap16ID")
    = 0x00000002,
    MTRClusterTestClusterAttributeBitmap32ID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap32ID")
    = 0x00000003,
    MTRClusterTestClusterAttributeBitmap64ID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeBitmap64ID")
    = 0x00000004,
    MTRClusterTestClusterAttributeInt8uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt8uID")
    = 0x00000005,
    MTRClusterTestClusterAttributeInt16uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt16uID")
    = 0x00000006,
    MTRClusterTestClusterAttributeInt24uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt24uID")
    = 0x00000007,
    MTRClusterTestClusterAttributeInt32uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt32uID")
    = 0x00000008,
    MTRClusterTestClusterAttributeInt40uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt40uID")
    = 0x00000009,
    MTRClusterTestClusterAttributeInt48uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt48uID")
    = 0x0000000A,
    MTRClusterTestClusterAttributeInt56uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt56uID")
    = 0x0000000B,
    MTRClusterTestClusterAttributeInt64uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt64uID")
    = 0x0000000C,
    MTRClusterTestClusterAttributeInt8sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt8sID")
    = 0x0000000D,
    MTRClusterTestClusterAttributeInt16sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt16sID")
    = 0x0000000E,
    MTRClusterTestClusterAttributeInt24sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt24sID")
    = 0x0000000F,
    MTRClusterTestClusterAttributeInt32sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt32sID")
    = 0x00000010,
    MTRClusterTestClusterAttributeInt40sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt40sID")
    = 0x00000011,
    MTRClusterTestClusterAttributeInt48sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt48sID")
    = 0x00000012,
    MTRClusterTestClusterAttributeInt56sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt56sID")
    = 0x00000013,
    MTRClusterTestClusterAttributeInt64sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeInt64sID")
    = 0x00000014,
    MTRClusterTestClusterAttributeEnum8ID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEnum8ID")
    = 0x00000015,
    MTRClusterTestClusterAttributeEnum16ID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEnum16ID")
    = 0x00000016,
    MTRClusterTestClusterAttributeFloatSingleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeFloatSingleID")
    = 0x00000017,
    MTRClusterTestClusterAttributeFloatDoubleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeFloatDoubleID")
    = 0x00000018,
    MTRClusterTestClusterAttributeOctetStringID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeOctetStringID")
    = 0x00000019,
    MTRClusterTestClusterAttributeListInt8uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeListInt8uID")
    = 0x0000001A,
    MTRClusterTestClusterAttributeListOctetStringID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeListOctetStringID")
    = 0x0000001B,
    MTRClusterTestClusterAttributeListStructOctetStringID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeListStructOctetStringID")
    = 0x0000001C,
    MTRClusterTestClusterAttributeLongOctetStringID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeLongOctetStringID")
    = 0x0000001D,
    MTRClusterTestClusterAttributeCharStringID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeCharStringID")
    = 0x0000001E,
    MTRClusterTestClusterAttributeLongCharStringID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeLongCharStringID")
    = 0x0000001F,
    MTRClusterTestClusterAttributeEpochUsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEpochUsID")
    = 0x00000020,
    MTRClusterTestClusterAttributeEpochSID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEpochSID")
    = 0x00000021,
    MTRClusterTestClusterAttributeVendorIdID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeVendorIdID")
    = 0x00000022,
    MTRClusterTestClusterAttributeListNullablesAndOptionalsStructID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeListNullablesAndOptionalsStructID")
    = 0x00000023,
    MTRClusterTestClusterAttributeEnumAttrID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeEnumAttrID")
    = 0x00000024,
    MTRClusterTestClusterAttributeStructAttrID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeStructAttrID")
    = 0x00000025,
    MTRClusterTestClusterAttributeRangeRestrictedInt8uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8uID")
    = 0x00000026,
    MTRClusterTestClusterAttributeRangeRestrictedInt8sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt8sID")
    = 0x00000027,
    MTRClusterTestClusterAttributeRangeRestrictedInt16uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16uID")
    = 0x00000028,
    MTRClusterTestClusterAttributeRangeRestrictedInt16sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeRangeRestrictedInt16sID")
    = 0x00000029,
    MTRClusterTestClusterAttributeListLongOctetStringID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeListLongOctetStringID")
    = 0x0000002A,
    MTRClusterTestClusterAttributeListFabricScopedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeListFabricScopedID")
    = 0x0000002B,
    MTRClusterTestClusterAttributeTimedWriteBooleanID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeTimedWriteBooleanID")
    = 0x00000030,
    MTRClusterTestClusterAttributeGeneralErrorBooleanID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeGeneralErrorBooleanID")
    = 0x00000031,
    MTRClusterTestClusterAttributeClusterErrorBooleanID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeClusterErrorBooleanID")
    = 0x00000032,
    MTRClusterTestClusterAttributeUnsupportedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeUnsupportedID")
    = 0x000000FF,
    MTRClusterTestClusterAttributeNullableBooleanID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBooleanID")
    = 0x00004000,
    MTRClusterTestClusterAttributeNullableBitmap8ID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap8ID")
    = 0x00004001,
    MTRClusterTestClusterAttributeNullableBitmap16ID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap16ID")
    = 0x00004002,
    MTRClusterTestClusterAttributeNullableBitmap32ID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap32ID")
    = 0x00004003,
    MTRClusterTestClusterAttributeNullableBitmap64ID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableBitmap64ID")
    = 0x00004004,
    MTRClusterTestClusterAttributeNullableInt8uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8uID")
    = 0x00004005,
    MTRClusterTestClusterAttributeNullableInt16uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16uID")
    = 0x00004006,
    MTRClusterTestClusterAttributeNullableInt24uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24uID")
    = 0x00004007,
    MTRClusterTestClusterAttributeNullableInt32uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32uID")
    = 0x00004008,
    MTRClusterTestClusterAttributeNullableInt40uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40uID")
    = 0x00004009,
    MTRClusterTestClusterAttributeNullableInt48uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48uID")
    = 0x0000400A,
    MTRClusterTestClusterAttributeNullableInt56uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56uID")
    = 0x0000400B,
    MTRClusterTestClusterAttributeNullableInt64uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64uID")
    = 0x0000400C,
    MTRClusterTestClusterAttributeNullableInt8sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt8sID")
    = 0x0000400D,
    MTRClusterTestClusterAttributeNullableInt16sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt16sID")
    = 0x0000400E,
    MTRClusterTestClusterAttributeNullableInt24sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt24sID")
    = 0x0000400F,
    MTRClusterTestClusterAttributeNullableInt32sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt32sID")
    = 0x00004010,
    MTRClusterTestClusterAttributeNullableInt40sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt40sID")
    = 0x00004011,
    MTRClusterTestClusterAttributeNullableInt48sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt48sID")
    = 0x00004012,
    MTRClusterTestClusterAttributeNullableInt56sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt56sID")
    = 0x00004013,
    MTRClusterTestClusterAttributeNullableInt64sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableInt64sID")
    = 0x00004014,
    MTRClusterTestClusterAttributeNullableEnum8ID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum8ID")
    = 0x00004015,
    MTRClusterTestClusterAttributeNullableEnum16ID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnum16ID")
    = 0x00004016,
    MTRClusterTestClusterAttributeNullableFloatSingleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatSingleID")
    = 0x00004017,
    MTRClusterTestClusterAttributeNullableFloatDoubleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableFloatDoubleID")
    = 0x00004018,
    MTRClusterTestClusterAttributeNullableOctetStringID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableOctetStringID")
    = 0x00004019,
    MTRClusterTestClusterAttributeNullableCharStringID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableCharStringID")
    = 0x0000401E,
    MTRClusterTestClusterAttributeNullableEnumAttrID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableEnumAttrID")
    = 0x00004024,
    MTRClusterTestClusterAttributeNullableStructID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableStructID")
    = 0x00004025,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt8uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8uID")
    = 0x00004026,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt8sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt8sID")
    = 0x00004027,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt16uID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16uID")
    = 0x00004028,
    MTRClusterTestClusterAttributeNullableRangeRestrictedInt16sID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeNullableRangeRestrictedInt16sID")
    = 0x00004029,
    MTRClusterTestClusterAttributeWriteOnlyInt8uID API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeWriteOnlyInt8uID")
    = 0x0000402A,
    MTRClusterTestClusterAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterTestClusterAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterTestClusterAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterTestClusterAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterTestClusterAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterUnitTestingAttributeClusterRevisionID")
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
    MTRClusterFaultInjectionAttributeGeneratedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFaultInjectionAttributeGeneratedCommandListID")
    = MTRClusterGlobalAttributeGeneratedCommandListID,
    MTRClusterFaultInjectionAttributeAcceptedCommandListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFaultInjectionAttributeAcceptedCommandListID")
    = MTRClusterGlobalAttributeAcceptedCommandListID,
    MTRClusterFaultInjectionAttributeAttributeListID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFaultInjectionAttributeAttributeListID")
    = MTRClusterGlobalAttributeAttributeListID,
    MTRClusterFaultInjectionAttributeFeatureMapID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFaultInjectionAttributeFeatureMapID")
    = MTRClusterGlobalAttributeFeatureMapID,
    MTRClusterFaultInjectionAttributeClusterRevisionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRAttributeIDTypeClusterFaultInjectionAttributeClusterRevisionID")
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
    MTRClusterIdentifyCommandIdentifyID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterIdentifyCommandIdentifyID")
    = 0x00000000,
    MTRClusterIdentifyCommandTriggerEffectID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterIdentifyCommandTriggerEffectID")
    = 0x00000040,

    // Cluster Identify commands
    MTRCommandIDTypeClusterIdentifyCommandIdentifyID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterIdentifyCommandTriggerEffectID MTR_NEWLY_AVAILABLE = 0x00000040,

    // Cluster Groups deprecated command id names
    MTRClusterGroupsCommandAddGroupID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandAddGroupID")
    = 0x00000000,
    MTRClusterGroupsCommandAddGroupResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandAddGroupResponseID")
    = 0x00000000,
    MTRClusterGroupsCommandViewGroupID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandViewGroupID")
    = 0x00000001,
    MTRClusterGroupsCommandViewGroupResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandViewGroupResponseID")
    = 0x00000001,
    MTRClusterGroupsCommandGetGroupMembershipID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipID")
    = 0x00000002,
    MTRClusterGroupsCommandGetGroupMembershipResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandGetGroupMembershipResponseID")
    = 0x00000002,
    MTRClusterGroupsCommandRemoveGroupID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandRemoveGroupID")
    = 0x00000003,
    MTRClusterGroupsCommandRemoveGroupResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandRemoveGroupResponseID")
    = 0x00000003,
    MTRClusterGroupsCommandRemoveAllGroupsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandRemoveAllGroupsID")
    = 0x00000004,
    MTRClusterGroupsCommandAddGroupIfIdentifyingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupsCommandAddGroupIfIdentifyingID")
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
    MTRClusterScenesCommandAddSceneID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandAddSceneID")
    = 0x00000000,
    MTRClusterScenesCommandAddSceneResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandAddSceneResponseID")
    = 0x00000000,
    MTRClusterScenesCommandViewSceneID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandViewSceneID")
    = 0x00000001,
    MTRClusterScenesCommandViewSceneResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandViewSceneResponseID")
    = 0x00000001,
    MTRClusterScenesCommandRemoveSceneID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandRemoveSceneID")
    = 0x00000002,
    MTRClusterScenesCommandRemoveSceneResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandRemoveSceneResponseID")
    = 0x00000002,
    MTRClusterScenesCommandRemoveAllScenesID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandRemoveAllScenesID")
    = 0x00000003,
    MTRClusterScenesCommandRemoveAllScenesResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandRemoveAllScenesResponseID")
    = 0x00000003,
    MTRClusterScenesCommandStoreSceneID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandStoreSceneID")
    = 0x00000004,
    MTRClusterScenesCommandStoreSceneResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandStoreSceneResponseID")
    = 0x00000004,
    MTRClusterScenesCommandRecallSceneID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandRecallSceneID")
    = 0x00000005,
    MTRClusterScenesCommandGetSceneMembershipID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandGetSceneMembershipID")
    = 0x00000006,
    MTRClusterScenesCommandGetSceneMembershipResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandGetSceneMembershipResponseID")
    = 0x00000006,
    MTRClusterScenesCommandEnhancedAddSceneID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneID")
    = 0x00000040,
    MTRClusterScenesCommandEnhancedAddSceneResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandEnhancedAddSceneResponseID")
    = 0x00000040,
    MTRClusterScenesCommandEnhancedViewSceneID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneID")
    = 0x00000041,
    MTRClusterScenesCommandEnhancedViewSceneResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandEnhancedViewSceneResponseID")
    = 0x00000041,
    MTRClusterScenesCommandCopySceneID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandCopySceneID")
    = 0x00000042,
    MTRClusterScenesCommandCopySceneResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterScenesCommandCopySceneResponseID")
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
    MTRClusterOnOffCommandOffID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOffID")
    = 0x00000000,
    MTRClusterOnOffCommandOnID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOnID")
    = 0x00000001,
    MTRClusterOnOffCommandToggleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandToggleID")
    = 0x00000002,
    MTRClusterOnOffCommandOffWithEffectID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOffWithEffectID")
    = 0x00000040,
    MTRClusterOnOffCommandOnWithRecallGlobalSceneID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOnWithRecallGlobalSceneID")
    = 0x00000041,
    MTRClusterOnOffCommandOnWithTimedOffID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOnOffCommandOnWithTimedOffID")
    = 0x00000042,

    // Cluster OnOff commands
    MTRCommandIDTypeClusterOnOffCommandOffID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterOnOffCommandOnID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterOnOffCommandToggleID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterOnOffCommandOffWithEffectID MTR_NEWLY_AVAILABLE = 0x00000040,
    MTRCommandIDTypeClusterOnOffCommandOnWithRecallGlobalSceneID MTR_NEWLY_AVAILABLE = 0x00000041,
    MTRCommandIDTypeClusterOnOffCommandOnWithTimedOffID MTR_NEWLY_AVAILABLE = 0x00000042,

    // Cluster LevelControl deprecated command id names
    MTRClusterLevelControlCommandMoveToLevelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandMoveToLevelID")
    = 0x00000000,
    MTRClusterLevelControlCommandMoveID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandMoveID")
    = 0x00000001,
    MTRClusterLevelControlCommandStepID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandStepID")
    = 0x00000002,
    MTRClusterLevelControlCommandStopID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandStopID")
    = 0x00000003,
    MTRClusterLevelControlCommandMoveToLevelWithOnOffID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandMoveToLevelWithOnOffID")
    = 0x00000004,
    MTRClusterLevelControlCommandMoveWithOnOffID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandMoveWithOnOffID")
    = 0x00000005,
    MTRClusterLevelControlCommandStepWithOnOffID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandStepWithOnOffID")
    = 0x00000006,
    MTRClusterLevelControlCommandStopWithOnOffID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandStopWithOnOffID")
    = 0x00000007,
    MTRClusterLevelControlCommandMoveToClosestFrequencyID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterLevelControlCommandMoveToClosestFrequencyID")
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
    MTRClusterActionsCommandInstantActionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandInstantActionID")
    = 0x00000000,
    MTRClusterActionsCommandInstantActionWithTransitionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandInstantActionWithTransitionID")
    = 0x00000001,
    MTRClusterActionsCommandStartActionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandStartActionID")
    = 0x00000002,
    MTRClusterActionsCommandStartActionWithDurationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandStartActionWithDurationID")
    = 0x00000003,
    MTRClusterActionsCommandStopActionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandStopActionID")
    = 0x00000004,
    MTRClusterActionsCommandPauseActionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandPauseActionID")
    = 0x00000005,
    MTRClusterActionsCommandPauseActionWithDurationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandPauseActionWithDurationID")
    = 0x00000006,
    MTRClusterActionsCommandResumeActionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandResumeActionID")
    = 0x00000007,
    MTRClusterActionsCommandEnableActionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandEnableActionID")
    = 0x00000008,
    MTRClusterActionsCommandEnableActionWithDurationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandEnableActionWithDurationID")
    = 0x00000009,
    MTRClusterActionsCommandDisableActionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandDisableActionID")
    = 0x0000000A,
    MTRClusterActionsCommandDisableActionWithDurationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterActionsCommandDisableActionWithDurationID")
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
    MTRClusterBasicCommandMfgSpecificPingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterBasicInformationCommandMfgSpecificPingID")
    = 0x10020000,

    // Cluster BasicInformation commands

    // Cluster OtaSoftwareUpdateProvider deprecated command id names
    MTRClusterOtaSoftwareUpdateProviderCommandQueryImageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageID")
    = 0x00000000,
    MTRClusterOtaSoftwareUpdateProviderCommandQueryImageResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageResponseID")
    = 0x00000001,
    MTRClusterOtaSoftwareUpdateProviderCommandApplyUpdateRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateRequestID")
    = 0x00000002,
    MTRClusterOtaSoftwareUpdateProviderCommandApplyUpdateResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateResponseID")
    = 0x00000003,
    MTRClusterOtaSoftwareUpdateProviderCommandNotifyUpdateAppliedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandNotifyUpdateAppliedID")
    = 0x00000004,

    // Cluster OTASoftwareUpdateProvider commands
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandQueryImageResponseID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateRequestID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandApplyUpdateResponseID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterOTASoftwareUpdateProviderCommandNotifyUpdateAppliedID MTR_NEWLY_AVAILABLE = 0x00000004,

    // Cluster OtaSoftwareUpdateRequestor deprecated command id names
    MTRClusterOtaSoftwareUpdateRequestorCommandAnnounceOtaProviderID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOTASoftwareUpdateRequestorCommandAnnounceOTAProviderID")
    = 0x00000000,

    // Cluster OTASoftwareUpdateRequestor commands
    MTRCommandIDTypeClusterOTASoftwareUpdateRequestorCommandAnnounceOTAProviderID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster GeneralCommissioning deprecated command id names
    MTRClusterGeneralCommissioningCommandArmFailSafeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeID")
    = 0x00000000,
    MTRClusterGeneralCommissioningCommandArmFailSafeResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeResponseID")
    = 0x00000001,
    MTRClusterGeneralCommissioningCommandSetRegulatoryConfigID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigID")
    = 0x00000002,
    MTRClusterGeneralCommissioningCommandSetRegulatoryConfigResponseID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigResponseID")
    = 0x00000003,
    MTRClusterGeneralCommissioningCommandCommissioningCompleteID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteID")
    = 0x00000004,
    MTRClusterGeneralCommissioningCommandCommissioningCompleteResponseID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteResponseID")
    = 0x00000005,

    // Cluster GeneralCommissioning commands
    MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterGeneralCommissioningCommandArmFailSafeResponseID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterGeneralCommissioningCommandSetRegulatoryConfigResponseID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterGeneralCommissioningCommandCommissioningCompleteResponseID MTR_NEWLY_AVAILABLE = 0x00000005,

    // Cluster NetworkCommissioning deprecated command id names
    MTRClusterNetworkCommissioningCommandScanNetworksID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksID")
    = 0x00000000,
    MTRClusterNetworkCommissioningCommandScanNetworksResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandScanNetworksResponseID")
    = 0x00000001,
    MTRClusterNetworkCommissioningCommandAddOrUpdateWiFiNetworkID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateWiFiNetworkID")
    = 0x00000002,
    MTRClusterNetworkCommissioningCommandAddOrUpdateThreadNetworkID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandAddOrUpdateThreadNetworkID")
    = 0x00000003,
    MTRClusterNetworkCommissioningCommandRemoveNetworkID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandRemoveNetworkID")
    = 0x00000004,
    MTRClusterNetworkCommissioningCommandNetworkConfigResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandNetworkConfigResponseID")
    = 0x00000005,
    MTRClusterNetworkCommissioningCommandConnectNetworkID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkID")
    = 0x00000006,
    MTRClusterNetworkCommissioningCommandConnectNetworkResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandConnectNetworkResponseID")
    = 0x00000007,
    MTRClusterNetworkCommissioningCommandReorderNetworkID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterNetworkCommissioningCommandReorderNetworkID")
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
    MTRClusterDiagnosticLogsCommandRetrieveLogsRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsRequestID")
    = 0x00000000,
    MTRClusterDiagnosticLogsCommandRetrieveLogsResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsResponseID")
    = 0x00000001,

    // Cluster DiagnosticLogs commands
    MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsRequestID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterDiagnosticLogsCommandRetrieveLogsResponseID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster GeneralDiagnostics deprecated command id names
    MTRClusterGeneralDiagnosticsCommandTestEventTriggerID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGeneralDiagnosticsCommandTestEventTriggerID")
    = 0x00000000,

    // Cluster GeneralDiagnostics commands
    MTRCommandIDTypeClusterGeneralDiagnosticsCommandTestEventTriggerID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster SoftwareDiagnostics deprecated command id names
    MTRClusterSoftwareDiagnosticsCommandResetWatermarksID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterSoftwareDiagnosticsCommandResetWatermarksID")
    = 0x00000000,

    // Cluster SoftwareDiagnostics commands
    MTRCommandIDTypeClusterSoftwareDiagnosticsCommandResetWatermarksID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster ThreadNetworkDiagnostics deprecated command id names
    MTRClusterThreadNetworkDiagnosticsCommandResetCountsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterThreadNetworkDiagnosticsCommandResetCountsID")
    = 0x00000000,

    // Cluster ThreadNetworkDiagnostics commands
    MTRCommandIDTypeClusterThreadNetworkDiagnosticsCommandResetCountsID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster WiFiNetworkDiagnostics deprecated command id names
    MTRClusterWiFiNetworkDiagnosticsCommandResetCountsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterWiFiNetworkDiagnosticsCommandResetCountsID")
    = 0x00000000,

    // Cluster WiFiNetworkDiagnostics commands
    MTRCommandIDTypeClusterWiFiNetworkDiagnosticsCommandResetCountsID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster EthernetNetworkDiagnostics deprecated command id names
    MTRClusterEthernetNetworkDiagnosticsCommandResetCountsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterEthernetNetworkDiagnosticsCommandResetCountsID")
    = 0x00000000,

    // Cluster EthernetNetworkDiagnostics commands
    MTRCommandIDTypeClusterEthernetNetworkDiagnosticsCommandResetCountsID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster TimeSynchronization deprecated command id names
    MTRClusterTimeSynchronizationCommandSetUtcTimeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterTimeSynchronizationCommandSetUtcTimeID")
    = 0x00000000,

    // Cluster TimeSynchronization commands
    MTRCommandIDTypeClusterTimeSynchronizationCommandSetUtcTimeID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster AdministratorCommissioning deprecated command id names
    MTRClusterAdministratorCommissioningCommandOpenCommissioningWindowID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenCommissioningWindowID")
    = 0x00000000,
    MTRClusterAdministratorCommissioningCommandOpenBasicCommissioningWindowID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenBasicCommissioningWindowID")
    = 0x00000001,
    MTRClusterAdministratorCommissioningCommandRevokeCommissioningID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterAdministratorCommissioningCommandRevokeCommissioningID")
    = 0x00000002,

    // Cluster AdministratorCommissioning commands
    MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenCommissioningWindowID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterAdministratorCommissioningCommandOpenBasicCommissioningWindowID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterAdministratorCommissioningCommandRevokeCommissioningID MTR_NEWLY_AVAILABLE = 0x00000002,

    // Cluster OperationalCredentials deprecated command id names
    MTRClusterOperationalCredentialsCommandAttestationRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationRequestID")
    = 0x00000000,
    MTRClusterOperationalCredentialsCommandAttestationResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAttestationResponseID")
    = 0x00000001,
    MTRClusterOperationalCredentialsCommandCertificateChainRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainRequestID")
    = 0x00000002,
    MTRClusterOperationalCredentialsCommandCertificateChainResponseID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCertificateChainResponseID")
    = 0x00000003,
    MTRClusterOperationalCredentialsCommandCSRRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCSRRequestID")
    = 0x00000004,
    MTRClusterOperationalCredentialsCommandCSRResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandCSRResponseID")
    = 0x00000005,
    MTRClusterOperationalCredentialsCommandAddNOCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAddNOCID")
    = 0x00000006,
    MTRClusterOperationalCredentialsCommandUpdateNOCID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateNOCID")
    = 0x00000007,
    MTRClusterOperationalCredentialsCommandNOCResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandNOCResponseID")
    = 0x00000008,
    MTRClusterOperationalCredentialsCommandUpdateFabricLabelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandUpdateFabricLabelID")
    = 0x00000009,
    MTRClusterOperationalCredentialsCommandRemoveFabricID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandRemoveFabricID")
    = 0x0000000A,
    MTRClusterOperationalCredentialsCommandAddTrustedRootCertificateID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterOperationalCredentialsCommandAddTrustedRootCertificateID")
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
    MTRClusterGroupKeyManagementCommandKeySetWriteID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetWriteID")
    = 0x00000000,
    MTRClusterGroupKeyManagementCommandKeySetReadID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadID")
    = 0x00000001,
    MTRClusterGroupKeyManagementCommandKeySetReadResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadResponseID")
    = 0x00000002,
    MTRClusterGroupKeyManagementCommandKeySetRemoveID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetRemoveID")
    = 0x00000003,
    MTRClusterGroupKeyManagementCommandKeySetReadAllIndicesID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesID")
    = 0x00000004,
    MTRClusterGroupKeyManagementCommandKeySetReadAllIndicesResponseID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesResponseID")
    = 0x00000005,

    // Cluster GroupKeyManagement commands
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetWriteID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadResponseID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetRemoveID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesID MTR_NEWLY_AVAILABLE = 0x00000004,
    MTRCommandIDTypeClusterGroupKeyManagementCommandKeySetReadAllIndicesResponseID MTR_NEWLY_AVAILABLE = 0x00000005,

    // Cluster ModeSelect deprecated command id names
    MTRClusterModeSelectCommandChangeToModeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterModeSelectCommandChangeToModeID")
    = 0x00000000,

    // Cluster ModeSelect commands
    MTRCommandIDTypeClusterModeSelectCommandChangeToModeID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster DoorLock deprecated command id names
    MTRClusterDoorLockCommandLockDoorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandLockDoorID")
    = 0x00000000,
    MTRClusterDoorLockCommandUnlockDoorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandUnlockDoorID")
    = 0x00000001,
    MTRClusterDoorLockCommandUnlockWithTimeoutID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandUnlockWithTimeoutID")
    = 0x00000003,
    MTRClusterDoorLockCommandSetWeekDayScheduleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandSetWeekDayScheduleID")
    = 0x0000000B,
    MTRClusterDoorLockCommandGetWeekDayScheduleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleID")
    = 0x0000000C,
    MTRClusterDoorLockCommandGetWeekDayScheduleResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetWeekDayScheduleResponseID")
    = 0x0000000C,
    MTRClusterDoorLockCommandClearWeekDayScheduleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandClearWeekDayScheduleID")
    = 0x0000000D,
    MTRClusterDoorLockCommandSetYearDayScheduleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandSetYearDayScheduleID")
    = 0x0000000E,
    MTRClusterDoorLockCommandGetYearDayScheduleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleID")
    = 0x0000000F,
    MTRClusterDoorLockCommandGetYearDayScheduleResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetYearDayScheduleResponseID")
    = 0x0000000F,
    MTRClusterDoorLockCommandClearYearDayScheduleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandClearYearDayScheduleID")
    = 0x00000010,
    MTRClusterDoorLockCommandSetHolidayScheduleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandSetHolidayScheduleID")
    = 0x00000011,
    MTRClusterDoorLockCommandGetHolidayScheduleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleID")
    = 0x00000012,
    MTRClusterDoorLockCommandGetHolidayScheduleResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetHolidayScheduleResponseID")
    = 0x00000012,
    MTRClusterDoorLockCommandClearHolidayScheduleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandClearHolidayScheduleID")
    = 0x00000013,
    MTRClusterDoorLockCommandSetUserID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandSetUserID")
    = 0x0000001A,
    MTRClusterDoorLockCommandGetUserID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetUserID")
    = 0x0000001B,
    MTRClusterDoorLockCommandGetUserResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetUserResponseID")
    = 0x0000001C,
    MTRClusterDoorLockCommandClearUserID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandClearUserID")
    = 0x0000001D,
    MTRClusterDoorLockCommandSetCredentialID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandSetCredentialID")
    = 0x00000022,
    MTRClusterDoorLockCommandSetCredentialResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandSetCredentialResponseID")
    = 0x00000023,
    MTRClusterDoorLockCommandGetCredentialStatusID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusID")
    = 0x00000024,
    MTRClusterDoorLockCommandGetCredentialStatusResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandGetCredentialStatusResponseID")
    = 0x00000025,
    MTRClusterDoorLockCommandClearCredentialID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterDoorLockCommandClearCredentialID")
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
    MTRClusterWindowCoveringCommandUpOrOpenID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterWindowCoveringCommandUpOrOpenID")
    = 0x00000000,
    MTRClusterWindowCoveringCommandDownOrCloseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterWindowCoveringCommandDownOrCloseID")
    = 0x00000001,
    MTRClusterWindowCoveringCommandStopMotionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterWindowCoveringCommandStopMotionID")
    = 0x00000002,
    MTRClusterWindowCoveringCommandGoToLiftValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftValueID")
    = 0x00000004,
    MTRClusterWindowCoveringCommandGoToLiftPercentageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToLiftPercentageID")
    = 0x00000005,
    MTRClusterWindowCoveringCommandGoToTiltValueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltValueID")
    = 0x00000007,
    MTRClusterWindowCoveringCommandGoToTiltPercentageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterWindowCoveringCommandGoToTiltPercentageID")
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
    MTRClusterBarrierControlCommandBarrierControlGoToPercentID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterBarrierControlCommandBarrierControlGoToPercentID")
    = 0x00000000,
    MTRClusterBarrierControlCommandBarrierControlStopID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterBarrierControlCommandBarrierControlStopID")
    = 0x00000001,

    // Cluster BarrierControl commands
    MTRCommandIDTypeClusterBarrierControlCommandBarrierControlGoToPercentID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterBarrierControlCommandBarrierControlStopID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster Thermostat deprecated command id names
    MTRClusterThermostatCommandSetpointRaiseLowerID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterThermostatCommandSetpointRaiseLowerID")
    = 0x00000000,
    MTRClusterThermostatCommandGetWeeklyScheduleResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleResponseID")
    = 0x00000000,
    MTRClusterThermostatCommandSetWeeklyScheduleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterThermostatCommandSetWeeklyScheduleID")
    = 0x00000001,
    MTRClusterThermostatCommandGetWeeklyScheduleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleID")
    = 0x00000002,
    MTRClusterThermostatCommandClearWeeklyScheduleID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterThermostatCommandClearWeeklyScheduleID")
    = 0x00000003,

    // Cluster Thermostat commands
    MTRCommandIDTypeClusterThermostatCommandSetpointRaiseLowerID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleResponseID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterThermostatCommandSetWeeklyScheduleID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterThermostatCommandGetWeeklyScheduleID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterThermostatCommandClearWeeklyScheduleID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster ColorControl deprecated command id names
    MTRClusterColorControlCommandMoveToHueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveToHueID")
    = 0x00000000,
    MTRClusterColorControlCommandMoveHueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveHueID")
    = 0x00000001,
    MTRClusterColorControlCommandStepHueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandStepHueID")
    = 0x00000002,
    MTRClusterColorControlCommandMoveToSaturationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveToSaturationID")
    = 0x00000003,
    MTRClusterColorControlCommandMoveSaturationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveSaturationID")
    = 0x00000004,
    MTRClusterColorControlCommandStepSaturationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandStepSaturationID")
    = 0x00000005,
    MTRClusterColorControlCommandMoveToHueAndSaturationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveToHueAndSaturationID")
    = 0x00000006,
    MTRClusterColorControlCommandMoveToColorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveToColorID")
    = 0x00000007,
    MTRClusterColorControlCommandMoveColorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveColorID")
    = 0x00000008,
    MTRClusterColorControlCommandStepColorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandStepColorID")
    = 0x00000009,
    MTRClusterColorControlCommandMoveToColorTemperatureID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveToColorTemperatureID")
    = 0x0000000A,
    MTRClusterColorControlCommandEnhancedMoveToHueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueID")
    = 0x00000040,
    MTRClusterColorControlCommandEnhancedMoveHueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandEnhancedMoveHueID")
    = 0x00000041,
    MTRClusterColorControlCommandEnhancedStepHueID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandEnhancedStepHueID")
    = 0x00000042,
    MTRClusterColorControlCommandEnhancedMoveToHueAndSaturationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandEnhancedMoveToHueAndSaturationID")
    = 0x00000043,
    MTRClusterColorControlCommandColorLoopSetID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandColorLoopSetID")
    = 0x00000044,
    MTRClusterColorControlCommandStopMoveStepID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandStopMoveStepID")
    = 0x00000047,
    MTRClusterColorControlCommandMoveColorTemperatureID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandMoveColorTemperatureID")
    = 0x0000004B,
    MTRClusterColorControlCommandStepColorTemperatureID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterColorControlCommandStepColorTemperatureID")
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
    MTRClusterChannelCommandChangeChannelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterChannelCommandChangeChannelID")
    = 0x00000000,
    MTRClusterChannelCommandChangeChannelResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterChannelCommandChangeChannelResponseID")
    = 0x00000001,
    MTRClusterChannelCommandChangeChannelByNumberID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterChannelCommandChangeChannelByNumberID")
    = 0x00000002,
    MTRClusterChannelCommandSkipChannelID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterChannelCommandSkipChannelID")
    = 0x00000003,

    // Cluster Channel commands
    MTRCommandIDTypeClusterChannelCommandChangeChannelID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterChannelCommandChangeChannelResponseID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterChannelCommandChangeChannelByNumberID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterChannelCommandSkipChannelID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster TargetNavigator deprecated command id names
    MTRClusterTargetNavigatorCommandNavigateTargetID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetID")
    = 0x00000000,
    MTRClusterTargetNavigatorCommandNavigateTargetResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetResponseID")
    = 0x00000001,

    // Cluster TargetNavigator commands
    MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterTargetNavigatorCommandNavigateTargetResponseID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster MediaPlayback deprecated command id names
    MTRClusterMediaPlaybackCommandPlayID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandPlayID")
    = 0x00000000,
    MTRClusterMediaPlaybackCommandPauseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandPauseID")
    = 0x00000001,
    MTRClusterMediaPlaybackCommandStopPlaybackID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandStopPlaybackID")
    = 0x00000002,
    MTRClusterMediaPlaybackCommandStartOverID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandStartOverID")
    = 0x00000003,
    MTRClusterMediaPlaybackCommandPreviousID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandPreviousID")
    = 0x00000004,
    MTRClusterMediaPlaybackCommandNextID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandNextID")
    = 0x00000005,
    MTRClusterMediaPlaybackCommandRewindID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandRewindID")
    = 0x00000006,
    MTRClusterMediaPlaybackCommandFastForwardID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandFastForwardID")
    = 0x00000007,
    MTRClusterMediaPlaybackCommandSkipForwardID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandSkipForwardID")
    = 0x00000008,
    MTRClusterMediaPlaybackCommandSkipBackwardID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandSkipBackwardID")
    = 0x00000009,
    MTRClusterMediaPlaybackCommandPlaybackResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandPlaybackResponseID")
    = 0x0000000A,
    MTRClusterMediaPlaybackCommandSeekID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaPlaybackCommandSeekID")
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
    MTRClusterMediaInputCommandSelectInputID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaInputCommandSelectInputID")
    = 0x00000000,
    MTRClusterMediaInputCommandShowInputStatusID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaInputCommandShowInputStatusID")
    = 0x00000001,
    MTRClusterMediaInputCommandHideInputStatusID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaInputCommandHideInputStatusID")
    = 0x00000002,
    MTRClusterMediaInputCommandRenameInputID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterMediaInputCommandRenameInputID")
    = 0x00000003,

    // Cluster MediaInput commands
    MTRCommandIDTypeClusterMediaInputCommandSelectInputID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterMediaInputCommandShowInputStatusID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterMediaInputCommandHideInputStatusID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterMediaInputCommandRenameInputID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster LowPower deprecated command id names
    MTRClusterLowPowerCommandSleepID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterLowPowerCommandSleepID")
    = 0x00000000,

    // Cluster LowPower commands
    MTRCommandIDTypeClusterLowPowerCommandSleepID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster KeypadInput deprecated command id names
    MTRClusterKeypadInputCommandSendKeyID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterKeypadInputCommandSendKeyID")
    = 0x00000000,
    MTRClusterKeypadInputCommandSendKeyResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterKeypadInputCommandSendKeyResponseID")
    = 0x00000001,

    // Cluster KeypadInput commands
    MTRCommandIDTypeClusterKeypadInputCommandSendKeyID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterKeypadInputCommandSendKeyResponseID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster ContentLauncher deprecated command id names
    MTRClusterContentLauncherCommandLaunchContentID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterContentLauncherCommandLaunchContentID")
    = 0x00000000,
    MTRClusterContentLauncherCommandLaunchURLID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterContentLauncherCommandLaunchURLID")
    = 0x00000001,
    MTRClusterContentLauncherCommandLaunchResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterContentLauncherCommandLaunchResponseID")
    = 0x00000002,

    // Cluster ContentLauncher commands
    MTRCommandIDTypeClusterContentLauncherCommandLaunchContentID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterContentLauncherCommandLaunchURLID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterContentLauncherCommandLaunchResponseID MTR_NEWLY_AVAILABLE = 0x00000002,

    // Cluster AudioOutput deprecated command id names
    MTRClusterAudioOutputCommandSelectOutputID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterAudioOutputCommandSelectOutputID")
    = 0x00000000,
    MTRClusterAudioOutputCommandRenameOutputID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterAudioOutputCommandRenameOutputID")
    = 0x00000001,

    // Cluster AudioOutput commands
    MTRCommandIDTypeClusterAudioOutputCommandSelectOutputID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterAudioOutputCommandRenameOutputID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster ApplicationLauncher deprecated command id names
    MTRClusterApplicationLauncherCommandLaunchAppID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterApplicationLauncherCommandLaunchAppID")
    = 0x00000000,
    MTRClusterApplicationLauncherCommandStopAppID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterApplicationLauncherCommandStopAppID")
    = 0x00000001,
    MTRClusterApplicationLauncherCommandHideAppID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterApplicationLauncherCommandHideAppID")
    = 0x00000002,
    MTRClusterApplicationLauncherCommandLauncherResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterApplicationLauncherCommandLauncherResponseID")
    = 0x00000003,

    // Cluster ApplicationLauncher commands
    MTRCommandIDTypeClusterApplicationLauncherCommandLaunchAppID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterApplicationLauncherCommandStopAppID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterApplicationLauncherCommandHideAppID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterApplicationLauncherCommandLauncherResponseID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster AccountLogin deprecated command id names
    MTRClusterAccountLoginCommandGetSetupPINID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINID")
    = 0x00000000,
    MTRClusterAccountLoginCommandGetSetupPINResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINResponseID")
    = 0x00000001,
    MTRClusterAccountLoginCommandLoginID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterAccountLoginCommandLoginID")
    = 0x00000002,
    MTRClusterAccountLoginCommandLogoutID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterAccountLoginCommandLogoutID")
    = 0x00000003,

    // Cluster AccountLogin commands
    MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterAccountLoginCommandGetSetupPINResponseID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterAccountLoginCommandLoginID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTRCommandIDTypeClusterAccountLoginCommandLogoutID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster ElectricalMeasurement deprecated command id names
    MTRClusterElectricalMeasurementCommandGetProfileInfoResponseCommandID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoResponseCommandID")
    = 0x00000000,
    MTRClusterElectricalMeasurementCommandGetProfileInfoCommandID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoCommandID")
    = 0x00000000,
    MTRClusterElectricalMeasurementCommandGetMeasurementProfileResponseCommandID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileResponseCommandID")
    = 0x00000001,
    MTRClusterElectricalMeasurementCommandGetMeasurementProfileCommandID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileCommandID")
    = 0x00000001,

    // Cluster ElectricalMeasurement commands
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoResponseCommandID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetProfileInfoCommandID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileResponseCommandID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTRCommandIDTypeClusterElectricalMeasurementCommandGetMeasurementProfileCommandID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster TestCluster deprecated command id names
    MTRClusterTestClusterCommandTestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestID")
    = 0x00000000,
    MTRClusterTestClusterCommandTestSpecificResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestSpecificResponseID")
    = 0x00000000,
    MTRClusterTestClusterCommandTestNotHandledID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestNotHandledID")
    = 0x00000001,
    MTRClusterTestClusterCommandTestAddArgumentsResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsResponseID")
    = 0x00000001,
    MTRClusterTestClusterCommandTestSpecificID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestSpecificID")
    = 0x00000002,
    MTRClusterTestClusterCommandTestSimpleArgumentResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentResponseID")
    = 0x00000002,
    MTRClusterTestClusterCommandTestUnknownCommandID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestUnknownCommandID")
    = 0x00000003,
    MTRClusterTestClusterCommandTestStructArrayArgumentResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentResponseID")
    = 0x00000003,
    MTRClusterTestClusterCommandTestAddArgumentsID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestAddArgumentsID")
    = 0x00000004,
    MTRClusterTestClusterCommandTestListInt8UReverseResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseResponseID")
    = 0x00000004,
    MTRClusterTestClusterCommandTestSimpleArgumentRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestSimpleArgumentRequestID")
    = 0x00000005,
    MTRClusterTestClusterCommandTestEnumsResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestEnumsResponseID")
    = 0x00000005,
    MTRClusterTestClusterCommandTestStructArrayArgumentRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestStructArrayArgumentRequestID")
    = 0x00000006,
    MTRClusterTestClusterCommandTestNullableOptionalResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalResponseID")
    = 0x00000006,
    MTRClusterTestClusterCommandTestStructArgumentRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestStructArgumentRequestID")
    = 0x00000007,
    MTRClusterTestClusterCommandTestComplexNullableOptionalResponseID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalResponseID")
    = 0x00000007,
    MTRClusterTestClusterCommandTestNestedStructArgumentRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructArgumentRequestID")
    = 0x00000008,
    MTRClusterTestClusterCommandBooleanResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandBooleanResponseID")
    = 0x00000008,
    MTRClusterTestClusterCommandTestListStructArgumentRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestListStructArgumentRequestID")
    = 0x00000009,
    MTRClusterTestClusterCommandSimpleStructResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandSimpleStructResponseID")
    = 0x00000009,
    MTRClusterTestClusterCommandTestListInt8UArgumentRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UArgumentRequestID")
    = 0x0000000A,
    MTRClusterTestClusterCommandTestEmitTestEventResponseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventResponseID")
    = 0x0000000A,
    MTRClusterTestClusterCommandTestNestedStructListArgumentRequestID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestNestedStructListArgumentRequestID")
    = 0x0000000B,
    MTRClusterTestClusterCommandTestEmitTestFabricScopedEventResponseID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventResponseID")
    = 0x0000000B,
    MTRClusterTestClusterCommandTestListNestedStructListArgumentRequestID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestListNestedStructListArgumentRequestID")
    = 0x0000000C,
    MTRClusterTestClusterCommandTestListInt8UReverseRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestListInt8UReverseRequestID")
    = 0x0000000D,
    MTRClusterTestClusterCommandTestEnumsRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestEnumsRequestID")
    = 0x0000000E,
    MTRClusterTestClusterCommandTestNullableOptionalRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestNullableOptionalRequestID")
    = 0x0000000F,
    MTRClusterTestClusterCommandTestComplexNullableOptionalRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestComplexNullableOptionalRequestID")
    = 0x00000010,
    MTRClusterTestClusterCommandSimpleStructEchoRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandSimpleStructEchoRequestID")
    = 0x00000011,
    MTRClusterTestClusterCommandTimedInvokeRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTimedInvokeRequestID")
    = 0x00000012,
    MTRClusterTestClusterCommandTestSimpleOptionalArgumentRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestSimpleOptionalArgumentRequestID")
    = 0x00000013,
    MTRClusterTestClusterCommandTestEmitTestEventRequestID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestEventRequestID")
    = 0x00000014,
    MTRClusterTestClusterCommandTestEmitTestFabricScopedEventRequestID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterUnitTestingCommandTestEmitTestFabricScopedEventRequestID")
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
    MTRClusterFaultInjectionCommandFailAtFaultID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterFaultInjectionCommandFailAtFaultID")
    = 0x00000000,
    MTRClusterFaultInjectionCommandFailRandomlyAtFaultID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTRCommandIDTypeClusterFaultInjectionCommandFailRandomlyAtFaultID")
    = 0x00000001,

    // Cluster FaultInjection commands
    MTRCommandIDTypeClusterFaultInjectionCommandFailAtFaultID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTRCommandIDTypeClusterFaultInjectionCommandFailRandomlyAtFaultID MTR_NEWLY_AVAILABLE = 0x00000001,

};

#pragma mark - Events IDs

typedef NS_ENUM(uint32_t, MTREventIDType) {

    // Cluster AccessControl deprecated event names
    MTRClusterAccessControlEventAccessControlEntryChangedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterAccessControlEventAccessControlEntryChangedID")
    = 0x00000000,
    MTRClusterAccessControlEventAccessControlExtensionChangedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterAccessControlEventAccessControlExtensionChangedID")
    = 0x00000001,

    // Cluster AccessControl events
    MTREventIDTypeClusterAccessControlEventAccessControlEntryChangedID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterAccessControlEventAccessControlExtensionChangedID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster Actions deprecated event names
    MTRClusterActionsEventStateChangedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterActionsEventStateChangedID")
    = 0x00000000,
    MTRClusterActionsEventActionFailedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterActionsEventActionFailedID")
    = 0x00000001,

    // Cluster Actions events
    MTREventIDTypeClusterActionsEventStateChangedID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterActionsEventActionFailedID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster Basic deprecated event names
    MTRClusterBasicEventStartUpID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterBasicInformationEventStartUpID")
    = 0x00000000,
    MTRClusterBasicEventShutDownID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterBasicInformationEventShutDownID")
    = 0x00000001,
    MTRClusterBasicEventLeaveID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterBasicInformationEventLeaveID")
    = 0x00000002,
    MTRClusterBasicEventReachableChangedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterBasicInformationEventReachableChangedID")
    = 0x00000003,

    // Cluster BasicInformation events
    MTREventIDTypeClusterBasicInformationEventStartUpID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterBasicInformationEventShutDownID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterBasicInformationEventLeaveID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTREventIDTypeClusterBasicInformationEventReachableChangedID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster OtaSoftwareUpdateRequestor deprecated event names
    MTRClusterOtaSoftwareUpdateRequestorEventStateTransitionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterOTASoftwareUpdateRequestorEventStateTransitionID")
    = 0x00000000,
    MTRClusterOtaSoftwareUpdateRequestorEventVersionAppliedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterOTASoftwareUpdateRequestorEventVersionAppliedID")
    = 0x00000001,
    MTRClusterOtaSoftwareUpdateRequestorEventDownloadErrorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterOTASoftwareUpdateRequestorEventDownloadErrorID")
    = 0x00000002,

    // Cluster OTASoftwareUpdateRequestor events
    MTREventIDTypeClusterOTASoftwareUpdateRequestorEventStateTransitionID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterOTASoftwareUpdateRequestorEventVersionAppliedID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterOTASoftwareUpdateRequestorEventDownloadErrorID MTR_NEWLY_AVAILABLE = 0x00000002,

    // Cluster PowerSource deprecated event names

    // Cluster PowerSource events
    MTREventIDTypeClusterPowerSourceEventWiredFaultChangeID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterPowerSourceEventBatFaultChangeID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterPowerSourceEventBatChargeFaultChangeID MTR_NEWLY_AVAILABLE = 0x00000002,

    // Cluster GeneralDiagnostics deprecated event names
    MTRClusterGeneralDiagnosticsEventHardwareFaultChangeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterGeneralDiagnosticsEventHardwareFaultChangeID")
    = 0x00000000,
    MTRClusterGeneralDiagnosticsEventRadioFaultChangeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterGeneralDiagnosticsEventRadioFaultChangeID")
    = 0x00000001,
    MTRClusterGeneralDiagnosticsEventNetworkFaultChangeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterGeneralDiagnosticsEventNetworkFaultChangeID")
    = 0x00000002,
    MTRClusterGeneralDiagnosticsEventBootReasonID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterGeneralDiagnosticsEventBootReasonID")
    = 0x00000003,

    // Cluster GeneralDiagnostics events
    MTREventIDTypeClusterGeneralDiagnosticsEventHardwareFaultChangeID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterGeneralDiagnosticsEventRadioFaultChangeID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterGeneralDiagnosticsEventNetworkFaultChangeID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTREventIDTypeClusterGeneralDiagnosticsEventBootReasonID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster SoftwareDiagnostics deprecated event names
    MTRClusterSoftwareDiagnosticsEventSoftwareFaultID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterSoftwareDiagnosticsEventSoftwareFaultID")
    = 0x00000000,

    // Cluster SoftwareDiagnostics events
    MTREventIDTypeClusterSoftwareDiagnosticsEventSoftwareFaultID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster ThreadNetworkDiagnostics deprecated event names
    MTRClusterThreadNetworkDiagnosticsEventConnectionStatusID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterThreadNetworkDiagnosticsEventConnectionStatusID")
    = 0x00000000,
    MTRClusterThreadNetworkDiagnosticsEventNetworkFaultChangeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterThreadNetworkDiagnosticsEventNetworkFaultChangeID")
    = 0x00000001,

    // Cluster ThreadNetworkDiagnostics events
    MTREventIDTypeClusterThreadNetworkDiagnosticsEventConnectionStatusID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterThreadNetworkDiagnosticsEventNetworkFaultChangeID MTR_NEWLY_AVAILABLE = 0x00000001,

    // Cluster WiFiNetworkDiagnostics deprecated event names
    MTRClusterWiFiNetworkDiagnosticsEventDisconnectionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterWiFiNetworkDiagnosticsEventDisconnectionID")
    = 0x00000000,
    MTRClusterWiFiNetworkDiagnosticsEventAssociationFailureID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterWiFiNetworkDiagnosticsEventAssociationFailureID")
    = 0x00000001,
    MTRClusterWiFiNetworkDiagnosticsEventConnectionStatusID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterWiFiNetworkDiagnosticsEventConnectionStatusID")
    = 0x00000002,

    // Cluster WiFiNetworkDiagnostics events
    MTREventIDTypeClusterWiFiNetworkDiagnosticsEventDisconnectionID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterWiFiNetworkDiagnosticsEventAssociationFailureID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterWiFiNetworkDiagnosticsEventConnectionStatusID MTR_NEWLY_AVAILABLE = 0x00000002,

    // Cluster BridgedDeviceBasic deprecated event names
    MTRClusterBridgedDeviceBasicEventStartUpID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterBridgedDeviceBasicInformationEventStartUpID")
    = 0x00000000,
    MTRClusterBridgedDeviceBasicEventShutDownID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterBridgedDeviceBasicInformationEventShutDownID")
    = 0x00000001,
    MTRClusterBridgedDeviceBasicEventLeaveID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterBridgedDeviceBasicInformationEventLeaveID")
    = 0x00000002,
    MTRClusterBridgedDeviceBasicEventReachableChangedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterBridgedDeviceBasicInformationEventReachableChangedID")
    = 0x00000003,

    // Cluster BridgedDeviceBasicInformation events
    MTREventIDTypeClusterBridgedDeviceBasicInformationEventStartUpID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterBridgedDeviceBasicInformationEventShutDownID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterBridgedDeviceBasicInformationEventLeaveID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTREventIDTypeClusterBridgedDeviceBasicInformationEventReachableChangedID MTR_NEWLY_AVAILABLE = 0x00000003,

    // Cluster Switch deprecated event names
    MTRClusterSwitchEventSwitchLatchedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventSwitchLatchedID")
    = 0x00000000,
    MTRClusterSwitchEventInitialPressID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventInitialPressID")
    = 0x00000001,
    MTRClusterSwitchEventLongPressID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventLongPressID")
    = 0x00000002,
    MTRClusterSwitchEventShortReleaseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventShortReleaseID")
    = 0x00000003,
    MTRClusterSwitchEventLongReleaseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventLongReleaseID")
    = 0x00000004,
    MTRClusterSwitchEventMultiPressOngoingID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventMultiPressOngoingID")
    = 0x00000005,
    MTRClusterSwitchEventMultiPressCompleteID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterSwitchEventMultiPressCompleteID")
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
    MTRClusterBooleanStateEventStateChangeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterBooleanStateEventStateChangeID")
    = 0x00000000,

    // Cluster BooleanState events
    MTREventIDTypeClusterBooleanStateEventStateChangeID MTR_NEWLY_AVAILABLE = 0x00000000,

    // Cluster DoorLock deprecated event names
    MTRClusterDoorLockEventDoorLockAlarmID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventDoorLockAlarmID")
    = 0x00000000,
    MTRClusterDoorLockEventDoorStateChangeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventDoorStateChangeID")
    = 0x00000001,
    MTRClusterDoorLockEventLockOperationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventLockOperationID")
    = 0x00000002,
    MTRClusterDoorLockEventLockOperationErrorID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventLockOperationErrorID")
    = 0x00000003,
    MTRClusterDoorLockEventLockUserChangeID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterDoorLockEventLockUserChangeID")
    = 0x00000004,

    // Cluster DoorLock events
    MTREventIDTypeClusterDoorLockEventDoorLockAlarmID MTR_NEWLY_AVAILABLE = 0x00000000,
    MTREventIDTypeClusterDoorLockEventDoorStateChangeID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterDoorLockEventLockOperationID MTR_NEWLY_AVAILABLE = 0x00000002,
    MTREventIDTypeClusterDoorLockEventLockOperationErrorID MTR_NEWLY_AVAILABLE = 0x00000003,
    MTREventIDTypeClusterDoorLockEventLockUserChangeID MTR_NEWLY_AVAILABLE = 0x00000004,

    // Cluster PumpConfigurationAndControl deprecated event names
    MTRClusterPumpConfigurationAndControlEventSupplyVoltageLowID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageLowID")
    = 0x00000000,
    MTRClusterPumpConfigurationAndControlEventSupplyVoltageHighID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSupplyVoltageHighID")
    = 0x00000001,
    MTRClusterPumpConfigurationAndControlEventPowerMissingPhaseID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventPowerMissingPhaseID")
    = 0x00000002,
    MTRClusterPumpConfigurationAndControlEventSystemPressureLowID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureLowID")
    = 0x00000003,
    MTRClusterPumpConfigurationAndControlEventSystemPressureHighID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSystemPressureHighID")
    = 0x00000004,
    MTRClusterPumpConfigurationAndControlEventDryRunningID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventDryRunningID")
    = 0x00000005,
    MTRClusterPumpConfigurationAndControlEventMotorTemperatureHighID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventMotorTemperatureHighID")
    = 0x00000006,
    MTRClusterPumpConfigurationAndControlEventPumpMotorFatalFailureID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
        tvos(16.1)) MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventPumpMotorFatalFailureID")
    = 0x00000007,
    MTRClusterPumpConfigurationAndControlEventElectronicTemperatureHighID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicTemperatureHighID")
    = 0x00000008,
    MTRClusterPumpConfigurationAndControlEventPumpBlockedID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventPumpBlockedID")
    = 0x00000009,
    MTRClusterPumpConfigurationAndControlEventSensorFailureID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventSensorFailureID")
    = 0x0000000A,
    MTRClusterPumpConfigurationAndControlEventElectronicNonFatalFailureID API_AVAILABLE(
        ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicNonFatalFailureID")
    = 0x0000000B,
    MTRClusterPumpConfigurationAndControlEventElectronicFatalFailureID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1),
        tvos(16.1)) MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventElectronicFatalFailureID")
    = 0x0000000C,
    MTRClusterPumpConfigurationAndControlEventGeneralFaultID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventGeneralFaultID")
    = 0x0000000D,
    MTRClusterPumpConfigurationAndControlEventLeakageID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventLeakageID")
    = 0x0000000E,
    MTRClusterPumpConfigurationAndControlEventAirDetectionID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventAirDetectionID")
    = 0x0000000F,
    MTRClusterPumpConfigurationAndControlEventTurbineOperationID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterPumpConfigurationAndControlEventTurbineOperationID")
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
    MTRClusterTestClusterEventTestEventID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterUnitTestingEventTestEventID")
    = 0x00000001,
    MTRClusterTestClusterEventTestFabricScopedEventID API_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
        MTR_NEWLY_DEPRECATED("Please use MTREventIDTypeClusterUnitTestingEventTestFabricScopedEventID")
    = 0x00000002,

    // Cluster UnitTesting events
    MTREventIDTypeClusterUnitTestingEventTestEventID MTR_NEWLY_AVAILABLE = 0x00000001,
    MTREventIDTypeClusterUnitTestingEventTestFabricScopedEventID MTR_NEWLY_AVAILABLE = 0x00000002,

};
